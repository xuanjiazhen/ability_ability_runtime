# Execution Plan

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Proposal | proposal.md | Draft |
| Spec | spec.md | Approved |
| Design | design.md | Approved |

## 执行原则

- **Spec 权威：** 若实现细节与 spec.md 的 AC、错误码冲突，先更新 spec/design。
- **文件边界：** Task 只能修改 Files 表列出的文件。
- **证据先行：** 每个 Task 先写测试再写实现。
- **反伪完成：** 只补声明或只跑 happy path 不能替代 AC 闭环。
- **JS API 测试优先：** 验收标准以 JS API 调用为测试入口，通过 register/unregister + execTool/execCmd/invokeFunction 组合验证业务逻辑。

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式（JS API 入口） |
|----|------|------|----------|
| AC-1.1 | spec.md | TASK-1, TASK-3 | `await cliManager.registerCliHook(hook)` 不抛异常 |
| AC-1.2 | spec.md | TASK-2, TASK-4 | 注册 hook（含 onBeforeCallTool）→ `cliManager.execTool(...)` → 回调断言 param 字段 |
| AC-1.3 | spec.md | TASK-4 | onBeforeCallTool 回调改写 param.args → execTool 结果反映改写 |
| AC-1.4 | spec.md | TASK-4 | onAfterCallTool 回调改写 execResult.outputText → execTool resolve 值为改写后 |
| AC-1.5 | spec.md | TASK-4 | 注册 hook（含 onBeforeCallCmd）→ `cliManager.execCmd(...)` → 回调断言 param.cmd |
| AC-1.6 | spec.md | TASK-4 | onAfterCallCmd 回调改写 → execCmd resolve 值为改写后 |
| AC-1.7 | spec.md | TASK-3 | `await cliManager.unregisterCliHook(hook)` → 再 execTool → 回调不触发 |
| AC-1.8 | spec.md | TASK-3, TASK-4 | 注册仅含 onBeforeCallTool 的 hook → execCmd → onBeforeCallCmd 不触发 |
| AC-2.1 | spec.md | TASK-1, TASK-3 | `await functionManager.registerFunctionHook(hook)` 不抛异常 |
| AC-2.2 | spec.md | TASK-2, TASK-4 | 注册 hook → `functionManager.invokeFunction(...)` → 回调断言 param 字段 |
| AC-2.3 | spec.md | TASK-4 | onBeforeInvokeFunction 回调改写 param.args → invokeFunction 结果反映改写 |
| AC-2.4 | spec.md | TASK-4 | onAfterInvokeFunction 回调改写 result.data → invokeFunction resolve 值为改写后 |
| AC-2.5 | spec.md | TASK-3 | `await functionManager.unregisterFunctionHook(hook)` → 再 invokeFunction → 回调不触发 |
| AC-3.1 | spec.md | TASK-5 | 非系统应用 register → `catch(err) => err.code == 202` |
| AC-3.2 | spec.md | TASK-5 | 缺权限 register → `catch(err) => err.code == 201` |
| AC-3.3 | spec.md | TASK-5 | 非开发模式 register → `catch(err) => err.code == 35600034` |
| AC-3.4 | spec.md | TASK-5 | 三层条件满足 → register/unregister 正常 |
| AC-4.1 | spec.md | TASK-3 | 连续两次 registerCliHook → 第二次 `err.code == 35600035` |
| AC-4.2 | spec.md | TASK-3 | 连续两次 registerFunctionHook → 第二次 `err.code == 35600035` |
| AC-4.3 | spec.md | TASK-3 | `registerCliHook(null)` → `err.code == 401` |
| AC-4.4 | spec.md | TASK-3 | `registerCliHook({})` → `err.code == 401` |
| AC-4.5 | spec.md | TASK-3 | 未注册时 unregisterCliHook(hook) → `err.code == 35600036` |
| AC-4.6 | spec.md | TASK-3 | 注册 hookA，unregisterCliHook(hookB) → `err.code == 401` |
| AC-4.7 | spec.md | TASK-3 | `unregisterCliHook()` → `err.code == 401` |
| AC-4.8 | spec.md | TASK-4 | 回调 throw → execTool 正常返回原值 |
| AC-4.9 | spec.md | TASK-4 | 回调阻塞 → 超时后 execTool 返回原值 |
| AC-5.1 | spec.md | TASK-2 | 不注册 Hook → execTool/execCmd/invokeFunction 正常 |
| AC-5.2 | spec.md | TASK-5 | 注册方进程死亡 → 服务侧 OnHookDied 清除 |
| AC-5.3 | spec.md | TASK-2 | 非开发模式已注册 Hook → 回调跳过 |
| AC-5.4 | spec.md | TASK-3 | env 销毁 → CleanupCliHookOnEnvDestroy 自动 unregister |

## 实现边界

**必须实现：** IDL 接口（ICliHookInterface/IFunctionHookInterface）+ 数据类型（ExecResultWrap/FunctionResultWrap/InvokeFunctionParam 等）+ 错误码 + 服务侧 Hook 存储/权限校验/独立 DeathRecipient/activeMethods 位掩码/回调分发/超时兜底 + NAPI Stub 缓存/JS 参数校验/unregister 对象校验/NAPI 回调同步分发（threadsafe_function） + 错误码映射（357xxxx→356xxxx） + env 清理 + SDK .d.ts。

**可后置：** 无。

## Task 列表

| Task ID | 目标 | AC 映射 |
|---------|------|---------|
| TASK-1 | IDL + 数据类型 + 错误码 | AC-1.1/2.1 |
| TASK-2 | 服务侧 Hook 回调点 + activeMethods + 超时兜底 | AC-1.2/1.5/2.2/5.1/5.3 |
| TASK-3 | NAPI Stub 缓存 + JS 参数校验 + unregister 对象校验 + 错误码映射 + env 清理 + SDK .d.ts | AC-1.7/2.5/4.1~4.7/5.4 |
| TASK-4 | NAPI 回调同步分发（threadsafe_function） | AC-1.3/1.4/1.6/2.3/2.4/4.8/4.9 |
| TASK-5 | 服务侧安全门控 + 独立 DeathRecipient | AC-3.1~3.4/5.2 |

---

### TASK-1: IDL + 数据类型 + 错误码

| 字段 | 内容 |
|------|------|
| 目标 | 新增 IDL 接口（ICliHookInterface/IFunctionHookInterface）+ 数据类型（ExecResultWrap/FunctionResultWrap/InvokeFunctionParam/InvokeFunctionResult）+ 错误码定义 + ICliToolManager 扩展 |
| AC 映射 | AC-1.1/2.1 |
| 前置 | 无 |
| 完成判据 | IDL 编译通过，Stub/Proxy 生成含完整参数 |

**Files**

| 操作 | 文件 |
|------|------|
| Create | `cli_tool_framework/interfaces/cli_tool/ICliHookInterface.idl` |
| Create | `cli_tool_framework/interfaces/cli_tool/IFunctionHookInterface.idl` |
| Modify | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` |
| Create | `cli_tool_framework/interfaces/cli_tool/include/exec_result_wrap.h` |
| Create | `cli_tool_framework/interfaces/cli_tool/src/exec_result_wrap.cpp` |
| Create | `cli_tool_framework/interfaces/cli_tool/include/function_result_wrap.h` |
| Create | `cli_tool_framework/interfaces/cli_tool/src/function_result_wrap.cpp` |
| Create | `cli_tool_framework/interfaces/cli_tool/include/invoke_function_param.h` |
| Create | `cli_tool_framework/interfaces/cli_tool/src/invoke_function_param.cpp` |
| Create | `cli_tool_framework/interfaces/cli_tool/include/invoke_function_result.h` |
| Create | `cli_tool_framework/interfaces/cli_tool/src/invoke_function_result.cpp` |
| Modify | `cli_tool_framework/interfaces/cli_tool/include/exec_cmd_param.h` |
| Modify | `cli_tool_framework/interfaces/cli_tool/src/exec_cmd_param.cpp` |
| Modify | `cli_tool_framework/interfaces/cli_tool/include/cli_error_code.h` |
| Modify | `cli_tool_framework/interfaces/cli_tool/BUILD.gn` |

**Steps**

- [ ] ICliHookInterface.idl：4 方法 `BeforeCallTool([inout] ExecToolParam)` / `AfterCallTool([inout] ExecResultWrap)` / `BeforeCallCmd([inout] ExecCmdParam)` / `AfterCallCmd([inout] ExecResultWrap)`
- [ ] IFunctionHookInterface.idl：2 方法 `BeforeInvokeFunction([inout] InvokeFunctionParam)` / `AfterInvokeFunction([inout] FunctionResultWrap)`
- [ ] ICliToolManager.idl：新增 `RegisterCliHook([in] ICliHookInterface, [in] int activeMethods)` / `UnregisterCliHook([in] ICliHookInterface)` / `RegisterFunctionHook([in] IFunctionHookInterface, [in] int activeMethods)` / `UnregisterFunctionHook([in] IFunctionHookInterface)` + sequenceable 声明
- [ ] exec_result_wrap.h/.cpp：ExecResultWrap{ExecResult execResult} + Parcelable
- [ ] function_result_wrap.h/.cpp：FunctionResultWrap{InvokeFunctionResult result} + Parcelable
- [ ] invoke_function_param.h/.cpp：InvokeFunctionParam{ns, name, args, context} + Parcelable
- [ ] invoke_function_result.h/.cpp：InvokeFunctionResult{success, data, errorCode, errorMsg} + Parcelable
- [ ] exec_cmd_param.h/.cpp：新增 execCmdOptions 字段 + Marshalling 扩展
- [ ] cli_error_code.h：新增 ERR_NOT_DEVELOPER_MODE(35700021)/ERR_HOOK_ALREADY_REGISTERED(35700022)/ERR_HOOK_NOT_REGISTERED(35700023)
- [ ] BUILD.gn：新增所有新源文件
- [ ] 编译验证：`./build.sh --product-name rk3568 --build-target abilityms`

---

### TASK-2: 服务侧 Hook 回调点 + activeMethods + 超时兜底

| 字段 | 内容 |
|------|------|
| 目标 | service 侧 ExecTool/ExecCmd/invokeFunction 回调点 + activeMethods 位掩码控制 + InvokeHookAsync 超时兜底 + 非开发模式跳过 |
| AC 映射 | AC-1.2/1.5/2.2/5.1/5.3 |
| 前置 | TASK-1 |
| 完成判据 | 回调仅调用已激活方法 + 超时返回原值 + 非开发模式跳过 + 无 Hook 时零开销 |

**Files**

| 操作 | 文件 |
|------|------|
| Modify | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` |
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` |
| Modify | `cli_tool_framework/interfaces/cli_tool/include/cli_tool_mgr_client.h` |
| Modify | `cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp` |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/src/invoke_function_executor.cpp` |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/include/invoke_function_executor.h` |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/src/invoke_function_callback_client.cpp` |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/include/invoke_function_callback_client.h` |

**Steps**

- [ ] cli_tool_manager_service.h：新增 `cliHookActiveMethods_`/`functionHookActiveMethods_` + 位掩码常量 + `HOOK_TIMEOUT_SECONDS=6` + `CheckCliHook(flag)`/`CheckFunctionHook(flag)` + `InvokeHookAsync` 模板 + `IsDeveloperMode()`
- [ ] cli_tool_manager_service.cpp：`isDeveloperMode_ = system::GetBoolParameter("const.security.developermode.state", false)`
- [ ] cli_tool_manager_service.cpp：`InvokeBeforeCallTool`/`InvokeBeforeCallCmd`/`InvokeAfterCallTool` 调用 `CheckCliHook(flag)` + `InvokeHookAsync`
- [ ] cli_tool_manager_service.cpp：`BeforeInvokeFunction`/`AfterInvokeFunction` 调用 `CheckFunctionHook(flag)` + `InvokeHookAsync`
- [ ] cli_tool_manager_service.cpp：`CheckCliHook`/`CheckFunctionHook` 先检查 `IsDeveloperMode()` → 检查 hook 非空 → 检查 `activeMethods & flag`
- [ ] cli_tool_manager_service.cpp：`InvokeHookAsync` 使用 condition_variable + `wait_for(HOOK_TIMEOUT_SECONDS)` 超时返回 false
- [ ] cli_tool_mgr_client.h/.cpp：新增 `BeforeInvokeFunction`/`AfterInvokeFunction` 代理方法
- [ ] invoke_function_executor.cpp：DoExecute 中调用 `CliToolMGRClient::GetInstance().BeforeInvokeFunction(param)` / `AfterInvokeFunction(result)`
- [ ] 单测：mock Hook Stub，验证回调仅调用已激活方法 + 超时返回原值 + 非开发模式跳过
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-3: NAPI Stub 缓存 + JS 参数校验 + unregister 对象校验 + 错误码映射 + env 清理 + SDK .d.ts

| 字段 | 内容 |
|------|------|
| 目标 | NAPI 层 Stub 缓存（g_cliHookStub/g_functionHookStub）；register 同类型已存在则 reject 35600035 不 IPC；JS 参数校验（argc + typeof + 至少一个方法为 function）；unregister 对象校验（napi_strict_equals）；错误码映射（357xxxx→356xxxx）；env 销毁自动 unregister；SDK .d.ts |
| AC 映射 | AC-1.7/2.5/4.1~4.7/5.4 |
| 前置 | TASK-1 |
| 完成判据 | Stub 缓存检查 + JS 参数校验 + unregister 对象校验在 NAPI 层完成；错误码为公共 356xxxx；.d.ts 含权限和错误码标注 |

**Files**

| 操作 | 文件 |
|------|------|
| Modify | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` |
| Modify | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/js_cli_manager.h` |
| Modify | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/cli_manager_error_utils.h` |
| Create | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/cli_manager_error_utils.cpp` |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_manager.cpp` |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/include/js_function_manager.h` |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/include/js_function_manager_utils.h` |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_manager_utils.cpp` |
| Modify | `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts` |
| Modify | `interface/sdk-js/api/@ohos.app.function.functionManager.d.ts` |
| Create | `interface/sdk-js/api/application/CliHook.d.ts` |
| Create | `interface/sdk-js/api/application/FunctionHook.d.ts` |
| Modify | `interface/sdk-js/kits/@kit.AbilityKit.d.ts` |

**Steps**

- [ ] cli_manager_error_utils.h/.cpp：`CliManagerErrorCode` 枚举（35600030~35600062）+ `NATIVE_TO_BUSINESS_ERROR_MAP` + `CreateCliJsErrorByNativeErr`
- [ ] js_cli_manager.cpp：新增 `static sptr<JsCliHook> g_cliHookStub` + `std::mutex g_cliHookMutex` + `ValidateCliHookObject`（检查对象类型 + 4 方法 bitmask）
- [ ] js_cli_manager.cpp OnRegisterCliHook：argc 检查 → ValidateCliHookObject → g_cliHookStub 检查 → 创建 JsCliHook + IPC → 成功则缓存 + napi_add_env_cleanup_hook
- [ ] js_cli_manager.cpp OnUnregisterCliHook：argc 检查 → g_cliHookStub 检查 → IsSameEnv 检查 → napi_strict_equals 校验 → IPC 注销 + 清空缓存
- [ ] js_cli_manager.cpp CleanupCliHookOnEnvDestroy：env 销毁时自动调用 UnregisterCliHook
- [ ] js_function_manager.cpp：同上模式（g_functionHookStub + ValidateFunctionHookObject + 2 方法 bitmask）
- [ ] cliManager.d.ts：registerCliHook(hook: CliHook) + @permission + @throws(201/202/35600034/35600035/35600050) + unregisterCliHook(hook: CliHook) + @throws(201/202/35600036/35600050) + export named types
- [ ] functionManager.d.ts：registerFunctionHook(hook: FunctionHook) + unregisterFunctionHook(hook: FunctionHook) + @permission + @throws + export named types
- [ ] CliHook.d.ts：CliHook(4 可选方法) + ExecToolParam + ExecCmdParam + ExecResultWrap
- [ ] FunctionHook.d.ts：FunctionHook(2 可选方法) + InvokeFunctionParam + FunctionResultWrap
- [ ] @kit.AbilityKit.d.ts：import + export CliHook/ExecToolParam/ExecCmdParam/ExecResultWrap/FunctionHook/InvokeFunctionParam/FunctionResultWrap
- [ ] 单测：register 已存在 → 35600035；register 非法参数 → 401；register 空对象 → 401；unregister 无 Stub → 35600036；unregister 对象不匹配 → 401；unregister 无参数 → 401
- [ ] 运行 `run -t UT -ts cli_tool_mgr_client_test`

---

### TASK-4: NAPI 回调同步分发（threadsafe_function）

| 字段 | 内容 |
|------|------|
| 目标 | JsCliHook/JsFunctionHook 的 threadsafe_function 同步分发 + CallJs 构造 JS param 对象 → napi_call_function → 解析返回值写回 C++ + 超时/异常兜底 |
| AC 映射 | AC-1.2~1.4/1.6/2.2~2.4/4.8/4.9 |
| 前置 | TASK-2, TASK-3 |
| 完成判据 | JS 回调在 JS 线程执行 + 返回值写回 C++ + 超时(5s)返回原值 + 异常返回原值 |

**Files**

| 操作 | 文件 |
|------|------|
| Create | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/js_cli_hook.h` |
| Create | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_hook.cpp` |
| Create | `cli_tool_framework/frameworks/js/napi/function_manager/include/js_function_hook.h` |
| Create | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_hook.cpp` |

**Steps**

- [ ] js_cli_hook.h：JsCliHook 类继承 CliHookInterfaceStub + HookCallData struct + HookMethodType enum + napi_ref/tsfn + DispatchToJs
- [ ] js_cli_hook.cpp：构造函数（napi_create_reference + napi_create_threadsafe_function）+ 4 方法 override（BeforeCallTool/AfterCallTool/BeforeCallCmd/AfterCallCmd）
- [ ] js_cli_hook.cpp DispatchToJs：打包 CallData → napi_call_threadsafe_function → promise.wait_for(5s)
- [ ] js_cli_hook.cpp CallJs：根据 methodType 构造 JS param 对象 → napi_call_function(jsCb, [param]) → 解析返回值写回 C++ param → promise.set_value
- [ ] js_cli_hook.cpp：超时 → 返回 false（服务侧使用原值）；异常 → try-catch 返回原值
- [ ] js_cli_hook.cpp ReleaseResources：napi_delete_reference + napi_release_threadsafe_function
- [ ] js_function_hook.h/.cpp：同上模式（HookCallData 含 InvokeFunctionParam/FunctionResultWrap + 2 方法）
- [ ] 单测：注册 CliHook → 触发 execTool → 验证 onBeforeCallTool 在 JS 线程执行 + param 改写生效
- [ ] 单测：JS 回调 throw → execTool 返回原值；JS 回调阻塞 → 超时后返回原值
- [ ] 运行 `run -t UT -ts cli_tool_mgr_client_test`

---

### TASK-5: 服务侧安全门控 + 独立 DeathRecipient

| 字段 | 内容 |
|------|------|
| 目标 | Register/Unregister Hook 入口新增 CheckSystemAndPermission（系统应用 + REGISTER_AGENT_HOOK 权限）；独立 DeathRecipient（HookDeathRecipient 带 HookType） |
| AC 映射 | AC-3.1~3.4/5.2 |
| 前置 | TASK-1 |
| 完成判据 | 非系统应用返回 202；无权限返回 201；非开发模式返回 35600034；进程死亡自动注销 |

**Files**

| 操作 | 文件 |
|------|------|
| Modify | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` |
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` |
| Modify | `cli_tool_framework/services/common/include/permission_util.h` |
| Modify | `cli_tool_framework/services/common/src/permission_util.cpp` |

**Steps**

- [ ] permission_util.h/.cpp：新增 `CheckSystemAndPermission(const std::string &permission)` → IsSystemApp + VerifyAccessToken，返回 ERR_NOT_SYSTEM_APP/ERR_PERMISSION_DENIED/ERR_OK
- [ ] cli_tool_manager_service.cpp：新增 `PERMISSION_REGISTER_AGENT_HOOK` 常量
- [ ] cli_tool_manager_service.cpp：RegisterCliHook/UnregisterCliHook/RegisterFunctionHook/UnregisterFunctionHook 入口调用 `PermissionUtil::CheckSystemAndPermission(PERMISSION_REGISTER_AGENT_HOOK)`
- [ ] cli_tool_manager_service.cpp：RegisterCliHook：非开发模式返回 ERR_NOT_DEVELOPER_MODE；已注册返回 ERR_HOOK_ALREADY_REGISTERED
- [ ] cli_tool_manager_service.cpp：UnregisterCliHook：非开发模式返回 ERR_NOT_DEVELOPER_MODE；对象不匹配返回 ERR_HOOK_NOT_REGISTERED
- [ ] cli_tool_manager_service.h：`HookDeathRecipient` 类带 `HookType type_`，OnRemoteDied 调用 `OnHookDied(remote, type_)`
- [ ] cli_tool_manager_service.cpp：`OnHookDied` 按 type 分别清除 cliHook_/functionHook_ + activeMethods + DeathRecipient
- [ ] Register/Unregister 使用各自独立 DeathRecipient（`new HookDeathRecipient(HookType::CLI/FUNCTION)`）
- [ ] 单测：非系统应用 → 202；无权限 → 201；非 dev mode → 35600034；三层通过 → 成功；进程死亡 → 自动注销
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`
