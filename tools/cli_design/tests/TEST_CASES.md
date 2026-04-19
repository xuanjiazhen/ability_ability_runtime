# 参数映射测试用例集

这是一套完整的测试用例，用于验证 `validate_and_run.py` 的参数映射功能。

## 📁 测试用例结构

```
tests/
├── configs/              # 测试配置文件
│   ├── test_001_flag_basic.json
│   ├── test_002_flag_array_bool.json
│   ├── test_003_positional.json
│   ├── test_004_flattened.json
│   ├── test_005_json_string.json
│   ├── test_006_mixed_simple.json
│   ├── test_007_mixed_complex.json
│   └── test_008_optional_params.json
├── inputs/               # 测试输入文件
│   ├── test_001_input.json
│   ├── test_002_input.json
│   ├── ... 
│   └── test_008_input.json
├── expected/             # 预期输出文件
│   ├── test_001_expected.json
│   ├── test_002_expected.json
│   ...
│   └── test_008_expected.json
├── run_tests.sh          # 批量运行脚本
├── verify_tests.py       # 验证脚本
└── TEST_CASES.md         # 测试用例说明
```

## 🎯 测试用例覆盖

### 基础模式测试 (001-005)

| 测试 | 模式 | 描述 | 验证点 |
|------|------|------|--------|
| 001 | Flag | 基础字符串和数字 | 标准 `--key=value` 格式 |
| 002 | Flag | 数组展开和布尔值 | 数组展开为多个参数，布尔条件模板 |
| 003 | Positional | 固定顺序参数 | 按 `order` 顺序输出 |
| 004 | Flattened | 嵌套对象展平 | 路径展平、深层嵌套 |
| 005 | JSON-String | 复杂对象序列化 | 对象和数组 JSON 序列化 |

### Mixed 模式测试 (006-007)

| 测试 | 描述 | 组合模式 | 关键验证 |
|------|------|---------|---------|
| 006 | 简单组合 | positional + flag | 输出顺序规则 |
| 007 | 复杂组合 | 所有 4 种模式 | 完整场景测试 |

### 特殊场景 (008)

| 测试 | 描述 | 验证点 |
|------|------|--------|
| 008 | 可选参数 | 部分输入处理，只映射存在的字段 |

## 🚀 运行测试

### 方法 1：批量运行所有测试

```bash
cd tests
./run_tests.sh
```

### 方法 2：运行单个测试

```bash
cd tests
python3 validate_and_run.py \
  --config configs/test_001_flag_basic.json \
  --input inputs/test_001_input.json
```

### 方法 3：验证测试结果

```bash
cd tests
python3 verify_tests.py
```

## 📝 测试用例格式

每个测试用例包含 3 个文件：

### 1. 配置文件 (configs/test_XXX_*.json)

定义工具的 `input` Schema 和 `argumentMapping` 规则。

### 2. 输入文件 (inputs/test_XXX_input.json)

提供测试输入参数。

### 3. 预期输出 (expected/test_XXX_expected.json)

定义预期的命令行参数。

```json
{
  "test_case": "测试描述",
  "executable": "/usr/bin/tool",
  "expected_argv": ["--arg1=value1", "--arg2=value2"],
  "expected_command": "/usr/bin/tool --arg1=value1 --arg2=value2",
  "note": "额外说明"
}
```

## ✅ 验证标准

测试通过标准：
1. ✅ `validate_and_run.py` 成功执行（返回码 0）
2. ✅ 生成的 argv 与预期完全一致
3. ✅ 完整命令字符串正确

## 📊 测试覆盖矩阵

| 特性 | Flag | Positional | Flattened | JSON-String | Mixed |
|------|------|-----------|-----------|-------------|-------|
| 字符串参数 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 数字参数 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 布尔参数 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 数组参数 | ✅ | ❌ | ❌ | ✅ | ✅ |
| 对象参数 | ❌ | ❌ | ✅ | ✅ | ✅ |
| 条件模板 | ✅ | ❌ | ✅ | ✅ | ✅ |
| 可选参数 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 顺序控制 | ❌ | ✅ | ❌ | ❌ | ✅ |

## 🔧 扩展测试

要添加新的测试用例：

1. 创建配置文件：`configs/test_009_description.json`
2. 创建输入文件：`inputs/test_009_input.json`
3. 创建预期输出：`expected/test_009_expected.json`
4. 更新 `TEST_CASES.md`
5. 运行测试验证

## 📖 示例

### 测试用例 001: Flag 模式

**配置**:
```json
{
  "argumentMapping": {
    "type": "flag",
    "templates": {
      "host": "--host={value}",
      "port": "--port={value}"
    }
  }
}
```

**输入**:
```json
{
  "host": "192.168.1.1",
  "port": 8080
}
```

**预期输出**:
```json
{
  "expected_argv": ["--host=192.168.1.1", "--port=8080"],
  "expected_command": "/usr/bin/test-tool --host=192.168.1.1 --port=8080"
}
```

## 🎓 学习路径

建议按顺序学习和测试：

1. **第一步**: 运行 001-005，理解基础模式
2. **第二步**: 运行 006，理解 mixed 的简单组合
3. **第三步**: 运行 007，理解复杂的真实场景
4. **第四步**: 运行 008，理解边界情况

## 📚 相关文档

- [参数解析文档.md](../../参数解析文档.md) - 完整的参数映射系统文档
- [README.md](../README.md) - CLI 参数映射系统说明
- [final-config.md](../../final-config.md) - 配置规范
