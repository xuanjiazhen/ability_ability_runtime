# Want 系统参数传递治理 架构问题分析报告

**子系统**: AppExecFwk  
**模块**: ability_runtime  
**一级特性**: abilitymgr  
**二级特性**: want_param_governance  
**分析日期**: 2026-08-12  
**问题范围**: ability_runtime 中通过 Want 传递系统参数（caller 身份、user/进程控制等）被外部设置后下游当权威使用的安全风险  

---

## 背景

ability_runtime 大量存在"往 `Want` 塞特定 key 的系统参数、下游读取当权威"的反模式。Want 参数表是开放容器，调用方可任意设置，可被伪造。本报告全量识别此类代码点并给出治理方案。

规模：ability_runtime 中 Want 参数 API 调用 SetParam 833、Get\*Param 611、HasParameter 171、RemoveParam 273 处，其中绝大多数是正常业务参数；真正"系统参数被外部设置后下游当权威"的有 HIGH 8 条、MED 9 条、LOW 17+ 条（见末尾附表）。

## 根因

存在清洗边界 `UpdateCallerInfo`（update_caller_info_util.cpp:73-92），用 `IPCSkeleton::GetCallingTokenID/Uid/Pid` 覆写 `CALLER_TOKEN/UID/PID`，但：
1. **时机太晚**：`StartAbilityInner` 中拦截器链（ability_manager_service.cpp:1500）在 `UpdateCallerInfo`（:1552）**之前**执行，:1489 即先读 `CALLER_TOKEN` 传入权限校验；
2. **覆盖不全**：`ClearProtectedWantParam`（:416-442）不清洗 caller 身份 key，完全不覆盖 `SPECIFED_PROCESS_CALLER_PROCESS`、`NEED_DESTROY_TEMPLATE`、`KEY_WATERMARK_*`、`REQUEST_PROC_CODE`、`CALL_TO_FOREGROUND`。

---

## 第一章：业务流程问题

### BP-000: 清洗边界晚于拦截器链执行（流程顺序缺陷）

**优先级**: P0  
**代码位置**: `ability_manager_service.cpp:1489-1552`

**问题描述**:  
`StartAbilityInner` 中执行顺序为：
1. line 1489 `StartAbilityInfoWrap(param.want, ...)` — 仅清洗 `localWant` 副本，**不修改 `param.want`**；
2. line 1492 `param.want.GetIntParam(CALLER_TOKEN)` — 读**未清洗的原始 Want**；
3. line 1500 `interceptorExecuter_->DoProcess(param)` — 拦截器链处理**未清洗 Want**；
4. line 1552/1641 `UpdateCallerInfo(abilityRequest.want)` — **此刻才用 IPC 覆写 caller 身份**。

1489→1500 之间所有读 caller 身份的代码（含拦截器）读到的都是调用方可控值。这是本治理所有 HIGH 案例的共同前置条件。

**修改建议**:  
将 `UpdateCallerInfo`（及补全后的 `SanitizeExternalWant`）调用点前移到**入口处、拦截器链之前**，使拦截器链与权限校验读到的是 IPC 派生值。

---

## 第二章：业务规格问题

### SP-000: ClearProtectedWantParam 清洗覆盖不全（受保护 key 范围缺失）

**优先级**: P0  
**代码位置**: `update_caller_info_util.cpp:416-442`

**问题描述**:  
`ClearProtectedWantParam` 仅 RemoveParam 4 个 key：`CALLER_NATIVE_NAME`、`COMPONENT_STARTUP_NEW_RULES`、`ABILITY_UNIFIED_DATA_KEY`、`SPECIFIED_ABILITY_FLAG`。以下系统参数 key **全程无清洗**：
- 进程控制：`SPECIFED_PROCESS_CALLER_PROCESS`（app_mgr_service_inner.cpp:7467）、`NEED_DESTROY_TEMPLATE`（:695）、`PARAM_RESV_REQUEST_PROC_CODE`（:2457）、`PARAM_RESV_CALL_TO_FOREGROUND`（ability_manager_service.cpp:10477）；
- 水印：`KEY_WATERMARK_BUSINESS_NAME`、`KEY_IS_WATERMARK_ENABLED`（app_mgr_service_inner.cpp:1968-1969）。

这些 key 被下游直接读取并影响进程命名/生命周期/水印/前台行为，且无 IPC 上下文二次校验。

**修改建议**:  
建立"受保护系统参数 key 注册表"（单一清单），`SanitizeExternalWant` 据此清洗；caller 身份类用 IPC 覆写（给可信值），控制类 RemoveParam 或迁移至 `AbilityRequest` 字段。

---

## 第三章：业务设计模式问题

### AP-000: 系统身份/控制参数承载于 Want 参数表而非 AbilityRequest 字段

**优先级**: P1  
**代码位置**: 全仓 `Want::PARAM_RESV_CALLER_*`、`SPECIFED_PROCESS_CALLER_PROCESS`、`NEED_DESTROY_TEMPLATE` 等的使用点

**问题描述**:  
系统内部契约参数（caller 身份、进程控制开关）本应是结构化字段，却被塞进 `Want` 的开放 key-value 表。这使调用方可控的容器同时承载系统权威信息，"清洗边界"成为唯一防线，一旦覆盖不全或时机错位即被绕过（BP-000/SP-000）。`Want` 应只承载面向开发者的业务参数。

**修改建议**:  
系统身份/控制参数迁移至 `AbilityRequest`/`PreloadRequest` 的显式字段，入口从 IPC 上下文或受信源设置；`Want` 参数表只留业务参数。

### AP-001: 缺少受保护系统参数 key 注册表

**优先级**: P1  
**代码位置**: 无（缺失）

**问题描述**:  
当前受保护 key 散落于 `UpdateCallerInfo`、`ClearProtectedWantParam`、各 `RemoveParam` 调用点，无单一清单。新增系统参数时无强制登记，导致持续失控（如 `NEED_DESTROY_TEMPLATE`、`KEY_WATERMARK_*` 均未纳入清洗）。

**修改建议**:  
定义单一 `ProtectedWantParamRegistry`（枚举/数组 + 是否需 IPC 覆写的标记），`SanitizeExternalWant` 遍历注册表清洗；新增系统参数必须登记，加 CI/编译期检查。

---

### AP-002: caller 身份内部传播靠 Want 间复制而非 IPC 重新派生

**优先级**: P1  
**代码位置**: `insight_intent/insight_intent_execute_param.cpp:340-358`（UpdateInsightIntentCallerInfo）、`ability_record.cpp:3277-3289`（RecordSaCallerInfo）、`scene_board/ui_ability_lifecycle_manager.cpp:5000-5012`

**问题描述**:  
caller 身份（CALLER_TOKEN/UID/PID/BUNDLE_NAME）在内部链路以 **Want→Want 复制**方式传播，而非每跳从 `IPCSkeleton` 重新派生：
- `InsightIntentExecuteParam::UpdateInsightIntentCallerInfo`（:340-358）从入参 `wantParams` 读 caller 身份（fallback 0）直接 SetParam 进 `insightIntentParam`，**全程不调 IPCSkeleton**；
- `AbilityRecord::RecordSaCallerInfo`（:3277-3289）把 caller 身份从 Want 复制进 SA 上报 Want；
- `UiAbilityLifecycleManager`（:5000-5012）转发 Want 时复制 caller 身份。

这使伪造值沿链路传播：即使入口 `SanitizeExternalWant` 清洗了原始 Want，这些**复制节点**若从其他未清洗 Want（如 dialog caller want、insight intent want）继承，则伪造身份继续扩散。属于 AP-000（系统身份承载于 Want）的具体 propagation 反模式。

**修改建议**:  
caller 身份的内部传递改为读 `AbilityRequest` 结构化字段（入口由 IPC 派生写入），而非从 Want 复制；确需在 Want 间转移时，每跳用 `IPCSkeleton::GetCalling*` 重新校验/派生，不信任入参 Want 的 caller 身份。

---

## 第四章：业务DFX设计问题

（无独立 DFX 问题。治理后建议在 `SanitizeExternalWant` 入口与未清洗 key 命中处打 INFO 日志，便于审计哪些 key 被覆写/移除，辅助故障定位。）

---

## 第五章：业务安全设计问题

### SC-000: CALLER_TOKEN 在清洗前被读取做权限校验

**优先级**: P0  
**代码位置**: `ability_manager_service.cpp:1492` → `start_ability_utils.cpp:537` → `permission_verification.cpp:62`

**问题描述**:  
`StartAbilityInner` 在 `UpdateCallerInfo`（:1552）**之前**于 :1492 读 `param.want.GetIntParam(CALLER_TOKEN)`，传入 `start_ability_utils.cpp:537` 的 `VerifyCallingPermission(PERMISSION_START_ABILITY_TO_PAGE, callerTokenId)`。`permission_verification.cpp:62` 当 `specifyTokenId != 0` 时**用传入值**（非 IPC token）做 `VerifyAccessToken`。攻击者伪造 `CALLER_TOKEN` 为有权限的 token 即可通过权限检查，使 ATOMIC_SERVICE_SHARE_ROUTER 等不被移除。

**修改建议**:  
- 权限校验点直接用 `IPCSkeleton::GetCallingTokenID()`，不读 Want；
- 入口处 `SanitizeExternalWant` 用 IPC token 覆写 `CALLER_TOKEN`，确保 :1492 读到可信值（纵深防御，二者皆需）。

### SC-001: 伪装同 bundle 绕过 ERMS 生态规则检查

**优先级**: P0  
**代码位置**: `ecological_rule_interceptor.cpp:89`（NoNeedErms）、`:105`（DoProcess）、`:153`（QueryAtomicServiceStartupRule）

**问题描述**:  
拦截器在 ability_manager_service.cpp:1500 执行（Want 未清洗）。三处逻辑均为：`CALLER_BUNDLE_NAME == 目标 bundleName` 则 `return true`/`ERR_INVALID_CALLER` **短路跳过 ERMS 生态规则**。`CALLER_BUNDLE_NAME` 是调用方可控值，攻击者设其=目标包名即可伪装"同 bundle"绕过生态管控与原子服务启动规则。

**修改建议**:  
同 bundle 判定改用从 `abilityRecord` 派生的真实 caller bundle（`UpdateCallerInfo:400` 已有派生逻辑），不读 Want；或入口清洗后覆写为派生值。

### SC-002: 绕过跨应用启动限制

**优先级**: P1  
**代码位置**: `start_other_app_interceptor.cpp:137`（CheckStartOtherApp）

**问题描述**:  
`CheckStartOtherApp` 用 `CALLER_BUNDLE_NAME != 目标包名` 判断是否跨应用启动。拦截器在 :1500 执行（未清洗），伪造 `CALLER_BUNDLE_NAME=目标包名` 则返回 false→line 55 `return ERR_OK` 放行。当 `persist.sys.abilityms.support.start_other_app=false` 时可绕过跨应用启动限制。

**修改建议**:  
同 SC-001，改用 `abilityRecord` 派生的真实 caller bundle。

### SC-003: ERMS 上报伪造调用方身份三元组

**优先级**: P1  
**代码位置**: `ecological_rule_interceptor.cpp:292-294`（InitErmsCallerInfo）

**问题描述**:  
`callerInfo.packageName = want.GetStringParam(CALLER_BUNDLE_NAME)`（**无 IPC fallback**）、`callerInfo.uid = want.GetIntParam(CALLER_UID, IPCSkeleton::GetCallingUid())`、`callerInfo.pid = want.GetIntParam(CALLER_PID, IPCSkeleton::GetCallingPid())`。后两者有 fallback 但**仅当 key 不存在时**；攻击者设了值则用攻击者值。这些上报 ERMS 影响生态路由决策。

**修改建议**:  
三元组一律改用 `IPCSkeleton::GetCallingUid/Pid` + 从 `abilityRecord` 派生的 bundle，不读 Want。

### SC-004: SPECIFED_PROCESS_CALLER_PROCESS 进程名外部可控

**优先级**: P1  
**代码位置**: `app_mgr_service_inner.cpp:7467`

**问题描述**:  
`processName = want.GetStringParam(SPECIFED_PROCESS_CALLER_PROCESS)` 直接用作应用进程名（:7468 RemoveParam）。该 key **全程无清洗**（不在 `UpdateCallerInfo`/`ClearProtectedWantParam` 中）。当目标 ability 配置 specifiedProcess 时，外部调用方可控进程名，影响进程命名隔离与归属。

**修改建议**:  
迁移至 `AbilityRequest` 显式字段，入口从受信源（abilityInfo/系统配置）设置；或纳入注册表用 IPC/受信值覆写。

### SC-005: NEED_DESTROY_TEMPLATE 未清洗影响模板进程生命周期

**优先级**: P2  
**代码位置**: `app_mgr_service_inner.cpp:695-698`

**问题描述**:  
`request.needDestroyTemplate = want.GetBoolParam(NEED_DESTROY_TEMPLATE, false)` 决定模板进程是否在镜像创建后销毁。该 key 无清洗、无 IPC 校验，外部可控，影响进程生命周期（可能保留本应销毁的模板进程，或误销毁）。

**修改建议**:  
纳入受保护 key 注册表 RemoveParam；或迁移至 `PreloadRequest` 字段由系统侧设置。

### SC-006: KEY_WATERMARK_* 未清洗影响水印显示

**优先级**: P2  
**代码位置**: `app_mgr_service_inner.cpp:1968-1969`（MakeKiaProcess）

**问题描述**:  
`watermarkBusinessName = want->GetStringParam(KEY_WATERMARK_BUSINESS_NAME)`、`isWatermarkEnabled = want->GetBoolParam(KEY_IS_WATERMARK_ENABLED, false)`。该 key 由 `IKiaInterceptor` 回调后写入 Want（半受信），但仍可被外部直接构造。无清洗、无校验，影响水印显示（非安全决策但可被外部操控）。

**修改建议**:  
`IKiaInterceptor` 回调结果应通过结构化返回值（非 Want 参数）传递；或纳入注册表限仅受信回调可写。

### SC-007: PARAM_RESV_REQUEST_PROC_CODE 未清洗影响进程复用

**优先级**: P2  
**代码位置**: `app_mgr_service_inner.cpp:2457-2459`（`requestProcCode != 0` 则 SetRequestProcCode + OnProcessReused）、`:4814` 存入 appRecord

**问题描述**:  
`REQUEST_PROC_CODE` 无清洗、无 IPC 校验，外部可控，影响进程复用决策与状态观察者通知。

**修改建议**:  
纳入注册表；或迁移至 `AbilityRequest` 字段。

### SC-008: PARAM_RESV_CALL_TO_FOREGROUND 未清洗影响前台行为

**优先级**: P2  
**代码位置**: `ability_manager_service.cpp:10477`（`want.GetBoolParam(PARAM_RESV_CALL_TO_FOREGROUND, false)` 决定 `isWithUI`）、`ability_record.cpp:2590`

**问题描述**:  
该 key 无清洗，影响 UI 可见性/前台行为。

**修改建议**:  
纳入注册表；或迁移至 `StartOptions` 结构化字段。

---

### SC-009: insight intent 从入参 Want 复制 caller 身份（无 IPC 派生，外部可达）

**优先级**: P1  
**代码位置**: `insight_intent/insight_intent_execute_param.cpp:231-258`（GenerateFromWant）→ `:340-358`（UpdateInsightIntentCallerInfo）；调用方 `ability_manager_service.cpp:14809`（ExecuteInsightIntent IPC 路径）、`free_install_manager.cpp:909`

**问题描述**:  
`InsightIntentExecuteParam::GenerateFromWant(const AAFwk::Want &want, ...)` 从入参 `want` 生成 intent 执行参数时，调用 `UpdateInsightIntentCallerInfo`（:256）将 `CALLER_TOKEN/UID/PID/BUNDLE_NAME` 从 `wantParams`（入参 Want 的 params）**直接复制**进 `insightIntentParam`（:343-357，RemoveParam 后 SetParam，fallback 0/空），**全程不调 `IPCSkeleton`**。该函数被 `ability_manager_service.cpp:14809` 的 `ExecuteInsightIntent`（对外 IPC，入参 Want 由调用方构造）调用 → 外部调用方可伪造 `CALLER_TOKEN/UID/PID/BUNDLE_NAME`，伪造身份被记录到 intent 执行参数，影响下游 intent 执行的鉴权/审计。

**修改建议**:  
`UpdateInsightIntentCallerInfo` 改为用 `IPCSkeleton::GetCallingTokenID/Uid/Pid` + `abilityRecord` 派生的 bundle 重新设置，不从入参 Want 复制；或确保 `GenerateFromWant` 调用前入参 Want 已经 `SanitizeExternalWant` 清洗（但纵深防御优先前者）。

---

## 治理方案

核心原则：**Want 不应承载系统身份/控制权威**。身份类一律用 IPC 上下文，控制类走 `AbilityRequest` 结构体字段，Want 只留业务参数。

### 步骤 1 — 建立单一、最早、完整的清洗边界
- 在每个对外 Want 入口（`AbilityManagerStub::StartAbility/ConnectAbility` 等桩）调用一次 `SanitizeExternalWant(want)`，**先于任何拦截器/读取**。
- `SanitizeExternalWant` = 合并 `UpdateCallerInfo` + `ClearProtectedWantParam`，补全覆盖：所有 `PARAM_RESV_CALLER_*` + `SPECIFED_PROCESS_CALLER_PROCESS` + `NEED_DESTROY_TEMPLATE` + `KEY_WATERMARK_*` + `REQUEST_PROC_CODE` + `CALL_TO_FOREGROUND`。
- caller 身份 key 用 `IPCSkeleton` **覆写**（给可信值而非空值，因下游依赖这些值）。

### 步骤 2 — 清洗调用点前移到拦截器链之前
- 修复 BP-000：`ability_manager_service.cpp` 将 `UpdateCallerInfo` 调用从 :1552/1641 前移到入口处（拦截器链 :1500 之前）。HIGH 案例 SC-000/001/002/003 自然消解。

### 步骤 3 — HIGH 安全决策点改用 IPC 上下文（纵深防御）
- `ecological_rule_interceptor.cpp:292-294`、`start_other_app_interceptor.cpp:137`、`ability_manager_service.cpp:1492` 等做安全决策的读取，直接改用 `IPCSkeleton::GetCallingUid/Pid/GetCallingTokenID()` + `abilityRecord` 派生 bundle，不读 Want。即使 Want 已清洗，安全决策也不应二次信任中间数据。

### 步骤 4 — 系统控制参数迁出 Want
- `SPECIFED_PROCESS_CALLER_PROCESS`、`NEED_DESTROY_TEMPLATE`、`REQUEST_PROC_CODE`、`KEY_WATERMARK_*` 迁移至 `AbilityRequest`/`PreloadRequest` 显式字段，入口从受信源设置。`Want` 只留业务参数。

### 落地优先级
1. **步骤 1+2**（前移并补全清洗边界）：收益最大、风险最低，先做。修复 BP-000/SP-000 + SC-000~004 的前置条件。
2. **步骤 3**（决策点改 IPC）：次之，消除最严重绕过。
3. **步骤 4**（迁出 Want）：改动面最大，长期重构。配合 AP-001 的注册表机制防止再失控。

### 附：MED/LOW 速查（非独立编号）

- MED（已清洗或有 IPC fallback 但设计脆弱）：`ability_record.cpp:2894/2906`（GrantUriPermission，UID 有 fallback）、`:3274`（DMS_UID 检查从 Want 读）、`update_caller_info_util.cpp:280-282`（dialogCallerWant 清洗前读取）。
- LOW（已清洗或非安全决策）：`PARAM_RESV_WINDOW_MODE`/`DISPLAY_ID`（UI）、`PARAM_ABILITY_RECOVERY_RESTART`（恢复）、`PARAM_RESV_APP_REQUEST_ID`/`SCB_REQUEST_ID`（操作性，读后即删）、`PARAM_RESV_CALLER_NATIVE_NAME`/`CALLER_ABILITY_NAME`/`CALLER_APP_CLONE_INDEX`/`CALLER_APP_IDENTIFIER`（UpdateCallerInfo 派生覆写后读取）。

---

## 问题统计

| 故障类型 | P0 | P1 | P2 | P3 | 合计 |
|---------|-----|-----|-----|-----|------|
| 业务流程 | 1 | 0 | 0 | 0 | 1 |
| 业务规格 | 1 | 0 | 0 | 0 | 1 |
| 设计模式 | 0 | 3 | 0 | 0 | 3 |
| DFX | 0 | 0 | 0 | 0 | 0 |
| 安全 | 2 | 4 | 4 | 0 | 10 |
| 其他 | 0 | 0 | 0 | 0 | 0 |
| **合计** | **4** | **7** | **4** | **0** | **15** |
