# ValidateInputSchemaProperties 类型验证增强

## 概述

本次优化增强了 `ToolUtil::ValidateInputSchemaProperties` 方法，添加了完整的参数值类型验证功能，确保传入的参数值类型与 `inputSchema` 中定义的类型严格一致。

## 主要改进

### 1. 类型验证

新增了对以下 JSON Schema 类型的完整验证：

| JSON Schema Type | WantParams 类型 | 说明 |
|-----------------|-----------------|------|
| `string` | IString | 字符串类型 |
| `boolean` | IBoolean | 布尔类型 |
| `integer` | IInteger, ILong | 整数类型 |
| `number` | IInteger, ILong, IFloat, IDouble | 数字类型（包含整数和浮点数） |
| `array` | IArray | 数组类型 |
| `object` | IWantParams | 对象类型（支持嵌套验证） |

### 2. 嵌套对象验证

支持递归验证嵌套对象的结构和类型：

```json
{
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
    }
  }
}
```

验证逻辑会：
1. 检查 `app` 是否为 object 类型
2. 递归验证 `app.name` 是否为 string 类型
3. 递归验证 `app.version` 是否为 string 类型
4. 递归验证 `app.env` 是否为 object 类型
5. 递归验证 `app.env.production` 是否为 boolean 类型
6. 递归验证 `app.env.debug` 是否为 boolean 类型

### 3. 数组元素类型验证

支持验证数组中每个元素的类型：

```json
{
  "properties": {
    "ports": {
      "type": "array",
      "items": {
        "type": "integer"
      }
    }
  }
}
```

验证逻辑会检查数组中的每个元素是否都是 `integer` 类型。

### 4. 必填字段验证

支持 `required` 字段来指定必填的嵌套属性：

```json
{
  "properties": {
    "server": {
      "type": "object",
      "properties": {
        "host": {"type": "string"},
        "port": {"type": "integer"}
      },
      "required": ["host", "port"]
    }
  }
}
```

如果 `server` 对象中缺少 `host` 或 `port` 字段，验证将失败。

## API 变更

### 头文件 (tool_util.h)

```cpp
// 主验证方法
static int32_t ValidateInputSchemaProperties(const std::string &inputSchema,
    const AAFwk::WantParams &args);

// 类型验证辅助方法
static bool ValidateParamType(const sptr<AAFwk::IInterface> &value,
    const std::string &expectedType, const nlohmann::json &propertySchema,
    const std::string &key = "");

// 嵌套对象和数组验证方法（降低嵌套深度和圈复杂度）
static bool ValidateNestedObject(const AAFwk::WantParams &nestedParams,
    const nlohmann::json &objectSchema, const std::string &parentKey);

static bool ValidateArrayType(const sptr<AAFwk::IInterface> &value,
    const nlohmann::json &propertySchema, const std::string &key);

static bool ValidateObjectType(const sptr<AAFwk::IInterface> &value,
    const nlohmann::json &propertySchema, const std::string &key);

static bool ValidateArrayItems(sptr<AAFwk::IArray> arrayObj,
    const nlohmann::json &itemsSchema, const std::string &key);

static bool ValidateBasicType(const sptr<AAFwk::IInterface> &value,
    const std::string &expectedType);

// 基础类型检查方法
static bool IsStringType(const sptr<AAFwk::IInterface> &value);
static bool IsBooleanType(const sptr<AAFwk::IInterface> &value);
static bool IsIntegerType(const sptr<AAFwk::IInterface> &value);
static bool IsNumberType(const sptr<AAFwk::IInterface> &value);
static bool IsArrayType(const sptr<AAFwk::IInterface> &value);
static bool IsObjectType(const sptr<AAFwk::IInterface> &value);
```

### 方法深度优化

为满足代码质量要求（方法嵌套深度不超过 4），进行了以下重构：

**原实现问题**：
- `ValidateParamType()` 方法深度为 10，超过建议值 4
- 数组和对象类型的验证逻辑嵌套过深

**重构方案**：
1. 将数组验证逻辑提取到 `ValidateArrayType()` 方法
2. 将对象验证逻辑提取到 `ValidateObjectType()` 方法
3. 将数组元素验证逻辑提取到 `ValidateArrayItems()` 方法
4. 使用早期返回（early return）减少嵌套层级

**重构后的方法职责**：

| 方法 | 职责 | 最大深度 |
|------|------|----------|
| `ValidateParamType()` | 根据类型分发到具体验证方法 | 2 |
| `ValidateArrayType()` | 验证数组类型及元素 | 3 |
| `ValidateObjectType()` | 验证对象类型及嵌套属性 | 3 |
| `ValidateArrayItems()` | 验证数组中每个元素的类型 | 3 |
| `ValidateNestedObject()` | 递归验证嵌套对象的属性 | 3 |

### 圈复杂度优化

为满足代码质量要求（方法圈复杂度不超过 20），进行了以下重构：

**原实现问题**：
- `ValidateParamType()` 方法圈复杂度为 21，超过建议值 20
- 包含过长的 if-else 链判断所有类型

**重构方案**：
1. 提取 `ValidateBasicType()` 方法处理基本类型（string, boolean, integer, number）
2. 复杂类型（array, object）继续使用专门的验证方法
3. 减少主方法的判断分支数量

**重构后的方法圈复杂度**：

| 方法 | 重构前圈复杂度 | 重构后圈复杂度 | 改进 |
|------|---------------|---------------|------|
| `ValidateParamType()` | 21 | 4 | ⬇️ 81% |
| `ValidateBasicType()` | - | 5 | ✅ 新增 |
| `ValidateArrayType()` | (内嵌) | 3 | ✅ 提取 |
| `ValidateObjectType()` | (内嵌) | 3 | ✅ 提取 |

**圈复杂度计算**：

```cpp
// ValidateParamType() - 圈复杂度 = 4
bool ToolUtil::ValidateParamType(...)
{
    if (value == nullptr) { ... }        // +1
    if (expectedType == "array") { ... } // +1
    if (expectedType == "object") { ... }// +1
    return ValidateBasicType(...);       // base + 0
}
// Total: 3 decisions + 1 (base) = 4

// ValidateBasicType() - 圈复杂度 = 5
bool ToolUtil::ValidateBasicType(...)
{
    if (expectedType == "string") { ... }   // +1
    if (expectedType == "boolean") { ... }  // +1
    if (expectedType == "integer") { ... }  // +1
    if (expectedType == "number") { ... }   // +1
    // default case                         // base + 1
}
// Total: 4 decisions + 1 (base) = 5
```

## 使用示例

### 示例 1: 基本类型验证

```cpp
// Schema 定义
std::string schema = R"({
  "properties": {
    "target": {"type": "string"},
    "timeout": {"type": "integer"},
    "verbose": {"type": "boolean"}
  }
})";

// 构造参数
AAFwk::WantParams args;
args.SetParam("target", AAFwk::String::Box("192.168.1.1"));
args.SetParam("timeout", AAFwk::Integer::Box(5000));
args.SetParam("verbose", AAFwk::Boolean::Box(true));

// 验证
int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);
// result == ERR_OK (验证通过)
```

### 示例 2: 嵌套对象验证

```cpp
// Schema 定义
std::string schema = R"({
  "properties": {
    "app": {
      "type": "object",
      "properties": {
        "name": {"type": "string"},
        "version": {"type": "string"}
      }
    }
  }
})";

// 构造嵌套参数
AAFwk::WantParams appParams;
appParams.SetParam("name", AAFwk::String::Box("myapp"));
appParams.SetParam("version", AAFwk::String::Box("1.0.0"));

AAFwk::WantParams args;
args.SetParam("app", AAFwk::WantParamWrapper::Box(appParams));

// 验证
int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);
// result == ERR_OK (验证通过)
```

### 示例 3: 数组元素类型验证

```cpp
// Schema 定义
std::string schema = R"({
  "properties": {
    "ports": {
      "type": "array",
      "items": {"type": "integer"}
    }
  }
})";

// 构造数组参数
sptr<AAFwk::IArray> array = new (std::nothrow) AAFwk::Array(3, AAFwk::g_IID_IInteger);
array->Set(0, AAFwk::Integer::Box(80).GetRefPtr());
array->Set(1, AAFwk::Integer::Box(443).GetRefPtr());
array->Set(2, AAFwk::Integer::Box(8080).GetRefPtr());

AAFwk::WantParams args;
args.SetParam("ports", array);

// 验证
int32_t result = ToolUtil::ValidateInputSchemaProperties(schema, args);
// result == ERR_OK (验证通过)
```

## 测试覆盖

新增了以下测试用例：

1. **基本类型验证测试** (0100-0600)
   - 正确类型匹配
   - 类型不匹配错误检测

2. **数组类型验证测试** (0700-0800)
   - 正确数组类型
   - 数组类型不匹配

3. **对象类型验证测试** (0900)
   - 正确对象类型

4. **混合类型验证测试** (1000)
   - 多种类型混合验证

5. **无类型定义测试** (1100)
   - 无类型定义时的默认行为

6. **嵌套对象验证测试** (NestedObject_0100-0400)
   - 基本嵌套对象验证
   - 嵌套对象类型不匹配
   - 必填字段缺失验证
   - 深度嵌套对象验证

7. **数组元素类型验证测试** (ArrayItems_0100-0200)
   - 正确的数组元素类型
   - 数组元素类型不匹配

## 向后兼容性

- 如果 `inputSchema` 中的属性没有定义 `type` 字段，则跳过类型验证（保持原有行为）
- 未知类型会被允许（为了保证向前兼容性）
- 所有现有的测试用例保持不变

## 错误处理

验证失败时返回 `ERR_INVALID_PARAM`，并在日志中记录详细的错误信息：

```
E CLI_TOOL: args key 'timeout' type mismatch, expected: integer
E CLI_TOOL: Array 'ports' element 1 type mismatch, expected: integer
E CLI_TOOL: Nested property 'app.version' type mismatch, expected: string
E CLI_TOOL: Required nested property 'server.port' not found
```

## 性能考虑

- 类型验证是在参数传递给工具之前进行的，不会影响运行时性能
- 嵌套验证采用深度优先遍历，时间复杂度为 O(n)，其中 n 是所有属性的总数
- 对于深层嵌套结构，验证时间是线性的

## 代码质量指标

所有验证相关的方法均满足以下代码质量要求：

| 质量指标 | 要求 | 实际值 | 状态 |
|---------|------|--------|------|
| 方法嵌套深度 | ≤ 4 | ≤ 3 | ✅ 通过 |
| 方法圈复杂度 | ≤ 20 | ≤ 5 | ✅ 通过 |
| 方法行数 | ≤ 150 | ≤ 80 | ✅ 通过 |

## 未来改进

1. 支持更多 JSON Schema 特性：
   - `enum` 枚举值验证
   - `pattern` 正则表达式验证（针对字符串）
   - `minimum/maximum` 数值范围验证
   - `minLength/maxLength` 字符串长度验证
   - `minItems/maxItems` 数组长度验证

2. 支持联合类型（oneOf, anyOf, allOf）

3. 提供更详细的验证错误报告

4. 性能优化：
   - 缓存已解析的 JSON Schema
   - 并行验证独立的属性

## 参考资料

- [argument_mapping_design.md](../../argument_mapping_design.md) - 参数映射设计文档
- [JSON Schema 规范](https://json-schema.org/) - JSON Schema 官方规范
