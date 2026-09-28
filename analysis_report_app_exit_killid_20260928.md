# 应用退出 killId 上报逻辑分析报告

| 项 | 内容 |
|---|---|
| 分析对象 | ability_runtime（AppMgrServiceInner / AppfreezeManager / AppExitReasonHelper） |
| 分析场景 | 应用正常关闭（滑卡/terminateSelf）、terminateAbility 超时强杀 |
| 核心问题 | 1) REASON_APP_EXIT 为什么没有成功上报；2) terminateAbility 场景为什么没有上报 killId |
| 分析日期 | 2026-09-28 |
| 修订 | v2：勘误 REASON_APP_EXIT 实际值（-1 → 4，见第 2/8 节），修复方案简化为单点改动 |
| 性质 | 静态代码分析报告，未修改任何源码 |

---

## 1. 结论速查

| 场景 | 上报路径 | killId 是否有兜底 | 实际上报结果 |
|---|---|---|---|
| 应用自行确认退出（graceful，且状态门禁通过） | `ApplicationTerminated` → 双参 `SendProcessKillEvent` | ✅ 有（`== -1` 未设置时转换为 REASON_APP_EXIT=4） | PROCESS_KILL_ID = 4 (REASON_APP_EXIT)，reason = "Kill Reason:ClearSession"/"Kill Reason:app exit" |
| 前台应用被直接关闭（状态门禁拦截 graceful） | `OnRemoteDied` → 单参 `SendProcessKillEvent` | ❌ 无 | 内核 ioctl 值，或 reason="InvalidKillId" / killId=-1 |
| terminateAbility 3s 超时强杀（`KillProcessByPid(pid,"TerminateAbility")`） | `OnRemoteDied` → 单参 `SendProcessKillEvent` | ❌ 无（链路全程无 `SetKillId`） | killId=-1 → 内核 ioctl；失败则 reason="InvalidKillId" |
| SCB 滑卡且 `isUserRequestedExit=true` | AMS 先记录，再走上述任一路径 | — | killId 被 `SetKillId(REASON_USER_REQUEST)` 覆盖，REASON_APP_EXIT 分支不生效 |
| SA 显式 kill（KillAppWithReason / KillProcessWithReason 等） | `NotifyAppMgrRecordExitReasonCompability` → `SetKillId(具体值)` | — | 上报 SA 指定的 KillEventId（或 -2 InvalidKillId） |

一句话结论：**REASON_APP_EXIT 兜底在全部代码中仅存在一处（app_mgr_service_inner.cpp:4844，双参重载，仅由 `ApplicationTerminated` 的 graceful 确认路径触达）。terminateAbility 超时强杀与 graceful 路径天然互斥，且其 kill 链路全程无 `SetKillId`、上报落在无兜底的单参重载，因此该场景结构上不可能上报 REASON_APP_EXIT。**

---

## 2. 关键数据结构与常量

| 名称 | 值/位置 | 说明 |
|---|---|---|
| `AppRunningRecord::killId_` | 默认 **-1**（app_running_record.h:1525） | 仅 `SetKillId`（app_running_record.cpp:2813，无锁、后写覆盖）可修改 |
| `DEFAULT_KILL_ID` | -1（app_mgr_service_inner.cpp:358；ability_manager_service.cpp:262；app_exit_reason_helper.cpp:31） | 兜底判断的基准值 |
| `KillEventId::REASON_APP_EXIT` | **4**（hiviewdfx_hicollie 仓 `interfaces/native/innerkits/include/xcollie/process_kill_reason.h`，"app exit: 1~1000" 段第 4 项；`GetKillReason(4)` = "app exit"） | `DEFAULT_KILL_ID(-1)` 仅为"未设置"哨兵，与该枚举值**无数值关联**；4844 行兜底是 -1→4 的真实语义转换 |
| `INVALID_KILL_ID` | -2（appfreeze_manager.cpp:84） | AMS 校验失败时经 `RecordInvalidKillId` 写入 |
| `INVALID_KILL_REASON` | "InvalidKillId"（appfreeze_manager.cpp:85） | 上报事件 reason 字段的失败占位值 |
| `EVENT_KEY_KILL_ID` | "PROCESS_KILL_ID"（app_mgr_service_inner.cpp:291） | PROCESS_KILL 事件中 killId 的参数名 |
| `SetKillReason` | **first-wins**（app_running_record.h:1207-1213） | 仅当 killReason_ 为空时写入，后续 reason（如 "TerminateAbility"、"OnRemoteDied"）被忽略 |
| `HisyseventReport(len)` | len 参数实际不生效，`length_` 恒为 64（hisysevent_report.h:30-39、hisysevent_report.cpp） | 已排除"双参路径 7 参数超出容量 6 导致 killId 丢参"的假设 |

---

## 3. 上报链路全景

### 3.1 两条互斥的 PROCESS_KILL 上报路径

`SendProcessKillEvent` 全仓只有 2 个调用点：

```
【路径 A：graceful（唯一有 REASON_APP_EXIT 兜底）】
用户关闭应用（滑卡 / terminateSelf）
  → AMS 侧 Ability 销毁
  → AppMgr AmsMgrScheduler::TerminateAbility (ams_mgr_scheduler.cpp:192)
  → AppRunningManager::TerminateAbility (app_running_manager.cpp:947)
      ├─ 最后一个 Ability：SetKillReason("Kill Reason:ClearSession")（942-943，仅 clearMissionFlag 时非空）
      ├─ 挂 3s 兜底任务 DELAY_KILL_PROCESS_<recordId>（1023-1027）
      └─ 模块记录清空 → ScheduleTerminateByDelayed (app_running_record.cpp:1297)
           → ScheduleTerminateApplication (app_lifecycle_deal.cpp:141)
  → 应用侧 MainThread::HandleTerminateApplication (main_thread.cpp:3145)
      → 回调 ApplicationTerminated(recordId)（main_thread.cpp:3174）
  → AppMgrServiceInner::ApplicationTerminated (app_mgr_service_inner.cpp:3325)
      ├─ 【门禁 1】状态必须 READY/BACKGROUND，否则直接 return（3338-3342）
      ├─ ApplicationTerminatedSendProcessEvent（3369 → 10842）→ OnProcessDied + PROCESS_EXIT 事件（非 killId 事件）
      ├─ CancelTask("DELAY_KILL_PROCESS_...")（3370）
      └─ 【门禁 2】ExtensionType==UNSPECIFIED 或 UIExtension 才执行（3372-3375）
           → SendProcessKillEvent(appRecord, "Kill Reason:app exit") 双参重载（4834）
                killId = GetKillId()==-1 ? REASON_APP_EXIT : GetKillId()   ← 唯一兜底点（4844-4845）

【路径 B：死亡通知（无兜底）】
进程死亡（自行 exit / 被 SIGKILL）
  → AppDeathRecipient → OnRemoteDied (app_mgr_service_inner.cpp:6766)
      → AppRunningManager::OnRemoteDied (app_running_manager.cpp:728)：record 出 map
           └─ 补刀 KillProcessByPid(pid, "OnRemoteDied")（767，进程已死则 ESRCH 返回）
      → SendProcessKillEvent(appRecord) 单参重载（6785 → 6719）
           → GetProcessKillReason(killInfo, GetKillId() 原始值, GetKillMsg())   ← 无兜底
```

### 3.2 单参路径内 killId 的取值逻辑（appfreeze_manager.cpp:1372-1392）

```
GetProcessKillReason(killInfo, killId, killMsg):
  killId == -2  → killReason = "InvalidKillId"，killId = -2
  killId <  0（即 -1）→ GetExitKernelReason：ioctl(/dev/sysload, GET_KILL_INFO)
        ├─ 成功 → FillProcessKillInfoFromKernel（1394-1432）：
        │          killId/killReason/adj/foreground 全部以内核记录为准（覆盖 -1）
        └─ 失败 → killReason="InvalidKillId"，返回 false
                   → 调用方 600ms 后 FFRT 重试一次（app_mgr_service_inner.cpp:227、6754-6760），
                     仍失败则带 killId=-1 + reason="InvalidKillId" 上报
  killId >= 0 → killReason = ProcessKillReason::GetKillReason(killId)，killId = killId
                并 SetExitKernelReason(pid) 标记内核记录已处理（1390）
```

注：`ProcessKillInfo.killId` 成员默认初始化为 -1（appfreeze_manager.h:100），因此失败分支事件中的 PROCESS_KILL_ID 数值仍为 -1，但语义字符串为 "InvalidKillId" 或空，无法体现 REASON_APP_EXIT。

---

## 4. 问题一：正常关闭场景 REASON_APP_EXIT 为什么没有成功上报

REASON_APP_EXIT 兜底的完整触发条件（必须同时满足）：

1. 应用进程在 3s 内回调 `ApplicationTerminated`（record 未被 OnRemoteDied 先行移除）；
2. appRecord 状态为 `APP_STATE_READY` 或 `APP_STATE_BACKGROUND`（app_mgr_service_inner.cpp:3338）；
3. ExtensionType 为 UNSPECIFIED 或 UIExtension（3372）；
4. `GetKillId() == -1`（4844，严格等于，任何 `SetKillId` 写入的 -2 / REASON_USER_REQUEST / 具体 KillEventId 均使兜底失效）。

任一条件不满足时的降级路径与后果：

| # | 失败条件 | 触发场景 | 后果 |
|---|---|---|---|
| 1 | 状态门禁（3338） | 前台应用被直接关闭、 terminateSelf 未先走 Background 调度（app record 状态仍为 FOREGROUND/FOCUS，状态仅由 `ApplicationBackgrounded` 置 BACKGROUND，3260-3288） | graceful 路径整体跳过（连清理都不做）→ 进程退出 → 走路径 B |
| 2 | 竞态：death 通知先于确认 IPC | 应用侧先 `runner->Stop()` 再发确认（main_thread.cpp:3169-3174），进程随即退出；AppMgr 先处理 OnRemoteDied 并移除 record | `ApplicationTerminated` 查不到 record（3333 "get appRecord fail"）→ 带兜底路径未执行 |
| 3 | 3s 超时未确认 | 应用 onTerminate/PerformTerminate 阻塞 | DELAY_KILL_PROCESS → SIGKILL → 路径 B（详见第 5 节） |
| 4 | killId 已被改写 | ① SCB `isUserRequestedExit=true` → `RecordUIAbilityExitReason`（app_exit_reason_helper.cpp:535-537）→ `SetKillId(REASON_USER_REQUEST)`（app_mgr_service_inner.cpp:8544）；② SA kill 流程写入具体值或 -2 | 双参路径上报该值，REASON_APP_EXIT 分支不生效 |
| 5 | Extension 过滤（3372） | 非 UI 的 Extension 进程正常关闭 | 该场景根本不发 PROCESS_KILL |
| 6 | 路径 B 本身无兜底 | 上述任何降级后 | killId=-1 交内核 ioctl；失败 → "InvalidKillId"；成功 → 内核值覆盖 |

> 补充：`KillProcessWithReasonInner` 存在同类兜底（killId==-1 → REASON_KILL_APPLICATION，ability_manager_service.cpp:14733-14734），说明"调用方未显式指定 killId 时由服务侧补语义值"是既有设计模式，但该模式未覆盖 AppMgr 的单参上报路径。

---

## 5. 问题二：terminateAbility 场景为什么没有上报 killId

### 5.1 场景定义

日志中 `kill reason=TerminateAbility`（app_mgr_service_inner.cpp:4815）来自 3s 兜底任务（app_running_manager.cpp:975）：

```
DELAY_KILL_PROCESS 超时 → KillProcessByPid(pid, "TerminateAbility")
  → KillProcessByPidInner → kill(pid, SIGKILL)（app_mgr_service_inner.cpp:4865-4901）
     该函数：SetExitMsg / SetKillReason("TerminateAbility") / SetIsKillPrecedeStart / AddToKillProcessMap
     ★ 不调用任何 SendProcessKillEvent，不调用 SetKillId
  → 进程死亡 → OnRemoteDied → 单参 SendProcessKillEvent（6785）
```

### 5.2 报不出有效 killId 的三个结构性原因

**原因一：整条 kill 链路无 `SetKillId`。**
`KillProcessByPid`/`KillProcessByPidInner`/`AppRunningManager::OnRemoteDied` 补刀（"OnRemoteDied"）均只写 killReason/exitMsg，killId_ 保持 -1。对比：能报出具体 killId 的场景全部先经 `NotifyAppMgrRecordExitReasonCompability → SetKillId`（8544）。

**原因二：上报落在无兜底的单参重载。**
GetKillId()=-1 → `GetProcessKillReason` 的 killId<0 分支 → 内核 ioctl：
- AppMgr 用户态主动 SIGKILL 的进程，内核 sysload 若无该 pid 记录（或 open/ioctl 失败）→ 600ms 重试后仍失败 → 事件 reason="InvalidKillId"、PROCESS_KILL_ID=-1；
- ioctl 成功 → killId/killReason 完全取内核记录，AppMgr 侧语义（"TerminateAbility"）无法补入。

**原因三：与 graceful 路径互斥。**
能执行到超时强杀，即证明 `ApplicationTerminated` 未成功完成（record 未清理、`DELAY_KILL_PROCESS` 未被 Cancel，对比 3370）。而 REASON_APP_EXIT 兜底恰好在 graceful 路径内部 → 两事件在当前代码中不可能同时发生。另外由于 `SetKillReason` first-wins，此时 record 的 killReason 已是更早写入的 "Kill Reason:ClearSession"，"TerminateAbility" 不会出现在事件中。

### 5.3 特例（区分日志用）

若关闭前 SCB 以 `isUserRequestedExit=true` 走过 `CloseUIAbilityBySCB`/`CleanUIAbilityBySCB`（ability_manager_service.cpp:5844-5846 / 18038-18040），record 的 killId 已被置为 REASON_USER_REQUEST，后续路径 B 走 killId>=0 分支，事件报 REASON_USER_REQUEST 而非 -1/InvalidKillId。

---

## 6. 已排除的假设

| 假设 | 排除依据 |
|---|---|
| 双参路径 `HisyseventReport(6)` 容量不足导致第 7 个参数（killId）被丢弃 | hisysevent_report.h:30-39：构造参数 len 仅做 >64 的防御检查，`length_` 恒为 SYSTEM_PARAM_MAX_LEN=64，7 个参数全部写入 |
| `SetKillId` 在 kill 链路中被调用但值被覆盖 | 全链路（KillProcessByPid/Inner、AppRunningManager::OnRemoteDied）确认无 SetKillId 调用 |
| 单参路径事件丢失 | `ReportProcessKillEvent` 在立即与 FFRT 分支均无条件调用（6754-6763）；AppRunningManager::OnRemoteDied 无抑制条件（728-777） |

---

## 7. 验证方法

hilog（APPMGR tag）关键日志：

```bash
hilog | grep -E "APPMGR" | grep -E \
  "kill reason=TerminateAbility|current state is not background or ready|get appRecord fail|Get killReason ioctl failed|InvalidKillId|send event \[FRAMEWORK,PROCESS_KILL\]"
```

判读：

- 出现 `current state is not background or ready` → 门禁 1 拦截（原因 #1）；
- 出现 `get appRecord fail` 且此前有 death 日志 → 竞态（原因 #2）；
- 出现 `kill reason=TerminateAbility` + `Get killReason ioctl failed` → 超时强杀且内核无记录（第 5 节）；
- 事件中 `PROCESS_KILL_ID` 为非 -1 正值 → 该值来自 `SetKillId`（SA/SCB 记录）或内核覆盖。

相关单测：`SendProcessKillEvent_001~008`（app_mgr_service_inner_tdd_test.cpp，其中 _008 验证 "SetKillId 后单参上报该值" 的设计意图）、`ApplicationTerminated_001`、`app_running_record_test`（SetKillId/GetKillId）。

---

## 8. 修复建议与影响评估

> **勘误（2026-09-28 v2）**：初版误将 `REASON_APP_EXIT` 推断为 -1，并据此认为 kill 链路补值存在"-1 哨兵数值碰撞"、需要新增 `killIdSet_` 标志。经核对 hiviewdfx_hicollie 仓 `xcollie/process_kill_reason.h` 真实定义，`REASON_APP_EXIT = 4`，与哨兵值无关联，**修复可简化为单点改动**，初版方案 A/B 的复杂度前提不成立。

### 8.1 推荐方案（单点改动）

在 terminateAbility 超时强杀的 killProcess lambda（app_running_manager.cpp:975 之前）显式置位：

```cpp
if (appRecordSptr->GetKillId() == DEFAULT_KILL_ID) {   // 文件内新增 constexpr int32_t DEFAULT_KILL_ID = -1;
    appRecordSptr->SetKillId(HiviewDFX::ProcessKillReason::KillEventId::REASON_APP_EXIT);
    appRecordSptr->SetKillMsg("Kill Reason:app exit"); // 可选：补齐事件 MESSAGE 字段（对齐 graceful 路径）
}
```

原理：`REASON_APP_EXIT = 4`，置位后单参 `SendProcessKillEvent → GetProcessKillReason` 走 `killId >= 0` 分支（appfreeze_manager.cpp:1386-1389），事件直接上报 `PROCESS_KILL_ID=4`、`reason=GetKillReason(4)="app exit"`，并顺带 `SetExitKernelReason` 标记内核记录；无需修改 appfreeze_manager、app_mgr_service_inner，无需新增 record 成员与 mock 桩。

行为影响：

| 场景 | 改前 | 改后 |
|---|---|---|
| terminateAbility/terminateSelf 超时强杀 | PROCESS_KILL_ID=-1 + "InvalidKillId"（或内核值） | **PROCESS_KILL_ID=4 (REASON_APP_EXIT)** + "app exit"，立即上报（无 600ms 重试） |
| 应用 3s 内正常确认（graceful） | 双参兜底 -1→4 | 不变（killId 已为 4，双参三元分支结果相同） |
| SCB isUserRequestedExit 已置 REASON_USER_REQUEST | 报 1000 | 不变（`== DEFAULT_KILL_ID` 守卫避免覆盖） |
| 内核/LMK 杀、SA 显式 kill（含 -2 InvalidKillId） | 不变 | 不变（不经此 lambda） |

### 8.2 根因方案（可选，需单独评估）

| 方案 | 改动点 | 收益 | 风险 |
|---|---|---|---|
| C. graceful 门禁放行 | 3338 状态门禁放行 FOREGROUND 态的终止确认 | 修复降级根因 #1，应用未超时也能走带兜底路径 | 状态机语义变更，影响 `ApplicationTerminated` 其余清理逻辑，需回归生命周期用例 |

### 8.3 合规性提示（依据 AGENTS.md 约束）

- 不修改 `hisysevent.yaml` 事件定义，仅改变参数取值语义；需与订阅方确认对存量 `PROCESS_KILL_ID=-1`、`reason="InvalidKillId"` 过滤逻辑的兼容（该取值在该场景将消失）。
- `IsExitReasonValid`（ability_manager_service.cpp:19607）按 `GetKillReason` 是否含 "InvalidKillId" 校验，置位 4 后可通过该校验。
- 不涉及公共 API（interfaces/kits）、IPC 序列化格式、权限校验变更；属 DFX 可观测性修复。
- 验证命令：`./build.sh --product-name <product> --build-target libappms`；单测 `SendProcessKillEvent_001~008`、`app_running_record_test`，并补充 "terminateAbility 超时强杀事件上报 PROCESS_KILL_ID=4" 的断言用例。

---

## 9. 附录：关键代码位置索引

| 位置 | 内容 |
|---|---|
| services/appmgr/src/app_mgr_service_inner.cpp:4834-4863 | 双参 SendProcessKillEvent（含 REASON_APP_EXIT 兜底 4844-4845） |
| services/appmgr/src/app_mgr_service_inner.cpp:6719-6764 | 单参 SendProcessKillEvent（OnRemoteDied 路径） |
| services/appmgr/src/app_mgr_service_inner.cpp:6677-6717 | ReportProcessKillEvent（26 参数版） |
| services/appmgr/src/app_mgr_service_inner.cpp:3325-3377 | ApplicationTerminated（门禁 3338、Extension 过滤 3372、上报 3374） |
| services/appmgr/src/app_mgr_service_inner.cpp:4790-4901 | KillProcessByPid / KillProcessByPidInner（无 SetKillId、无直接上报） |
| services/appmgr/src/app_mgr_service_inner.cpp:8527-8563 | NotifyAppMgrRecordExitReasonCompability（SetKillId 8544） |
| services/appdfr/src/appfreeze_manager.cpp:1372-1465 | GetProcessKillReason / GetExitKernelReason（内核 ioctl） |
| services/appdfr/include/appfreeze_manager.h:89-107 | ProcessKillInfo 结构（killId 默认 -1） |
| services/appmgr/src/app_running_manager.cpp:912-1030 | TerminateAbility（ClearSession 942、DELAY_KILL_PROCESS 1023-1027、killProcess lambda 957-981） |
| services/appmgr/src/app_running_manager.cpp:728-777 | AppRunningManager::OnRemoteDied（补刀 767） |
| services/appmgr/src/app_lifecycle_deal.cpp:141 | ScheduleTerminateApplication |
| frameworks/native/appkit/app/main_thread.cpp:3145-3175 | HandleTerminateApplication（runner->Stop 与确认 IPC 的顺序） |
| services/abilitymgr/src/app_exit_reason_helper.cpp:512-550 | RecordUIAbilityExitReason（REASON_USER_REQUEST 透传） |
| services/abilitymgr/src/ability_manager_service.cpp:14701-14744 | KillProcessWithReasonInner（REASON_KILL_APPLICATION 兜底 14733） |
