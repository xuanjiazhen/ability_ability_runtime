# Task 02 — 单元测试

> item: `upms-dynamic-unload` | depends on: task-01 | mode: 直接实现
> 规则来源：[spec.md](../spec.md) AC-1/2/4/5/6、[design.md](../design.md) §8。

## 目标
为卸载机制补 UT，覆盖安全关键路径与回归。

## 文件范围
- `test/unittest/uri_permission_impl_test/`（既有 stub_impl 单测套；按实际套名调整）
- 复用 mock：`test/new_test/mock/upms/uri_permission_mgr/`
- 不改生产代码（仅测试）。

## 用例清单

| 用例 | 覆盖 AC | 断言要点 |
|------|--------|---------|
| IsLedgerEmpty_四容器组合 | AC-1/2 | 4 容器分别非空→false；全空→true |
| RefreshUnloadTimer_空账本_arm | AC-1/2 | 空→`DelayUnloadTask` 挂载任务（验证 handler 有 task） |
| RefreshUnloadTimer_非空_cancel | AC-2/4 | 非空→任务被移除 |
| Grant路径_cancel穷尽 | AC-2/4 | 每个 Grant*/Active 调用后账本非空→无挂载卸载任务 |
| Revoke清空后_arm | AC-1/2 | 持权→清账→空→挂载卸载任务 |
| 持权不卸载 | AC-4 | 账本非空，`RefreshUnloadTimer` 不 arm |
| StopHandler清理 | AC-5 | `StopHandler` 后 handler_/runner_ 释放、无挂起任务 |
| OnStop_impl置空 | AC-5 | `SelfClean` 后 `impl_` 为空（间接验证重载建新账本） |

> 实现提示：卸载 lambda 调真实 `UnloadSystemAbility` 不利于 UT，可在测试中通过 mock SAMGR 或断言"handler 是否持有 `TASK_NAME` 任务"来间接验证 arm/cancel，避免触发真实卸载。

## 完成标准
- [ ] 上述用例实现并通过
- [ ] `run -t UT -tp uripermmgr` 全绿（含既有 `uri_permission_manager_test`/`uri_permission_impl_test`/`uri_permission_test`/`uri_perm_mgr_test`，AC-6 回归）
- [ ] 无既有用例因新机制而失败

## 风险点
- 真实卸载不可在 UT 触发——必须用间接断言（handler 任务存在性 / mock）。
- 13 处插桩若在 task-01 遗漏，"Grant路径_cancel穷尽"用例会暴露。
