# SDD：启动阶段采集应用 import 的 Native 模块并保存至应用沙箱

> 版本：v2（2026-07-27）
> 变更：输出位置改为 cacheDir、文件名防冲突设计、新增跨启动更新与预测策略

## 1. 需求分析

### 1.1 需求描述

在应用**启动阶段**，通过 `NativeModuleManager`（进程内单例，位于 `foundation/arkui/napi`）采集该应用进程 `import`/`requireNapi` 加载过的所有 Native 模块信息，保存到**应用自身沙箱 cacheDir**；文件按**跨启动更新策略**维护，使其内容尽可能准确**预测下一次启动将加载的模块集合**。

### 1.2 现状与可行性

| 要素 | 现状 | 结论 |
|------|------|------|
| 模块数据 | `NativeModuleManager` 单例内部维护 `headNativeModule_ → tailNativeModule_` 链表，每个 `NativeModule` 节点含 `name`（so 注册名）、`moduleName`（import 名）、`fileName`（so 实际路径）、`version`、`isAppModule`、`moduleLoaded` 等字段 | 数据源天然存在，无需额外埋点记录 |
| 枚举接口 | **无对外枚举接口**，链表头尾指针是 private 成员 | 需新增 public API（跨仓修改 `arkui/napi`） |
| import 入口 | 所有 import 均汇聚到 `ArkNativeEngine::LoadNativeModule` → `NativeModuleManager::LoadNativeModule`（系统模块 + 应用模块同路径） | 单点覆盖全部 import 行为 |
| 写入者 | 沙箱属应用自身，NativeModuleManager 在应用进程内 | **无需 IPC**，应用进程自写自身沙箱，无权限问题 |
| cacheDir 获取 | `ContextImpl::GetCacheDir()`（`context_impl.cpp:167`）→ `/data/storage/el2/base/cache`；已有先例：`DumpRuntimeHelper` 写该目录 | 路径获取方式成熟 |
| 启动阶段锚点 | `MainThread::HandleLaunchApplication` → `Runtime::Create` → `ApplicationImpl::PerformAppReady()`（`application_impl.cpp:51`，状态转 `APP_STATE_READY`） | 有明确的"启动完成"回调点 |

### 1.3 需求决策点（v2 已确认）

| # | 决策点 | v2 结论 |
|---|--------|---------|
| 1 | 保存位置 | **cacheDir**（`/data/storage/el2/base/cache/`）——数据本质是"可重建的预测性缓存"，语义与 cache 一致；系统清理后下一轮启动重建，无功能损失 |
| 2 | 文件命名 | **复杂化、系统托管风格**：隐藏点目录 + 命名空间 + 进程哈希 + 格式版本，见 3.2 |
| 3 | 采集时点 | 双检查点采样：PerformAppReady（启动关键模块）+ 首个 UIAbility 前台后延迟/首次后台（懒加载模块），**每轮启动仅 1 次原子写盘** |
| 4 | 更新策略 | **EWMA 评分 + 阈值预测**，跨启动增量合并，见 3.4 |
| 5 | preload 语义 | 全量记录（预加载模块确实在应用进程地址空间），不区分来源 |
| 6 | 开关 | 参数开关门控，默认关闭或仅 debug 版本开启 |

### 1.4 约束

- **跨仓依赖**：核心数据源在 `foundation/arkui/napi`，ability_runtime 无法独立完成，需 napi 仓先合入枚举接口。
- **性能**：采集与写盘必须异步（投递到非主线程），每轮启动至多 1 次落盘，不得阻塞启动主路径。
- **安全**：仅写自身沙箱；文件权限 `0600`；不写敏感信息（路径属公开信息，风险低）。
- **可重建**：文件必须允许被系统缓存清理删除，删除后按冷启动逻辑重建，不得影响功能。

## 2. 总体设计

```
应用进程（每轮启动）
┌────────────────────────────────────────────────────────────┐
│ Runtime::Create → requireNapi → ArkNativeEngine             │
│   └─ NativeModuleManager::LoadNativeModule → 模块链表（现有） │
│                                                             │
│ 检查点① PerformAppReady ──► Collector 采样（stage=app_ready）│
│ 检查点② 首Ability前台+延迟/首次后台 ──► 采样（stage=late）    │
│                                                             │
│ NativeModuleCollector（新增，appkit 侧）                     │
│   ├─ NativeModuleManager::GetLoadedModuleInfos()  ← napi 新增│
│   ├─ 读取旧 native_modules 文件 → EWMA 评分合并（3.4）        │
│   ├─ 预计算 predicted 预测集（score ≥ 阈值）                  │
│   └─ 异步原子写 cacheDir/.ohos.ability_runtime/             │
│       native_modules_<processHash>.v2.json（tmp + rename）   │
└────────────────────────────────────────────────────────────┘
```

**方案要点**：不改动加载路径、不加加载钩子（零性能侵入）；检查点快照采样；跨启动评分合并使文件收敛为"高置信预测集"。

## 3. 详细设计

### 3.1 接口变更

**（A）napi 仓新增枚举接口**（`arkui/napi/module_manager/native_module_manager.h`）：

```cpp
struct NativeModuleInfo {           // 纯数据快照，与内部 NativeModule 解耦
    std::string name;               // so 注册名 (.nm_modname)
    std::string moduleName;         // import/require 使用的模块名
    std::string fileName;           // so 实际路径
    int32_t version = 0;
    bool isAppModule = false;       // 应用模块 or 系统模块
    bool moduleLoaded = false;
};

class NativeModuleManager {
public:
    // 遍历内部链表，拷贝快照；调用方持有数据，无生命周期问题
    void GetLoadedModuleInfos(std::vector<NativeModuleInfo>& infos);
};
```

实现要点：持锁遍历 `headNativeModule_` 链表，仅拷贝字段（深拷贝字符串，因原节点 `const char*` 可能随 UnloadNativeModule 失效）。

**（B）ability_runtime 新增采集器**（`frameworks/native/appkit/app/` 新增 `native_module_collector.h/cpp`）：

```cpp
class NativeModuleCollector {
public:
    // 检查点①：采样并缓存启动关键模块（不写盘）
    static void SampleAtAppReady();
    // 检查点②：采样懒加载模块，合并评分，异步原子写盘（每轮启动唯一写盘）
    static void CollectMergeAndSave();
private:
    struct ModuleStat;                // 评分条目，见 3.4
    static bool IsEnabled();          // 参数/特性开关
    static std::string GetStorePath();// cacheDir + 防冲突文件名，见 3.2
    static bool LoadStats(std::map<std::string, ModuleStat>& stats, int64_t& appVer);
    static void MergeRound(const std::vector<NativeModuleInfo>& round,
                           std::map<std::string, ModuleStat>& stats);
    static std::string Serialize(const std::map<std::string, ModuleStat>& stats);
    static bool WriteAtomic(const std::string& path, const std::string& content);
};
```

### 3.2 输出位置与文件命名（v2 变更）

**目录**：`Context::GetCacheDir()` →

| 应用类型 | 实际路径 |
|---------|---------|
| 普通应用 | `/data/storage/el2/base/cache/` |
| 系统应用 | `/data/app/el2/<userId>/base/<bundleName>/cache/` |

**文件名**：

```
<cacheDir>/.ohos.ability_runtime/native_modules_<processHash8>.v2.json
```

防冲突设计（与开发者文件隔离）：

| 设计 | 作用 |
|------|------|
| 点前缀隐藏目录 `.ohos.ability_runtime/` | 明确"系统托管文件"语义；开发者约定不使用的命名空间；常规文件浏览不可见 |
| `native_modules_` 前缀 + 模块语义 | 可读性，便于调试定位 |
| `<processHash8>` 进程名哈希（FNV-1a，8 位 hex） | 多进程应用（:remote 等子进程）各自采集互不覆盖；避免超长进程名撑爆文件名 |
| `.v2` 格式版本后缀 | 格式演进时新旧文件天然隔离，解析方按版本路由 |
| 文件权限 `0600` | 仅应用自身可读写 |

**cache 语义**：系统/用户清理缓存后文件丢失 → 视为冷启动（3.4），下一轮启动全量重建，预测准确度暂降后随启动轮次快速收敛，无功能损失。

### 3.3 触发点与采样时序

```
Runtime::Create (main_thread.cpp:1951)
  → 业务代码执行，模块陆续 import（进链表）
  → PerformAppReady (application_impl.cpp:51)
      → Collector::SampleAtAppReady()        ← 检查点①：内存采样，stage=app_ready
  → 首个 UIAbility 前台 + 延迟（约 5s）或首次 OnBackground
      → Collector::CollectMergeAndSave()     ← 检查点②：补采 stage=late，
                                                 读旧文件 → 评分合并 → 原子写盘
```

- **每轮启动仅 1 次落盘**（检查点②）。①崩溃丢样本可接受（统计模型对单轮缺失鲁棒）。
- 采样为纯内存操作（持锁遍历链表 + 字符串拷贝），两次合计 < 1ms；写盘投递非主线程。
- 延迟窗口覆盖首帧后的懒加载模块（UI 组件、图片编解码等通常在 `onWindowStageCreate` 后才 import）。

### 3.4 跨启动更新与预测策略（v2 核心）

**问题**：每次启动加载的模块集合不完全一致——入口场景不同（launcher / deep-link / 卡片）、懒加载时机抖动。单轮快照作为预测会抖动、漏报。

**方案**：跨启动**指数加权移动平均（EWMA）评分**，文件内容从"上一轮快照"演进为"带置信度的预测集"。

#### 数据模型

每个模块维护一个统计条目：

```cpp
struct ModuleStat {
    std::string fileName;      // so 路径
    bool isAppModule;
    float score;               // ∈ [0,1]，EWMA 命中评分
    uint32_t hits;             // 累计命中轮数
    uint32_t lastSeenLaunch;   // 最后一次命中的启动序号
    uint8_t firstStage;        // 首次出现阶段：app_ready / late
};
```

#### 更新算法（每轮启动执行一次）

设本轮实际加载集合为 S_n（检查点① ∪ ②），α = 0.4（约最近 5 轮主导，可调）：

```
对 m ∈ S_n：        score ← α + (1−α)·score      // 命中奖励，快速逼近 1
对 m ∈ 历史 \ S_n：  score ← (1−α)·score          // 遗漏衰减，缓慢趋向 0
lastSeenLaunch 命中时刷新为当前启动序号
```

#### 预测与淘汰

| 规则 | 内容 |
|------|------|
| **预测集** | `predicted = { m \| score ≥ 0.5 }`，写盘时预计算输出，读方零成本直接使用 |
| **冷启动** | 首轮（无旧文件/文件被清理）：S_1 全部 score=1，直接全量入预测集 |
| **条目淘汰** | `score < 0.05` 或连续 16 轮未见 → 删除条目，文件规模有界 |
| **条数上限** | 1024 条，超出按 score 升序淘汰（防御异常膨胀） |
| **应用升级** | 记录 `appVersionCode`，不一致 → 清空重建（升级后模块集合可能剧变，历史无参考价值） |
| **损坏兜底** | JSON 解析失败 / version 字段不符 → 丢弃按冷启动重建 |

#### 策略参数（支持系统参数调优）

| 参数 | 默认 | 说明 |
|------|------|------|
| α（EWMA 系数） | 0.4 | 越大越"短视"（跟随最近几轮），越小越平滑 |
| 预测阈值 | 0.5 | 提高则预测集更保守（低误报高漏报） |
| 淘汰窗口 | 16 轮 | 长期未命中条目清理周期 |
| 检查点②延迟 | 5 s | 首个 Ability 前台后的补采等待 |

#### 收敛行为示例

- 每轮必载模块：3 轮后 score ≈ 0.94，稳定留在预测集；
- 偶发模块（30% 概率加载）：score 在 0.3~0.5 间波动，被阈值挡在预测集外（避免误报）；
- 某版本后不再加载的模块：约 5 轮后 score < 0.1，7~8 轮后淘汰。

#### 准确性验证

beta 期间每轮启动通过 HiLog/HiSysEvent 上报：

- 命中率 `hitRate = |predicted ∩ actual| / |actual|`（目标 ≥ 95%）
- 误报率 `falseRate = |predicted − actual| / |predicted|`

用于标定 α 与阈值；量产版本关闭上报。

### 3.5 输出文件格式（v2）

`<cacheDir>/.ohos.ability_runtime/native_modules_a1b2c3d4.v2.json`：

```json
{
  "version": 2,
  "bundleName": "com.example.app",
  "processName": "com.example.app",
  "appVersionCode": 1000000,
  "launchIndex": 42,
  "updatedAt": 1753507200000,
  "scenario": "launcher",
  "predicted": ["ohos.hilog", "ohos.window", "entry"],
  "modules": [
    {
      "moduleName": "ohos.hilog",
      "fileName": "/system/lib64/module/libhilog.z.so",
      "isAppModule": false,
      "score": 0.98,
      "hits": 41,
      "lastSeenLaunch": 42,
      "firstStage": "app_ready"
    },
    {
      "moduleName": "entry",
      "fileName": "/data/storage/el1/bundle/entry/libs/arm64/libentry.so",
      "isAppModule": true,
      "score": 1.0,
      "hits": 42,
      "lastSeenLaunch": 42,
      "firstStage": "app_ready"
    }
  ]
}
```

字段说明：

- `predicted`：按 3.4 阈值预计算的**下轮启动预测模块集**（读方主消费字段）
- `modules[].score/hits/lastSeenLaunch`：评分状态，跨轮累计
- `firstStage`：模块首次出现阶段，读方可区分"启动关键"与"懒加载"
- `scenario`：本轮启动入口场景（launcher/deep-link/card 等），v2 仅记录；后续版本可按场景分桶多份 profile 进一步提升预测准确度（见 5.演进方向）

### 3.6 异常与边界处理

| 场景 | 处理 |
|------|------|
| 开关关闭 | 直接返回，零开销（不采样、不读写） |
| cacheDir 不存在 | `CreateDirIfNotExist`（含 `.ohos.ability_runtime` 子目录），失败记 HiLog 放弃，不影响启动 |
| 文件被缓存清理 | 视为冷启动，首轮全量重建 |
| JSON 损坏 / version 不符 | 丢弃旧数据，按冷启动重建 |
| 应用升级（appVersionCode 变化） | 清空历史重建 |
| 写盘失败 | 记 `TAG_LOGE` + HiSysEvent 故障事件（参考 `preload_so_startup_task.cpp:134` 的 `EventReport` 模式）；旧文件保持原样（tmp+rename 保证） |
| 多进程并发 | 各进程写各自 `<processHash8>` 文件，无共享文件竞争 |
| 文件体积 | 条目上限 1024 + 淘汰规则，稳态约数十~数百 KB 级以内（实测通常 < 100KB） |

### 3.7 安全与隐私

- 仅写应用自身沙箱，无越权访问；文件 `0600`。
- 隐藏点目录 + 系统命名空间，不与开发者文件冲突，也不会被开发者误删依赖。
- 不采集任何用户数据，仅模块名/路径/版本/统计计数，属系统公开信息。
- 开关默认关闭时无任何行为变化，对存量应用零影响。

## 4. 测试设计（TDD）

新增 `test/unittest/native_module_collector_test/`：

| 用例 | 验证点 |
|------|--------|
| **基础采集** | |
| `CollectAndSave_Success` | mock cacheDir，注入若干 NativeModuleInfo，断言 JSON 生成且字段完整（含 score/predicted） |
| `CollectAndSave_Disabled` | 开关关闭时不采样、不产生文件 |
| `SampleAtAppReady_NoWrite` | 检查点① 只内存采样，不落盘 |
| **文件命名与位置** | |
| `GetStorePath_CacheDir` | 路径为 cacheDir 下 `.ohos.ability_runtime/` 子目录 |
| `GetStorePath_ProcessHash` | 不同进程名生成不同 `<processHash8>`；同进程名哈希稳定 |
| `FileName_NoCollision` | 文件名含 `.v2` 版本后缀与点前缀目录，不与开发者常见命名冲突 |
| **更新与预测策略** | |
| `Merge_ColdStart` | 无旧文件时首轮 score=1，predicted 为全量 |
| `Merge_HitReward` | 连续命中模块 score 单调逼近 1（验证 EWMA 公式） |
| `Merge_MissDecay` | 未命中模块 score 按 (1−α) 衰减 |
| `Predict_Threshold` | score 跨越 0.5 阈值时 predicted 集合正确增删 |
| `Evict_LowScore` | score < 0.05 条目被淘汰 |
| `Evict_LongUnseen` | 连续 16 轮未见条目被淘汰 |
| `Evict_CapLimit` | 超过 1024 条时按 score 升序淘汰 |
| `Reset_OnAppUpgrade` | appVersionCode 变化时历史清空重建 |
| `Rebuild_OnCorruptedJson` | 损坏 JSON / version 不符时按冷启动重建 |
| `Rebuild_AfterCacheClean` | 文件被删除后下轮正常重建 |
| **写盘可靠性** | |
| `WriteAtomic_PartialFailure` | 模拟写失败，正式文件不出现残缺内容 |
| `WriteAtomic_Permission` | 生成文件权限为 0600 |
| **napi 仓** | |
| `GetLoadedModuleInfos_Snapshot` | 枚举结果与链表一致，深拷贝（原节点释放后快照有效） |

## 5. 风险、依赖与演进方向

| 项 | 说明 |
|----|------|
| **跨仓依赖（最大风险）** | 需 `foundation/arkui/napi` 先合入 `NativeModuleInfo` + `GetLoadedModuleInfos`。建议与 napi 仓 owner 对齐接口形态后再启动 ability_runtime 侧开发 |
| 启动性能 | 两次内存采样 < 1ms + 每轮 1 次异步写盘，量产版本默认关闭 |
| 预测准确度 | EWMA 对"强场景分化"应用（如卡片入口与桌面入口模块集差异大）存在上限；演进方向：按 `scenario` 分桶保存多份 profile，按 launch reason 选用（v3 候选） |
| 缓存清理 | 清理后预测冷启动重建，前 1~2 轮预测偏弱，随后快速收敛；属可接受语义 |
| 多进程 | 每进程独立文件（`<processHash8>` 隔离），主进程文件可代表主场景 |

## 6. 下一步

1. 与 napi 仓 owner 对齐 `GetLoadedModuleInfos` 接口形态。
2. ability_runtime 侧实现 `NativeModuleCollector`（采样 + EWMA 合并 + 原子写）+ 双检查点接入。
3. 按第 4 节落地 TDD 用例。
4. beta 版本开启命中率上报，标定 α / 阈值后固化参数。
