# ability_runtime services 启动流程梳理、故障点分析与加固执行方案

> 本文档面向 `services/` 下所有核心系统服务，梳理启动/开机流程、识别数据异常下的故障时机与影响（崩溃/死循环/阻塞开机），并给出可落地的加固执行方案。加固项按 P0/P1/P2 分批推进。

---

## 一、服务总体架构与 SA 清单

### 1.1 SA 注册清单（来源 `services/sa_profile/`）

| SA ID | 进程 | 库 | run-on-create | 服务名 |
|-------|------|-----|---------------|--------|
| 180 | foundation | libabilityms.z.so | true | AbilityManagerService |
| 501 | foundation | libappms.z.so | true | AppManagerService |
| 182 | foundation | libdataobsms.z.so | true | DataObsMgrService |
| 183 | foundation | libupms.z.so | false（按需） | UriPermissionManagerService |
| 184 | quick_fix | libquickfixms.z.so | false（按需） | QuickFixManagerService |
| 185 | foundation | libagentmgr.z.so | true | AgentManagerService |
| 186 | aimgr | libclimgr.z.so | false（按需） | CliManagerService |

### 1.2 进程内启动依赖链

```
samgr
 └─ foundation 进程拉起
     ├─ AMS(180) OnStart
     │    ├─ AmsConfigurationParameter::Parse()
     │    ├─ SubManagersHelper::InitSubManagers()
     │    │    ├─ UIExtensionAbilityManager
     │    │    ├─ CommonExtensionManager / AbilityConnectManager
     │    │    ├─ DataAbilityManager
     │    │    ├─ PendingWantManager
     │    │    └─ MissionListManager / UIAbilityManager（依赖 SceneBoard）
     │    ├─ ImplicitStartProcessor / 焦点监听 / 首帧观察
     │    ├─ ConnectionStateManager
     │    ├─ 各类 Interceptor（BlockAllAppStart 等）
     │    ├─ AbilityAutoStartupService
     │    ├─ AbilityCacheManager
     │    ├─ 事件订阅（屏幕解锁/用户解锁）
     │    ├─ InitAppSpawnMsgPipe / InsightIntent / ModularExtensionEvent
     │    └─ ExtensionRunningTimeoutMonitor / BackgroundUserExtensionMonitor
     │
     ├─ AppMS(501) OnStart
     │    ├─ taskHandler / eventHandler
     │    ├─ 异步 AppMgrServiceInner::Init()
     │    │    ├─ InitGlobalConfiguration()
     │    │    ├─ AppStateObserverManager / RenderStateObserverManager
     │    │    ├─ SecurityModeManager
     │    │    ├─ NWeb 预加载（ffrt delay + timeout）
     │    │    └─ NativeSpawn / HybridSpawn MsgPipe
     │    ├─ OpenAppSpawnConnection()（同步，失败仅 WARN）
     │    ├─ Publish()
     │    └─ AddSystemAbilityListener(WMS / UPM)
     │
     ├─ DataObs(182) OnStart → Init → Publish
     ├─ UPM(183) OnStart → new StubImpl → AddSystemAbility
     └─ Agent(185) OnStart
 └─ quick_fix 进程
     └─ QuickFixMgr(184) OnStart → EventRunner("QuickFixMgrSvrMain") → Publish
```

### 1.3 关键跨服务时序

1. AMS 在 `OnStart` 末尾 `SetParameter(BOOTEVENT_APPFWK_READY,"true")`，并向 BMS/WMS/UPM 注册 `AddSystemAbilityListener`。
2. `OnAddSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID)` 触发 `SubscribeBundleEventCallback()` → 提交 `StartResidentApps(U0_USER_ID)` 任务。
3. 启动 Launcher/SystemUI：`StartHighestPriorityAbility()` → `WaitBootAnimationStart()`（调用 `WaitParameter(BOOTEVENT_BOOT_ANIMATION_READY,"true",timeout)`）→ `ImplicitQueryInfoByPriority`（按 `flag.home.intent.from.system` 实体查询）→ `StartAbility`。
4. 开机完成：`InitPushTask()` 内 `WatchParameter(BOOTEVENT_BOOT_COMPLETED, AppFwkBootEventCallback)`，或已就绪时直接回调。

---

## 二、各服务故障点详解（按风险等级）

### 2.1 AbilityManagerService（最高风险，影响整机开机）

**关键文件：** `services/abilitymgr/src/ability_manager_service.cpp`

| # | 位置 | 故障描述 | 影响 | 等级 |
|---|------|----------|------|------|
| A1 | `ability_manager_service.cpp:457-505` `Init()` | 任一关键子管理器构造失败（如 `freeInstallManager_`）`CHECK_POINTER_RETURN_BOOL` 返回 false，`OnStart` 早退 | AMS 启动失败，但 samgr 不会重启 → 开机后无组件框架，整机不可用 | P0 |
| A2 | `ability_manager_service.cpp:431-435` `Publish()` | Publish 失败仅打 ERROR 后 return | 服务"半启动"：内部对象已建，但外部查不到 SA | P0 |
| A3 | `mission/mission_info_mgr.cpp:40-58` `GenerateMissionId` | `missionIdMap_` 若被错误数据撑满至 `[MIN,MAX)` 全占用，`for` 循环找不到空 ID | 返回 false，新增任务失败，可能反复触发上层重试 | P1 |
| A4 | `mission/mission_data_storage.cpp:50-86` `LoadAllMissionInfo` | 任务 JSON 文件损坏：`FromJsonStr` 失败仅 `continue` | 该任务丢失，但若目录下大量文件损坏可能拖慢启动 | P2 |
| A5 | `mission_data_storage.cpp:93-101` | 目录创建失败仅 LOG，不 return | 后续 SaveMissionInfo 全失败，状态不持久化 | P2 |
| A6 | `ability_manager_service.cpp:613-614` `ParseJsonFromBoot` | 白名单 JSON 解析失败 | 静默使用空配置，`prevent_startability` 开启时会拦截所有应用启动 → Launcher 拉不起来 → 假死 | P0 |
| A7 | `ability_manager_service.cpp:3800-3834` `OnAddSystemAbility` | BundleMgr 未就绪或回调丢失 | Launcher 永不启动 | P0 |
| A8 | `ability_manager_service.cpp:15539-15549` `WaitBootAnimationStart` | `WaitParameter` 超时返回后是否继续？若动画参数一直为 "false" 且超时过短 | Launcher 启动窗口被错过 | P1 |
| A9 | `StartAbility` 中 `EnqueueWaitingAbility` | 多层启动等待链，无深度上限 | 理论上可构造链式等待 → 启动队列阻塞 | P2 |
| A10 | `ExtensionRunningTimeoutMonitor::StartMonitor` | 在 Init 阶段就启动 monitor | 若 monitor 自身线程异常或回调路径崩溃，影响 Init | P2 |

**已具备的防护：**
- `HiviewDFX::Watchdog::InitFfrtWatchdog()`（第 459 行）
- `FreezeUtil` 生命周期事件跟踪
- `ExtensionRunningTimeoutMonitor` 超时强杀

### 2.2 AppManagerService

**关键文件：** `services/appmgr/src/app_mgr_service.cpp`、`app_mgr_service_inner.cpp`

| # | 位置 | 故障描述 | 影响 | 等级 |
|---|------|----------|------|------|
| B1 | `app_mgr_service.cpp:127-131` | `Init()` 失败仅 return，不重启 | 进程孵化能力不可用 | P0 |
| B2 | `app_mgr_service.cpp:189-192` `OpenAppSpawnConnection` | Spawn 连接失败仅 WARN，服务继续 | 后续所有 StartAbility 失败，可能被上层反复重试 → 风暴 | P1 |
| B3 | `app_spawn_client.cpp:89-114` `OpenConnection` | 无自动重连 | Spawn 死亡后 AppMS 不会恢复连接 | P1 |
| B4 | `app_mgr_service_inner.cpp:506-509` NWeb 预加载 | ffrt 延迟任务带 timeout | 已加 timeout，风险较低 | P2 |
| B5 | `app_mgr_service_inner.cpp:9174/9208` XCollie 定时器 | 仅用于 DFX 上报，不主动恢复 | 故障可观测但不可自愈 | P2 |
| B6 | `app_running_record.cpp:257,1616-1626` 重启计数 | `MAX_RESTART_COUNT=3`/`RESTART_INTERVAL_TIME=120s` | 反复崩溃应用会被限流，但若限流逻辑本身计数异常（持久化丢失）→ 不限流 | P2 |
| B7 | `app_debug_manager.cpp:31,57` `unique_lock<mutex>` | 与其它锁的顺序若不一致 | 潜在死锁 | P2 |
| B8 | 异步 `AppMgrServiceInner::Init()` | 通过 `SubmitTask(TASK_INIT_APPMGRSERVICEINNER)` 异步执行 | Publish 之前 Inner 可能未就绪，首个请求落到未初始化对象 → 空指针 | P1 |

### 2.3 DataObsMgrService

**关键文件：** `services/dataobsmgr/src/dataobs_mgr_service.cpp`、`dataobs_mgr_inner.cpp`

| # | 位置 | 故障描述 | 影响 | 等级 |
|---|------|----------|------|------|
| C1 | `dataobs_mgr_service.cpp:79` `Init()` | 总是返回 true，无实质错误路径 | 隐患少 | - |
| C2 | `dataobs_mgr_inner.cpp:37` | `OBS_ALL_NUM_MAX` 限制 observer 数 | 超限注册失败，但不崩溃 | P2 |
| C3 | `TASK_COUNT_MAX = 50` | 任务超限 | 拒绝新任务 | P2 |
| C4 | **无持久化** | 重启后 observer 全丢 | 客户端感知不到，后续通知失效 | P1（功能性问题） |

### 2.4 UriPermissionManagerService

**关键文件：** `services/uripermmgr/src/uri_permission_manager_service.cpp`、`uri_permission_manager_stub_impl.cpp`

| # | 位置 | 故障描述 | 影响 | 等级 |
|---|------|----------|------|------|
| D1 | `uri_permission_manager_service.cpp:48` `AddSystemAbility` 失败 | 仅 return，无重试 | SA 注册不上，按需启动下次再失败 | P1 |
| D2 | `Init()` 重复调用 | 第二次直接 return true（防重） | OK | - |
| D3 | `uri_permission_manager_stub_impl.cpp`（74KB，逻辑复杂） | URI 授权/撤销的并发与异常路径 | 需要重点审计空指针、map 迭代器失效 | P1 |
| D4 | 与 BundleMgr/AbilityMgr 的依赖 | 在 stub 中通过代理调用 | 若代理为空仅返回错误码，不崩溃 | P2 |

### 2.5 QuickFixManagerService

**关键文件：** `services/quickfixmgr/src/quick_fix_manager_service.cpp`、`quick_fix_manager_apply_task.cpp`

| # | 位置 | 故障描述 | 影响 | 等级 |
|---|------|----------|------|------|
| E1 | `quick_fix_manager_service.cpp:71-81` | bundleQfMgr/appMgr 代理为空 → `QUICK_FIX_CONNECT_FAILED` | 应用补丁失败，不崩溃 | P2 |
| E2 | 任务链 `PostDeployQuickFixTask`/`PostSwitchQuickFixTask` | 链式任务，每步带超时 | 已具备超时保护，风险低 | P2 |
| E3 | 补丁文件损坏 | 加载阶段校验 | 设计上应跳过，需审计是否静默接受 | P2 |
| E4 | EventRunner 创建失败 | `Init()` 返回 false，`OnStart` 不 Publish | 服务不就绪，但不会崩溃 | P2 |

### 2.6 开机自启模块

**关键文件：**
- `services/abilitymgr/include/ability_auto_startup_service.h` + `.cpp`
- `services/abilitymgr/src/ability_auto_startup_data_manager.cpp`
- `services/abilitymgr/src/ability_manager_service.cpp`（`StartResidentApps` / `StartHighestPriorityAbility` / `WaitBootAnimationStart` / `InitPushTask`）
- `services/abilitymgr/src/interceptor/block_all_app_start_interceptor.cpp`
- 配置：`/etc/efficiency_manager/prevent_startability_whitelist.json`

| # | 位置 | 故障描述 | 影响 | 等级 |
|----|------|----------|------|------|
| F1 | `AbilityAutoStartupDataManager::GetKvStore()` | KVStore 初始化失败：仅重试 5 次 × 100ms | 持续不可用→自启配置读不到→自启功能失效（不崩溃） | P1 |
| F2 | `RestoreKvStore()` 检测 `DATA_CORRUPTED` | 删除数据库重建 | **用户自启配置丢失**，但避免了死循环 | P1 |
| F3 | KVStore 并发锁 `kvStorePtrMutex_` + 回调锁 `autoStartUpMutex_` | 若回调路径再次进入 data manager，存在重入 | 潜在死锁/迭代器失效 | P1 |
| F4 | `ParseJsonFromBoot()` 白名单解析失败 | 静默使用空配置 + `prevent_startability=true` → 全部拦截 | **Launcher/SystemUI 拉不起来，整机假死** | **P0** |
| F5 | `WaitBootAnimationStart()` `WaitParameter` 超时 | 超时时间过短或动画卡死 | Launcher 启动时机错乱 | P1 |
| F6 | `OnAddSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID)` 回调丢失 | BundleMgr 已就绪但事件未送达 | `StartResidentApps` 永不执行 → Launcher 不启动 | **P0** |
| F7 | `StartResidentApps` 中无总量上限 | 常驻应用过多 → 启动风暴 | 开机后 CPU/IO 飙升 | P1 |
| F8 | 自启递归：自启应用 StartAbility 又触发其它自启 | 无深度限制 | 链式自启风暴 | P1 |
| F9 | `WatchParameter(BOOTEVENT_BOOT_COMPLETED, ...)` 注册失败 | 无重试 | 错过开机完成事件 → 后续依赖该事件的逻辑全部失效 | P1 |
| F10 | `ExecuteCallbacks()` 遍历 `callbackVector_` | 回调中客户端死亡，`OnRemoteDied` 并发修改 vector | 迭代器失效/崩溃 | P1 |
| F11 | 权限校验仅基于 TokenID | Token 伪造 | 自启配置被非授权应用篡改 | P1 |

---

## 三、加固执行方案（按优先级落地）

### P0 — 必须加固（直接威胁开机可用性）

#### H1. AMS/AppMS 启动失败自愈
- **文件**：`services/abilitymgr/src/ability_manager_service.cpp:457-505`、`services/appmgr/src/app_mgr_service.cpp:127-131`
- **现状**：Init/Publish 失败仅 return，samgr 不会重启该 SA，整机进入无组件框架状态。
- **执行步骤**：
  1. 失败路径上抛 `HiSysEvent`（FAULT 类型，事件名 `AMS_INIT_FAIL` / `APPMS_INIT_FAIL`）。
  2. 通过 samgr 的"按需重启"或自定义看门狗，在 N 秒（建议 30s）后强制退出 foundation 进程，让 init 重拉。
  3. 关键子对象构造失败时区分"致命"（直接 `_exit()` 触发重启）与"可降级"（关闭该子能力但继续启动）。
- **验收**：注入 freeInstallManager 构造失败，foundation 进程在 30s 内被 init 重拉并恢复。

#### H2. 白名单解析失败的降级策略
- **文件**：`services/abilitymgr/src/ability_manager_service.cpp:613-614` `ParseJsonFromBoot`
- **现状**：解析失败 → 空 white_list → 配合 `prevent_startability=true` 会拦截 Launcher。
- **执行步骤**：
  1. 解析失败时强制将 `prevent_startability` 置为 false，或内置最小默认白名单（launcher/systemui/settings）。
  2. 解析失败上抛 HiSysEvent（`AMS_WHITELIST_PARSE_FAIL`）。
  3. 启动期白名单读取独立加 XCollie 超时（5s），防止读文件卡死。
- **验收**：损坏 `/etc/efficiency_manager/prevent_startability_whitelist.json` 后重启，Launcher 仍能启动。

#### H3. BundleMgr 就绪事件丢失的兜底
- **文件**：`services/abilitymgr/src/ability_manager_service.cpp:3800-3834`、`InitPushTask`
- **现状**：依赖 `OnAddSystemAbility` 单一通道，事件丢失则 Launcher 永不启动。
- **执行步骤**：
  1. `InitPushTask` 中增加延迟探活任务：T 秒（建议 10s）后若仍未触发 `StartResidentApps`，主动查询 `SystemAbilityManagerClient::GetSystemAbilityManager()->CheckSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID)`，若已存在则补触发。
  2. 为 `WatchParameter(BOOTEVENT_BOOT_COMPLETED)` 失败增加最多 3 次重试。
- **验收**：模拟 BundleMgr 延迟就绪 15s，AMS 兜底任务在 T+10s 补触发 Launcher。

#### H4. 关键 while/循环的边界保护
- **文件**：`services/abilitymgr/src/mission/mission_info_mgr.cpp:40-58` `GenerateMissionId`
- **执行步骤**：
  1. 增加 `missionIdMap_` 大小上限告警，超过阈值（如 90% MAX）上抛 HiSysEvent。
  2. for 循环遍历上限设为 `MAX-MIN`，避免任何边界 bug 导致超长循环。
- **验收**：单测构造 `missionIdMap_` 全占用场景，验证快速返回 false 且不卡死。

### P1 — 应当加固（功能性/稳定性显著提升）

#### H5. AppSpawn 连接自愈
- **文件**：`services/appmgr/src/app_spawn_client.cpp`、`services/appmgr/src/app_mgr_service.cpp:189-192`
- **执行步骤**：
  1. 增加 `OnRemoteDied` 监听，spawn 死亡后自动 `OpenConnection`，指数退避重试（初始 200ms，上限 5s）。
  2. `OpenAppSpawnConnection` 失败时启动周期性重连任务而非仅 WARN。
  3. 重连失败累计阈值告警（HiSysEvent `APPMS_SPAWN_RECONNECT_FAIL`）。
- **验收**：kill appspawn 后，AppMS 在 1s 内自动重连。

#### H6. AppMgrServiceInner 异步 Init 的就绪栅栏
- **文件**：`services/appmgr/src/app_mgr_service.cpp` Init + `services/appmgr/src/app_mgr_service_inner.cpp:487`
- **现状**：`Publish` 在异步 Init 提交之后立即执行，首个外部请求可能命中未就绪的 Inner。
- **执行步骤**：在 stub 入口对关键接口增加 `IsReady()` 检查 + 短时等待（或返回 `ERR_NO_INIT` 并让客户端重试），杜绝空指针。
- **验收**：在 Inner Init 完成前发起 IPC 请求，返回 `ERR_NO_INIT` 而非崩溃。

#### H7. 自启模块回调与并发加固
- **文件**：`services/abilitymgr/src/ability_auto_startup_data_manager.cpp`、`services/abilitymgr/src/ability_auto_startup_service.cpp`
- **执行步骤**：
  1. `ExecuteCallbacks` 改为拷贝 `callbackVector_` 快照后再遍历，避免 `OnRemoteDied` 并发修改迭代器。
  2. 审计 KVStore 操作路径是否会被回调路径重入；必要时拆分锁或改用单一锁串行化。
  3. `GetKvStore` 5 次重试改为指数退避（初始 100ms，倍增，上限 2s）+ 持续后台重连 + 失败上抛事件。
  4. 自启 StartAbility 增加全局并发数上限（同一时刻最多 N 个自启并发）和深度上限（防链式，建议 ≤3）。
- **验收**：并发注册回调 + 客户端中途死亡，无崩溃；KVStore 持续不可用时上抛事件且不卡死。

#### H8. 启动等待链超时
- **文件**：AMS `EnqueueWaitingAbility` / `WaitBootAnimationStart`
- **执行步骤**：所有"等待外部条件"的路径必须有硬超时 + 超时后的降级动作（继续启动/放弃启动），不允许无限等待。
- **验收**：构造等待链无法满足的场景，验证超时后降级而非永久阻塞。

#### H9. UriPermissionManagerStubImpl 深度审计
- **文件**：`services/uripermmgr/src/uri_permission_manager_stub_impl.cpp`（74KB）
- **执行步骤**：专项审计空指针、map 迭代器、循环、递归调用；为每个对外接口补 fuzz/单元测试覆盖异常输入。
- **验收**：每个对外接口都有对应 fuzz 用例，畸形输入不崩溃。

### P2 — 建议加固（健壮性/可观测性）

#### H10. 统一 HiSysEvent 上报
- 所有关键错误路径（A1-A10/B1-B8/D1-D4/E1-E4/F1-F11）补充 FAULT 事件上报。

#### H11. 关键服务 Init Watchdog
- 为 AMS/AppMS 的 `Init()` 增加 XCollie 看门狗，超过阈值（30s）强制上报并触发恢复。

#### H12. 持久化数据校验
- 任务 JSON、自启 KVStore、白名单 JSON 读取时做 schema 校验；损坏时自动备份原文件 + 重建。

#### H13. DataObs 持久化（可选）
- 评估是否需要把 observer 列表持久化，避免重启后静默失效。

#### H14. UPM 注册失败重试
- `AddSystemAbility` 失败时进入重试队列。

#### H15. QuickFix 补丁完整性校验
- 加载补丁前强制哈希校验，校验失败拒绝并清理残留文件。

---

## 四、验证方案

### 4.1 单元/模块测试
- `mission_info_mgr` 单测：构造 `missionIdMap_` 全占用场景，验证 `GenerateMissionId` 不死循环、返回正确。
- `ability_auto_startup_data_manager` 单测：注入 KVStore `DATA_CORRUPTED`，验证 `RestoreKvStore` 不丢用户配置（备份+重建）。
- `ParseJsonFromBoot` 单测：传入损坏 JSON，验证降级到默认白名单 + `prevent_startability` 被强制关闭。

### 4.2 Fuzz 测试
- `services/abilitymgr/test/fuzztest/` 下针对 `MissionDataStorage::LoadAllMissionInfo`、`UriPermissionManagerStubImpl` 各接口补充 fuzz 用例，构造畸形 JSON/IPC payload。

### 4.3 故障注入（端到端）
1. **白名单损坏**：替换 `/etc/efficiency_manager/prevent_startability_whitelist.json` 为损坏内容 + `param set persist.sys.abilityms.prevent_startability true`，重启验证 Launcher 仍能启动。
2. **Spawn 不在**：临时 kill appspawn，重启 foundation，验证 AppMS 自愈重连。
3. **KVStore 损坏**：破坏 `/data/service/el1/public/database/auto_startup_service`，验证自启服务恢复且不卡死。
4. **BundleMgr 延迟就绪**：用 `param` 模拟延迟，验证 AMS 兜底探活任务能补触发 `StartResidentApps`。
5. **任务 JSON 批量损坏**：向 mission data 目录投放大量坏文件，验证启动时延不显著劣化。

### 4.4 自动化守护
- 开启 HiviewDFX Watchdog 与 FFRT Watchdog 后，注入 Init 卡死，验证看门狗触发。
- 检查 HiSysEvent 通道能收到对应 FAULT 事件。

### 4.5 构建与回归命令
```bash
# 构建整个 ability_runtime 组件
./build.sh --product-name <product> --build-target ability_runtime

# 构建特定目标（abilityms / appms）
./build.sh --product-name <product> --build-target abilityms
./build.sh --product-name <product> --build-target abilityms

# 全量 tdd 测试
./build.sh --product-name <product> --build-target ability_runtime_test

# 运行单元测试
run -t UT -tp ability_runtime
run -t UT -ts ability_manager_service_first_test
run -t UT -ts ability_caller_fw_module_test
```

---

## 五、落地节奏

1. **第一批（P0）**：H1 / H2 / H3 / H4 —— 直接关系开机可用性，优先合入并补充对应单测与故障注入用例。
2. **第二批（P1）**：H5 / H6 / H7 / H8 / H9 —— 提升服务自愈与并发健壮性。
3. **第三批（P2）**：H10 / H11 / H12 / H13 / H14 / H15 —— 可观测性与数据完整性收尾。

每批落地后回归 `run -t UT -tp ability_runtime`、关键模块测试以及上述端到端故障注入验证。

---

## 六、关键文件索引

- AMS：`services/abilitymgr/src/ability_manager_service.cpp`
- AMS 子管理器：`services/abilitymgr/src/sub_managers_helper.cpp`、`mission/mission_list_manager.cpp`、`mission/mission_info_mgr.cpp`、`mission/mission_data_storage.cpp`、`task_data_persistence_mgr.cpp`
- 自启：`services/abilitymgr/src/ability_auto_startup_service.cpp`、`ability_auto_startup_data_manager.cpp`、`interceptor/block_all_app_start_interceptor.cpp`
- AppMS：`services/appmgr/src/app_mgr_service.cpp`、`app_mgr_service_inner.cpp`、`app_spawn_client.cpp`、`remote_client_manager.cpp`
- DataObs：`services/dataobsmgr/src/dataobs_mgr_service.cpp`、`dataobs_mgr_inner.cpp`
- UPM：`services/uripermmgr/src/uri_permission_manager_service.cpp`、`uri_permission_manager_stub_impl.cpp`
- QuickFix：`services/quickfixmgr/src/quick_fix_manager_service.cpp`、`quick_fix_manager_apply_task.cpp`
- SA 配置：`services/sa_profile/`
