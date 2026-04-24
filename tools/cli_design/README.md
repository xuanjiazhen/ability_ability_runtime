# CLI 参数映射系统

基于 OpenHarmony CliTool 设计规范实现的五种参数映射方法 Python 设计方案。

## 🎯 概述

本系统提供了灵活的参数到命令行参数的映射功能，支持五种映射模式：

- **Flag 模式** - `--key=value` 和 `-v` 格式
- **Positional 模式** - 固定顺序位置参数
- **Flattened 模式** - 嵌套对象展平为路径参数
- **Json-String 模式** - 对象序列化为 JSON 字符串
- **Mixed 模式** - 组合使用多种映射模式

## 🚀 快速开始

### 基本使用

```python
from mapper import MapperFactory

# 配置映射规则
config = {
    'type': 'flag',
    'templates': {
        'target': '--target={value}',
        'verbose': {'if_true': '-v', 'if_false': ''}
    }
}

# 创建映射器
mapper = MapperFactory.create(config)

# 执行映射
input_args = {'target': '192.168.1.1', 'verbose': True}
argv = mapper.map(input_args)

# 结果: ['--target=192.168.1.1', '-v']
```

### 验证 input

```python
from mapper import validate_input

# 定义 Schema
schema = {
    "type": "object",
    "properties": {
        "target": {"type": "string", "format": "ipv4"},
        "port": {"type": "integer", "minimum": 1, "maximum": 65535}
    },
    "required": ["target"]
}

# 验证输入
input_args = {"target": "192.168.1.1", "port": 80}
is_valid, errors = validate_input(input_args, schema)

if not is_valid:
    print("验证失败:", errors)
```

## 📋 五种映射模式

### 1. Flag 模式

标准命令行工具格式。

```python
config = {
    'type': 'flag',
    'templates': {
        'target': '--target={value}',
        'ports': '--port={value}',
        'verbose': {'if_true': '-v', 'if_false': ''}
    }
}

input_args = {
    'target': '192.168.1.1',
    'ports': [80, 443],
    'verbose': True
}

# 输出: ['--target=192.168.1.1', '--port=80', '--port=443', '-v']
```

### 2. Positional 模式

传统 Unix 风格，按固定顺序追加参数。

```python
config = {
    'type': 'positional',
    'order': ['source', 'dest', 'verbose']
}

input_args = {
    'source': '/data/file.txt',
    'dest': '/backup/file.txt',
    'verbose': True
}

# 输出: ['/data/file.txt', '/backup/file.txt', 'true']
```

### 3. Flattened 模式

将嵌套对象展平为路径参数。

```python
config = {
    'type': 'flattened',
    'separator': '.',
    'templates': {
        'app.name': '--app-name={value}',
        'app.env.production': {'if_true': '--env-prod', 'if_false': ''}
    }
}

input_args = {
    'app': {
        'name': 'myapp',
        'env': {'production': True}
    }
}

# 输出: ['--app-name=myapp', '--env-prod']
```

### 4. Json-String 模式

将复杂对象序列化为 JSON 字符串。

```python
config = {
    'type': 'jsonString',
    'templates': {
        'config': '--config={value}'
    }
}

input_args = {
    'config': {
        'application': {'name': 'webapp', 'replicas': 3}
    }
}

# 输出: ['--config={"application":{"name":"webapp","replicas":3}}']
```

### 5. Mixed 模式

组合使用多种映射模式，灵活处理复杂 CLI 工具。

```python
config = {
    'type': 'mixed',
    'mappings': {
        'action': {'mode': 'positional', 'order': 1},
        'service': {'mode': 'positional', 'order': 2},
        'verbose': {
            'mode': 'flag',
            'template': {'if_true': '--verbose', 'if_false': ''}
        },
        'server': {
            'mode': 'flattened',
            'template': {'server.host': '--host={value}'},
            'separator': '.'
        },
        'config': {'mode': 'jsonString', 'template': '--config={value}'}
    }
}

input_args = {
    'action': 'deploy',
    'service': 'webapp',
    'verbose': True,
    'server': {'host': '192.168.1.1', 'port': 8080},
    'config': {'replicas': 3}
}

# 输出: ['deploy', 'webapp', '--verbose', '--host=192.168.1.1', '--port=8080', '--config={"replicas":3}']
```

**输出顺序**：
1. 所有 `positional` 条目按 `order` 升序
2. 其余条目按 `mappings` 定义顺序

### Mixed 模式详细配置

**字段说明**：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `type` | string | 是 | 固定为 `"mixed"` |
| `mappings` | object | 是 | 参数映射表 |
| `mappings.<param>.mode` | string | 是 | 映射模式：`flag`、`positional`、`flattened`、`jsonString` |
| `mappings.<param>.template` | string/object | 条件必填 | 模板（flag/jsonString/flattened 需要） |
| `mappings.<param>.separator` | string | 否 | 分隔符（仅 flattened 使用，默认 `.`） |
| `mappings.<param>.order` | integer | 条件必填 | 顺序（仅 positional 使用，必须为正整数） |

**各子模式配置**：

**Positional 子模式**：
```json
{
  "action": {
    "mode": "positional",
    "order": 1
  }
}
```
- 必须指定 `order`，且为正整数
- 不同字段的 `order` 不能重复

**Flag 子模式**：
```json
{
  "verbose": {
    "mode": "flag",
    "template": {
      "if_true": "--verbose",
      "if_false": ""
    }
  }
}
```
- 必须指定 `template`
- `template` 可以是字符串或布尔对象

**Flattened 子模式**：
```json
{
  "server": {
    "mode": "flattened",
    "template": {
      "server.host": "--host={value}",
      "server.port": "--port={value}"
    },
    "separator": "."
  }
}
```
- 必须指定非空 object 类型的 `template`
- `template` 中的所有路径 key 必须以参数名为根
- `separator` 可选，默认为 `.`

**JSON-String 子模式**：
```json
{
  "config": {
    "mode": "jsonString",
    "template": "--config={value}"
  }
}
```
- 必须指定 `template`，且必须是字符串

**约束规则**：
- 同一字段不能被多个映射条目引用
- `positional.order` 不能重复
- `flattened.template` 中的路径必须以参数名为根

## 📁 项目结构

```
cli_design/
├── README.md                   # 本文件
├── mapper/                     # 映射器包
│   ├── __init__.py            # 包初始化
│   ├── all_mappers.py         # 所有映射器实现
│   └── schema_validator.py    # input 验证器
└── tests/                      # 测试和验证工具
    ├── test_all.py             # 所有测试
    ├── validate_and_run.py     # 验证工具
    ├── configs/                # 测试配置 (带完整 input)
    │   ├── flag_network_scanner.json
    │   ├── positional_file_copy.json
    │   ├── flattened_app_deploy.json
    │   ├── jsonstring_cloud_deploy.json
    │   ├── mixed_complex_deploy.json
    │   └── validation_user_create.json
    └── inputs/                 # 测试输入数据
```

## 🧪 测试与验证

**⚠️ 重要提示**: 以下命令默认从 `cli_design/` 项目根目录执行。

**📍 当前目录检查**:
```bash
# 确认你在正确的目录
pwd  # 应该显示: .../cli_design

# 检查 validate_and_run.py 是否存在
ls tests/validate_and_run.py  # 应该显示文件信息

# 如果在 tests/ 目录中，请返回上一级
cd ..
```

### 运行所有测试

```bash
cd tests
python3 test_all.py
```

**预期结果**: 所有 25 个测试通过 ✅

### 展示验证失败示例

查看 input 验证如何捕获各种错误：

```bash
cd tests

# 缺少必填字段
python3 validate_and_run.py \
  --config configs/validation_user_create.json \
  --input inputs/validation_missing_required.json

# 类型不匹配
python3 validate_and_run.py \
  --config configs/validation_user_create.json \
  --input inputs/validation_type_mismatch.json

# 约束违反
python3 validate_and_run.py \
  --config configs/validation_user_create.json \
  --input inputs/validation_constraint_violation.json

# 格式不匹配
python3 validate_and_run.py \
  --config configs/validation_user_create.json \
  --input inputs/validation_format_mismatch.json

# 正确输入（对比）
python3 validate_and_run.py \
  --config configs/validation_user_create.json \
  --input inputs/validation_valid_input.json
```

**展示场景**:
1. ❌ 缺少必填字段
2. ❌ 类型不匹配 (age: "twenty-five" 而非 integer)
3. ❌ 约束违反 (username 长度、age 范围、tags 数量)
4. ❌ 格式不匹配 (email 格式、username pattern)
5. ✅ 正确输入对比

**配置文件**: [validation_user_create.json](tests/configs/validation_user_create.json)

### 验证 Tests 配置

验证 `tests/` 中的 config 与 input 得到的映射结果：

**方法 1：从项目根目录运行**
```bash
# 在 cli_design/ 目录下执行

# 验证 Flag 模式测试
python3 tests/validate_and_run.py \
  --config tests/configs/flag_network_scanner.json \
  --input tests/inputs/flag_network_scanner_input.json

# 验证 Positional 模式测试
python3 tests/validate_and_run.py \
  --config tests/configs/positional_file_copy.json \
  --input tests/inputs/positional_file_copy_input.json

# 验证 Flattened 模式测试
python3 tests/validate_and_run.py \
  --config tests/configs/flattened_app_deploy.json \
  --input tests/inputs/flattened_app_deploy_input.json

# 验证 Json-String 模式测试
python3 tests/validate_and_run.py \
  --config tests/configs/jsonstring_cloud_deploy.json \
  --input tests/inputs/jsonstring_cloud_deploy_input.json

# 验证 Mixed 模式测试
python3 tests/validate_and_run.py \
  --config tests/configs/mixed_complex_deploy.json \
  --input tests/inputs/mixed_complex_deploy_input.json
```

**方法 2：从 tests 目录运行**
```bash
# 在 cli_design/tests/ 目录下执行

# 验证 Flag 模式测试
python3 validate_and_run.py \
  --config configs/flag_network_scanner.json \
  --input inputs/flag_network_scanner_input.json

# 验证 Positional 模式测试
python3 validate_and_run.py \
  --config configs/positional_file_copy.json \
  --input inputs/positional_file_copy_input.json

# 验证 Flattened 模式测试
python3 validate_and_run.py \
  --config configs/flattened_app_deploy.json \
  --input inputs/flattened_app_deploy_input.json

# 验证 Json-String 模式测试
python3 validate_and_run.py \
  --config configs/jsonstring_cloud_deploy.json \
  --input inputs/jsonstring_cloud_deploy_input.json

# 验证 Mixed 模式测试
python3 validate_and_run.py \
  --config configs/mixed_complex_deploy.json \
  --input inputs/mixed_complex_deploy_input.json
```

## 🔧 核心功能

### input 验证

支持的类型和约束：

| 类型 | 支持的约束 |
|------|-----------|
| `string` | minLength, maxLength, pattern, format (ipv4, ipv6, email, uri, date, date-time) |
| `number`/`integer` | minimum, maximum, exclusiveMinimum, exclusiveMaximum, multipleOf |
| `boolean` | default |
| `array` | minItems, maxItems, uniqueItems, items |
| `object` | properties, required |

### argumentMapping 映射

| 映射类型 | 适用场景 | 数组处理 | 嵌套对象 |
|---------|---------|---------|---------|
| `flag` | 标准工具 | 展开为多个参数 | 不支持 |
| `positional` | 传统工具 | 不支持 | 不支持 |
| `flattened` | 层级配置 | 不支持 | 展平为路径 |
| `jsonString` | 复杂配置 | 保持结构 | 序列化为 JSON |
| `mixed` | 复合工具 | 由子模式决定 | 由子模式决定 |

## 💡 完整示例

### 示例 1：使用 Flag 模式

```python
import json
from mapper import MapperFactory, validate_input

# 1. 加载工具描述
with open('tests/configs/flag_network_scanner.json') as f:
    tool_config = json.load(f)

# 2. 准备输入
input_args = {"target": "192.168.1.1", "ports": [80, 443]}

# 3. 验证 input
is_valid, errors = validate_input(input_args, tool_config['input'])
if not is_valid:
    print("验证失败:", errors)
    exit(1)

# 4. 执行映射
mapper = MapperFactory.create(tool_config['argumentMapping'])
argv = mapper.map(input_args)

# 5. 构建完整命令
executable_path = tool_config['executable']
complete_argv = [executable_path] + argv

print("执行命令:", ' '.join(complete_argv))
```

### 示例 2：使用 Mixed 模式

```python
import json
from mapper import MapperFactory, validate_input

# 1. 加载 mixed 模式工具描述
with open('tests/configs/mixed_complex_deploy.json') as f:
    tool_config = json.load(f)

# 2. 准备复杂输入
input_args = {
    "action": "deploy",
    "service": "gateway",
    "replicas": 3,
    "verbose": True,
    "server": {
        "host": "api.example.com",
        "port": 443,
        "tls": {"enabled": True, "certPath": "/etc/tls/cert.pem"}
    },
    "config": {
        "resources": {"cpu": "2", "memory": "4Gi"},
        "env": [{"name": "LOG_LEVEL", "value": "info"}]
    }
}

# 3. 验证 input
is_valid, errors = validate_input(input_args, tool_config['input'])
if not is_valid:
    print("验证失败:", errors)
    exit(1)

# 4. 执行映射
mapper = MapperFactory.create(tool_config['argumentMapping'])
argv = mapper.map(input_args)

# 5. 构建完整命令
executable_path = tool_config['executable']
complete_argv = [executable_path] + argv

print("执行命令:", ' '.join(complete_argv))
# 输出: /system/bin/cli_tool/executable/ohos-deploy deploy gateway --replicas=3 --verbose --host=api.example.com --port=443 --tls=true --cert=/etc/tls/cert.pem --config={"resources":...}
```

### Mixed 模式优势

**场景**: 一个 CLI 工具同时需要：
- 位置参数（action、service）
- 标志参数（replicas、verbose）
- 嵌套配置展平（server.host、server.port）
- 复杂对象序列化（config）

**使用 Mixed 模式前**：需要将配置拆分为多个工具或使用复杂的包装脚本
**使用 Mixed 模式后**：一个配置文件搞定所有映射需求

## 🎯 设计原则

### 职责划分

| 组件 | 作用 | 不负责 |
|------|------|--------|
| `inputSchema` | 定义参数类型、结构、约束 | 参数如何映射到命令行 |
| `argMapping` | 定义参数到命令行的转换方式 | 参数类型定义和校验 |

### 协同原则

1. **引用约束**: `argMapping` 只能引用 `inputSchema` 中定义的字段
2. **类型匹配**: 映射方式必须与字段类型兼容
3. **结构对应**: 嵌套结构影响映射方式的选择
4. **独立验证**: 两者独立校验，但需保证一致性

### 当前配置字段口径

- 工具配置使用 `executablePath`、`requirePermissions`、`inputSchema`、`outputSchema`
- 不再使用 `executablePath`、`permissions`、`inputSchema`
- 示例配置中的 `name` 使用当前规范的 `ohos-*`
- `enum`、`oneOf`、`anyOf`、`allOf` 不在当前示例中使用

## 📊 性能指标

- **测试运行**: ~2ms (25个测试)
- **Python 文件**: 5个 (已优化)
- **代码行数**: ~1650 行
- **示例配置**: 6个完整工具描述

## 📄 许可

遵循 OpenHarmony 许可证。

---

**版本**: 2.0.0  
**状态**: ✅ 所有功能完整，测试通过  
**文档**: 单一 README，信息完整
