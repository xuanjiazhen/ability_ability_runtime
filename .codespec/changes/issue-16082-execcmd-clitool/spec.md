# 特性规格

> 固化用户可见行为和验收标准。本文的「事实来源」分两类：**既有事实**（本特性之前就存在的接口、错误码、校验阈值、沙箱语义等）给出 `文件:行/符号`；**新增事实**标 `New` 并给出 `proposal.md` / `design.md`（ADR/INV）依据与落点文件。
>
> **THEN 边界：** 每条 AC 的 `THEN` 只描述终端用户或 System API / InnerAPI 边界可判定的结果（Promise resolve/reject、错误码、`CliSessionInfo` 字段、回调）；内部方法、调用顺序、数据结构、锁与回滚逻辑归 `design.md`。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | execCmd 工具命令模式（与 execTool 同源的受控工具命令执行通道） |
| 特性编号 | 20260626000935 |
| 所属 Epic | 无（独立特性；与 execTool 能力同源） |
| 优先级 | P0 |
| 目标版本 | OpenHarmony-7.1-Release（引用 `proposal.md` frontmatter `target_release.id`，非独立事实源） |
| SIG 归属 | Ability SIG（元能力运行时）— CLI 工具框架 |
| 状态 | Draft |
| 复杂度 | 标准（引用 `proposal.md`「基线信息」） |

## 本次变更范围（Delta）

> `manifest.lineage: new-on-legacy`，故填写本节。

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | `ExecCmdOptions.isShellCommand?: boolean`（System API，默认 `true`） | 模式开关：`false` 时 `cmd` 按已注册 CLI 工具命令字符串解析执行 |
| ADDED | `ExecCmdOptions.challenge?: string`（System API，默认 `""`） | 调用方从 access token manager 获取并透传给工具进程，服务端不生成、不改值 |
| ADDED | 工具命令字符串解析能力（长标志子集 + 单引号包裹含空格值 + schema 驱动类型恢复） | 新增行为面：产生 401 / 35600030 两类既有 shell 路径不会出现的错误 |
| ADDED | 工具名白名单校验（`isalnum ∪ {'-','_'}`） | 拦截路径分隔符与命令拼接字符，拒绝时不创建子进程 |
| MODIFIED | `cliManager.execCmd` 行为扩展（签名与返回类型不变） | 向后兼容：未传或显式传 `isShellCommand: true` 时行为与既有 shell 路径完全一致；`.d.ts` `@throws` 需补 401 与 35600030 |
| MODIFIED | `ExecCmdParam` IPC 序列化（InnerAPI）：Parcel 尾部追加 `isShellCommand`、`challenge` | 旧客户端不写尾部字段时服务端回退默认值；新客户端 → 旧服务端为「兼容但不等价」 |
| REMOVED | 无 | 不废弃任何既有 API、字段或错误码 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `.codespec/changes/issue-16082-execcmd-clitool/proposal.md` | Approved（2026-09-09，批准人 zhoulanzhi，baseline_digest `65909407…a3cf`） |
| 既有代码与声明 | `cli_tool_framework/**`（`interfaces/cli_tool`、`services/climgr`、`frameworks/js/napi/cli_tool_manager`）+ `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts` | 既有事实核对来源（接口签名、错误码、校验阈值、沙箱语义、NAPI 解包约定） |
| 设计 | `.codespec/changes/issue-16082-execcmd-clitool/design.md` | 新增事实（New）的落点与依据来源（ADR-1..ADR-8、INV-1..INV-7） |
| 旧模板参考 | `.codespec/changes/issue-16082-execcmd-clitool-legacy-template/spec.md` | 只读参考；与既有代码事实冲突处以本文为准（Q-S1/Q-S2 已按既有代码事实关闭，见「事实冲突与澄清项」） |

> 需求基线、不涉及项、受影响子系统与仓库详见 `proposal.md`，本文档不重复摘录。`design.md` 与本文档并行产出，互不依赖。

## 用户故事

> `WHEN` 为调用方（开发者/上层业务）可触发的输入；`THEN` 为 System API / InnerAPI 边界可观察结果。
> 各 AC 中的工具名（`ohos-aa` 等）为**已注册 CLI 工具的示例**，本规格适用于工具注册表中任意已注册工具。
> **AC 编号冻结**：沿用 `AC-1.1`..`AC-1.10` 编号（其中 `1` = 特性序号，非 US 序号），与 proposal「用户故事与 AC」逐条一致，不重编号（`design.md` 的 ADR/INV 与 `execution-plan.md` 的 Task 追溯均按该编号）。

### US-1: LLM 工具命令直通执行

**作为** AI/LLM 业务调用方（应用开发者），
**我想要** 将拼接好的工具命令字符串直接交给 `cliManager.execCmd` 执行，
**以便** 无需自行解析为结构化参数即可调用已注册 CLI 工具，一个接口统一覆盖 shell 命令与工具命令两类场景。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-1.1 | WHEN 系统应用且持有 `ohos.permission.EXEC_CLI_TOOL` 的调用方执行 `execCmd("ohos-aa start --bundleName=com.example", { isShellCommand: false })` THEN Promise resolve 为 `CliSessionInfo`，其 `toolName === "ohos-aa"`、`status === "running"`、`sessionId` 非空；WHEN 调用方非系统应用 THEN Promise reject `BusinessError` 202；WHEN 调用方为系统应用但未持有该权限 THEN reject `BusinessError` 201（两分支均不产生会话，后续 `querySession` 查不到该会话） | System API | 正常 |
| AC-1.7 | WHEN 工具命令使用长标志语法（`--key=value`、`--key value`、`--flag`、`--flag false`）或以单引号包裹含空格的值（`--name 'hello world'`）THEN 参数按工具 schema 声明的类型恢复后生效，Promise resolve `CliSessionInfo`；WHEN 参数位置出现非 `--` 前缀的裸 token（短标志 `-b`、前缀参数、条件标志）THEN reject `BusinessError` 401；WHEN 单引号未闭合、或使用双引号/反斜杠转义 THEN reject `BusinessError` 401；WHEN 同一非数组参数被重复传入（`--key=a --key=b`）THEN reject `BusinessError` 401 | System API | 正常 |

### US-2: 既有 shell 调用零改造升级

**作为** 既有 shell 命令调用方（应用开发者），
**我想要** 在未声明工具命令模式时 `execCmd` 行为完全不变，
**以便** 零改造成本完成版本升级。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-1.2 | WHEN 调用方不传 `ExecCmdOptions`（`execCmd("ls -l")`）THEN 命令按 shell 路径执行，Promise resolve 的 `CliSessionInfo` 与既有 shell 行为一致（`toolName` 为 shell 语义值、`status` 为 `running`），且不因命令中含 `/`、`;` 等字符而被拒 | System API | 正常 |
| AC-1.10 | WHEN 调用方显式传 `{ isShellCommand: true }`（`execCmd("/bin/echo hello", { isShellCommand: true })`）THEN 行为与 AC-1.2 一致（与既有 shell 行为一致），不进入工具命令模式的工具名/子命令/schema 校验，不返回 401（工具名非法类）与 35600030 | System API | 正常 |

### US-3: 命令执行安全边界

**作为** 系统（系统开发者视角的安全约束），
**我要** 确保工具命令模式与 `execTool` 共用同一权限/沙箱校验链路，并拦截非法工具名与非法命令，
**以便** 不因新增入口扩大命令执行攻击面。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-1.3 | WHEN 工具命令模式下 `cmd` 的首个 token 含非白名单字符（`"/system/bin/ohos-aa start"`、`"../ohos-aa"`、`"ohos-aa;rm"`、`"ohos.aa"`）THEN Promise reject `BusinessError` 401 `"Invalid input parameter."`，且不产生子进程（后续 `querySession` 查不到会话） | System API | 异常 |
| AC-1.4 | WHEN 工具名未在 CLI 工具注册信息中（`"ohos-unknown start"`），或第二个 token 非 `--` 前缀且不属于该工具已声明的子命令集合（含该工具本无子命令的情形）THEN Promise reject `BusinessError` 35600030 `"The tool does not exist."` | System API | 异常 |
| AC-1.5 | WHEN 工具命令模式下 `cmd` 为空或纯空白（`""`、`"   "`）THEN Promise reject `BusinessError` 401 `"Invalid input parameter."`（空串在 NAPI 入口即拒，纯空白在客户端入口即拒，均不发起跨进程调用） | System API / InnerAPI | 异常 |
| AC-1.6 | WHEN 命令含 schema `properties` 未定义的标志（`"ohos-aa start --unknownFlag=x"`）、标志名为空（`"--=x"`）、值无法恢复为 schema 声明类型（`--count=abc` 而 schema 为 integer）、或存在无法归属到任何标志的孤立值 THEN Promise reject `BusinessError` 401 `"Invalid input parameter."` | System API | 异常 |
| AC-1.9 | WHEN 当前并发会话数已达配置上限（默认 8，可由系统参数 `const.sys.cli.limit_maximum_concurrency_quantity` 配置）THEN 工具命令模式与 shell 模式同样 reject `BusinessError` 35600031 `"Maximum number of processes has been reached."` | System API | 边界 |

### US-4: IPC 跨版本兼容

**作为** 系统（跨版本混布约束），
**我要** 保证新增字段在跨进程传输中位于尾部且可缺省读取，
**以便** 新旧客户端/服务端混布时不崩溃、旧调用方行为不变。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-1.8 | WHEN 反序列化一个只写到既有 `options` 字段为止的 Parcel（模拟旧客户端）THEN 得到的参数对象 `isShellCommand === true`、`challenge === ""`，反序列化不返回失败、不崩溃，服务端按 shell 模式处理；WHEN 新客户端向旧服务端发起工具命令模式调用 THEN 调用不崩溃，但旧服务端按 shell 语义执行同一 `cmd`（兼容但不等价，见「兼容性声明」） | InnerAPI | 边界 |

## 验收追溯

> 只维护 Spec 内部的 AC → 规则 → 可观察表面。AC → Task 见 `execution-plan.md`；测试入口/Red/通过标准见下方「验证映射」。

| AC | 关联规则 | 可观察表面 |
|----|----------|--------------|
| AC-1.1 | R-1, R-2, R-9, R-11, R-13 | System API：Promise resolve `CliSessionInfo`（`toolName`/`status`/`sessionId`）；reject 201 / 202 |
| AC-1.2 | R-3, R-11 | System API：Promise resolve `CliSessionInfo`，与基线 shell 行为一致 |
| AC-1.3 | R-4 | System API：reject 401，且无会话可查 |
| AC-1.4 | R-5 | System API：reject 35600030 |
| AC-1.5 | R-6 | System API：reject 401；InnerAPI：客户端返回 `ERR_INVALID_PARAM` 且不发起 IPC |
| AC-1.6 | R-7, R-8 | System API：reject 401 |
| AC-1.7 | R-7, R-8, R-10 | System API：Promise resolve `CliSessionInfo`（参数按 schema 类型生效）；reject 401 |
| AC-1.8 | R-11, R-12 | InnerAPI：`ExecCmdParam` 反序列化结果字段值与不失败语义 |
| AC-1.9 | R-13 | System API：reject 35600031 |
| AC-1.10 | R-3, R-11 | System API：Promise resolve `CliSessionInfo`，与基线 shell 行为一致 |

## 规则定义

> 类型：**行为** / **边界** / **异常** / **恢复**。触发条件均可复现（给出精确输入），预期行为均可在 System API / InnerAPI 边界观测。

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | 已注册工具 + 合法长标志命令 + `isShellCommand: false` + 调用方为持权限系统应用 | Promise resolve `CliSessionInfo`：`toolName` = 命令首 token，`status` = `"running"`，`sessionId` 非空；命令在工具沙箱内执行 | 会话建立成功后事件订阅先于进程输出，早期事件不丢失（内部不变量见 design） | AC-1.1 |
| R-2 | 行为 | 工具命令模式 + 调用方提供了 `challenge` | `challenge` 原值透传给工具进程，服务端不生成、不改写、不回显到错误信息 | 透传不改值；日志脱敏 | AC-1.1 |
| R-3 | 行为 | `isShellCommand` 未传或显式为 `true` | 走 shell 路径：`cmd` 整体作为 shell 命令在沙箱内执行；不做工具名白名单、子命令与 schema 校验；`CliSessionInfo` 与基线一致 | 默认值在 NAPI 层与 IPC 反序列化层一致（均为 `true`） | AC-1.2, AC-1.10 |
| R-4 | 异常 | 工具命令模式下命令首 token 含白名单外字符（白名单 = 字母数字 ∪ `-` ∪ `_`） | reject 401 `"Invalid input parameter."`；不创建子进程、不建立会话 | `/`、`.`、`\`、空格、`;`、`$`、反引号、`|`、`&` 均被拒；首 token 由首个空白分隔符切分（前导空白被忽略） | AC-1.3 |
| R-5 | 异常 | 首 token 通过白名单但工具未注册；或第二个 token 非 `--` 前缀且不在该工具已声明子命令集合内（含工具未声明任何子命令） | reject 35600030 `"The tool does not exist."` | 子命令判定仅在第二个 token 非标志前缀时触发；`--help` 恒被允许 | AC-1.4 |
| R-6 | 异常 | 工具命令模式下 `cmd` 为空串或仅含空格/制表符 | reject 401 `"Invalid input parameter."`；不发起跨进程调用 | 空串在 System API 入口即拒；纯空白在 InnerAPI 客户端入口即拒 | AC-1.5 |
| R-7 | 行为 | 长标志四种写法：`--key=value`、`--key value`、`--flag`（schema 为 boolean）、`--flag false`；以及单引号包裹含空格值 `--key 'a b'` | 参数按 schema 声明类型恢复：boolean → `true`/`false`，integer/number → 数值，string → 原值（单引号被剥离，`a b` 作为单个值） | 仅支持单引号；重复的同名标志在 schema 为 array 时聚合为数组 | AC-1.6, AC-1.7 |
| R-8 | 异常 | 不支持的语法：参数位置裸 token（`-b`、前缀参数、条件标志）、单引号未闭合、双引号、反斜杠转义、标志名为空（`--=x`）、schema 外标志、非数组参数重复传入、`--key` 后无值且 schema 非 boolean、值无法恢复为 schema 类型 | reject 401 `"Invalid input parameter."`；不静默降级、不按字符串兜底执行 | 明确拒绝而非忽略：不允许把非法语法当作位置参数传给工具 | AC-1.6, AC-1.7 |
| R-9 | 异常 | 工具命令模式下 `ExecOptions` 非法：`timeout < 0` 或 `yieldMs < 0`；`timeout > 1800`（秒）；`background === false` 且 `yieldMs > timeout × 1000` | reject 401 `"Invalid input parameter."` | 边界值：`timeout = 1800` 合法；`timeout = 0` 且 `yieldMs > 0` 必然命中第三条；校验规则与 shell 路径、`execTool` 完全一致 | AC-1.1 |
| R-10 | 边界 | 命令解析与 `ExecOptions` 校验同时非法（如 `"ohos-aa start --unknownFlag=x"` + `timeout: -1`） | 先返回命令解析错误 401（两者优先级同为 401，对外不可区分先后，但均不产生子进程） | 校验次序属实现约束，见 design；对外结果稳定为 401 | AC-1.7 |
| R-11 | 边界 | 旧客户端不写尾部两个字段（Parcel 只写到既有 `options`） | 反序列化成功，`isShellCommand` 回退 `true`、`challenge` 回退 `""`，按 shell 模式执行；不崩溃、不返回错误码 | 字段序固定为 `cmd → workDir → env → policy → options → isShellCommand → challenge`，新增字段只能在尾部 | AC-1.2, AC-1.8, AC-1.10 |
| R-12 | 边界 | 新客户端（传 `isShellCommand: false`）→ 旧服务端 | 调用不崩溃（Parcel 允许残留未读数据），但旧服务端按 shell 语义执行同一 `cmd` 字符串，校验与沙箱链路与工具命令模式不同 | **工具命令模式要求客户端与服务端同版本配套**；本特性不引入协议层版本协商 | AC-1.8 |
| R-13 | 边界 | 当前并发会话数 ≥ 配置上限（默认 8） | reject 35600031 `"Maximum number of processes has been reached."` | 上限校验对 shell 与工具命令两条路径共用同一入口，先于模式分流 | AC-1.1, AC-1.9 |
| R-14 | 异常 | 权限门失败：调用方非系统应用 / 未持有 `ohos.permission.EXEC_CLI_TOOL` | 分别 reject 202 `"Not system application."` / 201 `"Permission denied."`；不建立会话、不创建子进程 | 权限门对两条路径共用且先于模式分流；不新增权限、不放宽既有校验 | AC-1.1 |
| R-15 | 恢复 | 工具命令模式下会话建立过程中任一步失败（订阅失败、子进程创建失败、监控注册失败） | Promise reject 对应错误码（未映射的内部失败统一为 35600050 `"Internal error."`），且不残留可查询会话、不残留孤儿进程组 | 失败后会话不可被 `querySession` 查到；进程组被终止（内部回滚顺序见 design） | AC-1.1 |
| R-16 | 行为 | 工具命令模式下调用方提供 `ExecCmdOptions.workDir` / `env` / `policy` | 三项**不生效**（工具路径的沙箱配置只注入调用方身份标识，不含调用方 workdir/policy/env，见 BF-3）；shell 模式下三项均生效。**需求方结论（2026-09-09，zhoulanzhi）：预期行为，`.d.ts` 声明不区分模式**，不判定为缺陷、不回 proposal 变更控制 | 调用方需知悉模式差异：工具的执行环境由工具自身与既有工具沙箱语义决定；本规格与 `.d.ts` 不为三项属性增加模式差异标注 | AC-1.1 |
| R-17 | 边界 | 调用方传 `ExecCmdOptions` 对象但未传 `timeout` | 生效 `timeout = 1800`（秒），与 `.d.ts` `@default 1800` 一致；**完全不传 options 对象**时生效 `timeout = 0`（不限时）。**需求方结论（2026-09-09，zhoulanzhi）：以既有代码事实为准**，legacy `spec.md` EB-2「未传即 0」的描述作废 | 两条默认路径并存且保持现状不统一；工具命令模式必然携带 options 对象，故其缺省超时为 1800 秒；`timeout = 1800` 为合法边界值，`> 1800` → 401 | AC-1.1, AC-1.2 |

## 验证映射

> 每个 AC 至少一个可执行测试入口。`Red 条件` 描述**实现前**该用例为什么必然失败、失败时可观察到什么；`通过标准` 为精确的对外可观察结果。测试入口分两类：既有测试目标内的既有用例（回归看护）与需新增的用例（本特性新增行为）。

| 编号 | 对应规格项 | 测试入口 | 验证方式 | Red 条件（实现前失败信号） | 通过标准 |
|------|------------|----------|----------|---------------------------|----------|
| VM-1 | R-14 / AC-1.1（权限门） | `cli_tool_mgr_service_test.cpp::ExecCmd_0100`..`ExecCmd_0700`（既有用例，含 `IsPermissionGateResult` 断言） | 单测（回归） | 无 Red 态——防回退守护用例：权限门为分流前既有逻辑，新增分流后这些用例必须仍绿；若变红说明分流点前移或权限判定被绕过 | 非系统应用 → 202；无 `EXEC_CLI_TOOL` → 201；两者均无会话建立 |
| VM-2 | R-1, R-2 / AC-1.1（工具命令成功路径） | 新增 `cli_tool_mgr_service_test.cpp::ExecCmd_ToolMode_Success_0100` | 单测 | 实现前 `ExecCmd` 无模式分流，`isShellCommand=false` 不被识别，命令被当作 shell 字符串执行：返回的 `CliSessionInfo.toolName` 为 shell 语义值而非命令首 token，且 mock 的工具查表/工具子进程创建路径调用次数为 0 | 返回 `ERR_OK`；`CliSessionInfo.toolName` = 命令首 token、`status` = `"running"`、`sessionId` 非空；`challenge` 原值透传给工具参数 |
| VM-3 | R-4 / AC-1.3（工具名白名单） | 新增 `cli_tool_mgr_service_test.cpp::ExecCmd_ToolMode_InvalidToolName_0100`（`"/system/bin/ohos-aa start --bundleName=com.x"`）、`::ExecCmd_ToolMode_InvalidToolName_0200`（`"ohos-aa;rm"`、`"../ohos-aa"`、`"ohos.aa"`）、`::ExecCmd_ToolMode_EmptyToolName_0100`（`"   "` 经服务端路径） | 单测（安全） | 实现前无白名单校验：`/system/bin/ohos-aa` 这类首 token 会被原样交给 shell 路径执行，用例断言的 401 得不到（返回 `ERR_OK` 或 shell 侧错误），且进程创建 mock 被调用 | 返回 `ERR_INVALID_PARAM`（对外 401）；工具查表与进程创建 mock 调用次数为 0；查询不到会话 |
| VM-4 | R-5 / AC-1.4（工具/子命令不存在） | 新增 `cli_tool_mgr_service_test.cpp::ExecCmd_ToolMode_ToolNotExist_0100`（`"ohos-unknown start"`）、`tool_util_test.cpp::ParseToolCommand_UnknownSubCommand_0100`（已注册工具 + 未声明子命令） | 单测 | 实现前 `execCmd` 不查工具注册信息：未注册工具名不会返回 `ERR_TOOL_NOT_EXIST`，用例断言的 35600030 得不到；解析入口 `ParseToolCommand` 不存在，编译期即缺符号 | 未注册工具 → `ERR_TOOL_NOT_EXIST`（对外 35600030）；未声明子命令 → `ERR_TOOL_NOT_EXIST`；均无子进程创建 |
| VM-5 | R-6 / AC-1.5（空/纯空白命令） | 新增 `cli_tool_mgr_client_test.cpp::ExecCmd_0300`（`cmd = "   "` + `isShellCommand = false`，断言不发 IPC）、`tool_util_test.cpp::ParseToolCommand_EmptyCmd_0100` | 单测 | 实现前客户端无模式相关前置校验：纯空白命令会照常发起 IPC 调用（proxy mock 被调用一次），断言的「不发 IPC + `ERR_INVALID_PARAM`」失败 | 客户端返回 `ERR_INVALID_PARAM` 且 proxy 调用次数为 0；解析层空命令返回 `ERR_INVALID_PARAM`；对外 401 |
| VM-6 | R-7 / AC-1.7（四种长标志 + 单引号） | 新增 `tool_util_test.cpp::ParseToolCommand_KeyEqualsValue_0100`、`::ParseToolCommand_KeySpaceValue_0100`、`::ParseToolCommand_BooleanFlag_0100`、`::ParseToolCommand_BooleanFalse_0100`、`::ParseToolCommand_SingleQuoteSpace_0100`、`::ParseToolCommand_SubCommandSchema_0100` | 单测 | 实现前 `ToolUtil::ParseToolCommand` 不存在（编译期缺符号）；即使以既有 `ExpandArgsFromJson` 替代也无法解析命令行语法，`--key value` 与 `'a b'` 均得不到预期键值与类型 | 各写法解析出的键值与 schema 类型一致（boolean/integer/number/string/array）；`--name 'hello world'` → 单值 `hello world`（引号被剥离）；子命令 schema 优先于工具级 schema |
| VM-7 | R-8 / AC-1.6, AC-1.7（非法语法拒绝） | 新增 `tool_util_test.cpp::ParseToolCommand_UnknownFlag_0100`、`::ParseToolCommand_ShortFlag_0100`（`-b`）、`::ParseToolCommand_UnclosedQuote_0100`（`--name 'hello`）、`::ParseToolCommand_RepeatNonArrayFlag_0100`（`--key=a --key=b`）、`::ParseToolCommand_TypeMismatch_0100`（`--count=abc` 而 schema 为 integer） | 单测 | 实现前解析入口不存在；若以「宽松解析」实现（忽略未知标志、按字符串兜底），上述用例断言的 `ERR_INVALID_PARAM` 得不到，非法命令会被静默传给工具 | 五条输入均返回 `ERR_INVALID_PARAM`（对外 401），且不产出半解析结果 |
| VM-8 | R-3, R-11 / AC-1.2, AC-1.10（shell 缺省与显式） | 新增 `cli_tool_mgr_service_test.cpp::ExecCmd_DefaultShellDispatch_0100`（不传模式标识，`cmd = "/bin/echo hello"`）、`::ExecCmd_ExplicitShellMode_0100`（显式 `isShellCommand = true`） | 回归 | `ExecCmd_DefaultShellDispatch_0100` 为防回退守护用例（无 Red 态）：含 `/` 的命令在缺省模式下必须**不**返回 401，若分流点误置于缺省路径则该用例变红。`ExecCmd_ExplicitShellMode_0100` 实现前失败：`ExecCmdOptions` 无该属性，显式 `true` 不被识别（NAPI 解包不接收，断言的「与缺省行为一致」无法建立） | 两条用例均走 shell 路径：不返回 401（工具名非法类）与 35600030，`CliSessionInfo` 与既有 shell 结果一致 |
| VM-9 | R-11 / AC-1.8（IPC 兼容） | 新增 `exec_cmd_param_test.cpp::ExecCmdParam_Unmarshalling_OldClient_0100`（Parcel 只写到 `options`）、`::ExecCmdParam_Parcelable_0100`/`_0200`（完整往返）、`::ExecCmdParam_ExtractToolName_0100`..`_0300`（首 token 提取边界） | 单测（IPC 兼容） | 实现前 `ExecCmdParam` 无尾部两字段：`Unmarshalling` 在读完 `options` 后即返回，旧客户端 Parcel 与新结构体的往返断言（`isShellCommand == true`、`challenge == ""`）无法成立；`ExtractToolName` 符号不存在 | 只写到 `options` 的 Parcel → 反序列化成功且 `isShellCommand == true`、`challenge == ""`；完整 Parcel 往返七字段值一致；`ExtractToolName` 对前导空白/无空白/空串分别返回首 token/整串/`""` |
| VM-10 | R-13 / AC-1.9（会话上限） | 新增 `cli_tool_mgr_service_test.cpp::ExecCmd_ToolMode_SessionLimit_0100`（预置会话记录至上限后以工具模式调用） | 单测 | 实现前工具模式路径不存在，调用会落到 shell 路径：预置满额会话时返回码虽同为上限错误，但断言的「工具模式与 shell 模式共用同一上限入口」不成立（工具查表/解析 mock 会被跳过或误调用） | 返回 `ERR_SESSION_LIMIT_EXCEEDED`（对外 35600031）；上限校验发生在模式分流之前，解析与进程创建 mock 调用次数为 0 |
| VM-11 | R-9 / AC-1.1（ExecOptions 校验） | `tool_util_test.cpp::ValidateExecOptionsProperties_0100`..`_0800`（既有用例：`_0200` 负 timeout、`_0300` 负 yieldMs、`_0400` timeout > 1800、`_0500` 非后台且 yieldMs > timeout、`_0600` 后台时允许、`_0700` yieldMs == timeout × 1000 边界、`_0800` 两者同时为负） | 回归 | 无 Red 态——既有约束复用守护：工具模式接入后这些用例必须仍绿；若工具模式另设阈值或跳过该校验，用例与实际行为不一致即暴露 | 越界组合均返回 `ERR_INVALID_PARAM`（对外 401）；`timeout = 1800` 与 `yieldMs == timeout × 1000` 为合法边界 |
| VM-12 | R-15 / AC-1.1（失败恢复） | 新增 `cli_tool_mgr_service_test.cpp::ExecCmd_ToolMode_SubscribeFail_Rollback_0100`、`::ExecCmd_ToolMode_CreateProcessFail_Rollback_0100`、`::ExecCmd_ToolMode_MonitorFail_Rollback_0100` | 单测（故障注入） | 实现前工具模式无会话建立路径，三步失败的回滚次序（订阅 → 进程 → 监控）不存在：注入失败后既得不到对应错误码，也无法断言「无残留会话、无孤儿进程组」 | 分别注入三步失败：返回对应内部码（未映射失败 → 35600050）；`querySession` 查不到该会话；订阅被反注册；已创建进程组被 `Killpg` |
| VM-13 | R-16, R-17 / AC-1.1（options 语义） | 新增 `cli_tool_mgr_service_test.cpp::ExecCmd_ToolMode_OptionsSemantics_0100`（工具模式传 `workDir`/`env`/`policy`）、`tool_util_test.cpp::GenerateSandboxConfig_NoCallerWorkDir_0100`（工具沙箱配置内容断言）、NAPI 层 `timeout` 缺省值断言 | 单测 | 实现前工具模式不存在，无法构造「传了三项属性但沙箱配置不含 workdir/policy/env」的断言；`GenerateSandboxConfig` 与 `GenerateCmdSandboxConfig` 的差异无用例看护，若误把三项接入工具沙箱（或反向）无信号 | 工具模式沙箱配置只含调用方身份标识（`ohos_cli_callerBundleName`/`callerUid`/`callerTokenId`），不含 `workdir`/`policy`/调用方 `env`；shell 模式三项均写入；传 options 未传 `timeout` → 生效 1800，不传 options → 生效 0 |

## API 变更分析

### 新增 API

| API 名称 | 开放级别 | 入参概要 | 返回值 | 错误码范围 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|------------|----------|---------|
| `cliManager.ExecCmdOptions.isShellCommand` | System | `isShellCommand?: boolean`，默认 `true` | —（接口属性） | —（属性本身不抛错；取值决定 `execCmd` 的错误面） | `true` = shell 命令；`false` = 已注册 CLI 工具命令字符串 | AC-1.1, AC-1.2, AC-1.8, AC-1.10 |
| `cliManager.ExecCmdOptions.challenge` | System | `challenge?: string`，默认 `""` | —（接口属性） | — | 调用方从 access token manager 获取的唯一标识，原值透传给工具进程 | AC-1.1 |

### 变更/废弃 API

| API 名称 | 当前开放级别 | 目标开放级别 | 变更类型 | 影响场景 | 迁移指引 | 关联 AC |
|----------|--------------|--------------|----------|----------|----------|---------|
| `cliManager.execCmd` | System | System | 变更（级别不变；行为扩展向后兼容 + 文档 `@throws` 补充） | 工具命令模式产生既有 shell 路径不存在的 401 与 35600030；既有 `.d.ts` 的 `@throws` 仅列 201/202/35600031/35600050。签名不变：`function execCmd(cmd: string, execCmdOptions?: ExecCmdOptions): Promise<CliSessionInfo>` | 未传或显式传 `isShellCommand: true` 的既有调用无需迁移；使用工具命令模式的调用方需在 `catch` 中新增 401 与 35600030 处理 | AC-1.2, AC-1.3, AC-1.4, AC-1.5, AC-1.6, AC-1.10 |
| `ICliToolManager.ExecCmd / ExecCmdParam` | InnerAPI | InnerAPI | 变更（级别不变；序列化格式尾部追加字段，接口签名不变） | 跨进程 CLI 工具管理调用；新旧客户端/服务端混布 | 无需迁移；新增字段位于 Parcel 尾部，读取失败回退默认值（R-11/R-12） | AC-1.8 |

> **废弃 API：** 无（本特性不废弃任何既有 API、字段或错误码）。

### API 与错误码事实

> **既有事实**给出 `文件:行/符号`（`cli_tool_framework/` 相对 ability_runtime 仓根，`.d.ts` 相对 `interface/sdk-js` 仓根）；**新增事实**标 `New`，依据列为 `proposal.md` / `design.md`（ADR/INV/TASK），落点只给文件与符号、不给行号（新增代码行号在实现前不存在）。

| 事实项 | 事实类型 | 开放级别 | 精确签名/数值 | 触发条件 | 对外行为 | 事实来源（文件:行/符号或 New 依据） | 关联 AC |
|--------|----------|----------|---------------|----------|----------|--------------------------------------|---------|
| `cliManager.execCmd` | Existing API | System | `function execCmd(cmd: string, execCmdOptions?: ExecCmdOptions): Promise<CliSessionInfo>`；标注 `@systemapi`、`@stagemodelonly`、`@syscap SystemCapability.Ability.AgentRuntime.Core`、`@since 26.0.0 dynamiconly`；`@permission ohos.permission.EXEC_CLI_TOOL` | 系统应用调用，持 `ohos.permission.EXEC_CLI_TOOL` | resolve `CliSessionInfo` / reject `BusinessError`；既有 `@throws` 仅列 201/202/35600031/35600050（`api/@ohos.app.cli.cliManager.d.ts:405-409`） | `interface/sdk-js:api/@ohos.app.cli.cliManager.d.ts:398-415` | AC-1.1, AC-1.2, AC-1.10 |
| `cliManager.ExecCmdOptions` | Existing API | System | `workDir?: string; env?: Record<string, string>; background?: boolean（@default false）; yieldMs?: long（@default 0）; timeout?: long（@default 1800）; policy?: string; callback?: ToolEventCallback` | 调用方按需传入 | 影响命令执行环境与前台等待/超时行为；`workDir`/`env`/`policy` 在工具命令模式下不生效（见「行为事实与约束」BF-3、Q-S1） | `interface/sdk-js:api/@ohos.app.cli.cliManager.d.ts:85-155` | AC-1.1, AC-1.2 |
| `cliManager.ExecCmdOptions.isShellCommand` | New API | System | `isShellCommand?: boolean`，默认 `true` | 调用方声明命令模式 | `false` → 工具命令路径（新增 401/35600030 错误面）；缺省或 `true` → shell 路径，行为与既有 shell 路径一致 | New: `proposal.md`「API 变更项清单」+ 本文 R-3/R-11 + AC-1.1/AC-1.2/AC-1.8/AC-1.10；落点与缺省值约束 `design.md` ADR-7 / INV-1 / TASK-1（`js_cli_manager_utils.cpp / UnwrapExecCmdOptions`，既有解包约定见 `:219-365`） | AC-1.1, AC-1.2, AC-1.8, AC-1.10 |
| `cliManager.ExecCmdOptions.challenge` | New API | System | `challenge?: string`，默认 `""` | 工具命令模式下调用方透传认证标识 | 原值传给工具进程；服务端不生成、不校验内容、不改写 | New: `proposal.md`「API 变更项清单」+ 本文 R-2 + AC-1.1；落点 `design.md` ADR-7 / INV-5 / TASK-1（`js_cli_manager_utils.cpp / UnwrapExecCmdOptions`） | AC-1.1 |
| `ICliToolManager.ExecCmd / ExecCmdParam` | Existing API | InnerAPI | 字段序 `cmd, workDir, env, policy, options, isShellCommand = true, challenge`；Marshalling 序 `WriteString(cmd) → workDir → env → policy → WriteParcelable(options) → WriteBool(isShellCommand) → WriteString(challenge)`；`static std::string ExtractToolName(const std::string &cmd)`（首个空白分隔 token，忽略前导空白，空/全空白返回 `""`） | 跨进程调用 `ExecCmd`；工具命令模式提取工具名 | 尾部两字段缺失时反序列化仍成功并回退 `true` / `""`；`ExtractToolName` 结果决定白名单校验与工具查表输入 | 既有：`cli_tool_framework/interfaces/cli_tool/include/exec_cmd_param.h:29-43 / ExecCmdParam`（既有成员见 `:31-35`）、`src/exec_cmd_param.cpp:22-52 / Marshalling`（既有写入序）、`:55-85 / Unmarshalling`（既有读取序）、`ICliToolManager.idl:21,34`；New：两个尾部字段与 `ExtractToolName`，依据 `proposal.md`「API 变更项清单」+ `design.md` ADR-1/ADR-2/「数据模型设计」序列化契约 + AC-1.8 | AC-1.3, AC-1.4, AC-1.8 |
| `CliSessionInfo` | Existing API | System | `sessionId: string; toolName: string; status: string（"running" / "completed" / "failed"）; result?: ExecResult` | 命令执行成功建立会话 | Promise resolve 的会话信息，调用方据此判定执行结果 | `cli_tool_framework/interfaces/cli_tool/include/cli_session_info.h:32-35` | AC-1.1, AC-1.2, AC-1.10 |
| `ERROR_PERMISSION_DENIED` | Existing Error Code | System | `201`（内部码 `ERR_PERMISSION_DENIED = 35700007`） | 调用方为系统应用但未持有 `ohos.permission.EXEC_CLI_TOOL` | reject `BusinessError` 201 `"Permission denied."`，不建立会话 | `cli_manager_error_utils.h:26`、`cli_manager_error_utils.cpp:42`、`cli_error_code.h:63`；触发点 `cli_tool_manager_service.cpp:898-901` | AC-1.1 |
| `ERROR_NOT_SYSTEM_APP` | Existing Error Code | System | `202`（内部码 `ERR_NOT_SYSTEM_APP = 35700008`） | 调用方非系统应用 | reject `BusinessError` 202 `"Not system application."`，不建立会话 | `cli_manager_error_utils.h:27`、`cli_manager_error_utils.cpp:43`、`cli_error_code.h:68`；触发点 `cli_tool_manager_service.cpp:892-896` | AC-1.1 |
| `ERROR_INVALID_PARAM` | Existing Error Code | System | `401`（内部码 `ERR_INVALID_PARAM = 35700002`） | 工具名含白名单外字符；`cmd` 空/纯空白；schema 外标志；标志名为空；类型恢复失败；非数组参数重复；裸 token；未闭合引号；`timeout`/`yieldMs` 越界 | reject `BusinessError` 401 `"Invalid input parameter."`，不创建子进程 | 既有：`cli_manager_error_utils.h:28`、`cli_manager_error_utils.cpp:44`、`cli_error_code.h:38`；既有触发点 `tool_util.cpp:93-114 / ValidateExecOptionsProperties`、`js_cli_manager.cpp:192-227 / OnExecCmd`（`cmd` 非空）。New 触发点：工具名白名单与命令解析（`design.md` INV-4/INV-7、ADR-5，落点 `cli_tool_manager_service.cpp / ExecCmdToolMode`、`tool_util.cpp / ParseToolCommand`）、客户端空白命令前置校验（`design.md` TASK-2，落点 `cli_tool_mgr_client.cpp / ExecCmd`） | AC-1.3, AC-1.5, AC-1.6, AC-1.7 |
| `ERROR_TOOL_NOT_FOUND` | Existing Error Code | System | `35600030`（内部码 `ERR_TOOL_NOT_EXIST = 35700005`） | 工具名未注册；第二个 token 非标志前缀且不在已声明子命令集合（含工具未声明子命令） | reject `BusinessError` 35600030 `"The tool does not exist."` | 既有：`cli_manager_error_utils.h:30`、`cli_manager_error_utils.cpp:45`、`cli_error_code.h:53`；既有触发点 `cli_tool_manager_service.cpp:917-936 / ValidateAndPrepareTool`、`tool_util.cpp:59-91 / ValidateProperties`（子命令不存在）。New 触发点：工具命令模式的工具查表与子命令解析（`design.md` ADR-3/ADR-5，落点 `ExecCmdToolMode`、`ParseToolCommand`） | AC-1.4 |
| `ERROR_REACH_LIMIT` | Existing Error Code | System | `35600031`（内部码 `ERR_SESSION_LIMIT_EXCEEDED = 35700001`） | 并发会话数 ≥ 上限（默认 `8`，系统参数 `const.sys.cli.limit_maximum_concurrency_quantity` 可配，非法值回退默认） | reject `BusinessError` 35600031 `"Maximum number of processes has been reached."` | `cli_manager_error_utils.h:31`、`cli_manager_error_utils.cpp:46`、`cli_error_code.h:33`；上限来源 `services/common/include/ccm_util.h:24`、`services/common/src/ccm_util.cpp:24,33-47`；触发点 `cli_tool_manager_service.cpp:906-915` | AC-1.9 |
| `ERROR_INNER` | Existing Error Code | System | `35600050` | 会话记录创建失败（`ERR_NO_INIT = 35700004`）、沙箱生成失败（`ERR_NOT_HAP = 35700003`）、子进程创建失败、监控注册失败、服务不可用、配置/KVStore 错误；未映射内部码统一回退至此 | reject `BusinessError` 35600050（`"System Error."` / `"Internal error."` 文案） | 既有：`cli_manager_error_utils.h:35`、`cli_manager_error_utils.cpp:56-59,77,94`；既有触发点 `cli_tool_manager_service.cpp:955-989 / SetupAndStartSession`（会话建立失败与 `Killpg` 回滚范式）、`ExecCmd` 入口 `SetScheduler` 失败 → `ERR_NO_INIT`。New 触发点：工具命令模式会话建立失败路径（`design.md` ADR-4 / INV-3 / TASK-4） | AC-1.1 |

### 行为事实与约束（非 API/错误码）

> 以下为本特性的行为约束与既有代码事实，供 `design.md` 承接内部实现；不属 API 声明或错误码契约，故独立于上表。**BF-2/BF-3 为既有事实**（给 `文件:行`）；**BF-1/BF-4/BF-5/BF-6 为新增行为的规格约束**（依据 `design.md` 的 ADR/INV，落点只给符号）。Q-S1/Q-S2 结论已由需求方于 2026-09-09 确认。

| 编号 | 行为事实 | 精确取值/谓词 | 对外可观察后果 | 事实来源（文件:行） | 关联 AC / 规则 |
|------|----------|---------------|----------------|---------------------|----------------|
| BF-1 | 工具名白名单谓词 | `toolName` 非空且每个字符满足 `std::isalnum(static_cast<unsigned char>(c)) \|\| c == '-' \|\| c == '_'`（非正则；`isalnum` 结果依赖当前 C locale） | `/`、`.`、`\`、空格、`;`、`$`、反引号、`\|`、`&` 均被拒 → 401，且不查表、不创建进程 | New（规格约束）：`design.md` INV-4 / ADR-5；落点 `cli_tool_manager_service.cpp / ExecCmdToolMode`（工具命令处理入口，白名单先于查表与进程创建） | AC-1.3 / R-4 |
| BF-2 | 超时上限与缺省超时 | `MAX_TIMEOUT = 30 * 60 = 1800`（秒）、`MILLISECOND_COEFFICIENT = 1000`；NAPI 层当调用方**提供了 options 对象但未提供 `timeout`** 时置 `options.timeout = MAX_TIMEOUT`；**完全不提供 options 对象**时保留原生默认 `timeout = 0`（不限时） | 工具命令模式必然携带 options 对象，故未传 `timeout` 时缺省为 1800 秒而非 0（Q-S2） | 既有：`tool_util.cpp:55-56,93-114`（`MAX_TIMEOUT` 与校验）、`js_cli_manager_utils.cpp:35,155-217 / UnwrapExecOptions`、`exec_options.h:30-38`（原生默认值） | AC-1.1 / R-9, R-17 |
| BF-3 | 沙箱配置生成的两套语义 | 工具路径按 `ExecToolParam` 生成，只注入调用方身份（`ohos_cli_callerBundleName`/`callerUid`/`callerTokenId`），**不含** `workdir`/`policy`/调用方 `env`；shell 路径按 `ExecCmdParam` 生成，写入 `policy`、`workdir`（非空时）、`env` | 工具命令模式下调用方传入的 `workDir`/`env`/`policy` 被忽略 —— 已确认为预期行为（Q-S1 结论），`.d.ts` 不区分模式 | 既有：`tool_util.cpp:187-219 / GenerateSandboxConfig`（工具）、`tool_util.cpp:221-241 / GenerateCmdSandboxConfig`（shell）、`exec_tool_param.h:30-37 / ExecToolParam`（无对应字段）；设计承接见 `design.md` ADR-8 | AC-1.1 / R-16 |
| BF-4 | 工具模式校验次序（对外错误优先级） | 权限门 → 调度器注册 → 会话上限 → 模式分流 → 工具名白名单 → 工具查表 → 命令解析（分词/子命令/标志/类型恢复）→ 参数与 options 校验 + 沙箱生成 → 会话建立 | 命令解析错误先于 `ExecOptions` 越界错误被返回，两者对外同为 401，不可区分先后 | 既有入口序：`cli_tool_manager_service.cpp:1088-1110 / ExecCmd`（权限门 → `SetScheduler` → 会话上限 → 取 caller 上下文）；New 次序：`design.md`「数据流/控制流」步骤 6-12 + BF-4 落点 `ExecCmdToolMode` | AC-1.1, AC-1.7 / R-10 |
| BF-5 | 重复标志的聚合语义 | 同名标志重复出现时：schema 类型为 `array` → 聚合为数组；非 `array` → 401 | `--key=a --key=b` 在非数组参数上被拒，不静默取首值或末值 | New（规格约束）：`design.md` ADR-5 / INV-7；落点 `tool_util.cpp / ParseToolCommand`（类型恢复阶段），array 语义与既有 `tool_util.cpp:116 / ValidateInputSchemaProperties`、`:803 / ValidateArrayType` 对齐 | AC-1.6, AC-1.7 / R-7, R-8 |
| BF-6 | `--help` 恒被允许 | 标志名为 `help` 时不查 schema `properties`，直接置布尔 `true` | `--help` 在任何已注册工具上都不会因 schema 未声明而被拒 | 既有例外：`tool_util.cpp:141 / ValidateInputSchemaProperties`（`key == "help"` 恒允许）；New 落点：`design.md` ADR-5 / INV-7，`tool_util.cpp / ParseToolCommand` 标志解析阶段沿用同一例外 | AC-1.6, AC-1.7 / R-5, R-7 |

## 接口规格

### 接口定义

**`cliManager.execCmd`**

| 属性 | 值 |
|------|-----|
| 函数签名 | `function execCmd(cmd: string, execCmdOptions?: ExecCmdOptions): Promise<CliSessionInfo>` |
| 返回值 | `Promise<CliSessionInfo>` — resolve 时会话已建立（`status` 为 `"running"`）；reject 时为 `BusinessError` |
| 开放级别 | System（`@systemapi`、`@stagemodelonly`、`@syscap SystemCapability.Ability.AgentRuntime.Core`、`@since 26.0.0 dynamiconly`） |
| 权限 | `ohos.permission.EXEC_CLI_TOOL` |
| 错误码 | 201、202、401、35600030、35600031、35600050（既有 `.d.ts` 仅声明 201/202/35600031/35600050，需补 401 与 35600030） |
| 关联 AC | AC-1.1..AC-1.10 |

**参数约束**

| 参数 | 类型 | 必填 | 默认值 | 约束条件 |
|------|------|------|--------|---------|
| `cmd` | `string` | 是 | — | 非空（空串 → 401）；工具命令模式下纯空白 → 401，首个空白分隔 token 为工具名且须满足白名单；无长度上限校验（与 `execTool`/既有 `execCmd` 一致，proposal Q-4 结论） |
| `execCmdOptions.isShellCommand` | `boolean` | 否 | `true` | `false` 时启用工具命令模式；缺省值在 NAPI 与 IPC 反序列化两处一致 |
| `execCmdOptions.challenge` | `string` | 否 | `""` | 工具命令模式下原值透传；服务端不生成、不校验内容、不改写 |
| `execCmdOptions.timeout` | `long`（秒） | 否 | `1800`（传了 options 对象但未传该字段）/ `0`（完全不传 options 对象） | `0` = 不限时；`< 0` 或 `> 1800` → 401；见 R-17 / Q-S2 |
| `execCmdOptions.yieldMs` | `long`（毫秒） | 否 | `0` | `0` = 不提前后台化；`< 0` → 401；`background === false` 且 `yieldMs > timeout × 1000` → 401 |
| `execCmdOptions.background` | `boolean` | 否 | `false` | `true` 时会话立即后台化并按既有语义回复 |
| `execCmdOptions.workDir` / `env` / `policy` | `string` / `Record<string,string>` / `string` | 否 | `""` / `""` / `""` | shell 模式生效；工具命令模式下**不生效**（R-16，需求方已确认为预期行为，`.d.ts` 不区分模式） |
| `execCmdOptions.callback` | `ToolEventCallback` | 否 | — | 提供则自动订阅会话事件；非法回调 → 401 |

**行为场景**

| # | 触发条件 | 可观察表面 | 预期行为 | 关联 AC |
|---|----------|--------------|----------|---------|
| 1 | `execCmd("ohos-aa start --bundleName=com.example", { isShellCommand: false })`，调用方为持权限系统应用，工具已注册 | System API | resolve `CliSessionInfo{ toolName: "ohos-aa", status: "running", sessionId ≠ "" }` | AC-1.1 |
| 2 | 同上但调用方非系统应用 / 未持权限 | System API | reject 202 / reject 201，无会话可查 | AC-1.1 |
| 3 | `execCmd("ls -l")`（不传 options） | System API | resolve，行为与基线 shell 路径一致 | AC-1.2 |
| 4 | `execCmd("/bin/echo hello", { isShellCommand: true })` | System API | resolve，不进入工具名/子命令/schema 校验 | AC-1.10 |
| 5 | `execCmd("/system/bin/ohos-aa start", { isShellCommand: false })` | System API | reject 401，无子进程 | AC-1.3 |
| 6 | `execCmd("ohos-unknown start", { isShellCommand: false })` | System API | reject 35600030 | AC-1.4 |
| 7 | `execCmd("   ", { isShellCommand: false })` | System API / InnerAPI | reject 401，不发起跨进程调用 | AC-1.5 |
| 8 | `execCmd("ohos-aa start --unknownFlag=x", { isShellCommand: false })` | System API | reject 401 | AC-1.6 |
| 9 | `execCmd("ohos-aa start --name 'hello world' --verbose false", { isShellCommand: false })` | System API | resolve；`name` 为单值 `hello world`，`verbose` 为 `false` | AC-1.7 |
| 10 | `execCmd("ohos-aa start -b", { isShellCommand: false })` / `--name 'hello`（未闭合） | System API | reject 401 | AC-1.7 |
| 11 | 并发会话数已达上限（默认 8）时调用工具命令模式 | System API | reject 35600031 | AC-1.9 |
| 12 | 旧客户端 Parcel（只写到 `options`）到达新服务端 | InnerAPI | 反序列化成功，`isShellCommand == true`、`challenge == ""`，按 shell 模式执行 | AC-1.8 |

## 兼容性声明

- **已有 API 行为变更:** 否（对既有调用方）——`execCmd` 签名与返回类型不变；未传或显式传 `isShellCommand: true` 时行为与既有 shell 路径一致。工具命令模式为**新增行为面**，仅对显式声明 `false` 的调用方生效，并新增 401/35600030 两类可观察错误。
- **配置文件格式变更:** 否（沿用既有工具注册信息与沙箱配置格式；会话上限仍由系统参数 `const.sys.cli.limit_maximum_concurrency_quantity` 配置，默认 8）
- **数据存储格式变更:** 否（无持久化数据格式变更；IPC Parcel 属运行时序列化，单列如下）
- **IPC 兼容性（旧客户端 → 新服务端）:** 兼容——新增两字段位于 Parcel 尾部，读取失败时回退 `isShellCommand = true` / `challenge = ""`，按 shell 路径执行（R-11 / AC-1.8）
- **IPC 兼容性（新客户端 → 旧服务端）:** 兼容但不等价——旧服务端读完 `options` 即返回，Parcel 残留数据不触发解析失败或崩溃；但旧服务端无工具命令分支，会把工具命令字符串按 shell 语义执行，校验与沙箱链路与工具命令模式不同。**约束：工具命令模式要求客户端与服务端同版本配套**（R-12）
- **版本协商:** 本特性不引入显式版本协商字段，兼容性依赖尾部字段可选读取；若后续需在协议层拒绝跨版本的工具命令调用，须另行设计并回 `proposal.md` 走变更控制
- **最低支持版本:** OpenHarmony 7.1（`@since 26.0.0 dynamiconly`，与 `cliManager` 模块既有声明一致）
- **API 版本号策略:** 新增属性沿用模块既有 `@since 26.0.0 dynamiconly` + `@systemapi` + `@stagemodelonly` + `@syscap SystemCapability.Ability.AgentRuntime.Core` 标注；中英 `.d.ts` 同步修改

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 权限校验不得绕过 | 工具命令模式必须经过与 `execTool` 同一权限门（系统应用判定 + `ohos.permission.EXEC_CLI_TOOL` 校验），不得因新增入口放宽或跳过 | AC-1.1 |
| 沙箱执行不得绕过 | 工具命令必须在既有工具沙箱约束下执行，不得直接以服务端进程身份执行命令 | AC-1.1, AC-1.3 |
| 命令日志脱敏 | `cmd` 含路径与参数，日志必须脱敏输出（Release 构建不可见原文） | AC-1.1, AC-1.3 |
| IPC 序列化只可尾部追加 | 新增字段只能追加在既有字段之后，不得调整既有字段顺序或改变既有字段编码方式 | AC-1.8 |
| 已发布错误码数值不可变 | 复用既有内部码与对外码映射，不新增、不改写错误码数值 | AC-1.3..AC-1.6, AC-1.9 |
| 不支持语法必须显式拒绝 | 短标志/前缀参数/条件标志/双引号/反斜杠转义一律拒绝，不得静默忽略或按字符串兜底传给工具 | AC-1.6, AC-1.7 |
| 会话不留残 | 会话建立过程中任一步失败后，不得残留可查询会话或孤儿进程组 | AC-1.1 |

## 非功能性需求

> N/A 判定见 `proposal.md`「不涉及项确认」（性能/内存/存储 = 否）。本节仅填适用项。

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 性能 | N/A：按需低频调用路径，非热路径；无每帧/每事件开销，复用既有校验与进程链路 | — | `proposal.md`「不涉及项确认」 |
| 功耗 | N/A：无常驻任务、无新增定时器（超时/后台化任务沿用既有 `ExecOptions` 语义） | — | 同上 |
| 内存 | N/A：无常驻结构增量；命令字符串与解析结果为单次调用内临时对象 | — | 同上 |
| 安全 | 权限门（系统应用 + `ohos.permission.EXEC_CLI_TOOL`）+ 工具名白名单 + schema 校验 + 沙箱执行；命令日志脱敏；不新增权限、不放宽既有校验 | 安全测试（VM-3）+ 单测（VM-1, VM-4, VM-7） | `cli_tool_mgr_service_test.cpp`、`tool_util_test.cpp` |
| 可靠性 | 会话建立失败不留残（无残留会话、无孤儿进程组）；会话数上限（默认 8）防资源耗尽 | 单测（VM-10, VM-12） | `cli_tool_mgr_service_test.cpp` |
| 可测试性 | 命令解析、白名单、IPC 缺省回退、会话上限均可在部件边界独立构造与断言（无需真实工具进程即可覆盖拒绝路径） | 单测（VM-3..VM-9） | `tool_util_test.cpp`、`exec_cmd_param_test.cpp`、`cli_tool_mgr_client_test.cpp` |
| 自动化维测 | 执行失败上报沿用既有 CLI 执行事件上报通道，携带失败原因分类（工具名非法、工具不存在、参数非法等），不新增 `hisysevent.yaml` 事件定义 | 既有用例 + 上报字段核查 | `cli_event_report`（既有） |
| 定界定位 | 对外错误码可区分：201/202（权限）、401（命令与参数非法）、35600030（工具/子命令不存在）、35600031（会话上限）、35600050（内部失败）；命令原文日志脱敏、失败原因可分类 | hilog + 错误码 | `cli_manager_error_utils.cpp:40-51` |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 | 验证方式 | 证据 |
|----------|----------|-----------|----------|------|
| 手机 | 无差异 | 行为由工具注册信息与系统参数决定，与设备类型无关 | 单测 | `cli_tool_mgr_service_test.cpp` |
| 平板 | 无差异 | 同上 | 单测 | 同上 |
| 折叠屏 | 无差异 | 同上 | 单测 | 同上 |
| 其他（手表/车机等） | 无差异；实际可用工具集取决于该设备是否预置对应 CLI 工具 | 工具未注册时统一 reject 35600030 | 单测（VM-4） | `tool_util_test.cpp` |

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | N/A | 无 UI、无用户可感知字符串 | — |
| 大字体 | N/A | 无 UI | — |
| 深色模式 | N/A | 无 UI | — |
| 多窗口/分屏 | N/A | 无窗口行为 | — |
| 多用户 | 否 | 权限门基于调用方 token 判定系统应用与权限，沿用既有 `execTool` 语义，未新增跨用户访问面 | AC-1.1 |
| 版本升级 | 是 | 旧客户端 → 新服务端兼容（尾部字段缺省回退）；新客户端 → 旧服务端兼容但不等价，工具命令模式要求同版本配套 | AC-1.2, AC-1.8, AC-1.10 |
| 生态兼容 | 是 | System API（`@systemapi`），仅系统应用可用，不对三方应用开放；新增属性为可选，既有调用方零改造 | AC-1.2, AC-1.10 |

## 行为场景（可选，Gherkin）

N/A：复杂度为「标准」（L1），按模板二选一规则使用上方「接口规格 → 行为场景」表（12 个场景），不重复填写 Gherkin。

## 事实冲突与澄清项

> 依据 `ohos-spec` 规则 10（API/错误码是事实契约，发现事实冲突先回 proposal/spec 澄清）。核对**既有代码与已发布声明**时发现 3 项与 legacy 文档不一致的事实，其中 Q-S1、Q-S2 已由需求方于 2026-09-09 给出结论并关闭；Q-S3 为跨仓交付项，转 `execution-plan.md` 跟踪。三项结论均未改变 `proposal.md` 的范围、AC 或 API 变更项，故不触发基线重新批准。

| 编号 | 冲突事实（既有代码/声明） | 与何者冲突 | 需求方结论（2026-09-09，zhoulanzhi） | 状态 |
|------|----------------------|------------|--------------------------------------|------|
| Q-S1 | 工具命令模式下 `ExecCmdOptions.workDir` / `env` / `policy` **不生效**：工具路径沙箱配置只注入调用方身份标识（`tool_util.cpp:187-219`），三项仅由 shell 路径写入（`tool_util.cpp:221-241`）；`ExecToolParam` 亦无对应字段（`exec_tool_param.h:30-37`） | `.d.ts` 对三项属性的语义声明（`@ohos.app.cli.cliManager.d.ts:86-155`）未区分模式，调用方可能预期两模式均生效 | **不区分即可**：接受现状为预期行为，不判定为缺陷；`.d.ts` 与本文档均不为三项属性增加模式差异标注；不回 `proposal.md` 变更控制 | 已关闭（固化于 R-16 / BF-3 / VM-13） |
| Q-S2 | 未传 `timeout` 时存在**两条不同默认路径**：传了 options 对象 → `timeout = 1800`（`js_cli_manager_utils.cpp:214-217`，与 `.d.ts` `@default 1800` 一致）；完全不传 options 对象 → `timeout = 0`（`exec_options.h:34`，不限时） | legacy `spec.md` EB-2 记为「`timeout` 未传 → 默认 `0`，不注册超时任务」，未区分两条路径 | **按事实来**：以既有代码事实为准（工具命令模式必然携带 options 对象，故缺省 1800 秒）；legacy EB-2 描述作废；两条默认路径保持现状、不统一 | 已关闭（固化于 R-17 / BF-2 / 接口规格参数约束） |
| Q-S3 | 既有 `.d.ts` **尚未包含** `isShellCommand` / `challenge`，且 `execCmd` 的 `@throws` 仍只列 201/202/35600031/35600050（`@ohos.app.cli.cliManager.d.ts:85-155,398-415`） | `proposal.md` R-1（跨仓声明同步交付）预期在主仓实现后同步声明 | 属跨仓交付项而非语义冲突：由 `interface_sdk-js` 提交声明 PR（中英同步），内容 = 新增两个可选属性 + `@throws` 补 401 与 35600030；依 Q-S1 结论**不**为 `workDir`/`env`/`policy` 增加模式差异说明 | 待交付（转 `execution-plan.md` 跨仓任务跟踪） |

## context-references

```yaml
context-queries:
  - repo: "openharmony/ability_ability_runtime"
    query: "cli_tool_framework 中 execTool 的权限校验、沙箱配置生成与会话/进程管理链路（工具命令模式复用对象）"
  - repo: "openharmony/ability_ability_runtime"
    query: "cli_tool_framework 工具注册信息（toolInfo/subcommands/inputSchema）的来源与加载方式，用于确认白名单字符集覆盖全部已注册工具名"
  - repo: "openharmony/interface_sdk-js"
    query: "@ohos.app.cli.cliManager.d.ts 中英双份的声明现状与 @throws 补充落点（跨仓交付项 proposal R-1 / 本文 Q-S3）"
```

**关键文档：** `.codespec/changes/issue-16082-execcmd-clitool/proposal.md`（Approved 基线）、`.codespec/changes/issue-15383-cli-exectool/`（execTool 既有能力）、`.codespec/changes/issue-16082-execcmd-clitool-legacy-template/spec.md`（旧模板参考，冲突处以本文为准）
