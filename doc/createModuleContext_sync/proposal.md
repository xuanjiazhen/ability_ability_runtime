# REQ003 createModuleContextSync — Proposal

> 版本: v1.0
> 日期: 2026-06-03
> 作者: @zexin_c

## 1. 需求背景

`createModuleContext` 接口已在 API 12 废弃，推荐使用 `application.createModuleContext` 接口替代。但新接口仅提供异步版本，而生态伙伴（如淘宝）在冷启动场景下需要资源加载完成后才能继续执行，异步回调模式无法满足该同步阻塞的执行流程。

因此，需要在 `@ohos.app.ability.application` 中提供同步的 `createModuleContextSync` 接口，供伙伴替代已废弃的同步接口使用。

## 2. 需求描述

为 `application.createModuleContext` 提供同步版本 `createModuleContextSync`，支持以下两种场景：

### 2.1 同应用内获取模块上下文

通过当前应用的 `moduleName` 同步获取对应模块的 `Context` 对象。

### 2.2 跨应用获取模块上下文（需系统权限）

通过 `bundleName` + `moduleName` 同步获取目标应用模块的 `Context` 对象。调用者需为系统应用并具备 `ohos.permission.GET_BUNDLE_INFO_PRIVILEGED` 权限。

### 2.3 接口约束

- 接口为 **同步** 调用，直接返回 `Context` 对象。
- API 参考中需明确说明该同步接口有一定耗时，建议在冷启动等必要场景下使用。

## 3. 输入/处理/输出

| 项目 | 描述 |
|------|------|
| **输入** | `createModuleContextSync` 同步接口调用（context, moduleName）或（context, bundleName, moduleName） |
| **处理** | 1. 在 `@ohos.app.ability.application.d.ts` 中新增 `createModuleContextSync` 接口声明<br>2. 增加 API 参考文档，说明该同步接口有一定耗时 |
| **输出** | `createModuleContextSync` 返回值为 `Context` 对象 |

## 4. 验收场景和标准

| AC 编号 | 验收场景 | 验收标准 |
|---------|---------|---------|
| AC-1.1 | 同应用内同步调用 | 调用 `application.createModuleContextSync(context, "feature_module")` 成功返回 `Context` 对象 |
| AC-1.2 | 跨应用同步调用（有权限） | 系统应用调用 `application.createModuleContextSync(context, "com.example.other", "library_module")` 成功返回 `Context` 对象 |
| AC-1.3 | 跨应用同步调用（无权限） | 非系统应用或无权限调用时抛出 `BusinessError` 异常，错误码正确 |
| AC-2.1 | 参数校验 — 非法 moduleName | 传入不存在的 `moduleName` 时抛出异常，错误信息准确 |
| AC-2.2 | 参数校验 — context 为空 | 传入 `null` 或非法 `context` 时抛出异常 |
| AC-3.1 | API 参考文档 | 新增 `createModuleContextSync` 的 API 参考，包含参数说明、返回值、异常说明及 **"该同步接口有一定耗时"** 的提示 |
| AC-3.2 | d.ts 声明准确 | `@ohos.app.ability.application.d.ts` 中的 `createModuleContextSync` 声明与实现一致，包含完整的重载签名 |

## 5. 约束与假设

- 同步接口内部复用现有的 `ContextImpl::CreateModuleContext` 实现，无 IPC 开销。
- 调用方需自行评估同步阻塞的影响，API 文档中需明确提示耗时风险。
- 错误场景（参数非法、模块不存在、权限不足）统一通过抛异常方式返回，不返回 `null`。
