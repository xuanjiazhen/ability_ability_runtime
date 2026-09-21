# Spec

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | AgentHook：JS Hook 注册/注销与回调拦截 |
| 特性编号 | FEAT-AGENT-HOOK |
| 优先级 | P1 |
| 复杂度 | 标准 |
| 目标版本 | 7.0 |
| API 起始版本 | 26.0.1 |

## 导入方式

```ts
// CLI Hook 相关类型为模块级 named export，非 namespace 成员
import { cliManager, CliHook, ExecToolParam, ExecCmdParam, ExecResultWrap } from '@kit.AbilityKit';
import { functionManager, FunctionHook, InvokeFunctionParam, FunctionResultWrap } from '@kit.AbilityKit';
```

> **注意**：`CliHook`、`ExecToolParam`、`ExecCmdParam`、`ExecResultWrap`、`FunctionHook`、`InvokeFunctionParam`、`FunctionResultWrap` 是 `@kit.AbilityKit` 的 named export，不能通过 `cliManager.CliHook` 或 `functionManager.FunctionHook` 访问。

## JS API 定义

### 注册/注销函数

| API | 签名 | 权限 | 开放级别 | 返回 |
|-----|------|------|----------|------|
| `cliManager.registerCliHook` | `(hook: CliHook): Promise<void>` | `ohos.permission.REGISTER_AGENT_HOOK` | system | Promise\<void\> |
| `cliManager.unregisterCliHook` | `(hook: CliHook): Promise<void>` | `ohos.permission.REGISTER_AGENT_HOOK` | system | Promise\<void\> |
| `functionManager.registerFunctionHook` | `(hook: FunctionHook): Promise<void>` | `ohos.permission.REGISTER_AGENT_HOOK` | system | Promise\<void\> |
| `functionManager.unregisterFunctionHook` | `(hook: FunctionHook): Promise<void>` | `ohos.permission.REGISTER_AGENT_HOOK` | system | Promise\<void\> |

> **说明**：unregister 接口需要传入与 register 时相同的 Hook 对象；NAPI 层校验对象一致性。

### CliHook 接口

```ts
// 来源：api/application/CliHook.d.ts
export interface ExecToolParam {
  toolName: string;
  subCommand: string;
  args: Record<string, Object>;
  challenge: string;
  execOptions?: ExecOptions;  // ExecOptions 为 cliManager namespace 成员
}

export interface ExecCmdParam {
  cmd: string;
  execCmdOptions?: ExecCmdOptions;  // ExecCmdOptions 为 cliManager namespace 成员
}

export interface ExecResultWrap {
  execResult: ExecResult;  // ExecResult 为 cliManager namespace 成员
}

export interface CliHook {
  onBeforeCallTool?(param: ExecToolParam): ExecToolParam;
  onAfterCallTool?(param: ExecResultWrap): ExecResultWrap;
  onBeforeCallCmd?(param: ExecCmdParam): ExecCmdParam;
  onAfterCallCmd?(param: ExecResultWrap): ExecResultWrap;
}
```

- 4 个方法均为**可选**，Hook 对象只需实现其中至少一个，否则 NAPI 层拒绝注册。
- 回调返回值替换原始参数/结果（"全部 hook 模式"）。
- 服务侧通过 `activeMethods` 位掩码（0x01|0x02|0x04|0x08）仅调用已实现的方法。
- 字符串参数（`toolName`/`subCommand`/`cmd`/`functionNamespace`/`functionName`/`challenge`）的长度限制遵循对应字段的既有 API 约束（如 `sendMessage` 的 message 最大 10240 字符），测试设计时可补充超长字符串边界测试点。

### FunctionHook 接口

```ts
// 来源：api/application/FunctionHook.d.ts
export interface InvokeFunctionParam {
  functionNamespace: string;
  functionName: string;
  args: Record<string, Object>;
  invokeOptions?: InvokeOptions;  // InvokeOptions 为 functionManager namespace 成员
}

export interface FunctionResultWrap {
  result: InvokeResult;  // InvokeResult 为 functionManager namespace 成员
}

export interface FunctionHook {
  onBeforeInvokeFunction?(param: InvokeFunctionParam): InvokeFunctionParam;
  onAfterInvokeFunction?(param: FunctionResultWrap): FunctionResultWrap;
}
```

- 2 个方法均为**可选**，Hook 对象只需实现其中至少一个。
- 服务侧通过 `activeMethods` 位掩码（0x01|0x02）仅调用已实现的方法。

## 公共错误码

| 错误码 | 值 | 含义 | 触发场景 |
|--------|------|------|----------|
| ERROR_PERMISSION_DENIED | 201 | 权限拒绝 | 无 `ohos.permission.REGISTER_AGENT_HOOK` 权限 |
| ERROR_NOT_SYSTEM_APP | 202 | 非系统应用 | 调用方不是系统应用 |
| ERROR_INVALID_PARAM | 401 | 参数无效 | NAPI 层校验失败（hook 非对象 / 无已实现方法 / unregister 对象不匹配） |
| ERROR_NOT_DEVELOPER_MODE | 35600034 | 设备未处于开发者模式 | `const.security.developermode.state`=false |
| ERROR_HOOK_ALREADY_REGISTERED | 35600035 | Hook 已注册 | 同类型 Hook 已存在，需先 unregister |
| ERROR_HOOK_NOT_REGISTERED | 35600036 | Hook 未注册 | unregister 时本地无 Stub 缓存 |
| ERROR_INNER | 35600050 | 系统错误 | 连接系统服务失败 / IPC 通信失败 |

> **内部错误码映射**：服务侧内部使用 357xxxx（35700021/35700022/35700023），NAPI 层将其映射为公共错误码 356xxxx。

## 用户故事与验收标准

### US-1: 注册 CliHook 拦截 tool/cmd 执行

| AC | 验收标准 | JS API 测试方式 | 类型 |
|----|----------|----------------|------|
| AC-1.1 | WHEN 系统应用 + 有权限 + 开发模式 + 传入合法 CliHook 对象（至少实现一个可选方法）THEN registerCliHook 返回 Promise resolved | `await cliManager.registerCliHook(hook)` 不抛异常 | 正常 |
| AC-1.2 | WHEN CliHook 的 onBeforeCallTool 被回调 THEN 回调入参为 ExecToolParam 对象（含 toolName/subCommand/args/challenge/execOptions） | 注册含 onBeforeCallTool 的 hook → 调用 `cliManager.execTool(...)` → 在回调中断言 `param.toolName` 等字段 | 正常 |
| AC-1.3 | WHEN onBeforeCallTool 回调返回改写后的 ExecToolParam THEN 服务侧使用改写后的参数执行 | 回调中修改 `param.args` → execTool 返回的 CliSessionInfo 反映改写后参数的执行效果 | 正常 |
| AC-1.4 | WHEN onAfterCallTool 回调返回改写后的 ExecResultWrap THEN 服务侧将改写后结果返回给原调用方 | 回调中修改 `param.execResult.outputText` → execTool 最终 resolve 的 CliSessionInfo.result.outputText 为改写后值 | 正常 |
| AC-1.5 | WHEN onBeforeCallCmd 被回调 THEN 回调入参为 ExecCmdParam（含 cmd/execCmdOptions） | 注册含 onBeforeCallCmd 的 hook → 调用 `cliManager.execCmd(...)` → 断言 `param.cmd` | 正常 |
| AC-1.6 | WHEN onAfterCallCmd 回调返回改写后的 ExecResultWrap THEN 调用方收到改写后结果 | 同 AC-1.4 但使用 execCmd 触发 | 正常 |
| AC-1.7 | WHEN 调用 unregisterCliHook(hook) 传入相同对象 THEN Promise resolved，后续 execTool/execCmd 不再触发回调 | `await cliManager.unregisterCliHook(hook)` → 再次 execTool → 回调未被调用 | 正常 |
| AC-1.8 | WHEN Hook 对象仅实现部分方法（如仅 onBeforeCallTool）THEN 未实现的方法不被调用 | 注册仅含 onBeforeCallTool 的 hook → execCmd → onBeforeCallCmd 不被触发 | 正常 |

### US-2: 注册 FunctionHook 拦截 function 执行

| AC | 验收标准 | JS API 测试方式 | 类型 |
|----|----------|----------------|------|
| AC-2.1 | WHEN 系统应用 + 有权限 + 开发模式 + 传入合法 FunctionHook 对象 THEN registerFunctionHook 返回 Promise resolved | `await functionManager.registerFunctionHook(hook)` 不抛异常 | 正常 |
| AC-2.2 | WHEN onBeforeInvokeFunction 被回调 THEN 回调入参为 InvokeFunctionParam（含 functionNamespace/functionName/args/invokeOptions） | 注册含 onBeforeInvokeFunction 的 hook → 调用 `functionManager.invokeFunction(...)` → 断言 `param.functionNamespace` 等 | 正常 |
| AC-2.3 | WHEN onBeforeInvokeFunction 回调返回改写后的 InvokeFunctionParam THEN 调用方使用改写后参数执行 | 回调中修改 `param.args` → invokeFunction 结果反映改写后参数执行效果 | 正常 |
| AC-2.4 | WHEN onAfterInvokeFunction 回调返回改写后的 FunctionResultWrap THEN 调用方收到改写后结果 | 回调中修改 `param.result.data` → invokeFunction 最终 resolve 的 InvokeResult.data 为改写后值 | 正常 |
| AC-2.5 | WHEN 调用 unregisterFunctionHook(hook) 传入相同对象 THEN Promise resolved，后续 invokeFunction 不再触发回调 | `await functionManager.unregisterFunctionHook(hook)` → 再次 invokeFunction → 回调未被调用 | 正常 |

### US-3: 安全门控

| AC | 验收标准 | JS API 测试方式 | 类型 |
|----|----------|----------------|------|
| AC-3.1 | WHEN 非系统应用调用 registerCliHook/registerFunctionHook THEN reject 错误码 202 | 非系统应用调用 → `catch(err) { expect(err.code).assertEqual(202) }` | 异常 |
| AC-3.2 | WHEN 系统应用但无 REGISTER_AGENT_HOOK 权限 THEN reject 错误码 201 | 缺权限调用 → `catch(err) { expect(err.code).assertEqual(201) }` | 异常 |
| AC-3.3 | WHEN 系统应用 + 有权限但非开发模式 THEN reject 错误码 35600034 | 关闭开发者模式 → `catch(err) { expect(err.code).assertEqual(35600034) }` | 异常 |
| AC-3.4 | WHEN 系统应用 + 有权限 + 开发模式 THEN register 返回 Promise resolved、unregister 返回 Promise resolved | 三层条件满足 → register resolved、unregister resolved | 正常 |

### US-4: NAPI Stub 缓存与参数校验

| AC | 验收标准 | JS API 测试方式 | 类型 |
|----|----------|----------------|------|
| AC-4.1 | WHEN 连续两次调用 registerCliHook 且同类型 Hook 已注册 THEN 第二次 reject 错误码 35600035 | 连续两次 registerCliHook → 第二次 `catch(err) { expect(err.code).assertEqual(35600035) }` | 异常 |
| AC-4.2 | WHEN 调用 registerFunctionHook 且同类型 Stub 已存在 THEN reject 35600035 | 连续两次 registerFunctionHook → 第二次 reject 35600035 | 异常 |
| AC-4.3 | WHEN 调用 registerCliHook 传入非对象（如 null/number/string）THEN reject 401 | `cliManager.registerCliHook(null)` → `catch(err) { expect(err.code).assertEqual(401) }` | 异常 |
| AC-4.4 | WHEN 调用 registerCliHook 传入对象但未实现任何可选方法 THEN reject 401 | `cliManager.registerCliHook({})` → `catch(err) { expect(err.code).assertEqual(401) }` | 异常 |
| AC-4.5 | WHEN 未注册时调用 unregisterCliHook THEN reject 错误码 35600036 | 未注册时调用 unregisterCliHook(hook) → `catch(err) { expect(err.code).assertEqual(35600036) }` | 异常 |
| AC-4.6 | WHEN 调用 unregisterCliHook 传入的对象与注册时不一致 THEN reject 401 | 注册 hookA，调用 `unregisterCliHook(hookB)` → reject 401 | 异常 |
| AC-4.7 | WHEN 调用 unregisterCliHook 未传参数 THEN reject 401 | `cliManager.unregisterCliHook()` → reject 401 | 异常 |
| AC-4.8 | WHEN JS 回调方法抛出异常 THEN 服务侧捕获异常，使用原始 param/result，不阻塞调用方 | 回调中 `throw new Error('test')` → execTool 仍正常返回，结果未被改写 | 异常 |
| AC-4.9 | WHEN JS 回调执行超时 THEN 超时后使用原始 param/result | 回调中 `while(true){}` 阻塞 → execTool 仍正常返回，结果未被改写 | 异常 |

### US-5: 生命周期与异常恢复

| AC | 验收标准 | JS API 测试方式 | 类型 |
|----|----------|----------------|------|
| AC-5.1 | WHEN 无 Hook 注册时 execTool/execCmd/invokeFunction 执行 THEN 行为与引入前一致（零开销） | 不注册 Hook → 正常调用 execTool/execCmd/invokeFunction → 结果不受影响 | 正常 |
| AC-5.2 | WHEN 注册方进程死亡 THEN 服务侧自动注销 Hook，后续不再触发回调 | 注册 Hook → 终止进程 → 服务侧自动注销 → 其他调用方 execTool 不触发回调 | 正常 |
| AC-5.3 | WHEN 非开发模式 + 已注册 Hook THEN 回调点跳过（零开销） | 开发模式注册 Hook → 关闭开发模式 → execTool → 回调不被调用 | 异常 |
| AC-5.4 | WHEN JS 运行环境销毁 THEN 自动注销已注册的 Hook | 注册 Hook → 关闭 env → 自动注销已注册的 Hook | 正常 |

## 业务规则

| 规则 ID | 规则描述 | 约束条件 | 关联 AC |
|---------|----------|----------|---------|
| BR-1 | 全部 hook 模式 | 回调返回值不影响 Hook 应用，修改后的 param/result 始终生效 | AC-1.3/1.4/1.6/2.3/2.4 |
| BR-2 | 失败式注册 | 同类型 Hook 已存在则返回 35600035，必须先 unregister | AC-4.1/4.2 |
| BR-3 | 单实例注册 | 同类型 Hook 仅允许注册一个；register 时同类型已注册则 reject 35600035；unregister 时无注册则 reject 35600036 | AC-4.1/4.2/4.5 |
| BR-4 | unregister 对象校验 | unregister 传入的 Hook 对象必须与 register 时传入的对象一致 | AC-1.7/2.5/4.6 |
| BR-5 | 三层安全门控 | 系统应用（202）→ 权限（201）→ 开发模式（35600034），依次校验 | AC-3.1~3.4 |
| BR-6 | JS 参数校验 | 参数个数校验 + 对象类型校验 + 至少一个可选方法为 function 类型校验 | AC-4.3/4.4/4.7 |
| BR-7 | activeMethods 位掩码 | 通过 activeMethods 位掩码（CLI: 0x01|0x02|0x04|0x08, FUNC: 0x01|0x02）仅调用已实现的方法 | AC-1.8 |
| BR-8 | 同步 IPC 回调 | Hook 回调为同步调用，超时阈值 6s；超时后返回原 param/result | AC-4.8/4.9 |
| BR-9 | 独立 DeathRecipient | CliHook 与 FunctionHook 的 DeathRecipient 相互独立，注销时互不影响 | AC-5.2 |

## 异常与边界规则

| 编号 | 场景 | 触发条件 | 系统行为 | 关联 AC |
|------|------|----------|----------|---------|
| EX-1 | 无 Hook 注册 | Hook 为空 | execTool/execCmd/invokeFunction 执行链路与引入前一致（零开销） | AC-5.1 |
| EX-2 | 注册方进程死亡 | DeathRecipient 触发 | 服务侧自动注销对应类型 Hook，后续不再触发回调 | AC-5.2 |
| EX-3 | JS 回调超时 | 回调执行超过超时阈值 | 返回原 param/result，记录日志 | AC-4.9 |
| EX-4 | JS 回调异常 | JS 方法 throw | try-catch 捕获，返回原 param/result | AC-4.8 |
| EX-5 | 非开发模式 + 已注册 Hook | dev mode 切换为 false | 回调点跳过（零开销） | AC-5.3 |
| EX-6 | 环境销毁 | JS 运行环境销毁 | 自动注销已注册的 Hook | AC-5.4 |
