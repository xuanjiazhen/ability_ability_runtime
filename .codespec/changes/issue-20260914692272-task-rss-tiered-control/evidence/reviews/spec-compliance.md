# Spec Compliance Review

> 独立记录实现对 `spec.md` 的逐 AC 符合性。不得用代码质量结论替代规格符合性结论。

## Review Metadata

| Item | Value |
|------|-------|
| Change ID | issue-20260914692272-task-rss-tiered-control |
| Spec Revision | `b3910d099d653e97070519a0529af31b336e7bc21774a33bacbf65902a72564c` |
| Plan Revision | `5b47a8fa22ed5ab2a287de35547d8233d7ac04c64c5aa151609f5182f8179358` |
| Base / Head | `fdacaa726e1ef3b0a01b59d13822a512446a6e0f` / `WORKTREE（未提交）` |
| Reviewer / Date | Agent / 2026-11-17 |

## Verdict

- [ ] Approved
- [ ] Needs Changes
- [x] Blocked（实现前 Red、ARM 运行验证和实现 Commit 尚未闭合；静态实现映射无已知语义偏差）

## AC → Task → Code → Commit → Review

| AC | Task | Code Ref | Commit | Verification Evidence | Result | Gap |
|----|------|----------|--------|-----------------------|--------|-----|
| AC-5.1 | TASK-1 | `process_schedule.cpp:97-119,428-443`；`resource_control_proxy.cpp:47-69,236-258` | N/A（未提交） | `verification.md#execution-records`；`ControlledEventContext_0100`、`TriggerReasonMatrix_0100` | PASS（静态映射） | ARM 运行与 Commit 未闭合 |
| AC-5.2 | TASK-1 | `process_schedule.cpp:71-119,223-267,332-368` | N/A（未提交） | `ControlledTaskMatrix_0100`、`ControlledEndPreviousContext_0100` | PASS（静态映射） | 同上 |
| AC-5.3 | TASK-1 | `process_schedule.cpp:183-214,223-328`；`resource_control_proxy.cpp:236-258` | N/A（未提交） | `ControlledContextOnlyMatrix_0100`、`UncontrolledToolsNoEvent_0100` | PASS（静态映射） | 同上 |
| AC-5.4 | TASK-1 | `resource_control_proxy.cpp:85-143`；`process_schedule.cpp:151-180` | N/A（未提交） | `ImmediateStartAndDeathEnd_0100` | PASS（静态映射） | 同上 |
| AC-5.5 | TASK-1 | `resource_control_proxy.cpp:95-218`；`process_schedule.cpp:406-439` | N/A（未提交） | `DebounceDownAndEnd_0100`、`TriggerReasonMatrix_0100` | PASS（静态映射） | 同上 |
| AC-5.6 | TASK-1 | `resource_control_proxy.cpp:146-218`；`process_schedule.cpp:406-439` | N/A（未提交） | `DebounceLatestState_0100`、`PendingDownThenEndUsesLatestPrevious_0100` | PASS（静态映射） | 同上 |
| AC-5.7 | TASK-1 | `process_schedule.cpp:223-368`；`task_manager_service.cpp:349-368,476-500` | N/A（未提交） | `FinalStateAction_0100`、`SelfManagedDeath_0100`、`CreatorDeathCleanup_0100` | PASS（静态映射） | 同上 |
| AC-5.8 | TASK-1 | `resource_control_proxy.cpp:236-258`；构建无新增真实 RSS 调用 | N/A（未提交） | `ResourceControlProxy_StubOnly_0100`；`codecheck_report_*#分维度速览` | PASS（静态映射） | 同上 |
| AC-5.9 | TASK-1 | 既有 `StartTask/RegisterManagedProcess` 上限未改；`resource_control_proxy.cpp:95-218` | N/A（未提交） | `DebounceLatestState_0100`；既有 `StartTask_0700`、`RegisterManagedProcessQuota_0900` | PASS（静态映射） | 同上 |
| AC-5.10 | TASK-1 | `task_manager_service.cpp:47-69,575-627`；`process_schedule.cpp:309-328,378-439` | N/A（未提交） | `ManagedUidCacheAndRetry_0100`、`UidFailureRecoveryMatrix_0100`、`InvalidUidCancelsPending_0100` | PASS（静态映射） | 同上 |

## Extra Implementation

| Extra Behavior | File:Line | Risk | Required Action |
|----------------|-----------|------|-----------------|
| 无超出 Spec/Plan 的生产行为；测试历史、队列同步和 Reset 为既有测试支撑，生产默认不录制历史 | `resource_control_proxy.h:49-63`、`process_schedule.h:103-107` | 无额外业务语义；生产内存增长已由录制门控与 END 清理控制 | Accept |

## Interpretation Deviations

| Topic | Spec Says | Implementation Does | Required Action |
|-------|-----------|---------------------|-----------------|
| 无已识别语义偏差 | AC-5.1～AC-5.10 与 R-1～R-9 | 受管集合、max/主类型、立即/尾随动作、uid 异常和死亡保序一致 | 无 |

## Spec Quality Boundary

| Check | Result | Evidence |
|-------|--------|----------|
| AC 可从终端用户或 Public/System/InnerAPI 表面观察 | PASS | InnerAPI 代理事件、事件计数、日志与既有任务/归因结果 |
| THEN 未混入内部数据结构、状态机、调用链、类/方法、锁或算法 | PASS | spec.md AC 保持外部可观察行为描述 |
| 变更/废弃 API 当前与目标开放级别一致、完整 | N/A | 无 Public/System API；内部枚举只追加且既有值不变 |
| API/错误码事实含精确签名/数值、触发条件、外部行为、来源和 AC/验证映射 | PASS/N/A | `VIRTUAL_SCREEN_ABILITY=5`；无新增错误码、无 Parcel 变化 |
| 每个 AC 的测试入口、Red 条件和通过标准真实可执行 | FAIL（Red 未执行） | 测试目标已编译，但实现前 Red 没有 fresh 输出；ARM 运行环境不可用 |

## Anti-Fake Completion Check

| Check | Result | Evidence |
|-------|--------|----------|
| 每个 AC 均有真实 Code Ref 和 Commit | FAIL（Commit 未创建） | Code Ref 完整；实现为用户未授权提交的 dirty worktree |
| 正向、异常、边界和兼容路径均按 Spec 闭合 | FAIL（运行未闭合） | 静态路径与测试入口齐全，ARM 无 qemu、hdc `E002110` |
| 没有用声明、占位实现或无关测试冒充完成 | PASS（静态真实性） | 真实代理实现、13 个直接聚合目标补齐、无真实 RSS 调用；不声称运行时 Green |

## Conclusion

当前源码与 Approved spec 的静态语义映射一致，没有未确认的实现偏差或计划外生产行为。因实现前 Red、设备运行断言和实现 Commit 缺失，本审查不能给 Approved；不得将 `PASS（静态映射）` 解释为 AC 已全部运行通过。后续需以可用设备执行聚焦测试并保留 fresh 输出，再按用户授权创建实现提交后复核追溯链。
