# Spec — UPMS 动态卸载

> id: `upms-dynamic-unload` | type: feature | complexity: standard | profile: none
> 承接：[proposal.md](proposal.md) §3 基线 + [design.md](design.md) ADR-1~6。
> `manifest.target_release` = 待确认（OQ-1）；`manifest.profile` = none；lineage 见 manifest。

---

## 1. 特性概述

为 UPMS（SA 183，`libupms.z.so`，`foundation` 进程）增加**安全延时自卸载**能力：当没有任何应用持有 URI 授权（授权账本全空）且空闲超时，UPMS 自调 `UnloadSystemAbility(183)` 卸载自身，把内存还给 `foundation` 进程；下次访问由 SAMGR 自动重载，从空账本重建。卸载仅发生在账本为空时，状态语义安全，规避误卸载导致的权限/文件泄漏。

## 2. 用户故事

- **US-1（系统/内存）**：作为精简产品/RSS 预算紧张的设备，我希望 UPMS 在无授权业务的空闲窗口能被卸载，以降低 `foundation` 进程常驻内存。
- **US-2（安全）**：作为系统安全维护者，我希望 UPMS 绝不会在仍有应用持有授权时被卸载，避免撤销链路断裂导致的共享文件/沙箱策略泄漏。
- **US-3（功能正确性）**：作为使用跨应用 URI 授权的应用，我希望 UPMS 卸载后再次授权时能被自动重载，授权功能不受影响。

## 3. 验收标准 (AC，WHEN/THEN，可测)

每条 AC 标注来源（proposal §3.5）与设计依据（design ADR）。

### P0
- **AC-1 卸载能力** [proposal P0-1 / ADR-1,2,6]
  - **WHEN** UPMS 的 `uriMap_`/`policyMap_`/`permissionTokenMap_`/`contentTokenIdSet_` 全部为空，且空闲持续 90s（`UNLOAD_DELAY_TIME`）
  - **THEN** UPMS 自调 `UnloadSystemAbility(183)`，SA 183 状态变为 unloaded。
  - **验证**：`hidumper -s 183` / samgr 查询，SA 183 不在 loaded 列表；日志见 `UnloadSA success`。

- **AC-2 状态驱动触发与取消** [proposal P0-2 / ADR-1,3]
  - **WHEN** 最后一份授权被撤销（账本由非空变空）**THEN** 启动 90s 延时卸载计时；
  - **WHEN** 计时未到且新授权/任意 grant 到来（账本变非空）**THEN** 立即取消计时。
  - **验证**：UT 覆盖"空→计时 arm"与"非空→计时 cancel / 新 grant→cancel"；`RefreshUnloadTimer` 幂等。

- **AC-3 ondemand 重载** [proposal P0-3 / ADR-6]
  - **WHEN** SA 183 已 unloaded，应用再次请求 `GrantUriPermission`
  - **THEN** SAMGR 按 `run-on-create:false` 自动重新加载 UPMS，并正常完成授权（账本从空重建）。
  - **验证**：卸载后触发一次跨应用 URI 授权，确认 SA 183 回到 loaded 且授权成功。
  - **门槛**：若重载不发生，触发 ADR-6 fallback（评估在 `183.json` 加 ondemand load 条件）。

### P1
- **AC-4 不误卸载（安全）** [proposal P1-1 / ADR-1,3]
  - **WHEN** 有应用仍持有授权（账本任一容器非空）
  - **THEN** 即使长时间（>90s）无调用，UPMS 也不卸载。
  - **验证**：授予权限后等待超过 90s，SA 183 仍 loaded。

- **AC-5 OnStop 清理** [design ADR-4]
  - **WHEN** UPMS 被卸载（`OnStop` 触发）
  - **THEN** EventHandler/Runner 停止、挂起任务移除、`impl_` 置空，无悬空任务；下次 `OnStart` 创建全新空账本 stub_impl。
  - **验证**：UT 覆盖 `StopHandler`/`SelfClean`；集成验证重载后账本为空。

- **AC-6 回归** [proposal P1-2]
  - **WHEN** 卸载机制启用
  - **THEN** uripermmgr 全套 UT 通过，授权/撤销/Check 主链路行为不变。
  - **验证**：`run -t UT -tp uripermmgr`（及 `uri_permission_manager_test` / `uri_permission_impl_test` / `uri_permission_test` / `uri_perm_mgr_test`）全绿。

## 4. 不参与触发的方法（明确边界，ADR-3）

读路径**不**调用 `RefreshUnloadTimer`，空账本时的读不延迟卸载：
- `CheckUriAuthorization`（×3 重载）/ `CheckUriAuthorizationWithType` / `VerifyUriPermission`。

## 5. API 影响

- **对外 API**：无新增、无变更。`IUriPermissionManager.idl` 不改。
- **内部接口**：`UriPermissionManagerStubImpl` 新增私有方法（`InitHandler`/`IsLedgerEmpty`/`RefreshUnloadTimer`/`DelayUnloadTask`/`CancelUnloadTask`/`StopHandler`）与私有成员，不暴露。
- **配置**：`183.json` 不改（ADR-6）。
- **依赖**：`uripermmgr/BUILD.gn` 新增 `"ffrt:libffrt"`（ADR-2）。

## 6. 兼容性

- **行为兼容**：授权/撤销/Check 主链路逻辑不变，仅在 public 变更方法尾部追加幂等旁路。
- **重载兼容**：依赖 SAMGR 对 `run-on-create:false` SA 的既有加载机制（SRMS 同构）。
- **配置兼容**：不改 sa_profile / gni 特性标志；BUILD.gn 仅 +`ffrt:libffrt`。
- **线程模型**：**不新增独立线程**——用 ffrt 共享线程池提交延时卸载任务（ADR-2）。`unloadHandle_` 由新增 `timerMutex_` 独立保护；`IsLedgerEmpty` 按既有顺序访问 4 把 ledger 锁（各自独立作用域，不并发持两把，ADR-1）。

## 7. 可追溯性矩阵

| AC | proposal 来源 | design 依据 | 验证手段 |
|----|--------------|------------|---------|
| AC-1 卸载能力 | §3.5 P0-1 | ADR-1,2,6 | hidumper/samgr + 日志 |
| AC-2 触发与取消 | §3.5 P0-2 | ADR-1,3 | UT |
| AC-3 重载 | §3.5 P0-3 | ADR-6 | 集成实测（硬门槛） |
| AC-4 不误卸载 | §3.5 P1-1 | ADR-1,3 | UT + 集成 |
| AC-5 OnStop 清理 | design §3 ADR-4 | ADR-4 | UT + 集成 |
| AC-6 回归 | §3.5 P1-2 | — | `run -t UT -tp uripermmgr` |

## 8. 关键参数

| 参数 | 值 | 依据 |
|------|-----|------|
| `UPMS_UNLOAD_DELAY_TIME_US` | 90000000 (μs = 90s) | ADR-5；ffrt `.delay()` 取微秒 |
| `UPMS_UNLOAD_TASK_NAME` | `"UriPermissionUnloadTask"` | ADR-2 |
| SA ID | 183 (`URI_PERMISSION_MGR_SERVICE_ID`) | 既有 |

## 9. 范围外（重申）

持久化、下游撤销服务改造、MB 量化、BUILD.gn/依赖裁剪、sa_profile 改动（除非 ADR-6 fallback 触发）、对外 API。
