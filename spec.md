# 独立 Skill 执行支持实施规格

## 1. 背景与当前现状

本规格用于指导独立 ArkTS script skill 通过 `cliManager.execTool(...)` 接入 CLI framework 的支持落地。目标不是继续补充高层设计说明，而是提供一份开发可直接执行的实施规格，覆盖规则归档、接口约定、文件级改动、阶段任务、影响分析和回归要求。

当前仓内真实现状如下：

- `cliManager.execTool` 已是 skill 调度统一入口，`CliToolManagerService::ExecTool` 中已有 `ValidateSkillTypeFromParam(param, skillType)` 能区分 in-app skill 与 independent skill。
- 当前 independent skill 仍未在 `skillType == SKILL_TYPE_INDEPENDENT` 时准备独立执行参数并拉起 `ohos-arkTSScript` CLI 链路。
- `scriptManager.completeArkTSScript` 当前仍是 not supported stub，会直接抛出 `"completeArkTSScript is not supported yet"`。
- `ohos-arkTSScript` 当前通过 `globalThis.CompletearktsScript` 将结果写入 completion context，再由监控线程统一输出到 stdout。
- `cli_tool_manager` 与 `scriptmanager_napi` 现有模块形态已能支撑当前链路，本规格不再新增 JS 包装与打包任务。

### 1.1 本次对齐更新

本次方案对前文中“`ValidateSkillTypeFromParam(param, skillType)` + `SKILL_TYPE_INDEPENDENT` 常量判断”的实现口径做如下收敛；如与后文旧描述冲突，以本节及第 4.1A、5.1A 节为准：

- `QuerySkillType(...)` 不再只返回 `int32_t skillType`，改为返回完整 `AppExecFwk::SkillInfo`。
- independent skill 的分流判断不再依赖仓内写死常量 `SKILL_TYPE_INDEPENDENT = -1`。
- independent skill 的唯一真相来源改为 `skillInfo.skillType == AppExecFwk::SkillType::INDEPENDENT_SKILL`。
- `CliToolManagerService` 不再只为 independent skill 改写 `abcPath`；还需要把 `execTool` 传入的 `record<string, Object>` 形态参数整理成 `ohos-arkTSScript` 能消费的 CLI 参数。
- independent skill 进入普通 CLI 路径前，必须显式完成两类参数适配：
  - `abcPath = skillInfo.skillPath + "/" + scriptPath`
  - `args` 从 `WantParams/record<string, Object>` 序列化为 JSON object string，再传给 `ohos-arkTSScript --args`
- in-app skill 继续沿用 `ExecuteInAppSkillWithTokenId`，不复用上述 CLI 参数改写逻辑。

独立 skill 不复用 in-app skill 的 `context + requestCode + AMS callback` 完成链路。原因如下：

- in-app skill 的完成接口是 `completeArkTSScriptInApp(context, requestCode, result)`，AMS 依赖 `requestCode` 查找执行记录并回调调用方。
- 独立 skill 的完成接口是 `completeArkTSScript(result)`，接口没有 `context` 和 `requestCode`，不具备复用 in-app 回调链路的必要标识。
- 当前 CLI framework 已具备 stdout/stderr 会话结果收集能力，`ohos-arkTSScript` 已具备基于 completion context 输出结果的执行模型。

因此，独立 skill 的结果传输统一以 CLI stdout 为准：

```text
cliManager.execTool("ohos-arkTSScript", subcommand, args, challenge, options?)
  -> NAPI OnExecTool
  -> CliToolMGRClient::ExecTool
  -> CliToolManagerService::ExecTool
  -> ValidateSkillTypeFromParam(param, skillType)
  -> skillType == SKILL_TYPE_INDEPENDENT
  -> prepare independent skill CLI args
  -> ohos-arkTSScript loads and runs ABC
  -> ArkTS calls scriptManager.completeArkTSScript(result)
  -> script_manager NAPI records result in ohos-arkTSScript completion context
  -> ohos-arkTSScript prints normalized ExecuteResult JSON to stdout
  -> CLI SA collects stdout into CliSessionInfo.result.outputText
  -> caller observes session result through CLI session APIs
```

## 2. 统一规则

本章用于集中整理独立 skill 支持开发过程中的所有规则，作为唯一查找入口。开发时不得再把规则分散定义到各阶段说明中。

### 2.1 职责边界

- `cliManager.execTool` 负责统一 JS 入口参数解析，并通过 `CliToolMGRClient::ExecTool` 进入 CLI SA。
- `CliToolManagerService::ExecTool` 负责 skill 类型分流；对 independent skill 复用 `ValidateSkillTypeFromParam` 的查询结果，准备独立执行参数后走普通 CLI session 调度。
- `scriptManager.completeArkTSScript` 负责独立 skill 完成接口、严格字段校验，以及把结果写入独立工具运行环境中的 completion helper。
- `ohos-arkTSScript` 负责 runtime 生命周期、completion context、完成信号、stdout 输出时机，以及脚本执行收口。
- 共享 completion helper 只负责 register / clear / complete 三类进程内协作，不负责业务字段校验和 stdout 输出。
- in-app skill 继续沿用 `completeArkTSScriptInApp` + AMS 链路，不复用独立 skill 的 helper 或 stdout 协议。

### 2.2 校验边界

- `cliManager.execTool` 只做统一 CLI 入口校验：
  - `toolName` 必填且为非空 string
  - `subcommand` 必须为 string
  - `args` 必须为 `WantParams`
  - `challenge` 必填且为非空 string
- independent skill 额外要求：
  - `bundleName/moduleName/abcPath/functionName` 必填
  - `skillName` 在 independent skill 场景允许为空
- `CliToolManagerService` 只在 independent skill 分支解析并改写 `abcPath`，不对 `scriptPath/args` 做重复 schema 校验。
- CLI tool 入参规则以 `tools/ohos-arkTSScript/config.json` 中的 `inputSchema` 为唯一真相。
- `scriptManager.completeArkTSScript` 对独立 skill 的结果对象做严格字段校验：
  - 参数个数必须为 1
  - 参数必须为 object
  - `code` 必填且为 number
  - `result` 可选且为 object
  - `uris` 不涉及
  - `flags` 不涉及
- `ohos-arkTSScript` 不对上层业务结果再做重复字段校验，只负责接收已规范化结果并推进 completion context。
- 不修改通用 `UnwrapSkillExecuteResult` 的宽松语义，避免波及 in-app skill 与其他复用方。

### 2.3 结果字段规范

- 独立 skill 的标准输出字段统一为 `code/result/uris/flags`。
- `code` 为必填 number。
- `result` 为可选 object。
- `uris` 为可选 string array。
- `flags` 为可选 number。
- 旧的 `uri/flag` 单值字段不再作为独立 skill 正式协议保留。

### 2.4 兼容与迁移规则

- `globalThis.CompletearktsScript` 本次完全替换，不做兼容保留，也不提供 deprecated 过渡期。
- `ohos-arkTSScript` 侧脚本调用统一迁移为 `scriptManager.completeArkTSScript(result)`。
- `config.json`、`README.md`、`test.js`、测试断言必须同步从 `uri/flag` 迁移为 `uris/flags`。

### 2.5 明确不做

- 不新增 `StandaloneSkillToolParam`。
- 不新增 AMS requestCode 记录链路。
- 不让独立 skill 依赖 `context` 或 `requestCode`。
- 不从 BMS `SkillInfo` 推导 `functionName/scriptPath/args`。
- independent skill 允许通过 BMS `bundleName/moduleName` 查询模块信息，只用于确认模块存在并按 runtime 约定拼接系统内完整 ABC 路径。
- 不支持 `ohos-insightIntent` 独立执行；本次仅覆盖 `ohos-arkTSScript`。
- 不修改 in-app skill 既有 `completeArkTSScriptInApp` 行为。

## 3. 接口与行为约定

### 3.1 `cliManager.execTool`

independent skill 与 in-app skill 统一从 `cliManager.execTool` 进入，不新增其他独立分流入口。

```ts
declare namespace cliManager {
  interface ExecOptions {
    background?: boolean;
    timeout?: number;
    yieldMs?: number;
  }

  interface ExecToolArgs {
    [key: string]: Object;
  }

  function execTool(
    toolName: string,
    subcommand: string,
    args: ExecToolArgs,
    challenge: string,
    options?: ExecOptions
  ): Promise<CliSessionInfo>;
}
```

独立 ArkTS script skill 的动态字段定义如下：

| 字段 | 必选 | 类型 | 说明 |
| --- | --- | --- | --- |
| `bundleName` | 是 | `string` | 目标 bundle 名，用于 `ValidateSkillTypeFromParam` 与 BMS 模块查询。 |
| `moduleName` | 是 | `string` | 目标模块名，用于 `ValidateSkillTypeFromParam` 与 ABC 路径拼接。 |
| `skillName` | 否 | `string` | independent skill 场景固定允许为空；in-app skill 场景仍按现有逻辑要求有效。 |
| `skillToolType` | 否 | `number` | 当前服务侧不使用该字段判断 skill 类型；如上层仍透传该路由元数据，会在进入普通 CLI schema 校验前被过滤，不作为 skillType 真相来源。 |
| `abcPath` | 是 | `string` | ABC 文件路径片段。服务侧会按 runtime 约定拼成系统内完整路径后传给 `ohos-arkTSScript --abcPath`。 |
| `functionName` | 是 | `string` | 目标函数名，对应 `ohos-arkTSScript --functionName`。 |
| `scriptPath` | 否 | `string` | 多文件 ABC 中的脚本文件或类名，对应 `ohos-arkTSScript --scriptPath`。 |
| `args` | 否 | `string` | JSON object 字符串，对应 `ohos-arkTSScript --args`。 |

行为约定如下：

- `toolName == "ohos-arkTSScript"` 时，`CliToolManagerService::ExecTool` 调用一次 `ValidateSkillTypeFromParam(param, skillType)`。
- `ValidateSkillTypeFromParam` 是唯一 skill 类型来源；`skillType` 只能来自包管理 `QuerySkillType` 查询结果，不允许在 service 侧根据 `skillToolType` 或其他本地字段自行推断。
- `skillType == SKILL_TYPE_INDEPENDENT` 时进入 independent skill CLI 参数准备分支。
- `skillRet == ERR_OK && skillType != SKILL_TYPE_INDEPENDENT` 时走现有 in-app skill 分支。
- independent skill 分支必须满足：
  - `bundleName/moduleName/abcPath/functionName` 非空
  - 能通过 BMS 查到 `bundleName` 下的目标 `moduleName`
  - 服务侧按 runtime `RunScript` 路径形态生成 `/data/storage/el1/bundle/<moduleName>/<abcPath>`
  - 写回完整 `abcPath` 后复用普通 CLI 校验、沙箱、子进程和 session 事件链路
- 以下场景必须 reject 或返回错误：
  - `ValidateSkillTypeFromParam` 查询失败
  - independent skill 缺少必要字段
  - BMS 查不到 bundle/module
  - CLI tool 启动失败

### 3.2 `scriptManager.completeArkTSScript`

`completeArkTSScriptInApp` 保持现有 in-app 行为不变。`completeArkTSScript` 用于独立 skill，调用方只传执行结果，不传 `context` 或 `requestCode`。

```ts
declare namespace scriptManager {
  interface ExecuteResult {
    code: number;
    result?: Record<string, Object>;
    uris?: Array<string>;
    flags?: number;
  }

  function completeArkTSScriptInApp(
    context: Context,
    requestCode: string,
    result: ExecuteResult
  ): Promise<void>;

  function completeArkTSScript(result: ExecuteResult): Promise<void>;
}
```

行为约定如下：

- `completeArkTSScriptInApp` 保持现有 `AbilityManagerClient::ExecuteSkillDone` 链路不变。
- `completeArkTSScript` 只服务独立 skill，不要求 Ability context，不调用 `AbilityManagerClient::ExecuteSkillDone`。
- `completeArkTSScript` 必须执行严格字段校验：
  - 参数个数为 1
  - 参数为 object
  - `code` 为必填 number
  - `result` 如存在必须为 object
  - `uris` 如存在必须为 string array
  - `flags` 如存在必须为 number
- 校验通过后调用共享 completion helper，将结果写入当前工具进程的 `ExecutionContext`。
- 当前进程未注册 standalone callback 时 reject，表示当前环境不是独立 ArkTS script 工具运行环境。

### 3.3 `ohos-arkTSScript` stdout 协议

独立 skill 的 stdout 协议由 `ohos-arkTSScript` 输出，统一格式如下：

```json
{
  "code": 0,
  "result": {},
  "uris": ["file://demo"],
  "flags": 1
}
```

行为约定如下：

- stdout 只在 `resultReady == true` 且 `scriptDone == true` 时输出。
- `scriptManager.completeArkTSScript(result)` 只负责写入 completion context，不直接输出 stdout。
- stdout 的字段规范必须与 independent skill 执行结果协议一致。
- 错误场景继续通过 stderr / error json 输出，由 CLI session 收集。

## 4. 实施方案

### 4.1 `cliManager.execTool` 独立 skill 分流

在 `CliToolManagerService::ExecTool` 中复用现有 `ValidateSkillTypeFromParam(param, skillType)` 查询结果处理 independent skill 分支，不新增 standalone executor。

方案要求：

- `toolName != "ohos-arkTSScript"` 时保持普通 CLI 调度不变。
- `toolName == "ohos-arkTSScript"` 时：
  - 调用一次 `ValidateSkillTypeFromParam(param, skillType)`
  - `skillRet != ERR_OK` 时直接返回错误
  - `skillType != SKILL_TYPE_INDEPENDENT` 时继续走 `SetupAndStartSkillSession`
  - `skillType == SKILL_TYPE_INDEPENDENT` 时进入 independent skill CLI 参数准备分支
- `ValidateSkillTypeFromParam` 作为唯一分流来源；`ExecTool` 不新增第二套 `skillToolType` 判断，也不在 service 侧本地推断 `SKILL_TYPE_INDEPENDENT`。
- independent skill 不复用 in-app `SkillCallbackAdapter` / `ExecuteInAppSkillWithTokenId`。
- independent skill 通过 `ExecToolParam` 副本进入普通 CLI 路径：
  - `toolName = "ohos-arkTSScript"` 保持不变
  - `subcommand/challenge/options` 沿用原始入参
  - `args` 过滤掉 `bundleName/moduleName/skillName/skillToolType`
  - `abcPath` 写入按 `bundleName/moduleName/abcPath` 解析出的完整系统路径
  - `functionName/scriptPath/args` 继续透传给 `ohos-arkTSScript`
- independent skill 的参数准备失败时必须在进入 `ValidateAndPrepareTool` 之前返回错误。
- independent skill 成功准备参数后复用 `ValidateExecToolPermissions`、`ValidateSessionLimit`、`ValidateAndPrepareTool`、`SetupAndStartSession`。

ABC 路径解析规则：

- 通过 BMS `GetBundleInfoV9(bundleName, GET_BUNDLE_INFO_WITH_HAP_MODULE, ...)` 查询目标 bundle。
- 在 `hapModuleInfos` 中确认 `moduleName` 对应模块存在。
- 输出路径形态参考 `js_runtime::RunScript`，统一为 `/data/storage/el1/bundle/<moduleName>/<abcPath>`。
- 服务侧不负责校验 `abcPath` 的相对/绝对路径合法性，只按 runtime 规则完成拼接；最终路径是否可用由后续实际加载路径的一方判断。
- 路径解析只服务 independent skill；普通 CLI 与 in-app skill 不受影响。

### 4.1A `SkillInfo` 查询与 CLI 参数改写对齐方案

为支持 independent skill 的真实枚举判断与 `skillPath` 读取，`execTool` 分流方案细化如下：

- `CliToolManagerService::ExecTool` 对 `toolName == "ohos-arkTSScript"` 的请求只查询一次 `SkillInfo`。
- `SkillInfo` 既用于判断 skill 类型，也用于提供 independent skill 所需的 `skillPath`。
- `ExecTool` 不再在 service 侧维护 `SKILL_TYPE_INDEPENDENT` 本地常量，也不再根据 `skillToolType` 或其他本地字段推断 independent skill。
- `js_skill_driver` 与 `climgr` 统一改用真实枚举值：
  - `skillInfo.skillType == AppExecFwk::SkillType::APP_SKILL`
  - `skillInfo.skillType == AppExecFwk::SkillType::INDEPENDENT_SKILL`

independent skill 的 service 侧处理顺序固定如下：

1. `ValidateExecToolPermissions()`
2. 若 `toolName != "ohos-arkTSScript"`，保持现有普通 CLI 路径不变
3. 若 `toolName == "ohos-arkTSScript"`，调用一次 `ValidateSkillInfoFromParam(param, skillInfo)`
4. 若 `skillInfo.skillType != AppExecFwk::SkillType::INDEPENDENT_SKILL`，继续走现有 `SetupAndStartSkillSession(...)`
5. 若 `skillInfo.skillType == AppExecFwk::SkillType::INDEPENDENT_SKILL`，构造 CLI 用 `ExecToolParam` 副本
6. 调用 `PrepareIndependentSkillCliParam(param, skillInfo, cliParam)` 完成参数改写
7. 对 `cliParam` 继续复用现有 `ValidateSessionLimit()`、`ValidateAndPrepareTool()`、`SetupAndStartSession()`

independent skill 的 CLI 参数改写规则固定如下：

- 输入来源仍为 `cliManager.execTool(..., args: record<string, Object>, ...)`
- service 侧先执行 `NormalizeSkillParamKeys(...)`
- 保留字段读取来源：
  - `bundleName`
  - `moduleName`
  - `skillName`
  - `scriptPath`
  - `functionName`
- `scriptPath` 是 independent skill 必需输入，用于生成最终 `abcPath`
- `abcPath` 最终不再使用原始入参中的路径片段值，而是改写为：
  - `skillInfo.skillPath + "/" + scriptPath`
- `functionName` 原样写入 CLI 参数
- `args` 必须转换成 JSON object string；`TransferToCmdParam(...)` 不负责该转换
- 最终进入普通 CLI schema 与子进程创建链路的 `cliParam.args` 只保留 `ohos-arkTSScript` 实际需要的字段：
  - `abcPath`
  - `functionName`
  - `args`
- `bundleName/moduleName/skillName/scriptPath/skillToolType` 仅用于 skill 分流与参数准备，不继续透传为最终 CLI 参数，除非后续 `ohos-arkTSScript` 协议显式要求。

`args` 的 JSON string 收口规则如下：

- 上层传入的业务参数最终统一收敛为 `--args <json-object-string>`
- 若原始 `WantParams` 中存在保留字段以外的顶层字段，这些字段应并入最终 `args` JSON object
- 若原始 `args["args"]` 本身是 object / `WantParams`，则递归序列化为 JSON object
- 若原始 `args["args"]` 本身是 string，且内容是合法 JSON object string，可直接沿用
- 若原始 `args["args"]` 既不是 object 也不是合法 JSON object string，应在 `PrepareIndependentSkillCliParam(...)` 阶段返回错误
- service 侧必须保证传递给 `ohos-arkTSScript --args` 的最终值始终是一个 JSON object string，而不是 `WantParams`、数组或其他 IPC 包装对象

该节落地后，以下旧口径作废：

- “`ValidateSkillTypeFromParam` 只返回 `int32_t skillType`”
- “independent skill 的唯一区别只是额外拼接 `abcPath`”
- “仓内用 `-1` 作为 independent skill 判断值”

### 4.2 `scriptManager` 独立完成接口

`scriptManager.completeArkTSScript` 从 stub 改为真实实现，单独承担独立 skill 结果入栈职责。

方案要求：

- `completeArkTSScriptInApp` 保持原样。
- `completeArkTSScript` 不复用当前宽松的 `UnwrapSkillExecuteResult`。
- 独立实现严格解析逻辑，确保独立 skill 结果协议可控。
- 校验通过后调用共享 helper 完成结果上报。
- reject 信息必须能区分：
  - 参数个数错误
  - 参数类型错误
  - `code/result/uris/flags` 字段类型错误
  - 当前进程未注册 standalone callback

### 4.3 共享 completion helper

新增进程内 completion helper，供 `scriptmanager_napi` 和 `ohos-arkTSScript` 共用。

方案要求：

- helper 不能做成两边各自静态编译的 source_set。
- helper 必须做成单独共享库，确保 `scriptmanager_napi` 与 `arkts_script` 在同一进程内共享同一份回调状态。
- helper 仅提供以下能力：
  - `RegisterStandaloneScriptCompleteCallback(callback)`
  - `ClearStandaloneScriptCompleteCallback()`
  - `CompleteStandaloneScript(result)`
- helper 只负责 register / clear / complete，不负责字段校验、stdout 输出和脚本生命周期管理。

### 4.4 `ohos-arkTSScript` 迁移

`ohos-arkTSScript` 从全局 `CompletearktsScript` 迁移到 `scriptManager.completeArkTSScript`。

方案要求：

- 删除全局 `CompletearktsScript` 绑定。
- 脚本侧通过 `scriptManager` 模块上报完成结果，开发者脚本必须通过模块 `import` 获取 `scriptManager`。
- `ohos-arkTSScript` 不新增 `globalThis.scriptManager`，也不主动加载 `scriptManager` 模块；模块解析与加载仍遵循脚本自身 `import` 语义。
- runtime 启动后注册 standalone completion callback，runtime 结束或失败时清理。
- completion context 的退出条件保持不变，仍为 `resultReady && scriptDone`。
- stdout 字段统一从 `code/result/uri/flag` 升级为 `code/result/uris/flags`。

## 5. 文件改动清单

### 5.1A 具体函数签名变化

以下函数签名变化为本次 independent skill 支持的实施基线；如实现中需要改名，以语义更清晰为优先，但不应改变本节规定的输入输出职责。

#### 5.1A.1 AbilityManager 查询接口

- `interfaces/inner_api/ability_manager/include/ability_manager_interface.h`
  - 旧：
    - `virtual int32_t QuerySkillType(const std::string &bundleName, const std::string &moduleName, const std::string &skillName, int32_t &skillType)`
  - 新：
    - `virtual int32_t QuerySkillType(const std::string &bundleName, const std::string &moduleName, const std::string &skillName, AppExecFwk::SkillInfo &skillInfo)`
- `interfaces/inner_api/ability_manager/include/ability_manager_client.h`
  - 旧：
    - `ErrCode QuerySkillType(const std::string &bundleName, const std::string &moduleName, const std::string &skillName, int32_t &skillType);`
  - 新：
    - `ErrCode QuerySkillType(const std::string &bundleName, const std::string &moduleName, const std::string &skillName, AppExecFwk::SkillInfo &skillInfo);`
- `services/abilitymgr/src/ability_manager_client.cpp`
  - 同步上述签名变化，直接透传 `SkillInfo`
- `services/abilitymgr/src/ability_manager_proxy.cpp`
  - 同步上述签名变化
  - IPC 返回值从 `reply.ReadInt32()` 改为读取 `SkillInfo` 对象
- `services/abilitymgr/src/ability_manager_stub.cpp`
  - 同步上述签名变化
  - IPC 写回值从 `reply.WriteInt32(skillType)` 改为写回 `SkillInfo`
- `services/abilitymgr/src/ability_manager_service.cpp`
  - 旧：
    - `int32_t AbilityManagerService::QuerySkillType(..., int32_t &skillType)`
  - 新：
    - `int32_t AbilityManagerService::QuerySkillType(..., AppExecFwk::SkillInfo &skillInfo)`
  - 服务侧继续复用 `SkillExecuteManager::QuerySkillInfo(...)`，但不再丢弃 `skillPath`、`skillType` 等字段

说明：

- 若后续决定将接口统一重命名为 `QuerySkillInfo(...)`，允许在接口、client、proxy、stub、service 五处同步重命名；但若本次仅改签名不改名，也满足本规格。
- `SkillInfo` 必须具备 IPC 可传输能力；若上游头文件尚未提供 `Parcelable/Marshalling/Unmarshalling`，需先补齐该能力后再切换接口签名。

#### 5.1A.2 climgr service 内部接口

- `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h`
  - 旧：
    - `int32_t ValidateSkillTypeFromParam(const ExecToolParam &param, int32_t &skillType);`
    - `int32_t ValidateSkillType(const std::string &bundleName, const std::string &moduleName, const std::string &skillName, int32_t &skillType);`
  - 新建议：
    - `int32_t ValidateSkillInfoFromParam(const ExecToolParam &param, AppExecFwk::SkillInfo &skillInfo);`
    - `int32_t QuerySkillInfo(const std::string &bundleName, const std::string &moduleName, const std::string &skillName, AppExecFwk::SkillInfo &skillInfo);`
    - `int32_t PrepareIndependentSkillCliParam(const ExecToolParam &param, const AppExecFwk::SkillInfo &skillInfo, ExecToolParam &cliParam);`
- `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp`
  - `TryDispatchSkillSession(...)` 需要按新的 `SkillInfo` 口径重写内部逻辑
  - 若保留原签名：
    - `int32_t TryDispatchSkillSession(const ExecToolParam &param, const std::string &eventId, const ToolInfo &toolInfo, bool &dispatched)`
  - 建议扩展为可返回 CLI 副本：
    - `int32_t TryDispatchSkillSession(const ExecToolParam &param, const std::string &eventId, const ToolInfo &toolInfo, bool &dispatched, std::optional<ExecToolParam> &cliParamOverride)`
  - 若不希望引入 `std::optional`，可改为：
    - `int32_t TryDispatchSkillSession(const ExecToolParam &param, const std::string &eventId, const ToolInfo &toolInfo, bool &dispatched, bool &useCliOverride, ExecToolParam &cliParamOverride)`

说明：

- `PrepareIndependentSkillCliParam(...)` 的职责是“生成最终进入普通 CLI 路径的 `ExecToolParam` 副本”，不负责权限、并发、sandbox、session 处理。
- `ValidateSkillInfoFromParam(...)` 是 `ohos-arkTSScript` skill 分流的唯一入口，不允许并行保留第二套基于 `skillToolType` 的判断逻辑。

#### 5.1A.3 ToolUtil 辅助接口

- `cli_tool_framework/services/climgr/include/tool_util.h`
  - 新增建议：
    - `static bool SerializeWantParamsToJsonString(const AAFwk::WantParams &args, const std::set<std::string> &excludedKeys, std::string &jsonString);`
    - `static bool BuildIndependentSkillArgsJson(const AAFwk::WantParams &args, std::string &jsonString);`
    - `static void BuildIndependentSkillCliArgs(const AAFwk::WantParams &srcArgs, const std::string &abcPath, const std::string &functionName, AAFwk::WantParams &dstArgs);`
- `cli_tool_framework/services/climgr/src/tool_util.cpp`
  - 负责把 `WantParams/record<string, Object>` 转成 `args` JSON string
  - 现有 `TransferToCmdParam(...)`、`GetParamStringValue(...)` 保持“只消费已规范化参数”的职责，不新增对象到 JSON 的隐式序列化逻辑

说明：

- 由于 `TransferToCmdParam(...)` 仅支持基础类型和数组，independent skill 的对象参数序列化必须在该函数调用前完成。
- `BuildIndependentSkillArgsJson(...)` 必须排除保留字段：
  - `bundleName`
  - `moduleName`
  - `skillName`
  - `scriptPath`
  - `functionName`
  - `skillToolType`

#### 5.1A.4 js skill driver

- `cli_tool_framework/frameworks/js/napi/skill_driver/src/js_skill_driver.cpp`
  - 旧：
    - `DispatchExecuteSkill(..., int32_t skillType)` 内部依赖 `SKILL_TYPE_INDEPENDENT = -1`
  - 新：
    - `DispatchExecuteSkill(...)` 查询 `AppExecFwk::SkillInfo`
    - 使用 `skillInfo.skillType == AppExecFwk::SkillType::INDEPENDENT_SKILL` 判断
  - 若当前阶段仍不支持 JS driver 直接执行 independent skill，可继续返回 “not supported yet”，但不得再依赖本地假值判断

#### 5.1A.5 `ExecTool` 主流程中的参数使用方式

`CliToolManagerService::ExecTool(...)` 中的参数使用方式按下面的规则收口：

- 原始请求参数 `param` 保持不变，不直接在其上修改 `abcPath`、`args` 或路由字段。
- 当请求被判定为 independent skill 时，先基于 `param` 生成一个改写后的 `ExecToolParam` 副本，例如 `cliParam`。
- `cliParam` 只用于后续普通 CLI 公共链路，内部应写入最终要传给 `ohos-arkTSScript` 的参数：
  - 已改写后的 `abcPath`
  - 已转成 JSON string 的 `args`
  - 保留的 `functionName`
- independent skill 完成副本构造后，后续普通 CLI 公共流程统一使用 `cliParam`，不再回头使用原始 `param`。
- 非 independent skill 场景继续沿用原始 `param`，不新增任何参数副本逻辑。

随后普通 CLI 公共流程的调用约束如下：

- 普通 CLI：继续使用原始 `param` 调用
  - `ValidateAndPrepareTool(param, ...)`
  - `SetupAndStartSession(param, ...)`
- independent skill：改为使用参数副本 `cliParam` 调用
  - `ValidateAndPrepareTool(cliParam, ...)`
  - `SetupAndStartSession(cliParam, ...)`

这样可以保证：

- 普通 CLI 完全不变
- in-app skill 完全不变
- independent skill 的特殊参数改写只发生一次，且只影响进入普通 CLI 路径前的参数副本

### 5.1 CLI manager / climgr service

| 类型 | 文件 | 目的 | 是否影响现有流程 | 备注 |
| --- | --- | --- | --- | --- |
| 修改 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 复用 `ValidateSkillTypeFromParam` 分流 independent skill，并将其调度到普通 CLI session | 是，影响 `ohos-arkTSScript` skill tool 路径 | 普通 CLI 与 in-app skill 路径保持不变 |
| 修改 | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 声明 independent skill 参数准备与 ABC 路径解析 helper | 是 | 内部接口 |
| 修改 | `cli_tool_framework/services/climgr/src/tool_util.cpp` | 增加 skill 路由字段过滤与参数副本辅助能力 | 是 | 避免路由字段进入 tool schema |
| 修改 | `cli_tool_framework/services/climgr/include/tool_util.h` | 声明对应工具方法 | 是 | 与 cpp 同步 |
| 修改 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | 如有需要，仅补齐 `execTool` 入参错误信息；不新增独立 skill 分流 | 否 | 分流在 service 侧完成 |

### 5.2 `scriptManager`

| 类型 | 文件 | 目的 | 是否影响现有流程 | 备注 |
| --- | --- | --- | --- | --- |
| 修改 | `cli_tool_framework/frameworks/js/napi/script_manager/src/js_script_manager.cpp` | 实现 `completeArkTSScript` 严格解析与 helper 调用 | 是，影响独立 skill 完成接口 | `completeArkTSScriptInApp` 保持不变 |
| 修改 | `cli_tool_framework/frameworks/js/napi/script_manager/BUILD.gn` | 增加 helper 依赖 | 是 | 不新增 JS 包装与打包任务 |

### 5.3 共享 completion helper

| 类型 | 文件 | 目的 | 是否影响现有流程 | 备注 |
| --- | --- | --- | --- | --- |
| 新增 | `cli_tool_framework/frameworks/common/standalone_script_completion/BUILD.gn` | 提供单独共享库构建入口 | 是 | 不能降级为 source_set |
| 新增 | `cli_tool_framework/frameworks/common/standalone_script_completion/include/standalone_script_completion.h` | 暴露 register / clear / complete 接口 | 否 | 供 NAPI 与 tool 共用 |
| 新增 | `cli_tool_framework/frameworks/common/standalone_script_completion/src/standalone_script_completion.cpp` | 实现进程内共享回调状态 | 否 | 同一进程只能有一份状态 |

### 5.4 `ohos-arkTSScript`

| 类型 | 文件 | 目的 | 是否影响现有流程 | 备注 |
| --- | --- | --- | --- | --- |
| 修改 | `tools/ohos-arkTSScript/src/arkts_script.cpp` | 注册/清理 helper callback，保持 completion context 收口 | 是 | 影响独立脚本主流程 |
| 修改 | `tools/ohos-arkTSScript/include/arkts_script.h` | 声明 helper 相关能力与结构调整 | 是 | 需与 cpp 同步 |
| 修改 | `tools/ohos-arkTSScript/src/js_arkts_script.cpp` | 删除 `CompletearktsScript` 绑定，迁移脚本调用方式 | 是 | breaking change |
| 修改 | `tools/ohos-arkTSScript/include/js_arkts_script.h` | 删除全局完成接口声明 | 是 | 需同步测试 |
| 修改 | `tools/ohos-arkTSScript/config.json` | 统一 input/output schema，输出改为 `uris/flags` | 是 | schema 为唯一真相 |
| 修改 | `tools/ohos-arkTSScript/README.md` | 更新开发者调用示例与字段说明 | 是 | 与实现保持一致 |
| 修改 | `tools/ohos-arkTSScript/test.js` | 迁移测试脚本，替换 `CompletearktsScript` 用法 | 是 | 所有场景同步改造 |
| 修改 | `tools/ohos-arkTSScript/BUILD.gn` | 增加 helper 依赖 | 是 | 需保证共享库链接正确 |

### 5.5 测试

| 类型 | 文件 | 目的 | 是否影响现有流程 | 备注 |
| --- | --- | --- | --- | --- |
| 修改 | `test/unittest/arkts_script_test/arkts_script_test.cpp` | 更新 `ohos-arkTSScript` 行为断言 | 是 | 从 `uri/flag` 迁移到 `uris/flags` |
| 修改 | `test/unittest/arkts_script_test/BUILD.gn` | 增加 helper 或新依赖 | 是 | 与主构建保持一致 |
| 修改 | `test/unittest/cli_tool_mgr/BUILD.gn` | 纳入新单测目标 | 是 | 新增 group 依赖 |
| 新增 | `test/unittest/cli_tool_mgr/standalone_script_completion_test/*` | 验证 helper 生命周期 | 否 | 新单元测试 |
| 新增 | `test/unittest/cli_tool_mgr/standalone_skill_exec_tool_test/*` | 验证 `cliManager.execTool` / `CliToolManagerService::ExecTool` 独立 skill 分流、路径解析与 CLI 调度 | 否 | 新单元测试 |

## 6. 分阶段开发任务

### 6.1 Phase 1：`execTool` 独立 skill 分流与 CLI 调度

目标：

- 打通 `cliManager.execTool` 到 independent skill CLI session 的调度入口。
- 复用 `ValidateSkillTypeFromParam(param, skillType)`，在 `skillType == SKILL_TYPE_INDEPENDENT` 时进入 independent skill 分支。
- 完成 `bundleName/moduleName/abcPath` 到系统内完整 ABC 路径的解析，并拉起 `ohos-arkTSScript`。

必须修改的文件：

- `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp`
- `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h`
- `cli_tool_framework/services/climgr/src/tool_util.cpp`
- `cli_tool_framework/services/climgr/include/tool_util.h`

必须新增的文件：

- 无。

关键实现点：

- `CliToolManagerService::ExecTool` 对 `ohos-arkTSScript` 调用一次 `ValidateSkillTypeFromParam(param, skillType)`。
- `skillType != SKILL_TYPE_INDEPENDENT` 时继续走现有 in-app `SetupAndStartSkillSession`。
- independent skill 的分流只依赖 `ValidateSkillTypeFromParam` 返回的包管理查询结果；`ExecTool` 不重复解析 `skillToolType`，也不自行推断 independent。
- `skillType == SKILL_TYPE_INDEPENDENT` 时构造 CLI 执行参数副本：
  - `skillName` 允许为空
  - 校验 `bundleName/moduleName/abcPath/functionName`
  - 通过 BMS 确认目标 bundle/module 存在
  - 将 `abcPath` 改写为 `/data/storage/el1/bundle/<moduleName>/<abcPath>`
  - 过滤 `bundleName/moduleName/skillName/skillToolType`
- independent skill 参数准备完成后复用普通 CLI 的权限、并发、schema、沙箱、子进程和 session 事件链路。

影响的现有流程：

- 新增 independent skill 在 `execTool` 中的调度路径。
- in-app `ExecuteInAppSkillWithTokenId` 路径不变。
- 普通 CLI tool 路径不变。

回归要求：

- in-app skill 原成功/失败回调不回归。
- 普通 `ohos-arkTSScript` CLI 调用不回归。
- independent skill 缺少必要字段、BMS 查不到 bundle/module、CLI 启动失败时能返回可定位错误。

完成判定：

- `ValidateSkillTypeFromParam` 返回 `SKILL_TYPE_INDEPENDENT` 时，`ExecTool` 能进入 independent CLI 路径。
- 输入合法 independent skill 参数时，可生成完整 `abcPath` 并发起 CLI 调度。
- independent skill 中 `skillName` 为空时，是否能完成分流取决于包管理 `QuerySkillType` 对该输入的返回结果；service 侧不再做本地兼容推断。
- in-app skill 仍只进入 `ExecuteInAppSkillWithTokenId`。

本阶段不做：

- 不改 `scriptManager.completeArkTSScript`。
- 不改 `ohos-arkTSScript` 脚本调用方式。
- 不做 stdout 协议解析与 JS 结果对象收敛。

### 6.2 Phase 2：`scriptManager` 独立完成接口、helper 与 `ohos-arkTSScript` 迁移

目标：

- 实现 `scriptManager.completeArkTSScript`。
- 引入共享 completion helper，打通进程内结果回传。
- 完成 `ohos-arkTSScript` 从全局完成接口到 `scriptManager` 的迁移，并用真实工具链路验证打通情况。

必须修改的文件：

- `cli_tool_framework/frameworks/js/napi/script_manager/src/js_script_manager.cpp`
- `cli_tool_framework/frameworks/js/napi/script_manager/BUILD.gn`
- `tools/ohos-arkTSScript/src/arkts_script.cpp`
- `tools/ohos-arkTSScript/include/arkts_script.h`
- `tools/ohos-arkTSScript/src/js_arkts_script.cpp`
- `tools/ohos-arkTSScript/include/js_arkts_script.h`
- `tools/ohos-arkTSScript/BUILD.gn`

必须新增的文件：

- `cli_tool_framework/frameworks/common/standalone_script_completion/BUILD.gn`
- `cli_tool_framework/frameworks/common/standalone_script_completion/include/standalone_script_completion.h`
- `cli_tool_framework/frameworks/common/standalone_script_completion/src/standalone_script_completion.cpp`

关键实现点：

- `completeArkTSScriptInApp` 原样保留。
- `completeArkTSScript` 执行严格字段校验。
- helper 必须为单独共享库，不得静态复制状态。
- 删除 `globalThis.CompletearktsScript` 绑定。
- `ohos-arkTSScript` runtime 启动时注册 helper callback，退出时清理。
- 用 `ohos-arkTSScript -> scriptManager.completeArkTSScript -> helper -> completion context` 真实链路验证 `completeArkTSScript` 已打通，而不是只停留在 NAPI 单侧实现。

影响的现有流程：

- 新增独立 skill 完成接口。
- in-app AMS 回调链路不变。
- `ohos-arkTSScript` 脚本完成结果的上报方式发生 breaking change。

回归要求：

- 非独立工具环境调用 `completeArkTSScript` 时必须拒绝。
- in-app `completeArkTSScriptInApp` 无行为变化。
- `ohos-arkTSScript` 仍保持 `resultReady && scriptDone` 后才输出 stdout。

完成判定：

- `scriptManager.completeArkTSScript(result)` 能把合法结果写入 helper。
- helper register / complete / clear 生命周期可验证。
- `CompletearktsScript` 无残留绑定。
- 脚本通过 `scriptManager.completeArkTSScript` 可完成一次完整结果上报。

本阶段不做：

- 不调整 README/test/schema。

子任务拆分：
1. standalone completion helper
    先新增共享 helper 库，只做 RegisterStandaloneScriptCompleteCallback / ClearStandaloneScriptCompleteCallback / CompleteStandaloneScript，不碰业务字段、不碰 stdout。

2. scriptManager.completeArkTSScript
    把现在的 stub 改成真实实现，只做严格参数校验和 helper 上报。
    completeArkTSScriptInApp 保持原样，避免影响 in-app 链路。

3. ohos-arkTSScript 接 helper
    在工具 runtime 启动时注册 callback，退出时清理；把 helper 回调接回现有 ExecutionContext，但先不动脚本侧调用方式。

4. 脚本侧迁移
    删除 globalThis.CompletearktsScript 绑定，改成脚本通过 scriptManager.completeArkTSScript(...) 上报结果。
    独立脚本完成结果的对外上报接口统一收敛为 scriptManager.completeArkTSScript(...)。开发者脚本必须通过模块 import 获取 scriptManager；ohos-arktsScript 不新增 globalThis.scriptManager，也不主动加载 scriptManager 模块，模块解析与加载仍遵循脚本自身 import 语义。ohos-arktsScript 仅保留构建、链接与运行环境层面的内部依赖，以保证该模块在工具执行环境中可正常解析和使用；该内部依赖不构成对外 API 接口。


5. 最小验证
    做 Phase 2 范围内的检查：
    completeArkTSScript 非工具环境拒绝；
    helper register/complete/clear 生命周期正常；
    ohos-arkTSScript 仍然只在 resultReady && scriptDone 后输出结果。


### 6.3 Phase 3：CLI 结果收敛

目标：

- 完成 independent skill CLI session stdout 结果收敛和失败分支收口。

必须修改的文件：

- `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp`

必须新增的文件：

- 无。

关键实现点：

- independent skill stdout 成功解析后转换为上层约定的 `code/result/uris/flags`。
- `session.status != completed`、`exitCode != 0`、`stdout` 非法时按 CLI session 失败规则收口。

影响的现有流程：

- 独立 skill 的 CLI 结果收敛逻辑从“仅能启动 session”变为“可正确识别成功/失败结果”。

回归要求：

- independent skill CLI session 成功路径和失败路径都能稳定收敛。

完成判定：

- stdout 协议错误、session failed、子进程非 0 退出都能被 CLI session 结果收敛逻辑正确识别。

本阶段不做：

- 不新增额外兼容层。
- 不保留旧 `uri/flag` 输出协议。
- 不新增 JS 包装文件或打包链路。

### 6.4 Phase 4：测试、README、schema、回归收口

目标：

- 完成测试补齐、README 更新、schema 统一和回归收口。

必须修改的文件：

- `tools/ohos-arkTSScript/config.json`
- `tools/ohos-arkTSScript/README.md`
- `tools/ohos-arkTSScript/test.js`
- `test/unittest/arkts_script_test/arkts_script_test.cpp`
- `test/unittest/arkts_script_test/BUILD.gn`
- `test/unittest/cli_tool_mgr/BUILD.gn`

必须新增的文件：

- `test/unittest/cli_tool_mgr/standalone_script_completion_test/*`
- `test/unittest/cli_tool_mgr/standalone_skill_exec_tool_test/*`

关键实现点：

- `config.json` output schema 统一改为 `uris/flags`。
- README 示例同步迁移到 `scriptManager.completeArkTSScript`。
- `test.js` 所有场景同步迁移。
- 新增 helper 与 independent skill `execTool` 分流单测。

影响的现有流程：

- 开发文档、测试脚本、schema 同步更新。

回归要求：

- 第 7 章测试矩阵全部覆盖。
- in-app skill 原流程不回归。

完成判定：

- 所有目标场景有对应用例或脚本覆盖。
- 文档、schema、测试与实现字段保持一致。

本阶段不做：

- 不继续扩展到 `ohos-insightIntent`。

## 7. 测试与验收

测试矩阵固定如下：

- `cliManager.execTool` 成功拉起 independent skill CLI session，并能通过 session 结果获得 `code/result/uris/flags` stdout。
- `ValidateSkillTypeFromParam` 返回 independent 时，`ExecTool` 能进入独立 CLI 路径；`skillType` 仅来自包管理查询。
- independent skill 缺少 `bundleName/moduleName/abcPath/functionName` 时返回可定位错误。
- BMS 查不到 bundle/module 时返回可定位错误。
- CLI session `failed`、`timedOut`、`exitCode != 0` 时 reject。
- stdout 为空、非 JSON、缺少 `code`、`uris` 含非 string、`flags` 非 number 时 reject。
- `scriptManager.completeArkTSScript` 在非工具运行环境下 reject。
- `scriptManager.completeArkTSScript` 缺 `code`、`result` 非 object、`uris` 非 array、`flags` 非 number 时 reject。
- helper register / complete / clear 生命周期正确。
- `ohos-arkTSScript` 仍保持 `resultReady && scriptDone` 后才输出 stdout。
- `completeArkTSScriptInApp` 原流程回归不变。
- `config.json` 的 output schema、README 示例、`test.js` 场景全部同步到 `uris/flags`。

验收标准：

- 独立 skill 主流程可完成端到端调用与结果返回。
- 所有失败分支能返回可定位的错误信息。
- 无 in-app skill 行为回归。
- 无旧 `CompletearktsScript` 残留依赖。

## 8. bugfix记录

本板块用于记录开发与联调阶段发现的问题，便于追踪修复状态与回归结论。新增记录时按时间倒序追加。

| 日期 | 模块 | 触发条件 | 问题现象 | 原因分析 | 修复方案 | 回归用例 | 是否影响 in-app | 状态 | 备注 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| TBD | `cliManager` / `scriptManager` / `ohos-arkTSScript` | 待补充 | 待补充 | 待补充 | 待补充 | 待补充 | 待补充 | Open | 初始化模板 |

记录约定：

- `日期`：使用 `YYYY-MM-DD`。
- `模块`：尽量精确到子模块或接口，例如 `OnCompleteArkTSScript`、`DispatchExecuteSkill`。
- `触发条件`：说明问题出现的前置条件、输入或环境。
- `问题现象`：描述用户可见行为或日志特征，不只写“执行失败”。
- `原因分析`：记录根因，不只写表层报错。
- `修复方案`：说明修复点，必要时附关键 commit 或测试名。
- `回归用例`：记录用于兜底该问题的测试或脚本场景。
- `是否影响 in-app`：建议使用 `Yes/No`。
- `状态`：建议使用 `Open`、`Fixed`、`Verified`。
