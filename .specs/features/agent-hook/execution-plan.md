# 执行计划

> 将 Approved Spec/Design 拆成可独立执行、可验证、可审查的 Task。v2.0：门控改为运行时 dev mode 判断（ADR-8），注册改为失败式（ADR-4），functionHook 改为 service 直接分发（ADR-5），新增 NAPI 层（ADR-9）。v3.0：每个 hook 独立 DeathRecipient（ADR-7 修订），新增系统应用 + 权限校验（ADR-10），hook 接口新增 HookOption 参数（ADR-11），NAPI Stub 缓存 + JS 参数校验（ADR-12），.d.ts 新增 @permission + @throws 标注（CHG-9），JS 侧系统内部错误返回 35600050（ADR-13/CHG-10）。v4.0：Hook 方法可选位掩码（ADR-14/CHG-11），Hook 调用异步化 + 6s 超时（ADR-15/CHG-12），HookOption JS 字段可选（CHG-13）。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-AGENT-HOOK |
| 关联 Feature | FEAT-AGENT-HOOK |
| 关联文档 | proposal.md / design.md / spec.md（v3.0） |
| 复杂度 | 标准 |
| 状态 | Draft (v6.0) |
| Owner | 待确认 |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | 通过（v2.0 变更，待用户确认） |
| Design | `design.md` | 通过（v2.0） |
| Spec | `spec.md` | Draft (v2.0) |

## Phase 声明（v2.0 统一）

> **v2.0 统一实现**：不再分 Phase 1/Phase 2。门控用 `system::GetBoolParameter("const.security.developermode.state", false)` 运行时判断（ADR-8），非开发模式注册返回 ERR_NOT_DEVELOPER_MODE(35700021) + 回调跳过。注册语义为失败式（ADR-4），已注册返回 ERR_HOOK_ALREADY_REGISTERED(35700022)。functionHook 改为 service 直接分发（ADR-5）。新增 NAPI 层（ADR-9）。

## 受影响文件全量清单

| 仓 | 层 | 文件路径 | 修改类型 | 说明 |
|----|----|----------|----------|------|
| ability_ability_runtime | IPC 契约层 | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` | 修改 | 新增 6 方法，移除 GetFunctionHook |
| ability_ability_runtime | IPC 回调契约层 | `cli_tool_framework/interfaces/cli_tool/ICliHookInterface.idl` | 新增 | 4 方法 |
| ability_ability_runtime | IPC 回调契约层 | `cli_tool_framework/interfaces/cli_tool/IFunctionHookInterface.idl` | 新增 | 2 方法 |
| ability_ability_runtime | IPC 契约层 | `cli_tool_framework/interfaces/cli_tool/BUILD.gn` | 修改 | 新增 2 个 .idl |
| ability_ability_runtime | 客户端层 | `cli_tool_framework/interfaces/cli_tool/include/cli_tool_mgr_client.h` | 修改 | 新增 6 方法声明 |
| ability_ability_runtime | 客户端层 | `cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp` | 修改 | 新增 6 方法代理 |
| ability_ability_runtime | 服务层 | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 修改 | 新增成员 + dev mode + 6 方法声明 |
| ability_ability_runtime | 服务层 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 修改 | 6 方法实现 + 回调点 + dev mode + 失败式注册 |
| ability_ability_runtime | 函数执行器层 | `cli_tool_framework/frameworks/js/napi/function_manager/include/invoke_function_executor.h` | 修改 | 调 BeforeInvokeFunction/AfterInvokeFunction |
| ability_ability_runtime | 函数执行器层 | `cli_tool_framework/frameworks/js/napi/function_manager/src/invoke_function_executor.cpp` | 修改 | 调 CliToolMGRClient::BeforeInvokeFunction/AfterInvokeFunction |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/js_cli_hook.h` | 新增 | JsCliHook（IPC Stub + per-object threadsafe_function） |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_hook.cpp` | 新增 | IPC Stub 实现 + 同步分发 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | 修改 | 新增 registerCliHook/unregisterCliHook 绑定 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/BUILD.gn` | 修改 | 新增 js_cli_hook.cpp + idl_gen_ICliHookInterface 依赖 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/function_manager/include/js_function_hook.h` | 新增 | JsFunctionHook |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_hook.cpp` | 新增 | IPC Stub 实现 + 同步分发 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_manager.cpp` | 修改 | 新增 registerFunctionHook/unregisterFunctionHook 绑定 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/function_manager/BUILD.gn` | 修改 | 新增 js_function_hook.cpp + idl_gen_IFunctionHookInterface 依赖 + cli_tool_mgr_client 依赖 |
| interface/sdk-js | SDK JS | `api/application/CliHook.d.ts` | 新增 | CliHook interface |
| interface/sdk-js | SDK JS | `api/application/FunctionHook.d.ts` | 新增 | FunctionHook interface |
| interface/sdk-js | SDK JS | `api/@ohos.app.cli.cliManager.d.ts` | 修改 | 新增 4 注册函数（返回 Promise<void>） |
| interface/sdk-js | SDK JS | `kits/@kit.AbilityKit.d.ts` | 修改 | 导出新类型 |
| interface/sdk-js | SDK JS | `zh-cn/api/application/CliHook.d.ts` | 新增 | 中文镜像 |
| interface/sdk-js | SDK JS | `zh-cn/api/application/FunctionHook.d.ts` | 新增 | 中文镜像 |
| interface/sdk-js | SDK JS | `zh-cn/api/@ohos.app.cli.cliManager.d.ts` | 修改 | 中文镜像新增注册函数 |
| ability_ability_runtime | 测试 | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | 修改 | 新增 Hook 回调 + dev mode + 失败式注册用例 |
| ability_ability_runtime | 测试 | `cli_tool_framework/test/unittest/invoke_function_executor_test/` | 修改 | 新增 BeforeInvokeFunction/AfterInvokeFunction 用例 |
| ability_ability_runtime | 测试 | `cli_tool_framework/test/unittest/cli_tool_manager_napi_test/` | 新增 | NAPI 层 JS CliHook 回调用例 |
| ability_ability_runtime | 测试 | `cli_tool_framework/test/unittest/function_manager_napi_test/` | 新增 | NAPI 层 JS FunctionHook 回调用例 |
| ability_ability_runtime | IPC 契约层 | ~~`cli_tool_framework/interfaces/cli_tool/include/hook_option.h`~~ | ~~新增（v3.0）~~ **已移除（v5.0 CHG-16）** | ~~HookOption struct~~ |
| ability_ability_runtime | IPC 契约层 | ~~`cli_tool_framework/interfaces/cli_tool/src/hook_option.cpp`~~ | ~~新增（v3.0）~~ **已移除（v5.0 CHG-16）** | ~~HookOption Parcelable~~ |
| ability_ability_runtime | 服务层 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 修改（v3.0） | 系统应用 + 权限校验（PermissionUtil）；独立 HookDeathRecipient(HookType) |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | 修改（v3.0） | Stub 缓存 + JS 参数校验 + `CreateCliJsErrorByNativeErr` default→35600050 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_manager.cpp` | 修改（v3.0） | Stub 缓存 + JS 参数校验 + 错误码映射 default→35600050 |
| ability_ability_runtime | 测试 | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | 修改（v3.0） | 新增权限校验 + 独立 HookDeathRecipient 用例 |
| ability_ability_runtime | 测试 | `cli_tool_framework/test/unittest/cli_tool_manager_napi_test/` | 修改（v3.0） | 新增 35600050 系统内部错误 + JS 参数校验用例 |
| ability_ability_runtime | IPC 契约层 | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` | 修改（v4.0） | RegisterCliHook/RegisterFunctionHook 新增 `[in] int activeMethods` 参数 |
| ability_ability_runtime | 服务层 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 修改（v4.0） | 位掩码跳过 + 5 个 Invoke* 改 FFRT+cv+6s + activeMethods_ 成员 |
| ability_ability_runtime | 服务层 | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 修改（v4.0） | activeMethods_ 成员 + InvokeHookAsync 模板声明 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | 修改（v4.0） | 构建位掩码 + JS 校验改为"至少 1 个方法" |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_manager.cpp` | 修改（v4.0） | 构建位掩码 + JS 校验改为"至少 1 个方法" |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_hook.cpp` | 修改（v4.0） | HOOK_TIMEOUT_MS 5000ms + CallJs 方法存在性检查 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_hook.cpp` | 修改（v4.0） | HOOK_TIMEOUT_MS 5000ms + CallJs 方法存在性检查 |
| interface/sdk-js | SDK JS | `api/application/CliHook.d.ts` | 修改（v4.0） | 方法改为可选（`?`） |
| interface/sdk-js | SDK JS | `api/application/FunctionHook.d.ts` | 修改（v4.0） | 方法改为可选（`?`） |
| ability_ability_runtime | 测试 | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | 修改（v4.0） | 位掩码跳过 + 异步超时 + 方法可选用例 |
| ability_ability_runtime | 测试 | `cli_tool_framework/test/unittest/cli_tool_manager_napi_test/` | 修改（v4.0） | 方法可选 + NAPI 5000ms 超时用例 |

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式 | 覆盖？ |
|----|------|------|----------|--------|
| AC-1.1 | spec.md | TASK-AH-002 | 单测 | 是 |
| AC-1.2 | spec.md | TASK-AH-003 | 单测 | 是 |
| AC-1.3 | spec.md | TASK-AH-003 | 单测 | 是 |
| AC-1.4 | spec.md | TASK-AH-003 | 单测 | 是 |
| AC-1.5 | spec.md | TASK-AH-003 | 单测 | 是 |
| AC-1.6 | spec.md | TASK-AH-004 | 单测 | 是 |
| AC-1.7 | spec.md | TASK-AH-005 | 单测 | 是 |
| AC-1.8 | spec.md | TASK-AH-012 | 单测 | 是 |
| AC-1.9 | spec.md | TASK-AH-002 | 单测 | 是 |
| AC-1.10 | spec.md | TASK-AH-011 | 单测 | 是 |
| AC-1.11 | spec.md | TASK-AH-005 | 单测 | 是 |
| AC-2.1 | spec.md | TASK-AH-006 | 单测 | 是 |
| AC-2.2 | spec.md | TASK-AH-007/008 | 单测 | 是 |
| AC-2.3 | spec.md | TASK-AH-008 | 单测 | 是 |
| AC-2.4 | spec.md | TASK-AH-007/008 | 单测 | 是 |
| AC-2.5 | spec.md | TASK-AH-008 | 单测 | 是 |
| AC-2.6 | spec.md | TASK-AH-006 | 单测 | 是 |
| AC-2.7 | spec.md | TASK-AH-012 | 单测 | 是 |
| AC-2.8 | spec.md | TASK-AH-006 | 单测 | 是 |
| AC-2.9 | spec.md | TASK-AH-009 | 单测 | 是 |
| AC-2.10 | spec.md | TASK-AH-011 | 单测 | 是 |
| AC-3.1 | spec.md | TASK-AH-012 | 单测 | 是 |
| AC-3.2 | spec.md | TASK-AH-012 | 单测 | 是 |
| AC-3.3 | spec.md | TASK-AH-012 | 单测 | 是 |
| AC-4.1 | spec.md | TASK-AH-015 | 单测 | 是 |
| AC-4.2 | spec.md | TASK-AH-016 | 单测 | 是 |
| AC-4.3 | spec.md | TASK-AH-015 | 单测 | 是 |
| AC-4.4 | spec.md | TASK-AH-016 | 单测 | 是 |
| AC-4.5 | spec.md | TASK-AH-016 | 单测 | 是 |
| AC-4.6 | spec.md | TASK-AH-015 | 单测 | 是 |
| AC-4.7 | spec.md | TASK-AH-019 | 单测 | 是 |
| AC-4.8 | spec.md | TASK-AH-019 | 单测 | 是 |
| AC-4.9 | spec.md | TASK-AH-019 | 单测 | 是 |
| AC-4.10 | spec.md | TASK-AH-019 | 单测 | 是 |
| AC-4.11 | spec.md | TASK-AH-020/021 | 单测 | 是 |
| AC-4.12 | spec.md | TASK-AH-022 | 单测 | 是 |
| AC-4.13 | spec.md | TASK-AH-023 | 单测 | 是 |
| AC-4.14 | spec.md | TASK-AH-024 | 单测 | 是 |
| AC-3.4 | spec.md | TASK-AH-017 | 单测 | 是 |
| AC-3.5 | spec.md | TASK-AH-017 | 单测 | 是 |
| AC-3.6 | spec.md | TASK-AH-017 | 单测 | 是 |
| AC-5.1 | spec.md | TASK-AH-018 | 单测 | 是 |
| AC-5.2 | spec.md | TASK-AH-018 | 单测 | 是 |
| AC-5.3 | spec.md | TASK-AH-018 | 单测 | 是 |

## 首批实现边界

**首批必须实现：** IDL + Stub/Proxy 生成（TASK-AH-001）；cliHook 存储 + dev mode 门控 + 失败式注册 + ExecTool 回调点（TASK-AH-002/003）。

**可后置：** NAPI 层（TASK-AH-013~016）；CliToolMGRClient 代理（TASK-AH-010）。

**不建议延后：** IDL 生成（所有 Task 依赖）；ExecTool 回调点（核心业务闭环）；dev mode 门控（安全约束）。

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 |
|---------|------|----------|---------|----------|----------|----------|
| TASK-AH-001 | 新增 IDL + Stub/Proxy + ICliToolManager 扩展 6 方法（移除 GetFunctionHook） | IDL + BUILD.gn | AC-1.1（部分） | 无 | IDL 编译通过 | `./build.sh --product-name rk3568 --build-target abilityms` |
| TASK-AH-002 | cliHook 存储 + dev mode 门控 + 失败式注册 RegisterCliHook | cli_tool_manager_service.h/.cpp | AC-1.1/1.8/1.9 | TASK-AH-001 | dev mode + 未注册时保存；非 dev mode 或已注册返回失败 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-003 | ExecTool 链路插入 BeforeCallTool/AfterCallTool（dev mode 门控） | cli_tool_manager_service.cpp | AC-1.2~1.5 | TASK-AH-002 | 回调触发 + 改写生效 + 非开发模式跳过 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-004 | ExecCmd 链路插入 BeforeCallCmd/AfterCallCmd | cli_tool_manager_service.cpp | AC-1.6 | TASK-AH-003 | ExecCmd 前后回调触发 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-005 | UnregisterCliHook + DeathRecipient 自动注销 | cli_tool_manager_service.cpp | AC-1.7/1.11 | TASK-AH-002 | 注销后不触发 + 进程死亡自动清 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-006 | functionHook 存储 + 失败式注册 RegisterFunctionHook/UnregisterFunctionHook | cli_tool_manager_service.h/.cpp | AC-2.1/2.6/2.7/2.8 | TASK-AH-001 | dev mode + 未注册时保存 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-007 | BeforeInvokeFunction/AfterInvokeFunction service 内部分发（dev mode 门控） | cli_tool_manager_service.cpp | AC-2.2/2.4 | TASK-AH-006 | service 内部判断有 functionHook 则 IPC 分发，无则直接返回 ERR_OK | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-008 | InvokeFunctionExecutor 调 CliToolMGRClient::BeforeInvokeFunction/AfterInvokeFunction | invoke_function_executor.h/.cpp | AC-2.2~2.5 | TASK-AH-007/010 | 调用方调 Before/After 到 service，service 分发到 hook | `run -t UT -ts invoke_function_executor_test` |
| TASK-AH-009 | DeathRecipient 自动注销 functionHook | cli_tool_manager_service.cpp | AC-2.9 | TASK-AH-006 | 进程死亡后 functionHook 清空，Before/After 返回 ERR_OK | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-010 | CliToolMGRClient 新增 6 方法代理 | cli_tool_mgr_client.h/.cpp | - | TASK-AH-001 | 客户端可调 Register/Unregister/Before/After | `run -t UT -ts cli_tool_mgr_client_test` |
| TASK-AH-011 | 无 Hook 时零开销验证 | cli_tool_manager_service.cpp | AC-1.10/2.10 | TASK-AH-003/007 | 无 Hook 时回调点跳过，零开销 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-012 | 非开发模式门控验证 | cli_tool_manager_service.cpp | AC-1.8/2.7/3.1~3.3 | TASK-AH-003/007 | 非 dev mode 注册失败 + 回调跳过 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-013 | SDK JS：CliHook.d.ts + FunctionHook.d.ts + cliManager 注册函数（Promise<void>） + @kit.AbilityKit + zh-cn 镜像 | interface/sdk-js/ | AC-4.1（部分） | TASK-AH-001 | .d.ts 编译通过 | SDK 构建验证 |
| TASK-AH-014 | NAPI Stub：JsCliHook（cli_tool_manager） + JsFunctionHook（function_manager） | frameworks/js/napi/cli_tool_manager/ + function_manager/ | AC-4.2/4.4 | TASK-AH-010/013 | IPC Stub 封装 JS 对象 + per-object threadsafe_function 同步分发 | `run -t UT -ts cli_tool_manager_napi_test` |
| TASK-AH-015 | NAPI JS 函数：registerCliHook/unregisterCliHook（cli_tool_manager） + registerFunctionHook/unregisterFunctionHook（function_manager），返回 Promise<void> | frameworks/js/napi/cli_tool_manager/ + function_manager/ | AC-4.1/4.3/4.6 | TASK-AH-014 | JS 调用注册成功 | `run -t UT -ts cli_tool_manager_napi_test` |
| TASK-AH-016 | NAPI 同步回调验证（binder→JS 线程→返回 + 超时兜底 5000ms） | frameworks/js/napi/cli_tool_manager/ + function_manager/ | AC-4.2/4.4/4.5 | TASK-AH-015 | JS 回调到 JS 线程执行 + 改写生效 + 超时返回原值 | `run -t UT -ts cli_tool_manager_napi_test` |
| TASK-AH-017 | v3.0：系统应用 + 权限校验（Register/Unregister Hook 入口 + REGISTER_AGENT_HOOK 权限声明） | cli_tool_manager_service.cpp | AC-3.4/3.5/3.6 | TASK-AH-002/006 | 非系统应用返回 ERR_NOT_SYSTEM_APP；无权限返回 ERR_PERMISSION_DENIED；系统应用 + 权限通过 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-018 | ~~v3.0：HookOption 参数~~ **已移除（v5.0 CHG-16）** | ~~ICliHookInterface.idl + IFunctionHookInterface.idl + ICliToolManager.idl + hook_option.h/.cpp~~ | ~~AC-5.1/5.2/5.3~~ | ~~TASK-AH-001~~ | ~~HookOption 完全移除~~ | — |
| TASK-AH-019 | v3.0：NAPI Stub 缓存 + JS 参数校验 + unregister 传 hook 对象 | js_cli_manager.cpp + js_function_manager.cpp | AC-4.6/4.7/4.8/4.9/4.10 | TASK-AH-015 | 同类型 Stub 已存在则直接返回异常不 IPC；unregister 传 hook 对象 + napi_strict_equals 校验；JS 参数校验（register argc + type + 方法存在）；无 Stub 返回 ERR_HOOK_NOT_REGISTERED | `run -t UT -ts cli_tool_manager_napi_test` |
| TASK-AH-020 | v3.0：.d.ts @permission + @throws 标注（含 35600050） | interface/sdk-js/ | AC-4.1/4.3/4.11 | TASK-AH-013 | .d.ts 含 @permission + @throws 标注；@throws 含 35600050（system error） | SDK 构建验证 |
| TASK-AH-021 | v3.0：NAPI `CreateCliJsErrorByNativeErr` 错误码映射（default→35600050） | js_cli_manager.cpp + js_function_manager.cpp | AC-4.11 | TASK-AH-019 | `CreateCliJsErrorByNativeErr` default 分支返回 35600050；不暴露 ERR_INVALID_PARAM(35700002) | `run -t UT -ts cli_tool_manager_napi_test` |
| TASK-AH-022 | v4.0：IDL + service 位掩码方法可选 + NAPI 构建掩码 + .d.ts 方法可选 | ICliToolManager.idl + cli_tool_manager_service.cpp/.h + js_cli_manager.cpp + js_function_manager.cpp + CliHook.d.ts + FunctionHook.d.ts | AC-4.12 | TASK-AH-017/019 | RegisterCliHook/RegisterFunctionHook 新增 activeMethods；service 按掩码跳过；NAPI 构建 JS→掩码；.d.ts 方法加 `?` | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-023 | v4.0：Hook 调用异步化（FFRT + cv + 6s 超时） | cli_tool_manager_service.cpp + js_cli_hook.cpp + js_function_hook.cpp | AC-4.13 | TASK-AH-022 | 5 个 Invoke* 方法改为 FFRT+cv+6s；NAPI HOOK_TIMEOUT_MS 5000ms | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-AH-024 | ~~v4.0：HookOption JS 字段可选~~ **已移除（v5.0 CHG-16）** | ~~CliHook.d.ts + js_cli_hook.cpp + js_function_hook.cpp~~ | ~~AC-4.14~~ | ~~TASK-AH-022~~ | ~~HookOption 完全移除~~ | — |

## Task 详情

### TASK-AH-001: IDL + Stub/Proxy + InvokeFunctionResult Parcelable 化

| 字段 | 内容 |
|------|------|
| 任务目标 | 新增 ICliHookInterface.idl、IFunctionHookInterface.idl；ICliToolManager.idl 新增 6 方法（移除 GetFunctionHook，新增 BeforeInvokeFunction/AfterInvokeFunction）；InvokeFunctionResult 改为 Parcelable + IDL sequenceable 声明；更新 BUILD.gn |
| AC 映射 | AC-1.1（部分：接口骨架） |
| 前置依赖 | 无 |
| 非目标 | 不实现业务逻辑 |
| 完成判据 | IDL 编译通过，Stub/Proxy 生成含新方法；InvokeFunctionResult Parcelable Marshalling/Unmarshalling 编译通过 |
| 停止条件 | IDL 语法错误无法生成 Stub/Proxy |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | `cli_tool_framework/interfaces/cli_tool/ICliHookInterface.idl` | 4 方法：BeforeCallTool/AfterCallTool/BeforeCallCmd/AfterCallCmd |
| Create | `cli_tool_framework/interfaces/cli_tool/IFunctionHookInterface.idl` | 2 方法：BeforeInvokeFunction/AfterInvokeFunction |
| Modify | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` | 新增 import + 6 方法（RegisterCliHook/UnregisterCliHook/RegisterFunctionHook/UnregisterFunctionHook/BeforeInvokeFunction/AfterInvokeFunction）；移除 GetFunctionHook |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/include/invoke_function_executor.h` | InvokeFunctionResult 改为继承 Parcelable + Marshalling/Unmarshalling |
| Modify | `cli_tool_framework/interfaces/cli_tool/BUILD.gn` | idl_gen_interface 新增 2 个 .idl + sequenceable InvokeFunctionResult |

**Design Context**

- ADR-1：独立 IPC 接口（ICliHookInterface/IFunctionHookInterface）
- ADR-2：异名（BeforeCallTool/AfterCallTool/BeforeCallCmd/AfterCallCmd）
- ADR-3：AfterCallTool/AfterCallCmd 入参为 ExecResult
- ADR-5：移除 GetFunctionHook，新增 BeforeInvokeFunction/AfterInvokeFunction
- InvokeFunctionResult Parcelable 化：当前为 `invoke_function_executor.h:36` 普通 struct，需改为继承 Parcelable

**Steps**

- [ ] 创建 ICliHookInterface.idl（package OHOS.CliTool；sequenceable ExecToolParam/ExecCmdParam/ExecResult/CliSessionInfo；4 方法）
- [ ] 创建 IFunctionHookInterface.idl（sequenceable InvokeFunctionResult/WantParams；2 方法）
- [ ] 修改 ICliToolManager.idl（新增 import + 6 方法；移除 GetFunctionHook）
- [ ] InvokeFunctionResult 改为继承 Parcelable + 实现 Marshalling/Unmarshalling
- [ ] 修改 BUILD.gn（idl_gen_interface 新增 2 个 .idl）
- [ ] 编译验证：`./build.sh --product-name rk3568 --build-target abilityms`

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 编译 | `./build.sh --product-name rk3568 --build-target abilityms` | PASS |

---

### TASK-AH-002: cliHook 存储 + dev mode 门控 + 失败式注册 RegisterCliHook

| 字段 | 内容 |
|------|------|
| 任务目标 | CliToolManagerService 新增 cliHook_ 存储成员 + IsDeveloperMode() + RegisterCliHook 实现（dev mode 门控 + 失败式注册） |
| AC 映射 | AC-1.1/1.8/1.9 |
| 前置依赖 | TASK-AH-001 |
| 非目标 | 不实现 ExecTool 回调点（TASK-AH-003）；不实现 UnregisterCliHook（TASK-AH-005） |
| 完成判据 | dev mode + 未注册时保存 Hook；非 dev mode 返回 ERR_NOT_DEVELOPER_MODE(35700021)；已注册返回 ERR_HOOK_ALREADY_REGISTERED(35700022) |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 新增 hookMutex_、cliHook_ 成员；IsDeveloperMode() 声明；RegisterCliHook 声明 |
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | RegisterCliHook 实现 + IsDeveloperMode() |

**Spec Context**

- AC-1.1: WHEN 开发模式 + 未注册 THEN 保存 Hook，返回 ERR_OK
- AC-1.8: WHEN 非开发模式 THEN 返回 ERR_NOT_DEVELOPER_MODE(35700021)
- AC-1.9: WHEN 已注册 THEN 返回 ERR_HOOK_ALREADY_REGISTERED(35700022)（失败式）

**Design Context**

- ADR-4: 失败式注册——已注册返回 ERR_HOOK_ALREADY_REGISTERED(35700022)，不覆盖
- ADR-8: `system::GetBoolParameter("const.security.developermode.state", false)` 运行时判断；`#include "parameters.h"`
- BUILD 依赖 `"init:libbegetutil"`（climgr BUILD.gn 已有）

**Steps**

- [ ] cli_tool_manager_service.h 新增成员：`ffrt::mutex hookMutex_; sptr<ICliHookInterface> cliHook_;`
- [ ] 新增 `bool IsDeveloperMode() const;` — 返回 `isDeveloperMode_`（在 `Init()` 中从 `system::GetBoolParameter("const.security.developermode.state", false)` 缓存）
- [ ] 实现 RegisterCliHook：PermissionUtil::CheckSystemAndPermission(REGISTER_AGENT_HOOK) → IsDeveloperMode() → 锁 hookMutex_，检查 cliHook_ 是否已存在，条件满足则保存；非 dev mode 返回 ERR_NOT_DEVELOPER_MODE(35700021)，已注册返回 ERR_HOOK_ALREADY_REGISTERED(35700022)
- [ ] 单测：dev mode + 未注册 → 成功；非 dev mode → 失败；已注册 → 失败
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-003: ExecTool 链路插入 BeforeCallTool/AfterCallTool 回调点

| 字段 | 内容 |
|------|------|
| 任务目标 | ExecTool 链路在 ValidateAndPrepareTool 之前插入 BeforeCallTool；BuildSessionInfo 之后插入 AfterCallTool（含 dev mode 门控） |
| AC 映射 | AC-1.2/1.3/1.4/1.5 |
| 前置依赖 | TASK-AH-002 |
| 非目标 | 不实现 ExecCmd 回调点（TASK-AH-004） |
| 完成判据 | dev mode + cliHook_ 非空时回调触发 + 改写生效；非 dev mode 或 cliHook_ 空时跳过 |
| 停止条件 | 回调导致既有执行链路中断 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | ExecTool 插入 BeforeCallTool/AfterCallTool |

**Design Context**

- ADR-6: 全部 hook 模式——返回值不影响 Hook 应用
- ADR-8: 回调点需检查 IsDeveloperMode()
- ExecTool 执行链路：ValidateExecToolPermissions → TryDispatchSkillSession → ValidateSessionLimit → ValidateAndPrepareTool → SetupAndStartSession
- BeforeCallTool 插入点：ValidateAndPrepareTool 之前
- AfterCallTool 插入点：BuildSessionInfo 之后

**Steps**

- [ ] ExecTool 链路在 ValidateAndPrepareTool 之前：IsDeveloperMode() + cliHook_ 非空则回调 BeforeCallTool(param)
- [ ] ExecTool 执行完成后：IsDeveloperMode() + cliHook_ 非空则回调 AfterCallTool(execResult)
- [ ] AfterCallTool 返回后用改写后的 ExecResult 填充 CliSessionInfo.result
- [ ] 单测：回调触发 + param/result 改写生效 + 非 dev mode 跳过
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-004: ExecCmd 链路插入 BeforeCallCmd/AfterCallCmd 回调点

| 字段 | 内容 |
|------|------|
| 任务目标 | ExecCmd 链路插入 BeforeCallCmd/AfterCallCmd（含 dev mode 门控） |
| AC 映射 | AC-1.6 |
| 前置依赖 | TASK-AH-003 |
| 完成判据 | dev mode + cliHook_ 非空时 ExecCmd 前后回调触发 |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | ExecCmd 插入 BeforeCallCmd/AfterCallCmd |

**Design Context**

- ExecCmd 执行链路：ValidateExecToolPermissions → ValidateSessionLimit → ValidateAndPrepareCmd → CreateShellProcess
- BeforeCallCmd 插入点：ValidateAndPrepareCmd 之前
- AfterCallCmd 插入点：BuildSessionInfo 之后

**Steps**

- [ ] ExecCmd 链路插入 BeforeCallCmd(param) + AfterCallCmd(execResult)
- [ ] 单测：验证 ExecCmd 前后回调 + 改写生效
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-005: UnregisterCliHook + DeathRecipient 自动注销

| 字段 | 内容 |
|------|------|
| 任务目标 | 实现 UnregisterCliHook + HookDeathRecipient 自动注销 cliHook_ |
| AC 映射 | AC-1.7/1.11 |
| 前置依赖 | TASK-AH-002 |
| 完成判据 | Unregister 后不触发回调；注册方进程死亡后 cliHook_ 自动清空 |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | HookDeathRecipient 内部类声明 |
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | UnregisterCliHook + AddDeathRecipient + OnHookDied |

**Design Context**

- ADR-7（修订）: 每个 hook 独立 HookDeathRecipient(HookType)，HookType 枚举区分 CLI/FUNCTION
- 既有模式参考：`event_dispatcher.h:81`

**Steps**

- [ ] 实现 HookDeathRecipient(HookType)（继承 IRemoteObject::DeathRecipient），OnRemoteDied 直接调 service OnHookDied
- [ ] RegisterCliHook 中 AddDeathRecipient
- [ ] 实现 UnregisterCliHook：锁 hookMutex_，清空 cliHook_，返回 ERR_OK
- [ ] 实现 OnHookDied：按 HookType 锁 hookMutex_，清空对应 cliHook_ 或 functionHook_
- [ ] 单测：注册 → 注销 → 验证不触发；模拟 Proxy 死亡 → 验证自动清空
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-006: functionHook 存储 + 失败式注册 RegisterFunctionHook/UnregisterFunctionHook

| 字段 | 内容 |
|------|------|
| 任务目标 | CliToolManagerService 新增 functionHook_ 存储 + RegisterFunctionHook/UnregisterFunctionHook 实现（dev mode + 失败式） |
| AC 映射 | AC-2.1/2.6/2.7/2.8 |
| 前置依赖 | TASK-AH-001 |
| 完成判据 | dev mode + 未注册时保存；非 dev mode 或已注册返回失败 |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 新增 functionHook_ 成员；RegisterFunctionHook/UnregisterFunctionHook 声明 |
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | RegisterFunctionHook/UnregisterFunctionHook 实现 |

**Steps**

- [ ] 新增成员：`sptr<IFunctionHookInterface> functionHook_;`（共用 hookMutex_）
- [ ] 实现 RegisterFunctionHook：PermissionUtil::CheckSystemAndPermission(REGISTER_AGENT_HOOK) → IsDeveloperMode() + 未注册 → 保存；非 dev mode 返回 ERR_NOT_DEVELOPER_MODE(35700021)，已注册返回 ERR_HOOK_ALREADY_REGISTERED(35700022)
- [ ] 实现 UnregisterFunctionHook：清空 functionHook_，返回 ERR_OK
- [ ] 单测：dev mode + 未注册 → 成功；非 dev mode → 失败；已注册 → 失败
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-007: BeforeInvokeFunction/AfterInvokeFunction service 内部分发

| 字段 | 内容 |
|------|------|
| 任务目标 | 实现 BeforeInvokeFunction/AfterInvokeFunction：service 内部判断 dev mode + functionHook_ 非空则 IPC 分发到注册方，无则直接返回 ERR_OK（零开销） |
| AC 映射 | AC-2.2/2.4 |
| 前置依赖 | TASK-AH-006 |
| 完成判据 | 有 functionHook + dev mode 则 IPC 分发回调；无则直接返回 ERR_OK（零开销） |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | BeforeInvokeFunction/AfterInvokeFunction 声明 |
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | BeforeInvokeFunction/AfterInvokeFunction 实现 |

**Design Context**

- ADR-5: service 直接分发——调用方 IPC 到 service，service 内部分发到 functionHook_
- 无 functionHook 或非 dev mode 时直接返回 ERR_OK（零开销，调用方无感知）
- 回调为 FFRT 异步包裹 + condition_variable wait_for(6s) 超时（HOOK_TIMEOUT_SECONDS = 6）

**Steps**

- [ ] 实现 BeforeInvokeFunction(InvokeFunctionParam& param)：PermissionUtil::CheckSystemAndPermission(ACCESS_FUNCTION) → IsDeveloperMode() → CheckFunctionHook(FUNC_HOOK_BEFORE_INVOKE) 非空 → InvokeHookAsync(hook->BeforeInvokeFunction, param)；否则直接返回 ERR_OK
- [ ] 实现 AfterInvokeFunction(FunctionResultWrap& functionResultWrap)：同上逻辑（CheckFunctionHook(FUNC_HOOK_AFTER_INVOKE)）
- [ ] 单测：有 functionHook → 分发回调；无 functionHook → 直接返回 ERR_OK
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-008: InvokeFunctionExecutor 调用 CliToolMGRClient::BeforeInvokeFunction/AfterInvokeFunction

| 字段 | 内容 |
|------|------|
| 任务目标 | InvokeFunctionExecutor::DoExecute 中调用 CliToolMGRClient::BeforeInvokeFunction/AfterInvokeFunction（IPC 到 service），替代旧 GetFunctionHook 查询 |
| AC 映射 | AC-2.2/2.3/2.4/2.5 |
| 前置依赖 | TASK-AH-007/010 |
| 完成判据 | 调用方调 Before/After 到 service，service 分发到 hook，改写生效 |
| 停止条件 | 回调导致 invokeFunction 链路中断 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/include/invoke_function_executor.h` | 移除 functionHook_ 成员 |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/src/invoke_function_executor.cpp` | DoExecute 插入 BeforeInvokeFunction/AfterInvokeFunction 调用 |

**Spec Context**

- AC-2.2: WHEN functionHook 已注册 THEN 调用方调 BeforeInvokeFunction（IPC 到 service），service 分发
- AC-2.3: WHEN BeforeInvokeFunction 返回改写后 param THEN 调用方使用改写后 param 执行
- AC-2.4: WHEN invokeFunction 执行完成 THEN 调用方调 AfterInvokeFunction
- AC-2.5: WHEN AfterInvokeFunction 返回改写后 result THEN 调用方将改写后 result 返回给上层

**Design Context**

- ADR-5: 调用方不需感知 Hook 存在，只需调 service 的 Before/After
- InvokeFunctionExecutor::DoExecute：GetFunctionInfo → functionType 校验 → ExecuteIntentByFunctionCall
- BeforeInvokeFunction 插入点：GetFunctionInfo 之后、ExecuteIntentByFunctionCall 之前
- AfterInvokeFunction 插入点：InvokeFunctionCallbackClient 回调结果到达后

**Steps**

- [ ] DoExecute 中 GetFunctionInfo 之后插入：`CliToolMGRClient::GetInstance().BeforeInvokeFunction(invokeFunctionParam)`
- [ ] 结果回调到达后插入：`CliToolMGRClient::GetInstance().AfterInvokeFunction(functionResultWrap)`
- [ ] 移除旧 GetFunctionHook 相关代码
- [ ] 单测：mock CliToolMGRClient，验证 Before/After 调用 + 改写生效
- [ ] 运行 `run -t UT -ts invoke_function_executor_test`

---

### TASK-AH-009: DeathRecipient 自动注销 functionHook

| 字段 | 内容 |
|------|------|
| 任务目标 | OnHookDied(HookType::FUNCTION) 清除 functionHook_（复用 TASK-AH-005 的 HookDeathRecipient 机制） |
| AC 映射 | AC-2.9 |
| 前置依赖 | TASK-AH-005/006 |
| 完成判据 | 进程死亡后 functionHook_ 清空，Before/After 返回 ERR_OK |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | OnHookDied 按 HookType::FUNCTION 清空 functionHook_ |

**Steps**

- [ ] OnHookDied(HookType::FUNCTION)：锁 hookMutex_，清空 functionHook_
- [ ] 单测：模拟 Proxy 死亡，验证 functionHook_ 清空 + Before/After 返回 ERR_OK
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-010: CliToolMGRClient 新增 6 方法代理

| 字段 | 内容 |
|------|------|
| 任务目标 | CliToolMGRClient 新增 RegisterCliHook/UnregisterCliHook/RegisterFunctionHook/UnregisterFunctionHook/BeforeInvokeFunction/AfterInvokeFunction 6 方法代理 |
| AC 映射 | -（基础设施） |
| 前置依赖 | TASK-AH-001 |
| 完成判据 | 客户端可调 Register/Unregister/Before/After |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/interfaces/cli_tool/include/cli_tool_mgr_client.h` | 新增 6 方法声明 |
| Modify | `cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp` | 新增 6 方法代理（GetCliToolMgrProxy → proxy->Method()） |

**Steps**

- [ ] cli_tool_mgr_client.h 新增 6 方法声明
- [ ] cli_tool_mgr_client.cpp 实现：GetCliToolMgrProxy() → proxy->Method()
- [ ] 单测：验证客户端代理调用
- [ ] 运行 `run -t UT -ts cli_tool_mgr_client_test`

---

### TASK-AH-011: 无 Hook 时零开销验证

| 字段 | 内容 |
|------|------|
| 任务目标 | 验证无 Hook 时 service 直接返回 ERR_OK（零开销），执行链路与引入前一致 |
| AC 映射 | AC-1.10/2.10 |
| 前置依赖 | TASK-AH-003/007 |
| 完成判据 | 无 Hook 时回调点跳过，零开销 |
| 停止条件 | — |

**Steps**

- [ ] 单测：cliHook_ + functionHook_ 均空时 ExecTool/ExecCmd 无 IPC 回调
- [ ] 单测：BeforeInvokeFunction/AfterInvokeFunction 无 functionHook 时直接返回 ERR_OK
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-012: 非开发模式门控验证

| 字段 | 内容 |
|------|------|
| 任务目标 | 验证非开发模式注册失败 + 回调跳过 |
| AC 映射 | AC-1.8/2.7/3.1~3.3 |
| 前置依赖 | TASK-AH-003/007 |
| 完成判据 | 非 dev mode 注册返回 ERR_NOT_DEVELOPER_MODE(35700021) + 回调跳过 |
| 停止条件 | — |

**Steps**

- [ ] 单测：mock isDeveloperMode_ 为 false，RegisterCliHook/RegisterFunctionHook 返回 ERR_NOT_DEVELOPER_MODE(35700021)
- [ ] 单测：非 dev mode 时 ExecTool/ExecCmd/BeforeInvokeFunction/AfterInvokeFunction 回调跳过
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-013: SDK JS — CliHook.d.ts + FunctionHook.d.ts + cliManager 注册函数 + zh-cn 镜像

| 字段 | 内容 |
|------|------|
| 任务目标 | 新增 CliHook.d.ts、FunctionHook.d.ts；cliManager.d.ts 新增 4 注册函数（返回 Promise<void>）；@kit.AbilityKit 导出新类型；zh-cn 镜像文件 |
| AC 映射 | AC-4.1（部分） |
| 前置依赖 | TASK-AH-001 |
| 完成判据 | .d.ts 编译通过 |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | `interface/sdk-js/api/application/CliHook.d.ts` | CliHook interface + ExecToolParam/ExecCmdParam 类型 |
| Create | `interface/sdk-js/api/application/FunctionHook.d.ts` | FunctionHook interface |
| Modify | `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts` | 新增 4 函数（Promise<void>） |
| Modify | `interface/sdk-js/kits/@kit.AbilityKit.d.ts` | 导出 CliHook、FunctionHook |
| Create | `interface/sdk-js/zh-cn/api/application/CliHook.d.ts` | 中文镜像 |
| Create | `interface/sdk-js/zh-cn/api/application/FunctionHook.d.ts` | 中文镜像 |
| Modify | `interface/sdk-js/zh-cn/api/@ohos.app.cli.cliManager.d.ts` | 中文镜像新增注册函数 |

**Design Context**

- 参数复用已有 JS API 类型：ExecOptions（cliManager）、InvokeResult（functionManager）
- JS callback 方法返回修改后对象（非 in-place）
- @systemapi @stagemodelonly @since 26.0.0 dynamiconly

**Steps**

- [ ] 创建 CliHook.d.ts（interface CliHook + ExecToolParam + ExecCmdParam，import ExecOptions/ExecResult）
- [ ] 创建 FunctionHook.d.ts（interface FunctionHook，import InvokeResult）
- [ ] 修改 cliManager.d.ts（新增 registerCliHook/unregisterCliHook/registerFunctionHook/unregisterFunctionHook，返回 Promise<void>）
- [ ] 修改 @kit.AbilityKit.d.ts（导出 CliHook、FunctionHook）
- [ ] 创建 zh-cn 镜像文件
- [ ] SDK 构建验证

---

### TASK-AH-014: NAPI Stub — JsCliHook（cli_tool_manager） + JsFunctionHook（function_manager）

| 字段 | 内容 |
|------|------|
| 任务目标 | 在 cli_tool_manager 模块新增 JsCliHook；在 function_manager 模块新增 JsFunctionHook。IPC Stub 封装 JS 对象 + per-object threadsafe_function 同步分发 |
| AC 映射 | AC-4.2/4.4 |
| 前置依赖 | TASK-AH-010/013 |
| 完成判据 | IPC Stub 封装 JS 对象成功 + threadsafe_function 同步分发到 JS 线程 |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/js_cli_hook.h` | JsCliHook 声明 |
| Create | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_hook.cpp` | JsCliHook 实现 |
| Create | `cli_tool_framework/frameworks/js/napi/function_manager/include/js_function_hook.h` | JsFunctionHook 声明 |
| Create | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_hook.cpp` | JsFunctionHook 实现 |
| Modify | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/BUILD.gn` | 新增 stub.cpp + idl_gen_ICliHookInterface |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/BUILD.gn` | 新增 stub.cpp + idl_gen_IFunctionHookInterface |

**Design Context**

- ADR-9: per-object 1 个 napi_threadsafe_function（非 per-method）
- ADR-9: JS callback 返回修改后对象，NAPI 从 napi_call_function 返回值解析
- 同步分发：binder 线程 → std::promise → napi_call_threadsafe_function(blocking) → future.wait_for(5000ms) → JS 线程 CallJs → promise.set_value
- 超时兜底：5000ms，超时返回原 param

**Steps**

- [ ] 实现 JsCliHook（继承 IRemoteStub<ICliHookInterface>）：napi_ref + 1 个 tsfn_ + DispatchSync 模板
- [ ] CallJs 回调：从 CallData 取 method 名 + param → 转 JS 对象 → napi_call_function(jsObj, method, ...) → 从返回值解析 → promise.set_value
- [ ] 实现 JsFunctionHook（继承 IRemoteStub<IFunctionHookInterface>）：同上结构
- [ ] 更新 BUILD.gn
- [ ] 单测：验证 Stub 封装 + 同步分发
- [ ] 运行 `run -t UT -ts cli_tool_manager_napi_test`

---

### TASK-AH-015: NAPI JS 函数 — registerCliHook/unregisterCliHook + registerFunctionHook/unregisterFunctionHook

| 字段 | 内容 |
|------|------|
| 任务目标 | 在 cli_tool_manager 模块绑定 registerCliHook/unregisterCliHook；在 function_manager 模块绑定 registerFunctionHook/unregisterFunctionHook。返回 Promise<void> |
| AC 映射 | AC-4.1/4.3/4.6 |
| 前置依赖 | TASK-AH-014 |
| 完成判据 | JS 调用注册成功，返回 Promise |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | 新增 RegisterCliHook/UnregisterCliHook 绑定 |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_manager.cpp` | 新增 RegisterFunctionHook/UnregisterFunctionHook 绑定 |

**Design Context**

- JS 函数返回 Promise<void>：napi_create_promise → 注册 → napi_resolve_deferred/napi_reject_deferred
- 创建 JsCliHook/JsFunctionHook 封装 JS 对象 → 调 CliToolMGRClient 注册

**Steps**

- [ ] js_cli_manager.cpp 新增 RegisterCliHook/UnregisterCliHook：创建 Promise + 创建 Stub + 调 CliToolMGRClient + resolve/reject
- [ ] js_function_manager.cpp 新增 RegisterFunctionHook/UnregisterFunctionHook：同上
- [ ] BindNativeFunction 绑定到 JS export 对象
- [ ] 单测：JS 调用注册成功
- [ ] 运行 `run -t UT -ts cli_tool_manager_napi_test`

---

### TASK-AH-016: NAPI 同步回调验证

| 字段 | 内容 |
|------|------|
| 任务目标 | 验证 binder 线程 → JS 线程同步回调 + 改写生效 + 超时兜底 5000ms |
| AC 映射 | AC-4.2/4.4/4.5 |
| 前置依赖 | TASK-AH-015 |
| 完成判据 | JS 回调到 JS 线程执行 + 改写后数据通过 IPC 返回 + 超时返回原值 |
| 停止条件 | — |

**Steps**

- [ ] 单测：注册 CliHook → 触发 ExecTool → 验证 beforeCallTool 在 JS 线程执行 + param 改写生效
- [ ] 单测：注册 FunctionHook → 触发 invokeFunction → 验证 beforeInvokeFunction/afterInvokeFunction 在 JS 线程执行 + 改写生效
- [ ] 单测：JS 回调超时（>5000ms）→ 验证返回原 param（未改写）
- [ ] 单测：JS 回调抛异常 → 验证返回原 param
- [ ] 单测：JS 调用 unregisterCliHook → 验证后续不触发回调
- [ ] 运行 `run -t UT -ts cli_tool_manager_napi_test`

---

### TASK-AH-017: v3.0 — 系统应用 + 权限校验

| 字段 | 内容 |
|------|------|
| 任务目标 | Register/Unregister Hook 入口新增系统应用校验（`IsSystemAppByFullTokenID`）+ 权限校验（`ohos.permission.REGISTER_AGENT_HOOK`）；新增权限常量 |
| AC 映射 | AC-3.4/3.5/3.6 |
| 前置依赖 | TASK-AH-002/006 |
| 完成判据 | 非系统应用返回 ERR_NOT_SYSTEM_APP；无权限返回 ERR_PERMISSION_DENIED；系统应用 + 权限通过 |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | RegisterCliHook/UnregisterCliHook/RegisterFunctionHook/UnregisterFunctionHook 入口新增系统应用 + 权限校验 |
| Modify | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | `PermissionUtil::CheckSystemAndPermission()` 调用 |

**Design Context**

- ADR-10: 系统应用 + 权限校验（Register/Unregister 用 `ohos.permission.REGISTER_AGENT_HOOK`；Before/AfterInvokeFunction 用 `ohos.permission.ACCESS_FUNCTION`）
- 校验顺序：PermissionUtil::CheckSystemAndPermission(IsSystemApp||IsSystemSA → VerifyAccessToken) → IsDeveloperMode
- 参照既有 Pattern A（`cli_tool_manager_service.cpp:542-552`）
- 新增权限常量：`constexpr const char* PERMISSION_REGISTER_AGENT_HOOK = "ohos.permission.REGISTER_AGENT_HOOK";` + `constexpr const char* PERMISSION_ACCESS_FUNCTION = "ohos.permission.ACCESS_FUNCTION";`

**Steps**

- [ ] 新增 `PERMISSION_REGISTER_AGENT_HOOK` + `PERMISSION_ACCESS_FUNCTION` 常量
- [ ] 新增 `PermissionUtil::CheckSystemAndPermission()` 方法：IsSystemApp||IsSystemSA → VerifyAccessToken
- [ ] RegisterCliHook/UnregisterCliHook/RegisterFunctionHook/UnregisterFunctionHook 入口调用 PermissionUtil::CheckSystemAndPermission(REGISTER_AGENT_HOOK)；BeforeInvokeFunction/AfterInvokeFunction 入口调用 PermissionUtil::CheckSystemAndPermission(ACCESS_FUNCTION)
- [ ] 单测：非系统应用 → ERR_NOT_SYSTEM_APP；无权限 → ERR_PERMISSION_DENIED；系统应用 + 权限 → 通过
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-018: ~~v3.0 — HookOption 参数~~ **已移除（v5.0 CHG-16）**

> **此 Task 已废弃。** HookOption struct 完全移除（v5.0 CHG-16）。无 hook_option.h/.cpp，IDL 方法无 HookOption 参数，service 无 HookOption 透传。InvokeFunctionParam.invokeOptions 仅含 `{ context: string }`。

---

### TASK-AH-019: v3.0 — NAPI Stub 缓存 + JS 参数校验 + unregister 传 hook 对象

| 字段 | 内容 |
|------|------|
| 任务目标 | NAPI 层保存 Stub 引用（per hook type）；同类型已存在则直接返回异常不 IPC；unregister 传 hook 对象 + napi_strict_equals 校验同一性；增加 JS 参数校验（register argc + napi_typeof + 回调方法存在性） |
| AC 映射 | AC-4.6/4.7/4.8/4.9/4.10 |
| 前置依赖 | TASK-AH-015 |
| 完成判据 | Stub 缓存检查 + JS 参数校验在 NAPI 层完成；unregister 传 hook 对象 + napi_strict_equals 校验后用本地 Stub |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | OnRegisterCliHook 新增 Stub 缓存 + JS 参数校验 + 去除 napi_wrap；OnUnregisterCliHook 传 hook 对象 + napi_strict_equals 校验 |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_manager.cpp` | OnRegisterFunctionHook/OnUnregisterFunctionHook 同上模式 |

**Design Context**

- ADR-12: NAPI Stub 缓存——静态变量保存 Stub 引用；同类型已存在则 reject 不 IPC
- unregister 传 hook 对象：用 napi_strict_equals 校验与 g_cliHookStub/g_functionHookStub 同一性，无则 reject ERR_HOOK_NOT_REGISTERED 不 IPC
- 注册时不再使用 napi_wrap/napi_unwrap（Stub 生命周期由 g_cliHookStub sptr 管理）
- JS 参数校验：argc 检查 + `napi_typeof` 检查对象类型 + 检查必需回调方法是否存在（`napi_get_named_property` + `napi_typeof` 检查 function 类型）
- CliHook 需检查：onBeforeCallTool, onAfterCallTool, onBeforeCallCmd, onAfterCallCmd
- FunctionHook 需检查：beforeInvokeFunction, afterInvokeFunction

**Steps**

- [ ] 新增静态变量 `static sptr<JsCliHook> g_cliHookStub` 和 `static sptr<JsFunctionHook> g_functionHookStub`
- [ ] OnRegisterCliHook：先校验 JS 参数（argc + 对象类型 + 4 方法存在），再检查 g_cliHookStub 是否已存在（有则 reject 不 IPC），再创建 Stub + IPC；去除 napi_wrap
- [ ] OnUnregisterCliHook：无参数；检查 g_cliHookStub 是否存在（无则 reject ERR_HOOK_NOT_REGISTERED 不 IPC），有则 IPC 注销 + 清除 g_cliHookStub
- [ ] OnRegisterFunctionHook/OnUnregisterFunctionHook 同上模式
- [ ] 单测：register 已存在 → 返回 ERR_HOOK_ALREADY_REGISTERED；register 无参数/非对象/缺方法 → 返回 ERR_INVALID_PARAM；unregister 无缓存 → 返回 ERR_HOOK_NOT_REGISTERED；unregister 正常 → 注销成功
- [ ] 运行 `run -t UT -ts cli_tool_manager_napi_test`

---

### TASK-AH-020: v3.0 — .d.ts @permission + @throws 标注（含 35600050）

| 字段 | 内容 |
|------|------|
| 任务目标 | SDK .d.ts 文件新增 @permission 标注（`ohos.permission.REGISTER_AGENT_HOOK`）和 @throws 错误码说明（含 35600050 system error） |
| AC 映射 | AC-4.1/4.3/4.11 |
| 前置依赖 | TASK-AH-013 |
| 完成判据 | .d.ts 含 @permission + @throws 标注；@throws 含 35600050（system error） |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts` | 4 个注册函数新增 @permission + @throws（含 35600050） |
| Modify | `interface/sdk-js/zh-cn/api/@ohos.app.cli.cliManager.d.ts` | 中文镜像同步 |

**Design Context**

- CHG-9: .d.ts 新增 @permission 标注和 @throws 错误码说明
- CHG-10/ADR-13: JS 侧系统内部错误返回 35600050（system error）
- 权限：`ohos.permission.REGISTER_AGENT_HOOK`
- 错误码：
  - register：201（参数错误）、202（非系统应用）、401（参数错误）、35700021（非开发模式）、35700022（已注册）、**35600050（系统内部错误）**
  - unregister：201（权限拒绝）、202（非系统应用）、35700023（无 Hook 注册）、**35600050（系统内部错误）**

**Steps**

- [ ] cliManager.d.ts registerCliHook/unregisterCliHook/registerFunctionHook/unregisterFunctionHook 新增 @permission + @throws
- [ ] register 类函数 @throws 含 35600050（system error）
- [ ] unregister 类函数 @throws 含 35600050（system error）；修正错误码为 35700007/35700008（与 spec 一致，不再用 201/202）
- [ ] zh-cn 镜像同步
- [ ] SDK 构建验证

---

### TASK-AH-021: v3.0 — NAPI `CreateCliJsErrorByNativeErr` 错误码映射（default→35600050）

| 字段 | 内容 |
|------|------|
| 任务目标 | NAPI 层 `CreateCliJsErrorByNativeErr` 新增 default 分支返回 35600050（system error）；JS 侧不暴露 `ERR_INVALID_PARAM(35700002)` |
| AC 映射 | AC-4.11 |
| 前置依赖 | TASK-AH-019 |
| 完成判据 | `CreateCliJsErrorByNativeErr` 未匹配到特定业务错误码的原生错误统一返回 35600050；单测验证 ERR_INVALID_PARAM→35600050 |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | `CreateCliJsErrorByNativeErr` 新增 default→35600050 |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_manager.cpp` | 同上（如果 function_manager 有独立错误码映射函数） |

**Design Context**

- ADR-13/CHG-10: JS 侧不暴露 ERR_INVALID_PARAM(35700002)；系统内部错误返回 35600050
- 映射表：
  - `ERR_PERMISSION_DENIED(35700007)` → 201
  - `ERR_NOT_SYSTEM_APP(35700008)` → 202
  - `ERR_NOT_DEVELOPER_MODE(35700021)` → 35700021
  - `ERR_HOOK_ALREADY_REGISTERED(35700022)` → 35700022
  - `ERR_HOOK_NOT_REGISTERED(35700023)` → 35700023
  - 其他（含 ERR_INVALID_PARAM、IPC 失败、service 未启动等） → 35600050
- NAPI 参数校验失败（argc 不足、非对象、缺方法）直接返回 201，不经过 `CreateCliJsErrorByNativeErr`

**Steps**

- [ ] `CreateCliJsErrorByNativeErr` 新增 default 分支：`default: return CreateJsErrorByErrCode(env, 35600050, "system error");`
- [ ] 确认既有映射表中无 ERR_INVALID_PARAM(35700002) 泄漏到 JS 侧
- [ ] 单测：service 返回 ERR_INVALID_PARAM → JS 侧收到 35600050
- [ ] 单测：IPC 失败 → JS 侧收到 35600050
- [ ] 单测：参数校验失败 → JS 侧收到 201（不走 CreateCliJsErrorByNativeErr）
- [ ] 运行 `run -t UT -ts cli_tool_manager_napi_test`

---

### TASK-AH-022: v4.0 — IDL + service 位掩码方法可选 + NAPI 构建掩码 + .d.ts 方法可选

| 字段 | 内容 |
|------|------|
| 任务目标 | IDL RegisterCliHook/RegisterFunctionHook 新增 `[in] int activeMethods`；service 存储 activeMethods_ 并按掩码跳过未激活方法；NAPI 根据方法存在性构建掩码；JS 校验改为"至少 1 个方法"；.d.ts 方法加 `?` |
| AC 映射 | AC-4.12 |
| 前置依赖 | TASK-AH-017/019 |
| 完成判据 | 位掩码跳过生效；NAPI 构建→service 跳过；.d.ts 方法可选 |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `interfaces/cli_tool/ICliToolManager.idl` | RegisterCliHook/RegisterFunctionHook 新增 `[in] int activeMethods` |
| Modify | `services/climgr/include/cli_tool_manager_service.h` | 方法签名加 `int32_t activeMethods` override；新增 `uint32_t cliHookActiveMethods_` + `functionHookActiveMethods_` + 6 个位掩码常量 |
| Modify | `services/climgr/src/cli_tool_manager_service.cpp` | Register 时保存 activeMethods_；Invoke* 方法检查掩码跳过；Unregister/DeathRecipient 清零 |
| Modify | `interfaces/cli_tool/include/cli_tool_mgr_client.h` | RegisterCliHook/RegisterFunctionHook 签名加 `int32_t activeMethods = 0` |
| Modify | `interfaces/cli_tool/src/cli_tool_mgr_client.cpp` | 透传 activeMethods 到 proxy |
| Modify | `frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | `ValidateCliHookObject` 改为"至少 1 个方法"；构建位掩码传入 service |
| Modify | `frameworks/js/napi/function_manager/src/js_function_manager.cpp` | `ValidateFunctionHookObject` 改为"至少 1 个方法"；构建位掩码 |
| Modify | `frameworks/js/napi/cli_tool_manager/src/js_cli_hook.cpp` | CallJs 中 `napi_get_named_property` 后 `napi_typeof` 检查——非 function 则直接 set_value 原值 |
| Modify | `frameworks/js/napi/function_manager/src/js_function_hook.cpp` | 同上 |
| Modify | `interface/sdk-js/api/application/CliHook.d.ts` | 方法加 `?` |
| Modify | `interface/sdk-js/api/application/FunctionHook.d.ts` | 方法加 `?` |

**Design Context**

- ADR-14: 位掩码方案。bit0=BeforeCallTool, bit1=AfterCallTool, bit2=BeforeCallCmd, bit3=AfterCallCmd（FunctionHook: bit0=BeforeInvokeFunction, bit1=AfterInvokeFunction）
- 0=全部有效（默认）；NAPI 根据 JS 对象哪些方法存在构建掩码
- service 在 Invoke* 方法中：`if (activeMethods_ != 0 && !(activeMethods_ & BIT_X)) return;`
- NAPI CallJs：若 JS 方法不存在（napi_typeof != napi_function），直接 promise.set_value 原值返回，不走 threadsafe_function 分发

**Steps**

- [x] ICliToolManager.idl RegisterCliHook/RegisterFunctionHook 新增 `[in] int activeMethods`
- [x] cli_tool_manager_service.h 方法签名加 `int32_t activeMethods` override；新增 `cliHookActiveMethods_`/`functionHookActiveMethods_` + 6 个位掩码常量（CLI_HOOK_BEFORE_CALL_TOOL 等）
- [x] cli_tool_manager_service.cpp Register 时保存 `cliHookActiveMethods_ = static_cast<uint32_t>(activeMethods)`
- [x] cli_tool_manager_service.cpp InvokeBeforeCallTool/InvokeAfterCallTool/InvokeBeforeCallCmd: 检查 `if (cliHookActiveMethods_ != 0 && !(cliHookActiveMethods_ & BIT)) return;`
- [x] cli_tool_manager_service.cpp BeforeInvokeFunction/AfterInvokeFunction: 检查 `functionHookActiveMethods_` 掩码
- [x] cli_tool_manager_service.cpp Unregister/DeathRecipient 清零 `cliHookActiveMethods_ = 0` / `functionHookActiveMethods_ = 0`
- [x] cli_tool_mgr_client.h/.cpp RegisterCliHook/RegisterFunctionHook 签名加 `int32_t activeMethods = 0`，透传到 proxy
- [x] js_cli_manager.cpp `ValidateCliHookObject` 改为返回 `uint32_t& activeMethods`，至少 1 个方法；`OnRegisterCliHook` 传掩码
- [x] js_function_manager.cpp `ValidateFunctionHookObject` 改为返回 `uint32_t& activeMethods`，至少 1 个方法；`OnRegisterFunctionHook` 传掩码
- [x] js_cli_hook.cpp CallJs 增加 `napi_typeof` 检查——非 function 则 `set_value` 原值返回
- [x] js_function_hook.cpp CallJs 同上
- [x] CliHook.d.ts / FunctionHook.d.ts 方法加 `?`
- [x] 编译通过（`./build.sh --product-name rk3568 --build-target climgr climanager_napi functionmanager_napi`）
- [ ] 单测 + 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-023: v4.0 — Hook 调用异步化（FFRT + cv + 6s 超时）

| 字段 | 内容 |
|------|------|
| 任务目标 | 5 个 Invoke* 方法改为 FFRT 任务包裹同步 IPC + condition_variable::wait_for(6s) + 超时用原始数据；NAPI HOOK_TIMEOUT_MS 5000ms |
| AC 映射 | AC-4.13 |
| 前置依赖 | TASK-AH-022 |
| 完成判据 | hook 调用 6s 超时后用原始数据继续；FFRT task 后续完成安全丢弃 |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `services/climgr/include/cli_tool_manager_service.h` | 新增 `#include <chrono>`；`InvokeHookAsync` 模板完整定义（shared_ptr + ffrt::submit + cv->wait_for）+ `HOOK_TIMEOUT_SECONDS = 6` |
| Modify | `services/climgr/src/cli_tool_manager_service.cpp` | 5 个 Invoke* 方法改为调用 `InvokeHookAsync` + 超时日志 |
| Modify | `frameworks/js/napi/cli_tool_manager/src/js_cli_hook.cpp` | `HOOK_TIMEOUT_MS = 500` → `5000` |
| Modify | `frameworks/js/napi/function_manager/src/js_function_hook.cpp` | 同上 |

**Design Context**

- ADR-15: FFRT 包裹方案。`InvokeHookAsync` 模板定义在 header 中：
  1. `shared_ptr<ParamType> paramCopy = make_shared(param)` 拷贝参数
  2. `shared_ptr<ffrt::mutex>` + `shared_ptr<ffrt::condition_variable>` + `shared_ptr<atomic<bool>>` 共享同步状态
  3. `ffrt::submit` 任务内执行 `hookFunc(*paramCopy)` → lock + `completed->store(true)` + `cv->notify_one()`
  4. service 线程 `cv->wait_for(chrono::seconds(6), pred)` → 若 ok: `param = *paramCopy`；若 timeout: 用原始 param
- `shared_ptr` 保证 FFRT task 后续完成时修改 paramCopy 和状态对象不影响已继续的 service 线程
- NAPI HOOK_TIMEOUT_MS 统一为 5000ms（内层先触发，与 service 外层 5s 一致）
- IPC 默认超时 8s（TF_WAIT_TIME=8），FFRT worker 最坏阻塞 8s 后释放

**Steps**

- [x] cli_tool_manager_service.h 新增 `#include <chrono>` + `HOOK_TIMEOUT_SECONDS = 6` + `InvokeHookAsync` 模板完整定义
- [x] cli_tool_manager_service.cpp InvokeBeforeCallTool 改为 `InvokeHookAsync([hook, option](ExecToolParam &p) { hook->BeforeCallTool(p, option); }, param)`
- [x] 同理改 InvokeBeforeCallCmd / InvokeAfterCallTool（含 AfterCallTool/AfterCallCmd 按 toolName 分流）/ BeforeInvokeFunction / AfterInvokeFunction
- [x] js_cli_hook.cpp `HOOK_TIMEOUT_MS = 5000`
- [x] js_function_hook.cpp `HOOK_TIMEOUT_MS = 5000`
- [x] 编译通过（`./build.sh --product-name rk3568 --build-target climgr climanager_napi functionmanager_napi`）
- [ ] 单测：hook 6s 超时→原始数据继续；FFRT task 完成后安全丢弃
- [ ] 运行 `run -t UT -ts cli_tool_mgr_service_test`

---

### TASK-AH-024: ~~v4.0 — HookOption JS 字段可选 + NAPI 条件设置~~ **已移除（v5.0 CHG-16）**

> **此 Task 已废弃。** HookOption 完全移除（v5.0 CHG-16）。无 dmSessionId/toolCallId 字段，无 NAPI 条件设置逻辑。

---

## Plan 自审清单

- [x] 每个 P0/P1 AC 至少映射到一个 Task
- [x] 每个 Task 文件范围明确
- [x] 每个 Task 明确前置依赖、非目标、完成判据和停止条件
- [x] 每个 Task 有验证命令
- [x] Task 粒度形成能力闭环
- [x] 没有 TBD/TODO/占位符
- [x] 没有要求 Agent 自行寻找未列出的上下文
- [x] 交接信息自包含（Handoff Summary 在各 Task 的 Spec/Design Context 中）
- [x] 每个 Task 验证在完成时立即执行
- [x] 无超 3000 行阈值的 Task
- [x] v2.0 变更已融入：dev mode 门控（ADR-8）、失败式注册（ADR-4）、service 直接分发（ADR-5）、NAPI 层（ADR-9）
- [x] v3.0 变更已融入：每个 hook 独立 DeathRecipient（ADR-7 修订）、系统应用 + 权限校验（ADR-10）、HookOption 参数（ADR-11）、NAPI Stub 缓存 + JS 参数校验（ADR-12）、.d.ts @permission + @throws 标注（CHG-9）、JS 侧系统内部错误返回 35600050（ADR-13/CHG-10）
- [x] v4.0 变更已融入：Hook 方法可选位掩码（ADR-14/CHG-11）、Hook 调用异步化 + 6s 超时（ADR-15/CHG-12）、HookOption JS 字段可选（CHG-13，v5.0 已移除）
- [x] v5.0 变更已融入：移除 @throws 401（CHG-14）、unregister 传 hook 对象 + napi_strict_equals 校验（CHG-15）、HookOption 参数完全移除（CHG-16）、方法名改 on 前缀（CHG-17）、FunctionHook 参数封装 + 简化（CHG-18）、Hook 参数结构体对齐原始函数签名（CHG-19）
- [x] v6.0 变更已融入：C++ 接口重命名 ICliHookScheduler→ICliHookInterface / IFunctionHookScheduler→IFunctionHookInterface（CHG-20/21）、AfterCall* 参数封装为 ExecResultWrap（CHG-22）、BeforeInvokeFunction 参数封装为 InvokeFunctionParam（CHG-23）、AfterInvokeFunction 参数封装为 FunctionResultWrap + 去掉 namespace/functionName（CHG-24）、NAPI Stub 类重命名 JsCliHookSchedulerStub→JsCliHook / JsFunctionHookSchedulerStub→JsFunctionHook（CHG-25/26）
