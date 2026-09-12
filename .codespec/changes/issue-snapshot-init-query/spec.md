# 特性规格：快照初始化状态查询

> 本规格覆盖元能力侧新增 Public JS API 查询当前进程是否正在快照初始化。范围限定：查询接口的用户可见行为、参数契约、返回值语义、跨进程调用链路。快照制作流程本身的改造、快照状态变更通知机制不在本规格范围内。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 快照初始化状态查询 |
| 特性编号 | REQ-SNAPSHOT-001 |
| 所属 Epic | 快照特性使能 |
| 优先级 | P1 |
| 目标版本 | 参见 proposal.md |
| SIG 归属 | Sig-Ability |
| 状态 | Draft |
| 复杂度 | 简单 |

## 本次变更范围（Delta）

本特性为新增能力（lineage: new）。当前代码库中不存在 `IsSnapshot`/`snapshotInit`/`SnapshotInit` 符号，快照初始化状态仅通过内部 `MakeImageState` 枚举和 `ImageProcessType` 枚举管理，无对外查询接口。

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | `hyperSnapManager.isHyperSnapInitializing()` JS API | 应用侧查询当前进程是否正在快照初始化，新增到 `@ohos.app.ability.hyperSnapManager.d.ts` |
| ADDED | `GetSnapshotInitState` IPC 接口（Inner API） | AppMgrClient → AppMgrServiceInner 跨进程查询 |
| ADDED | IPC 事务码 `GET_SNAPSHOT_INIT_STATE` | IAppMgr::Message 枚举新增项 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Proposal | `D:\Code\SDD\proposal\20260730752255-snapshot-launch-proposal-snapshot-init-query.md` | Approved |
| Design | 待产出 | Pending |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md，本文档不重复摘录。

## 代码现状分析

基于 `ability_ability_runtime` 代码库验证：

| 现有实现 | 位置 | 说明 |
|----------|------|------|
| `MakeImageState` 枚举 | `interfaces/inner_api/app_manager/include/appmgr/app_mgr_constants.h:126` | NONE→MAKE_PRELOAD_START→MAKE_PRELOAD_FINISH→MAKE_IMAGE_START→MAKE_IMAGE_FINISH→DESTROY_* |
| `ImageProcessType` 枚举 | 同上:146 | UNSPECIFIED / TEMPLATE / WORK |
| `AppRunningRecord::GetImageProcessType()` | `services/appmgr/include/app_running_record.h` | 获取当前进程的镜像进程类型 |
| `AppRunningRecord::GetMakeImageState()` | `services/appmgr/include/app_running_record.h` | 获取当前快照制作状态 |
| HyperSnap NAPI 模块 | `frameworks/js/napi/app/hyper_snap_manager/` | 已有 `setHyperSnapEnabled`/`requestRebuildHyperSnap`/`getLastError`，无初始化状态查询 |
| HyperSnap JS API 声明 | `interface_sdk-js/api/@ohos.app.ability.hyperSnapManager.d.ts` | 已有 3 个函数声明，无 `isHyperSnapInitializing` |

**关键发现：** 代码库中存在两套无关的 "snapshot" 子系统：(a) `ISnapshotHandler`/`Snapshot` = 屏幕像素图快照（`interfaces/inner_api/ability_manager/include/snapshot.h`），`MissionSnapshot` 定义在同目录的 `interfaces/inner_api/ability_manager/include/mission_snapshot.h`；(b) HyperSnap = 进程检查点镜像。本特性仅涉及 (b)。

## 用户故事

### US-1: 模板进程制作快照过程中查询返回 true

**作为** 生态应用开发者
**我想要** 查询当前进程是否正在快照初始化
**以便** 区分普通预加载进程和模板进程，走不同的初始化流程

### US-2: 快照完成或失败后查询返回 false

**作为** 生态应用开发者
**我想要** 快照完成或失败后接口返回 false
**以便** 正确恢复正常流程

### US-3: 非模板进程查询返回 false

**作为** 生态应用开发者
**我想要** 非模板进程调用接口返回 false
**以便** 非快照应用无需额外处理

## 验收标准 (AC)

| AC 编号 | 验收标准 | 类型 | 关联 US |
|---------|----------|------|---------|
| AC-1 | WHEN 模板进程（`ImageProcessType::TEMPLATE`）处于 `MAKE_PRELOAD_START` 或 `MAKE_PRELOAD_FINISH` 状态时调用查询接口 THEN `Promise` resolve(`true`) | 正常 | US-1 |
| AC-2 | WHEN 模板进程处于 `MAKE_IMAGE_START` 状态时调用查询接口 THEN `Promise` resolve(`true`)（镜像制作中属于快照初始化阶段） | 正常 | US-1 |
| AC-3 | WHEN 模板进程快照制作完成（`MAKE_IMAGE_FINISH`）后调用查询接口 THEN `Promise` resolve(`false`) | 正常 | US-2 |
| AC-4 | WHEN 快照制作失败后调用查询接口 THEN `Promise` resolve(`false`)（失败后状态回退或进程退出） | 异常 | US-2 |
| AC-5 | WHEN 工作进程（`ImageProcessType::WORK`）调用查询接口 THEN `Promise` resolve(`false`)（工作进程非模板进程） | 正常 | US-3 |
| AC-6 | WHEN 非快照进程（`ImageProcessType::UNSPECIFIED`）调用查询接口 THEN `Promise` resolve(`false`) | 正常 | US-3 |
| AC-7 | WHEN 查询接口被调用但 `AppRunningRecord` 不存在或 `appRunningManager_` 为空 THEN `Promise` resolve(`false`)（安全降级，不崩溃） | 异常 | US-3 |
| AC-8 | WHEN AppMgrService SA 未加载或代理获取失败 THEN `Promise` reject，错误码 `16000050`（客户端降级处理） | 恢复 | US-3 |

## 验收追溯

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1 | R-1 | 单测 + 集成 + hidumper 交叉验证 | `appmgr/test/unittest/` + `hidumper -s AppMgrService` 查询模板进程 `MakeImageState` 为 Creating 状态 |
| AC-2 | R-1 | 单测 + 集成 + hidumper 交叉验证 | 同上，`MakeImageState` 为 `MAKE_IMAGE_START` 时 hidumper 显示 Creating |
| AC-3 | R-2 | 单测 + hidumper 交叉验证 | hidumper 显示 `MakeImageState` 非 Creating，接口返回 false |
| AC-4 | R-2 | 单测 + hidumper 交叉验证 | hidumper 显示快照失败后状态回退或进程退出，接口返回 false |
| AC-5 | R-3 | 单测 + hidumper 交叉验证 | hidumper 显示 `ImageProcessType` 为 WORK，接口返回 false |
| AC-6 | R-3 | 单测 + hidumper 交叉验证 | hidumper 显示 `ImageProcessType` 为 UNSPECIFIED，接口返回 false |
| AC-7 | R-4 | 单测 | `appmgr/test/unittest/` |
| AC-8 | R-5 | 单测 | `frameworks/js/napi/app/hyper_snap_manager/` |

## 规则定义

| 规则 ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联 AC |
|---------|------|----------|----------|-----------|---------|
| R-1 | 行为 | 调用方进程为模板进程（`ImageProcessType::TEMPLATE`）且 `MakeImageState` 处于 `MAKE_PRELOAD_START`、`MAKE_PRELOAD_FINISH` 或 `MAKE_IMAGE_START` | `Promise` resolve(`true`) | 查询为只读，不改变状态机 | AC-1, AC-2 |
| R-2 | 行为 | 模板进程的 `MakeImageState` 为 `MAKE_IMAGE_FINISH`、`DESTROY_START`、`DESTROY_FINISH` 或 `NONE` | `Promise` resolve(`false`) | 快照完成后状态不再视为"初始化中" | AC-3, AC-4 |
| R-3 | 行为 | 调用方进程的 `ImageProcessType` 为 `WORK` 或 `UNSPECIFIED` | `Promise` resolve(`false`) | 非模板进程不处于快照初始化 | AC-5, AC-6 |
| R-4 | 异常 | `AppRunningRecord` 不存在或 `appRunningManager_` 为空 | `Promise` resolve(`false`)，记 `TAG_LOGW` 日志 | 安全降级，不崩溃 | AC-7 |
| R-5 | 恢复 | AppMgrService SA 未加载或 IPC 代理获取失败 | `Promise` reject(`16000050`)，记 `TAG_LOGE` 日志 | 客户端按需拉起 SA；拉起失败即 reject | AC-8 |

## API 变更分析

### 新增 API

| API 名称 | 开放范围 | 入参概要 | 返回值 | 功能描述 | 关联 AC |
|----------|----------|----------|--------|----------|---------|
| `hyperSnapManager.isHyperSnapInitializing()` | Public | 无 | `Promise<boolean>` | 查询当前进程是否正在快照初始化 | AC-1~AC-8 |

### 变更/废弃 API

无变更或废弃项。现有 `setHyperSnapEnabled`、`requestRebuildHyperSnap`、`getLastError` 签名与行为不变。

## 接口规格

### JS API

| 属性 | 值 |
|------|-----|
| 函数签名 | `isHyperSnapInitializing(): Promise<boolean>` |
| 返回值 | `Promise<boolean>` — `true`：当前进程正在快照初始化；`false`：不在快照初始化（已完成/失败/非模板进程/SA 不可用） |
| 开放范围 | Public |
| 权限 | 无新增权限要求 |
| d.ts 位置 | `api/@ohos.app.ability.hyperSnapManager.d.ts`（新增到现有 `hyperSnapManager` namespace） |
| SystemCapability | `SystemCapability.Ability.AbilityRuntime.Core` |
| 范式约束 | `@stagemodelonly` |
| 关联 AC | AC-1~AC-8 |

### Inner API（IPC 接口）

| 属性 | 值 |
|------|-----|
| 接口 | `IAppMgr::GetSnapshotInitState(bool &isInitializing)` |
| IPC 事务码 | `IAppMgr::Message::GET_SNAPSHOT_INIT_STATE`（在枚举 END 前递增） |
| 调用模式 | `MessageOption::TF_SYNC`（同步查询，NAPI 层包装为 Promise） |
| 序列化 | `WriteInterfaceToken` → 无额外参数（按调用方 pid 查询） → reply 写 `bool` |
| 服务端实现 | `AppMgrServiceInner::GetSnapshotInitState` → 通过 `IPCSkeleton::GetCallingPid()` 获取调用方进程 → `GetAppRunningRecordByPid` → 读取 `GetImageProcessType()` + `GetMakeImageState()` |
| NAPI 实现 | `frameworks/js/napi/app/hyper_snap_manager/js_hyper_snap_manager.cpp` — 新增 `napi_is_hyper_snap_initializing`，创建 Promise，调用 `AppMgrClient::GetSnapshotInitState`，resolve/reject 回调（参照 `getLastError` 的 Promise 封装模式） |

### 行为场景

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 模板进程 + MAKE_PRELOAD_START/MAKE_PRELOAD_FINISH | Promise resolve(true) | AC-1 |
| 2 | 模板进程 + MAKE_IMAGE_START | Promise resolve(true) | AC-2 |
| 3 | 模板进程 + MAKE_IMAGE_FINISH | Promise resolve(false) | AC-3 |
| 4 | 模板进程 + 快照失败（状态回退或进程退出） | Promise resolve(false) | AC-4 |
| 5 | 工作进程（WORK） | Promise resolve(false) | AC-5 |
| 6 | 普通进程（UNSPECIFIED） | Promise resolve(false) | AC-6 |
| 7 | AppRunningRecord 不存在 | Promise resolve(false)，记 WARN 日志 | AC-7 |
| 8 | SA 不可用 | Promise reject(16000050)，记 ERROR 日志 | AC-8 |

## 兼容性声明

- **已有 API 行为变更:** 否。本特性为新增 Public JS API，不改变 `setHyperSnapEnabled`、`requestRebuildHyperSnap`、`getLastError` 等已有 API 行为。
- **配置文件格式变更:** 否。
- **数据存储格式变更:** 否。查询为运行态内存读取，不持久化。
- **IPC 序列化格式变更:** 新增事务码，不影响已有事务码的序列化格式。
- **最低支持版本:** 参见 proposal.md。
- **API 版本号策略:** 新增 API 自首个引入版本起标注 `@since`，与 `hyperSnapManager` namespace 现有 API 版本策略一致。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 查询仅反映当前时刻状态 | 状态可能在查询后立即变化，不保证状态不变 | AC-1~AC-4 |
| 查询通过 IPC 同步调用 | NAPI 层包装为 Promise 返回，服务端为内存只读查询 | AC-1~AC-8 |
| 客户端须安全降级 | SA 不可用时 Promise reject(`16000050`)，与 `getLastError` 一致 | AC-8 |
| SUPPORT_SCREEN 宏无关 | HyperSnap 不受 `#ifdef SUPPORT_SCREEN` 约束（与窗口焦点不同） | 全部 |
| 复用 `hyperSnapManager` namespace | 新增 API 放入现有 namespace，与 `setHyperSnapEnabled` 等同属 HyperSnap 能力集 | 全部 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 可靠性 | 单次查询不应导致 AppMgrService SA 崩溃 | 压力测试 | `appmgr/test/unittest/` |
| 性能 | 不涉及 | — | — |
| 可测试性 | 查询行为可经 mock 服务端校验；同时可通过 `hidumper` 查询模板进程 `MakeImageState` 是否为 Creating 状态，交叉验证接口返回正确性 | 单测 + hidumper | `appmgr/test/unittest/` + `hidumper -s AppMgrService` |
| 定界定位 | 查询路径有 HiLog 日志（tag: AAFwkTag::APPMGR） | 日志检查 | — |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 | 验证方式 |
|----------|----------|-----------|----------|
| 手机 | 无差异 | — | 集成 |
| 平板 | 无差异 | — | 集成 |
| 折叠屏 | 无差异 | — | 集成 |
| 无屏设备 | HyperSnap 不依赖 SUPPORT_SCREEN，行为不变 | — | 集成 |

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | 查询接口无 UI | — |
| 大字体 | N/A | — | — |
| 深色模式 | N/A | — | — |
| 多窗口/分屏 | 否 | 与窗口无关 | — |
| 多用户 | 否 | 查询按调用方 pid，不依赖用户切换 | — |
| 版本升级 | 否 | 无持久化数据需迁移 | — |
| 生态兼容 | 是 | 新增 Public API，应用侧需主动适配 | — |

## Spec 自审清单

- [x] 无"待定""TBD""TODO"等占位符
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（仅查询接口；快照制作流程不涉及）
- [x] 无语义模糊表述
- [x] AC 与规则表交叉一致
- [x] 规则表每条通过 5 项质量检查

## context-references

```yaml
context-queries:
  - repo: "ability_ability_runtime"
    query: "MakeImageState 枚举定义与 AppRunningRecord 的 GetMakeImageState/GetImageProcessType 方法"
  - repo: "ability_ability_runtime"
    query: "IAppMgr IPC 接口新增事务码的模式（参照 RegisterAppForegroundStateObserver）"
  - repo: "interface_sdk-js"
    query: "@ohos.app.ability.hyperSnapManager.d.ts 现有 API 声明模式（setHyperSnapEnabled/requestRebuildHyperSnap/getLastError）"
```

**关键文档：**
- `services/appmgr/include/appmgr/app_mgr_constants.h`（`MakeImageState` / `ImageProcessType` 枚举定义）
- `services/appmgr/include/app_running_record.h`（`GetImageProcessType` / `GetMakeImageState` 方法声明）
- `interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h`（IAppMgr 接口定义，新增虚方法位置）
- `interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.cpp`（Proxy 序列化模式参考）
- `interfaces/inner_api/app_manager/src/appmgr/app_mgr_stub.cpp`（Stub 分发模式参考）
- `frameworks/js/napi/app/hyper_snap_manager/js_hyper_snap_manager.cpp`（现有 HyperSnap NAPI 模块，新增 `isHyperSnapInitializing` 的参照模板）
- `interface_sdk-js/api/@ohos.app.ability.hyperSnapManager.d.ts`（现有 JS API 声明文件，新增函数声明位置）
