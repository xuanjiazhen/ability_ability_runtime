# config.json Validator

`tools/validator/` 提供一个基于 [cofig校验规则.md](/data/home/d00838850/oh/foundation/ability/ability_runtime/cofig校验规则.md) 的 Python 校验工具，用于检查 CLI 工具 `config.json` 是否符合当前严格规范。

## 目录结构

```text
tools/validator/
├── main.py
├── loader.py
├── model.py
├── reporter.py
├── schema_utils.py
├── validators.py
└── tests/
```

## 功能

- 校验单个 `config.json`
- 校验目录中的 `config.json`
- 递归发现并校验多个 `config.json`
- 文本输出
- `--json` 结构化输出
- 严格错误码校验

## 使用方式

### 校验单个文件

```bash
python3 tools/validator/main.py validate tools/cli-aa/config.json
```

### 校验目录

```bash
python3 tools/validator/main.py validate tools/cli-aa
```

### 递归校验目录

```bash
python3 tools/validator/main.py validate tools --recursive
```

### JSON 输出

```bash
python3 tools/validator/main.py validate tools/cli-aa/config.json --json
```

### 遇到首个非法文件即停止

```bash
python3 tools/validator/main.py validate tools --recursive --fail-fast
```

## 退出码

- `0`：全部通过
- `1`：存在校验错误
- `2`：参数错误或输入路径无效
- `3`：工具内部异常

## 输出格式

### 文本模式

```text
[FAIL] tools/cli-aa/config.json
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
      "file": "tools/cli-aa/config.json",
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

- [valid_simple_config.json](/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/validator/tests/valid_simple_config.json)：合法的无子命令工具配置
- [valid_subcommand_config.json](/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/validator/tests/valid_subcommand_config.json)：合法的子命令工具配置
- [invalid_mixed_mapping.json](/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/validator/tests/invalid_mixed_mapping.json)：非法的 `argumentMapping.type`
- [invalid_missing_required_schema.json](/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/validator/tests/invalid_missing_required_schema.json)：非法的 `required` 引用
- [invalid_unknown_template_field.json](/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/validator/tests/invalid_unknown_template_field.json)：非法的模板字段引用
- [test_validator.py](/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/validator/tests/test_validator.py)：基础正反例回归测试
- [test_validator_coverage.py](/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/validator/tests/test_validator_coverage.py)：规则覆盖测试

### 基础测试

用于验证校验器的基本可用性和典型正反例：

```bash
python3 tools/validator/tests/test_validator.py
```

### 覆盖测试

用于验证当前实现的全部错误码是否都有对应测试用例：

```bash
python3 tools/validator/tests/test_validator_coverage.py
```

当前覆盖口径：

- 覆盖 [tools/validator/](/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/validator) 中**已经实现**且**可触发**的全部规则码
- 当前覆盖结果为 `125` 个规则码
- 覆盖统计以实现代码为准，不以 [cofig校验规则.md](/data/home/d00838850/oh/foundation/ability/ability_runtime/cofig校验规则.md) 的理论规则总数为准

原因：

- 规则文档中的个别条目尚未落到实现
- 少数规则在当前 JSON 解析方式下不可直接构造，例如依赖重复对象键语义的场景

因此，这里的“100% 覆盖率”含义是：

- 对当前校验器已实现的规则码，测试覆盖率为 `100%`

### 新增规则时的测试约束

后续若新增或调整规则，实现和测试必须同时更新：

- 若新增错误码，必须在 [test_validator_coverage.py](/data/home/d00838850/oh/foundation/ability/ability_runtime/tools/validator/tests/test_validator_coverage.py) 中增加对应触发用例
- 若某条规则只是调整文案，不改变错误码，可只更新断言期望
- 若某条规则在实现中删除，需同步从覆盖集合中移除
- 若文档新增规则但实现尚未支持，不应先把它计入覆盖目标

### 建议的执行顺序

提交前建议至少执行这两条命令：

```bash
python3 tools/validator/tests/test_validator.py
python3 tools/validator/tests/test_validator_coverage.py
```
