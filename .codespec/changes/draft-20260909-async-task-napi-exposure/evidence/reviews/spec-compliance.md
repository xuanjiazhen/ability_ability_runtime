# Spec Compliance Review

> 独立记录实现对 `spec.md` 的逐 AC 符合性。不得用代码质量结论替代规格符合性结论。

## Review Metadata

| Item | Value |
|------|-------|
| Change ID | draft-20260909-async-task-napi-exposure |
| Spec Revision | a8d715dea6ff |
| Plan Revision | bcd743677594 |
| Base / Head | 53cfaad1e7 / 本笔（squash 单笔） |
| Reviewer / Date | opencode (AI reviewer) / 2026-09-11 |

## Verdict

- [x] Approved
- [ ] Needs Changes
- [ ] Blocked

## AC → Task → Code → Commit → Review

| AC | Task | Code Ref | Commit | Verification Evidence | Result | Gap |
|----|------|----------|--------|-----------------------|--------|-----|
| AC-1 | TASK-1, TASK-3 | frameworks/native/appkit/app_startup/js_startup_task.cpp:20-23（getter 声明，无模块名引用）；frameworks/js/napi/app_startup/async_task_*/ 目录删除 | 本笔提交 | verification.md#静态负向与符号验证 | PASS | 设备侧 requireNapi 负向用例待真机（环境无设备） |
| AC-2 | TASK-1 | frameworks/js/napi/BUILD.gn:49-52（napi_packages 仅 4 公共模块）；frameworks/native/appkit/BUILD.gn:72-99（appkit_startup_task 无 relative_install_dir） | 本笔提交 | verification.md#构建产物与目标检查 | PASS | — |
| AC-3 | TASK-2, TASK-3 | frameworks/native/runtime/js_runtime.cpp:632-665（LoadInternalModuleByBuffer）；js_startup_task.cpp:180-220（两 Load 函数） | 本笔提交 | verification.md#编译与链接验证 | PASS | 单测/设备行为回归待 CI |
| AC-4 | TASK-3 | js_startup_task.cpp RunOnMainThread 路径零触碰（diff 范围核对） | 本笔提交 | verification.md#变更范围核对 | PASS | — |
| AC-5 | TASK-2, TASK-3 | js_runtime.cpp:636-660（失败分支返回 nullptr + TAG_LOGE）；js_startup_task.cpp:184-197（ERR_STARTUP_INTERNAL_ERROR + 既有 STARTUP_TASK_ERROR 上报保留） | 本笔提交 | verification.md#编译与链接验证 | PASS | — |
| AC-6 | TASK-1, TASK-2, TASK-3 | 全部受影响文件 | 本笔提交 | verification.md#编译与链接验证 | PASS | 全量 ninja 构建因 out 目录异机损坏不可用，待 CI |

## Extra Implementation

无额外：实现范围与 execution-plan 受影响文件全量清单（R1 修订版）逐项一致，无未声明行为。（R0→R1 重构为需求方评审要求的计划内修订，记录于 design.md 3.6 修订记录与 proposal 澄清 #2）

## Interpretation Deviations

无偏差：AC-3"行为不变"以"同一 JSNApi 执行入口 + 相同 es2abc --module 编译参数 + sendable/回调链逐行未改"实现等价论证，与 design.md D2 一致。

## Spec Quality Boundary

| Check | Result | Evidence |
|-------|--------|----------|
| AC 可从终端用户或 Public/System/InnerAPI 表面观察 | PASS | AC-1 返回值（requireNapi 结果）、AC-2 制品存在性、AC-5 错误码+事件均为可观测表面 |
| THEN 未混入内部数据结构、状态机、调用链、类/方法、锁或算法 | PASS | spec.md AC 仅含模块名/制品/错误码/事件等外部可观测项 |
| 变更/废弃 API 当前与目标开放级别一致、完整 | N/A | 本次无 API 变更（两模块从未声明） |
| API/错误码事实含精确签名/数值、触发条件、外部行为、来源和 AC/验证映射 | N/A | 无新增 API/错误码；沿用 ERR_STARTUP_INTERNAL_ERROR |
| 每个 AC 的测试入口、Red 条件和通过标准真实可执行 | PASS | 验证映射（spec.md）每项均有命令级入口；Red 条件=注册残留/制品存在/编译失败 |

## Anti-Fake Completion Check

| Check | Result | Evidence |
|-------|--------|----------|
| 每个 AC 均有真实 Code Ref 和 Commit | PASS | AC 表 6 行均含 file:line 与 SHA（git log 可溯） |
| 正向、异常、边界和兼容路径均按 Spec 闭合 | PASS | 正向=AC-3 等价论证+编译；异常=AC-5 错误分支；兼容=AC-4 零触碰；边界=AC-1 负向 |
| 没有用声明、占位实现或无关测试冒充完成 | PASS | 全部结论基于当次会话真实执行的命令输出（见 verification.md） |

## Conclusion

实现与 spec（AC-1~AC-6）、execution-plan（R1 修订版）一致，静态可验证项全部闭合。遗留设备/CI 侧运行时验证已显式记录于 Gap 列与 review.md 遗留项，不构成规格符合性缺口。可进入代码质量审查与最终验证。
