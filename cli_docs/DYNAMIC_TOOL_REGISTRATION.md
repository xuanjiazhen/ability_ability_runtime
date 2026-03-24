# 动态工具注册 InnerKits 使用指南

## 概述

CLI 工具动态注册功能允许系统服务和底层组件在运行时动态注册和管理 CLI 工具，无需重启系统。

## 架构设计

```
┌─────────────────────────────────────────────────────────────┐
│                    底层服务/系统服务                          │
│  (NetworkService, StorageService, SecurityService, ...)      │
└────────────────────┬────────────────────────────────────────┘
                     │ 调用 InnerKits
                     ▼
        ┌────────────────────────┐
        │  CliToolRegistry       │
        │  (InnerKits 客户端)     │
        └────────┬───────────────┘
                 │ IPC (Binder/HDF)
                 ▼
        ┌────────────────────────┐
        │ CLIManagerService      │
        │ (系统服务)              │
        └────────┬───────────────┘
                 │
                 ▼
        ┌────────────────────────┐
        │  RegistryManager       │
        │  (注册表管理器)          │
        └────────────────────────┘
```

## 文件结构

```
interfaces/inner_api/cli_tool_registry/
├── cli_tool_registry.h          # InnerKits 头文件
└── cli_tool_registry.cpp        # InnerKits 实现

services/climgr/src/
├── cli_manager_service.h        # 服务头文件（新增动态注册接口）
├── cli_manager_service.cpp      # 服务实现（新增业务逻辑）
├── cli_manager_service_rpc.cpp  # RPC 处理（新增注册相关 RPC）
└── registry_manager.{h,cpp}    # 注册表管理器
```

## API 接口

### 1. 获取实例

```cpp
#include "cli_tool_registry.h"

using namespace OHOS::CliMgr;

CliToolRegistry* registry = CliToolRegistry::GetInstance();
```

### 2. 初始化

```cpp
// 连接到 CLI Manager Service
if (!registry->Initialize()) {
    HILOG_ERROR("Failed to initialize CliToolRegistry");
    return;
}
```

### 3. 注册单个工具

```cpp
ToolInfo tool;
tool.name = "network-scanner";
tool.version = "1.0.0";
tool.description = "Network port scanner tool";
tool.executablePath = "/etc/cli/tools/net-scanner";
tool.permissions.push_back("ohos.permission.INTERNET");
tool.permissions.push_back("ohos.permission.GET_NETWORK_INFO");

// 设置输入参数 Schema
tool.inputSchema["type"] = "object";
tool.inputSchema["properties"]["target"]["type"] = "string";
tool.inputSchema["properties"]["target"]["description"] = "Target IP address";

// 设置参数映射
tool.argMapping["mode"] = "flag";
tool.argMapping["mapping"]["target"] = "--target";

// 设置超时时间（30秒）
tool.timeoutMs = 30000;

// 注册工具（持久化到文件）
int32_t ret = registry->RegisterTool(tool, true);
if (ret == ErrCode::OK) {
    HILOG_INFO("Tool registered successfully");
} else if (ret == ErrCode::ERR_ALREADY_EXISTS) {
    HILOG_WARN("Tool already exists");
} else {
    HILOG_ERROR("Failed to register tool: %{public}d", ret);
}
```

### 4. 批量注册工具

```cpp
std::vector<ToolInfo> tools;

// 工具 1
ToolInfo scanner;
scanner.name = "network-scanner";
scanner.version = "1.0.0";
scanner.executablePath = "/etc/cli/tools/net-scanner";
tools.push_back(scanner);

// 工具 2
ToolInfo monitor;
monitor.name = "log-monitor";
monitor.version = "1.0.0";
monitor.executablePath = "/etc/cli/tools/log-mon";
tools.push_back(monitor);

// 批量注册（持久化）
int32_t ret = registry->RegisterTools(tools, true);
if (ret == ErrCode::OK) {
    HILOG_INFO("All tools registered successfully");
}
```

### 5. 更新工具

```cpp
ToolInfo tool;
tool.name = "network-scanner";  // 必须指定已存在的工具名称
tool.version = "1.1.0";  // 更新版本
tool.description = "Updated network scanner";
tool.executablePath = "/etc/cli/tools/net-scanner";  // 路径不能修改
tool.permissions.push_back("ohos.permission.INTERNET");

int32_t ret = registry->UpdateTool(tool, true);
```

### 6. 注销工具

```cpp
int32_t ret = registry->UnregisterTool("network-scanner", true);
```

### 7. 查询工具

```cpp
// 查询单个工具
ToolInfo tool;
int32_t ret = registry->QueryTool("network-scanner", tool);
if (ret == ErrCode::OK) {
    HILOG_INFO("Tool found: %{public}s, version=%{public}s",
              tool.name.c_str(), tool.version.c_str());
}

// 查询所有工具
std::vector<ToolInfo> allTools;
ret = registry->QueryAllTools(allTools);
for (const auto& t : allTools) {
    HILOG_INFO("Tool: %{public}s", t.name.c_str());
}
```

### 8. 检查工具是否已注册

```cpp
bool exists;
int32_t ret = registry->IsToolRegistered("network-scanner", exists);
if (ret == ErrCode::OK) {
    if (exists) {
        HILOG_INFO("Tool is registered");
    } else {
        HILOG_INFO("Tool is not registered");
    }
}
```

### 9. 重新加载注册表

```cpp
int32_t ret = registry->ReloadRegistry();
if (ret == ErrCode::OK) {
    HILOG_INFO("Registry reloaded successfully");
}
```

### 10. 释放资源

```cpp
registry->Release();
```

## 使用场景示例

### 场景 1：系统服务启动时注册工具

```cpp
#include "cli_tool_registry.h"

class NetworkService : public SystemAbility {
public:
    void OnStart() override {
        // 1. 初始化 CLI 工具注册器
        auto registry = CliToolRegistry::GetInstance();
        if (!registry->Initialize()) {
            HILOG_ERROR("Failed to initialize CLI tool registry");
            return;
        }

        // 2. 注册网络扫描工具
        RegisterNetworkScannerTool();

        // 3. 注册网络监控工具
        RegisterNetworkMonitorTool();
    }

    void OnStop() override {
        // 注销工具
        auto registry = CliToolRegistry::GetInstance();
        registry->UnregisterTool("network-scanner", false);
        registry->UnregisterTool("network-monitor", false);

        registry->Release();
    }

private:
    void RegisterNetworkScannerTool() {
        ToolInfo tool;
        tool.name = "network-scanner";
        tool.version = "1.0.0";
        tool.description = "Network port scanner";
        tool.executablePath = "/etc/cli/tools/network-scanner";
        tool.permissions = {
            "ohos.permission.INTERNET",
            "ohos.permission.GET_NETWORK_INFO"
        };

        // 配置输入 Schema
        tool.inputSchema["type"] = "object";
        tool.inputSchema["properties"]["target"] = Json::objectValue;
        tool.inputSchema["properties"]["target"]["type"] = "string";
        tool.inputSchema["properties"]["target"]["description"] = "Target address";
        tool.inputSchema["properties"]["portRange"] = Json::objectValue;
        tool.inputSchema["properties"]["portRange"]["type"] = "string";
        tool.inputSchema["properties"]["portRange"]["description"] = "Port range (e.g., 1-1024)";
        tool.inputSchema["required"] = Json::arrayValue;
        tool.inputSchema["required"].append("target");

        // 配置参数映射
        tool.argMapping["mode"] = "flag";
        tool.argMapping["mapping"]["target"] = "--target";
        tool.argMapping["mapping"]["portRange"] = "--ports";

        // 设置超时
        tool.timeoutMs = 60000;  // 60 秒

        int32_t ret = CliToolRegistry::GetInstance()->RegisterTool(tool, true);
        if (ret != ErrCode::OK) {
            HILOG_ERROR("Failed to register network-scanner: %{public}d", ret);
        }
    }

    void RegisterNetworkMonitorTool() {
        ToolInfo tool;
        tool.name = "network-monitor";
        tool.version = "1.0.0";
        tool.description = "Real-time network traffic monitor";
        tool.executablePath = "/etc/cli/tools/network-monitor";
        tool.permissions = {"ohos.permission.INTERNET"};

        // 标记为长时运行工具
        tool.timeoutMs = 0;  // 无超时限制
        tool.eventTypes = {"stdout", "stderr", "exit"};

        int32_t ret = CliToolRegistry::GetInstance()->RegisterTool(tool, true);
        if (ret != ErrCode::OK) {
            HILOG_ERROR("Failed to register network-monitor: %{public}d", ret);
        }
    }
};
```

### 场景 2：服务启动时检查并更新工具

```cpp
void MyService::OnStart() {
    auto registry = CliToolRegistry::GetInstance();
    if (!registry->Initialize()) {
        return;
    }

    // 检查工具是否已存在
    bool exists;
    registry->IsToolRegistered("my-tool", exists);

    if (exists) {
        // 工具已存在，更新版本
        ToolInfo tool;
        registry->QueryTool("my-tool", tool);

        if (tool.version != "2.0.0") {
            // 更新到新版本
            tool.version = "2.0.0";
            tool.description = "Updated tool description";
            registry->UpdateTool(tool, true);
            HILOG_INFO("Tool updated to version 2.0.0");
        }
    } else {
        // 工具不存在，注册新工具
        ToolInfo tool;
        tool.name = "my-tool";
        tool.version = "2.0.0";
        tool.executablePath = "/etc/cli/tools/my-tool";
        registry->RegisterTool(tool, true);
        HILOG_INFO("New tool registered");
    }
}
```

### 场景 3：动态工具卸载

```cpp
void MyService::OnStop() {
    auto registry = CliToolRegistry::GetInstance();

    // 方式 1: 仅从内存注销（系统重启后恢复）
    registry->UnregisterTool("my-tool", false);

    // 方式 2: 从内存和文件都删除（永久删除）
    registry->UnregisterTool("my-tool", true);

    registry->Release();
}
```

## 持久化策略

### persist 参数说明

- `persist = true`:
  - 工具信息会保存到 `/etc/cli/cli_tools.json`
  - 系统重启后工具仍然可用
  - 适用于长期使用的工具

- `persist = false`:
  - 工具信息仅在内存中
  - 系统重启后工具消失
  - 适用于临时工具或测试环境

## 权限要求

### 所需权限

调用动态注册接口需要以下权限之一：

1. **系统权限**: `ohos.permission.MANAGE_CLI_TOOLS`
2. **Root 用户**: UID = 0
3. **System 用户**: UID = 1000

### 权限检查

服务端会自动检查调用者权限：

```cpp
bool CLIManagerService::CheckRegisterPermission()
{
    // 获取调用者 UID
    uid_t callingUid = IPCSkeleton::GetCallingUid();

    // 只允许 root 和 system 用户
    return (callingUid == 0 || callingUid == 1000);
}
```

## 错误处理

### 常见错误码

| 错误码 | 说明 | 处理建议 |
|--------|------|----------|
| `ERR_OK` | 成功 | - |
| `ERR_SERVICE_NOT_READY` | 服务未就绪 | 等待服务启动或检查服务状态 |
| `ERR_INVALID_ARGS` | 参数无效 | 检查工具信息是否完整 |
| `ERR_ALREADY_EXISTS` | 工具已存在 | 使用 UpdateTool 更新或先注销 |
| `ERR_NOT_FOUND` | 工具不存在 | 检查工具名称是否正确 |
| `ERR_PERMISSION_DENIED` | 权限不足 | 确认进程有足够权限 |

### 错误处理示例

```cpp
int32_t ret = registry->RegisterTool(tool, true);
switch (ret) {
    case ErrCode::OK:
        HILOG_INFO("Tool registered successfully");
        break;
    case ErrCode::ERR_ALREADY_EXISTS:
        HILOG_WARN("Tool already exists, updating...");
        registry->UpdateTool(tool, true);
        break;
    case ErrCode::ERR_PERMISSION_DENIED:
        HILOG_ERROR("Permission denied");
        // 处理权限不足的情况
        break;
    default:
        HILOG_ERROR("Failed to register tool: %{public}d", ret);
        break;
}
```

## 线程安全

- 所有接口都是线程安全的
- 可以在多线程环境中并发调用
- 内部使用互斥锁保护共享资源

## 性能考虑

1. **初始化**: 第一次连接服务时需要建立 IPC 通信，耗时约 10-50ms
2. **注册工具**: 单个工具注册耗时约 5-20ms（不含持久化）
3. **批量注册**: 批量注册比单个注册更高效
4. **持久化**: persist=true 时需要写入文件，耗时约 10-100ms

## 最佳实践

### 1. 服务启动时注册

```cpp
void MyService::OnStart() {
    // 在服务启动时注册工具
    RegisterTools();
}

void MyService::OnStop() {
    // 在服务停止时清理资源（不删除文件）
    auto registry = CliToolRegistry::GetInstance();
    registry->UnregisterTool("my-tool", false);  // persist=false
    registry->Release();
}
```

### 2. 批量注册优化

```cpp
// 推荐：批量注册
std::vector<ToolInfo> tools = PrepareAllTools();
registry->RegisterTools(tools, true);

// 避免：循环单个注册
for (const auto& tool : tools) {
    registry->RegisterTool(tool, true);  // 多次 IPC 调用
}
```

### 3. 版本管理

```cpp
void UpdateToolIfNeeded(const std::string& toolName, const std::string& targetVersion) {
    auto registry = CliToolRegistry::GetInstance();

    ToolInfo existingTool;
    int32_t ret = registry->QueryTool(toolName, existingTool);

    if (ret == ErrCode::OK) {
        if (existingTool.version != targetVersion) {
            // 版本不匹配，更新工具
            ToolInfo newTool = existingTool;
            newTool.version = targetVersion;
            registry->UpdateTool(newTool, true);
        }
    } else {
        // 工具不存在，注册新工具
        ToolInfo newTool = CreateToolInfo(toolName, targetVersion);
        registry->RegisterTool(newTool, true);
    }
}
```

### 4. 错误重试

```cpp
bool RegisterToolWithRetry(const ToolInfo& tool, int maxRetries = 3) {
    auto registry = CliToolRegistry::GetInstance();

    for (int i = 0; i < maxRetries; ++i) {
        if (!registry->Initialize()) {
            HILOG_WARN("Failed to initialize, retry %{public}d/%{public}d", i+1, maxRetries);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        int32_t ret = registry->RegisterTool(tool, true);
        if (ret == ErrCode::OK) {
            return true;
        }

        if (ret != ErrCode::ERR_SERVICE_NOT_READY) {
            // 非服务未就绪错误，不重试
            return false;
        }

        HILOG_WARN("Service not ready, retry %{public}d/%{public}d", i+1, maxRetries);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return false;
}
```

## 调试建议

### 1. 启用详细日志

```cpp
// 在服务端启用详细日志
// services/climgr/src/cli_manager_service.cpp
constexpr bool DEBUG_MODE = true;
```

### 2. 验证工具注册

```bash
# 查询已注册的工具
cli-tool query --all

# 查询特定工具
cli-tool query --name network-scanner
```

### 3. 检查注册表文件

```bash
# 查看注册表内容
cat /etc/cli/cli_tools.json | jq
```

## 限制和注意事项

1. **工具数量限制**: 单次批量注册最多 1000 个工具
2. **路径限制**: 可执行文件必须在 `/etc/cli/tools/` 目录下
3. **权限限制**: 只有系统服务和 root 用户可以注册工具
4. **原子性**: 批量注册时，如果任一工具失败，操作会回滚
5. **性能**: 持久化操作（persist=true）需要写文件，会影响性能

## 总结

动态工具注册 InnerKits 提供了一套完整的 API，供系统服务和底层组件管理 CLI 工具的生命周期。通过合理使用这些 API，可以实现：

- ✅ 服务启动时自动注册工具
- ✅ 工具版本动态更新
- ✅ 临时工具和永久工具的灵活管理
- ✅ 工具信息的查询和验证
- ✅ 注册表的热更新

遵循最佳实践，可以确保工具注册的高效、安全和可靠。
