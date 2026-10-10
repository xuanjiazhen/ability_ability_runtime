# PC 双模式意图存储 Key 扩展 — 代码修改说明

> 本文档详细描述本次代码修改的原因、每个文件的具体修改内容，以及遗漏检查结果。

---

## 一、修改原因

### 1.1 业务背景

PC 设备支持双模式运行（PC 模式 / PAD 模式），同一应用可能以相同 bundleName 安装主/副两份实例（同名不同包）。现有意图存储 Key 仅按 `userId/bundleName/moduleName/intentName/versionCode` 区分，无法区分同名不同包应用的主/副模式意图，导致数据混淆。

### 1.2 同名不同包判断依据

通过应用包信息的 `DeviceModeDistributionPolicy` 分发策略判断（定义见 `bundle_info.h:162`）：

```cpp
enum class DeviceModeDistributionPolicy : int32_t {
    UNSPECIFIED = 0,
    MAIN_ONLY = 1,
    SUB_ONLY = 2,
    UNIVERSAL_IDENTICAL_PACKAGE = 3,              // 同名同包
    UNIVERSAL_DIFFERENT_PACKAGE = 4,              // 同名不同包 ←
    PARTIAL_COMPATIBLE_IDENTICAL_PACKAGE = 5,     // 同名同包
    PARTIAL_COMPATIBLE_DIFFERENT_PACKAGE = 6,     // 同名不同包 ←
    FULL_COMPATIBLE_IDENTICAL_PACKAGE = 7,        // 同名同包
    FULL_COMPATIBLE_DIFFERENT_PACKAGE = 8,        // 同名不同包 ←
};
```

**判断逻辑**：`BundleInfo.deviceModeDistributionPolicy` 值为 4 / 6 / 8（`*_DIFFERENT_PACKAGE`）时，该应用为同名不同包，意图存储 Key 追加 `appIndex` 段；否则为同名同包，Key 不追加（向后兼容）。

**appIndex 来源**：`BundleInfo.appIndex`（`bundle_info.h:202`），主模式=0，副模式=PC_TABLET_INDEX(10000)。

### 1.3 核心修改思路

1. **Key 结构扩展**：同名不同包应用的 Key 追加 `appIndex` 段（主模式 0 也追加），同名同包 Key 不变
2. **判断入口**：事件层调 `GetBundleInfoV9` 获取 `BundleInfo`，读 `deviceModeDistributionPolicy` 判断是否 4/6/8
3. **分层透传**：事件层判断一次，结果 `(appIndex, isDualMode)` 透传存储/缓存/执行层，避免每层重复查询

---

## 二、修改文件清单（13 个文件）

| # | 文件 | 层 | 修改概述 |
|---|---|---|---|
| 1 | `insight_intent_event_mgr.h` | 事件处理 | 新增 `DualModeContext` 结构体；方法签名加 appIndex |
| 2 | `insight_intent_event_mgr.cpp` | 事件处理 | 新增 `ResolveDualModeContext`（读 `deviceModeDistributionPolicy`）；Update/Delete 适配 |
| 3 | `insight_intent_rdb_storage_mgr.h` | 存储层 | `InsightIntentSaveParam` 加字段；方法签名加参数；新增 `BuildIntentKey`/`BuildBundlePrefix` |
| 4 | `insight_intent_rdb_storage_mgr.cpp` | 存储层 | 新增 Key 拼接辅助函数；8 处内联 Key 替换 |
| 5 | `insight_intent_db_cache.h` | 缓存层 | 10 个方法签名加 appIndex/isDualMode 参数 |
| 6 | `insight_intent_db_cache.cpp` | 缓存层 | 各方法透传；`bundleVersionMap_` key 按 appIndex 区分 |
| 7 | `insight_intent_execute_manager.h` | 执行层 | `CheckAndUpdateParam`/`CheckAndUpdateQueryEntityParam` 加参数；record 加字段 |
| 8 | `insight_intent_execute_manager.cpp` | 执行层 | 实现签名匹配；`CheckAndUpdateQueryEntityParam` 内部透传 |
| 9 | `ability_bundle_event_callback.cpp` | 包事件入口 | 2 处 `UpdateInsightIntentEvent` 调用补 appIndex |
| 10 | `insight_intent_sys_event_receiver.h` | 系统事件 | `SaveInsightIntentInfos`/`CollectInsightIntentSaveParam`/`DeleteInsightIntent` 加参数 |
| 11 | `insight_intent_sys_event_receiver.cpp` | 系统事件 | 包扫描批量加载路径从 `bundleInfo.deviceModeDistributionPolicy` 判断并透传 |
| 12 | `mock_insight_intent_db_cache.h` | 测试 mock | mock 头文件签名同步 |
| 13 | `mock_insight_intent_db_cache.cpp` | 测试 mock | mock 实现签名同步 |

---

## 三、各文件详细修改内容

### 3.1 `insight_intent_event_mgr.h` — 事件处理头文件

**修改原因**：`UpdateInsightIntentEvent` 签名无 appIndex，无法接收同名不同包信息；需要携带判断结果的数据结构。

**修改内容**：
- 新增 `DualModeContext` 结构体（`int32_t appIndex` + `bool isDualMode`），携带事件层一次判断的结果
- `UpdateInsightIntentEvent` 加 `int32_t appIndex = 0` 参数（默认 0 = 同名同包，保证旧调用编译通过）
- `DeleteInsightIntent`（私有）加 `const DualModeContext &ctx` 参数

### 3.2 `insight_intent_event_mgr.cpp` — 事件处理实现

**修改原因**：这是同名不同包判断的入口层。原有 `DeleteInsightIntentEvent` 对 `appIndex > 0` 一刀切跳过（注释 "simulation"），误伤副模式主应用；原有 `UpdateInsightIntentEvent` 签名无 appIndex。

**修改内容**：

1. **新增 `ResolveDualModeContext` 辅助函数**（匿名 namespace）：
   - 调 `GetBundleInfoV9` 获取 `BundleInfo`
   - 读 `bundleInfo.deviceModeDistributionPolicy`，判断是否 `UNIVERSAL_DIFFERENT_PACKAGE(4)` / `PARTIAL_COMPATIBLE_DIFFERENT_PACKAGE(6)` / `FULL_COMPATIBLE_DIFFERENT_PACKAGE(8)`
   - 是 → `ctx.isDualMode = true`，`ctx.appIndex = bundleInfo.appIndex`
   - 否 → `ctx.isDualMode = false`，`ctx.appIndex = 0`
   - 事件层判断一次，结果透传下游，存储/缓存层不再发起 IPC

2. **`UpdateInsightIntentEvent`**：签名加 `appIndex`；lambda 前调 `ResolveDualModeContext`；lambda 内 saveParams 每项填入 `ctx.appIndex`/`ctx.isDualMode`；`DeleteInsightIntent` 调用透传 `ctx`

3. **`DeleteInsightIntentEvent`**：核心判断逻辑从 `appIndex > 0 跳过` 改为：
   - 调 `ResolveDualModeContext` 判断
   - 卸载兜底：`GetBundleInfoV9` 失败时按 `appIndex == PC_TABLET_INDEX(10000)` 判断为同名不同包
   - `!ctx.isDualMode && appIndex > 0` 才跳过（克隆/DLP/AI Agent 索引）
   - 同名不同包应用（主模式 0 / 副模式 10000）都执行删除

4. **`DeleteInsightIntent`**：签名加 `ctx`；透传给 `DeleteInsightIntentTotalInfo`

5. **include**：新增 `#include "global_constant.h"`（PC_TABLET_INDEX 常量）

### 3.3 `insight_intent_rdb_storage_mgr.h` — 存储层头文件

**修改原因**：8 处 Key 拼接散落，无统一入口；方法签名无 appIndex；`InsightIntentSaveParam` 无 appIndex 字段。

**修改内容**：
- `InsightIntentSaveParam` 加 `int32_t appIndex = 0` + `bool isDualMode = false`
- 8 个方法（Load/Save/Delete 系列）签名加 `int32_t appIndex = 0, bool isDualMode = false`
- 新增私有静态方法 `BuildIntentKey` / `BuildBundlePrefix`（集中 Key 拼接逻辑）
- 全量加载/删除方法（`LoadInsightIntentBundleInfos`/`LoadInsightIntentInfos`/`LoadConfigInsightIntentInfos`/`DeleteStorageInsightIntentByUserId`）不改（不区分单个 appIndex）

### 3.4 `insight_intent_rdb_storage_mgr.cpp` — 存储层实现

**修改原因**：8 处内联 Key 拼接需统一按 `isDualMode` 决定是否追加 appIndex 段。

**修改内容**：

1. **新增 `BuildIntentKey`**：完整 Key `userId/bundleName/moduleName/intentName/versionCode`，`isDualMode` 时追加 `/appIndex`

2. **新增 `BuildBundlePrefix`**：前缀 `userId/bundleName/`，`isDualMode` 时追加 `appIndex/`

3. **替换 8 处内联 Key 拼接**：
   - `:114-115`/`:129-130`（按 bundle 查询前缀）→ `BuildBundlePrefix`
   - `:145-146`/`:173-174`（按意图名查询）→ `BuildBundlePrefix` + module/intent
   - `:247-248`/`:262-263`（Save profile/config）→ `BuildIntentKey`
   - `:293-295`/`:309-311`（Batch Save profile/config）→ `BuildIntentKey`（appIndex 从 `saveParam.appIndex` 取）
   - `:361-362`/`:364-365`（Delete）→ `BuildBundlePrefix`

4. **兼容性**：`INTENT_KEY_LENGTH = 5` 检查保持 `>= 5`，6 段 Key（同名不同包）也通过；`BUNDLE_NAME = 1`/`VERSION = 4` 索引在 6 段时仍正确（前 5 段位置不变）

### 3.5 `insight_intent_db_cache.h` — 缓存层头文件

**修改原因**：缓存层是存储层薄封装，签名必须对齐；缓存与存储 Key 不一致会导致命中缓存 vs 落库结果不一致。

**修改内容**：10 个方法（Get/Save/Delete/Has 系列）加 `int32_t appIndex = 0, bool isDualMode = false`。全量加载方法不改。

### 3.6 `insight_intent_db_cache.cpp` — 缓存层实现

**修改原因**：透传 appIndex/isDualMode 给存储层；`bundleVersionMap_` 的 key 需按 appIndex 区分。

**修改内容**：
- 各方法透传参数给 `InsightRdbStorageMgr` 对应方法
- `bundleVersionMap_` key 改为 `bundleName + "/" + appIndex`（isDualMode）或 `bundleName`（非 isDualMode）
- `SaveInsightIntentTotalInfo`/`SaveBatchInsightIntentTotalInfo`/`DeleteInsightIntentTotalInfo`/`HasInsightIntentByName`/`HasBundleCache` 内部 map 操作统一使用上述 key 格式

### 3.7 `insight_intent_execute_manager.h` — 执行层头文件

**修改原因**：执行入口需接收 appIndex/isDualMode 用于按模式匹配。

**修改内容**：
- `CheckAndUpdateParam` 加 `int32_t appIndex = 0, bool isDualMode = false`
- `CheckAndUpdateQueryEntityParam` 加 `int32_t appIndex = 0, bool isDualMode = false`
- `InsightIntentExecuteRecord` 加 `int32_t appIndex = 0` + `bool isDualMode = false`（供 `OnInsightAppDied` 精确清理）

### 3.8 `insight_intent_execute_manager.cpp` — 执行层实现

**修改原因**：实现签名匹配头文件；查询意图时透传 appIndex。

**修改内容**：
- `CheckAndUpdateParam` 实现签名匹配；appIndex/isDualMode 存入 `InsightIntentExecuteRecord`
- `CheckAndUpdateQueryEntityParam` 实现签名匹配；内部 `GetInsightIntentInfo` 调用透传 appIndex/isDualMode

### 3.9 `ability_bundle_event_callback.cpp` — 包事件入口

**修改原因**：安装（PACKAGE_ADDED）/更新（PACKAGE_CHANGED）链路 `UpdateInsightIntentEvent` 调用未传 appIndex，导致同名不同包应用安装/更新时意图模块拿不到 appIndex。`appIndex` 已在 `:61` 读取（`want.GetIntParam(KEY_APP_INDEX, 0)`），只需透传。

**修改内容**：`:93` 和 `:113` 两处 `UpdateInsightIntentEvent` 调用补 `appIndex` 参数。

### 3.10 `insight_intent_sys_event_receiver.h` — 系统事件头文件

**修改原因**：包扫描完成后批量加载路径（`LoadInsightIntentInfos`）构造 saveParams 时未填 appIndex/isDualMode，同名不同包应用 Key 不带 appIndex，后续查询/删除用错误 Key 格式。

**修改内容**：`SaveInsightIntentInfos`/`CollectInsightIntentSaveParam`/`DeleteInsightIntent` 三个私有方法加 `int32_t appIndex, bool isDualMode` 参数。

### 3.11 `insight_intent_sys_event_receiver.cpp` — 系统事件实现

**修改原因**：批量加载路径需从 `bundleInfo.deviceModeDistributionPolicy` 判断同名不同包并透传。

**修改内容**：
- `LoadInsightIntentInfos`：遍历 `bundleInfos` 时读 `bundleInfo.deviceModeDistributionPolicy`，判断是否 4/6/8，得到 `isDifferentPackage` 和 `appIndex`；`HasInsightIntentByName` 和 `SaveInsightIntentInfos` 调用透传
- `SaveInsightIntentInfos`：透传给 `CollectInsightIntentSaveParam`
- `CollectInsightIntentSaveParam`：saveParam 填入 `appIndex`/`isDualMode`
- `DeleteInsightIntent`：透传给 `DeleteInsightIntentTotalInfo`
- `CollectInsightIntentSaveParam` 内部 2 处 `DeleteInsightIntent` 调用补参数

### 3.12-3.13 mock 文件

**修改原因**：mock 头文件/实现签名必须与真实头文件一致，否则编译失败。

**修改内容**：`mock_insight_intent_db_cache.h` 和 `.cpp` 中 9 个方法签名同步加 `int32_t appIndex = 0, bool isDualMode = false`。

---

## 四、遗漏检查结果

### 4.1 已修复的遗漏（本轮检查发现并修复）

| # | 遗漏点 | 位置 | 修复状态 |
|---|---|---|---|
| 1 | 包扫描批量加载路径未透传 appIndex | `sys_event_receiver.cpp` `SaveInsightIntentInfos` | ✅ 已修复 |
| 2 | 包扫描删除路径未透传 appIndex | `sys_event_receiver.cpp` `DeleteInsightIntent` | ✅ 已修复 |
| 3 | 包扫描版本检查未透传 appIndex | `sys_event_receiver.cpp` `HasInsightIntentByName` | ✅ 已修复 |
| 4 | `CollectInsightIntentSaveParam` 内部 Delete 调用未透传 | `sys_event_receiver.cpp` :89/:102 | ✅ 已修复 |

### 4.2 已确认无遗漏的路径

| 路径 | 验证结果 |
|---|---|
| 安装/更新（PACKAGE_ADDED/PACKAGE_CHANGED） | ✅ `ability_bundle_event_callback.cpp` 透传 appIndex |
| 卸载（PACKAGE_REMOVED） | ✅ `DeleteInsightIntentEvent` 精确判断 + 卸载兜底 |
| 包扫描批量加载（BUNDLE_SCAN_FINISHED） | ✅ `LoadInsightIntentInfos` 从 `deviceModeDistributionPolicy` 判断并透传 |
| 存储层 Key 拼接（8 处） | ✅ 全部替换为 `BuildIntentKey`/`BuildBundlePrefix` |
| 缓存层 map key 一致性 | ✅ `bundleVersionMap_` 按 appIndex 区分 |
| mock 签名一致性 | ✅ 与真实头文件一致 |

### 4.3 已知限制（用默认值，留作后续优化）

以下路径因方法签名有默认值（`appIndex=0, isDualMode=false`）能编译通过，但对同名不同包应用会行为不正确。这些是执行/查询路径，不影响安装/更新/卸载数据存储。

| 路径 | 位置 | 影响 | 优先级 |
|---|---|---|---|
| 意图匹配器查询 | `insight_intent_matcher.cpp` `GetMatchedIntentInfos` | 同名不同包应用 matcher 查不到意图 | 后续 |
| 执行入口 `CheckAndUpdateParam` 内部调用 | `execute_manager.cpp` `UpdateEntryDecoratorParams`/`CheckAndUpdateDecoratorParams` | 同名不同包执行时查不到意图 | 后续 |
| `OnInsightAppDied` 清理 | `execute_manager.cpp` :854 | 按 bundleName 清理可能误清另一模式 | 后续 |
| 上游 `ability_manager_service.cpp` 调用点 | 多处 | 执行/查询入口未传 appIndex | 后续 |

**说明**：安装/更新/卸载/包扫描路径已完整适配，同名不同包应用的意图能正确存储和删除。执行/查询路径的精确匹配需要上游（`ability_manager_service.cpp`）传入 caller 的 appIndex 后才能生效，留作后续优化。

---

## 五、兼容性说明

| 场景 | Key 格式 | 兼容性 |
|---|---|---|
| 同名同包应用（policy 非 4/6/8） | `userId/bundleName/moduleName/intentName/versionCode`（不变） | ✅ 向后兼容 |
| 同名不同包应用主模式（policy=4/6/8, appIndex=0） | `userId/bundleName/moduleName/intentName/versionCode/0`（追加） | 新格式 |
| 同名不同包应用副模式（policy=4/6/8, appIndex=10000） | `userId/bundleName/moduleName/intentName/versionCode/10000`（追加） | 新格式 |
| RDB 老数据（5 段 Key） | 全量加载时 `INTENT_KEY_LENGTH=5` 检查通过，`BUNDLE_NAME=1`/`VERSION=4` 索引正确 | ✅ 兼容 |

---

## 六、验证命令

```bash
# 编译（需完整 OpenHarmony 构建环境，ohos-sdk 不含服务层 target）
./build.sh --product-name <product> --build-target ability_runtime

# 单元测试
run -t UT -ts insight_intent_rdb_storage_mgr_test
run -t UT -ts insight_intent_db_cache_test
run -t UT -ts insight_intent_execute_manager_test
run -t UT -ts insight_intent_sys_event_receiver_test

# AMS 回归
run -t UT -ts ability_manager_service_first_test
```
