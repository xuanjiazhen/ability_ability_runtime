---
target_release: "7.0"
change_type: new-feature
issue: "https://gitcode.com/openharmony/ability_ability_runtime/issues/16613"
author: "@RuiChen_01"
date: "2026-11-03"
status: Draft
---

# Proposal：意图框架撤销/重做能力（IntentUndo）

## 背景与问题

Intents Kit 意图框架把应用能力以 InsightIntent 注册给系统（小艺、服务卡片、跨设备等入口均可触发），执行单元是 `InsightIntentExecutor` 的 `onExecute` 生命周期。当前存在两类缺口：

1. **用户视角**：意图执行产生的是"业务事务"（下单、删除订单、导航规划），用户没有统一入口撤销"刚才那次意图执行"；模型层/UI 层的撤销无法覆盖小艺、卡片等应用外入口。
2. **框架视角**：意图是黑盒业务操作（网络/数据库/跨页面），框架无法自动推导逆操作；且外部触发时 executor 可能是新实例，内存态凭据不可用。

需要在意图定义中引入**可撤销（undoable）声明与 undo/redo 生命周期**，使意图天然携带撤销语义。

### 业界先例：iOS 26 的 `UndoableIntent`

Apple 在 iOS 26 / macOS 26 正式为 App Intents 提供了撤销能力，形态与本提案**部分重合、关键处不同**：

| 维度 | Apple `UndoableIntent` | 本提案 |
|---|---|---|
| 声明方式 | 遵守协议（编译期） | `undoable` 字段 + 实现探测（运行期，适配 JSON 配置） |
| 撤销回调 | **无** `onUndo`/`onRedo`，只在 `perform()` 内 `registerUndo` 注册闭包 | 显式 `onUndo` / `onRedo` 生命周期 |
| 凭据 | 闭包捕获内存快照 | `(params, executeResult)` 可序列化凭据 |
| 撤销栈 | 共享 `NSUndoManager`（与 UI 同栈，保证顺序） | 同样共享时间线，但组 payload 可序列化 |
| 持久化 | 无（内存态，杀进程即丢） | `ToJsonString` 落盘，支持跨会话撤销 |
| 触发面 | 仅 App 界面手势/菜单 | 小艺 / 卡片 / 跨设备 / 应用内 |

两点结论：①**共享时间线**是 Apple 已验证的正确选择，本提案采纳（见 design §4）；②Apple 的闭包方案受限于"App 活着 + 用户在界面旁"，而意图框架的入口天然在应用外，因此**可序列化凭据 + 显式 `onRedo`** 是本提案的必要差异化，而非重复造轮子。

完整分析见两份参考文档：[reference/ios-undomanager-insight.md](reference/ios-undomanager-insight.md)（引擎层 `NSUndoManager`）与 [reference/ios-undo-system-analysis.md](reference/ios-undo-system-analysis.md)（意图层 `UndoableIntent` + 开发者/执行体验，共 8 张机制图）。

## 用户场景与业务触发

### US-1: 用户撤销一次小艺触发的业务操作
- **角色**：终端用户
- **业务触发**：用户通过小艺触发"删除订单"意图，随后反悔，在应用内或通过系统入口发起撤销
- **业务价值**：意图执行被完整回滚（订单恢复），无需用户手动重做相反操作

### US-2: 开发者注册可撤销意图
- **角色**：应用开发者
- **业务触发**：注册 InsightIntent 时除 `onExecute` 外，实现 `onUndo` / `onRedo`
- **业务价值**：框架自动保存执行凭据（params + ExecuteResult）并在撤销/重做时回传，开发者零记忆负担；未实现 `onUndo` 的意图自动视为不可撤销

### US-3: 跨会话与跨设备撤销
- **角色**：终端用户
- **业务触发**：应用重启后（或跨设备场景下）仍可撤销历史意图
- **业务价值**：意图凭据可序列化（`InsightIntentExecuteResult` 已支持 `ToJsonString`），撤销记录可持久化

## 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 变更类型 | new-feature | 意图生命周期新增 undo/redo，新增撤销栈宿主 |
| 复杂度 | 标准 | 主仓内多模块联动（abilitymgr 服务 + 框架层执行器 + ETS 基类） |
| 涉及仓数量 | 1 | ability_ability_runtime（库层过渡方案可独立落地，不依赖系统改动） |
| 是否涉及 Public/System API | 是 | `@InsightIntentEntry` / `InsightIntentEntryExecutor` 新增可选回调与声明字段 |
| 是否涉及安全/性能关键路径 | 否 | 复用现有意图执行通道与权限校验；凭据落盘需脱敏（见 design） |
| 是否跨 SIG | 否 | 单仓；规范发布需文档仓同步（另立） |

## 目标

1. 意图注册支持 `undoable` 声明（装饰器与配置文件两条路径）。
2. 执行器基类新增可选生命周期 `onUndo` / `onRedo`，框架在撤销/重做时回传 `(params, executeResult)` 凭据。
3. 提供应用进程内 `AppUndoStack`：**与模型层共享同一条撤销时间线**（单一 double-stack，组带 kind 与全局 sequence），一次意图执行成功自动入组，撤销/重做整组迁移。
4. 撤销动作复用现有意图执行通道（携带历史凭据 + action 字段的意图调用），不新增 IPC 接口。
5. 意图执行窗口内产生的模型层细粒度组**收编进该意图组**并原子提交（应用内撤销按钮与全局入口共用同一条栈，需要局部撤销时用作用域过滤而非另建栈）。

## 非目标

- 不做模型层（ArkTS 闭包式）撤销引擎本身（已有独立设计，本提案只做嵌套集成）。
- 不做撤销记录的系统级持久化服务（M3 内以应用侧落盘实现跨会话）。
- 不做协同编辑 / OT / CRDT。

## Agent Scope Guard

| 维度 | 范围/限制 | 需人工确认的触发条件 |
|------|-----------|----------------------|
| 允许仓库 | ability_ability_runtime | 文档仓规范同步另立 |
| 允许模块/目录 | `services/abilitymgr/src|include/insight_intent/`、`frameworks/native/ability/native/insight_intent_executor/`、`frameworks/ets/ets/`、`interfaces/inner_api/ability_manager/include/insight_intent/`、`.codespec/changes/` | 超出时先确认 |
| 禁止修改项 | 既有 IPC 序列化格式（只增不改）；`InsightIntentExecuteManager` 既有状态机语义（只扩不破） | 需变更时先评估兼容性 |
| 外部依赖/网络访问 | 无 | 无 |

## 成功标准

| 标准 | 可观察指标 | 验证方式 |
|------|-----------|---------|
| 可撤销意图执行后可撤销 | undo 后业务状态回滚，onUndo 收到原 params + 原 executeResult | demo：删除订单 → 撤销 → 订单恢复 |
| 不可撤销意图不入栈 | 未实现 onUndo 的意图执行后 canUndo == false | demo：导航意图执行 → 撤销不可用 |
| 撤销后可重做 | redo 后 onRedo 收到同一凭据且执行成功 | demo 全链路 |
| 凭据可序列化 | UndoGroup 落盘后应用重启仍可撤销 | demo 重启场景 |

## 影响范围

| 子系统 | 模块/路径 | 影响类型 |
|--------|-----------|---------|
| AppFwk | `services/abilitymgr/{include,src}/insight_intent/`（profile 解析、execute manager） | 新增字段与生命周期 |
| AppFwk | `interfaces/inner_api/ability_manager/include/insight_intent/`（execute_param） | 新增 action/credential 字段 |
| AppFwk | `frameworks/native/ability/native/insight_intent_executor/`（executor 分发） | 新增两个虚生命周期 |
| AppFwk | `frameworks/ets/ets/@ohos.app.ability.InsightIntentEntryExecutor.ets` | 新增可选方法 |
| AppFwk | 应用侧新增 IntentUndoKit（库层，M1 可独立交付） | 新增 |

## 假设与开放问题

### 假设
- `InsightIntentExecuteResult.ToJsonString/FromJsonString` 序列化能力可直接承载撤销凭据（已核实源码）。
- 服务层 `records_`（intentId → 执行记录）可在撤销时提供"意图确实执行过"的校验。

### 开放问题
- `undoable` 字段是否需要进入标准意图 Schema（垂域规范）还是仅自定义意图支持。
- 跨设备撤销的发起方鉴权边界（凭据由源设备还是目标设备持有）。

## 不涉及项确认

| 维度 | 是否涉及 | 依据 |
|------|---------|------|
| 性能 | 不涉及 | 撤销栈为应用进程内存态 + 可选落盘，无系统级 SLA |
| 安全/权限 | 是 | 凭据含业务结果，落盘需脱敏/加密；撤销调用复用现有权限校验 |
| 兼容性 | 不涉及 | 只增不改：未声明 undoable 的意图行为完全不变 |
| API/SDK | 是 | ETS 基类新增可选方法（向后兼容） |
| IPC/跨进程 | 是 | 复用既有 execute 通道，仅增字段 |
| 构建/组件 | 是 | 库层新增模块，沿用既有 gn 构建 |
| 国际化/无障碍 | 不涉及 | 撤销文案由业务层提供 |
| 数据迁移 | 不涉及 | RDB 新增列按现有字段迁移方式处理 |
