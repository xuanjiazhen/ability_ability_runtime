
# 4. 工具注册表设计

## 4.1 目录结构

CLI 服务的工具统一安装在 `/etc/cli/` 目录下：

```
/etc/cli/
├── cli_tools.json      # 工具注册表（所有工具的描述信息）
└── tools/              # 工具可执行文件目录
    ├── net-scan
    ├── log-mon
    ├── app-deploy
    ├── cloud-deploy
    ├── file-copy
    ├── pkg-mgr
    └── sysctl
```

### 目录说明

| 路径 | 说明 | 权限要求 |
|------|------|----------|
| `/etc/cli/` | CLI 配置根目录 | root:root 755 |
| `/etc/cli/cli_tools.json` | 工具注册表文件 | root:root 644 |
| `/etc/cli/tools/` | 可执行文件目录 | root:root 755 |
| `/etc/cli/tools/*` | 工具可执行文件 | root:root 755 (不可设置 setuid/setgid) |

---

## 4.2 注册表格式

### 4.2.1 基本结构

`cli_tools.json` 是一个只包含列表的JSON 文件包含所有可用工具的完整描述信息。

```json
[
  {"name": "工具1", ... }, // 其他字段忽略
  {"name": "工具2", ... }
]
```
---


# 5. CLI工具描述设计
新增一个CLI工具，基本格式目录格式如下：
```
cli-<name>/
├── config.json # 工具配置文件（单个工具的描述信息）
├── BUILD.gn   # 编译安装配置
├── skills.md   # 文档
├── tests/    # 测试文件
└── src/
    └── main.cpp # bin主命令
```

## 5.1 工具描述格式 config.json

**文件格式：** JSON

CLI 工具描述支持两种格式：

| 格式类型 | 适用场景 | 特征 |
|---------|---------|------|
| **无子命令工具** | 单一功能工具（如文件拷贝、网络扫描） | 顶层定义 `inputSchema` 和 `argumentMapping` |
| **有子命令工具** | 多功能集合工具（如包管理器、系统控制） | 使用 `hasSubcommands: true` 和 `subcommands` |

---

### 5.1.1 无子命令工具格式

**适用场景：** 工具只执行单一功能，无需子命令区分。

**完整字段说明：**

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `name` | string | 是 | 工具唯一标识 |
| `version` | string | 是 | 工具版本号（语义化版本） |
| `description` | string | 是 | 工具功能描述 |
| `executablePath` | string | 是 | 可执行文件绝对路径 |
| `permissions` | string[] | 否 | 所需权限列表 |
| `inputSchema` | object | 是 | 输入参数 JSON Schema |
| `outputSchema` | object | 否 | 最终结果 JSON Schema |
| `argumentMapping` | object | 是 | 参数映射规则 |
| `timeout` | number | 否 | 超时时间（毫秒），0 表示无限制 |
| `eventTypes` | string[] | 否 | 支持的事件类型 |
| `registeredTime` | integer | 否 | 注册时间 |
| `enabled` | boolean | 否 | 是否使能 |
| `eventSchemas` | object | 否 | 事件 Schema 定义（key 为事件类型） |
| `hasSubcommands` | boolean | 否 | 必须为 `false` 或不设置 |
| `subcommands` | object | 否 | 子命令集合 |

---

**字段详解：**

| 字段 | 详细说明 |
|------|---------|
| **outputSchema** | 定义工具执行完成后的最终结果格式。当 `eventTypes` 包含 `result` 时，框架会根据此 Schema 验证最终结果。 |
| **eventTypes** | 声明工具支持的事件类型数组。可选值：`stdout`、`stderr`、`exit`、`error`、`progress`、`result` |
| **eventSchemas** | 定义自定义事件的数据格式。key 为事件类型（如 `progress`），value 为 JSON Schema。 |

---

**示例：网络扫描工具（带进度报告）**

```json
{
  "name": "network-scanner",
  "version": "1.0.0",
  "description": "Network scanning utility tool",
  "executablePath": "/usr/bin/net-scan",
  "permissions": [
    "ohos.permission.INTERNET",
    "ohos.permission.GET_NETWORK_INFO"
  ],
  "inputSchema": {
    "type": "object",
    "properties": {
      "target": {
        "type": "string",
        "format": "ipv4",
        "description": "Target IP address"
      },
      "ports": {
        "type": "array",
        "items": {"type": "integer"},
        "description": "Ports to scan"
      }
    },
    "required": ["target"]
  },

  "outputSchema": {
    "type": "object",
    "description": "最终扫描结果",
    "properties": {
      "results": {
        "type": "array",
        "description": "扫描结果列表",
        "items": {
          "type": "object",
          "properties": {
            "port": {"type": "integer"},
            "status": {
              "type": "string",
              "enum": ["open", "closed", "filtered"]
            },
            "service": {"type": "string"},
            "responseTime": {"type": "integer"}
          }
        }
      },
      "summary": {
        "type": "object",
        "description": "扫描汇总信息",
        "properties": {
          "totalPorts": {"type": "integer"},
          "openPorts": {"type": "integer"},
          "scanTime": {"type": "integer"}
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

  "timeout": 30000,
  "eventTypes": ["progress", "result"],

  "eventSchemas": {
    "progress": {
      "type": "object",
      "description": "扫描进度事件",
      "properties": {
        "percentage": {
          "type": "integer",
          "minimum": 0,
          "maximum": 100,
          "description": "完成百分比"
        },
        "currentPort": {
          "type": "integer",
          "description": "当前扫描的端口"
        },
        "totalPorts": {
          "type": "integer",
          "description": "总端口数"
        },
        "status": {
          "type": "string",
          "enum": ["scanning", "completed", "failed"],
          "description": "当前状态"
        },
        "timestamp": {
          "type": "integer",
          "description": "时间戳（毫秒）"
        }
      },
      "required": ["percentage", "status"]
    }
  }
}
```

**调用方式：**

```javascript
await CLI.CLIManager.execTool('network-scanner', {
  target: "192.168.1.1",
  ports: [80, 443]
});
```

**生成的命令：**

```bash
/usr/bin/net-scan --target=192.168.1.1 --port=80 --port=443
```

**工具进程输出约定：**

工具通过向 **stdout 输出 JSON 行**来触发事件：

```bash
# 进度事件
{"event": "progress", "data": {"percentage": 50, "currentPort": 80, "totalPorts": 2, "status": "scanning", "timestamp": 1711468800000}}

# 最终结果事件
{"event": "result", "data": {"results": [{"port": 80, "status": "open", "service": "http", "responseTime": 50}], "summary": {"totalPorts": 2, "openPorts": 1, "scanTime": 1500}}}
```

**框架处理流程：**

```
工具进程 stdout
    ↓
解析 JSON 行
    ↓
识别 event 类型
    ↓
根据 eventSchemas 验证数据格式
    ↓
分发事件到调用方
```

**调用方接收事件：**

```javascript
const session = await CLI.CLIManager.execTool('network-scanner', {
  target: "192.168.1.1",
  ports: [80, 443]
}, {
  onEvent: (event) => {
    if (event.type === 'progress') {
      // progress 事件数据符合 eventSchemas.progress 定义的格式
      console.log(`进度: ${event.data.percentage}%, 端口: ${event.data.currentPort}`);
    } else if (event.type === 'result') {
      // result 事件数据符合 output 定义的格式
      console.log('扫描完成:', event.data.summary);
    }
  }
});
```

---

### 5.1.2 事件输出机制

**核心原则：** 工具进程通过 **stdout 输出标准格式的 JSON 行** 来触发事件，框架解析并分发事件。

#### 事件输出格式

工具输出的每一行必须是有效的 JSON 对象，包含以下字段：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `event` | string | 是 | 事件类型，必须是 `eventTypes` 数组中声明的类型 |
| `data` | any | 是 | 事件数据，必须符合对应事件的 Schema 定义 |
| `timestamp` | number | 否 | 事件时间戳（毫秒），框架会自动添加 |

#### 事件类型与输出对应关系

| 事件类型 | 触发方式 | 数据来源 | Schema 位置 |
|---------|---------|---------|------------|
| `stdout` | 工具输出非 JSON 行 | 原始文本行 | 不需要 Schema |
| `stderr` | 工具向 stderr 输出 | 原始错误行 | 不需要 Schema |
| `exit` | 进程退出 | 框架自动生成 | 不需要 Schema |
| `error` | 执行失败 | 框架自动生成 | 不需要 Schema |
| `progress` | 输出 `{"event": "progress", ...}` | 工具定义 | `eventSchemas.progress` |
| `result` | 输出 `{"event": "result", ...}` | 工具定义 | `outputSchema` |

#### 标准事件示例

**1. progress 事件**

```bash
# 工具输出
{"event": "progress", "data": {"percentage": 50, "currentPort": 80, "totalPorts": 2, "status": "scanning"}}
```

**2. result 事件**

```bash
# 工具输出
{"event": "result", "data": {"results": [...], "summary": {"totalPorts": 2, "openPorts": 1}}}
```

**3. 混合输出（stdout + progress + result）**

```bash
# 工具输出
Starting scan...
{"event": "progress", "data": {"percentage": 50, "status": "scanning"}}
Scanning port 443...
{"event": "progress", "data": {"percentage": 100, "status": "completed"}}
{"event": "result", "data": {"results": [...], "summary": {...}}}
Scan complete.
```

#### 非事件输出处理

**非 JSON 行：** 当工具输出非 JSON 格式的文本时，框架会将其作为 `stdout` 事件处理：

```bash
# 工具输出普通文本
Scanning port 80...
Port 80 is open
Scanning port 443...
```

框架自动转换为：
```javascript
{type: 'stdout', data: 'Scanning port 80...'}
{type: 'stdout', data: 'Port 80 is open'}
{type: 'stdout', data: 'Scanning port 443...'}
```

#### 错误处理

**无效 JSON：** 如果工具输出的 JSON 格式错误，框架会生成 `error` 事件：

```javascript
{type: 'error', data: 'Invalid JSON output on line 5: ...'}
```

**Schema 验证失败：** 如果事件数据不符合 Schema 定义，框架会生成 `error` 事件：

```javascript
{type: 'error', data: 'Progress event validation failed: missing required field "percentage"'}
```

---

### 5.1.3 有子命令工具格式

**适用场景：** 工具包含多个独立功能模块，通过子命令区分。

**完整字段说明：**

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `name` | string | 是 | 工具唯一标识 |
| `version` | string | 是 | 工具版本号 |
| `description` | string | 是 | 工具功能描述 |
| `executablePath` | string | 是 | 可执行文件绝对路径 |
| `permissions` | string[] | 否 | 所需权限列表 |
| `timeout` | number | 否 | 超时时间（毫秒） |
| `registeredTime` | integer | 否 | 注册时间 |
| `eventTypes` | string[] | 否 | 支持的事件类型 |
| `hasSubcommands` | boolean | **是** | 必须为 `true` |
| `subcommands` | object | **是** | 子命令集合 |
| `enabled` | boolean | 否 | 是否使能 |

**子命令对象 (`subcommands.<name>`) 字段：**

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `description` | string | 是 | 子命令描述 |
| `inputSchema` | object | 是 | 子命令输入参数 Schema |
| `argumentMapping` | object | 是 | 子命令参数映射规则 |
| `outputSchema` | object | 否 | 最终结果 JSON Schema |

**示例：包管理工具**

```json
{
  "name": "pkg-manager",
  "version": "1.0.0",
  "description": "Package management tool with subcommands",
  "executablePath": "/usr/bin/pkg-mgr",
  "permissions": [
    "ohos.permission.INSTALL_BUNDLE",
    "ohos.permission.ACCESS_BUNDLE"
  ],
  "timeout": 300000,
  "eventTypes": ["progress"],
  "hasSubcommands": true,
  "subcommands": {
    "install": {
      "description": "Install a package from local file or remote repository",
      "inputSchema": {
        "type": "object",
        "properties": {
          "package": {
            "type": "string",
            "description": "Package file path or package name"
          },
          "force": {
            "type": "boolean",
            "default": false,
            "description": "Force installation"
          },
          "silent": {
            "type": "boolean",
            "default": false,
            "description": "Silent installation"
          }
        },
        "required": ["package"]
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}",
          "force": "--force",
          "silent": "--silent"
        }
      }
    },
    "remove": {
      "description": "Remove an installed package",
      "inputSchema": {
        "type": "object",
        "properties": {
          "package": {
            "type": "string",
            "description": "Package name to remove"
          },
          "purge": {
            "type": "boolean",
            "default": false,
            "description": "Purge configuration files"
          }
        },
        "required": ["package"]
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}",
          "purge": "--purge"
        }
      }
    },
    "list": {
      "description": "List installed packages",
      "inputSchema": {
        "type": "object",
        "properties": {
          "verbose": {
            "type": "boolean",
            "default": false,
            "description": "Show detailed information"
          },
          "filter": {
            "type": "string",
            "description": "Filter packages by pattern"
          }
        }
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "verbose": "--verbose",
          "filter": "--filter={value}"
        }
      }
    }
  }
}
```

**调用方式：**

```javascript
// 安装包
await CLI.CLIManager.execTool('pkg-manager', {
  subcommand: 'install',
  package: '/path/to/app.hap',
  force: true
});

// 列出已安装包
await CLI.CLIManager.execTool('pkg-manager', {
  subcommand: 'list',
  verbose: true
});
```

**生成的命令：**

```bash
# install 子命令
/usr/bin/pkg-mgr install /path/to/app.hap --force

# list 子命令
/usr/bin/pkg-mgr list --verbose
```

---

### 5.1.4 两种格式对比

| 特性 | 无子命令工具 | 有子命令工具 |
|------|-------------|-------------|
| **字段 `hasSubcommands`** | 不存在或 `false` | `true` |
| **字段 `subcommands`** | 不存在 | 必填，包含子命令集合 |
| **顶层 `inputSchema`** | 定义工具参数 | 不使用（参数在子命令中定义） |
| **顶层 `argumentMapping`** | 定义参数映射 | 不使用（映射在子命令中定义） |
| **调用时 `subcommand`** | 不需要 | 必须指定子命令名称 |
| **生成的命令格式** | `executable [args]` | `executable <subcommand> [args]` |

---

### 5.1.5 选择建议

| 场景 | 推荐格式 | 示例 |
|------|---------|------|
| 单一功能操作 | 无子命令 | 文件拷贝、网络扫描、日志查询 |
| 多功能集合 | 有子命令 | 包管理器、系统控制、版本管理 |
| 可扩展功能 | 有子命令 | CLI 框架工具 |

---

## 5.2 参数Schema定义

### 5.2.1 嵌套参数支持

#### 平面嵌套（推荐）

**设计思路：** 将嵌套对象展平为命令行参数，使用前缀或点号分隔。

**工具描述文件：**
```json
{
  "name": "app-deploy",
  "description": "应用部署工具",
  "executablePath": "/usr/bin/app-deploy",
  "inputSchema": {
    "type": "object",
    "properties": {
      "app": {
        "type": "object",
        "description": "应用配置",
        "properties": {
          "name": {"type": "string"},
          "version": {"type": "string"},
          "env": {
            "type": "object",
            "properties": {
              "production": {"type": "boolean"},
              "debug": {"type": "boolean"}
            }
          }
        },
        "required": ["name"]
      },
      "server": {
        "type": "object",
        "description": "服务器配置",
        "properties": {
          "host": {"type": "string"},
          "port": {"type": "integer"},
          "ssl": {
            "type": "object",
            "properties": {
              "enabled": {"type": "boolean"},
              "cert": {"type": "string"}
            }
          }
        },
        "required": ["host", "port"]
      }
    },
    "required": ["app"]
  },

  "argumentMapping": {
    "type": "flattened",
    "separator": ".",
    "templates": {
      "app.name": "--app-name={value}",
      "app.version": "--app-version={value}",
      "app.env.production": "--env-prod",
      "app.env.debug": "--env-debug",
      "server.host": "--server-host={value}",
      "server.port": "--server-port={value}",
      "server.ssl.enabled": "--ssl",
      "server.ssl.cert": "--ssl-cert={value}"
    }
  }
}
```

**调用方式：**
```typescript
const session = await CLI.CLIManager.execTool('app-deploy', {
  app: {
    name: "myapp",
    version: "1.0.0",
    env: {
      production: true,
      debug: false
    }
  },
  server: {
    host: "192.168.1.1",
    port: 8080,
    ssl: {
      enabled: true,
      cert: "/path/to/cert.pem"
    }
  }
});
```

**生成的命令：**
```bash
/usr/bin/app-deploy \
  --app-name myapp \
  --app-version 1.0.0 \
  --env-prod \
  --server-host 192.168.1.1 \
  --server-port 8080 \
  --ssl \
  --ssl-cert /path/to/cert.pem
```

#### JSON 字符串传递

**设计思路：** 保持嵌套结构，将对象序列化为 JSON 字符串传递。

**工具描述文件：**
```json
{
  "name": "cloud-deploy",
  "description": "云端部署工具",
  "executablePath": "/usr/bin/cloud-deploy",

  "inputSchema": {
    "type": "object",
    "properties": {
      "config": {
        "type": "object",
        "description": "部署配置（JSON 对象）",
        "properties": {
          "application": {
            "type": "object",
            "properties": {
              "name": {"type": "string"},
              "replicas": {"type": "integer"}
            }
          },
          "resources": {
            "type": "object",
            "properties": {
              "cpu": {"type": "string"},
              "memory": {"type": "string"},
              "storage": {"type": "string"}
            }
          }
        }
      },
      "options": {
        "type": "object",
        "properties": {
          "dryRun": {"type": "boolean"},
          "timeout": {"type": "integer"}
        }
      }
    },
    "required": ["config"]
  },

  "argumentMapping": {
    "type": "json-string",
    "templates": {
      "config": "--config={value}",
      "options.dryRun": {"if_true": "--dry-run", "if_false": ""},
      "options.timeout": "--timeout={value}"
    }
  }
}
```

**调用方式：**
```typescript
const session = await CLI.CLIManager.execTool('cloud-deploy', {
  config: {
    application: {
      name: "webapp",
      replicas: 3
    },
    resources: {
      cpu: "2",
      memory: "4Gi",
      storage: "100Gi"
    }
  },
  options: {
    dryRun: true,
    timeout: 300
  }
});
```

**生成的命令：**
```bash
/usr/bin/cloud-deploy \
  --config '{"application":{"name":"webapp","replicas":3},"resources":{"cpu":"2","memory":"4Gi","storage":"100Gi"}}' \
  --dry-run \
  --timeout 300
```

## 5.3 子命令支持

### 5.3.1 基本概念

子命令（Subcommands）是指一个工具有多个独立的功能模块，例如：
- `pkg-manager install` - 安装包
- `pkg-manager remove` - 卸载包
- `pkg-manager list` - 列出已安装包
- `system-control status` - 查看系统状态
- `system-control restart` - 重启服务

### 5.3.2 注册表中的子命令定义

在 `cli_tools.json` 中，支持子命令的工具需要添加以下字段：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `hasSubcommands` | boolean | 是 | 标识工具支持子命令 |
| `subcommands` | object | 是 | 子命令集合 |
| `subcommands.<name>` | object | - | 单个子命令的描述 |
| `subcommands.<name>.description` | string | 是 | 子命令描述 |
| `subcommands.<name>.inputSchema` | object | 是 | 子命令参数 Schema |
| `subcommands.<name>.outputSchema` | object | 否 | 子命令最终结果 Schema |
| `subcommands.<name>.eventTypes` | string[] | 否 | 子命令支持的事件类型 |
| `subcommands.<name>.eventSchemas` | object | 否 | 子命令事件 Schema 定义 |
| `subcommands.<name>.argumentMapping` | object | 是 | 子命令参数映射规则 |

### 5.3.3 子命令示例（pkg-manager）

```json
{
  "name": "pkg-manager",
  "version": "1.0.0",
  "description": "Package management tool with subcommands",
  "executablePath": "/etc/cli/tools/pkg-mgr",
  "permissions": [
    "ohos.permission.INSTALL_BUNDLE",
    "ohos.permission.ACCESS_BUNDLE"
  ],
  "timeout": 300000,
  "eventTypes": ["progress"],
  "hasSubcommands": true,
  "subcommands": {
    "install": {
      "description": "Install a package from local file or remote repository",
      "inputSchema": {
        "type": "object",
        "properties": {
          "package": {
            "type": "string",
            "description": "Package file path or package name"
          },
          "force": {
            "type": "boolean",
            "default": false,
            "description": "Force installation even if already installed"
          },
          "replace": {
            "type": "boolean",
            "default": false,
            "description": "Replace existing package"
          },
          "silent": {
            "type": "boolean",
            "default": false,
            "description": "Silent installation without prompts"
          }
        },
        "required": ["package"]
      },
      "outputSchema": {
        "type": "object",
        "description": "安装结果",
        "properties": {
          "packageName": {
            "type": "string",
            "description": "包名称"
          },
          "version": {
            "type": "string",
            "description": "安装版本"
          },
          "path": {
            "type": "string",
            "description": "安装路径"
          },
          "success": {
            "type": "boolean",
            "description": "是否成功"
          }
        }
      },
      "eventTypes": ["progress"],
      "eventSchemas": {
        "progress": {
          "type": "object",
          "description": "安装进度事件",
          "properties": {
            "stage": {
              "type": "string",
              "enum": ["downloading", "installing", "configuring", "completed"],
              "description": "当前阶段"
            },
            "percentage": {
              "type": "integer",
              "minimum": 0,
              "maximum": 100,
              "description": "完成百分比"
            },
            "downloaded": {
              "type": "integer",
              "description": "已下载字节数"
            },
            "total": {
              "type": "integer",
              "description": "总字节数"
            },
            "currentFile": {
              "type": "string",
              "description": "当前处理的文件"
            }
          },
          "required": ["stage", "percentage"]
        }
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}",
          "force": "--force",
          "replace": "--replace",
          "silent": "--silent"
        }
      }
    },
    "remove": {
      "description": "Remove an installed package",
      "inputSchema": {
        "type": "object",
        "properties": {
          "package": {
            "type": "string",
            "description": "Package name to remove"
          },
          "purge": {
            "type": "boolean",
            "default": false,
            "description": "Purge package configuration files"
          },
          "keepData": {
            "type": "boolean",
            "default": true,
            "description": "Keep package data directories"
          }
        },
        "required": ["package"]
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}",
          "purge": "--purge",
          "keepData": "--keep-data"
        }
      }
    },
    "list": {
      "description": "List installed packages",
      "inputSchema": {
        "type": "object",
        "properties": {
          "installed": {
            "type": "boolean",
            "default": true,
            "description": "Show installed packages"
          },
          "all": {
            "type": "boolean",
            "default": false,
            "description": "Show all available packages"
          },
          "verbose": {
            "type": "boolean",
            "default": false,
            "description": "Show detailed package information"
          },
          "filter": {
            "type": "string",
            "description": "Filter packages by name pattern"
          }
        }
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "installed": "--installed",
          "all": "--all",
          "verbose": "-v",
          "filter": "--filter={value}"
        }
      }
    },
    "update": {
      "description": "Update installed packages to latest version",
      "inputSchema": {
        "type": "object",
        "properties": {
          "package": {
            "type": "string",
            "description": "Package name to update (empty for all packages)"
          },
          "checkOnly": {
            "type": "boolean",
            "default": false,
            "description": "Only check for updates without installing"
          }
        }
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}",
          "checkOnly": "--check-only"
        }
      }
    },
    "info": {
      "description": "Show detailed information about a package",
      "inputSchema": {
        "type": "object",
        "properties": {
          "package": {
            "type": "string",
            "description": "Package name"
          },
          "json": {
            "type": "boolean",
            "default": false,
            "description": "Output in JSON format"
          }
        },
        "required": ["package"]
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}",
          "json": "--json"
        }
      }
    }
  }
}
```

### 5.3.4 子命令示例（system-control）

```json
{
  "name": "system-control",
  "version": "1.0.0",
  "description": "System control and monitoring tool with subcommands",
  "executablePath": "/etc/cli/tools/sysctl",
  "permissions": ["ohos.permission.ACCESS_SYSTEM"],
  "timeout": 60000,
  "eventTypes": [],
  "hasSubcommands": true,
  "subcommands": {
    "status": {
      "description": "Show system status and health",
      "inputSchema": {
        "type": "object",
        "properties": {
          "detailed": {
            "type": "boolean",
            "default": false,
            "description": "Show detailed status"
          },
          "component": {
            "type": "string",
            "description": "Show status of specific component"
          }
        }
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "detailed": "--detailed",
          "component": "--component={value}"
        }
      }
    },
    "restart": {
      "description": "Restart a system service or component",
      "inputSchema": {
        "type": "object",
        "properties": {
          "service": {
            "type": "string",
            "description": "Service name to restart"
          },
          "force": {
            "type": "boolean",
            "default": false,
            "description": "Force restart without checks"
          },
          "timeout": {
            "type": "integer",
            "minimum": 1,
            "maximum": 300,
            "default": 30,
            "description": "Restart timeout in seconds"
          }
        },
        "required": ["service"]
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "service": "{value}",
          "force": "--force",
          "timeout": "--timeout={value}"
        }
      }
    },
    "logs": {
      "description": "View and filter system logs",
      "inputSchema": {
        "type": "object",
        "properties": {
          "service": {
            "type": "string",
            "description": "Service name to show logs for"
          },
          "lines": {
            "type": "integer",
            "minimum": 1,
            "maximum": 10000,
            "default": 100,
            "description": "Number of log lines to show"
          },
          "follow": {
            "type": "boolean",
            "default": false,
            "description": "Follow log output (like tail -f)"
          },
          "level": {
            "type": "string",
            "enum": ["DEBUG", "INFO", "WARN", "ERROR"],
            "default": "INFO",
            "description": "Minimum log level"
          }
        }
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "service": "--service={value}",
          "lines": "--lines={value}",
          "follow": "--follow",
          "level": "--level={value}"
        }
      }
    },
    "config": {
      "description": "Get or set system configuration",
      "inputSchema": {
        "type": "object",
        "properties": {
          "get": {
            "type": "string",
            "description": "Get configuration value"
          },
          "set": {
            "type": "string",
            "description": "Set configuration value"
          },
          "value": {
            "type": "string",
            "description": "Configuration value (used with --set)"
          },
          "json": {
            "type": "boolean",
            "default": false,
            "description": "Output in JSON format"
          }
        }
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "get": "--get={value}",
          "set": "--set={value}",
          "value": "--value={value}",
          "json": "--json"
        }
      }
    }
  }
}
```

### 5.3.5 JavaScript 调用方式

```typescript
import CLI from '@ohos.cli.manager';

// 1. 安装包
await CLI.CLIManager.execTool('pkg-manager', {
  subcommand: 'install',
  package: '/data/downloads/com.example.app.hap',
  force: true,
  silent: true
});

// 2. 列出已安装的包
await CLI.CLIManager.execTool('pkg-manager', {
  subcommand: 'list',
  installed: true,
  verbose: true,
  filter: 'com.example'
});

// 3. 系统状态
await CLI.CLIManager.execTool('system-control', {
  subcommand: 'status',
  detailed: true
});

// 4. 重启服务
await CLI.CLIManager.execTool('system-control', {
  subcommand: 'restart',
  service: 'climgr',
  force: false,
  timeout: 60
});

// 5. 查看日志（长时运行，后台执行）
await CLI.CLIManager.execTool('system-control', {
  subcommand: 'logs',
  service: 'climgr',
  lines: 500,
  follow: true,
  level: 'ERROR'
}, {
  background: true
});
```

### 5.3.6 生成的命令示例

```bash
# pkg-manager install
/etc/cli/tools/pkg-mgr install /data/downloads/com.example.app.hap --force --silent

# pkg-manager list
/etc/cli/tools/pkg-mgr list --installed -v --filter=com.example

# system-control status
/etc/cli/tools/sysctl status --detailed

# system-control restart
/etc/cli/tools/sysctl restart climgr --force --timeout=60

# system-control logs
/etc/cli/tools/sysctl logs --service=climgr --lines=500 --follow --level=ERROR
```

### 5.3.7 数据结构扩展

```cpp
// interfaces/inner_api/cli_manager/cli_tool_info.h

struct SubCommandInfo {
    std::string description;              // 子命令描述
    Json::Value inputSchema;             // 输入参数 Schema
    Json::Value argMapping;              // 参数映射规则
};

struct ToolInfo {
    std::string name;
    std::string version;
    std::string description;
    std::string executablePath;
    std::vector<std::string> permissions;
    uint32_t timeoutMs;

    // 子命令支持
    bool hasSubcommands = false;
    std::map<std::string, SubCommandInfo> subcommands;

    // ... 其他字段
};
```

### 5.3.8 参数映射处理

```cpp
// services/climgr/src/parameter_mapper.cpp

std::vector<std::string> ParameterMapper::MapToCommandLine(
    const ToolInfo& toolInfo,
    const Json::Value& abstractArgs)
{
    std::vector<std::string> argv;
    argv.push_back(toolInfo.executablePath);  // argv[0]

    // 检查是否有子命令
    if (toolInfo.hasSubcommands) {
        std::string subcommandName = abstractArgs.get("subcommand", "").asString();
        if (subcommandName.empty()) {
            TAG_LOGE(TAG, "Missing 'subcommand' field");
            return {};
        }

        // 查找子命令
        auto it = toolInfo.subcommands.find(subcommandName);
        if (it == toolInfo.subcommands.end()) {
            TAG_LOGE(TAG, "Unknown subcommand: %{public}s", subcommandName.c_str());
            return {};
        }

        // 添加子命令名称
        argv.push_back(subcommandName);

        // 使用子命令的参数映射规则
        const SubCommandInfo& subcmd = it->second;
        ApplyArgumentMapping(subcmd.argMapping, abstractArgs, argv);
    } else {
        // 普通工具，使用工具级别的参数映射
        ApplyArgumentMapping(toolInfo.argMapping, abstractArgs, argv);
    }

    return argv;
}
```

---

## 5.4 完整示例

### 5.4.1 完整的 cli_tools.json 示例

以下是包含 7 个工具（5 个普通工具 + 2 个带子命令的工具）的完整注册表示例：

```json
[
  {
    "name": "network-scanner",
    "version": "1.0.0",
    "description": "Network scanning utility for diagnosing network connectivity",
    "executablePath": "/etc/cli/tools/net-scan",
    "permissions": [
      "ohos.permission.INTERNET",
      "ohos.permission.GET_NETWORK_INFO"
    ],
    "timeout": 30000,
    "eventTypes": ["progress", "result"],
    "inputSchema": {
      "type": "object",
      "properties": {
        "target": {
          "type": "string",
          "format": "ipv4",
          "description": "Target IP address or hostname"
        },
        "ports": {
          "type": "array",
          "items": {"type": "integer", "minimum": 1, "maximum": 65535},
          "description": "List of ports to scan"
        },
        "timeout": {
          "type": "integer",
          "minimum": 1000,
          "maximum": 60000,
          "default": 5000,
          "description": "Scan timeout in milliseconds"
        },
        "verbose": {
          "type": "boolean",
          "default": false,
          "description": "Enable verbose output"
        }
      },
      "required": ["target"]
    },
    "argumentMapping": {
      "type": "flag",
      "templates": {
        "target": "--target={value}",
        "ports": "--port={value}",
        "timeout": "--timeout={value}",
        "verbose": {"if_true": "-v", "if_false": ""}
      }
    }
  },
  {
    "name": "log-monitor",
    "version": "1.0.0",
    "description": "Long-running log monitoring tool with real-time filtering",
    "executablePath": "/etc/cli/tools/log-mon",
    "permissions": ["ohos.permission.READ_LOGS"],
    "timeout": 0,
    "eventTypes": ["stdout", "stderr", "exit", "error"],
    "inputSchema": {
      "type": "object",
      "properties": {
        "filter": {
          "type": "string",
          "minLength": 1,
          "description": "Log filter pattern"
        },
        "level": {
          "type": "string",
          "enum": ["DEBUG", "INFO", "WARN", "ERROR", "FATAL"],
          "default": "INFO"
        },
        "tail": {
          "type": "boolean",
          "default": true
        },
        "lines": {
          "type": "integer",
          "minimum": 1,
          "maximum": 10000,
          "default": 100
        }
      },
      "required": ["filter"]
    },
    "argumentMapping": {
      "type": "flag",
      "templates": {
        "filter": "-f {value}",
        "level": "--level={value}",
        "tail": {"if_true": "--follow", "if_false": ""},
        "lines": "-n {value}"
      }
    }
  },
  {
    "name": "app-deploy",
    "version": "1.0.0",
    "description": "Application deployment tool with nested configuration",
    "executablePath": "/etc/cli/tools/app-deploy",
    "permissions": ["ohos.permission.INSTALL_BUNDLE"],
    "timeout": 120000,
    "eventTypes": ["progress"],
    "inputSchema": {
      "type": "object",
      "properties": {
        "app": {
          "type": "object",
          "properties": {
            "name": {"type": "string"},
            "version": {"type": "string"},
            "env": {
              "type": "object",
              "properties": {
                "production": {"type": "boolean", "default": false},
                "debug": {"type": "boolean", "default": false}
              }
            }
          },
          "required": ["name"]
        },
        "server": {
          "type": "object",
          "properties": {
            "host": {"type": "string"},
            "port": {"type": "integer", "minimum": 1, "maximum": 65535, "default": 8080}
          },
          "required": ["host"]
        }
      },
      "required": ["app", "server"]
    },
    "argumentMapping": {
      "type": "flattened",
      "separator": ".",
      "templates": {
        "app.name": "--app-name={value}",
        "app.version": "--app-version={value}",
        "app.env.production": "--env-prod",
        "server.host": "--server-host={value}",
        "server.port": "--server-port={value}"
      }
    }
  },
  {
    "name": "cloud-deploy",
    "version": "1.0.0",
    "description": "Cloud deployment tool with complex JSON configuration",
    "executablePath": "/etc/cli/tools/cloud-deploy",
    "permissions": ["ohos.permission.INTERNET"],
    "timeout": 600000,
    "eventTypes": ["progress"],
    "inputSchema": {
      "type": "object",
      "properties": {
        "config": {
          "type": "object",
          "properties": {
            "application": {
              "type": "object",
              "properties": {
                "name": {"type": "string"},
                "replicas": {"type": "integer", "minimum": 1, "maximum": 100},
                "image": {"type": "string"}
              }
            },
            "resources": {
              "type": "object",
              "properties": {
                "cpu": {"type": "string"},
                "memory": {"type": "string"}
              }
            }
          }
        },
        "namespace": {
          "type": "string",
          "default": "default"
        }
      },
      "required": ["config"]
    },
    "argumentMapping": {
      "type": "json-string",
      "templates": {
        "config": "--config={json}",
        "namespace": "-n {value}"
      }
    }
  },
  {
    "name": "file-copy",
    "version": "1.0.0",
    "description": "File copy tool using positional arguments",
    "executablePath": "/etc/cli/tools/file-copy",
    "permissions": [],
    "timeout": 300000,
    "eventTypes": [],
    "inputSchema": {
      "type": "object",
      "properties": {
        "source": {"type": "string"},
        "destination": {"type": "string"},
        "recursive": {"type": "boolean", "default": false},
        "preserve": {"type": "boolean", "default": false},
        "force": {"type": "boolean", "default": false}
      },
      "required": ["source", "destination"]
    },
    "argumentMapping": {
      "type": "positional",
      "order": ["source", "destination"],
      "templates": {
        "recursive": {"if_true": "-r", "if_false": ""},
        "preserve": {"if_true": "-p", "if_false": ""},
        "force": {"if_true": "-f", "if_false": ""}
      }
    }
  },
  {
    "name": "pkg-manager",
    "version": "1.0.0",
    "description": "Package management tool with subcommands",
    "executablePath": "/etc/cli/tools/pkg-mgr",
    "permissions": [
      "ohos.permission.INSTALL_BUNDLE",
      "ohos.permission.ACCESS_BUNDLE"
    ],
    "timeout": 300000,
    "eventTypes": ["progress"],
    "hasSubcommands": true,
    "subcommands": {
      "install": {
        "description": "Install a package",
        "inputSchema": {
          "type": "object",
          "properties": {
            "package": {"type": "string"},
            "force": {"type": "boolean", "default": false},
            "silent": {"type": "boolean", "default": false}
          },
          "required": ["package"]
        },
        "argumentMapping": {
          "type": "flag",
          "templates": {
            "package": "{value}",
            "force": "--force",
            "silent": "--silent"
          }
        }
      },
      "remove": {
        "description": "Remove a package",
        "inputSchema": {
          "type": "object",
          "properties": {
            "package": {"type": "string"},
            "purge": {"type": "boolean", "default": false}
          },
          "required": ["package"]
        },
        "argumentMapping": {
          "type": "flag",
          "templates": {
            "package": "{value}",
            "purge": "--purge"
          }
        }
      },
      "list": {
        "description": "List packages",
        "inputSchema": {
          "type": "object",
          "properties": {
            "installed": {"type": "boolean", "default": true},
            "verbose": {"type": "boolean", "default": false}
          }
        },
        "argumentMapping": {
          "type": "flag",
          "templates": {
            "installed": "--installed",
            "verbose": "-v"
          }
        }
      }
    }
  },
  {
    "name": "system-control",
    "version": "1.0.0",
    "description": "System control and monitoring tool",
    "executablePath": "/etc/cli/tools/sysctl",
    "permissions": ["ohos.permission.ACCESS_SYSTEM"],
    "timeout": 60000,
    "eventTypes": [],
    "hasSubcommands": true,
    "subcommands": {
      "status": {
        "description": "Show system status",
        "inputSchema": {
          "type": "object",
          "properties": {
            "detailed": {"type": "boolean", "default": false}
          }
        },
        "argumentMapping": {
          "type": "flag",
          "templates": {
            "detailed": "--detailed"
          }
        }
      },
      "restart": {
        "description": "Restart a service",
        "inputSchema": {
          "type": "object",
          "properties": {
            "service": {"type": "string"},
            "force": {"type": "boolean", "default": false}
          },
          "required": ["service"]
        },
        "argumentMapping": {
          "type": "flag",
          "templates": {
            "service": "{value}",
            "force": "--force"
          }
        }
      }
    }
  }
]
```

### 5.4.2 使用示例

```typescript
import CLI from '@ohos.cli.manager';

// 1. 查询所有可用工具
const tools = await CLI.CLIManager.queryTools();
console.log(`Found ${tools.length} tools`);

// 2. 使用普通工具 - 网络扫描
const scanResult = await CLI.CLIManager.execTool('network-scanner', {
  target: "192.168.1.1",
  ports: [80, 443, 8080],
  verbose: true
});

// 3. 使用长时运行工具 - 日志监控
const logSession = await CLI.CLIManager.execTool('log-monitor', {
  filter: "ERROR",
  level: "ERROR",
  follow: true
}, {
  background: true
});

// 订阅日志输出
CLI.CLIManager.subscribeSession(logSession.sessionId, (event) => {
  console.log(event.data);
});

// 4. 使用子命令工具 - 包管理
await CLI.CLIManager.execTool('pkg-manager', {
  subcommand: 'install',
  package: '/data/downloads/app.hap',
  force: true
});

// 5. 使用子命令工具 - 系统控制
await CLI.CLIManager.execTool('system-control', {
  subcommand: 'restart',
  service: 'climgr',
  force: false
});
```