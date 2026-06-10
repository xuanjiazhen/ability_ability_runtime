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
  data?: any;          // result (WantParams) 转换的 JS 对象，仅在非空时返回
  errorCode?: number;  // executeResult.code，仅在非 0 时返回
  message?: string;    // 描述信息，仅在非空时返回（预留，当前为空）
}
```

> 注：`InvokeResult` 的 `success` 与 `errorCode` 由应用返回的 `executeResult.code` 决定（业务级口径），与 Promise 的 resolve/reject（由框架级 `resultCode` 决定）解耦——详见 6.3.1。`CreateJsInvokeResult` 入参为 `(env, resultCode=executeResult.code, result, message)`，由调用方显式传入这四个字段，函数内部不再读取 `InsightIntentExecuteResult` 结构体其它字段。

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
ERR_FUNCTION_QUERY_FAILED = 35700018,           // Function query failed
ERR_FUNCTION_TYPE_NOT_SUPPORTED = 35700019,     // Function type not supported
ERR_FUNCTION_PARAM_VALIDATE_FAILED = 35700020,  // Function param validate failed
ERR_FUNCTION_EXECUTE_FAILED = 35700021,          // Function execute failed
ERR_FUNCTION_EXECUTE_TIMEOUT = 35700022,          // Function execute timeout
```

> 注：invokeFunction 新增的错误码统一从 `35700019` 起编，避免与既有的 `ERR_JSON_PARSE_FAILED` (35700014)、`ERR_FILE_NOT_FOUND` (35700015)、`ERR_KVSTORE_NOT_READY` (35700016)、`ERR_FUNCTION_EXIST` (35700017) 冲突。`ERR_FUNCTION_QUERY_FAILED` = 35700018 为本组起始衔接值。

对应的业务错误码（`cli_manager_error_utils.h`，356xxxxx 段，无冲突）：

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
| `ERR_PERMISSION_DENIED` | GetFunctionInfo 返回 `ERR_PERMISSION_DENIED`（缺少 `ohos.permission.QUERY_CLI_TOOL`） | Promise reject |
| `ERR_FUNCTION_NOT_EXIST` | GetFunctionInfo 返回 `ERR_FUNCTION_NOT_EXIST`（KVStore KEY_NOT_FOUND） | Promise reject |
| `ERR_FUNCTION_QUERY_FAILED` | GetFunctionInfo 返回其它非 ERR_OK 值（IPC 失败、KVStore 未就绪、JSON 解析失败等） | Promise reject |
| `ERR_FUNCTION_TYPE_NOT_SUPPORTED` | FunctionInfo.functionType 不是 INTENT_FUNCTION | Promise reject |
| `ERR_PERMISSION_DENIED` | ExecuteIntentByFunctionCall 返回 `OHOS::ERR_PERMISSION_DENIED`（调用方缺少 `ohos.permission.EXECUTE_INSIGHT_INTENT`） | Promise reject |
| `ERR_FUNCTION_EXECUTE_FAILED` | ExecuteIntentByFunctionCall 返回其它非0值，或意图回调 resultCode != 0 | Promise reject |
| `ERR_FUNCTION_EXECUTE_TIMEOUT` | ffrt 超时任务触发 | Promise reject |

### 4.1 GetFunctionInfo 错误归因映射

`ExecuteInvokeFunction` 的 Step 1 查询阶段对 GetFunctionInfo 的返回值按下表归因，保留权限/不存在等关键语义，其余兜底为 `ERR_FUNCTION_QUERY_FAILED`：

| GetFunctionInfo 返回值 | 来源 | 归因（reportErr） | 最终业务码 |
|----------------------|------|------------------|-----------|
| `ERR_OK` | 查询成功 | 不 reject，继续 Step 2 | — |
| `ERR_PERMISSION_DENIED` (35700007) | Service 权限校验 | `ERR_PERMISSION_DENIED` | 201 |
| `ERR_FUNCTION_NOT_EXIST` (35700013) | KVStore KEY_NOT_FOUND | `ERR_FUNCTION_NOT_EXIST` | 35600060 |
| `ERR_NO_INIT` (35700004) | KVStore 未就绪 | `ERR_FUNCTION_QUERY_FAILED` | 35600065 |
| `ERR_JSON_PARSE_FAILED` (35700014) | JSON 解析/反序列化失败 | `ERR_FUNCTION_QUERY_FAILED` | 35600065 |
| `GET_CLI_TOOL_MGR_SERVICE_FAILED` (35700000) | SA 未注册 | `ERR_FUNCTION_QUERY_FAILED` | 35600065 |
| IPC 层错误（binder 死亡/超时等） | Proxy `SendRequest` 失败 | `ERR_FUNCTION_QUERY_FAILED` | 35600065 |
| KVStore 原生 Status（DB_ERROR/IPC_ERROR 等） | KVStore Get 非成功 | `ERR_FUNCTION_QUERY_FAILED` | 35600065 |

### 4.2 ExecuteIntentByFunctionCall 错误归因映射

`ExecuteInvokeFunction` 的 Step 3 执行阶段对 `IntentClient::ExecuteIntentByFunctionCall` 的返回值按下表归因。注意该接口返回 **AAFwk/通用错误空间**（非 cli 357xxxxx 段），权限失败由 `InsightIntentExecuteManager` 的授权门以通用 `OHOS::ERR_PERMISSION_DENIED`（= EPERM = 1）返回，需单独保留其精确语义，其余兜底为 `ERR_FUNCTION_EXECUTE_FAILED`：

| ExecuteIntentByFunctionCall 返回值 | 来源 | 归因（reportErr） | 最终业务码 |
|------------------------------------|------|------------------|-----------|
| `ERR_OK` (0) | IPC 成功，等待回调 | 不 reject，等待 `ProcessInsightIntentExecute` | — |
| `OHOS::ERR_PERMISSION_DENIED` (1) | Service 授权校验失败（缺 `EXECUTE_INSIGHT_INTENT`） | `ERR_PERMISSION_DENIED` | 201 |
| `ABILITY_SERVICE_NOT_CONNECTED` (2097154) | AbilityManagerService 未连接 | `ERR_FUNCTION_EXECUTE_FAILED` | 35600063 |
| `INNER_ERR` (2097167) | IPC parcel 写入失败 | `ERR_FUNCTION_EXECUTE_FAILED` | 35600063 |
| 其它非0 AAFwk/IPC 错误 | Proxy `SendRequest` 失败或服务端其它返回 | `ERR_FUNCTION_EXECUTE_FAILED` | 35600063 |

> 注：代码中以 `err == OHOS::ERR_PERMISSION_DENIED` 限定通用命名空间符号判定（值 1），与 cli 自身的 `ERR_PERMISSION_DENIED`（35700007，位于 `OHOS::CliTool` 命名空间）区分，避免不加限定的同名遮蔽导致误比较。

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

使用执行器成员 `std::atomic<bool> completed_` 标志 + `compare_exchange_strong` 保证只有第一个完成者（正常回调或超时）能上报结果（并经 tsfn resolve/reject promise、释放 tsfn）。该标志属于业务层，封装在 `InvokeFunctionExecutor` 内部。

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

### 6.2 业务执行器与回调客户端

invokeFunction 的业务逻辑（查询、类型校验、意图执行、超时、竞态保护）封装在纯 C++ 执行器 `InvokeFunctionExecutor` 中，**不依赖任何 napi 类型**，可独立编译/测试。执行结果通过注入的 `InvokeResultCallback`（`std::function<void(const InvokeFunctionResult &)>`）上抛，NAPI 层在该回调内调用 `napi_call_threadsafe_function` 将结果搬运回 JS 线程。

`InvokeFunctionExecutor`（`invoke_function_executor.h/.cpp`）：
- `Create()` 工厂创建（`std::make_shared` 单次分配；默认构造为 public 以支持 make_shared）；`Execute(bundleName, name, wantParams, callback)` 发起异步流程
- 内部 `ffrt::submit` 跑主任务（`DoExecute`）+ 延迟 30s 超时任务（`SetupTimeout`），二者共享执行器成员 `std::atomic<bool> completed_` 做竞态保护
- 生命周期由内部 ffrt/binder 闭包经 `shared_from_this` 保活，结果恰好上报一次后自动析构，NAPI 层无需长期持有

`InvokeFunctionCallbackClient`（独立类，`invoke_function_callback_client.h/.cpp`，便于脱离 IPC/napi 单测）继承 `InsightIntentExecuteCallbackInterface`：
- 直接持有共享的 `completed` 标志（`shared_ptr<atomic<bool>>`）与结果 `callback`，**不依赖执行器或 napi**，可注入 `callback` + 原子标志独立测试
- `ProcessInsightIntentExecute` 被调用时，CAS 命中 `completed` 后构造纯 C++ `InvokeFunctionResult`（`success`/`errorCode` 由框架 `resultCode` 决定，`resultCode` 字段承载应用级 `executeResult.code`，`result` 共享 `WantParams` 所有权）并回调
- 回调生命周期由 `IntentClient` + `InsightIntentHostClient::OnExecuteDone` 自动管理：
  - IPC 失败时：`IntentClient` 内部立即 `RemoveInsightIntentExecute(key)`
  - IPC 成功后：服务端 `OnExecuteDone` 触发时自动从 `InsightIntentHostClient` 移除

NAPI 层（`js_cli_manager.cpp`）的 `InvokeFunctionTsfnContext` 仅保留 napi 侧资源：
| 字段 | 类型 | 用途 |
|------|------|------|
| `deferred` | `napi_deferred` | Promise resolve/reject 句柄 |
| `tsfn` | `napi_threadsafe_function` | 跨线程回调到 JS 线程 |

> 注：相比耦合版本，`completed` 竞态标志已从 `InvokeFunctionTsfnContext` 下沉为执行器成员 `completed_`；`InvokeFunctionResultData` 删除，tsfn 直接搬运执行器的 `InvokeFunctionResult`。

### 6.3 结果转换

#### 6.3.1 双权威：框架 resultCode 与应用 executeResult.code 解耦（核心契约）

意图执行结果将 **框架级 `resultCode`** 与 **应用级 `executeResult.code`** 解耦，分别驱动 Promise 决策与 `InvokeResult` 字段：

| 判定 | 依据 | 取值来源 |
|------|------|---------|
| **Promise resolve/reject** | `resultCode`（callback 第 1 参数） | `InsightIntentExecuteManager` 框架级状态，由 `innerErr` 透传 |
| **InvokeResult.success / errorCode** | `executeResult.code`（callback 第 2 参数 `executeResult.code`） | 应用通过 `InsightIntentExecuteResult.code` 返回的业务级状态 |

**契约**：
- **Promise**：`resultCode == 0` → resolve；`resultCode != 0` → reject `ERR_FUNCTION_EXECUTE_FAILED`。
- **InvokeResult**：`executeResult.code == 0` → `success = true`；`executeResult.code != 0` → `success = false` 且 `errorCode = executeResult.code`。

> 两层口径相互独立：可能出现框架投递成功（`resultCode == 0`，Promise resolve）但应用业务失败（`executeResult.code != 0`，`InvokeResult.success = false`）的情况，调用方需依据 `InvokeResult.success` 进一步判断业务结果。

`InvokeFunctionResultData.resultCode` 携带 `executeResult.code`，连同 `result`（WantParams）、`message` 透传到 `CreateJsInvokeResult`，构造 `InvokeResult`；Promise 决策由同一结构体的 `success`/`errorCode`（仍按框架 `resultCode` 计算）驱动。

#### 6.3.2 resultCode 取值表（`ProcessInsightIntentExecute` 第 1 参数）

| resultCode | 值 | 来源 | 触发场景 |
|-----------|-----|------|---------|
| `INSIGHT_INTENT_ERR_OK` | 0 | 应用 `ExecuteInsightIntentDone`、`OpenLink` 成功 | 意图执行成功 → **resolve** |
| `INSIGHT_INTENT_INTERNAL_ERROR` | 1 | 应用显式设置 innerErr（罕见） | 应用内部错误 → reject `EXECUTE_FAILED` |
| `INSIGHT_INTENT_EXECUTE_REPLY_FAILED` | 2097278 | 应用 `ReplyFailedInner`、调用方进程死亡清理、remote 空回复 | 应用执行失败 / JS 回复异常 / 调用方死亡 → reject `EXECUTE_FAILED` |
| `ERR_INTENT_DEVICE_DISCONNECTED` | 2099421 | 远程意图超时、远程链路断开 | 分布式设备断连 → reject `EXECUTE_FAILED` |

> 注：`free_install_manager` / `remote_intent_result_callback` 路径会透传上游 resultCode，理论上可能传入上述之外的值，仍统一归到 reject `ERR_FUNCTION_EXECUTE_FAILED`。

#### 6.3.3 InvokeFunctionCallJs 处理逻辑

在 `InvokeFunctionCallJs`（JS 线程 tsfn 回调）中（注意 `resultData->resultCode` 实际承载 `executeResult.code`）：

```
success=true (框架 resultCode == 0):
  → resolve(CreateJsInvokeResult(env, resultData->resultCode, result, message))
    - success   = (executeResult.code == 0)    // 应用业务级口径
    - data      = result → WantParams → JS Object (result 非空时)
    - errorCode = executeResult.code (仅非0时设置)
    - message   = message (仅非空时设置；当前预留为空)
success=false (框架 resultCode != 0):
  → reject(CreateCliJsErrorByNativeErr(errorCode))   // errorCode = ERR_FUNCTION_EXECUTE_FAILED
```

`CreateJsInvokeResult(env, resultCode, result, message)` 接收四个显式字段，函数内部不再依赖 `InsightIntentExecuteResult` 结构体其它字段。

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
       │   ├─ 返回 OHOS::ERR_PERMISSION_DENIED → reject(ERR_PERMISSION_DENIED)
       │   └─ 其它非0 → reject(ERR_FUNCTION_EXECUTE_FAILED)
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
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/invoke_function_executor.h` | **新增**：纯 C++ 执行器 + `InvokeFunctionResult` + `InvokeResultCallback`（不含 napi 头） |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/invoke_function_executor.cpp` | **新增**：业务流程（查询/校验/执行/超时/竞态） |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/invoke_function_callback_client.h` | **新增**：独立回调客户端 `InvokeFunctionCallbackClient`（可单测，不含 napi） |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/invoke_function_callback_client.cpp` | **新增**：`ProcessInsightIntentExecute` 实现，竞态保护 + 结果组装 |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | NAPI 适配层：参数解析、promise/tsfn、bridge 回调注入执行器、`InvokeFunctionCallJs` resolve/reject |
| `cli_tool_framework/frameworks/js/napi/cli_tool_manager/BUILD.gn` | `sources` 增加 `invoke_function_executor.cpp`；依赖 `intent_client` SO，保留 ffrt 等其他依赖 |
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
