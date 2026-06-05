# invokeFunction 方案

## 1. 概述

新增 `invokeFunction` 方法，支持通过 bundleName 和 functionName 调用应用注册的 function。当前仅支持意图（INTENT_FUNCTION）类型的 function 执行。

方法添加到 JSCliManager：
`cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp`

## 2. JS 接口定义

### 2.1 方法签名

```TypeScript
invokeFunction(
  bundleName: string,
  name: string,
  args: Record<string, Object>,
  options?: InvokeOptions
): Promise<InvokeResult>
```

### 2.2 InvokeOptions 定义

当前无用，预留扩展能力。

```TypeScript
interface InvokeOptions {
  context?: Context;
}
```

### 2.3 InvokeResult 定义

```TypeScript
interface InvokeResult {
  success: boolean;    // 由 executeResult.code 决定，code == 0 时为 true
  data?: any;          // executeResult.result (WantParams) 转换的 JS 对象
  errorCode?: number;  // executeResult.code，仅在非 0 时返回
}
```

## 3. 依赖的数据结构

### 3.1 FunctionInfo

定义路径：`cli_tool_framework/interfaces/cli_tool/include/function_info.h`

```cpp
enum class FunctionType : int32_t {
    INTENT_FUNCTION = 0,  // Intent function type
};

class FunctionInfo : public Parcelable {
public:
    std::string functionName;      // Function name (unique identifier)
    std::string bundleName;        // Bundle name that owns this function
    std::string description;       // Human-readable description
    std::string inputSchema;       // Input parameter JSON Schema
    std::string outputSchema;      // Output result JSON Schema (optional)
    FunctionType functionType;     // Function type
};
```

### 3.2 查询接口

定义路径：`cli_tool_framework/interfaces/cli_tool/include/cli_tool_mgr_client.h`

```cpp
ErrCode GetFunctionInfo(const std::string &bundleName, const std::string &functionName,
    FunctionInfo &function);
```

通过 `CliToolMGRClient::GetInstance()` 获取实例后调用，按 bundleName + functionName 查询 FunctionInfo。

## 4. 错误码定义

在 `cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h` 中新增：

```cpp
ERR_FUNCTION_NOT_EXIST = 35700013,              // Function not exist（已有）
ERR_FUNCTION_TYPE_NOT_SUPPORTED = 35700014,     // Function type not supported
ERR_FUNCTION_PARAM_VALIDATE_FAILED = 35700015,  // Function param validate failed
ERR_FUNCTION_EXECUTE_FAILED = 35700016,          // Function execute failed
ERR_FUNCTION_EXECUTE_TIMEOUT = 35700017,          // Function execute timeout
ERR_FUNCTION_QUERY_FAILED = 35700018,             // Function query failed
```

对应的业务错误码（`cli_manager_error_utils.h`）：

```cpp
ERROR_FUNCTION_NOT_EXIST = 35600060,
ERROR_FUNCTION_TYPE_NOT_SUPPORTED = 35600061,
ERROR_FUNCTION_PARAM_VALIDATE_FAILED = 35600062,
ERROR_FUNCTION_EXECUTE_FAILED = 35600063,
ERROR_FUNCTION_EXECUTE_TIMEOUT = 35600064,
ERROR_FUNCTION_QUERY_FAILED = 35600065,
```

### 错误码使用场景

| 错误码 | 触发场景 | 返回方式 |
|--------|----------|----------|
| `ERR_FUNCTION_QUERY_FAILED` | GetFunctionInfo 返回非 ERR_OK | Promise reject |
| `ERR_FUNCTION_TYPE_NOT_SUPPORTED` | FunctionInfo.functionType 不是 INTENT_FUNCTION | Promise reject |
| `ERR_FUNCTION_EXECUTE_FAILED` | IntentClient::ExecuteIntentByFunctionCall 返回非0，或意图回调 resultCode != 0 | Promise reject |
| `ERR_FUNCTION_EXECUTE_TIMEOUT` | ffrt 超时任务触发 | Promise reject |

## 5. 异步方案设计

### 5.1 技术选型

采用 **ffrt + napi_threadsafe_function** 方案：

| 组件 | 用途 |
|------|------|
| `ffrt::submit` | 在工作线程执行 GetFunctionInfo（同步 IPC）和 ExecuteIntentByFunctionCall |
| `ffrt::task_attr().delay()` | 超时等待（延迟调度，不占用线程资源） |
| `napi_threadsafe_function` | 从工作线程/binder 线程安全回调到 JS 线程执行 resolve/reject |
| `napi_create_promise` | 创建单层 Promise，直接 resolve/reject |

### 5.2 执行流程

```
JS 线程: OnInvokeFunction
  ├─ 1. 解析参数（bundleName, name, args）
  ├─ 2. napi_create_promise → deferred + promise
  ├─ 3. napi_create_threadsafe_function → tsfn
  │     失败 → reject(ERR_FUNCTION_EXECUTE_FAILED) → return promise
  ├─ 4. ffrt::submit [工作线程任务] ──────────────────────┐
  ├─ 5. ffrt::submit(delay=30s) [超时任务] ──────────────┐│
  └─ 6. return promise                                   ││
                                                          ││
  ┌───────────────────────────────────────────────────────┘│
  │ ffrt 工作线程:                                         │
  │   ├─ GetFunctionInfo(bundleName, name, info)           │
  │   │   失败 → call_tsfn({errorCode: QUERY_FAILED})      │
  │   ├─ functionInfo.functionType != INTENT_FUNCTION      │
  │   │   → call_tsfn({errorCode: TYPE_NOT_SUPPORTED})     │
  │   ├─ IntentClient::ExecuteIntentByFunctionCall(param)  │
  │   │   （内部完成 AddInsightIntentExecute + IPC 请求）   │
  │   │   失败 → call_tsfn({errorCode: EXECUTE_FAILED})     │
  │   └─ 成功: 等待 ProcessInsightIntentExecute 回调        │
  │                                                        │
  │ ┌──────────────────────────────────────────────────────┘
  │ │ ffrt 超时任务 (delay 30s 后调度):
  │ │   └─ completed == false → call_tsfn({errorCode: TIMEOUT})
  │ │
  │ └─── binder 线程 (稍后):
  │      ProcessInsightIntentExecute(resultCode, result)
  │        └─ completed == false → call_tsfn({success, result})
  │
  └─ JS 线程: InvokeFunctionCallJs (tsfn 回调)
      ├─ success=true → resolve(InvokeResult{由 executeResult 生成})
      ├─ success=false → napi_reject_deferred(error)
      └─ napi_release_threadsafe_function(tsfn)
```

### 5.3 竞态保护

使用 `std::atomic<bool> completed` 标志 + `compare_exchange_strong` 保证只有第一个完成者（正常回调或超时）能 resolve/reject promise 并释放 tsfn。

### 5.4 超时机制

- 默认超时：**30 秒**（`INVOKE_FUNCTION_TIMEOUT_US = 30000000`μs）
- 通过 `ffrt::submit` + `ffrt::task_attr().delay()` 提交延迟任务，由 ffrt 调度器在指定时间后执行
- 超时后通过 `napi_call_threadsafe_function` 回调到 JS 线程执行 reject
- 与正常回调共享同一个 `completed` 原子标志，保证只有一个生效

## 6. 意图执行详细设计

### 6.1 调用接口

意图执行统一通过 `AAFwk::IntentClient::ExecuteIntentByFunctionCall` 完成（位于 `interfaces/kits/native/intent_client`）。该接口对外只暴露简洁的 `ExecuteIntentParam`，内部自动完成 `InsightIntentHostClient` 注册、IPC 请求以及失败时的注册回滚，调用方不再需要关心 `intentKey`、`InsightIntentExecuteParam`、`ExecuteMode` 等底层细节。

调用方仅需构造以下结构体：

| `AAFwk::ExecuteIntentParam` 字段 | 来源 |
|----------------------------------|------|
| `bundleName` | invokeFunction 入参 `bundleName` |
| `intentName` | invokeFunction 入参 `name` |
| `wantParam` | invokeFunction 入参 `args`（Record → WantParams） |
| `callback` | `JsInvokeFunctionCallbackClient` 实例 |

`executeMode_`、`moduleName_`、`abilityName_` 等字段由 `AbilityManagerService::ExecuteIntentByFunctionCall` 在服务端根据意图配置自动解析并填充，客户端无需感知。

### 6.2 回调客户端

`JsInvokeFunctionCallbackClient` 继承 `InsightIntentExecuteCallbackInterface`：
- 仅持有 `InvokeFunctionTsfnContext` shared_ptr，通过它访问 tsfn、completed 等资源
- `ProcessInsightIntentExecute` 被调用时，通过 tsfn 将结果（含 `executeResult`）传回 JS 线程
- 使用 `tsfnContext->completed` 原子标志避免与超时竞态
- 回调生命周期由 `IntentClient` + `InsightIntentHostClient::OnExecuteDone` 自动管理：
  - IPC 失败时：`IntentClient` 内部立即 `RemoveInsightIntentExecute(key)`
  - IPC 成功后：服务端 `OnExecuteDone` 触发时自动从 `InsightIntentHostClient` 移除

`InvokeFunctionTsfnContext` 字段：
| 字段 | 类型 | 用途 |
|------|------|------|
| `deferred` | `napi_deferred` | Promise resolve/reject 句柄 |
| `tsfn` | `napi_threadsafe_function` | 跨线程回调到 JS 线程 |
| `completed` | `shared_ptr<atomic<bool>>` | 竞态保护标志，保证只有一个完成者 |

> 注：相比上一版方案，本设计移除了 `intentKey` 字段。`intentKey` 由 `IntentClient` 内部持有，不再暴露给调用方，`InvokeFunctionFinalize` 因此也不再需要回滚 `InsightIntentHostClient` 注册。

### 6.3 结果转换

在 `InvokeFunctionCallJs`（JS 线程 tsfn 回调）中：

```
success=true (resultCode == 0):
  → resolve(CreateJsInvokeResult(executeResult))
    - success = (executeResult.code == 0)
    - data = executeResult.result → WantParams → JS Object
    - errorCode = executeResult.code (仅非0时设置)
success=false:
  → reject(CreateCliJsErrorByNativeErr(errorCode))
```

`CreateJsInvokeResult` 直接接收 `InsightIntentExecuteResult`，内部统一处理 `code`、`result` 到 `InvokeResult` 的映射。

## 7. 错误处理策略

### 7.1 错误返回方式

所有错误场景统一使用 **Promise reject**。仅参数解析失败使用 JS 异常抛出。

### 7.2 错误处理流程

```
调用 invokeFunction
  │
  ├─ 参数解析失败 → JS 异常（ThrowInvalidParamError）
  │
  ├─ tsfn 创建失败 → reject(ERR_FUNCTION_EXECUTE_FAILED)
  │
  └─ 进入 ffrt 异步流程
       │
       ├─ GetFunctionInfo 失败
       │   → reject(ERR_FUNCTION_QUERY_FAILED)
       │
       ├─ Function 类型不支持
       │   → reject(ERR_FUNCTION_TYPE_NOT_SUPPORTED)
       │
       ├─ IntentClient::ExecuteIntentByFunctionCall 返回非0
       │   → reject(ERR_FUNCTION_EXECUTE_FAILED)
       │
       ├─ 意图执行回调 resultCode != 0
       │   → reject(ERR_FUNCTION_EXECUTE_FAILED)
       │
       ├─ 超时 30s
       │   → reject(ERR_FUNCTION_EXECUTE_TIMEOUT)
       │
       └─ 意图执行成功
           → resolve(InvokeResult{由 executeResult 生成})
```

## 8. 涉及修改的文件

| 文件 | 修改内容 |
|------|----------|
| `cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h` | 新增 5 个错误码 |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/cli_manager_error_utils.h` | 新增 6 个业务错误码枚举值 |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/cli_manager_error_utils.cpp` | 错误消息 + native-to-business 映射 |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/js_cli_manager.h` | 新增 InvokeFunction/OnInvokeFunction 声明 |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/js_cli_manager_utils.h` | 新增 CreateJsInvokeResult 声明 |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager_utils.cpp` | 实现 CreateJsInvokeResult |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | 核心实现：ffrt + tsfn + 回调客户端 + 超时；通过 `IntentClient::ExecuteIntentByFunctionCall` 发起意图执行 |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/BUILD.gn` | 依赖 `intent_client` SO（替换原 `ability_manager` 依赖），保留 ffrt 等其他依赖 |
| `interfaces/kits/native/intent_client` | 已存在的轻量级 SO，提供 `IntentClient::ExecuteIntentByFunctionCall` 及 `ExecuteIntentParam`；本方案直接复用 |

## 9. 编译验证

```bash
cd /data/home/w00926586/repo
./build.sh --product-name rk3568 --ccache --build-target climanager_napi
```

## 10. 后续扩展点

1. **FunctionInfo 字段扩展**：补充 `moduleName`、`abilityName` 等意图执行所需字段
2. **InvokeOptions 扩展**：添加 `timeout` 字段支持自定义超时时间
3. **新 FunctionType 支持**：后续支持 SYSTEM_ABC、HSP_ABC 类型
4. **参数校验增强**：基于 inputSchema 实现完整 JSON Schema 校验
