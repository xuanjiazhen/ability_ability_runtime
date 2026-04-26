# CLI Config Validator

`cli_config_validator` 是用于本地校验 CLI `config.json` 的开发辅助工具。

它复用 build 侧公共校验内核，目标是让开发阶段的本地校验结果尽量与编译期校验保持一致，适合在提交前快速发现字段、Schema、参数映射和事件声明问题。

当前正式规则文档：

- [config校验规则.md](config校验规则.md)

补充说明：

- `cli_config_validator` 负责静态 `config.json` 规则校验
- `config.name == ohos_cli_executable tool_name/target_name` 属于模板级约束，仍由构建期校验负责

## 目录结构

当前目录的主要内容如下：

```text
./
├── _build_validation.py          # build 侧校验实现导入入口
├── main.py                       # 命令行入口
├── readme.md                     # 本文档
├── utils/                        # 本地包装层
│   ├── loader.py                 # JSON 加载包装
│   ├── model.py                  # 结果模型包装
│   ├── reporter.py               # 文本 / JSON 输出
│   ├── schema_utils.py           # Schema 工具包装
│   └── validators.py             # 规则校验包装
└── tests/
    ├── configs/                  # 最小正反例配置
    ├── testcase.md               # 测试用例说明
    ├── test_validator.py         # 基础回归测试
    └── test_validator_coverage.py # 错误码覆盖测试
```

说明：

- `utils/*` 本身不是独立规则实现，而是对 build 侧校验器的包装
- 真正规则来源于 `build/ohos/cli/cli_info_process`

## 功能

- 校验单个或多个 `config.json`
- 支持目录扫描和递归扫描
- 支持只扫描 `config.json`，或通过 `--all-json` 扫描全部 `.json`
- 支持文本和 JSON 两种输出格式
- 支持 `--fail-fast` 在首个非法文件处停止
- 提供基础回归测试和错误码覆盖测试

## 使用方式

以下命令默认以仓根目录为当前目录执行。

### 校验单个文件

```bash
python3 cli_config_validator/main.py validate cli_config_validator/tests/configs/valid_simple_config.json
```

### 校验目录中的 `config.json`

```bash
python3 cli_config_validator/main.py validate some/tool/dir
```

### 递归校验目录中的 `config.json`

```bash
python3 cli_config_validator/main.py validate cli_config_validator/tests --recursive
```

### 校验目录中的所有 `.json` 文件

```bash
python3 cli_config_validator/main.py validate cli_config_validator/tests/configs --all-json
```

### 递归校验目录中的所有 `.json` 文件

```bash
python3 cli_config_validator/main.py validate cli_config_validator/tests --recursive --all-json
```

### JSON 输出

```bash
python3 cli_config_validator/main.py validate cli_config_validator/tests/configs/valid_simple_config.json --json
```

### 遇到首个非法文件即停止

```bash
python3 cli_config_validator/main.py validate cli_config_validator/tests --recursive --all-json --fail-fast
```

## 退出码

- `0`：全部通过
- `1`：存在校验错误
- `2`：参数错误或输入路径无效

## 输出格式

### 文本模式

```text
[FAIL] cli_config_validator/tests/configs/invalid_unknown_template_field.json
  - MAP104 /argMapping/templates/missing: template key 'missing' is not defined in inputSchema.properties
Summary: checked=1 valid=0 invalid=1
```

### JSON 模式

```json
{
  "summary": {
    "files_checked": 1,
    "files_valid": 1,
    "files_invalid": 0
  },
  "results": [
    {
      "file": "cli_config_validator/tests/configs/valid_simple_config.json",
      "valid": true,
      "issues": []
    }
  ]
}
```

## 测试

`tests/` 目录提供最小正反例配置、回归测试和覆盖测试。

主要文件：

- [valid_simple_config.json](tests/configs/valid_simple_config.json)
- [valid_subcommand_config.json](tests/configs/valid_subcommand_config.json)
- [valid_mixed_mapping.json](tests/configs/valid_mixed_mapping.json)
- [json_string.json](tests/configs/json_string.json)
- [invalid_mixed_mapping.json](tests/configs/invalid_mixed_mapping.json)
- [invalid_missing_required_schema.json](tests/configs/invalid_missing_required_schema.json)
- [invalid_unknown_template_field.json](tests/configs/invalid_unknown_template_field.json)
- [test_validator.py](tests/test_validator.py)
- [test_validator_coverage.py](tests/test_validator_coverage.py)
- [testcase.md](tests/testcase.md)

### 基础测试

```bash
python3 cli_config_validator/tests/test_validator.py
```

### 覆盖测试

```bash
python3 cli_config_validator/tests/test_validator_coverage.py
```

当前覆盖情况：

- 覆盖校验内核中已实现且可触发的全部规则码
- 当前覆盖结果为 `158 codes`
- 覆盖脚本同时包含错误码触发用例和对应的正向看护用例

## 推荐工作流

1. 先参考 [config校验规则.md](config校验规则.md) 编写 `config.json`
2. 用 `cli_config_validator` 做本地快速校验
3. 再进入 `ohos_cli_executable` 构建链路验证模板级约束

推荐的最小自检命令：

```bash
python3 cli_config_validator/main.py validate <path/to/config.json>
python3 cli_config_validator/main.py validate <path/to/config.json> --json
```

如果配置用于 `ohos_cli_executable`，还应进一步确认：

- `config.name == target_name`
- 模板相关约束在构建期也通过
