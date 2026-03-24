# 阶段 6：JS API 开发 - 详细总结

## 概述

阶段6实现了 CLI Manager 的 JavaScript/ArkTS API 接口，使系统应用（如 AIClaw）能够通过 JS 调用 CLI 工具。

## 完成的工作

### 1. NAPI 绑定层 (frameworks/js/napi/cli_manager/)

#### js_cli_manager.h
- 定义 JSCliManager 类
- 声明所有静态方法接口
- 定义 AsyncContext 异步上下文结构
- 声明辅助方法（ReadExecOptions、CreateToolInfoObject 等）

#### js_cli_manager.cpp (约 600 行)

**核心实现：**

1. **模块初始化**
```cpp
napi_value JSCliManager::Init(napi_env env, napi_value exports)
```
- 定义类属性
- 导出 CLIManager 类

2. **查询工具接口**
```cpp
napi_value QueryTools(napi_env env, napi_callback_info info)
napi_value QueryToolByName(napi_env env, napi_callback_info info)
napi_value QueryToolsByPermission(napi_env env, napi_callback_info info)
```
- 异步工作队列
- 回调函数处理
- 结果对象构造

3. **执行工具接口**
```cpp
napi_value ExecTool(napi_env env, napi_callback_info info)
napi_value ExecToolAsync(napi_env env, napi_callback_info info)
```
- 参数解析和验证
- 异步执行管理
- Promise 支持

4. **任务管理接口**
```cpp
napi_value CancelTask(napi_env env, napi_callback_info info)
napi_value QueryTaskStatus(napi_env env, napi_callback_info info)
```

5. **辅助方法**
- `ReadExecOptions` - 读取执行选项
- `ReadArguments` - JS 对象转 JSON
- `CreateToolInfoObject` - 创建 JS 工具信息对象
- `CreateExecResultObject` - 创建 JS 执行结果对象
- `CreateTaskStatusObject` - 创建 JS 任务状态对象
- `CreateError` - 创建 JS 错误对象

#### js_cli_manager_module.cpp
- NAPI 模块入口点
- 导出符号定义

### 2. TypeScript 类型定义 (interfaces/kits/js/cli/)

#### @ohos.cli.manager.d.ts

**定义的类型：**

1. **ExecOptions** - 执行选项
```typescript
interface ExecOptions {
  mode?: 'sync' | 'async';
  timeout?: number;
  env?: Record<string, string>;
  cwd?: string;
}
```

2. **ToolInfo** - 工具信息
```typescript
interface ToolInfo {
  name: string;
  version: string;
  description: string;
  type: 'short-lived' | 'long-lived';
  permissions: string[];
  timeout: number;
  supportedEvents: string[];
}
```

3. **ExecResult** - 执行结果
```typescript
interface ExecResult {
  taskId: string;
  exitCode: number;
  stdout: string;
  stderr: string;
  signal: number;
  timedOut: boolean;
  executionTime: number;
}
```

4. **ToolEvent** - 工具事件
```typescript
interface ToolEvent {
  type: 'stdout' | 'stderr' | 'exit' | 'error';
  data: string;
  exitCode?: number;
  timestamp: number;
}
```

5. **TaskStatus** - 任务状态
```typescript
interface TaskStatus {
  taskId: string;
  toolName: string;
  status: 'running' | 'completed' | 'failed' | 'timeout' | 'cancelled';
  startTime: number;
  endTime?: number;
  exitCode?: number;
}
```

6. **CLIManager** - 主接口类
```typescript
class CLIManager {
  static queryTools(callback: (error: Error | null, tools?: ToolInfo[]) => void): void;
  static queryToolByName(name: string, callback: (error: Error | null, tool?: ToolInfo) => void): void;
  static queryToolsByPermission(permission: string, callback: (error: Error | null, tools?: ToolInfo[]) => void): void;
  static execTool(name: string, args: Record<string, any>, options: { options?: ExecOptions; callback: (error: Error | null, result?: ExecResult) => void }): void;
  static execToolAsync(name: string, args: Record<string, any>, eventCallback: (event: ToolEvent) => void, options?: ExecOptions): Promise<string>;
  static cancelTask(taskId: string): boolean;
  static queryTaskStatus(taskId: string): TaskStatus;
}
```

### 3. 构建配置 (BUILD.gn)

```gn
ohos_native_shared_library("cli_manager_napi") {
  sources = [
    "js_cli_manager.cpp",
    "js_cli_manager_module.cpp",
  ]

  deps = [
    "../interfaces/kits/native/cli:cli_manager_client",
    "../interfaces/inner_api/cli_manager:cli_manager_proxy",
  ]

  external_deps = [
    "napi:ace_napi.z",
    "hilog:hilog",
    "jsoncpp:json_shared",
  ]
}
```

## 技术要点

### 1. NAPI 异步调用模式

**Callback 模式：**
```cpp
napi_create_async_work(env, nullptr, resourceName,
    ExecAsyncWork,      // 执行函数
    ExecAsyncComplete,  // 完成回调
    asyncContext,
    &asyncWork
);
```

**Promise 模式：**
```cpp
napi_value promise;
napi_deferred deferred;
napi_create_promise(env, &deferred, &promise);
// ... 异步工作 ...
napi_resolve_deferred(env, deferred, result);
```

### 2. JS 对象与 C++ 类型转换

**JS → C++：**
- String: `napi_get_value_string_utf8`
- Int32: `napi_get_value_int32`
- Uint32: `napi_get_value_uint32`
- Object: 逐个属性读取

**C++ → JS：**
- String: `napi_create_string_utf8`
- Int32: `napi_create_int32`
- Object: `napi_create_object` + `napi_set_named_property`
- Array: `napi_create_array` + `napi_set_element`

### 3. 错误处理

```cpp
napi_value CreateError(napi_env env, int32_t errorCode, const std::string& errorMessage) {
    napi_value code;
    napi_create_int32(env, errorCode, &code);

    napi_value message;
    napi_create_string_utf8(env, errorMessage.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_value error;
    napi_create_error(env, code, message, &error);

    return error;
}
```

## 使用示例

### 同步调用

```typescript
import CLI from '@ohos.cli.manager';

CLI.CLIManager.execTool('network-scanner',
  { target: '192.168.1.1', ports: [80, 443] },
  {
    options: { timeout: 30000 },
    callback: (error, result) => {
      if (error) {
        console.error('Error:', error.message);
      } else {
        console.log('Exit code:', result.exitCode);
        console.log('Output:', result.stdout);
      }
    }
  }
);
```

### 异步调用

```typescript
const taskId = await CLI.CLIManager.execToolAsync(
  'log-monitor',
  { level: 'verbose', path: '/var/log/syslog' },
  (event) => {
    if (event.type === 'stdout') {
      console.log('[STDOUT]', event.data);
    } else if (event.type === 'stderr') {
      console.error('[STDERR]', event.data);
    } else if (event.type === 'exit') {
      console.log('[EXIT] Code:', event.exitCode);
    }
  },
  { timeout: 60000 }
);

console.log('Task started:', taskId);
```

### 查询工具

```typescript
CLI.CLIManager.queryTools((error, tools) => {
  if (!error) {
    tools.forEach(tool => {
      console.log(`${tool.name} (${tool.type})`);
      console.log(`  ${tool.description}`);
      console.log(`  Permissions: ${tool.permissions.join(', ')}`);
    });
  }
});
```

## 架构图

```
┌─────────────────────────────────────────────────┐
│          AIClaw App (JS/ArkTS)                  │
│  import CLI from '@ohos.cli.manager'            │
└─────────────────────┬───────────────────────────┘
                      │ NAPI
                      ↓
┌─────────────────────────────────────────────────┐
│         JSCliManager (NAPI Binding)             │
│  ├─ QueryTools()                                │
│  ├─ ExecTool()                                  │
│  ├─ ExecToolAsync()                             │
│  └─ CancelTask()                                │
└─────────────────────┬───────────────────────────┘
                      │ C++ Call
                      ↓
┌─────────────────────────────────────────────────┐
│        CLIManagerClient (Native SDK)            │
└─────────────────────┬───────────────────────────┘
                      │ IPC
                      ↓
┌─────────────────────────────────────────────────┐
│          CLI Manager Service                    │
└─────────────────────────────────────────────────┘
```

## 性能考虑

1. **异步执行** - 所有 I/O 操作都是异步的，不阻塞 JS 线程
2. **对象复用** - AsyncContext 复用减少内存分配
3. **错误快速返回** - 参数验证在 C++ 层完成，避免不必要的 IPC 调用

## 文件清单

| 文件 | 行数 | 描述 |
|------|------|------|
| js_cli_manager.h | ~150 | NAPI 类定义 |
| js_cli_manager.cpp | ~600 | NAPI 实现 |
| js_cli_manager_module.cpp | ~40 | 模块入口 |
| BUILD.gn | ~30 | 构建配置 |
| @ohos.cli.manager.d.ts | ~150 | TypeScript 定义 |
| **总计** | **~970** | |

## 验证方法

### 单元测试
```bash
# 运行 NAPI 单元测试
./build.sh --product-name <product> --build-target cli_manager_napi_test
```

### 集成测试
创建测试 JS 应用：
```typescript
import CLI from '@ohos.cli.manager';

async function test() {
  // 测试查询
  const tools = await new Promise((resolve) => {
    CLI.CLIManager.queryTools((err, tools) => resolve(tools));
  });

  console.log('Found tools:', tools.length);

  // 测试执行
  CLI.CLIManager.execTool('echo', { msg: 'Hello' }, {
    options: { timeout: 5000 },
    callback: (err, result) => {
      console.log('Result:', result.stdout);
    }
  });
}
```

---

**阶段 6 完成标志：**
- ✅ NAPI 绑定完整实现
- ✅ TypeScript 类型定义完整
- ✅ 支持同步和异步调用
- ✅ 支持所有核心功能
- ✅ 提供完整使用示例
