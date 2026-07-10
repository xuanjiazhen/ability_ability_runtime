# 特性规格：UIExtension sessionInfo 支持更新

## 1. 特性概述

当 UIExtension 组件（UEC）需要从当前窗口迁移到同进程内的另一个窗口（可属于不同 Ability 实例）时，ability_runtime 通过复用现有 `StartUIExtensionAbility` 接口（不新增 API），在 sessionInfo 中携带 `originalCallerInfo` 标识"迁移更新请求"，在 `GetOrCreateExtensionRecord` 中按 `originalCallerInfo` 定位迁移前的原 extension record，更新其 sessionInfo 为新窗口信息，并通过已有 `CommandAbilityWindow` 链路将新 sessionInfo 传递到 UIExtension 提供方的 window 子系统。

迁移更新分支是 `GetOrCreateExtensionRecord` 的新增前置分支，与现有按 `uiExtensionComponentId` 匹配的主路径、预加载路径、创建新 record 路径完全隔离，现有代码逐行不变。

## 2. 用户故事

### US-1: UEC 跨窗口迁移
**作为** 浏览器（或同类嵌入式应用）用户
**我想要** 把嵌入的 UEC 界面从一个窗口拖出/迁移到另一个独立窗口，或反向合并
**以便** 灵活组织窗口布局，提升多窗口操作体验

### US-2: 迁移后 UEC 提供方感知新窗口
**作为** UIExtension 提供方（如被嵌入的 Qt 程序）
**我想要** 迁移后我的 window 子系统能收到更新后的 sessionInfo（新窗口信息）
**以便** 正确绑定到新窗口进行渲染和交互

### US-3: 框架状态一致性
**作为** ability_runtime 维护者
**我想要** 迁移更新通过现有 `StartUIExtensionAbility` 入口进入，在 `GetOrCreateExtensionRecord` 内部处理，而非新增独立 API
**以便** 复用现有状态机保证一致性，避免新增接口带来的状态并发问题

### US-4: 现有路径不受影响
**作为** ability_runtime 维护者
**我想要** 非 migration 场景（无 originalCallerInfo）的 StartUIExtensionAbility 调用行为与变更前完全一致
**以便** 不引入回归风险

## 3. API 影响

### 3.1 公共 SDK / JS / ArkTS 接口变更

**无变更**。本次不新增、不修改任何 JS/ArkTS 公共 SDK 接口。应用侧通过 window_manager / arkui 组件接口（各自独立 proposal）发起迁移请求。

### 3.2 Inner API / IPC 接口变更

**无新增 IPC 接口**。复用现有 `StartUIExtensionAbility` IPC 入口。

### 3.3 Native 内部接口（ability_runtime 内部）

| 接口 | 类型 | 说明 |
|------|------|------|
| `ExtensionRecordManager::GetAbilityRecordByPersistentId(persistentId)` | 新增私有方法 | 按 persistentId 匹配已存在的 extension record（迁移时 persistentId 不变） |
| `ExtensionRecordManager::IsMigrationUpdateRequest(sessionInfo)` | 新增私有方法 | 判断 sessionInfo 是否为迁移更新请求（originalCallerInfo.callerToken 非空） |
| `UIExtension::HandleSessionUpdate(want, sessionInfo)` | 新增虚方法 | 基类默认空实现；OnCommandWindow 检测到迁移时调用 |
| `JsUIExtension::HandleSessionUpdate(want, sessionInfo)` | 重写方法 | 按 persistentId 索引查找 oldComponentId + 更新 contentSessions_/uiWindowMap_ key + napi_unwrap 调 `DoUpdateExtensionParent` |
| `JsUIExtensionContentSession::DoUpdateExtensionParent(sessionInfo)` | 新增内部方法（非 napi） | 更新 sessionInfo_ + 调 `uiWindow_->UpdateExtensionParent(WindowOption)` + 遍历 jsExtensionWindowRefs_ 经 napi_unwrap 调老 proxy 的 C++ `UpdateExtensionParent(WindowOption)`。**无需 napi 暴露**；uiWindow_ 对象复用不变（ADR-8） |
| `JsUIExtensionContentSession::jsExtensionWindowRefs_` | 新增成员 | `vector<napi_ref>` 跟踪应用通过 `getUIExtensionHostWindowProxy` / `getUIExtensionWindowProxy` 获取的老 proxy |
| `JsUIExtensionContentSession::env_` | 新增成员 | `napi_env`，CreateJsUIExtensionContentSession 时存入，供 DoUpdateExtensionParent 使用 |
| ~~`JsUIExtension::persistentIdToComponentId_`~~ | ~~新增成员~~ | **Rev2 废弃**：原 Rev1 `map<int32_t,uint64_t>` 索引，代码中不存在；Rev2 用 `GetEffectiveComponentId`（稳定 X）替代 |

### 3.4 数据结构依赖（跨仓）

| 结构 | 仓 | 说明 |
|------|----|------|
| `SessionInfo::originalCallerInfo`（含 `callerToken` + `persistentId`） | ability_base | 新增字段，默认值为 `{ nullptr, 0 }`。本仓消费此字段，不定义 |

**注**：T0a 已在本仓（ability_base）实现 `OriginalCallerInfo { callerToken, persistentId }` + Marshalling。ADR-3 决定 uiExtensionMap_ 清理按 persistentId 查找，不扩展 originalCallerInfo 结构。

### 3.5 现有接口行为变更

**无签名变更**。以下现有函数的**行为**在 originalCallerInfo 为默认值时保持完全不变：
- `StartUIExtensionAbility` — 入口行为不变
- `GetOrCreateExtensionRecord` — 新增前置分支仅在 originalCallerInfo 非默认值时触发
- `GetAbilityRecordBySessionInfo` — 逐行不变
- `StartAbilityLocked` — uiExtensionMap_ 更新逻辑新增旧映射清理（仅在迁移场景）

## 4. 行为规则

### 4.1 迁移更新标识规则

| 条件 | 行为 |
|------|------|
| `originalCallerInfo.callerToken != nullptr && persistentId != 0` | 识别为迁移更新请求，进入 `GetOrCreateExtensionRecord` 迁移更新分支 |
| `originalCallerInfo.callerToken == nullptr \|\| persistentId == 0` | 非迁移请求，走现有逻辑（按 uiExtensionComponentId 匹配 / preload / create-new） |

### 4.2 原 record 定位规则

**关键前提**：迁移时 persistentId 不变，只有 callerToken 变化。

`GetAbilityRecordByPersistentId` 遍历 `extensionRecords_`，对每个 record：
1. 获取 record 的 `sessionInfo`（via `GetSessionInfo()`）
2. 比较 `recordSessionInfo->persistentId == sessionInfo.persistentId`
3. 匹配 → 返回该 record

`originalCallerInfo.callerToken` 仅作为迁移标识（ADR-6 `IsMigrationUpdateRequest` 判断），不参与 record 匹配。

### 4.3 迁移更新执行规则（AMS 服务侧）

定位到原 record 后：
1. `GetOrCreateExtensionRecord` 返回该 record，`isLoaded = true`
2. `StartAbilityLocked` 执行 `SetSessionInfo(新 sessionInfo)` — 原 record 的 sessionInfo 中 callerToken 更新为新窗口值（persistentId 不变）
3. `uiExtensionMap_` 按 persistentId 查找并清理旧 sessionToken 映射
4. `uiExtensionMap_` 建立新 sessionToken → 原 record 的映射
5. `DoForegroundUIExtension` 通过 `CommandAbilityWindow` → `ScheduleCommandAbilityWindow` IPC 将新 sessionInfo 传递到提供方进程

### 4.3a 迁移更新执行规则（应用侧 — ADR-7/ADR-8）

UIExtension 提供方进程收到 `ScheduleCommandAbilityWindow(WIN_CMD_FOREGROUND)` 后：
1. `UIExtension::OnCommandWindow` 检测 `originalCallerInfo.callerToken != nullptr && winCmd == FOREGROUND` → 调用虚方法 `HandleSessionUpdate` + `OnCommandWindowDone` 并 return，不进入现有 `ForegroundWindow`
2. `JsUIExtension::HandleSessionUpdate`（重写，Rev2）执行：
   a. `compId = GetEffectiveComponentId(sessionInfo)`（稳定 X，迁移前后不变）
   b. `contentSessions_.find(compId)` 直接命中——**无 key 滚动**（`uiWindowMap_` 同以 stable X 为 key，uiWindow 对象复用不变，ADR-8）
   c. `napi_unwrap` 拿 ContentSession C++ 指针，调 `DoUpdateExtensionParent(sessionInfo)`
   > Rev2：不操作 `persistentIdToComponentId_`（已废弃），不 erase/re-emplace `contentSessions_`/`uiWindowMap_` 的 key
3. `JsUIExtensionContentSession::DoUpdateExtensionParent` 执行：
   a. 更新 `sessionInfo_` 指针（hostWindowId、componentId 等随之变化）—— uiWindow_ 对象复用不变（ADR-8）
   b. 调 `uiWindow_->UpdateExtensionParent(WindowOption)`（C++ 接口）；遍历 `jsExtensionWindowRefs_`，经 `napi_unwrap` 取 JsExtensionWindow C++ 指针，直接调 `UpdateExtensionParent(WindowOption)`（**无需 napi 暴露**，window_manager T0b 提供 C++ 接口）
   c. 新 proxy 不主动创建：应用下次调 `getUIExtension*Proxy` 时，因 `sessionInfo_->hostWindowId` 已更新，自然用新值 `CreateJsExtensionWindow`

### 4.4 容错降级规则

| 异常场景 | 行为 |
|----------|------|
| originalCallerInfo 非默认值但匹配不到原 record | 记 TAG_LOGW 日志（含 originalCallerToken、originalPersistentId），**继续走现有逻辑**（按 uiExtensionComponentId 匹配 / preload / create-new），不返回错误 |
| sessionInfo 为 nullptr | `GetOrCreateExtensionRecord` 行 216 现有 CHECK_POINTER_AND_RETURN 生效，返回 `ERR_INVALID_VALUE` |
| 原 record 的 sessionInfo 为 nullptr | 遍历匹配时跳过该 record（continue），不影响其他 record 匹配 |

### 4.5 多次迁移规则（ADR-4）

persistentId 迁移时不变，AMS 侧始终按 persistentId 匹配 record，天然支持多次迁移：
- 第 1 次迁移 A→B：originalCallerInfo.callerToken = A（迁移标识），按 persistentId P 匹配 → 更新 callerToken 为 B
- 第 2 次迁移 B→C：originalCallerInfo.callerToken = B（迁移标识），按 persistentId P 匹配 → 更新 callerToken 为 C

`originalCallerInfo.callerToken` 记录"上一次"的调用方，用于迁移标识和审计追溯。

### 4.6 兼容性规则

- `GetAbilityRecordBySessionInfo`（按 uiExtensionComponentId 匹配）逐行不变
- `IsPreloadExtensionRecord` 逐行不变
- `GetOrCreateExtensionRecordInner` 逐行不变
- 非 migration 场景的 `StartAbilityLocked` 行为完全不变
- 现有所有单元测试继续通过

## 5. 验收标准 (AC)

### P0 — 核心路径

#### AC1: originalCallerInfo 非默认值标识迁移更新请求
**WHEN** `StartUIExtensionAbility` 收到的 sessionInfo 中 `originalCallerInfo.callerToken` 不为空且 `originalCallerInfo.persistentId` 不为 0
**THEN** `GetOrCreateExtensionRecord` 进入迁移更新分支，调用 `IsMigrationUpdateRequest` 返回 true，调用 `GetAbilityRecordByOriginalCallerInfo` 进行匹配

#### AC2: 按 persistentId 定位原 extension record
**WHEN** 迁移更新分支执行 `GetAbilityRecordByPersistentId`
**THEN** 函数遍历 `extensionRecords_`，按 `sessionInfo.persistentId`（迁移时不变）匹配到迁移前的原 record 并返回（而非创建新 record）；`originalCallerInfo.callerToken` 仅作为迁移标识，不参与匹配

#### AC3: 原 record 的 sessionInfo 中 callerToken 被更新（persistentId 不变）
**WHEN** 通过 persistentId 成功定位到原 record，`GetOrCreateExtensionRecord` 返回后
**THEN** `StartAbilityLocked` 执行 `SetSessionInfo`，原 record 的 sessionInfo 中 callerToken 被更新为新窗口的 callerToken；persistentId 保持不变；`GetSessionInfo()` 返回更新后的 sessionInfo

#### AC4: 迁移更新不创建新 record（isLoaded=true）
**WHEN** 迁移更新分支成功定位到原 record
**THEN** `GetOrCreateExtensionRecord` 返回 `isLoaded=true`，`StartAbilityLocked` 走 `DoForegroundUIExtension` 路径（行 705），**不**执行 `LoadAbility`（行 701），不创建新 Ability 实例

#### AC5: 新 sessionInfo 传递到 UIExtension 提供方 window 子系统
**WHEN** 迁移更新完成后执行 `DoForegroundUIExtension`
**THEN** 通过 `CommandAbilityWindow` → `ScheduleCommandAbilityWindow` IPC 链路，新 sessionInfo 被传递到 UIExtension 提供方进程；提供方进程的 `OnCommandWindow` 收到 `WIN_CMD_FOREGROUND`（携带 originalCallerInfo），流经正常 `ForegroundWindow` 路径（方案 B，2026-07-09）

#### AC5a: 应用侧迁移感知 — HandleSessionCreate else 分支增量 UpdateExtensionParent（方案 B）
**WHEN** UIExtension 提供方进程的 `OnCommandWindow` 收到 `WIN_CMD_FOREGROUND`（originalCallerInfo 非空，moveTo 模式）
**THEN** 流经 `ForegroundWindow` → `HandleSessionCreate` else 分支（window 已存在，天然不触发 onSessionCreate）；else 分支内检测 `originalCallerInfo.callerToken != nullptr` 时，经 `contentSessions_[compId]` → `napi_unwrap` 取 `JsUIExtensionContentSession` 指针调 `DoUpdateExtensionParent`（更新 sessionInfo_ + `uiWindow_->UpdateExtensionParent(WindowOption)` + 遍历 proxy refs `UpdateExtensionParent`）；随后 `ForegroundWindow` 执行 `uiWindow->Show()`（不跳过）；`OnCommandWindowDone` 上报 `ABILITY_CMD_FOREGROUND`。**无独立 HandleSessionUpdate 短路方法**（方案 B 移除）

#### AC6: dump 信息反映迁移后状态
**WHEN** 迁移更新完成后执行 `aa dump -a` 或等效 dump 命令
**THEN** dump 信息中该 UIExtension session 关联的 abilityName 为迁移后的目标 abilityName，persistentId 为目标窗口的 persistentId

#### AC7: uiExtensionMap_ 一致性维护
**WHEN** 迁移更新执行时
**THEN** 由于迁移后 sessionToken 不变（需求方确认），`uiExtensionMap_` 现有行 672 `uiExtensionMap_[sessionToken] = {record, 新 sessionInfo}` 已是对同一 key 的 value 原地更新（新 callerToken），无需清理旧映射；`AddUIExtWindowDeathRecipient` 对同一 sessionToken 幂等返回；迁移后通过新 sessionInfo 查询 `GetUIExtensionBySessionInfo` 能正确返回原 record（sessionToken 命中 + callerToken 一致性校验通过）

### P1 — 边界路径

#### AC8: 无 originalCallerInfo 时走原有逻辑
**WHEN** sessionInfo 中 `originalCallerInfo` 为默认值（callerToken 为空、persistentId 为 0）或字段不存在
**THEN** `GetOrCreateExtensionRecord` 跳过迁移更新分支，走原有逻辑（按 `uiExtensionComponentId` 匹配 → preload → create-new），行为与本次变更前完全一致

#### AC9: originalCallerInfo 无法匹配到原 record 时的容错
**WHEN** 迁移更新分支被触发（originalCallerInfo 非默认值），但 `GetAbilityRecordByOriginalCallerInfo` 在 `extensionRecords_` 中无法匹配到任何已存在的 record
**THEN** 记 `TAG_LOGW(AAFwkTag::ABILITYMGR, ...)` 日志（含 originalPersistentId），继续走现有逻辑，不返回错误码，不发生 crash

#### AC10: 并发安全性
**WHEN** 迁移更新执行期间，其他线程并发访问同一 extension record 的 sessionInfo 或 uiExtensionMap_
**THEN** "定位原 record + 更新 sessionInfo + 更新 uiExtensionMap_" 过程中，`ExtensionRecordManager::mutex_` 保护遍历匹配，`sessionLock_` 保护 SetSessionInfo，`uiExtensionMapMutex_` 保护 map 更新；不出现部分更新导致的中间状态

#### AC11: 单元测试覆盖
**WHEN** 执行 `test/unittest/` 下本次新增/扩展的测试套件
**THEN** 至少覆盖：IsMigrationUpdateRequest 判断、GetAbilityRecordByOriginalCallerInfo 匹配成功、匹配失败容错降级、迁移后 isLoaded=true 不 LoadAbility、SetSessionInfo 更新成功、uiExtensionMap_ 旧映射清理、无 originalCallerInfo 走原逻辑；所有用例通过

#### AC12: 日志可观测性
**WHEN** 迁移更新分支任意环节被触发
**THEN** 关键节点有 `TAG_LOGI`/`TAG_LOGW`/`TAG_LOGD` 日志（tag：`AAFwkTag::ABILITYMGR`），含 `originalPersistentId`、`originalCallerToken`、新 `persistentId`、`uiExtensionComponentId`、`bundleName` 等关键字段，便于线上问题定位

## 6. 测试场景

### 6.1 单元测试场景

| # | 场景描述 | 前置条件 | 预期结果 |
|---|----------|----------|----------|
| T1 | 迁移更新标识识别 | sessionInfo.originalCallerInfo = {callerToken非空, persistentId非0} | IsMigrationUpdateRequest 返回 true，进入迁移更新分支 |
| T2 | 非迁移请求标识识别 | sessionInfo.originalCallerInfo = {callerToken空, persistentId=0} 或不存在 | IsMigrationUpdateRequest 返回 false，跳过迁移更新分支 |
| T3 | 按 originalCallerInfo 匹配成功 | extensionRecords_ 中存在 record 其 sessionInfo.callerToken/persistentId 与 originalCallerInfo 一致 | GetAbilityRecordByOriginalCallerInfo 返回该 record |
| T4 | 按 originalCallerInfo 匹配失败 | extensionRecords_ 中无匹配 record | GetAbilityRecordByOriginalCallerInfo 返回 nullptr，记 TAG_LOGW，继续走现有逻辑 |
| T5 | 迁移更新返回 isLoaded=true | 匹配成功 | GetOrCreateExtensionRecord 返回原 record，isLoaded=true |
| T6 | 迁移后不执行 LoadAbility | isLoaded=true | StartAbilityLocked 走 DoForegroundUIExtension 路径，不调用 LoadAbility |
| T7 | sessionInfo 更新成功 | 迁移更新完成 | 原 record 的 GetSessionInfo() 返回新 sessionInfo（新 callerToken/persistentId/uiExtensionComponentId） |
| T8 | uiExtensionMap_ 旧映射清理 | 迁移场景，oldSessionToken 可用 | uiExtensionMap_ 中旧 sessionToken 条目被 erase |
| T9 | uiExtensionMap_ 新映射建立 | 迁移场景 | uiExtensionMap_ 中新 sessionToken → 原 record 的映射被建立 |
| T10 | 原 record sessionInfo 为空 | record 存在但 GetSessionInfo()=nullptr | 遍历时跳过该 record，不影响匹配 |
| T11 | sessionInfo 为空 | abilityRequest.sessionInfo=nullptr | GetOrCreateExtensionRecord 返回 ERR_INVALID_VALUE（现有行 216 逻辑） |
| T12 | 无 originalCallerInfo 走原逻辑 | originalCallerInfo 为默认值 | GetAbilityRecordBySessionInfo 被调用，行为与变更前一致 |
| T13 | 现有 GetAbilityRecordBySessionInfo 不受影响 | 非 migration 场景 | 按 uiExtensionComponentId 匹配，行为不变 |

### 6.2 兼容性回归测试

| # | 场景描述 | 预期结果 |
|---|----------|----------|
| R1 | 现有 `extension_record_manager_test` 全部用例 | 全部通过 |
| R2 | 现有 `ui_extension_ability_manager_test` 全部用例 | 全部通过 |
| R3 | 现有 `ability_manager_service_test` UIExtension 相关用例 | 全部通过 |
| R4 | 普通 StartUIExtensionAbility（无 originalCallerInfo） | 行为不变，创建新 record |
| R5 | preload UIExtension 场景 | 预加载路径行为不变 |

### 6.3 集成测试场景（手动 / 跨仓联调）

| # | 场景描述 | 验证点 |
|---|----------|--------|
| I1 | UEC 从窗口 A 迁移到窗口 B（同进程同调用方） | UEC 在窗口 B 正常呈现，窗口 A 中 UEC 消失 |
| I2 | UEC 从 Ability1 的窗口迁移到 Ability2 的窗口（同进程跨 Ability） | dump 信息 abilityName 为目标 Ability2，persistentId 为目标窗口 |
| I3 | UEC 连续迁移 A→B→C（多次迁移） | 每次迁移后 record sessionInfo 正确更新，UEC 在最新窗口呈现 |
| I4 | 迁移后 UIExtension 提供方 window 子系统收到新 sessionInfo | 提供方进程日志显示 ScheduleCommandAbilityWindow 收到新 sessionInfo |
| I5 | 迁移后旧 sessionToken 映射不残留 | GetUIExtensionBySessionInfo 用旧 sessionToken 查询返回 nullptr |
