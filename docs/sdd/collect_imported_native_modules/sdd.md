# SDD：启动阶段采集应用 import 的 Native 模块并保存至应用沙箱

## 1. 需求分析

### 1.1 需求描述

在应用**启动阶段**，通过 `NativeModuleManager`（进程内单例，位于 `foundation/arkui/napi`）采集该应用进程 `import`/`requireNapi` 加载过的所有 Native 模块信息，并以文件形式保存到**应用自身沙箱**目录。

### 1.2 现状与可行性

| 要素 | 现状 | 结论 |
|------|------|------|
| 模块数据 | `NativeModuleManager` 单例内部维护 `headNativeModule_ → tailNativeModule_` 链表，每个 `NativeModule` 节点含 `name`（so 注册名）、`moduleName`（import 名）、`fileName`（so 实际路径）、`version`、`isAppModule`、`moduleLoaded` 等字段 | 数据源天然存在，无需额外埋点记录 |
| 枚举接口 | **无对外枚举接口**，链表头尾指针是 private 成员 | 需新增 public API（跨仓修改 `arkui/napi`） |
| import 入口 | 所有 import 均汇聚到 `ArkNativeEngine::LoadNativeModule` → `NativeModuleManager::LoadNativeModule`（系统模块 + 应用模块同路径） | 单点覆盖全部 import 行为 |
| 写入者 | 沙箱属应用自身，NativeModuleManager 在应用进程内 | **无需 IPC**，应用进程自写自身沙箱，无权限问题 |
| 沙箱路径 | 框架侧可经 `ContextDeal::GetDataDir()` / `ContextImpl::GetBaseDir()` 得 `/data/storage/el2/base`；已有先例：`DumpRuntimeHelper` 写 `/data/storage/el2/base/cache` | 路径获取方式成熟 |
| 启动阶段锚点 | `MainThread::HandleLaunchApplication` → `Runtime::Create` → `ApplicationImpl::PerformAppReady()`（`application_impl.cpp:51`，状态转 `APP_STATE_READY`） | 有明确的"启动完成"回调点 |

### 1.3 关键需求决策点（需明确）

1. **采集时点**：模块是**按需懒加载**的——`PerformAppReady` 时只有启动链路加载的模块，首个 Ability `onWindowStageCreate` 后才有 UI 相关模块。建议定义为"启动阶段结束 = 首个 UIAbility 前台后"，或分两次采集（ready 时 + 首个 ability 前台时）合并写盘。
2. **预加载（preload/fork）语义**：`ETSRuntime::PreFork` 在预孵化进程加载的系统模块会被 fork 进每个应用进程。"应用 import 过的模块"是否包含这些？建议全量记录（它们确实在应用进程地址空间内），但可增加 `isPreloaded` 区分标记（fork 前快照计数，fork 后比对）。
3. **文件格式与位置**：建议 JSON，路径 `/data/storage/el2/base/files/native_modules.json`（或 `cache/` 下，视是否允许系统清理而定）。
4. **开关**：必须有参数/特性开关门控，默认关闭或仅 debug 版本开启，避免影响量产版本启动性能。

### 1.4 约束

- **跨仓依赖**：核心数据源在 `foundation/arkui/napi`，ability_runtime 无法独立完成，需 napi 仓先合入枚举接口。
- **性能**：采集+写盘必须异步（投递到非主线程或延迟任务），不得阻塞启动主路径。
- **安全**：仅写自身沙箱；文件权限 `0600`；不写敏感信息（路径属公开信息，风险低）。

## 2. 总体设计

```
应用进程
┌─────────────────────────────────────────────────────┐
│ MainThread::HandleLaunchApplication                  │
│   └─ Runtime::Create → JsRuntime/ETSRuntime 初始化   │
│        └─ requireNapi 绑定 → ArkNativeEngine          │
│             └─ NativeModuleManager::LoadNativeModule │  ← 模块进链表（现有行为）
│                                                      │
│ PerformAppReady / 首个Ability前台  ──触发──►          │
│   NativeModuleCollector（新增，appkit 侧）            │
│     ├─ NativeModuleManager::GetLoadedModuleInfos()   │  ← napi 仓新增 API
│     ├─ 序列化为 JSON                                  │
│     └─ 异步写 /data/storage/el2/base/files/          │
│         native_modules.json（tmp + rename 原子写）    │
└─────────────────────────────────────────────────────┘
```

**方案要点**：不改动加载路径、不加每次加载的钩子（零性能侵入），仅在检查点**一次性拉取**链表快照并异步落盘。

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
    // 在启动检查点调用；内部判开关、取沙箱路径、异步写盘
    static void CollectAndSave(const std::string& dataDir);
private:
    static bool IsEnabled();              // 读参数/特性开关
    static std::string Serialize(const std::vector<NativeModuleInfo>& infos);
    static bool WriteAtomic(const std::string& path, const std::string& content);
};
```

### 3.2 输出文件格式

`/data/storage/el2/base/files/native_modules.json`：

```json
{
  "bundleName": "com.example.app",
  "pid": 12345,
  "collectTime": 1753507200000,
  "stage": "app_ready",
  "modules": [
    {
      "moduleName": "ohos.hilog",
      "name": "hilog",
      "fileName": "/system/lib64/module/libhilog.z.so",
      "version": 1,
      "isAppModule": false,
      "moduleLoaded": true
    },
    {
      "moduleName": "entry",
      "name": "entry",
      "fileName": "/data/storage/el1/bundle/entry/libs/arm64/libentry.so",
      "version": 1,
      "isAppModule": true,
      "moduleLoaded": true
    }
  ]
}
```

### 3.3 触发点与时序

```
Runtime::Create (main_thread.cpp:1951)
  → 业务代码执行，模块陆续 import（进链表）
  → PerformAppReady (application_impl.cpp:51)
      → NativeModuleCollector::CollectAndSave()   ← 检查点1（标记 stage=app_ready）
  → 首个 UIAbility 前台（可选检查点2，增量合并后重写，stage=first_foreground）
```

- 检查点1必选；检查点2视需求 1.3 决策点 1 而定。
- 写盘投递到非主线程（可参考 `MainThread` 现有 task runner 或新建一次性 detached task）。
- **preload 场景**：fork 后进程已有预加载模块链表。若需区分，在 `PreFork` 时记录链表节点数快照，`PostFork` 后节点标记为应用期加载；否则全量记录即可。

### 3.4 异常与边界处理

| 场景 | 处理 |
|------|------|
| 开关关闭 | 直接返回，零开销 |
| 沙箱目录不存在 | `mkdirs`，失败记 HiLog 并放弃（不抛错，不影响启动） |
| 写盘失败 | 记 `TAG_LOGE` + 可选 HiSysEvent 故障事件（参考 `preload_so_startup_task.cpp:134` 的 `EventReport` 模式） |
| 模块链表为空 | 写空 `modules: []`（合法输出，便于排查） |
| 文件体积 | 模块数量级为数十~数百，KB 级，无需截断；但建议设上限（如 1MB）防御异常膨胀 |
| 并发 | 写盘用 `tmp 文件 + rename` 保证原子性；同进程多次检查点写同一文件，后写覆盖 |

### 3.5 安全与隐私

- 仅写应用自身沙箱，无越权访问；文件 `chmod 0600`。
- 不采集任何用户数据，仅模块名/路径/版本，属系统公开信息。
- 开关默认关闭时无任何行为变化，对存量应用零影响。

## 4. 测试设计（TDD）

新增 `test/unittest/native_module_collector_test/`：

| 用例 | 验证点 |
|------|--------|
| `CollectAndSave_Success` | mock 沙箱目录，注入若干 NativeModuleInfo，断言 JSON 文件生成且字段完整 |
| `CollectAndSave_Disabled` | 开关关闭时不产生文件 |
| `Serialize_FieldMapping` | `moduleName/name/fileName/isAppModule` 等字段序列化正确 |
| `WriteAtomic_PartialFailure` | 模拟写入失败，不产生残缺 JSON（tmp 残留可容忍，正式文件不出现） |
| `CollectAndSave_EmptyList` | 空链表输出合法空 JSON |
| `GetLoadedModuleInfos_Snapshot`（napi 仓测试） | 枚举结果与 `Register`/`LoadNativeModule` 后的链表一致，且为深拷贝（原节点释放后快照仍有效） |

## 5. 风险与依赖

| 项 | 说明 |
|----|------|
| **跨仓依赖（最大风险）** | 需 `foundation/arkui/napi` 先合入 `NativeModuleInfo` + `GetLoadedModuleInfos`。建议与 napi 仓 owner 对齐接口形态后再启动 ability_runtime 侧开发 |
| 启动性能 | 采集在检查点一次性执行 + 异步写盘，预估主线程开销 < 1ms（仅链表遍历拷贝），量产版本默认关闭 |
| 语义歧义 | "启动阶段"结束点、preload 模块归属两点需与需求方书面确认（1.3 节） |
| 多进程应用 | 每个进程各自采集，文件名建议带进程名后缀（如 `native_modules_<processName>.json`）避免互相覆盖 |

## 6. 下一步

1. 确认 1.3 节决策点（采集时点、preload 语义、是否默认开启）。
2. napi 仓合入枚举接口。
3. ability_runtime 侧实现 `NativeModuleCollector` + 检查点接入 + TDD 用例。
