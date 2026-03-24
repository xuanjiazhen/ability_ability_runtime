# CLI Manager 工具管理框架

## 概述

CLI Manager 是 OpenHarmony ability_runtime 组件的一部分，提供统一的 CLI 工具管理框架，允许系统应用（如 AIClaw）通过 JS API 调用和管理系统中的 CLI 工具。

### 主要特性

- **统一工具管理**：自动扫描和注册系统中的 CLI 工具
- **同步/异步调用**：支持短时和长时运行工具
- **进程隔离**：每个工具运行在独立进程中
- **安全加固**：权限校验、参数过滤、资源限制
- **JS API**：提供易用的 JavaScript 接口

## 架构

```
┌─────────────────────────────────────────────────────────────┐
│                         AIClaw App                         │
│                    (系统应用 - JS/ArkTS)                    │
└──────────────────────────┬──────────────────────────────────┘
                           │ JS API
                           ↓
┌─────────────────────────────────────────────────────────────┐
│                      NAPI Binding Layer                     │
│                  (frameworks/js/napi/cli_manager)            │
└──────────────────────────┬──────────────────────────────────┘
                           │ IPC
                           ↓
┌─────────────────────────────────────────────────────────────┐
│                   CLI Manager Client                        │
│                  (interfaces/kits/native/cli)                │
└──────────────────────────┬──────────────────────────────────┘
                           │ IPC
                           ↓
┌─────────────────────────────────────────────────────────────┐
│                  CLI Manager Service                        │
│                   (services/climgr)                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │   Registry   │  │   Process    │  │    Event     │     │
│  │   Manager    │  │   Manager    │  │  Dispatcher  │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│  │  Permission  │  │    Timeout   │  │   Resource   │     │
│  │   Checker    │  │   Manager    │  │   Manager    │     │
│  └──────────────┘  └──────────────┘  └──────────────┘     │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ↓
┌─────────────────────────────────────────────────────────────┐
│                    CLI Tools (进程)                         │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │ tool-1   │  │ tool-2   │  │ tool-3   │  │  ...     │  │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## 目录结构

```
ability_runtime/
├── services/climgr/                          # CLI Manager Service
│   ├── src/
│   │   ├── cli_manager_service.cpp/h         # 主服务实现
│   │   ├── registry_manager.cpp/h            # 工具注册管理
│   │   ├── process_manager.cpp/h             # 进程管理
│   │   ├── process_monitor.cpp/h             # 进程监控
│   │   ├── timeout_manager.cpp/h             # 超时管理
│   │   ├── resource_manager.cpp/h            # 资源管理
│   │   ├── event_dispatcher.cpp/h            # 事件分发
│   │   ├── permission_checker.cpp/h          # 权限校验
│   │   ├── command_sanitizer.cpp/h           # 命令清理
│   │   └── io_monitor.cpp/h                  # I/O 监控
│   ├── sa_profile/9001.xml                   # SystemAbility 配置
│   └── BUILD.gn
├── interfaces/
│   ├── inner_api/cli_manager/                # IPC 接口
│   │   ├── cli_manager_service_interface.h
│   │   ├── cli_manager_proxy.cpp/h
│   │   ├── cli_callback.h
│   │   ├── cli_callback_stub.cpp/h
│   │   ├── cli_callback_proxy.cpp/h
│   │   ├── cli_tool_info.h
│   │   ├── cli_execution_context.h/cpp
│   │   ├── cli_manager_constants.h
│   │   └── cli_manager_error.h
│   └── kits/native/cli/                      # Native SDK
│       ├── cli_manager_client.h/cpp
│       └── (导出接口)
├── frameworks/js/napi/cli_manager/            # NAPI 绑定
│   ├── js_cli_manager.cpp/h
│   ├── js_cli_manager_module.cpp
│   └── BUILD.gn
├── interfaces/kits/js/cli/                    # JS 类型定义
│   └── @ohos.cli.manager.d.ts
└── test/unittest/climgr/                      # 单元测试
    ├── cli_manager_service_test.cpp
    ├── process_manager_test.cpp
    └── BUILD.gn
```

## 使用示例

### JS API 使用

```typescript
import CLI from '@ohos.cli.manager';

// 1. 查询所有可用工具
CLI.CLIManager.queryTools((error, tools) => {
  if (!error) {
    tools.forEach(tool => {
      console.log(`${tool.name}: ${tool.description}`);
    });
  }
});

// 2. 同步执行工具
CLI.CLIManager.execTool('network-scanner',
  { target: '192.168.1.1', ports: [80, 443] },
  {
    options: { timeout: 30000 },
    callback: (error, result) => {
      if (result && result.exitCode === 0) {
        console.log('Scan result:', result.stdout);
      }
    }
  }
);

// 3. 异步执行工具
const taskId = await CLI.CLIManager.execToolAsync(
  'log-monitor',
  { level: 'verbose' },
  (event) => {
    if (event.type === 'stdout') {
      console.log('Output:', event.data);
    } else if (event.type === 'exit') {
      console.log('Exit code:', event.exitCode);
    }
  },
  { timeout: 60000 }
);

// 4. 查询任务状态
const status = CLI.CLIManager.queryTaskStatus(taskId);
console.log('Task status:', status.status);

// 5. 取消任务
CLI.CLIManager.cancelTask(taskId);
```

### C++ API 使用

```cpp
#include "cli_manager_client.h"

using namespace OHOS::CliMgr;

// 查询工具
CLIManagerClient client;
std::vector<ToolInfo> tools;
int32_t ret = client.QueryTools(tools);

// 同步执行
Json::Value args;
args["target"] = "192.168.1.1";
ExecutionResult result;
ExecOptions options;
options.timeoutMs = 30000;

ret = client.ExecTool("network-scanner", args, result, options);

// 异步执行
std::string taskId;
auto callback = [](const std::string& taskId, const ToolEvent& event) {
    std::cout << "Event: " << event.type << ", Data: " << event.data << std::endl;
};

ret = client.ExecToolAsync("log-monitor", args, callback, taskId, options);
```

## 工具描述文件格式

CLI 工具需要提供 JSON 格式的描述文件：

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tool/v1",
  "name": "network-scanner",
  "version": "1.0.0",
  "description": "Network scanning utility tool",
  "executable": "/usr/bin/net-scan",
  "type": "short-lived",
  "permissions": [
    "ohos.permission.INTERNET",
    "ohos.permission.GET_NETWORK_INFO"
  ],
  "input": {
    "type": "object",
    "properties": {
      "target": {
        "type": "string",
        "format": "ipv4",
        "description": "Target IP address"
      },
      "ports": {
        "type": "array",
        "items": {"type": "integer"},
        "description": "Ports to scan"
      }
    },
    "required": ["target"]
  },
  "output": {
    "type": "object",
    "properties": {
      "results": {
        "type": "array",
        "description": "Scan results"
      }
    }
  },
  "timeout": 30000,
  "events": ["progress", "result"]
}
```

描述文件位置：`/etc/cli/tools/<tool_name>.json`

## 构建和测试

### 构建服务

```bash
# 构建服务端
./build.sh --product-name <product> --build-target climgr_service

# 构建客户端
./build.sh --product-name <product> --build-target cli_manager_client

# 构建 JS API
./build.sh --product-name <product> --build-target cli_manager_napi
```

### 运行测试

```bash
# 运行所有单元测试
./build.sh --product-name <product> --build-target ability_runtime_test

# 运行特定测试
run -t UT -ts cli_manager_service_test
run -t UT -ts process_manager_test
```

## 安全特性

### 1. 权限校验
- 系统应用签名验证
- 细粒度权限检查
- 审计日志记录

### 2. 参数过滤
- JSON Schema 验证
- 类型检查
- 范围验证
- 命令注入防护

### 3. 资源限制
- 内存限制：512MB
- CPU 限制：80%
- 文件描述符：1024
- 全局进程数：100
- 单应用进程数：10

### 4. 进程隔离
- 每个工具独立进程
- 沙箱环境
- 超时强制终止

## 错误码

| 错误码 | 常量 | 描述 |
|--------|------|------|
| 0 | ERR_OK | 成功 |
| 1 | ERR_NOT_FOUND | 工具不存在 |
| 2 | ERR_PERMISSION_DENIED | 权限不足 |
| 3 | ERR_INVALID_ARGS | 参数无效 |
| 4 | ERR_TIMEOUT | 执行超时 |
| 5 | ERR_PROCESS_CRASH | 进程崩溃 |
| 6 | ERR_SERVICE_NOT_READY | 服务未就绪 |
| 7 | ERR_TOO_MANY_TASKS | 任务数过多 |
| 8 | ERR_INVALID_SCHEMA | 描述文件格式错误 |

## 性能指标

- 同步调用响应：< 100ms（不含工具执行）
- 异步消息延迟：< 50ms
- 工具查询响应：< 50ms
- 支持工具数量：> 1000
- 并发任务数：100

## 依赖项

- OpenHarmony SDK
- SystemAbility Manager (samgr)
- IPC 框架
- HiLog
- JsonCpp

## 相关文档

- [设计文档](./DESIGN.md)
- [需求文档](./CLI.md)
- [使用示例](./cli_manager_examples.md)

## 版本信息

- **版本**: 1.0.0
- **最后更新**: 2025-03-24
- **状态**: 已完成（Phase 1-8）
