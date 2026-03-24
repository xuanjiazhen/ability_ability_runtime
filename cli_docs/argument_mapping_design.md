# argumentMapping 参数映射设计详解

## 目录

1. [核心概念](#核心概念)
2. [六种映射类型详解](#六种映射类型详解)
   - [1. flag 模式（最常用）](#1-flag-模式最常用)
     - [Template 语法详解](#template-语法详解)
   - [2. positional 模式（位置参数）](#2-positional-模式位置参数)
   - [3. flattened 模式（展平嵌套）](#3-flattened-模式展平嵌套)
   - [4. json-string 模式（JSON 字符串传递）](#4-json-string-模式json-字符串传递)
   - [5. javascript 模式（自定义映射器）](#5-javascript-模式自定义映射器)
   - [6. mixed 模式（混合映射）](#6-mixed-模式混合映射)
3. [参数映射实现流程](#参数映射实现流程)
4. [总结对比](#总结对比)

---

## 核心概念

### 定义

**`argumentMapping` 的作用**：将抽象的 JSON 参数对象转换为具体的命令行参数字符串。

### 问题场景

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

---

## 六种映射类型详解

### 1. flag 模式（最常用）

基于模板字符串的映射，适用于大多数标准命令行工具。

#### 设计思想

使用模板字符串定义参数格式，支持占位符替换和条件映射。

#### Template 语法详解

**Template 的作用**：定义如何将抽象参数转换为命令行参数字符串。

##### 基本语法

| 语法 | 说明 | 示例模板 | 输入值 | 生成结果 |
|------|------|---------|--------|----------|
| `{value}` | 值占位符 | `"--target={value}"` | `"192.168.1.1"` | `"--target=192.168.1.1"` |
| `if_true` | 布尔 true 时的值 | `{ "if_true": "-v" }` | `true` | `"-v"` |
| `if_false` | 布尔 false 时的值 | `{ "if_false": "" }` | `false` | `""`（空） |

##### Template 格式速查表

| 格式类型 | 模板写法 | 示例输出 | 适用场景 |
|---------|---------|----------|----------|
| **GNU 风格** | `"--key={value}"` | `--target=192.168.1.1` | 标准 GNU 工具 |
| **空格格式** | `"--key {value}"` | `--port 8080` | 传统 Unix 工具 |
| **短标志** | `"-k {value}"` | `-p 8080` | 单字母参数 |
| **等号格式** | `"--key={value}"` | `--timeout=5000` | 带值的标志 |
| **固定前缀** | `"key {value}"` | `appName com.example.abc` | 自定义格式 |
| **无值标志** | `"--flag"` | `--verbose` | 布尔开关 |
| **条件标志** | `{"if_true": "-v"}` | `-v` 或空 | 可选标志 |

##### 不同数据类型的处理

**1. 字符串**

```json
{
  "target": "--target={value}"
}
```

| 输入 | 输出 |
|------|------|
| `"192.168.1.1"` | `"--target=192.168.1.1"` |

**2. 数字（自动转字符串）**

```json
{
  "timeout": "--timeout={value}"
}
```

| 输入 | 输出 |
|------|------|
| `5000` | `"--timeout=5000"` |

**3. 布尔值 - 使用对象语法**

```json
{
  "verbose": {
    "if_true": "-v",
    "if_false": ""
  }
}
```

| 输入 | 输出 |
|------|------|
| `true` | `"-v"` |
| `false` | `""`（不添加） |

**4. 数组 - 展开为多个参数**

```json
{
  "ports": "--port={value}"
}
```

| 输入 | 输出 |
|------|------|
| `[80, 443, 8080]` | `"--port=80"`, `"--port=443"`, `"--port=8080"` |

##### 高级用法示例

**1. 固定前缀 + 可变值**

```json
{
  "bundleName": "appName {value}"
}
```

| 输入 | 生成命令 |
|------|----------|
| `{bundleName: "com.example.abc"}` | `tool appName com.example.abc` |
| `{bundleName: "com.example.def"}` | `tool appName com.example.def` |

**2. 带分隔符的复杂格式**

```json
{
  "config": "--config {value} --enabled"
}
```

| 输入 | 输出 |
|------|------|
| `"app.cfg"` | `"--config app.cfg --enabled"` |

**3. 多单词参数名**

```json
{
  "workingDir": "--working-directory {value}"
}
```

| 输入 | 输出 |
|------|------|
| `"/home/user"` | `"--working-directory /home/user"` |

**4. 混合风格（GNU + 自定义）**

```json
{
  "argumentMapping": {
    "type": "flag",
    "templates": {
      "bundleName": "appName {value}",
      "debug": "--debug",
      "logLevel": "--log-level={value}"
    }
  }
}
```

| 输入 | 生成命令 |
|------|----------|
| `{bundleName: "com.example.abc", debug: true, logLevel: "verbose"}` | `tool appName com.example.abc --debug --log-level=verbose` |

##### Template 实现原理

```cpp
static std::string FormatTemplate(
    const std::string& tmpl,
    const std::string& value)
{
    // 替换 {value} 占位符
    std::string result = tmpl;
    size_t pos = result.find("{value}");
    if (pos != std::string::npos) {
        result.replace(pos, 7, value);  // 7 = strlen("{value}")
    }
    return result;
}
```

##### 常见模式速查

| 需求 | Template 写法 | 示例输出 |
|------|--------------|----------|
| `--key value` | `"--key {value}"` | `--key 8080` |
| `--key=value` | `"--key={value}"` | `--key=8080` |
| `-k value` | `"-k {value}"` | `-k 8080` |
| `key value` | `"key {value}"` | `appName com.example.abc` |
| 仅标志 | `"--flag"` | `--verbose` |
| 条件标志 | `{"if_true": "-v", "if_false": ""}` | `-v` 或 空 |

#### 示例：网络扫描工具

**工具描述文件**：
```json
{
  "name": "network-scanner",
  "executable": "/usr/bin/net-scan",

  "input": {
    "type": "object",
    "properties": {
      "target": {
        "type": "string",
        "description": "目标IP地址"
      },
      "ports": {
        "type": "array",
        "items": {"type": "integer"},
        "description": "端口列表"
      },
      "timeout": {
        "type": "integer",
        "description": "超时时间(毫秒)"
      },
      "verbose": {
        "type": "boolean",
        "description": "详细输出"
      },
      "force": {
        "type": "boolean",
        "description": "强制扫描"
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

**转换逻辑**：
```
target: "192.168.1.1"    → --target=192.168.1.1
ports: [80, 443, 8080]    → --port=80 --port=443 --port=8080  (数组展开)
timeout: 5000            → --timeout=5000
verbose: true            → -v  (使用 if_true)
force: false             → (空，不添加参数)
```

#### 处理规则

| 输入类型 | 处理方式 |
|---------|---------|
| string | 直接替换占位符 |
| number | 转为字符串后替换 |
| boolean | 使用 if_true/if_false |
| array | 遍历展开，每个元素生成一个参数 |
| object | 不支持（使用 flattened 或 json-string 模式） |

---

### 2. positional 模式（位置参数）

按参数顺序排列，适用于传统的位置参数命令。

#### 设计思想

不使用参数名，严格按照定义的顺序传递参数值。

#### 示例：文件拷贝工具

**工具描述文件**：
```json
{
  "name": "file-copy",
  "executable": "/usr/bin/cp-tool",

  "input": {
    "type": "object",
    "properties": {
      "source": {"type": "string"},
      "destination": {"type": "string"},
      "recursive": {"type": "boolean"},
      "verbose": {"type": "boolean"}
    },
    "required": ["source", "destination"]
  },

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

#### 适用场景

- 简单的脚本工具
- 传统 Unix 命令风格
- 参数顺序固定的工具
- 不需要参数名的场景

#### 注意事项

⚠️ **局限性**：
- 参数顺序必须严格遵守
- 缺少可读性
- 不适合参数较多的场景

---

### 3. flattened 模式（展平嵌套）

将嵌套对象展平为带前缀的参数。

#### 设计思想

将嵌套的 JSON 对象路径展平为带分隔符的单层参数名。

#### 分隔符选项

| 分隔符 | 示例 | 说明 |
|--------|------|------|
| `.` (点号) | `app.name` | 推荐，符合常规习惯 |
| `-` (连字符) | `app-name` | 适合 CLI 风格 |
| `_` (下划线) | `app_name` | 编程风格 |

#### 示例：应用部署工具

**工具描述文件**：
```json
{
  "name": "app-deploy",
  "executable": "/usr/bin/app-deploy",

  "input": {
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
              "production": {"type": "boolean"},
              "debug": {"type": "boolean"}
            }
          }
        }
      },
      "server": {
        "type": "object",
        "properties": {
          "host": {"type": "string"},
          "port": {"type": "integer"}
        }
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
      production: true,
      debug: false
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

#### 转换逻辑

```
app.name = "myapp"              → --app-name=myapp
app.version = "1.0.0"           → --app-version=1.0.0
app.env.production = true       → --env-prod
app.env.debug = false           → (不添加，因为没有值)
server.host = "192.168.1.100"   → --host=192.168.1.100
server.port = 8080              → --port=8080
```

#### 嵌套路径展开规则

| JSON 路径 | 分隔符 `.` | 展平结果 |
|-----------|------------|---------|
| `app.name` | `.` | `app.name` |
| `app.env.production` | `.` | `app.env.production` |
| `app.config.ssl.enabled` | `.` | `app.config.ssl.enabled` |

---

### 4. json-string 模式（JSON 字符串传递）

将整个对象序列化为 JSON 字符串传递。

#### 设计思想

保留完整的 JSON 结构，将对象序列化为字符串传递给工具。

#### 序列化选项

| 选项 | 说明 | 示例 |
|------|------|------|
| 标准序列化 | 无格式化 | `{"key":"value"}` |
| 美化序列化 | 带缩进 | `{\n  "key": "value"\n}` |
| Base64 编码 | 编码后传递 | `eyJrZXkiOiJ2YWx1ZSJ9` |

#### 示例：云端部署工具

**工具描述文件**：
```json
{
  "name": "cloud-deploy",
  "executable": "/usr/bin/cloud-deploy",

  "input": {
    "type": "object",
    "properties": {
      "config": {
        "type": "object",
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
              "memory": {"type": "string"}
            }
          }
        }
      },
      "dryRun": {"type": "boolean"}
    }
  },

  "argumentMapping": {
    "type": "json-string",
    "templates": {
      "config": "--config={value}",
      "dryRun": {
        "if_true": "--dry-run",
        "if_false": ""
      }
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
    },
    resources: {
      cpu: "2",
      memory: "4Gi"
    }
  },
  dryRun: true
});
```

**生成的命令行**：
```bash
/usr/bin/cloud-deploy --config='{"application":{"name":"webapp","replicas":3},"resources":{"cpu":"2","memory":"4Gi"}}' --dry-run
```

#### 适用场景

- 配置复杂的工具
- 支持 JSON 配置文件的工具（如 `kubectl`）
- 需要保持嵌套结构的场景
- 云原生工具和容器编排工具

#### 注意事项

⚠️ **转义问题**：
- Shell 转义：使用单引号包裹 JSON 字符串
- 特殊字符：注意引号、空格的转义
- 长度限制：某些系统对命令行长度有限制

---

### 5. javascript 模式（自定义映射器）

使用 JavaScript 代码自定义转换逻辑。

#### 设计思想

提供最大的灵活性，允许使用 JavaScript 代码实现任意复杂的参数转换逻辑。

#### JavaScript 映射器接口

```javascript
function mapper(args) {
  // args: 输入的参数对象
  // 返回值: 字符串数组，每个元素是一个命令行参数

  return ["--arg1", "value1", "--arg2", "value2"];
}
```

#### 示例：复杂的文件处理工具

**工具描述文件**：
```json
{
  "name": "file-processor",
  "executable": "/usr/bin/file-proc",

  "input": {
    "type": "object",
    "properties": {
      "inputs": {
        "type": "array",
        "items": {"type": "string"}
      },
      "output": {"type": "string"},
      "options": {
        "type": "object",
        "properties": {
          "format": {"type": "string"},
          "quality": {"type": "integer"},
          "optimize": {"type": "boolean"}
        }
      }
    }
  },

  "argumentMapping": {
    "type": "javascript",
    "mapper": "function(args) { \
      let result = []; \
      \
      /* 添加输入文件 */ \
      if (args.inputs && args.inputs.length > 0) { \
        args.inputs.forEach(file => { \
          result.push('--input=' + file); \
        }); \
      } \
      \
      /* 添加输出文件 */ \
      if (args.output) { \
        result.push('--output=' + args.output); \
      } \
      \
      /* 处理选项 */ \
      if (args.options) { \
        if (args.options.format) { \
          result.push('--format=' + args.options.format); \
        } \
        if (args.options.quality) { \
          result.push('--quality=' + args.options.quality); \
        } \
        if (args.options.optimize) { \
          result.push('--optimize'); \
        } \
      } \
      \
      return result; \
    }"
  }
}
```

**JS 调用**：
```typescript
await CLI.CLIManager.execTool('file-processor', {
  inputs: ["/data/img1.jpg", "/data/img2.jpg"],
  output: "/output/result.jpg",
  options: {
    format: "jpeg",
    quality: 90,
    optimize: true
  }
});
```

**生成的命令行**：
```bash
/usr/bin/file-proc --input=/data/img1.jpg --input=/data/img2.jpg --output=/output/result.jpg --format=jpeg --quality=90 --optimize
```

#### 高级示例：条件逻辑和复杂处理

```javascript
function mapper(args) {
  let result = [];

  // 根据文件扩展名决定格式
  if (args.output && args.output.endsWith('.jpg')) {
    result.push('--format=jpeg');
    result.push('--quality=' + (args.quality || 85));
  } else if (args.output && args.output.endsWith('.png')) {
    result.push('--format=png');
    result.push('--compression=' + (args.compression || 9));
  }

  // 处理输入文件
  if (args.inputs) {
    args.inputs.forEach((file, index) => {
      result.push('--input');
      result.push(file);

      // 为每个输入文件添加可选的标签
      if (args.labels && args.labels[index]) {
        result.push('--label');
        result.push(args.labels[index]);
      }
    });
  }

  // 条件标志
  if (args.verbose) {
    result.push('-v');
  }

  if (args.dryRun) {
    result.push('--dry-run');
  }

  return result;
}
```

#### 适用场景

- 复杂的条件逻辑
- 需要动态生成参数
- 参数之间存在依赖关系
- 其他模式无法满足的复杂需求

#### 安全注意事项

⚠️ **安全风险**：
- 代码注入：确保 mapper 代码来源可信
- 资源限制：限制执行时间和内存使用
- 沙箱执行：考虑使用受限的 JavaScript 环境

---

### 6. mixed 模式（混合映射）

允许在同一个工具中混合使用多种映射方式，为不同参数应用最适合的映射策略。

#### 设计思想

在实际场景中，一个工具可能同时需要：
- 简单的 flag 参数（如 `--verbose`）
- 位置参数（如输入文件）
- 嵌套配置对象（如服务器配置）
- 复杂的 JSON 配置（如容器编排）

`mixed` 模式允许为每个参数指定最适合的映射方式。

#### 格式定义

```json
{
  "argumentMapping": {
    "type": "mixed",
    "mappings": {
      "<参数名>": {
        "mode": "flag|positional|flattened|json-string",
        "template": "...",
        "separator": ".",
        ...
      },
      ...
    }
  }
}
```

#### 字段说明

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `type` | string | 是 | 固定为 `"mixed"` |
| `mappings` | object | 是 | 参数映射配置表 |
| `mappings.<param>.mode` | string | 是 | 该参数使用的映射模式 |
| `mappings.<param>.template` | string/object | 条件必填 | 模板字符串（flag/flattened/json-string 需要） |
| `mappings.<param>.separator` | string | 否 | 展平分隔符（flattened 模式，默认 `.`） |
| `mappings.<param>.order` | number | 否 | 位置顺序（positional 模式） |

#### 模式选择建议

| 参数类型 | 推荐模式 | 原因 |
|---------|---------|------|
| 简单字符串/数字 | flag | 最直观 |
| 布尔开关 | flag | 条件标志 |
| 数组 | flag | 自动展开 |
| 文件路径 | positional | 符合传统习惯 |
| 嵌套对象 | flattened | 保持层级关系 |
| 复杂配置 | json-string | 保留完整结构 |

#### 示例 1：智能部署工具（基础混合）

**工具描述文件**：
```json
{
  "name": "smart-deploy",
  "executable": "/usr/bin/smart-deploy",

  "input": {
    "type": "object",
    "properties": {
      "configFile": {
        "type": "string",
        "description": "配置文件路径"
      },
      "outputFile": {
        "type": "string",
        "description": "输出文件路径（位置参数）"
      },
      "settings": {
        "type": "object",
        "properties": {
          "server": {
            "type": "object",
            "properties": {
              "host": {"type": "string"},
              "port": {"type": "integer"}
            }
          }
        }
      },
      "metadata": {
        "type": "object",
        "properties": {
          "author": {"type": "string"},
          "version": {"type": "string"}
        }
      },
      "verbose": {
        "type": "boolean",
        "default": false
      }
    }
  },

  "argumentMapping": {
    "type": "mixed",
    "mappings": {
      "configFile": {
        "mode": "flag",
        "template": "--config={value}"
      },
      "outputFile": {
        "mode": "positional",
        "order": 1
      },
      "settings": {
        "mode": "flattened",
        "separator": ".",
        "template": "--{key}={value}"
      },
      "metadata": {
        "mode": "json-string",
        "template": "--meta={json}"
      },
      "verbose": {
        "mode": "flag",
        "template": {
          "if_true": "-v",
          "if_false": ""
        }
      }
    }
  }
}
```

**JS 调用**：
```typescript
await CLI.CLIManager.execTool('smart-deploy', {
  configFile: "/path/to/config.yaml",
  outputFile: "/path/to/output.txt",
  settings: {
    server: {
      host: "localhost",
      port: 8080
    }
  },
  metadata: {
    author: "admin",
    version: "1.0"
  },
  verbose: true
});
```

**生成的命令行**：
```bash
/usr/bin/smart-deploy --config=/path/to/config.yaml /path/to/output.txt --settings.server.host=localhost --settings.server.port=8080 --meta='{"author":"admin","version":"1.0"}' -v
```

#### 示例 2：云原生部署工具（复杂混合）

**场景描述**：
- 配置文件：flag 模式
- 输出目录：位置参数
- 应用配置：flattened 模式
- 资源配置：json-string 模式
- 环境变量：flag 数组展开
- 调试标志：flag 条件

**工具描述文件**：
```json
{
  "name": "cloud-deploy",
  "executable": "/usr/bin/k8s-deploy",

  "input": {
    "type": "object",
    "properties": {
      "manifest": {
        "type": "string",
        "description": "K8s 清单文件"
      },
      "namespace": {
        "type": "string",
        "description": "目标命名空间"
      },
      "outputDir": {
        "type": "string",
        "description": "输出目录（位置参数）"
      },
      "appConfig": {
        "type": "object",
        "properties": {
          "name": {"type": "string"},
          "replicas": {"type": "integer"},
          "image": {"type": "string"}
        }
      },
      "resources": {
        "type": "object",
        "properties": {
          "limits": {
            "type": "object",
            "properties": {
              "cpu": {"type": "string"},
              "memory": {"type": "string"}
            }
          },
          "requests": {
            "type": "object",
            "properties": {
              "cpu": {"type": "string"},
              "memory": {"type": "string"}
            }
          }
        }
      },
      "envVars": {
        "type": "array",
        "items": {"type": "string"},
        "description": "环境变量列表"
      },
      "labels": {
        "type": "object",
        "properties": {
          "app": {"type": "string"},
          "env": {"type": "string"},
          "tier": {"type": "string"}
        }
      },
      "debug": {
        "type": "boolean",
        "default": false
      }
    }
  },

  "argumentMapping": {
    "type": "mixed",
    "mappings": {
      "manifest": {
        "mode": "flag",
        "template": "-f {value}"
      },
      "namespace": {
        "mode": "flag",
        "template": "-n {value}"
      },
      "outputDir": {
        "mode": "positional",
        "order": 1
      },
      "appConfig": {
        "mode": "flattened",
        "separator": ".",
        "templates": {
          "appConfig.name": "--app-name={value}",
          "appConfig.replicas": "--replicas={value}",
          "appConfig.image": "--image={value}"
        }
      },
      "resources": {
        "mode": "json-string",
        "template": "--resources={json}"
      },
      "envVars": {
        "mode": "flag",
        "template": "--env={value}"
      },
      "labels": {
        "mode": "flattened",
        "separator": ".",
        "prefix": "--labels.",
        "templates": {
          "labels.app": "--labels.app={value}",
          "labels.env": "--labels.env={value}",
          "labels.tier": "--labels.tier={value}"
        }
      },
      "debug": {
        "mode": "flag",
        "template": {
          "if_true": "--debug",
          "if_false": ""
        }
      }
    }
  }
}
```

**JS 调用**：
```typescript
await CLI.CLIManager.execTool('cloud-deploy', {
  manifest: "/k8s/deployment.yaml",
  namespace: "production",
  outputDir: "/output/deploy",
  appConfig: {
    name: "webapp",
    replicas: 3,
    image: "myapp:v2.0"
  },
  resources: {
    limits: {
      cpu: "2",
      memory: "4Gi"
    },
    requests: {
      cpu: "1",
      memory: "2Gi"
    }
  },
  envVars: ["DATABASE_URL=mysql://localhost/db", "CACHE_URL=redis://localhost/6379"],
  labels: {
    app: "webapp",
    env: "prod",
    tier: "frontend"
  },
  debug: true
});
```

**生成的命令行**：
```bash
/usr/bin/k8s-deploy -f /k8s/deployment.yaml -n production /output/deploy --app-name=webapp --replicas=3 --image=myapp:v2.0 --resources='{"limits":{"cpu":"2","memory":"4Gi"},"requests":{"cpu":"1","memory":"2Gi"}}' --env=DATABASE_URL=mysql://localhost/db --env=CACHE_URL=redis://localhost/6379 --labels.app=webapp --labels.env=prod --labels.tier=frontend --debug
```

#### 示例 3：文件处理工具（含位置顺序）

**工具描述文件**：
```json
{
  "name": "file-processor",
  "executable": "/usr/bin/file-proc",

  "input": {
    "type": "object",
    "properties": {
      "inputFiles": {
        "type": "array",
        "items": {"type": "string"}
      },
      "outputFile": {
        "type": "string"
      },
      "options": {
        "type": "object",
        "properties": {
          "format": {"type": "string"},
          "quality": {"type": "integer"},
          "optimize": {"type": "boolean"}
        }
      },
      "advanced": {
        "type": "object",
        "properties": {
          "filters": {
            "type": "array",
            "items": {"type": "string"}
          },
          "transformations": {
            "type": "object",
            "properties": {
              "resize": {"type": "boolean"},
              "crop": {"type": "boolean"}
            }
          }
        }
      }
    }
  },

  "argumentMapping": {
    "type": "mixed",
    "mappings": {
      "inputFiles": {
        "mode": "positional",
        "order": [1, 2, 3]
      },
      "outputFile": {
        "mode": "positional",
        "order": 4
      },
      "options": {
        "mode": "flattened",
        "separator": ".",
        "templates": {
          "options.format": "--format={value}",
          "options.quality": "--quality={value}",
          "options.optimize": {
            "if_true": "--optimize",
            "if_false": ""
          }
        }
      },
      "advanced": {
        "mode": "json-string",
        "template": "--advanced={json}"
      }
    }
  }
}
```

**JS 调用**：
```typescript
await CLI.CLIManager.execTool('file-processor', {
  inputFiles: ["/data/img1.jpg", "/data/img2.jpg", "/data/img3.jpg"],
  outputFile: "/output/result.jpg",
  options: {
    format: "jpeg",
    quality: 90,
    optimize: true
  },
  advanced: {
    filters: ["sharpen", "denoise"],
    transformations: {
      resize: true,
      crop: false
    }
  }
});
```

**生成的命令行**：
```bash
/usr/bin/file-proc /data/img1.jpg /data/img2.jpg /data/img3.jpg /output/result.jpg --format=jpeg --quality=90 --optimize --advanced='{"filters":["sharpen","denoise"],"transformations":{"resize":true,"crop":false}}'
```

#### 转换流程

```
1. 读取 inputSchema 和 argumentMapping.mappings
2. 遍历 mappings 中的每个参数配置
3. 根据配置的 mode 选择处理方式：
   ├─ flag: 使用 flag 模式处理
   ├─ positional: 使用位置参数处理
   ├─ flattened: 展平嵌套对象
   ├─ json-string: 序列化为 JSON 字符串
   └─ javascript: 执行自定义映射器
4. 按 positional.order 的顺序排列位置参数
5. 按其他参数在 mappings 中的定义顺序排列
6. 生成最终的 argv 数组
```

#### 参数顺序规则

**mixed 模式的参数顺序**：
1. **位置参数（positional）**：按照 `order` 指定的顺序
2. **其他参数**：按照在 `mappings` 中定义的顺序

**示例**：
```json
{
  "argumentMapping": {
    "type": "mixed",
    "mappings": {
      "configFile": {"mode": "flag", ...},        // 映射中的第1个
      "outputFile": {"mode": "positional", "order": 2},
      "inputFile": {"mode": "positional", "order": 1},
      "verbose": {"mode": "flag", ...}             // 映射中的第2个
    }
  }
}
```

**生成顺序**：
```
tool [inputFile] [outputFile] --config=... -v
     ↑order=1   ↑order=2   ↑mappings[0]  ↑mappings[1]
```

#### 适用场景

- **需要同时使用多种映射方式的工具**
- **参数类型多样的复杂工具**
- **需要灵活控制参数顺序的场景**
- **兼容多种命令行风格的工具**

#### 优缺点分析

**优点**：
- ✅ 最大灵活性，可以为每个参数选择最优映射方式
- ✅ 清晰明确，每个参数的映射方式一目了然
- ✅ 支持复杂场景，不受单一模式限制

**缺点**：
- ❌ 配置相对复杂
- ❌ 需要为每个参数明确指定映射模式
- ❌ 学习成本较高

#### 使用建议

1. **优先使用单一模式**
   - 如果 flag/flattened/json-string 其中一种能满足需求，优先使用单一模式
   - 只在确实需要时才使用 mixed 模式

2. **合理分配模式**
   - 简单参数用 flag
   - 文件路径用 positional
   - 嵌套对象用 flattened 或 json-string

3. **保持配置清晰**
   - 使用注释说明每个参数的映射原因
   - 按逻辑分组相关参数

4. **测试覆盖**
   - 为每种映射类型的参数编写测试
   - 确保混合后的参数顺序符合预期

#### 最佳实践示例

**场景**：一个 CI/CD 部署工具

```json
{
  "name": "ci-deploy",
  "argumentMapping": {
    "type": "mixed",
    "mappings": {
      // 位置参数：必需的核心参数
      "targetEnv": {
        "mode": "positional",
        "order": 1,
        "comment": "目标环境（必填）"
      },
      "buildDir": {
        "mode": "positional",
        "order": 2,
        "comment": "构建目录（必填）"
      },

      // Flag 参数：简单配置
      "branch": {
        "mode": "flag",
        "template": "--branch={value}",
        "comment": "Git 分支"
      },
      "tag": {
        "mode": "flag",
        "template": "--tag={value}",
        "comment": "版本标签"
      },
      "dryRun": {
        "mode": "flag",
        "template": {
          "if_true": "--dry-run",
          "if_false": ""
        },
        "comment": "试运行"
      },

      // Flattened 参数：嵌套配置
      "deployment": {
        "mode": "flattened",
        "separator": ".",
        "templates": {
          "deployment.strategy": "--strategy={value}",
          "deployment.replicas": "--replicas={value}"
        },
        "comment": "部署配置（嵌套）"
      },

      // JSON String 参数：复杂配置
      "advancedConfig": {
        "mode": "json-string",
        "template": "--advanced={json}",
        "comment": "高级配置（复杂结构）"
      }
    }
  }
}
```

---

## 参数映射实现流程

### C++ 实现框架

```cpp
// services/climgr/src/param_mapper.cpp

#include "param_mapper.h"
#include "tool_info.h"
#include <json/json.h>
#include <vector>
#include <string>

namespace OHOS {
namespace CliMgr {

class ParameterMapper {
public:
    /**
     * @brief 将抽象参数转换为命令行参数数组
     * @param toolInfo 工具信息（包含映射规则）
     * @param abstractArgs 抽象参数（来自 JS 调用方）
     * @return 命令行参数数组（用于 execve）
     */
    static std::vector<std::string> MapToCommandLine(
        const ToolInfo& toolInfo,
        const Json::Value& abstractArgs);

private:
    // Flag 模式处理
    static void ApplyFlagMapping(
        const ToolInfo& toolInfo,
        const Json::Value& abstractArgs,
        std::vector<std::string>& argv);

    // Positional 模式处理
    static void ApplyPositionalMapping(
        const ToolInfo& toolInfo,
        const Json::Value& abstractArgs,
        std::vector<std::string>& argv);

    // Flattened 模式处理
    static void ApplyFlattenedMapping(
        const ToolInfo& toolInfo,
        const Json::Value& abstractArgs,
        std::vector<std::string>& argv);

    // JSON String 模式处理
    static void ApplyJsonStringMapping(
        const ToolInfo& toolInfo,
        const Json::Value& abstractArgs,
        std::vector<std::string>& argv);

    // JavaScript 模式处理
    static void ApplyJavaScriptMapping(
        const ToolInfo& toolInfo,
        const Json::Value& abstractArgs,
        std::vector<std::string>& argv);

    // 工具函数：格式化模板
    static std::string FormatTemplate(
        const std::string& tmpl,
        const std::string& value);

    // 工具函数：展平对象
    static void FlattenObject(
        const Json::Value& obj,
        const std::string& prefix,
        const std::string& separator,
        std::vector<std::string>& result);
};

} // namespace CliMgr
} // namespace OHOS
```

### 详细实现

```cpp
std::vector<std::string> ParameterMapper::MapToCommandLine(
    const ToolInfo& toolInfo,
    const Json::Value& abstractArgs)
{
    std::vector<std::string> argv;
    argv.push_back(toolInfo.executablePath);  // argv[0]

    auto& mapping = toolInfo.argMapping;

    // 根据映射类型选择处理方式
    if (mapping.type == "flag") {
        ApplyFlagMapping(toolInfo, abstractArgs, argv);
    }
    else if (mapping.type == "positional") {
        ApplyPositionalMapping(toolInfo, abstractArgs, argv);
    }
    else if (mapping.type == "flattened") {
        ApplyFlattenedMapping(toolInfo, abstractArgs, argv);
    }
    else if (mapping.type == "json-string") {
        ApplyJsonStringMapping(toolInfo, abstractArgs, argv);
    }
    else if (mapping.type == "javascript") {
        ApplyJavaScriptMapping(toolInfo, abstractArgs, argv);
    }
    else {
        // 默认使用 flag 模式
        ApplyFlagMapping(toolInfo, abstractArgs, argv);
    }

    return argv;
}

void ParameterMapper::ApplyFlagMapping(
    const ToolInfo& toolInfo,
    const Json::Value& abstractArgs,
    std::vector<std::string>& argv)
{
    auto& templates = toolInfo.argMapping.templates;

    for (const auto& key : abstractArgs.getMemberNames()) {
        // 检查是否有对应的模板
        if (!templates.isMember(key)) {
            continue;
        }

        Json::Value value = abstractArgs[key];
        auto& template = templates[key];

        if (value.isString()) {
            // 字符串：--key=value
            std::string formatted = FormatTemplate(
                template.asString(),
                value.asString()
            );
            argv.push_back(formatted);
        }
        else if (value.isInt() || value.isUInt() ||
                 value.isInt64() || value.isUInt64()) {
            // 整数：--key=value
            std::string formatted = FormatTemplate(
                template.asString(),
                std::to_string(value.asInt64())
            );
            argv.push_back(formatted);
        }
        else if (value.isBool()) {
            // 布尔：if_true 或 if_false
            std::string flag = value.asBool()
                ? template["if_true"].asString()
                : template["if_false"].asString();
            if (!flag.empty()) {
                argv.push_back(flag);
            }
        }
        else if (value.isArray()) {
            // 数组：--key=v1 --key=v2
            for (const auto& item : value) {
                std::string itemValue;
                if (item.isString()) {
                    itemValue = item.asString();
                } else if (item.isIntegral()) {
                    itemValue = std::to_string(item.asInt64());
                }
                std::string formatted = FormatTemplate(
                    template.asString(),
                    itemValue
                );
                argv.push_back(formatted);
            }
        }
    }
}

void ParameterMapper::ApplyPositionalMapping(
    const ToolInfo& toolInfo,
    const Json::Value& abstractArgs,
    std::vector<std::string>& argv)
{
    auto& order = toolInfo.argMapping.order;

    for (const auto& key : order) {
        if (!abstractArgs.isMember(key)) {
            continue;
        }

        Json::Value value = abstractArgs[key];
        std::string strValue;

        if (value.isString()) {
            strValue = value.asString();
        } else if (value.isBool()) {
            strValue = value.asBool() ? "true" : "false";
        } else if (value.isIntegral()) {
            strValue = std::to_string(value.asInt64());
        } else if (value.isDouble()) {
            strValue = std::to_string(value.asDouble());
        }

        argv.push_back(strValue);
    }
}

void ParameterMapper::ApplyFlattenedMapping(
    const ToolInfo& toolInfo,
    const Json::Value& abstractArgs,
    std::vector<std::string>& argv)
{
    std::string separator = toolInfo.argMapping.separator;
    if (separator.empty()) {
        separator = ".";
    }

    std::vector<std::string> flattenedPaths;
    FlattenObject(abstractArgs, "", separator, flattenedPaths);

    auto& templates = toolInfo.argMapping.templates;

    for (const auto& path : flattenedPaths) {
        // 从路径中提取键和值
        size_t lastSep = path.find_last_of('=');
        if (lastSep == std::string::npos) {
            continue;
        }

        std::string key = path.substr(0, lastSep);
        std::string value = path.substr(lastSep + 1);

        if (!templates.isMember(key)) {
            continue;
        }

        std::string formatted = FormatTemplate(
            templates[key].asString(),
            value
        );
        argv.push_back(formatted);
    }
}

void ParameterMapper::ApplyJsonStringMapping(
    const ToolInfo& toolInfo,
    const Json::Value& abstractArgs,
    std::vector<std::string>& argv)
{
    auto& templates = toolInfo.argMapping.templates;

    for (const auto& key : abstractArgs.getMemberNames()) {
        if (!templates.isMember(key)) {
            continue;
        }

        Json::Value value = abstractArgs[key];
        std::string strValue;

        if (value.isObject()) {
            // 对象：序列化为 JSON 字符串
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            strValue = Json::writeString(builder, value);
        }
        else if (value.isString()) {
            strValue = value.asString();
        }
        else if (value.isBool()) {
            if (templates[key].isObject()) {
                strValue = value.asBool()
                    ? templates[key]["if_true"].asString()
                    : templates[key]["if_false"].asString();
            } else {
                strValue = value.asBool() ? "true" : "false";
            }
        }
        else if (value.isIntegral()) {
            strValue = std::to_string(value.asInt64());
        }

        if (!strValue.empty()) {
            std::string formatted = FormatTemplate(
                templates[key].asString(),
                strValue
            );
            argv.push_back(formatted);
        }
    }
}

void ParameterMapper::ApplyJavaScriptMapping(
    const ToolInfo& toolInfo,
    const Json::Value& abstractArgs,
    std::vector<std::string>& argv)
{
    // 获取 JavaScript 映射器代码
    std::string mapperCode = toolInfo.argMapping.mapper;

    // TODO: 使用 JavaScript 引擎执行映射器
    // 这里需要集成 JavaScript 引擎（如 QuickJS 或 V8）
    //
    // 伪代码：
    // std::vector<std::string> result = jsEngine->Execute(mapperCode, abstractArgs);
    // argv.insert(argv.end(), result.begin(), result.end());
}

std::string ParameterMapper::FormatTemplate(
    const std::string& tmpl,
    const std::string& value)
{
    std::string result = tmpl;
    size_t pos = result.find("{value}");

    if (pos != std::string::npos) {
        result.replace(pos, 7, value);
    }

    return result;
}

void ParameterMapper::FlattenObject(
    const Json::Value& obj,
    const std::string& prefix,
    const std::string& separator,
    std::vector<std::string>& result)
{
    for (const auto& key : obj.getMemberNames()) {
        std::string fullKey = prefix.empty() ? key : prefix + separator + key;
        Json::Value value = obj[key];

        if (value.isObject() && !value.isNull()) {
            // 递归处理嵌套对象
            FlattenObject(value, fullKey, separator, result);
        }
        else if (value.isBool()) {
            result.push_back(fullKey + "=" + (value.asBool() ? "true" : "false"));
        }
        else if (value.isIntegral()) {
            result.push_back(fullKey + "=" + std::to_string(value.asInt64()));
        }
        else if (value.isString()) {
            result.push_back(fullKey + "=" + value.asString());
        }
    }
}

} // namespace CliMgr
} // namespace OHOS
```

---

## 总结对比

### 六种映射类型对比表

| 类型 | 适用场景 | 优点 | 缺点 | 配置复杂度 | 推荐指数 |
|------|---------|------|------|-----------|---------|
| **flag** | 标准命令行工具 | 简单直观，可读性强 | 不支持 object 类型 | ⭐⭐ | ⭐⭐⭐⭐⭐ |
| **positional** | 传统位置参数 | 按顺序自然排列 | 缺乏灵活性，参数顺序固定 | ⭐ | ⭐⭐ |
| **flattened** | 嵌套配置对象 | 保持结构清晰 | 需要手动定义所有路径 | ⭐⭐⭐ | ⭐⭐⭐⭐ |
| **json-string** | 复杂配置 | 保留完整结构 | 命令行长度可能很长 | ⭐⭐ | ⭐⭐⭐ |
| **javascript** | 复杂自定义逻辑 | 最灵活 | 可维护性差，安全风险 | ⭐⭐⭐⭐⭐ | ⭐⭐ |
| **mixed** | 需要多种映射方式 | 最大灵活性 | 配置最复杂 | ⭐⭐⭐⭐⭐ | ⭐⭐⭐⭐ |

### 选择建议

#### 使用 flag 模式（推荐）

**适合场景**：
- 大多数标准 CLI 工具
- 参数类型多样（字符串、数字、布尔、数组）
- 需要清晰的参数名

**示例工具**：
- `net-scan`：网络扫描工具
- `log-monitor`：日志监控工具
- `file-copy`：文件拷贝工具

#### 使用 positional 模式

**适合场景**：
- 简单的脚本工具
- 参数顺序固定且数量少（≤ 3 个）
- 不需要参数名的工具

**示例工具**：
- `cp-tool source dest`：简单拷贝
- `move-tool src dst`：文件移动

#### 使用 flattened 模式

**适合场景**：
- 配置有层级结构
- 需要保持配置的可读性
- 参数命名有明确的层级关系

**示例工具**：
- `app-deploy`：应用部署（app.config、server.config）
- `system-config`：系统配置工具

#### 使用 json-string 模式

**适合场景**：
- 配置非常复杂（深度嵌套）
- 工具原生支持 JSON 配置
- 云原生、容器化工具

**示例工具**：
- `cloud-deploy`：云部署（类似 kubectl）
- `k8s-deploy`：Kubernetes 部署

#### 使用 javascript 模式

**适合场景**：
- 参数之间存在复杂依赖
- 需要动态生成参数
- 其他模式无法满足的特殊需求

**示例工具**：
- `batch-processor`：批量处理（需要根据输入类型动态调整参数）
- `smart-deploy`：智能部署（根据环境自动调整配置）

#### 使用 mixed 模式

**适合场景**：
- 需要同时使用多种映射方式
- 参数类型多样的复杂工具
- 需要灵活控制参数顺序
- 兼容多种命令行风格

**示例工具**：
- `ci-deploy`：CI/CD 部署工具（位置参数 + flag + json-string）
- `smart-deploy`：智能部署工具（flag + positional + flattened + json-string）
- `cloud-deploy`：云原生部署工具（多种映射方式组合）

### 最佳实践

1. **优先使用单一模式**
   - flag 模式覆盖 80% 的使用场景
   - 只在确实需要时才使用 mixed 模式
   - 单一模式配置更简单，可维护性更好

2. **合理使用 mixed 模式**
   - 当确实需要混合多种映射方式时使用
   - 为每个参数选择最适合的映射模式
   - 使用注释说明参数映射的原因
   - 按逻辑分组相关参数

3. **避免过度使用 javascript 模式**
   - 安全风险
   - 可维护性差
   - 难以调试

4. **参数命名规范**
   - 使用小写字母和连字符：`--max-count`
   - 避免使用特殊字符
   - 保持命名的一致性

5. **复杂配置考虑文件方式**
   - 对于超长配置，考虑使用配置文件
   - 使用 `--config-file=/path/to/config.json`

6. **测试覆盖**
   - 为每种参数类型编写测试用例
   - 测试边界情况（空值、特殊字符等）
   - mixed 模式要测试参数顺序

7. **flag 模式扩展 object 支持**
   - 对于需要处理 object 的场景，考虑使用 `autoFlatten` 选项
   ```json
   {
     "argumentMapping": {
       "type": "flag",
       "autoFlatten": true,
       "flattenSeparator": ".",
       "templates": {
         "server.host": "--host={value}",
         "server.port": "--port={value}"
       }
     }
   }
   ```

---

## 附录

### A. 完整示例对比

#### 同一工具的不同映射方式

**工具**：文件压缩工具

**输入参数**：
```typescript
{
  input: "/data/file.txt",
  output: "/data/file.txt.gz",
  level: 9,
  fast: false
}
```

**flag 模式**：
```bash
/usr/bin/compress --input=/data/file.txt --output=/data/file.txt.gz --level=9
```

**positional 模式**：
```bash
/usr/bin/compress /data/file.txt /data/file.txt.gz 9 false
```

**flattened 模式**（假设有嵌套配置）：
```bash
/usr/bin/compress --input=/data/file.txt --output=/data/file.txt.gz --compress.level=9
```

### B. 常见问题解答

**Q: 如何处理特殊字符？**
A: 使用 shell 转义规则，必要时使用引号包裹。

**Q: 数组参数如何处理？**

A: 根据映射类型不同：
- flag: 遍历展开为多个参数
- json-string: 保持数组结构
- javascript: 自定义处理逻辑

**Q: 如何处理可选参数？**
A: 在 inputSchema 中不加入 required，映射时会自动跳过。

**Q: 参数顺序有要求吗？**
A: 除了 positional 模式，其他模式不强制顺序。

### C. 相关文档

- [DESIGN.md](./DESIGN.md) - 完整设计文档
- [README.md](./README.md) - 文档索引
- [example_tool_descriptor.json](./example_tool_descriptor.json) - 基础示例
- [examples/](./examples/) - 更多完整示例

---

**文档版本**: v1.2
**最后更新**: 2025-04-16
**更新内容**:
- 新增第6章：mixed 模式（混合映射）完整说明
- 补充 flag 模式对 object 类型的支持方案（autoFlatten）
- 更新总结对比表，从5种扩展到6种映射类型
- 新增 mixed 模式的使用建议和最佳实践
**作者**: CLI 工具管理框架设计团队
