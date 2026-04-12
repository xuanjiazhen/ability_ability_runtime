# CLI 工具示例

本目录包含两个 CLI 工具示例，演示不同的使用模式。

## cli-example - 有子命令的工具

**目录**: `cli-example/`

**特点**: 使用 `hasSubcommands: true`，包含多个子命令

**子命令**:
- `run [args...]` - 运行工具并处理参数
- `version` - 显示版本信息

**配置格式**:
```json
{
  "hasSubcommands": true,
  "subcommands": {
    "run": {
      "description": "运行示例工具",
      "inputSchema": {...},
      "outputSchema": {...},
      "eventTypes": ["progress", "result"],
      "argumentMapping": {...}
    },
    "version": {...}
  }
}
```

**使用示例**:
```bash
# 运行子命令
cli-example run arg1 arg2
# 输出: {"event": "progress", "data": {...}}
#      {"event": "result", "data": {"result": "执行完成 arg1 arg2"}}

# 显示版本
cli-example version
# 输出: {"event": "result", "data": {"version": "1.0.0", "build_time": "..."}}
```

---

## cli-simple - 无子命令的工具

**目录**: `cli-simple/`

**特点**: 使用 `hasSubcommands: false`，直接在顶层定义 inputSchema、outputSchema

**功能**: 处理命令行参数并重复显示消息

**配置格式**:
```json
{
  "hasSubcommands": false,
  "inputSchema": {
    "type": "object",
    "properties": {
      "message": {"type": "string"},
      "count": {"type": "integer"},
      "verbose": {"type": "boolean"}
    }
  },
  "outputSchema": {
    "type": "object",
    "properties": {
      "status": {"type": "string"},
      "message": {"type": "string"},
      "repeat_count": {"type": "integer"}
    }
  },
  "argumentMapping": {
    "type": "flag",
    "templates": {
      "message": "--message={value}",
      "count": "--count={value}",
      "verbose": "--verbose"
    }
  }
}
```

**使用示例**:
```bash
# 基本使用
cli-simple
# 输出: {"event": "result", "data": {"status": "success", "message": "Hello from cli-simple", "repeat_count": 1}}

# 设置消息
cli-simple --message="Hello World"
# 输出: {"event": "result", "data": {"status": "success", "message": "Hello World", "repeat_count": 1}}

# 重复消息
cli-simple --message="Hi" --count=3
# 输出: {"event": "result", "data": {"status": "success", "message": "Hi Hi Hi", "repeat_count": 3}}

# 详细模式
cli-simple --message="Test" --count=2 --verbose
# 输出: {"event": "stdout", "data": {"message": "Processing with parameters:"}}
#      {"event": "stdout", "data": {"message": "Iteration 1/2"}}
#      {"event": "stdout", "data": {"message": "Iteration 2/2"}}
#      {"event": "result", "data": {"status": "success", "message": "Test Test", "repeat_count": 2}}
```

---

## 何时使用哪种模式？

### 使用子命令模式 (hasSubcommands: true)
- 工具需要执行多个不同的操作
- 类似 git、npm、docker 等工具的用法
- 每个子命令有不同的参数和输出格式

**示例**: `git commit`, `git push`, `git pull`

### 使用无子命令模式 (hasSubcommands: false)
- 工具只执行单一功能
- 所有参数都是针对同一个操作
- 参数通过标志传递

**示例**: `ls -la`, `grep --count=10`, `cli-simple --message="Hi"`

---

## 事件输出

两个工具都使用标准的事件输出格式：

```json
{"event": "事件类型", "data": {...}}
```

**支持的事件类型**:
- `stdout` - 标准输出消息
- `stderr` - 标准错误消息
- `progress` - 进度更新（包含 percentage 和 status）
- `result` - 最终结果
- `error` - 错误信息

---

## 编译

```bash
# 编译 cli-example
./build.sh --product-name rk3568 --build-target cli-example

# 编译 cli-simple
./build.sh --product-name rk3568 --build-target cli-simple

# 编译所有工具
./build.sh --product-name rk3568 --build-target ability_runtime_tools
```
