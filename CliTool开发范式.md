# 3. 参数设计

## 3.1 参数类型支持

### 3.1.1 基于标准 JSON Schema

工具描述文件的 `inputSchema` 和 `outputSchema` 采用 JSON Schema 的受限子集，用于描述参数结构、基本约束和结果格式。

当前支持的基础类型共 6 种：

- `string`
- `number`
- `integer`
- `boolean`
- `array`
- `object`

### 3.1.2 通用字段说明

以下字段可用于多种类型：

| 字段 | 适用类型 | 含义 |
|------|---------|------|
| `type` | 全部 | 参数类型标识 |
| `description` | 全部 | 参数说明，用于文档展示和动态构造 UI |
| `default` | 多数类型 | 默认值提示，不等价于框架自动补值 |

### 3.1.3 String（字符串）

**支持字段：**

| 字段 | 含义 |
|------|------|
| `type: "string"` | 声明该参数是字符串 |
| `description` | 字符串参数说明 |
| `default` | 默认字符串值 |
| `minLength` | 最小长度 |
| `maxLength` | 最大长度 |
| `pattern` | 正则表达式约束 |
| `format` | 标准格式提示或校验约束 |

**支持的 `format`：**

- `ipv4`：应符合 JSON Schema 定义的 IPv4 地址格式
- `ipv6`：应符合 JSON Schema 定义的 IPv6 地址格式
- `hostname`：应符合 JSON Schema 定义的主机名格式
- `email`：应符合 JSON Schema 定义的邮箱地址格式
- `uri`：应符合 JSON Schema 定义的 URI 格式
- `date`：应符合 RFC 3339 中 `full-date` 定义，即 `YYYY-MM-DD`
- `date-time`：应符合 RFC 3339 中 `date-time` 定义，即完整日期时间表示，包含时间和时区信息

说明：

- `date` 和 `date-time` 的语义采用 JSON Schema 对 RFC 3339 的引用定义
- `date` 不是“看起来像日期字符串”即可，还应是合法日历日期
- `date-time` 应包含时区信息，例如 `Z` 或 `+08:00`

实现建议：

- `format` 校验应尽量遵循 JSON Schema 的标准语义，不建议随意自定义解释
- 对 `date`、`date-time`、`uri`、`ipv6` 这类规则较复杂的格式，优先使用标准库或成熟解析器校验
- 若实现中同时支持 `pattern` 和 `format`，建议要求字符串同时满足两者约束

**示例：**

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

### 3.1.4 Number（数字）

**支持字段：**

| 字段 | 含义 |
|------|------|
| `type: "number"` | 可包含小数的数字 |
| `description` | 参数说明 |
| `default` | 默认数值 |
| `minimum` | 最小值，包含边界 |
| `maximum` | 最大值，包含边界 |
| `exclusiveMinimum` | 严格大于某值 |
| `exclusiveMaximum` | 严格小于某值 |
| `multipleOf` | 必须是指定数值的倍数 |

**示例：**

```json
{
  "properties": {
    "ratio": {
      "type": "number",
      "description": "采样比例",
      "minimum": 0,
      "maximum": 1,
      "default": 0.5,
      "multipleOf": 0.1
    }
  }
}
```

### 3.1.5 Integer（整数）

**支持字段：**

| 字段 | 含义 |
|------|------|
| `type: "integer"` | 整数参数 |
| `description` | 参数说明 |
| `default` | 默认整数值 |
| `minimum` | 最小值，包含边界 |
| `maximum` | 最大值，包含边界 |
| `exclusiveMinimum` | 严格大于某值 |
| `exclusiveMaximum` | 严格小于某值 |
| `multipleOf` | 必须是指定整数的倍数 |

**示例：**

```json
{
  "properties": {
    "count": {
      "type": "integer",
      "description": "重复次数",
      "minimum": 1,
      "maximum": 100,
      "default": 1,
      "multipleOf": 1
    }
  }
}
```

### 3.1.6 Boolean（布尔值）

**支持字段：**

| 字段 | 含义 |
|------|------|
| `type: "boolean"` | 布尔参数 |
| `description` | 参数说明 |
| `default` | 默认布尔值 |

**示例：**

```json
{
  "properties": {
    "verbose": {
      "type": "boolean",
      "description": "是否开启详细输出",
      "default": false
    }
  }
}
```

### 3.1.7 Array（数组）

**支持字段：**

| 字段 | 含义 |
|------|------|
| `type: "array"` | 数组参数 |
| `description` | 参数说明 |
| `default` | 默认数组值 |
| `items` | 数组元素的 Schema 定义 |
| `minItems` | 最少元素个数 |
| `maxItems` | 最多元素个数 |
| `uniqueItems` | 是否要求元素唯一 |

**示例：**

```json
{
  "properties": {
    "ports": {
      "type": "array",
      "description": "待扫描端口列表",
      "items": {"type": "integer", "minimum": 1, "maximum": 65535},
      "minItems": 1,
      "maxItems": 100,
      "uniqueItems": true,
      "default": [80, 443]
    }
  }
}
```

### 3.1.8 Object（对象）

**支持字段：**

| 字段 | 含义 |
|------|------|
| `type: "object"` | 对象参数 |
| `description` | 参数说明 |
| `properties` | 对象内部字段定义 |
| `required` | 必填字段列表 |
| `additionalProperties` | 是否允许未声明字段 |
| `minProperties` | 最少属性个数 |
| `maxProperties` | 最多属性个数 |

**示例：**

```json
{
  "properties": {
    "config": {
      "type": "object",
      "description": "连接配置",
      "properties": {
        "host": {"type": "string"},
        "port": {"type": "integer"},
        "ssl": {"type": "boolean"}
      },
      "required": ["host", "port"],
      "additionalProperties": false,
      "minProperties": 2,
      "maxProperties": 3
    }
  }
}
```

## 3.2 参数映射规则

### 3.2.1 支持的映射类型

工具描述文件通过 `argumentMapping` 将抽象参数对象转换为命令行参数。

当前仅支持 4 种映射类型：

| 映射类型 | 说明 | 典型输出 |
|----------|------|----------|
| `flag` | 标志模式 | `--key=value`、`-v` |
| `positional` | 位置参数模式 | `arg1 arg2 arg3` |
| `flattened` | 展平嵌套对象 | `app.name -> --app-name=value` |
| `json-string` | 对象序列化为 JSON 字符串 | `--config='{"key":"value"}'` |

### 3.2.2 通用规则

`argumentMapping` 的目标是生成进程启动时使用的 `argv[1...]`，其中：

- `argv[0]` 固定为工具的 `executablePath`
- 映射规则只负责参数组织，不替代 `inputSchema` 的合法性校验
- 只处理调用参数中实际出现且映射规则中定义过的字段

模板语法：

| 写法 | 含义 | 示例 |
|------|------|------|
| `{value}` | 参数值占位符 | `--target={value}` |
| `if_true` | 布尔值为 `true` 时使用的模板 | `{ "if_true": "-v" }` |
| `if_false` | 布尔值为 `false` 时使用的模板 | `{ "if_false": "" }` |

### 3.2.3 flag 模式

适用于大多数标准 CLI 工具。

**处理规则：**

| 输入类型 | 处理方式 |
|---------|---------|
| `string` | 替换 `{value}` |
| `number` / `integer` | 转成字符串后替换 `{value}` |
| `boolean` | 使用 `if_true` / `if_false` |
| `array` | 遍历数组，每个元素生成一个参数 |
| `object` | 不建议直接使用，推荐改用 `flattened` 或 `json-string` |

**工具描述：**

```json
{
  "name": "network-scanner",
  "executablePath": "/usr/bin/net-scan",
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
  }
}
```

**JS 输入：**

```javascript
{
  target: "192.168.1.1",
  ports: [80, 443],
  timeout: 5000,
  verbose: true
}
```

**最终 `argv`：**

```cpp
argv[0] = "/usr/bin/net-scan"
argv[1] = "--target=192.168.1.1"
argv[2] = "--port=80"
argv[3] = "--port=443"
argv[4] = "--timeout=5000"
argv[5] = "-v"
argv[6] = nullptr
```

**执行命令：**

```bash
/usr/bin/net-scan --target=192.168.1.1 --port=80 --port=443 --timeout=5000 -v
```

### 3.2.4 positional 模式

按 `order` 定义的顺序依次追加参数值，不生成参数名。

**适用场景：**

- 传统 Unix 风格工具
- 参数顺序固定的脚本
- 参数较少且含义明确的场景

**工具描述：**

```json
{
  "name": "file-copy",
  "executablePath": "/usr/bin/cp-tool",
  "argumentMapping": {
    "type": "positional",
    "order": ["source", "destination", "verbose"]
  }
}
```

**JS 输入：**

```javascript
{
  source: "/data/file.txt",
  destination: "/backup/file.txt",
  verbose: true
}
```

**最终 `argv`：**

```cpp
argv[0] = "/usr/bin/cp-tool"
argv[1] = "/data/file.txt"
argv[2] = "/backup/file.txt"
argv[3] = "true"
argv[4] = nullptr
```

**执行命令：**

```bash
/usr/bin/cp-tool /data/file.txt /backup/file.txt true
```

### 3.2.5 flattened 模式

将嵌套对象递归展平为路径形式，再按路径匹配模板。

**分隔符：**

- 默认使用 `.`
- 也可根据实现支持 `-` 或 `_`

**工具描述：**

```json
{
  "name": "app-deploy",
  "executablePath": "/usr/bin/app-deploy",
  "argumentMapping": {
    "type": "flattened",
    "separator": ".",
    "templates": {
      "app.name": "--app-name={value}",
      "app.env.production": "--env-prod",
      "server.host": "--host={value}",
      "server.port": "--port={value}"
    }
  }
}
```

**JS 输入：**

```javascript
{
  app: {
    name: "myapp",
    env: {
      production: true
    }
  },
  server: {
    host: "192.168.1.100",
    port: 8080
  }
}
```

**最终 `argv`：**

```cpp
argv[0] = "/usr/bin/app-deploy"
argv[1] = "--app-name=myapp"
argv[2] = "--env-prod"
argv[3] = "--host=192.168.1.100"
argv[4] = "--port=8080"
argv[5] = nullptr
```

**执行命令：**

```bash
/usr/bin/app-deploy --app-name=myapp --env-prod --host=192.168.1.100 --port=8080
```

### 3.2.6 json-string 模式

保留对象或数组的完整结构，序列化为 JSON 字符串后传递给工具。

**适用场景：**

- 配置对象层级较深
- 工具本身支持 `--config=<json>`
- 不希望使用路径展开

**工具描述：**

```json
{
  "name": "cloud-deploy",
  "executablePath": "/usr/bin/cloud-deploy",
  "argumentMapping": {
    "type": "json-string",
    "templates": {
      "config": "--config={value}",
      "dryRun": {"if_true": "--dry-run", "if_false": ""}
    }
  }
}
```

**JS 输入：**

```javascript
{
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
}
```

**最终 `argv`：**

```cpp
argv[0] = "/usr/bin/cloud-deploy"
argv[1] = "--config={\"application\":{\"name\":\"webapp\",\"replicas\":3},\"resources\":{\"cpu\":\"2\",\"memory\":\"4Gi\"}}"
argv[2] = "--dry-run"
argv[3] = nullptr
```

**执行命令：**

```bash
/usr/bin/cloud-deploy --config='{"application":{"name":"webapp","replicas":3},"resources":{"cpu":"2","memory":"4Gi"}}' --dry-run
```

### 3.2.7 `inputSchema` 与 `argumentMapping` 的协同关系

`inputSchema` 和 `argumentMapping` 分别解决两个不同问题：

- `inputSchema` 定义“参数长什么样”
- `argumentMapping` 定义“参数如何变成命令行”

两者需要配合使用，但职责不能混淆。

#### 职责划分

| 组件 | 作用 |
|------|------|
| `inputSchema` | 描述参数类型、结构、必填项和基础约束 |
| `argumentMapping` | 描述参数到命令行参数的转换方式 |

#### 协同原则

- `inputSchema` 中定义的字段，才允许在 `argumentMapping` 中被引用
- `argumentMapping` 不负责定义字段类型，只负责组织命令行参数
- `inputSchema.required` 表示调用方必须提供哪些字段，但是否最终映射到命令行由 `argumentMapping` 决定
- `inputSchema` 中的嵌套结构会影响应选择哪种映射方式

#### 常见对应关系

| `inputSchema` 中的字段类型 | 推荐映射方式 | 说明 |
|--------------------------|-------------|------|
| 顶层 `string` / `number` / `integer` | `flag` 或 `positional` | 最常见 |
| 顶层 `boolean` | `flag` | 常与 `if_true` / `if_false` 配合 |
| 顶层 `array` | `flag` | 通常展开为多个同名参数 |
| 嵌套 `object` | `flattened` | 将路径映射为命令行参数 |
| 复杂 `object` / `array` | `json-string` | 保留结构整体传递 |

#### 设计示例

```json
{
  "inputSchema": {
    "type": "object",
    "properties": {
      "target": {
        "type": "string",
        "format": "ipv4"
      },
      "ports": {
        "type": "array",
        "items": {"type": "integer"}
      },
      "verbose": {
        "type": "boolean",
        "default": false
      }
    },
    "required": ["target"]
  },
  "argumentMapping": {
    "type": "flag",
    "templates": {
      "target": "--target={value}",
      "ports": "--port={value}",
      "verbose": {"if_true": "-v", "if_false": ""}
    }
  }
}
```

在这个例子中：

- `inputSchema` 决定 `target` 是必填字符串，且应符合 IPv4 格式
- `ports` 是整数数组，因此在 `flag` 模式下会展开为多个 `--port`
- `verbose` 是布尔值，因此映射为条件开关 `-v`

#### 设计建议

- 先设计 `inputSchema`，再设计 `argumentMapping`
- 若字段无法自然映射到命令行，优先调整映射方式，而不是弱化 `inputSchema`
- 对嵌套对象，不要勉强使用 `flag`，应优先考虑 `flattened` 或 `json-string`
- 文档、校验器和实现应统一保证：`argumentMapping` 只能引用 `inputSchema` 中真实存在的字段

## 3.3 参数传递流程

### 3.3.1 总流程

从调用方传参到最终进程启动，完整流程如下：

```text
1. 调用方根据 inputSchema 构造参数对象
   ↓
2. CLIManager 读取目标工具信息
   ↓
3. 读取 argumentMapping 配置
   ↓
4. 根据 argumentMapping.type 选择映射策略
   ↓
5. 将参数对象转换为 argv[1...n]
   ↓
6. 在 argv[0] 填入 executablePath
   ↓
7. 启动工具进程
```

### 3.3.2 四种映射方式对比

| 映射类型 | 适用场景 | 数组处理 | 嵌套对象处理 | 配置复杂度 |
|---------|---------|---------|-------------|-----------|
| `flag` | 标准命令行工具 | 展开为多个参数 | 不支持 | 低 |
| `positional` | 传统位置参数 | 按实现约定处理 | 不支持 | 低 |
| `flattened` | 层级配置 | 不推荐直接使用数组 | 展平为路径后处理 | 中 |
| `json-string` | 复杂配置 | 保持数组结构 | 序列化为 JSON 字符串 | 中 |

# 4. 工具注册表设计

## 4.1 目录结构

CLI 服务的工具统一安装在 `/etc/cli/` 目录下：

```text
/etc/cli/
├── cli_tools.json      # 工具注册表
└── tools/              # 工具可执行文件目录
    ├── net-scan
    ├── log-mon
    ├── app-deploy
    ├── pkg-mgr
    └── sysctl
```

## 4.2 注册表格式

`cli_tools.json` 是一个 JSON 数组，每个元素都是一个工具的完整描述信息。

```json
[
  { "name": "tool-a", "...": "..." },
  { "name": "tool-b", "...": "..." }
]
```

## 4.3 基本约束

- 每个工具都必须有唯一 `name`
- 每个工具都必须有 `executablePath`
- 注册表中的每个工具必须符合第 5 章的工具描述格式

# 5. CLI 工具描述设计

## 5.1 概述

当前支持两种工具格式：

| 格式类型 | 适用场景 | 特征 |
|---------|---------|------|
| 无子命令工具 | 单一功能工具 | 顶层定义 `inputSchema`、`outputSchema`、`argumentMapping` |
| 有子命令工具 | 多功能集合工具 | 使用 `hasSubcommands: true` 和 `subcommands` |

说明：

- 当前不支持“顶层既有主命令参数，又同时定义业务子命令参数”的混合格式
- 需要多功能时，应统一使用子命令工具格式

## 5.2 公共字段

以下字段适用于所有工具：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `name` | string | 是 | 工具唯一标识 |
| `version` | string | 是 | 工具版本号 |
| `description` | string | 是 | 工具描述 |
| `executablePath` | string | 是 | 可执行文件绝对路径 |
| `permissions` | string[] | 否 | 所需权限列表 |
| `timeout` | number | 否 | 超时时间，单位毫秒，`0` 表示不限制 |
| `eventTypes` | string[] | 否 | 支持的事件类型 |
| `eventSchemas` | object | 否 | 自定义事件的数据 Schema |
| `hasSubcommands` | boolean | 否 | 是否为子命令工具 |

### 5.2.1 `eventTypes` 与 `eventSchemas`

`eventTypes` 和 `eventSchemas` 用于描述工具在运行过程中可能向框架发出的事件。

两者职责如下：

| 字段 | 作用 |
|------|------|
| `eventTypes` | 声明工具支持哪些事件类型 |
| `eventSchemas` | 为需要结构化校验的事件定义数据格式 |

#### `eventTypes`

`eventTypes` 是字符串数组，用于声明工具可能产生的事件类型。

当前推荐使用的事件类型包括：

| 事件类型 | 含义 | 是否需要在 `eventSchemas` 中定义 |
|---------|------|-------------------------------|
| `stdout` | 普通标准输出文本 | 否 |
| `stderr` | 标准错误输出文本 | 否 |
| `exit` | 进程退出事件 | 否 |
| `error` | 运行或解析错误事件 | 否 |
| `progress` | 进度更新事件 | 是，建议定义 `eventSchemas.progress` |
| `result` | 最终结果事件 | 否，使用 `outputSchema` 描述 |

说明：

- `stdout`、`stderr`、`exit`、`error` 更偏框架级事件，一般不需要再定义结构化 Schema
- `progress` 这类业务事件通常需要结构化字段，建议显式定义 `eventSchemas`
- `result` 表示最终业务结果，其数据结构优先由 `outputSchema` 描述

#### `eventSchemas`

`eventSchemas` 是一个对象，key 为事件类型，value 为该事件 `data` 字段对应的 JSON Schema。

示例：

```json
{
  "eventTypes": ["progress", "result"],
  "eventSchemas": {
    "progress": {
      "type": "object",
      "properties": {
        "percentage": {"type": "integer"},
        "status": {"type": "string"}
      },
      "required": ["percentage", "status"]
    }
  }
}
```

在这个例子中：

- 工具声明自己会发出 `progress` 和 `result`
- `progress` 的 `data` 必须符合 `eventSchemas.progress`
- `result` 的 `data` 必须符合 `outputSchema`

#### 约束关系

- `eventSchemas` 中出现的事件类型，必须同时出现在 `eventTypes` 中
- 若 `eventTypes` 包含 `progress`，建议同时定义 `eventSchemas.progress`
- 若 `eventTypes` 包含 `result`，建议同时定义 `outputSchema`
- `eventSchemas` 只描述事件 `data` 的结构，不描述最外层的 `event` 包装对象

#### 简单工具与子命令工具中的使用方式

- 无子命令工具：`eventTypes` 和 `eventSchemas` 定义在工具顶层
- 有子命令工具：可在顶层定义通用事件，也可在 `subcommands.<name>` 中为特定子命令单独定义
- 如果子命令定义了自己的 `eventTypes` / `eventSchemas`，应优先使用子命令级定义

## 5.3 无子命令工具格式

### 5.3.1 结构要求

无子命令工具必须满足：

- `hasSubcommands` 不存在或为 `false`
- 顶层必须存在 `inputSchema`
- 顶层必须存在 `argumentMapping`
- 顶层可选 `outputSchema`
- 顶层不应存在 `subcommands`

### 5.3.2 字段表

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `inputSchema` | object | 是 | 输入参数 Schema |
| `outputSchema` | object | 否 | 结果 Schema |
| `argumentMapping` | object | 是 | 参数映射规则 |

### 5.3.3 最小示例

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
  "timeout": 30000,
  "eventTypes": ["progress", "result"],
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
    "properties": {
      "summary": {
        "type": "object",
        "properties": {
          "totalPorts": {"type": "integer"},
          "openPorts": {"type": "integer"}
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
  "eventSchemas": {
    "progress": {
      "type": "object",
      "properties": {
        "percentage": {"type": "integer"},
        "status": {"type": "string"}
      },
      "required": ["percentage", "status"]
    }
  }
}
```

## 5.4 有子命令工具格式

### 5.4.1 结构要求

有子命令工具必须满足：

- `hasSubcommands` 必须为 `true`
- 顶层必须存在 `subcommands`
- 顶层不再定义业务用 `inputSchema`
- 顶层不再定义业务用 `argumentMapping`
- 每个子命令独立定义自己的 `inputSchema`、`outputSchema`、`argumentMapping`

### 5.4.2 子命令字段表

`subcommands.<name>` 支持以下字段：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `description` | string | 是 | 子命令描述 |
| `inputSchema` | object | 是 | 子命令输入参数 Schema |
| `outputSchema` | object | 否 | 子命令结果 Schema |
| `eventTypes` | string[] | 否 | 子命令支持的事件类型 |
| `eventSchemas` | object | 否 | 子命令事件 Schema |
| `argumentMapping` | object | 是 | 子命令参数映射规则 |

### 5.4.3 最小示例

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
  "hasSubcommands": true,
  "subcommands": {
    "install": {
      "description": "Install a package",
      "inputSchema": {
        "type": "object",
        "properties": {
          "package": {"type": "string"},
          "force": {"type": "boolean", "default": false}
        },
        "required": ["package"]
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "package": "{value}",
          "force": "--force"
        }
      }
    },
    "list": {
      "description": "List installed packages",
      "inputSchema": {
        "type": "object",
        "properties": {
          "verbose": {"type": "boolean", "default": false}
        }
      },
      "argumentMapping": {
        "type": "flag",
        "templates": {
          "verbose": "--verbose"
        }
      }
    }
  }
}
```

### 5.4.4 调用方式

```javascript
await CLI.CLIManager.execTool('pkg-manager', {
  subcommand: 'install',
  package: '/path/to/app.hap',
  force: true
});
```

生成命令：

```bash
/usr/bin/pkg-mgr install /path/to/app.hap --force
```

## 5.5 事件输出约定

### 5.5.1 核心原则

工具进程通过标准输出和标准错误输出向框架发送运行信息：

- 若某一行是合法的事件 JSON，则按事件处理
- 若某一行不是事件 JSON，则普通 stdout 文本行作为 `stdout` 事件处理
- stderr 输出作为 `stderr` 事件处理

事件机制的核心目的是：

- 让调用方在工具执行过程中接收实时进度
- 让框架对结构化事件进行格式校验
- 将“普通文本输出”和“业务事件输出”统一接入同一分发机制

### 5.5.2 事件 JSON 格式

事件 JSON 行必须包含：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `event` | string | 是 | 事件类型 |
| `data` | any | 是 | 事件数据 |
| `timestamp` | number | 否 | 时间戳，框架可补充 |

推荐输出形式：

```json
{"event": "progress", "data": {"percentage": 50, "status": "scanning"}}
```

其中：

- `event` 决定事件类型
- `data` 是实际业务负载
- 若工具未输出 `timestamp`，可由框架补充

### 5.5.3 事件类型与 Schema

| 事件类型 | 来源 | Schema 位置 |
|---------|------|------------|
| `stdout` | 非 JSON stdout 文本 | 不需要 Schema |
| `stderr` | stderr 文本 | 不需要 Schema |
| `exit` | 进程退出 | 框架生成 |
| `error` | 执行失败或解析失败 | 框架生成 |
| `progress` | 工具输出事件 JSON | `eventSchemas.progress` |
| `result` | 工具输出事件 JSON | `outputSchema` |

补充说明：

- `progress` 适用于过程性反馈，例如百分比、阶段、当前对象
- `result` 适用于最终业务结果，例如安装结果、扫描摘要、查询结果
- 一个工具可以同时产生 `stdout`、`progress`、`result`
- `result` 一般应作为最终结构化结果，不建议用普通文本替代

### 5.5.4 示例

工具输出：

```text
Starting scan...
{"event": "progress", "data": {"percentage": 50, "status": "scanning"}}
{"event": "result", "data": {"summary": {"totalPorts": 2, "openPorts": 1}}}
```

框架处理结果：

- `Starting scan...` -> `stdout`
- `progress` 事件 -> 使用 `eventSchemas.progress` 验证
- `result` 事件 -> 使用 `outputSchema` 验证

再例如：

```text
initializing...
{"event": "progress", "data": {"percentage": 10, "status": "starting"}}
{"event": "progress", "data": {"percentage": 90, "status": "running"}}
{"event": "result", "data": {"status": "success", "message": "done"}}
```

调用方可在同一会话中依次收到：

- 普通 `stdout` 文本
- 多次 `progress` 事件
- 一次 `result` 事件

### 5.5.5 错误处理

- 非法 JSON 行：生成 `error` 事件
- 事件数据不符合 Schema：生成 `error` 事件
- 工具异常退出：生成 `exit` 或 `error` 事件

常见错误场景：

- `eventTypes` 未声明 `progress`，但工具输出了 `progress` 事件
- `eventSchemas.progress` 要求 `percentage` 为整数，但工具输出了字符串
- `result` 事件的数据与 `outputSchema` 不匹配
- 工具输出了看似 JSON 但结构不完整的事件行

设计建议：

- 工具若计划输出结构化事件，应先在描述文件中声明 `eventTypes`
- 对需要被调用方稳定消费的事件，应提供明确的 `eventSchemas`
- 对最终结果，应优先使用 `result + outputSchema`，避免调用方解析普通文本

## 5.6 两种格式对比

| 特性 | 无子命令工具 | 有子命令工具 |
|------|-------------|-------------|
| `hasSubcommands` | 缺省或 `false` | `true` |
| `subcommands` | 不存在 | 必填 |
| 顶层 `inputSchema` | 必填 | 不使用 |
| 顶层 `argumentMapping` | 必填 | 不使用 |
| 参数定义位置 | 顶层 | 子命令内 |
| 调用时是否传 `subcommand` | 否 | 是 |

## 5.7 选择建议

- 单一功能工具：优先使用无子命令工具格式
- 多功能工具：统一使用子命令工具格式
- 参数层级较深时：优先考虑 `flattened` 或 `json-string`
- 需要流式进度或结果回传时：显式定义 `eventTypes` 和 `eventSchemas`

# 附录 A. 实现提示

以下内容用于指导实现，不属于配置格式规范本身。

## A.1 参数映射实现思路

```cpp
std::vector<std::string> MapToCommandLine(const ToolInfo& toolInfo,
                                          const Json::Value& abstractArgs)
{
    std::vector<std::string> argv;
    argv.push_back(toolInfo.executablePath);

    if (toolInfo.hasSubcommands) {
        std::string subcommandName = abstractArgs.get("subcommand", "").asString();
        argv.push_back(subcommandName);
        ApplyArgumentMapping(toolInfo.subcommands[subcommandName].argMapping, abstractArgs, argv);
    } else {
        ApplyArgumentMapping(toolInfo.argMapping, abstractArgs, argv);
    }
    return argv;
}
```

## A.2 建议的校验方向

- 校验两种工具格式互斥
- 校验 `argumentMapping` 引用的字段是否真实存在于 `inputSchema`
- 校验 `eventSchemas` 与 `eventTypes` 是否匹配
- 校验 `inputSchema` 是否只使用当前支持的 JSON Schema 子集
