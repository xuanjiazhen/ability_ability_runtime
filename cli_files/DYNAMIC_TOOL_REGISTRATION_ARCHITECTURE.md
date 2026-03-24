# 动态工具注册架构设计文档

## 版本信息

- **版本**: v1.0
- **日期**: 2025-03-26
- **状态**: 设计完成

## 1. 概述

### 1.1 功能描述

动态工具注册功能允许系统服务和底层组件在运行时动态注册和管理 CLI 工具，无需修改注册表文件或重启系统。这是对静态文件扫描机制的重要补充。

### 1.2 设计目标

1. **运行时注册**: 支持在系统运行时动态注册工具
2. **权限控制**: 确保只有授权的组件可以注册工具
3. **持久化选项**: 支持内存持久和文件持久两种模式
4. **IPC 通信**: 跨进程调用，服务隔离
5. **原子操作**: 批量注册支持事务性

### 1.3 适用场景

- 系统服务启动时注册自己的 CLI 工具
- 第三方服务动态注册工具
- 工具版本更新和升级
- 临时工具的注册和注销

## 2. 架构设计

### 2.1 整体架构

```
┌─────────────────────────────────────────────────────────────────┐
│                      调用者（系统服务）                          │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │NetworkService│  │StorageService│  │SecuritySvc   │  ...     │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘          │
└─────────┼──────────────────┼──────────────────┼────────────────┘
          │                  │                  │
          │ InnerKits API    │                  │
          └──────────────────┼──────────────────┘
                             ▼
          ┌─────────────────────────────────────┐
          │    CliToolRegistry (InnerKits)      │
          │  interfaces/inner_api/              │
          │  cli_tool_registry.{h,cpp}         │
          └──────────────┬──────────────────────┘
                         │ IPC (Binder)
                         ▼
          ┌─────────────────────────────────────┐
          │      CLIManagerService              │
          │  (SystemAbility, SA ID=9001)        │
          │  services/climgr/src/               │
          └──────────────┬──────────────────────┘
                         │
          ┌──────────────┼──────────────┐
          ▼              ▼              ▼
    ┌──────────┐  ┌──────────┐  ┌──────────┐
    │Registry  │  │Process   │  │Permission│
    │Manager   │  │Manager   │  │Checker   │
    └──────────┘  └──────────┘  └──────────┘
```

### 2.2 组件职责

| 组件 | 职责 | 文件位置 |
|------|------|----------|
| CliToolRegistry | 提供客户端 API，封装 IPC 通信 | interfaces/inner_api/cli_tool_registry/ |
| CLIManagerService | 服务端实现，处理注册请求 | services/climgr/src/ |
| RegistryManager | 工具注册表管理 | services/climgr/src/registry_manager.{h,cpp} |
| PermissionChecker | 权限校验 | services/climgr/src/permission_checker.{h,cpp} |

## 3. 接口设计

### 3.1 InnerKits 接口 (cli_tool_registry.h)

```cpp
class CliToolRegistry {
public:
    static CliToolRegistry* GetInstance();
    bool Initialize();
    void Release();

    // 工具注册
    int32_t RegisterTool(const ToolInfo& tool, bool persist = true);
    int32_t RegisterTools(const std::vector<ToolInfo>& tools, bool persist = true);

    // 工具更新和注销
    int32_t UpdateTool(const ToolInfo& tool, bool persist = true);
    int32_t UnregisterTool(const std::string& name, bool persist = true);

    // 工具查询
    int32_t QueryTool(const std::string& toolName, ToolInfo& tool);
    int32_t QueryAllTools(std::vector<ToolInfo>& tools);
    int32_t IsToolRegistered(const std::string& name, bool& exists);

    // 注册表管理
    int32_t ReloadRegistry();
    int32_t SetRegistryChangeListener(std::shared_ptr<RegistryChangeListener> callback);
};
```

### 3.2 服务端接口 (cli_manager_service_interface.h)

```cpp
class ICLIManagerService : public IRemoteBroker {
public:
    // 动态工具注册接口
    virtual int32_t RegisterTool(const ToolInfo& tool, bool persist) = 0;
    virtual int32_t RegisterTools(const std::vector<ToolInfo>& tools, bool persist) = 0;
    virtual int32_t UpdateTool(const ToolInfo& tool, bool persist) = 0;
    virtual int32_t UnregisterTool(const std::string& name, bool persist) = 0;
    virtual int32_t QueryTool(const std::string& toolName, ToolInfo& tool) = 0;
    virtual int32_t ReloadRegistry() = 0;
};
```

### 3.3 IPC 消息定义

```cpp
enum {
    // 查询和执行
    QUERY_TOOLS = 1,
    EXEC_TOOL,
    SUBSCRIBE_SESSION,
    CANCEL_SESSION,
    QUERY_SESSION_STATUS,

    // 动态注册
    REGISTER_TOOL = 100,
    REGISTER_TOOLS,
    UPDATE_TOOL,
    UNREGISTER_TOOL,
    QUERY_TOOL,
    RELOAD_REGISTRY,
};
```

## 4. 数据流设计

### 4.1 工具注册流程

```
┌─────────────┐
│ 调用者服务   │
└──────┬──────┘
       │ 1. RegisterTool(tool, true)
       ▼
┌─────────────────────┐
│ CliToolRegistry     │
│ (InnerKits Client)  │
└──────┬──────────────┘
       │ 2. IPC 调用
       ▼
┌─────────────────────┐
│ CLIManagerService   │
│ (RPC Handler)       │
└──────┬──────────────┘
       │ 3. CheckRegisterPermission()
       ▼
┌─────────────────────┐
│ PermissionChecker   │
└──────┬──────────────┘
       │ 权限验证结果
       ▼
┌─────────────────────┐
│ CLIManagerService   │
└──────┬──────────────┘
       │ 4. RegisterTool(tool)
       ▼
┌─────────────────────┐
│ RegistryManager     │
│ (Memory Update)     │
└──────┬──────────────┘
       │ 5. persist? Yes
       ▼
┌─────────────────────┐
│ PersistToolToFile() │
│ (File Update)       │
└──────┬──────────────┘
       │
       ▼
    ┌────────┐
    │ 返回结果 │
    └────────┘
```

### 4.2 批量注册流程

```
┌─────────────┐
│ 调用者服务   │
└──────┬──────┘
       │ RegisterTools({tool1, tool2, ...}, true)
       ▼
┌─────────────────────┐
│ CliToolRegistry     │
└──────┬──────────────┘
       │ IPC 调用
       ▼
┌─────────────────────┐
│ CLIManagerService   │
└──────┬──────────────┘
       │
       ├─► Register(tool1) ──► OK ─┐
       ├─► Register(tool2) ──► OK ─┤
       └─► Register(tool3) ──► FAIL│
                                  │
                     全部失败 ────┘
                     (原子操作)
```

## 5. 持久化设计

### 5.1 持久化策略

| persist 值 | 内存 | 文件 | 说明 |
|-----------|------|------|------|
| `true` | ✅ | ✅ | 持久化到 `/etc/cli/cli_tools.json` |
| `false` | ✅ | ❌ | 仅内存，系统重启后失效 |

### 5.2 持久化流程

```
RegisterTool(tool, true)
       │
       ▼
┌──────────────────┐
│ 1. 注册到内存     │
│   RegistryManager │
│   ::RegisterTool()│
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ 2. 读取现有文件   │
│   Read JSON File │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ 3. 合并工具信息   │
│   Merge Tool Info│
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ 4. 写回文件       │
│   Write JSON File│
└──────────────────┘
```

### 5.3 文件格式

`/etc/cli/cli_tools.json`:

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tools-registry/v1",
  "version": "1.0.0",
  "lastUpdated": "2025-03-26T10:30:00Z",
  "tools": [
    {
      "name": "network-scanner",
      "version": "1.0.0",
      "description": "Network port scanner",
      "executable": "/etc/cli/tools/network-scanner",
      "permissions": ["ohos.permission.INTERNET"],
      "timeout": 30000,
      "input": { ... },
      "argumentMapping": { ... }
    }
  ]
}
```

## 6. 权限控制

### 6.1 权限要求

调用动态注册接口需要满足以下条件之一：

1. **系统权限**: 持有 `ohos.permission.MANAGE_CLI_TOOLS`
2. **Root 用户**: UID = 0
3. **System 用户**: UID = 1000

### 6.2 权限检查流程

```
IPC Request Arrives
       │
       ▼
┌──────────────────┐
│ Get Calling UID  │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ Check UID        │
│ - root (0)?      │
│ - system (1000)? │
└────────┬─────────┘
         │
    Yes  │  No
    ┌────┴────┐
    ▼         ▼
┌─────┐  ┌────────────┐
│Allow│  │Check Token  │
└─────┘  └────────────┘
              │
         Has Token?
         ┌────┴────┐
         ▼         ▼
      ┌─────┐  ┌────────┐
      │Allow│  │Deny    │
      └─────┘  └────────┘
```

### 6.3 权限验证实现

```cpp
bool CLIManagerService::CheckRegisterPermission()
{
    uid_t callingUid = IPCSkeleton::GetCallingUid();

    // 允许 root 和 system 用户
    if (callingUid == 0 || callingUid == 1000) {
        return true;
    }

    // 检查系统权限
    // TODO: 实现权限 token 检查

    return false;
}
```

## 7. 错误处理

### 7.1 错误码定义

| 错误码 | 值 | 说明 |
|--------|---|------|
| `ERR_OK` | 0 | 成功 |
| `ERR_NOT_FOUND` | 1 | 工具不存在 |
| `ERR_PERMISSION_DENIED` | 2 | 权限不足 |
| `ERR_INVALID_ARGS` | 3 | 参数无效 |
| `ERR_SERVICE_NOT_READY` | 6 | 服务未就绪 |
| `ERR_INVALID_SCHEMA` | 8 | Schema 无效 |
| `ERR_ALREADY_EXISTS` | 9 | 工具已存在 |

### 7.2 错误处理策略

| 错误 | 处理策略 |
|------|----------|
| 服务未就绪 | 返回错误，调用者重试 |
| 参数无效 | 立即返回，不重试 |
| 权限不足 | 记录日志，拒绝操作 |
| 工具已存在 | 返回错误，调用者决定是否更新 |
| 持久化失败 | 内存注册成功，返回警告 |

## 8. 性能考虑

### 8.1 性能指标

| 操作 | 预期耗时 |
|------|---------|
| 初始化（首次连接） | 10-50ms |
| 单个工具注册 | 5-20ms |
| 单个工具注册（持久化） | 15-120ms |
| 批量注册（100个工具） | 100-500ms |
| 工具查询 | 1-5ms |
| 工具注销 | 5-20ms |

### 8.2 优化策略

1. **批量操作**: 使用 `RegisterTools` 而非循环调用 `RegisterTool`
2. **延迟持久化**: 对于高频更新场景，使用 `persist=false`，定期批量持久化
3. **缓存机制**: 客户端缓存工具信息，减少查询调用
4. **异步持久化**: 文件写操作异步执行，不阻塞返回

## 9. 线程安全

### 9.1 线程安全保证

- 所有公共接口都是线程安全的
- 使用互斥锁保护共享资源
- 使用原子变量处理状态标志

### 9.2 锁策略

```cpp
class CLIManagerService {
private:
    std::mutex registryMutex_;      // 保护注册表操作
    std::atomic<bool> running_;     // 原子状态标志
};
```

## 10. 安全考虑

### 10.1 安全威胁

| 威胁 | 防护措施 |
|------|----------|
| 未授权注册 | 权限检查（UID、Token） |
| 路径遍历攻击 | 路径白名单验证 |
| 注入攻击 | 输入验证和 Schema 校验 |
| 拒绝服务 | 请求频率限制、资源配额 |

### 10.2 安全验证

```cpp
// 1. 路径验证
if (tool.executablePath.find("/etc/cli/tools/") != 0) {
    return ErrCode::ERR_INVALID_ARGS;
}

// 2. 文件验证
if (stat(tool.executablePath.c_str(), &st) != 0) {
    return ErrCode::ERR_INVALID_ARGS;
}

// 3. 权限验证
if ((st.st_mode & S_ISUID) || (st.st_mode & S_ISGID)) {
    return ErrCode::ERR_INVALID_ARGS;
}
```

## 11. 使用示例

### 11.1 服务启动注册

```cpp
void MyService::OnStart() {
    auto registry = CliToolRegistry::GetInstance();
    registry->Initialize();

    ToolInfo tool;
    tool.name = "my-tool";
    tool.version = "1.0.0";
    tool.executablePath = "/etc/cli/tools/my-tool";

    registry->RegisterTool(tool, true);
}

void MyService::OnStop() {
    auto registry = CliToolRegistry::GetInstance();
    registry->UnregisterTool("my-tool", false);  // 不删除文件
    registry->Release();
}
```

### 11.2 批量注册

```cpp
std::vector<ToolInfo> tools = {tool1, tool2, tool3};
auto registry = CliToolRegistry::GetInstance();
registry->Initialize();
registry->RegisterTools(tools, true);
```

### 11.3 版本更新

```cpp
ToolInfo tool;
registry->QueryTool("my-tool", tool);
if (tool.version != "2.0.0") {
    tool.version = "2.0.0";
    registry->UpdateTool(tool, true);
}
```

## 12. 测试建议

### 12.1 单元测试

- ✅ 工具注册/注销
- ✅ 工具查询
- ✅ 批量注册
- ✅ 权限检查
- ✅ 参数验证

### 12.2 集成测试

- ✅ IPC 通信
- ✅ 持久化
- ✅ 并发访问
- ✅ 错误恢复

### 12.3 安全测试

- ✅ 权限绕过
- ✅ 路径遍历
- ✅ 注入攻击
- ✅ 拒绝服务

## 13. 未来扩展

### 13.1 计划功能

1. **变更监听**: 支持注册表变更事件回调
2. **工具依赖**: 支持工具依赖关系管理
3. **版本管理**: 支持多版本工具共存
4. **动态卸载**: 自动清理失效工具

### 13.2 性能优化

1. **缓存优化**: 客户端缓存工具信息
2. **批量持久化**: 延迟批量写入文件
3. **增量更新**: 仅更新变化的工具

## 14. 总结

动态工具注册功能为 OpenHarmony CLI 工具管理框架提供了强大的运行时扩展能力。通过 InnerKits API，系统服务可以方便地注册和管理自己的 CLI 工具，无需修改配置文件或重启系统。

### 14.1 主要优势

- ✅ **运行时扩展**: 无需重启即可添加工具
- ✅ **权限控制**: 确保只有授权组件可以注册
- ✅ **灵活持久化**: 支持内存和文件两种模式
- ✅ **原子操作**: 批量注册支持事务性
- ✅ **IPC 隔离**: 跨进程通信，服务隔离

### 14.2 适用场景

- 系统服务启动时注册工具
- 第三方服务动态注册
- 工具版本升级
- 临时工具管理

---

**文档结束**
