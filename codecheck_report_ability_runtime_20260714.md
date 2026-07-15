# 代码检视报告 — ability_runtime (HEAD~5..HEAD)

> 范围：`services/abilitymgr/` + `services/appmgr/` + `services/common/` + `frameworks/native/appkit/` + `interfaces/inner_api/`
> 日期：2026-07-14
> 检视维度：deep-scan (high-impact-bug-audit + logic_analyzer + security_review) + external-input-audit
> 非合并提交：`f2802e5` 优化foundation进程判断 / `84aad3cf` fix(skill): match target death by pid / `c037deff` add snapshot report version name / `349ff027` refactor public deps / `d16ef7f3` add telemetry kotlin type

---

## 1. 总览

| 子 skill | 发现数 | P0 | P1 | P2 | P3 |
|----------|--------|----|----|----|-----|
| high-impact-bug-audit (Layer 1) | 3 | 0 | 2 | 1 | 0 |
| logic_analyzer (Layer 2) | 4 | 0 | 1 | 3 | 0 |
| security_review (Layer 3) | 4 | 0 | 0 | 0 | 4 (含 2 个"非漏洞"确认) |
| external-input-audit | 4 | 0 | 0 | 1 | 3 |
| **合计（跨维度去重后）** | **8** | **0** | **2** | **3** | **3** |

> 无 P0（致命）发现。P1 发现 2 条，均为 `SkillExecuteManager` 状态管理风险。

---

## 2. 高优先级发现（P1，跨维度去重后）

### 2.1 OnTargetProcessDied 在 targetPid 未就绪时无法检测目标进程死亡

| 维度来源 | high-impact-bug-audit, logic_analyzer |
|---------|--------------------------------------|
| 位置 | `services/abilitymgr/src/skill/skill_execute_manager.cpp:486-489` |
| 风险等级 | P1 |
| 证据级别 | Confirmed |

**问题描述：**
`OnTargetProcessDied` 要求 `record->targetPid != 0 && record->targetPid == pid` 才能匹配。但 `targetPid` 仅在 `OnLaunchCompleted`（line 448）中被设置。如果目标进程在 `OnLaunchCompleted` 之前崩溃（如启动过程中 segfault / OOM kill），`targetPid` 仍为 0，匹配跳过，record 滞留在 `EXECUTING` 状态直到超时触发。

代码注释已承认此 gap：`"pid 未就绪（OnLaunchCompleted 前）跳过，依赖 timeout 兜底"`。

**触发路径：**
```
1. AMS 发起 Skill 执行 → CreateExecuteRecord (targetPid=0, state=EXECUTING)
2. 目标进程启动中崩溃（fork 后 exec 前）
3. OnProcessDied 回调触发 → OnTargetProcessDied(bundleName, pid)
4. record->targetPid == 0 → 跳过匹配
5. 用户等待超时 → 收到 ERR_TIMED_OUT（应为 ERR_SKILL_EXECUTE_TARGET_DIED）
```

**影响：**
- 错误码不准确（超时 vs 目标死亡），影响调用方错误处理逻辑
- 无资源泄漏（超时兜底），但用户体验受损

**修复建议：**
在 `OnTargetProcessDied` 中增加 fallback：当所有 record 的 `targetPid == 0` 且 `targetBundleName` 匹配且 `state == EXECUTING` 时，也将其视为候选。需加去重逻辑避免误杀。

---

### 2.2 OnLaunchCompleted 重复调用移除超时且不重新设置

| 维度来源 | logic_analyzer, high-impact-bug-audit |
|---------|--------------------------------------|
| 位置 | `services/abilitymgr/src/skill/skill_execute_manager.cpp:438-453` |
| 风险等级 | P1 |
| 证据级别 | Likely |

**问题描述：**
`OnLaunchCompleted` 不检查 record 的当前状态或 `targetPid` 是否已设置。如果 `NotifySkillFunctionInvoked` 被重复调用（目标进程重启或 bug 导致重复通知），会：
1. 覆盖 `targetPid` 为新 PID
2. 再次调用 `RemoveSkillExecuteTimeoutLocked`（超时已移除，第二次调用无效）
3. 超时不会重新设置 → record 失去超时兜底

如果此后新 PID 对应的进程也死亡，但 `OnTargetProcessDied` 中的 PID 匹配已因覆盖而失效，record 将永久滞留（无超时、无死亡检测）。

**触发路径：**
```
1. Skill 执行 → 创建 record (requestCode=R1, targetPid=0)
2. 第一次 NotifySkillFunctionInvoked → OnLaunchCompleted: targetPid=P1, timeout removed
3. 目标进程重启 → 第二次 NotifySkillFunctionInvoked → OnLaunchCompleted: targetPid=P2, timeout already gone
4. P1 进程死亡 → OnTargetProcessDied 匹配 targetPid==P1 → 找不到（已被覆盖为 P2）→ 跳过
5. P2 不死亡 → record 永久滞留
```

**触发条件验证：**
`NotifySkillFunctionInvoked`（line 15311-15324）仅通过 `JudgeSelfCalled` 验证 token，不验证调用者是否为实际的 skill target 进程。只要 token 匹配，重复调用可通过校验。

**修复建议：**
在 `OnLaunchCompleted` 中增加防御：
```cpp
if (it->second->targetPid != 0) {
    TAG_LOGW(AAFwkTag::ABILITYMGR, "OnLaunchCompleted duplicate, pid already set");
    return;
}
```
或在 `state != EXECUTING` 时提前返回。

---

## 3. 分维度明细

### 3.1 high-impact-bug-audit（Layer 1）

| # | 位置 | 问题 | 等级 | 证据 |
|---|------|------|------|------|
| 1 | `skill_execute_manager.cpp:486-489` | targetPid 未就绪时无法检测目标进程死亡 | P1 | Confirmed |
| 2 | `skill_execute_manager.cpp:438-453` | 重复 OnLaunchCompleted 移除超时不重设 | P1 | Likely |
| 3 | `skill_execute_manager.cpp:157-194` | externalRequestCode 非空时 records_ 静默覆写 | P2 | Confirmed (latent) |

**Finding 3 详情：**
`CreateExecuteRecord` 中若 `externalRequestCode` 非空且与已有 key 重复，旧 record 被静默覆写（death recipient 未移除、callback 未调用）。当前所有调用点传空字符串（line 15141, 15202），不可利用，但代码为未来使用预留了此路径。

---

### 3.2 logic_analyzer（Layer 2）

| # | 位置 | 问题 | 等级 | 证据 |
|---|------|------|------|------|
| 1 | `skill_execute_manager.cpp:438-453` | 重复 OnLaunchCompleted 移除超时不重设 | P1 | Likely |
| 2 | `ability_manager_service.cpp:3817,9208` | pid→uid 变更对现有调用者安全 | 信息 | Confirmed |
| 3 | `skill_execute_manager.cpp:486-489` | targetPid==0 fallback 依赖超时 | P2 | Confirmed |
| 4 | `app_mgr_service_inner.cpp` | SnapshotStartReport 锁粒度改进（swap 后释放锁） | 正面 | Confirmed |

**Finding 2 详情（正面评价）：**
旧代码 `GetCallingPid() != getprocpid()` 对 IPC 调用者始终失败（PID 永远不等于本地 PID）。新 UID 检查修复了此 bug。所有现有调用者均满足新检查：
- `IN_PROCESS_CALL` 路径（line 10618, 10690）：进程内调用，UID = FOUNDATION_UID
- `AbilityManagerStub` IPC 路径（line 4442）：由 BundleManagerService 调用，BMS 运行在 foundation 进程

**Finding 4 详情（正面评价）：**
`SnapshotStartReport` 重构将 `reportMap.swap(imageStartReportMap_)` 移到锁内，确保 swap 和清零操作原子性。旧代码在锁外访问 `imageStartReportMap_` 和 `imageStartCount_` 存在潜在竞态。

---

### 3.3 security_review（Layer 3）

| # | 位置 | 问题 | 等级 | 证据 |
|---|------|------|------|------|
| 1 | `ability_manager_service.cpp:3817,9208` | PID→UID 信任边界扩展为有意设计，内核强制 UID 不可伪造 | 低 (非漏洞) | Confirmed |
| 2 | `skill_execute_manager.cpp:487-489` | PID 精确匹配比旧 bundleName-only 匹配更安全 | 低 (改进) | Confirmed |
| 3 | `ability_manager_service.cpp:3817,9208` | UID 检查与后续操作间无 TOCTOU | N/A (安全) | Confirmed |
| 4 | `event_report.cpp:864-889` | SendSnapshotEvent 新增 appVersionName 字段写入 HiSysEvent | Info | Confirmed |

**结论：** 安全维度无 Critical/High/Medium 发现。信任边界变更经验证为内核强制、与 codebase 中其他 FOUNDATION_UID 检查（lines 859, 1063, 12459, 13988, 18863）模式一致。

---

### 3.4 external-input-audit

| # | 链路 | 位置 | 问题 | 等级 |
|---|------|------|------|------|
| 1 | CHAIN-001 | `app_mgr_service_inner.cpp:8130-8147` | HiSysEventWrite 直接写入 exitMsg/appVersionName 未净化 | P2 |
| 2 | CHAIN-002 | `skill_execute_manager.cpp:157-194` | externalRequestCode → records_ 静默覆写（latent） | P2 |
| 3 | CHAIN-003 | `event_report.cpp:864-889` | SendSnapshotEvent snapshotReason 来自内部状态 | P3 |
| 4 | CHAIN-004 | `app_mgr_service_inner.cpp:8108-8128` | GetImageReportAppVersionName 查询 BundleInfo → versionName | P3 |

**CHAIN-001 详情（P2）：**
`SendDestroyImageEvent`（line 8130-8147）直接将 `exitMsg`（来自 IPC/进程退出消息）写入 HiSysEvent 的 `REASON` 字段，无长度限制或控制字符过滤。恶意应用可通过异常退出消息注入 `\r\n` 到日志流。不过该字段仅由 foundation 进程写入（`SubmitDestroyImageTask` 调用链在 foundation 进程内），攻击面有限。

---

## 4. 变更正面评价

1. **PID→UID 权限检查修复了原有 bug**：旧代码 `GetCallingPid() != getprocpid()` 对 IPC 调用者始终失败（PID 永远不等于本地 PID），新 UID 检查正确且与 codebase 一致。

2. **PID 精确匹配改善了 Skill 执行的进程死亡检测精度**：旧代码仅按 bundleName 匹配，可能误杀同 bundle 的多个执行记录。新代码要求 `(bundleName, targetPid, state)` 三重匹配。

3. **SnapshotStartReport 锁粒度改进**：重构将数据交换移到锁内，消除了旧代码在锁外访问共享数据的竞态窗口。

4. **LeakType 枚举扩展**：新增 `KMP_DUMP = 9` 和 `ATLT_RSS_KMP_DUMP = 1 << 9`，与现有枚举值无冲突，位移正确。

5. **测试覆盖**：新增 359 行 `app_mgr_service_inner_test` 和 66 行 `event_report_test`。

---

## 5. 待跟进（需进一步确认）

| # | 项目 | 原因 |
|---|------|------|
| 1 | `app_mgr_service_inner.cpp` SnapshotStartReport 重构后的 reportMap swap 语义 | 确认 swap 后 `imageStartReportMap_` 是否为空（应为空 map），以及 `imageStartCount_` 清零是否在锁内 |
| 2 | `CreateExecuteRecord` 的 externalRequestCode 覆写风险 | 当前不可利用，但需确认未来调用点是否可能传入用户可控的 requestCode |
| 3 | HiSysEvent 字段净化策略 | `exitMsg`/`appVersionName`/`snapshotReason` 等字符串字段是否需要统一长度限制 |

---

## 6. 附录：各 skill 原始产出

| Skill | 产出 |
|-------|------|
| high-impact-bug-audit (Layer 1) | 3 findings (0 P0, 2 P1, 1 P2) |
| logic_analyzer (Layer 2) | 4 findings (0 P0, 1 actionable P1, 1 P2, 2 informational/positive) |
| security_review (Layer 3) | 4 findings (0 Critical/High/Medium, all Low/Info) |
| external-input-audit | 4 chains (0 P0, 2 P2, 2 P3) |
