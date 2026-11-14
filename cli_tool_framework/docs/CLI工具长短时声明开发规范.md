# CLI 工具长短时声明开发规范

## 1. 概述

CLI 工具框架支持工具在 `config.json` 中声明"长短时"属性，供调用方（如语音助手等智能入口）在执行前查询，以便选择超时策略、是否后台执行、是否提示用户等待等。

字段定义：

| 属性 | 值 |
| ---- | ---- |
| 字段名 | `isLongRunning` |
| 类型 | boolean（可选） |
| 声明位置 | 工具级：`config.json` 顶层；子命令级：`subcommands.<name>` 内 |
| 缺省值 | `false`（短时） |

声明字段仅描述工具属性，**不改变任何运行时行为**：`execTool`/`execCmd` 的超时上限（`MAX_TIMEOUT`，30 分钟）与未传 `timeout` 时的默认取值保持不变，由调用方依据声明自行决定超时与后台策略。

## 2. 优先级规则

子命令的长短时生效值按以下优先级解析（由 CLImgr 在配置加载时一次性计算）：

```
子命令级声明 > 工具级声明 > 缺省 false
```

- 子命令级一旦声明（含显式 `false`），即覆盖工具级声明；
- 子命令级未声明时，回退工具级声明；
- 工具级也未声明时，为短时（`false`）。

对外查询接口的语义：

| 查询面 | 语义 |
| ------ | ---- |
| `ToolInfo.isLongRunning` | 工具级声明值（非整体生效值） |
| `SubCommandInfo.isLongRunning` | 该子命令的**生效值**（已按上述优先级解析） |

调用方查询子命令属性时直接使用 `ToolInfo.subcommands.<name>.isLongRunning`，无需自行组合回退规则。

## 3. 配置示例

工具级声明（简单工具）：

```json
{
    "name": "ohos-example",
    "version": "1.0.0",
    "description": "Example CLI tool with a tool-level long-running declaration",
    "executablePath": "/system/bin/cli_tool/executable/ohos-example",
    "requirePermissions": [],
    "inputSchema": { "type": "object", "properties": {} },
    "outputSchema": { "type": "object", "properties": {} },
    "isLongRunning": true
}
```

子命令级差异化声明（`wait` 显式长时、`status` 未声明回退工具级、`version` 显式覆盖为短时）：

```json
{
    "name": "ohos-timer",
    "hasSubCommand": true,
    "isLongRunning": true,
    "subcommands": {
        "wait": {
            "description": "Wait for the given duration",
            "isLongRunning": true
        },
        "status": {
            "description": "Query the status of the running timer"
        },
        "version": {
            "description": "Print the tool version (short-running)",
            "isLongRunning": false
        }
    }
}
```

完整样例见 `cli_tool_framework/tools/ohos-timer/config.json` 与 `cli_tool_framework/tools/ohos-example/config.json`。

## 4. 构建与校验

- `isLongRunning` 已在构建配置校验白名单（build 仓 `ohos/cli/cli_info_process`）中登记，工具级与子命令级均支持；
- 构建期仅做**类型校验**：非布尔值（如字符串 `"yes"`）会报 `TOP_isLongRunning_001`/`SUB_isLongRunning_001` 校验错误；
- 构建期不做语义拦截：不校验子命令级声明与 `hasSubCommand` 的关系，也不因声明长时而拒绝构建；层级语义由运行时容错处理（未启用子命令的配置中该字段无副作用）。

## 5. 调用方配合方式与注意事项

调用方配合：

- 长时工具/子命令：调用方按需在 `execTool`/`execCmd` 中**显式传 `timeout`**（仍受 30 分钟上限约束），并考虑后台执行与用户等待提示；
- 短时工具/子命令：沿用默认超时策略。

注意事项：

- 未声明该字段的存量工具一律按短时处理，加载行为与合入前完全一致；
- `ToolSummary`（`queryToolSummaries` 轻量列表）不携带该字段，需要长短时属性时用 `queryTools`/`getToolInfoByName` 查询；
- 子命令级声明仅在 `hasSubCommand: true` 时有意义，简单工具只使用工具级声明；
- 旧版本服务端解析时忽略未知字段，无新旧错配拒绝场景；但旧构建脚本会拒绝未登记字段，工具侧需在合入顺序上确保构建校验先行。
