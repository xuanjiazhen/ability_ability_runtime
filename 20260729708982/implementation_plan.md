# PC 双模式意图存储 Key 扩展 — 实施方案

> 本文档融合 `proposal.md` / `spec.md` / `design.md` / `execution-plan.md` 四件套的有效内容，并叠加对 `ability_ability_runtime` 仓库真实源码的验证结果与最终确定的实施修改方案。所有文件路径、类名、行号均已经源码核实。

---

## 第一部分：需求背景

### 1.1 业务背景

PC 设备支持双模式运行（PC 模式 / PAD 模式），同一应用可能存在 PC 模式版本和 PAD 模式版本。两个版本的包名相同但功能不同、签名不同。

**当前问题**：现有意图存储 Key 仅按 `userId/bundleName/moduleName/intentName/versionCode` 区分（见 `services/abilitymgr/src/insight_intent/insight_intent_rdb_storage_mgr.cpp` 多处内联拼接），无法满足双模式场景下的精确操作需求。包事件入口 `AbilityBundleEventCallback` 仅在卸载链路透传 `appIndex`，安装/更新链路根本未传；且卸载链路 `InsightIntentEventMgr::DeleteInsightIntentEvent` 对 `appIndex > 0` 一刀切跳过（注释 "this application is a simulation"），误伤双模式副模式主应用。导致用户无法精确管理同名不同包应用，可能误操作、数据混淆或功能异常。

**解决思路**：通过在意图存储 Key 中增加 AppIndex 字段，区分不同模式的应用，实现完整的双模式生命周期管理。

### 1.2 术语定义

| 术语 | 含义 | appIndex | 触发设备 |
|------|------|----------|---------|
| 同名同包 | 不支持双模式的设备，应用仅一份实例，走原有逻辑 | 0 | 非双模式设备（全部应用）+ 双模式设备的主模式应用 |
| 同名不同包 | 支持双模式的设备，应用以相同 bundleName 安装主/副两份实例 | 主模式=0，副模式=PC_TABLET_INDEX(10000) | 双模式设备 |

说明：
- "同名"指 bundleName 相同；"同包/不同包"指是否因双模式产生多个 appIndex 实例
- appIndex=0 在非双模式设备上行为与原有一致（向后兼容）
- 本次扩展仅针对"同名不同包"场景，即双模式设备上的主/副模式应用

### 1.3 AppIndex 完整分类体系

源码 `utils/global/constant/global_constant.h:21-50` 已定义完整分类，本特性只处理主应用维度：

| 区间 | 含义 | 判断函数 | 本特性处理 |
|---|---|---|---|
| 0 | 主模式主应用 | — | 双模式设备追加 Key，非双模式不追加 |
| 1–1000 | 普通应用克隆 | `IsAppCloneIndex` | 不处理（维持现有跳过） |
| 2000–3000 | 沙箱克隆 | `IsSandboxCloneIndex` | 不处理 |
| **10000 (PC_TABLET_INDEX)** | **PC/平板双模式副模式主应用** | `== PC_TABLET_INDEX` | **本特性核心扩展点** |
| 10001–11000 | PC/平板模式克隆 | `IsAppCloneIndex`（第二区间） | 不处理 |
| 11001–11999 | PC/平板 DLP | `IsDlpIndex`（兜底） | 不处理 |
| 12000–13000 | PC/平板 AI Agent | `IsSandboxCloneIndex`（第二区间） | 不处理 |

### 1.4 同名不同包判断入口

**判断方式**：通过应用包信息的 `DeviceModeDistributionPolicy` 分发策略判断（定义见 `foundation/bundlemanager/bundle_framework/interfaces/inner_api/appexecfwk_base/include/bundle_info.h:162`）：

```cpp
enum class DeviceModeDistributionPolicy : int32_t {
    UNSPECIFIED = 0,                              // 未指定策略（默认）
    MAIN_ONLY = 1,                                // 仅主模式
    SUB_ONLY = 2,                                 // 仅副模式
    UNIVERSAL_IDENTICAL_PACKAGE = 3,              // 通用，相同包体（同名同包）
    UNIVERSAL_DIFFERENT_PACKAGE = 4,              // 通用，不同包体（同名不同包）
    PARTIAL_COMPATIBLE_IDENTICAL_PACKAGE = 5,     // 部分兼容，相同包体（同名同包）
    PARTIAL_COMPATIBLE_DIFFERENT_PACKAGE = 6,     // 部分兼容，不同包体（同名不同包）
    FULL_COMPATIBLE_IDENTICAL_PACKAGE = 7,        // 完全兼容，相同包体（同名同包）
    FULL_COMPATIBLE_DIFFERENT_PACKAGE = 8,        // 完全兼容，不同包体（同名不同包）
};
```

**判断逻辑**：调用 `BundleMgrHelper::GetBundleInfoV9` 获取 `BundleInfo`，读取 `bundleInfo.deviceModeDistributionPolicy`：
- 值为 4 / 6 / 8（`*_DIFFERENT_PACKAGE`）→ 同名不同包 → Key 追加 appIndex 段
- 其他值 → 同名同包 → Key 不追加（原有逻辑，向后兼容）

`BundleInfo` 中同时包含 `appIndex` 字段（`:202`），用于 Key 拼接。主模式 appIndex=0，副模式 appIndex=PC_TABLET_INDEX(10000)。

**关键决策**：判断"是否同名不同包"基于应用自身的分发策略（`DeviceModeDistributionPolicy`），而非设备是否支持双模式。同名不同包应用在双模式设备上主模式（appIndex=0）和副模式（appIndex=10000）都追加 Key 段，实现对等管理。

**卸载兜底**：卸载场景下应用可能已卸载，`GetBundleInfoV9` 会失败。此时按 `appIndex == PC_TABLET_INDEX(10000)` 兜底判断为同名不同包副模式。

---

## 第二部分：目标与非目标

### 2.1 目标

1. **安装意图管理**：双模式应用安装时，按 GetDualModeBundleInfo 返回的 appIndex 精确存储意图（主模式 0 与副模式 10000 都追加 Key）
2. **更新意图管理**：升级时按目标模式更新意图，删除旧模式意图
3. **卸载意图管理**：按 appIndex 精确删除对应模式应用意图（修复现有"appIndex>0 一刀切跳过"误伤副模式主应用的问题）
4. **查询意图管理**：双模式应用按 appIndex 精确匹配当前模式意图
5. **执行意图管理**：默认执行当前模式意图，优先匹配当前模式应用
6. **脏数据清理**：执行失败时清理对应模式无效意图数据（复用现有 Transform/DeleteInsightIntent 扩展 appIndex 维度，非新建）

### 2.2 非目标

- 不涉及 Public/System/InnerAPI 接口变更（仅内部实现调整）
- 不涉及 UX/交互设计变更
- 不支持从非同名同包回退到同名同包
- 不回溯 OpenHarmony 7.0 数据（非双模式设备老数据 Key 不含 appIndex，由 GetDualModeBundleInfo 返回失败自动兼容）
- 不处理克隆/DLP/AI Agent 索引区间（1-1000/2000-3000/10001-13000 等维持现有跳过行为）

### 2.3 验收标准（AC）

| AC 编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-1.1 | 安装非双模式应用（GetDualModeBundleInfo 返回失败），查询意图返回结果与升级前一致（Key 不含 appIndex，向后兼容） | 正常 |
| AC-1.2 | 安装双模式应用（GetDualModeBundleInfo 返回成功），主模式 appIndex=0 与副模式 appIndex=10000 意图独立存储，Key 均含 appIndex 段 | 正常 |
| AC-2.1 | 查询双模式应用意图时，按当前模式 appIndex 精确匹配 | 正常 |
| AC-3.1 | 执行双模式应用意图时，默认执行当前模式意图 | 正常 |
| AC-4.1 | 升级双模式应用时，按目标模式更新意图并清理旧模式意图 | 正常 |
| AC-5.1 | 卸载双模式应用某模式时，仅删除对应模式意图，另一模式意图仍存在 | 正常 |
| AC-6.1 | 双模式应用意图执行失败时，自动清理对应模式无效意图数据 | 恢复 |

---

## 第三部分：源码现状基线（已验证）

### 3.1 真实模块结构

意图模块位于 `services/abilitymgr/` 下，由以下子模块协作（`services/intent/` 目录不存在，原 design.md 引用为虚构路径）：

| 层 | 真实类名 | 头文件 | 源文件 |
|---|---|---|---|
| 包事件入口 | `AbilityBundleEventCallback` | `services/abilitymgr/include/ability_bundle_event_callback.h` | `services/abilitymgr/src/ability_bundle_event_callback.cpp` |
| 事件处理 | `InsightIntentEventMgr` | `services/abilitymgr/include/insight_intent/insight_intent_event_mgr.h` | `services/abilitymgr/src/insight_intent/insight_intent_event_mgr.cpp` |
| 存储管理 | `InsightRdbStorageMgr` | `services/abilitymgr/include/insight_intent/insight_intent_rdb_storage_mgr.h` | `services/abilitymgr/src/insight_intent/insight_intent_rdb_storage_mgr.cpp` |
| 缓存层 | `InsightIntentDbCache` | `services/abilitymgr/include/insight_intent/insight_intent_db_cache.h` | `services/abilitymgr/src/insight_intent/insight_intent_db_cache.cpp` |
| 执行层 | `InsightIntentExecuteManager` | `services/abilitymgr/include/insight_intent/insight_intent_execute_manager.h` | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp` |

### 3.2 Key 拼接现状（8 处内联，均不含 appIndex）

文件：`services/abilitymgr/src/insight_intent/insight_intent_rdb_storage_mgr.cpp`

| 行号 | 用途 | 现有 Key 结构 |
|---|---|---|
| 114-115, 129-130 | 按 bundle 查询前缀 | `userId/bundleName/` |
| 145-146 | 按意图名查询 profile | `userId/bundleName/moduleName/intentName/` |
| 173-174 | 按意图名查询 config | 同上 |
| 247-248 | 保存 profile（完整 Key） | `userId/bundleName/moduleName/intentName/versionCode` |
| 262-263 | 保存 config（完整 Key） | 同上 |
| 293-295, 309-311 | 批量保存 | 同上 |

### 3.3 关键缺口（本次要修复的核心问题）

| # | 缺口 | 位置 | 影响 |
|---|---|---|---|
| 1 | 安装/更新链路未透传 appIndex | `ability_bundle_event_callback.cpp:93, :113`（`UpdateInsightIntentEvent` 调用未传 appIndex） | 双模式应用安装/更新时意图模块拿不到 appIndex |
| 2 | `UpdateInsightIntentEvent` 签名无 appIndex | `insight_intent_event_mgr.h:29` | 无法接收双模式信息 |
| 3 | 卸载链路对 appIndex>0 一刀切跳过 | `insight_intent_event_mgr.cpp:178-182`（注释 "simulation"） | 误伤 PC_TABLET_INDEX=10000 副模式主应用 |
| 4 | 存储/缓存层方法签名均无 appIndex | `insight_intent_rdb_storage_mgr.h:37-58`, `insight_intent_db_cache.h:30-68` | 无法按模式区分 Key |
| 5 | `InsightIntentSaveParam` 无 appIndex 字段 | `insight_intent_rdb_storage_mgr.h:28-32` | batch save 无法携带 appIndex |
| 6 | 脏数据清理不区分 appIndex | `insight_intent_rdb_storage_mgr.cpp:210-239` Transform + `insight_intent_event_mgr.cpp:36-45` DeleteInsightIntent | 清理会误删另一模式 |

### 3.4 已具备的条件

- `appIndex` 通道已打通：`ability_bundle_event_callback.cpp:61` 已从 Want 读取 `KEY_APP_INDEX`，`:89` 卸载链路已透传
- `PC_TABLET_INDEX=10000` 常量已定义：`utils/global/constant/global_constant.h:25`
- `GetDualModeBundleInfo` 双模式判断入口已存在：`bundle_mgr_helper.cpp:1101-1120`
- `ApplicationInfo.multiAppMode` 字段已有：`appspawn_util.h:86` 已使用
- 现有脏数据清理入口可用：`Transform`（解析失败调 `DeleteDataBeginWithKey`）+ `DeleteInsightIntent`

---

## 第四部分：具体修改方案

### 4.1 修改文件清单（11 个）

| # | 文件 | 修改类型 | 所属层 | 阶段 |
|---|---|---|---|---|
| 1 | `services/abilitymgr/src/ability_bundle_event_callback.cpp` | 修改 | 包事件入口 | 5 |
| 2 | `services/abilitymgr/include/insight_intent/insight_intent_event_mgr.h` | 修改 | 事件处理 | 1 |
| 3 | `services/abilitymgr/src/insight_intent/insight_intent_event_mgr.cpp` | 修改 | 事件处理 | 4 |
| 4 | `services/abilitymgr/include/insight_intent/insight_intent_rdb_storage_mgr.h` | 修改 | 存储层 | 1 |
| 5 | `services/abilitymgr/src/insight_intent/insight_intent_rdb_storage_mgr.cpp` | 修改 | 存储层 | 2 |
| 6 | `services/abilitymgr/include/insight_intent/insight_intent_db_cache.h` | 修改 | 缓存层 | 1 |
| 7 | `services/abilitymgr/src/insight_intent/insight_intent_db_cache.cpp` | 修改 | 缓存层 | 3 |
| 8 | `services/abilitymgr/include/insight_intent/insight_intent_execute_manager.h` | 修改 | 执行层 | 1 |
| 9 | `services/abilitymgr/src/insight_intent/insight_intent_execute_manager.cpp` | 修改 | 执行层 | 6 |
| 10 | `services/abilitymgr/BUILD.gn` + `abilitymgr.gni` | 检查 | 构建 | 1 |
| 11 | `test/unittest/insight_intent_*_test/*` | 修改/新增 | 测试 | 8 |

---

### 4.2 阶段 1：头文件签名扩展（编译通过 stub）

#### 文件 2：`insight_intent_event_mgr.h`

**修改原因**：`UpdateInsightIntentEvent` 签名无 appIndex，无法接收双模式信息；需要携带双模式判断结果的数据结构。

**修改点 2.1**：新增 `DualModeContext` 结构体（携带事件层一次判断的结果，透传下游，避免存储层每次操作都发起 BMS IPC）

**修改前（:27-37）**：
```cpp
class InsightIntentEventMgr : public std::enable_shared_from_this<InsightIntentEventMgr> {
public:
    static void UpdateInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId);
    static void DeleteInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId, int32_t appIndex);
    void SubscribeSysEventReceiver();
private:
    std::shared_ptr<InsightIntentSysEventReceiver> insightIntentSysEventReceiver_ = nullptr;
    static void DeleteInsightIntent(const std::string &bundleName, const std::string &moduleName, int32_t userId);
};
```

**修改后**：
```cpp
class InsightIntentEventMgr : public std::enable_shared_from_this<InsightIntentEventMgr> {
public:
    struct DualModeContext {
        int32_t appIndex = 0;
        bool isDualMode = false;
    };
    static void UpdateInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId,
        int32_t appIndex);
    static void DeleteInsightIntentEvent(const AppExecFwk::ElementName &elementName, int32_t userId,
        int32_t appIndex);
    void SubscribeSysEventReceiver();
private:
    std::shared_ptr<InsightIntentSysEventReceiver> insightIntentSysEventReceiver_ = nullptr;
    static void DeleteInsightIntent(const std::string &bundleName, const std::string &moduleName,
        int32_t userId, const DualModeContext &ctx);
};
```

新增 include：`#include "global_constant.h"`（PC_TABLET_INDEX 常量所在）。

#### 文件 4：`insight_intent_rdb_storage_mgr.h`

**修改原因**：存储层所有方法签名无 appIndex，无法按模式区分 Key；`InsightIntentSaveParam` 无 appIndex 字段，batch save 无法携带。

**修改点 4.1**：`InsightIntentSaveParam` 加 appIndex + isDualMode（:28-32）

**修改前**：
```cpp
struct InsightIntentSaveParam {
    std::string moduleName;
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::vector<InsightIntentInfo> configInfos;
};
```

**修改后**：
```cpp
struct InsightIntentSaveParam {
    std::string moduleName;
    ExtractInsightIntentProfileInfoVec profileInfos;
    std::vector<InsightIntentInfo> configInfos;
    int32_t appIndex = 0;
    bool isDualMode = false;
};
```

**修改点 4.2**：新增 Key 拼接辅助方法声明（private 区，:60 后）

```cpp
private:
    static std::string BuildIntentKey(const std::string &bundleName, const std::string &moduleName,
        const std::string &intentName, int32_t userId, uint32_t versionCode,
        int32_t appIndex, bool isDualMode);
    static std::string BuildBundlePrefix(int32_t userId, const std::string &bundleName,
        int32_t appIndex, bool isDualMode);
```

**修改原因**：8 处内联 Key 拼接散落，扩展时易漏改；统一为辅助方法，集中处理"是否追加 appIndex 段"逻辑。

**修改点 4.3**：现有方法签名扩展（:37-58）

为避免循环 include，存储层不引用 `InsightIntentEventMgr::DualModeContext`，直接传 `int32_t appIndex, bool isDualMode` 两个原始参数。

| 方法 | 修改 |
|---|---|
| `LoadInsightIntentInfo`（:46） | +`int32_t appIndex, bool isDualMode` |
| `LoadConfigInsightIntentInfo`（:48） | +`int32_t appIndex, bool isDualMode` |
| `LoadInsightIntentInfoByName`（:42） | +`int32_t appIndex, bool isDualMode` |
| `LoadConfigInsightIntentInfoByName`（:44） | +`int32_t appIndex, bool isDualMode` |
| `SaveStorageInsightIntentData`（:50） | +`int32_t appIndex, bool isDualMode` |
| `SaveStorageInsightIntentDataBatch`（:53） | 不改（appIndex 在 saveParam 内） |
| `DeleteStorageInsightIntentData`（:55） | +`int32_t appIndex, bool isDualMode` |

**不改的方法**：`LoadInsightIntentBundleInfos`、`LoadInsightIntentInfos`、`LoadConfigInsightIntentInfos`、`DeleteStorageInsightIntentByUserId`、`BackupRdb`——全量加载/按 userId 删除，不区分单个 appIndex。

#### 文件 6：`insight_intent_db_cache.h`

**修改原因**：缓存层是存储层薄封装，签名必须对齐；缓存与存储 Key 不一致会导致命中缓存 vs 落库结果不一致。

**修改点 6.1**：方法签名扩展（:30-68），与存储层对齐加 `int32_t appIndex, bool isDualMode`：

| 方法 | 修改 |
|---|---|
| `GetInsightIntentGenericInfo`（:35） | +2 参数 |
| `GetInsightIntentInfoByName`（:43） | +2 参数 |
| `GetConfigInsightIntentInfoByName`（:45） | +2 参数 |
| `GetInsightIntentInfo`（:49） | +2 参数 |
| `GetConfigInsightIntentInfo`（:51） | +2 参数 |
| `SaveInsightIntentTotalInfo`（:53） | +2 参数 |
| `SaveBatchInsightIntentTotalInfo`（:56） | 不改（appIndex 在 saveParam 内） |
| `DeleteInsightIntentTotalInfo`（:61） | +2 参数 |
| `HasInsightIntentByName`（:64） | +2 参数 |
| `HasBundleCache`（:67） | +2 参数 |

**不改**：`InitInsightIntentCache`、`GetAllInsightIntentGenericInfo`、`GetAllInsightIntentInfo`、`GetAllInsightIntentInfoForRegister`、`GetAllConfigInsightIntentInfo`、`DeleteInsightIntentByUserId`、`IsCacheInitialized`、`BackupRdb`。

**修改点 6.2**：`bundleVersionMap_`（:73）语义说明

当前 map key 是 `bundleName`。双模式扩展后同一 bundleName 有两个 appIndex 版本可能不同。**处理策略**（不改字段类型）：map key 改为 `bundleName + "/" + std::to_string(appIndex)`（双模式）或 `bundleName`（非双模式），在 `HasInsightIntentByName`/`HasBundleCache` 实现里按 isDualMode 决定拼接方式。

#### 文件 8：`insight_intent_execute_manager.h`

**修改原因**：执行入口需按当前模式 appIndex 优先匹配。

**修改点 8.1**：`CheckAndUpdateParam`（:81-84）追加 2 参数：

**修改后**：
```cpp
int32_t CheckAndUpdateParam(uint64_t key, const sptr<IRemoteObject> &callerToken,
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param, std::string callerBundleName = "",
    const bool ignoreAbilityName = false, bool isDistributed = false,
    const std::string &srcDeviceId = "", uint64_t requestCode = 0, uint64_t specifiedFullTokenId = 0,
    int32_t appIndex = 0, bool isDualMode = false);
```

**修改点 8.2**：`CheckAndUpdateQueryEntityParam`（:124-125）追加 2 参数：

```cpp
int32_t CheckAndUpdateQueryEntityParam(uint64_t key, const sptr<IRemoteObject> &callerToken,
    std::shared_ptr<OHOS::AppExecFwk::InsightIntentQueryParam> &param, const std::string& callerBundleName,
    int32_t appIndex, bool isDualMode);
```

**修改点 8.3**：`InsightIntentExecuteRecord`（:39-49）加 appIndex 字段：

```cpp
struct InsightIntentExecuteRecord {
    ...
    int32_t appIndex = 0;   // 新增
    bool isDualMode = false; // 新增
};
```

**修改原因**：`OnInsightAppDied`（:122）按 bundleName 清理会误清另一模式，改为 bundleName+appIndex 精确清理。

#### 文件 10：`services/abilitymgr/BUILD.gn` + `abilitymgr.gni`

**修改原因**：新增 `#include "global_constant.h"`，需确认 include 路径。

**检查项**：`utils/global/constant/` 是否已在 `abilitymgr` 的 `include_dirs` 或 `public_configs` 中。若未在，需在 `BUILD.gn` 添加 `"//foundation/ability/ability_runtime/utils/global/constant"`。本次无新增源文件，`.gni` 源文件列表无需改。

---

### 4.3 阶段 2：存储层 Key 拼接实现（文件 5）

#### 文件 5：`insight_intent_rdb_storage_mgr.cpp`

**修改原因**：8 处内联 Key 拼接需统一按 isDualMode 决定是否追加 appIndex 段。

**修改点 5.1**：新增 `BuildIntentKey` / `BuildBundlePrefix` 实现（文件头部 namespace 内）

```cpp
std::string InsightRdbStorageMgr::BuildIntentKey(const std::string &bundleName,
    const std::string &moduleName, const std::string &intentName, int32_t userId,
    uint32_t versionCode, int32_t appIndex, bool isDualMode)
{
    std::string key = std::to_string(userId) + "/" + bundleName + "/" + moduleName + "/"
        + intentName + "/" + std::to_string(versionCode);
    if (isDualMode) {
        key += "/" + std::to_string(appIndex);
    }
    return key;
}

std::string InsightRdbStorageMgr::BuildBundlePrefix(int32_t userId, const std::string &bundleName,
    int32_t appIndex, bool isDualMode)
{
    std::string prefix = std::to_string(userId) + "/" + bundleName + "/";
    if (isDualMode) {
        prefix += std::to_string(appIndex) + "/";
    }
    return prefix;
}
```

**修改原因**：统一 Key 拼接逻辑，isDualMode=true 时追加 appIndex 段（含主模式 0），false 时不追加（向后兼容）。

**修改点 5.2**：替换 8 处内联 Key 拼接为辅助方法调用

| 行号 | 修改前 | 修改后 |
|---|---|---|
| 114-115, 129-130 | `std::to_string(userId) + "/" + bundleName + "/"` | `BuildBundlePrefix(userId, bundleName, appIndex, isDualMode)` |
| 145-146, 173-174 | `std::to_string(userId) + "/" + bundleName + "/" + moduleName + "/" + intentName + "/"` | `BuildBundlePrefix(...)` + `moduleName + "/" + intentName + "/"` |
| 247-248, 262-263 | `std::to_string(userId) + "/" + bundleName + "/" + moduleName + "/" + intentName + "/" + std::to_string(versionCode)` | `BuildIntentKey(bundleName, moduleName, intentName, userId, versionCode, appIndex, isDualMode)` |
| 293-295, 309-311 | 同上（batch） | `BuildIntentKey(...)`，appIndex 从 `saveParams[i].appIndex` / `saveParams[i].isDualMode` 取 |

**修改点 5.3**：`Transform`（:210-239）解析逻辑兼容两种 Key 格式

**修改原因**：RDB 老数据 Key 不含 appIndex 段（5 段），新数据双模式 Key 含 appIndex 段（6 段），解析需兼容。

**修改策略**：按 `/` 分割 Key 后判断段数：
- 5 段（`userId/bundleName/moduleName/intentName/versionCode`）→ 非双模式老数据，appIndex=0
- 6 段（多一个 appIndex）→ 双模式数据，取末段为 appIndex

具体实现需读取 `:210-239` 现有 tokenize 代码后适配，本方案给出解析策略。

---

### 4.4 阶段 3：缓存层透传实现（文件 7）

#### 文件 7：`insight_intent_db_cache.cpp`

**修改原因**：缓存层是存储层薄封装，需透传 appIndex/isDualMode 给存储层，保持 Key 一致。

**修改方式**：每个方法实现体把新增的 `appIndex, isDualMode` 参数透传给 `InsightRdbStorageMgr` 对应方法。缓存层自身不做双模式判断（不调 GetDualModeBundleInfo），只按传入参数转发。

**示例**：
```cpp
// 修改前
void InsightIntentDbCache::GetInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName, const int32_t userId, ExtractInsightIntentInfo &infos)
{
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfo(
        bundleName, moduleName, intentName, userId, infos);
}

// 修改后
void InsightIntentDbCache::GetInsightIntentInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName, const int32_t userId, ExtractInsightIntentInfo &infos,
    int32_t appIndex, bool isDualMode)
{
    DelayedSingleton<InsightRdbStorageMgr>::GetInstance()->LoadInsightIntentInfo(
        bundleName, moduleName, intentName, userId, infos, appIndex, isDualMode);
}
```

**关键约束**：缓存层的内存缓存 key（如有）必须与存储层 RDB Key 一致使用 appIndex，否则缓存命中 vs 落库结果不一致。`bundleVersionMap_` 的 key 按 4.2 修改点 6.2 策略处理。

---

### 4.5 阶段 4：事件处理层实现（文件 3）

#### 文件 3：`insight_intent_event_mgr.cpp`

**修改原因**：这是双模式判断的入口层，需调 GetDualModeBundleInfo 一次，结果透传下游；同时修复卸载链路"appIndex>0 一刀切跳过"的误伤。

**修改点 3.1**：新增双模式判断辅助函数（文件头部 namespace 内，:35 前）

```cpp
static InsightIntentEventMgr::DualModeContext ResolveDualModeContext(
    const std::string &bundleName, int32_t userId, int32_t appIndexFromEvent)
{
    InsightIntentEventMgr::DualModeContext ctx;
    ctx.appIndex = appIndexFromEvent;
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGW(AAFwkTag::INTENT, "bundleMgrHelper null, fallback non-dual-mode");
        return ctx;
    }
    int32_t dualAppIndex = 0;
    auto ret = IN_PROCESS_CALL(bundleMgrHelper->GetDualModeBundleInfo(bundleName, userId, dualAppIndex));
    if (ret == ERR_OK) {
        ctx.isDualMode = true;
        ctx.appIndex = dualAppIndex;
        TAG_LOGI(AAFwkTag::INTENT, "dual-mode app, bundle:%{public}s, appIndex:%{public}d",
            bundleName.c_str(), dualAppIndex);
    } else {
        ctx.isDualMode = false;
        ctx.appIndex = 0;
        TAG_LOGI(AAFwkTag::INTENT, "non-dual-mode app, bundle:%{public}s", bundleName.c_str());
    }
    return ctx;
}
```

**修改原因**：
1. 双模式判断入口 = `GetDualModeBundleInfo`，返回 ERR_OK 即双模式
2. 事件层判断一次，结果封装为 `DualModeContext` 透传下游，存储/缓存层不再发起 IPC，避免每次 Key 操作都跨进程查 BMS
3. 主模式 appIndex=0 也被带回（来自 `DualModeBundleInfo.appIndex`），下游据此拼接 Key

**修改点 3.2**：`UpdateInsightIntentEvent`（:47-165）签名加 appIndex + 内部调 ResolveDualModeContext

**修改前签名**：
```cpp
void InsightIntentEventMgr::UpdateInsightIntentEvent(
    const AppExecFwk::ElementName &elementName, int32_t userId)
```

**修改后签名**：
```cpp
void InsightIntentEventMgr::UpdateInsightIntentEvent(
    const AppExecFwk::ElementName &elementName, int32_t userId, int32_t appIndex)
```

函数体修改：
- `:65 ffrt::submit` 前调 `ResolveDualModeContext`，lambda 捕获 `ctx`
- `:142 SaveBatchInsightIntentTotalInfo` 调用透传 `ctx`（或拆为 appIndex + isDualMode 透传，取决于缓存层签名设计）
- `:93/:104 DeleteInsightIntent` 调用补 `ctx`

**修改前（:65, :142-143）**：
```cpp
ffrt::submit([moduleName, bundleName, userId] () {
    ...
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->SaveBatchInsightIntentTotalInfo(
        bundleName, userId, bundleInfo.versionCode, saveParams);
```

**修改后**：
```cpp
auto ctx = ResolveDualModeContext(bundleName, userId, appIndex);
ffrt::submit([moduleName, bundleName, userId, ctx] () {
    ...
    // saveParams 内每个 InsightIntentSaveParam 填入 ctx.appIndex / ctx.isDualMode
    for (auto &sp : saveParams) {
        sp.appIndex = ctx.appIndex;
        sp.isDualMode = ctx.isDualMode;
    }
    DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance()->SaveBatchInsightIntentTotalInfo(
        bundleName, userId, bundleInfo.versionCode, saveParams);
```

**修改点 3.3**：`DeleteInsightIntent`（:36-45）签名加 ctx + 透传

**修改前**：
```cpp
void InsightIntentEventMgr::DeleteInsightIntent(const std::string &bundleName,
    const std::string &moduleName, int32_t userId)
{
    if (!DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(
        bundleName, moduleName, userId)) {
        return;
    }
    ...
}
```

**修改后**：
```cpp
void InsightIntentEventMgr::DeleteInsightIntent(const std::string &bundleName,
    const std::string &moduleName, int32_t userId, const DualModeContext &ctx)
{
    if (!DelayedSingleton<InsightIntentDbCache>::GetInstance()->DeleteInsightIntentTotalInfo(
        bundleName, moduleName, userId, ctx.appIndex, ctx.isDualMode)) {
        return;
    }
    ...
}
```

**修改点 3.4**：`DeleteInsightIntentEvent`（:167-205）核心判断逻辑修改

**修改前（:178-182）**：
```cpp
if (appIndex > 0) {
    TAG_LOGI(AAFwkTag::INTENT, "this application is a simulation, not support to delete intent info, "
        "bundleName: %{public}s, appIndex: %{public}d", bundleName.c_str(), appIndex);
    return;
}
```

**修改后**：
```cpp
auto ctx = ResolveDualModeContext(bundleName, userId, appIndex);

// 非双模式应用且 appIndex>0：维持原有跳过行为（克隆/DLP/AI Agent 索引）
if (!ctx.isDualMode && appIndex > 0) {
    TAG_LOGI(AAFwkTag::INTENT, "non-dual-mode app with appIndex>0 (clone/dlp), skip delete, "
        "bundleName: %{public}s, appIndex: %{public}d", bundleName.c_str(), appIndex);
    return;
}
// 双模式应用：无论主模式(0)还是副模式(10000)都执行删除
// 非双模式应用且 appIndex==0：原有逻辑执行删除
```

并把 `:194-204` lambda 捕获 `ctx`，调 `DeleteInsightIntent` 透传：

**修改前（:194-198）**：
```cpp
ffrt::submit([bundleName, moduleName, userId]() {
    auto dbCache = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance();
    if (!dbCache->DeleteInsightIntentTotalInfo(bundleName, moduleName, userId)) {
        return;
    }
```

**修改后**：
```cpp
ffrt::submit([bundleName, moduleName, userId, ctx]() {
    auto dbCache = DelayedSingleton<AbilityRuntime::InsightIntentDbCache>::GetInstance();
    if (!dbCache->DeleteInsightIntentTotalInfo(
        bundleName, moduleName, userId, ctx.appIndex, ctx.isDualMode)) {
        return;
    }
```

**修改原因**：原 `appIndex > 0` 一刀切跳过把 PC_TABLET_INDEX=10000 副模式主应用也跳过了（注释误标为 "simulation"）。修正后：双模式应用（GetDualModeBundleInfo 成功）按 appIndex 精确删除；非双模式应用 appIndex>0 仍跳过（克隆/DLP 索引）。

---

### 4.6 阶段 5：包事件入口透传（文件 1）

#### 文件 1：`ability_bundle_event_callback.cpp`

**修改原因**：安装/更新链路 `UpdateInsightIntentEvent` 调用未传 appIndex，导致双模式应用安装/更新时意图模块拿不到 appIndex。

**修改点**：`:93` 和 `:113` 两处 `UpdateInsightIntentEvent` 调用补 appIndex 参数。

**修改前（:89, :93, :113）**：
```cpp
// :89（PACKAGE_REMOVED，无变化）
AbilityRuntime::InsightIntentEventMgr::DeleteInsightIntentEvent(want.GetElement(), userId, appIndex);

// :93（PACKAGE_ADDED，未传 appIndex）
AbilityRuntime::InsightIntentEventMgr::UpdateInsightIntentEvent(want.GetElement(), userId);

// :113（PACKAGE_CHANGED，未传 appIndex）
AbilityRuntime::InsightIntentEventMgr::UpdateInsightIntentEvent(want.GetElement(), userId);
```

**修改后**：
```cpp
// :89（无变化）
AbilityRuntime::InsightIntentEventMgr::DeleteInsightIntentEvent(want.GetElement(), userId, appIndex);

// :93（透传 appIndex）
AbilityRuntime::InsightIntentEventMgr::UpdateInsightIntentEvent(want.GetElement(), userId, appIndex);

// :113（透传 appIndex）
AbilityRuntime::InsightIntentEventMgr::UpdateInsightIntentEvent(want.GetElement(), userId, appIndex);
```

**改动量**：2 处调用补 `appIndex` 参数。`appIndex` 已在 `:61` 读取（`want.GetIntParam(KEY_APP_INDEX, 0)`），无需新增读取逻辑。

---

### 4.7 阶段 6：执行层实现（文件 9）

#### 文件 9：`insight_intent_execute_manager.cpp`

**修改原因**：执行入口查询意图信息时需按 appIndex 精确匹配当前模式。

**修改点 9.1**：`CheckAndUpdateParam` 实现内查询意图信息时透传 appIndex/isDualMode

```cpp
// 修改前（概念）
DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfo(
    bundleName, moduleName, intentName, userId, info);

// 修改后
DelayedSingleton<InsightIntentDbCache>::GetInstance()->GetInsightIntentInfo(
    bundleName, moduleName, intentName, userId, info, appIndex, isDualMode);
```

**修改点 9.2**：`OnInsightAppDied`（:122）按 bundleName + appIndex 精确清理

**修改前**：按 bundleName 清理执行记录。

**修改后**：按 bundleName + appIndex 精确清理（`InsightIntentExecuteRecord` 已加 appIndex 字段，见修改点 8.3）。

**修改原因**：原按 bundleName 清理会误清另一模式执行记录。

---

### 4.8 阶段 7：上游调用方适配

#### `ability_manager_service.cpp`

**修改原因**：`AbilityManagerService` 调用意图执行/查询接口的入口需从 caller AbilityRecord 取 appIndex + isDualMode 透传。

**适配方式**：caller 的 `AbilityRecord` 已有 `appIndex_` 字段（`ability_record.cpp:197-205` 已读取 `applicationInfo.appIndex`）。`isDualMode` 判断建议复用事件层已缓存的判断结果，或在 caller 侧调 `GetDualModeBundleInfo`（首次调用后建议缓存，避免重复 IPC）。

**概念示例**：
```cpp
auto callerRecord = ...;
int32_t appIndex = callerRecord->GetAppIndex();
bool isDualMode = ...; // 调 GetDualModeBundleInfo 或从 ApplicationInfo.multiAppMode 判断
DelayedSingleton<InsightIntentExecuteManager>::GetInstance()->CheckAndUpdateParam(
    key, callerToken, param, callerBundleName, false, false, "", 0, 0,
    appIndex, isDualMode);
```

`ability_manager_service.cpp` 中所有调用意图接口的入口需逐一适配，本方案给出方向，实施时按调用点逐一修改。

---

### 4.9 阶段 8：测试用例

**修改原因**：覆盖 AC-1.1~AC-6.1 全场景。

| 测试文件 | 新增用例 | 覆盖 AC |
|---|---|---|
| `test/unittest/insight_intent_rdb_storage_mgr_test/*` | 双模式主模式 Key 追加 appIndex=0；副模式 Key 追加 10000；非双模式 Key 不追加；按 appIndex 精确删除 | AC-1.1, AC-1.2, AC-5.1 |
| `test/unittest/insight_intent_db_cache_test/*` | 双模式缓存与存储 Key 一致性 | AC-1.2 |
| `test/unittest/insight_intent/insight_intent_execute_manager_test/*` | 执行默认当前模式 | AC-3.1 |
| 新建 `insight_intent_event_mgr_test` 或挂到 `insight_intent_sys_event_receiver_test` | 安装链路透传 appIndex；卸载副模式不跳过；卸载克隆索引仍跳过 | AC-1.2, AC-4.1, AC-5.1 |

mock `BundleMgrHelper::GetDualModeBundleInfo` 返回 ERR_OK 或错误，验证 `DualModeContext.isDualMode` 取值。

---

## 第五部分：实施顺序与依赖

| 阶段 | 文件 | 依赖 | 验证 |
|---|---|---|---|
| 1 | 文件 2/4/6/8 头文件签名 + 文件 10 构建 | 无 | 编译通过（stub 实现） |
| 2 | 文件 5（storage_mgr.cpp） | 阶段 1 | `insight_intent_rdb_storage_mgr_test` 通过 |
| 3 | 文件 7（db_cache.cpp） | 阶段 2 | `insight_intent_db_cache_test` 通过 |
| 4 | 文件 3（event_mgr.cpp） | 阶段 3 | `insight_intent_event_mgr` 相关测试通过 |
| 5 | 文件 1（bundle_event_callback.cpp） | 阶段 4 | 编译通过 + 安装/更新事件端到端 |
| 6 | 文件 9（execute_manager.cpp） | 阶段 3 | `insight_intent_execute_manager_test` 通过 |
| 7 | `ability_manager_service.cpp` 上游适配 | 阶段 6 | 全量编译 + `ability_manager_service_*_test` 通过 |
| 8 | 测试用例补齐 | 阶段 1-7 | 全量 UT 通过 |

---

## 第六部分：风险与注意事项

| # | 风险 | 缓解 |
|---|---|---|
| 1 | `GetDualModeBundleInfo` 是 IPC 调用，事件层每次安装/更新/卸载调一次 | 仅事件层调一次，结果透传 `DualModeContext`，存储/缓存层不发起 IPC |
| 2 | RDB 老数据 Key 不含 appIndex 段，`Transform` 解析需兼容 5 段/6 段两种格式 | 解析按 `/` 分割后判断段数：5 段=老数据 appIndex=0，6 段=双模式 |
| 3 | `bundleVersionMap_` 同一 bundleName 两个 appIndex 版本可能不同 | map key 改为 `bundleName/appIndex`（双模式）或 `bundleName`（非双模式） |
| 4 | `InsightIntentExecuteRecord` 加 appIndex 字段影响执行记录查找 | 现有按 `uint64_t key` 查找，appIndex 作为附加过滤，不破坏现有索引 |
| 5 | 上游 `AbilityManagerService` 多处调用意图接口需适配 | 阶段 7 集中处理，按 caller AbilityRecord 取 appIndex |
| 6 | `global_constant.h` include 路径 | 阶段 1 编译验证，必要时改 BUILD.gn |
| 7 | `OnInsightAppDied` 按 bundleName 清理可能误清另一模式 | 改为 bundleName + appIndex 精确清理 |
| 8 | 现有 `DeleteInsightIntentEvent` 的 `appIndex>0 跳过` 注释 "simulation" | 改为精确判断后更新注释，避免误导 |
| 9 | 缓存层与存储层 Key 不一致 | 同 Task 内同步修改两层，Anti-Fake Completion 增加一致性检查 |

---

## 第七部分：验证命令（参照 AGENTS.md）

```bash
# 编译验证
./build.sh --product-name <product> --build-target ability_runtime

# AMS 特定编译
./build.sh --product-name <product> --build-target abilityms

# 单元测试
run -t UT -ts insight_intent_rdb_storage_mgr_test
run -t UT -ts insight_intent_db_cache_test
run -t UT -ts insight_intent_execute_manager_test
run -t UT -ts insight_intent_execute_manager_second_test

# AMS 全量回归
run -t UT -ts ability_manager_service_first_test
run -t UT -ts ability_manager_service_second_test

# Fuzz
run -t UT -ts AbilityAppDebugInfoFuzzTest
```

---

## 附录：兼容性 / 权限 / DFX 影响评估

| 维度 | 影响 | 评估 |
|------|------|------|
| 公共 API | 无 | 仅内部实现变更，不涉及 Public/System/InnerAPI 接口签名 |
| 权限 | 无 | 不涉及 PermissionVerification 校验逻辑 |
| IPC 协议 | 无 | 不修改 Stub/Proxy 序列化格式 |
| DFX | 无 | 不修改 hisysevent.yaml 事件定义 |
| 构建组件 | 无 | 无新增 component，无新增依赖；可能需补 `global_constant.h` include 路径 |
| 数据迁移 | 无 | 非双模式设备老数据由 GetDualModeBundleInfo 返回失败自动兼容；不回溯 7.0 数据 |
| 特性标志 | 无 | 不修改 `ability_runtime.gni` 特性标志默认值 |

---

**方案状态**：核心逻辑已稳定，判断入口 `GetDualModeBundleInfo` 已确认，真实路径已验证，11 个修改文件全部映射到真实源码。可按阶段 1-8 顺序实施。
