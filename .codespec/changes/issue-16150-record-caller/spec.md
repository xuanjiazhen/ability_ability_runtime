# Spec

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 终止应用接口记录调用方归因信息 |
| 特性编号 | issue-16150 |
| 优先级 | P1 |
| 复杂度 | 标准（引用 proposal.md 初始分级判断） |
| 目标版本 | 7.1 |

## 用户故事或场景

### US-1: 应用异常终止后的跨层定界

**作为** 系统维测人员/应用开发者, **我想要** 在应用被系统终止后，应用退出原因记录中能查到"是哪个系统服务/进程发起的终止"（调用方 pid + processname）, **以便** 完成跨层定界与大数据二级拆解分类，缩短故障定位时间。

**验收标准：**

> 使用 Given/When/Then 格式。Then 描述通过三层接口边界可观测的结果（此处为 inner API `AppExitReasonDataManager::GetAppExitReason` 返回的持久化记录 / `aa dump` 输出 / HiSysEvent 字段）。
> 调用方 pid 在服务侧通过 `IPCSkeleton::GetCallingPid()` 自动获取，进程名经 `/proc/<pid>/cmdline` 解析；不修改 `ExitReason`/`ExitReasonCompability` 的 IPC 序列化字段。
> 注意：`IN_PROCESS_CALL` 宏会先 `ResetCallingIdentity()`，各入口的 caller 捕获必须发生在任何 `IN_PROCESS_CALL` 之前；`ForceExitApp` 经 `IN_PROCESS_CALL` 委托 `RecordAppWithReason`，须在自身入口捕获并经内部参数透传。

- **AC-1.1:** Given 产品配置支持 SA kill 权限（`SupportSystemAbilityPermission::IsSupportSaKillPermission()` 为真）或调用方为 shell，系统服务 SA-X 通过 IPC 调用 `AbilityManagerService::KillAppWithReason(pid, exitReason)` When 服务侧完成权限校验并写入退出原因记录 Then 通过 inner API `AppExitReasonDataManager::GetAppExitReason` 查询该被终止应用的记录时，记录中 `caller_pid` 字段值等于 SA-X 进程的 pid（由 `IPCSkeleton::GetCallingPid()` 在服务侧获取）。
- **AC-1.2:** Given 系统服务 SA-X 调用 `KillBundleWithReason(bundleName, userId, appIndex, exitReason)` When 服务侧写入退出原因记录 Then 查询记录中同时包含 `caller_pid`（= SA-X pid）与 `caller_process_name`（= SA-X 进程名，由服务侧读取 `/proc/<caller_pid>/cmdline` 解析，对系统服务进程同样可解析）。
- **AC-1.3:** Given 调用方通过 `ForceExitApp(pid, exitReason)`（aa 命令 / 客户端）终止应用 When 服务侧写入退出原因记录 Then 记录中 `caller_pid` 为 `ForceExitApp` IPC 直接调用方进程 pid（在 `ForceExitApp` 自身入口捕获，而非委托链内的 `RecordAppWithReason` 中捕获——后者经 `IN_PROCESS_CALL` 后 calling identity 已重置）。
- **AC-1.4:** Given 上层框架进程通过 C 接口 `RecordAppWithReason(pid, uid, exitReason)` 经 `AbilityManagerClient` 代理 IPC 调用 AMS When 服务侧写入记录 Then 记录中 `caller_pid` 为该上层框架代理进程 pid（即 IPC 直接对端），`caller_process_name` 为该代理进程名（`/proc/<pid>/cmdline` 解析）。
- **AC-1.5:** （异常-调用方 processname 为空）Given 调用方进程已退出（`/proc/<caller_pid>/cmdline` 不可读）When 写入退出原因记录 Then `caller_pid` 仍准确记录，`caller_process_name` 字段为空字符串（兜底），不阻塞终止流程、不新增错误码，记录写入成功。
- **AC-1.6:** （异常-调用方解析失败）Given 调用方进程已退出或 processname 解析返回失败（非 `ERR_OK`）When 写入退出原因记录 Then 终止流程照常完成并返回原有错误码，记录中 caller 字段置空，不新增错误码。
- **AC-1.7:** （权限边界-沿用既有）Given 非 SA kill 权限、非 shell 调用方（且 `GetCallingUid() != uid`）调用 `KillAppWithReason`/`KillBundleWithReason`/`ForceExitApp` When 权限校验 Then 返回 `ERR_PERMISSION_DENIED`（通用公共错误码，值 = `BASE_ERR_OFFSET + EPERM` = 1，非 `CHECK_PERMISSION_FAILED`；仅 `RecordAppWithReasonByUserId` 对非 foundation 调用方返回 `CHECK_PERMISSION_FAILED`(2097177)；行为不因 caller 记录而变）。
- **AC-1.8:** （向前兼容-旧记录）Given 升级前已存在的旧退出原因记录（KV 存储中无 `caller_pid`/`caller_process_name` key）When 通过 `GetAppExitReason` 读取 Then 读取侧按空/0 兜底，不报错、不破坏既有查询。
- **AC-1.9:** （读-改-写不丢失）Given 一条已含 `caller_pid`/`caller_process_name` 的退出原因记录 When `GetAppExitReason` 命中 ability 并经 `UpdateAppExitReason` 回写记录 Then 回写后的记录中 `caller_pid`/`caller_process_name` 与原值一致，不被覆盖为空/0。

## 业务规则

> 业务规则是约束声明（非验收标准）。

| 规则 ID | 规则描述 | 约束条件 | 关联 AC |
|---------|----------|----------|---------|
| BR-1 | 调用方 pid 在服务侧通过 `IPCSkeleton::GetCallingPid()` 自动获取，进程名经 `/proc/<pid>/cmdline` 解析；不修改 `ExitReason`/`ExitReasonCompability` 的 Parcel 序列化字段 | 调用方信息不进入 IPC payload；随 `RecordExitReasonParams`/helper 内部传递；捕获必须在任何 `IN_PROCESS_CALL` 之前 | AC-1.1~1.4 |
| BR-2 | caller 信息记录为 best-effort：解析失败不阻塞终止流程，不新增错误码 | 失败时 caller 字段置空，终止返回原有错误码 | AC-1.5、1.6 |
| BR-3 | `caller_pid` 记录的是 IPC 直接调用方（直连 AMS 的对端进程 pid） | 嵌套 IPC 场景下不追溯原始发起方（design 已收敛为非目标）；内部调用点归因：`UninstallAppInner`(:9418) 直接调用记录为 BMS 进程；`RemoveUserId`(:10891/:10959) 经 `IN_PROCESS_CALL` 调用记录为 AMS 自身 | AC-1.1、1.3、1.4 |
| BR-4 | 持久化 JSON 新增 `caller_pid`、`caller_process_name` 两个 key，向前兼容 | 旧记录无此 key 时读取侧按空/0 兜底；不删改既有 key | AC-1.8 |
| BR-5 | caller 字段在读-改-写路径必须 round-trip | `GetAppExitReason` 命中 ability 经 `UpdateAppExitReason` 回写时透传 caller 字段，不得覆盖为空 | AC-1.9 |

## 异常与边界规则

> 超时操作必须给出精确阈值；跨进程字段必须给出精确限制。

| 编号 | 场景 | 触发条件 | 系统行为 | 关联 AC |
|------|------|----------|----------|---------|
| EX-1 | 调用方 processname 解析失败 | 调用方进程已退出（`/proc/<pid>/cmdline` 不可读）或读取失败 | `caller_process_name` 置空字符串，`caller_pid` 仍记录，记录写入继续 | AC-1.5、1.6 |
| EX-2 | 嵌套 IPC（SA-A→SA-B→AMS） | 多跳跨进程调用 | `caller_pid` 取 IPC 直接对端（SA-B）pid，不回溯 SA-A | AC-1.1、1.4 |
| EX-3 | 非法 exitReason | exitReason.reason 越界（< REASON_MIN / > REASON_MAX）或 killId 非法 | 沿用既有 `RecordInvalidKillId` 路径返回 `ERR_INVALID_VALUE`；无效记录本期不附加 caller 信息（`RecordInvalidKillId` 签名不变，调用方诊断留待后续） | AC-1.7 |
| EX-4 | processname 长度上限 | `/proc/<pid>/cmdline` 解析结果 ≤ 256 字节（沿用既有 `process_name` 限制） | 超长截断或置空，不阻塞 | AC-1.2 |

## 错误码定义

> 本次特性不新增错误码：caller 记录为 best-effort，沿用既有终止接口错误码。下表为既有相关错误码（`2097xxx` 为 AAFwk 内部码基线 2097152 起；`1`/`22` 为 c_utils 通用公共码；均非公共 160xxxxx）。

| 错误码 ID | 错误码值 | 含义 | 关联 AC |
|-----------|----------|------|---------|
| ERR_PERMISSION_DENIED | 1（`BASE_ERR_OFFSET + EPERM`，c_utils 通用公共码） | `KillAppWithReason`/`KillBundleWithReason`/`ForceExitApp`/`RecordAppWithReason` 权限校验失败（非 SA kill 权限且非 shell 且 uid 不匹配）。注意：**不是** `CHECK_PERMISSION_FAILED` 的别名 | AC-1.7 |
| CHECK_PERMISSION_FAILED | 2097177 | 仅 `RecordAppWithReasonByUserId` 对非 foundation 调用方返回 | AC-1.7 |
| ERR_INVALID_VALUE | 22（`BASE_ERR_OFFSET + EINVAL`，c_utils 通用公共码） | exitReason 非法 / processInfo 无效 | AC-1.6、1.7 |
| KILL_PROCESS_FAILED | 2097174 | Kill 进程失败 | AC-1.6 |
| GET_BUNDLE_INFO_FAILED | 2097183 | KillBundleWithReason 解析 bundle 信息失败 | AC-1.6 |
| KILL_PROCESS_KEEP_ALIVE | 2097184 | 保活进程在内存充足时不可杀 | AC-1.6 |
| ERR_KILL_APP_WHILE_FOREGROUND | 2097339 | 前台应用不可终止 | AC-1.6 |
| ERR_KILL_APP_WHILE_STARTING | 2097331 | 启动中应用不可终止 | AC-1.6 |
| ERR_GET_ACTIVE_ABILITY_LIST_EMPTY | 2097222 | 记录退出原因时活跃 ability 列表为空 | AC-1.6 |
| ERR_NULL_APP_EXIT_REASON_HELPER | 2097288 | AppExitReasonHelper 为空 | AC-1.6 |

## API 规格定义

不涉及。理由：本次为内部维测接口扩展，调用方信息在服务侧通过 `IPCSkeleton` 上下文自动获取并写入既有退出原因持久化记录，不变更 `ExitReason`/`ExitReasonCompability` 结构与 IPC 序列化、不新增/修改对外公共或系统 API。`API/SDK` 维度在 proposal 中判定为"否"。

## 接口变更分析

> 开放级别取值：Public / System / Inner。本次不新增/不变更对外接口签名；内部接口（helper/data manager）扩展内部参数以透传/读回 caller 信息。

### 新增接口

| 接口名称 | 开放级别 | 参数概要 | 返回值 | 错误码 | 关联 AC |
|----------|----------|----------|--------|--------|---------|
| 无 | — | — | — | — | — |

### 变更/废弃接口

| 接口名称 | 开放级别 | 变更类型 | 影响场景 | 迁移指引 | 关联 AC |
|----------|----------|----------|----------|----------|---------|
| `AbilityManagerService::KillAppWithReason` | Inner | 行为扩展（不变签名） | 退出原因记录新增 caller 字段 | 无需迁移 | AC-1.1 |
| `AbilityManagerService::KillBundleWithReason` | Inner | 行为扩展（不变签名） | 退出原因记录新增 caller 字段 | 无需迁移 | AC-1.2 |
| `AbilityManagerService::ForceExitApp` | Inner | 行为扩展（不变签名；入口自行捕获并经内部重载透传） | 退出原因记录新增 caller 字段 | 无需迁移 | AC-1.3 |
| `AbilityManagerService::RecordAppWithReason` | Inner | 行为扩展（不变签名；新增带 `CallerInfo` 的内部重载） | 退出原因记录新增 caller 字段 | 无需迁移 | AC-1.4 |
| `AppExitReasonDataManager::SetAppExitReason` / `UpdateAppExitReason` / `ConvertAppExitReasonInfoToValue` | Inner | 持久化格式扩展（新增 JSON key；签名追加默认 `CallerInfo` 参数） | 记录新增 `caller_pid`/`caller_process_name` | 旧记录向前兼容（读取兜底） | AC-1.1~1.5、1.8 |
| `AppExitReasonDataManager::GetAppExitReason` | Inner | 内部签名扩展（新增 caller 出参，供读-改-写 round-trip） | `GetAppExitReason`→`UpdateAppExitReason` 回写不丢失 caller | 仓内调用点同步更新 | AC-1.9 |

## 兼容性声明

- **已有 API 行为变更:** 否（终止接口签名不变；仅扩展持久化记录内容，对外可观察的接口返回值/错误码不变）
- **配置文件格式变更:** 否
- **数据存储格式变更:** 是（app_exit_reason KV 存储的 JSON value 新增 `caller_pid`、`caller_process_name` 两个 key；向前兼容：读取侧对缺失 key 按空/0 兜底，不破坏旧记录读取）

## 验证映射

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1.1 | BR-1、BR-3 | UT：mock SA-X 进程调用 `KillAppWithReason`，断言 `AppExitReasonDataManager` 写入的 JSON 含 `caller_pid`==SA-X pid | app_exit_reason_data_manager_test / ability_manager_service 单测 |
| AC-1.2 | BR-1 | UT：调用 `KillBundleWithReason`，断言记录含 `caller_pid` 与 `caller_process_name`（`/proc/<pid>/cmdline` 解析） | app_exit_reason_data_manager_test |
| AC-1.3 | BR-1 | UT：调用 `ForceExitApp`，断言记录含 `caller_pid` 且等于 `ForceExitApp` 的直接调用方（而非 AMS 自身 pid） | ability_manager_service 单测 |
| AC-1.4 | BR-1、BR-3 | UT：经 `AbilityManagerClient` 代理调用 `RecordAppWithReason`，断言 `caller_pid` 为代理进程 pid | ability_manager_client_test |
| AC-1.5 | BR-2、EX-1 | UT：mock 调用方进程已退出（`/proc/<pid>/cmdline` 不可读），断言 `caller_process_name` 为空、`caller_pid` 准确、写入成功 | app_exit_reason_data_manager_test |
| AC-1.6 | BR-2、EX-1 | UT：mock processname 解析返回非 `ERR_OK`，断言终止返回原有错误码、caller 字段置空 | ability_manager_service 单测 |
| AC-1.7 | EX-3 | UT：无权限调用，断言 `Kill*`/`ForceExitApp` 返回 `ERR_PERMISSION_DENIED`(1)、`RecordAppWithReasonByUserId` 返回 `CHECK_PERMISSION_FAILED`(2097177) | ability_manager_service 单测 |
| AC-1.8 | BR-4 | UT：读取无 caller key 的旧 JSON value，断言按空/0 兜底、不报错 | app_exit_reason_data_manager_test |
| AC-1.9 | BR-5 | UT：写入含 caller 的记录 → `GetAppExitReason` 命中 ability 触发 `UpdateAppExitReason` 回写 → 再读，断言 caller 字段不变 | app_exit_reason_data_manager_test |

## 测试设计提示

> 每条 AC 至少给出一个测试入口；Red 条件说明实现前为何失败。

| AC | 测试类型 | 测试文件 | 测试名称 | 输入/触发 | 期望输出/错误 | Red 条件 |
|----|----------|----------|----------|-----------|---------------|----------|
| AC-1.1 | UT | ability_manager_service_third_test | RecordCallerPid_OnKillAppWithReason | SA-X 调 `KillAppWithReason` | 记录 JSON `caller_pid`==SA-X pid | 实现前 `ConvertAppExitReasonInfoToValue` 不写 `caller_pid`，断言缺失 key 失败 |
| AC-1.2 | UT | ability_manager_service_third_test | RecordCallerName_OnKillBundleWithReason | 调 `KillBundleWithReason` | 记录含 `caller_pid`+`caller_process_name`（cmdline 解析） | 实现前无 caller_process_name key |
| AC-1.3 | UT | ability_manager_service_third_test | RecordCaller_OnForceExitApp | 调 `ForceExitApp` | `caller_pid`==直接调用方（非 AMS 自身） | 实现前 `ForceExitApp` 未采集；若在委托链内采集将得到 AMS pid 而失败 |
| AC-1.4 | UT | ability_manager_client_test | RecordCaller_OnRecordAppWithReasonViaProxy | 经 client 代理调 `RecordAppWithReason` | `caller_pid`==代理进程 pid | 实现前 C 接口路径未透传 caller |
| AC-1.5 | UT | app_exit_reason_helper_test | CallerProcessNameEmpty_ProcessExited | mock `/proc/<pid>/cmdline` 不可读 | `caller_process_name` 空、`caller_pid` 准确、写入 OK | 实现前进程已退出路径未兜底 |
| AC-1.6 | UT | ability_manager_service_third_test | CallerResolveFailure_NotBlockKill | mock processname 解析返回非 `ERR_OK` | 终止返回原错误码、caller 空 | 实现前失败未兜底会中断主流程 |
| AC-1.7 | UT | ability_manager_service_third_test | NonPrivilegedCaller_PermissionDenied | 无权限调 `KillAppWithReason` | 返回 `ERR_PERMISSION_DENIED`(1) | 既有用例（回归） |
| AC-1.8 | UT | app_exit_reason_data_manager_test | ReadOldRecord_NoCallerKeyFallback | 读取无 caller key 的旧 value | 兜底空/0、不报错 | 实现前读取侧未兼容缺失 key |
| AC-1.9 | UT | app_exit_reason_data_manager_test | UpdateRecord_CallerRoundTrip | 写含 caller → `GetAppExitReason` 命中回写 → 再读 | caller 字段不变 | 实现前 `UpdateAppExitReason` 以默认空 `CallerInfo` 回写，覆盖丢失 |

> AC → 实现文件 + Task + 验证状态映射见 `execution-plan.md`「AC 到 Task 追溯」+「代码范围映射」。
