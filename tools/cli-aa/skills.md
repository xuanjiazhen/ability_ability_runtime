# cli-aa 工具使用说明

## 简介

cli-aa 是 OpenHarmony 的 Ability 启动 CLI 工具，提供标准化的 Ability 启动功能。它是 `aa` 工具的轻量级版本，只包含 `start` 功能，使用 OpenHarmony 标准的 CLI 框架实现。

## 功能特性

- 轻量级：只包含核心的 start 功能
- 标准化：使用 OpenHarmony CLI 工具规范
- 事件驱动：使用标准化的 JSON 事件输出格式
- 易于集成：与系统 CLI 工具生态无缝集成

## 安装

工具会被安装到 `/etc/cli/tools/cli-aa`，配置文件位于 `/etc/cli/configs/cli-aa.json`。

## 使用方法

### 基本语法

```bash
cli-aa [选项]
```

### 常用选项

| 短选项 | 长选项 | 参数 | 说明 |
|--------|--------|------|------|
| `-a` | `--ability` | `<name>` | Ability 名称 |
| `-b` | `--bundle` | `<name>` | Bundle 名称 |
| `-m` | `--module` | `<name>` | Module 名称 |
| `-A` | `--action` | `<name>` | Action 名称 |
| `-U` | `--uri` | `<URI>` | URI |
| `-e` | `--entity` | `<entity>` | Entity |
| `-t` | `--type` | `<mime-type>` | MIME 类型 |
| `-u` | `--user` | `<id>` | 用户 ID |
| `-W` | `--wait` | 无 | 等待启动完成 |
| `-h` | `--help` | 无 | 显示帮助信息 |
| `--pi` | | `<key> <int>` | 整数参数 |
| `--ps` | | `<key> <string>` | 字符串参数 |
| `--pb` | | `<key> <bool>` | 布尔参数 |
| `--psn` | | `<key>` | 空字符串参数 |
| `--wl` | | `<left>` | 窗口左边距 |
| `--wt` | | `<top>` | 窗口上边距 |
| `--wh` | | `<height>` | 窗口高度 |
| `--ww` | | `<width>` | 窗口宽度 |

## 使用示例

### 1. 基本 Ability 启动

```bash
# 启动指定 Bundle 和 Ability
cli-aa -a MainAbility -b com.example.app

# 启动带 Module 名称的 Ability
cli-aa -a MainAbility -b com.example.app -m entry
```

### 2. 使用 Action 启动

```bash
# 通过 Action 启动
cli-aa -A android.intent.action.VIEW -b com.example.app
```

### 3. 使用 URI 启动

```bash
# 通过 URI 启动
cli-aa -U "http://www.example.com" -b com.example.app
```

### 4. 带参数启动

```bash
# 带整数参数
cli-aa -a MainAbility -b com.example.app --pi count 100 --pi level 5

# 带字符串参数
cli-aa -a MainAbility -b com.example.app --ps name "Example" --ps description "Test"

# 带布尔参数
cli-aa -a MainAbility -b com.example.app --pb enabled true --pb debug false

# 带空字符串参数
cli-aa -a MainAbility -b com.example.app --psn emptyParam
```

### 5. 设置窗口属性

```bash
# 设置窗口位置和大小
cli-aa -a MainAbility -b com.example.app --wl 100 --wt 200 --wh 800 --ww 600
```

### 6. 等待模式启动

```bash
# 等待 Ability 启动完成
cli-aa -a MainAbility -b com.example.app -W
```

### 7. 指定用户 ID

```bash
# 为特定用户启动 Ability
cli-aa -a MainAbility -b com.example.app -u 100
```

### 8. 组合使用

```bash
# 完整示例
cli-aa -a MainAbility -b com.example.app -m entry \
  --pi count 10 \
  --ps mode "test" \
  --pb debug true \
  --wl 0 --wt 0 --wh 1920 --ww 1080 \
  -u 100 \
  -W
```

## 输出格式

cli-aa 使用标准化的 JSON 事件格式输出结果：

### 成功事件

```json
{"event": "result", "data": {"status": "success", "message": "start ability successfully"}}
```

### 错误事件

```json
{"event": "error", "data": {"error": "error: failed to start ability: Invalid parameter"}}
```

### 进度事件

```json
{"event": "progress", "data": {"percentage": 50, "status": "Starting ability..."}}
```

### 标准输出事件

```json
{"event": "stdout", "data": {"message": "Starting ability..."}}
```

## 错误码

| 错误码 | 说明 |
|--------|------|
| 0 | 成功 |
| -1 | 无效参数 |
| -2 | 未初始化 |

## 与 aa 工具的对比

| 特性 | aa 工具 | cli-aa 工具 |
|------|---------|------------|
| 大小 | 较大（包含所有功能） | 较小（仅 start 功能） |
| 功能 | start、stop、dump、force-stop 等 | 仅 start |
| 输出格式 | 文本格式 | JSON 事件格式 |
| 集成方式 | 独立工具 | 标准 CLI 框架集成 |

## 最佳实践

1. **参数验证**：确保 bundle 名称和 ability 名称正确
2. **错误处理**：检查返回的事件中的 status 字段
3. **超时设置**：工具默认超时为 30 秒
4. **权限**：确保有足够的权限启动目标 Ability

## 故障排查

### Ability 启动失败

1. 检查 bundle 名称和 ability 名称是否正确
2. 检查应用是否已安装
3. 检查是否有足够的权限
4. 查看错误事件中的详细错误信息

### 参数解析错误

1. 使用 `-h` 查看帮助信息
2. 检查参数格式是否正确
3. 确保参数值包含在引号中（如果包含空格）

## 技术支持

如有问题，请参考：
- OpenHarmony Ability 运行时文档
- aa 工具使用文档
- AbilityManagerService 接口文档
