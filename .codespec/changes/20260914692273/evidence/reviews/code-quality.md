# Code Quality Review

> 独立记录已通过 Spec Compliance 的实现质量。若 Spec Compliance 未通过，本审查不得给出最终 Approved。

## Review Metadata

| Item | Value |
|------|-------|
| Change ID | 20260914692273 |
| Base / Head | `a02a58e8` / `5dced55396` |
| Reviewed Scope | `feat/task-priority-judgment-v1.0`：实现提交 `5dced55396`（32 文件，1750 新增 / 190 删除）+ interface/sdk-js 跨仓 d.ts 工作区改动（2 文件） |
| Reviewer / Date | AI reviewer（opencode/kimi-k3） / 2026-11-05 |

## Verdict

- [x] Approved
- [ ] Needs Changes
- [ ] Blocked

## Plan Scope vs Actual Code Scope

| Task | Planned Files/Symbols | Actual Files/Symbols | Commit | Deviation | Result |
|------|-----------------------|----------------------|--------|-----------|--------|
| TASK-1 | interface_sdk-js api/@ohos.app.agent.taskManager.d.ts | 同上 + zh-cn 文档（跨仓，未提交） | 跨仓 | 无 | PASS |
| TASK-2 | task_priority_judge.*、system_load_provider.*；test/unittest/task_mgr/ | 同名源文件；测试落于既有 `test/unittest/task_manager_service_test/` target（仓内无 test/unittest/task_mgr 目录） | `5dced55396` | 测试目录沿用既有 target（plan 偏差已记录） | PASS |
| TASK-3 | IAppMgr 链 + NAPI/ANI/ETS + Service + mock | 同上；连带 4 个既有单测签名同步（接口变更必然连带，编译门禁） | `5dced55396` | 无 | PASS |
| TASK-4 | services/appmgr/src/process_schedule/ | 同上；app_mgr_service.cpp 追加 --psched 分发/usage | `5dced55396` | 无 | PASS |
| TASK-5 | task_mgr/ 判定器挂点 + process_schedule 重算 | 同上（RejudgeTasks/OnAbilityForegroundChanged/OnSystemLoadLevelChanged/RegisterScheduleListeners + task_schedule_observer.h） | `5dced55396` | 无 | PASS |
| TASK-6 | AppDeathRecipient 挂点 | 落于 `TaskManagerService::OnProcessDied`（既有死亡链汇聚点，更小侵入） | `5dced55396` | 挂点位置优化（plan 偏差已记录） | PASS |

## Findings

| Severity | File:Line | Finding | Impact | Required Fix |
|----------|-----------|---------|--------|--------------|
| 无 Blocker/Major | — | — | — | — |
| Minor | `process_schedule.cpp` `ApplyControlLocked`/`RevokeControlLocked` | RSS 施加/撤销为留空桩（仅日志），真实交付在 20260914692272 | 预期交付边界，非缺陷 | 无（属交付顺序确认项） |
| Minor | `task_manager_service.cpp` `RegisterScheduleListeners` 权限校验失败仅告警 | AppMS 进程内注册 observer 经权限校验；单测环境 mock 后兜底 MEDIUM（spec R-7） | 兜底语义符合 spec | 无 |
| Minor | `judgeMutex_`（std::mutex）与 `tasksMutex_`（ffrt::mutex）并存 | 锁顺序 tasksMutex_ → judgeMutex_ 单向下嵌套，判定器回调不回调业务层 | 无死锁风险 | 无 |
| Minor | ressched 监听回调线程不直接加业务锁（仅缓存档位） | 回调链路仅写 judge 缓存并触发重判（重判在调用线程取 tasksMutex_） | 符合并发模型 | 无 |
| Minor | 并发上限读取沿用 GetIntParameter + 默认兜底 | 沿用既有 parameter 先例（common.h TASK_TIMEOUT_PARAM 同款） | 无 | 无 |

## 架构与规范符合性

- 分层合规：判定器/归属在服务层（appmgr 🔴 高风险区），绑定层只做参数透传变更，无跨层下沉/上侵。
- PermissionVerification 不绕过：startTask 入口仍走 `IsSystemAppCall`（app_mgr_service.cpp）。
- IPC 兼容：仅 START_TASK(147) 消息体移除 priority；事务码与其余 IAppMgr 接口未动。
- 高风险目录最小侵入：appmgr 新增改动仅新增模块（process_schedule）与既有挂点最小接入（任务记录内 OnProcessDied 汇聚）。
- 生成物/序列化：无生成物手工合入；START_TASK 消息体同版本同步发布。
- 单元测试新增 13 用例（judge 6 + process_schedule 7）且目录/target 命名遵循仓内惯例。

**结论：** 代码质量符合 OpenHarmony 工程规范与本仓 AGENTS.md 约束，可合入。
