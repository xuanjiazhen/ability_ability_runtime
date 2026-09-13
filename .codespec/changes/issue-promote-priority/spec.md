# 特性规格：ServiceExtension 优先级提升

> 本规格覆盖 ability_runtime 侧在 StartServiceExtensionAbility 路径中新增 `ohos.param.promotePriority` Want 参数解析、权限校验和 `ExtensionRecord.PromotePriority` 方法。范围限定：参数解析、新权限校验（权限名待定）、优先级通知链路及调用前置日志。RSS 侧适配、UIAbility 已有 PromotePriority 路径不在本规格范围内。

> **权限校验方式：** `ohos.param.promotePriority` 参数是否生效，通过校验调用方是否持有新增权限（权限名待定）来决定，**而非**直接判断调用方是否为系统应用或 SA。

> **优先级提升成功确认：** 优先级提升是否成功，通过在调用 RSS 接口（`ResSchedUtil::PromotePriorityToRSS`）的位置补充**前置日志打印**来确认——即在调用 RSS 接口前打印日志，以此作为 AMS 侧已发起优先级提升通知的验证依据。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | ServiceExtension 优先级提升 |
| 特性编号 | REQ-PROMOTE-PRIORITY-001 |
| 所属 Epic | 息屏刷卡快速响应 |
| 优先级 | P1 |
| 目标版本 | 参见 proposal.md |
| SIG 归属 | Sig-Ability |
| 状态 | Draft |
| 复杂度 | 简单 |

## 本次变更范围（Delta）

本特性为新增能力（lineage: new）。当前 `StartExtensionAbility` 路径无优先级提升能力，而 UIAbility 已有完整 `PromotePriority` 链路（`StartAbilityByCall` → `AbilityRecord.PromotePriority` → `ResSchedUtil::PromotePriorityToRSS`）。本特性通过新增 Want 参数 + 新权限校验 + RSS 调用前置日志，实现 ServiceExtension 的优先级提升。

| 类型 | 内容 | 说明 |
|------|------|------|
| ADDED | `ohos.param.promotePriority` Want 参数常量 | 新增系统级参数声明 |
| ADDED | 新增权限常量（权限名待定） | `permission_constants.h` 新增，用于校验 `promotePriority` 参数是否生效 |
| ADDED | `StartExtensionAbility` 路径 Want 参数解析 | 解析 `promotePriority=true` 并校验调用方持有新增权限（非系统应用/SA 直接判断） |
| ADDED | `ExtensionRecord::SetPromotePriority` / `PromotePriority` 方法 | 仿照 `AbilityRecord::PromotePriority` 模式，`PromotePriority` 内调用 RSS 接口前打印前置日志 |
| REUSED | `ResSchedUtil::PromotePriorityToRSS` | 复用已有 AMS → RSS 通知链路，无修改；调用前补充前置日志确认调用发生 |
| REUSED | `PermissionVerification::VerifyCallingPermission` | 复用已有权限校验函数，传入新增权限名 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Proposal | `D:\Code\SDD\proposal\20260808948787\05-proposal-P-01.md` | Approved |
| Design | 待产出 | Pending |

## 代码现状分析（关键发现）

基于 `ability_ability_runtime` 代码库验证：

### 发现 1：UIAbility 已有完整 PromotePriority 链路

`AbilityRecord::PromotePriority`（`ability_record.cpp:3533-3542`）实现：

```cpp
bool AbilityRecord::PromotePriority()
{
    if (IsStartedByCall() && GetPromotePriority()) {
        ResSchedUtil::GetInstance().PromotePriorityToRSS(
            uiAbilityProperty_->byCallCallerSaUid,
            uiAbilityProperty_->byCallCallerSaPid,
            GetAbilityInfo().bundleName, GetUid(), GetPid());
        return true;
    }
    return false;
}
```

- `SetPromotePriority`（:3514）从 `IPCSkeleton::GetCallingUid()/GetCallingPid()` 捕获 SA caller
- `GetPromotePriority`（:3528）读取 `uiAbilityProperty_->promotePriority` 标记
- `UIAbilityProperty` 结构体（`ability_record.h:934`）含 `bool promotePriority` + `int32_t byCallCallerSaUid` + `int32_t byCallCallerSaPid`

`ExtensionRecord` 继承自 `BaseExtensionRecord`，后者继承自 `AbilityRecord`（`base_extension_record.h:25`）。因此 `ExtensionRecord` 理论上可调用继承的 `PromotePriority`，但当前 `ExtensionRecord` 未设置 `UIAbilityProperty`，需新增独立实现。

### 发现 2：StartAbilityByCall 的 promotePriority 是方法参数非 Want 参数

`AbilityManagerService::StartAbilityByCall`（`ability_manager_service.cpp:10429`）签名：

```cpp
int StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
    const sptr<IRemoteObject> &callerToken, int32_t accountId,
    bool isSilent, bool promotePriority, bool isVisible, uint64_t specifiedFullTokenId);
```

`promotePriority` 是方法签名的 `bool` 参数，**非 Want 参数**。在 :10544 行被 AND-gate 校验：
```cpp
abilityRequest.promotePriority = PermissionVerification::GetInstance()->IsSACall() && promotePriority;
```

本特性改为通过 **Want 参数** `ohos.param.promotePriority` 传递，与 `StartAbilityByCall` 的方法参数模式不同，但权限校验逻辑可复用。

### 发现 3：ExtensionRecord 无优先级方法

`ExtensionRecord`（`extension_record.h:32`）方法：`GetCallToken` / `GetRootCallerToken` / `SetRootCallerToken` / `GetFocusedCallerToken` / `SetFocusedCallerToken` / `ContinueToGetCallerToken` / `Update` / `UnloadUIExtensionAbility` / `RegisterStateObserver`。无 `PromotePriority` / `SetPromotePriority`。

需在 `ExtensionRecord` 或 `BaseExtensionRecord` 中新增。

### 发现 4：ResSchedUtil::PromotePriorityToRSS 可直接复用

`ResSchedUtil::PromotePriorityToRSS`（`res_sched_util.h:63`，`res_sched_util.cpp:241-257`）签名：

```cpp
void PromotePriorityToRSS(int32_t callerUid, int32_t callerPid,
    const std::string &targetBundleName, int32_t targetUid, int32_t targetPid);
```

内部调用 `ResourceSchedule::ResSchedClient::GetInstance().ReportData()` 传递 `RES_TYPE_SA_PULL_APP_IDENTIFIER`（:247），受 `#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE` 保护。**无需修改，可直接复用**。

### 发现 5：ohos.param.promotePriority 常量不存在

全库搜索 `ohos.param.promotePriority` 返回零匹配。现有 `ohos.param.*` 常量示例：
- `ability_manager_service.h:101` — `KEY_SESSION_ID = "com.ohos.param.sessionId"`
- `ability_command.h:232` — `DEBUG_FROM = "ohos.param.debugFrom"`

需新增 `ohos.param.promotePriority` 常量声明。

### 发现 6：权限校验应使用新增权限而非系统应用/SA 判断

Proposal 称"通过 callingUid 判定系统应用"，实际代码库 `StartAbilityByCall` 在 :10544 使用 `IsSACall()` 校验。但本特性规格调整为：**通过校验调用方是否持有新增权限（权限名待定）来决定 `promotePriority` 参数是否生效**，而非直接判断是否为系统应用或 SA。

可复用的权限校验函数：
- `PermissionVerification::VerifyCallingPermission`（`permission_verification.h:46`，`permission_verification.cpp:59-75`）— 通用权限校验，传入权限名返回 bool
- `permission_constants.h`（:25）— 已有 `PERMISSION_CONNECT_AGENT` 等权限常量模式，新增权限常量在此文件声明

> **与 UIAbility 路径的区别：** UIAbility 的 `StartAbilityByCall` 使用 `IsSACall()` 校验，本特性 ServiceExtension 路径改用 `VerifyCallingPermission(NEW_PERMISSION_TBD)` 校验，权限粒度更细。

### 发现 7：ResSchedUtil 调用位置可补充前置日志确认

`ResSchedUtil::PromotePriorityToRSS`（`res_sched_util.cpp:241-257`）内部调用 `ResSchedClient::GetInstance().ReportData()`，受 `#ifdef RESOURCE_SCHEDULE_SERVICE_ENABLE` 保护。当前该函数内部无前置日志（仅有 `HITRACE` 标记）。

本特性在 `ExtensionRecord::PromotePriority` 调用 `PromotePriorityToRSS` **之前**打印前置日志（TAG_LOGI），以此作为"AMS 侧已发起优先级提升通知"的验证依据。优先级提升是否成功，通过该前置日志确认。

### 发现 8：StartExtensionAbility 是 AMS 入口

`AbilityManagerService::StartExtensionAbility`（`ability_manager_service.cpp:4344`）是 AMS 侧入口，`StartServiceExtensionAbility` 是上下文层方法（`service_extension_context.cpp:175`），经 IPC 调用到达 AMS。Want 参数解析应在 `StartExtensionAbility` 或其 Inner 方法中执行。

## 用户故事

### US-1: 持有权限的应用指定高优先级启动 ServiceExtension

**作为** 持有新增权限（权限名待定）的应用开发者
**我想要** 在启动 ServiceExtension 时通过 Want 参数指定高优先级
**以便** 关键场景（如息屏刷卡）下 ServiceExtension 能快速启动

### US-2: 未持有权限的应用被拒绝

**作为** 安全负责人
**我想要** 未持有新增权限的应用无法使用高优先级参数
**以便** 防止优先级提升被滥用

### US-3: 原有行为不变

**作为** 应用开发者
**我想要** 原有 startServiceExtensionAbility 调用行为不变
**以便** 避免回归问题

### US-4: AMS 通知 RSS 并可验证

**作为** AMS 开发者
**我想要** ExtensionRecord.PromotePriority 调用 ResSchedUtil 通知 RSS，且在调用 RSS 接口前打印前置日志
**以便** 通过日志确认 AMS 侧已发起优先级提升通知，RSS 侧对目标进程执行优先级提升

## 验收标准 (AC)

| AC 编号 | 验收标准 | 类型 | 关联 US |
|---------|----------|------|---------|
| AC-1 | WHEN 持有新增权限（权限名待定）的调用方传入 `ohos.param.promotePriority=true` 调用 `StartExtensionAbility` THEN AMS 解析参数，`VerifyCallingPermission(NEW_PERMISSION_TBD)` 校验通过后触发 `ExtensionRecord.PromotePriority` 链路 | 正常 | US-1 |
| AC-2 | WHEN 未持有新增权限的调用方传入 `ohos.param.promotePriority=true` THEN AMS 忽略该参数，行为与未传参数一致 | 异常 | US-2 |
| AC-3 | WHEN `ExtensionRecord.PromotePriority` 被调用 THEN 在调用 `ResSchedUtil::PromotePriorityToRSS` 前打印前置日志（TAG_LOGI），随后 `PromotePriorityToRSS` 被调用，参数完整（callerUid/callerPid/targetBundleName/targetUid/targetPid） | 正常 | US-4 |
| AC-4 | WHEN 未传 `promotePriority` 参数 THEN `ExtensionRecord` 不调用 `PromotePriority`，行为不变 | 兼容 | US-3 |
| AC-5 | WHEN 进程销毁后 THEN RSS 不再保持高优先级（AMS 侧已发通知） | 边界 | US-4 |

## 验收追溯

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1 | R-1 | 单测 + 集成 | `ability_manager_service_test/`（持有新增权限 + promotePriority=true） |
| AC-2 | R-2 | 权限校验单测 | `ability_manager_service_test/`（未持有新增权限 + promotePriority=true） |
| AC-3 | R-3 | 单测 + 日志验证 | `extension_record_test/` + `res_sched_util_test/`（断言前置日志 + RSS 调用参数） |
| AC-4 | R-4 | 回归测试 | `ability_manager_service_test/` |
| AC-5 | R-5 | 集成测试 | 进程销毁后优先级恢复验证 |

## 规则定义

| 规则 ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联 AC |
|---------|------|----------|----------|-----------|---------|
| R-1 | 行为 | 持有新增权限（权限名待定）+ Want 含 `ohos.param.promotePriority=true` | `StartExtensionAbility` 解析参数，`VerifyCallingPermission(NEW_PERMISSION_TBD)` 校验通过，设置 `ExtensionRecord` 优先级标记并调用 `PromotePriority` | 参数级权限管控，未持有权限时忽略 | AC-1 |
| R-2 | 异常 | 未持有新增权限 + Want 含 `ohos.param.promotePriority` | 忽略该参数，行为与未传参一致 | `VerifyCallingPermission` 返回 false 时跳过 PromotePriority | AC-2 |
| R-3 | 行为 | `ExtensionRecord.PromotePriority` 被调用 | 调用 `ResSchedUtil::PromotePriorityToRSS` **前**打印前置日志（TAG_LOGI，含 callerUid/callerPid/bundleName/targetUid/targetPid），随后调用 `PromotePriorityToRSS(callerUid, callerPid, bundleName, uid, pid)` | 复用已有 AMS → RSS 通知链路，受 `RESOURCE_SCHEDULE_SERVICE_ENABLE` 保护；前置日志用于确认优先级提升通知已发起 | AC-3 |
| R-4 | 兼容 | Want 不含 `promotePriority` 参数 | `ExtensionRecord` 不调用 `PromotePriority` | 旧接口行为逐行不变 | AC-4 |
| R-5 | 边界 | Extension 进程销毁 | RSS 不再保持高优先级 | AMS 侧已通过 `OnProcessDied` 通知 | AC-5 |

## 接口规格

### Want 参数

| 属性 | 值 |
|------|-----|
| 参数键 | `ohos.param.promotePriority` |
| 参数类型 | `bool` |
| 默认值 | `false`（未传参时） |
| 开放范围 | 持有新增权限（权限名待定）的应用可用，未持有权限的应用忽略 |
| 声明位置 | `ability_manager_service.h` 新增常量 |

### 新增权限

| 属性 | 值 |
|------|-----|
| 权限名 | 待定（TBD） |
| 级别 | 待定（建议 `system_basic` 或 `system_core`） |
| 授权方式 | ACL |
| 常量声明 | `permission_constants.h` 新增 `PERMISSION_PROMOTE_PRIORITY_TBD` |
| 校验方式 | `PermissionVerification::VerifyCallingPermission(PERMISSION_PROMOTE_PRIORITY_TBD)` |

### ExtensionRecord 新增方法

| 属性 | 值 |
|------|-----|
| 方法 | `void ExtensionRecord::SetPromotePriority(bool enable, int32_t callerUid, int32_t callerPid)` |
| 方法 | `bool ExtensionRecord::PromotePriority()` |
| 仿照 | `AbilityRecord::SetPromotePriority` / `AbilityRecord::PromotePriority` |
| 实现 | `PromotePriority` 内先打印前置日志（`TAG_LOGI`，含 callerUid/callerPid/bundleName/targetUid/targetPid），再调用 `ResSchedUtil::GetInstance().PromotePriorityToRSS(callerUid, callerPid, GetAbilityInfo().bundleName, GetUid(), GetPid())` |
| 日志目的 | 前置日志用于确认 AMS 侧已发起优先级提升通知，作为优先级提升是否成功的验证依据 |

### 行为场景

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | 持有新增权限 + promotePriority=true | 解析参数 → 权限校验通过 → PromotePriority 链路触发 | AC-1 |
| 2 | 未持有新增权限 + promotePriority=true | 忽略参数，行为不变 | AC-2 |
| 3 | PromotePriority 调用 | 前置日志打印 → ResSchedUtil::PromotePriorityToRSS 发出 | AC-3 |
| 4 | 无 promotePriority 参数 | 不调用 PromotePriority | AC-4 |
| 5 | 进程销毁 | RSS 不再保持高优先级 | AC-5 |

## 兼容性声明

- **已有 API 行为变更:** 否。原有 `StartExtensionAbility` / `StartServiceExtensionAbility` 行为不变，新参数为可选 opt-in。
- **UIAbility PromotePriority 路径:** 不修改。UIAbility 的 `StartAbilityByCall` 路径保持不变。
- **IPC 序列化格式变更:** 否。Want 参数通过已有 Want 序列化机制传递，无新增 IPC。
- **配置文件格式变更:** 否。
- **最低支持版本:** 参见 proposal.md。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 参数级权限管控 | `ohos.param.promotePriority` 仅持有新增权限（权限名待定）的应用可用，未持有权限的应用忽略 | AC-2 |
| 权限校验非系统应用/SA 判断 | 通过 `VerifyCallingPermission(NEW_PERMISSION_TBD)` 校验，而非 `IsSACall()` 或 `IsSystemAppCall()` | AC-1, AC-2 |
| RSS 调用前置日志 | `PromotePriority` 调用 `PromotePriorityToRSS` 前打印前置日志，作为优先级提升已发起的验证依据 | AC-3 |
| 复用 ResSchedUtil | 不新增 RSS 通知接口，复用 `PromotePriorityToRSS` | AC-3 |
| ExtensionRecord 继承 AbilityRecord | 可复用 `AbilityRecord` 的 uid/pid 获取方法 | 全部 |
| RESOURCE_SCHEDULE_SERVICE_ENABLE | `PromotePriorityToRSS` 受此宏保护，未启用时为空操作 | AC-3 |
| opt-in 设计 | 不传参数时行为不变，无回归风险 | AC-4 |

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 可靠性 | 优先级提升不应导致 AMS 崩溃 | 压力测试 | `ability_manager_service_test/` |
| 安全 | 未持有新增权限的应用无法使用高优先级参数 | 权限校验单测 | — |
| DFX/可观测性 | RSS 调用前置日志可被检索，用于确认优先级提升已发起 | 日志验证 | `extension_record_test/`（断言前置日志输出） |
| 性能 | 不涉及 | — | — |

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
| 多用户 | 否 | 权限校验按调用方 Token，不依赖用户切换 | — |
| 版本升级 | 否 | 无持久化数据 | — |
| 生态兼容 | 是 | 新参数为可选 opt-in，旧应用行为不变 | AC-4 |

## Spec 自审清单

- [x] `promotePriority` 参数生效校验方式为新增权限（权限名 TBD 已在 Q-3 记录），非系统应用/SA 直接判断
- [x] 优先级提升成功确认方式为 RSS 调用前置日志，已体现在 AC-3 / R-3 / 架构约束 / 非功能性需求
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确（仅 ability_runtime 侧；RSS 侧适配不涉及）
- [x] 无语义模糊表述（权限名 TBD 已在 Q-3 显式记录并跟踪）
- [x] AC 与规则表交叉一致（AC-1~AC-5 ↔ R-1~R-5）
- [x] 规则表每条通过 5 项质量检查

## 已记录问题（不在本规格解决）

| # | 问题 | 影响 | 处理建议 |
|---|------|------|----------|
| Q-1 | RSS 侧是否已处理 `RES_TYPE_SA_PULL_APP_IDENTIFIER` 事件对 ServiceExtension 进程的覆盖 | 端到端效果待验证 | 不阻塞本规格，需与相关团队对齐 |
| Q-2 | `ohos.param.promotePriority` 常量声明位置——`ability_manager_service.h` 还是 Want 框架层 | 常量归属 | 建议在 `ability_manager_service.h` 声明，与现有 `KEY_SESSION_ID` 等同列 |
| Q-3 | 新增权限名待定（TBD）——用于校验 `promotePriority` 参数是否生效 | 权限定义/注册/ACL 配置 | 需与安全团队对齐权限名、级别（建议 `system_basic` 或 `system_core`）和授权方式，确定后在 `permission_constants.h` 声明常量并注册到权限框架 |

## context-references

```yaml
context-queries:
  - repo: "ability_ability_runtime"
    query: "AbilityRecord::PromotePriority / SetPromotePriority 的完整实现与 UIAbilityProperty 结构"
  - repo: "ability_ability_runtime"
    query: "ExtensionRecord / BaseExtensionRecord 类继承关系与现有方法"
  - repo: "ability_ability_runtime"
    query: "ResSchedUtil::PromotePriorityToRSS 实现与 RES_TYPE_SA_PULL_APP_IDENTIFIER"
  - repo: "ability_ability_runtime"
    query: "StartExtensionAbility / StartExtensionAbilityInner 的 Want 参数解析流程"
  - repo: "ability_ability_runtime"
    query: "PermissionVerification::VerifyCallingPermission 实现与权限校验模式"
  - repo: "ability_ability_runtime"
    query: "permission_constants.h 权限常量定义模式（如 PERMISSION_CONNECT_AGENT）"
  - repo: "ability_ability_runtime"
    query: "ResSchedUtil::PromotePriorityToRSS 调用位置的日志打印模式"
```

**关键文档：**
- `services/abilitymgr/include/ability_record.h`（`PromotePriority` :851，`UIAbilityProperty` :934）
- `services/abilitymgr/src/ability_record.cpp`（`PromotePriority` :3533，`SetPromotePriority` :3514）
- `services/abilitymgr/include/extension_record/extension_record.h`（:32）
- `services/abilitymgr/include/extension_record/base_extension_record.h`（:25，继承 AbilityRecord）
- `services/common/include/res_sched_util.h`（`PromotePriorityToRSS` :63）
- `services/common/src/res_sched_util.cpp`（:241-257）
- `services/abilitymgr/src/ability_manager_service.cpp`（`StartExtensionAbility` :4344，`StartAbilityByCall` :10429）
- `services/common/include/permission_verification.h`（`VerifyCallingPermission` :46）
- `services/common/src/permission_verification.cpp`（:59-75）
- `services/common/include/permission_constants.h`（权限常量模式，:25）
