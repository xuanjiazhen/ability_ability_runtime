# 独立 Skill 分流实施规格

## 1. 分流方案

本规格只覆盖 `cliManager.execTool(...)` 对独立 skill 的分流实现，不覆盖 `scriptManager`、`ohos-arkTSScript` 完成态协议、stdout 协议或其他非分流主题。

### 1.1 分流目标

- `cliManager.execTool` 继续作为 `ohos-arkTSScript` skill 的统一入口。
- `CliToolManagerService::ExecTool` 基于 skill 查询结果区分应用 skill 与独立 skill。
- 应用 skill 继续走现有 `SetupAndStartSkillSession(...)`。
- 独立 skill 不走 in-app `ExecuteInAppSkillWithTokenId`，而是在进入普通 CLI 链路前额外准备 `abcPath`，然后复用现有 CLI 公共链路。

### 1.2 分流真相来源

- skill 类型的唯一真相来源是 skill 查询接口返回的 `AppExecFwk::SkillInfo`。
- 分流判断统一使用真实枚举值：
  - `skillInfo.skillType == AppExecFwk::SkillType::APP_SKILL`
  - `skillInfo.skillType == AppExecFwk::SkillType::INDEPENDENT_SKILL`
- 不允许在 `climgr` 或 `js_skill_driver` 中继续使用本地写死常量，例如 `SKILL_TYPE_INDEPENDENT = -1`。
- 不允许根据 `skillToolType` 或其他本地字段自行推断 independent skill。

### 1.3 参数处理规则

- 独立 skill 和应用 skill 的参数处理规则保持一致。
- 独立 skill 不单独定义一套 `args` 改写协议。
- 独立 skill 不单独定义一套 `scriptPath/functionName/args` 过滤规则。
- 独立 skill 相比应用 skill 的唯一区别是：在调用 `ohos-arkTSScript` 前，需要额外补齐 `abcPath`。

### 1.4 `abcPath` 生成规则

`abcPath` 的生成顺序固定如下：

1. 从 skill 查询结果中读取 `skillInfo.skillPath`。
2. 将 `skillInfo.skillPath` 映射为 sandbox path，得到 `mappedSkillPath`。
3. 读取调用参数中的 `scriptPath`。
4. 按 `mappedSkillPath + "/" + scriptPath` 生成最终 `abcPath`。

约束如下：

- `scriptPath` 是独立 skill 必需输入。
- `skillInfo.skillPath` 是独立 skill 生成 `abcPath` 的唯一 skill 元数据来源。
- 服务侧只负责 sandbox path 映射和字符串拼接。
- 服务侧不负责校验 `scriptPath` 或最终 `abcPath` 的业务合法性。

### 1.5 `ExecTool` 主流程

`CliToolManagerService::ExecTool(...)` 的分流顺序固定如下：

1. `ValidateExecToolPermissions()`
2. 若 `toolName != "ohos-arkTSScript"`，保持现有普通 CLI 路径不变
3. 若 `toolName == "ohos-arkTSScript"`，调用一次 skill 查询接口并获取 `SkillInfo`
4. 若 skill 查询失败，直接返回错误
5. 若 `skillInfo.skillType != AppExecFwk::SkillType::INDEPENDENT_SKILL`，继续走现有 `SetupAndStartSkillSession(...)`
6. 若 `skillInfo.skillType == AppExecFwk::SkillType::INDEPENDENT_SKILL`，构造一个 `ExecToolParam` 副本 `cliParam`
7. 调用 `PrepareIndependentSkillCliParam(param, skillInfo, cliParam)`，只补齐独立 skill 所需的 `abcPath`
8. 对 `cliParam` 继续复用现有普通 CLI 公共链路：
   - `ValidateSessionLimit()`
   - `ValidateAndPrepareTool(cliParam, ...)`
   - `SetupAndStartSession(cliParam, ...)`

### 1.6 参数对象使用方式

- 原始请求参数 `param` 保持不变，不直接在其上修改业务字段。
- 当请求被判定为独立 skill 时，先基于 `param` 生成一个副本 `cliParam`。
- `cliParam` 只在 `toolName == "ohos-arkTSScript"` 且命中 independent skill 分支时创建和使用。
- 其他工具以及非 independent skill 场景不创建 `cliParam`，也不会看到 `abcPath` 参数。
- `cliParam` 只用于后续普通 CLI 公共链路。
- `cliParam` 与 `param` 的区别只在于：`cliParam` 中额外补入最终要传给 `ohos-arkTSScript` 的 `abcPath`。
- 除 `abcPath` 外，其余参数沿用与应用 skill 相同的处理语义。

## 2. 函数签名变化

### 2.1 AbilityManager 查询接口

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
  - 同步改为透传 `SkillInfo`

- `services/abilitymgr/src/ability_manager_proxy.cpp`
  - 同步改为透传 `SkillInfo`
  - IPC 返回值从读取 `int32_t skillType` 改为读取 `SkillInfo`

- `services/abilitymgr/src/ability_manager_stub.cpp`
  - 同步改为透传 `SkillInfo`
  - IPC 写回值从写回 `int32_t skillType` 改为写回 `SkillInfo`

- `services/abilitymgr/src/ability_manager_service.cpp`
  - 旧：
    - `int32_t AbilityManagerService::QuerySkillType(..., int32_t &skillType)`
  - 新：
    - `int32_t AbilityManagerService::QuerySkillType(..., AppExecFwk::SkillInfo &skillInfo)`
  - 服务侧继续复用 `SkillExecuteManager::QuerySkillInfo(...)`，但不再丢弃 `skillPath`、`skillType` 等字段

说明：

- 若后续决定统一重命名为 `QuerySkillInfo(...)`，允许在 interface、client、proxy、stub、service 五处同步重命名。
- 若本次只改签名不改名，也满足本规格。

### 2.2 climgr service 内部接口

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
  - 若需要显式返回 CLI 副本，建议扩展为：
    - `int32_t TryDispatchSkillSession(const ExecToolParam &param, const std::string &eventId, const ToolInfo &toolInfo, bool &dispatched, bool &useCliOverride, ExecToolParam &cliParamOverride)`

说明：

- `PrepareIndependentSkillCliParam(...)` 的职责只包括：
  - 读取 `skillInfo.skillPath`
  - 映射 sandbox path
  - 生成最终 `abcPath`
  - 写入 `cliParam`
- `PrepareIndependentSkillCliParam(...)` 不负责权限、并发、sandbox、session 处理。

### 2.3 ToolUtil 辅助接口

- `cli_tool_framework/services/climgr/include/tool_util.h`
  - 新增建议：
    - `static bool MapSkillPathToSandboxPath(const std::string &skillPath, std::string &mappedSkillPath);`
    - `static bool BuildIndependentSkillAbcPath(const std::string &mappedSkillPath, const std::string &scriptPath, std::string &abcPath);`

- `cli_tool_framework/services/climgr/src/tool_util.cpp`
  - 新增职责：
    - `skillPath -> sandbox path` 映射
    - 最终 `abcPath` 拼接
  - 现有 `TransferToCmdParam(...)`、`GetParamStringValue(...)` 保持原有职责

说明：

- `ToolUtil` 对独立 skill 的新增职责只围绕 `abcPath` 准备展开。
- 不新增只服务独立 skill 的 `args` 编码或 JSON string 改写逻辑。

### 2.4 js skill driver

- `cli_tool_framework/frameworks/js/napi/skill_driver/src/js_skill_driver.cpp`
  - 旧：
    - `DispatchExecuteSkill(..., int32_t skillType)` 内部依赖 `SKILL_TYPE_INDEPENDENT = -1`
  - 新：
    - `DispatchExecuteSkill(...)` 查询 `AppExecFwk::SkillInfo`
    - 使用 `skillInfo.skillType == AppExecFwk::SkillType::INDEPENDENT_SKILL` 判断

说明：

- 即使当前阶段 `js_skill_driver` 仍不支持直接执行独立 skill，也不能再依赖本地假值判断。

## 3. 影响文件

### 3.1 必改文件

| 类型 | 文件 | 目的 |
| --- | --- | --- |
| 修改 | `interfaces/inner_api/ability_manager/include/ability_manager_interface.h` | skill 查询接口返回 `SkillInfo` |
| 修改 | `interfaces/inner_api/ability_manager/include/ability_manager_client.h` | skill 查询 client 签名同步 |
| 修改 | `services/abilitymgr/src/ability_manager_client.cpp` | client 实现同步 |
| 修改 | `services/abilitymgr/src/ability_manager_proxy.cpp` | proxy IPC 读写 `SkillInfo` |
| 修改 | `services/abilitymgr/src/ability_manager_stub.cpp` | stub IPC 读写 `SkillInfo` |
| 修改 | `services/abilitymgr/src/ability_manager_service.cpp` | service 直接返回 `SkillInfo` |
| 修改 | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 声明 independent skill 分流与 `abcPath` 准备 helper |
| 修改 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 基于 `SkillInfo` 分流 independent skill |
| 修改 | `cli_tool_framework/services/climgr/include/tool_util.h` | 声明 sandbox path 映射与 `abcPath` 拼接 helper |
| 修改 | `cli_tool_framework/services/climgr/src/tool_util.cpp` | 实现 sandbox path 映射与 `abcPath` 拼接 |
| 修改 | `cli_tool_framework/frameworks/js/napi/skill_driver/src/js_skill_driver.cpp` | 改为基于真实枚举判断 independent skill |

### 3.2 可选同步文件

| 类型 | 文件 | 目的 |
| --- | --- | --- |
| 修改 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | 如有需要，仅补齐入参错误信息；不承担分流逻辑 |

## 4. 回归要求

- 普通 CLI tool 路径不回归。
- in-app skill `SetupAndStartSkillSession(...)` 路径不回归。
- skill 查询失败时，`ExecTool` 能返回可定位错误。
- `skillInfo.skillType == AppExecFwk::SkillType::INDEPENDENT_SKILL` 时，`ExecTool` 能进入独立 skill CLI 路径。
- independent skill 缺少 `skillName/scriptPath/functionName` 时，能返回可定位错误。
- `skillInfo.skillPath` 为空或 sandbox path 映射失败时，能返回可定位错误。
- 输入合法 independent skill 参数时，可正确生成 `abcPath = mappedSkillPath + "/" + scriptPath` 并继续发起 CLI 调度。
- `skillName` 为空时，是否能完成分流取决于 skill 查询接口返回结果，service 侧不再做本地兼容推断。
