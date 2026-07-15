# 代码检视报告 — ability_manager_service.cpp UID 权限检查优化

> 范围：`services/abilitymgr/src/ability_manager_service.cpp` (commit f2802e5446)
> 日期：2026-07-15
> 检视维度：deep-scan (high-impact-bug-audit + logic_analyzer + security_review) + external-input-audit

---

## 1. 总览

| 检视维度 | 发现数 | P0 | P1 | P2 |
|----------|--------|----|----|----|
| high-impact-bug-audit | 0 | 0 | 0 | 0 |
| logic_analyzer | 0 | 0 | 0 | 0 |
| security_review | 0 | 0 | 0 | 0 |
| external-input-audit | 0 | 0 | 0 | 0 |
| **合计** | **0** | **0** | **0** | **0** |

**结论**：本次变更安全、正确、一致，未发现缺陷。

---

## 2. 变更摘要

**提交**：`f2802e5446` — 优化foundation进程判断

**变更内容**：将两处 IPC 调用者身份校验从 PID 检查升级为 UID 检查。

### 变更点 1：`RecordAppWithReasonByUserId` (line 3817)

```diff
- if (IPCSkeleton::GetCallingPid() != getprocpid()) {
-     TAG_LOGE(AAFwkTag::ABILITYMGR, "calling pid is not local process");
+ if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
+     TAG_LOGE(AAFwkTag::ABILITYMGR, "calling uid is not foundation");
```

### 变更点 2：`UninstallAppInner` (line 9208)

```diff
- pid_t callingPid = IPCSkeleton::GetCallingPid();
- pid_t pid = getprocpid();
- if (callingPid != pid) {
+ if (IPCSkeleton::GetCallingUid() != FOUNDATION_UID) {
```

---

## 3. 分维度分析

### 3.1 high-impact-bug-audit（Layer 1）

**模块风险画像**：AbilityManagerService 是系统核心组件，通过 IPC 接收请求。本次变更仅修改两处权限校验逻辑。

**扫描结果**：

- **崩溃/UAF/OOM/死锁/资源泄漏**：未发现。变更仅替换条件判断，不引入新资源或控制流。
- **IN_PROCESS_CALL 兼容性**：已确认安全。SA Profile (`services/sa_profile/180.json`) 明确 `"process": "foundation"`，AMS 运行在 foundation 进程，UID 即 FOUNDATION_UID (5523)。`IN_PROCESS_CALL` 绕过 IPC 层，`GetCallingUid()` 返回进程自身 UID，与 FOUNDATION_UID 相等，内部调用路径（`StopUser` line 10618、`LogoutUser` line 10690）不受影响。
- **模式一致性**：本次变更后，文件内共 7 处 `GetCallingUid() != FOUNDATION_UID` 检查（lines 859, 1063, 3817, 9208, 12459, 13988, 18863），与 `StartAbilityWithSpecifyTokenId`、`StartAbilityOnlyUIAbility`、`CompleteFirstFrameDrawing` 等函数保持一致。

**风险等级**：无

### 3.2 logic_analyzer（Layer 2）

**变更类型**：数据流变更（条件表达式替换）

**逻辑影响分析**：

| 检查维度 | 结果 |
|----------|------|
| 控制流 | 无变化。`if` 分支结构不变，true/false 路径不变 |
| 数据流 | `GetCallingPid()` / `getprocpid()` 替换为 `GetCallingUid()` / `FOUNDATION_UID`。功能等价且更强 |
| 状态机 | 不涉及。两函数无状态转换 |
| 边界条件 | UID 比较为 int32_t 等值比较，无溢出/越界风险 |
| 错误处理 | 返回值 `CHECK_PERMISSION_FAILED` 不变，日志级别/标签不变 |
| 并发安全 | 无新共享状态，`IPCSkeleton::GetCallingUid()` 为线程安全 API |
| 业务规则 | `UninstallAppInner` 移除了两个不再使用的局部变量 (`callingPid`, `pid`)，代码更简洁 |

**调用链影响**：
- `RecordAppWithReasonByUserId` 被 IPC 入口 (`AbilityManagerStub::RecordAppWithReasonByUserIdInner`) 和内部调用 (`StopUser`/`LogoutUser`) 使用，均安全
- `UninstallAppInner` 被 IPC 入口 (`AbilityManagerStub::UninstallAppInner`) 和 `UninstallApp`/`UpgradeApp` 使用，均安全

**风险等级**：无

### 3.3 security_review（Layer 3）

**攻击面分析**：

| 检查项 | 结果 |
|--------|------|
| 权限校验有效性 | UID 检查比 PID 检查更强——PID 可被回收复用，UID 是稳定的进程身份标识。恶意进程无法伪造 FOUNDATION_UID |
| 权限绕过 | 无。只有 UID 为 5523 的进程（foundation）才能通过校验 |
| 信息泄漏 | 日志 `"calling uid is not foundation"` 不泄露敏感信息 |
| 敏感数据 | 无硬编码凭据或密钥 |
| 注入/反序列化 | 不涉及。仅做整数等值比较 |
| 合规性 | 符合最小权限原则——仅允许 foundation 进程调用敏感操作 |

**安全等级**：安全（Security Improvement）

### 3.4 external-input-audit

**IPC 输入链路分析**：

| 函数 | IPC 入口 | 参数 | 校验 | 持久化目标 |
|------|----------|------|------|-----------|
| `RecordAppWithReasonByUserId` | `AbilityManagerStub::RecordAppWithReasonByUserIdInner` | `userId`, `exitReason` | UID 校验 + `IsExitReasonValid()` | `appExitReasonHelper_->RecordAppsWithReasonByProcessInfoList` |
| `UninstallAppInner` | `AbilityManagerStub::UninstallAppInner` | `bundleName`, `uid`, `appIndex` | UID 校验 | Extension 管理器、KeepAlive 管理器 |

两条链路均有 FOUNDATION_UID 前置校验，外部输入在进入持久化前已通过身份验证。变更未改变输入校验逻辑，仅增强了身份验证强度。

**风险等级**：无

---

## 4. 高优先级发现

无。

---

## 5. 变更评估总结

| 维度 | 评估 |
|------|------|
| **正确性** | ✅ SA Profile 确认 AMS 在 foundation 进程运行，IN_PROCESS_CALL 路径安全 |
| **安全性** | ✅ UID 检查比 PID 检查更强，防止 PID 回收攻击 |
| **一致性** | ✅ 与文件内其他 5 处 FOUNDATION_UID 检查模式一致 |
| **兼容性** | ✅ IPC 接口签名不变，Stub/Proxy 层无变化，跨版本兼容 |
| **DFX** | ✅ 日志文本已同步更新，HiSysEvent 不受影响 |
| **性能** | ✅ UID 比较与 PID 比较性能等价 |

**总体评价**：✅ 推荐合入。变更小而精，是一次安全加固，将弱 PID 检查升级为更强的 UID 检查，与代码库既有模式对齐。

---

## 6. 附录：检视产出文件

| 文件 | 路径 |
|------|------|
| 本报告 | `codecheck_report_abilityms_uid_check_20260715.md` |
