# 特性规格：隐式拉起 Extension 系统应用优先消费适配

> 本规格覆盖 ability_ability_runtime 侧对 bundlemanager_bundle_framework 新增"系统应用优先于三方应用"排序逻辑的消费适配。范围限定：识别全部 `QueryExtensionAbilityInfos` / `GetSandboxExtAbilityInfos` 返回结果的 `[0]` / `.front()` 消费点，确认新排序在隐式拉起链路端到端生效，保证系统应用不被三方应用拦截（AC-5）。排序逻辑本身的实现不在本规格范围内（属 bundlemanager_bundle_framework 仓）。

> **关键判断：** ability_runtime 侧无新增排序代码——排序由 bundle framework 在 `QueryExtensionAbilityInfos` / `GetSandboxExtAbilityInfos` 接口返回前完成。ability_runtime 侧的适配本质是"确认消费侧 `[0]` 取值在新排序下行为正确 + 补充可观测性日志 + 补充验证测试"。本规格不考虑 priority 字段排序，仅关注"系统应用优先、三方应用在后"的二分排序。

> **隐式拉起拦截修复确认：** `skill_execute_manager.cpp:322` 取 `extensionInfos[0]` 作为匹配结果，新排序后 [0] 为系统应用（当系统应用与三方应用同时匹配时），端到端验证系统应用被拉起（AC-5）。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 隐式拉起 Extension 系统应用优先消费适配 |
| 特性编号 | FEAT-RR30092970-AMS |
| 所属 Epic | 隐式拉起 Extension 排序 |
| 优先级 | P1 |
| 目标版本 | OpenHarmony 7.1 |
| SIG 归属 | Sig-Ability |
| 状态 | Draft |
| 复杂度 | 简单 |

## 本次变更范围（Delta）

本特性为适配变更（lineage: adapt）。bundlemanager_bundle_framework 侧修改 `QueryExtensionAbilityInfos` 和 `GetSandboxExtAbilityInfos` 的返回排序逻辑，将系统应用排在三方应用之前。ability_ability_runtime 作为消费方需确认全部 `[0]` / `.front()` 取值点在新排序下行为正确，并补充可观测性日志与验证测试。

| 类型 | 内容 | 说明 |
|------|------|------|
| REUSED | `SkillExecuteManager::ResolveTargetType` 的 `extensionInfos[0]` 取值 | 排序由 BMS 完成，[0] 现为系统应用（系统应用与三方应用同时匹配时），无需修改取值逻辑，补充前置确认日志 |
| REUSED | `AppMgrServiceInner::CreateAbilityInfo` 的 `extensionInfos.front()` 取值 | 同上，沙箱与非沙箱路径均受益 |
| REUSED | `AppPreloader::GetAbilityInfo` 的 `extensionInfos.front()` 取值 | 同上，预加载路径同样受益 |
| REUSED | `AgentManagerService` 三处 `extensionInfos[0]` 取值 | 同上，Agent 连接路径同样受益 |
| ADDED | 隐式拉起匹配结果排序确认日志 | 在 `ResolveTargetType` 取 `extensionInfos[0]` 前打印匹配结果数量与首项 bundleName，用于确认排序生效 |
| ADDED | 验证测试用例 | 覆盖系统应用优先于三方应用的端到端验证 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Proposal | `D:\Code\SDD\proposal\20260905542521\proposal.md` | Draft |
| Design | 待产出 | Pending |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md，本文档不重复摘录。proposal.md 原述排序含 priority 维度，本规格按最新口径收敛为仅"系统应用优先于三方应用"。

## 代码现状分析（关键发现）

基于 `ability_ability_runtime` 代码库验证，共识别 6 处 `QueryExtensionAbilityInfos` / `GetSandboxExtAbilityInfos` 返回结果的 `[0]` / `.front()` 消费点：

### 发现 1：SkillExecuteManager::ResolveTargetType 直接取 [0] 决定拉起目标

`SkillExecuteManager::ResolveTargetType`（`skill_execute_manager.cpp:307-334`）实现：

```cpp
bool ret = IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(
    queryWant, flags, userId, extensionInfos));
if (ret && !extensionInfos.empty()) {
    auto type = extensionInfos[0].type;
    return type;
}
```

- 该函数在 `GenerateSkillWant`（`skill_execute_manager.cpp:136`）中被调用，用于判断目标是 Extension 还是 UIAbility
- `extensionInfos[0].type` 直接决定拉起的目标类型
- **新排序后**：当系统应用与三方应用同时匹配时 [0] 为系统应用，隐式拉起不再被三方应用拦截（AC-5）

### 发现 2：AppMgrServiceInner::CreateAbilityInfo 取 front() 创建 AbilityInfo

`AppMgrServiceInner::CreateAbilityInfo`（`app_mgr_service_inner.cpp:10195-10230`）实现：

```cpp
if (appIndex == 0) {
    IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(want, abilityInfoFlag,
        userId, extensionInfos));
} else {
    IN_PROCESS_CALL(bundleMgrHelper->GetSandboxExtAbilityInfos(want, appIndex,
        abilityInfoFlag, userId, extensionInfos));
}
AppExecFwk::ExtensionAbilityInfo extensionInfo = extensionInfos.front();
AbilityRuntime::StartupUtil::InitAbilityInfoFromExtension(extensionInfo, abilityInfo);
```

- 同时覆盖非沙箱（`QueryExtensionAbilityInfos`）和沙箱（`GetSandboxExtAbilityInfos`）两条路径
- `extensionInfos.front()` 等价于 `[0]`，新排序后取系统应用（当系统应用与三方应用同时匹配时）
- **沙箱路径 AC-3/AC-4 由此消费点覆盖**

### 发现 3：AppPreloader::GetAbilityInfo 取 front() 预加载

`AppPreloader::GetAbilityInfo`（`app_preloader.cpp:125-153`）实现：

```cpp
if (appIndex == 0) {
    IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(want, abilityInfoFlag,
        userId, extensionInfos));
} else {
    IN_PROCESS_CALL(bundleMgrHelper->GetSandboxExtAbilityInfos(want, appIndex,
        abilityInfoFlag, userId, extensionInfos));
}
AppExecFwk::ExtensionAbilityInfo extensionInfo = extensionInfos.front();
request.extensionProcessMode = extensionInfo.extensionProcessMode;
```

- 预加载路径同样取 `front()`，新排序后预加载目标为系统应用（当系统应用与三方应用同时匹配时）

### 发现 4：AgentManagerService 三处取 [0] 校验 Extension 类型

`AgentManagerService`（`agent_manager_service.cpp`）三处消费点：

| 行号 | 函数 | 取值 | 用途 |
|------|------|------|------|
| :785 | `PrepareStandardAgentConnectWant` | `extensionInfos[0].type` | 校验目标是否为 AGENT 类型 |
| :831 | `PrepareServiceConnectWant` | `extensionInfos[0].type` | 校验目标是否为 SERVICE 类型 |
| :907-911 | `ResolveLowCodeHostInfo` | `extensionInfos[0].type` + `extensionInfos[0].applicationInfo.uid` | 校验类型并获取 hostUid |

- 三处均取 [0] 判断 Extension 类型，新排序后类型判断基于系统应用（当系统应用与三方应用同时匹配时）

### 发现 5：CrowdTestInterceptor 仅判空不取 [0]

`CrowdTestInterceptor::DoProcess`（`crowd_test_interceptor.cpp:55-58`）实现：

```cpp
hasUIExtension = IN_PROCESS_CALL(bundleMgrHelper->QueryExtensionAbilityInfos(queryWant,
    flags, param.userId, extensionInfos));
hasUIExtension = hasUIExtension && !extensionInfos.empty();
```

- 仅判断结果是否非空，不取具体项，排序不影响行为

### 发现 6：ability_runtime 侧无排序逻辑

全库搜索 ability_ability_runtime 中无系统应用优先排序逻辑。排序完全依赖 bundle framework 在接口返回前完成。ability_runtime 侧为纯消费方。

### 发现 7：ResolveTargetType 无匹配结果确认日志

`ResolveTargetType`（`skill_execute_manager.cpp:323-329`）取 [0] 前仅打印 type，未打印匹配结果数量与首项 bundleName。新排序生效后，需补充日志以确认排序行为（可观测性）。

## 用户故事

### US-1: 隐式拉起系统应用不被三方拦截

**作为** 系统应用开发者
**我想要** 隐式拉起 Extension 时，系统应用和三方应用同时匹配时系统应用被拉起
**以便** 关键场景下系统应用不被三方应用拦截

### US-2: 沙箱环境下系统应用优先同样生效

**作为** 系统应用开发者
**我想要** 沙箱环境下隐式拉起时系统应用同样排在三方应用前面
**以便** 沙箱场景行为与主场景一致

### US-3: 预加载目标为系统应用

**作为** AMS 开发者
**我想要** 预加载 Extension 时取系统应用（当系统应用与三方应用同时匹配时）
**以便** 预加载的进程与实际拉起的目标一致

### US-4: 排序行为可观测

**作为** AMS 开发者
**我想要** 隐式拉起匹配时打印匹配结果数量与首项信息
**以便** 通过日志确认系统应用优先排序已生效

## 验收标准 (AC)

| AC 编号 | 验收标准 | 类型 | 关联 US |
|---------|----------|------|---------|
| AC-1 | WHEN `ResolveTargetType` 调用 `QueryExtensionAbilityInfos` 返回含系统应用与三方应用的结果 THEN `extensionInfos[0]` 为系统应用（非三方应用） | 正常 | US-1 |
| AC-2 | WHEN `ResolveTargetType` 返回结果全部为系统应用或全部为三方应用 THEN [0] 取值行为与改动前一致（排序不影响同类内部顺序） | 兼容 | US-1 |
| AC-3 | WHEN `CreateAbilityInfo` 走沙箱路径（appIndex != 0）调用 `GetSandboxExtAbilityInfos` 返回含系统应用与三方应用的结果 THEN `extensionInfos.front()` 为系统应用 | 正常 | US-2 |
| AC-4 | WHEN 沙箱路径返回结果全部为系统应用或全部为三方应用 THEN front() 取值行为与改动前一致 | 兼容 | US-2 |
| AC-5 | WHEN 系统应用与三方应用同时匹配隐式拉起 THEN ability_runtime 侧取 [0] 拉起系统应用，三方应用不被拉起 | 正常 | US-1 |
| AC-6 | WHEN `ResolveTargetType` 取 [0] 前打印匹配结果数量与首项 bundleName THEN 日志可被检索，用于确认系统应用优先排序生效 | 兼容 | US-4 |

## 验收追溯

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1 | R-1 | 单测 + 集成 | `skill_execute_manager_test/`（构造系统应用 + 三方应用同时匹配） |
| AC-2 | R-2 | 回归测试 | `skill_execute_manager_test/`（全部系统应用 / 全部三方应用） |
| AC-3 | R-3 | 单测 | `app_mgr_service_inner_test/`（沙箱路径构造系统应用 + 三方应用） |
| AC-4 | R-4 | 回归测试 | `app_mgr_service_inner_test/`（沙箱路径同类结果） |
| AC-5 | R-5 | 集成测试 | 系统应用和三方应用同时匹配时验证系统应用被拉起 |
| AC-6 | R-6 | 日志验证 | `skill_execute_manager_test/`（断言前置日志输出） |

## 规则定义

| 规则 ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联 AC |
|---------|------|----------|----------|-----------|---------|
| R-1 | 行为 | `ResolveTargetType` 调用 `QueryExtensionAbilityInfos` 返回含系统应用与三方应用的结果 | `extensionInfos[0]` 为系统应用 | 排序由 BMS 完成，AMS 侧取 [0] 即可；系统应用判断由 BMS IsSystemApp() 完成 | AC-1 |
| R-2 | 兼容 | 返回结果全部为系统应用或全部为三方应用 | [0] 取值行为与改动前一致 | 同类内部顺序不由本特性约束 | AC-2 |
| R-3 | 行为 | `CreateAbilityInfo` 走沙箱路径调用 `GetSandboxExtAbilityInfos` 返回含系统应用与三方应用的结果 | `extensionInfos.front()` 为系统应用 | 沙箱路径有 appIndex 参数，不涉及主应用/分身排序 | AC-3 |
| R-4 | 兼容 | 沙箱路径返回结果全部为系统应用或全部为三方应用 | front() 取值行为与改动前一致 | — | AC-4 |
| R-5 | 行为 | 系统应用与三方应用同时匹配隐式拉起 | ability_runtime 侧取 [0] 拉起系统应用 | 端到端由 BMS 排序 + AMS 取 [0] 共同保证 | AC-5 |
| R-6 | 可观测性 | `ResolveTargetType` 取 [0] 前 | 打印匹配结果数量与首项 bundleName（TAG_LOGD/TAG_LOGI），用于确认系统应用优先排序生效 | 日志需可区分系统应用/三方应用以便排查 | AC-6 |

## 接口规格

### 消费点清单

| # | 文件 | 行号 | 函数 | 取值方式 | 调用接口 | 路径类型 |
|---|------|------|------|----------|----------|----------|
| 1 | `skill_execute_manager.cpp` | :321-324 | `ResolveTargetType` | `extensionInfos[0].type` | `QueryExtensionAbilityInfos` | 非沙箱 |
| 2 | `app_mgr_service_inner.cpp` | :10211-10227 | `CreateAbilityInfo` | `extensionInfos.front()` | `QueryExtensionAbilityInfos` | 非沙箱 |
| 3 | `app_mgr_service_inner.cpp` | :10217-10227 | `CreateAbilityInfo` | `extensionInfos.front()` | `GetSandboxExtAbilityInfos` | 沙箱 |
| 4 | `app_preloader.cpp` | :133-149 | `GetAbilityInfo` | `extensionInfos.front()` | `QueryExtensionAbilityInfos` | 非沙箱 |
| 5 | `app_preloader.cpp` | :139-149 | `GetAbilityInfo` | `extensionInfos.front()` | `GetSandboxExtAbilityInfos` | 沙箱 |
| 6 | `agent_manager_service.cpp` | :785 | `PrepareStandardAgentConnectWant` | `extensionInfos[0].type` | `QueryExtensionAbilityInfos` | 非沙箱 |
| 7 | `agent_manager_service.cpp` | :831 | `PrepareServiceConnectWant` | `extensionInfos[0].type` | `QueryExtensionAbilityInfos` | 非沙箱 |
| 8 | `agent_manager_service.cpp` | :907-911 | `ResolveLowCodeHostInfo` | `extensionInfos[0].type` + `.applicationInfo.uid` | `QueryExtensionAbilityInfos` | 非沙箱 |

### 新增日志

| 属性 | 值 |
|------|-----|
| 位置 | `SkillExecuteManager::ResolveTargetType`（`skill_execute_manager.cpp:323` 取 [0] 前） |
| 内容 | 匹配结果数量 + 首项 bundleName |
| 级别 | TAG_LOGD（AAFwkTag::ABILITYMGR） |
| 目的 | 确认系统应用优先排序生效的可观测性依据 |

### 行为场景

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 隐式拉起，系统应用与三方应用同时匹配 | BMS 返回排序后结果，AMS 取 [0] 拉起系统应用 | AC-5 |
| 2 | 隐式拉起，结果全部为系统应用 | [0] 取值行为与改动前一致 | AC-2 |
| 3 | 隐式拉起，结果全部为三方应用 | [0] 取值行为与改动前一致 | AC-2 |
| 4 | 沙箱隐式拉起，系统应用与三方应用同时匹配 | front() 为系统应用 | AC-3 |
| 5 | 预加载 Extension | front() 为系统应用（当系统应用与三方应用同时匹配时） | US-3 |
| 6 | Agent 连接 Extension | [0] 用于类型校验 | — |
| 7 | ResolveTargetType 取 [0] | 前置日志打印匹配数量与首项 bundleName | AC-6 |

## 兼容性声明

- **已有 API 行为变更:** 否。ability_runtime 侧不修改任何公共 API 签名或 IPC 协议。排序变更由 BMS 侧完成，AMS 侧取 [0] 的消费方式不变。
- **返回排序行为变更:** 是。`QueryExtensionAbilityInfos` / `GetSandboxExtAbilityInfos` 返回顺序变更为系统应用优先于三方应用。ability_runtime 侧 [0] 取值的语义变化由 BMS 侧保证。
- **IPC 序列化格式变更:** 否。不新增 IPC code，不修改 IPC 协议。
- **配置文件格式变更:** 否。
- **最低支持版本:** OpenHarmony 7.1。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 排序由 BMS 完成 | ability_runtime 侧不实现排序逻辑，完全依赖 BMS 在接口返回前完成排序 | 全部 |
| [0] 取值不变 | AMS 侧消费方式（取 [0] / .front()）不修改，排序语义变化由 BMS 保证 | AC-1~AC-5 |
| 沙箱与非沙箱分离 | 沙箱路径走 `GetSandboxExtAbilityInfos`（有 appIndex），非沙箱走 `QueryExtensionAbilityInfos`，两条路径均受益 | AC-3/AC-4 |
| 可观测性 | 取 [0] 前补充匹配结果日志，用于确认系统应用优先排序生效 | AC-6 |
| FA/Stage 模型共存 | 消费点覆盖 FA 和 Stage 模型路径，修改时必须确认不影响另一模型 | 全部 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 可靠性 | 排序消费不应导致 AMS 崩溃 | 回归测试 | `ability_manager_service_test/` |
| 安全 | 系统应用不被三方应用拦截（隐式拉起） | 集成测试 | 系统应用 + 三方应用同时匹配场景 |
| DFX/可观测性 | 匹配结果日志可被检索，确认系统应用优先排序生效 | 日志验证 | `skill_execute_manager_test/` |
| 性能 | 不涉及（排序由 BMS 完成，AMS 侧无额外开销） | — | — |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 |
|----------|----------|-----------|
| 手机 | 无差异 | — |
| 平板 | 无差异 | — |
| 折叠屏 | 无差异 | — |

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | 无 UI 交互 | — |
| 多用户 | 否 | 排序消费按 userId 查询，不依赖用户切换 | — |
| 版本升级 | 否 | 无持久化数据 | — |
| 生态兼容 | 是 | 返回排序行为变更，需确认所有调用方兼容性 | AC-2/AC-4 |

## Spec 自审清单

- [x] 范围边界明确（仅 ability_runtime 侧消费适配；排序逻辑实现属 BMS 仓）
- [x] 全部 8 处 [0] / .front() 消费点已识别并列出
- [x] 沙箱与非沙箱路径均已覆盖（AC-3/AC-4）
- [x] 不涉及 priority 字段排序（已按最新口径收敛为仅"系统应用优先于三方应用"）
- [x] AC 使用 WHEN/THEN 格式，可独立测试
- [x] AC 与规则表交叉一致（AC-1~AC-6 ↔ R-1~R-6）
- [x] 无语义模糊表述（ability_runtime 侧无排序代码已明确）
- [x] 可观测性需求已体现（AC-6 / R-6）

## 已记录问题（不在本规格解决）

| # | 问题 | 影响 | 处理建议 |
|---|------|------|----------|
| Q-1 | BMS 侧排序是否已在 `QueryExtensionAbilityInfosOnlyWithTypeName`（picker 路径）中同步实现 | `ability_manager_service.cpp:4990` 的 picker 路径取 [0]，若 BMS 未同步排序则 picker 路径不受益 | 不阻塞本规格，需与 BMS 团队确认 `QueryExtensionAbilityInfosOnlyWithTypeName` 是否纳入排序范围 |
| Q-2 | `AgentManagerService` 三处 [0] 取值是否需补充日志 | Agent 路径的可观测性需求待确认 | 不阻塞本规格，Phase 2 开发时确认 |
| Q-3 | 同类内部（全系统应用或全三方应用）的排序顺序是否需确定性保证 | 当前同类内部顺序不由本特性约束，可能出现非确定性顺序 | 不阻塞本规格，建议与 BMS 团队确认是否需 bundleName 字母序作为同类内部 tiebreaker |

## context-references

```yaml
context-queries:
  - repo: "ability_ability_runtime"
    query: "SkillExecuteManager::ResolveTargetType 的 QueryExtensionAbilityInfos 调用与 [0] 取值"
  - repo: "ability_ability_runtime"
    query: "AppMgrServiceInner::CreateAbilityInfo 的 QueryExtensionAbilityInfos / GetSandboxExtAbilityInfos 调用与 front() 取值"
  - repo: "ability_ability_runtime"
    query: "AppPreloader::GetAbilityInfo 的 QueryExtensionAbilityInfos / GetSandboxExtAbilityInfos 调用与 front() 取值"
  - repo: "ability_ability_runtime"
    query: "AgentManagerService::PrepareStandardAgentConnectWant / PrepareServiceConnectWant / ResolveLowCodeHostInfo 的 [0] 取值"
  - repo: "ability_ability_runtime"
    query: "CrowdTestInterceptor 的 QueryExtensionAbilityInfos 调用（仅判空）"
  - repo: "ability_ability_runtime"
    query: "QueryExtensionAbilityInfosOnlyWithTypeName 的 [0] 取值（picker 路径）"
```

**关键文档：**
- `services/abilitymgr/src/skill/skill_execute_manager.cpp`（`ResolveTargetType` :307-334，`[0]` 取值 :324）
- `services/appmgr/src/app_mgr_service_inner.cpp`（`CreateAbilityInfo` :10195-10230，`front()` 取值 :10227）
- `services/appmgr/src/app_preloader.cpp`（`GetAbilityInfo` :125-153，`front()` 取值 :149）
- `agent_runtime_framework/services/agentmgr/src/agent_manager_service.cpp`（:785，:831，:907-911）
- `services/abilitymgr/src/interceptor/crowd_test_interceptor.cpp`（:55-58，仅判空）
- `services/abilitymgr/src/ability_manager_service.cpp`（picker 路径 :4990-5003）
