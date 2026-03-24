# CLI 工具开发规范

## 文档信息

**版本**: v2.0
**创建日期**: 2025-04-11
**最后更新**: 2025-04-11
**适用框架版本**: CLI Manager v2.8
**目标读者**: CLI 工具开发者

---

## 目录

### 第一部分：入门指南
1. [概述](#1-概述)
2. [快速开始](#2-快速开始)
   - [2.1 什么是 CLI 工具](#21-什么是-cli-工具)
   - [2.2 为什么需要 CLI 框架](#22-为什么需要-cli-框架)
   - [2.3 第一个工具：5分钟上手](#23-第一个工具5分钟上手)
   - [2.4 开发流程全景图](#24-开发流程全景图)

### 第二部分：开发准备
3. [CLI 工具代码目录结构](#3-cli-工具代码目录结构)
   - [3.1 标准目录结构](#31-标准目录结构)
   - [3.2 文件组织方式](#32-文件组织方式)
   - [3.3 BUILD.gn 配置](#33-buildgn-配置)
   - [3.4 安装与注册](#34-安装与注册)
4. [文件命名规范](#4-文件命名规范)
   - [4.1 工具命名规则](#41-工具命名规则)
   - [4.2 描述文件命名](#42-描述文件命名)
   - [4.3 可执行文件命名](#43-可执行文件命名)

### 第三部分：核心配置
5. [CLI 工具描述文件规范](#5-cli-工具描述文件规范)
   - [5.1 基本结构](#51-基本结构)
   - [5.2 字段详解](#52-字段详解)
   - [5.3 工具类型分类](#53-工具类型分类)
   - [5.4 输入参数设计](#54-输入参数设计)
   - [5.5 输出参数设计](#55-输出参数设计)
   - [5.6 参数映射设计](#56-参数映射设计)

### 第四部分：实践指南
6. [完整示例](#6-完整示例)
   - [6.1 示例1：网络扫描工具（无子命令）](#61-示例1网络扫描工具无子命令)
   - [6.2 示例2：日志监控工具（异步输出）](#62-示例2日志监控工具异步输出)
   - [6.3 示例3：应用部署工具（嵌套参数）](#63-示例3应用部署工具嵌套参数)
   - [6.4 示例4：包管理工具（子命令）](#64-示例4包管理工具子命令)
7. [开发最佳实践](#7-开发最佳实践)
   - [7.1 编码规范](#71-编码规范)
   - [7.2 测试策略](#72-测试策略)
   - [7.3 安全实践](#73-安全实践)
   - [7.4 性能优化](#74-性能优化)
8. [工具开发路线图](#8-工具开发路线图)
   - [8.1 MVP 版本开发](#81-mvp-版本开发)
   - [8.2 功能扩展](#82-功能扩展)
   - [8.3 持续维护](#83-持续维护)

### 第五部分：参考与调试
9. [常见问题与调试](#9-常见问题与调试)
   - [9.1 常见错误及解决方案](#91-常见错误及解决方案)
   - [9.2 调试技巧](#92-调试技巧)
   - [9.3 日志分析](#93-日志分析)
   - [9.4 FAQ](#94-faq)
10. [参考资料](#10-参考资料)
    - [10.1 相关文档](#101-相关文档)
    - [10.2 工具与资源](#102-工具与资源)
    - [10.3 术语表](#103-术语表)
11. [版本历史](#11-版本历史)
    - [11.1 更新记录](#111-更新记录)
    - [11.2 计划中的功能](#112-计划中的功能)

---

# 第一部分：入门指南

## 1. 概述

### 1.1 目的

本规范旨在为 CLI 工具开发者提供完整的开发指南，确保工具能够正确集成到 OpenHarmony CLI Manager 框架中。

### 1.2 核心原则

| 原则 | 说明 |
|------|------|
| **声明式配置** | 所有工具能力通过 JSON 描述文件声明 |
| **标准化接口** | 遵循统一的参数传递和事件输出规范 |
| **安全优先** | 工具必须通过安全验证才能注册 |
| **可测试性** | 工具应该支持独立运行和测试 |

### 1.3 什么是 CLI 工具

CLI（Command Line Interface）工具是命令行界面工具，用于通过命令行与系统进行交互。在 OpenHarmony 系统中，CLI 工具是指可以被应用程序通过统一框架调用的系统命令行工具。

**典型应用场景**：
- 网络诊断（ping、traceroute、netstat）
- 系统管理（文件操作、进程管理）
- 日志分析（日志查询、监控、过滤）
- 应用管理（安装、卸载、更新）

### 1.4 为什么需要 CLI 框架

传统的命令行工具调用方式存在以下问题：

| 问题 | 传统方式 | CLI 框架解决方案 |
|------|---------|----------------|
| **参数验证** | 工具自行验证 | 框架统一验证，减少重复代码 |
| **权限管理** | 需要手动管理 | 框架自动检查权限 |
| **错误处理** | 各工具不一致 | 统一的错误处理机制 |
| **安全性** | 参数注入风险 | 框架级别的安全防护 |
| **开发效率** | 重复编写样板代码 | 声明式配置，快速开发 |

---

## 2. 快速开始

欢迎使用 CLI 工具管理框架！本章节帮助你在 **15 分钟内**创建第一个 CLI 工具。

### 2.1 什么是 CLI 工具

CLI 工具是可以通过命令行调用的可执行程序。在本框架中，CLI 工具具有以下特点：

- **声明式配置**：通过 JSON 文件描述工具的能力
- **自动参数转换**：框架自动将 JSON 参数转换为命令行参数
- **标准化输出**：支持结构化的 JSON 输出和实时事件流
- **安全验证**：框架级别的权限检查和参数验证

**典型工作流程**：

```
应用程序调用
    ↓
CLI Manager 框架
    ↓
参数验证和转换
    ↓
执行 CLI 工具
    ↓
返回结果给应用
```

### 2.2 为什么需要 CLI 框架

**传统方式的问题**：

```typescript
// ❌ 传统方式：应用程序直接调用命令行
const command = `net-scan --target=${target} --port=${port} --timeout=${timeout}`;
exec(command, (error, stdout, stderr) => {
  // 需要手动解析输出
  // 需要处理参数注入风险
  // 需要自己管理权限
});
```

**CLI 框架的优势**：

```typescript
// ✅ 使用 CLI 框架：类型安全、自动验证
const result = await CLI.CLIManager.execTool('network-scanner', {
  target: '192.168.1.1',
  ports: [80, 443],
  timeout: 5000
});
// 框架自动：
// - 验证参数类型和范围
// - 转换为命令行参数
// - 解析结构化输出
// - 检查权限
```

### 2.3 第一个工具：5分钟上手

让我们创建一个最简单的 CLI 工具——**Hello World 工具**，它只输出 "Hello, World!"。

#### 步骤 1: 创建目录结构（2分钟）

```bash
# 创建工具目录
mkdir -p /usr/local/src/cli-tools/hello-world/{src,include,tests}
cd /usr/local/src/cli-tools/hello-world
```

#### 步骤 2: 编写工具代码（3分钟）

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

#### 步骤 3: 创建描述文件（5分钟）

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

#### 步骤 4: 编译和安装（2分钟）

```bash
# 编译工具
g++ -std=c++17 -o hello-world src/main.cpp -I/usr/include/nlohmann

# 安装可执行文件
sudo cp hello-world /usr/bin/
sudo chmod +x /usr/bin/hello-world

# 安装描述文件
sudo mkdir -p /etc/cli/tools
sudo cp hello-world.json /etc/cli/tools/
sudo chmod 644 /etc/cli/tools/hello-world.json
```

#### 步骤 5: 测试工具（3分钟）

```bash
# 直接运行测试
/usr/bin/hello-world
# 预期输出: {"event":"result","data":{"message":"Hello, World!","timestamp":1712834400}}

# 验证 JSON 格式
/usr/bin/hello-world | jq .
```

#### 步骤 6: 从应用调用（最后一步）

```typescript
import CLI from '@ohos.cli.manager';

async function sayHello() {
  try {
    const result = await CLI.CLIManager.execTool('hello-world', {});
    console.log(result.data.message);
    // 输出: Hello, World!
    console.log('Timestamp:', new Date(result.data.timestamp * 1000));
    // 输出: Timestamp: 2025-04-11T10:00:00.000Z
  } catch (error) {
    console.error('Failed to execute tool:', error);
  }
}
```

🎉 **恭喜！你已经创建了第一个 CLI 工具！**

### 2.4 开发流程全景图

创建一个完整的 CLI 工具需要经过以下步骤：

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

阶段 5: 维护迭代
├─ 5.1 监控运行状态
├─ 5.2 收集用户反馈
└─ 5.3 版本更新
```

### 2.5 下一步学习路径

现在你已经完成了第一个工具，接下来可以：

1. **深入学习参数设计** → 查看第 5.4 节学习输入参数设计
2. **了解参数映射** → 查看第 5.6 节学习各种映射模式
3. **查看完整示例** → 查看第 6 章参考实际项目示例
4. **学习最佳实践** → 查看第 7 章了解开发规范和技巧

---

# 第二部分：开发准备

## 3. CLI 工具代码目录结构

### 3.1 标准目录结构

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

### 3.2 文件组织方式

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

### 3.3 BUILD.gn 配置

BUILD.gn 是 OpenHarmony 构建系统的配置文件。

**完整示例**：

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

**字段说明**：

| 字段 | 说明 |
|------|------|
| `sources` | 源文件列表 |
| `include_dirs` | 头文件搜索路径 |
| `deps` | 内部依赖 |
| `external_deps` | 外部依赖 |
| `install_images` | 安装镜像 |
| `part_name` | 所属部件 |
| `subsystem_name` | 所属子系统 |

### 3.4 安装与注册

#### 安装可执行文件

```bash
# 编译工具
./scripts/build.sh

# 安装到系统目录
sudo install -m 755 net-scan /usr/bin/net-scan

# 设置所有者为 root
sudo chown root:root /usr/bin/net-scan

# 确保没有 setuid/setgid
sudo chmod 0755 /usr/bin/net-scan
```

#### 安装描述文件

```bash
# 创建目录
sudo mkdir -p /etc/cli/tools

# 安装描述文件
sudo install -m 644 net-scan.json /etc/cli/tools/network-scanner.json

# 设置所有者为 root
sudo chown root:root /etc/cli/tools/network-scanner.json

# 验证 JSON 格式
jq . /etc/cli/tools/network-scanner.json
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

#### 重新加载服务

```bash
# 重新加载 CLI Manager
sudo systemctl reload cli-manager

# 或者发送 HUP 信号
sudo killall -HUP cli-manager
```

#### 验证注册

```bash
# 查看已注册工具
cli-tool list

# 测试工具调用
cli-tool exec network-scanner --target=127.0.0.1 --port=80
```

---

## 4. 文件命名规范

### 4.1 工具命名规则

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

### 4.2 描述文件命名

**格式**：`<tool-name>.json`

**位置**：`/etc/cli/tools/`

**示例**：
```
/etc/cli/tools/network-scanner.json
/etc/cli/tools/log-monitor.json
/etc/cli/tools/file-copy.json
```

**命名规则**：
- 必须与工具名称一致
- 使用小写字母和连字符
- 必须以 `.json` 扩展名结尾

### 4.3 可执行文件命名

**位置**：`/usr/bin/`

**命名规则**：
- 优先使用工具名称
- 可以使用更短的命令名
- 必须在描述文件的 `executable` 字段中明确指定

**示例**：

```
# 情况 1: 使用完整工具名
工具名称: network-scanner
可执行文件: /usr/bin/network-scanner

# 情况 2: 使用简化名称
工具名称: network-scanner
可执行文件: /usr/bin/net-scan

描述文件中指定:
{
  "name": "network-scanner",
  "executable": "/usr/bin/net-scan"
}
```

**命名建议**：
- 保持简洁但易于理解
- 避免与系统命令冲突
- 使用常用缩写（如 `net`、`pkg`、`cfg`）

---

# 第三部分：核心配置

## 5. CLI 工具描述文件规范

### 5.1 基本结构

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

### 5.2 字段详解

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

### 5.3 工具类型分类

CLI 工具根据功能复杂度分为两类：**无子命令工具**和**有子命令工具**。

#### 5.3.1 无子命令工具

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
  "description": "Network port scanning utility",
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
        "description": "Target IP address"
      },
      "ports": {
        "type": "array",
        "items": {
          "type": "integer",
          "minimum": 1,
          "maximum": 65535
        }
      }
    },
    "required": ["target", "ports"]
  },
  "output": {
    "type": "object",
    "properties": {
      "results": {
        "type": "array",
        "items": {
          "type": "object",
          "properties": {
            "port": {"type": "integer"},
            "status": {"type": "string"}
          }
        }
      }
    }
  },
  "argumentMapping": {
    "type": "flag",
    "templates": {
      "target": "--target={value}",
      "ports": "--port={value}"
    }
  },
  "timeout": 60000,
  "events": ["result"]
}
```

#### 5.3.2 有子命令工具

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
  "description": "Package management tool",
  "executable": "/usr/bin/pkg-mgr",
  "hasSubcommands": true,
  "subcommands": {
    "install": {
      "description": "Install a package",
      "input": {
        "type": "object",
        "properties": {
          "package": {"type": "string"},
          "force": {"type": "boolean"}
        },
        "required": ["package"]
      },
      "output": {
        "type": "object",
        "properties": {
          "success": {"type": "boolean"},
          "packageName": {"type": "string"}
        }
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}",
          "force": "--force"
        }
      }
    },
    "remove": {
      "description": "Remove a package",
      "input": {
        "type": "object",
        "properties": {
          "package": {"type": "string"}
        },
        "required": ["package"]
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
  "events": ["result"]
}
```

### 5.4 输入参数设计

CLI 工具使用 JSON Schema 定义输入参数。框架会自动验证参数。

#### 5.4.1 基本类型

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

#### 5.4.2 复合类型

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
        }
      },
      "required": ["host", "port"],
      "additionalProperties": false
    }
  }
}
```

#### 5.4.3 验证规则

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

#### 5.4.4 设计建议

1. **提供合理的默认值**：减少用户输入负担
2. **使用枚举限制选择**：对于有限选项使用 `enum`
3. **添加描述信息**：帮助用户理解参数用途
4. **设置适当的验证规则**：确保数据有效性
5. **保持参数扁平**：避免过度嵌套

### 5.5 输出参数设计

#### 5.5.1 输出格式选择

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

#### 5.5.2 事件机制

| 事件类型 | 触发方式 | 数据格式 | Schema 位置 |
|---------|---------|---------|------------|
| `stdout` | 输出非 JSON 行 | 原始文本 | 无需定义 |
| `stderr` | 输出到 stderr | 原始错误行 | 无需定义 |
| `result` | 输出 `{"event": "result", ...}` | 自定义 | `output` |
| `exit` | 进程退出 | 退出码 | 无需定义 |
| `error` | 执行失败 | 错误信息 | 无需定义 |

#### 5.5.3 Schema 定义

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

### 5.6 参数映射设计

**`argumentMapping` 的作用**：将抽象的 JSON 参数对象转换为具体的命令行参数字符串。

CLI Manager 支持 5 种参数映射类型：

#### 5.6.1 flag 模式（最常用）⭐⭐⭐⭐⭐

基于模板字符串的映射，适用于大多数标准命令行工具（GNU 风格）。

**模板语法**：

| 语法 | 说明 | 示例 |
|------|------|------|
| `{value}` | 值占位符 | `--target={value}` → `--target=192.168.1.1` |
| `if_true` | 布尔 true 时的值 | `{ "if_true": "-v" }` |
| `if_false` | 布尔 false 时的值 | `{ "if_false": "" }` |

**示例**：

```json
{
  "argumentMapping": {
    "type": "flag",
    "templates": {
      "target": "--target={value}",
      "ports": "--port={value}",
      "verbose": {
        "if_true": "-v",
        "if_false": ""
      }
    }
  }
}
```

#### 5.6.2 positional 模式（位置参数）⭐⭐

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

#### 5.6.3 flattened 模式（展平嵌套）⭐⭐⭐⭐

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
      "server.host": "--host={value}"
    }
  }
}
```

#### 5.6.4 json-string 模式（JSON 字符串传递）⭐⭐⭐

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

#### 5.6.5 javascript 模式（自定义映射器）⭐⭐

使用 JavaScript 代码自定义转换逻辑。

**示例**：

```json
{
  "argumentMapping": {
    "type": "javascript",
    "mapper": "function(args) { return ['--arg1', args.value]; }"
  }
}
```

#### 5.6.6 映射类型对比

| 类型 | 适用场景 | 优点 | 缺点 | 推荐指数 |
|------|---------|------|------|---------|
| **flag** | 标准命令行工具 | 简单直观 | 需要定义模板 | ⭐⭐⭐⭐⭐ |
| **positional** | 位置参数 | 按顺序排列 | 缺乏灵活性 | ⭐⭐ |
| **flattened** | 嵌套配置 | 保持结构 | 需手动定义路径 | ⭐⭐⭐⭐ |
| **json-string** | 复杂配置 | 保留结构 | 命令行过长 | ⭐⭐⭐ |
| **javascript** | 复杂逻辑 | 最灵活 | 安全风险 | ⭐⭐ |

#### 5.6.7 选择建议

1. **优先使用 flag 模式**：覆盖 80% 的使用场景
2. **嵌套配置使用 flattened**：保持配置的可读性
3. **复杂配置使用 json-string**：深度嵌套的场景
4. **避免过度使用 javascript**：存在安全风险

---

# 第四部分：实践指南

## 6. 完整示例

### 6.1 示例1：网络扫描工具（无子命令）

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

### 6.2 示例2：日志监控工具（异步输出）

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

### 6.3 示例3：应用部署工具（嵌套参数）

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

### 6.4 示例4：包管理工具（子命令）

#### 需求

创建一个包管理工具，支持多个子命令：
- `install` - 安装包
- `remove` - 卸载包
- `list` - 列出已安装包
- `update` - 更新包

#### 工具描述文件

**文件**: `/etc/cli/tools/package-manager.json`

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

#### 调用方代码

**JavaScript/TypeScript**:

```typescript
import CLI from '@ohos.cli.manager';

// 1. 安装包
async function installPackage() {
  const result = await CLI.CLIManager.execTool('package-manager', {
    subcommand: 'install',
    package: '/path/to/app.hap',
    force: false,
    silent: false,
    dependencies: true
  });

  console.log('Install result:', result.data);
  // 输出:
  // {
  //   "success": true,
  //   "packageName": "com.example.app",
  //   "version": "1.0.0",
  //   "installPath": "/data/app/com.example.app"
  // }
}

// 2. 列出已安装包
async function listPackages() {
  const result = await CLI.CLIManager.execTool('package-manager', {
    subcommand: 'list',
    verbose: true,
    filter: 'com.example',
    format: 'json'
  });

  console.log('Packages:', result.data);
  // 输出:
  // {
  //   "packages": [
  //     {"name": "com.example.app", "version": "1.0.0", "size": 1024000, "installDate": "2025-04-11"},
  //     {"name": "com.example.service", "version": "2.1.0", "size": 512000, "installDate": "2025-04-10"}
  //   ],
  //   "total": 2,
  //   "totalSize": 1536000
  // }
}

// 3. 卸载包
async function removePackage() {
  const result = await CLI.CLIManager.execTool('package-manager', {
    subcommand: 'remove',
    package: 'com.example.app',
    purge: true,
    autoRemove: true
  });

  console.log('Remove result:', result.data);
  // 输出:
  // {
  //   "success": true,
  //   "packageName": "com.example.app",
  //   "removedSize": 1024000
  // }
}

// 4. 更新包
async function updatePackage() {
  const result = await CLI.CLIManager.execTool('package-manager', {
    subcommand: 'update',
    package: 'com.example.app',
    checkOnly: false
  });

  console.log('Update result:', result.data);
  // 输出:
  // {
  //   "updated": 1,
  //   "failed": 0,
  //   "skipped": 0,
  //   "packages": [
  //     {"name": "com.example.app", "oldVersion": "1.0.0", "newVersion": "1.1.0"}
  //   ]
  // }
}

// 5. 查看包信息
async function getPackageInfo() {
  const result = await CLI.CLIManager.execTool('package-manager', {
    subcommand: 'info',
    package: 'com.example.app'
  });

  console.log('Package info:', result.data);
  // 输出:
  // {
  //   "name": "com.example.app",
  //   "version": "1.0.0",
  //   "description": "Example application",
  //   "author": "Example Corp",
  //   "dependencies": ["libfoo", "libbar"],
  //   "size": 1024000,
  //   "installDate": "2025-04-11T10:00:00Z",
  //   "permissions": ["ohos.permission.INTERNET"]
  // }
}
```

#### 生成的命令行

```bash
# install 子命令
/usr/bin/pkg-mgr install /path/to/app.hap --with-deps

# list 子命令
/usr/bin/pkg-mgr list -v --filter=com.example --format=json

# remove 子命令
/usr/bin/pkg-mgr remove com.example.app --purge --auto-remove

# update 子命令
/usr/bin/pkg-mgr update com.example.app

# info 子命令
/usr/bin/pkg-mgr info com.example.app
```

---

## 7. 开发最佳实践

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

### 7.4 性能优化

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

## 8. 工具开发路线图

### 8.1 MVP 版本开发

**目标**：在 1-2 周内开发出最小可行产品（MVP）

#### Week 1: 基础功能

**Day 1-2: 设计与准备**
- [ ] 定义工具功能和范围
- [ ] 设计输入输出接口
- [ ] 确定参数映射方式
- [ ] 创建项目目录结构

**Day 3-4: 核心功能**
- [ ] 实现主程序入口
- [ ] 实现核心功能逻辑
- [ ] 实现 JSON 输出
- [ ] 本地测试工具

**Day 5: 描述文件**
- [ ] 编写工具描述文件
- [ ] 定义输入参数 Schema
- [ ] 定义输出参数 Schema
- [ ] 配置参数映射

#### Week 2: 测试与部署

**Day 1-2: 测试**
- [ ] 编写单元测试
- [ ] 测试边界情况
- [ ] 修复发现的问题

**Day 3-4: 集成**
- [ ] 安装可执行文件
- [ ] 注册描述文件
- [ ] 集成测试

**Day 5: 文档**
- [ ] 编写 README
- [ ] 编写使用说明
- [ ] 记录已知问题

### 8.2 功能扩展

**Phase 1: 功能增强（1-2 周）**
- [ ] 添加更多参数选项
- [ ] 支持配置文件
- [ ] 改进错误处理
- [ ] 添加进度显示

**Phase 2: 性能优化（1 周）**
- [ ] 分析性能瓶颈
- [ ] 优化启动时间
- [ ] 减少内存占用
- [ ] 并发处理优化

**Phase 3: 高级功能（2-3 周）**
- [ ] 支持异步输出
- [ ] 添加子命令
- [ ] 支持插件系统
- [ ] 国际化支持

### 8.3 持续维护

#### 定期维护任务

**每周**
- [ ] 检查错误日志
- [ ] 收集用户反馈
- [ ] 修复紧急问题

**每月**
- [ ] 性能监控
- [ ] 安全审计
- [ ] 依赖更新
- [ ] 文档更新

**每季度**
- [ ] 功能规划
- [ ] 架构评审
- [ ] 技术债务清理
- [ ] 版本发布

#### 版本管理

**语义化版本号**
```
MAJOR.MINOR.PATCH

MAJOR: 不兼容的 API 变更
MINOR: 向后兼容的功能新增
PATCH: 向后兼容的问题修复
```

**版本发布流程**
1. 创建 release 分支
2. 更新版本号
3. 完善更新日志
4. 测试验证
5. 合并到主分支
6. 打 tag 发布

#### 问题跟踪

**问题分类**
- Bug: 功能缺陷
- Feature: 新功能请求
- Enhancement: 功能改进
- Performance: 性能问题
- Security: 安全问题

**优先级**
- P0: 阻塞性问题，立即修复
- P1: 高优先级，本周修复
- P2: 中优先级，本月修复
- P3: 低优先级，有空再修

---

# 第五部分：参考与调试

## 9. 常见问题与调试

### 9.1 常见错误及解决方案

#### 错误 1: 参数映射错误

**问题**: 工具接收到错误的参数格式

**原因**: `argumentMapping` 配置错误

**表现**:
```bash
/usr/bin/net-scan --target 192.168.1.1  # 参数值没有用等号连接
```

**解决**:
1. 检查映射类型是否正确
2. 验证模板语法
3. 确认参数名称匹配

```json
{
  "argumentMapping": {
    "type": "flag",
    "templates": {
      "target": "--target={value}",  // 正确
      "target": "--target {value}"   // 错误：空格而不是等号
    }
  }
}
```

#### 错误 2: JSON Schema 验证失败

**问题**: 参数被框架拒绝

**原因**: 输入参数不符合 Schema 定义

**表现**:
```typescript
Error: Parameter validation failed
  Field: ports
  Issue: Array too long (maxItems: 100, actual: 150)
```

**解决**:
1. 检查必填参数是否提供
2. 验证参数类型是否正确
3. 确认参数值在允许范围内

```json
{
  "input": {
    "properties": {
      "ports": {
        "type": "array",
        "maxItems": 100  // 增加 maxItems 或减少传入的端口数量
      }
    }
  }
}
```

#### 错误 3: 工具未注册

**问题**: 调用工具时返回 "Tool not found"

**原因**:
- 描述文件未安装
- 工具名称不匹配
- 注册表未更新

**解决**:
```bash
# 1. 检查描述文件
ls /etc/cli/tools/network-scanner.json

# 2. 检查可执行文件
ls /usr/bin/net-scan

# 3. 重新加载注册表
sudo systemctl reload cli-manager

# 4. 验证注册
cli-tool list | grep network-scanner
```

#### 错误 4: 权限不足

**问题**: 工具执行时权限被拒绝

**原因**: 工具需要的权限未在描述文件中声明

**解决**:
```json
{
  "permissions": [
    "ohos.permission.INTERNET",
    "ohos.permission.GET_NETWORK_INFO"
  ]
}
```

然后重新注册工具。

#### 错误 5: 超时

**问题**: 工具执行超时被强制终止

**原因**: 工具执行时间超过 `timeout` 设置

**解决**:
```json
{
  "timeout": 120000  // 增加超时时间到 120 秒
}
```

或优化工具性能，减少执行时间。

### 9.2 调试技巧

#### 本地测试工具

```bash
# 直接运行工具测试
/usr/bin/net-scan --target=127.0.0.1 --port=80

# 查看工具输出
/usr/bin/net-scan --target=127.0.0.1 --port=80 | jq .

# 查看详细输出
/usr/bin/net-scan --target=127.0.0.1 --port=80 -v
```

#### 使用 strace 跟踪系统调用

```bash
# 跟踪系统调用
strace -e trace=network /usr/bin/net-scan --target=127.0.0.1

# 跟踪文件访问
strace -e trace=file /usr/bin/net-scan --target=127.0.0.1
```

#### 使用 gdb 调试

```bash
# 启动 gdb
gdb /usr/bin/net-scan

# 设置断点
(gdb) break main

# 运行
(gdb) run --target=127.0.0.1 --port=80

# 查看堆栈
(gdb) bt

# 继续执行
(gdb) continue
```

### 9.3 日志分析

#### 查看服务日志

```bash
# 查看 CLI Manager 日志
hilog -x | grep CLIManager

# 查看工具执行日志
hilog -x | grep net-scan

# 实时查看日志
hilog -x | tail -f
```

#### 启用调试日志

```bash
# 设置日志级别
hilog -b D

# 查看调试日志
hilog -x | grep DEBUG
```

#### 日志分析要点

1. **查找错误信息**
   ```bash
   hilog -x | grep -i error
   ```

2. **查找超时记录**
   ```bash
   hilog -x | grep -i timeout
   ```

3. **统计执行次数**
   ```bash
   hilog -x | grep "net-scan" | wc -l
   ```

### 9.4 FAQ

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

## 10. 参考资料

### 10.1 相关文档

**OpenHarmony 文档**
- [OpenHarmony 官方文档](https://docs.openharmony.cn/)
- [应用包管理规范](https://docs.openharmony.cn/v3.2/zh-cn/application-bundle/overview.md)
- [权限管理指南](https://docs.openharmony.cn/v3.2/zh-cn/security/permission/overview.md)

**JSON Schema 规范**
- [JSON Schema 官方规范](https://json-schema.org/)
- [JSON Schema 验证器](https://github.com/epoberezkin/ajv)

**构建系统**
- [GN 构建系统](https://gn.googlesource.com/gn/+/main/docs/reference.md)
- [Ninja 构建工具](https://ninja-build.org/)

### 10.2 工具与资源

**开发工具**
- **jq**: JSON 数据处理工具
  ```bash
  sudo apt-get install jq
  ```

- **ajv**: JSON Schema 验证器
  ```bash
  npm install -g ajv-cli
  ```

- **strace**: 系统调用跟踪工具
  ```bash
  sudo apt-get install strace
  ```

- **gdb**: GNU 调试器
  ```bash
  sudo apt-get install gdb
  ```

**测试框架**
- **Google Test**: C++ 测试框架
  ```bash
  sudo apt-get install libgtest-dev
  ```

- **Check**: C 测试框架
  ```bash
  sudo apt-get install check
  ```

**在线工具**
- JSON 在线编辑器: https://jsoneditoronline.org/
- JSON Schema 验证: https://www.jsonschemavalidator.net/
- 正则表达式测试: https://regex101.com/

### 10.3 术语表

| 术语 | 英文 | 说明 |
|------|------|------|
| **CLI 工具** | CLI Tool | 命令行界面工具 |
| **描述文件** | Descriptor File | 描述工具能力的 JSON 文件 |
| **参数映射** | Argument Mapping | JSON 参数到命令行参数的转换规则 |
| **子命令** | Subcommand | 工具的子功能模块 |
| **JSON Schema** | JSON Schema | JSON 数据结构的验证规范 |
| **事件** | Event | 工具输出的异步通知 |
| **异步输出** | Async Output | 工具执行过程中的实时输出 |
| **MVP** | Minimum Viable Product | 最小可行产品 |
| **RAII** | Resource Acquisition Is Initialization | 资源获取即初始化（C++ 编程范式） |
| **退出码** | Exit Code | 进程退出时返回的状态码 |
| **语义化版本** | Semantic Versioning | 使用 MAJOR.MINOR.PATCH 格式的版本号 |

---

## 11. 版本历史

### 11.1 更新记录

**v2.0 (2025-04-11)**
- 🎉 完全重构文档结构
- ✨ 新增"快速开始"章节
- ✨ 新增"开发最佳实践"章节
- ✨ 新增"工具开发路线图"章节
- ✨ 新增"参考资料"章节
- 🔧 整合参数相关章节
- 🔧 调整章节顺序，更符合开发流程
- 📝 更新所有示例代码
- 🐛 修复文档中的错误

**v1.1 (2025-04-11)**
- 📝 完善参数映射说明
- ✨ 新增 javascript 模式
- 🔧 修复示例代码中的错误
- 📝 添加更多调试技巧

**v1.0 (2025-04-10)**
- 🎉 初始版本发布
- ✨ 基本框架结构
- ✨ 核心配置说明
- ✨ 完整示例

### 11.2 计划中的功能

**短期计划（v2.1）**
- [ ] 添加更多参数映射示例
- [ ] 完善错误处理指南
- [ ] 添加性能优化案例

**中期计划（v2.2）**
- [ ] 添加插件开发指南
- [ ] 完善国际化支持
- [ ] 添加视频教程链接

**长期计划（v3.0）**
- [ ] 支持更多编程语言示例
- [ ] 提供工具开发模板
- [ ] 集成 CI/CD 流程

---

**文档版本**: v2.0
**最后更新**: 2025-04-11
**维护者**: CLI 工具管理框架团队
**反馈**: 请通过项目 Issue 提交问题和建议
