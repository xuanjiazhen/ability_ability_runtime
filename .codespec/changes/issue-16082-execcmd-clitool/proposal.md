---
phase_status: approved
approval:
  status: approved
  approver: "zhoulanzhi"
  evidence: "Q-1..Q-4 澄清项已关闭，批准对象为 baseline_digest 所指的需求基线 v1.0"
  approved_at: "2026-09-08"
  baseline_digest: "3db9de167ccfa8ec17a01509d4a5997606b05d23a643df94d4cbb979a563a0e9"
target_release:
  id: OpenHarmony-7.1-Release
  status: proposed
---

# 需求文档

> 主文档只保存需求输入和已确认基线。本次为 `issue-16082` 按 `ohos-sdd/v1` 模板重建的 proposal。
> 附录「澄清与决策记录」因**多仓交付**（ability_runtime + interface_sdk-js）触发启用；澄清项 Q-1..Q-4 已全部关闭。

## 需求输入

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | REQ-20260626000935（特性编号 `20260626000935`，条目号 issue-16082） |
| 需求名称 | execCmd 支持工具命令模式：与 execTool 同源的受控工具命令执行通道 |
| 来源 | Issue（issue-16082）+ 特性编号 20260626000935 |
| 提出人 | zhoulanzhi |
| 目标发行版本 | OpenHarmony-7.1-Release |
| 候选 Profile | none |
| 优先级 | P0 |
| Define 阶段状态 | Approved |

### 原始描述

**原始问题：** 以下为 legacy 文档记录的问题陈述原文，保留不做二次技术翻译（本基线按需求方已确认口径固化，原话补齐不再作为基线前置条件）：

> 为 `cliManager.execCmd` 提供一条与 `execTool` 同源的受控工具命令执行通道。当前 `execCmd` 仅支持 shell 命令：`cmd` 字符串透传给 `/bin/sh -c "<cmd>"` 在沙箱中执行，无法调用 CLI 工具注册表中已注册的工具（`ohos-aa`、`ohos-hdc` 等）；而 `execTool` 只接受结构化参数，LLM 直接产出的工具命令字符串（如 `ohos-aa start --bundleName=com.example --abilityName=EntryAbility`）必须先由调用方自行解析为 toolName + params 才能执行。

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 应用开发者（AI/Agent 业务集成方） | `execCmd` 只能执行 shell 命令，无法调用已注册 CLI 工具；LLM 产出的工具命令字符串需自行解析为结构化参数再走 `execTool`，解析规则由各调用方各自实现 | 集成成本重复、各调用方解析行为不一致，工具命令无统一执行入口 |
| 系统开发者（CLI 工具框架系统侧调用方） | `execCmd`（shell）与 `execTool`（结构化）是两条独立路径，调用方需自行判断该用哪个接口 | 接口语义割裂、易误用，能力难以对外统一表达 |
| 最终用户 | 无直接感知（间接：Agent 类业务调用工具链的成败与稳定性） | Agent 业务功能可用性 |
| 既有 shell 命令调用方 | 任何新增分流若改变默认行为即构成回退 | 全量升级回归风险，必须零改造 |

**期望结果：** 调用方用同一个接口即可执行 shell 命令与已注册 CLI 工具命令两类场景；工具命令的执行与 `execTool` 走同一套权限校验、沙箱与进程管理约束，不因新增入口扩大命令执行攻击面；既有 shell 调用方零改造完成版本升级。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| Issue | issue-16082（完整链接待需求方补充） | 需求条目来源；离线未取到正文 |
| 历史文档（旧模板） | `.codespec/changes/issue-16082-execcmd-clitool-legacy-template/{proposal,spec,design,execution-plan}.md` | 旧模板产物，本基线的需求内容来源 |
| 源码 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp:1046-1073` | `ExecCmd` 既有权限校验 + 会话上限 + shell 路径入口 |
| 源码 | `cli_tool_framework/interfaces/cli_tool/src/exec_cmd_param.cpp:22-97` | `ExecCmdParam` 既有字段序与 Marshalling/Unmarshalling 实现 |
| 源码 | `cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h:33,38,48,53` | 既有内部错误码，本次无需新增数值 |
| 关联需求 | `.codespec/changes/issue-15383-cli-exectool` | `execTool` 既有校验/沙箱链路，本次复用对象 |
| 本分支代码现状 | `cli_tool_framework/{interfaces/cli_tool, services/climgr, frameworks/js/napi/cli_tool_manager, test/unittest}` | 基线按**本分支现状**描述（不追溯具体 commit），现状即本需求的事实基线 |

## 需求基线

### 基线信息

| 字段 | 内容 |
|------|------|
| 基线版本 | v1.0 |
| 基线日期 | 2026-09-04 |
| Owner | CLI 工具框架负责人（团队级 Owner） |
| 确认人 | zhoulanzhi（需求提出人） |
| 确认依据 | 2026-09-08 会话内明确批准原文「批准 define」；澄清关闭记录见本文档附录 Q-1..Q-4；批准对象为 `approval.baseline_digest` 所指基线 |
| 复杂度 | 标准 |
| Profile | none |
| 目标发行版本 | OpenHarmony-7.1-Release（引用 frontmatter `target_release.id`） |
| 版本状态 | proposed |
| 基线结论 | 通过（Q-1..Q-4 澄清项全部关闭；需求提出人 zhoulanzhi 于 2026-09-08 批准 Define，基线 v1.0 生效） |

### 问题、目标与成功指标

**问题陈述：** `execCmd` 与 `execTool` 是两条互不相通的命令执行通道：前者只接受 shell 字符串、后者只接受结构化参数。AI/Agent 业务方拿到的是 LLM 直接产出的工具命令字符串，必须先自行实现一套解析规则才能调用已注册 CLI 工具，解析行为在各调用方之间不一致且无法统一受控。现在解决的原因是 Agent 类业务已进入集成期，各调用方重复实现解析会带来不一致的命令执行语义与不可控的攻击面。受影响用户为 AI/Agent 业务集成方、CLI 工具框架的系统侧调用方，以及必须零改造升级的既有 shell 命令调用方。

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| 同一接口覆盖 shell 与工具命令两类执行场景 | 调用方以工具命令模式提交已注册工具的命令字符串，一次调用即被执行，会话信息可查询且工具名与运行状态正确 | 集成测试 + 单元测试 |
| 不因新增入口扩大命令执行攻击面 | 含路径分隔符/注入字符的工具名、未注册工具、未声明子命令、空白命令、schema 外参数一律被拒，且拒绝时不产生子进程 | 安全测试 + 单元测试 |
| 既有 shell 调用零改造升级 | 调用方未声明工具命令模式（或显式声明 shell 模式）时，会话结果与改动前一致，既有 shell 用例全绿无回退 | 回归测试 |
| 新旧客户端/服务端混布不崩溃 | 旧客户端不提交新增模式字段时，服务端按 shell 模式处理，调用不崩溃、结果与旧版一致 | IPC 兼容性测试 |
| 权限与沙箱约束与 `execTool` 同源 | 工具命令模式下的权限拒绝分支、会话数上限、沙箱执行约束与 `execTool` 表现一致 | 单元测试 + 设计约束核查（design.md 信任边界） |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为 AI/LLM 业务调用方，我想要把拼接好的工具命令字符串直接交给 `execCmd` 执行，以便无需自行解析为结构化参数即可调用已注册 CLI 工具，一个接口统一覆盖 shell 与工具命令两类场景 | P0 |
| US-2 | 作为既有 shell 命令调用方，我想要在未声明工具命令模式时行为完全不变，以便零改造成本完成版本升级 | P0 |
| US-3 | 作为系统，我要确保工具命令模式与 `execTool` 共用同一权限/沙箱校验链路并拦截非法工具名与非法命令，以便不因新增入口扩大命令执行攻击面 | P0 |
| US-4 | 作为系统，我要保证新增字段在跨进程传输中可缺省读取，以便新旧客户端/服务端混布时不崩溃、旧调用方行为不变 | P1 |

> **AC 编号冻结说明**：AC 沿用 legacy 基线的 `AC-1.x` 编号（`1` 为特性序号，非 US 序号），因为 legacy `design.md`（DD-1..DD-6、信任边界）、`execution-plan.md`（AC→Task 追溯）与**本分支现状的实现与单测**均按该编号追溯，重编号会打断追溯链。本表只写业务可观察期望；精确错误码数值、字段默认值、Parcel 字段序等接口细节由 `spec.md` 持有。

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-1.1 | WHEN 系统应用且持有 CLI 工具执行权限的调用方以工具命令模式提交已注册工具的命令字符串 THEN 命令被执行并返回可查询的会话信息（工具名正确、会话处于运行态）；WHEN 调用方非系统应用或未持有该权限 THEN 请求被拒且不建立会话 | 正常 | US-1, US-3 |
| AC-1.7 | WHEN 工具命令使用长标志语法（`--key=value`、`--key value`、`--flag`、`--flag false`）或以单引号包裹含空格的值 THEN 参数按工具 schema 恢复为正确类型后生效；WHEN 参数位置出现短标志/前缀参数/条件标志，或引号未闭合，或使用双引号/反斜杠转义 THEN 请求被拒 | 正常 | US-1 |
| AC-1.2 | WHEN 调用方未声明工具命令模式 THEN 命令按原 shell 路径执行，会话结果与改动前一致、无行为回退 | 正常 | US-2 |
| AC-1.10 | WHEN 调用方显式声明 shell 模式 THEN 命令按原 shell 路径执行，会话结果与改动前一致、无行为回退 | 正常 | US-2 |
| AC-1.3 | WHEN 命令中提取出的工具名含路径分隔符或注入类非法字符（如 `/system/bin/ohos-aa`、`../ohos-aa`、`ohos-aa;rm`）THEN 请求以「参数无效」被拒且不创建子进程 | 异常 | US-3 |
| AC-1.4 | WHEN 工具名未在 CLI 工具注册表中注册，或第二 token 非标志前缀且不在该工具已声明的子命令集合内 THEN 请求以「工具不存在」被拒 | 异常 | US-3 |
| AC-1.5 | WHEN 命令为纯空白 THEN 请求以「参数无效」被拒 | 异常 | US-3 |
| AC-1.6 | WHEN 命令含工具 schema 未定义的标志，或存在无法归属到任何标志的孤立值 THEN 请求以「参数无效」被拒 | 异常 | US-3 |
| AC-1.9 | WHEN 并发会话数已达上限 THEN 请求以「进程数已达上限」被拒（上限校验与 shell 路径共用同一入口） | 边界 | US-3 |
| AC-1.8 | WHEN 旧客户端不提交新增的模式与认证标识字段 THEN 服务端按 shell 模式处理、调用不崩溃、结果与旧版一致；反方向（新客户端 → 旧服务端）为兼容但不等价，工具命令模式要求客户端与服务端同版本配套 | 边界 | US-4 |

### 范围边界

**包含：**

- 服务端 `execCmd` 增加工具命令模式分流，复用 `execTool` 既有权限校验、沙箱配置生成、会话与进程管理链路
- 工具命令字符串的分词与 schema 驱动参数类型恢复（长标志语法子集 + 单引号包裹含空格值）
- 工具名白名单校验，拦截路径注入与命令拼接字符
- 跨进程数据结构在**尾部**追加「模式标识」与「认证标识」两字段，并保证旧客户端缺省时回退默认值
- NAPI 层 `ExecCmdOptions` 两个可选属性的解包与默认值一致性
- `interface_sdk-js` 中 `@ohos.app.cli.cliManager.d.ts`（中英）声明新增与 `@throws` 补充
- 对应单元测试、回归测试、安全测试与 IPC 兼容性测试

**不包含：**

- 既有 shell 路径的任何行为变更
- 短标志（`-b`）、前缀参数、条件标志、双引号与反斜杠转义的解析支持（列入后续迭代）
- `execTool` 接口签名变更
- `ICliToolManager.idl` 中 `ExecCmd` 接口签名变更（`ExecCmdParam` 已是 sequenceable）
- 命令字符串长度上限校验（既有 `execTool` 与 `execCmd` 均无显式长度限制，本次保持一致、暂不新增，见风险 R-2）
- 显式 IPC 版本协商字段
- 新增权限、新增沙箱逻辑、新增第三方依赖、新增构建目标/部件
- `templates` 配置与 `required` 校验补齐（与 `execTool` 保持一致，暂不补齐）

| 阶段（如适用） | 范围 | 不做 | 进入条件 |
|----------------|------|------|----------|
| 本次交付（完整） | 长标志语法子集的工具命令模式 + 白名单校验 + IPC 尾部兼容 + .d.ts 声明 | 短标志/前缀参数/条件标志/双引号/反斜杠转义 | 本基线批准 Define |
| 后续迭代 | 完整命令行语法解析（短标志、前缀参数、条件标志、引号与转义）、命令长度上限策略 | — | 业务方明确提出 + 安全评估通过 + 重新基线 |

### Agent Scope Guard

> 只约束需求探索和上下文收集阶段可访问的仓库、模块与知识源，不维护实现文件清单。精确代码文件范围由 `execution-plan.md` / `task.md` 持有。

| 维度 | 允许范围/限制 | 越界触发条件 | 处理规则 |
|------|---------------|--------------|----------|
| 允许检索仓库 | `ability_runtime`（本仓，`cli_tool_framework`）；`interface_sdk-js`（仅 `@ohos.app.cli.cliManager.d.ts` 中英声明核对，当前不在工作区，需人工提供） | 需要访问未列仓库（如 `interface_sdk-js` 之外的 SDK 仓、`abilitymgr` 上游依赖仓） | 停止探索，补充影响分析并重新确认基线 |
| 允许模块/目录 | `cli_tool_framework/{interfaces/cli_tool, services/climgr, frameworks/js/napi/cli_tool_manager, test}`；`interface_sdk-js/api/@ohos.app.cli.cliManager.d.ts`(+`zh-cn`) | 需要扩大到 `services/abilitymgr`、`services/appmgr`、`frameworks/native/child_process` 等其他子系统 | 更新范围与影响范围，经 Owner 确认后继续 |
| 禁止访问/修改 | `ICliToolManager.idl` 的 `ExecCmd` 签名；既有 shell 路径行为；`hisysevent.yaml` 既有事件定义；已发布公共错误码数值（`ability_business_error.h`、`ability_runtime_error_util.h`、`cli_error_code.h`）；构建生成物（`services/dialog_ui/**/build`、`.hvigor`、`local.properties`）；`../issue-16082-execcmd-clitool-legacy-template/`（只读审计参考，不改写） | 命中禁止项 | 标记 Blocked，不读取、不修改 |
| 外部网络/私有知识源 | 当前会话离线：`oh-gc` 不可用，issue-16082 原话与跨仓 `.d.ts` 现状未检索到。已授权知识源 = 本仓源码 + `AGENTS.md`/`CLAUDE.md` + legacy 目录旧文档 + ohos-sdd 模板/profile 资产 | 需要拉取 issue 原文、跨仓声明文件或外部知识库 | 请求人工授权或由需求方提供原文；未授权前不得据猜测填写「原始描述」与跨仓声明现状 |

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| CLI 工具框架 | ability_runtime | `cli_tool_framework/interfaces/cli_tool`（`exec_cmd_param`、`cli_tool_mgr_client`） | 客户端侧命令参数结构与 IPC 序列化/反序列化 | 修改（尾部追加字段）+ InnerAPI | CLI 工具框架 |
| CLI 工具框架 | ability_runtime | `cli_tool_framework/services/climgr`（`cli_tool_manager_service`、`tool_util`） | 服务端命令执行入口、权限校验、沙箱配置、会话与子进程管理 | 修改 + 新增（工具命令分流与命令解析） | CLI 工具框架 |
| CLI 工具框架 | ability_runtime | `cli_tool_framework/frameworks/js/napi/cli_tool_manager`（`js_cli_manager`、`js_cli_manager_utils`） | NAPI 绑定与 `ExecCmdOptions` 解包 | 修改（新增可选属性解包与默认值） | CLI 工具框架 |
| CLI 工具框架 | ability_runtime | `cli_tool_framework/test/unittest/*`（`tool_util_test`、`cli_tool_mgr_service_test`、`cli_tool_mgr_client_test`、`exec_cmd_param_test`） | 既有单元测试 | 新增用例 | CLI 工具框架 |
| 公共 API | interface_sdk-js | `api/@ohos.app.cli.cliManager.d.ts`（中英） | `cliManager` System API 声明 | API（新增 2 个可选属性 + `@throws` 补充） | interface_sdk-js 声明仓维护者 |

### API 变更项清单

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| `cliManager.ExecCmdOptions.isShellCommand` | 新增 | System | 可选布尔，默认 `true`（shell 模式）；置 `false` 时 `cmd` 按已注册工具命令字符串解析执行 |
| `cliManager.ExecCmdOptions.challenge` | 新增 | System | 可选字符串，默认 `""`；由调用方从 access token manager 获取并透传给工具进程，服务端不生成 |
| `cliManager.execCmd` | 修改 | System | 函数签名与返回值不变，行为扩展（新增工具命令分支）；`@throws` 需补充参数无效与工具不存在两类错误 |
| `ICliToolManager.ExecCmd` / `ExecCmdParam` | 修改 | InnerAPI | 接口签名不变；序列化格式在 Parcel 尾部追加两字段，读取失败回退默认值 |

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 否 | CLI 命令执行为按需低频路径，非热路径；工具命令模式复用既有校验/沙箱/进程链路；命令分词为单次 O(n) 解析，无每帧/每事件开销。无常驻内存增量（命令字符串与解析结果为单次调用内临时对象），无新增镜像产物或大体量预置数据（仅既有 `.so` 内增量代码） | N/A |
| 安全与权限 | 是 | 命令执行路径跨信任边界（IPC/Binder）；复用权限校验链路且不绕过 `PermissionVerification` 与沙箱配置生成；处理 access token challenge 认证标识；工具名白名单防路径注入；命令日志需脱敏 | design.md / spec.md |
| 兼容性 | 是 | IPC Parcel 尾部追加字段需向后兼容（旧→新兼容；新→旧兼容但不等价）；System API 新增可选属性需默认行为不变 | spec.md |
| API/SDK | 是 | `ExecCmdOptions` 新增 2 个可选 System API 属性 + `execCmd` 文档 `@throws` 补充（已与开发者确认涉及 API/SDK） | design.md / spec.md |
| IPC/跨进程 | 是 | `ExecCmdParam` Marshalling/Unmarshalling 变更，跨进程 CLI 工具管理调用 | design.md |
| 构建与部件 | 否 | 仅修改既有源文件与既有测试目标，无新增构建目标/部件/第三方依赖 | N/A |
| 国际化/无障碍 | 否 | 无 UI 文本、无用户可感知字符串；命令解析不涉及国际化 | N/A |
| 数据迁移 | 否 | 无持久化数据格式变更（不涉及 RDB/文件/配置格式）；IPC Parcel 属运行时序列化，归「兼容性」维度 | N/A |

### 依赖、风险与外部约束

| 依赖/风险 | 类型 | 影响 | 缓解措施/Owner | 状态 |
|-----------|------|------|----------------|------|
| R-1 `interface_sdk-js` 的 `.d.ts` 跨仓同步交付（中英双份 + `@throws` 补充） | 外部 | 中 | 主仓实现 PR 合入后由 interface 仓提交声明 PR，不另起独立流程；Owner=CLI 工具框架 + interface_sdk-js 声明仓维护者 | 已确认（随基线 v1.0 批准） |
| R-2 命令字符串最大长度未定义（框架内无长度上限常量） | 技术 | 低 | 需求方结论：既有 `execTool` 与 `execCmd` 均无显式长度限制，本次**暂时忽略**、不新增长度校验，与既有行为保持一致；实际约束来自 IPC Parcel 单次传输上限与 `execve` 参数上限，二者均不由本部件定义。后续如需长度策略须单独重新基线 / Owner=CLI 工具框架负责人 | 已确认 |
| R-3 新客户端 → 旧服务端语义不等价：旧服务端无工具命令分支，会把工具命令字符串当 shell 命令执行 | 技术 | 中 | 约束「工具命令模式要求客户端与服务端同版本配套」写入 spec 兼容性声明；不引入版本协商字段（后续如需协议层拒绝跨版本调用须另行设计） | 已确认 |
| R-4 新增攻击面：工具名路径注入 / 命令拼接字符 | 技术 | 中 | 工具名白名单 `[a-zA-Z0-9-_]` + 复用 `ValidateAndPrepareTool`，不绕过 `PermissionVerification` 与 `GenerateSandboxConfig`；实现前核对注册表内全部已注册工具名是否被白名单覆盖 | 已确认 |
| R-5 命令解析仅支持长标志子集，LLM 可能产出短标志/引号变体 | 技术 | 低 | 明确不支持的语法一律以「参数无效」拒绝（不静默降级、不误执行）；完整语法列入后续迭代 | 已确认 |

### 依据与上下文引用

| 来源 | 查询/读取内容 | 基线采用的结论 | 可信度 | 证据路径/链接 |
|------|---------------|----------------|--------|---------------|
| 源码 | `ExecCmdParam` 既有字段与序列化实现 | 既有字段序为 `cmd→workDir→env→policy→options`，新增字段只能尾部追加并在读取失败时回退默认值 | 高 | `cli_tool_framework/interfaces/cli_tool/include/exec_cmd_param.h:29-43`、`src/exec_cmd_param.cpp:22-97` |
| 源码 | `ExecCmd` 服务端入口 | 既有权限校验 + 会话上限校验对两条路径共用，分流点位于其后、shell 逻辑之前 | 高 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp:1046-1073` |
| 源码 | `ToolUtil` 命令解析与沙箱配置生成入口 | schema 驱动的参数类型恢复可复用既有工具信息，无需新增数据源 | 高 | `cli_tool_framework/services/climgr/include/tool_util.h:68-69` |
| 源码 | 既有内部错误码常量 | 参数无效/工具不存在/会话上限/未初始化等错误码均已存在，本次不新增错误码数值 | 高 | `cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h:33,38,48,53` |
| Agent 指南 | `AGENTS.md` §3 Constraints / Do not | 不改 IPC Stub/Proxy 序列化格式（本变更以尾部追加 + 缺省回退满足兼容）、不改已发布错误码数值、不为过测删日志/事件；`services/common` 的 `PermissionVerification` 是全局安全入口，不得绕过 | 高 | `AGENTS.md` |
| 旧模板文档 | legacy `spec.md` 的 BR-1..BR-8、EB-1..EB-10 与「兼容性声明」 | challenge 来源、工具名白名单字符集、引号支持三项开放问题在 legacy 基线中已关闭；EB-3（长度上限）已按「与既有 `execTool`/`execCmd` 一致、不新增校验」关闭 | 中（待需求方/Owner 确认） | `.codespec/changes/issue-16082-execcmd-clitool-legacy-template/spec.md` |
| 旧模板文档 | legacy `design.md` 的「代码事实基线」与「设计约束」 | `.d.ts` 现状：`ExecCmdOptions` 既有 workDir/env/background/yieldMs/timeout/policy/callback，`execCmd` `@throws` 现列 201/202/35600031/35600050，需补参数无效与工具不存在 | 中（跨仓未直接检索） | `.codespec/changes/issue-16082-execcmd-clitool-legacy-template/design.md` |
| Profile 资产 | `profiles/{arkweb,arkui,arkgraphic,arkdata}/profile.md` 的 `repos` 字段扫描 | 本仓 `ability_ability_runtime` 未命中任一 profile → `profile: none`、`profile_source: inferred` | 高 | ohos-sdd assets `profiles/*` |

### 变更控制与下游条件

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增用户故事或仓/模块（如扩展到 ANI/CJ 绑定、`abilitymgr` 侧改动） | 重新评估复杂度和影响范围，重新确认基线 |
| AC 变更 | 修改可观察行为或错误码（含调整白名单字符集、放开短标志支持） | 重新审批基线和 Spec |
| API 变更 | 新增/修改 Public/System API（`ExecCmdOptions` 属性增删或默认值变化） | 触发设计审批 |
| Agent 探索范围变更 | 需要访问未列仓库/模块、禁止项或新知识源（如拉取 issue 原文、跨仓 `.d.ts`） | 停止探索，更新 Agent Scope Guard 后重新确认基线 |
| 非功能指标变更 | 性能/安全/兼容性阈值变化（如新增命令长度上限、引入版本协商） | 重新确认验证计划 |
| 目标版本变更 | 交付版本调整（OpenHarmony-7.1-Release → 其他） | 更新 `proposal.target_release` |

- [x] 所有 P0/P1 用户故事有可测试、可度量的 AC
- [ ] 范围内外、影响范围、API 变化和不涉及项已确认（内容已填齐，待 Owner/需求方确认）
- [x] `proposal.target_release` 已确认为具体版本（`OpenHarmony-7.1-Release` / `proposed`）
- [x] `manifest.profile` 已确认或明确 none
- [x] Agent Scope Guard 已基线化，未复制 Plan/Task 的实现文件清单
- [x] 基线结论为「通过」，且确认人和确认依据完整（zhoulanzhi，2026-09-08）
- [x] 条件附录触发时，待澄清项和争议决策已关闭（Q-1..Q-4 全部关闭）

## 附录：澄清与决策记录（条件启用）

> 触发原因：**多仓交付**（`ability_runtime` + `interface_sdk-js`）。复杂度为「标准」，澄清项已全部关闭，故本附录只记录触发项与决策，不展开常态化澄清日志。

### 触发与待澄清问题

| 项目 | 内容 | 验证方式/依据 | 状态 |
|------|------|---------------|------|
| 初始假设 | `ExecToolParam` 已支持 challenge 透传，可直接复用 | `execTool` 既有实现源码核对 | 已验证 |
| 初始假设 | 已注册 CLI 工具的 schema 可驱动命令参数类型恢复 | `tool_util` + 工具注册表数据管理源码核对 | 已验证 |
| 初始假设 | 长标志语法子集足以覆盖 LLM 产出的工具命令 | 需求方/legacy 基线 BR-8 | 已验证 |
| 基线口径 | 基线按**本分支现状代码**描述，不追溯分支上的具体 commit；本分支现状即事实基线 | 本分支 `cli_tool_framework` 源码现状 | 已确认 |
| 复杂度触发 | 标准：跨 NAPI/客户端/IPC/服务端多层 + 2 仓交付，但复用既有 `execTool` 链路，无新安全模型、无新数据面 | 分级判断依据 | 已确认 |
| 附录触发 | 多仓交付（非 complex/critical） | 本附录开头声明 | 已确认 |

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | `challenge` 字段来源与获取时机是否与 `execTool` 完全一致 | 影响 API 语义与调用方改造成本 | 已澄清：由调用方从 access token manager 获取并透传，服务端不生成、不改值 |
| Q-2 | 工具名白名单字符集 `[a-zA-Z0-9-_]` 是否覆盖所有已注册工具名 | 覆盖不全会误拒合法工具 | 已澄清：采用该白名单；实现前核对工具注册表（`ohos-aa`、`ohos-hdc` 等均命中） |
| Q-3 | 带空格的参数值是否需引号支持 | 影响解析器范围与拒绝策略 | 已澄清：支持单引号包裹（`--name 'hello world'`）；双引号与反斜杠转义不支持，一律拒绝 |
| Q-4 | 是否需为命令字符串新增长度上限校验，精确字节上限为多少 | 影响边界规则与拒绝服务面（R-2） | 已澄清：既有 `execTool` 与 `execCmd` 均无显式长度限制，暂时忽略、不新增长度校验 |

### 讨论与方案决策

| 日期 | 参与人 | 讨论主题 | 结论 | 后续动作 |
|------|--------|----------|------|----------|
| 2026-09-03 | zhoulanzhi（提出人）/ CLI 工具框架 | `execCmd` 工具命令模式的方案选型 | 采用 A-1（复用 `execTool` 链路），不新增权限与沙箱逻辑 | 按本基线进入 Specify（Q-1..Q-4 已全部关闭） |

| 方案 | 核心思路 | 优势 | 风险/代价 | 结论 |
|------|----------|------|-----------|------|
| A-1 | 服务端 `execCmd` 内增加分流，复用 `execTool` 的校验/沙箱/会话/进程链路，仅新增命令分词与 schema 类型恢复 | 安全语义与 `execTool` 同源、无重复安全代码、两路径会话/沙箱/监控一致 | 需新增分词器；长标志子集限制需明确拒绝策略 | 推荐（采纳） |
| A-2 | 为工具命令模式独立实现校验与进程管理 | 与 shell 路径完全解耦，可独立演进 | 重复安全代码、易出现校验遗漏、攻击面翻倍 | 放弃 |
| A-3 | 在 NAPI/客户端侧把命令字符串解析为结构化参数后走 `execTool` | 服务端零改动 | 解析规则需在 NAPI/ANI/CJ 各绑定重复实现；校验下沉到不可信侧，绕过服务端信任边界 | 放弃 |

### 检索与影响分析

| 来源 | 查询/读取内容 | 关键发现 | 可信度 | 用于 | 命中/未查询原因 |
|------|---------------|----------|--------|------|-----------------|
| 源码 | `ExecCmd`、`ExecCmdParam`、`ToolUtil` 相关实现与头文件 | 分流点位于权限/会话上限校验之后；序列化字段可尾部追加；错误码常量齐备无需新增 | 高 | 范围/API/设计 | 命中 |
| 源码 | `cli_tool_framework/test/unittest/*` 既有用例 | `tool_util_test`、`cli_tool_mgr_service_test`、`cli_tool_mgr_client_test`、`exec_cmd_param_test` 已存在，AC 可挂到既有测试目标 | 高 | 测试 | 命中 |
| Agent 指南 | `AGENTS.md` 约束与禁止项 | IPC 序列化格式、已发布错误码、`hisysevent.yaml`、`PermissionVerification` 为硬约束，已写入 Scope Guard 禁止项 | 高 | 范围/安全 | 命中 |
| gitcode issue | issue-16082 正文与需求方原话 | 未获取 | — | 原始描述 | 未命中：离线环境，`oh-gc` 不可用 |
| 跨仓源码 | `interface_sdk-js` 的 `@ohos.app.cli.cliManager.d.ts` 现状 | 未直接检索，现状依据 legacy `design.md` 记录 | 中 | API | 未查询：跨仓不在本工作区，需人工提供或授权 |

| 维度 | 澄清结论 | 确认人 | 状态 |
|------|----------|--------|------|
| 子系统与仓 | `ability_runtime`（`cli_tool_framework`）+ `interface_sdk-js`（`.d.ts` 中英） | zhoulanzhi | 已确认 |
| API 与权限 | 新增 2 个可选 System API 属性；权限沿用既有 CLI 工具执行权限，不新增权限、不绕过 `PermissionVerification` | zhoulanzhi | 已确认 |
| 兼容性与非功能 | IPC 尾部追加 + 缺省回退（旧→新兼容；新→旧兼容但不等价，需同版本配套）；性能/内存/存储均无增量关注点 | zhoulanzhi | 已确认 |
| 依赖与风险 | R-1 跨仓声明同步、R-2（长度上限暂时忽略）、R-3/R-4/R-5 结论均随基线批准生效 | zhoulanzhi | 已确认 |

### 澄清结论

- [x] 待澄清问题均已关闭或明确 N/A（Q-1..Q-4 全部关闭）
- [x] 范围、API、兼容性、非功能需求、依赖和风险已形成基线结论
- [x] complex/critical 已记录方案取舍和需求方/Owner/SIG 确认证据 —— N/A（复杂度为标准；方案取舍已按多仓触发记录于上表）
- [x] P0/P1 AC 覆盖正常、异常和边界路径（正常 AC-1.1/1.2/1.7/1.10；异常 AC-1.3/1.4/1.5/1.6；边界 AC-1.8/1.9）

**结论：** 通过

> 澄清项已全部关闭（Q-1..Q-4），基线结论为「通过」。需求提出人 zhoulanzhi 于 2026-09-08 明确回复「批准 define」，`approval` 的 status/approver/evidence/approved_at/baseline_digest 均已填写并与当前「需求基线」一致，`phase_status` 置为 `approved`，可进入 Specify（`ohos-spec`）。
> 后续若需求基线发生实质变更（范围/AC/API/兼容性/目标版本），必须将 `phase_status` 改回 `awaiting_approval`、重新生成 `approval-digest` 并重新取得批准；原 approval 字段保留作审计记录。
> 批准时运行 `ohos-sdd approval-digest .codespec/changes/issue-16082-execcmd-clitool` 取 64 位摘要写入 `approval.baseline_digest`；需求基线发生任何实质变更后须改回 `awaiting_approval`、重新生成摘要并重新取得批准。
