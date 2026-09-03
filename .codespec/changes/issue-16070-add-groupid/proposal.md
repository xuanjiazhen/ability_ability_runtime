---
phase_status: approved
approval:
  status: approved
  approver: "xuzheheng"
  evidence: "用户会话明确回复'批准 Define'"
  approved_at: "2026-09-03"
  baseline_digest: "bd0012c2690c267eefcf4aa9cbc2b2c0474fd13dd3b64e7ef70b19ff0f22bddc"
target_release:
  id: OpenHarmony-6.0-Release
  status: committed
---

# 需求文档

> 主文档只保存需求输入和已确认基线。澄清过程仅在 complex/critical，或出现澄清阻断、争议决策、跨 SIG/多仓协调时写入条件附录。

## 需求输入

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | REQ-issue-16070-add-groupid |
| 需求名称 | setAbilityInstanceInfo 接口新增 groupId 可选参数并传递给窗口 |
| 来源 | 用户输入 |
| 提出人 | xuzheheng |
| 目标发行版本 | OpenHarmony-6.0-Release（API 26.1.0+） |
| 候选 Profile | none |
| 优先级 | P1 |
| Define 阶段状态 | Approved |

### 原始描述

**原始问题：** 在原 setAbilityInstanceInfo 接口中新增 groupId 可选参数，传给给窗口。

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 应用开发者 | setAbilityInstanceInfo 无法指定 ability 实例的分组标识，窗口侧无法基于 groupId 进行实例分组管理 | 同类型 ability 实例无法分组聚合显示 |

**期望结果：** setAbilityInstanceInfo 接口支持传入 groupId 可选参数，该参数透传至窗口管理器（Rosen SceneBoard），使窗口侧能基于 groupId 进行实例分组。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 源码 | `interfaces/kits/native/ability/ability_runtime/ability_context.h:490` | C++ SDK 接口声明，groupId 为第三参数 |
| 源码 | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp:1149` | 实现，调用 `SetSessionLabelAndIcon(label, icon, groupId)` |
| 源码 | `frameworks/ets/ets/application/UIAbilityContext.ets:780` | ETS 接口，groupId 可选默认空串 |
| 源码 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp:62` | `MAX_ABILITY_INSTANCE_GROUP_ID_LENGTH = 64` |
| 源码 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp:88` | NAPI 层同样定义长度上限 64 |

## 需求基线

### 基线信息

| 字段 | 内容 |
|------|------|
| 基线版本 | v1.0 |
| 基线日期 | 2026-09-03 |
| Owner | xuzheheng |
| 确认人 | xuzheheng |
| 确认依据 | 用户会话确认：当前代码由提出人实现，不涉及窗口侧修改，API 版本确认 |
| 复杂度 | 标准 |
| Profile | none |
| 目标发行版本 | OpenHarmony-6.0-Release（API 26.1.0+） |
| 版本状态 | committed |
| 基线结论 | 通过 |

### 问题、目标与成功指标

**问题陈述：** ability 实例缺少分组标识，窗口侧无法基于 groupId 对同类型 ability 实例进行分组聚合管理。需在 setAbilityInstanceInfo 接口中增加 groupId 可选参数并透传给窗口。

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| setAbilityInstanceInfo 支持 groupId 可选参数 | 调用方传入 groupId 后，窗口侧 ISession 收到该值 | 单元测试 + 接口签名检查 |
| groupId 透传至窗口 | SetSessionLabelAndIcon 第三参数携带 groupId | 实现代码检查 |
| groupId 长度限制 | 超过 64 字节返回参数错误 | NAPI/ANI 绑定层单测 |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为应用开发者，我想要在调用 setAbilityInstanceInfo 时传入 groupId，以便窗口侧能基于分组标识管理 ability 实例 | P0 |
| US-2 | 作为应用开发者，我想要 groupId 为可选参数（不传时行为不变），以便保持向后兼容 | P0 |
| US-3 | 作为应用开发者，我想要 groupId 超长时得到明确错误，以便避免传入非法值 | P1 |

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-1 | WHEN 应用调用 `setAbilityInstanceInfo(label, icon, groupId)` 传入非空 groupId THEN 窗口侧 ISession::SetSessionLabelAndIcon 收到该 groupId 值 | 正常 | US-1 |
| AC-2 | WHEN 应用调用 `setAbilityInstanceInfo(label, icon)` 不传 groupId（ETS 默认空串） THEN 行为与变更前一致，窗口侧收到空 groupId | 边界 | US-2 |
| AC-3 | WHEN 应用传入 groupId 长度超过 64 字节 THEN 绑定层（NAPI/ANI）返回 INVALID_PARAMETERS_ERR，不向窗口侧透传 | 异常 | US-3 |
| AC-4 | WHEN 设备未启用 SceneBoard THEN setAbilityInstanceInfo 返回 ERR_CAPABILITY_NOT_SUPPORT | 异常 | US-1 |

### 范围边界

**包含：**
- C++ SDK 接口 `AbilityContext::SetAbilityInstanceInfo` 新增 groupId 第三参数
- C++ SDK 实现 `AbilityContextImpl::SetAbilityInstanceInfo` 透传 groupId 到 ISession
- ETS 接口 `setAbilityInstanceInfo` 新增 groupId 可选参数（默认空串）
- ANI 绑定层参数解析与 groupId 长度校验
- NAPI 绑定层参数解析与 groupId 长度校验
- FA 模型 JS 绑定层 groupId 参数

**不包含：**
- 窗口管理器（Rosen/window_manager）侧 ISession::SetSessionLabelAndIcon 的实现（窗口侧已支持或无需修改）
- 窗口侧基于 groupId 的分组逻辑实现
- C++ SDK 层 groupId 长度校验（在 NAPI/ANI 绑定层校验，SDK 层透传）

| 阶段（如适用） | 范围 | 不做 | 进入条件 |
|----------------|------|------|----------|
| 完整 | ability_runtime 全层 groupId 参数新增与透传 | 窗口侧实现 | 窗口侧 ISession 已支持或无需修改 |

### Agent Scope Guard

| 维度 | 允许范围/限制 | 越界触发条件 | 处理规则 |
|------|---------------|--------------|----------|
| 允许检索仓库 | `foundation/ability/ability_runtime`（本仓） | 需要访问未列仓库 | 停止探索，补充影响分析并重新确认基线 |
| 允许模块/目录 | `interfaces/kits/native/`、`frameworks/native/`、`frameworks/ets/`、`frameworks/js/napi/` | 需要扩大模块/目录 | 更新范围与影响范围，经 Owner 确认后继续 |
| 禁止访问/修改 | 凭据、生成物、`.codecheck_reports/`（只读参考） | 命中禁止项 | 标记 Blocked，不读取、不修改 |
| 外部网络/私有知识源 | DeepWiki MCP（arkui_ace_engine 仓文档查询）；本仓 AGENTS.md | 需要新增权限或未授权来源 | 请求人工授权；未授权前不得使用 |

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| 元能力 | ability_runtime | `interfaces/kits/native/ability/ability_runtime/ability_context.h` | 公共 SDK 接口声明 | 修改 | xuzheheng |
| 元能力 | ability_runtime | `interfaces/kits/native/ability/ability_runtime/ability_context_impl.h` | 公共 SDK 实现声明 | 修改 | xuzheheng |
| 元能力 | ability_runtime | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp` | 公共 SDK 实现 | 修改 | xuzheheng |
| 元能力 | ability_runtime | `frameworks/ets/ets/application/UIAbilityContext.ets` | Stage 模型 ETS 接口 | 修改 | xuzheheng |
| 元能力 | ability_runtime | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp` | ANI 绑定 | 修改 | xuzheheng |
| 元能力 | ability_runtime | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp` | NAPI 绑定 | 修改 | xuzheheng |
| 元能力 | ability_runtime | `frameworks/js/napi/ability_context/ability_context.js` | FA 模型 JS 绑定 | 修改 | xuzheheng |
| 元能力 | ability_runtime | `test/unittest/.../ability_context_impl_second_test.cpp` | 单元测试 | 修改 | xuzheheng |

### API 变更项清单

| API 名称 | 变更类型 | 开放范围 | 概要说明 |
|----------|----------|----------|----------|
| `AbilityContext::SetAbilityInstanceInfo` | 修改 | Public | C++ SDK 新增第三参数 `const std::string& groupId` |
| `setAbilityInstanceInfo` (ETS) | 修改 | Public | ETS 接口新增可选参数 `groupId: string = ''` |
| `setAbilityInstanceInfo` (NAPI) | 修改 | Public | FA 模型 NAPI 绑定新增 groupId 参数解析与校验 |
| `setAbilityInstanceInfo` (ANI) | 修改 | Public | Stage 模型 ANI 绑定新增 groupId 参数解析与校验 |

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 否 | groupId 为字符串透传，无额外计算开销 | — |
| 安全与权限 | 否 | 权限校验在窗口侧执行（SET_ABILITY_INSTANCE_INFO 权限），本变更不涉及权限逻辑 | — |
| 兼容性 | 是 | 新增可选参数，向后兼容（不传时行为不变） | spec.md |
| API/SDK | 是 | 涉及 Public API 接口签名修改 | design.md / spec.md |
| IPC/跨进程 | 否 | 已有 IPC 通道（ISession），仅新增参数透传，无协议变更 | — |
| 构建与部件 | 否 | 无新增依赖 | — |
| 国际化/无障碍 | 否 | groupId 为技术标识，非用户可见字符串 | — |
| 数据迁移 | 否 | 无持久化数据结构变更 | — |

### 依赖、风险与外部约束

| 依赖/风险 | 类型 | 影响 | 缓解措施/Owner | 状态 |
|-----------|------|------|----------------|------|
| 窗口侧 ISession::SetSessionLabelAndIcon 已支持 groupId | 跨仓 | 低 | 用户确认不涉及窗口修改 | 已确认 |
| C++ SDK 层缺 groupId 长度校验 | 技术 | 低 | 绑定层（NAPI/ANI）已做 64 字节限制 | 已确认 |

### 依据与上下文引用

| 来源 | 查询/读取内容 | 基线采用的结论 | 可信度 | 证据路径/链接 |
|------|---------------|----------------|--------|---------------|
| 源码 | `ability_context.h:490` — SetAbilityInstanceInfo 声明 | groupId 已作为 C++ SDK 第三参数 | 高 | `interfaces/kits/native/ability/ability_runtime/ability_context.h:490` |
| 源码 | `ability_context_impl.cpp:1149-1186` — 实现 | 已调用 `SetSessionLabelAndIcon(label, icon, groupId)` 透传至窗口 | 高 | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp:1149` |
| 源码 | `UIAbilityContext.ets:780` — ETS 接口 | `groupId: string = ''`（可选参数，默认空串） | 高 | `frameworks/ets/ets/application/UIAbilityContext.ets:780` |
| 源码 | `ets_ability_context.cpp:62,811,2775` — ANI 绑定 | groupId 已在 ANI 注册中；长度校验 64 字节 | 高 | `frameworks/ets/ani/ui_ability/src/ets_ability_context.cpp` |
| 源码 | `js_ability_context.cpp:88,3079` — NAPI 绑定 | groupId 已在 NAPI 注册中；长度校验 64 字节 | 高 | `frameworks/native/ability/native/ability_runtime/js_ability_context.cpp` |
| 源码 | `ability_context_impl.cpp:1691-1704` — 错误码转换 | TransferSetAbilityInstanceInfoErr 映射 WSError→ability 错误码 | 高 | `frameworks/native/ability/ability_runtime/ability_context_impl.cpp:1691` |
| 用户确认 | 会话澄清 | 当前代码由提出人实现；不涉及窗口侧修改；API 26.1.0+ 确认 | 高 | 本会话 |

### 变更控制与下游条件

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增用户故事或仓/模块 | 重新评估复杂度和影响范围，重新确认基线 |
| AC 变更 | 修改可观察行为或错误码 | 重新审批基线和 Spec |
| API 变更 | 新增/修改 Public/System API | 触发设计审批 |
| Agent 探索范围变更 | 需要访问未列仓库/模块、禁止项或新知识源 | 停止探索，更新 Agent Scope Guard 后重新确认基线 |
| 非功能指标变更 | 性能/安全/兼容性阈值变化 | 重新确认验证计划 |
| 目标版本变更 | 交付版本调整 | 更新 proposal.target_release |

- [x] 所有 P0/P1 用户故事有可测试、可度量的 AC
- [x] 范围内外、影响范围、API 变化和不涉及项已确认
- [x] `proposal.target_release` 已确认
- [x] `manifest.profile` 已确认或明确 none
- [x] Agent Scope Guard 已基线化，未复制 Plan/Task 的实现文件清单
- [x] 基线结论为"通过"，且确认人和确认依据完整
- [x] 条件附录触发时，待澄清项和争议决策已关闭（澄清阻断已解决，附录关闭）
