# SDK 接口设计文档：快照初始化状态查询

> 基于 `spec.md`（已调整为 JS API 方案）设计。
> 参照 `interface_sdk-js/api/@ohos.app.ability.hyperSnapManager.d.ts` 现有 API 声明模式。

## 1. 设计概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 快照初始化状态查询 |
| API 命名空间 | `hyperSnapManager`（`@ohos.app.ability.hyperSnapManager`） |
| 函数名 | `isHyperSnapInitializing` |
| 开放范围 | Public（与 `setHyperSnapEnabled`、`requestRebuildHyperSnap`、`getLastError` 一致） |
| 权限 | 无（无需权限） |
| SystemCapability | `SystemCapability.Ability.AbilityRuntime.Core` |
| 范式约束 | `@stagemodelonly`（与 namespace 内所有 API 一致） |
| 返回模式 | `Promise<boolean>`（异步，参照 `getLastError` 的 Promise 模式） |
| 目标版本 | API 27 |
| 模式参照 | `getLastError(): Promise<HyperSnapErrorInfo>`（同为 IPC 查询 + Promise 返回） |
| 新增类型文件 | 无（不引入新接口类型/数据结构） |

## 2. 产出文件清单

| 文件 | 用途 |
|------|------|
| `hyperSnapManager.d.ts.snippet.md` | 需添加到 `@ohos.app.ability.hyperSnapManager.d.ts` 的函数声明片段 + 使用示例 |

> 本特性**无需新增独立 .d.ts 类型文件**，仅在现有 `hyperSnapManager.d.ts` 中追加一个函数声明。无需新增导入、无需新增 export type。

## 3. 接口契约

### 3.1 JS API

```typescript
function isHyperSnapInitializing(): Promise<boolean>;
```

### 3.2 返回值语义

| 返回结果 | 语义 | 触发条件 |
|----------|------|----------|
| `Promise` resolve(`true`) | 当前进程正在快照初始化 | 模板进程 + `MakeImageState` 为 `MAKE_PRELOAD_START`/`MAKE_PRELOAD_FINISH`/`MAKE_IMAGE_START` |
| `Promise` resolve(`false`) | 当前进程不在快照初始化 | 快照已完成/失败、非模板进程、`AppRunningRecord` 不存在 |
| `Promise` reject(`16000050`) | 系统服务连接失败 | AppMgrService SA 未加载或 IPC 代理获取失败 |

### 3.3 错误码

| 错误码 | 含义 | 触发条件 | 关联 AC |
|--------|------|----------|---------|
| 16000050 | Connect to system service failed | AppMgrService SA 未加载或代理获取失败 | AC-8 |

> 与 `getLastError` 的 `@throws 16000050` 一致。无权限错误码（Public API，无权限要求），无参数错误码（无入参）。

## 4. 设计决策

### 4.1 命名：`isHyperSnapInitializing`

| 决策 | 理由 |
|------|------|
| 前缀 `is` | 布尔查询函数的标准命名约定 |
| 主体 `HyperSnapInitializing` | 与 namespace 名 `hyperSnapManager` 和现有 API 名（`setHyperSnapEnabled`、`requestRebuildHyperSnap`）中的 `HyperSnap` 术语一致 |
| 不用 `isSnapshotInitializing` | namespace 上下文为 HyperSnap，使用 `HyperSnap` 更明确；`Snapshot` 可能与屏幕快照（`ISnapshotHandler`/`MissionSnapshot`）混淆 |
| 不用 `getInitState` / `getMakeImageState` | `get` 前缀暗示获取完整状态对象；此处仅需布尔判断，`is` 前缀更精确 |

### 4.2 返回模式：`Promise<boolean>` 而非 `boolean`

| 对比 | `setHyperSnapEnabled` / `requestRebuildHyperSnap` | `getLastError` | **`isHyperSnapInitializing`** |
|------|---------------------------------------------------|----------------|-------------------------------|
| 返回值 | `void`（fire-and-forget） | `Promise<HyperSnapErrorInfo>` | **`Promise<boolean>`** |
| 是否需 IPC 响应 | 否（仅发送请求） | 是（需读取错误信息） | **是（需查询状态）** |
| 异步/同步 | 同步（无等待） | 异步（Promise） | **异步（Promise）** |

选择 `Promise<boolean>` 的理由：
- 查询需要 IPC 同步调用 AppMgrService 并等待响应，与 `getLastError` 场景一致
- JS 不支持同步 IPC 阻塞（会卡死 JS 线程），必须用 Promise 包装
- 与 `getLastError` 的 Promise 模式形成一致的开发者体验

### 4.3 无参数设计

函数不接受任何参数，因为：
- 查询的是**当前进程**的状态，无需指定 pid/bundleName（通过 `IPCSkeleton::GetCallingPid()` 获取）
- 不需要 `HyperSnapErrorType` 之类的分类参数（与 `getLastError` 不同，此处仅查布尔状态）
- 与 `setHyperSnapEnabled(boolean)` 和 `requestRebuildHyperSnap()` 的简洁签名风格一致

### 4.4 访问级别：Public

与 namespace 内现有 API 一致：
- `setHyperSnapEnabled` — Public，无 `@systemapi`
- `requestRebuildHyperSnap` — Public，无 `@systemapi`
- `getLastError` — Public，无 `@systemapi`
- **`isHyperSnapInitializing`** — Public，无 `@systemapi`

spec 明确标注"开放范围: Public"。

### 4.5 版本标签格式：`dynamic&static`

`hyperSnapManager.d.ts` 使用 `@since X dynamic&static` 格式（不同于 `appManager.d.ts` 的 `@since X dynamic` / `@since Y static` 分行格式）。这是因为 `hyperSnapManager` 自 API 24 起同时支持动态和静态模式，且两种模式的 API 行为完全一致。

新增 API 遵循此格式：`@since 27 dynamic&static`。

### 4.6 无需新增类型文件

本接口不引入新的接口类型或数据结构：
- 无入参
- 返回 `Promise<boolean>`（基本类型）
- 错误为 `BusinessError`（已有通用类型）
- 无 observer/data interface 需要定义

## 5. 与现有 API 的关系

| 现有 API | 关系 | 说明 |
|----------|------|------|
| `setHyperSnapEnabled(enableFlag)` | 前置 | 先启用 HyperSnap，系统才会制作快照；`isHyperSnapInitializing` 查询制作状态 |
| `requestRebuildHyperSnap()` | 互补 | 请求重建快照后，可通过 `isHyperSnapInitializing` 查询是否正在重建 |
| `getLastError(errType)` | 互补 | 快照失败后用 `getLastError` 查错误详情；`isHyperSnapInitializing` 此时返回 `false` |

### 典型配合使用流程

```
1. setHyperSnapEnabled(true)              → 启用 HyperSnap
2. isHyperSnapInitializing() → true/false  → 查询是否正在初始化
3. 如果失败：getLastError(CREATE_SNAPSHOT)  → 获取错误详情
4. 如果需重建：requestRebuildHyperSnap()   → 请求重建
```

## 6. NAPI 绑定侧参考

现有 `js_hyper_snap_manager.cpp` 中 `getLastError` 的 NAPI 实现（Promise 封装模式）可作为模板：

```
新增 NAPI 绑定:
  - 属性描述符: {"isHyperSnapInitializing", ..., napi_is_hyper_snap_initializing, ...}
  - 实现: napi_is_hyper_snap_initializing(env, info)
    → 无参数解析
    → 创建 Promise (napi_create_promise)
    → 调用 AppMgrClient::GetSnapshotInitState(isInitializing)
    → 成功: napi_get_boolean(env, isInitializing) → resolve
    → 失败: 构造 BusinessError(16000050) → reject
  → 参照 getLastError 的 napi_get_last_error Promise 分支
```

## 7. 验证对照表

| Spec AC | SDK 接口覆盖 | 验证方式 |
|---------|-------------|----------|
| AC-1: 模板进程 + 预加载中 → true | `Promise<boolean>` resolve(`true`) 声明 | 集成测试 |
| AC-2: 模板进程 + 镜像制作中 → true | 同上 | 集成测试 |
| AC-3: 快照完成 → false | `Promise<boolean>` resolve(`false`) 声明 | 单测 |
| AC-4: 快照失败 → false | 同上 | 单测 |
| AC-5: 工作进程 → false | 同上 | 单测 |
| AC-6: 普通进程 → false | 同上 | 单测 |
| AC-7: AppRunningRecord 不存在 → false | 同上（resolve(false) 安全降级） | 单测 |
| AC-8: SA 不可用 → reject | `@throws BusinessError 16000050` | NAPI 单测 |

## 8. 待确认项

| # | 问题 | 处理建议 |
|---|------|----------|
| 1 | API 版本号 `@since 27` 为推断值 | 需 API Review 确认实际引入版本 |
| 2 | 是否需要 AsyncCallback 版本 | 现有 `getLastError` 仅 Promise 版本，保持一致不提供 Callback |
| 3 | 中英文翻译同步 | `zh-cn/` 目录需同步产出中文翻译 |
