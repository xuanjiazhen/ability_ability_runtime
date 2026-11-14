# ohos-aa

`ohos-aa` 是 Ability 调试管理相关的 CLI 工具，经 CLI 工具框架（CLImgr）注册后供系统侧调用方通过 `@ohos.app.cli.cliManager` 执行。

## 长短时声明

工具可在 `config.json` 中声明 `isLongRunning`（可选布尔，缺省 `false`），用于向调用方（如语音助手）标识工具或子命令是否为长时任务：

- 工具级声明写在 `config.json` 顶层；
- 子命令级声明写在 `subcommands.<name>` 内，优先级高于工具级声明；
- 未声明时一律按短时处理，存量行为不变。

字段语义、默认值、优先级规则与调用方超时策略配合方式，统一参见[《CLI 工具长短时声明开发规范》](../../docs/CLI工具长短时声明开发规范.md)。

注意：声明仅用于查询，不改变 `execTool`/`execCmd` 的超时上限（30 分钟）与默认超时取值；长时任务建议调用方显式传 `timeout` 并考虑后台执行。
