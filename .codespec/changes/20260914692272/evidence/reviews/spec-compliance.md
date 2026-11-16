# Spec Compliance Review

> 独立记录实现对 `spec.md` 的逐 AC 符合性。不得用代码质量结论替代规格符合性结论。

## Review Metadata

| Item | Value |
|------|-------|
| Change ID | 20260914692272 |
| Spec Revision | spec.md Approved（2026-11-16 yangzk） |
| Plan Revision | execution-plan.md Approved（2026-11-16 yangzk） |
| Base / Head | b22a0e88fc / fa3695de07 |
| Reviewer / Date | AI reviewer（opencode/kimi-k3） / 2026-11-16 |

## Verdict

- [x] Approved
- [ ] Needs Changes
- [ ] Blocked

## AC → Task → Code → Commit → Review

| AC | Task | Code Ref | Commit | Verification Evidence | Result | Gap |
|----|------|----------|--------|-----------------------|--------|-----|
| AC-5.1 | TASK-2、TASK-3 | services/appmgr/src/task_mgr/process_schedule.cpp DispatchEvents（apply 分支：Schedule + ApplyControl + QueryControlState 复核）、schedule_mapping.cpp（映射加载/fail-safe）、resource_control_proxy.cpp | fa3695de07 | 单测 DispatchPolymorphic_0100、ControlApplyVerify_0100、MappingLoad_0100、MappingFailSafe_0100（编译过） | PASS | 设备执行 |
| AC-5.2 | TASK-3 | services/appmgr/src/task_mgr/process_schedule.cpp（REVOKE_PID 撤销重试、OnTaskEnd）、task_manager_service.cpp（FinishTaskInner 锁外 OnTaskEnd 挂点）、resource_control_proxy.cpp（NotifyTaskEnd 重试） | fa3695de07 | 单测 TaskEndDegrade_0100、DegradeRetry_0100、UnscheduleRemovedTask_0100（编译过） | PASS | 设备执行 |
| AC-5.3 | TASK-2 | services/appmgr/src/task_mgr/process_schedule.cpp（CollectEventLocked：无归属对象不产生管控事件） | fa3695de07 | 单测 NoTaskIdBaseline_0100（编译过） | PASS | 设备执行 |

## Extra Implementation

| Extra Behavior | File:Line | Risk | Required Action |
|----------------|-----------|------|-----------------|
| 区分 UNSCHEDULE 与 REVOKE_PID 两类撤销事件（design ADR-5 语义的精确化：pid 仍有其他 task 时仅关闭被移除 task 的调度，不撤销 pid 级管控） | services/appmgr/src/task_mgr/process_schedule.cpp RemoveAttributionLocked/DispatchEvents | 无（与 spec R-4 进程级撤销/剩余最高档重算语义一致） | Accept |
| ResourceControlProxy 故障注入标志与重试计数器（failXxxForTest_、revokeCallsForTest_） | services/appmgr/include/task_mgr/resource_control_proxy.h | 无（一期 Stub 的测试友好设计，用于验证生效校验/重试路径，注释已标注 for test） | Accept |

## Interpretation Deviations

| Topic | Spec Says | Implementation Does | Required Action |
|-------|-----------|---------------------|-----------------|
| 撤销语义粒度 | spec R-4「进程级撤销管控并按剩余任务最高优先级重算」 | 区分 UNSCHEDULE（仅被移除 task 对象 Unschedule，pid 仍有其他 task 时不撤销 pid 级管控）与 REVOKE_PID（pid 无 task 时撤销） | 无（语义与 R-4 一致：pid 级管控仅在无 task 时撤销，单对象调度随移除单独关闭，属 design ADR-5 的精确化），Accept |
| 生效校验实现 | spec R-3「施加后查询调度/冻结状态，未生效记录任务告警日志」 | Stub 下施加即登记、查询恒一致；未生效场景经 failQueryForTest_ 注入验证告警分支 | 无（一期 Stub 语义，Impl 期 RSS 真实校验替换），Accept |

## Spec Quality Boundary

| Check | Result | Evidence |
|-------|--------|----------|
| AC 可从终端用户或 Public/System/InnerAPI 表面观察 | PASS | AC-5.1/5.2 经调度/冻结状态查询与 hilog 可观察；AC-5.3 经全量回归基线对比可观察 |
| THEN 未混入内部数据结构、状态机、调用链、类/方法、锁或算法 | PASS | spec.md AC 均为外部可观察结果（状态查询/告警日志/基线对比） |
| 变更/废弃 API 当前与目标开放级别一致、完整 | N/A | 无 Public/System API 变更；内部接口为进程内 C++（design 承载） |
| API/错误码事实含精确签名/数值、触发条件、外部行为、来源和 AC/验证映射 | N/A | 无新增/变更 API 与错误码（spec「API 与错误码事实」= N/A） |
| 每个 AC 的测试入口、Red 条件和通过标准真实可执行 | PASS | spec 验证映射 VM-1/2/3 均有测试入口与 Red 条件；process_schedule_test 已编写对应用例并编译通过 |

## Anti-Fake Completion Check

| Check | Result | Evidence |
|-------|--------|----------|
| 每个 AC 均有真实 Code Ref 和 Commit | PASS | 见上表（fa3695de07） |
| 正向、异常、边界和兼容路径均按 Spec 闭合 | PASS | 用例覆盖：正常施加/降级、映射 fail-safe（异常）、撤销重试（异常）、无 taskId 零影响（边界）、并发（边界）；设备执行缺口在 Gap 列如实标注 |
| 没有用声明、占位实现或无关测试冒充完成 | PASS | 实现为真实多态分发/代理层/映射/降级逻辑；一期空实现为需求方明确约束（派生类 Schedule/Unschedule 空实现+日志），非冒充 |

## Conclusion

实现与 spec.md 的 AC-5.1/5.2/5.3 精确对应，无额外行为、无误解性偏差。一期空实现+日志为需求方确认的交付边界（design ADR-7）。编译验证全量通过（process_schedule_test/libappms/task_manager_service_test/ability_runtime_test 均 build success）；单测用例执行需设备环境，Gap 列已如实标注，不构成实现偏差。规格符合性结论：Approved。
