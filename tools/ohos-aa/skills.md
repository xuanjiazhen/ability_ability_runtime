# ohos-aa 工具使用说明

## 简介

`ohos-aa` 是 OpenHarmony 的轻量级 Ability 启动工具。

当前命令结构：

- 顶层：
  - `-v`, `--version`
- 子命令：
  - `help`
  - `start`

## 基本语法

```bash
ohos-aa --version
ohos-aa help
ohos-aa start [options]
```

## `help` 子命令

```bash
ohos-aa help
```

用于显示 `ohos-aa` 的命令帮助，输出 `stdout` 事件，帮助内容组织参考 `aa help`。

## `start` 子命令支持的参数

| 短选项 | 长选项 | 参数 | 说明 |
|--------|--------|------|------|
| `-h` | `--help` | 无 | 显示 `start` 子命令帮助 |
| `-d` | `--device` | `<id>` | 设备 ID |
| `-a` | `--ability` | `<name>` | Ability 名称 |
| `-b` | `--bundle` | `<name>` | Bundle 名称 |
| `-s` | `--setting` | `<value>` | 窗口模式设置 |
| `-m` | `--module` | `<name>` | Module 名称 |
| `-A` | `--action` | `<name>` | Action 名称 |
| `-U` | `--URI` | `<uri>` | URI |
| `-W` | `--wait` | 无 | 等待启动完成 |
| `-e` | `--entity` | `<entity>` | Entity |
| `-t` | `--type` | `<mime-type>` | MIME 类型 |
| `-u` | `--user` | `<id>` | 用户 ID |
|  | `--pi` | `<key> <int>` | 整数参数 |
|  | `--ps` | `<key> <string>` | 字符串参数 |
|  | `--pb` | `<key> <bool>` | 布尔参数 |
|  | `--psn` | `<key>` | 空字符串参数 |
|  | `--wl` | `<left>` | 窗口左边距 |
|  | `--wt` | `<top>` | 窗口上边距 |
|  | `--wh` | `<height>` | 窗口高度 |
|  | `--ww` | `<width>` | 窗口宽度 |

## 使用示例

### 顶层版本

```bash
ohos-aa --version
```

### 帮助

```bash
ohos-aa help
```

### 启动 Ability

```bash
ohos-aa start -a MainAbility -b com.example.app
```

### 带 Module 启动

```bash
ohos-aa start -a MainAbility -b com.example.app -m entry
```

### 通过 Action 启动

```bash
ohos-aa start -A android.intent.action.VIEW -b com.example.app
```

### 通过 URI 启动

```bash
ohos-aa start -U "http://www.example.com" -b com.example.app
```

### 携带自定义参数

```bash
ohos-aa start -a MainAbility -b com.example.app --pi count 100 --ps name Example --pb debug true
```

### 设置窗口属性

```bash
ohos-aa start -a MainAbility -b com.example.app --wl 100 --wt 200 --wh 800 --ww 600
```

## 输出格式

当前实现只输出以下结构化事件：

### `stdout`

帮助信息输出：

```json
{"event": "stdout", "data": {"message": "usage: ohos-aa <command> <options>..."}}
```

### `result`

成功启动：

```json
{"event": "result", "data": {"status": "success", "message": "start ability successfully"}}
```

版本输出：

```json
{"event": "result", "data": {"version": "1.0.0"}}
```

## 错误处理

- `ohos-aa` 不输出 `error` 事件
- 错误信息直接输出到 `stderr`
- 错误时返回非零退出码

## 当前约束

- 旧用法 `ohos-aa --ability ...` 已完全废弃
- 当前配置文件采用有子命令格式，顶层与 `start` 子命令独立定义
- `help` 子命令运行时不需要额外参数；当前 `config.json` 中使用了一个未实际使用的占位字段来满足现有校验模型
- `--ps`、`--pi`、`--pb`、`--psn` 在源码中支持，但当前 `config.json` 不对这些组合参数做正式建模
