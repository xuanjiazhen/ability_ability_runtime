# config.json Validator

基于 config 校验规则的 Python 校验工具，用于检查 CLI 工具 `config.json` 是否符合当前严格规范。

**特点**：
- 复用 build 侧公共校验内核，确保编译期校验与开发者本地校验的一致性
- 提供批量发现、输出渲染和测试等开发者友好功能
- 支持严格错误码校验，便于快速定位配置问题

## 目录结构

**注意**：以下目录结构基于当前工作目录为 `tools/validator/`

```text
./
├── _build_validation.py          # 构建验证相关
├── main.py                       # 主入口文件
├── readme.md                     # 本文档
├── cli_info_process/             # CLI信息处理模块
│   ├── __init__.py
│   ├── merge_cli_info.py         # CLI信息合并
│   ├── validate_cli_config.py    # CLI配置验证
│   ├── validation_loader.py      # 验证加载器
│   ├── validation_model.py       # 验证模型
│   ├── validation_rules.py       # 验证规则
│   └── validation_schema.py      # 验证模式
├── utils/                        # 工具函数库
│   ├── __init__.py
│   ├── loader.py                 # JSON文件加载
│   ├── model.py                  # 数据模型定义
│   ├── reporter.py               # 报告生成和渲染
│   ├── schema_utils.py           # 模式工具函数
│   └── validators.py             # 验证器实现
├── tests/                        # 测试用例
│   ├── configs/                  # 测试配置文件
│   │   ├── invalid_mixed_mapping.json
│   │   ├── invalid_missing_required_schema.json
│   │   ├── invalid_unknown_template_field.json
│   │   ├── valid_simple_config.json
│   │   └── valid_subcommand_config.json
│   ├── test_validator.py         # 基础测试
│   └── test_validator_coverage.py # 覆盖率测试
└── example/                      # 示例CLI工具
    ├── cli-example/              # ohos-example 示例工具
    │   ├── BUILD.gn
    │   ├── config.json
    │   ├── skills.md
    │   └── src/main.cpp
    ├── cli-simple/               # ohos-simple 示例工具
    │   ├── BUILD.gn
    │   ├── config.json
    │   ├── skills.md
    │   └── src/main.cpp
    └── cli-timer/                # ohos-timer 示例工具
        ├── BUILD.gn
        ├── config.json
        └── src/
```

## 功能

- **多种校验模式**：支持单文件、目录、递归目录校验
- **灵活输出**：支持文本和 JSON 格式输出
- **批量处理**：自动发现并校验多个配置文件
- **错误定位**：提供精确的错误码和路径信息
- **快速失败**：支持 `--fail-fast` 选项，遇到错误立即停止
- **完整覆盖**：125个规则码的完整测试覆盖

## 使用方式

**注意**：以下命令假设当前工作目录为 `tools/validator/`

### 校验单个文件

```bash
python3 main.py validate example/ohos-example/config.json
```

### 校验目录

```bash
python3 main.py validate example/ohos-example
```

### 递归校验目录

```bash
python3 main.py validate example --recursive
```

### JSON 输出

```bash
python3 main.py validate example/ohos-example/config.json --json
```

### 遇到首个非法文件即停止

```bash
python3 main.py validate example --recursive --fail-fast
```

## 退出码

- `0`：全部通过
- `1`：存在校验错误
- `2`：参数错误或输入路径无效
- `3`：工具内部异常

## 输出格式

### 文本模式

```text
[FAIL] example/ohos-timer/config.json
  - MAP104 /argumentMapping/templates/help: template key 'help' is not defined in inputSchema.properties
Summary: checked=1 valid=0 invalid=1
```

### JSON 模式

```json
{
  "summary": {
    "files_checked": 1,
    "files_valid": 0,
    "files_invalid": 1
  },
  "results": [
    {
      "file": "example/ohos-timer/config.json",
      "valid": false,
      "issues": [
        {
          "code": "MAP104",
          "path": "/argumentMapping/templates/help",
          "message": "template key 'help' is not defined in inputSchema.properties"
        }
      ]
    }
  ]
}
```

## 测试

`tests/` 下提供最小正反例样本和测试脚本。

测试文件说明：

- [valid_simple_config.json](tests/configs/valid_simple_config.json)：合法的无子命令工具配置
- [valid_subcommand_config.json](tests/configs/valid_subcommand_config.json)：合法的子命令工具配置
- [invalid_mixed_mapping.json](tests/configs/invalid_mixed_mapping.json)：非法的 `argumentMapping.type`
- [invalid_missing_required_schema.json](tests/configs/invalid_missing_required_schema.json)：非法的 `required` 引用
- [invalid_unknown_template_field.json](tests/configs/invalid_unknown_template_field.json)：非法的模板字段引用
- [test_validator.py](tests/test_validator.py)：基础正反例回归测试
- [test_validator_coverage.py](tests/test_validator_coverage.py)：规则覆盖测试

### 基础测试

用于验证校验器的基本可用性和典型正反例：

```bash
python3 tests/test_validator.py
```

### 覆盖测试

用于验证当前实现的全部错误码是否都有对应测试用例：

```bash
python3 tests/test_validator_coverage.py
```

**当前覆盖情况**：
- 覆盖校验内核中**已经实现**且**可触发**的全部规则码
- 当前覆盖结果为 `125` 个规则码
- 测试覆盖率为 `100%`（针对已实现的规则码）

### 新增规则时的测试约束

后续若新增或调整规则，实现和测试必须同时更新：

- 若新增错误码，必须在 [test_validator_coverage.py](tests/test_validator_coverage.py) 中增加对应触发用例
- 若某条规则只是调整文案，不改变错误码，可只更新断言期望
- 若某条规则在实现中删除，需同步从覆盖集合中移除
- 若文档新增规则但实现尚未支持，不应先把它计入覆盖目标

### 建议的执行顺序

提交前建议至少执行这两条命令：

```bash
python3 tests/test_validator.py
python3 tests/test_validator_coverage.py
```
