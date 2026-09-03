# 执行计划

> 将 Approved Spec/Design 拆成可独立执行、可验证、可审查的 Task。v2.0：门控改为运行时 dev mode 判断（ADR-8），注册改为失败式（ADR-4），functionHook 改为 service 直接分发（ADR-5），新增 NAPI 层（ADR-9）。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-AGENT-HOOK |
| 关联 Feature | FEAT-AGENT-HOOK |
| 关联文档 | proposal.md / design.md / spec.md（v2.0） |
| 复杂度 | 标准 |
| 状态 | Draft (v2.0) |
| Owner | 待确认 |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | 通过（v2.0 变更，待用户确认） |
| Design | `design.md` | 通过（v2.0） |
| Spec | `spec.md` | Draft (v2.0) |

## Phase 声明（v2.0 统一）

> **v2.0 统一实现**：不再分 Phase 1/Phase 2。门控用 `system::GetBoolParameter("const.security.developermode.state", false)` 运行时判断（ADR-8），非开发模式注册返回 ERR_INVALID_VALUE + 回调跳过。注册语义为失败式（ADR-4）。functionHook 改为 service 直接分发（ADR-5）。新增 NAPI 层（ADR-9）。

## 受影响文件全量清单

| 仓 | 层 | 文件路径 | 修改类型 | 说明 |
|----|----|----------|----------|------|
| ability_ability_runtime | IPC 契约层 | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` | 修改 | 新增 6 方法，移除 GetFunctionHook |
| ability_ability_runtime | IPC 回调契约层 | `cli_tool_framework/interfaces/cli_tool/ICliHookScheduler.idl` | 新增 | 4 方法 |
| ability_ability_runtime | IPC 回调契约层 | `cli_tool_framework/interfaces/cli_tool/IFunctionHookScheduler.idl` | 新增 | 2 方法 |
| ability_ability_runtime | IPC 契约层 | `cli_tool_framework/interfaces/cli_tool/BUILD.gn` | 修改 | 新增 2 个 .idl |
| ability_ability_runtime | 客户端层 | `cli_tool_framework/interfaces/cli_tool/include/cli_tool_mgr_client.h` | 修改 | 新增 6 方法声明 |
| ability_ability_runtime | 客户端层 | `cli_tool_framework/interfaces/cli_tool/src/cli_tool_mgr_client.cpp` | 修改 | 新增 6 方法代理 |
| ability_ability_runtime | 服务层 | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 修改 | 新增成员 + dev mode + 6 方法声明 |
| ability_ability_runtime | 服务层 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 修改 | 6 方法实现 + 回调点 + dev mode + 失败式注册 |
| ability_ability_runtime | 函数执行器层 | `cli_tool_framework/frameworks/js/napi/function_manager/include/invoke_function_executor.h` | 修改 | 调 BeforeInvokeFunction/AfterInvokeFunction |
| ability_ability_runtime | 函数执行器层 | `cli_tool_framework/frameworks/js/napi/function_manager/src/invoke_function_executor.cpp` | 修改 | 调 CliToolMGRClient::BeforeInvokeFunction/AfterInvokeFunction |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/js_cli_hook_scheduler_stub.h` | 新增 | JsCliHookSchedulerStub（IPC Stub + per-object threadsafe_function） |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_hook_scheduler_stub.cpp` | 新增 | IPC Stub 实现 + 同步分发 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_manager.cpp` | 修改 | 新增 registerCliHook/unregisterCliHook 绑定 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/BUILD.gn` | 修改 | 新增 js_cli_hook_scheduler_stub.cpp + idl_gen_ICliHookScheduler 依赖 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/function_manager/include/js_function_hook_scheduler_stub.h` | 新增 | JsFunctionHookSchedulerStub |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_hook_scheduler_stub.cpp` | 新增 | IPC Stub 实现 + 同步分发 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_manager.cpp` | 修改 | 新增 registerFunctionHook/unregisterFunctionHook 绑定 |
| ability_ability_runtime | NAPI 绑定层 | `cli_tool_framework/frameworks/js/napi/function_manager/BUILD.gn` | 修改 | 新增 js_function_hook_scheduler_stub.cpp + idl_gen_IFunctionHookScheduler 依赖 + cli_tool_mgr_client 依赖 |
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
| TASK-AH-014 | NAPI Stub：JsCliHookSchedulerStub（cli_tool_manager） + JsFunctionHookSchedulerStub（function_manager） | frameworks/js/napi/cli_tool_manager/ + function_manager/ | AC-4.2/4.4 | TASK-AH-010/013 | IPC Stub 封装 JS 对象 + per-object threadsafe_function 同步分发 | `run -t UT -ts cli_tool_manager_napi_test` |
| TASK-AH-015 | NAPI JS 函数：registerCliHook/unregisterCliHook（cli_tool_manager） + registerFunctionHook/unregisterFunctionHook（function_manager），返回 Promise<void> | frameworks/js/napi/cli_tool_manager/ + function_manager/ | AC-4.1/4.3/4.6 | TASK-AH-014 | JS 调用注册成功 | `run -t UT -ts cli_tool_manager_napi_test` |
| TASK-AH-016 | NAPI 同步回调验证（binder→JS 线程→返回 + 超时兜底 500ms） | frameworks/js/napi/cli_tool_manager/ + function_manager/ | AC-4.2/4.4/4.5 | TASK-AH-015 | JS 回调到 JS 线程执行 + 改写生效 + 超时返回原值 | `run -t UT -ts cli_tool_manager_napi_test` |

## Task 详情

### TASK-AH-001: IDL + Stub/Proxy + InvokeFunctionResult Parcelable 化

| 字段 | 内容 |
|------|------|
| 任务目标 | 新增 ICliHookScheduler.idl、IFunctionHookScheduler.idl；ICliToolManager.idl 新增 6 方法（移除 GetFunctionHook，新增 BeforeInvokeFunction/AfterInvokeFunction）；InvokeFunctionResult 改为 Parcelable + IDL sequenceable 声明；更新 BUILD.gn |
| AC 映射 | AC-1.1（部分：接口骨架） |
| 前置依赖 | 无 |
| 非目标 | 不实现业务逻辑 |
| 完成判据 | IDL 编译通过，Stub/Proxy 生成含新方法；InvokeFunctionResult Parcelable Marshalling/Unmarshalling 编译通过 |
| 停止条件 | IDL 语法错误无法生成 Stub/Proxy |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | `cli_tool_framework/interfaces/cli_tool/ICliHookScheduler.idl` | 4 方法：BeforeCallTool/AfterCallTool/BeforeCallCmd/AfterCallCmd |
| Create | `cli_tool_framework/interfaces/cli_tool/IFunctionHookScheduler.idl` | 2 方法：BeforeInvokeFunction/AfterInvokeFunction |
| Modify | `cli_tool_framework/interfaces/cli_tool/ICliToolManager.idl` | 新增 import + 6 方法（RegisterCliHook/UnregisterCliHook/RegisterFunctionHook/UnregisterFunctionHook/BeforeInvokeFunction/AfterInvokeFunction）；移除 GetFunctionHook |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/include/invoke_function_executor.h` | InvokeFunctionResult 改为继承 Parcelable + Marshalling/Unmarshalling |
| Modify | `cli_tool_framework/interfaces/cli_tool/BUILD.gn` | idl_gen_interface 新增 2 个 .idl + sequenceable InvokeFunctionResult |

**Design Context**

- ADR-1：独立 IPC 接口（ICliHookScheduler/IFunctionHookScheduler）
- ADR-2：异名（BeforeCallTool/AfterCallTool/BeforeCallCmd/AfterCallCmd）
- ADR-3：AfterCallTool/AfterCallCmd 入参为 ExecResult
- ADR-5：移除 GetFunctionHook，新增 BeforeInvokeFunction/AfterInvokeFunction
- InvokeFunctionResult Parcelable 化：当前为 `invoke_function_executor.h:36` 普通 struct，需改为继承 Parcelable

**Steps**

- [ ] 创建 ICliHookScheduler.idl（package OHOS.CliTool；sequenceable ExecToolParam/ExecCmdParam/ExecResult/CliSessionInfo；4 方法）
- [ ] 创建 IFunctionHookScheduler.idl（sequenceable InvokeFunctionResult/WantParams；2 方法）
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
| 完成判据 | dev mode + 未注册时保存 Hook；非 dev mode 或已注册返回 ERR_INVALID_VALUE |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 新增 hookMutex_、cliHook_ 成员；IsDeveloperMode() 声明；RegisterCliHook 声明 |
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | RegisterCliHook 实现 + IsDeveloperMode() |

**Spec Context**

- AC-1.1: WHEN 开发模式 + 未注册 THEN 保存 Hook，返回 ERR_OK
- AC-1.8: WHEN 非开发模式 THEN 返回 ERR_INVALID_VALUE
- AC-1.9: WHEN 已注册 THEN 返回 ERR_INVALID_VALUE（失败式）

**Design Context**

- ADR-4: 失败式注册——已注册返回 ERR_INVALID_VALUE，不覆盖
- ADR-8: `system::GetBoolParameter("const.security.developermode.state", false)` 运行时判断；`#include "parameters.h"`
- BUILD 依赖 `"init:libbegetutil"`（climgr BUILD.gn 已有）

**Steps**

- [ ] cli_tool_manager_service.h 新增成员：`ffrt::mutex hookMutex_; sptr<ICliHookScheduler> cliHook_;`
- [ ] 新增 `bool IsDeveloperMode() const;` — 返回 `system::GetBoolParameter("const.security.developermode.state", false)`
- [ ] 实现 RegisterCliHook：锁 hookMutex_，检查 IsDeveloperMode() + cliHook_ 是否已存在，条件满足则保存，否则返回 ERR_INVALID_VALUE
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
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | UnregisterCliHook + AddDeathRecipient + OnRemoteDied |

**Design Context**

- ADR-7: 单 DeathRecipient（同进程 cliHook_+functionHook_ 共用）
- 既有模式参考：`event_dispatcher.h:81`

**Steps**

- [ ] 实现 HookDeathRecipient（继承 IRemoteObject::DeathRecipient）
- [ ] RegisterCliHook 中 AddDeathRecipient
- [ ] 实现 UnregisterCliHook：锁 hookMutex_，清空 cliHook_，返回 ERR_OK
- [ ] OnRemoteDied：锁 hookMutex_，清空 cliHook_（functionHook_ 在 TASK-AH-009 追加）
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

- [ ] 新增成员：`sptr<IFunctionHookScheduler> functionHook_;`（共用 hookMutex_）
- [ ] 实现 RegisterFunctionHook：IsDeveloperMode() + 未注册 → 保存；否则 ERR_INVALID_VALUE
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
- 回调为同步 IPC（TF_SYNC），不设超时

**Steps**

- [ ] 实现 BeforeInvokeFunction(ns, name, param)：IsDeveloperMode() + functionHook_ 非空 → functionHook_->BeforeInvokeFunction(ns, name, param)；否则直接返回 ERR_OK
- [ ] 实现 AfterInvokeFunction(ns, name, result)：同上逻辑
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

- [ ] DoExecute 中 GetFunctionInfo 之后插入：`CliToolMGRClient::GetInstance().BeforeInvokeFunction(ns, name, wantParams)`
- [ ] 结果回调到达后插入：`CliToolMGRClient::GetInstance().AfterInvokeFunction(ns, name, result)`
- [ ] 移除旧 GetFunctionHook 相关代码
- [ ] 单测：mock CliToolMGRClient，验证 Before/After 调用 + 改写生效
- [ ] 运行 `run -t UT -ts invoke_function_executor_test`

---

### TASK-AH-009: DeathRecipient 自动注销 functionHook

| 字段 | 内容 |
|------|------|
| 任务目标 | OnRemoteDied 中追加清除 functionHook_（复用 TASK-AH-005 的 HookDeathRecipient） |
| AC 映射 | AC-2.9 |
| 前置依赖 | TASK-AH-005/006 |
| 完成判据 | 进程死亡后 functionHook_ 清空，Before/After 返回 ERR_OK |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | OnRemoteDied 追加清空 functionHook_ |

**Steps**

- [ ] OnRemoteDied 中追加：锁 hookMutex_，清空 functionHook_
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
| 完成判据 | 非 dev mode 注册返回 ERR_INVALID_VALUE + 回调跳过 |
| 停止条件 | — |

**Steps**

- [ ] 单测：mock GetBoolParameter 返回 false，RegisterCliHook/RegisterFunctionHook 返回 ERR_INVALID_VALUE
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

### TASK-AH-014: NAPI Stub — JsCliHookSchedulerStub（cli_tool_manager） + JsFunctionHookSchedulerStub（function_manager）

| 字段 | 内容 |
|------|------|
| 任务目标 | 在 cli_tool_manager 模块新增 JsCliHookSchedulerStub；在 function_manager 模块新增 JsFunctionHookSchedulerStub。IPC Stub 封装 JS 对象 + per-object threadsafe_function 同步分发 |
| AC 映射 | AC-4.2/4.4 |
| 前置依赖 | TASK-AH-010/013 |
| 完成判据 | IPC Stub 封装 JS 对象成功 + threadsafe_function 同步分发到 JS 线程 |
| 停止条件 | — |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/include/js_cli_hook_scheduler_stub.h` | JsCliHookSchedulerStub 声明 |
| Create | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/src/js_cli_hook_scheduler_stub.cpp` | JsCliHookSchedulerStub 实现 |
| Create | `cli_tool_framework/frameworks/js/napi/function_manager/include/js_function_hook_scheduler_stub.h` | JsFunctionHookSchedulerStub 声明 |
| Create | `cli_tool_framework/frameworks/js/napi/function_manager/src/js_function_hook_scheduler_stub.cpp` | JsFunctionHookSchedulerStub 实现 |
| Modify | `cli_tool_framework/frameworks/js/napi/cli_tool_manager/BUILD.gn` | 新增 stub.cpp + idl_gen_ICliHookScheduler |
| Modify | `cli_tool_framework/frameworks/js/napi/function_manager/BUILD.gn` | 新增 stub.cpp + idl_gen_IFunctionHookScheduler |

**Design Context**

- ADR-9: per-object 1 个 napi_threadsafe_function（非 per-method）
- ADR-9: JS callback 返回修改后对象，NAPI 从 napi_call_function 返回值解析
- 同步分发：binder 线程 → std::promise → napi_call_threadsafe_function(blocking) → future.wait_for(500ms) → JS 线程 CallJs → promise.set_value
- 超时兜底：500ms，超时返回原 param

**Steps**

- [ ] 实现 JsCliHookSchedulerStub（继承 IRemoteStub<ICliHookScheduler>）：napi_ref + 1 个 tsfn_ + DispatchSync 模板
- [ ] CallJs 回调：从 CallData 取 method 名 + param → 转 JS 对象 → napi_call_function(jsObj, method, ...) → 从返回值解析 → promise.set_value
- [ ] 实现 JsFunctionHookSchedulerStub（继承 IRemoteStub<IFunctionHookScheduler>）：同上结构
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
- 创建 JsCliHookSchedulerStub/JsFunctionHookSchedulerStub 封装 JS 对象 → 调 CliToolMGRClient 注册

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
| 任务目标 | 验证 binder 线程 → JS 线程同步回调 + 改写生效 + 超时兜底 500ms |
| AC 映射 | AC-4.2/4.4/4.5 |
| 前置依赖 | TASK-AH-015 |
| 完成判据 | JS 回调到 JS 线程执行 + 改写后数据通过 IPC 返回 + 超时返回原值 |
| 停止条件 | — |

**Steps**

- [ ] 单测：注册 CliHook → 触发 ExecTool → 验证 beforeCallTool 在 JS 线程执行 + param 改写生效
- [ ] 单测：注册 FunctionHook → 触发 invokeFunction → 验证 beforeInvokeFunction/afterInvokeFunction 在 JS 线程执行 + 改写生效
- [ ] 单测：JS 回调超时（>500ms）→ 验证返回原 param（未改写）
- [ ] 单测：JS 回调抛异常 → 验证返回原 param
- [ ] 单测：JS 调用 unregisterCliHook → 验证后续不触发回调
- [ ] 运行 `run -t UT -ts cli_tool_manager_napi_test`



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
