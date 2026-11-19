# 独立证伪记录 — task-rss-tiered-control（2026-11-17）

## 方法与边界

- 范围：`fdacaa726e1ef3b0a01b59d13822a512446a6e0f` 到实现提交 `2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99`，限计划内 appmgr 与聚焦测试文件。
- 证据：当前源码逐路径反例推导、spec AC-5.1～AC-5.6、两轮独立只读复核、最终聚焦构建结果。
- 限制：ARM 运行时未执行；实现前 Red 未执行；实现提交已创建。以下“修复成立”均指当前源码可达性被排除，不冒充运行时 Green。

## K-001：同三元组重复归因释放泄漏

- **中间轮结论**：曾确认。`toolRefs > 0` 早退使 `taskRefs` 不同步减少，重复移除后可能残留空工具任务。
- **当前复核**：`process_schedule.cpp:239-267` 每次有效移除均递减两个计数；工具计数归零仅移除工具，任务总引用归零才删除任务与索引。被擦除迭代器后续未使用。
- **反例检查**：同一 task/pid/tool 添加两次、移除两次，`process_schedule_test.cpp:337-374` 覆盖第一次剩余引用、第二次完全清理和最终 END。
- **裁决**：当前代码路径下证伪；不计入正式 Finding。

## K-002：无效 uid 使旧 pending 到期输出

- **中间轮结论**：曾确认。有效 END pending 中重新以 uid=-1 纳管，旧 pending 未取消且出口未校验。
- **当前复核**：`process_schedule.cpp:378-403` 无效 Apply/Restore 先调用 `DiscardPendingControl`；`BuildLatestEvent` 在 `417-420` 再次拒绝无效 uid；`resource_control_proxy.cpp:236-242` 出口最后拒绝，不更新已通知状态。
- **反例检查**：`ProcessSchedule_InvalidUidCancelsPending_0100` 覆盖 pending END 中无效重纳管、无效死亡及有效恢复；`UidFailureRecoveryMatrix_0100` 覆盖持续失败和死亡 uid 补偿。
- **裁决**：当前代码路径下证伪；不计入正式 Finding。

## K-003：死亡形成 D1→ADD→D2

- **中间轮结论**：曾确认。首次死亡提交在任务锁外，可与并发 ADD 交错，出现 END→START→END。
- **当前复核**：`task_manager_service.cpp:349-368` 在任务锁内设置 `dyingPids_` 后提交 D1，D2 后清除；`RegisterManagedProcessInner` 在 `756-761` 对窗口内 pid 返回既有 `ERR_INVALID_PID`。D2 后的新 ADD 仍可创建新生命周期。
- **反例检查**：`TaskManagerService_DyingPidRejectsRegistration_0100` 验证窗口内拒绝且无记录；既有死亡后重新归因路径保持可用。
- **裁决**：当前代码路径下证伪；不计入正式 Finding。

## K-004：测试 getter/Reset 与 timer 并发访问

- **中间轮结论**：曾确认。测试线程直接读代理 map，可能与到期写操作形成数据竞争。
- **当前复核**：`resource_control_proxy.cpp:72-91` 的同步辅助将测试操作投递到 `eventQueue_`；`266-365` 的 Reset、getter、setter 和历史读取均经该队列。`GetDebounceTaskForTest` 在队列内取句柄，测试线程仅对真实句柄 `Sync()` 后再 drain。
- **反例检查**：所有调用点均来自测试线程，没有队列内同步等待自身的路径；普通立即断言与延时到期断言分别使用队列屏障和真实 timer Sync，不使用 sleep 伪造时间。
- **裁决**：当前代码路径下证伪；不计入正式 Finding。

## K-005：pending down→final END 使用首次窗口旧快照

- **中间轮结论**：补充审查确认。首次降档后又最终移除时，若只保存首次 windowStart，END 会错误包含已不存在的任务和旧档位。
- **当前复核**：`resource_control_proxy.cpp:146-175` 每次 rearm 保存最新候选及新 epoch；`process_schedule.cpp:429-439` 保留该候选 previous，仅替换最新 current。
- **反例检查**：`ProcessSchedule_PendingDownThenEndUsesLatestPrevious_0100` 要求无中间 START，END previous 仅含最后剩余 MEDIUM 任务；连续降档也以最后一次降档前状态为 previous。
- **裁决**：当前代码路径下证伪；不计入正式 Finding。

## K-006：测试计数和已通知 map 长期增长

- **中间轮结论**：稳定性自查确认风险。若测试计数始终递增、已通知 map 不清理，生产进程 churn 可导致状态累积。
- **当前复核**：`resource_control_proxy.cpp:236-258` 仅在 `recordEventsForTest_` 开启时写计数/历史；最终 END 擦除 lastNotified 两项，其默认读取仍为 0/UNKNOWN；录制状态 Reset 清空，生产默认不保存历史。
- **反例检查**：死亡/最终移除后重新纳管仍创建新 START；事件计数仅在测试录制开启时用于精确断言。
- **裁决**：当前代码路径下证伪；不计入正式 Finding。

## 独立复核结论

独立只读上下文复核确认原四项问题均已修复，最新 previous 基线无新增高/中严重度问题。主审追加的 K-005/K-006 亦完成修复和源码证伪。当前没有未解决的正式代码 Finding；运行验证与提交追溯限制转交 SDD Verification Evidence。
