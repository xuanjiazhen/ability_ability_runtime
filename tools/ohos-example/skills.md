# CLI Example Tool

## 概述

这是一个示例 CLI 工具，演示如何使用 `ohos_cli_executable` 模板来编译和安装 OpenHarmony CLI 工具。

## 功能

本工具提供以下命令：

### run
运行示例工具

**参数：**
- `string` - 输入字符串

**输出：**
```json
{
  "result": "string"
}
```

### version
显示版本信息

**参数：**
- 无

**输出：**
```json
{
  "version": "string",
  "build_time": "string"
}
```

## 构建

使用以下命令构建此工具：

```bash
./build.sh --product-name rk3568 --build-target cli-example
```

或构建整个 ability_runtime：

```bash
./build.sh --product-name rk3568 --build-target ability_runtime
```

## 安装位置

构建完成后，文件将被安装到：

- 可执行文件：`/system/bin/cli_tool/tools/ohos-example`
- 配置文件：`/system/bin/cli_tool/cli_tools.json`
- 注册表：`/system/bin/cli_tool/cli_tools.json`

## 使用

### 运行工具

```bash
/system/bin/cli_tool/tools/ohos-example run "Hello OpenHarmony"
```

### 查看版本

```bash
/system/bin/cli_tool/tools/ohos-example version
```

## 开发指南

### 创建新的 CLI 工具

1. **创建目录结构：**
```
your-tool/
├── src/
│   └── main.cpp
├── config.json
├── skills.md
└── BUILD.gn
```

2. **编写 config.json：**
```json
{
  "name": "your-tool",
  "description": "工具描述",
  "command_list": [
    {
      "command_name": "command1",
      "command_description": "命令描述",
      "command_param_list": ["param1", "param2"],
      "command_out_schema": {}
    }
  ]
}
```

3. **编写 BUILD.gn：**
```gni
import("//build/templates/cxx/ohos_cli_executable.gni")

ohos_cli_executable("your-tool") {
  sources = [ "src/main.cpp" ]
  subsystem_name = "your_subsystem"
  part_name = "your_part"

  cli_config_file = "config.json"

  install_enable = true
  install_images = [ "system" ]

  deps = [...]
  external_deps = [...]
}
```

### 命令冲突检测

如果两个工具定义了相同的命令名，构建会失败并显示冲突信息。例如：

```
错误: 命令冲突: 'run' 在工具 'tool1' 和 'tool2' 中都定义了
```

要解决这个问题，请确保每个工具的命令名称唯一。

## 参考

- `ohos_cli_executable` 模板：`/build/templates/cxx/ohos_cli_executable.gni`
- 配置验证脚本：`/build/ohos/cli/merge_cli_config.py`
- 打包安装脚本：`/build/ohos/packages/install_cli_tools.py`
