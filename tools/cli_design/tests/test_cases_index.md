# 参数映射测试用例集

## 测试用例列表

### 001-005: 基础模式测试
| 编号 | 模式 | 描述 | 关键点 |
|------|------|------|--------|
| 001 | Flag | 基础字符串和数字参数 | 标准flag格式 |
| 002 | Flag | 数组展开和布尔条件 | 数组展开、条件模板 |
| 003 | Positional | 固定顺序参数 | 按order输出 |
| 004 | Flattened | 嵌套对象展平 | 路径展平 |
| 005 | JSON-String | 复杂对象序列化 | JSON序列化 |

### 006-007: Mixed 模式测试
| 编号 | 描述 | 组合模式 | 关键点 |
|------|------|---------|--------|
| 006 | 简单组合 | positional + flag | 输出顺序 |
| 007 | 复杂组合 | 所有模式 | 完整场景 |

### 008: 特殊场景
| 编号 | 描述 | 关键点 |
|------|------|--------|
| 008 | 可选参数 | 部分输入处理 |

## 文件结构

```
tests/
├── configs/
│   ├── test_001_flag_basic.json
│   ├── test_002_flag_array_bool.json
│   ├── test_003_positional.json
│   ├── test_004_flattened.json
│   ├── test_005_json_string.json
│   ├── test_006_mixed_simple.json
│   ├── test_007_mixed_complex.json
│   └── test_008_optional_params.json
├── inputs/
│   ├── test_001_input.json
│   ├── test_002_input.json
│   ├── test_003_input.json
│   ├── test_004_input.json
│   ├── test_005_input.json
│   ├── test_006_input.json
│   ├── test_007_input.json
│   └── test_008_input.json
└── expected/
    ├── test_001_expected.json
    ├── test_002_expected.json
    ├── test_003_expected.json
    ├── test_004_expected.json
    ├── test_005_expected.json
    ├── test_006_expected.json
    ├── test_007_expected.json
    └── test_008_expected.json
```

## 运行测试

使用 `run_tests.sh` 批量运行所有测试：

```bash
cd tests
chmod +x run_tests.sh
./run_tests.sh
```

或手动运行单个测试：

```bash
python3 validate_and_run.py \
  --config configs/test_001_flag_basic.json \
  --input inputs/test_001_input.json \
  --expected expected/test_001_expected.json
```
