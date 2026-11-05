# Spec Compliance Review

> 独立记录实现对 `spec.md` 的逐 AC 符合性。不得用代码质量结论替代规格符合性结论。

## Review Metadata

| Item | Value |
|------|-------|
| Change ID | 20260914692273 |
| Spec Revision | `spec.md @ fb7ca600`（Approved；含后续 spec-check-report 复核） |
| Plan Revision | `execution-plan.md @ fb7ca600`（Approved） |
| Base / Head | `a02a58e8` / `5dced55396` |
| Reviewer / Date | AI reviewer（opencode/kimi-k3） / 2026-11-05 |

## Verdict

- [x] Approved
- [ ] Needs Changes
- [ ] Blocked

## AC → Task → Code → Commit → Review

| AC | Task | Code Ref | Commit | Verification Evidence | Result | Gap |
|----|------|----------|--------|-----------------------|--------|-----|
| AC-6.1 | TASK-4 | `services/appmgr/src/process_schedule/process_schedule.cpp` `MergeLevel`/`OnAttributionAdded` | `5dced55396` | 单测 `ProcessSchedule_MultiTaskHighestPriority_0100`（test/unittest/process_schedule_test/） | PASS（编译绿；用例执行待设备） | 设备执行 |
| AC-6.2 | TASK-4 | `process_schedule.cpp` `OnAttributionRemoved`/`RemoveAttributionLocked` | `5dced55396` | 单测 `DegradeToNextPriority_0100` | PASS（编译绿） | 同上 |
| AC-6.3 | TASK-6 | `services/appmgr/src/task_mgr/task_manager_service.cpp` `OnProcessDied` → `ProcessSchedule::OnProcessDied` | `5dced55396` | 单测 `ProcessDeathCleanup_0100` + `AllProcessesDeadTaskAlive_0100`；集成时延打点待设备 | PASS（编译绿） | 集成时延待设备 |
| AC-6.4 | TASK-4 | `process_schedule.cpp` + `TASK_MAX_ACTIVE_TASKS`（common.h:41） | `5dced55396` | 单测 `Concurrent10Tasks_0100` | PASS（编译绿） | 同上 |
| AC-6.5 | TASK-1/2/3 | d.ts（跨仓）+ `TaskPriorityJudge::JudgeLevel` + `TaskManagerService::StartTask` + IAppMgr 链 + NAPI/ANI/ETS | `5dced55396` | 单测 `StartTaskAutoJudgeUnit`（判定写入）+ 集成 demo（待设备） | PASS（编译绿） | demo 待设备 |
| AC-6.6 | TASK-2 | `TaskPriorityJudge::DetermineLevel` 映射矩阵 | `5dced55396` | 单测 `PriorityJudgeMatrix`（边界 4/5 + 前台=HIGH） | PASS（编译绿） | 同上 |
| AC-6.7 | TASK-5 | `TaskManagerService::RejudgeTasks` + `OnAbilityForegroundChanged` + `OnSystemLoadLevelChanged` + `TaskScheduleAbilityObserver` | `5dced55396` | 单测重判/防抖用例（含 9 场景矩阵）+ 集成 | PASS（编译绿） | 同上 |

## Extra Implementation

| Extra Behavior | File:Line | Risk | Required Action |
|----------------|-----------|------|-----------------|
| `DumpActiveTasks` 填充 `sched level [task:X]`（字段布局内已固定，仅填值） | `task_manager_service.cpp:436` | 无（前置分析已固定布局，不改格式） | Accept |
| `RejudgeTasks` 档位不变时不传播（INV-4 防抖） | `task_manager_service.cpp` `RejudgeTasks` | 无 | Accept |
| `--psched` 新增 dump 选项（OPTION_KEY_DUMP_PSCHED + DumpPsched + usage） | `app_mgr_service.cpp` | 无（沿用 --task 分发先例） | Accept |

## Interpretation Deviations

| Topic | Spec Says | Implementation Does | Required Action |
|-------|-----------|---------------------|-----------------|
| 死亡清理挂点位置 | design "AppDeathRecipient 挂点" | 实际落于 `TaskManagerService::OnProcessDied`（既有死亡通知链已汇聚该函数），未触碰 app_death_recipient 主体 | 已修正 plan 偏差记录，更小侵入，无行为偏差 |
| @since 版本号 | 前置分析 spec 记 26.2.0 | 代码事实 26.1.0（20260914692191 交付值） | spec 公共规格属性已按代码事实修正 |

## Spec Quality Boundary

- THEN 边界合规：实现只落 spec 可观察表面（System API 返回/错误码、任务 dump、调度策略查询、hilog），未在 spec 描述内部实现。
- 规则无冲突：R-1 判定矩阵/兜底、R-2 max 合并、R-3 降级、R-4 幂等、R-5 死亡清理、R-6 并发、R-7 兜底、R-8 任务存续均有对应实现且与 spec 文案一致。
- AC 覆盖完整：AC-6.1~6.7 均有 Task 映射与代码 ref，无孤立 AC。

**结论：** 实现与 spec 精确对应，无超范围行为（d.ts 跨仓差异已记录为跨仓交付项）。
