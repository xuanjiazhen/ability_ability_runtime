# Code Quality Review

- 变更：issue-16150-record-caller
- 评审日期：2026-09-05
- 评审范围：9 个修改文件（services/abilitymgr 6 + test/unittest 3），453+/32-，基于 `git diff` 静态检视
- 评审约束：无编译环境，未执行编译与静态分析工具，结论基于人工检视与模式一致性核对

## Verdict

- [x] Approved
- [ ] Needs Changes
- [ ] Blocked

## Strengths

1. **复用既有模式，无新抽象**：调用方采集复用 `IPCSkeleton::GetCallingPid()` 既有调用模式（参照 ability_connect_manager.cpp:3519）；JSON key 沿用 `JSON_KEY_*` 常量 + `contains`+类型校验解析模式；日志沿用 `TAG_LOG*` + `AAFwkTag::ABILITYMGR`。
2. **兼容性设计稳健**：全部签名扩展采用末位默认参数，既有调用方（含跨模块：mission_list_manager、ui_ability_lifecycle_manager、keep_alive mock、client mock）源码兼容零改动；`ExitReason`/`ExitReasonCompability` Parcelable 未触碰，IPC 序列化格式零变化。
3. **安全实践到位**：调用方身份取自内核/IPC 上下文不可伪造；进程名日志使用 `%{private}s`（pid 为 `%{public}d`）；采集置于权限检查之后（denied 路径零开销零记录）；无新增对外暴露面。
4. **容错与简洁**：pid≤0 早退、进程名反查失败仅 WARN 不重试不阻塞；无新增锁、无新增异步任务、无新增文件/组件/构建目标。
5. **测试先行且断言具体**：12 个新用例覆盖正向/容错/权限/拦截/缺省兼容/顺序等价，断言精确到错误码值（1/22/2097339）与字段值（-1/空串），并遵循各目标既有命名与风格（HWTEST_F + 编号后缀）。
6. **实现期主动修复数据保持缺陷**：发现并修复读回重写路径（UpdateAppExitReason）会丢失调用方字段的问题，符合 design「同存储同生命周期」不变量。

## Issues

| Severity | File:Line | Finding | Required Fix |
|----------|-----------|---------|--------------|
| Info | services/abilitymgr/src/ability_manager_service.cpp:3856/:3951/:3988 | 采集位于权限检查之后、但早于 helper 判空/原因校验/拦截检查；早期失败路径会多一次进程名反查（1 次低频 IPC） | 不要求修复：与 design D3「入口同步采集」一致，低频路径开销可忽略；如后续需要进一步优化可将采集点移至 Add*/RecordAppWithReason 调用前一行 |
| Info | services/abilitymgr/include/app_exit_reason_data_manager.h:32-35 | `ExitCallerInfo` 结构体无注释说明字段语义（-1 缺省约定） | 不要求修复（与同文件其余结构体风格一致）；建议合入前可选补充一行字段注释 |
| Info | test/unittest/ability_manager_service_thirteenth_test/ability_manager_service_thirteenth_test.cpp（0200/0500 用例） | 用例将 `permPermission_` 置 0 后未恢复默认值 1（与 suite 既有惯例「恢复为 0」一致，既有用例如 0100/0200/0300 亦恢复为 0） | 不要求修复：遵循 suite 既有惯例，无新增风险 |

## Recommendations

1. 编译环境就绪后：先跑 `app_exit_reason_data_manager_test`、`app_exit_reason_helper_fourth_test`、`ability_manager_service_thirteenth_test` 三个目标确认新增用例通过，再做全量 abilitymgr 单测回归。
2. 若后续需要 AC-1.2 的 UT 级完整覆盖，可为 thirteenth 目标补充 bundle manager mock（涉及 BUILD.gn，另行立项评估）。
3. `KillAppWithReason_ProcessNameLookupFailed_0400` 与 `KillAppWithReason_RecordsCallerInfo_0100` 结构高度相似，如后续维护成本敏感可考虑合并参数化；当前保持独立用例以维持 AC 追溯清晰。

## Assessment

变更规模小、边界清晰、模式一致、风险面收敛（低频维测路径、无新暴露面、无序列化变更）。静态检视未发现需要阻断的问题；3 条 Info 级观察均不要求修复。代码质量评审结论：**Approved**（编译级验证待环境就绪补充）。
