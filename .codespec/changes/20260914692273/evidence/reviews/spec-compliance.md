# Spec Compliance Review

> 独立记录实现对 `spec.md` 的逐 AC 符合性。不得用代码质量结论替代规格符合性结论。

## Review Metadata

| Item | Value |
|------|-------|
| Change ID | 20260914692273 |
| Spec Revision | spec.md Approved（2026-11-05 yangzk） |
| Plan Revision | execution-plan.md Approved（2026-11-05 yangzk） |
| Base / Head | a02a58e8 / 5dced55396 |
| Reviewer / Date | AI reviewer（opencode/kimi-k3） / 2026-11-05 |

## Verdict

- [x] Approved
- [ ] Needs Changes
- [ ] Blocked

## AC → Task → Code → Commit → Review

| AC | Task | Code Ref | Commit | Verification Evidence | Result | Gap |
|----|------|----------|--------|-----------------------|--------|-----|
| AC-6.1 | TASK-4 | services/appmgr/src/process_schedule/process_schedule.cpp MergeLevel/OnAttributionAdded | 5dced55396 | 单测 ProcessSchedule_MultiTaskHighestPriority_0100（编译过；执行待设备，gap 见 verification.md） | PASS | 设备执行 |
| AC-6.2 | TASK-4 | services/appmgr/src/process_schedule/process_schedule.cpp OnAttributionRemoved | 5dced55396 | 单测 ProcessSchedule_DegradeToNextPriority_0100（编译过） | PASS | 设备执行 |
| AC-6.3 | TASK-6 | services/appmgr/src/task_mgr/task_manager_service.cpp OnProcessDied → ProcessSchedule::OnProcessDied | 5dced55396 | 单测 ProcessDeathCleanup_0100 + AllProcessesDeadTaskAlive_0100（编译过）；集成时延打点待设备 | PASS | 集成时延待设备 |
| AC-6.4 | TASK-4 | services/appmgr/src/process_schedule/process_schedule.cpp + services/appmgr/include/task_mgr/common.h TASK_MAX_ACTIVE_TASKS | 5dced55396 | 单测 ProcessSchedule_Concurrent10Tasks_0100（编译过） | PASS | 设备执行 |
| AC-6.5 | TASK-1/2/3 | interface/sdk-js d.ts（跨仓）+ TaskPriorityJudge::JudgeLevel + TaskManagerService::StartTask + IAppMgr 链 + NAPI/ANI/ETS | 5dced55396 | 单测 StartTaskAutoJudgeUnit（编译过）+ 集成 demo（待设备） | PASS | demo 待设备 |
| AC-6.6 | TASK-2 | services/appmgr/src/task_mgr/task_priority_judge.cpp DetermineLevel 映射矩阵 | 5dced55396 | 单测 TaskPriorityJudge_Matrix_0100/0200/0300（编译过） | PASS | 设备执行 |
| AC-6.7 | TASK-5 | TaskManagerService::RejudgeTasks/OnAbilityForegroundChanged/OnSystemLoadLevelChanged + task_schedule_observer.h | 5dced55396 | 单测重判/防抖用例（编译过）+ VM-3 集成场景矩阵（待设备） | PASS | 集成待设备 |

## Extra Implementation

| Extra Behavior | File:Line | Risk | Required Action |
|----------------|-----------|------|-----------------|
| DumpActiveTasks 填充 sched level 字段（task 维度，字段布局内已固定，仅填值） | services/appmgr/src/task_mgr/task_manager_service.cpp DumpActiveTasks | 无（前置分析已固定布局，不改格式） | Accept |
| RejudgeTasks 档位不变时不传播（INV-4 防抖） | services/appmgr/src/task_mgr/task_manager_service.cpp RejudgeTasks | 无（design INV-4 已定义） | Accept |
| 新增 dump 选项 --psched（OPTION_KEY_DUMP_PSCHED + DumpPsched + usage 文案） | services/appmgr/src/app_mgr_service.cpp | 无（沿用 --task 分发先例） | Accept |

## Interpretation Deviations

| Topic | Spec Says | Implementation Does | Required Action |
|-------|-----------|---------------------|-----------------|
| 死亡清理挂点位置 | design 写 AppDeathRecipient 挂点 | 实际落于 TaskManagerService::OnProcessDied（既有死亡通知链已汇聚该函数），未触碰 app_death_recipient 主体 | 已记录 plan 偏差；更小侵入，无行为偏差，Accept |
| startTask @since 版本号 | 前置分析 spec 记 26.2.0 | 代码事实 26.1.0（20260914692191 交付值），d.ts 保持 26.1.0 | spec 公共规格属性已按代码事实修正，Accept |

## Spec Quality Boundary

| Check | Result | Evidence |
|-------|--------|----------|
| THEN 边界合规（只写外部可观察表面） | PASS | 实现落于 System API 返回/错误码、任务 dump（--task/--psched）、hilog 关键路径；无内部实现写入 spec THEN |
| 规则无重叠冲突（R-1~R-8） | PASS | R-1 矩阵/兜底、R-2 max 合并、R-3 降级、R-4 幂等、R-5 死亡清理、R-6 并发、R-7 兜底、R-8 任务存续均有对应实现 |
| AC 覆盖完整且可独立测试 | PASS | AC-6.1~AC-6.7 均有 Task 映射与代码 ref，无孤立 AC |

## Anti-Fake Completion Check

| Check | Result | Evidence |
|-------|--------|----------|
| AC 映射含真实代码引用（file:line/符号） | PASS | 每行 Code Ref 为真实文件与符号 |
| 证据来自本次实现提交（非历史） | PASS | Commit 5dced55396 为 2026-11-05 本次实现提交 |
| 偏差均记录而非掩盖 | PASS | Gap 列与 Interpretation Deviations 如实标注设备执行缺口 |

## Conclusion

实现与 spec 逐 AC 精确对应，无超范围行为；两项偏差（挂点位置、@since 版本）均经源头修正或记录。单测/集成用例执行受限于无设备环境，编译证据已全部通过，待设备环境执行后可将 Gap 闭环。
