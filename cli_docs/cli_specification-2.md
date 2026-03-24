# CLI 工具开发规范

## 文档信息

**版本**: v1.2
**创建日期**: 2025-04-11
**最后更新**: 2025-04-11
**适用框架版本**: CLI Manager v2.8
**目标读者**: CLI 工具开发者

---

## 目录

1. [概述](#概述)
2. [快速开始](#快速开始)
   - [2.1 5分钟上手指南](#21-5分钟上手指南)
   - [2.2 开发流程概览](#22-开发流程概览)
   - [2.3 最小化示例](#23-最小化示例)
3. [CLI 工具代码目录结构](#cli-工具代码目录结构)
4. [文件命名规范](#文件命名规范)
5. [CLI 工具描述文件规范](#cli-工具描述文件规范)
   - [5.1 基本结构与字段](#51-基本结构与字段)
   - [5.2 工具类型分类](#52-工具类型分类)
   - [5.3 输入参数规范](#53-输入参数规范)
   - [5.4 输出参数规范](#54-输出参数规范)
   - [5.5 参数映射规范](#55-参数映射规范)
6. [完整示例](#完整示例)
7. [开发最佳实践](#开发最佳实践)
   - [7.1 编码规范](#71-编码规范)
   - [7.2 测试策略](#72-测试策略)
   - [7.3 安全实践](#73-安全实践)
   - [7.4 性能优化建议](#74-性能优化建议)
8. [常见问题与调试](#常见问题与调试)
   - [8.1 常见错误](#81-常见错误)
   - [8.2 调试技巧](#82-调试技巧)
   - [8.3 FAQ](#83-faq)

---

## 概述

### 目的

本规范旨在为 CLI 工具开发者提供完整的开发指南，确保工具能够正确集成到 OpenHarmony CLI Manager 框架中。

### 核心原则

| 原则 | 说明 |
|------|------|
| **声明式配置** | 所有工具能力通过 JSON 描述文件声明 |
| **标准化接口** | 遵循统一的参数传递和事件输出规范 |
| **安全优先** | 工具必须通过安全验证才能注册 |
| **可测试性** | 工具应该支持独立运行和测试 |

### 开发流程

```
1. 创建工具目录结构
   ↓
2. 编写工具可执行文件
   ↓
3. 创建工具描述文件
   ↓
4. 本地测试工具
   ↓
5. 注册到系统
   ↓
6. 集成测试
```

---

## 快速开始

欢迎使用 CLI 工具管理框架！本章节帮助你在 **15 分钟内**创建第一个 CLI 工具。

### 2.1 5分钟上手指南

创建 CLI 工具只需 5 步：

```
步骤1: 创建目录结构      [2分钟]
   ↓
步骤2: 编写工具代码      [5分钟]
   ↓
步骤3: 创建描述文件      [5分钟]
   ↓
步骤4: 本地测试工具      [2分钟]
   ↓
步骤5: 注册到系统        [1分钟]
```

### 2.2 开发流程概览

```
┌─────────────────────────────────────────────────────────────┐
│                    CLI 工具开发流程                          │
└─────────────────────────────────────────────────────────────┘

阶段 1: 准备工作
├─ 1.1 确定工具功能需求
├─ 1.2 设计输入输出接口
└─ 1.3 确定参数映射方式

阶段 2: 开发实现
├─ 2.1 创建目录结构
├─ 2.2 编写工具代码
│   ├─ 参数解析
│   ├─ 核心功能实现
│   └─ 结果输出
└─ 2.3 创建描述文件

阶段 3: 测试验证
├─ 3.1 单元测试
├─ 3.2 本地运行测试
└─ 3.3 集成测试

阶段 4: 部署上线
├─ 4.1 编译打包
├─ 4.2 安装可执行文件
├─ 4.3 注册描述文件
└─ 4.4 验证注册
```

### 2.3 最小化示例

让我们创建一个最简单的 CLI 工具——**Hello World 工具**，它只输出 "Hello, World!"。

#### 步骤 1: 创建目录结构

```bash
# 创建工具目录
mkdir -p /usr/local/src/cli-tools/hello-world/{src,include,tests}
cd /usr/local/src/cli-tools/hello-world
```

#### 步骤 2: 编写工具代码

创建 `src/main.cpp`：

```cpp
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main() {
    try {
        // 构造输出结果
        json result;
        result["event"] = "result";
        result["data"]["message"] = "Hello, World!";
        result["data"]["timestamp"] = std::time(nullptr);

        // 输出 JSON 结果
        std::cout << result.dump() << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

#### 步骤 3: 创建描述文件

创建 `/etc/cli/tools/hello-world.json`：

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tool/v1",
  "name": "hello-world",
  "version": "1.0.0",
  "description": "A simple Hello World tool that demonstrates CLI framework integration",
  "executable": "/usr/bin/hello-world",
  "permissions": [],
  "input": {
    "type": "object",
    "properties": {},
    "required": []
  },
  "output": {
    "type": "object",
    "properties": {
      "message": {
        "type": "string",
        "description": "Output message"
      },
      "timestamp": {
        "type": "integer",
        "description": "Unix timestamp of the message"
      }
    }
  },
  "argumentMapping": {
    "type": "flag",
    "templates": {}
  },
  "timeout": 5000,
  "events": ["result"]
}
```

#### 步骤 4: 本地测试

```bash
# 编译
g++ -std=c++17 -o hello-world src/main.cpp -I/usr/include/nlohmann
sudo cp hello-world /usr/bin/

# 直接运行测试
/usr/bin/hello-world
# 输出: {"event":"result","data":{"message":"Hello, World!","timestamp":1712834400}}

# 验证 JSON 格式
jq . /etc/cli/tools/hello-world.json
```

#### 步骤 5: 从应用调用

```typescript
import CLI from '@ohos.cli.manager';

const result = await CLI.CLIManager.execTool('hello-world', {});
console.log(result.data.message);
// 输出: Hello, World!
```

🎉 **恭喜！你已经创建了第一个 CLI 工具！**

### 2.4 下一步

- 查看第3章了解完整的代码组织方式
- 查看第5章学习高级配置选项
- 查看第6章参考完整示例

---

## CLI 工具代码目录结构

### 标准目录结构

```
/usr/local/src/cli-tools/<tool-name>/
├── src/                    # 源代码目录
│   ├── main.c/cpp          # 主程序入口
│   ├── core.c/cpp          # 核心逻辑
│   └── utils.c/cpp         # 工具函数
├── include/                # 头文件目录（C/C++）
│   ├── main.h
│   ├── core.h
│   └── utils.h
├── tests/                  # 测试代码
│   ├── test_main.c/cpp
│   └── test_utils.c/cpp
├── docs/                   # 文档
│   ├── README.md
│   └── USAGE.md
├── scripts/                # 构建和部署脚本
│   ├── build.sh
│   └── install.sh
├── BUILD.gn                # GN 构建配置
├── README.md               # 项目说明
└── tool-descriptor.json    # 工具描述文件（开发时参考）
```

### 目录说明

#### src/ - 源代码

存放所有源代码文件。

**命名规范**:
- C 文件: `*.c`
- C++ 文件: `*.cpp`
- 头文件: `*.h` 或 `*.hpp`

**建议的文件组织**:
```
src/
├── main.cpp          # 主入口，参数解析
├── scanner.cpp       # 核心功能实现
├── parser.cpp        # 输出解析
└── utils.cpp         # 工具函数
```

#### include/ - 头文件

存放公共头文件。

```
include/
├── main.h
├── scanner.h
└── utils.h
```

#### tests/ - 测试代码

存放单元测试和集成测试。

```
tests/
├── test_scanner.cpp
├── test_parser.cpp
└── mocks/
    └── mock_scanner.h
```

#### docs/ - 文档

存放工具文档。

**必需文件**:
- `README.md` - 项目概述
- `USAGE.md` - 使用说明

**可选文件**:
- `API.md` - API 文档（如果是库）
- `DESIGN.md` - 设计文档
- `CHANGELOG.md` - 变更日志

#### scripts/ - 脚本

存放构建和部署脚本。

```
scripts/
├── build.sh          # 构建脚本
├── install.sh        # 安装脚本
├── test.sh           # 测试脚本
└── clean.sh          # 清理脚本
```

### BUILD.gn 配置示例

```gni
import("//build/ohos.gni")

ohos_executable("net-scan") {
  sources = [
    "src/main.cpp",
    "src/scanner.cpp",
    "src/parser.cpp",
    "src/utils.cpp",
  ]

  include_dirs = [
    "include",
    "//utils/include",
  ]

  deps = [
    "//utils/log:liblog",
    "//utils/json:libjson",
  ]

  external_deps = [ "hilog:libhilog" ]

  install_images = [
    "system",
  ]

  part_name = "cli-tools"
  subsystem_name = "utils"
}
```

### 安装规范

#### 可执行文件安装

```bash
# 安装到系统目录
install -m 755 net-scan /usr/bin/net-scan

# 设置所有者为 root
chown root:root /usr/bin/net-scan

# 确保没有 setuid/setgid
chmod 0755 /usr/bin/net-scan
```

#### 描述文件安装

```bash
# 安装描述文件
install -m 644 net-scan.json /etc/cli/tools/network-scanner.json

# 设置所有者为 root
chown root:root /etc/cli/tools/network-scanner.json
```

#### 注册到工具注册表

将工具描述添加到 `/etc/cli/cli_tools.json`:

```json
{
  "version": "1.0.0",
  "tools": [
    {
      "name": "network-scanner",
      "descriptor": "/etc/cli/tools/network-scanner.json"
    }
  ]
}
```

---

## 文件命名规范

### 工具命名规则

#### 基本规则

| 规则 | 说明 | 正确示例 | 错误示例 |
|------|------|----------|----------|
| **长度** | 1-64 个字符 | `"net-scan"` | `"a"` (过短) |
| **字符集** | 小写字母、数字、连字符 | `"my-tool-123"` | `"MyTool"` |
| **开头** | 必须是小写字母 | `"tool-name"` | `"-tool"` |
| **结尾** | 不能是连字符 | `"tool-name"` | `"tool-"` |
| **连字符** | 不能连续使用 | `"tool-name"` | `"tool--name"` |

#### 命名建议

1. **使用小写字母和连字符**
   ```
   ✅ network-scanner
   ✅ log-monitor
   ✅ file-copy
   ❌ network_scanner
   ❌ networkScanner
   ```

2. **使用动词-名词结构**
   ```
   ✅ file-copy
   ✅ log-monitor
   ✅ app-deploy
   ❌ copy-file
   ```

3. **避免缩写**
   ```
   ✅ network-scanner
   ❌ net-scan
   ```

### 文件路径规范

#### 描述文件

```
/etc/cli/tools/<tool-name>.json
```

**示例**:
```
/etc/cli/tools/network-scanner.json
/etc/cli/tools/log-monitor.json
```

#### 可执行文件

```
/usr/bin/<executable-name>
```

**命名规则**:
- 优先使用工具名称
- 可以使用更短的命令名
- 必须在描述文件的 `executable` 字段中明确指定

**示例**:
```
工具描述文件: /etc/cli/tools/network-scanner.json
可执行文件: /usr/bin/net-scan

描述文件中指定:
{
  "name": "network-scanner",
  "executable": "/usr/bin/net-scan"
}
```

---

## CLI 工具描述文件规范

### 5.1 基本结构与字段

CLI 工具描述文件是一个 JSON 文件，用于声明工具的元数据、输入输出格式和参数映射规则。

**文件位置**: `/etc/cli/tools/<tool-name>.json`

**基本格式**:

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tool/v1",
  "name": "tool-name",
  "version": "1.0.0",
  "description": "工具功能描述",
  "executable": "/usr/bin/tool-name",
  "permissions": [],
  "input": {
    "type": "object",
    "properties": {},
    "required": []
  },
  "output": {
    "type": "object",
    "properties": {}
  },
  "argumentMapping": {
    "type": "flag",
    "templates": {}
  },
  "timeout": 30000,
  "events": [],
  "eventSchemas": {}
}
```

#### 必填字段

| 字段 | 类型 | 说明 | 示例 |
|------|------|------|------|
| `name` | string | 工具唯一标识，小写字母、数字、连字符 | `"network-scanner"` |
| `version` | string | 语义化版本号 | `"1.0.0"` |
| `description` | string | 工具功能描述 | `"网络端口扫描工具"` |
| `executable` | string | 可执行文件绝对路径 | `"/usr/bin/net-scan"` |
| `input` | object | 输入参数 JSON Schema | 见下文 |
| `argumentMapping` | object | 参数映射规则 | 见下文 |

#### 可选字段

| 字段 | 类型 | 说明 | 默认值 |
|------|------|------|--------|
| `$schema` | string | JSON Schema 验证 URL | - |
| `permissions` | array | 所需权限列表 | `[]` |
| `output` | object | 输出结果 JSON Schema | - |
| `timeout` | number | 超时时间（毫秒），0 表示无限制 | `30000` |
| `events` | array | 支持的事件类型 | `[]` |
| `eventSchemas` | object | 事件 Schema 定义 | `{}` |
| `hasSubcommands` | boolean | 是否有子命令 | `false` |
| `subcommands` | object | 子命令集合 | - |

### 5.2 工具类型分类

CLI 工具根据功能复杂度分为两类：**无子命令工具**和**有子命令工具**。

#### 类型 1: 无子命令工具

**适用场景**: 单一功能的工具，如文件拷贝、网络扫描、日志查询等。

**特征**:
- 顶层定义 `input` 和 `argumentMapping`
- 不包含 `hasSubcommands` 字段或设置为 `false`
- 调用时直接传递参数
- 生成的命令格式: `executable [args]`

**完整示例**:

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tool/v1",
  "name": "network-scanner",
  "version": "1.0.0",
  "description": "Network port scanning utility for diagnosing network connectivity",
  "executable": "/usr/bin/net-scan",
  "permissions": [
    "ohos.permission.INTERNET",
    "ohos.permission.GET_NETWORK_INFO"
  ],

  "input": {
    "type": "object",
    "properties": {
      "target": {
        "type": "string",
        "format": "ipv4",
        "description": "Target IP address or hostname",
        "minLength": 7,
        "maxLength": 255,
        "example": "192.168.1.1"
      },
      "ports": {
        "type": "array",
        "description": "List of ports to scan",
        "items": {
          "type": "integer",
          "minimum": 1,
          "maximum": 65535,
          "description": "Port number"
        },
        "minItems": 1,
        "maxItems": 100,
        "example": [80, 443, 8080]
      },
      "timeout": {
        "type": "integer",
        "description": "Connection timeout in milliseconds",
        "minimum": 100,
        "maximum": 10000,
        "default": 1000
      },
      "verbose": {
        "type": "boolean",
        "description": "Enable verbose output",
        "default": false
      },
      "outputFormat": {
        "type": "string",
        "enum": ["json", "text", "csv"],
        "description": "Output format",
        "default": "json"
      }
    },
    "required": ["target", "ports"]
  },

  "output": {
    "type": "object",
    "description": "Scan results",
    "properties": {
      "results": {
        "type": "array",
        "description": "Port scan results",
        "items": {
          "type": "object",
          "properties": {
            "port": {
              "type": "integer",
              "description": "Port number"
            },
            "status": {
              "type": "string",
              "enum": ["open", "closed", "filtered", "timeout"],
              "description": "Port status"
            },
            "responseTime": {
              "type": "integer",
              "description": "Response time in milliseconds"
            },
            "service": {
              "type": "string",
              "description": "Service name (if detected)"
            }
          },
          "required": ["port", "status"]
        }
      },
      "summary": {
        "type": "object",
        "description": "Scan summary",
        "properties": {
          "target": {
            "type": "string",
            "description": "Scanned target"
          },
          "totalPorts": {
            "type": "integer",
            "description": "Total ports scanned"
          },
          "openPorts": {
            "type": "integer",
            "description": "Number of open ports"
          },
          "scanTime": {
            "type": "integer",
            "description": "Total scan time in milliseconds"
          }
        },
        "required": ["target", "totalPorts", "openPorts", "scanTime"]
      }
    }
  },

  "argumentMapping": {
    "type": "flag",
    "templates": {
      "target": "--target={value}",
      "ports": "--port={value}",
      "timeout": "--timeout={value}",
      "verbose": {
        "if_true": "-v",
        "if_false": ""
      },
      "outputFormat": "--format={value}"
    }
  },

  "timeout": 60000,
  "events": ["stdout", "result"]
}
```

**调用示例**:

```typescript
import CLI from '@ohos.cli.manager';

const result = await CLI.CLIManager.execTool('network-scanner', {
  target: '192.168.1.1',
  ports: [80, 443, 8080],
  timeout: 2000,
  verbose: true,
  outputFormat: 'json'
});
```

**生成的命令行**:

```bash
/usr/bin/net-scan --target=192.168.1.1 --port=80 --port=443 --port=8080 --timeout=2000 -v
```

#### 类型 2: 有子命令工具

**适用场景**: 多功能集合工具，如包管理器、版本控制系统、系统控制工具等。

**特征**:
- 必须设置 `hasSubcommands: true`
- 定义 `subcommands` 对象
- 每个子命令有独立的 `input`、`output` 和 `argumentMapping`
- 调用时必须指定 `subcommand` 字段
- 生成的命令格式: `executable <subcommand> [args]`

**完整示例**:

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tool/v1",
  "name": "package-manager",
  "version": "1.0.0",
  "description": "Package management tool for installing, removing, and managing software packages",
  "executable": "/usr/bin/pkg-mgr",
  "permissions": [
    "ohos.permission.INSTALL_BUNDLE",
    "ohos.permission.REMOVE_BUNDLE",
    "ohos.permission.ACCESS_BUNDLE"
  ],

  "hasSubcommands": true,

  "subcommands": {
    "install": {
      "description": "Install a package from local file or remote repository",
      "input": {
        "type": "object",
        "properties": {
          "package": {
            "type": "string",
            "description": "Package file path (e.g., /path/to/app.hap) or package name"
          },
          "force": {
            "type": "boolean",
            "description": "Force reinstall even if package already exists",
            "default": false
          },
          "silent": {
            "type": "boolean",
            "description": "Silent installation without user interaction",
            "default": false
          },
          "dependencies": {
            "type": "boolean",
            "description": "Install dependencies automatically",
            "default": true
          },
          "installDir": {
            "type": "string",
            "description": "Custom installation directory"
          }
        },
        "required": ["package"]
      },
      "output": {
        "type": "object",
        "properties": {
          "success": {
            "type": "boolean",
            "description": "Installation success status"
          },
          "packageName": {
            "type": "string",
            "description": "Installed package name"
          },
          "version": {
            "type": "string",
            "description": "Installed package version"
          },
          "installPath": {
            "type": "string",
            "description": "Installation path"
          },
          "message": {
            "type": "string",
            "description": "Installation message or error details"
          }
        },
        "required": ["success", "packageName"]
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}",
          "force": "--force",
          "silent": "--silent",
          "dependencies": {
            "if_true": "--with-deps",
            "if_false": "--no-deps"
          },
          "installDir": "--install-dir={value}"
        }
      }
    },

    "remove": {
      "description": "Remove an installed package and its configuration",
      "input": {
        "type": "object",
        "properties": {
          "package": {
            "type": "string",
            "description": "Package name to remove"
          },
          "purge": {
            "type": "boolean",
            "description": "Remove configuration files along with package",
            "default": false
          },
          "autoRemove": {
            "type": "boolean",
            "description": "Automatically remove unused dependencies",
            "default": true
          }
        },
        "required": ["package"]
      },
      "output": {
        "type": "object",
        "properties": {
          "success": {
            "type": "boolean"
          },
          "packageName": {
            "type": "string"
          },
          "removedSize": {
            "type": "integer",
            "description": "Size of removed package in bytes"
          },
          "message": {
            "type": "string"
          }
        },
        "required": ["success", "packageName"]
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}",
          "purge": "--purge",
          "autoRemove": "--auto-remove"
        }
      }
    },

    "list": {
      "description": "List all installed packages with optional filtering",
      "input": {
        "type": "object",
        "properties": {
          "verbose": {
            "type": "boolean",
            "description": "Show detailed information including version, size, etc.",
            "default": false
          },
          "filter": {
            "type": "string",
            "description": "Filter packages by name pattern (supports wildcards)"
          },
          "format": {
            "type": "string",
            "enum": ["table", "json", "csv"],
            "description": "Output format",
            "default": "table"
          },
          "sort": {
            "type": "string",
            "enum": ["name", "size", "date"],
            "description": "Sort field",
            "default": "name"
          }
        }
      },
      "output": {
        "type": "object",
        "properties": {
          "packages": {
            "type": "array",
            "items": {
              "type": "object",
              "properties": {
                "name": {
                  "type": "string"
                },
                "version": {
                  "type": "string"
                },
                "size": {
                  "type": "integer",
                  "description": "Package size in bytes"
                },
                "installDate": {
                  "type": "string",
                  "description": "Installation date (ISO 8601)"
                }
              }
            }
          },
          "total": {
            "type": "integer",
            "description": "Total number of packages"
          },
          "totalSize": {
            "type": "integer",
            "description": "Total size of all packages"
          }
        },
        "required": ["packages", "total"]
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "verbose": "-v",
          "filter": "--filter={value}",
          "format": "--format={value}",
          "sort": "--sort={value}"
        }
      }
    },

    "update": {
      "description": "Update installed packages to their latest versions",
      "input": {
        "type": "object",
        "properties": {
          "package": {
            "type": "string",
            "description": "Specific package to update (empty for all packages)"
          },
          "checkOnly": {
            "type": "boolean",
            "description": "Only check for updates without installing",
            "default": false
          },
          "security": {
            "type": "boolean",
            "description": "Only install security updates",
            "default": false
          }
        }
      },
      "output": {
        "type": "object",
        "properties": {
          "updated": {
            "type": "integer",
            "description": "Number of packages updated"
          },
          "failed": {
            "type": "integer",
            "description": "Number of packages that failed to update"
          },
          "skipped": {
            "type": "integer",
            "description": "Number of packages skipped"
          },
          "packages": {
            "type": "array",
            "description": "List of updated packages",
            "items": {
              "type": "object",
              "properties": {
                "name": {
                  "type": "string"
                },
                "oldVersion": {
                  "type": "string"
                },
                "newVersion": {
                  "type": "string"
                }
              }
            }
          }
        },
        "required": ["updated", "failed"]
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}",
          "checkOnly": "--check-only",
          "security": "--security-only"
        }
      }
    },

    "info": {
      "description": "Display detailed information about a specific package",
      "input": {
        "type": "object",
        "properties": {
          "package": {
            "type": "string",
            "description": "Package name"
          }
        },
        "required": ["package"]
      },
      "output": {
        "type": "object",
        "properties": {
          "name": {
            "type": "string"
          },
          "version": {
            "type": "string"
          },
          "description": {
            "type": "string"
          },
          "author": {
            "type": "string"
          },
          "dependencies": {
            "type": "array",
            "items": {
              "type": "string"
            }
          },
          "size": {
            "type": "integer"
          },
          "installDate": {
            "type": "string"
          },
          "permissions": {
            "type": "array",
            "items": {
              "type": "string"
            }
          }
        }
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}"
        }
      }
    }
  },

  "timeout": 600000,
  "events": ["stdout", "result"]
}
```

**调用示例**:

```typescript
import CLI from '@ohos.cli.manager';

// 1. 安装包
const installResult = await CLI.CLIManager.execTool('package-manager', {
  subcommand: 'install',
  package: '/path/to/app.hap',
  force: false,
  silent: true,
  dependencies: true
});
console.log('Install result:', installResult.data);
// 输出: { success: true, packageName: "com.example.app", version: "1.0.0", installPath: "/data/app/com.example.app" }

// 2. 列出已安装包
const listResult = await CLI.CLIManager.execTool('package-manager', {
  subcommand: 'list',
  verbose: true,
  filter: 'com.example',
  format: 'json',
  sort: 'name'
});
console.log('Packages:', listResult.data);

// 3. 卸载包
const removeResult = await CLI.CLIManager.execTool('package-manager', {
  subcommand: 'remove',
  package: 'com.example.app',
  purge: true,
  autoRemove: true
});
console.log('Remove result:', removeResult.data);

// 4. 更新包
const updateResult = await CLI.CLIManager.execTool('package-manager', {
  subcommand: 'update',
  package: 'com.example.app',
  checkOnly: false
});
console.log('Update result:', updateResult.data);

// 5. 查看包信息
const infoResult = await CLI.CLIManager.execTool('package-manager', {
  subcommand: 'info',
  package: 'com.example.app'
});
console.log('Package info:', infoResult.data);
```

**生成的命令行**:

```bash
# install 子命令
/usr/bin/pkg-mgr install /path/to/app.hap --with-deps --silent

# list 子命令
/usr/bin/pkg-mgr list -v --filter=com.example --format=json --sort=name

# remove 子命令
/usr/bin/pkg-mgr remove com.example.app --purge --auto-remove

# update 子命令
/usr/bin/pkg-mgr update com.example.app

# info 子命令
/usr/bin/pkg-mgr info com.example.app
```

#### 两种类型对比

| 特性 | 无子命令工具 | 有子命令工具 |
|------|-------------|-------------|
| **字段 `hasSubcommands`** | 不存在或 `false` | `true` |
| **字段 `subcommands`** | 不存在 | 必填，包含子命令集合 |
| **顶层 `input`** | 定义工具参数 | 不使用（参数在子命令中定义） |
| **顶层 `argumentMapping`** | 定义参数映射 | 不使用（映射在子命令中定义） |
| **顶层 `output`** | 定义输出格式 | 不使用（输出在子命令中定义） |
| **调用时 `subcommand`** | 不需要 | 必须指定子命令名称 |
| **生成的命令格式** | `executable [args]` | `executable <subcommand> [args]` |
| **典型示例** | 网络扫描、文件拷贝 | 包管理器、版本控制、Docker |

### 5.3 输入参数规范

CLI 工具使用 JSON Schema 定义输入参数。框架会自动验证参数。

#### JSON Schema 基础

**基本数据类型**

**String（字符串）**

```json
{
  "properties": {
    "target": {
      "type": "string",
      "description": "目标地址",
      "minLength": 1,
      "maxLength": 255,
      "pattern": "^[a-zA-Z0-9.-]+$",
      "format": "ipv4",
      "default": "127.0.0.1"
    }
  }
}
```

**支持的格式**:
- `ipv4` - IPv4 地址
- `ipv6` - IPv6 地址
- `hostname` - 主机名
- `email` - 邮箱地址
- `uri` - URI 地址
- `date` - 日期
- `date-time` - 日期时间
- `binary` - Base64 编码的二进制数据

**Number（数字）**

```json
{
  "properties": {
    "port": {
      "type": "number",
      "description": "端口号",
      "minimum": 1,
      "maximum": 65535,
      "default": 8080,
      "multipleOf": 1
    }
  }
}
```

**Integer（整数）**

```json
{
  "properties": {
    "count": {
      "type": "integer",
      "description": "重复次数",
      "minimum": 0,
      "maximum": 100,
      "default": 1
    }
  }
}
```

**Boolean（布尔值）**

```json
{
  "properties": {
    "verbose": {
      "type": "boolean",
      "description": "详细输出",
      "default": false
    }
  }
}
```

#### 复合数据类型

**Array（数组）**

```json
{
  "properties": {
    "ports": {
      "type": "array",
      "description": "端口列表",
      "items": {
        "type": "integer",
        "minimum": 1,
        "maximum": 65535
      },
      "minItems": 1,
      "maxItems": 100,
      "uniqueItems": true,
      "default": [80, 443]
    }
  }
}
```

**Object（对象）**

```json
{
  "properties": {
    "config": {
      "type": "object",
      "description": "配置对象",
      "properties": {
        "host": {
          "type": "string",
          "description": "主机地址"
        },
        "port": {
          "type": "integer",
          "description": "端口号"
        },
        "ssl": {
          "type": "boolean",
          "description": "启用SSL"
        }
      },
      "required": ["host", "port"],
      "additionalProperties": false
    }
  }
}
```

#### 特殊类型

**Enum（枚举）**

```json
{
  "properties": {
    "level": {
      "type": "string",
      "description": "日志级别",
      "enum": ["DEBUG", "INFO", "WARN", "ERROR"],
      "default": "INFO"
    }
  }
}
```

**Union（联合类型）**

```json
{
  "properties": {
    "input": {
      "description": "输入数据",
      "oneOf": [
        {"type": "string"},
        {"type": "number"},
        {"type": "boolean"}
      ]
    }
  }
}
```

#### 必填参数

使用 `required` 数组指定必填参数：

```json
{
  "properties": {
    "source": {"type": "string"},
    "destination": {"type": "string"},
    "verbose": {"type": "boolean"}
  },
  "required": ["source", "destination"]
}
```

#### 参数验证规则

| 规则 | 适用类型 | 说明 |
|------|----------|------|
| `minimum` | number, integer | 最小值 |
| `maximum` | number, integer | 最大值 |
| `exclusiveMinimum` | number, integer | 严格大于最小值 |
| `exclusiveMaximum` | number, integer | 严格小于最大值 |
| `minLength` | string | 最小长度 |
| `maxLength` | string | 最大长度 |
| `pattern` | string | 正则表达式 |
| `minItems` | array | 最少元素个数 |
| `maxItems` | array | 最多元素个数 |
| `uniqueItems` | array | 元素是否唯一 |
| `format` | string | 预定义格式 |

### 5.4 输出参数规范

#### 输出格式分类

CLI 工具有两种输出方式：

**1. 标准输出（stdout）**

适用于不需要结构化输出的工具，如文本处理、格式转换等。

**特点**:
- 输出任意文本格式
- 不需要 JSON Schema
- 框架直接传递给调用方

**示例**:
```
This is plain text output.
Line 2
Line 3
```

**2. 结构化输出（JSON）**

适用于需要结构化结果的工具，如扫描工具、查询工具等。

**特点**:
- 输出符合 JSON Schema 的结构化数据
- 支持事件输出
- 框架自动解析和验证

**格式**:
工具通过 stdout 输出 JSON 行来触发事件：

```json
{"event": "result", "data": {"results": [...], "summary": {...}}}
```

#### 事件类型

| 事件类型 | 触发方式 | 数据格式 | Schema 位置 |
|---------|---------|---------|------------|
| `stdout` | 输出非 JSON 行 | 原始文本 | 无需定义 |
| `stderr` | 输出到 stderr | 原始错误行 | 无需定义 |
| `result` | 输出 `{"event": "result", ...}` | 自定义 | `output` |
| `exit` | 进程退出 | 退出码 | 无需定义 |
| `error` | 执行失败 | 错误信息 | 无需定义 |

#### 输出 Schema 定义

**最终结果（output）**

```json
{
  "output": {
    "type": "object",
    "description": "扫描结果",
    "properties": {
      "results": {
        "type": "array",
        "description": "结果列表",
        "items": {
          "type": "object",
          "properties": {
            "port": {"type": "integer"},
            "status": {
              "type": "string",
              "enum": ["open", "closed", "filtered"]
            },
            "service": {"type": "string"}
          }
        }
      },
      "summary": {
        "type": "object",
        "description": "汇总信息",
        "properties": {
          "total": {"type": "integer"},
          "success": {"type": "integer"},
          "failed": {"type": "integer"}
        }
      }
    }
  }
}
```

#### 输出最佳实践

1. **使用结构化输出**
   ```json
   ✅ {"event": "result", "data": {"count": 10, "items": [...]}}
   ❌ {"count": 10}  // 缺少 event 字段
   ```

2. **错误输出到 stderr**
   ```bash
   echo "Error: Invalid input" >&2
   ```

3. **退出码规范**
   ```c
   #define EXIT_SUCCESS 0
   #define EXIT_ERROR 1
   #define EXIT_INVALID_ARGS 2
   #define EXIT_TIMEOUT 124
   ```

### 5.5 参数映射规范

#### 核心概念

**`argumentMapping` 的作用**：将抽象的 JSON 参数对象转换为具体的命令行参数字符串。

#### 问题场景示例

**输入（JS 层调用）**：
```json
{
  "target": "192.168.1.1",
  "ports": [80, 443],
  "verbose": true
}
```

**输出（命令行参数）**：
```bash
/usr/bin/net-scan --target=192.168.1.1 --port=80 --port=443 -v
```

这个转换过程就是由 `argumentMapping` 定义的。

#### 映射类型详解

CLI Manager 支持 5 种参数映射类型，每种类型适用于不同的命令行参数风格。

##### 1. flag 模式（最常用）⭐⭐⭐⭐⭐

基于模板字符串的映射，适用于大多数标准命令行工具（GNU 风格）。

**模板语法**：

| 语法 | 说明 | 示例 |
|------|------|------|
| `{value}` | 值占位符 | `--target={value}` → `--target=192.168.1.1` |
| `if_true` | 布尔 true 时的值 | `{ "if_true": "-v" }` |
| `if_false` | 布尔 false 时的值 | `{ "if_false": "" }` |

**详细示例：网络扫描工具**

```json
{
  "argumentMapping": {
    "type": "flag",
    "templates": {
      "target": "--target={value}",
      "ports": "--port={value}",
      "timeout": "--timeout={value}",
      "verbose": {
        "if_true": "-v",
        "if_false": ""
      },
      "force": {
        "if_true": "-f",
        "if_false": ""
      }
    }
  }
}
```

**JS 调用**：
```typescript
const result = await CLI.CLIManager.execTool('network-scanner', {
  target: "192.168.1.1",
  ports: [80, 443, 8080],
  timeout: 5000,
  verbose: true,
  force: false
});
```

**生成的命令行**：
```bash
/usr/bin/net-scan --target=192.168.1.1 --port=80 --port=443 --port=8080 --timeout=5000 -v
```

**处理规则**：

| 输入类型 | 处理方式 | 示例 |
|---------|---------|------|
| string | 直接替换占位符 | `"192.168.1.1"` → `--target=192.168.1.1` |
| number | 转为字符串后替换 | `5000` → `--timeout=5000` |
| boolean | 使用 if_true/if_false | `true` → `-v`, `false` → (空) |
| array | 遍历展开，每个元素生成一个参数 | `[80, 443]` → `--port=80 --port=443` |
| object | 不支持（使用 flattened 或 json-string 模式） | - |

##### 2. positional 模式（位置参数）⭐⭐

按参数顺序排列，适用于传统的位置参数命令。

**示例**：

```json
{
  "argumentMapping": {
    "type": "positional",
    "order": ["source", "destination", "recursive", "verbose"]
  }
}
```

**JS 调用**：
```typescript
await CLI.CLIManager.execTool('file-copy', {
  source: "/data/file.txt",
  destination: "/backup/file.txt",
  recursive: true,
  verbose: true
});
```

**生成的命令行**：
```bash
/usr/bin/cp-tool /data/file.txt /backup/file.txt true true
```

##### 3. flattened 模式（展平嵌套）⭐⭐⭐⭐

将嵌套对象展平为带前缀的参数。

**分隔符选项**：

| 分隔符 | 示例 | 说明 |
|--------|------|------|
| `.` (点号) | `app.name` | 推荐，符合常规习惯 |
| `-` (连字符) | `app-name` | 适合 CLI 风格 |
| `_` (下划线) | `app_name` | 编程风格 |

**示例**：

```json
{
  "argumentMapping": {
    "type": "flattened",
    "separator": ".",
    "templates": {
      "app.name": "--app-name={value}",
      "app.version": "--app-version={value}",
      "app.env.production": "--env-prod",
      "server.host": "--host={value}",
      "server.port": "--port={value}"
    }
  }
}
```

**JS 调用**：
```typescript
await CLI.CLIManager.execTool('app-deploy', {
  app: {
    name: "myapp",
    version: "1.0.0",
    env: {
      production: true
    }
  },
  server: {
    host: "192.168.1.100",
    port: 8080
  }
});
```

**生成的命令行**：
```bash
/usr/bin/app-deploy --app-name=myapp --app-version=1.0.0 --env-prod --host=192.168.1.100 --port=8080
```

##### 4. json-string 模式（JSON 字符串传递）⭐⭐⭐

将整个对象序列化为 JSON 字符串传递。

**示例**：

```json
{
  "argumentMapping": {
    "type": "json-string",
    "templates": {
      "config": "--config={value}"
    }
  }
}
```

**JS 调用**：
```typescript
await CLI.CLIManager.execTool('cloud-deploy', {
  config: {
    application: {
      name: "webapp",
      replicas: 3
    }
  }
});
```

**生成的命令行**：
```bash
/usr/bin/cloud-deploy --config='{"application":{"name":"webapp","replicas":3}}'
```

##### 5. javascript 模式（自定义映射器）⭐⭐

使用 JavaScript 代码自定义转换逻辑。

**示例**：

```json
{
  "argumentMapping": {
    "type": "javascript",
    "mapper": "function(args) { \
      let result = []; \
      if (args.inputs && args.inputs.length > 0) { \
        args.inputs.forEach(file => { \
          result.push('--input=' + file); \
        }); \
      } \
      if (args.output) { \
        result.push('--output=' + args.output); \
      } \
      return result; \
    }"
  }
}
```

#### 映射类型对比表

| 类型 | 适用场景 | 优点 | 缺点 | 配置复杂度 | 推荐指数 |
|------|---------|------|------|-----------|---------|
| **flag** | 标准命令行工具 | 简单直观，可读性强 | 需要为每个参数定义模板 | ⭐⭐ | ⭐⭐⭐⭐⭐ |
| **positional** | 传统位置参数 | 按顺序自然排列 | 缺乏灵活性，参数顺序固定 | ⭐ | ⭐⭐ |
| **flattened** | 嵌套配置对象 | 保持结构清晰 | 需要手动定义所有路径 | ⭐⭐⭐ | ⭐⭐⭐⭐ |
| **json-string** | 复杂配置 | 保留完整结构 | 命令行长度可能很长 | ⭐⭐ | ⭐⭐⭐ |
| **javascript** | 复杂自定义逻辑 | 最灵活 | 可维护性差，安全风险 | ⭐⭐⭐⭐⭐ | ⭐⭐ |

#### 选择建议

**使用 flag 模式（推荐）**

**适合场景**：
- 大多数标准 CLI 工具
- 参数类型多样（字符串、数字、布尔、数组）
- 需要清晰的参数名

**示例工具**：
- `net-scan`：网络扫描工具
- `log-monitor`：日志监控工具
- `file-copy`：文件拷贝工具

**使用 flattened 模式**

**适合场景**：
- 配置有层级结构
- 需要保持配置的可读性
- 参数命名有明确的层级关系

**示例工具**：
- `app-deploy`：应用部署（app.config、server.config）
- `system-config`：系统配置工具

**使用 json-string 模式**

**适合场景**：
- 配置非常复杂（深度嵌套）
- 工具原生支持 JSON 配置
- 云原生、容器化工具

**示例工具**：
- `cloud-deploy`：云部署（类似 kubectl）
- `k8s-deploy`：Kubernetes 部署

#### 最佳实践

1. **优先使用 flag 模式**
   - 覆盖 80% 的使用场景
   - 配置简单，易于维护
   - 可读性好

2. **避免过度使用 javascript 模式**
   - 安全风险
   - 可维护性差
   - 难以调试

3. **参数命名规范**
   - 使用小写字母和连字符：`--max-count`
   - 避免使用特殊字符
   - 保持命名的一致性

4. **复杂配置考虑文件方式**
   - 对于超长配置，考虑使用配置文件
   - 使用 `--config-file=/path/to/config.json`

5. **测试覆盖**
   - 为每种参数类型编写测试用例
   - 测试边界情况（空值、特殊字符等）

---

## 完整示例

### 示例 1: 网络扫描工具（无子命令）

#### 需求

创建一个简单的网络端口扫描工具，功能：
- 扫描指定主机的多个端口
- 显示端口开放状态
- 支持超时设置
- 支持详细输出模式

#### 工具描述文件

**文件**: `/etc/cli/tools/network-scanner.json`

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tool/v1",
  "name": "network-scanner",
  "version": "1.0.0",
  "description": "Network port scanning utility for diagnosing network connectivity",
  "executable": "/usr/bin/net-scan",
  "permissions": [
    "ohos.permission.INTERNET",
    "ohos.permission.GET_NETWORK_INFO"
  ],

  "input": {
    "type": "object",
    "properties": {
      "target": {
        "type": "string",
        "format": "ipv4",
        "description": "Target IP address to scan",
        "minLength": 7,
        "maxLength": 15,
        "example": "192.168.1.1"
      },
      "ports": {
        "type": "array",
        "description": "List of ports to scan",
        "items": {
          "type": "integer",
          "minimum": 1,
          "maximum": 65535,
          "description": "Port number"
        },
        "minItems": 1,
        "maxItems": 100,
        "example": [80, 443, 8080]
      },
      "timeout": {
        "type": "integer",
        "description": "Connection timeout in milliseconds",
        "minimum": 100,
        "maximum": 10000,
        "default": 1000
      },
      "verbose": {
        "type": "boolean",
        "description": "Enable verbose output",
        "default": false
      }
    },
    "required": ["target", "ports"]
  },

  "output": {
    "type": "object",
    "description": "Scan results",
    "properties": {
      "results": {
        "type": "array",
        "description": "Port scan results",
        "items": {
          "type": "object",
          "properties": {
            "port": {
              "type": "integer",
              "description": "Port number"
            },
            "status": {
              "type": "string",
              "enum": ["open", "closed", "filtered", "timeout"],
              "description": "Port status"
            },
            "responseTime": {
              "type": "integer",
              "description": "Response time in milliseconds"
            },
            "service": {
              "type": "string",
              "description": "Service name (if detected)"
            }
          },
          "required": ["port", "status"]
        }
      },
      "summary": {
        "type": "object",
        "description": "Scan summary",
        "properties": {
          "target": {
            "type": "string",
            "description": "Scanned target"
          },
          "totalPorts": {
            "type": "integer",
            "description": "Total ports scanned"
          },
          "openPorts": {
            "type": "integer",
            "description": "Number of open ports"
          },
          "scanTime": {
            "type": "integer",
            "description": "Total scan time in milliseconds"
          }
        },
        "required": ["target", "totalPorts", "openPorts", "scanTime"]
      }
    }
  },

  "argumentMapping": {
    "type": "flag",
    "templates": {
      "target": "--target={value}",
      "ports": "--port={value}",
      "timeout": "--timeout={value}",
      "verbose": {
        "if_true": "-v",
        "if_false": ""
      }
    }
  },

  "timeout": 60000,
  "events": ["stdout", "result"]
}
```

#### 调用方代码

**JavaScript/TypeScript**:

```typescript
import CLI from '@ohos.cli.manager';

async function scanNetwork() {
  try {
    const result = await CLI.CLIManager.execTool('network-scanner', {
      target: '192.168.1.1',
      ports: [80, 443, 8080, 22],
      timeout: 2000,
      verbose: true
    });

    console.log('Scan results:', result.data);

    // 输出示例:
    // {
    //   "results": [
    //     {"port": 80, "status": "open", "responseTime": 15, "service": "http"},
    //     {"port": 443, "status": "open", "responseTime": 20, "service": "https"},
    //     {"port": 8080, "status": "closed", "responseTime": 0},
    //     {"port": 22, "status": "open", "responseTime": 18, "service": "ssh"}
    //   ],
    //   "summary": {
    //     "target": "192.168.1.1",
    //     "totalPorts": 4,
    //     "openPorts": 3,
    //     "scanTime": 352
    //   }
    // }
  } catch (error) {
    console.error('Scan failed:', error);
  }
}
```

#### 生成的命令行

```bash
/usr/bin/net-scan --target=192.168.1.1 --port=80 --port=443 --port=8080 --port=22 --timeout=2000 -v
```

---

### 示例 2: 日志监控工具（异步输出）

#### 需求

创建一个日志监控工具，功能：
- 实时监控日志文件
- 支持模式匹配
- 异步输出匹配的日志行

#### 工具描述文件

**文件**: `/etc/cli/tools/log-monitor.json`

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tool/v1",
  "name": "log-monitor",
  "version": "1.0.0",
  "description": "Real-time log file monitoring tool",
  "executable": "/usr/bin/log-mon",
  "permissions": [
    "ohos.permission.READ_LOGS"
  ],

  "input": {
    "type": "object",
    "properties": {
      "file": {
        "type": "string",
        "description": "Log file path to monitor",
        "minLength": 1
      },
      "pattern": {
        "type": "string",
        "description": "Regex pattern to match",
        "default": ".*"
      },
      "lines": {
        "type": "integer",
        "description": "Number of lines to read (0 for unlimited)",
        "minimum": 0,
        "default": 100
      },
      "follow": {
        "type": "boolean",
        "description": "Follow file growth (like tail -f)",
        "default": false
      }
    },
    "required": ["file"]
  },

  "output": {
    "type": "object",
    "description": "Monitoring summary",
    "properties": {
      "totalLines": {
        "type": "integer",
        "description": "Total lines processed"
      },
      "matchedLines": {
        "type": "integer",
        "description": "Lines matching the pattern"
      },
      "duration": {
        "type": "integer",
        "description": "Monitoring duration in milliseconds"
      }
    }
  },

  "argumentMapping": {
    "type": "flag",
    "templates": {
      "file": "--file={value}",
      "pattern": "--pattern={value}",
      "lines": "--lines={value}",
      "follow": {
        "if_true": "-f",
        "if_false": ""
      }
    }
  },

  "timeout": 0,
  "events": ["stdout", "result"]
}
```

#### 调用方代码

**JavaScript/TypeScript**:

```typescript
import CLI from '@ohos.cli.manager';

async function monitorLogs() {
  const session = await CLI.CLIManager.execTool('log-monitor', {
    file: '/var/log/app.log',
    pattern: 'ERROR|WARN',
    lines: 1000,
    follow: false
  }, {
    mode: 'async',
    onEvent: (event) => {
      switch (event.type) {
        case 'stdout':
          // 实时输出匹配的日志行
          console.log('Log:', event.data);
          break;

        case 'result':
          // 最终结果
          console.log('Summary:', event.data);
          // 输出示例:
          // {
          //   "totalLines": 1000,
          //   "matchedLines": 45,
          //   "duration": 5230
          // }
          break;
      }
    }
  });

  // 等待监控完成
  await session.wait();
}
```

#### 生成的命令行

```bash
/usr/bin/log-mon --file=/var/log/app.log --pattern='ERROR|WARN' --lines=1000
```

---

### 示例 3: 应用部署工具（嵌套参数）

#### 需求

创建一个应用部署工具，功能：
- 支持应用配置（名称、版本、环境）
- 支持服务器配置（主机、端口）
- 使用 flattened 映射处理嵌套参数

#### 工具描述文件

**文件**: `/etc/cli/tools/app-deploy.json`

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tool/v1",
  "name": "app-deploy",
  "version": "1.0.0",
  "description": "Application deployment tool",
  "executable": "/usr/bin/app-deploy",
  "permissions": [
    "ohos.permission.DEPLOY_APP"
  ],

  "input": {
    "type": "object",
    "properties": {
      "app": {
        "type": "object",
        "description": "Application configuration",
        "properties": {
          "name": {
            "type": "string",
            "description": "Application name",
            "minLength": 1
          },
          "version": {
            "type": "string",
            "description": "Application version",
            "pattern": "^\\d+\\.\\d+\\.\\d+$"
          },
          "env": {
            "type": "object",
            "description": "Environment variables",
            "properties": {
              "production": {
                "type": "boolean",
                "description": "Production environment"
              },
              "debug": {
                "type": "boolean",
                "description": "Debug mode"
              }
            }
          }
        },
        "required": ["name", "version"]
      },
      "server": {
        "type": "object",
        "description": "Server configuration",
        "properties": {
          "host": {
            "type": "string",
            "description": "Server host",
            "format": "hostname"
          },
          "port": {
            "type": "integer",
            "description": "Server port",
            "minimum": 1,
            "maximum": 65535,
            "default": 8080
          },
          "ssl": {
            "type": "boolean",
            "description": "Use SSL/TLS",
            "default": false
          }
        },
        "required": ["host"]
      }
    },
    "required": ["app", "server"]
  },

  "output": {
    "type": "object",
    "properties": {
      "success": {
        "type": "boolean",
        "description": "Deployment success status"
      },
      "deploymentId": {
        "type": "string",
        "description": "Unique deployment ID"
      },
      "message": {
        "type": "string",
        "description": "Deployment message"
      }
    }
  },

  "argumentMapping": {
    "type": "flattened",
    "separator": ".",
    "templates": {
      "app.name": "--app-name={value}",
      "app.version": "--app-version={value}",
      "app.env.production": "--env-prod",
      "app.env.debug": "--env-debug",
      "server.host": "--host={value}",
      "server.port": "--port={value}",
      "server.ssl": "--ssl"
    }
  },

  "timeout": 300000,
  "events": ["result"]
}
```

#### 调用方代码

**JavaScript/TypeScript**:

```typescript
import CLI from '@ohos.cli.manager';

async function deployApp() {
  try {
    const result = await CLI.CLIManager.execTool('app-deploy', {
      app: {
        name: 'my-webapp',
        version: '1.2.3',
        env: {
          production: true,
          debug: false
        }
      },
      server: {
        host: 'prod-server.example.com',
        port: 8443,
        ssl: true
      }
    });

    console.log('Deployment result:', result.data);

    // 输出示例:
    // {
    //   "success": true,
    //   "deploymentId": "deploy-20250411-123456",
    //   "message": "Application deployed successfully"
    // }
  } catch (error) {
    console.error('Deployment failed:', error);
  }
}
```

#### 生成的命令行

```bash
/usr/bin/app-deploy --app-name=my-webapp --app-version=1.2.3 --env-prod --host=prod-server.example.com --port=8443 --ssl
```

---

### 示例 4: 包管理工具（子命令）

完整的包管理工具示例请参考第 5.2 节"有子命令工具"部分。

---

## 开发最佳实践

### 7.1 编码规范

#### 命名规范

1. **文件命名**
   - 使用小写字母和连字符
   - 源文件：`scanner.cpp`、`parser.cpp`
   - 头文件：`scanner.h`、`parser.h`

2. **函数命名**
   - 使用驼峰命名法（C++）或下划线命名法（C）
   ```cpp
   // C++
   void scanNetwork() { }
   int parseResult() { }

   // C
   void scan_network(void);
   int parse_result(void);
   ```

3. **常量命名**
   ```cpp
   // 使用全大写+下划线
   const int MAX_PORT_COUNT = 100;
   const char* DEFAULT_CONFIG_PATH = "/etc/config.json";
   ```

#### 注释要求

1. **文件头注释**
   ```cpp
   /**
    * @file scanner.cpp
    * @brief Network port scanning implementation
    * @author Your Name
    * @date 2025-04-11
    */
   ```

2. **函数注释**
   ```cpp
   /**
    * @brief Scan network ports on target host
    * @param target Target IP address
    * @param ports List of ports to scan
    * @param timeout Connection timeout in milliseconds
    * @return Scan results in JSON format
    */
   json scanPorts(const string& target, const vector<int>& ports, int timeout);
   ```

3. **关键逻辑注释**
   ```cpp
   // Validate port range before scanning
   if (port < 1 || port > 65535) {
       return json::object();
   }
   ```

#### 错误处理

1. **使用标准退出码**
   ```c
   #define EXIT_SUCCESS 0
   #define EXIT_ERROR 1
   #define EXIT_INVALID_ARGS 2
   #define EXIT_TIMEOUT 124
   ```

2. **输出错误到 stderr**
   ```cpp
   std::cerr << "Error: Invalid target address" << std::endl;
   return EXIT_INVALID_ARGS;
   ```

3. **提供清晰的错误信息**
   ```cpp
   // ❌ 不好
   cerr << "Error" << endl;

   // ✅ 好
   cerr << "Error: Invalid target '192.168.1.999'. Expected valid IPv4 address." << endl;
   ```

#### 资源管理

1. **使用 RAII（C++）**
   ```cpp
   // 使用智能指针自动管理内存
   auto scanner = std::make_unique<NetworkScanner>();

   // 使用 vector 管理动态数组
   std::vector<int> ports = {80, 443, 8080};
   ```

2. **及时释放资源**
   ```cpp
   FILE* file = fopen("/tmp/data", "r");
   if (!file) {
       return;
   }
   // 使用文件
   fclose(file);  // 及时关闭
   ```

### 7.2 测试策略

#### 单元测试

1. **测试框架选择**
   - C++: Google Test (gtest)
   - C: Check Framework

2. **测试组织**
   ```
   tests/
   ├── test_scanner.cpp
   ├── test_parser.cpp
   └── test_utils.cpp
   ```

3. **测试示例**
   ```cpp
   #include <gtest/gtest.h>

   TEST(ScannerTest, ValidPortScan) {
       Scanner scanner;
       auto result = scanner.scan("127.0.0.1", {80});
       EXPECT_EQ(result.size(), 1);
       EXPECT_TRUE(result[0].isOpen());
   }

   TEST(ScannerTest, InvalidPortRange) {
       Scanner scanner;
       auto result = scanner.scan("127.0.0.1", {99999});
       EXPECT_TRUE(result.empty());
   }
   ```

#### 集成测试

1. **测试工具调用**
   ```bash
   # 测试基本功能
   /usr/bin/net-scan --target=127.0.0.1 --port=80

   # 测试参数验证
   /usr/bin/net-scan --target=invalid --port=80
   # 应该返回错误
   ```

2. **测试输出格式**
   ```bash
   # 验证 JSON 输出
   /usr/bin/net-scan --target=127.0.0.1 --port=80 | jq .
   ```

#### 参数映射测试

1. **测试 flag 模式**
   ```bash
   # 测试布尔参数
   /usr/bin/tool --verbose
   /usr/bin/tool --no-verbose
   ```

2. **测试数组参数**
   ```bash
   # 测试数组展开
   /usr/bin/tool --port=80 --port=443 --port=8080
   ```

3. **测试嵌套参数（flattened 模式）**
   ```bash
   # 测试路径展开
   /usr/bin/tool --app-name=myapp --server-host=localhost
   ```

### 7.3 安全实践

#### 输入验证

1. **验证参数类型**
   ```cpp
   // 验证端口号
   if (port < 1 || port > 65535) {
       std::cerr << "Invalid port number: " << port << std::endl;
       return EXIT_INVALID_ARGS;
   }
   ```

2. **验证字符串格式**
   ```cpp
   // 验证 IP 地址格式
   if (!isValidIPv4(target)) {
       std::cerr << "Invalid IPv4 address: " << target << std::endl;
       return EXIT_INVALID_ARGS;
   }
   ```

3. **防止缓冲区溢出**
   ```cpp
   // 使用安全的字符串函数
   strncpy(buffer, input, sizeof(buffer) - 1);
   buffer[sizeof(buffer) - 1] = '\0';
   ```

#### 权限检查

1. **最小权限原则**
   - 只申请必要的权限
   - 使用普通用户运行
   - 避免 setuid/setgid

2. **文件权限**
   ```bash
   # 可执行文件权限
   chmod 0755 /usr/bin/tool

   # 描述文件权限
   chmod 0644 /etc/cli/tools/tool.json
   ```

#### 防止代码注入

1. **避免 shell 注入**
   ```cpp
   // ❌ 危险：直接拼接命令
   std::string cmd = "ping " + target;
   system(cmd.c_str());

   // ✅ 安全：使用参数化方式
   execl("/bin/ping", "ping", target.c_str(), nullptr);
   ```

2. **验证 JSON 输入**
   ```cpp
   // 验证 JSON 结构
   try {
       json input = json::parse(inputString);
       if (!input.contains("required_field")) {
           throw std::runtime_error("Missing required field");
       }
   } catch (const json::exception& e) {
       std::cerr << "Invalid JSON: " << e.what() << std::endl;
       return EXIT_INVALID_ARGS;
   }
   ```

### 7.4 性能优化建议

#### 启动时间优化

1. **延迟加载**
   ```cpp
   // 延迟初始化大型资源
   class Scanner {
       void init() {
           if (!initialized) {
               // 初始化资源
               initialized = true;
           }
       }
   };
   ```

2. **减少依赖**
   ```bash
   # 静态链接减少依赖
   g++ -static-libgcc -static-libstdc++ -o tool main.cpp
   ```

#### 内存使用优化

1. **使用适当的数据结构**
   ```cpp
   // 小数组使用 std::vector
   std::vector<int> ports = {80, 443};

   // 大数据使用流式处理
   while (std::getline(file, line)) {
       processLine(line);
   }
   ```

2. **及时释放内存**
   ```cpp
   {
       std::vector<int> data = loadData();
       processData(data);
   }  // 自动释放内存
   ```

#### 并发处理建议

1. **使用线程池**
   ```cpp
   #include <thread>
   #include <vector>

   std::vector<std::thread> threads;
   for (int port : ports) {
       threads.emplace_back(scanPort, target, port);
   }
   for (auto& t : threads) {
       t.join();
   }
   ```

2. **避免竞态条件**
   ```cpp
   #include <mutex>

   std::mutex resultMutex;
   std::vector<ScanResult> results;

   void scanPort(int port) {
       ScanResult result = doScan(port);
       std::lock_guard<std::mutex> lock(resultMutex);
       results.push_back(result);
   }
   ```

---

## 常见问题与调试

### 8.1 常见错误

#### 错误 1: 参数映射错误

**问题**: 工具接收到错误的参数格式

**原因**: `argumentMapping` 配置错误

**解决**:
- 检查映射类型是否正确
- 验证模板语法
- 确认参数名称匹配

#### 错误 2: JSON Schema 验证失败

**问题**: 参数被框架拒绝

**原因**: 输入参数不符合 Schema 定义

**解决**:
- 检查必填参数是否提供
- 验证参数类型是否正确
- 确认参数值在允许范围内

#### 错误 3: 工具未注册

**问题**: 调用工具时返回 "Tool not found"

**原因**:
- 描述文件未安装
- 工具名称不匹配
- 注册表未更新

**解决**:
```bash
# 1. 检查描述文件
ls /etc/cli/tools/<tool-name>.json

# 2. 检查可执行文件
ls /usr/bin/<executable>

# 3. 重新加载注册表
killall -HUP climgr
```

### 8.2 调试技巧

#### 本地测试工具

```bash
# 直接运行工具测试
/usr/bin/net-scan --target=127.0.0.1 --port=80

# 查看工具输出
/usr/bin/net-scan --target=127.0.0.1 --port=80 | jq .
```

#### 查看工具日志

```bash
# 查看服务日志
hilog -x | grep CLIManager

# 查看工具执行日志
hilog -x | grep net-scan
```

#### 验证描述文件

```bash
# 使用 jq 验证 JSON 格式
jq . /etc/cli/tools/network-scanner.json

# 使用 ajv 验证 JSON Schema
ajv validate -s cli-tool-schema.json -d /etc/cli/tools/network-scanner.json
```

### 8.3 FAQ

**Q1: 如何快速验证描述文件格式？**

```bash
# 使用 jq 验证 JSON 格式
jq . /etc/cli/tools/network-scanner.json

# 如果输出内容，说明格式正确
# 如果报错，说明格式有问题
```

**Q2: 工具调用后没有返回结果怎么办？**

1. 检查工具是否正常退出
   ```bash
   echo $?  # 查看退出码
   ```

2. 检查是否有输出
   ```bash
   /usr/bin/tool --arg=value 2>&1
   ```

3. 检查是否有超时
   ```bash
   # 增加超时时间
   timeout 120 /usr/bin/tool --arg=value
   ```

**Q3: 如何测试异步输出的工具？**

```bash
# 使用后台任务
/usr/bin/log-mon --file=/var/log/app.log > output.txt 2>&1 &

# 查看输出
tail -f output.txt
```

**Q4: 参数映射不生效怎么办？**

1. 检查映射类型是否正确
2. 验证模板语法
3. 查看生成的命令行
   ```bash
   # 在描述文件中添加调试输出
   "argumentMapping": {
     "type": "flag",
     "debug": true,  // 启用调试
     "templates": {...}
   }
   ```

**Q5: 如何处理大量参数的映射？**

考虑使用 `json-string` 或 `javascript` 模式：
```json
{
  "argumentMapping": {
    "type": "json-string",
    "templates": {
      "config": "--config={value}"
    }
  }
}
```

---

**文档版本**: v1.2
**最后更新**: 2025-04-11
**维护者**: CLI 工具管理框架团队
