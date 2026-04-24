# 参数映射测试用例集

## 测试用例列表

### 基础测试用例 (001-008)

| 用例序号 | config 文件名 | input 文件名 | 预期输出结果 |
|---------|--------------|--------------|-------------|
| 001 | test_001_flag_basic.json | test_001_input.json | `/usr/bin/test-tool --host=192.168.1.100 --port=8080 --timeout=30` |
| 002 | test_002_flag_array_bool.json | test_002_input.json | `/usr/bin/scanner --target=10.0.0.1 --target=10.0.0.2 --target=10.0.0.3 -v` |
| 003 | test_003_positional.json | test_003_input.json | `/usr/bin/copy /data/source.txt /backup/dest.txt true` |
| 004 | test_004_flattened.json | test_004_input.json | `/usr/bin/deploy --app-name=webapp --replicas=3 --prod --region=us-west-1` |
| 005 | test_005_json_string.json | test_005_input.json | `/usr/bin/cloud-deploy --config={"resources": {"cpu": "2", "memory": "4Gi"}, "tags": ["production", "v1.0"]} --mode=rolling` |
| 006 | test_006_mixed_simple.json | test_006_input.json | `/usr/bin/service start database -v` |
| 007 | test_007_mixed_complex.json | test_007_input.json | `/usr/bin/deployer deploy api-gateway --replicas=5 --host=api.example.com --port=8443 --config={"env": ["production", "secure"], "resources": {"cpu": "4", "memory": "8Gi"}}` |
| 008 | test_008_optional_params.json | test_008_input.json | `/usr/bin/tool --required=value1 --opt2=42` |

### 扩展测试用例 (009-020)

| 用例序号 | config 文件名 | input 文件名 | 预期输出结果 |
|---------|--------------|--------------|-------------|
| 009 | test_009_flag_if_false.json | test_009_input.json | `/usr/bin/logger --release --trace-disable` |
| 010 | test_010_flag_number.json | test_010_input.json | `/usr/bin/scaler --ratio=0.75 --threshold=3.14` |
| 011 | test_011_positional_number.json | test_011_input.json | `/usr/bin/calculator 10.5 3.2 add` |
| 012 | test_012_flattened_separator.json | test_012_input.json | `/usr/bin/deploy-v2 --name=myapp --version=v2.0 --env=production` |
| 013 | test_013_flattened_deep.json | test_013_input.json | `/usr/bin/config-tool --node-name=worker-1 --replicas=5` |
| 014 | test_014_json_string_bool.json | test_014_input.json | `/usr/bin/optimizer --options='{\"enabled\": true, \"level\": \"verbose\"}' --debug` |
| 015 | test_015_mixed_flag_json.json | test_015_input.json | `/usr/bin/analyzer -v --output=json --config='{\"format\": \"detailed\", \"details\": true}'` |
| 016 | test_016_mixed_flag_flat.json | test_016_input.json | `/usr/bin/deployer --force --app=frontend --namespace=default --replicas=3` |
| 017 | test_017_mixed_pos_json.json | test_017_input.json | `/usr/bin/pipeline run myapp --stages='{\"build\": true, \"test\": false, \"deploy\": true}'` |
| 018 | test_018_mixed_pos_flat.json | test_018_input.json | `/usr/bin/installer install /opt/myapp --install-path=/usr/local/bin --mode=symlink` |
| 019 | test_019_mixed_flat_json.json | test_019_input.json | `/usr/bin/executor --service=backend --port=9443 --tls --features='{\"cache\": true, \"monitoring\": false}'` |
| 020 | test_020_edge_cases.json | test_020_input.json | `/usr/bin/processor myapp --metadata={}` |

## 目录结构

```
testcase/
├── config/           # 测试配置文件 (20个)
│   ├── test_001_flag_basic.json
│   ├── test_002_flag_array_bool.json
│   ├── test_003_positional.json
│   ├── test_004_flattened.json
│   ├── test_005_json_string.json
│   ├── test_006_mixed_simple.json
│   ├── test_007_mixed_complex.json
│   ├── test_008_optional_params.json
│   ├── test_009_flag_if_false.json
│   ├── test_010_flag_number.json
│   ├── test_011_positional_number.json
│   ├── test_012_flattened_separator.json
│   ├── test_013_flattened_deep.json
│   ├── test_014_json_string_bool.json
│   ├── test_015_mixed_flag_json.json
│   ├── test_016_mixed_flag_flat.json
│   ├── test_017_mixed_pos_json.json
│   ├── test_018_mixed_pos_flat.json
│   ├── test_019_mixed_flat_json.json
│   └── test_020_edge_cases.json
├── input/            # 测试输入文件 (20个)
│   ├── test_001_input.json
│   ├── test_002_input.json
│   ├── test_003_input.json
│   ├── test_004_input.json
│   ├── test_005_input.json
│   ├── test_006_input.json
│   ├── test_007_input.json
│   ├── test_008_input.json
│   ├── test_009_input.json
│   ├── test_010_input.json
│   ├── test_011_input.json
│   ├── test_012_input.json
│   ├── test_013_input.json
│   ├── test_014_input.json
│   ├── test_015_input.json
│   ├── test_016_input.json
│   ├── test_017_input.json
│   ├── test_018_input.json
│   ├── test_019_input.json
│   └── test_020_input.json
├── expect/           # 预期输出文件 (20个)
│   ├── test_001_expected.json
│   ├── test_002_expected.json
│   ├── test_003_expected.json
│   ├── test_004_expected.json
│   ├── test_005_expected.json
│   ├── test_006_expected.json
│   ├── test_007_expected.json
│   ├── test_008_expected.json
│   ├── test_009_expected.json
│   ├── test_010_expected.json
│   ├── test_011_expected.json
│   ├── test_012_expected.json
│   ├── test_013_expected.json
│   ├── test_014_expected.json
│   ├── test_015_expected.json
│   ├── test_016_expected.json
│   ├── test_017_expected.json
│   ├── test_018_expected.json
│   ├── test_019_expected.json
│   └── test_020_expected.json
└── testcase.md       # 本文件
```

## 测试用例详情

### 001 - Flag 模式 - 基础参数

**映射类型**: Flag  
**测试描述**: 基础字符串和数字参数映射  
**输入参数**: `{"host": "192.168.1.100", "port": 8080, "timeout": 30}`  
**预期输出**: `/usr/bin/test-tool --host=192.168.1.100 --port=8080 --timeout=30`  
**验证点**: 标准 `--key=value` 格式，类型转换正确

---

### 002 - Flag 模式 - 数组和布尔

**映射类型**: Flag  
**测试描述**: 数组展开和布尔条件模板  
**输入参数**: `{"targets": ["10.0.0.1", "10.0.0.2", "10.0.0.3"], "verbose": true, "debug": false}`  
**预期输出**: `/usr/bin/scanner --target=10.0.0.1 --target=10.0.0.2 --target=10.0.0.3 -v`  
**验证点**: 
- 数组展开为多个同名参数
- 布尔值 true 使用 if_true 分支
- 布尔值 false 使用 if_false 分支（空字符串被跳过）

---

### 003 - Positional 模式

**映射类型**: Positional  
**测试描述**: 固定顺序位置参数  
**输入参数**: `{"source": "/data/source.txt", "destination": "/backup/dest.txt", "overwrite": true}`  
**预期输出**: `/usr/bin/copy /data/source.txt /backup/dest.txt true`  
**验证点**: 
- 参数按 order 顺序输出
- 布尔值转换为字符串 "true"

---

### 004 - Flattened 模式

**映射类型**: Flattened  
**测试描述**: 嵌套对象展平为路径参数  
**输入参数**: `{"app": {"name": "webapp", "replicas": 3, "env": {"production": true, "region": "us-west-1"}}}`  
**预期输出**: `/usr/bin/deploy --app-name=webapp --replicas=3 --prod --region=us-west-1`  
**验证点**: 
- 深层嵌套对象正确展平
- 布尔条件模板在展平模式下正常工作

---

### 005 - JSON-String 模式

**映射类型**: JSON-String  
**测试描述**: 复杂对象序列化为 JSON 字符串  
**输入参数**: `{"config": {"resources": {"cpu": "2", "memory": "4Gi"}, "tags": ["production", "v1.0"]}, "mode": "rolling"}`  
**预期输出**: `/usr/bin/cloud-deploy --config={"resources": {"cpu": "2", "memory": "4Gi"}, "tags": ["production", "v1.0"]} --mode=rolling`  
**验证点**: 
- 对象和数组正确序列化为 JSON
- 标量参数正常处理

---

### 006 - Mixed 模式 - 简单组合

**映射类型**: Mixed  
**测试描述**: Positional + Flag 组合  
**输入参数**: `{"action": "start", "name": "database", "verbose": true}`  
**预期输出**: `/usr/bin/service start database -v`  
**验证点**: 
- positional 参数按 order 先输出
- flag 参数按定义顺序后输出
- 输出顺序正确

---

### 007 - Mixed 模式 - 复杂组合

**映射类型**: Mixed  
**测试描述**: 所有 4 种模式组合  
**输入参数**: `{"action": "deploy", "service": "api-gateway", "replicas": 5, "dryRun": false, "server": {"host": "api.example.com", "port": 8443}, "config": {"env": ["production", "secure"], "resources": {"cpu": "4", "memory": "8Gi"}}}`  
**预期输出**: `/usr/bin/deployer deploy api-gateway --replicas=5 --host=api.example.com --port=8443 --config={"env": ["production", "secure"], "resources": {"cpu": "4", "memory": "8Gi"}}`  
**验证点**: 
- 所有模式正确组合
- 输出顺序完全符合规则
- 布尔 false 参数被正确跳过
- 复杂嵌套结构正确处理

---

### 008 - 特殊场景 - 可选参数

**映射类型**: Flag  
**测试描述**: 只提供部分可选参数  
**输入参数**: `{"required_param": "value1", "optional2": 42}`  
**预期输出**: `/usr/bin/tool --required=value1 --opt2=42`  
**验证点**: 
- 只映射输入中实际存在的字段
- 不存在的字段不会报错
- 可选参数处理正确

---

## 扩展测试用例详情 (009-020)

### 009 - Flag 模式 - if_false 分支

**映射类型**: Flag  
**测试描述**: 验证布尔值 false 时使用 if_false 分支  
**输入参数**: `{"debug": false, "trace": false}`  
**预期输出**: `/usr/bin/logger --release --trace-disable`  
**验证点**: 
- 布尔值 false 正确使用 if_false 分支
- if_false 分支的值正确输出

---

### 010 - Flag 模式 - number 类型

**映射类型**: Flag  
**测试描述**: 验证 number 类型（浮点数）参数映射  
**输入参数**: `{"ratio": 0.75, "threshold": 3.14}`  
**预期输出**: `/usr/bin/scaler --ratio=0.75 --threshold=3.14`  
**验证点**: 
- number 类型正确转换为字符串
- 浮点数精度保持正确

---

### 011 - Positional 模式 - number 类型

**映射类型**: Positional  
**测试描述**: 验证 number 类型在 positional 模式下的处理  
**输入参数**: `{"value1": 10.5, "value2": 3.2, "operation": "add"}`  
**预期输出**: `/usr/bin/calculator 10.5 3.2 add`  
**验证点**: 
- number 类型在 positional 模式下正确输出
- 浮点数保持原样输出

---

### 012 - Flattened 模式 - 下划线分隔符

**映射类型**: Flattened  
**测试描述**: 验证使用下划线作为分隔符  
**输入参数**: `{"app_config": {"app_name": "myapp", "app_version": "v2.0", "app_env": {"app_env_type": "production"}}}`  
**预期输出**: `/usr/bin/deploy-v2 --name=myapp --version=v2.0 --env=production`  
**验证点**: 
- 使用 `_` 作为分隔符正确解析路径
- 路径 `app_config_app_name` 正确提取

---

### 013 - Flattened 模式 - 三层嵌套

**映射类型**: Flattened  
**测试描述**: 验证三层嵌套对象的展平  
**输入参数**: `{"cluster": {"node": {"name": "worker-1", "replicas": 5}}}`  
**预期输出**: `/usr/bin/config-tool --node-name=worker-1 --replicas=5`  
**验证点**: 
- 三层嵌套 `cluster.node.name` 正确展平
- 深层嵌套路径提取正确

---

### 014 - JSON-String 模式 - 布尔条件

**映射类型**: JSON-String  
**测试描述**: 验证 jsonString 模式下的布尔条件模板  
**输入参数**: `{"options": {"enabled": true, "level": "verbose"}, "debug": true}`  
**预期输出**: `/usr/bin/optimizer --options='{\"enabled\": true, \"level\": \"verbose\"}' --debug`  
**验证点**: 
- jsonString 模式下支持布尔条件模板
- 对象序列化和布尔条件混合使用

---

### 015 - Mixed 模式 - flag + jsonString

**映射类型**: Mixed  
**测试描述**: 验证 flag 和 jsonString 组合  
**输入参数**: `{"verbose": true, "outputSchema": "json", "config": {"format": "detailed", "details": true}}`  
**预期输出**: `/usr/bin/analyzer -v --output=json --config='{\"format\": \"detailed\", \"details\": true}'`  
**验证点**: 
- flag 和 jsonString 正确组合
- flag 参数按定义顺序输出

---

### 016 - Mixed 模式 - flag + flattened

**映射类型**: Mixed  
**测试描述**: 验证 flag 和 flattened 组合  
**输入参数**: `{"force": true, "dryRun": false, "app": {"name": "frontend", "namespace": "default", "replicas": 3}}`  
**预期输出**: `/usr/bin/deployer --force --app=frontend --namespace=default --replicas=3`  
**验证点**: 
- flag 和 flattened 正确组合
- 布尔值 false 参数被正确跳过

---

### 017 - Mixed 模式 - positional + jsonString

**映射类型**: Mixed  
**测试描述**: 验证 positional 和 jsonString 组合  
**输入参数**: `{"action": "run", "name": "myapp", "stages": {"build": true, "test": false, "deploy": true}}`  
**预期输出**: `/usr/bin/pipeline run myapp --stages='{\"build\": true, \"test\": false, \"deploy\": true}'`  
**验证点**: 
- positional 参数按 order 先输出
- jsonString 参数按定义顺序后输出

---

### 018 - Mixed 模式 - positional + flattened

**映射类型**: Mixed  
**测试描述**: 验证 positional 和 flattened 组合  
**输入参数**: `{"operation": "install", "target": "/opt/myapp", "settings": {"install": {"path": "/usr/local/bin", "mode": "symlink"}}}`  
**预期输出**: `/usr/bin/installer install /opt/myapp --install-path=/usr/local/bin --mode=symlink`  
**验证点**: 
- positional 和 flattened 正确组合
- 三层嵌套路径正确展平

---

### 019 - Mixed 模式 - flattened + jsonString

**映射类型**: Mixed  
**测试描述**: 验证 flattened 和 jsonString 组合  
**输入参数**: `{"verbose": false, "service": {"name": "backend", "port": 9443, "tls": {"enabled": true}}, "features": {"cache": true, "monitoring": false}}`  
**预期输出**: `/usr/bin/executor --service=backend --port=9443 --tls --features='{\"cache\": true, \"monitoring\": false}'`  
**验证点**: 
- flattened 和 jsonString 正确组合
- 布尔值在 flattened 中使用条件模板

---

### 020 - 边界场景 - 空对象和空数组

**映射类型**: Mixed  
**测试描述**: 验证空对象和空数组的处理  
**输入参数**: `{"name": "myapp", "tags": [], "metadata": {}}`  
**预期输出**: `/usr/bin/processor myapp --metadata={}`  
**验证点**: 
- 空数组不生成任何参数
- 空对象序列化为 `{}`
- 边界情况正确处理

---

## 运行测试

### 运行单个测试

```bash
# 在 tests/ 目录下执行
python3 validate_and_run.py \
  --config testcase/config/test_001_flag_basic.json \
  --input testcase/input/test_001_input.json
```

### 批量运行所有测试

```bash
# 使用测试脚本（需要先创建）
./run_testcase_tests.sh
```

## 验证标准

每个测试用例通过标准：
1. ✅ `validate_and_run.py` 执行成功（返回码 0）
2. ✅ 生成的 argv 与预期输出一致
3. ✅ 完整命令字符串正确

## 测试覆盖矩阵

| 映射类型 | 001 | 002 | 003 | 004 | 005 | 006 | 007 | 008 | 009 | 010 | 011 | 012 | 013 | 014 | 015 | 016 | 017 | 018 | 019 | 020 |
|---------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
| Flag | ✅ | ✅ |   |   |   |   | ✅ | ✅ | ✅ | ✅ |   |   |   |   |   |   |   |   |   |   |
| Positional |   |   | ✅ |   |   | ✅ | ✅ |   |   |   | ✅ |   |   |   |   |   | ✅ |   | ✅ |   |
| Flattened |   |   |   | ✅ |   |   | ✅ |   |   |   |   | ✅ | ✅ |   |   | ✅ |   |   | ✅ |   |
| JSON-String |   |   |   |   | ✅ |   | ✅ |   |   |   |   |   |   | ✅ |   |   | ✅ |   | ✅ | ✅ |
| Mixed |   |   |   |   |   | ✅ | ✅ |   |   |   |   |   |   |   | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |

| 数据类型 | 001 | 002 | 003 | 004 | 005 | 006 | 007 | 008 | 009 | 010 | 011 | 012 | 013 | 014 | 015 | 016 | 017 | 018 | 019 | 020 |
|---------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
| string | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |   |   | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| number/integer | ✅ |   |   | ✅ |   |   |   | ✅ |   | ✅ | ✅ |   |   |   |   |   |   |   |   |   |
| number/float |   |   |   |   |   |   |   |   |   | ✅ | ✅ |   |   |   |   |   |   |   |   |   |
| boolean |   | ✅ | ✅ | ✅ |   | ✅ |   |   | ✅ |   |   |   |   | ✅ | ✅ | ✅ |   |   |   |   |
| array |   | ✅ |   |   | ✅ |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ✅ |
| object |   |   |   | ✅ | ✅ |   | ✅ |   |   |   |   | ✅ | ✅ | ✅ | ✅ | ✅ |   | ✅ | ✅ | ✅ |

| 特性 | 001 | 002 | 003 | 004 | 005 | 006 | 007 | 008 | 009 | 010 | 011 | 012 | 013 | 014 | 015 | 016 | 017 | 018 | 019 | 020 |
|------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
| 数组展开 |   | ✅ |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ✅ |
| 条件模板 (if_true) |   | ✅ |   | ✅ |   | ✅ |   |   |   |   |   |   |   | ✅ | ✅ | ✅ |   |   |   |   |
| 条件模板 (if_false) |   |   |   |   |   |   |   |   | ✅ |   |   |   |   |   |   |   |   |   |   |   |
| 顺序控制 |   |   | ✅ |   |   | ✅ | ✅ |   |   |   | ✅ |   |   |   |   |   | ✅ |   | ✅ |   |
| 路径展平 |   |   |   | ✅ |   |   | ✅ |   |   |   |   | ✅ | ✅ |   |   | ✅ |   |   | ✅ |   |
| 深层嵌套 (3层+) |   |   |   |   |   |   |   |   |   |   |   |   |   | ✅ |   |   |   |   | ✅ |   |
| JSON 序列化 |   |   |   |   | ✅ |   | ✅ |   |   |   |   |   |   | ✅ |   |   | ✅ |   | ✅ | ✅ |
| 可选参数 |   |   |   |   |   |   |   | ✅ |   |   |   |   |   |   |   |   |   |   |   |   |
| 浮点数处理 |   |   |   |   |   |   |   |   |   | ✅ | ✅ |   |   |   |   |   |   |   |   |   |
| 分隔符变体 |   |   |   |   |   |   |   |   |   |   |   |   | ✅ |   |   |   |   |   |   |   |
| 空数组/对象 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   | ✅ |

## 扩展指南

添加新测试用例时，请遵循以下命名规范：

1. **配置文件**: `test_XXX_description.json`（XXX 为 3 位编号）
2. **输入文件**: `test_XXX_input.json`
3. **预期文件**: `test_XXX_expected.json`

然后在 `testcase.md` 中添加对应的条目。
