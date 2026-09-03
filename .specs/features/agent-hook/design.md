# 架构设计

> 确认目标仓和模块的架构约束、关键设计决策、Spec 拆分方向。本设计承接 `proposal.md` 基线与 `spec.md` 验收标准，实现 `agenthook_design.txt` 粗方案。

## 设计元数据

| 字段 | 内容 |
|------|------|
| Design ID | DESIGN-AGENT-HOOK |
| 关联需求 | `.specs/features/agent-hook/proposal.md` |
| 关联 Epic | 无 |
| 目标 Feature | FEAT-AGENT-HOOK |
| 复杂度 | 标准 |
| 目标版本 | OpenHarmony-6.0-Release |
| Owner | 待确认 |
| 状态 | Draft (v2.0 — 基线变更后重新设计) |

## 需求基线

> 需求基线详见 proposal.md。以下仅列出设计阶段需要额外强调的要点。

| 项 | 补充说明 |
|----|----------|
| cliHook 在 service 进程内触发 | ExecTool/ExecCmd 执行链路在 CliToolManagerService 内，Hook 回调由 service 持 Proxy 发起 IPC 到注册方进程 |
| functionHook 在 service 进程内分发 | InvokeFunctionExecutor 调用 `CliToolMGRClient::BeforeInvokeFunction`/`AfterInvokeFunction`（IPC 到 service），service 内部分发到已注册 functionHook（再 IPC 到注册方进程） |
| v1 单 Hook 语义（失败式） | 同类 Hook 仅保留一个；已注册时重复注册返回 `ERR_INVALID_VALUE`，必须先 Unregister 再 Register |
| Hook 回调为同步 IPC（不设超时） | 阻塞主链路直到注册方返回；由注册方保证回调快速返回 |
| 仅开发模式生效 | `system::GetBoolParameter("const.security.developermode.state", false)` 运行时判断；非开发模式注册返回失败、回调点跳过 |
| NAPI 层提供 JS 系统接口 | JS 调用方注册 JS 对象（实现 CliHook/FunctionHook interface），NAPI 层封装为 IPC Stub（RemoteObject），回调时同步到 JS 线程执行 |

## 上下文和现状

### 涉及仓和模块

| 仓库 | 补充架构说明 |
|------|-------------|
| ability_ability_runtime | cli_tool_framework 子系统；既有四层分层（客户端 CliToolMGRClient ↔ IPC ICliToolManager ↔ 服务 CliToolManagerService ↔ 权限 PermissionUtil）；本设计在既有分层上插入 Hook 回调链路，不新增层 |

### 调用链层级分析

| 层 | 模块 | 职责 | 修改类型 |
|----|------|------|----------|
| IPC 契约层 | `interfaces/cli_tool/ICliToolManager.idl` | 跨进程接口契约 | 修改（新增 6 方法：RegisterCliHook/UnregisterCliHook/RegisterFunctionHook/UnregisterFunctionHook/BeforeInvokeFunction/AfterInvokeFunction；移除 GetFunctionHook） |
| IPC 回调契约层 | `interfaces/cli_tool/ICliHookScheduler.idl`（新增）、`IFunctionHookScheduler.idl`（新增） | Hook 回调跨进程契约 | 新增 |
| 客户端层 | `interfaces/cli_tool/include/cli_tool_mgr_client.h/.cpp` | 单例客户端，供应用调用 | 修改（新增 6 方法代理） |
| 服务层 | `services/climgr/cli_tool_manager_service.h/.cpp` | SA 实现，ExecTool/ExecCmd/BeforeInvokeFunction/AfterInvokeFunction 执行链路 | 修改（新增 6 方法实现 + 回调点 + dev mode 门控 + 失败式注册） |
| 函数执行器层 | `frameworks/js/napi/function_manager/invoke_function_executor.h/.cpp` | invokeFunction 执行器（调用方进程） | 修改（调用 CliToolMGRClient::BeforeInvokeFunction/AfterInvokeFunction 替代 GetFunctionHook 查询） |
| NAPI 绑定层 | `frameworks/js/napi/cli_tool_manager/`（修改） | JS CliHook callback → IPC Stub 封装 + JS 线程同步回调 + registerCliHook/unregisterCliHook | 修改（新增 `js_cli_hook_scheduler_stub.h/.cpp` + `js_cli_manager.cpp` 新增绑定） |
| NAPI 绑定层 | `frameworks/js/napi/function_manager/`（修改） | JS FunctionHook callback → IPC Stub 封装 + JS 线程同步回调 + registerFunctionHook/unregisterFunctionHook | 修改（新增 `js_function_hook_scheduler_stub.h/.cpp` + `js_function_manager.cpp` 新增绑定） |
| SDK JS 层 | `interface/sdk-js/api/application/CliHook.d.ts`（新增）、`FunctionHook.d.ts`（新增） | JS 回调 interface 类型声明 | 新增 |
| SDK JS 层 | `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts`（修改） | JS 注册函数声明 | 修改（新增 4 注册函数） |
| SDK JS 层 | `interface/sdk-js/kits/@kit.AbilityKit.d.ts` | Kit 聚合器 | 修改（导出新类型） |

**检查项：**
- [x] 调用链每一层都已覆盖（从 IPC 契约到服务到函数执行器到权限）
- [x] 每层职责边界清晰，无跨层违规调用（Hook 回调走独立 IPC 接口，不绕过既有分层）
- [x] 每层修改类型明确

### 适用架构规则

| Rule ID | 适用原因 | 设计结论 | 验证方式 |
|---------|----------|----------|----------|
| OH-ARCH-LAYERING | 涉及客户端→IPC→服务→权限调用链 | Hook 注册走客户端→IPC→服务→权限，与既有 ExecTool 等一致；Hook 回调走服务→IPC→注册方，不跨层 | 架构评审/依赖检查 |
| OH-ARCH-IPC-SAF | 涉及跨进程 IPC（Hook 注册 + Hook 回调 + GetFunctionHook 查询） | 新增 2 个回调 IPC 接口（ICliHookScheduler/IFunctionHookScheduler），复用既有 SA 注册模式 | 集成测试 |
| OH-ARCH-API-LEVEL | 涉及新增 SystemAPI | 5 个新方法 + 2 个回调接口，标注 `@since 6.0`，开放范围 SystemAPI | API 评审/XTS |
| OH-ARCH-COMPONENT-BUILD | 涉及新增 IDL 文件 | 新增 2 个 .idl 文件，更新 BUILD.gn 的 idl_gen_interface 依赖 | 构建验证 |
| OH-ARCH-ERROR-LOG | 涉及新增错误码 + HiSysEvent | 新增错误码 16001000~16001003；Hook 超时/触发/注销记录 HiSysEvent | 单测/hilog |

## 不涉及项承接

> proposal.md 已完成 N/A 判定。本节仅对 proposal 中标记为"涉及"且需展开设计的维度给出结论。

| 维度 | 设计结论 |
|------|----------|
| 性能 | Hook 回调为同步 IPC（阻塞主链路，由 Hook 注册方保证回调快速返回）；`BeforeInvokeFunction`/`AfterInvokeFunction` 为调用方→service→注册方两跳 IPC，P95 目标 <50ms；无 Hook 时 service 侧直接返回 ERR_OK（零开销，调用方无感知）；**非开发模式跳过所有 Hook 逻辑，零开销** |
| 安全与权限 | 见下方「安全基础检查」章节；门控用 `system::GetBoolParameter("const.security.developermode.state", false)` 运行时判断，仅开发模式生效（ADR-8） |
| IPC/跨进程 | 见下方「时序设计」与「线程与并发模型」 |
| 构建与部件 | 无新增特性开关（ADR-8 已移除宏隔离方案）；NAPI 层扩展 `cli_tool_manager` + `function_manager` 两个现有模块；SDK JS 新增 2 个 .d.ts + 修改 1 个 .d.ts |

## 关键设计决策

| 决策 ID | 问题 | 推荐方案 | 探索过的替代方案 | 取舍理由 | 影响 |
|---------|------|----------|-----------------|------|------|
| ADR-1 | Hook 回调用独立 IPC 接口还是复用 ICliToolManagerScheduler？ | 独立 IPC 接口（ICliHookScheduler / IFunctionHookScheduler） | 备选1：复用 ICliToolManagerScheduler，通过 event type 区分——放弃，因回调接口语义臃肿且 event 携带 Hook 改写语义复杂 | 与既有 `ICliToolManagerScheduler` 回调模式一致；职责清晰；IDL 可读性好 | 新增 2 个 IDL 文件 |
| ADR-2 | cliHook 方法命名：重载（beforeCallCli 同名按参数类型区分）还是异名（BeforeCallTool/BeforeCallCmd）？ | 异名（BeforeCallTool/AfterCallTool/BeforeCallCmd/AfterCallCmd） | 备选1：IDL 重载——放弃，因 IDL 不支持方法重载，需生成不同名 Stub 方法 | 需求方确认：异名。IDL 规范要求方法名唯一；异名可读性好；与 `agenthook_design.txt` 语义一致（4 个方法按 tool/cmd 区分） | IDL 方法名 4 个 |
| ADR-3 | afterCallCli(toolParam) 中参数名——是改写"工具执行后的最终输出参数"还是改写"执行结果对象"？ | 改写执行结果对象（ExecResult） | 备选1：改写 CliSessionInfo——放弃，因 CliSessionInfo 仅含 sessionId/toolName/status + shared_ptr<ExecResult> 包装，Hook 真正要改的是 outputText/exitCode 等 ExecResult 字段；备选2：改写 ExecToolParam——放弃，执行后改写入参无业务意义 | 需求方确认：原始输入 `afterCallCli(toolParam)` 中 toolParam 为笔误，afterCallCli 入参应为结果。执行后 Hook 的价值是改写结果（如脱敏 outputText），故入参为 ExecResult（`exec_result.h:28`：exitCode/outputText/errorText/signalNumber/timeout/executionTime）；service 回调后用改写后的 ExecResult 填充 CliSessionInfo.result 再回传原调用方 | AfterCallTool/AfterCallCmd 入参为 ExecResult& |
| ADR-4 | v1 单 Hook 还是多 Hook 链？ | 单 Hook（失败语义），最多只存在一个 functionHook | 备选1：多 Hook 链按注册顺序执行——放弃，因 v1 场景单一（Agent 框架独占），多 Hook 引入顺序/优先级复杂度；备选2：单 Hook 覆盖语义——放弃，覆盖语义可能意外覆盖他人的 Hook 导致难以排查 | 需求方 v2.0 确认：单 Hook，已注册时重复注册返回 `ERR_INVALID_VALUE`，必须先 Unregister 再 Register。失败语义比覆盖语义更安全——避免意外覆盖；数据结构从 sptr 扩展为 vector 可向后兼容 | Register 已存在则返回 ERR_INVALID_VALUE |
| ADR-5 | functionHook 调用路径：调用方查询 Hook 还是由 service 直接分发？ | service 直接分发：移除 `GetFunctionHook`，在 ICliToolManager 提供 `BeforeInvokeFunction`/`AfterInvokeFunction`，调用方 IPC 到 service，service 内部分发到 functionHook | 备选1：调用方经 `GetFunctionHook` 查询 Hook Proxy 再本地回调——放弃（v2.0 变更），因调用方需持 Proxy 管理生命周期，查询 IPC 增加开销，且 Proxy 缓存一致性复杂；备选2：service 全代理（service 替调用方执行整个 invokeFunction）——放弃，职责越界 | 需求方 v2.0 确认：直接提供 `BeforeInvokeFunction`/`AfterInvokeFunction`。调用方不需感知 Hook 存在，只需调 service 的 Before/After；service 内部判断有无 functionHook，有则 IPC 到注册方，无则直接返回 ERR_OK（零开销）；调用方不再需要 GetFunctionHook 查询 IPC | InvokeFunctionExecutor 调 CliToolMGRClient::BeforeInvokeFunction/AfterInvokeFunction |
| ADR-6 | Hook 回调返回值语义：非零是否跳过/阻断？ | 全部 hook 模式（非零也 hook） | 备选1：非零跳过——放弃，需求方确认用"全部 hook 模式"；备选2：非零阻断——放弃，Hook 不应导致主链路失败 | 需求方确认：全部 hook 模式——返回值不影响 Hook 应用，修改后的 param/result 始终生效；返回值仅用于日志/审计 | 返回值被忽略，Hook 修改始终生效 |
| ADR-7 | Hook 注册方死亡检测：DeathRecipient 还是定期心跳？ | 单 DeathRecipient（同进程 cliHook_+functionHook_ 共用） | 备选1：cliHook_ 与 functionHook_ 各挂一个 DeathRecipient——放弃，因同一注册方进程通常同时注册两类 Hook，两个 DeathRecipient 重复监听同一进程死亡属冗余；备选2：定期心跳——放弃，因增加无效 IPC 且时延不可控 | 与既有 `event_dispatcher.h:81` 单 caller 单 deathRecipient 模式一致；注册方进程死亡时一次回调清除 cliHook_+functionHook_ 两项；若两不同进程分别注册，各自独立挂 DeathRecipient | 复用 DeathRecipient，单实例覆盖同进程两类 Hook |
| ADR-8 | 本能力如何门控？ | 运行时 `system::GetBoolParameter("const.security.developermode.state", false)` 判断开发模式，仅开发模式生效 | 备选1：`#ifdef ABILITY_RUNTIME_AGENT_HOOK` 宏隔离 + `ability_runtime_agent_hook=false` 特性开关——放弃（v2.0 变更），因宏隔离需修改构建配置且编译时固化，开发模式切换需重新编译；运行时判断更灵活，与既有 `ability_command.cpp:985`、`app_mgr_service.cpp:495` 等开发模式门控模式一致；备选2：永久不加门控——放弃，生产环境有安全风险 | 需求方 v2.0 确认：`system::GetBoolParameter` 运行时判断。Header: `#include "parameters.h"`（`base/startup/init/interfaces/innerkits/include/syspara/parameters.h`）；BUILD 依赖 `"init:libbegetutil"`（climgr BUILD.gn 已有）。门控点：RegisterCliHook/RegisterFunctionHook 入口（非开发模式返回 `ERR_INVALID_VALUE`）；ExecTool/ExecCmd/BeforeInvokeFunction/AfterInvokeFunction 回调点（非开发模式跳过，零开销） | 非开发模式：注册失败、回调跳过 |
| ADR-9 | NAPI 层如何设计？JS 对象如何封装为 IPC Stub？回调如何到 JS 线程？ | NAPI 层封装 JS 对象为 `IRemoteStub<ICliHookScheduler>`/`IRemoteStub<IFunctionHookScheduler>`；回调用 `napi_threadsafe_function` + `std::promise/future` 同步到 JS 线程 | 备选1：用既有 `NapiAsyncTask::Schedule` 异步分发——放弃，因 Hook 回调需同步返回（改写后的 param/result），异步无法阻塞 IPC 线程等待 JS 执行完成；备选2：`uv_async_send` + 条件变量——可用但不如 threadsafe_function 规范；备选3：将 Hook 回调设计为异步——放弃，需求方确认同步 IPC 不设超时 | NAPI 层创建 `JsCliHookSchedulerStub`（继承 `IRemoteStub<ICliHookScheduler>`），持有 `napi_ref`（JS callback 对象引用）+ **per-object 1 个 `napi_threadsafe_function`**（CliHook 对象 1 个、FunctionHook 对象 1 个，共 2 个）。IPC Stub 在 binder 线程收到 `BeforeCallTool` → 创建 `std::promise<ExecToolParam>` → `napi_call_threadsafe_function(tsfn, callData, napi_tsfn_blocking)` 入队 → `future.wait_for(500ms)` 阻塞等待 → JS 线程 CallJs 回调执行 JS `beforeCallTool` → JS **返回修改后对象** → NAPI 从 `napi_call_function` 的返回值解析改写后数据 → `promise.set_value(改写后param)` → binder 线程恢复 → IPC 返回。SDK 目录 `interface/sdk-js/`：新增 `api/application/CliHook.d.ts`、`api/application/FunctionHook.d.ts`；`api/@ohos.app.cli.cliManager.d.ts` 新增 `registerCliHook`/`unregisterCliHook`/`registerFunctionHook`/`unregisterFunctionHook`（返回 `Promise<void>`）。**NAPI 模块组织（ADR-9 确认）**：扩展 `cli_tool_manager` + `function_manager` 两个现有模块（非新建独立模块）——`JsCliHookSchedulerStub` + `registerCliHook`/`unregisterCliHook` 放入 `cli_tool_manager`；`JsFunctionHookSchedulerStub` + `registerFunctionHook`/`unregisterFunctionHook` 放入 `function_manager` | 扩展 `cli_tool_manager` + `function_manager` 两个现有 NAPI 模块 |

## 设计骨架

### 骨架范围

| 骨架项 | 目标 | 不包含 | 验证方式 |
|--------|------|--------|----------|
| API/接口骨架 | ICliHookScheduler/IFunctionHookScheduler IDL + Stub/Proxy 生成 + ICliToolManager 扩展 5 方法 | 完整业务逻辑 | 编译通过 + IDL 生成验证 |
| 模块骨架 | CliToolManagerService 新增成员（cliHook_/functionHook_ sptr + mutex_） | 完整业务逻辑 | 编译通过 |
| 测试骨架 | mock ICliHookScheduler/IFunctionHookScheduler Stub | 全场景 | 最小注册-回调用例通过 |

### 骨架 Spec 拆分

| Task ID | 目标 | 受影响文件 | AC |
|---------|------|------------|-----|
| TASK-AH-S1 | 建立 IDL + Stub/Proxy 骨架 | ICliHookScheduler.idl、IFunctionHookScheduler.idl、ICliToolManager.idl、BUILD.gn | WHEN IDL 编译 THEN Stub/Proxy 生成成功 |
| TASK-AH-S2 | CliToolManagerService 新增 Hook 存储成员 | cli_tool_manager_service.h/.cpp | WHEN 编译 THEN 通过 |

## 后 Task 拆分（v2.0 更新）

| Task ID | 目标 | 受影响文件 | 依赖 |
|---------|------|------------|------|
| TASK-AH-001 | 建立 IDL + Stub/Proxy 骨架（ICliHookScheduler/IFunctionHookScheduler + ICliToolManager 扩展 6 方法，移除 GetFunctionHook；InvokeFunctionResult Parcelable 化 + sequenceable 声明） | ICliHookScheduler.idl、IFunctionHookScheduler.idl、ICliToolManager.idl、invoke_function_executor.h、BUILD.gn | 无 |
| TASK-AH-002 | CliToolManagerService 新增 Hook 存储成员 + dev mode 门控 + RegisterCliHook（失败式注册） | cli_tool_manager_service.h/.cpp | TASK-AH-S1 |
| TASK-AH-003 | ExecTool 链路插入 BeforeCallTool/AfterCallTool 回调点（含 dev mode 门控） | cli_tool_manager_service.cpp | TASK-AH-002 |
| TASK-AH-004 | ExecCmd 链路插入 BeforeCallCmd/AfterCallCmd 回调点 | cli_tool_manager_service.cpp | TASK-AH-002 |
| TASK-AH-005 | 实现 UnregisterCliHook + DeathRecipient 自动注销 cliHook | cli_tool_manager_service.cpp | TASK-AH-002 |
| TASK-AH-006 | 实现 RegisterFunctionHook/UnregisterFunctionHook（失败式注册） | cli_tool_manager_service.h/.cpp | TASK-AH-001 |
| TASK-AH-007 | 实现 BeforeInvokeFunction/AfterInvokeFunction（service 内部分发到 functionHook_，含 dev mode 门控） | cli_tool_manager_service.cpp | TASK-AH-006 |
| TASK-AH-008 | InvokeFunctionExecutor 调用 CliToolMGRClient::BeforeInvokeFunction/AfterInvokeFunction（替代 GetFunctionHook 查询） | invoke_function_executor.h/.cpp | TASK-AH-007 |
| TASK-AH-009 | DeathRecipient 自动注销 functionHook | cli_tool_manager_service.cpp | TASK-AH-006 |
| TASK-AH-010 | CliToolMGRClient 新增 6 方法代理 | cli_tool_mgr_client.h/.cpp | TASK-AH-001 |
| TASK-AH-011 | 无 Hook 时零开销验证（service 直接返回 ERR_OK） | cli_tool_manager_service.cpp | TASK-AH-003/007/008 |
| TASK-AH-012 | 非开发模式门控验证（注册失败 + 回调跳过） | cli_tool_manager_service.cpp | TASK-AH-003/007 |
| TASK-AH-013 | SDK JS：新增 CliHook.d.ts + FunctionHook.d.ts + cliManager 注册函数 + @kit.AbilityKit 导出 | interface/sdk-js/ | TASK-AH-001 |
| TASK-AH-014 | NAPI 层：JsCliHookSchedulerStub + JsFunctionHookSchedulerStub（napi_ref + threadsafe_function 同步分发） | frameworks/js/napi/hook_manager/（新增） | TASK-AH-010/013 |
| TASK-AH-015 | NAPI 层：registerCliHook/unregisterCliHook/registerFunctionHook/unregisterFunctionHook JS 函数绑定 | frameworks/js/napi/hook_manager/ | TASK-AH-014 |
| TASK-AH-016 | NAPI 层：JS 回调同步分发验证（binder 线程 → JS 线程 → 返回） | frameworks/js/napi/hook_manager/ | TASK-AH-015 |

## API 签名、Kit 与权限

### 新增 API

**C++/IPC 层 API：**

| API 签名 | 类型 | Kit | d.ts 位置 | 权限要求 | SysCap |
|----------|------|-----|-----------|----------|--------|
| `int32_t ICliToolManager::RegisterCliHook(const sptr<ICliHookScheduler>& hook)` | SystemAPI | CliToolKit | `interfaces/cli_tool/ICliToolManager.idl` | 仅开发模式（`const.security.developermode.state`） | SystemCapability.Ability.AgentRuntime.Core |
| `int32_t ICliToolManager::UnregisterCliHook(const sptr<ICliHookScheduler>& hook)` | SystemAPI | CliToolKit | 同上 | 同上 | 同上 |
| `int32_t ICliToolManager::RegisterFunctionHook(const sptr<IFunctionHookScheduler>& hook)` | SystemAPI | CliToolKit | 同上 | 同上 | 同上 |
| `int32_t ICliToolManager::UnregisterFunctionHook(const sptr<IFunctionHookScheduler>& hook)` | SystemAPI | CliToolKit | 同上 | 同上 | 同上 |
| `int32_t ICliToolManager::BeforeInvokeFunction([in] String namespace, [in] String functionName, [in,out] WantParams param)` | SystemAPI | CliToolKit | 同上 | -（供 InvokeFunctionExecutor 调用，内部判断 dev mode + 有无 functionHook） | 同上 |
| `int32_t ICliToolManager::AfterInvokeFunction([in] String namespace, [in] String functionName, [in,out] InvokeFunctionResult result)` | SystemAPI | CliToolKit | 同上 | -（同上） | 同上 |
| `int32_t ICliHookScheduler::BeforeCallTool([in,out] ExecToolParam param)` | SystemAPI | CliToolKit | `interfaces/cli_tool/ICliHookScheduler.idl`（新增） | -（由注册方实现） | 同上 |
| `int32_t ICliHookScheduler::AfterCallTool([in,out] ExecResult result)` | SystemAPI | CliToolKit | 同上 | -（由注册方实现） | 同上 |
| `int32_t ICliHookScheduler::BeforeCallCmd([in,out] ExecCmdParam param)` | SystemAPI | CliToolKit | 同上 | - | 同上 |
| `int32_t ICliHookScheduler::AfterCallCmd([in,out] ExecResult result)` | SystemAPI | CliToolKit | 同上 | - | 同上 |
| `int32_t IFunctionHookScheduler::BeforeInvokeFunction([in] String namespace, [in] String functionName, [in,out] WantParams param)` | SystemAPI | CliToolKit | `interfaces/cli_tool/IFunctionHookScheduler.idl`（新增） | - | 同上 |
| `int32_t IFunctionHookScheduler::AfterInvokeFunction([in] String namespace, [in] String functionName, [in,out] InvokeFunctionResult result)` | SystemAPI | CliToolKit | 同上 | - | 同上 |

> **v2.0 变更：** 移除 `ICliToolManager::GetFunctionHook`（v1.0 定义，由 `BeforeInvokeFunction`/`AfterInvokeFunction` 替代）。
>
> **InvokeFunctionResult Parcelable 化：** `InvokeFunctionResult` 当前为 `invoke_function_executor.h:36` 中的普通 struct（非 Parcelable、不在 IDL 中）。`AfterInvokeFunction([in,out] InvokeFunctionResult result)` IDL 参数要求 Parcelable + `sequenceable` 声明。需将 `InvokeFunctionResult` 改为继承 `Parcelable`，新增 `Marshalling`/`Unmarshalling`，并在 IDL 中声明 `sequenceable InvokeFunctionResult..OHOS.CliTool.InvokeFunctionResult;`。字段序列化：`invokeSuccess`(bool)、`errorCode`(int32_t)、`resultCode`(int32_t)、`result`(WantParams)、`message`(string)。

**JS/SDK 层 API（新增 3 个 interface）：**

| API 签名 | 类型 | 文件位置 | SysCap |
|----------|------|----------|--------|
| `interface CliHook { beforeCallTool(param): ExecToolParam; afterCallTool(result): ExecResult; beforeCallCmd(param): ExecCmdParam; afterCallCmd(result): ExecResult }` | SystemAPI | `interface/sdk-js/api/application/CliHook.d.ts`（新增） | SystemCapability.Ability.AgentRuntime.Core |
| `interface FunctionHook { beforeInvokeFunction(namespace: string, functionName: string, args: Record<string, Object>): Record<string, Object>; afterInvokeFunction(namespace: string, functionName: string, result: InvokeResult): InvokeResult }` | SystemAPI | `interface/sdk-js/api/application/FunctionHook.d.ts`（新增） | 同上 |
| `function registerCliHook(hook: CliHook): Promise<void>` / `function unregisterCliHook(hook: CliHook): Promise<void>` / `function registerFunctionHook(hook: FunctionHook): Promise<void>` / `function unregisterFunctionHook(hook: FunctionHook): Promise<void>` | SystemAPI | `interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts`（修改：新增 4 函数到已有 namespace） | 同上 |

> **JS interface 设计说明：**
> - `CliHook` interface 的参数类型 `ExecToolParam`/`ExecCmdParam` 在 `CliHook.d.ts` 中定义（复用已有 `ExecOptions` 类型）；`ExecResult` 复用已有 `cliManager.ExecResult`。
> - `FunctionHook` interface 的参数 `Record<string, Object>` 和 `InvokeResult` 复用已有 `functionManager` 类型。
> - JS 调用方传入的 JS 对象（实现 CliHook/FunctionHook interface）由 NAPI 层内部封装为 `IRemoteStub`（IPC Stub），注册到 CliToolManagerService。
> - 回调时，IPC Stub 在 binder 线程收到请求 → `napi_threadsafe_function`（blocking 模式）分发到 JS 线程 → JS 回调执行 → `std::promise.set_value()` → binder 线程恢复 → IPC 返回改写后的数据。

### 变更/废弃 API

无变更/废弃。

## 构建系统影响

### BUILD.gn 变更

```
文件路径: cli_tool_framework/interfaces/cli_tool/BUILD.gn
变更说明: 新增 ICliHookScheduler.idl、IFunctionHookScheduler.idl 到 idl_gen_interface；
         ICliToolManager.idl 新增 6 方法（移除 GetFunctionHook）；
         新增 sequenceable InvokeFunctionResult..OHOS.CliTool.InvokeFunctionResult

文件路径: cli_tool_framework/services/climgr/BUILD.gn
变更说明: 新增 cli_hook_manager.cpp 到源文件列表；
         确认 external_deps 含 "init:libbegetutil"（已有，提供 parameters.h）

文件路径: cli_tool_framework/frameworks/js/napi/cli_tool_manager/BUILD.gn
变更说明: 新增 js_cli_hook_scheduler_stub.cpp 到源文件列表；
         新增 idl_gen_ICliHookScheduler 依赖；
         registerCliHook/unregisterCliHook JS 函数绑定到 js_cli_manager.cpp

文件路径: cli_tool_framework/frameworks/js/napi/function_manager/BUILD.gn
变更说明: 新增 js_function_hook_scheduler_stub.cpp 到源文件列表；
         新增 idl_gen_IFunctionHookScheduler 依赖；
         registerFunctionHook/unregisterFunctionHook JS 函数绑定到 js_function_manager.cpp；
         新增 cli_tool_mgr_client 依赖（调 BeforeInvokeFunction/AfterInvokeFunction）

文件路径: interface/sdk-js/api/application/CliHook.d.ts（新增）
变更说明: 新增 CliHook interface 类型声明

文件路径: interface/sdk-js/api/application/FunctionHook.d.ts（新增）
变更说明: 新增 FunctionHook interface 类型声明

文件路径: interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts
变更说明: 新增 registerCliHook/unregisterCliHook/registerFunctionHook/unregisterFunctionHook 4 个函数（返回 Promise<void>）

文件路径: interface/sdk-js/kits/@kit.AbilityKit.d.ts
变更说明: 导出 CliHook、FunctionHook 类型
```

> **v2.0 门控方案（ADR-8）：** 不加 `ability_runtime_agent_hook` 特性开关、不加 `#ifdef ABILITY_RUNTIME_AGENT_HOOK` 宏隔离。Hook 代码正常编译，运行时由 `system::GetBoolParameter("const.security.developermode.state", false)` 判断开发模式，非开发模式注册返回 `ERR_INVALID_VALUE`、回调点跳过（零开销）。

### bundle.json 变更

无新增 component；`cli_tool_framework` 既有 component 内新增源文件。新增 `hook_manager` NAPI 模块在 `cli_tool_framework/frameworks/js/napi/` 下。

---

## 可选设计扩展

### 架构图

> 跨模块依赖：Hook 注册方 ↔ CliToolManagerService ↔ tool/cmd/function 调用方。

```mermaid
graph TD
    subgraph "Hook 注册方进程（授权调用方 / JS 进程）"
        HReg[CliHookSchedulerStub/FunctionHookSchedulerStub<br/>实现 before/after 回调]
        JsStub[NAPI: JsCliHookSchedulerStub（在 cli_tool_manager 模块）<br/>封装 JS 对象为 IPC Stub<br/>per-object threadsafe_function 同步到 JS 线程]
        JsObj[JS CliHook/FunctionHook 对象<br/>实现 beforeCallTool/afterCallTool 等]
    end

    subgraph "CliToolManagerService 进程（SA）"
        CTMS[CliToolManagerService]
        HookStore[CliHookManager<br/>存储 cliHook_/functionHook_ sptr<br/>+ 单 hookDeathRecipient_<br/>+ dev mode 门控]
        DevMode[system::GetBoolParameter<br/>const.security.developermode.state]
        ExecTool[ExecTool 执行链路]
        ExecCmd[ExecCmd 执行链路]
        FuncDispatch[BeforeInvokeFunction/AfterInvokeFunction<br/>内部分发到 functionHook_]
    end

    subgraph "tool/cmd 调用方进程"
        ToolCaller[CliToolMGRClient<br/>调用 ExecTool/ExecCmd]
    end

    subgraph "invokeFunction 调用方进程"
        FuncExec[InvokeFunctionExecutor<br/>DoExecute]
        FuncCaller[上层调用方]
    end

    JsObj -->|napi_wrap| JsStub
    JsStub -->|IPC: RegisterCliHook/RegisterFunctionHook| CTMS
    HReg -->|IPC: RegisterCliHook/RegisterFunctionHook| CTMS
    CTMS --> DevMode
    CTMS --> HookStore
    ToolCaller -->|IPC: ExecTool/ExecCmd| CTMS
    CTMS --> ExecTool
    CTMS --> ExecCmd
    ExecTool -.->|before/after 回调| HReg
    ExecCmd -.->|before/after 回调| HReg
    FuncCaller --> FuncExec
    FuncExec -->|IPC: BeforeInvokeFunction/AfterInvokeFunction| CTMS
    CTMS --> FuncDispatch
    FuncDispatch -.->|有 functionHook 则 IPC 分发| HReg
    FuncDispatch -.->|有 functionHook 则 IPC 分发| JsStub
    JsStub -.->|threadsafe_function 到 JS 线程| JsObj
```

### 数据流/控制流

#### cliHook 主流程

| 步骤 | 调用方 | 被调用方 | 数据/接口 | 说明 |
|------|--------|----------|-----------|------|
| 1 | Hook 注册方 | CliToolManagerService | `RegisterCliHook(hook)` | 注册 cliHook Proxy |
| 2 | CliToolManagerService | GetBoolParameter | `IsDeveloperMode()` | dev mode 门控 |
| 3 | CliToolManagerService | hookMutex_ | 检查 cliHook_ 是否已存在 | 失败式注册：已存在返回 ERR_INVALID_VALUE |
| 4 | CliToolManagerService | DeathRecipient | `RegisterDeathRecipient(hook)` | 注册死亡监听 |
| 5 | tool 调用方 | CliToolManagerService | `ExecTool(param)` | 触发执行 |
| 6 | CliToolManagerService | Hook 注册方 | `BeforeCallTool(param)` | before 回调（dev mode + cliHook_ 非空才回调；同步 IPC，不设超时） |
| 7 | CliToolManagerService | ExecTool 链路 | `TryDispatchSkillSession / ValidateAndPrepareTool / SetupAndStartSession` | 用改写后 param 执行 |
| 8 | CliToolManagerService | Hook 注册方 | `AfterCallTool(execResult)` | after 回调；service 回调后用改写 ExecResult 填充 CliSessionInfo.result |
| 9 | CliToolManagerService | tool 调用方 | `SchedulerExecToolReplyEvent(result)` | 返回改写后结果 |

#### functionHook 主流程（v2.0 — service 直接分发）

| 步骤 | 调用方 | 被调用方 | 数据/接口 | 说明 |
|------|--------|----------|-----------|------|
| 1 | Hook 注册方 | CliToolManagerService | `RegisterFunctionHook(hook)` | 注册 functionHook（dev mode 门控 + 失败式注册） |
| 2 | invokeFunction 调用方 | CliToolManagerService | `BeforeInvokeFunction(ns, name, param)` | IPC 到 service |
| 3 | CliToolManagerService | Hook 注册方 | `functionHook_->BeforeInvokeFunction(ns, name, param)` | service 内部分发（dev mode + 有 functionHook 才回调；同步 IPC） |
| 4 | invokeFunction 调用方 | InvokeFunctionExecutor | `DoExecute(改写后 param)` | 用改写后 param 执行；service 无 functionHook 则 param 不变 |
| 5 | invokeFunction 调用方 | CliToolManagerService | `AfterInvokeFunction(ns, name, result)` | IPC 到 service |
| 6 | CliToolManagerService | Hook 注册方 | `functionHook_->AfterInvokeFunction(ns, name, result)` | service 内部分发（同步骤 3） |
| 7 | invokeFunction 调用方 | 上层 | 返回改写后 result | service 无 functionHook 则 result 不变 |

### 时序设计

> 用户明确要求：参与方为注册 hook 的调用方、climgr、调用 tool/cmd/function 的调用方。

#### 时序图 1：注册 cliHook + tool 调用全链路

```mermaid
sequenceDiagram
    participant Reg as Hook 注册方（授权调用方）
    participant CTMS as CliToolManagerService（climgr）
    participant ToolCaller as tool/cmd 调用方

    Note over Reg,CTMS: 阶段一：注册 cliHook
    Reg->>CTMS: RegisterCliHook(cliHook)
    CTMS->>CTMS: GetBoolParameter("const.security.developermode.state")
    alt 开发模式 + 未注册
        CTMS->>CTMS: 保存 cliHook_ sptr + 注册 DeathRecipient
        CTMS-->>Reg: ERR_OK
    else 非开发模式
        CTMS-->>Reg: ERR_INVALID_VALUE
    else 已注册
        CTMS-->>Reg: ERR_INVALID_VALUE（需先 Unregister）
    end

    Note over Reg,ToolCaller: 阶段二：tool 调用方触发 ExecTool，Hook 回调
    ToolCaller->>CTMS: ExecTool(param, eventId, scheduler)
    alt cliHook_ 非空
        CTMS->>Reg: BeforeCallTool(param) [同步 IPC]
        Reg-->>CTMS: ERR_OK + 改写后 param
        Note right of CTMS: 用改写后 param 执行
    end
    CTMS->>CTMS: TryDispatchSkillSession / ValidateAndPrepareTool / SetupAndStartSession
    CTMS->>CTMS: 执行 tool（fork/会话/输出）
    Note right of CTMS: BuildExecResult 生成 ExecResult
    alt cliHook_ 非空
        CTMS->>Reg: AfterCallTool(execResult) [同步 IPC]
        Reg-->>CTMS: ERR_OK + 改写后 execResult
        Note right of CTMS: 用改写 ExecResult 填充 CliSessionInfo.result
    end
    CTMS-->>ToolCaller: SchedulerExecToolReplyEvent(改写后 CliSessionInfo)
```

#### 时序图 2：注册 cliHook + cmd 调用全链路

```mermaid
sequenceDiagram
    participant Reg as Hook 注册方（授权调用方）
    participant CTMS as CliToolManagerService（climgr）
    participant CmdCaller as cmd 调用方

    Note over Reg,CTMS: 阶段一：注册 cliHook（同上图，省略）

    Note over Reg,CmdCaller: 阶段二：cmd 调用方触发 ExecCmd
    CmdCaller->>CTMS: ExecCmd(param, eventId, scheduler, subscriptionId)
    alt cliHook_ 非空
        CTMS->>Reg: BeforeCallCmd(cmdParam) [同步 IPC]
        Reg-->>CTMS: ERR_OK + 改写后 cmdParam
    end
    CTMS->>CTMS: ValidateAndPrepareCmd + CreateShellProcess
    Note right of CTMS: ExecCmd 路径无 skill 分派/无 ToolInfo, 直接 shell 进程
    CTMS->>CTMS: 执行 cmd（shell 进程）
    Note right of CTMS: BuildExecResult 生成 ExecResult
    alt cliHook_ 非空
        CTMS->>Reg: AfterCallCmd(execResult) [同步 IPC]
        Reg-->>CTMS: ERR_OK + 改写后 execResult
    end
    CTMS-->>CmdCaller: SchedulerSessionEvent(改写后 CliSessionInfo)
```

#### 时序图 3：注册 functionHook + invokeFunction 调用全链路（v2.0 — service 直接分发）

```mermaid
sequenceDiagram
    participant Reg as Hook 注册方（授权调用方）
    participant CTMS as CliToolManagerService（climgr）
    participant FuncCaller as invokeFunction 调用方

    Note over Reg,CTMS: 阶段一：注册 functionHook
    Reg->>CTMS: RegisterFunctionHook(functionHook)
    CTMS->>CTMS: system::GetBoolParameter("const.security.developermode.state")
    alt 非开发模式
        CTMS-->>Reg: ERR_INVALID_VALUE
    else 开发模式
        CTMS->>CTMS: 检查 functionHook_ 是否已存在
    alt 已存在
            CTMS-->>Reg: ERR_INVALID_VALUE（已注册）
    else 不存在
            CTMS->>CTMS: 保存 functionHook_ sptr + 注册 DeathRecipient
            CTMS-->>Reg: ERR_OK
        end
    end

    end
    Note over Reg,FuncCaller: 阶段二：invokeFunction 调用方经 service 分发 Hook
    FuncCaller->>CTMS: BeforeInvokeFunction(ns, name, param) [IPC]
    CTMS->>CTMS: dev mode 门控 + functionHook_ 非空？
    alt functionHook 非空 + dev mode
        CTMS->>Reg: functionHook->BeforeInvokeFunction(ns, name, param) [同步 IPC]
        Reg-->>CTMS: ERR_OK + 改写后 param
        Note right of CTMS: 返回改写后 param 给调用方
    else 无 functionHook 或非 dev mode
        CTMS-->>FuncCaller: ERR_OK（零开销，param 不变）
    end
    FuncCaller->>FuncCaller: InvokeFunctionExecutor.DoExecute(改写后 param)
    FuncCaller->>FuncCaller: GetFunctionInfo + ExecuteIntentByFunctionCall
    Note right of FuncCaller: 函数执行（可能跨进程到目标 Extension）

    FuncCaller->>CTMS: AfterInvokeFunction(ns, name, result) [IPC]
    CTMS->>CTMS: dev mode 门控 + functionHook_ 非空？
    alt functionHook 非空 + dev mode
        CTMS->>Reg: functionHook->AfterInvokeFunction(ns, name, result) [同步 IPC]
        Reg-->>CTMS: ERR_OK + 改写后 result
        Note right of CTMS: 返回改写后 result 给调用方
    else 无 functionHook 或非 dev mode
        CTMS-->>FuncCaller: ERR_OK（零开销，result 不变）
    end
    FuncCaller-->>FuncCaller: 返回改写后 result 给上层
```

#### 时序图 5：NAPI 层 JS 回调同步分发（v2.0 新增）

```mermaid
sequenceDiagram
    participant JS as JS 线程
    participant NAPI as NAPI Stub（binder 线程）
    participant CTMS as CliToolManagerService
    participant Reg as JS Hook 注册方进程

    Note over JS,NAPI: 注册阶段
    JS->>NAPI: cliManager.registerCliHook(jsHookObj)
    NAPI->>NAPI: napi_wrap → JsCliHookSchedulerStub<br/>创建 napi_ref 持有 JS 对象<br/>为每个 hook 方法创建 napi_threadsafe_function
    NAPI->>CTMS: IPC: RegisterCliHook(stub) [sptr<ICliHookScheduler>]
    CTMS-->>NAPI: ERR_OK

    Note over CTMS,Reg: 回调阶段（ExecTool 触发）
    CTMS->>NAPI: stub->BeforeCallTool(param) [同步 IPC]
    NAPI->>NAPI: 创建 std::promise<ExecToolParam><br/>打包 {param, &promise} 为 CallData
    NAPI->>JS: napi_call_threadsafe_function(tsfn, callData, napi_tsfn_blocking)
    Note right of NAPI: binder 线程阻塞在 future.get()

    JS->>JS: CallJs 回调（JS 线程）：<br/>1. 创建 JS ExecToolParam 对象<br/>2. napi_call_function(jsObj, "beforeCallTool", ...)<br/>3. JS 回调修改 param<br/>4. 解析返回值为 ExecToolParam
    JS->>NAPI: promise.set_value(改写后 param)
    NAPI->>NAPI: future.get() 返回改写后 param
    NAPI-->>CTMS: IPC 返回改写后 param

    Note over NAPI: 同理 AfterCallTool/BeforeCallCmd/AfterCallCmd/<br/>BeforeInvokeFunction/AfterInvokeFunction
```

#### 时序图 4：Hook 注册方进程死亡自动注销

```mermaid
sequenceDiagram
    participant Reg as Hook 注册方（授权调用方）
    participant CTMS as CliToolManagerService（climgr）
    participant Caller as tool/function 调用方

    Note over Reg: 注册方进程死亡
    CTMS->>CTMS: DeathRecipient.OnRemoteDied
    CTMS->>CTMS: 清除 cliHook_ + functionHook_（单次回调）
    CTMS->>CTMS: 记录 HiSysEvent（HOOK 自动注销）

    Note over Caller: 后续调用不再触发回调
    Caller->>CTMS: ExecTool(param)
    CTMS->>CTMS: cliHook_ 为空, 跳过回调
    CTMS-->>Caller: 正常执行结果（无 Hook 开销）
```

### 数据模型设计

#### Hook 接口涉及的数据结构

以下数据结构在 Hook 回调中作为参数传递，Hook 注册方可读取和改写其字段。

**ExecToolParam**（`exec_tool_param.h:30`，BeforeCallTool 入参，[inout]）

| 字段 | 类型 | 说明 | Hook 可改写 |
|------|------|------|------------|
| toolName | std::string | 工具名称 | 是 |
| subcommand | std::string | 子命令 | 是 |
| challenge | std::string | 挑战码（用于权限校验） | 是 |
| options | ExecOptions | 执行选项（见下表） | 是 |
| args | AAFwk::WantParams | 工具参数（键值对） | 是 |

**ExecOptions**（`exec_options.h:30`，ExecToolParam/ExecCmdParam 内嵌字段）

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| background | bool | false | 是否后台执行（true 立即返回 sessionId） |
| yieldMs | int64_t | 0 | 让出超时时间（毫秒），前台有效 |
| timeout | int64_t | 0 | 执行超时时间（秒） |

**ExecCmdParam**（`exec_cmd_param.h:29`，BeforeCallCmd 入参，[inout]）

| 字段 | 类型 | 说明 | Hook 可改写 |
|------|------|------|------------|
| cmd | std::string | shell 命令字符串 | 是 |
| workDir | std::string | 工作目录 | 是 |
| env | std::string | 环境变量 | 是 |
| policy | std::string | 沙箱策略（透传不解释） | 是 |
| options | ExecOptions | 执行选项（同上表） | 是 |

**ExecResult**（`exec_result.h:28`，AfterCallTool/AfterCallCmd 入参，[inout]）

| 字段 | 类型 | 默认值 | 说明 | Hook 可改写 |
|------|------|--------|------|------------|
| exitCode | int32_t | 1 | 进程退出码（0=成功） | 是（如改写为 0 伪装成功） |
| outputText | std::string | "" | 标准输出内容 | 是（如脱敏） |
| errorText | std::string | "" | 标准错误内容 | 是 |
| signalNumber | int32_t | 0 | 终止信号编号 | 是 |
| timeout | bool | false | 是否超时终止 | 是 |
| executionTime | int64_t | 0 | 执行耗时（毫秒） | 是 |

**CliSessionInfo**（`cli_session_info.h:30`，service 回传给原调用方的完整会话信息）

| 字段 | 类型 | 说明 | Hook 可改写 |
|------|------|------|------------|
| sessionId | std::string | 会话 ID | 否（Hook 通过 ExecResult 间接改写） |
| toolName | std::string | 工具名（ExecCmd 时为 "shell"） | 否 |
| status | std::string | 状态："running"/"completed"/"failed" | 否 |
| result | shared_ptr\<ExecResult\> | 执行结果（status="completed"/"failed" 时非空） | 是（AfterCallTool/AfterCallCmd 改写 *result） |

> **改写路径：** AfterCallTool/AfterCallCmd 入参为 `ExecResult&`（非 `CliSessionInfo&`）。service 回调后用改写后的 `ExecResult` 填充 `CliSessionInfo.result`，再回传给原调用方。

**WantParams**（`OHOS::AAFwk::WantParams`，BeforeInvokeFunction/AfterInvokeFunction 入参，[inout]）

| 特性 | 说明 |
|------|------|
| 类型 | 键值对容器（`unordered_map<string, sptr<IInterface>>`），标准 Parcelable |
| 用途 | function 的输入参数（before）和输出结果（after） |
| Hook 可改写 | 是（增删改任意 key-value） |
| 声明 | `sequenceable WantParams..OHOS.AAFwk.WantParams;`（IDL 中声明） |

**FunctionInfo**（`function_info.h:37`，invokeFunction 查询的函数元数据，非 Hook 参数，仅参考）

| 字段 | 类型 | 说明 |
|------|------|------|
| functionName | std::string | 函数名 |
| functionNamespace | std::string | 命名空间 |
| version | std::string | 版本 |
| description | std::string | 描述 |
| inputSchema | std::string | 输入 schema（JSON） |
| outputSchema | std::string | 输出 schema（JSON） |
| functionType | FunctionType | 函数类型（INTENT_FUNCTION=0） |

#### Hook 存储数据结构

```cpp
// Hook 存储（CliToolManagerService 新增成员）
class CliToolManagerService {
private:
    ffrt::mutex hookMutex_;  // 同时保护 cliHook_ 与 functionHook_（同进程通常同时注册）
    sptr<ICliHookScheduler> cliHook_;  // 单 Hook，失败式注册（已存在则拒绝）
    sptr<IFunctionHookScheduler> functionHook_;  // 单 Hook，失败式注册

    // 单 DeathRecipient：同进程注册两类 Hook 时，一次 OnRemoteDied 同时清除
    // 不同进程分别注册时，各自独立挂 DeathRecipient
    class HookDeathRecipient;  // 复用既有 DeathRecipient 模式（event_dispatcher.h:81）
    sptr<HookDeathRecipient> hookDeathRecipient_;

    // 开发模式门控（ADR-8）
    // #include "parameters.h" → system::GetBoolParameter("const.security.developermode.state", false)
    // 非开发模式：Register* 返回 ERR_INVALID_VALUE；回调点跳过（零开销）
    bool IsDeveloperMode() const;
};
```

**存储方案：**
| 数据类型 | 存储方式 | 位置 | 生命周期 |
|----------|----------|------|----------|
| cliHook_ | sptr<ICliHookScheduler>（内存） | CliToolManagerService 成员 | Register 时创建（dev mode + 不存在时）；Unregister/DeathRecipient 时清除 |
| functionHook_ | sptr<IFunctionHookScheduler>（内存） | 同上 | 同上 |
| hookDeathRecipient_ | sptr<HookDeathRecipient>（内存） | 同上 | 首次 Register 时创建；OnRemoteDied 清除 cliHook_+functionHook_；SA OnStop 析构 |
| dev mode 状态 | `system::GetBoolParameter` 运行时查询（不缓存） | 无存储，每次调用实时判断 | - |

### 算法与状态机

> Hook 生命周期状态机。

```mermaid
stateDiagram-v2
    [*] --> NoHook: 初始
    NoHook --> HookRegistered: RegisterCliHook/RegisterFunctionHook（dev mode + 未注册）
    NoHook --> NoHook: Register 失败（非 dev mode 或已注册）
    HookRegistered --> NoHook: UnregisterCliHook/UnregisterFunctionHook
    HookRegistered --> NoHook: DeathRecipient 触发（注册方进程死亡）
    NoHook --> [*]: SA 停止
```

### 测试性设计

| 测试层级 | 测试目标 | Mock 策略 | 验证方式 |
|----------|----------|-----------|----------|
| 单元测试 | RegisterCliHook/UnregisterCliHook 正常路径 + 失败式注册（已注册返回失败） | mock ICliHookScheduler Stub | `run -t UT -ts cli_tool_mgr_service_test` |
| 单元测试 | ExecTool 链路 BeforeCallTool/AfterCallTool 回调触发与改写 | mock ICliHookScheduler Stub + mock ExecTool 下游 | 同上 |
| 单元测试 | dev mode 门控（非开发模式注册失败 + 回调跳过） | mock GetBoolParameter 返回 false | 同上 |
| 单元测试 | DeathRecipient 触发注销 | 模拟 Proxy 死亡 | 同上 |
| 单元测试 | BeforeInvokeFunction/AfterInvokeFunction service 内部分发 | mock IFunctionHookScheduler Stub | 同上 |
| 单元测试 | InvokeFunctionExecutor 调用 BeforeInvokeFunction/AfterInvokeFunction | mock CliToolMGRClient | `run -t UT -ts invoke_function_executor_test` |
| 单元测试 | 无 Hook 时 service 直接返回 ERR_OK（零开销） | cliHook_/functionHook_ 为空 | 同上 |
| NAPI 测试 | JS 对象封装为 IPC Stub + 同步回调到 JS 线程 | mock JS CliHook/FunctionHook 对象 | NAPI 模块单测 |
| NAPI 测试 | registerCliHook/registerFunctionHook JS 函数 | mock service | NAPI 模块单测 |

### 异常传播时序图

> 覆盖 Hook 回调超时、权限拒绝、注册方死亡三类异常。

```mermaid
sequenceDiagram
    participant Caller as tool/function 调用方
    participant CTMS as CliToolManagerService
    participant Reg as Hook 注册方

    Note over Caller,Reg: 异常场景 1：Hook 回调阻塞
    Caller->>CTMS: ExecTool(param)
    CTMS->>Reg: BeforeCallTool(param)
    Note right of Reg: 回调耗时较长（但 <1s IPC 默认超时）
    Reg-->>CTMS: ERR_OK + 改写后 param
    CTMS->>CTMS: 正常执行（阻塞期间主链路等待）

    Note over Caller,Reg: 异常场景 2：非开发模式注册
    Reg->>CTMS: RegisterCliHook(hook)
    CTMS->>CTMS: GetBoolParameter("const.security.developermode.state") → false
    CTMS-->>Reg: ERR_INVALID_VALUE
    Note right of CTMS: 不保存 Hook

    Note over Caller,Reg: 异常场景 3：已注册（失败式）
    Reg->>CTMS: RegisterCliHook(hook2)
    CTMS->>CTMS: cliHook_ 已存在
    CTMS-->>Reg: ERR_INVALID_VALUE
    Note right of CTMS: 必须先 Unregister

    Note over Caller,Reg: 异常场景 4：注册方死亡（单 DeathRecipient 清两类 Hook）
    Note over Reg: 进程死亡
    CTMS->>CTMS: DeathRecipient.OnRemoteDied
    CTMS->>CTMS: 清除 cliHook_ + functionHook_（一次回调）
    CTMS->>CTMS: HiSysEvent(FAULT, HOOK_AUTO_UNREGISTER)
    Caller->>CTMS: ExecTool(param)
    CTMS->>CTMS: cliHook_ 为空, 跳过回调
    CTMS-->>Caller: 正常结果

    Note over Caller,Reg: 异常场景 5：NAPI Stub JS 回调超时
    CTMS->>Reg: BeforeCallTool(param) [IPC 到 NAPI Stub]
    Reg->>Reg: napi_call_threadsafe_function → future.wait_for(500ms) 超时
    Reg-->>CTMS: ERR_OK + 原 param（未改写）
    Note right of Reg: 超时返回原值，不阻塞 IPC 1s 兜底
```

| 异常场景 | 触发层 | 传播路径 | 最终处理 |
|----------|--------|----------|----------|
| Hook 回调阻塞主链路 | service/调用方 → 注册方 | 回调 IPC 阻塞（无超时，IPC 默认 1s 兜底） | 调用方等待回调返回后继续；由 Hook 注册方保证回调快速返回 |
| 非开发模式注册 | 注册方 → service → GetBoolParameter | 返回 false → service 返回 ERR_INVALID_VALUE | 注册方收到错误码，Hook 未保存 |
| 已注册重复注册 | 注册方 → service → hookMutex_ 检查 | cliHook_/functionHook_ 非空 → service 返回 ERR_INVALID_VALUE | 注册方需先 Unregister |
| NAPI JS 回调超时 | NAPI Stub → JS 线程 | future.wait_for(500ms) 超时 → 返回原 param | 不阻塞 IPC 1s 兜底；记录 HiSysEvent |
| NAPI JS 回调异常 | NAPI Stub → JS 线程 | napi try-catch → set 原值 | 返回原 param；记录 HiSysEvent |
| 注册方死亡 | service DeathRecipient | OnRemoteDied → 清除 Hook → HiSysEvent | 后续调用零开销执行 |

### 资源所有权矩阵

| 资源 | 创建方 | 持有方 | 销毁触发 | 实际释放 | 异常回收 |
|------|--------|--------|----------|----------|----------|
| cliHook_ sptr | Hook 注册方（C++ Stub 或 NAPI JsCliHookSchedulerStub） | CliToolManagerService（持有 Proxy） | UnregisterCliHook / DeathRecipient | sptr 引用计数归零 | 单 DeathRecipient 即时清除 |
| functionHook_ sptr | 同上 | CliToolManagerService（持有 Proxy） | UnregisterFunctionHook / DeathRecipient | 同上 | 同上 |
| hookDeathRecipient_ | CliToolManagerService | CliToolManagerService | 首次 Register 创建；SA OnStop | 析构 | 无（SA 生命周期内常驻） |
| napi_ref (JS 对象引用) | NAPI Stub 构造时 | NAPI Stub（JsCliHookSchedulerStub） | Stub 析构 | napi_delete_reference | JS 线程关闭时 threadsafe_function finalize 回调 |
| napi_threadsafe_function | NAPI Stub 构造时 | NAPI Stub | Stub 析构 / JS 线程关闭 | napi_release_threadsafe_function | finalize 回调清理 |
| std::promise/future | IPC Stub 回调时（binder 线程） | IPC Stub（栈上） | JS 线程 set_value 或 wait_for 超时 | 栈上析构 | 超时返回原 param |

### 接口参数规约

| 接口 | 参数 | 类型 | 合法范围 | 非法处理 | 边界说明 |
|------|------|------|----------|----------|----------|
| RegisterCliHook | hook | `sptr<ICliHookScheduler>` | 非空 Proxy | 空时返回 ERR_INVALID_VALUE | 已注册则返回 ERR_INVALID_VALUE（失败式，ADR-4）；非 dev mode 返回 ERR_INVALID_VALUE |
| RegisterFunctionHook | hook | `sptr<IFunctionHookScheduler>` | 非空 Proxy | 空时返回 ERR_INVALID_VALUE | 同上 |
| BeforeInvokeFunction | namespace, functionName, param | `String, String, WantParams&` | 既有 WantParams 约束 | 改写值需通过 functionType 校验 | service 内部判断 dev mode + functionHook_ 非空，无则直接返回 ERR_OK（零开销） |
| AfterInvokeFunction | namespace, functionName, result | `String, String, InvokeFunctionResult&` | 既有约束 | 改写值需可序列化 | 同上 |
| BeforeCallTool | param | `ExecToolParam&` | 既有约束 | 改写值需通过 ValidateAndPrepareTool | Hook 可改写全部字段 |
| AfterCallTool | result | `ExecResult&` | 既有约束 | 改写值需可序列化 | Hook 可改写 outputText 等 |
| BeforeCallCmd | param | `ExecCmdParam&` | 既有约束 | 改写值需通过 ValidateAndPrepareCmd | ExecCmd 路径直接 CreateShellProcess |
| AfterCallCmd | result | `ExecResult&` | 同 AfterCallTool | 同上 | 同上 |

### 线程与并发模型

> **关键约束：** Hook 回调为纯同步 IPC（`MessageOption::TF_SYNC`），阻塞主链路直到 Hook 注册方返回。**不设超时**（需求方确认），由 Hook 注册方保证回调快速返回。IPC 框架默认 1s 超时兜底（`ipc_process_skeleton.cpp:984` 用 `std::chrono::seconds(TF_WAIT_TIME)`）。

| 操作 | 发起线程 | 回调线程 | 跨进程边界 | 线程安全 | 重入约束 |
|------|----------|----------|------------|----------|----------|
| RegisterCliHook | Binder 线程（IPC 入口） | 同 Binder 线程 | 是（注册方→service） | hookMutex_ 保护 cliHook_ | 已注册则返回失败（不覆盖） |
| ExecTool + BeforeCallTool | Binder 线程 | 同 Binder 线程（同步 IPC） | 是（service→注册方） | ExecTool 链路持 cliHook_ shared_ptr 快照 | 不允许同一 Hook 重入（v1 单 Hook 不涉及） |
| BeforeInvokeFunction | ffrt 任务线程（调用方进程） | 同线程（同步 IPC） | 是（调用方→service→注册方，两跳 IPC） | service 侧 hookMutex_ 保护 functionHook_ | 允许并发调用 invokeFunction |
| AfterInvokeFunction | binder/ffrt 回调线程 | 同线程（同步 IPC） | 同上 | 同上 | 同上 |
| NAPI Stub 回调 | binder 线程（Stub 收到 IPC） | JS 线程（threadsafe_function 分发） | 是（service→注册方进程 Stub） | napi_ref 线程安全 + threadsafe_function | binder 线程阻塞 future.get()，JS 线程执行后 set_value |
| DeathRecipient | Binder 死亡通知线程 | 同线程 | 是（service 内） | hookMutex_ 保护清除 | 与 Register/Unregister 互斥 |

**并发场景：**

| 场景 | 竞争对象 | 保护机制 | 预期行为 |
|------|----------|----------|----------|
| Register 与 DeathRecipient 并发 | cliHook_/functionHook_ | hookMutex_ | DeathRecipient 先清除则 Register 成功（hook_ 为空）；Register 先成功则 DeathRecipient 清除新 Hook（注册方已死，合理） |
| ExecTool 与 Unregister 并发 | cliHook_ sptr 快照 | ExecTool 入口持 shared_ptr 快照（sptr 拷贝） | 快照非空则仍回调，回调返回后快照释放；Unregister 清除 service 侧成员不影响已持快照 |
| 多调用方并发 invokeFunction | functionHook_（service 侧） | hookMutex_ 保护 functionHook_ 快照 | 并发 BeforeInvokeFunction/AfterInvokeFunction IPC 到 service，service 串行分发到注册方 |
| NAPI Stub 回调与 JS 线程关闭并发 | napi_threadsafe_function | napi_threadsafe_function 内部引用计数 + finalize 回调 | JS 线程关闭时 tsfn finalize 触发，binder 线程 future.get() 需设超时兜底（返回原 param） |

### NAPI 层设计（v2.0 新增 — ADR-9）

> JS 调用方通过 NAPI 层注册 JS 对象（实现 CliHook/FunctionHook interface），NAPI 层内部封装为 IPC Stub（RemoteObject），回调时同步到 JS 线程执行。

#### 架构分层

```
JS 调用方                    NAPI 层                           IPC 层                          CliToolManagerService
    |                           |                                 |                                 |
    | cliManager.registerCliHook(jsHookObj)                      |                                 |
    |-------------------------->|                                 |                                 |
    |                           | napi_wrap → JsCliHookSchedulerStub                                 |
    |                           | 创建 napi_ref 持有 JS 对象                                        |
    |                           | 为对象创建 1 个 napi_threadsafe_function（per-object）          |
    |                           |------------------>| IPC: RegisterCliHook(stub sptr) |------------------>|
    |                           |                   |                                 | 保存 cliHook_
    |<--------------------------|<------------------|<--------------------------------| ERR_OK
    |                           |                                                                 |
    ...                         ...                                                               ...
    |                           |<-- IPC: stub->BeforeCallTool(param) <-------------------------|  (ExecTool 触发)
    |                           | binder 线程收到 IPC                                             |
    |                           | 创建 std::promise<ExecToolParam>                                 |
    |                           | napi_call_threadsafe_function(tsfn, callData, blocking)         |
    |                           | future.get() 阻塞                                                |
    | JS 线程: CallJs 回调        |                                                                 |
    |<--------------------------| 调用 JS beforeCallTool(param)                                   |
    | JS 修改 param              |                                                                 |
    |-------------------------->| promise.set_value(改写后 param)                                 |
    |                           | future.get() 返回                                               |
    |                           |-- IPC 返回改写后 param ---------------------------------->|      |
```

#### NAPI Stub 类设计

```cpp
// JsCliHookSchedulerStub — 封装 JS CliHook 对象为 IPC Stub
class JsCliHookSchedulerStub : public IRemoteStub<ICliHookScheduler> {
public:
    JsCliHookSchedulerStub(napi_env env, napi_value jsHookObj);
    ~JsCliHookSchedulerStub();

    // IRemoteStub 接口实现（binder 线程收到 IPC 时调用）
    int32_t BeforeCallTool(ExecToolParam& param) override;
    int32_t AfterCallTool(ExecResult& result) override;
    int32_t BeforeCallCmd(ExecCmdParam& param) override;
    int32_t AfterCallCmd(ExecResult& result) override;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data,
        MessageParcel& reply, MessageOption& option) override;

private:
    napi_ref callbackRef_;  // 持有 JS CliHook 对象引用
    napi_threadsafe_function tsfn_;  // per-object：1 个 tsfn 服务所有 hook 方法
    std::string methodName_;  // CallJs 时标识当前回调的方法名

    // 同步分发模板：binder 线程 → JS 线程 → 返回
    template<typename ParamType, typename JsConverter, typename NativeParser>
    int32_t DispatchSync(const std::string& method, ParamType& param,
        JsConverter toJs, NativeParser toNative);
};

// JsFunctionHookSchedulerStub — 封装 JS FunctionHook 对象为 IPC Stub
class JsFunctionHookSchedulerStub : public IRemoteStub<IFunctionHookScheduler> {
    // 同上结构，2 个方法：BeforeInvokeFunction / AfterInvokeFunction
};
```

#### 同步回调分发机制

IPC Stub 在 binder 线程收到回调请求后的处理流程：

1. **打包数据**：将 native param + `std::promise<ParamType>` 打包为 `CallData`
2. **入队**：`napi_call_threadsafe_function(tsfn, callData, napi_tsfn_blocking)` — blocking 模式确保入队成功
3. **阻塞等待**：`future.get()` — binder 线程阻塞，等待 JS 线程完成
4. **JS 线程 CallJs 回调**：
   - 从 `CallData` 取出 native param
   - 转换为 JS 对象（`CreateJsExecToolParam(env, param)` 等，复用 `js_cli_manager_utils.cpp` 既有转换函数）
   - 调用 JS 对象方法：`napi_call_function(env, jsHookObj, "beforeCallTool", 1, &jsParam, &jsResult)`
   - 解析 JS 返回值：从 `jsResult` 解析回 native param（复用既有 `Unwrap` 函数）
   - `promise->set_value(改写后 param)`
5. **恢复**：binder 线程 `future.get()` 返回改写后 param，IPC 返回

> **超时兜底**：`future.wait_for(500ms)` 替代 `future.get()`，超时则返回原 param（不阻塞 IPC 默认 1s 超时）。JS 回调异常（throw）时 CallJs 中 try-catch 捕获并 set 原值。

#### NAPI 注册函数

```cpp
// napi 注册函数（JS 调用方调用 registerCliHook，返回 Promise<void>）
napi_value RegisterCliHook(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    // 创建 deferred（Promise）
    napi_deferred deferred;
    napi_value promise;
    napi_create_promise(env, &deferred, &promise);

    // 创建 JsCliHookSchedulerStub，封装 JS 对象
    auto stub = sptr<JsCliHookSchedulerStub>(new JsCliHookSchedulerStub(env, argv[0]));

    // 调用 CliToolMGRClient 注册
    int32_t ret = CliToolMGRClient::GetInstance().RegisterCliHook(stub);
    if (ret == ERR_OK) {
        napi_resolve_deferred(env, deferred, nullptr);  // resolve(undefined)
    } else {
        napi_value jsError = CreateCliJsErrorByNativeErr(env, ret);
        napi_reject_deferred(env, deferred, jsError);
    }
    return promise;  // 返回 Promise<void>
}
```

> **JS callback 返回值机制：** JS 端 `beforeCallTool(param)` 等 callback 方法**返回修改后对象**（非 in-place 修改）。NAPI 层通过 `napi_call_function` 的 `jsResult` 输出参数获取 JS 返回值，再用既有 `Unwrap` 函数解析为 native param。CallJs 回调中需区分当前调用的方法名（`beforeCallTool`/`afterCallTool`/`beforeCallCmd`/`afterCallCmd`），从 `CallData` 中的 `method` 字段获取。

#### SDK JS interface 文件设计

**`interface/sdk-js/api/application/CliHook.d.ts`（新增）：**
```ts
import { ExecResult, ExecOptions } from '../@ohos.app.cli.cliManager';

export interface ExecToolParam {
  toolName: string;
  subCommand: string;
  challenge: string;
  args: Record<string, Object>;
  options: ExecOptions;
}

export interface ExecCmdParam {
  cmd: string;
  workDir?: string;
  env?: Record<string, string>;
  policy?: string;
  options: ExecOptions;
}

export interface CliHook {
  beforeCallTool(param: ExecToolParam): ExecToolParam;
  afterCallTool(result: ExecResult): ExecResult;
  beforeCallCmd(param: ExecCmdParam): ExecCmdParam;
  afterCallCmd(result: ExecResult): ExecResult;
}
export default CliHook;
```

**`interface/sdk-js/api/application/FunctionHook.d.ts`（新增）：**
```ts
import { InvokeResult } from '../@ohos.app.function.functionManager';

export interface FunctionHook {
  beforeInvokeFunction(functionNamespace: string, functionName: string,
    args: Record<string, Object>): Record<string, Object>;
  afterInvokeFunction(functionNamespace: string, functionName: string,
    result: InvokeResult): InvokeResult;
}
export default FunctionHook;
```

**`interface/sdk-js/api/@ohos.app.cli.cliManager.d.ts`（修改，新增 4 函数）：**
```ts
// 在已有 namespace cliManager 中新增：
import { CliHook } from './application/CliHook';
import { FunctionHook } from './application/FunctionHook';

declare namespace cliManager {
  // ... 既有函数 ...

  /** @syscap SystemCapability.Ability.AgentRuntime.Core @systemapi @stagemodelonly @since 26.0.0 dynamiconly */
  function registerCliHook(hook: CliHook): Promise<void>;

  /** @syscap SystemCapability.Ability.AgentRuntime.Core @systemapi @stagemodelonly @since 26.0.0 dynamiconly */
  function unregisterCliHook(hook: CliHook): Promise<void>;

  /** @syscap SystemCapability.Ability.AgentRuntime.Core @systemapi @stagemodelonly @since 26.0.0 dynamiconly */
  function registerFunctionHook(hook: FunctionHook): Promise<void>;

  /** @syscap SystemCapability.Ability.AgentRuntime.Core @systemapi @stagemodelonly @since 26.0.0 dynamiconly */
  function unregisterFunctionHook(hook: FunctionHook): Promise<void>;
}
```

#### NAPI 模块组织

| 选项 | 方案 | 优势 | 劣势 |
|------|------|------|------|
| 选定 | 扩展 `cli_tool_manager` + `function_manager` 两个现有模块 | 复用既有模块注册入口和 BUILD.gn；JS namespace 不变（`cliManager`/`functionManager`）；registerCliHook/registerFunctionHook 与各自模块的现有函数在同一 so 内 | 两个现有模块各新增 Stub 源文件 |
| 备选 | 新建 `frameworks/js/napi/hook_manager/` 模块 | 职责清晰，独立 so | 多一个 so；JS 调用方需额外 import；与现有 cliManager namespace 分离 |

> **确认（ADR-9）：** 扩展 `cli_tool_manager` + `function_manager` 两个现有模块。`JsCliHookSchedulerStub` + `registerCliHook`/`unregisterCliHook` 放入 `cli_tool_manager`（`js_cli_hook_scheduler_stub.h/.cpp` 新增，`js_cli_manager.cpp` 新增绑定函数）；`JsFunctionHookSchedulerStub` + `registerFunctionHook`/`unregisterFunctionHook` 放入 `function_manager`（`js_function_hook_scheduler_stub.h/.cpp` 新增，`js_function_manager.cpp` 新增绑定函数）。JS 调用方用 `import cliManager from '@ohos.app.cli.cliManager'` 直接获得 `registerCliHook` 等函数。



> proposal「不涉及项确认」的「安全与权限」= 是，必填。

#### 信任边界交叉分析

| 边界类型 | 跨越的交互 | 风险 | 约束 |
|----------|----------|------|------|
| 跨进程 | Hook 注册方 ↔ CliToolManagerService（IPC: RegisterCliHook/RegisterFunctionHook） | 未授权调用方注册恶意 Hook 劫持命令 | `system::GetBoolParameter("const.security.developermode.state", false)` 门控——仅开发模式可注册；生产环境非开发模式返回 ERR_INVALID_VALUE（关联 AC-1.8/AC-2.7） |
| 跨进程 | CliToolManagerService ↔ Hook 注册方（IPC: BeforeCallTool/AfterCallTool/BeforeCallCmd/AfterCallCmd） | 回调阻塞主链路 | 同步 IPC（不设超时），由注册方保证回调快速返回；IPC 默认 1s 兜底（关联 AC-1.2/AC-1.4） |
| 跨进程 | invokeFunction 调用方 ↔ CliToolManagerService（IPC: BeforeInvokeFunction/AfterInvokeFunction） | service 分发回调阻塞 | 同上（关联 AC-2.2/AC-2.4） |
| 跨进程 | CliToolManagerService ↔ Hook 注册方（IPC: functionHook_->BeforeInvokeFunction/AfterInvokeFunction） | service 内部分发回调 | 同上（关联 AC-2.2/AC-2.4） |
| 跨线程 | NAPI Stub binder 线程 → JS 线程（threadsafe_function 同步分发） | JS 回调异常导致 binder 线程阻塞 | future.wait_for(500ms) 超时兜底 + try-catch 捕获 JS 异常（关联 NAPI 层设计） |

#### 基础安全要求

| 检查项 | 结论 | 说明/措施 |
|--------|------|-----------|
| 加密算法 | N/A | Hook 能力不涉及加密 |
| 密钥管理 | N/A | 不涉及密钥 |
| 随机数 | N/A | 不涉及 |
| 输入验证 | ✅ | Hook 入参（ExecToolParam/ExecCmdParam/CliSessionInfo/WantParams/InvokeFunctionResult）为既有 Parcelable，Marshalling 时校验；改写值需通过既有 ValidateAndPrepareTool/ValidateAndPrepareCmd/functionType 校验 |
| 错误处理 | ✅ | 权限拒绝返回 ERR_PERMISSION_DENIED；Hook 为空返回 ERR_INVALID_VALUE；错误码不泄露敏感信息 |
| 配置 | N/A | 无配置文件 |
| 敏感数据处理 | ✅ | Hook 可改写参数/结果（含 outputText），注册方需自行脱敏；HiSysEvent 不记录参数内容（仅记录事件类型+sessionId） |

### 深度威胁分析(如需)

> 命中高风险判据（Hook 可修改其它调用方的参数与结果，属"可劫持执行链路"的安全敏感场景）。但本设计已在 dev mode 门控、DeathRecipient 自动注销两层做了防御。建议在 spec.md/spec for validation 批准后，由 Owner 决定是否产出独立 `threat-model.md`（STRIDE + 合规）。本设计暂不升级，记录决定如下：

- 升级决定：暂不产出独立 threat-model.md，因本设计已覆盖信任边界分析 + 基础安全要求 + 异常传播时序图；如 Owner 在 review 阶段认为风险等级为"关键"，则触发 `ohos-security-threat-model` skill 补充产出。

## 风险和开放问题

| 项 | 类型 | 影响 | 处理方式 | Owner |
|----|------|------|----------|-------|
| NAPI Stub JS 回调阻塞 binder 线程 | 技术 | 中 | future.wait_for(500ms) 超时兜底，返回原 param；JS 回调异常 try-catch | 待确认 |
| NAPI threadsafe_function 与 JS 引擎关闭竞态 | 技术 | 中 | threadsafe_function 引用计数 + finalize 回调清理；binder 线程超时后返回 | 待确认 |
| BeforeInvokeFunction/AfterInvokeFunction 两跳 IPC 时延 | 性能 | 中 | service 内部判断无 functionHook 时直接返回 ERR_OK（零开销）；有 Hook 时两跳 IPC P95 <50ms | 待确认 |
| Hook 链式注册（多 Hook） | 架构 | 低 | v1 单 Hook（失败式注册）；数据结构从 sptr 扩展为 vector 可向后兼容 | 已确认 |
| Hook 回调死锁（注册方回调中再调用 CliToolManagerService 接口） | 技术 | 中 | 注册方回调中禁止再调用 CliToolManagerService 接口（文档约束） | 待确认 |

## 设计审批

- [x] 需求基线已确认，设计覆盖 P0/P1 AC（含 v2.0 变更：dev mode 门控、失败式注册、service 直接分发、NAPI 层）
- [x] 不涉及项已承接，N/A 和展开项都有结论
- [x] 涉及仓和模块职责清楚（ability_ability_runtime + interface/sdk-js，8 层调用链覆盖含 NAPI 层）
- [x] 调用链层级分析完整，每层覆盖到位
- [x] 适用架构规则已识别并形成设计结论
- [x] 分层和子系统边界合规（Hook 回调走独立 IPC 接口，NAPI 层封装 JS 对象为 Stub，不跨层）
- [x] API 变更有签名、权限/门控、错误码和兼容性说明
- [x] BUILD.gn/bundle.json 影响明确（无新增特性开关；新增 NAPI 模块 + SDK JS 文件）
- [x] 设计输出和后续 Task 拆分明确（TASK-AH-001~016，含 NAPI 层 TASK-AH-013~016）
- [x] 关键设计决策有理由和影响说明（ADR-1~9，含 v2.0 修订的 ADR-4/5/8 和新增 ADR-9）
- [x] 风险和开放问题有 Owner

**结论:** 通过（v2.0 基线变更：CHG-1~CHG-4 已融入设计；门控方式改为运行时 dev mode 判断，注册改为失败式，functionHook 改为 service 直接分发，新增 NAPI 层）
