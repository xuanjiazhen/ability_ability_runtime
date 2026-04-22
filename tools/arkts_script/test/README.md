# ohos-arkts 测试用例

## 测试用例结构

```
test/
├── standalone_skill/          # 独立 Skill 测试用例
│   ├── Calculator.ets         # 计算器 Skill
│   └── Calculator.abc         # 编译后的 ABC 文件（需手动编译）
├── app_skill/                 # 应用 Skill 测试用例
│   ├── main.ets               # 主模块
│   └── main.abc               # 编译后的 ABC 文件（需手动编译）
└── README.md                  # 本文件
```

## 编译测试用例

### 1. 独立 Skill 编译

```bash
# 进入独立 Skill 目录
cd test/standalone_skill

# 使用 ets_frontend 编译（假设已配置环境）
ets_frontend --output Calculator.abc Calculator.ets

# 或指定完整路径
/path/to/ets_frontend --output Calculator.abc Calculator.ets
```

### 2. 应用 Skill 编译

```bash
# 进入应用 Skill 目录
cd test/app_skill

# 编译
ets_frontend --output main.abc main.ets
```

## 测试方法

### 独立 Skill 测试

独立 Skill 需要指定 script 类名，格式：
```bash
ohos-arkts -abc <abc文件路径> <ScriptClass> <functionName> [arguments...]
```

#### 测试用例 1：同步加法
```bash
ohos-arkts -abc test/standalone_skill/Calculator.abc Calculator add "10" "20"
```
**预期输出**：
```json
{"success":true,"operation":"add","result":30,"inputs":[10,20]}
```

#### 测试用例 2：同步减法
```bash
ohos-arkts -abc test/standalone_skill/Calculator.abc Calculator subtract "50" "15"
```
**预期输出**：
```json
{"success":true,"operation":"subtract","result":35,"inputs":[50,15]}
```

#### 测试用例 3：异步乘法（Promise）
```bash
ohos-arkts -abc test/standalone_skill/Calculator.abc Calculator multiply "5" "6"
```
**预期输出**：
```json
{"success":true,"operation":"multiply","result":30,"inputs":[5,6]}
```

#### 测试用例 4：异步除法（Promise resolve）
```bash
ohos-arkts -abc test/standalone_skill/Calculator.abc Calculator divide "100" "4"
```
**预期输出**：
```json
{"success":true,"operation":"divide","result":25,"inputs":[100,4]}
```

#### 测试用例 5：异步除法（Promise reject）
```bash
ohos-arkts -abc test/standalone_skill/Calculator.abc Calculator divide "100" "0"
```
**预期输出**（错误）：
```json
{"success":false,"error":"Execution error","details":"{\"success\":false,...}"}
```

#### 测试用例 6：手动完成模式
```bash
ohos-arkts -abc test/standalone_skill/Calculator.abc Calculator manualComplete "testOp" "testResult"
```
**预期输出**：
```json
{"success":true,"operation":"testOp","result":"testResult","mode":"manual"}
```

#### 测试用例 7：字符串拼接
```bash
ohos-arkts -abc test/standalone_skill/Calculator.abc Calculator concat "hello" "world" "test"
```
**预期输出**：
```json
{"success":true,"operation":"concat","result":"hello-world-test","count":3}
```

#### 测试用例 8：无参数调用
```bash
ohos-arkts -abc test/standalone_skill/Calculator.abc Calculator noArgs
```
**预期输出**：
```json
{"success":true,"operation":"noArgs","message":"No arguments provided"}
```

### 应用 Skill 测试

应用 Skill 不需要指定 script 类名，格式：
```bash
ohos-arkts -abc <abc文件路径> <functionName> [arguments...]
```

#### 测试用例 1：同步计算（加法）
```bash
ohos-arkts -abc test/app_skill/main.abc calculate "add" "10" "20"
```
**预期输出**：
```json
{"success":true,"operation":"add","result":30,"inputs":[10,20]}
```

#### 测试用例 2：同步计算（除法错误）
```bash
ohos-arkts -abc test/app_skill/main.abc calculate "divide" "10" "0"
```
**预期输出**：
```json
{"success":false,"error":"Division by zero"}
```

#### 测试用例 3：异步计算
```bash
ohos-arkts -abc test/app_skill/main.abc asyncCalculate "200" "testValue"
```
**预期输出**：
```json
{"success":true,"operation":"asyncCalculate","delay":200,"value":"testValue","timestamp":1234567890}
```

#### 测试用例 4：字符串处理（转大写）
```bash
ohos-arkts -abc test/app_skill/main.abc processString "upper" "hello world"
```
**预期输出**：
```json
{"success":true,"operation":"upper","input":"hello world","result":"HELLO WORLD"}
```

#### 测试用例 5：数组操作
```bash
ohos-arkts -abc test/app_skill/main.abc arrayOperations "join" "a" "b" "c" "d"
```
**预期输出**：
```json
{"success":true,"operation":"join","result":"a,b,c,d","items":["a","b","c","d"]}
```

#### 测试用例 6：手动完成模式
```bash
ohos-arkts -abc test/app_skill/main.abc manualProcess "step2"
```
**预期输出**：
```json
{"success":true,"step":"step2","message":"Processing completed","mode":"manual"}
```

#### 测试用例 7：异步错误（Promise reject）
```bash
ohos-arkts -abc test/app_skill/main.abc asyncError "Custom async error message"
```
**预期输出**（错误）：
```json
{"success":false,"error":"Execution error","details":"{\"success\":false,...}"}
```

## 错误场景测试

### 1. 文件不存在
```bash
ohos-arkts -abc test/standalone_skill/NonExistent.abc Calculator add "1" "2"
```
**预期输出**：
```json
{"success":false,"errorType":"LOAD_ERROR","error":"failed to load ABC file: ..."}
```

### 2. 函数不存在
```bash
ohos-arkts -abc test/standalone_skill/Calculator.abc Calculator nonExistentFunction "1" "2"
```
**预期输出**：
```json
{"success":false,"errorType":"FUNCTION_ERROR","error":"failed to resolve function: nonExistentFunction"}
```

### 3. 参数缺失
```bash
ohos-arkts -abc test/standalone_skill/Calculator.abc
```
**预期输出**：
```json
{"success":false,"errorType":"ARGUMENT_ERROR","error":"Missing functionName"}
```

### 4. 超时测试（需要创建长时间运行的函数）
```bash
# 创建一个 sleep 函数，运行时间超过 300 秒
ohos-arkts -abc test/app_skill/main.abc asyncCalculate "310000" "timeout"
```
**预期输出**（超时）：
```json
{"success":false,"errorType":"TIMEOUT_ERROR","error":"execution timeout"}
```

## 自动化测试脚本

可以创建 shell 脚本批量运行测试：

```bash
#!/bin/bash
# test_runner.sh

ABC_PATH="$1"
TEST_TYPE="$2"  # standalone 或 app

run_test() {
    local name="$1"
    shift
    echo "Running: $name"
    echo "Command: ohos-arkts $*"
    result=$(ohos-arkts "$@" 2>&1)
    echo "Result: $result"
    echo "---"
}

if [ "$TEST_TYPE" == "standalone" ]; then
    # 独立 Skill 测试
    run_test "Add" -abc "$ABC_PATH" Calculator add "10" "20"
    run_test "Subtract" -abc "$ABC_PATH" Calculator subtract "50" "15"
    run_test "Multiply" -abc "$ABC_PATH" Calculator multiply "5" "6"
    run_test "Concat" -abc "$ABC_PATH" Calculator concat "hello" "world"
elif [ "$TEST_TYPE" == "app" ]; then
    # 应用 Skill 测试
    run_test "Calculate Add" -abc "$ABC_PATH" calculate "add" "10" "20"
    run_test "Process String" -abc "$ABC_PATH" processString "upper" "test"
    run_test "Array Operations" -abc "$ABC_PATH" arrayOperations "count" "a" "b" "c"
fi
```

使用方法：
```bash
./test_runner.sh test/standalone_skill/Calculator.abc standalone
./test_runner.sh test/app_skill/main.abc app
```

## 注意事项

1. **编译依赖**：确保 `ets_frontend` 工具可用并正确配置
2. **路径问题**：测试时使用绝对路径或相对于当前工作目录的相对路径
3. **权限**：确保 ohos-arkts 可执行文件有执行权限
4. **超时**：默认超时时间为 300 秒，可通过修改代码调整
5. **日志**：查看系统日志获取详细执行信息
   ```bash
   hilog | grep ohos-arkts
   ```