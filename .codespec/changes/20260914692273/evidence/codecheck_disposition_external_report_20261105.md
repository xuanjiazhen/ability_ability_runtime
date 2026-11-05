---

## 附：维护者处置回执（Round 2，2026-11-05，基于修复后工作区）

对外部检视报告 8 项发现的逐条处置：

| 报告项 | 处置 | 证据 |
|--------|------|------|
| LOG-01 | **已修复** | 前台计数改 pid 维度 + TERMINATED/END 计负（计数器 ≤0 保护，正常生命周期重复计负安全）；检视报告"死亡路径仅派发 TERMINATED"的前提经代码证实（app_mgr_service_inner.cpp:6525-6526），修复精确闭环该路径 |
| LOG-02 | **已修复** | ProcessSchedule 增加 (taskId,pid) 引用计数与豁免计数镜像（add 递增/remove 递减/归零移除）；死亡路径整键清除豁免；新增 process_schedule 与服务层双层回归用例 |
| API-01 | **可关闭（证据补齐）** | 【代码】interface_sdk-js `OpenHarmony-7.0-Release` 分支 ls-tree 查无 `api/@ohos.app.agent.taskManager.d.ts`——该 API 未随任何已发布 SDK 流出，变更发生在未发布版本内（@since 26.1.0），无在役调用方；PR !20914 描述已声明兼容性结论，跨仓同步 PR !37176 已发起 |
| LOG-03 | **已处置** | spec R-4 / design ADR-3 对齐：注明类型维度随 20260914692272 落地，当前单类型按 taskId+pid 计数、dump 以 CPU 占位标签渲染；代码注释同步标注 |
| LOG-04 | **已修复** | 仓根 `20260914692273/` 目录已从分支移除（本地备份 /tmp/opencode/backup，git 历史可溯）；SDD 文档统一归 `.codespec/changes/20260914692273/` |
| LOG-05 | **已修复** | `RegisterScheduleListeners` 移除 `std::call_once`：负载监听经 judge 幂等守卫随任务创建重试（覆盖 ressched 晚启动）；observer 注册失败在下一任务创建时重试 |
| LOG-06 | **已修复** | 6 处测试 BUILD.gn 新增行缩进统一为 4 空格 |
| LOG-07 | **已修复** | `OnSystemLoadLevelChanged` 未使用参数改为匿名参数并注释 |
| API-02 | **已修复** | NAPI 报错信息改为 "task priority is judged by the system now; remove the extra parameter" |

**验证**：`task_manager_service_test` / `process_schedule_test` / 全量 `ability_runtime_test`（rk3568, use_thin_lto=false）全部 build success（2026-11-05 22:52:14）。单测执行证据仍待设备环境。
