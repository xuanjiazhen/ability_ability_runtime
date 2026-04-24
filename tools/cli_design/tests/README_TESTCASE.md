# 测试用例使用说明

## 快速开始

### 运行单个测试用例

```bash
# 在 tests/ 目录下执行
python3 validate_and_run.py \
  --config testcase/config/test_001_flag_basic.json \
  --input testcase/input/test_001_input.json
```

### 批量运行所有测试用例

```bash
# 在 tests/ 目录下执行
chmod +x run_testcase_tests.sh
./run_testcase_tests.sh
```

## 目录结构

```
tests/
├── testcase/                    # 测试用例目录
│   ├── config/                  # 配置文件
│   │   ├── test_001_flag_basic.json
│   │   ├── test_002_flag_array_bool.json
│   │   ├── test_003_positional.json
│   │   ├── test_004_flattened.json
│   │   ├── test_005_json_string.json
│   │   ├── test_006_mixed_simple.json
│   │   ├── test_007_mixed_complex.json
│   │   └── test_008_optional_params.json
│   ├── input/                   # 输入文件
│   │   ├── test_001_input.json
│   │   ├── test_002_input.json
│   │   ├── test_003_input.json
│   │   ├── test_004_input.json
│   │   ├── test_005_input.json
│   │   ├── test_006_input.json
│   │   ├── test_007_input.json
│   │   └── test_008_input.json
│   ├── expect/                  # 预期输出文件
│   │   ├── test_001_expected.json
│   │   ├── test_002_expected.json
│   │   ├── test_003_expected.json
│   │   ├── test_004_expected.json
│   │   ├── test_005_expected.json
│   │   ├── test_006_expected.json
│   │   ├── test_007_expected.json
│   │   └── test_008_expected.json
│   ├── testcase.md              # 测试用例列表（含4列表格）
│   └── README.md                # 本文件
├── run_testcase_tests.sh        # 批量运行脚本
└── validate_and_run.py          # 验证工具
```

## 测试用例列表

详见 `testcase/testcase.md`，包含以下 4 列：

| 用例序号 | config 文件名 | input 文件名 | 预期输出结果 |
|---------|--------------|--------------|-------------|
| 001 | test_001_flag_basic.json | test_001_input.json | `--host=192.168.1.100 --port=8080 --timeout=30` |
| ... | ... | ... | ... |

## 测试用例覆盖

- ✅ 5 种映射模式（flag, positional, flattened, jsonString, mixed）
- ✅ 所有基础数据类型（string, number, integer, boolean）
- ✅ 复杂类型（array, object）
- ✅ 特殊场景（数组展开、条件模板、可选参数）

## 验证方式

每个测试用例通过以下方式验证：

1. **执行验证**: `validate_and_run.py` 成功执行
2. **参数匹配**: 生成的 argv 与预期输出一致
3. **命令正确**: 完整命令字符串符合预期

## 添加新测试用例

1. 在 `testcase/config/` 创建 `test_XXX_description.json`
2. 在 `testcase/input/` 创建 `test_XXX_input.json`
3. 在 `testcase/expect/` 创建 `test_XXX_expected.json`
4. 在 `testcase/testcase.md` 中添加条目

## 相关文档

- [testcase/testcase.md](testcase/testcase.md) - 完整的测试用例列表和详情
- [README.md](../README.md) - CLI 参数映射系统说明
- [参数解析文档.md](../../参数解析文档.md) - 参数映射系统文档
