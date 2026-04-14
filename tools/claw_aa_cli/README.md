# ohos-aa 命令行工具

## 1. 概述

`ohos-aa` 是 OpenHarmony Ability 组件管理的命令行工具，遵循 Claw 规范设计。该工具提供 Ability 组件的启动、强制停止等操作能力，所有命令结果均以 JSON 格式输出，便于脚本化调用和自动化测试。

`ohos-aa` 是对原有 `aa` 命令行工具（`tools/aa/`）的 Claw 化封装，复用了 `aa` 工具的核心能力（ShellCommand 框架、AbilityStartWithWait 等），并在输出格式、错误信息等方面进行了规范化增强。

### 目录结构

```
tools/claw_aa_cli/
├── include/
│   └── claw_aa_command.h        # ClawAaShellCommand 类声明，命令定义与参数常量
├── src/
│   ├── main.cpp                 # 程序入口，命令执行与超时控制
│   └── claw_aa_command.cpp      # 命令实现，参数解析与 JSON 输出
├── tests/
│   ├── claw_aa_command_test.cpp # 单元测试
│   └── BUILD.gn                 # 测试构建配置
└── BUILD.gn                     # 构建配置，定义 ohos-aa 可执行目标
```

## 2. CLI 子命令表

| 子命令 | 作用 | 可选参数 |
|--------|------|----------|
| `help` | 显示可用命令列表及帮助信息 | 无 |
| `version` | 显示工具版本号（当前版本：1.0.0） | 无 |
| `start` | 启动指定的 Ability 组件，支持显式启动和隐式启动 | `-h`, `-d`, `-a`, `-b`, `-m`, `-p`, `--ps`, `--pi`, `--pb`, `--psn`, `-A`, `-U`, `-e`, `-t`, `-W`（详见下方参数说明） |
| `force-stop` | 根据包名强制停止应用进程 | `<bundle-name>`（必填，位置参数） |

### start 子命令参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `-h`, `--help` | 标志 | 显示 start 命令的帮助信息 |
| `-d <device-id>` | 字符串 | 指定目标设备 ID |
| `-a <ability-name>` | 字符串 | 指定 Ability 名称（与 `-b` 搭配使用进行显式启动） |
| `-b <bundle-name>` | 字符串 | 指定应用包名（与 `-a` 搭配使用进行显式启动） |
| `-m <module-name>` | 字符串 | 指定模块名称 |
| `-p <perf-cmd>` | 字符串 | 性能调试命令，支持：`dumpheap`、`sleep <N>`、`baseLineProfile`、`profile nativeperf/jsperf` |
| `--ps <key> <value>` | 字符串键值对 | 向 Want 中注入字符串参数 |
| `--pi <key> <value>` | 整数键值对 | 向 Want 中注入整数参数 |
| `--pb <key> <value>` | 布尔键值对 | 向 Want 中注入布尔参数（值为 `true`/`t`/`false`/`f`） |
| `--psn <key>` | 键（无值） | 向 Want 中注入空字符串参数 |
| `-A <action-name>` | 字符串 | 指定 Action，用于隐式启动 |
| `-U <URI>` | 字符串 | 指定 URI |
| `-e <entity>` | 字符串 | 指定 Entity（可多次使用） |
| `-t <mime-type>` | 字符串 | 指定 MIME 类型 |
| `-W` | 标志 | 等待 Ability 启动完成并输出启动耗时信息（不支持隐式启动和非 UIAbility） |

## 3. Claw 规范遵循情况

### 3.1 命令命名规范

- **可执行文件命名**：采用 `<平台前缀>-<工具名>` 格式，即 `ohos-aa`，使用小写字母和连字符
- **子命令命名**：使用小写字母，多词子命令以连字符连接（如 `force-stop`）
- **短选项**：单字母，单连字符前缀（如 `-a`、`-b`、`-W`）
- **长选项**：多字母，双连字符前缀（如 `--help`、`--ps`、`--pi`、`--pb`、`--psn`）

### 3.2 输入格式规范

- **命令格式**：`ohos-aa <command> [options] [arguments]`
- **参数解析**：使用 `getopt_long` 进行标准化 POSIX 风格参数解析
- **键值对参数**：`--ps`/`--pi`/`--pb` 采用 `--<option> <key> <value>` 格式，空字符串参数 `--psn` 采用 `--psn <key>` 格式
- **布尔参数**：支持 `true`/`t`/`false`/`f`（不区分大小写）
- **整数参数**：严格校验为合法整数值，拒绝非整数字符串
- **性能命令参数**（`-p`）：通过正则校验，仅允许 `dumpheap`、`sleep <N>`、`baseLineProfile`、`profile nativeperf/jsperf` 及其合法变体

### 3.3 输出格式规范

所有输出均以 JSON 格式输出到标准输出（stdout），遵循统一的响应结构：

**成功响应**：

```json
{
  "success": true,
  "message": "start ability successfully."
}
```

**错误响应**：

```json
{
  "success": false,
  "message": "error: failed to start ability.",
  "error": {
    "code": "10104001",
    "message": "The specified ability does not exist",
    "cause": "The specified Ability is not installed"
  },
  "solution": "Check if the parameter abilityName of aa -a and the parameter bundleName of -b are correct;Check if the application corresponding to the specified bundleName is installed;For multi-HAP applications, it is necessary to confirm whether the HAP to which the ability belongs has been installed"
}
```

**`-W` 等待模式输出**（非 JSON，结构化文本）：

```
StartMode: Cold
BundleName: com.example.myapp
AbilityName: MainAbility
ModuleName: entry
TotalTime: 1000
WaitTime: 1500
```

### 3.4 错误码体系

工具内置了 20 个标准化错误码映射，覆盖以下场景：

| 错误码 | 含义 |
|--------|------|
| 10100101 | BMS 无法解析 Want 对应的应用信息 |
| 10100102 | 不支持通过 aa start 启动 UIExtensionAbility |
| 10103001 | 目标 Ability 可见性校验失败 |
| 10103101 | 隐式启动未找到匹配的应用 |
| 10103102 | appCloneIndex 参数无效 |
| 10104001 | 指定的 Ability 不存在 |
| 10104002 | 获取指定包信息失败 |
| 10105001 | Ability 服务连接失败 |
| 10105002 | 获取 Ability 服务失败 |
| 10106002 | 不支持 Release 签名应用使用窗口选项 |
| 10106101 | 另一个 Ability 正在启动中，需等待 |
| 10106102 | 开发者模式下锁屏解锁失败 |
| 10106103 | 众测应用已过期 |
| 10106105 | 目标应用受管控 |
| 10106106 | 目标应用受 EDM 管理 |
| 10106107 | 当前设备不支持窗口选项 |
| 10106401 | 杀进程失败 |
| 10106402 | 常驻进程无法终止 |
| 10107102 | 指定进程权限不足 |
| 10108101 | 内部错误（内存分配、超时等） |

### 3.5 其他规范

- **超时保护**：命令执行设有 60 秒超时机制（基于 XCollie），防止命令无限挂起
- **`-W` 等待上限**：最长等待 15 秒获取启动结果
- **日志记录**：使用 HiLog（`AAFwkTag::AA_TOOL` 标签）记录关键操作日志
- **进程退出**：命令执行完毕后调用 `_exit(0)` 确保进程干净退出

## 4. 使用示例

### 查看帮助信息

```bash
# 显示所有可用命令
ohos-aa help

# 显示 start 子命令的帮助信息
ohos-aa start -h
```

### 查看版本号

```bash
ohos-aa version
```

### 显式启动 Ability

```bash
# 基本显式启动
ohos-aa start -a MainAbility -b com.example.myapp

# 指定模块名称启动
ohos-aa start -a MainAbility -b com.example.myapp -m entry

# 指定设备启动
ohos-aa start -d device-001 -a MainAbility -b com.example.myapp
```

### 隐式启动 Ability

```bash
# 通过 Action 隐式启动
ohos-aa start -A ohos.want.action.viewData

# 通过 URI 和 MIME 类型隐式启动
ohos-aa start -U https://www.example.com -t text/html

# 通过 Action + Entity 隐式启动
ohos-aa start -A ohos.want.action.viewData -e entity.default
```

### 携带参数启动

```bash
# 携带字符串参数
ohos-aa start -a MainAbility -b com.example.myapp --ps key1 value1

# 携带整数参数
ohos-aa start -a MainAbility -b com.example.myapp --pi count 100

# 携带布尔参数
ohos-aa start -a MainAbility -b com.example.myapp --pb enabled true

# 携带空字符串参数
ohos-aa start -a MainAbility -b com.example.myapp --psn optionalKey

# 组合使用多种参数
ohos-aa start -a MainAbility -b com.example.myapp \
  --ps name "test" \
  --pi timeout 30000 \
  --pb debug true
```

### 等待启动完成（-W 模式）

```bash
# 等待 Ability 启动完成并输出启动耗时
ohos-aa start -a MainAbility -b com.example.myapp -W
```

### 强制停止应用

```bash
# 根据包名强制停止应用进程
ohos-aa force-stop com.example.myapp
```