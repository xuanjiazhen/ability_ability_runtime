# Execution Plan

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Proposal | proposal.md | Approved |
| Spec | spec.md | Approved |
| Design | design.md | Approved |

> 注：本计划按正常前向流程编排，TASK-1（API 声明）排在首位，功能实现以 TASK-2 起；各 Task 须先写失败测试（RED）再做最小实现（GREEN），验证通过后回填状态。

## 执行原则

- **Spec 权威：** 若实现细节与 `spec.md` 的 AC、错误码或兼容性声明冲突，先更新 spec/design，再继续实现。
- **测试/证据先行：** 每个 Task 先写失败测试；无法单测的集成行为必须先写明可复现证据缺口。
- **任务小型化：** 一个 Task 只覆盖一个独立闭环。跨 API、事件链、状态缓存、渲染链、生命周期的需求必须拆成多个 Task。
- **文件边界：** Task 只能修改 `Files` 表列出的文件；若构建暴露额外声明或 fixture 需求，先更新本计划。
- **状态所有权唯一：** 新增状态必须明确 owner、key/index、创建时机、清理触发和只读消费者。
- **证据回填：** Task 完成后必须回填本计划「AC 到 Task 追溯」验证状态、「代码范围映射」实际文件、per-task `Actual Result`。
- **反伪完成：** 只补声明、只写存储结构、只覆盖 happy path、只跑非相关测试，都不能替代 AC 闭环。
- **可交接执行（Agent 执行契约）：** 本计划须能被新 Agent 在无历史对话上下文下逐 Task 执行；每 Step 为含命令或代码方向的 2–5 分钟动作。

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式 | 验证状态（Pass/Fail/Blocked） |
|----|------|------|----------|----------------------------------|
| AC-1.1 | spec.md | TASK-2, TASK-3, TASK-5, TASK-6 | 单测 ExecCmdToolMode 正常执行返回 CliSessionInfo | Pending |
| AC-1.2 | spec.md | TASK-3, TASK-2 | 回归测试：不传 isShellCommand 走 shell 路径 | Pending |
| AC-1.3 | spec.md | TASK-6 | 安全测试：toolName 含 `/`/`.`/`;` reject 401 无进程 | Pending |
| AC-1.4 | spec.md | TASK-6, TASK-5 | 单测：未注册 toolName/未知子命令 reject 35600030 | Pending |
| AC-1.5 | spec.md | TASK-4 | 单测：纯空白 cmd reject 401 | Pending |
| AC-1.6 | spec.md | TASK-5 | 单测：未知 flag reject 401 | Pending |
| AC-1.7 | spec.md | TASK-5 | 单测：4 种长标志格式 + 单引号空格值按 schema 恢复 | Pending |
| AC-1.8 | spec.md | TASK-2 | IPC 兼容单测：旧 Parcel 回退 shell 模式不崩溃 | Pending |
| AC-1.9 | spec.md | TASK-6 | 单测：会话达上限 reject 35600031 | Pending |
| AC-1.3/1.4/1.5/1.6（文档） | spec.md 接口变更分析 | TASK-1 | .d.ts execCmd @throws 含 401 与 35600030 | Pending |

## 实现边界

**必须实现：** ExecCmdOptions.isShellCommand/challenge 声明；ExecCmdParam 字段+尾部兼容序列化；NAPI 解析；客户端 cmd 校验+eventKey；ParseToolCommand（分词→子命令→参数→类型恢复）；ExecCmdToolMode 四步+SetupCmdSession 先订阅后创建+回滚；execCmd @throws 补 401/35600030。

**可后置：** 短标志/双引号/反斜杠转义/templates 配置/required 校验（与 execTool 统一，列入后续迭代）。

**不建议延后：** execCmd @throws 补 401/35600030（TASK-1，公共 API 文档与运行时一致性，延后则调用方误处理错误）。

## 禁止项

- 每个 AC 必须有明确的验证方式。
- Agent 不得自行寻找未列出的上下文文件作为修改依据；需要新增上下文时先更新 Task。
- 不得修改 Task 列出范围外的文件。
- 不得在未通过验证时标记 Task 完成。
- 不得使用 `TBD`、`TODO`、`适当处理`、`补充测试`、`参考上文` 等不可执行占位描述。
- 不得修改 `ICliToolManager.idl` 的 ExecCmd 签名（ExecCmdParam 已 sequenceable）。
- 不得变更既有 shell 路径（isShellCommand=true 默认）行为。
- 不得为通过测试删除既有 HiSysEvent 事件或 `%{private}s` 日志脱敏。

## Task 依赖

```mermaid
graph TD
    T1[TASK-1 .d.ts 声明+@throws]
    T2[TASK-2 ExecCmdParam 字段+序列化]
    T3[TASK-3 NAPI 解析]
    T4[TASK-4 客户端校验+eventKey]
    T5[TASK-5 ParseToolCommand]
    T6[TASK-6 服务端分流+ExecCmdToolMode+SetupCmdSession]
    T1 -.->|API 契约| T3
    T2 -->|ExecCmdParam 字段| T3
    T2 -->|ExtractToolName| T4
    T3 -->|ExecCmdParam| T4
    T2 -->|ExecCmdParam| T5
    T5 -->|ParseToolCommand| T6
    T4 -->|客户端 IPC| T6
```

## Task 列表

| TASK ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 状态 |
|---------|------|----------|---------|----------|----------|----------|------|
| TASK-1 | .d.ts ExecCmdOptions 声明 + execCmd @throws 补 401/35600030 | interface/sdk-js/api 与 zh-cn 的 @ohos.app.cli.cliManager.d.ts | AC-1.1, AC-1.3/1.4/1.5/1.6(文档) | 无 | isShellCommand?/challenge? 声明；execCmd @throws 含 401、35600030 | diff 检查 @throws 列表 | Pending |
| TASK-2 | ExecCmdParam 新增字段 + 尾部兼容序列化 + ExtractToolName | exec_cmd_param.h, exec_cmd_param.cpp | AC-1.1, AC-1.8 | 无 | 新增 isShellCommand/challenge/ExtractToolName；旧 Parcel 回退默认值 | exec_cmd_param_test | Pending |
| TASK-3 | NAPI 解析 isShellCommand/challenge | js_cli_manager_utils.cpp | AC-1.1, AC-1.2 | TASK-2 | UnwrapExecCmdOptions 解析两属性；默认 true | NAPI 模块编译 | Pending |
| TASK-4 | 客户端 cmd 非空校验 + eventKey=toolName | cli_tool_mgr_client.cpp | AC-1.1, AC-1.5 | TASK-2, TASK-3 | 工具模式 cmd 空白 reject 401；eventKey 用 ExtractToolName | cli_tool_mgr_client_test | Pending |
| TASK-5 | ParseToolCommand 分词→子命令→参数→类型恢复 | tool_util.h, tool_util.cpp | AC-1.6, AC-1.7, AC-1.4 | TASK-2 | 未知 flag reject；4 种长标志格式按 schema 恢复 | tool_util_test | Pending |
| TASK-6 | ExecCmd 分流 + ExecCmdToolMode 四步 + SetupCmdSession | cli_tool_manager_service.h, cli_tool_manager_service.cpp | AC-1.1, AC-1.3, AC-1.4, AC-1.9 | TASK-5, TASK-4 | 分流+白名单+GetToolByName+ParseToolCommand+ValidateAndPrepareTool+先订阅后创建+回滚 | cli_tool_mgr_service_test | Pending |

## Task 详情

### TASK-1: .d.ts ExecCmdOptions 声明 + execCmd @throws 补充

**目标：** 确认 `ExecCmdOptions.isShellCommand?`/`challenge?` 声明完整，并补齐 `execCmd` `@throws` 列表中的 401（参数无效）与 35600030（工具不存在）。

**AC 映射：** AC-1.1（API 面），AC-1.3/1.4/1.5/1.6（文档：@throws 与运行时错误码一致）

**前置依赖：** 无（声明文件为下游实现契约来源）

**非目标：** 不修改 execCmd 函数签名；不修改其他函数的 @throws；不新增声明文件。

**状态所有权：** 无（纯声明文档变更）。

**任务间接口：** Produces=`ExecCmdOptions.isShellCommand?: boolean`（默认 true）、`ExecCmdOptions.challenge?: string`（默认 ""）；Consumes=无。供 TASK-3 NAPI 解析对齐属性名。

**只读上下文**

| 路径 | 读取目的 |
|------|----------|
| spec.md `## API 规格定义` | 声明规格与 @throws 错误码集合 |
| interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts | 既有声明格式（@kit/@syscap/@systemapi/@stagemodelonly/@since） |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts | 新增 isShellCommand?/challenge? 声明；execCmd @throws 补 401 与 35600030 |
| 修改 | interface/sdk-js/zh-cn/api/@ohos.app.cli.cliManager.d.ts | 同上（中文版） |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| ICliToolManager.idl | ExecCmd 签名不变（ExecCmdParam 已 sequenceable） |

**Steps**

- [ ] Step 1: 定义证据缺口——execCmd 当前 @throws 缺 401/35600030。

```text
证据缺口：cli_tool_framework/interfaces/cli_tool/... 经 cli_manager_error_utils 映射，工具命令模式会产生 401（ERR_INVALID_PARAM）与 35600030（ERR_TOOL_NOT_EXIST），但 .d.ts execCmd @throws 仅列 201/202/35600031/35600050。复现：读 .d.ts 420-437 行 @throws 段，对照 spec.md 错误码定义表。
```

- [ ] Step 2: 确认缺口存在。

```bash
grep -n "@throws" interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts | sed -n '/execCmd/,+8p'
```

Expected: execCmd 的 @throws 段无 401 与 35600030 行。

- [ ] Step 3: 在 execCmd 的 @throws 段补充两行（en + zh-cn）。

```text
execCmd 函数 @throws 段在 35600031 前补充：
  @throws { BusinessError } 401 - Invalid input parameter. The cmd is empty, the toolName contains
      invalid characters, or the command contains an unknown flag.
  @throws { BusinessError } 35600030 - No tool with the specified name exists.
zh-cn 版同步补充对应中文描述。
```

- [ ] Step 4: 确认 GREEN。

```bash
grep -n "401\|35600030" interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts interface/sdk-js/zh-cn/api/@ohos.app.cli.cliManager.d.ts
```

Expected: 两个文件 execCmd 段均含 401 与 35600030 @throws。

- [ ] Step 5: 如环境可用，运行 oh-api-definition（可选，不阻塞）。
- [ ] 回填「AC 到 Task 追溯」验证状态与「代码范围映射」。
- [ ] 回填本 Task 的 `Actual Result`。

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 证明 .d.ts execCmd @throws 含 401/35600030，与 spec 错误码定义一致 |
| Scope respected | 证明仅修改 en/zh-cn 两个 .d.ts，未改 IDL/函数签名 |
| State lifecycle complete | N/A（纯声明文档） |

**Verification**

| Command / Evidence | Expected Result | Actual Result |
|--------------------|-----------------|---------------|
| grep @throws execCmd 段 | 含 201/202/401/35600030/35600031/35600050 | 待回填 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | @throws 与 spec 错误码定义表完全一致；未改签名 |
| Code Quality | en/zh-cn 两文件同步；diff 仅增 @throws 行 |

### TASK-2: ExecCmdParam 字段 + 尾部兼容序列化 + ExtractToolName

**目标：** ExecCmdParam 新增 `isShellCommand=true`/`challenge`/`ExtractToolName`，Parcel 序列化追加尾部两字段，Unmarshalling 失败回退默认值保证旧客户端兼容。

**AC 映射：** AC-1.1（数据载体），AC-1.8（IPC 向后兼容）

**前置依赖：** 无

**非目标：** 不修改 ExecOptions 结构；不调整既有字段顺序；不改 IDL。

**状态所有权：** Compatibility 维度——IPC Parcel 状态格式 `cmd→workDir→env→policy→options→isShellCommand→challenge`，新增字段在尾部，旧客户端不写时回退 `isShellCommand=true`/`challenge=""`。owner=ExecCmdParam::Unmarshalling；清理=无（运行时临时对象）；只读消费者=Service::ExecCmd。验证方法：exec_cmd_param_test 模拟旧 Parcel。

**任务间接口：** Produces=`ExecCmdParam.isShellCommand`(bool,默认true)、`ExecCmdParam.challenge`(string,默认"")、`static ExecCmdParam::ExtractToolName(cmd)`；Consumes=无。供 TASK-3/4/5/6 使用。

**只读上下文**

| 路径 | 读取目的 |
|------|----------|
| exec_cmd_param.h / exec_cmd_param.cpp | 既有结构体与序列化实现 |
| spec.md BR-3/BR-4 | 默认值与字段顺序不变量 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | cli_tool_framework/interfaces/cli_tool/include/exec_cmd_param.h | 新增 isShellCommand/challenge 字段 + ExtractToolName 声明 |
| 修改 | cli_tool_framework/interfaces/cli_tool/src/exec_cmd_param.cpp | Marshalling 追加 WriteBool/WriteString；Unmarshalling 回退默认值；ExtractToolName 实现 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| ICliToolManager.idl | 签名不变 |
| exec_options.h | ExecOptions 结构不变 |

**Steps**

- [ ] Step 1: 写失败测试 exec_cmd_param_test（旧客户端兼容）。

```text
测试 Unmarshalling_OldClientCompat：构造仅写到 options 的 Parcel，Unmarshalling 后断言 isShellCommand==true、challenge==""、不崩溃。
```

- [ ] Step 2: 确认 RED（实现前 Unmarshalling 无尾部默认值回退）。
- [ ] Step 3: 实现——h 新增两字段+ExtractToolName 声明；cpp Marshalling 追加 WriteBool(isShellCommand)/WriteString(challenge)；Unmarshalling 预设默认值后 ReadBool/ReadString 失败即 return result；ExtractToolName 用 find_first_not_of 跳前导空白。
- [ ] Step 4: 确认 GREEN。

```bash
cd cli_tool_framework && run -t UT -ts exec_cmd_param_test
```

Expected: 兼容测试通过。

- [ ] Step 5: 如有必要，在保持 GREEN 的前提下重构。
- [ ] 回填验证状态与代码范围映射。
- [ ] 回填 Actual Result。

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.8 旧 Parcel 回退默认值不崩溃；AC-1.1 字段可承载工具模式数据 |
| Scope respected | 仅改 exec_cmd_param.h/.cpp |
| State lifecycle complete | Compatibility 状态格式不变量已验证 |

**Verification**

| Command / Evidence | Expected Result | Actual Result |
|--------------------|-----------------|---------------|
| run -t UT -ts exec_cmd_param_test | 通过 | 待验证 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.8 覆盖；字段顺序与 BR-4 一致 |
| Code Quality | 尾部追加；ReadBool/ReadString 失败回退；ExtractToolName 跳前导空白 |

### TASK-3: NAPI 解析 isShellCommand/challenge

**目标：** `UnwrapExecCmdOptions` 解析 `isShellCommand`（可选默认 true）与 `challenge`（可选默认 ""）属性，填入 ExecCmdParam。

**AC 映射：** AC-1.1，AC-1.2（默认 true 行为不变）

**前置依赖：** TASK-2（ExecCmdParam 字段）、TASK-1（属性名契约）

**非目标：** 不改 OnExecCmd 调用结构（已有 UnwrapExecCmdOptions 调用）；不解析其他属性。

**状态所有权：** 无（纯参数解析，无新增运行时状态）。

**任务间接口：** Produces=ExecCmdParam 经 NAPI 填充后供客户端使用；Consumes=ExecCmdParam.isShellCommand/challenge（TASK-2）、属性名 isShellCommand/challenge（TASK-1）。

**只读上下文**

| 路径 | 读取目的 |
|------|----------|
| js_cli_manager_utils.cpp | 既有 UnwrapExecCmdOptions 解析 policy/callback 的模式 |
| spec.md BR-3 | 默认 true 不变量 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager_utils.cpp | UnwrapExecCmdOptions 在 policy 后、callback 前新增 isShellCommand/challenge 解析 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| js_cli_manager.cpp | OnExecCmd 调用结构不变 |

**Steps**

- [ ] Step 1: 写失败测试（NAPI 层默认值与传入值）。
- [ ] Step 2: 确认 RED。
- [ ] Step 3: 实现——napi_has_named_property 判断后 UnwrapBoolFromJS2/UnwrapStringFromJS2 填充；未传时保持默认 true/""。
- [ ] Step 4: 确认 GREEN。

```bash
./build.sh --product-name <product> --build-target abilityms
```

Expected: NAPI 模块编译通过。

- [ ] Step 5: 如有必要，在保持 GREEN 的前提下重构。
- [ ] 回填验证状态与代码范围映射。

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.2 未传 isShellCommand 默认 true 行为不变 |
| Scope respected | 仅改 js_cli_manager_utils.cpp |
| State lifecycle complete | N/A |

**Verification**

| Command / Evidence | Expected Result | Actual Result |
|--------------------|-----------------|---------------|
| 编译 abilityms | 通过 | 待验证 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.2 默认值与 spec BR-3 一致 |
| Code Quality | 复用既有 UnwrapBool/UnwrapString 模式 |

### TASK-4: 客户端 cmd 非空校验 + eventKey=toolName

**目标：** `CliToolMgrClient::ExecCmd` 在工具命令模式下校验 cmd 非空（reject 401），eventKey 用 `ExtractToolName`（shell 用 "shell"）。

**AC 映射：** AC-1.1（eventKey），AC-1.5（cmd 空白 reject）

**前置依赖：** TASK-2（ExtractToolName），TASK-3（NAPI 填充的 ExecCmdParam 实例）

**非目标：** 不改 IPC 调用签名；不改回调注册逻辑。

**状态所有权：** 无。

**任务间接口：** Produces=客户端 ExecCmdParam 经校验后 IPC 发往服务端；Consumes=ExecCmdParam::ExtractToolName（TASK-2）、NAPI 填充的 ExecCmdParam 实例（TASK-3）。

**只读上下文**

| 路径 | 读取目的 |
|------|----------|
| cli_tool_mgr_client.cpp | 既有 ExecCmd 方法与 eventKey 逻辑 |
| spec.md AC-1.5 | cmd 空白校验要求 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp | 工具模式 cmd 纯空白校验返回 ERR_INVALID_PARAM；eventKey=ExtractToolName |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| cli_tool_mgr_client.h | 公共客户端接口不变 |

**Steps**

- [ ] Step 1: 写失败测试 cli_tool_mgr_client_test（空白 cmd reject 401）。
- [ ] Step 2: 确认 RED。
- [ ] Step 3: 实现——`!isShellCommand && cmd.find_first_not_of(" \t")==npos` 返回 ERR_INVALID_PARAM；eventKey 三元 `isShellCommand ? "shell" : ExtractToolName(cmd)`。
- [ ] Step 4: 确认 GREEN。

```bash
cd cli_tool_framework && run -t UT -ts cli_tool_mgr_client_test
```

Expected: 空白 cmd 测试通过。

- [ ] Step 5: 如有必要，在保持 GREEN 的前提下重构。
- [ ] 回填验证状态与代码范围映射。

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.5 空白 cmd reject 401 |
| Scope respected | 仅改 cli_tool_mgr_client.cpp |
| State lifecycle complete | N/A |

**Verification**

| Command / Evidence | Expected Result | Actual Result |
|--------------------|-----------------|---------------|
| run -t UT -ts cli_tool_mgr_client_test | 通过 | 待验证 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.5 覆盖 |
| Code Quality | eventKey 与 execTool 一致按工具名分发 |

### TASK-5: ParseToolCommand 分词→子命令→参数→类型恢复

**目标：** `ToolUtil::ParseToolCommand(cmd, toolInfo, param, detail)` 将拼接命令字符串解析为 ExecToolParam，schema 驱动类型恢复，拒绝未知 flag/孤立值。

**AC 映射：** AC-1.6（未知 flag reject），AC-1.7（4 种长标志格式 + 单引号空格值），AC-1.4（未知子命令→ERR_TOOL_NOT_EXIST）

**前置依赖：** TASK-2（ExecCmdParam/ExecToolParam 数据结构）

**非目标：** 不支持短标志/双引号/反斜杠转义/前缀参数/条件标志；不做 required 校验。

**状态所有权：** 无（纯解析，输出临时 ExecToolParam）。

**任务间接口：** Produces=`ToolUtil::ParseToolCommand(cmd, toolInfo, param, detail)→int32_t`（返回 ERR_OK/ERR_INVALID_PARAM/ERR_TOOL_NOT_EXIST，ExecToolParam 由引用参数填充为输出）；Consumes=`cmd` 字符串（来自 ExecCmdParam，TASK-2 间接经 TASK-6 传入）、ToolInfo.inputSchema（既有，来自 CliToolDataManager）。

**只读上下文**

| 路径 | 读取目的 |
|------|----------|
| tool_util.h / tool_util.cpp | 既有 ValidateProperties 等模式 |
| spec.md BR-8, EB-5, EB-6 | 语法约束与边界 |
| design.md 命令解析设计 | 四阶段与类型恢复表 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | cli_tool_framework/services/climgr/include/tool_util.h | 新增 ParseToolCommand 声明 |
| 修改 | cli_tool_framework/services/climgr/src/tool_util.cpp | TokenizeCommand/ResolveSubCommand/ParseArgTokens/ConvertParsedArgs 实现 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| cli_tool_data_manager.h | 数据管理接口不变 |

**Steps**

- [ ] Step 1: 写失败测试 tool_util_test（未知 flag + 4 格式 + 单引号空格）。
- [ ] Step 2: 确认 RED。
- [ ] Step 3: 实现——TokenizeCommand（空格/Tab/单引号）→tokens[0]=toolName→ResolveSubCommand→ParseArgTokens（--key=value/--key value/--flag/--flag false）→ConvertParsedArgs（schema string/boolean/integer/number/array）。
- [ ] Step 4: 确认 GREEN。

```bash
cd cli_tool_framework && run -t UT -ts tool_util_test
```

Expected: 解析格式测试与未知 flag 拒绝测试通过。

- [ ] Step 5: 如有必要，在保持 GREEN 的前提下重构。
- [ ] 回填验证状态与代码范围映射。

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.6 未知 flag reject；AC-1.7 四格式+单引号按 schema 恢复；AC-1.4 未知子命令 |
| Scope respected | 仅改 tool_util.h/.cpp |
| State lifecycle complete | N/A |

**Verification**

| Command / Evidence | Expected Result | Actual Result |
|--------------------|-----------------|---------------|
| run -t UT -ts tool_util_test | 通过 | 待验证 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.6/1.7/1.4 覆盖 |
| Code Quality | schema 驱动类型恢复；未知 flag 拒绝 |

### TASK-6: ExecCmd 分流 + ExecCmdToolMode + SetupCmdSession

**目标：** 服务端 `ExecCmd` 在权限/会话上限校验后分流 `!isShellCommand→ExecCmdToolMode`；ExecCmdToolMode 四步（ExtractToolName+白名单→ParseToolCommand→ValidateAndPrepareTool→SetupCmdSession）；SetupCmdSession 先订阅后创建进程+失败回滚。

**AC 映射：** AC-1.1（正常执行），AC-1.3（白名单拦截），AC-1.4（GetToolByName），AC-1.9（会话上限）

**前置依赖：** TASK-5（ParseToolCommand），TASK-4（客户端 IPC），TASK-2（ExecCmdParam/ExtractToolName）

**非目标：** 不修改 shell 路径；不改 ValidateAndPrepareTool；不改 ProcessManager。

**状态所有权：** Ownership——SessionRecord 由 CliToolManagerService 持有于 sessionRecords_（key=sessionId）；Lifecycle——RUNNING→COMPLETED/FAILED，创建=SetupCmdSession Step1，清理=WaitPid/超时/clearSession，回滚=订阅失败 RemoveSessionRecord/进程失败 UnregisterSubscriber+RemoveSessionRecord/监控失败 Killpg+UnregisterSubscriber+RemoveSessionRecord；Concurrency——sessionsMutex_ 保护 sessionRecords_；不变量=进程已创建必有 subscriber 已注册。验证方法：cli_tool_mgr_service_test 断言失败路径 QuerySession NOT_FOUND。

**任务间接口：** Produces=会话执行结果（CliSessionInfo）；Consumes=ExecCmdParam（TASK-2）、ExtractToolName（TASK-2）、ToolUtil::ParseToolCommand（TASK-5）、ValidateAndPrepareTool/ProcessManager/EventDispatcher（既有）。

**只读上下文**

| 路径 | 读取目的 |
|------|----------|
| cli_tool_manager_service.h / .cpp | 既有 ExecCmd shell 路径与 SetupCmdSession 顺序 |
| design.md 状态归属与不变量 | 回滚顺序不变量 |
| spec.md BR-1/BR-2/BR-5 | 白名单/复用校验/先订阅后创建 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | cli_tool_framework/services/climgr/include/cli_tool_manager_service.h | 新增 ExecCmdToolMode/SetupCmdSession 私有声明 |
| 修改 | cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp | ExecCmd 分流 + ExecCmdToolMode 四步 + SetupCmdSession 先订阅后创建+回滚 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| ICliToolManager.idl | 签名不变 |
| process_manager.* | 进程管理接口不变 |
| tool_util.* | TASK-5 范围 |

**Steps**

- [ ] Step 1: 写失败测试 cli_tool_mgr_service_test（正常执行 + 白名单拦截 + 工具不存在 + 会话上限）。
- [ ] Step 2: 确认 RED。
- [ ] Step 3: 实现——ExecCmd 在 caller 信息获取后 `if(!param.isShellCommand) return ExecCmdToolMode(...)`；ExecCmdToolMode Step1 ExtractToolName+std::all_of 白名单+GetToolByName；Step2 ParseToolCommand；options/challenge 透传；Step3 ValidateAndPrepareTool→sandboxConfig；Step4 SetupCmdSession（CreateRecord→Subscribe→CreateChildProcess→RegisterMonitors，失败按反序回滚）。
- [ ] Step 4: 确认 GREEN。

```bash
cd cli_tool_framework && run -t UT -ts cli_tool_mgr_service_test
```

Expected: 正常/白名单/工具不存在/会话上限测试通过。

- [ ] Step 5: 如有必要，在保持 GREEN 的前提下重构。
- [ ] 回填验证状态与代码范围映射。

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1.1 正常执行；AC-1.3 白名单拦截无进程；AC-1.4 GetToolByName 失败；AC-1.9 会话上限 |
| Scope respected | 仅改 cli_tool_manager_service.h/.cpp |
| State lifecycle complete | SessionRecord 创建/清理/回滚均覆盖；进程已创建必有 subscriber |

**Verification**

| Command / Evidence | Expected Result | Actual Result |
|--------------------|-----------------|---------------|
| run -t UT -ts cli_tool_mgr_service_test | 通过 | 待验证 |
| ./build.sh --product-name <product> --build-target abilityms | 编译通过 | 待验证 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1.1/1.3/1.4/1.9 覆盖；复用 ValidateAndPrepareTool 不绕权 |
| Code Quality | 先订阅后创建；回滚反序；cmd 用 %{private}s |

## Review Gates

| Gate | When | Required Evidence | Blocks Next Step |
|------|------|-------------------|------------------|
| Gate-1 | TASK-1（API 声明）完成后 | @throws 与 spec 错误码一致、属性名一致 | TASK-3 解析对齐 |
| Gate-2 | TASK-6（核心逻辑）完成后 | AC 覆盖、异常路径、兼容性、回归测试 | Gate-Final |
| Gate-Final | 集成/最终验证后 | 端到端证据、AC 到 Task 追溯全 Pass、代码范围映射回填、Actual Result 回填 | 归档 |

## 代码范围映射

| TASK ID | 文件 | 操作 |
|--------|------|------|
| TASK-1 | interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts | 修改（@throws 补 401/35600030） |
| TASK-1 | interface/sdk-js/zh-cn/api/@ohos.app.cli.cliManager.d.ts | 修改（同上中文） |
| TASK-2 | cli_tool_framework/interfaces/cli_tool/include/exec_cmd_param.h | 修改 |
| TASK-2 | cli_tool_framework/interfaces/cli_tool/src/exec_cmd_param.cpp | 修改 |
| TASK-3 | cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager_utils.cpp | 修改 |
| TASK-4 | cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp | 修改 |
| TASK-5 | cli_tool_framework/services/climgr/include/tool_util.h | 修改 |
| TASK-5 | cli_tool_framework/services/climgr/src/tool_util.cpp | 修改 |
| TASK-6 | cli_tool_framework/services/climgr/include/cli_tool_manager_service.h | 修改 |
| TASK-6 | cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp | 修改 |
