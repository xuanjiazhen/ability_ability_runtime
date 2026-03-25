# CLI Manager 实现说明

## 概述

本实现为 OpenHarmony ability_runtime 组件添加了 CLI Manager 功能，提供命令行工具管理和执行能力。

## 实现的功能

根据设计文档要求，本实现包含以下两个核心接口：

1. **queryTools** - 查询所有可用的工具
2. **execTool** - 执行指定的工具

## 架构设计

本实现遵循 OpenHarmony 的标准三层架构模式：

### 1. IPC 接口层 (interfaces/inner_api/cli_manager)

包含以下核心组件（参考 quick_fix_manager_client 的简化设计）：

- **cli_manager_client.h** - 合并了 IPC 接口定义和客户端实现
  - ICliManager 接口定义
  - CliManagerClient 客户端单例
- **cli_manager_data.h** - 数据结构定义 (ToolInfo, ExecOptions, ExecResult)
- **cli_manager_proxy.h/cpp** - 客户端代理实现
- **cli_manager_stub.h/cpp** - 服务端存根实现

### 2. JS 绑定层 (frameworks/js/napi/cli_manager)

包含以下组件：

- **js_cli_manager.h/cpp** - JS 接口绑定
- **js_cli_manager_utils.h/cpp** - JS 数据转换工具
- **cli_manager_module.cpp** - 模块注册

### 3. 服务层 (services/climgr)

包含以下组件：

- **cli_manager_service.h/cpp** - CLI Manager 系统服务实现
  - SystemAbility 继承实现
  - 工具配置管理
  - 命令执行逻辑

## 使用方式

### TypeScript 接口

```typescript
import CLIManager from '@ohos.cli.manager';

// 查询可用工具
CLIManager.queryTools((error, tools) => {
  if (error) {
    console.error('Query tools failed:', error);
    return;
  }
  console.log('Available tools:', tools);
});

// 示例1: 执行 ls 命令
CLIManager.execTool('ls',
  { '-l': '', '/tmp': '' },  // args - 键值对形式，空字符串表示无值的flag
  {                              // options
    options: {
      mode: 'sync',
      timeout: 5000
    },
    callback: (error, result) => {
      if (error) {
        console.error('Exec tool failed:', error);
        return;
      }
      console.log('Tool output:', result.stdout);
    }
  }
);

// 示例2: 执行 aa start 命令（实际场景）
CLIManager.execTool('hdc shell aa start',
  {
    '-a': 'EntryAbility2',
    '-b': 'com.example.bundleName'
  },
  {
    options: {
      mode: 'sync',
      timeout: 10000
    },
    callback: (error, result) => {
      if (error) {
        console.error('Start ability failed:', error);
        return;
      }
      console.log('Exit code:', result.exitCode);
      console.log('Stdout:', result.stdout);
      console.log('Stderr:', result.stderr);
    }
  }
);

// 示例3: 执行 grep 命令
CLIManager.execTool('grep',
  {
    'pattern': 'error',
    '/var/log/syslog': ''
  },
  {
    options: {
      mode: 'sync',
      timeout: 5000
    },
    callback: (error, result) => {
      if (error) {
        console.error('Grep failed:', error);
        return;
      }
      console.log('Grep result:', result.stdout);
    }
  }
);
```

## 构建说明

### 添加到构建系统

需要在以下文件中添加构建目标：

1. **ability_runtime.gni** - 添加源文件列表
2. **bundle.json** - 添加组件依赖

### 构建命令

```bash
# 构建整个组件
./build.sh --product-name <product> --build-target ability_runtime

# 构建 CLI Manager 特定目标
./build.sh --product-name <product> --build-target climgr
./build.sh --product-name <product> --build-target cli_manager
./build.sh --product-name <product> --build-target climanager_napi
```

## 系统能力配置

CLI Manager Service 需要注册为系统能力，SA ID 为 9999（待分配正式 ID）。

需要在系统配置中添加服务注册信息。

## 安全考虑

当前实现使用 popen 执行命令，存在以下安全风险：
- 命令注入风险
- 权限提升风险

在生产环境中，应该：
1. 实现白名单机制
2. 添加参数验证和转义
3. 实现细粒度权限控制
4. 添加资源限制和超时控制

## 后续工作

1. 实现工具配置文件加载机制
2. 添加异步执行支持
3. 实现任务取消和状态查询接口
4. 添加更完善的错误处理
5. 添加单元测试

## 文件清单

### interfaces/inner_api/cli_manager
- include/cli_manager_data.h - 数据结构定义
- include/cli_manager_client.h - IPC 接口和客户端定义（合并了 ICliManager 和 CliManagerClient）
- include/cli_manager_proxy.h - IPC 代理
- include/cli_manager_stub.h - IPC 存根
- src/cli_manager_data.cpp
- src/cli_manager_proxy.cpp
- src/cli_manager_stub.cpp
- src/cli_manager_client.cpp
- BUILD.gn

### frameworks/js/napi/cli_manager
- js_cli_manager.h
- js_cli_manager.cpp
- js_cli_manager_utils.h
- js_cli_manager_utils.cpp
- cli_manager_module.cpp
- BUILD.gn

### services/climgr
- include/cli_manager_service.h
- src/cli_manager_service.cpp
- BUILD.gn

## 参考文档

- OpenHarmony IPC 通信指南
- N-API 开发指南
- SystemAbility 开发指南
