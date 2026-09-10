# 执行计划

> 将 Approved Spec 拆成可独立执行、可验证、可审查的 Task。每个 Task 用稳定 ID 和章节锚点引用上游事实（`AC-*`/`R-*`/`BF-*`/`ADR-*`/`STATE-*`/`INV-*`/`VM-*`），不复制 Spec/Design 正文。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-20260626000935 |
| 关联 Feature/Bug | FEAT-20260626000935（execCmd 工具命令模式） |
| 关联文档 | `proposal.md`（Approved）/ `spec.md` / `design.md` |
| 复杂度 | 标准 |
| 状态 | Draft |
| Owner | CLI 工具框架负责人（团队级） |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `.codespec/changes/issue-16082-execcmd-clitool/proposal.md` | Approved（2026-09-09，批准人 zhoulanzhi，baseline_digest `3db9de16…a0e9`） |
| Design | `.codespec/changes/issue-16082-execcmd-clitool/design.md` | Draft（本文以其 ADR-1..ADR-8、STATE-1..5、INV-1..7、TASK-1..TASK-8 为上游事实；设计评审通过后本文不需修订） |
| Spec | `.codespec/changes/issue-16082-execcmd-clitool/spec.md` | Draft（AC-1.1..AC-1.10、R-1..R-17、BF-1..BF-6、VM-1..VM-13 为本文追溯锚点） |

## 执行原则

- **Spec 权威：** 实现与 AC、错误码、兼容性声明冲突时，停止并修订 spec/design/plan，不能在代码里自行改语义。已固化的两处语义结论不得在实现期翻案：`workDir`/`env`/`policy` 在工具模式不接入（spec R-16 / design ADR-8）、`timeout` 双默认路径保持现状（spec R-17 / design 需求基线）。
- **测试/证据先行：** 每个实现 Task（TASK-1..TASK-4、TASK-8）在其 Files 的测试文件内先写**最小失败用例**取得 Red 证据，再做最小实现取得 Green 证据；TASK-5/6/7 负责把 `spec.md#验证映射` 中该层的其余用例补齐至全绿（含 VM-1/VM-8/VM-11 三条防回退守护用例的回归确认）。
- **任务小型化：** 一个 Task 只覆盖一个独立闭环；跨接口、事件链、状态生命周期或构建边界的工作已拆分（NAPI / IPC 数据层 / 解析 / 服务端分流与会话 / 三层测试 / 跨仓声明）。
- **文件边界：** 只能修改 Task 的 Files 与「允许修改」范围；需要额外文件时先修订 Plan。
- **状态所有权唯一：** STATE-1..STATE-5 的 owner、key/index、创建、读取、更新、清理触发以 `design.md#状态归属与不变量` 为唯一事实源，本文只在 Task 内引用。
- **证据回填：** Task 完成当次会话回填追溯状态、实际文件、Commit、Review Evidence 和 Actual Result，不跨会话补证。
- **反伪完成：** 只补声明、只写存储、只覆盖 happy path、只跑无关测试或只填写 PASS 文本，都不能替代 AC 闭环。
- **生成文件：** 本计划**不修改** `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl`（`ExecCmdParam` 已声明为 `sequenceable`，字段追加不触及接口签名），故无 proxy/stub 生成物需要重新生成；若实现期确需改 IDL，必须停止并回 `design.md#api-签名kit-与权限` 修订后重跑生成命令（详见「生成文件声明」）。

## 受影响文件全量清单

> 对照 `design.md#调用链层级分析`，逐层列出具体文件。

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| interface_sdk-js | ArkTS 声明层 | `api/@ohos.app.cli.cliManager.d.ts` | 修改 | `ExecCmdOptions` 新增 `isShellCommand?`/`challenge?`；`execCmd` 的 `@throws` 补 401 与 35600030 |
| interface_sdk-js | ArkTS 声明层 | `zh-cn/api/@ohos.app.cli.cliManager.d.ts` | 修改 | 中文声明同步（属性、默认值、`@throws`） |
| ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager_utils.cpp` | 修改 | `UnwrapExecCmdOptions` 内按既有「属性存在才解、解失败抛 401」模式接入两个新属性 |
| ability_runtime | 客户端/IPC 数据层 | `cli_tool_framework/interfaces/cli_tool/include/exec_cmd_param.h` | 修改 | `ExecCmdParam` 尾部新增 `isShellCommand = true`、`challenge`；新增静态工具名提取函数声明 |
| ability_runtime | 客户端/IPC 数据层 | `cli_tool_framework/interfaces/cli_tool/src/exec_cmd_param.cpp` | 修改 | `Marshalling` 尾部写入两字段；`Unmarshalling` 尾部读取，失败置默认值并成功返回；工具名提取实现 |
| ability_runtime | 客户端/IPC 数据层 | `cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp` | 修改 | `ExecCmd` 的 eventKey 按模式取值；工具模式下空白命令前置拒绝（不发 IPC） |
| ability_runtime | 服务端工具命令处理层 | `cli_tool_framework/services/climgr/include/tool_util.h` | 修改 | 新增命令解析静态函数声明 |
| ability_runtime | 服务端工具命令处理层 | `cli_tool_framework/services/climgr/src/tool_util.cpp` | 修改 | 命令解析实现：分词（单引号）→ 工具名/子命令 → 标志解析 → schema 类型恢复 |
| ability_runtime | 服务端入口层 + 工具命令处理层 | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 修改 | 新增 caller 上下文聚合结构与两个私有方法声明 |
| ability_runtime | 服务端入口层 + 工具命令处理层 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 修改 | `ExecCmd` 增加分流点；工具名白名单、工具查表、解析调用、复用 `ValidateAndPrepareTool`、会话建立与反向回滚、失败点 DFX 上报 |
| ability_runtime | 测试（解析层） | `cli_tool_framework/test/unittest/tool_util_test/tool_util_test.cpp` | 修改 | 新增 `ParseToolCommand_*` 与 `GenerateSandboxConfig_NoCallerWorkDir_0100` 用例 |
| ability_runtime | 测试（服务层） | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/cli_tool_mgr_service_test.cpp` | 修改 | 新增 `ExecCmd_ToolMode_*`、`ExecCmd_DefaultShellDispatch_0100`、`ExecCmd_ExplicitShellMode_0100` 用例 |
| ability_runtime | 测试（客户端层） | `cli_tool_framework/test/unittest/cli_tool_mgr_client_test/cli_tool_mgr_client_test.cpp` | 修改 | 新增工具模式空白命令前置拒绝与 eventKey 路由用例 |
| ability_runtime | 测试（序列化层） | `cli_tool_framework/test/unittest/exec_cmd_param_test/exec_cmd_param_test.cpp` | 修改 | 新增尾部字段往返、旧客户端缺省回退、工具名提取边界用例 |
| ability_runtime | 既有能力层（复用，不改） | `cli_tool_framework/services/climgr/src/process_manager.cpp`、`event_dispatcher.cpp`、`cli_tool_data_manager.cpp`、`services/common/src/cli_event_report.cpp`、`ccm_util.cpp` | 不修改 | 子进程/事件/工具信息/DFX 上报/并发上限均按 `design.md#既有模式复用` 直接复用 |
| ability_runtime | 构建 | `cli_tool_framework/**/BUILD.gn`、`bundle.json` | 不修改 | 无新增源文件与依赖（`design.md#构建系统影响`）；若实现期新增源文件，必须先回补该节并修订本清单 |

**检查项：**
- [x] design.md 调用链每一层都有对应文件列出（声明层 / NAPI / 客户端-IPC / 服务端入口 / 工具命令处理 / 既有能力层 / 测试 / 构建）
- [x] 每个文件修改类型和职责说明明确
- [x] 无映射行的层意味着设计到实现的翻译缺失 —— 无缺失层；「沙箱执行层」由既有 `ProcessManager` 承接，落在「既有能力层（复用，不改）」行

## 生成文件声明

| 生成物 | 生成源 | 生成命令 | 本计划是否触及 |
|--------|--------|----------|----------------|
| `ICliToolManager` proxy/stub（IDL 生成） | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` | 部件构建时由 IDL 生成步骤产出（`./build.sh --product-name rk3568 --build-target ability_runtime`） | 否：接口签名与 `sequenceable` 声明均不变（`design.md#api-签名kit-与权限`「变更/废弃 API」），仅 `ExecCmdParam` 内部字段追加，无需重新生成 |
| ArkTS `.d.ts` | 手写声明（非生成物） | 无生成命令 | 是：TASK-8 直接修改中英双份声明文件，须保持标注组一致 |

## AC 到 Task 追溯

> 本表是 AC→Task 的**单一事实源**。计划阶段填写 AC/Task/计划代码范围；实施后当次会话回填实际 Code Ref、Commit、Review Evidence 与验证状态。

| AC | 来源 | Task | 计划代码范围 | 实际 Code Ref | Commit | Review Evidence | 验证状态 |
|----|------|------|--------------|-----------------|--------|-----------------|----------|
| AC-1.1 | spec.md | TASK-SKELETON-1, TASK-1, TASK-4, TASK-6, TASK-8 | `cli_tool_manager_service.cpp:ExecCmd/ExecCmdToolMode/SetupCmdSession`、`js_cli_manager_utils.cpp:UnwrapExecCmdOptions`、`cli_tool_mgr_service_test.cpp:ExecCmd_ToolMode_Success_0100` | 实施后回填 | 实施后回填 | `evidence/reviews/spec-compliance.md`（实施后生成） | Pending |
| AC-1.2 | spec.md | TASK-SKELETON-1, TASK-1, TASK-4, TASK-6 | `exec_cmd_param.h:ExecCmdParam.isShellCommand`、`cli_tool_manager_service.cpp:ExecCmd`（分流点）、`cli_tool_mgr_service_test.cpp:ExecCmd_DefaultShellDispatch_0100` | 实施后回填 | 实施后回填 | `evidence/reviews/spec-compliance.md` | Pending |
| AC-1.3 | spec.md | TASK-2, TASK-4, TASK-6 | `exec_cmd_param.cpp:ExecCmdParam::ExtractToolName`、`cli_tool_manager_service.cpp:ExecCmdToolMode`（白名单）、`cli_tool_mgr_service_test.cpp:ExecCmd_ToolMode_InvalidToolName_0100/_0200`、`::ExecCmd_ToolMode_EmptyToolName_0100` | 实施后回填 | 实施后回填 | `evidence/reviews/spec-compliance.md` | Pending |
| AC-1.4 | spec.md | TASK-3, TASK-4, TASK-5, TASK-6 | `tool_util.cpp:ParseToolCommand`（工具名/子命令阶段）、`cli_tool_manager_service.cpp:ExecCmdToolMode`（查表）、`tool_util_test.cpp:ParseToolCommand_UnknownSubCommand_0100`、`cli_tool_mgr_service_test.cpp:ExecCmd_ToolMode_ToolNotExist_0100` | 实施后回填 | 实施后回填 | `evidence/reviews/spec-compliance.md` | Pending |
| AC-1.5 | spec.md | TASK-2, TASK-3, TASK-5, TASK-6, TASK-7 | `cli_tool_mgr_client.cpp:CliToolMGRClient::ExecCmd`（前置拒绝）、`tool_util.cpp:ParseToolCommand`（空命令）、`cli_tool_mgr_client_test.cpp:ExecCmd_0300`、`tool_util_test.cpp:ParseToolCommand_EmptyCmd_0100` | 实施后回填 | 实施后回填 | `evidence/reviews/spec-compliance.md` | Pending |
| AC-1.6 | spec.md | TASK-3, TASK-5 | `tool_util.cpp:ParseToolCommand`（标志解析与类型恢复）、`tool_util_test.cpp:ParseToolCommand_UnknownFlag_0100`、`::ParseToolCommand_TypeMismatch_0100`、`::ParseToolCommand_RepeatNonArrayFlag_0100` | 实施后回填 | 实施后回填 | `evidence/reviews/spec-compliance.md` | Pending |
| AC-1.7 | spec.md | TASK-3, TASK-5 | `tool_util.cpp:ParseToolCommand`、`tool_util_test.cpp:ParseToolCommand_KeyEqualsValue_0100`/`_KeySpaceValue_0100`/`_BooleanFlag_0100`/`_BooleanFalse_0100`/`_SingleQuoteSpace_0100`/`_SubCommandSchema_0100`/`_ShortFlag_0100`/`_UnclosedQuote_0100` | 实施后回填 | 实施后回填 | `evidence/reviews/spec-compliance.md` | Pending |
| AC-1.8 | spec.md | TASK-2, TASK-7 | `exec_cmd_param.cpp:Marshalling/Unmarshalling`、`exec_cmd_param_test.cpp:ExecCmdParam_Unmarshalling_OldClient_0100`、`::ExecCmdParam_Parcelable_0100/_0200`、`::ExecCmdParam_ExtractToolName_0100/_0200/_0300` | 实施后回填 | 实施后回填 | `evidence/reviews/spec-compliance.md` | Pending |
| AC-1.9 | spec.md | TASK-4, TASK-6 | `cli_tool_manager_service.cpp:ExecCmd`（上限校验先于分流）、`cli_tool_mgr_service_test.cpp:ExecCmd_ToolMode_SessionLimit_0100` | 实施后回填 | 实施后回填 | `evidence/reviews/spec-compliance.md` | Pending |
| AC-1.10 | spec.md | TASK-1, TASK-4, TASK-6 | `js_cli_manager_utils.cpp:UnwrapExecCmdOptions`（显式 `true`）、`cli_tool_manager_service.cpp:ExecCmd`、`cli_tool_mgr_service_test.cpp:ExecCmd_ExplicitShellMode_0100` | 实施后回填 | 实施后回填 | `evidence/reviews/spec-compliance.md` | Pending |

## 首批实现边界

**首批必须实现：** 端到端骨架（TASK-SKELETON-1）+ NAPI 属性解包（TASK-1）+ IPC 字段与客户端前置校验（TASK-2）+ 服务端分流、白名单与会话建立（TASK-4）+ 三条防回退守护用例（VM-1/VM-8/VM-11）保持绿。这是「shell 零回退 + 工具命令可执行 + 安全边界成立」的稳定性底线。

**可后置：** 命令解析的四条边缘拒绝分支用例（VM-7 中裸 token、未闭合引号、非数组重复标志、类型恢复失败）与失败回滚故障注入用例（VM-12）可在核心链路 Green 后补齐；跨仓 `.d.ts` 交付（TASK-8）在主仓实现定稿后提交。

**不建议延后：** 命令解析器本体（TASK-3）—— 无解析器则工具模式无法产出 `ExecToolParam`，TASK-4 主链不闭合；`Unmarshalling` 的缺省回退（TASK-2）—— 缺失即破坏 IPC 兼容（AC-1.8）；白名单校验（TASK-4）—— 缺失即扩大命令执行攻击面（INV-4）。

## 阶段计划

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 骨架接线 | 声明→NAPI→客户端→IPC→服务端分流全链路可传递模式标识，缺省仍走 shell | TASK-SKELETON-1, TASK-1, TASK-2 | 缺省与显式 shell 调用行为不变（VM-8 守护用例 Green）；Parcel 往返与旧客户端缺省回退 Green（VM-9） | `run -t UT -ts exec_cmd_param_test`、`run -t UT -ts cli_tool_mgr_client_test`、`run -t UT -ts cli_tool_mgr_service_test` |
| Phase-2 解析与执行 | 工具命令解析、服务端校验复用与会话建立/回滚 | TASK-3, TASK-4 | 工具模式成功路径 Green（VM-2）；白名单/工具不存在/上限三类拒绝 Green（VM-3/VM-4/VM-10）；权限门回归 Green（VM-1） | `run -t UT -ts tool_util_test`、`run -t UT -ts cli_tool_mgr_service_test` |
| Phase-3 验证闭环 | 三层用例补齐至 `spec.md#验证映射` 全量 | TASK-5, TASK-6, TASK-7 | VM-1..VM-13 全部有对应用例且 Green；三条守护用例仍 Green | `run -t UT -tp ability_runtime`（部件全量单测） |
| Phase-4 声明交付 | 跨仓 System API 声明与错误面一致 | TASK-8 | 中英双份声明新增两属性且标注组一致；`@throws` 含 201/202/401/35600030/35600031/35600050 | 声明双份 diff 核对 + `./build.sh --product-name rk3568 --build-target ability_runtime` |

## Task 粒度原则

- 每个 Task 对应一个可独立验收的最小能力闭环（一个层或一个测试目标）
- TASK-1 与 TASK-2 共享「新属性/新字段」上下文但分属 NAPI 与 IPC 两个构建边界，故拆分
- TASK-3 与 TASK-4 共享规则上下文，但解析为纯函数（无需 SA/mock）、分流与会话建立需服务侧 mock 与故障注入，验证闭环分离，故拆分
- TASK-5/6/7 按测试目标（`tool_util_test` / `cli_tool_mgr_service_test` + `cli_tool_mgr_client_test` / `exec_cmd_param_test`）拆分，各自独立可跑
- 单 Task 上下文（Spec + Design + 代码 + 测试）均低于 3000 行阈值，无需进一步拆分

## 禁止项

- [x] 没有未决项、待办标记或占位符（计划期未产生的实际范围、Commit、Review Evidence 统一写「实施后回填」，并在 Task 完成当次会话补齐）
- [x] 没有"根据需要实现""酌情处理"等模糊指令
- [x] 没有跨 Task 隐式依赖（依赖均显式写在「前置依赖」列）
- [x] 没有要求 Agent 自行寻找未列出的上下文文件（每 Task 有 Read-only Context）
- [x] 没有无验证方式的 AC（AC→Task→VM→验证命令齐备）
- [x] 没有"与 Task-N 类似""参考 Task-N 实现"等引用（每 Task 自包含 Reference Pattern 与 Steps）

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 状态 |
|---------|------|----------|---------|----------|----------|----------|------|
| TASK-SKELETON-1 | 打通声明→NAPI→客户端→IPC→服务端分流的空骨架：模式标识可传递，缺省走 shell，工具模式入口可命中 | `exec_cmd_param.h/.cpp`、`js_cli_manager_utils.cpp`、`cli_tool_mgr_client.cpp`、`cli_tool_manager_service.h/.cpp` | AC-1.1（骨架可达）, AC-1.2 | 无 | 缺省调用行为不变；`isShellCommand=false` 能进入工具命令处理入口并返回既有内部码 | `run -t UT -ts cli_tool_mgr_service_test` | Pending |
| TASK-1 | NAPI 层两个可选属性按既有模式解包，缺省保留 C++ 默认值，类型非法抛 401 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager_utils.cpp` | AC-1.2, AC-1.10 | TASK-SKELETON-1 | 传/不传该属性两条路径的缺省值一致为 `true`；`challenge` 缺省 `""`；非布尔/非字符串入参抛 401 | `./build.sh --product-name rk3568 --build-target ability_runtime` | Pending |
| TASK-2 | IPC 数据层尾部追加两字段 + 缺省回退 + 工具名提取；客户端 eventKey 按模式取值、工具模式空白命令前置拒绝 | `exec_cmd_param.h`、`exec_cmd_param.cpp`、`cli_tool_mgr_client.cpp` | AC-1.3, AC-1.5, AC-1.8 | TASK-1 | 字段序冻结为 7 项且新增两项在尾部；旧客户端 Parcel 反序列化成功且回退默认值；工具模式空白命令不发 IPC | `run -t UT -ts exec_cmd_param_test`、`run -t UT -ts cli_tool_mgr_client_test` | Pending |
| TASK-3 | 服务端命令解析：长标志子集分词 + 子命令解析 + schema 类型恢复，非法语法显式拒绝 | `cli_tool_framework/services/climgr/include/tool_util.h`、`src/tool_util.cpp` | AC-1.4, AC-1.6, AC-1.7 | TASK-2 | 四种长标志写法与单引号值解析正确；五类非法输入返回 `ERR_INVALID_PARAM`；未声明子命令返回 `ERR_TOOL_NOT_EXIST` | `run -t UT -ts tool_util_test` | Pending |
| TASK-4 | 服务端分流与会话建立：白名单、查表、解析调用、复用 `ValidateAndPrepareTool`、按序建立会话并反向回滚、失败点 DFX 上报 | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h`、`src/cli_tool_manager_service.cpp` | AC-1.1, AC-1.2, AC-1.3, AC-1.9, AC-1.10 | TASK-3 | 工具模式成功返回会话信息；白名单/未注册/上限三类拒绝且不建进程；三步失败均不留残；shell 路径逐位不变 | `run -t UT -ts cli_tool_mgr_service_test` | Pending |
| TASK-5 | 解析层用例补齐至 VM-6/VM-7/VM-13 全量 | `cli_tool_framework/test/unittest/tool_util_test/tool_util_test.cpp` | AC-1.4, AC-1.6, AC-1.7 | TASK-3 | 10 条 `ParseToolCommand_*` 用例 + 1 条工具沙箱配置断言用例全绿 | `run -t UT -ts tool_util_test` | Pending |
| TASK-6 | 服务层与客户端层用例补齐至 VM-1/VM-2/VM-3/VM-4/VM-5/VM-8/VM-10/VM-12/VM-13 全量 | `cli_tool_mgr_service_test.cpp`、`cli_tool_mgr_client_test.cpp` | AC-1.1, AC-1.2, AC-1.3, AC-1.4, AC-1.5, AC-1.9, AC-1.10 | TASK-4 | 成功路径、白名单、未注册、上限、缺省/显式 shell、三步失败回滚、options 语义、客户端前置拒绝用例全绿 | `run -t UT -ts cli_tool_mgr_service_test`、`run -t UT -ts cli_tool_mgr_client_test` | Pending |
| TASK-7 | 序列化层用例补齐至 VM-9 全量 | `cli_tool_framework/test/unittest/exec_cmd_param_test/exec_cmd_param_test.cpp` | AC-1.8 | TASK-2 | 完整往返、旧客户端缺省回退、工具名提取三类边界用例全绿 | `run -t UT -ts exec_cmd_param_test` | Pending |
| TASK-8 | 跨仓 System API 声明交付（中英双份）+ `@throws` 补 401 与 35600030 | `interface/sdk-js:api/@ohos.app.cli.cliManager.d.ts`、`zh-cn/api/@ohos.app.cli.cliManager.d.ts` | AC-1.1, AC-1.2, AC-1.10 | TASK-1, TASK-4（运行时错误面定稿） | 两属性声明与标注组一致（`@systemapi`/`@stagemodelonly`/`@syscap`/`@since 26.0.0 dynamiconly`）；`@throws` 含 6 个错误码；不为 `workDir`/`env`/`policy` 增加模式差异说明 | 中英双份 diff 核对 + `./build.sh --product-name rk3568 --build-target ability_runtime` | Pending |

## Task 详情

### TASK-SKELETON-1: 端到端骨架接线

| 字段 | 内容 |
|------|------|
| 任务目标 | 让「模式标识」能从 ArkTS 调用一路传递到服务端分流点：新增字段可序列化/反序列化、NAPI 可解包、客户端可路由、服务端可判定并进入工具命令处理入口（入口先返回既有内部码，不含解析与会话建立） |
| AC 映射 | AC-1.1（骨架可达性）、AC-1.2 |
| 前置依赖 | 无 |
| 非目标 | 不实现命令解析、不实现会话建立、不修改 shell 路径任何一行逻辑、不改 IDL |
| 完成判据 | ①不传 options 与显式传 `isShellCommand: true` 两种调用均走 shell 路径且结果与既有行为一致；②传 `isShellCommand: false` 时进入工具命令处理入口（服务侧 mock 可断言入口被命中）；③Parcel 完整往返七字段一致 |
| 停止条件 | 若发现必须修改 shell 路径既有校验次序或 IDL 签名才能传递字段 → 停止，回 `design.md#关键设计决策` ADR-1 修订 |
| Design 状态/不变量 | STATE-1、INV-1 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| `STATE-1 / INV-1` | 命令模式标识 `isShellCommand` | `ExecCmdParam`（随调用传递） | Parcel 尾部字段 6 | NAPI 解包时写入，缺省 `true` | 只读：服务端分流判定一次 | 随参数对象析构 | 三处默认值（C++ 成员初值 / NAPI 缺省 / IPC 缺省回退）必须同为 `true` |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `ExecCmdParam{ cmd, workDir, env, policy, options, isShellCommand = true, challenge }`；Parcel 字段序冻结为 `cmd → workDir → env → policy → options → isShellCommand → challenge` | TASK-SKELETON-1 | TASK-1, TASK-2, TASK-4, TASK-7 消费 | 只可尾部追加；`Unmarshalling` 对尾部字段读取失败必须置默认值并成功返回（不得 `nullptr`） |
| Produces | `CliToolManagerService::ExecCmd` 分流点位置：权限门 → `SetScheduler` → 会话上限 → 取 caller 上下文 → **分流** → shell 逻辑 | TASK-SKELETON-1 | TASK-4 消费 | 分流点不得前移（INV-6）；shell 分支代码逐位不变 |
| Consumes | 既有 NAPI 解包约定 `UnwrapExecCmdOptions`（`js_cli_manager_utils.cpp:219-365`）与既有 `ExecCmdParam::Marshalling/Unmarshalling`（`exec_cmd_param.cpp:22-52,55-85`） | 既有代码 | 无 | 新属性插入不得破坏末尾对 `UnwrapExecOptions` 的复用 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `spec.md#规则定义`（R-3, R-11）、`spec.md#验证映射`（VM-8, VM-9） | 缺省行为与 IPC 兼容的验收口径与 Red 条件 |
| `design.md#代码事实基线`（`ExecCmdParam`、`ExecCmd`、`UnwrapExecCmdOptions` 三行） | 既有字段序、入口次序、解包约定 |
| `design.md#数据模型设计`（序列化契约表） | 七字段序与读取失败处理 |
| `cli_tool_framework/interfaces/cli_tool/include/exec_options.h` | 确认 `ExecOptions` 原生默认值（`timeout = 0`） |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/interfaces/cli_tool/include/exec_cmd_param.h` | 尾部新增两成员（骨架阶段仅声明与默认值） |
| Modify | `cli_tool_framework/interfaces/cli_tool/src/exec_cmd_param.cpp` | `Marshalling` 尾部写入、`Unmarshalling` 尾部读取并缺省回退 |
| Modify | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager_utils.cpp` | `UnwrapExecCmdOptions` 内接入两属性（骨架阶段可只解 `isShellCommand`） |
| Modify | `cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp` | eventKey 按模式取值（骨架阶段仅接线） |
| Modify | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h`、`src/cli_tool_manager_service.cpp` | 新增分流判定与工具命令处理入口声明/空实现（返回既有内部码） |
| Test | `cli_tool_framework/test/unittest/exec_cmd_param_test/exec_cmd_param_test.cpp` | 最小 Red 用例：完整往返断言七字段 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` | 接口签名不变（design ADR-1）；改动会触发 proxy/stub 重新生成并扩大兼容面 |
| `cli_tool_manager_service.cpp` 的 shell 分支与 `ValidateAndPrepareCmd` | 属既有 shell 路径，AC-1.2/AC-1.10 要求逐位不变 |
| `cli_tool_framework/services/climgr/src/tool_util.cpp` | TASK-3 所有 |
| `hisysevent.yaml`、`cli_error_code.h`、`cli_manager_error_utils.h` | 已发布事件与错误码数值，AGENTS.md 禁止项 |

**Spec References**

- AC / Rules: `AC-1.1`, `AC-1.2`, `R-3`, `R-11`, `BF-4`
- Anchors: `spec.md#用户故事`, `spec.md#规则定义`, `spec.md#验证映射`

**Design References**

- IDs / Anchors: `ADR-1`, `ADR-2`, `ADR-7`, `STATE-1`, `INV-1`, `INV-6`, `design.md#数据模型设计`, `design.md#数据流控制流`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-IPC-SAF | Must：新增字段只在 Parcel 尾部；缺省回退不得返回失败。Must Not：修改既有字段顺序或编码方式、新增 IPC 方法 |
| OH-ARCH-LAYERING | Must：分流点位于权限门/调度器/会话上限之后。Must Not：把模式判定下沉到客户端或 NAPI |
| OH-ARCH-API-LEVEL | Must：NAPI 新属性沿用「存在才解、失败抛 401」。Must Not：改变既有属性的解包次序与错误行为 |

**Steps**

- [ ] 写失败测试：在 `exec_cmd_param_test.cpp` 增加完整往返用例（断言七字段）与旧客户端缺省回退用例（Parcel 只写到 `options`），运行确认编译失败/断言失败（Red 证据见 spec VM-9）
- [ ] 运行 `run -t UT -ts exec_cmd_param_test`，记录 Red 输出
- [ ] 做最小实现：`ExecCmdParam` 两成员 + 序列化尾部读写与缺省回退；NAPI 解包 `isShellCommand`；客户端 eventKey 接线；服务端分流点与入口空实现
- [ ] 运行 `run -t UT -ts exec_cmd_param_test` 与 `run -t UT -ts cli_tool_mgr_service_test`，确认新用例 Green 且既有 shell 用例（`ExecCmd_0100`..`ExecCmd_0700`）仍 Green
- [ ] 回填 AC 追溯（AC-1.1/AC-1.2 行）、实际文件、Commit、Review Evidence、Actual Result 与计划偏差

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `cli_tool_framework/interfaces/cli_tool/src/exec_tool_param.cpp / ExecToolParam::Marshalling` | 尾部写入 + 逐项失败即整体失败的序列化约定 | 无 |
| `cli_tool_framework/interfaces/cli_tool/src/exec_cmd_param.cpp:55-85 / ExecCmdParam::Unmarshalling` | 既有字段读取失败即 `return nullptr` | 偏差：新增两字段读取失败**不**返回 `nullptr`，而是置默认值后成功返回（IPC 兼容要求，spec R-11 / design ADR-1） |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager_utils.cpp:219-365 / UnwrapExecCmdOptions` | `napi_has_named_property → napi_get_named_property → Unwrap*FromJS2 → ThrowInvalidParamError` | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.2 正向（缺省走 shell）+ AC-1.1 骨架（工具模式入口命中）+ AC-1.8 兼容（旧 Parcel 回退）三类证据；VM-8 守护用例 Green 截图/日志 |
| Scope respected | 实际 diff 仅含 Files 列出的 6 个文件；shell 分支与 IDL 无 diff |
| State lifecycle complete | STATE-1 的创建（NAPI 写入）、读取（服务端分流）、清理（对象析构）与 INV-1 三处默认值一致性均有断言 |
| Interface contract matched | TASK-1/2/4/7 引用的字段序与 `ExtractToolName` 签名与本 Task Produces 完全一致 |
| Relevant verification | `exec_cmd_param_test` 与 `cli_tool_mgr_service_test` 直接覆盖本 Task，非无关测试 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -ts exec_cmd_param_test` | 新增往返与缺省回退用例 PASS；既有用例不回归 | 实施后回填（真实输出摘要 + PASS/FAIL + 日志路径） |
| 测试 | `run -t UT -ts cli_tool_mgr_service_test` | `ExecCmd_0100`..`ExecCmd_0700` 全 PASS（shell 与权限门无回退）；分流命中用例 PASS | 实施后回填 |
| 静态检查 | `./build.sh --product-name rk3568 --build-target ability_runtime` | 编译通过，无新增 warning | 实施后回填 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-SKELETON-1：模式标识端到端传递骨架，缺省 shell 行为不变 |
| 允许修改 | `exec_cmd_param.h`、`exec_cmd_param.cpp`、`js_cli_manager_utils.cpp`、`cli_tool_mgr_client.cpp`、`cli_tool_manager_service.h`、`cli_tool_manager_service.cpp`（仅分流点与入口声明） |
| 允许新建 | 无（不新增源文件，故不触及 BUILD.gn） |
| 只读参考 | `spec.md#规则定义`、`spec.md#验证映射`、`design.md#代码事实基线`、`design.md#数据模型设计`、`exec_options.h`、`exec_tool_param.cpp` |
| Spec 引用 | `AC-1.1`, `AC-1.2`, `R-3`, `R-11`, `BF-4`, `spec.md#验证映射`（VM-8, VM-9） |
| Design 引用 | `ADR-1`, `ADR-2`, `ADR-7`, `STATE-1`, `INV-1`, `INV-6`, `design.md#数据模型设计` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -ts exec_cmd_param_test`、`run -t UT -ts cli_tool_mgr_service_test` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.1/AC-1.2/AC-1.8 → TASK-SKELETON-1 → 实际文件:符号 → Commit；shell 分支零 diff 的边界证明 |
| Code Quality | 实际 diff 范围、序列化字段序、Base/Head SHA、编译与单测结果 |
| Verification | VM-8/VM-9 的 Expected/Actual、fresh evidence 路径、既有用例回归结果 |

### TASK-1: NAPI 层两个可选属性解包

| 字段 | 内容 |
|------|------|
| 任务目标 | 在 `UnwrapExecCmdOptions` 内按既有模式解包 `isShellCommand` 与 `challenge`：属性存在才解、类型非法抛 401、缺省保留 C++ 默认值（`true` / `""`） |
| AC 映射 | AC-1.2, AC-1.10 |
| 前置依赖 | TASK-SKELETON-1（字段与 Parcel 契约已建立） |
| 非目标 | 不修改 `UnwrapExecOptions`（保护 `execTool` 的缺省 `timeout` 语义，spec R-17）；不做服务端逻辑；不改 `.d.ts`（TASK-8） |
| 完成判据 | ①不传该属性 → `param.isShellCommand == true`；②显式传 `true`/`false` → 原值传递；③传非布尔/非字符串 → 抛 401 且不发起调用；④`challenge` 缺省为 `""`，传值原样保留 |
| 停止条件 | 若发现必须修改 `UnwrapExecOptions` 才能满足缺省语义 → 停止，回 `design.md` ADR-7 修订（会影响 `execTool`） |
| Design 状态/不变量 | STATE-1, STATE-2, INV-1, INV-5 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| `STATE-1 / INV-1` | `isShellCommand` | NAPI 解包（写入）→ `ExecCmdParam` | JS 属性名 `isShellCommand` | `napi_has_named_property` 为真时写入 | 只读传递，不在 NAPI 侧消费 | 随 `ExecCmdParam` 析构 | 缺省必须为 `true`，与 C++ 成员初值、IPC 回退值一致 |
| `STATE-2 / INV-5` | `challenge` | NAPI 解包（写入）→ `ExecCmdParam` | JS 属性名 `challenge` | 属性存在时写入，缺省 `""` | 只读搬运，不校验内容、不打印 | 随 `ExecCmdParam` 析构 | 不得在 NAPI 侧生成、改写或记入日志 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `ExecCmdParam.isShellCommand`（`bool`，缺省 `true`）、`ExecCmdParam.challenge`（`std::string`，缺省 `""`）由 JS 侧填充完毕 | TASK-1 | TASK-2, TASK-4, TASK-6 消费 | 缺省值三处一致（INV-1）；类型非法一律 401，不得静默取默认值 |
| Consumes | `ExecCmdParam` 字段与默认值 | TASK-SKELETON-1 | 前置 | 不得新增/重命名字段 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager_utils.cpp`（`UnwrapExecCmdOptions`、`UnwrapExecOptions`） | 既有解包次序、错误抛出方式、`timeout` 缺省填充位置 |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp`（`OnExecCmd`） | JS 入口的 `cmd` 非空校验与异步任务分发方式 |
| `spec.md#接口规格`（参数约束表） | 每个属性的合法范围与非法处理 |
| `design.md#接口参数规约` | 类型非法 → 401 的统一口径 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager_utils.cpp` | 两个新属性解包，插入在既有属性之后、`UnwrapExecOptions` 复用之前 |
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_client_test/cli_tool_mgr_client_test.cpp` | 以 `ExecCmdParam` 断言解包结果与缺省值（NAPI 层无独立测试目标，缺省语义由客户端/服务侧用例与代码评审共同看护） |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `js_cli_manager_utils.cpp` 的 `UnwrapExecOptions` 函数体 | 修改会同时改变 `execTool` 的缺省 `timeout` 语义（spec R-17 明确保持现状） |
| `js_cli_manager.cpp` 的 `OnExecTool` | 属 `execTool` 路径，超出范围 |
| `interface/sdk-js` 下任何文件 | TASK-8 所有 |

**Spec References**

- AC / Rules: `AC-1.2`, `AC-1.10`, `R-3`, `R-17`, `BF-2`
- Anchors: `spec.md#接口规格`, `spec.md#规则定义`, `spec.md#验证映射`（VM-8, VM-13）

**Design References**

- IDs / Anchors: `ADR-7`, `STATE-1`, `STATE-2`, `INV-1`, `INV-5`, `design.md#接口参数规约`, `design.md#既有模式复用`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-API-LEVEL | Must：可选属性缺省即既有行为；类型非法抛 401。Must Not：新增权限、改变既有属性语义 |
| OH-ARCH-ERROR-LOG | Must：错误信息不回显 `cmd`/`challenge` 内容。Must Not：新增错误码数值 |
| OH-ARCH-LAYERING | Must Not：在 NAPI 侧做工具名校验或命令解析（安全判定留在服务端，design ADR-3） |

**Steps**

- [ ] 写失败测试：断言「传 options 但不含 `isShellCommand`」→ 参数对象为 `true`；「显式 `false`」→ 为 `false`；「非布尔」→ 抛 401（Red：属性未被解包，断言值恒为默认或抛错缺失）
- [ ] 运行 `run -t UT -ts cli_tool_mgr_client_test`，记录 Red 输出
- [ ] 做最小实现：两属性按既有 `has → get → Unwrap*FromJS2 → ThrowInvalidParamError` 模式接入
- [ ] 运行同一命令确认 Green；再运行 `./build.sh --product-name rk3568 --build-target ability_runtime` 确认编译无新增 warning
- [ ] 回填 AC 追溯（AC-1.2/AC-1.10 行）、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `js_cli_manager_utils.cpp:237-292 / UnwrapExecCmdOptions`（`workDir`/`env`/`policy` 解包段） | 属性存在才解、解失败抛 401、缺省保留 C++ 默认值 | 无 |
| `js_cli_manager_utils.cpp:155-217 / UnwrapExecOptions` | 布尔/整型属性的解包写法 | 只读参考，不修改 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.2（缺省）与 AC-1.10（显式 `true`）两条路径均有断言；类型非法路径有 401 证据 |
| Scope respected | diff 仅 `js_cli_manager_utils.cpp` 的新增属性段 + 一个测试文件；`UnwrapExecOptions` 零 diff |
| State lifecycle complete | STATE-1/STATE-2 的创建（解包写入）与 INV-1 三处默认值一致性、INV-5「不生成不改写」均有证据 |
| Interface contract matched | 产出的字段名/类型/缺省值与 TASK-SKELETON-1 Produces 一致 |
| Relevant verification | `cli_tool_mgr_client_test` + 编译验证直接覆盖本 Task |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -ts cli_tool_mgr_client_test` | 新增缺省/显式/类型非法用例 PASS，既有用例不回归 | 实施后回填 |
| 静态检查 | `./build.sh --product-name rk3568 --build-target ability_runtime` | 编译通过，无新增 warning | 实施后回填 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-1：NAPI 两个可选属性解包与缺省值一致性 |
| 允许修改 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager_utils.cpp`（仅 `UnwrapExecCmdOptions` 内新增段）、`cli_tool_mgr_client_test.cpp` |
| 允许新建 | 无 |
| 只读参考 | `js_cli_manager_utils.cpp`（`UnwrapExecOptions`）、`js_cli_manager.cpp`（`OnExecCmd`）、`spec.md#接口规格`、`design.md#接口参数规约` |
| Spec 引用 | `AC-1.2`, `AC-1.10`, `R-3`, `R-17`, `BF-2`, `spec.md#接口规格` |
| Design 引用 | `ADR-7`, `STATE-1`, `STATE-2`, `INV-1`, `INV-5`, `design.md#既有模式复用` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -ts cli_tool_mgr_client_test` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；不得触碰 `UnwrapExecOptions`；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.2/AC-1.10 → TASK-1 → `js_cli_manager_utils.cpp:UnwrapExecCmdOptions` → Commit；确认未新增 spec 之外的属性语义 |
| Code Quality | 解包段与既有模式一致性、错误信息不含敏感内容、diff 范围 |
| Verification | 缺省/显式/类型非法三类 Expected/Actual 与 fresh evidence 路径 |

### TASK-2: IPC 数据层字段与客户端前置校验

| 字段 | 内容 |
|------|------|
| 任务目标 | 完成 `ExecCmdParam` 尾部两字段的序列化契约（含旧客户端缺省回退）与工具名提取函数；客户端 eventKey 按模式取值，工具模式下空白命令在发起 IPC 前拒绝 |
| AC 映射 | AC-1.3, AC-1.5, AC-1.8 |
| 前置依赖 | TASK-SKELETON-1（骨架已接线）；TASK-1（NAPI 侧字段已可填充） |
| 非目标 | 不实现服务端白名单（TASK-4）；不实现解析（TASK-3）；不改 IDL |
| 完成判据 | ①完整 Parcel 往返七字段值一致；②只写到 `options` 的 Parcel 反序列化成功且 `isShellCommand == true`、`challenge == ""`；③`ExtractToolName` 对前导空白/无空白/空串分别返回首 token/整串/`""`；④工具模式 + 空白 `cmd` → 客户端返回 `ERR_INVALID_PARAM` 且 proxy 调用次数为 0；⑤工具模式 eventKey = 工具名，shell 模式 eventKey = `"shell"` |
| 停止条件 | 若旧客户端 Parcel 无法在不返回 `nullptr` 的前提下回退 → 停止，回 `design.md` ADR-1 修订 |
| Design 状态/不变量 | STATE-1, STATE-2, INV-1 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| `STATE-1 / INV-1` | `isShellCommand`（Parcel 字段 6） | `ExecCmdParam::Marshalling/Unmarshalling` | Parcel 尾部序号 6 | 写入端 `WriteBool`；读取端 `ReadBool` 失败置 `true` | 服务端分流只读；客户端 eventKey 计算只读 | 对象析构 | 读取失败必须成功返回对象，不得 `nullptr` |
| `STATE-2 / INV-5` | `challenge`（Parcel 字段 7） | 同上 | Parcel 尾部序号 7 | 写入端 `WriteString`；读取端失败置 `""` | 只搬运 | 对象析构 | 不入日志、不参与鉴权判定 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `static std::string ExecCmdParam::ExtractToolName(const std::string &cmd)`：忽略前导空白，返回首个空白分隔 token；无空白返回整串；空/全空白返回 `""` | TASK-2 | TASK-4（服务端白名单输入）、客户端 eventKey 计算 | 两端必须使用同一函数，避免客户端/服务端工具名判定不一致 |
| Produces | eventKey 契约：`isShellCommand ? "shell" : ExtractToolName(cmd)` | TASK-2 | TASK-6（回调路由用例） | 与 `execTool` 的按工具名路由语义一致（design ADR-2） |
| Produces | 客户端前置校验：工具模式下 `cmd.find_first_not_of(" \t") == npos` → `ERR_INVALID_PARAM`（不发 IPC） | TASK-2 | TASK-4, TASK-6 | 只做无需工具信息的校验；不得在客户端做白名单/schema 判定（OH-ARCH-LAYERING） |
| Consumes | `ExecCmdParam` 字段与缺省值 | TASK-SKELETON-1, TASK-1 | 前置 | 字段序不得调整 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `cli_tool_framework/interfaces/cli_tool/src/exec_cmd_param.cpp` | 既有写入/读取序与失败处理约定 |
| `cli_tool_framework/interfaces/cli_tool/src/exec_tool_param.cpp` | 同类 `Parcelable` 的序列化写法参考 |
| `cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp`（`ExecCmd`、`ExecTool`） | eventKey 与回调/订阅注册、失败清理方式 |
| `spec.md#验证映射`（VM-5, VM-9） | Red 条件与通过标准 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/interfaces/cli_tool/include/exec_cmd_param.h` | 新增静态提取函数声明（成员已由骨架建立） |
| Modify | `cli_tool_framework/interfaces/cli_tool/src/exec_cmd_param.cpp` | 尾部读写、缺省回退、提取函数实现 |
| Modify | `cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp` | eventKey 按模式取值 + 空白命令前置拒绝 |
| Test | `cli_tool_framework/test/unittest/exec_cmd_param_test/exec_cmd_param_test.cpp` | 往返/缺省回退/提取边界（最小 Red 集，全量在 TASK-7） |
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_client_test/cli_tool_mgr_client_test.cpp` | 空白命令不发 IPC（最小 Red 集，全量在 TASK-6） |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `ICliToolManager.idl` 及其生成的 proxy/stub | 接口签名不变（design ADR-1） |
| `cli_tool_mgr_client.cpp` 的 `ExecTool` | 属既有 `execTool` 路径 |
| `services/climgr/**` | TASK-3/TASK-4 所有 |

**Spec References**

- AC / Rules: `AC-1.3`, `AC-1.5`, `AC-1.8`, `R-4`, `R-6`, `R-11`, `R-12`, `BF-1`
- Anchors: `spec.md#规则定义`, `spec.md#兼容性声明`, `spec.md#验证映射`（VM-5, VM-9）

**Design References**

- IDs / Anchors: `ADR-1`, `ADR-2`, `STATE-1`, `STATE-2`, `INV-1`, `INV-5`, `design.md#数据模型设计`, `design.md#异常传播时序图`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-IPC-SAF | Must：字段只追加在尾部；缺省回退成功返回。Must Not：改既有字段序、引入版本协商字段 |
| OH-ARCH-LAYERING | Must：客户端只做无需工具信息的前置校验。Must Not：在客户端做白名单/schema/权限判定 |
| OH-ARCH-ERROR-LOG | Must：返回既有内部码 `ERR_INVALID_PARAM`。Must Not：日志输出 `cmd` 原文（用 `%{private}s`） |

**Steps**

- [ ] 写失败测试：旧客户端 Parcel（只写到 `options`）断言回退默认值；`ExtractToolName` 三类边界；工具模式空白 `cmd` 断言 proxy 调用次数为 0（Red：符号不存在/断言失败）
- [ ] 运行 `run -t UT -ts exec_cmd_param_test` 与 `run -t UT -ts cli_tool_mgr_client_test`，记录 Red
- [ ] 做最小实现：尾部读写 + 缺省回退 + 提取函数 + eventKey 分流 + 前置拒绝
- [ ] 运行同两命令确认 Green
- [ ] 回填 AC 追溯（AC-1.3/AC-1.5/AC-1.8 行）、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `exec_tool_param.cpp / ExecToolParam::Marshalling, Unmarshalling` | 逐项写、失败即整体失败；读取失败返回 `nullptr` | 偏差：尾部两字段读取失败置默认值并成功返回（spec R-11） |
| `cli_tool_mgr_client.cpp:74-115 / CliToolMGRClient::ExecCmd` | `EnsureSchedulerStubCreated → proxy → eventKey → 回调/订阅注册 → 失败清理` | 扩展：eventKey 由固定串改为按模式取值；新增前置校验置于 proxy 调用之前 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.8 正向（完整往返）+ 兼容（旧客户端回退）+ AC-1.5 异常（空白命令不发 IPC）+ AC-1.3 前置（提取函数边界）四类证据 |
| Scope respected | diff 仅 3 个源文件 + 2 个测试文件；IDL 与 proxy/stub 零 diff |
| State lifecycle complete | STATE-1/STATE-2 的写入端、读取端、缺省回退与析构路径均有断言 |
| Interface contract matched | `ExtractToolName` 签名与 eventKey 契约与 TASK-4/TASK-6 的 Consumes 完全一致 |
| Relevant verification | 两个测试目标直接覆盖序列化与客户端行为 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -ts exec_cmd_param_test` | 往返/缺省回退/提取边界用例 PASS | 实施后回填 |
| 测试 | `run -t UT -ts cli_tool_mgr_client_test` | 空白命令前置拒绝（proxy 调用 0 次）PASS；既有 `ExecCmd_0100`/`_0200` 等不回归 | 实施后回填 |
| 静态检查 | `./build.sh --product-name rk3568 --build-target ability_runtime` | 编译通过，无新增 warning | 实施后回填 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-2：IPC 尾部字段契约 + 工具名提取 + 客户端 eventKey 与前置校验 |
| 允许修改 | `exec_cmd_param.h`、`exec_cmd_param.cpp`、`cli_tool_mgr_client.cpp`、`exec_cmd_param_test.cpp`、`cli_tool_mgr_client_test.cpp` |
| 允许新建 | 无 |
| 只读参考 | `exec_tool_param.cpp`、`cli_tool_mgr_client.cpp`（`ExecTool`）、`spec.md#兼容性声明`、`design.md#数据模型设计` |
| Spec 引用 | `AC-1.3`, `AC-1.5`, `AC-1.8`, `R-4`, `R-6`, `R-11`, `R-12`, `BF-1`, `spec.md#兼容性声明` |
| Design 引用 | `ADR-1`, `ADR-2`, `STATE-1`, `STATE-2`, `INV-1`, `INV-5`, `design.md#数据模型设计` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -ts exec_cmd_param_test`、`run -t UT -ts cli_tool_mgr_client_test` / 期望: PASS |
| 完成规则 | 不得修改 IDL 与既有字段序；不得在客户端引入安全判定；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.5/AC-1.8 → TASK-2 → `exec_cmd_param.cpp`/`cli_tool_mgr_client.cpp` 符号 → Commit；确认字段序与 spec 序列化契约表一致 |
| Code Quality | 缺省回退实现是否可能被误改为 `nullptr`、日志脱敏、diff 范围 |
| Verification | VM-5/VM-9 的 Expected/Actual、proxy 调用次数断言、回归结果 |

### TASK-3: 服务端命令解析

| 字段 | 内容 |
|------|------|
| 任务目标 | 在 `ToolUtil` 新增静态纯函数，把工具命令字符串解析为 `ExecToolParam`：分词（仅单引号）→ 工具名/子命令 → 标志解析 → schema 类型恢复；不支持的语法显式拒绝 |
| AC 映射 | AC-1.4, AC-1.6, AC-1.7 |
| 前置依赖 | TASK-2（工具名提取契约与字段可用） |
| 非目标 | 不做权限/沙箱/会话（TASK-4）；不支持短标志、前缀参数、条件标志、双引号、反斜杠转义（spec R-8）；不接入 `workDir`/`env`/`policy`（ADR-8） |
| 完成判据 | ①四种长标志写法与单引号含空格值解析出正确键值与类型；②子命令命中时取子命令 `inputSchema`，否则取工具级 `inputSchema`；③未知子命令 → `ERR_TOOL_NOT_EXIST`；④裸 token/空标志名/schema 外标志/非 boolean 缺值/类型恢复失败/非数组重复标志/未闭合引号 → `ERR_INVALID_PARAM` 且写 `detail` 原因串；⑤`--help` 恒允许；⑥函数不依赖 SA 实例与 IPC，可在单测中直接调用 |
| 停止条件 | 若 schema 语义与既有 `ValidateInputSchemaProperties` 冲突（如 array 重复键、`help` 例外）→ 停止，回 `spec.md#规则定义` R-7/R-8 澄清 |
| Design 状态/不变量 | INV-7 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| `INV-7` | 解析产物 `ExecToolParam.args`（`AAFwk::WantParams`） | `ToolUtil::ParseToolCommand`（产出）→ 调用方（TASK-4）持有 | schema `properties` 的键 | 类型恢复阶段写入 | 只读：`ValidateProperties` → `ValidateInputSchemaProperties` 消费 | 随 `ExecToolParam` 析构 | 只含 schema 声明的键；值类型与 schema 一致；失败时不得产出半解析结果 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `static int32_t ToolUtil::ParseToolCommand(const std::string &cmd, const ToolInfo &toolInfo, ExecToolParam &param, std::string &detail)`；返回 `ERR_OK` / `ERR_INVALID_PARAM` / `ERR_TOOL_NOT_EXIST` | TASK-3 | TASK-4 消费 | 签名与错误码集合冻结；`detail` 用于 DFX 上报原因分类，不含命令原文 |
| Produces | 支持的语法集合：`--key=value`、`--key value`、`--flag`（schema boolean）、`--flag false`、`--key 'a b'`、`--help`；重复键仅在 schema 为 `array` 时合法 | TASK-3 | TASK-5（用例）、TASK-8（声明说明） | 与 spec R-7/R-8、BF-5/BF-6 完全一致，不得静默扩展 |
| Consumes | `ToolInfo`（`name`、`hasSubCommand`、`subcommands`、`inputSchema`） | 既有 `CliToolDataManager` | 无 | 只读消费，不修改工具注册信息结构 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `cli_tool_framework/services/climgr/include/tool_info.h`、`sub_command_info.h` | 工具与子命令的 schema 字段结构 |
| `cli_tool_framework/services/climgr/src/tool_util.cpp`（`ValidateInputSchemaProperties`、`ValidateArrayType`、`Is*Type`） | schema 类型语义、`help` 例外、array 校验口径 |
| `cli_tool_framework/interfaces/cli_tool/include/exec_tool_param.h` | 解析产物结构（确认无 `workDir`/`env`/`policy` 字段） |
| `spec.md#规则定义`（R-7, R-8）、`spec.md#api-变更分析` 下「行为事实与约束」表（BF-5, BF-6） | 支持语法与拒绝语法的精确边界 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/include/tool_util.h` | 新增静态函数声明（含注释说明支持语法） |
| Modify | `cli_tool_framework/services/climgr/src/tool_util.cpp` | 分词/子命令/标志/类型恢复四阶段实现（文件内匿名命名空间承载辅助函数） |
| Test | `cli_tool_framework/test/unittest/tool_util_test/tool_util_test.cpp` | 最小 Red 集：一种长标志写法 + 一条拒绝分支（全量在 TASK-5） |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `tool_util.cpp` 的 `ValidateProperties`/`ValidateInputSchemaProperties`/`GenerateSandboxConfig`/`GenerateCmdSandboxConfig` | 既有校验与沙箱语义，`execTool` 与 shell 路径共用，改动会扩大影响面 |
| `cli_tool_manager_service.cpp` | TASK-4 所有 |
| `interfaces/cli_tool/**` | TASK-2 所有 |

**Spec References**

- AC / Rules: `AC-1.4`, `AC-1.6`, `AC-1.7`, `R-5`, `R-7`, `R-8`, `R-10`, `BF-5`, `BF-6`
- Anchors: `spec.md#规则定义`, `spec.md#api-变更分析`（「行为事实与约束」表：BF-5, BF-6）, `spec.md#验证映射`（VM-4, VM-6, VM-7）

**Design References**

- IDs / Anchors: `ADR-3`, `ADR-5`, `INV-7`, `design.md#算法与状态机`, `design.md#既有模式复用`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must：解析在服务端、以纯函数形态实现，输出可被既有校验消费。Must Not：依赖 SA 实例、IPC 或全局状态 |
| OH-ARCH-ERROR-LOG | Must：失败返回既有内部码并写 `detail` 原因分类。Must Not：新增错误码、在 `detail` 或日志中回显命令原文 |
| OH-ARCH-SUBSYSTEM | Must：array/类型语义与既有 `ValidateInputSchemaProperties` 对齐。Must Not：另建一套 schema 校验体系 |

**Steps**

- [ ] 写失败测试：`ParseToolCommand_KeyEqualsValue` 与 `ParseToolCommand_UnknownFlag` 两条最小用例（Red：符号不存在，编译失败即为 Red 证据）
- [ ] 运行 `run -t UT -ts tool_util_test`，记录 Red
- [ ] 做最小实现：分词 → 工具名/子命令 → 标志解析 → 类型恢复，四阶段各自失败即返回
- [ ] 运行同一命令确认两条用例 Green，且既有 `ToolUtil` 用例（`ValidateExecOptionsProperties_0100`..`_0800`、`GenerateCmdSandboxConfig_*`、`TransferToCmdParam_0100`）不回归
- [ ] 回填 AC 追溯（AC-1.4/AC-1.6/AC-1.7 行）、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `tool_util.cpp:116-171 / ValidateInputSchemaProperties` | schema `properties` 读取方式、`help` 例外、类型判定入口 | 无（解析产物必须能被该函数二次校验通过） |
| `tool_util.cpp:914 / ExpandArgsFromJson` | 从字符串还原 `WantParams` 的产物形态与错误返回约定 | 偏差：输入为命令行语法而非 JSON，需自建分词器（`design.md#既有模式复用` 记录 no prior art） |
| `tool_util.cpp:284 / TransferToCmdParam` | `WantParams → execArgs` 的下游消费方式 | 只读参考，确保解析产物可被直接消费 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.7 正常路径（四种写法 + 单引号）+ AC-1.6 异常路径（schema 外标志、类型失败、重复键）+ AC-1.4 子命令路径三类证据 |
| Scope respected | diff 仅 `tool_util.h/.cpp` 新增函数与匿名命名空间辅助函数 + 一个测试文件；既有函数零 diff |
| State lifecycle complete | INV-7：解析产物的键集合与类型有断言；失败路径断言不产出半结果（`param.args` 为空） |
| Interface contract matched | 函数签名与错误码集合与 TASK-4 的 Consumes 一致 |
| Relevant verification | `tool_util_test` 直接覆盖解析行为 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -ts tool_util_test` | 新增用例 PASS；既有 `ToolUtil` 用例全 PASS | 实施后回填 |
| 静态检查 | `./build.sh --product-name rk3568 --build-target ability_runtime` | 编译通过，无新增 warning | 实施后回填 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-3：命令字符串 → `ExecToolParam` 的服务端纯函数解析器（长标志子集） |
| 允许修改 | `cli_tool_framework/services/climgr/include/tool_util.h`、`src/tool_util.cpp`（新增函数与其匿名命名空间辅助函数）、`tool_util_test.cpp` |
| 允许新建 | 无（辅助函数置于 `tool_util.cpp` 匿名命名空间，不新增文件，故不触及 BUILD.gn） |
| 只读参考 | `tool_info.h`、`sub_command_info.h`、`exec_tool_param.h`、`tool_util.cpp` 既有校验函数、`spec.md#规则定义` |
| Spec 引用 | `AC-1.4`, `AC-1.6`, `AC-1.7`, `R-5`, `R-7`, `R-8`, `R-10`, `BF-5`, `BF-6` |
| Design 引用 | `ADR-3`, `ADR-5`, `INV-7`, `design.md#算法与状态机` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -ts tool_util_test` / 期望: PASS |
| 完成规则 | 不得修改既有校验/沙箱函数；不得扩展支持语法；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.4/1.6/1.7 → TASK-3 → `tool_util.cpp:ParseToolCommand` → Commit；确认支持/拒绝语法集合与 spec R-7/R-8 一致，无静默扩展 |
| Code Quality | 纯函数性（无全局状态）、`detail` 不含命令原文、匿名命名空间辅助函数命名与既有风格一致 |
| Verification | VM-4/VM-6/VM-7 的 Expected/Actual、既有用例回归结果 |

### TASK-4: 服务端分流、白名单与会话建立

| 字段 | 内容 |
|------|------|
| 任务目标 | 在 `ExecCmd` 增加分流点；工具命令处理完成白名单校验、工具查表、解析调用、装配 `ExecToolParam`（只搬 `options` 与 `challenge`）、复用 `ValidateAndPrepareTool`，并按「先订阅后创建进程」建立会话，任一步失败反向回滚且不留残 |
| AC 映射 | AC-1.1, AC-1.2, AC-1.3, AC-1.9, AC-1.10 |
| 前置依赖 | TASK-2（`ExtractToolName` 与字段契约）、TASK-3（`ParseToolCommand` 签名与错误码） |
| 非目标 | 不修改 shell 分支逻辑；不修改权限门/会话上限/`ValidateAndPrepareTool`/`GenerateSandboxConfig`；不接入 `workDir`/`env`/`policy`（ADR-8）；不新增会话类型或状态枚举值 |
| 完成判据 | ①工具模式成功路径返回 `ERR_OK` 且 `CliSessionInfo.toolName` 为命令首 token、`status` 为 `"running"`、`sessionId` 非空；②白名单失败 → `ERR_INVALID_PARAM` 且查表与进程创建调用次数为 0；③未注册工具 → `ERR_TOOL_NOT_EXIST`；④会话达上限 → `ERR_SESSION_LIMIT_EXCEEDED`（校验先于分流）；⑤订阅/进程/监控三步分别失败 → 对应错误码 + 无残留会话 + 无孤儿进程组 + 订阅被反注册；⑥缺省与显式 `true` 走 shell 且结果与既有行为一致；⑦失败点均调用既有 DFX 上报 |
| 停止条件 | 若必须修改 `ValidateAndPrepareTool`/`GenerateSandboxConfig`/权限门才能完成 → 停止，回 `design.md` ADR-3 修订；若发现 `workDir`/`env`/`policy` 需接入 → 停止，回 `spec.md` R-16 与 proposal 变更控制 |
| Design 状态/不变量 | STATE-3, STATE-4, STATE-5, INV-2, INV-3, INV-4, INV-5, INV-6 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| `STATE-3 / INV-3` | 工具命令会话记录 | `CliToolManagerService::sessionRecords_` | `sessionId = GenerateCliSessionId(toolName, record)` | 会话建立第 1 步（`CreateSessionRecord` + `AddSessionRecord`） | 查询接口、IO 监控、超时任务只读；状态迁移由既有路径写 | 正常：进程退出既有收尾；异常：任一步失败即 `RemoveSessionRecord` | 失败后不得可查询到该会话 |
| `STATE-4 / INV-2` | 事件订阅与调度器 | `EventDispatcher` | `(sessionId, subscriptionId, callerPid, callerUid)` | 入口 `SetScheduler`；会话建立第 2 步 `SubscribeSession` | 事件投递路径读写 | 进程创建或监控注册失败 → `UnregisterSubscriber` | 订阅成功之前不得创建子进程 |
| `STATE-5 / INV-3` | 工具子进程与管道 | `ProcessManager`（创建）/ `SessionRecord.processId`（记账） | `record->processId` | 会话建立第 3 步 `CreateChildProcess` | IO 监控、超时任务、reaper | 监控注册失败 → `Killpg`；退出 → 既有 reaper | 白名单/查表/解析/校验未全部通过前不得创建进程（INV-4） |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `int32_t CliToolManagerService::ExecCmdToolMode(const ExecCmdParam &, CmdSessionContext)` 与 `int32_t CliToolManagerService::SetupCmdSession(const ExecToolParam &, const ToolInfo &, const std::string &, const std::string &, const CmdSessionContext &)`（私有） | TASK-4 | TASK-6（用例按此路径断言） | 私有方法，不进入 IDL/头文件对外接口；caller 上下文以结构体聚合传递，避免 8+ 参数列表 |
| Produces | 工具模式失败点 DFX 上报：`ReportCliExecuteFailed(bundleName, toolName, reason[, detail])`，`reason` 取既有 `GetFailureReason(内部码)`，工具名非法时用原因串 `invalid_tool_name` | TASK-4 | TASK-6 | 不新增 `hisysevent.yaml` 事件定义；`detail` 不含命令原文 |
| Consumes | `ExecCmdParam::ExtractToolName`、eventKey 契约 | TASK-2 | 前置 | 工具名判定必须与服务端白名单使用同一提取结果 |
| Consumes | `ToolUtil::ParseToolCommand(cmd, toolInfo, param, detail)` 及其错误码集合 | TASK-3 | 前置 | 错误码原样向上传递，由既有映射表转对外码 |
| Consumes | `ValidateAndPrepareTool`、`CreateSessionRecord`、`SubscribeSession`、`RegisterSessionWithMonitors`、`ProcessManager::CreateChildProcess/Killpg`、`EventDispatcher::UnregisterSubscriber` | 既有代码 | 无 | 只调用不修改 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp`（`ExecCmd`、`ExecTool`、`ValidateAndPrepareTool`、`SetupAndStartSession`、`CreateSessionRecord`、`SubscribeSession`、`RegisterSessionWithMonitors`） | 分流点位置、既有会话建立与回滚范式 |
| `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 私有方法声明位置与既有成员 |
| `cli_tool_framework/services/climgr/include/process_manager.h`、`event_dispatcher.h`、`session_record.h` | 子进程/事件/会话记录的接口与状态枚举 |
| `cli_tool_framework/services/common/include/cli_event_report.h` | 失败上报入口与原因分类函数 |
| `spec.md#验证映射`（VM-1, VM-2, VM-3, VM-4, VM-10, VM-12, VM-13）、`design.md#数据流控制流`（步骤 5-14） | 验收口径、Red 条件与调用次序 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 新增 caller 上下文聚合结构与两个私有方法声明 |
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 分流点 + 工具命令处理 + 会话建立与回滚 + DFX 上报 |
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/cli_tool_mgr_service_test.cpp` | 最小 Red 集：白名单拒绝 + 缺省 shell 守护（全量在 TASK-6） |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `cli_tool_manager_service.cpp` 的 shell 分支、`ValidateAndPrepareCmd`、`ValidateExecToolPermissions`、`ValidateSessionLimit`、`ValidateAndPrepareTool`、`SetupAndStartSession` | 两路径共用的安全与既有语义，AC-1.2/AC-1.10 与 INV-6 要求不变 |
| `tool_util.cpp`、`process_manager.cpp`、`event_dispatcher.cpp`、`cli_tool_data_manager.cpp` | TASK-3 所有 / 既有能力层复用不修改 |
| `hisysevent.yaml`、`cli_error_code.h`、`cli_manager_error_utils.h` | 已发布事件与错误码，AGENTS.md 禁止项 |
| `services/sa_profile/186.json` | SA 注册配置不变（AGENTS.md「Ask before」项） |

**Spec References**

- AC / Rules: `AC-1.1`, `AC-1.2`, `AC-1.3`, `AC-1.9`, `AC-1.10`, `R-1`, `R-2`, `R-3`, `R-4`, `R-5`, `R-13`, `R-14`, `R-15`, `R-16`, `BF-1`, `BF-3`, `BF-4`
- Anchors: `spec.md#用户故事`, `spec.md#规则定义`, `spec.md#架构约束`, `spec.md#验证映射`

**Design References**

- IDs / Anchors: `ADR-3`, `ADR-4`, `ADR-8`, `STATE-3`, `STATE-4`, `STATE-5`, `INV-2`, `INV-3`, `INV-4`, `INV-5`, `INV-6`, `design.md#数据流控制流`, `design.md#资源所有权矩阵`, `design.md#线程与并发模型`, `design.md#安全基础检查`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must：分流点在权限门/调度器/会话上限之后。Must Not：在工具模式内另建权限或上限判定 |
| OH-ARCH-SUBSYSTEM | Must：工具信息只经 `CliToolDataManager` 获取；子进程只经 `ProcessManager` 创建。Must Not：调用其他 SA 内部方法 |
| OH-ARCH-ERROR-LOG | Must：`cmd` 以 `%{private}s` 输出；失败点接入既有上报；`challenge` 不入日志。Must Not：新增错误码或事件定义 |
| 安全（design#安全基础检查） | Must：白名单先于查表与进程创建；沙箱由 `GenerateSandboxConfig` 生成。Must Not：绕过 `ValidateAndPrepareTool`、以服务端身份直接执行命令 |

**Steps**

- [ ] 写失败测试：`ExecCmd_ToolMode_InvalidToolName_0100`（`"/system/bin/ohos-aa start"` 断言 `ERR_INVALID_PARAM` 且查表/进程 mock 调用 0 次）与 `ExecCmd_DefaultShellDispatch_0100`（缺省模式断言不返回 401）；运行确认 Red
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`，记录 Red 输出
- [ ] 做最小实现：上下文结构体 + 分流点 + 白名单 + 查表 + 解析调用 + 装配 + 复用校验 + 会话建立与反向回滚 + DFX 上报
- [ ] 运行同一命令确认 Green，且 `ExecCmd_0100`..`ExecCmd_0700`、`ExecTool_*` 既有用例全 Green
- [ ] 补故障注入用例（订阅/进程/监控三步失败）并确认无残留会话与孤儿进程组
- [ ] 回填 AC 追溯（AC-1.1/1.2/1.3/1.9/1.10 行）、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `cli_tool_manager_service.cpp:955-989 / SetupAndStartSession` | 会话记录构造、监控注册失败时 `Killpg` + 移除记录 | 偏差：采用 shell 路径次序（先 `AddSessionRecord` + `SubscribeSession` 再 `CreateChildProcess`），因 `ExecCmd` 带 `subscriptionId`，需在进程产生输出前完成订阅（design ADR-4 / INV-2） |
| `cli_tool_manager_service.cpp:1088-1165 / ExecCmd`（shell 分支） | 分流点位置、`SessionRecord` 填充、`RegisterAppStateObserver` 与 `background` 回复时机 | 无（工具模式复用同一收尾语义） |
| `cli_tool_manager_service.cpp:917-936 / ValidateAndPrepareTool` | 查表 → 参数/选项/schema 校验 → 沙箱生成 | 直接调用，不拆解 |
| `cli_event_report.h:71-108 / ReportCliExecuteFailed, GetFailureReason` | 失败上报的入参与原因分类 | 扩展：新增原因串 `invalid_tool_name`（既有原因枚举不覆盖白名单失败） |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.1 正常（成功路径 + `CliSessionInfo` 三字段）+ AC-1.3/1.4/1.9 异常与边界 + AC-1.2/1.10 防回退 + R-15 恢复路径，五类证据齐备 |
| Scope respected | diff 仅服务侧头文件与实现（新增分流点、两个私有方法、上下文结构体）+ 一个测试文件；shell 分支与共用校验函数零 diff |
| State lifecycle complete | STATE-3/4/5 的创建、读取、更新、清理与回滚在正常与三步失败场景均有断言（INV-2/INV-3） |
| Interface contract matched | 消费的 `ExtractToolName` 与 `ParseToolCommand` 签名与 TASK-2/TASK-3 Produces 完全一致 |
| Relevant verification | `cli_tool_mgr_service_test` 直接覆盖分流、白名单、上限、成功与回滚路径 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -ts cli_tool_mgr_service_test` | 新增工具模式用例 PASS；`ExecCmd_0100`..`_0700` 与 `ExecTool_*` 既有用例全 PASS | 实施后回填 |
| 测试 | `run -t UT -ts tool_util_test` | 既有校验用例不回归（确认未误改共用函数） | 实施后回填 |
| 静态检查 | `./build.sh --product-name rk3568 --build-target ability_runtime` | 编译通过，无新增 warning；`cmd` 日志格式抽查为 `%{private}s` | 实施后回填 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-4：服务端分流 + 白名单 + 复用校验/沙箱 + 会话建立与反向回滚 |
| 允许修改 | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h`、`src/cli_tool_manager_service.cpp`（新增分流点、上下文结构体、两个私有方法）、`cli_tool_mgr_service_test.cpp` |
| 允许新建 | 无 |
| 只读参考 | `ExecCmd`/`ExecTool`/`ValidateAndPrepareTool`/`SetupAndStartSession`/`CreateSessionRecord`/`SubscribeSession`、`process_manager.h`、`event_dispatcher.h`、`session_record.h`、`cli_event_report.h`、`design.md#数据流控制流` |
| Spec 引用 | `AC-1.1`, `AC-1.2`, `AC-1.3`, `AC-1.9`, `AC-1.10`, `R-1`..`R-5`, `R-13`..`R-16`, `BF-1`, `BF-3`, `BF-4`, `spec.md#架构约束` |
| Design 引用 | `ADR-3`, `ADR-4`, `ADR-8`, `STATE-3`, `STATE-4`, `STATE-5`, `INV-2`..`INV-6`, `design.md#资源所有权矩阵`, `design.md#安全基础检查` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -ts cli_tool_mgr_service_test` / 期望: PASS |
| 完成规则 | 不得修改 shell 分支与共用校验/沙箱函数；不得接入 `workDir`/`env`/`policy`；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.1/1.2/1.3/1.9/1.10 → TASK-4 → `cli_tool_manager_service.cpp:ExecCmd/ExecCmdToolMode/SetupCmdSession` → Commit；确认无 spec 之外的行为（如 options 三项接入） |
| Code Quality | 分流点位置、回滚次序、锁使用（不在持 `sessionsMutex_` 时创建进程）、日志脱敏、diff 范围 |
| Verification | VM-1/VM-2/VM-3/VM-4/VM-10/VM-12/VM-13 的 Expected/Actual、故障注入结果、既有用例回归 |

### TASK-5: 解析层用例补齐

| 字段 | 内容 |
|------|------|
| 任务目标 | 把 `spec.md#验证映射` VM-6/VM-7/VM-13 中解析层与沙箱配置层的用例补齐至全量并全绿 |
| AC 映射 | AC-1.4, AC-1.6, AC-1.7 |
| 前置依赖 | TASK-3 |
| 非目标 | 不修改解析实现（发现缺陷则回传 TASK-3 并记录偏差）；不新增测试目标 |
| 完成判据 | 10 条 `ParseToolCommand_*` 用例（`KeyEqualsValue`/`KeySpaceValue`/`BooleanFlag`/`BooleanFalse`/`SingleQuoteSpace`/`SubCommandSchema`/`UnknownFlag`/`UnknownSubCommand`/`EmptyCmd`/`ShortFlag`/`UnclosedQuote`/`RepeatNonArrayFlag`/`TypeMismatch` 中按 VM-6/VM-7 列出的全部）+ 1 条工具沙箱配置断言用例全绿；每条用例的断言与 VM 表「通过标准」逐字对应 |
| 停止条件 | 若某条用例无法在不改实现的前提下 Green → 停止，记录为实现缺陷并回传 TASK-3 |
| Design 状态/不变量 | INV-7 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| `INV-7` | 解析产物键集合与类型 | 用例断言（看护） | 用例名 → VM 编号 | 用例构造 `ToolInfo` + `inputSchema` | 断言 `ExecToolParam.args` | 用例结束自动释放 | 每条 VM 通过标准都有对应断言，无孤立用例 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `tool_util_test.cpp` 内 `ParseToolCommand_*` 与 `GenerateSandboxConfig_NoCallerWorkDir_0100` 用例集（对应 VM-6/VM-7/VM-13） | TASK-5 | Review 阶段 Verification 证据消费 | 用例名与 VM 表一致，便于 AC→用例追溯 |
| Consumes | `ToolUtil::ParseToolCommand` 签名与错误码集合；支持/拒绝语法集合 | TASK-3 | 前置 | 不得为通过测试而放宽实现语义 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `spec.md#验证映射`（VM-6, VM-7, VM-13） | 测试入口、Red 条件与通过标准 |
| `cli_tool_framework/test/unittest/tool_util_test/tool_util_test.cpp`（`ToolUtilTest` fixture 与既有用例写法） | fixture 复用与断言风格 |
| `cli_tool_framework/services/climgr/include/tool_info.h` | 构造带子命令与 schema 的 `ToolInfo` |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/tool_util_test/tool_util_test.cpp` | 补齐 VM-6/VM-7/VM-13 用例 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `tool_util.cpp`/`tool_util.h` | TASK-3 所有；本 Task 只补用例 |
| `test/unittest/tool_util_test/BUILD.gn` | 无新增测试文件，不需改构建定义 |

**Spec References**

- AC / Rules: `AC-1.4`, `AC-1.6`, `AC-1.7`, `R-5`, `R-7`, `R-8`, `R-16`, `BF-3`, `BF-5`, `BF-6`
- Anchors: `spec.md#验证映射`, `spec.md#规则定义`

**Design References**

- IDs / Anchors: `ADR-5`, `ADR-8`, `INV-7`, `design.md#测试性设计`, `design.md#算法与状态机`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：断言精确内部码（`ERR_INVALID_PARAM`/`ERR_TOOL_NOT_EXIST`）。Must Not：以「非 ERR_OK 即通过」放宽断言 |
| 测试规范（仓内） | Must：沿用 `HWTEST_F(ToolUtilTest, <Name>, TestSize.Level1/2)` 与既有 `@tc.name`/`@tc.desc` 注释格式。Must Not：为通过测试删除或跳过既有用例 |

**Steps**

- [ ] 按 VM-6/VM-7/VM-13 逐条列出用例名、输入、期望码，与 TASK-3 的支持/拒绝语法集合核对
- [ ] 写用例并运行 `run -t UT -ts tool_util_test`，确认新增用例在实现语义正确时 Green；若 Red，判定为实现缺陷并回传 TASK-3
- [ ] 补齐沙箱配置断言用例（工具路径不含 `workdir`/`policy`/调用方 `env`，shell 路径三项均写入）
- [ ] 运行 `run -t UT -ts tool_util_test` 全量确认 Green
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `tool_util_test.cpp:1692-1860 / ValidateExecOptionsProperties_0100.._0800` | 一条用例一个边界、`@tc.desc` 说明意图、断言精确错误码 | 无 |
| `tool_util_test.cpp:1864-1970 / GenerateCmdSandboxConfig_0100.._0400` | 沙箱配置内容断言方式（JSON 字段逐项校验） | 扩展：新增工具路径的反向断言（不含三项） |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.4/1.6/1.7 的正常、异常、边界三类用例齐备且断言与 VM 通过标准逐字对应 |
| Scope respected | diff 仅一个测试文件 |
| State lifecycle complete | INV-7 的键集合/类型/失败不产半结果均有断言 |
| Interface contract matched | 用例调用的函数签名与 TASK-3 Produces 一致 |
| Relevant verification | `tool_util_test` 全量运行结果 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -ts tool_util_test` | 新增用例全 PASS；既有用例全 PASS | 实施后回填 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-5：解析层与沙箱配置层用例补齐至 VM-6/VM-7/VM-13 全量 |
| 允许修改 | `cli_tool_framework/test/unittest/tool_util_test/tool_util_test.cpp` |
| 允许新建 | 无 |
| 只读参考 | `spec.md#验证映射`、`tool_util_test.cpp` 既有用例、`tool_info.h` |
| Spec 引用 | `AC-1.4`, `AC-1.6`, `AC-1.7`, `R-5`, `R-7`, `R-8`, `R-16`, `BF-3`, `BF-5`, `BF-6` |
| Design 引用 | `ADR-5`, `ADR-8`, `INV-7`, `design.md#测试性设计` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -ts tool_util_test` / 期望: PASS |
| 完成规则 | 不得修改实现文件；发现缺陷回传 TASK-3 并记录偏差；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | VM-6/VM-7/VM-13 → 用例名 → 断言与通过标准逐条对照 |
| Code Quality | 用例命名与注释规范、无 `EXPECT_TRUE(true)` 式空断言 |
| Verification | `tool_util_test` 全量 fresh 输出与日志路径 |

### TASK-6: 服务层与客户端层用例补齐

| 字段 | 内容 |
|------|------|
| 任务目标 | 把 VM-1/VM-2/VM-3/VM-4/VM-5/VM-8/VM-10/VM-12/VM-13 中服务侧与客户端侧的用例补齐至全量并全绿，含故障注入与防回退守护 |
| AC 映射 | AC-1.1, AC-1.2, AC-1.3, AC-1.4, AC-1.5, AC-1.9, AC-1.10 |
| 前置依赖 | TASK-4 |
| 非目标 | 不修改实现；不新增测试目标或 mock 文件（复用既有 fixture 与 `common_mock/climgr`） |
| 完成判据 | 成功路径、白名单（含 `/`、`.`、`;`、`$`、空格）、未注册工具、未知子命令、空白命令（客户端不发 IPC）、会话上限、缺省 shell 守护、显式 shell、三步失败回滚、options 语义十类用例全绿；权限门既有用例（`ExecCmd_0100`..`_0700`）不回归 |
| 停止条件 | 若回滚类用例需要新增 mock 能力才能注入失败 → 停止，先修订 Plan 的 Files 与「受影响文件全量清单」 |
| Design 状态/不变量 | INV-1, INV-2, INV-3, INV-4, INV-6 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| `STATE-3 / INV-3` | 会话记录（用例视角） | 用例 SetUp/TearDown | `sessionId` | SetUp 预置或调用产生 | 断言查询结果 | TearDown 清理预置记录，避免污染上限计数 | 上限类用例必须精确控制预置记录数（默认上限 8） |
| `STATE-4 / INV-2` | 订阅（用例视角） | mock `EventDispatcher` | 订阅键 | 调用产生 | 断言注册/反注册调用次数 | TearDown 重置 mock 计数 | 订阅注册必须先于进程创建（调用序断言） |
| `STATE-5 / INV-3` | 子进程（用例视角） | mock `ProcessManager` | `processId` | 调用产生 | 断言创建/`Killpg` 调用次数 | TearDown 重置 | 拒绝路径下创建调用次数必须为 0 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `cli_tool_mgr_service_test.cpp` 的 `ExecCmd_ToolMode_*`/`ExecCmd_DefaultShellDispatch_0100`/`ExecCmd_ExplicitShellMode_0100` 用例集 + `cli_tool_mgr_client_test.cpp` 的工具模式前置校验用例集 | TASK-6 | Review 阶段 Verification 证据消费 | 用例名与 VM 表一致 |
| Consumes | `ExecCmdToolMode`/`SetupCmdSession` 路径、`ExtractToolName`、客户端前置校验 | TASK-2, TASK-4 | 前置 | 不得为通过测试修改实现语义 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `spec.md#验证映射`（VM-1..VM-5, VM-8, VM-10, VM-12, VM-13） | 入口、Red 条件、通过标准 |
| `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/`（fixture、`mock/src/ipc_skeleton.cpp`、`mock/src/accesstoken_kit.cpp`、`permission_util_mock.cpp`、`IsPermissionGateResult`） | 权限门与 caller 上下文的构造方式 |
| `cli_tool_framework/test/unittest/common_mock/climgr/include/`（`process_manager.h`、`event_dispatcher.h`、`cli_tool_data_manager.h`、`io_monitor.h`） | 可注入失败的 mock 能力 |
| `cli_tool_framework/test/unittest/cli_tool_mgr_client_test/mock/include/mock_cli_tool_mgr_service.h` | 断言 proxy 调用次数的方式 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/cli_tool_mgr_service_test.cpp` | 服务侧用例补齐 |
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_client_test/cli_tool_mgr_client_test.cpp` | 客户端侧用例补齐 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `services/climgr/**`、`interfaces/cli_tool/**`、`frameworks/js/napi/**` | 实现文件属 TASK-1..TASK-4 |
| `common_mock/**`、`mock/**` | 若确需扩展 mock 能力，先修订 Plan（停止条件） |

**Spec References**

- AC / Rules: `AC-1.1`, `AC-1.2`, `AC-1.3`, `AC-1.4`, `AC-1.5`, `AC-1.9`, `AC-1.10`, `R-1`..`R-6`, `R-13`..`R-16`, `BF-1`, `BF-3`, `BF-4`
- Anchors: `spec.md#验证映射`, `spec.md#用户故事`, `spec.md#架构约束`

**Design References**

- IDs / Anchors: `ADR-3`, `ADR-4`, `ADR-8`, `INV-1`, `INV-2`, `INV-3`, `INV-4`, `INV-6`, `design.md#测试性设计`, `design.md#线程与并发模型`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：断言精确内部码；权限门用例允许 `IsPermissionGateResult` 兜底（既有约定）。Must Not：以「非 ERR_OK 即通过」放宽其他断言 |
| 测试规范（仓内） | Must：TearDown 清理预置会话记录与 mock 计数。Must Not：删除或跳过既有用例、修改既有用例断言以适配新行为 |

**Steps**

- [ ] 按 VM 表逐条列出用例名、输入、mock 预置、期望码与调用次数断言
- [ ] 写成功路径与白名单用例，运行 `run -t UT -ts cli_tool_mgr_service_test` 确认 Green
- [ ] 写三步失败回滚用例（分别注入订阅/进程/监控失败），断言错误码 + 无残留会话 + `Killpg`/`UnregisterSubscriber` 调用
- [ ] 写客户端空白命令与 eventKey 用例，运行 `run -t UT -ts cli_tool_mgr_client_test` 确认 Green（proxy 调用次数为 0）
- [ ] 运行两个测试目标全量 + `run -t UT -ts exec_options_test` 确认无回归
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `cli_tool_mgr_service_test.cpp:2242-2400 / ExecCmd_0100.._0700` | 权限门用例的 caller 构造与 `IsPermissionGateResult` 兜底断言 | 无（新用例沿用同一兜底以适配 mock 权限门） |
| `cli_tool_mgr_service_test.cpp` 中 `ExecTool_*` 用例 | 工具查表 mock、会话记录预置与上限构造方式 | 无 |
| `cli_tool_mgr_client_test.cpp:409-545 / ExecCmd_0100.._0600` | 客户端回调/订阅断言与 proxy mock 使用方式 | 扩展：新增「proxy 调用次数为 0」断言以证明前置拒绝 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 七条 AC 的正常/异常/边界/恢复四类路径均有用例；VM-1/VM-8/VM-11 三条守护用例 Green |
| Scope respected | diff 仅两个测试文件 |
| State lifecycle complete | STATE-3/4/5 在用例中的创建、断言与 TearDown 清理齐备；INV-2 的调用序断言存在 |
| Interface contract matched | 用例调用的方法签名与 TASK-2/TASK-4 Produces 一致 |
| Relevant verification | 两个测试目标 + `exec_options_test` 回归结果 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -ts cli_tool_mgr_service_test` | 新增用例全 PASS；`ExecCmd_0100`..`_0700` 与 `ExecTool_*` 全 PASS | 实施后回填 |
| 测试 | `run -t UT -ts cli_tool_mgr_client_test` | 前置拒绝用例 PASS（proxy 调用 0 次）；既有用例 PASS | 实施后回填 |
| 测试 | `run -t UT -tp ability_runtime` | 部件全量单测无新增失败 | 实施后回填 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-6：服务侧与客户端侧用例补齐（含故障注入与守护用例） |
| 允许修改 | `cli_tool_mgr_service_test.cpp`、`cli_tool_mgr_client_test.cpp` |
| 允许新建 | 无 |
| 只读参考 | `spec.md#验证映射`、服务侧 fixture 与 mock、`common_mock/climgr/include/*`、客户端 mock |
| Spec 引用 | `AC-1.1`..`AC-1.5`, `AC-1.9`, `AC-1.10`, `R-1`..`R-6`, `R-13`..`R-16`, `BF-1`, `BF-3`, `BF-4` |
| Design 引用 | `ADR-3`, `ADR-4`, `ADR-8`, `INV-1`..`INV-4`, `INV-6`, `design.md#测试性设计` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -ts cli_tool_mgr_service_test`、`run -t UT -ts cli_tool_mgr_client_test` / 期望: PASS |
| 完成规则 | 不得修改实现与 mock；需扩 mock 能力时停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | VM-1..VM-5/VM-8/VM-10/VM-12/VM-13 → 用例名 → 断言与通过标准逐条对照；确认无 spec 之外行为被固化 |
| Code Quality | 用例隔离性（TearDown 清理）、mock 计数断言、无空断言 |
| Verification | 三个测试命令的 fresh 输出与日志路径、部件全量回归结果 |

### TASK-7: 序列化层用例补齐

| 字段 | 内容 |
|------|------|
| 任务目标 | 把 VM-9 的序列化与工具名提取用例补齐至全量并全绿，固化 IPC 兼容契约 |
| AC 映射 | AC-1.8 |
| 前置依赖 | TASK-2 |
| 非目标 | 不修改 `ExecCmdParam` 实现；不测试服务端行为 |
| 完成判据 | ①完整 Parcel 往返七字段值一致（含 `isShellCommand = false` 与非空 `challenge`）；②只写到 `options` 的 Parcel 反序列化成功且回退 `true`/`""`；③`ExtractToolName` 对前导空白、无空白、空串、全空白四类输入返回预期值；④既有 `ExecCmdParam_Parcelable_*` 用例不回归 |
| 停止条件 | 若旧客户端 Parcel 反序列化返回 `nullptr` → 停止，回传 TASK-2（违反 INV-1/R-11） |
| Design 状态/不变量 | INV-1 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| `INV-1` | Parcel 尾部字段缺省回退 | 用例断言（看护） | 用例名 → VM-9 | 用例内构造 `Parcel` | 断言反序列化结果字段值 | 用例结束释放 `Parcel` 与返回对象（`delete`） | 尾部字段缺失时不得返回 `nullptr`；字段序不得被用例改写 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `exec_cmd_param_test.cpp` 的 `ExecCmdParam_Parcelable_*`/`_Unmarshalling_OldClient_0100`/`_ExtractToolName_0100.._0300` 用例集 | TASK-7 | Review 阶段 Verification 证据消费 | 用例名与 VM-9 一致 |
| Consumes | `ExecCmdParam` 字段序、缺省回退语义、`ExtractToolName` 签名 | TASK-2 | 前置 | 字段序冻结（spec 序列化契约表） |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `spec.md#验证映射`（VM-9）、`spec.md#兼容性声明` | IPC 双向兼容口径 |
| `design.md#数据模型设计`（序列化契约表） | 七字段序与读取失败处理 |
| `cli_tool_framework/test/unittest/exec_cmd_param_test/exec_cmd_param_test.cpp`、`exec_tool_param_test.cpp` | 既有 Parcel 用例写法与内存释放约定 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/exec_cmd_param_test/exec_cmd_param_test.cpp` | 补齐 VM-9 用例 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `interfaces/cli_tool/**` | TASK-2 所有 |
| `ICliToolManager.idl` | 接口签名不变 |

**Spec References**

- AC / Rules: `AC-1.8`, `R-11`, `R-12`, `BF-1`
- Anchors: `spec.md#兼容性声明`, `spec.md#验证映射`（VM-9）

**Design References**

- IDs / Anchors: `ADR-1`, `ADR-2`, `STATE-1`, `STATE-2`, `INV-1`, `design.md#数据模型设计`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-IPC-SAF | Must：断言字段序与缺省回退。Must Not：在用例中构造与 spec 序列化契约表不同的字段序 |
| 测试规范（仓内） | Must：`Unmarshalling` 返回的裸指针在用例内 `delete`，避免泄漏。Must Not：以「不崩溃」代替字段值断言 |

**Steps**

- [ ] 按 VM-9 列出用例矩阵（往返、旧客户端缺省、提取边界）
- [ ] 写用例并运行 `run -t UT -ts exec_cmd_param_test`，确认 Green；若缺省回退断言 Red，回传 TASK-2
- [ ] 运行 `run -t UT -ts exec_tool_param_test` 与 `run -t UT -ts exec_options_test` 确认相邻序列化用例无回归
- [ ] 回填 AC 追溯（AC-1.8 行）、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `exec_cmd_param_test.cpp:40-95 / ExecCmdParam_Parcelable_0100/_0200` | Parcel 构造、往返断言、返回对象释放 | 扩展：新增只写到 `options` 的旧客户端场景 |
| `exec_tool_param_test.cpp` | 同类 `Parcelable` 的往返用例组织方式 | 无 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.8 的正向（完整往返）与边界（旧客户端缺省）两类证据；提取函数四类边界证据 |
| Scope respected | diff 仅一个测试文件 |
| State lifecycle complete | INV-1 的缺省回退断言 + 返回对象释放（无内存泄漏告警） |
| Interface contract matched | 断言的字段序与 spec 序列化契约表、TASK-2 Produces 完全一致 |
| Relevant verification | `exec_cmd_param_test` + 相邻序列化测试目标回归 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -ts exec_cmd_param_test` | 新增用例全 PASS；既有用例 PASS | 实施后回填 |
| 测试 | `run -t UT -ts exec_tool_param_test`、`run -t UT -ts exec_options_test` | 相邻序列化用例无回归 | 实施后回填 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-7：IPC 序列化与工具名提取用例补齐，固化兼容契约 |
| 允许修改 | `cli_tool_framework/test/unittest/exec_cmd_param_test/exec_cmd_param_test.cpp` |
| 允许新建 | 无 |
| 只读参考 | `spec.md#兼容性声明`、`design.md#数据模型设计`、`exec_tool_param_test.cpp` |
| Spec 引用 | `AC-1.8`, `R-11`, `R-12`, `BF-1`, `spec.md#兼容性声明` |
| Design 引用 | `ADR-1`, `ADR-2`, `STATE-1`, `STATE-2`, `INV-1`, `design.md#数据模型设计` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -ts exec_cmd_param_test` / 期望: PASS |
| 完成规则 | 不得修改实现与 IDL；缺省回退断言失败时回传 TASK-2；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.8 → TASK-7 → `exec_cmd_param_test.cpp` 用例名 → Commit；字段序与 spec 契约表一致性 |
| Code Quality | 内存释放、断言精确性、无「不崩溃即通过」式弱断言 |
| Verification | VM-9 的 Expected/Actual 与相邻测试目标回归结果 |

### TASK-8: 跨仓 System API 声明交付

| 字段 | 内容 |
|------|------|
| 任务目标 | 在 `interface_sdk-js` 中英双份 `.d.ts` 中新增 `isShellCommand?`/`challenge?` 声明，并把 `execCmd` 的 `@throws` 补为 201/202/401/35600030/35600031/35600050，使对外声明与运行时错误面一致 |
| AC 映射 | AC-1.1, AC-1.2, AC-1.10（声明层可发现性与错误面） |
| 前置依赖 | TASK-1（属性语义定稿）、TASK-4（运行时错误面定稿） |
| 非目标 | 不改 `execCmd` 签名与返回类型；不改既有属性语义；依 Q-S1 结论**不**为 `workDir`/`env`/`policy` 增加模式差异说明；不修改本仓任何文件 |
| 完成判据 | ①中英双份均含两个新属性，标注组为 `@syscap SystemCapability.Ability.AgentRuntime.Core` + `@systemapi` + `@stagemodelonly` + `@since 26.0.0 dynamiconly`，并标注 `@default true` / `@default ""`；②`@throws` 六项齐备且文案与运行时一致；③中英内容语义一致（仅语言差异）；④声明 PR 独立于主仓实现 PR 提交 |
| 停止条件 | 若运行时错误面在实现期发生变化（新增/减少错误码）→ 停止，先回 `spec.md#api-与错误码事实` 修订再改声明 |
| Design 状态/不变量 | N/A（纯声明交付，无运行时状态变更；理由：不涉及 STATE-*/INV-* 的创建、读取或清理） |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| `N/A` | N/A | N/A | N/A | N/A | N/A | N/A | N/A：本 Task 只交付声明文件，不引入或修改任何运行时状态（`design.md#状态归属与不变量` 未登记声明层状态） |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `ExecCmdOptions.isShellCommand?: boolean`（`@default true`）、`ExecCmdOptions.challenge?: string`（`@default ""`）声明 + `execCmd` 的六项 `@throws` | TASK-8 | 三方调用方与 API 评审消费 | 标注组必须与模块既有属性一致；中英双份同步；可选属性不得改变既有调用方编译结果 |
| Consumes | 运行时错误面（201/202/401/35600030/35600031/35600050）与属性缺省语义 | TASK-1, TASK-4 | 前置 | 声明不得超出或落后于运行时错误面 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `interface/sdk-js:api/@ohos.app.cli.cliManager.d.ts`（`ExecCmdOptions`、`execCmd`、`execTool` 的 `challenge` 注释） | 既有标注组、注释风格、`challenge` 语义表述可对齐 `execTool` |
| `interface/sdk-js:zh-cn/api/@ohos.app.cli.cliManager.d.ts` | 中文声明的对应位置与措辞风格 |
| `spec.md#api-变更分析`、`spec.md#api-与错误码事实` | 属性签名、默认值、错误码精确值与触发条件 |
| `design.md#api-签名kit-与权限` | Kit、权限、SysCap、`.d.ts` 落点 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `interface/sdk-js:api/@ohos.app.cli.cliManager.d.ts` | 新增两属性 + `@throws` 补充 |
| Modify | `interface/sdk-js:zh-cn/api/@ohos.app.cli.cliManager.d.ts` | 中文同步 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `ability_runtime` 仓内任何文件 | 本 Task 为跨仓声明交付，主仓实现由 TASK-1..TASK-7 完成 |
| `.d.ts` 中既有属性与既有 API 的签名/标注 | 超出范围，且属已发布公共声明（AGENTS.md「Ask before」项） |
| `prebuilts/ohos-sdk/**`、`out/**` 下的 `.d.ts` 副本 | 生成物/预置产物，不作为交付源 |

**Spec References**

- AC / Rules: `AC-1.1`, `AC-1.2`, `AC-1.10`, `R-3`, `R-16`, `R-17`, `BF-2`
- Anchors: `spec.md#api-变更分析`, `spec.md#api-与错误码事实`, `spec.md#兼容性声明`, `spec.md#事实冲突与澄清项`（Q-S1, Q-S3）

**Design References**

- IDs / Anchors: `ADR-6`, `ADR-7`, `ADR-8`, `design.md#api-签名kit-与权限`, `design.md#风险和开放问题`（跨仓交付节奏）

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-API-LEVEL | Must：保持 System API 标注组与 `@since` 一致；`@throws` 与运行时错误面一致。Must Not：把属性声明为 public、修改既有属性开放级别 |
| OH-ARCH-COMPONENT-BUILD | Must：中英双份同步提交。Must Not：只改一份导致声明不一致 |
| 文档规范（仓内） | Must：注释描述行为与默认值，不描述内部实现。Must Not：在声明注释中写入服务端方法名或内部错误码数值（35700xxx） |

**Steps**

- [ ] 对照 `spec.md#api-与错误码事实` 列出两属性的注释要点（语义、默认值、模式差异不涉及项）与六项 `@throws` 文案
- [ ] 修改英文 `.d.ts`：在 `ExecCmdOptions` 既有属性之后新增两属性（含标注组与 `@default`），并补 `execCmd` 的 `@throws`
- [ ] 修改中文 `.d.ts`：同步语义与措辞
- [ ] 双份 diff 核对：属性顺序、标注组、`@throws` 六项、中英一致性
- [ ] 运行 `./build.sh --product-name rk3568 --build-target ability_runtime` 确认主仓构建不受影响，并提交跨仓声明 PR
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `api/@ohos.app.cli.cliManager.d.ts`（`ExecCmdOptions.workDir`/`background`/`timeout` 注释块） | JSDoc 结构：一句行为描述 + `@default`（如适用）+ 标注组 | 无 |
| `api/@ohos.app.cli.cliManager.d.ts`（`execTool` 的 `challenge` 参数注释） | `challenge` 的语义表述（来自 access token manager 的唯一标识） | 复用表述，避免同一概念在两个 API 上语义漂移 |
| `api/@ohos.app.cli.cliManager.d.ts:405-409`（`execCmd` 既有 `@throws`） | 错误码文案与缩进风格 | 扩展：新增 401 与 35600030 两项（design ADR-6） |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.1/1.2/1.10 的声明层证据：两属性可见、缺省值标注正确、错误面六项齐备 |
| Scope respected | diff 仅两份 `.d.ts`；主仓零 diff；既有属性与签名零 diff |
| State lifecycle complete | N/A：无运行时状态变更（理由见「状态所有权和生命周期」） |
| Interface contract matched | 声明的默认值与 TASK-1 的 NAPI 缺省值一致（`true` / `""`）；`@throws` 与 TASK-4 的运行时错误码一致 |
| Relevant verification | 中英双份 diff 核对记录 + 主仓构建通过输出 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 静态检查 | 中英双份 `.d.ts` diff 核对（属性、标注组、`@throws` 六项） | 两份语义一致，标注组与既有属性相同，`@throws` 含 201/202/401/35600030/35600031/35600050 | 实施后回填 |
| 构建 | `./build.sh --product-name rk3568 --build-target ability_runtime` | 构建通过，主仓不受声明变更影响 | 实施后回填 |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-8：跨仓 `.d.ts` 中英双份声明交付 + `@throws` 补齐 |
| 允许修改 | `interface/sdk-js:api/@ohos.app.cli.cliManager.d.ts`、`interface/sdk-js:zh-cn/api/@ohos.app.cli.cliManager.d.ts` |
| 允许新建 | 无 |
| 只读参考 | 既有 `.d.ts`（`ExecCmdOptions`、`execCmd`、`execTool`）、`spec.md#api-与错误码事实`、`design.md#api-签名kit-与权限` |
| Spec 引用 | `AC-1.1`, `AC-1.2`, `AC-1.10`, `R-3`, `R-16`, `R-17`, `spec.md#api-变更分析`, `spec.md#事实冲突与澄清项`（Q-S1, Q-S3） |
| Design 引用 | `ADR-6`, `ADR-7`, `ADR-8`, `design.md#api-签名kit-与权限` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | 中英双份 diff 核对 + `./build.sh --product-name rk3568 --build-target ability_runtime` / 期望: PASS |
| 完成规则 | 不得修改主仓文件与既有声明；声明不得超出运行时错误面；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.1/1.2/1.10 → TASK-8 → 两份 `.d.ts` 差异 → 声明 PR 链接；确认未加模式差异说明（Q-S1 结论） |
| Code Quality | JSDoc 风格一致性、中英一致性、标注组完整性、无内部实现细节泄漏 |
| Verification | diff 核对记录、主仓构建输出、与运行时错误面的一致性核对 |

## 代码范围映射

> Plan 审批时填写计划范围；实施后当次会话回填实际范围与 Commit。本计划全部 Task 的实际范围均含代码文件，故归档时需三份 Approved Review Evidence（`evidence/reviews/spec-compliance.md`、`code-quality.md`、`verification.md`）。

| Task ID | 计划文件/符号 | 实际文件/符号 | 操作 | Commit | Spec Compliance | Code Quality | Verification | 偏差 |
|---------|---------------|---------------|------|--------|-----------------|--------------|--------------|------|
| TASK-SKELETON-1 | `exec_cmd_param.h:ExecCmdParam`、`exec_cmd_param.cpp:Marshalling/Unmarshalling`、`js_cli_manager_utils.cpp:UnwrapExecCmdOptions`、`cli_tool_mgr_client.cpp:ExecCmd`、`cli_tool_manager_service.h/.cpp:ExecCmd` | 实施后回填 | Modify | 实施后回填 | `evidence/reviews/spec-compliance.md`（实施后生成） | `evidence/reviews/code-quality.md`（实施后生成） | `evidence/reviews/verification.md`（实施后生成） | 实施后回填 |
| TASK-1 | `js_cli_manager_utils.cpp:UnwrapExecCmdOptions`、`cli_tool_mgr_client_test.cpp` | 实施后回填 | Modify | 实施后回填 | 同上 | 同上 | 同上 | 实施后回填 |
| TASK-2 | `exec_cmd_param.h/.cpp:ExtractToolName`、`cli_tool_mgr_client.cpp:ExecCmd`、`exec_cmd_param_test.cpp`、`cli_tool_mgr_client_test.cpp` | 实施后回填 | Modify | 实施后回填 | 同上 | 同上 | 同上 | 实施后回填 |
| TASK-3 | `tool_util.h:ParseToolCommand`、`tool_util.cpp:ParseToolCommand` 及匿名命名空间辅助函数、`tool_util_test.cpp` | 实施后回填 | Modify | 实施后回填 | 同上 | 同上 | 同上 | 实施后回填 |
| TASK-4 | `cli_tool_manager_service.h:CmdSessionContext/ExecCmdToolMode/SetupCmdSession`、`cli_tool_manager_service.cpp:ExecCmd/ExecCmdToolMode/SetupCmdSession`、`cli_tool_mgr_service_test.cpp` | 实施后回填 | Modify | 实施后回填 | 同上 | 同上 | 同上 | 实施后回填 |
| TASK-5 | `tool_util_test.cpp:ParseToolCommand_*`、`GenerateSandboxConfig_NoCallerWorkDir_0100` | 实施后回填 | Modify（测试） | 实施后回填 | 同上 | 同上 | 同上 | 实施后回填 |
| TASK-6 | `cli_tool_mgr_service_test.cpp:ExecCmd_ToolMode_*`/`ExecCmd_DefaultShellDispatch_0100`/`ExecCmd_ExplicitShellMode_0100`、`cli_tool_mgr_client_test.cpp:ExecCmd_0300` 等 | 实施后回填 | Modify（测试） | 实施后回填 | 同上 | 同上 | 同上 | 实施后回填 |
| TASK-7 | `exec_cmd_param_test.cpp:ExecCmdParam_Parcelable_*`/`_Unmarshalling_OldClient_0100`/`_ExtractToolName_*` | 实施后回填 | Modify（测试） | 实施后回填 | 同上 | 同上 | 同上 | 实施后回填 |
| TASK-8 | `interface/sdk-js:api/@ohos.app.cli.cliManager.d.ts:ExecCmdOptions/execCmd`、`zh-cn/api/@ohos.app.cli.cliManager.d.ts` | 实施后回填 | Modify（声明，跨仓） | 实施后回填（声明 PR 链接） | 同上 | 同上 | 同上 | 实施后回填 |

## 实现偏差和开放问题

| Task ID | 类型 | 问题/偏差 | 处理状态 | 回退能力/Owner |
|---------|------|-----------|----------|----------------|
| TASK-4 | spec | 若实现期发现 `workDir`/`env`/`policy` 必须接入工具沙箱才能满足业务（与 Q-S1 结论冲突） | Blocked（需先回 `spec.md` R-16 与 `proposal.md` 变更控制重新批准） | ohos-spec / 需求提出人 zhoulanzhi |
| TASK-1 | design | 若缺省 `timeout` 需统一为单一值（与 Q-S2 结论冲突），必须修改 `UnwrapExecOptions`，将同时影响 `execTool` | Blocked（需先回 `design.md` ADR-7 与 `spec.md` R-17 修订） | ohos-design / CLI 工具框架负责人 |
| TASK-6 | dependency | 三步失败回滚用例可能需要扩展 `common_mock/climgr` 的 mock 能力（新增可注入失败点） | Blocked（需先修订本 Plan 的 Files 与「受影响文件全量清单」） | ohos-plan / CLI 工具框架 |
| TASK-8 | scope | 跨仓声明 PR 与主仓实现 PR 的合入顺序（先后不一致会造成声明与运行时短期不一致，spec Q-S3） | Resolved（结论：主仓实现定稿后提交声明 PR，Phase-4 收口；发布前核对一致性） | CLI 工具框架 + interface_sdk-js 声明仓维护者 |
| 全部 | spec | `cmd` 字符串长度上限仍无校验（proposal Q-4 结论：与既有 `execTool`/`execCmd` 一致，暂时忽略） | Resolved（不在本计划范围内；后续如需长度策略须单独基线） | CLI 工具框架负责人 |
