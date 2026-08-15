# Want 系统参数传递治理 — 现状架构

**子系统**: AppExecFwk  
**模块**: ability_runtime  
**一级特性**: abilitymgr  
**二级特性**: want_param_governance  
**分析日期**: 2026-08-12  

---

## 一、功能说明

ability_runtime 在组件调度（StartAbility/ConnectAbility 等）中大量使用 `AAFwk::Want` 携带参数。Want 的参数表（`WantParams`）是开放的 key-value 容器，外部调用方可在构造 Want 时塞入任意 key。其中存在一类**系统参数**：调用方身份（caller uid/pid/token/bundle name）、用户与进程控制（user id、window mode、display id、进程名、预加载/水印/销毁开关等）。这些参数本应由系统侧从 IPC 上下文（`IPCSkeleton::GetCallingUid/Pid/GetCallingTokenID`）派生，但实际却被塞进 Want 在各处读取，形成"调用方可控、下游当权威"的反模式。

## 二、架构设计

### 输入层（Who 构造/携带 Want 参数）

- **外部调用方**：应用/SA 通过 `AbilityManagerProxy::StartAbility(want, ...)` 等接口传入 Want，可任意设置 `Want::PARAM_RESV_*` 与自定义 key。
- **系统内部**：部分 key 由 `AbilityManagerService`/`AppMgrServiceInner` 在派发过程中内部塞入（如 `UpdateCallerInfo` 用 IPC 覆写 caller 身份、`NotifyAppStatus` 内部塞 `Constants::USER_ID`）。

### 服务层（关键模块）

| 模块 | 职责 | 与本治理的关联 |
|---|---|---|
| `AbilityManagerService` (SA 180) | 组件调度入口、拦截器链编排 | StartAbilityInner 调用清洗与拦截器的**顺序**是根因 |
| `UpdateCallerInfoUtil` | 清洗 Want 中的 caller 身份 key | 既有清洗边界，但时机/覆盖不全 |
| 拦截器链 `AbilityInterceptorExecuter` | 生态规则/跨应用/启动控制 | 在清洗之前执行，读到调用方可控值 |
| `PermissionVerification` | 权限校验 | 部分 API 用 Want 里的 token 而非 IPC token |
| `AppMgrServiceInner` (SA 501) | 进程管理 | 读 SPECIFED_PROCESS_CALLER_PROCESS/NEED_DESTROY_TEMPLATE/KEY_WATERMARK_* 等未清洗 key |

### 依赖层

- `IPCSkeleton`：调用方真实 uid/pid/token 的唯一可信源。
- `Want` / `WantParams`：参数载体，本治理目标。
- `AbilityRequest` / `PreloadRequest`：结构化字段载体，治理后系统参数应迁移至此。

## 三、关键业务流程：StartAbility 的清洗与拦截时序

```
AbilityManagerStub::OnRemoteRequest(StartAbility)
  └─ AbilityManagerService::StartAbility(want, ...)            ← 外部 Want 入口
       └─ StartAbilityInner
            ├─ line 1489: StartAbilityInfoWrap(param.want,...)  ← 仅清洗 localWant 副本
            ├─ line 1492: param.want.GetIntParam(CALLER_TOKEN)  ← 读未清洗原始 Want
            ├─ line 1500: interceptorExecuter_->DoProcess(param)← 拦截器链，Want 未清洗
            │     ├─ EcologicalRuleInterceptor    (读 CALLER_BUNDLE_NAME/UID/PID)
            │     ├─ StartOtherAppInterceptor     (读 CALLER_BUNDLE_NAME)
            │     └─ ...
            ├─ line 1552: UpdateCallerInfo(abilityRequest.want)  ← 此刻才用 IPC 覆写 caller 身份
            └─ line 1641: UpdateCallerInfo(...)                 ← 另一路径清洗
```

**关键事实**：清洗（`UpdateCallerInfo`）发生在拦截器链（line 1500）**之后**。1489→1500 之间所有读 caller 身份的代码读到的都是调用方可控值。

## 四、清洗边界现状

### `UpdateCallerInfo`（update_caller_info_util.cpp:73-92）
用 `IPCSkeleton::GetCallingTokenID/Uid/Pid` **覆写** `PARAM_RESV_CALLER_TOKEN/UID/PID`。属于"给可信值"而非"删除"。

### `ClearProtectedWantParam`（update_caller_info_util.cpp:416-442）
**仅清洗**（RemoveParam）：`CALLER_NATIVE_NAME`、`COMPONENT_STARTUP_NEW_RULES`、`ABILITY_UNIFIED_DATA_KEY`、`SPECIFIED_ABILITY_FLAG`。

### 未覆盖的系统参数 key（全程无清洗）
- caller 身份：`PARAM_RESV_CALLER_BUNDLE_NAME`、`PARAM_RESV_CALLER_ABILITY_NAME`、`PARAM_RESV_CALLER_APP_CLONE_INDEX`、`PARAM_RESV_CALLER_APP_IDENTIFIER`（注：部分在 UpdateCallerInfo 中派生覆写，但 ClearProtectedWantParam 不删）
- 进程控制：`SPECIFED_PROCESS_CALLER_PROCESS`（`"ohoSpecifiedProcessCallerProcess"`）、`NEED_DESTROY_TEMPLATE`（`"ohos.ability.runtime.needDestroyTemplate"`）、`PARAM_RESV_REQUEST_PROC_CODE`、`PARAM_RESV_CALL_TO_FOREGROUND`
- 水印：`KEY_WATERMARK_BUSINESS_NAME`、`KEY_IS_WATERMARK_ENABLED`

## 五、对外 API

本治理涉及的 Want 参数 key 多为 `Want::PARAM_RESV_*`（Want 模块保留参数）与 `ability_runtime` 内部常量（`Constants::*`、`SPECIFED_PROCESS_CALLER_PROCESS` 等），无独立对外 TypeScript/NDK API。开发者面向的 StartAbility 等 API 的参数语义不在本报告范围。
