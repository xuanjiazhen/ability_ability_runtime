# ohos_cli_executable 模板使用说明

## 文档信息

| 项 | 内容 |
|------|------|
| 文档版本 | `v1.0` |
| 更新时间 | `2026-04-26` |
| 适用范围 | `ohos_cli_executable` 模板使用、`config.json` 编写与本地校验 |
| 对应实现 | `//build/ohos/cli/ohos_cli_executable.gni`、`//build/ohos/cli/cli_profile.py` |
| 维护说明 | 模板行为变更时，需同步更新本文档中的输入结构、输出路径和限制说明 |

## 版本更新记录

| 版本 | 日期 | 变更内容 |
|------|------|------|
| `v1.0` | `2026-04-26` | 首次发布模板使用说明；说明当前模板构建与配置导出方式；明确 `target_name == config.name`、禁用 `output_name`；补充 `cli_config_validator` 本地校验说明 |

## 1. 模板简介

`ohos_cli_executable` 是基于 `ohos_executable` 封装的 CLI 工具模板，用于同时完成以下工作：

1. 校验 `config.json`
2. 生成最终导出的工具配置文件
3. 构建 CLI 二进制
4. 将二进制和导出配置分别安装到系统镜像

与普通 `ohos_executable` 相比，`ohos_cli_executable` 额外负责处理 CLI 配置文件，并将导出配置直接安装到系统镜像。

## 2. 使用指南

### 2.1 最小调用方式

```gn
import("//build/ohos/cli/ohos_cli_executable.gni")

ohos_cli_executable("ohos-demo-cli") {
  cli_config_file = "config.json"
  subsystem_name = "ability"
  part_name = "ability_runtime"

  sources = [ "src/main.cpp" ]
}
```

### 2.2 必填参数

| 参数 | 说明 |
|------|------|
| `cli_config_file` | 工具配置文件路径，通常为当前目录下的 `config.json` |
| `subsystem_name` | 所属 subsystem |
| `part_name` | 所属 part |

### 2.3 常用透传参数

以下参数会继续传给内部的 `ohos_executable`：

- `sources`
- `deps`
- `external_deps`
- `defines`
- `visibility`
- `testonly`

### 2.4 使用限制

- 不支持 `output_name`
- `target_name` 必须与 `config.name` 完全一致
- 模板会将最终二进制路径固定到 `/system/bin/cli_tool/executable/<tool>`
- 模板会将导出配置文件固定安装到 `/system/bin/cli_tool/configs/<tool>.json`

### 2.5 配置文件规则说明

`config.json` 的字段、类型、映射模式和事件规则，以正式规则文档为准：

- [config校验规则.md](config校验规则.md)

其中需要特别注意：

- `ohos_cli_executable` 构建阶段额外要求 `config.name == target_name`
- 该约束属于模板级校验，不属于单纯的 `config.json` 静态字段语义

### 2.6 本地校验说明

在提交前，可使用 `cli_config_validator` 对 `config.json` 做本地自检：

```bash
python3 cli_config_validator/main.py validate <path/to/config.json>
python3 cli_config_validator/main.py validate <path/to/config.json> --json
```

说明：

- `cli_config_validator` 用于校验 `config.json` 本身是否符合正式规则
- `config.name == target_name` 这类 `ohos_cli_executable` 模板级约束，仍以构建期校验结果为准

### 2.7 示例编译命令

若你在业务目录中新增了一个名为 `ohos-demo-cli` 的模板实例，可直接使用如下命令单独编译：

```bash
./build.sh --product-name rk3568 --build-target ohos-demo-cli
```

## 3. 输入文件结构

一个最小示例工具目录可以组织为：

```text
tools/ohos_cli_demo/
├── BUILD.gn
├── config.json
└── src/
    └── main.cpp
```

### 3.1 BUILD.gn

```gn
import("//build/ohos/cli/ohos_cli_executable.gni")

ohos_cli_executable("ohos-demo-cli") {
  cli_config_file = "config.json"
  subsystem_name = "ability"
  part_name = "ability_runtime"

  sources = [ "src/main.cpp" ]
}
```

### 3.2 config.json

```json
{
  "name": "ohos-demo-cli",
  "version": "1.0.0",
  "description": "Minimal ohos_cli_executable demo tool",
  "executablePath": "/system/bin/cli_tool/executable/ohos-demo-cli",
  "inputSchema": {
    "type": "object",
    "properties": {
      "message": {
        "type": "string",
        "description": "Message to print"
      }
    },
    "required": ["message"]
  },
  "outputSchema": {
    "type": "object",
    "properties": {
      "message": {
        "type": "string"
      }
    },
    "required": ["message"]
  },
  "argMapping": {
    "type": "flag",
    "templates": {
      "message": "--message={value}"
    }
  }
}
```

### 3.3 src/main.cpp

```cpp
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    std::string message = "default";
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        constexpr const char *prefix = "--message=";
        if (arg.rfind(prefix, 0) == 0) {
            message = arg.substr(std::char_traits<char>::length(prefix));
        }
    }

    std::cout << "{\"message\":\"" << message << "\"}" << std::endl;
    return 0;
}
```

## 4. 输出目录结构

### 4.1 构建中间产物

模板执行后，会在构建目录生成以下中间产物：

```text
out/<product>/
└── cli_profile/
    ├── inputs/
    │   └── <part_name>/
    │       └── <tool>_info.json
    └── configs/
        └── <part_name>/
            └── <tool>.json
```

以示例工具为例：

```text
out/rk3568/
└── cli_profile/
    ├── inputs/
    │   └── ability_runtime/
    │       └── ohos-demo-cli_info.json
    └── configs/
        └── ability_runtime/
            └── ohos-demo-cli.json
```

### 4.2 镜像安装结果

最终安装到系统镜像中的路径如下：

```text
/system/bin/cli_tool/
├── configs/
│   └── ohos-demo-cli.json
└── executable/
    └── ohos-demo-cli
```

### 4.3 导出配置与原始配置的区别

导出的 `<tool>.json` 保持原始 `config.json` 的字段顺序与主体内容，仅做一项构建期修正：

- `executablePath` 被替换为最终系统绝对路径

原始 `config.json` 文件不会被修改。

## 5. 示例说明

本文中的 `ohos-demo-cli` 仅作为说明性示例，不要求仓内必须存在同名目录。你可以参考本文结构，在自己的业务目录中创建等价示例。

该示例适用于：

- 学习模板最小用法
- 本地校验 `config.json`
- 手动验证模板构建链路

若按本文示例目录创建，其目标 label 可以写为：

```text
//foundation/ability/ability_runtime/tools/ohos_cli_demo:ohos-demo-cli
```

## 6. 执行流程

`ohos_cli_executable` 当前实现按以下顺序工作：

1. 校验 `config.json`
2. 生成 `<tool>_info.json`
3. 生成导出配置 `<tool>.json`
4. 调用 `ohos_executable` 生成二进制
5. 通过 `ohos_copy` 安装导出配置文件

该方案的特点是：

- 每个工具独立导出和安装自己的配置文件
- 二进制与配置文件由模板直接完成构建和安装

## 7. 注意事项

- `config.name` 必须等于 `target_name`
- `output_name` 不允许使用
- `executablePath` 在源码配置中建议直接写最终目标路径，便于阅读和比对
- 正式规则变更时，应同步更新：
  - 本文档
  - `config校验规则.md`
  - 示例工具中的 `config.json`
- 文中的示例目录结构为说明性内容，不代表仓内必须保留对应 demo 目标
