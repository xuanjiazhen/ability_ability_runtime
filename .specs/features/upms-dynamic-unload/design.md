# Design — UPMS 动态卸载

> id: `upms-dynamic-unload` | type: feature | complexity: standard | profile: none
> 承接：[proposal.md](proposal.md) §3 基线。本文件给出关键设计决策（ADR）、架构、约束/取舍、Stage 1→2 不涉及项 carry-through。
> 取证依据：本轮已读 SRMS 先例（[service_router_mgr_service.cpp](../../../../service_router_framework/services/srms/src/service_router_mgr_service.cpp)）、UPMS SA 类（[uri_permission_manager_service.cpp](../../../../services/uripermmgr/src/uri_permission_manager_service.cpp)）、账本结构（[uri_permission_manager_stub_impl.h](../../../../services/uripermmgr/include/uri_permission_manager_stub_impl.h)）、清账路径（stub_impl.cpp）、BUILD.gn、sa_profile。

---

## 1. 设计目标（一句话）

让 UPMS 在**授权账本全空**时安全延时自卸载，把 `libupms` 在空闲期的内存还给 `foundation` 进程；不持久化、不改下游、不绑定 MB。

## 2. 关键架构差异：UPMS ≠ SRMS

| 维度 | SRMS（先例） | UPMS（本需求） |
|------|-------------|----------------|
| SA 类与 Stub 关系 | SA **即** Stub（`ServiceRouterMgrService : ... ServiceRouterMgrStub`，自己持 map） | SA 与 Stub **分离**（`UriPermissionManagerService` 持 `sptr<UriPermissionManagerStubImpl> impl_`） |
| 状态可重建 | 是（`LoadAllBundleInfos` 从 bundle 重载） | 否（`uriMap_`/`policyMap_`/`permissionTokenMap_`/`contentTokenIdSet_` 是临时授权账本，无持久化） |
| 触发语义 | 调用活跃度（每方法重挂） | **授权状态**（账本全空才挂，非空必取消） |
| 结论 | 直接照搬 SRMS 的"每方法 DelayUnloadTask"会误卸载 UPMS | 必须**状态驱动**：仅在账本空时 arm，任何 grant 路径必须 cancel |

> 这张表是本设计的根本出发点：**不能照搬 SRMS 的触发逻辑**，只能复用其卸载机制（EventHandler + `UnloadSystemAbility`）。

## 3. 关键设计决策 (ADR)

### ADR-1：触发策略 = 状态驱动 reconcile（不是 SRMS 的调用活跃度）
- 新增 `bool UriPermissionManagerStubImpl::IsLedgerEmpty()`：四个容器**各自持锁**判定
  `uriMap_.empty() && policyMap_.empty() && permissionTokenMap_.empty() && contentTokenIdSet_.empty()`。
  （锁顺序：`mutex_`(uriMap) → `policyMapMutex_`(policyMap) → `ptMapMutex_`(permissionTokenMap) → `contentTokenIdSetMutex_`(contentTokenIdSet)，与现有清账路径一致，避免死锁。）
- 新增 `void RefreshUnloadTimer()`：空 → `DelayUnloadTask()`；非空 → `CancelUnloadTask()`。幂等，可重复调用。
- **取舍**：用"账本全空"作为唯一安全信号，规避 [unload_upms.md](../../../../unload_upms.md) §4 的误卸载风险。代价：系统常有持久授权时 UPMS 长期不卸载，内存回收尽力而为（已被需求方接受，定性验收）。

### ADR-2：卸载机制放置 = stub_impl 持 ffrt 延时任务，task 零捕获（实现期修订：EventHandler → ffrt）
- **修订背景**：初版用 `EventHandler`+`EventRunner`，会为 UPMS **创建独立常驻线程**，不合适（多余线程、生命周期管理复杂）。**改用 ffrt**（共享线程池，本仓 abilitymgr/appmgr 已大量使用），无独立线程。
- `UriPermissionManagerStubImpl` 新增成员：`std::optional<ffrt::task_handle> unloadHandle_`、`std::mutex timerMutex_`；常量 `UPMS_UNLOAD_DELAY_TIME_US = 90000000`（90s，微秒）。
- **依赖**：`uripermmgr/BUILD.gn` 的 `external_deps` 新增 `"ffrt:libffrt"`（abilitymgr/common 均如此用）。
- `DelayUnloadTask()`：`lock(timerMutex_); if (unloadHandle_) { auto old = std::move(*unloadHandle_); unloadHandle_.reset(); }`（丢弃旧 handle）；`unloadHandle_ = ffrt::submit_h(zeroCaptureLambda, {}, {}, ffrt::task_attr().delay(UPMS_UNLOAD_DELAY_TIME_US).name(UPMS_UNLOAD_TASK_NAME));`
- `CancelUnloadTask()`：`lock(timerMutex_); if (unloadHandle_) { ffrt::skip(*unloadHandle_); unloadHandle_.reset(); }`（`ffrt::skip` 返回 0 表示取消成功，与仓内 `FfrtTaskHandlerWrap::CancelTaskInner` 一致）。
- **零捕获 lambda**：`[]{ auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager(); if (sam) sam->UnloadSystemAbility(URI_PERMISSION_MGR_SERVICE_ID); }`。只用全局 SAMGR + 常量 183，**不捕获 `this`/成员**。
- **理由（关键）**：① ffrt 走共享线程池，无独立线程、无线程生命周期负担；② 放状态拥有者 stub_impl，零跨类耦合、可测；③ 零捕获 lambda 规避悬空——即使 stub_impl 在 task 执行期被析构，lambda 也不 deref 任何成员；④ `submit_h` 返回 handle 可 `ffrt::skip` 取消，满足"任意新 grant 取消挂起卸载"。
- **取舍**：依赖 `ffrt:libffrt`（uripermmgr 新增依赖，需重跑构建验证）。

### ADR-3：取消/挂载口径 = 所有 public 状态变更方法尾部 reconcile（OQ-3）
- 读路径（`CheckUriAuthorization*`、`VerifyUriPermission`）**不**调用 `RefreshUnloadTimer` —— 空账本时的读不应延迟卸载。
- **所有 public 状态变更方法尾部**调用 `RefreshUnloadTimer()`：grant 侧（`GrantUriPermission`×4、`GrantUriPermissionPrivileged`×2、`GrantUriPermissionWithType`、`GrantUriPermissionByKey`、`GrantUriPermissionByKeyAsCaller`、`Active`）+ revoke 侧（`RevokeAllUriPermissions`、`RevokeUriPermissionManually`、`ClearPermissionTokenByMap`）。
- **安全论证**：cancel 必须穷尽所有 grant 路径——任何遗漏 = 账本非空仍被卸载 = 撤销断裂 = 权限/文件泄漏。统一在每个 public 变更方法尾部调幂等的 `RefreshUnloadTimer()`，保证 cancel 穷尽且 arm 也穷尽。arm 遗漏的极端情况只会"该卸未卸"（内存未回收），不破坏安全。
- 取舍：约 13 处一行调用，但每处幂等、低风险，优于在 5~6 个内部 erase 点分散判空。

### ADR-4：OnStop/SelfClean —— 镜像 SRMS，不改 SelfClean（实现期修订，已接受）
- 现状：`SelfClean()` 仅置 `ready_=false`/`registerToService_=false`（[uri_permission_manager_service.cpp:82-90](../../../../services/uripermmgr/src/uri_permission_manager_service.cpp)）。
- **实现决策（修订）**：**不改 `SelfClean`**，镜像 SRMS（SA 404）"零捕获卸载 task + 极简 `OnStop`、不主动碰 EventRunner 生命周期"。
- **修订理由**：原草案（`StopHandler` + `impl_=nullptr`）在自卸载场景下引入 EventRunner 线程生命周期的不确定性（Stop/reset 的线程语义难以在不构建的环境验证）；SRMS 是本仓生产参考，其极简 OnStop 模式已验证，镜像它最稳妥、不引入新风险。
- **AC-5 功能性满足**：① 零捕获卸载 task 无悬空引用；② 任意新 grant → `CancelUnloadTask`→`RemoveTask`，无陈旧卸载 task；③ 仅账本空才卸载，重载即空账本。
- **残留（与 SRMS 同构）**：专用 EventRunner 线程在 SA 拆除时的退出依赖框架处理，非本改动引入。
- **需求方确认**：2026-06-26 接受镜像 SRMS 方案。

### ADR-5：空闲超时阈值 = 90s（OQ-2）
- 常量 `UNLOAD_DELAY_TIME = 90000`（ms），与 SRMS 一致。
- 理由：与系统内同类 SA 统一；UPMS 撤销通常紧跟应用退出，90s 足够覆盖"退出→清账→空闲"窗口。值可后续按实测调整。
- 取舍：定值可能偏长/偏短，但定性验收不敏感；放常量便于调。

### ADR-6：sa_profile 不改（OQ-4）
- **不改 `services/sa_profile/183.json`**。`run-on-create:false` 已配置；SAMGR 对 `run-on-create:false` 的 SA 在 IPC 请求时自动加载/重载——SRMS（404.json 同结构、无 ondemand）已证明此路径成立。
- **强制验证点**：Stage 3/4 必须实测"卸载后再次 `GrantUriPermission` 能触发重载"（AC-3）。
- **Fallback**：若重载不发生，则在 `183.json` 增加 ondemand load 条件（属本需求范围外的兜底改动，届时评估）。

## 4. 架构（卸载链路）

```
应用退出 / Revoke* / Clear*
        │ (清账: uriMap_/policyMap_/permissionTokenMap_/contentTokenIdSet_ 缩减)
        ▼
public 变更方法尾部 → RefreshUnloadTimer()
        │
        ├─ IsLedgerEmpty() == true  → DelayUnloadTask(): RemoveTask + PostTask(zero-capture unload, 90s)
        │                                         │ (90s 内无新 grant)
        │                                         ▼
        │                              lambda: SAMGR->UnloadSystemAbility(183)
        │                                         │
        │                                         ▼
        │                       safwk → SA::OnStop() → SelfClean()(停 handler + impl_=nullptr) → dlclose(libupms)
        │
        └─ IsLedgerEmpty() == false → CancelUnloadTask(): RemoveTask (账本非空绝不卸载)

(新授权到达) Grant* → ... → Add* → 方法尾部 RefreshUnloadTimer() → 非空 → CancelUnloadTask()

(SA 已 unloaded) 应用再请求 GrantUriPermission → SAMGR 按 run-on-create:false 自动重载 → OnStart→Init→新空账本
```

## 5. 关键代码点位（实现锚点，Stage 3 task.md 展开）

| 位置 | 改动 |
|------|------|
| `uri_permission_manager_stub_impl.h` | 新增成员 `std::optional<ffrt::task_handle> unloadHandle_`、`std::mutex timerMutex_`；新增私有方法 `IsLedgerEmpty()`、`RefreshUnloadTimer()`、`DelayUnloadTask()`、`CancelUnloadTask()`；include `ffrt/ffrt.h` |
| `uri_permission_manager_stub_impl.cpp` | 实现上述（`DelayUnloadTask` 用 `ffrt::submit_h`+`.delay().name()`，`CancelUnloadTask` 用 `ffrt::skip`）；10 处 public 变更方法尾部插 `RefreshUnloadTimer()` |
| `uri_permission_manager_service.cpp` | **不改**（ADR-4 镜像 SRMS） |
| `services/sa_profile/183.json` | **不改**（ADR-6） |
| `services/uripermmgr/BUILD.gn` | `external_deps` 新增 `"ffrt:libffrt"`（ADR-2） |

## 6. Stage 1 → Stage 2 不涉及项 carry-through

| 项 | Stage 1 结论 | Stage 2 确认 |
|----|-------------|-------------|
| 授权账本持久化 | N/A | 确认 N/A（ADR-1：空账本才卸，无需落盘） |
| 下游撤销服务改造 | N/A | 确认 N/A（卸载发生在撤销完成后） |
| 具体 MB 量化目标 | N/A | 确认 N/A（定性验收） |
| BUILD.gn 依赖改动 | 待定 | **确认需改**：uripermmgr/BUILD.gn `external_deps` +`"ffrt:libffrt"`（ADR-2 修订） |
| sa_profile 改动 | 待定 | **确认为不改**（ADR-6） |
| 授权/撤销主链路改写 | N/A | 确认 N/A（仅尾部加幂等旁路，ADR-3） |
| 新增对外 API | N/A | 确认 N/A |
| 跨仓 / SIG 协调 | N/A | 确认 N/A（单仓） |

## 7. 约束 / 风险 / 取舍

| 风险 | 等级 | 缓解 |
|------|------|------|
| **误卸载 → 权限/文件泄漏** | 高（安全相关） | ADR-1/3：账本全空才卸 + cancel 穷尽所有 grant 路径；UT 必须覆盖"非空不卸载"(AC-4) |
| **重载不发生（AC-3 失败）** | 中 | ADR-6：Stage 3/4 实测验证；Fallback 加 ondemand 条件 |
| **dlclose 实际 RSS 收益有限** | 中 | 已知（[unload_upms.md](../../../../unload_upms.md) §5）；定性验收覆盖，RSS 降幅仅作观测 |
| **自身 handler 线程卸载自身悬空** | 低 | ADR-2：零捕获 lambda + 参考 SRMS 同形结构；`OnStop` 停 handler |
| **多锁死锁** | 低 | ADR-1：锁顺序与现有清账路径一致 |
| **可达性：UPMS 长期不卸载** | 中 | 已被需求方接受；定性验收 |

## 8. 验证策略（design 层面，详见 spec.md §AC）

- **UT**：`IsLedgerEmpty` 四容器组合；`RefreshUnloadTimer` 空→arm/非空→cancel；各 grant 路径 cancel 穷尽；revoke 清空后 arm；`OnStop` 停 handler。
- **集成/实测**：账本空 + 等 90s → `hidumper -s 183` / samgr 确认 unloaded（AC-1）；持有授权等超时 → 仍 loaded（AC-4）；unloaded 后再授权 → 重载成功（AC-3）。
- **回归**：`run -t UT -tp uripermmgr` 全绿（AC-5）。

## 9. 待确认（进 Stage 3 前）
- OQ-1 `target_release`：Stage 2 期间向 owner 确认，暂置。
- ADR-6 的重载验证是 Stage 3/4 的硬门槛；若失败需评估 fallback 是否扩范围。
