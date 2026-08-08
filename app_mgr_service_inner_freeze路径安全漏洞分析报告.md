# C++代码安全漏洞分析报告 — app_mgr_service_inner.cpp Freeze/FaultData 相关路径

**分析日期**: 2026-08-07  
**分析文件**: `services/appmgr/src/app_mgr_service_inner.cpp`（13242行）  
**分析范围**: 与 freeze/faultData 相关的所有代码路径  
**分析依据**: C++代码防故障注入安全排查指南（8大类规则）

---

## 一、分析范围

本次分析聚焦 `app_mgr_service_inner.cpp` 中与 freeze/faultData 相关的函数，包括：

| 函数名 | 行号范围 | 功能概述 |
|--------|---------|---------|
| `GetEventName` | 440-451 | 事件ID转AppFreezeType名称 |
| `GetEventTypeAndMsg` | 453-490 | 事件ID转类型属性和消息 |
| `Init` (dfx部分) | 515-517 | 初始化dfx_freeze_task_queue |
| `KillProcessByPid` (freeze部分) | 4664-4665 | 杀进程时插入KillThread记录 |
| `SendProcessKillEvent` | 6457-6508 | 发送进程杀死事件，调用GetProcessKillReason |
| `OnRemoteDied` (freeze部分) | 6530-6532 | 进程死亡时上报AppFreeze并清理 |
| `SendHiSysEvent` | 8069-8105 | 生命周期超时发送HiSysEvent并调用LifecycleTimeoutHandle |
| `RecordAppRecoveryNotifyAppReason` | 9225-9236 | 记录AppRecovery通知原因 |
| `AppRecoveryNotifyApp` | 9238-9280 | AppRecovery通知应用处理 |
| `ParseInfoToAppfreeze` | 9282-9304 | 解析FaultData到AppfreezeManager |
| `GetExceptionTimerId` | 9307-9344 | 获取异常定时器ID |
| `UpdateForeground` | 9346-9359 | 更新前台状态 |
| `SubmitDfxFaultTask` | 9361-9411 | 提交DFX故障任务 |
| `NotifyAppFault` | 9413-9490 | 通知应用故障（主入口） |
| `CheckAppFault` | 9492-9502 | 检查应用故障 |
| `RecordAppfreezeKillReason` | 9505-9515 | 记录Appfreeze杀死原因 |
| `KillFaultApp` | 9524-9549 | 杀死故障应用 |
| `TimeoutNotifyApp` | 9551-9574 | 超时通知应用 |
| `TransformedNotifyAppFault` | 9576-9627 | 转换并通知应用故障（SA调用入口） |
| `NotifyAppFaultBySA` | 9629-9650 | SA调用通知应用故障 |
| `SetAppFreezeFilter` | 9652-9678 | 设置AppFreeze过滤 |
| `ConvertDataTypes` | 9680-9725 | AppFaultDataBySA转FaultData |
| `FaultTypeToString` | 9727-9749 | FaultDataType转字符串 |

---

## 二、发现汇总

| 严重级别 | 数量 |
|---------|------|
| P0（高危） | 5 |
| P1（中危） | 8 |
| P2（低危） | 4 |
| **合计** | **17** |

---

## 三、详细发现清单

### 【P0-01】外部输入未校验 — IPC传入的FaultData字段未校验即使用

| 项目 | 内容 |
|------|------|
| **规则** | 1.1 外部输入未校验 |
| **文件** | `app_mgr_service_inner.cpp:9413-9490` |
| **函数** | `NotifyAppFault` |
| **代码** | `int32_t AppMgrServiceInner::NotifyAppFault(const FaultData &faultData)` → 直接使用 `faultData.errorObject.name`、`faultData.faultType`、`faultData.forceExit`、`faultData.waitSaveState` 等 |
| **问题** | `NotifyAppFault` 是 IPC 入口，`FaultData` 来自客户端进程。`faultData.errorObject.name` 是字符串，直接用于 `AppFreezeType` 枚举比较（如 `THREAD_BLOCK_3S`、`LIFECYCLE_TIMEOUT` 等），但没有校验该字符串是否为合法的 `AppFreezeType` 值。恶意客户端可以构造任意 `name` 值，绕过特定事件类型的处理逻辑。例如：构造 `name = "LIFECYCLE_TIMEOUT"` 可触发进程杀死逻辑，而正常情况下需要经过完整的3s/6s超时检测流程。 |
| **攻击路径** | 恶意客户端通过 IPC 调用 `NotifyAppFault` → 传入 `faultData.errorObject.name = "LIFECYCLE_TIMEOUT"` → 跳过正常超时检测 → 直接触发进程杀死 |
| **建议** | 在 `NotifyAppFault` 入口处校验 `faultData.errorObject.name` 是否为合法的 `AppFreezeType` 枚举值；校验 `faultData.faultType` 是否为合法的 `FaultDataType` |

---

### 【P0-02】外部输入未校验 — TransformedNotifyAppFault 中 pid 来自 IPC 数据

| 项目 | 内容 |
|------|------|
| **规则** | 1.1 外部输入未校验 |
| **文件** | `app_mgr_service_inner.cpp:9576-9627` |
| **函数** | `TransformedNotifyAppFault` |
| **代码** | `int32_t pid = faultData.pid;` → `auto record = GetAppRunningRecordByPid(pid);` |
| **问题** | `AppFaultDataBySA.pid` 来自 IPC 调用方（SA），调用方可以传入任意 pid。虽然 `GetAppRunningRecordByPid` 会返回 null（如果 pid 不存在），但攻击者可以传入其他应用的 pid，导致该应用的 freeze 检测逻辑被触发，可能造成目标应用被误杀。此外，`faultData.errorObject.name`、`faultData.errorObject.message`、`faultData.errorObject.stack` 等字段也来自 IPC，未做长度校验。 |
| **攻击路径** | 恶意 SA 传入目标应用的 pid + 构造的 FaultData → 触发 `TimeoutNotifyApp` → `KillFaultApp` → 目标应用被误杀 |
| **建议** | 校验 `faultData.pid` 是否与 `IPCSkeleton::GetCallingPid()` 对应的进程一致，或至少校验 `pid` 属于调用方的 UID 范围；对字符串字段做长度上限校验 |

---

### 【P0-03】数据竞争 — const_cast 移除 const 后修改共享数据

| 项目 | 内容 |
|------|------|
| **规则** | 4.2 数据竞争 |
| **文件** | `app_mgr_service_inner.cpp:9439-9463` |
| **函数** | `NotifyAppFault` |
| **代码** | `FaultData& nonConstFaultData = const_cast<FaultData&>(faultData);` → `nonConstFaultData.isInForeground = UpdateForeground(...);` → `nonConstFaultData.appRunningUniqueId = ...;` |
| **问题** | `NotifyAppFault` 参数为 `const FaultData&`，但通过 `const_cast` 移除 const 后修改了 `isInForeground` 和 `appRunningUniqueId` 字段。`const_cast` 本身是代码异味，表明接口设计不当。更严重的是，`nonConstFaultData` 是对原始 IPC 反序列化数据的引用，如果 IPC 框架对反序列化数据有缓存或共享，修改可能导致数据竞争。此外，`nonConstFaultData` 被传递到 `SubmitDfxFaultTask` 和 `KillFaultApp`，这些函数可能在其他线程执行，形成对同一数据的并发访问。 |
| **攻击路径** | 故障注入修改时序 → `nonConstFaultData` 在主线程被修改的同时，另一个线程读取 → 数据竞争 |
| **建议** | 创建 `FaultData` 的副本进行修改，而非 `const_cast`；或修改函数签名为非 const |

---

### 【P0-04】数据竞争 — KillFaultApp 中 lambda 捕获 faultData 的副本但 weak_ptr 可能失效

| 项目 | 内容 |
|------|------|
| **规则** | 4.2 数据竞争 / 2.2 悬垂指针 |
| **文件** | `app_mgr_service_inner.cpp:9524-9549` |
| **函数** | `KillFaultApp` |
| **代码** | `auto killAppTask = [pid, bundleName, faultData, isNeedExit, innerServiceWeak = weak_from_this(), recordId]() { auto innerService = innerServiceWeak.lock(); ... innerService->RecordAppfreezeKillReason(pid, faultData); ... }` → `taskHandler_->SubmitTaskJust(killAppTask, "killAppTask", waitTime);` |
| **问题** | `killAppTask` 通过 `weak_from_this()` 捕获了 `AppMgrServiceInner` 的弱引用。`SubmitTaskJust` 延迟 `waitTime`（3.5s 或 6s）后执行。如果 `AppMgrServiceInner` 在延迟期间被销毁，`innerServiceWeak.lock()` 返回 nullptr，虽然有 `CHECK_POINTER_AND_RETURN_LOG` 检查，但 `faultData` 是按值捕获的，其中包含 `token`（`sptr<IRemoteObject>`）等智能指针。如果 `faultData.token` 引用的远程对象在延迟期间死亡，`RecordAppfreezeKillReason` 中可能使用已失效的 token。 |
| **攻击路径** | 故障注入使 `AppMgrServiceInner` 在延迟期间被销毁 → `innerService` 为 nullptr → 任务静默失败；或 `faultData.token` 指向的远程对象已死亡 → `RecordAppfreezeKillReason` 中使用失效 token |
| **建议** | 在延迟任务中减少对 `faultData` 中复杂对象（如 `token`）的依赖，或确保 `RecordAppfreezeKillReason` 不依赖 `token` |

---

### 【P0-05】TOCTOU — NotifyAppFault 中 appRecord 状态检查与使用存在时间窗口

| 项目 | 内容 |
|------|------|
| **规则** | 4.1 TOCTOU |
| **文件** | `app_mgr_service_inner.cpp:9418-9466` |
| **函数** | `NotifyAppFault` |
| **代码** | `auto appRecord = GetAppRunningRecordByPid(pid);` → `if (appRecord->GetState() == ApplicationState::APP_STATE_TERMINATED ...)` → ... → `if (faultData.faultType == FaultDataType::APP_FREEZE) { if (CheckAppFault(appRecord, faultData)) { ... } }` → ... → `SubmitDfxFaultTask(nonConstFaultData, bundleName, appRecord, pid)` |
| **问题** | `NotifyAppFault` 获取 `appRecord` 后，在多个步骤中使用它。但 `appRecord` 的状态可能在这些步骤之间被其他线程修改（如 `OnRemoteDied` 在另一个线程中修改了 `appRecord` 的状态）。`GetAppRunningRecordByPid` 返回的 `shared_ptr` 保证了 `appRecord` 不被释放，但 `appRecord` 的内部状态（如 `GetState()`、`GetBundleName()`）可能已经改变。 |
| **攻击路径** | 故障注入修改时序 → `NotifyAppFault` 检查 `appRecord->GetState()` 为非 TERMINATED → 另一个线程将状态改为 TERMINATED → `NotifyAppFault` 继续处理 → 对已终止的应用执行 freeze 逻辑 |
| **建议** | 在关键操作前重新检查 `appRecord` 的状态，或使用锁保护状态检查和使用 |

---

### 【P1-01】外部输入未校验 — ConvertDataTypes 中 IPC 字段未校验

| 项目 | 内容 |
|------|------|
| **规则** | 1.1 外部输入未校验 |
| **文件** | `app_mgr_service_inner.cpp:9680-9725` |
| **函数** | `ConvertDataTypes` |
| **代码** | `newfaultData.errorObject.message += faultData.errorObject.message;` → `newfaultData.errorObject.stack = faultData.errorObject.stack;` → `newfaultData.procStatm = faultData.procStatm;` → ... |
| **问题** | `ConvertDataTypes` 将 `AppFaultDataBySA`（来自 IPC）转换为 `FaultData`。所有字段直接复制，没有对字符串长度做校验。`faultData.errorObject.stack` 和 `faultData.errorObject.message` 可能包含极长的字符串，如果攻击者构造超长数据，会导致后续处理中内存消耗过大。`faultData.procStatm` 也来自 IPC，未校验格式。 |
| **攻击路径** | 构造超长的 `errorObject.stack`（如 1MB）→ 传递到 `AppfreezeHandleWithStack` → 写入文件时消耗大量内存和磁盘空间 |
| **建议** | 对 IPC 传入的字符串字段增加长度上限校验 |

---

### 【P1-02】整数溢出 — SystemTimeMillisecond 可能溢出

| 项目 | 内容 |
|------|------|
| **规则** | 1.2 整数溢出导致逻辑绕过 |
| **文件** | `app_mgr_service_inner.cpp:9602-9603` |
| **函数** | `TransformedNotifyAppFault` |
| **代码** | `transformedFaultData.timeoutMarkers = "notifyFault:" + eventName + std::to_string(pid) + "-" + std::to_string(SystemTimeMillisecond());` |
| **问题** | `SystemTimeMillisecond()` 返回 `int64_t`，但 `std::to_string` 转换后的字符串拼接不会溢出。然而，`timeoutMarkers` 被用作 `dfxTaskHandler_->SubmitTask` 的任务标识符，如果 `timeoutMarkers` 字符串过长（`eventName` 来自 IPC 未校验），可能导致任务标识符冲突或内存问题。 |
| **建议** | 对 `eventName` 做长度校验 |

---

### 【P1-03】错误恢复不完整 — SubmitDfxFaultTask 重复提交相同任务

| 项目 | 内容 |
|------|------|
| **规则** | 3.2 错误恢复不完整-资源泄露可被反复触发 |
| **文件** | `app_mgr_service_inner.cpp:9397-9398` |
| **函数** | `SubmitDfxFaultTask` |
| **代码** | `dfxTaskHandler_->SubmitTask(notifyAppTask, "NotifyAppFaultTask");` `dfxTaskHandler_->SubmitTask(notifyAppTask, "NotifyAppFaultTask");` |
| **问题** | **同一任务被连续提交了两次**（第 9397 行和第 9398 行完全相同）。这看起来是一个 bug，会导致 `ParseInfoToAppfreeze` 被执行两次，`AppfreezeHandleWithStack` 被调用两次，可能导致重复的 HiSysEvent 上报和文件写入。如果攻击者反复触发 `NotifyAppFault`，这种重复提交会加倍资源消耗。 |
| **攻击路径** | 反复触发 `NotifyAppFault` → 每次 `SubmitDfxFaultTask` 提交两个相同任务 → `AppfreezeHandleWithStack` 执行两次 → 资源消耗加倍 |
| **建议** | 删除第 9398 行的重复提交 |

---

### 【P1-04】错误恢复不完整 — GetExceptionTimerId 中 std::thread 创建但异常时未清理

| 项目 | 内容 |
|------|------|
| **规则** | 3.2 错误恢复不完整-资源泄露可被反复触发 |
| **文件** | `app_mgr_service_inner.cpp:9307-9344` |
| **函数** | `GetExceptionTimerId` |
| **代码** | `auto exceptionCallback = [...](void *) { ... std::thread dfxThread(threadTask); if (dfxThread.joinable()) { dfxThread.join(); } };` → `exceptionId = HiviewDFX::XCollie::GetInstance().SetTimer("DfxFault::Exception", timeout, exceptionCallback, nullptr, HiviewDFX::XCOLLIE_FLAG_NOOP);` |
| **问题** | `GetExceptionTimerId` 设置了一个 XCollie 定时器，超时回调中创建新线程执行 `threadTask`。如果 `threadTask` 中的 `innerService->CheckAppFault` 或 `ParseInfoToAppfreeze` 抛出异常，`std::thread` 会在析构时调用 `std::terminate`（因为未 join 的线程会导致 terminate）。此外，每次 XCollie 超时回调都会创建一个新线程，如果攻击者能反复触发 XCollie 超时，会创建大量线程。 |
| **攻击路径** | 故障注入使 XCollie 超时反复触发 → 每次创建新线程 → 线程数激增 → OOM |
| **建议** | 使用线程池替代直接创建 `std::thread`；确保 `threadTask` 中的异常被捕获 |

---

### 【P1-05】数据竞争 — AppRecoveryNotifyApp 中异步回调访问 appRecord

| 项目 | 内容 |
|------|------|
| **规则** | 4.2 数据竞争 |
| **文件** | `app_mgr_service_inner.cpp:9238-9280` |
| **函数** | `AppRecoveryNotifyApp` |
| **代码** | `auto waitSaveTask = [pid, bundleName, innerServiceWeak = weak_from_this(), recordId]() { ... auto appRecord = innerService->GetAppRunningRecordByPid(pid); ... if (bundleName == name) { innerService->KillProcessByPid(pid, ...); } };` → `taskHandler_->SubmitTaskJust(waitSaveTask, timeOutName, timeOut);` |
| **问题** | `waitSaveTask` 在 2 秒延迟后执行，此时 `appRecord` 的状态可能已经改变。`bundleName == name` 的比较可能已过时（进程可能已重新启动，但 pid 被复用）。此外，`GetAppRunningRecordByPid(pid)` 在延迟期间可能返回不同的 `appRecord`（pid 被复用），导致错误的进程被杀死。 |
| **攻击路径** | 故障注入使进程在 2 秒延迟内死亡并重新启动 → pid 被复用 → `waitSaveTask` 杀死新进程 |
| **建议** | 在 `waitSaveTask` 中增加进程启动时间校验，或使用 `appRunningUniqueId` 替代 pid |

---

### 【P1-06】数据竞争 — SetAppFreezeFilter 中异步任务与状态重置

| 项目 | 内容 |
|------|------|
| **规则** | 4.2 数据竞争 |
| **文件** | `app_mgr_service_inner.cpp:9652-9678` |
| **函数** | `SetAppFreezeFilter` |
| **代码** | `bool result = AppExecFwk::AppfreezeManager::GetInstance()->CancelAppFreezeDetect(pid, bundleName);` → `auto resetAppfreezeTask = [pid, bundleName]() { AppExecFwk::AppfreezeManager::GetInstance()->ResetAppfreezeState(pid, bundleName); };` → `dfxTaskHandler_->SubmitTaskJust(resetAppfreezeTask, "resetAppfreezeTask", waitTime);` |
| **问题** | `CancelAppFreezeDetect` 将状态设为 `CANCELING`，120 秒后 `ResetAppfreezeState` 将状态重置为 `CANCELED`。在 120 秒窗口内，如果进程再次触发 freeze 检测，`IsProcessDebug` 会返回 `true`（因为状态为 `CANCELING`），导致 freeze 检测被跳过。攻击者可以在 120 秒内反复触发 `SetAppFreezeFilter`，使 freeze 检测长期失效。 |
| **攻击路径** | 反复调用 `SetAppFreezeFilter` → freeze 检测持续被跳过 → 应用冻结无法被检测 |
| **建议** | 增加 `SetAppFreezeFilter` 的调用频率限制，或缩短重置时间 |

---

### 【P1-07】系统调用返回值未检查 — KillProcessByPid 返回值未用于流程控制

| 项目 | 内容 |
|------|------|
| **规则** | 3.1 系统调用返回值未检查 |
| **文件** | `app_mgr_service_inner.cpp:4664-4665` |
| **函数** | `KillProcessByPid` |
| **代码** | `AppExecFwk::AppfreezeManager::GetInstance()->InsertKillThread(ret, pid, appRecord->GetUid(), appRecord->GetBundleName());` |
| **问题** | `InsertKillThread` 在 `KillProcessByPid` 中被调用，无论 `ret`（`KillProcessByPid` 的返回值）是否成功。`InsertKillThread` 内部使用 `ret` 作为 `state` 参数，如果 `ret` 为负数（杀进程失败），`CheckThreadKilled` 中 `it->second.state >= 0` 会返回 false，不会触发重复杀死检测。但 `InsertKillThread` 仍然会将失败记录插入 map，导致 map 中存在无效条目。 |
| **攻击路径** | 反复触发杀进程失败 → `appfreezeFilterMap_` 积累无效条目 → 内存泄漏 |
| **建议** | 仅在 `ret >= 0` 时调用 `InsertKillThread` |

---

### 【P1-08】无界分配 — SendProcessKillEvent 中 HiSysEvent 参数无长度限制

| 项目 | 内容 |
|------|------|
| **规则** | 5.1 无界分配 |
| **文件** | `app_mgr_service_inner.cpp:6457-6508` |
| **函数** | `SendProcessKillEvent` |
| **代码** | `hisyseventReport->InsertParam(EVENT_KEY_MESSAGE, killInfo.killMsg);` → `hisyseventReport->InsertParam(EVENT_KEY_REASON, killInfo.killReason);` → `hisyseventReport->InsertParam(EVENT_KEY_PROCESS_NAME, eventInfo.processName);` |
| **问题** | `killInfo.killMsg` 和 `killInfo.killReason` 来自 `GetProcessKillReason`，其中 `killMsg` 可能包含 `faultData.errorObject.name`（来自 IPC，未校验长度）。如果 `killMsg` 极长，`InsertParam` 可能分配大量内存。 |
| **攻击路径** | 构造超长的 `killMsg` → `InsertParam` 分配大量内存 → OOM |
| **建议** | 对 `killMsg` 和 `killReason` 做长度截断 |

---

### 【P2-01】外部输入未校验 — SendHiSysEvent 中 GetParameter 读取系统属性

| 项目 | 内容 |
|------|------|
| **规则** | 1.1 外部输入未校验 |
| **文件** | `app_mgr_service_inner.cpp:8076-8083` |
| **函数** | `SendHiSysEvent` |
| **代码** | `char paramOutBuf[bufferLen] = {0}; const char *hook_mode = "startup:"; int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen); if (ret > 0 && strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) == 0) { return; }` |
| **问题** | `GetParameter("libc.hook_mode", ...)` 读取系统属性。如果攻击者能修改该属性，可以跳过生命周期超时处理。虽然 `libc.hook_mode` 是系统级属性，修改需要 root 权限，但在越狱设备上可能被利用。 |
| **攻击路径** | 修改 `libc.hook_mode` 系统属性 → `SendHiSysEvent` 直接返回 → 生命周期超时不被处理 |
| **建议** | 此检查属于调试辅助逻辑，生产版本中应移除或使用更严格的条件 |

---

### 【P2-02】数据竞争 — NotifyAppFaultBySA 中权限检查与处理存在时间窗口

| 项目 | 内容 |
|------|------|
| **规则** | 4.1 TOCTOU |
| **文件** | `app_mgr_service_inner.cpp:9629-9650` |
| **函数** | `NotifyAppFaultBySA` |
| **代码** | `if (AAFwk::PermissionVerification::GetInstance()->IsSACall() || callerBundleName == SCENE_BOARD_BUNDLE_NAME) { return TransformedNotifyAppFault(faultData); }` |
| **问题** | 权限检查（`IsSACall()`）和实际处理（`TransformedNotifyAppFault`）之间存在时间窗口。`IsSACall()` 检查调用方的 PID/UID，但在 IPC 处理期间，调用方的进程可能已经退出，其 PID 可能被复用。虽然这个窗口很小，但理论上存在。 |
| **建议** | 在 `TransformedNotifyAppFault` 内部再次校验调用方身份 |

---

### 【P2-03】错误恢复不完整 — AppRecoveryNotifyApp 中 KillProcessByPid 后未检查结果

| 项目 | 内容 |
|------|------|
| **规则** | 3.1 系统调用返回值未检查 |
| **文件** | `app_mgr_service_inner.cpp:9245-9246` |
| **函数** | `AppRecoveryNotifyApp` |
| **代码** | `KillProcessByPid(pid, "AppRecoveryNotifyApp");` |
| **问题** | `KillProcessByPid` 的返回值被忽略。如果杀进程失败，`AppRecoveryNotifyApp` 不会重试或记录失败，可能导致应用处于不一致状态。 |
| **建议** | 检查 `KillProcessByPid` 的返回值并记录失败 |

---

### 【P2-04】权限降级不完整 — SetAppFreezeFilter 中 callingPid == pid 检查不足

| 项目 | 内容 |
|------|------|
| **规则** | 7.3 权限降级不完整 |
| **文件** | `app_mgr_service_inner.cpp:9652-9678` |
| **函数** | `SetAppFreezeFilter` |
| **代码** | `int32_t callingPid = IPCSkeleton::GetCallingPid();` → `if (callingPid == pid && AppExecFwk::AppfreezeManager::GetInstance()->IsValidFreezeFilter(pid, bundleName)) {` |
| **问题** | `SetAppFreezeFilter` 仅检查 `callingPid == pid`，即只允许进程为自己设置 freeze 过滤。但没有检查调用方的 UID 是否与目标进程的 UID 一致。如果攻击者通过 PID 复用（进程死亡后 PID 被新进程复用），可能在 `callingPid == pid` 检查通过后，目标进程已经变为不同 UID 的进程。 |
| **攻击路径** | 进程 A 调用 `SetAppFreezeFilter(自己的pid)` → 进程 A 死亡 → PID 被进程 B 复用 → freeze 过滤对进程 B 生效 |
| **建议** | 增加 UID 校验，确保调用方的 UID 与目标进程的 UID 一致 |

---

## 四、按漏洞规则类别统计

| 规则类别 | 发现数 | P0 | P1 | P2 |
|---------|--------|----|----|-----|
| 1.1 外部输入未校验 | 4 | 2 | 1 | 1 |
| 1.2 整数溢出 | 1 | 0 | 1 | 0 |
| 2.2 悬垂指针/UAF | 1 | 1 | 0 | 0 |
| 3.1 系统调用返回值未检查 | 2 | 0 | 1 | 1 |
| 3.2 错误恢复不完整 | 2 | 0 | 2 | 0 |
| 4.1 TOCTOU | 2 | 1 | 0 | 1 |
| 4.2 数据竞争 | 4 | 2 | 2 | 0 |
| 5.1 无界分配 | 1 | 0 | 1 | 0 |
| 7.3 权限降级不完整 | 1 | 0 | 0 | 1 |
| **合计** | **17** | **5** | **8** | **4** |

---

## 五、重点修复建议

### 最高优先级（P0）

1. **P0-01/P0-02 IPC 输入校验**：`NotifyAppFault` 和 `TransformedNotifyAppFault` 中，`FaultData` 来自 IPC，必须校验 `errorObject.name` 是否为合法 `AppFreezeType`，校验 `pid` 是否属于调用方 UID 范围，对字符串字段做长度上限校验。

2. **P0-03 const_cast 数据竞争**：`NotifyAppFault` 中 `const_cast<FaultData&>(faultData)` 是严重的代码异味，应创建副本进行修改。

3. **P0-04 延迟任务中的弱引用**：`KillFaultApp` 中延迟 3.5s/6s 执行的任务依赖 `weak_from_this()`，如果 `AppMgrServiceInner` 被销毁，任务静默失败。应确保关键流程不被静默跳过。

4. **P0-05 TOCTOU**：`NotifyAppFault` 中 `appRecord` 的状态检查与使用之间存在时间窗口，应在关键操作前重新检查状态。

### 中优先级（P1）

1. **P1-03 重复提交**：`SubmitDfxFaultTask` 第 9397-9398 行完全重复，应删除第 9398 行。

2. **P1-04 线程创建**：`GetExceptionTimerId` 中 XCollie 超时回调直接创建 `std::thread`，应改用线程池。

3. **P1-05 PID 复用**：`AppRecoveryNotifyApp` 延迟任务中应增加进程启动时间校验。

4. **P1-06 Freeze 过滤**：`SetAppFreezeFilter` 应增加调用频率限制。

---

## 六、未发现问题的规则

| 规则 | 说明 |
|------|------|
| 1.3 数组越界 | 未发现。所有容器访问前均有 `size()` 检查或使用 `at()` |
| 2.1 空指针解引用 | 未发现。`appRecord` 在使用前均有 nullptr 检查 |
| 2.3 缓冲区溢出 | 未发现。未使用不安全的 C 字符串操作 |
| 3.3 异常穿越C边界 | 未发现。代码中未使用 C++ 异常 |
| 4.3 死锁-锁顺序不一致 | 未发现明显的反向锁序。`exceptionLock_` 在 `OnRemoteDied` 中使用，与 `AppfreezeManager` 内部锁不冲突 |
| 5.2 FD/句柄泄露 | 未发现。freeze 路径中未直接操作 FD |
| 5.3 递归深度无限制 | 未发现递归调用 |
| 6.1 类型标签伪造 | 未发现。`FaultData` 的类型字段在 IPC 反序列化时由框架处理 |
| 6.2 格式化字符串漏洞 | 未发现。TAG_LOG 宏使用固定格式串 |
| 7.1 代码注入-命令行 | 未发现 `system()`/`popen()` 调用 |
| 7.2 动态加载 | 未发现 `dlopen` 调用 |
| 8.1-8.4 编译与链接防护 | 属于构建配置层面，不在源码分析范围内 |

---

## 七、代码行级问题标记

| 行号 | 问题 | 严重级别 |
|------|------|---------|
| 9413-9490 | `NotifyAppFault` 缺少 IPC 输入校验 | P0 |
| 9576-9627 | `TransformedNotifyAppFault` 中 pid 来自 IPC | P0 |
| 9439 | `const_cast<FaultData&>(faultData)` | P0 |
| 9524-9549 | `KillFaultApp` 延迟任务中弱引用 | P0 |
| 9418-9466 | `NotifyAppFault` 中 appRecord 状态 TOCTOU | P0 |
| 9680-9725 | `ConvertDataTypes` 缺少字符串长度校验 | P1 |
| 9602-9603 | `timeoutMarkers` 拼接中 `eventName` 未校验 | P1 |
| 9397-9398 | **重复提交相同任务（bug）** | P1 |
| 9307-9344 | `GetExceptionTimerId` 中直接创建 `std::thread` | P1 |
| 9238-9280 | `AppRecoveryNotifyApp` 中 PID 复用风险 | P1 |
| 9652-9678 | `SetAppFreezeFilter` 可被反复调用 | P1 |
| 4664-4665 | `InsertKillThread` 不检查 `ret` | P1 |
| 6457-6508 | `SendProcessKillEvent` 中参数无长度限制 | P1 |
| 8076-8083 | `SendHiSysEvent` 中 `GetParameter` | P2 |
| 9629-9650 | `NotifyAppFaultBySA` 中 TOCTOU | P2 |
| 9245-9246 | `KillProcessByPid` 返回值未检查 | P2 |
| 9652-9678 | `SetAppFreezeFilter` 缺少 UID 校验 | P2 |
