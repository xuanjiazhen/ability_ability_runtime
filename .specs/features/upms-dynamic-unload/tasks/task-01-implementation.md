# Task 01 — 实现：ffrt 卸载机制 + 触发插桩

> **[待重写 2026-06-26]** 本卡原描述 EventHandler 实现，已**作废**。ADR-2 改为 ffrt（`submit_h`/`.delay`/`skip`、`+ffrt:libffrt`）。批准后据 design.md ADR-2（ffrt 版）重写本卡与代码；当前工作区的 EventHandler 代码需回滚。
> item: `upms-dynamic-unload` | depends on: — | mode: 直接实现
> 规则来源：[design.md](../design.md) ADR-1~6（ADR-2 已修订为 ffrt）、§5 代码点位。本卡只含本任务所需上下文，不重复 Stage 1-2 全文。

## 目标
在 stub_impl 增加"账本全空才延时卸载"机制，并在所有 public 状态变更方法尾部 reconcile；SA 类 `OnStop` 补强清理。

## 文件范围（仅这 3 个文件）
- `services/uripermmgr/include/uri_permission_manager_stub_impl.h`
- `services/uripermmgr/src/uri_permission_manager_stub_impl.cpp`
- `services/uripermmgr/src/uri_permission_manager_service.cpp`

**不改**：`183.json`、`BUILD.gn`、对外 API、`IUriPermissionManager.idl`。

## 改动清单

### A. stub_impl.h — 新增 include / 成员 / 方法声明
1. include：`event_handler.h`、`event_runner.h`、`if_system_ability_manager.h`、`iservice_registry.h`、`system_ability_definition.h`（按需，部分可能已间接包含）。
2. 新增私有成员：
   ```cpp
   std::shared_ptr<OHOS::AppExecFwk::EventHandler> handler_ = nullptr;
   std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner_ = nullptr;
   std::mutex timerMutex_;
   ```
   （命名空间按既有代码风格：`OHOS::AAFwk` 内，EventHandler/EventRunner 用全限定或 using。）
3. 新增私有方法声明：
   - `bool InitHandler();`
   - `bool IsLedgerEmpty() const;`（注：需在持锁下访问四个 map/set；若 const 与锁冲突则去掉 const）
   - `void RefreshUnloadTimer();`
   - `void DelayUnloadTask();`
   - `void CancelUnloadTask();`
   - `void StopHandler();`

### B. stub_impl.cpp — 实现方法（ADR-1,2,5）
- 顶部匿名命名空间加常量：
  ```cpp
  const std::string TASK_NAME = "UriPermissionUnloadTask";
  const int64_t UNLOAD_DELAY_TIME = 90000;
  ```
- `InitHandler()`：`runner_ = EventRunner::Create("UriPermissionUnloadRunner"); handler_ = std::make_shared<EventHandler>(runner_);` 失败返回 false。
- `IsLedgerEmpty()`：按锁顺序 `mutex_`→`policyMapMutex_`→`ptMapMutex_`→`contentTokenIdSetMutex_` 分别 `lock_guard`，返回四容器全空。
- `DelayUnloadTask()`：`std::lock_guard lock(timerMutex_); if (!handler_) return; handler_->RemoveTask(TASK_NAME); auto task = [](){ auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager(); if (sam) sam->UnloadSystemAbility(URI_PERMISSION_MGR_SERVICE_ID); }; handler_->PostTask(task, TASK_NAME, UNLOAD_DELAY_TIME);`
  - **强约束**：lambda **零捕获**，不得捕获 `this` 或成员（ADR-2 安全论证）。
- `CancelUnloadTask()`：`std::lock_guard lock(timerMutex_); if (handler_) handler_->RemoveTask(TASK_NAME);`
- `RefreshUnloadTimer()`：`if (!InitHandler()) return;`（懒初始化，幂等）；`if (IsLedgerEmpty()) DelayUnloadTask(); else CancelUnloadTask();`
- `StopHandler()`：`std::lock_guard lock(timerMutex_); if (handler_) handler_->RemoveTask(TASK_NAME); handler_.reset(); runner_.reset();`

### C. stub_impl.cpp — 13 处 public 变更方法尾部插桩（ADR-3）
在以下**每个**方法的 `return` 前（成功路径尾）调用 `RefreshUnloadTimer();`：
- Grant 侧（10）：`GrantUriPermission`(Uri) ~222、`GrantUriPermission`(vec,bundle) ~258、`GrantUriPermission`(vec,tokenId) ~377、`GrantUriPermission`(rawData) ~432、`GrantUriPermissionPrivileged`(vec) ~818、`GrantUriPermissionPrivileged`(rawData) ~867、`GrantUriPermissionWithType` ~887、`GrantUriPermissionByKey` ~1025、`GrantUriPermissionByKeyAsCaller` ~1007、`Active`(sandbox) ~1820
- Revoke 侧（3）：`RevokeAllUriPermissions` ~1427、`RevokeUriPermissionManually` ~1510、`ClearPermissionTokenByMap` ~1717
- **不插**：`CheckUriAuthorization*`、`VerifyUriPermission`（读路径，ADR-3）。
- 注：行号为取证时快照，以当前实际方法体为准；插桩前逐一核对方法名，避免误插内部 Inner/Impl 方法。

### D. service.cpp — SelfClean 增强（ADR-4）
[uri_permission_manager_service.cpp:82-90](../../../../services/uripermmgr/src/uri_permission_manager_service.cpp) `SelfClean()` 内，在置 flag 后加：
```cpp
if (impl_ != nullptr) {
    impl_->StopHandler();
    impl_ = nullptr;   // 下次 OnStart→Init 创建全新空账本 stub_impl
}
```

## 完成标准
- [ ] 6 个新方法实现，零捕获 lambda 正确
- [ ] 13 处插桩逐一核对（grant 10 + revoke 3），无遗漏、无误插读路径
- [ ] `SelfClean` 增强正确
- [ ] `libupms` 编译通过：`./build.sh --product-name <product> --build-target libupms`
- [ ] 无 lint/规范问题（hilog 用 `AAFwkTag::URIPERMMGR`、include guard、命名空间一致）

## 风险点
- cancel 穷尽：漏一个 grant → 误卸载泄漏（最高优先核对）。
- 零捕获：若误捕获 `this`，自身线程卸载悬空。
- 锁顺序：`IsLedgerEmpty` 必须按既有顺序，避免与清账路径死锁。
