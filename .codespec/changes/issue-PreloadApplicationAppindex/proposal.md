---
target_release: "OpenHarmony 7.1"
feature_id: "FEAT-30095352"
proposal_id: "P-01"
rr_id: "RR-30095352"
ac_refs:
  - AC-1
  - AC-2
  - AC-3
  - AC-4
  - AC-5
  - AC-6
issue: ""
author: "金森钧"
date: "2026-09-07"
status: Draft
gate_a: ""
---
# Proposal

## 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | FEAT-30095352 |
| Proposal ID | P-01 |
| RR单号 | RR-30095352 |

## 1. 背景与问题（Why）

对外 JS 接口 `appManager.preloadApplication`（frameworks/js/napi/app/js_app_manager/js_app_manager.cpp:1442 `OnPreloadApplication`）的 `appIndex` 参数已预留，JS 层已透传 `appIndex` 至 `IAppMgr::PreloadApplication`，但其服务端实现 `AppMgrServiceInner::PreloadApplication`（services/appmgr/src/app_mgr_service_inner.cpp:656）在 line 668 处 `if (appIndex != 0 && appIndex != PC_TABLET_INDEX) return ERR_INVALID_VALUE` 阻断了 appIndex ∈ [1, 1000] 的普通分身预加载（PC_TABLET_INDEX=10000 已放行）。此外 `AppPreloader::GetBundleAndHapInfo` 在 appIndex > 0 时仍调用 `GetBundleInfo` 获取主应用 BundleInfo，未使用 `GetCloneBundleInfo` 获取分身信息。分身应用启动时无法复用预加载进程，每次冷启动走完整启动流程，启动速度慢于主应用。

> **路径澄清**：JS 接口 `appManager.preloadApplication` 的调用链为 JsAppManager → IAppMgr(AppMgr SA 501) → AppMgrServiceInner::PreloadApplication（line 656）。`PreloadManagerService::PreloadApplication`（services/abilitymgr/，AMS SA 180 入口 AbilityManagerClient::PreloadApplication）是另一条独立的预加载路径，不在本需求 JS 接口调用链上。

需要移除 line 668 对 appIndex ∈ [1, 1000] 的阻断并适配分身场景的查询逻辑（`GetBundleAndHapInfo` 在 appIndex > 0 时调用 `GetCloneBundleInfo`），使分身应用享受与主应用同等的预加载启动加速。

## 2. 初始分级判断

> L0/简单变更：非目标等章节可简化为一句话；L1+ 建议完整填写。

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 复杂度 | 简单（≤5 人月） | 端到端总工作量 1.4 人月 |
| 涉及仓数量 | 1（ability_runtime） | 04-feature.md §4 |
| 是否涉及 Public/System API | 否（签名不变） | JS 接口 appManager.preloadApplication 为 Public API，但 appIndex 参数已预留，本次仅移除服务端内部限制，不改 API 签名 |
| 是否涉及安全/权限关键路径 | 否 | 复用现有权限检查逻辑 |
| 是否跨 SIG | 否 | 仅涉及元能力 SIG |

## 3. 目标 / 非目标

### 目标

1. `AppMgrServiceInner::PreloadApplication`（app_mgr_service_inner.cpp:656）移除 line 668 的 `appIndex != 0 && appIndex != PC_TABLET_INDEX` 限制，支持 appIndex [0, 1000] ∪ {PC_TABLET_INDEX}
2. `AppPreloader::GetBundleAndHapInfo`（app_preloader.cpp）适配：appIndex > 0 时调用 `GetCloneBundleInfo` 获取分身 BundleInfo
3. `CheckPreloadAppRecordExist` 适配：按 appIndex 区分预加载记录去重
4. 分身预加载进程复用现有进程复用机制

### 非目标

接口签名变更、进程复用机制开发、性能指标量化、bundle_framework 代码修改。

## 4. 用户故事与能力（Capabilities）

| US# | As a... I want... so that... | 验收标准(AC) |
|-----|------------------------------|-------------|
| US-1 | 作为应用开发者，我希望通过 preloadApplication 传入 appIndex 预加载分身应用，使分身应用启动时可复用预加载进程 | AC-1, AC-4 |
| US-2 | 作为系统开发者，我希望不同 appIndex 的预加载记录互不冲突，支持同时预加载多个分身实例 | AC-2, AC-3, AC-6 |
| US-3 | 作为系统开发者，我希望 appIndex=0 的主应用预加载行为不受影响 | AC-5 |

## 5. 成功标准

| 标准 | 可观察指标 | 验证方式 |
|------|-----------|---------|
| 分身预加载成功 | 调用 preloadApplication(appIndex>0) 返回 ERR_OK，分身预加载进程存在 | 接口调用 + 进程列表检查 |
| 分身 AbilityInfo 正确 | 预加载进程加载的是分身应用信息 | 验证预加载进程的应用信息 |
| 分身记录去重正确 | 不同 appIndex 可分别预加载，不互相覆盖 | 预加载 appIndex=0 和 appIndex=1，两条记录均存在 |
| 进程复用成功 | 预加载后正式启动分身应用，跳过初始化流程 | 检查进程复用标志 |
| 向后兼容 | 不传 appIndex 时与修改前行为一致 | 回归测试主应用预加载 |
| 多分身预加载 | appIndex=1 和 appIndex=2 同时预加载均成功 | 并发预加载测试 |

## 6. 影响范围

| 子系统 | 仓库 | 模块/路径 | 影响类型 |
|--------|------|-----------|---------|
| 元能力 | ability_ability_runtime | frameworks/js/napi/app/js_app_manager/js_app_manager.cpp | JS 接口入口（已透传 appIndex，无需修改） |
| 元能力 | ability_ability_runtime | services/appmgr/src/app_mgr_service_inner.cpp | 核心修改（line 656/668 移除 appIndex 分身限制） |
| 元能力 | ability_ability_runtime | services/appmgr/src/app_preloader.cpp | 适配（GetBundleAndHapInfo 分身查询 GetCloneBundleInfo） |
| 元能力 | ability_ability_runtime | services/appmgr/include/app_preloader.h | 适配（GetBundleAndHapInfo 签名增加 appIndex） |
| 元能力 | ability_ability_runtime | interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.cpp | 无需修改（IPC 已透传 appIndex） |
| 元能力 | ability_ability_runtime | interfaces/inner_api/app_manager/src/appmgr/app_mgr_client.cpp | 无需修改 |
| 元能力 | ability_ability_runtime | interfaces/inner_api/app_manager/src/appmgr/app_mgr_stub.cpp | 无需修改 |
| 元能力 | ability_ability_runtime | services/abilitymgr/src/preload_manager_service.cpp | 不在本需求 JS 接口调用链上（AMS SA 180 独立路径） |

## 7. 假设与开放问题

### 假设

- 包管理的分身查询接口在 OpenHarmony 7.1 中已可用
- 分身预加载进程复用机制与主应用一致，无需额外开发

### 开放问题

- 无

## 8. 不涉及项确认（8 维）

| 维度 | 是否涉及 | 依据 |
|------|---------|------|
| 性能 | 否 | 用户明确不关注性能量化指标 |
| 安全/权限 | 否 | 复用现有权限检查逻辑 |
| 兼容性 | 是 | appIndex=0 行为保持不变，向后兼容 |
| API/SDK | 否 | JS 接口 appManager.preloadApplication 签名不变，appIndex 参数已预留，仅移除服务端内部限制 |
| IPC/跨进程 | 否 | IPC 调用链路已透传 appIndex |
| 构建/组件 | 否 | 不涉及构建系统变更 |
| 国际化/无障碍 | 否 | 不涉及 UI 交互 |
| 数据迁移 | 否 | 不涉及数据迁移 |
