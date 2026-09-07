# Spec Compliance Review

- 变更：issue-16150-record-caller（终止/记录接口调用方归因信息记录）
- 评审日期：2026-09-05
- 评审基线：spec.md（9 AC）+ execution-plan.md（4 Task）；实现位于工作区（未提交）
- 评审约束：无编译环境，本评审基于静态代码检视 + diff 核对，运行验证证据为 Blocked 状态

## Verdict

- [x] Approved
- [ ] Needs Changes
- [ ] Blocked

> 附条件：所有 AC 的实现与测试代码均已落库且静态核对一致；但 AC 运行验证（编译 + UT 执行 + AC-1.9 设备抽样）因无编译环境全部 Blocked。归档（Gate-Final）前必须在编译环境就绪后补运行证据并将追溯表 Blocked→Pass 回填。

## 需求覆盖

| AC | Covered? | Evidence | Gap |
|----|----------|----------|-----|
| AC-1.1 | 是（代码+测试落库，运行 Blocked） | 采集 ability_manager_service.cpp:3856 → 透传 :3889；测试 KillAppWithReason_RecordsCallerInfo_0100（thirteenth_test）；持久化往返 app_exit_reason_data_manager_test 0100 | UT 执行待编译环境 |
| AC-1.2 | 是（代码+测试落库，运行 Blocked） | 采集 :3951 → 透传 :3973；测试 KillBundleWithReason_RecordsCallerInfo_0200 | UT 层为权限拒绝等价路径断言（目标缺 bundle manager mock）；bundle 快乐路径记录链由 TASK-1/2 链路用例承担，端到端由 AC-1.9 设备抽样兜底 |
| AC-1.3 | 是（代码+测试落库，运行 Blocked） | 采集 :3988 → 透传 :3995；测试 RecordAppWithReason_RecordsCallerInfo_0300（含 callingUid==uid 自记录）+ helper 0200/0300 | UT 执行待编译环境 |
| AC-1.4 | 是（代码+测试落库，运行 Blocked） | CaptureExitCallerInfo 容错（pid≤0 早退、name 空仅 WARN）；测试 CaptureExitCallerInfo_0800 + KillAppWithReason_ProcessNameLookupFailed_0400 + data manager 默认值省略断言 | UT 执行待编译环境 |
| AC-1.5 | 是（代码+测试落库，运行 Blocked） | 采集置于权限检查之后（denied 路径不采集不记录）；测试 KillAppWithReason_PermissionDenied_NoRecord_0500（含 RecordAppWithReason denied） | UT 执行待编译环境 |
| AC-1.6 | 是（代码+测试落库，运行 Blocked） | FromValue `contains`+类型校验解析，缺失回退默认（callerPid=-1/name 空）；测试 GetAppExitReasonLegacyWithoutCallerFields_0200（手工构造旧格式 JSON 值） | UT 执行待编译环境 |
| AC-1.7 | 是（代码+测试落库，运行 Blocked） | 拦截路径（前台/启动期/保活）在 AddAppExitReason/AddBundleExitReason 之前返回，不产生记录；测试 KillAppWithReason_BlockedByProtection_NoRecord_0600 | UT 执行待编译环境 |
| AC-1.8 | 是（代码+测试落库，运行 Blocked） | 请求作用域同步采集、按值下传（无共享可变状态）；测试 RecordAppWithReason_ConcurrentCallers_0700 | UT 为顺序等价用例（IPC mock 为进程级全局状态）；真实并发归设备验证 |
| AC-1.9 | 代码已覆盖，验证 Blocked | 覆盖率指标依赖编译产物 + 设备抽样 | 无编译环境与设备，无法执行抽样；环境就绪后按 spec 验证映射执行 |

## 额外实现

| Extra Behavior | File | Risk | Required Action |
|----------------|------|------|-----------------|
| `GetAppExitReason` 新增 callerInfo 出参 + 读回重写路径（UpdateAppExitReason）贯通 callerInfo，防止 ability 清理重写时丢失调用方字段 | app_exit_reason_data_manager.h/.cpp | 低（默认参数 nullptr，既有 3 处调用方零改动；重写保持字段符合 BR-7 同生命周期不变量） | 无需额外动作；已由 data manager 0100 用例覆盖重写路径 |
| 直接观测用例 CaptureExitCallerInfo_0800 | thirteenth_test | 无（纯测试） | 无 |
| E-5 落地结论：RecordInvalidKillId 仅通知 AppMgrService、不写 KV，故非法原因路径无持久化记录、无需携带调用方信息 | app_exit_reason_helper.cpp:611-621（未修改，事实确认） | 无 | 已在 design 中声明，无需改 spec |

## 理解偏差

| Topic | Spec Says | Implementation Does | Required Action |
|-------|-----------|---------------------|-----------------|
| FromValue 出参形式 | spec 不约束（内部实现） | 计划的引用出参改为指针默认参数（`ExitCallerInfo *callerInfo = nullptr`），避免 2 个无关调用点未用变量 | 无（内部实现偏差，已记录于 execution-plan 执行回填偏差 #1） |
| AC-1.2 UT 可观测性 | 验证映射写"UT mock IPC 调用方 + 读回断言" | thirteenth 目标缺 bundle manager mock，快乐路径断言降级为权限拒绝等价 + 链路用例承担 | 已在执行回填偏差 #3 记录；如需 UT 级完整覆盖需扩展该目标 mock（涉及 BUILD.gn/include，超出本次范围，建议随环境就绪后评估） |

## 结论

9 条 AC 的实现代码与测试代码全部落库，静态核对与 spec 行为契约一致（采集时机、容错语义、权限/拦截路径不变、存量兼容、缺省值约定 -1/空）。无未记录的额外行为，无未解决的理解偏差。评审结论：**Approved（附条件）**——编译与 UT 运行、AC-1.9 设备抽样为 Blocked 遗留项，须在编译环境就绪后执行并回填证据，Gate-Final 方可通过。
