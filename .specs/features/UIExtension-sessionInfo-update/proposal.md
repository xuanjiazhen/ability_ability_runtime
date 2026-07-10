# UIExtension sessionInfo 支持更新

## 原始需求

**需求来源**：【543-2生态项目】【P3】【PC专项】红莲花浏览器（海泰浏览器）

**需求名称**：UIExtension sessionInfo 支持更新

**需求背景（RR）**：
浏览器通过插件机制跨进程嵌入 Qt 程序的场景，浏览器侧需要支持把嵌入 Qt 界面的那个页面通过窗口分合拆成一个单独的窗口，或者反过来合成一个窗口，依赖系统 UEC（UIExtension Component）跨窗口能力。当前 UEC 组件无法从原窗口拖出/迁移到新窗口，影响用户体验。

**SR 描述**：

- **输入**：同进程的 UEC 组件窗口间迁移
- **处理**：
  1. `startUIExtension` 支持更新 sessionInfo 入参
  2. 新的 sessionInfo 信息传递给 UIExtension 提供方的 window 子系统
- **输出**：sessionInfo 设置成功
- **验收场景和标准**：
  1. 通过 arkUI 组件接口，将 UEC 迁移到另一个窗口上成功
  2. 跨 Ability 迁移时，dump 信息的 abilityName 为迁移后的目标 abilityName

**SR 功能需求设计（原始）**：
1. 为了防止新增接口带来状态并发问题，使用老接口加新参数的方式保证状态机一致性
2. 在 sessionInfo 中新增 `originalCallerInfo`，内含 `callerToken`（默认空）和 `persistentId`（默认 0）
3. 如果 `originalCallerInfo` 里的 `callerToken` 和 `persistentId` 不为默认值，标识 SessionInfo 需要更新
4. 在 `StartUIExtensionAbility` 的流程中适配 `GetOrCreateExtensionRecord`：增加 SessionInfo 更新的逻辑
5. 应用侧调用窗口的更新接口更新 session

---

## 现有逻辑分析（前期代码调研结论）

本次需求的核心改动点在 `GetOrCreateExtensionRecord`。以下是对现有逻辑的关键分析结论：

### 现有匹配机制

`ExtensionRecordManager::GetAbilityRecordBySessionInfo`（`extension_record_manager.cpp:264-293`）通过 **`uiExtensionComponentId` 精确匹配** 来查找已存在的 extension record。

`uiExtensionComponentId` 的生成公式（`ability_manager_service.cpp:4942-4944`）：

```cpp
// OFFSET = 32
uiExtensionComponentId = (callerRecordId << OFFSET) | persistentId
```

- 高 32 位：`callerRecordId`（调用方 Ability 的内部 record ID）
- 低 32 位：`persistentId`（session/window 的持久 ID）

### 迁移场景的核心问题

当 UEC 从窗口 A 迁移到窗口 B（同进程内跨 Ability）时：
- 新窗口 B 对应不同的 `callerRecordId` 和 `persistentId`
- 因此新的 `uiExtensionComponentId` 与原 record 不同
- `GetAbilityRecordBySessionInfo` 用新 sessionInfo **无法匹配到原 record**
- 结果：会错误地创建一个新 record，而非更新原 record

**结论**：需要一种机制，用迁移前的原始调用方信息定位到原 record，再更新其 sessionInfo。这正是 `originalCallerInfo` 的设计目的。

### 现有 sessionInfo 更新链路

- `StartAbilityLocked`（`ui_extension_ability_manager.cpp:664-676`）在调用 `GetOrCreateExtensionRecord` 之前已无条件执行 `SetSessionInfo` 和 `uiExtensionMap_` 更新
- `DoForegroundUIExtension`（同文件 :759-805）通过 `CommandAbilityWindow` → `ScheduleCommandAbilityWindow` 将 sessionInfo 传递到应用进程/window 子系统
- 已有 `Update()` 模式（preload record 和 reuse record 场景）调用 `SetSessionInfo`，可参考

---

## 澄清记录

| # | 模糊点 | 澄清结论 |
|---|--------|----------|
| 1 | 本次 proposal 的仓范围边界 | **仅 ability_runtime 部分**。本次 proposal 覆盖 ability_runtime 仓的改动（`StartUIExtensionAbility` 流程适配、`GetOrCreateExtensionRecord` 更新逻辑）。ability_base（SessionInfo 结构体新增字段）、window_manager（窗口更新接口）、arkui（组件接口）等各自建独立 proposal。本仓作为下游消费方依赖 ability_base 的结构体变更 |
| 2 | `originalCallerInfo` 与现有 callerToken/persistentId 的语义关系 | 迁移时 sessionInfo 的 `callerToken`/`persistentId` 更新为新窗口值；`originalCallerInfo` 保存迁移前的原始调用方信息，用于在 `GetOrCreateExtensionRecord` 里**定位原 record** 再更新其 sessionInfo。`originalCallerInfo` 非默认值 = "这是一次迁移更新请求" |
| 3 | 迁移场景确切范围 | **同进程内，支持跨调用方 Ability**（不同 Ability 实例的窗口间迁移）。不限于同一调用方 Ability，但限于同一进程内。不支持跨进程/分布式迁移 |
| 4 | SR 验收标准 2"跨 Ability 迁移"与范围对齐 | **确实需要支持跨 Ability**。迁移后 dump 信息的 abilityName 应为迁移后的目标 abilityName |
| 5 | "老接口加新参数"的含义 | 不新增独立的 "UpdateSession" API；复用现有 `StartUIExtensionAbility` 入口，通过 sessionInfo 携带 `originalCallerInfo` 标识迁移更新请求，在 `GetOrCreateExtensionRecord` 内部分流处理。避免新增接口带来的状态并发问题 |
| 6 | 现有 `GetAbilityRecordBySessionInfo` 的匹配机制 | 按 `uiExtensionComponentId` 精确匹配。迁移后 `uiExtensionComponentId` 变化导致无法找到原 record，需要通过 `originalCallerInfo` 提供替代匹配键 |
| 7 | 多次迁移时 originalCallerInfo 的更新策略 | **开放设计问题，留待 design.md 决策**。选项：(A) originalCallerInfo 始终指向最初调用方；(B) 每次迁移后更新为上一次的调用方。涉及多领域（ability_runtime / window_manager）之间的约定 |

---

## 需求基线

### 范围

- **包含**：
  - `StartUIExtensionAbility` 流程中识别 `originalCallerInfo` 非默认值，进入"迁移更新"分支
  - `ExtensionRecordManager` 新增按 `originalCallerInfo`（原始 callerToken + persistentId）定位原 extension record 的逻辑
  - 定位到原 record 后，更新其 sessionInfo 为新值（callerToken、persistentId、uiExtensionComponentId 等）
  - 更新后通过已有 `CommandAbilityWindow` → `ScheduleCommandAbilityWindow` 链路将新 sessionInfo 传递到 UIExtension 提供方 window 子系统
  - `uiExtensionMap_` 的一致性维护（旧 sessionToken 映射清理 + 新 sessionToken 映射建立）
  - 单元测试覆盖迁移更新分支

- **排除**：
  - ability_base 的 `SessionInfo` 结构体新增 `originalCallerInfo` 字段及 Marshalling/Unmarshalling（独立 proposal，本仓依赖）
  - window_manager 的窗口更新接口（独立 proposal）
  - arkui 的 ArkUI 组件接口（独立 proposal）
  - 跨进程 / 分布式迁移场景
  - 多次迁移策略的最终设计（design.md 决策，见澄清记录 #7）
  - 修改现有 `GetAbilityRecordBySessionInfo` 按 `uiExtensionComponentId` 匹配的主路径逻辑（新增并行分支，不破坏现有路径）

### 涉及子系统/仓

| 仓 | 子系统/路径 | 说明 |
|----|------------|------|
| ability_runtime | `services/abilitymgr/src/ability_manager_service.cpp` | `StartUIExtensionAbility` 入口，识别 originalCallerInfo 标识 |
| ability_runtime | `services/abilitymgr/src/extension_record/extension_record_manager.cpp` | `GetOrCreateExtensionRecord` 新增迁移更新分支，按 originalCallerInfo 定位原 record |
| ability_runtime | `services/abilitymgr/include/extension_record/extension_record_manager.h` | 新增按 originalCallerInfo 查找的函数声明 |
| ability_runtime | `services/abilitymgr/src/ui_extension/ui_extension_ability_manager.cpp` | `StartAbilityLocked` / `DoForegroundUIExtension` 适配，确保 sessionInfo 更新后正确传递 |
| ability_runtime | `test/unittest/` | 新增迁移更新分支的单元测试 |

**跨仓依赖（非本次范围，但需对齐）**：

| 仓 | 说明 |
|----|------|
| ability_base | `session_info.h` 新增 `originalCallerInfo` 字段 + Marshalling（前置依赖） |
| window_manager | 应用侧窗口更新接口（后置消费方） |
| arkui_ace_engine | ArkUI 组件接口（后置消费方） |

### 复杂度级别

**标准** — 单仓（ability_runtime）特性，涉及 `GetOrCreateExtensionRecord` 核心调度路径的新增分支 + sessionInfo 更新 + map 一致性维护；无跨 SIG 协调，无安全/性能关键路径。有一个开放设计问题（多次迁移策略）留待 design.md 决策。

---

## 验收标准 (P0)

### AC1: originalCallerInfo 非默认值标识迁移更新请求
**WHEN** `StartUIExtensionAbility` 收到的 sessionInfo 中 `originalCallerInfo.callerToken` 不为空且 `originalCallerInfo.persistentId` 不为 0
**THEN** AMS 识别为"迁移更新请求"，进入 `GetOrCreateExtensionRecord` 的新增迁移更新分支（不走默认的"创建新 record"路径）

### AC2: 按 originalCallerInfo 定位原 extension record
**WHEN** 迁移更新分支被触发执行
**THEN** `ExtensionRecordManager` 使用 `originalCallerInfo` 中的原始 callerToken 和 persistentId，在已存在的 extensionRecords_ 中匹配到迁移前的原 record（而非创建新 record）

### AC3: 原 record 的 sessionInfo 被更新为新值
**WHEN** 通过 originalCallerInfo 成功定位到原 record
**THEN** 原 record 的 sessionInfo 被更新为新的 callerToken、persistentId、uiExtensionComponentId（反映迁移后的目标窗口/Ability），更新后 `GetSessionInfo()` 返回的是新 sessionInfo

### AC4: 迁移更新不创建新 record（isLoaded=true）
**WHEN** 迁移更新分支成功执行
**THEN** `GetOrCreateExtensionRecord` 返回 `isLoaded=true`，不触发 `LoadAbility` 创建新 Ability 实例；原 UIExtensionAbility 进程/组件保持存活，仅 session 绑定关系变更

### AC5: 新 sessionInfo 传递到 UIExtension 提供方 window 子系统
**WHEN** 迁移更新完成后触发 `DoForegroundUIExtension`
**THEN** 通过 `CommandAbilityWindow` → `ScheduleCommandAbilityWindow` IPC 链路，新的 sessionInfo（含迁移后的目标窗口信息）被传递到 UIExtension 提供方进程的 window 子系统

### AC6: dump 信息反映迁移后状态
**WHEN** 迁移更新完成后执行 `aa dump -a` 或等效 dump 命令
**THEN** dump 信息中该 UIExtension session 关联的 abilityName 为迁移后的目标 abilityName，persistentId 为目标窗口的 persistentId

### AC7: uiExtensionMap_ 一致性维护
**WHEN** 迁移更新执行时
**THEN** `uiExtensionMap_` 中旧的 sessionToken 映射被清理，新的 sessionToken 映射被建立；迁移后通过新 sessionToken 查询 `GetUIExtensionBySessionInfo` 能正确返回原 record

---

## 验收标准 (P1)

### AC8: 无 originalCallerInfo 时走原有逻辑
**WHEN** sessionInfo 中 `originalCallerInfo` 为默认值（callerToken 为空、persistentId 为 0）或不存在
**THEN** `GetOrCreateExtensionRecord` 走原有逻辑（按 `uiExtensionComponentId` 匹配或创建新 record），行为与本次变更前完全一致

### AC9: originalCallerInfo 无法匹配到原 record 时的容错
**WHEN** 迁移更新分支被触发，但 `originalCallerInfo` 在 extensionRecords_ 中无法匹配到任何已存在的 record（如原 record 已销毁）
**THEN** AMS 侧有错误日志（AAFwkTag::ABILITYMGR），返回错误码，不发生 crash，不影响其他 Ability 调度

### AC10: 并发安全性
**WHEN** 迁移更新执行期间，其他线程并发访问同一 extension record 的 sessionInfo 或 uiExtensionMap_
**THEN** "按 originalCallerInfo 定位原 record + 更新 sessionInfo + 更新 uiExtensionMap_" 过程具有原子性或足够的锁保护，不出现部分更新导致的中间状态

### AC11: 单元测试覆盖
**WHEN** 执行 `test/unittest/` 下本次新增/扩展的测试套件
**THEN** 覆盖以下场景：originalCallerInfo 标识识别、按 originalCallerInfo 定位原 record、sessionInfo 更新成功、isLoaded=true 不创建新 record、dump 信息正确、uiExtensionMap_ 一致性、无 originalCallerInfo 走原逻辑、匹配失败容错；所有用例通过

### AC12: 日志可观测性
**WHEN** 迁移更新分支任意环节被触发
**THEN** 关键节点有 TAG_LOGI/TAG_LOGD 日志（含 originalCallerToken、originalPersistentId、新 callerToken、新 persistentId、uiExtensionComponentId、bundleName 等关键字段），便于线上问题定位
