# CLI 工具注册表实现完成报告

## 实现完成时间
2025-03-26

## 实现概述

根据 `cli_docs/DESIGN.md` 中的设计文档，完成了 CLI 工具注册表管理器的实现，包括工具注册、查询、验证和热更新功能。

---

## 实现的文件

### 1. registry_manager.h
- **位置**: `services/climgr/src/registry_manager.h`
- **行数**: 200 行
- **主要功能**: 工具注册管理器的头文件定义

### 2. registry_manager.cpp
- **位置**: `services/climgr/src/registry_manager.cpp`
- **行数**: 728 行
- **主要功能**: 工具注册管理器的实现

---

## 主要功能模块

### 1. 工具注册与注销

```cpp
// 注册工具
int32_t RegisterTool(const ToolInfo& tool);

// 注销工具
int32_t UnregisterTool(const std::string& name);
```

**功能说明**:
- 支持动态注册和注销工具
- 自动验证工具名称的有效性
- 防止重复注册

### 2. 工具查询

```cpp
// 查询所有工具
int32_t QueryTools(std::vector<ToolInfo>& tools);

// 按名称查询工具
int32_t QueryToolByName(const std::string& name, ToolInfo& tool);

// 按权限过滤工具
int32_t QueryToolsByPermission(const std::string& permission, std::vector<ToolInfo>& tools);

// 检查工具是否存在
bool ToolExists(const std::string& name);
```

**功能说明**:
- 支持多种查询方式
- 自动过滤未启用的工具
- 线程安全的查询操作

### 3. 注册表加载与解析

```cpp
// 加载注册表
int32_t LoadRegistry(const std::string& registryPath);

// 重新加载注册表
int32_t ReloadRegistry();

// 创建空注册表
bool CreateEmptyRegistry(const std::string& registryPath);
```

**功能说明**:
- 支持从 JSON 文件加载工具注册信息
- 自动验证注册表格式和版本
- 创建空注册表以便后续添加工具

### 4. 工具验证

```cpp
// 验证单个工具
bool ValidateToolEntry(const ToolInfo& tool);

// 验证工具名称
bool ValidateToolName(const std::string& name);

// 验证所有工具
bool ValidateAllTools();
```

**安全验证包括**:
1. 可执行文件路径必须在 `/etc/cli/tools/` 目录下
2. 文件必须存在
3. 必须是常规文件
4. 不允许 setuid/setgid
5. 文件所有者必须是 root
6. 权限不能超过 755
7. 必须可执行

**工具名称验证**:
1. 长度限制：1-64 个字符
2. 只允许小写字母、数字和连字符
3. 不能以连字符开头或结尾
4. 不能有连续的连字符

### 5. 热更新 (文件监控)

```cpp
// 启动文件监控
void StartRegistryWatcher();

// 停止文件监控
void StopRegistryWatcher();
```

**功能说明**:
- 使用 `inotify` 监控注册表文件变化
- 当文件修改时自动重新加载
- 支持以下事件：
  - `IN_MODIFY`: 文件内容被修改
  - `IN_MOVED_TO`: 文件被移动到监控位置
  - `IN_DELETE_SELF`: 文件被删除

**RegistryWatcher 类**:
- 独立的文件监控线程
- 非阻塞 I/O 操作
- 自动延迟重载（100ms），确保文件写入完成

### 6. 工具描述解析

```cpp
// 从 JSON 解析工具
bool ParseToolFromJson(const Json::Value& toolJson, ToolInfo& tool);
```

**支持的字段**:
- `name`: 工具名称（必填）
- `version`: 工具版本（默认：1.0.0）
- `description`: 工具描述
- `executable`: 可执行文件路径（必填）
- `permissions`: 所需权限列表
- `input`: 输入参数 Schema
- `output`: 输出结果 Schema
- `argumentMapping`: 参数映射规则
- `timeout`: 超时时间（默认：30000ms）
- `events`: 支持的事件类型
- `hasSubcommands`: 是否支持子命令
- `subcommands`: 子命令映射

### 7. 注册表状态

```cpp
// 获取注册表状态
int32_t GetRegistryStatus(RegistryStatus& status);
```

**返回信息**:
- `registryPath`: 注册表文件路径
- `lastModified`: 最后修改时间
- `toolCount`: 工具数量
- `isValid`: 是否有效
- `errorMessage`: 错误消息

---

## 注册表文件格式

### 文件位置
`/etc/cli/cli_tools.json`

### 格式示例

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tools-registry/v1",
  "version": "1.0.0",
  "lastUpdated": "2025-03-26T10:30:00Z",
  "tools": [
    {
      "name": "net-scan",
      "version": "1.0.0",
      "description": "Network scanner tool",
      "executable": "/etc/cli/tools/net-scan",
      "permissions": ["ohos.permission.INTERNET"],
      "timeout": 30000,
      "input": {
        "type": "object",
        "properties": {
          "target": {
            "type": "string",
            "description": "Target address"
          }
        }
      },
      "argumentMapping": {
        "mode": "flag",
        "mapping": {
          "target": "--target"
        }
      }
    }
  ]
}
```

---

## 线程安全

### 互斥锁保护

```cpp
std::mutex toolsMutex_;      // 保护工具注册表
std::mutex watcherMutex_;    // 保护文件监控器
```

### 线程安全操作
- 所有读写 `tools_` 的操作都使用 `toolsMutex_` 保护
- 文件监控器的启动/停止使用 `watcherMutex_` 保护
- 使用 `std::atomic<bool>` 保证 `running_` 标志的原子性

---

## 日志记录

### 日志标签

**RegistryManager**:
```cpp
constexpr OHOS::HiviewDFX::HiLogLabel TAG = {LOG_CORE, 0, "RegistryManager"};
```

**RegistryWatcher**:
```cpp
OHOS::HiviewDFX::HiLogLabel tag_ = {LOG_CORE, 0, "RegistryWatcher"};
```

### 日志级别
- `HILOG_INFO`: 正常操作日志
- `HILOG_WARN`: 警告信息
- `HILOG_ERROR`: 错误信息

---

## 错误处理

### 错误码定义

```cpp
namespace ErrCode {
    constexpr int32_t OK = 0;
    constexpr int32_t ERR_NOT_FOUND = 1;
    constexpr int32_t ERR_INVALID_ARGS = 3;
    constexpr int32_t ERR_INVALID_SCHEMA = 8;
    constexpr int32_t ERR_ALREADY_EXISTS = 9;
}
```

### 错误处理策略
1. **注册表加载失败**: 创建空注册表
2. **工具验证失败**: 记录警告日志，跳过该工具
3. **文件监控启动失败**: 记录警告日志，继续运行
4. **JSON 解析失败**: 返回错误码，记录详细错误信息

---

## 使用示例

### 1. 初始化

```cpp
RegistryManager registry;
if (!registry.Initialize()) {
    // 处理初始化失败
}
```

### 2. 查询工具

```cpp
// 查询所有工具
std::vector<ToolInfo> tools;
int32_t ret = registry.QueryTools(tools);

// 按名称查询
ToolInfo tool;
ret = registry.QueryToolByName("net-scan", tool);

// 按权限查询
std::vector<ToolInfo> filteredTools;
ret = registry.QueryToolsByPermission("ohos.permission.INTERNET", filteredTools);
```

### 3. 注册工具

```cpp
ToolInfo tool;
tool.name = "my-tool";
tool.version = "1.0.0";
tool.executablePath = "/etc/cli/tools/my-tool";
tool.permissions.push_back("ohos.permission.INTERNET");

int32_t ret = registry.RegisterTool(tool);
if (ret == ErrCode::OK) {
    // 注册成功
}
```

### 4. 重新加载

```cpp
int32_t ret = registry.ReloadRegistry();
```

---

## 依赖项

### 系统库
- `libjson`: JSON 解析
- `libhilog`: 日志记录
- `libinotify`: 文件监控（Linux）

### 内部模块
- `cli_tool_info.h`: 工具信息定义
- `cli_manager_error.h`: 错误码定义

---

## 安全考虑

### 1. 可执行文件验证
- 路径白名单：只允许 `/etc/cli/tools/` 目录
- 所有权检查：必须由 root 拥有
- 权限检查：不允许 setuid/setgid
- 可执行性检查：必须有执行权限

### 2. 工具名称验证
- 防止路径遍历攻击
- 限制字符集和格式

### 3. 文件监控
- 监控注册表文件变化
- 防止未授权修改

---

## 性能特性

### 1. 启动性能
- 延迟加载：启动时只加载注册表
- 按需验证：首次使用时验证可执行文件

### 2. 查询性能
- 内存哈希表：O(1) 查询复杂度
- 读多锁少：高效的并发读取

### 3. 热更新性能
- 非阻塞监控：不影响主线程
- 延迟重载：避免频繁重载

---

## 待完善功能

### 1. 持久化
- 当前工具注册只在内存中
- 可考虑添加将运行时注册的工具写入注册表文件的功能

### 2. 通知机制
- 注册表更新后通知相关组件
- 实现观察者模式

### 3. 版本管理
- 支持多版本工具共存
- 工具版本切换

### 4. 依赖管理
- 工具依赖检查
- 依赖冲突检测

---

## 测试建议

### 1. 单元测试
- 工具注册/注销
- 工具查询
- 工具验证
- JSON 解析

### 2. 集成测试
- 注册表加载
- 热更新
- 并发访问

### 3. 安全测试
- 路径遍历攻击
- 权限提升攻击
- 注入攻击

---

## 总结

✅ **已完成的功能**:
1. 工具注册与注销
2. 工具查询（全部、按名称、按权限）
3. 注册表加载与解析
4. 工具验证（安全、格式）
5. 热更新（inotify 文件监控）
6. 线程安全保护
7. 完整的日志记录
8. 错误处理

📊 **代码统计**:
- 头文件: 200 行
- 实现文件: 728 行
- 总计: 928 行

🔐 **安全特性**:
- 可执行文件白名单
- 所有者验证（root）
- 权限检查（无 setuid/setgid）
- 工具名称格式验证

⚡ **性能特性**:
- O(1) 查询复杂度
- 非阻塞文件监控
- 高效并发读取

---

**工具注册表实现完成！** 🎉
