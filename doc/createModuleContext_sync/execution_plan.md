# REQ003 createModuleContextSync 执行计划

> 版本: v1.0
> 日期: 2026-06-09
> 基于: design.md v1.0 / spec.md v1.0 / proposal.md v1.0

## 一、实现概览

本执行计划按**自下而上**的依赖关系排列：先实现 JS/NAPI 和 ANI 原生层同步方法，再实现 ArkTS 同步封装层。

```
实现顺序：JS/NAPI 层 → ANI 层 → ArkTS API 层
```

## 二、任务分解

### Task 1：JS/NAPI 层 — 新增同步接口

**文件**：
- `frameworks/js/napi/application/js_application.h` — 新增 2 个方法声明
- `frameworks/js/napi/application/js_application.cpp` — 新增 1 个方法实现 + 1 个 native 函数注册

**实现内容**：

| 方法 | 说明 | 关键逻辑 |
|------|------|---------|
| `CreateModuleContextSync` | 静态入口 | `GET_NAPI_INFO_AND_CALL` 分发到 `OnCreateModuleContextSync` |
| `OnCreateModuleContextSync` | 同步实现核心 | 参数校验 → `ContextImpl::CreateModuleContext` 同步调用 → `CreateJsContext` 直接返回 JS 对象 |

**参数校验**：
- `argc >= 2`
- `IsStageContext` + `GetStageModeContext` + `ConvertTo<Context>`
- 跨应用时 `CheckCallerIsSystemApp` + `CheckCallerPermission(PERMISSION_GET_BUNDLE_INFO)`

**错误处理**：所有失败路径通过 `ThrowInvalidParamError` / `ThrowNotSystemAppError` / `ThrowNoPermissionError` 抛异常，不返回 null。

**关联 AC**：AC-1.1, AC-2.1~2.3, AC-3.1~3.3

### Task 2：ANI 层 — 新增同步接口

**文件**：
- `frameworks/ets/ani/application/include/ets_application.h` — 新增 1 个方法声明
- `frameworks/ets/ani/application/src/ets_application.cpp` — 新增 1 个方法实现 + 1 个 `ani_native_function` 注册

**实现内容**：

| 方法 | 说明 | 关键逻辑 |
|------|------|---------|
| `CreateModuleContextSync` | 同步实现核心 | `IsStageContext` + `GetStageModeContext` + `ConvertTo<Context>` → `ContextImpl::CreateModuleContext` → `SetNativeContextLong` + `ContextUtil::CreateEtsBaseContext` 构造 ETS 对象并直接返回 |

**参数校验**：
- `IsStageContext` + `GetStageModeContext` + `ConvertTo<Context>`
- 跨应用时 `PermissionVerification::IsSystemAppCall` + `VerifyGetBundleInfoPrivilegedPermission`

**错误处理**：所有失败路径通过 `EtsErrorUtil::ThrowInvalidParamError` / `ThrowNotSystemAppError` / `ThrowNoPermissionError` 抛异常，不返回 null。

**关联 AC**：AC-1.1, AC-2.1~2.3, AC-3.1~3.3

### Task 3：ArkTS API 层 — 同步封装

**文件**：
- `frameworks/ets/ets/@ohos.app.ability.application.ets` — 新增 `nativeCreateModuleContextSync` 声明 + 2 个 `createModuleContextSync` 重载实现

**实现内容**：

| 函数 | 说明 |
|------|------|
| `nativeCreateModuleContextSync` | native 函数声明，签名：`Context, String, String → Context` |
| `createModuleContextSync(context, moduleName)` | 同应用内封装，先调用 `nativeCreateModuleContextCheck` 校验，再调用 `nativeCreateModuleContextSync(context, "", moduleName)` |
| `createModuleContextSync(context, bundleName, moduleName)` | 跨应用封装，先调用 `nativeCreateModuleContextCheck` 校验（含权限预校验），再调用 `nativeCreateModuleContextSync(context, bundleName, moduleName)` |

**关联 AC**：AC-1.1, AC-2.1, AC-4.1, AC-4.2

## 三、实现顺序与依赖

```
Task 1 (JS/NAPI 层) ──┐
                       ├── Task 3 (ArkTS API 层)
Task 2 (ANI 层) ───────┘
```

**推荐实现顺序**：Task 1 → Task 2 → Task 3（Task 1 和 Task 2 可并行）

- Task 1 和 Task 2 相互独立，分别对应 JS 和 ETS 两条技术栈
- Task 3 依赖 Task 2 的 `nativeCreateModuleContextSync` 签名确认后方可实施

## 四、文件变更清单

| # | 层级 | 文件路径 | 操作 | Task |
|---|------|---------|------|------|
| 1 | JS/NAPI 头文件 | `frameworks/js/napi/application/js_application.h` | 修改 | Task 1 |
| 2 | JS/NAPI 实现 | `frameworks/js/napi/application/js_application.cpp` | 修改 | Task 1 |
| 3 | ANI 头文件 | `frameworks/ets/ani/application/include/ets_application.h` | 修改 | Task 2 |
| 4 | ANI 实现 | `frameworks/ets/ani/application/src/ets_application.cpp` | 修改 | Task 2 |
| 5 | ArkTS API | `frameworks/ets/ets/@ohos.app.ability.application.ets` | 修改 | Task 3 |

共 **5 个文件**，全部为修改（无新增文件），不涉及编译宏保护。

## 五、风险与注意事项

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| 同步接口在主线程阻塞 | 冷启动等场景 UI 响应延迟 | API 参考文档中明确标注 **"该同步接口有一定耗时"**，引导开发者仅在必要场景使用 |
| 参数校验与异步版本不一致 | 行为差异导致用户困惑 | 同步版本复用与异步版本完全相同的校验逻辑（`CreateModuleContextCheck`、`IsStageContext`、`ConvertTo<Context>`） |
| `ContextImpl::CreateModuleContext` 内部异常 | 同步接口无 Promise reject 通道 | 失败时通过 ANI/JS 异常机制抛出 `BusinessError`，调用侧使用 `try-catch` 捕获 |
| ANI native 函数签名不匹配 | 运行时链接失败 | 严格按照 ArkTS `.ets` 中声明的参数类型和返回值类型编写 `ani_native_function` 签名 |

## 六、测试验证计划

| 测试类型 | 覆盖范围 | 关联 TC |
|---------|---------|---------|
| 单元测试 | JS/NAPI 参数校验（null context、非 stageMode、非法 moduleName）、ANI 参数校验 | TC-006~008 |
| 功能测试 | 同应用内同步获取 Context、跨应用有权限同步获取、返回对象属性正确性 | TC-001~003 |
| 安全测试 | 非系统应用跨应用调用、无权限系统应用调用 | TC-004~005 |
| 文档测试 | API 参考中是否包含耗时提示、d.ts 签名是否完整准确 | TC-009~010 |
