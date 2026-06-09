# REQ003 createModuleContext 同步接口 — Spec

> 为 `@ohos.app.ability.application` 提供同步的 `createModuleContextSync` 接口，替代已废弃的同步 `createModuleContext` 接口
> 创建日期: 2026-06-08
> 特性编号: create-module-context-sync
> 优先级: P1

## 一、概述

`createModuleContext` 接口已在 API 12 废弃，推荐使用 `application.createModuleContext` 异步接口替代。但生态伙伴（如淘宝）在冷启动等场景需要同步获取模块上下文后方能继续执行，异步 Promise 模式无法满足该同步阻塞流程。

本特性在 `@ohos.app.ability.application` 中新增同步的 `createModuleContextSync` 接口，直接返回 `Context` 对象，供伙伴替代已废弃的同步接口使用。

| 属性 | 值 |
|------|-----|
| 特性名称 | createModuleContext 同步接口 |
| 特性编号 | create-module-context-sync |
| 优先级 | P1 |
| 复杂度 | 简单 |
| 目标版本 | TBD |
| Profile | ability_runtime |

---

## 二、用户故事与验收标准

### US-01: 同应用内同步获取模块上下文

**作为** 应用开发者，**我想要** 同步获取当前应用内指定模块的 `Context`，**以便** 在冷启动等同步流程中直接使用模块资源。

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-1.1 | WHEN 调用 `application.createModuleContextSync(context, "feature_module")` 且该模块存在 THEN 同步返回 `Context` 对象，非 null | 正常 | US-01 |
| AC-1.2 | WHEN 返回的 `Context` 对象 THEN 可通过其获取 moduleName、resourceDir 等属性，行为与异步 `createModuleContext` 一致 | 正常 | US-01 |

### US-02: 跨应用同步获取模块上下文（需系统权限）

**作为** 系统应用开发者，**我想要** 同步获取其他应用指定模块的 `Context`，**以便** 在系统场景下访问其他应用的模块资源。

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-2.1 | WHEN 系统应用调用 `application.createModuleContextSync(context, "com.example.other", "library_module")` 且具备 `ohos.permission.GET_BUNDLE_INFO_PRIVILEGED` 权限 THEN 同步返回 `Context` 对象 | 正常 | US-02 |
| AC-2.2 | WHEN 非系统应用调用跨应用 `createModuleContextSync` THEN 抛出 `BusinessError`，错误码为权限不足/非系统应用 | 异常 | US-02 |
| AC-2.3 | WHEN 系统应用调用跨应用 `createModuleContextSync` 但不具备 `ohos.permission.GET_BUNDLE_INFO_PRIVILEGED` THEN 抛出 `BusinessError`，错误码为权限不足 | 异常 | US-02 |

### US-03: 参数校验与错误处理

**作为** 应用开发者，**我想要** 接口对非法参数返回明确的异常信息，**以便** 在开发阶段快速定位问题。

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-3.1 | WHEN 传入的 `context` 为 null 或非 stageMode 上下文 THEN 抛出 `BusinessError`，错误码为 `401`（参数非法） | 异常 | US-03 |
| AC-3.2 | WHEN 传入的 `moduleName` 为空字符串或不存在的模块 THEN 抛出 `BusinessError` | 异常 | US-03 |
| AC-3.3 | WHEN 同步接口内部 `ContextImpl::CreateModuleContext` 返回 nullptr THEN 抛出 `BusinessError` | 异常 | US-03 |

### US-04: API 参考文档

**作为** 应用开发者，**我想要** API 参考中明确标注同步接口的耗时特性，**以便** 合理评估使用场景。

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-4.1 | WHEN 查看 `createModuleContextSync` 的 API 参考文档 THEN 文档中包含 **"该同步接口有一定耗时"** 的提示 | 文档 | US-04 |
| AC-4.2 | WHEN 查看 `@ohos.app.ability.application.d.ts` THEN `createModuleContextSync` 的两个重载签名完整、准确 | 文档 | US-04 |

---

## 三、业务规则

| 规则 ID | 规则描述 | 约束条件 | 关联 AC |
|---------|----------|----------|---------|
| BR-001 | `createModuleContextSync` 为同步接口，直接返回 `Context` 对象，不返回 Promise | 调用线程会被阻塞至 `ContextImpl::CreateModuleContext` 完成 | AC-1.1, AC-2.1 |
| BR-002 | 同应用内（仅传入 `moduleName`）无需额外系统权限 | 复用现有的 `CreateModuleContextCheck` 参数校验 | AC-1.1 |
| BR-003 | 跨应用（传入 `bundleName` + `moduleName`）需系统应用权限及 `ohos.permission.GET_BUNDLE_INFO_PRIVILEGED` | 复用现有的 `IsSystemAppCall` + `VerifyGetBundleInfoPrivilegedPermission` 校验 | AC-2.1, AC-2.2, AC-2.3 |
| BR-004 | 参数校验复用现有 `CreateModuleContextCheck`：校验 context 非空、stageMode、moduleName 有效性 | 校验失败时通过 `EtsErrorUtil::ThrowInvalidParamError` 抛出异常 | AC-3.1, AC-3.2 |
| BR-005 | 同步接口内部复用 `ContextImpl::CreateModuleContext` 实现，无 IPC，无额外异步开销 | 耗时主要来自 BundleManager 查询和 Context 构造 | AC-1.1, AC-2.1 |
| BR-006 | 所有失败路径均通过抛异常返回，不返回 null 或 undefined | 调用侧需使用 `try-catch` 捕获 `BusinessError` | AC-3.1 ~ AC-3.3 |
| BR-007 | API 参考文档必须包含 **"该同步接口有一定耗时"** 的显式提示 | 提示开发者仅在必要场景（如冷启动）下使用 | AC-4.1 |
| BR-008 | 新增接口在 `@ohos.app.ability.application.d.ts` 中声明两个重载签名 | 与 ArkTS `.ets` 层及 ANI 原生层实现一致 | AC-4.2 |

---

## 四、异常与边界规则

| 编号 | 场景 | 触发条件 | 系统行为 | 关联 AC |
|------|------|----------|----------|---------|
| EX-001 | context 为 null | 传入 null 或 undefined 作为 context | 抛出 `BusinessError`，错误码 `401` | AC-3.1 |
| EX-002 | context 非 stageMode | 传入 FA 模型或其他非法上下文 | 抛出 `BusinessError`，错误码 `401` | AC-3.1 |
| EX-003 | moduleName 为空字符串 | moduleName 为 "" | 抛出 `BusinessError` | AC-3.2 |
| EX-004 | 模块不存在 | bundle 中不存在指定的 moduleName | `ContextImpl::CreateModuleContext` 返回 nullptr，抛出 `BusinessError` | AC-3.2 |
| EX-005 | 跨应用非系统应用 | bundleName 非空且调用者非系统应用 | 抛出 `BusinessError`，错误码为 `202`（非系统应用） | AC-2.2 |
| EX-006 | 跨应用无权限 | bundleName 非空且调用者无 `ohos.permission.GET_BUNDLE_INFO_PRIVILEGED` | 抛出 `BusinessError`，错误码为 `201`（权限不足） | AC-2.3 |
| EX-007 | ContextImpl 返回 nullptr | 内部构造失败（内存不足或其他异常） | 抛出 `BusinessError`，错误码为内部错误 | AC-3.3 |
| EX-008 | 同步接口在主线程调用 | 在主线程（UI 线程）调用 | 允许调用，但文档提示有一定耗时，建议评估影响 | AC-4.1 |

---

## 五、错误码定义

| 错误码 ID | 错误码值 | 含义 | 关联 AC |
|-----------|----------|------|---------|
| ERROR_CODE_INVALID_PARAM | 401 | 参数非法（context 为 null、非 stageMode、moduleName 为空等） | AC-3.1, AC-3.2 |
| ERROR_CODE_PERMISSION_DENIED | 201 | 权限不足（跨应用时缺少 `ohos.permission.GET_BUNDLE_INFO_PRIVILEGED`） | AC-2.3 |
| ERROR_CODE_NOT_SYSTEM_APP | 202 | 非系统应用调用跨应用接口 | AC-2.2 |
| ERROR_CODE_INNER | 16000050 | 内部错误（ContextImpl 构造失败、内存不足等） | AC-3.3 |

---

## 六、接口变更分析

### 新增接口

| 接口名称 | 开放级别 | 参数概要 | 返回值 | 错误码 | 关联 AC |
|----------|----------|----------|--------|--------|---------|
| `createModuleContextSync` | ArkTS (Public) | `context: Context, moduleName: string` | `Context` | 401 / 内部错误 | AC-1.1, AC-1.2, AC-3.1 ~ AC-3.3 |
| `createModuleContextSync` | ArkTS (Public) | `context: Context, bundleName: string, moduleName: string` | `Context` | 201 / 202 / 401 / 内部错误 | AC-2.1 ~ AC-2.3, AC-3.1 ~ AC-3.3 |

### 新增/变更 ANI 原生接口

| 接口名称 | 说明 | 关联 AC |
|-----------|------|---------|
| `nativeCreateModuleContextSync` | ANI 原生函数，对应 `createModuleContextSync` 的同步实现 | AC-1.1, AC-2.1 |

### 变更/废弃接口

无。本次仅新增接口，不修改已有 `createModuleContext` 异步接口行为。

---

## 七、兼容性声明

- **已有 API 行为变更:** 否。新增独立同步接口，不影响任何已有异步接口行为
- **配置文件格式变更:** 否
- **数据存储格式变更:** 否
- **编译宏保护:** 无新增编译宏，所有平台均可用
- **ABI 兼容性:** 新增 ANI native 函数注册，不影响已有 ABI

---

## 八、验证映射

| ID | 规格项 | 验证方式 | 验证命令/方法 | 状态 |
|----|--------|---------|---------------|------|
| V-001 | AC-1.1 ~ AC-1.2（同应用内同步获取） | 功能测试 | ETS 应用调用 `createModuleContextSync(baseContext, "entry")` 验证返回 Context | 待实现 |
| V-002 | AC-2.1（跨应用有权限同步获取） | 功能测试 | 系统应用调用 `createModuleContextSync(baseContext, "com.example", "library")` | 待实现 |
| V-003 | AC-2.2 ~ AC-2.3（跨应用权限校验） | 安全测试 | 非系统应用/无权限调用时捕获异常并校验错误码 | 待实现 |
| V-004 | AC-3.1 ~ AC-3.3（参数校验与错误处理） | 单元测试 | null context、非法 moduleName、内部失败场景 | 待实现 |
| V-005 | AC-4.1 ~ AC-4.2（API 参考与 d.ts） | 文档检视 | 检查 API 参考中的耗时提示及 d.ts 签名完整性 | 待实现 |

### 测试用例详情

#### 同应用内同步获取测试

| 用例 ID | 用例名称 | 前置条件 | 操作步骤 | 预期结果 |
|---------|----------|----------|----------|----------|
| TC-001 | 同步获取同应用模块上下文 | 当前应用存在 `feature_module` | `let ctx = application.createModuleContextSync(baseContext, "feature_module")` | 返回非 null 的 Context 对象，可正常获取 resourceDir |
| TC-002 | 同步获取的 Context 与异步一致 | 同 TC-001 | 对比同步返回的 Context 属性与异步 `createModuleContext` 返回的属性 | moduleName、resourceDir 等属性一致 |

#### 跨应用同步获取测试

| 用例 ID | 用例名称 | 前置条件 | 操作步骤 | 预期结果 |
|---------|----------|----------|----------|----------|
| TC-003 | 系统应用跨应用同步获取 | 系统应用具备 `ohos.permission.GET_BUNDLE_INFO_PRIVILEGED` | `application.createModuleContextSync(baseContext, "com.example.other", "library")` | 返回非 null 的 Context 对象 |
| TC-004 | 非系统应用跨应用调用失败 | 普通应用 | 调用跨应用 `createModuleContextSync` | 抛出 BusinessError，错误码为 202 |
| TC-005 | 无权限系统应用调用失败 | 系统应用无 `ohos.permission.GET_BUNDLE_INFO_PRIVILEGED` | 调用跨应用 `createModuleContextSync` | 抛出 BusinessError，错误码为 201 |

#### 参数校验测试

| 用例 ID | 用例名称 | 前置条件 | 操作步骤 | 预期结果 |
|---------|----------|----------|----------|----------|
| TC-006 | context 为 null | 无 | `application.createModuleContextSync(null, "entry")` | 抛出 BusinessError，错误码 401 |
| TC-007 | moduleName 为空字符串 | 无 | `application.createModuleContextSync(baseContext, "")` | 抛出 BusinessError |
| TC-008 | moduleName 不存在 | 无 | `application.createModuleContextSync(baseContext, "not_exist_module")` | 抛出 BusinessError |

#### 文档与 d.ts 测试

| 用例 ID | 用例名称 | 前置条件 | 操作步骤 | 预期结果 |
|---------|----------|----------|----------|----------|
| TC-009 | API 参考包含耗时提示 | 无 | 查看 `createModuleContextSync` API 参考 | 包含 "该同步接口有一定耗时" 的提示 |
| TC-010 | d.ts 签名完整 | 无 | 查看 `@ohos.app.ability.application.d.ts` | 包含两个完整的重载签名 |

---

## 九、代码映射

| AC 编号 | 验收标准摘要 | 预期实现模块/文件 | 实际实现文件 | 关联 Task | 验证状态 |
|---------|-------------|------------------|-------------|-----------|---------|
| AC-1.1 | 同应用内同步返回 Context | `@ohos.app.ability.application.ets` / `ets_application.cpp` | | | |
| AC-1.2 | 返回的 Context 属性正确 | `ContextImpl::CreateModuleContext`（已有实现） | | | |
| AC-2.1 | 跨应用有权限同步返回 | `@ohos.app.ability.application.ets` / `ets_application.cpp` | | | |
| AC-2.2 | 非系统应用抛出异常 | `ets_application.cpp`（`IsSystemAppCall` 校验） | | | |
| AC-2.3 | 无权限抛出异常 | `ets_application.cpp`（`VerifyGetBundleInfoPrivilegedPermission` 校验） | | | |
| AC-3.1 | context 为 null/非 stageMode | `CreateModuleContextCheck`（已有实现） | | | |
| AC-3.2 | moduleName 非法 | `ContextImpl::CreateModuleContext`（已有实现） | | | |
| AC-3.3 | 内部构造失败 | `ets_application.cpp`（nullptr 检查后抛异常） | | | |
| AC-4.1 | API 参考含耗时提示 | API 参考文档 | | | |
| AC-4.2 | d.ts 签名完整 | `@ohos.app.ability.application.d.ts` | | | |

---

## 十、术语表

| 术语 | 定义 |
|------|------|
| `createModuleContextSync` | `@ohos.app.ability.application` 模块新增的同步接口，用于获取指定模块的 Context |
| `createModuleContext` | `@ohos.app.ability.application` 中已有的异步接口（Promise 模式），API 12 起替代废弃的旧接口 |
| `Context` | 应用上下文对象，提供资源访问、路径获取等能力 |
| `ContextImpl::CreateModuleContext` | 框架层已有实现，根据 moduleName（或 bundleName + moduleName）构造模块 Context |
| `CreateModuleContextCheck` | 框架层已有参数校验函数，校验 context、stageMode、moduleName 等 |
| `ApplicationInit` | ANI 层初始化函数，负责注册 native 函数到 ArkTS 运行时 |
| `ANI` | ArkTS Native Interface，ArkTS 与 C++ 原生层的桥接接口 |
| `BusinessError` | ArkTS 异常类型，包含 code 和 message 字段 |
| `stageMode` | Stage 应用模型，与 FA 模型相对，API 9+ 推荐使用 |
| `GET_BUNDLE_INFO_PRIVILEGED` | 跨应用获取 bundle 信息的特权权限 |
