# Commit 逐行分析：`aa3692318bfecf558e973451c8e343f805a93fa3`

> **提交标题**：前台组件 并发问题修复
> **作者**：liuzongze@h-partners.com
> **日期**：2026-08-18
> **规模**：17 文件，+649 / −284 行
> **Change-Id**：`Id86fc9a3d51195932a82d84b9fbe2a75c80faf2a`

---

## 目录

- [1. 总体目标与设计思路](#1-总体目标与设计思路)
- [2. `ability_stage.h` — 新增 StageState 状态机](#2-ability_stageh--新增-stagestate-状态机)
- [3. `ohos_application.h` — 新增成员与访问约束声明](#3-ohos_applicationh--新增成员与访问约束声明)
- [4. `ohos_application.cpp` — 核心并发修复（~395 行）](#4-ohos_applicationcpp--核心并发修复约-395-行)
- [5. `ability_lifecycle_executor.h / .cpp` — 状态原子化](#5-ability_lifecycle_executorh--cpp--状态原子化)
- [6. `extension_ability_thread.h / .cpp` — ContentEmbed EventRunner 竞争修复](#6-extension_ability_threadh--cpp--contentembed-eventrunner-竞争修复)
- [7. `task_handler_client.h / .cpp` — 单例与 taskHandler 竞争修复](#7-task_handler_clienth--cpp--单例与-taskhandler-竞争修复)
- [8. `js_worker.cpp` — 全局标志原子化](#8-js_workercpp--全局标志原子化)
- [9. `dynamic_loader.h / .cpp` — dlopen 错误缓冲与初始化表竞争修复](#9-dynamic_loaderh--cpp--dlopen-错误缓冲与初始化表竞争修复)
- [10. `assert_fault_proxy.cpp` — 简化 DCLP](#10-assert_fault_proxycpp--简化-dclp)
- [11. `lifecycle_deal.cpp` — 改用线程安全访问器](#11-lifecycle_dealcpp--改用线程安全访问器)
- [12. 测试文件变更分析](#12-测试文件变更分析)
- [13. 风险评估](#13-风险评估)
- [14. 测试覆盖评估](#14-测试覆盖评估)

---

## 1. 总体目标与设计思路

本提交系统性消除 `ability_runtime` 前台组件路径上的一批**数据竞争（data race）**，涉及以下共享状态：

| 共享状态 | 原问题 | 修复策略 |
|----------|--------|----------|
| `OHOSApplication::abilityStages_`（unordered_map） | 多线程并发读写无同步 | 新增 `abilityStagesMutex_` + 快照迭代 + 早插入消除重入窗口 |
| `OHOSApplication::runtime_`（unique_ptr） | SetRuntime/UpdateETSRuntime 与读取方竞争 | `runtimeMutex_` 全路径加锁 |
| `AbilityStage` 中间态 | 无状态标记，并发可操作半初始化对象 | 新增 `StageState` 原子状态机 |
| `AbilityLifecycleExecutor::state_` | 非原子读写 | `std::atomic` + acquire/release |
| `TaskHandlerClient::instance_` + `taskHandler_` | DCLP 单例 + 成员无锁读 | Meyers 单例 + 成员锁保护 |
| `ExtensionAbilityThread::contentEmbedEventRunner_` | 静态成员 check-then-create 竞争 | 静态 mutex 保护创建 |
| `JsWorker::g_jsFramework` | bool + mutex | `std::atomic<bool>` |
| `DynamicLoader::g_dlError` / `g_hasInited` | 全局 buffer 线程间覆盖 / 无锁集合 | thread_local + mutex |

**贯穿全提交的设计模式**：
1. **快照迭代（snapshot-then-iterate）**：锁内拷贝 `shared_ptr` 到局部容器，锁外遍历调用——避免持锁回调引发死锁与重入。
2. **早插入（early insertion）**：AbilityStage 在 `Init()` 后立即入表，消除"null 占位 → 回填"窗口。
3. **状态机守卫（state guard）**：用 `StageState` 阻止对 INITIALIZING/DESTROYING 态对象的操作。
4. **锁外析构（destroy outside lock）**：`OnDestroy` 在锁外调用，局部 `shared_ptr` 保活。

---

## 2. `ability_stage.h` — 新增 StageState 状态机

**文件**：`interfaces/kits/native/appkit/ability_runtime/app/ability_stage.h`
**性质**：公共 SDK 头文件（🔴 公共 API 兼容性边界）

### 2.1 新增 `<atomic>` 头

```cpp
+#include <atomic>
```

**意义**：为下方 `std::atomic<StageState>` 成员提供依赖。原先此头文件不含并发原语，引入 `atomic` 标志着 `AbilityStage` 正式成为线程安全可观测的对象。

### 2.2 新增 `StageState` 枚举

```cpp
+enum class StageState : uint8_t {
+    UNINITIALIZED,
+    INITIALIZING,
+    READY,
+    DESTROYING,
+};
```

**意义**：定义 AbilityStage 的四态生命周期：
- `UNINITIALIZED`：对象已构造但尚未 `Init()`。
- `INITIALIZING`：`Init()` 完成、已入表，但 `LoadModule / RunAutoStartupTask / OnCreate` 尚在进行中。
- `READY`：`OnCreate` 完成，可安全接收配置更新、内存告警等回调。
- `DESTROYING`：已从表移除或即将移除，`OnDestroy` 待执行或执行中。

**`uint8_t` 显式指定**：节省内存（atomic 原子操作按平台可能按 1/2/4/8 字节处理），`uint8_t` 保证单字节，多数平台可映射到单字节原子指令。

**scoped enum（`enum class`）**：防止命名污染，避免隐式转换为 int，类型安全。

### 2.3 新增 `GetStageState / SetStageState` 内联访问器

```cpp
+    StageState GetStageState() const
+    {
+        return stageState_.load(std::memory_order_acquire);
+    }
+    void SetStageState(StageState state) const
+    {
+        stageState_.store(state, std::memory_order_release);
+    }
```

**意义**：
- `GetStageState` 用 `acquire` 读：保证调用方看到 `READY` 时，也看到该状态之前（`release` store 前）的所有写入（如 `OnCreate` 内对成员的初始化），即建立了 **happens-before** 关系。
- `SetStageState` 用 `release` 写：保证状态变更之前的所有内存写入（如 `Init()` 中的赋值）对后续 `acquire` 读的线程可见。
- `const` 修饰（`SetStageState` 也标 `const`）：因为 `stageState_` 是 `mutable`，可在 const 方法中修改。这允许 `ScheduleAcceptWant` 等 const 语义的方法读取状态。

### 2.4 新增 `stageState_` 原子成员

```cpp
+    mutable std::atomic<StageState> stageState_{StageState::UNINITIALIZED};
```

**意义**：
- `mutable`：允许在 const 成员函数中修改（与 `GetStageState/SetStageState` 的 `const` 配合）。
- `std::atomic`：无锁原子访问，比 mutex 轻量，适合单一标志的发布-订阅场景。
- 默认值 `UNINITIALIZED`：对象构造即处于未初始化态。

---

## 3. `ohos_application.h` — 新增成员与访问约束声明

**文件**：`interfaces/kits/native/appkit/app/ohos_application.h`
**性质**：公共 SDK 头文件（🔴）

### 3.1 新增 `RunStageStartupTasks` 私有方法声明

```cpp
+    bool RunStageStartupTasks(
+        std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
+        std::shared_ptr<AbilityLocalRecord> abilityRecord,
+        const AppExecFwk::HapModuleInfo &hapModuleInfo,
+        const std::function<void(std::shared_ptr<AbilityRuntime::Context>)> &callback,
+        bool &isAsyncCallback,
+        const std::shared_ptr<AbilityRuntime::Context> &stageContext,
+        const AAFwk::Want &want);
```

**意义**：将原先内联在 `AddAbilityStage` 中的 `LoadModule → RunAutoStartupTask → OnCreate` 流程提取为独立私有方法。原因有二：
1. 提取后 `AddAbilityStage` 更清晰，锁逻辑与启动流程分离。
2. 使得"早插入后执行启动任务"的控制流更可读。

### 3.2 新增 `abilityStagesMutex_`

```cpp
     std::unordered_map<std::string, std::shared_ptr<AbilityRuntime::AbilityStage>> abilityStages_;
+    mutable std::mutex abilityStagesMutex_;
```

**意义**：
- 紧邻 `abilityStages_` 声明，表明二者绑定，所有对 `abilityStages_` 的访问须持此锁。
- `mutable`：允许 const 方法（如 `ScheduleAcceptWant` 等）加锁读 map。

### 3.3 `runtimeMutex_` 注释约束

```cpp
     std::unique_ptr<AbilityRuntime::Runtime> runtime_ = nullptr;
+    // runtime_ 由 unique_ptr 独占，调用方须持 runtimeMutex_ 才能保证对象在调用期间不被
+    // SetRuntime/UpdateETSRuntime 并发析构，故锁内会调用 Runtime::*（NotifyApplicationState /
+    // PreloadModule 等）。锁序不变量：这些 Runtime::* 方法不得反向回调 SetRuntime/UpdateETSRuntime。
```

**意义**：这是**锁序不变量（lock ordering invariant）**的显式文档化声明：
- 锁内调用 `Runtime::NotifyApplicationState / PreloadModule` 等方法，如果这些方法反向回调 `SetRuntime` 或 `UpdateETSRuntime`（它们也要获取 `runtimeMutex_`），会**自死锁**。
- 此注释是对后续维护者的警告，但无编译期或运行期强制保障。

---

## 4. `ohos_application.cpp` — 核心并发修复（~395 行）

**文件**：`frameworks/native/appkit/app/ohos_application.cpp`

### 4.1 新增 `<vector>` 头

```cpp
+#include <vector>
```

**意义**：快照迭代模式需要在锁内将 `shared_ptr` 拷贝到 `std::vector`，锁外遍历。引入 vector 依赖。

---

### 4.2 `OnForeground()` — runtime_ 读取加锁

```cpp
-    if (runtime_ == nullptr) {
-        TAG_LOGD(AAFwkTag::APPKIT, "NotifyApplicationState, runtime_ is nullptr");
-        return;
-    }
-    runtime_->NotifyApplicationState(false);
+    {
+        std::lock_guard<std::mutex> lock(runtimeMutex_);
+        if (runtime_ == nullptr) {
+            TAG_LOGD(AAFwkTag::APPKIT, "NotifyApplicationState, runtime_ is nullptr");
+            return;
+        }
+        runtime_->NotifyApplicationState(false);
+    }
```

**逐行分析**：
- `{` 开作用域 + `lock_guard`：RAII 加锁，作用域结束时自动释放。
- `if (runtime_ == nullptr)`：空指针检查在锁内，保证检查与使用之间不被 `SetRuntime(nullptr)` 或 `UpdateETSRuntime` 修改。
- `runtime_->NotifyApplicationState(false)`：在锁内调用 Runtime 方法。根据头文件注释的约束，此方法不应反向回调 SetRuntime/UpdateETSRuntime。
- 原代码**先判空、后在无锁状态下调用 `runtime_->NotifyApplicationState`**，如果另一线程在此期间执行 `UpdateETSRuntime`（会 `std::move` 走 runtime_），此处解引用的是已 move-from 的 unique_ptr（空指针或未定义行为），属于 **TOCTOU（time-of-check-to-time-of-use）竞争**。

---

### 4.3 `OnBackground()` — 同理加锁

```cpp
-    if (runtime_ == nullptr) {
-        TAG_LOGD(AAFwkTag::APPKIT, "runtime_ is nullptr");
-        return;
-    }
-    runtime_->NotifyApplicationState(true);
+    {
+        std::lock_guard<std::mutex> lock(runtimeMutex_);
+        if (runtime_ == nullptr) {
+            TAG_LOGD(AAFwkTag::APPKIT, "runtime_ is nullptr");
+            return;
+        }
+        runtime_->NotifyApplicationState(true);
+    }
```

**意义**：与 `OnForeground` 完全对称，参数为 `true`（表示后台）。修复同样的 TOCTOU 竞争。

---

### 4.4 `SetRuntime()` — 加锁写入

```cpp
+    std::lock_guard<std::mutex> lock(runtimeMutex_);
     runtime_ = std::move(runtime);
```

**意义**：`SetRuntime` 与所有 `runtime_` 读取方现在通过 `runtimeMutex_` 互斥。`std::move` 是非原子操作（修改 unique_ptr 内部指针），必须加锁。原先 `SetRuntime` 无锁，可与其他线程的读取竞争。

---

### 4.5 `OnConfigurationUpdated()` — 快照迭代 + 状态守卫

```cpp
-    for (auto it = abilityStages_.begin(); it != abilityStages_.end(); it++) {
-        auto abilityStage = it->second;
-        if (abilityStage && !abilityStage->IsSkipAbilityStageLifecycle()) {
-            abilityStage->OnConfigurationUpdated(config);
+    std::vector<std::shared_ptr<AbilityRuntime::AbilityStage>> stages;
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        for (const auto &item : abilityStages_) {
+            if (item.second && !item.second->IsSkipAbilityStageLifecycle()) {
+                stages.push_back(item.second);
+            }
+        }
+    }
+    for (const auto &stage : stages) {
+        if (stage->GetStageState() != AbilityRuntime::StageState::READY) {
+            continue;
+        }
+        stage->OnConfigurationUpdated(config);
     }
```

**逐行分析**：
- `std::vector<...> stages`：局部快照容器。
- `lock(abilityStagesMutex_)`：锁内遍历 map，将符合条件的 `shared_ptr` 拷贝到 `stages`。`shared_ptr` 拷贝增引用计数，即使 map 中对应条目被并发 erase，stage 对象也不会被析构。
- 锁外遍历 `stages`：避免持锁调用 `OnConfigurationUpdated`，该回调可能间接触发 `AddAbilityStage` 或 `CleanAbilityStage`（会获取同一把锁），导致死锁。
- `GetStageState() != READY → continue`：**新增的行为守卫**。快照捕获时 stage 可能是 READY，但在锁外调用前，另一线程可能将其置为 DESTROYING。此检查在调用前再次校验状态。注意这是 **best-effort check**（check 与 use 之间仍有窗口），但显著降低风险。

---

### 4.6 `OnMemoryLevel()` — 快照迭代 + 状态守卫

```cpp
-    TAG_LOGD(AAFwkTag::APPKIT, "Number of abilityStage to be notified : [%{public}zu]", abilityStages_.size());
-    for (auto it = abilityStages_.begin(); it != abilityStages_.end(); it++) {
-        auto abilityStage = it->second;
-        if (abilityStage && !abilityStage->IsSkipAbilityStageLifecycle()) {
-            abilityStage->OnMemoryLevel(level);
+    std::vector<std::shared_ptr<AbilityRuntime::AbilityStage>> stages;
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        TAG_LOGD(AAFwkTag::APPKIT, "Number of abilityStage to be notified : [%{public}zu]", abilityStages_.size());
+        for (const auto &item : abilityStages_) {
+            if (item.second && !item.second->IsSkipAbilityStageLifecycle()) {
+                stages.push_back(item.second);
+            }
+        }
+    }
+    for (const auto &stage : stages) {
+        if (stage->GetStageState() != AbilityRuntime::StageState::READY) {
+            continue;
+        }
+        stage->OnMemoryLevel(level);
     }
```

**意义**：与 `OnConfigurationUpdated` 完全同构。日志 `abilityStages_.size()` 也移入锁内，避免读取 size 时与 erase/emplace 竞争（unordered_map 的 size() 读在并发写时是 UB）。

---

### 4.7 `OnHyperSnapUpdate()` — 快照迭代 + 状态守卫

```cpp
-    for (auto& item : abilityStages_) {
-        if (item.second && !item.second->IsSkipAbilityStageLifecycle()) {
-            item.second->OnLaunchFromHyperSnap();
+    std::vector<std::shared_ptr<AbilityRuntime::AbilityStage>> stages;
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        for (const auto &item : abilityStages_) {
+            if (item.second && !item.second->IsSkipAbilityStageLifecycle()) {
+                stages.push_back(item.second);
+            }
+        }
+    }
+    for (const auto &stage : stages) {
+        if (stage->GetStageState() != AbilityRuntime::StageState::READY) {
+            continue;
+        }
+        stage->OnLaunchFromHyperSnap();
     }
```

**意义**：同一模式第三处应用。三处一致使用快照 + 状态守卫。

---

### 4.8 `PreloadHybridModule()` — runtime_ 加锁

```cpp
-    runtime_->PreloadModule(hapModuleInfo.moduleName, hapModuleInfo.hapPath, isEsmode, useCommonTrunk);
+    {
+        std::lock_guard<std::mutex> lock(runtimeMutex_);
+        if (runtime_ == nullptr) {
+            TAG_LOGD(AAFwkTag::APPKIT, "null runtime in PreloadHybridModule");
+            return;
+        }
+        runtime_->PreloadModule(hapModuleInfo.moduleName, hapModuleInfo.hapPath, isEsmode, useCommonTrunk);
+    }
```

**意义**：
- 原代码直接解引用 `runtime_` 无任何保护，且无空指针检查——若 runtime 未设置则空指针解引用崩溃。
- 新增空指针检查 + 加锁。注意此函数本身是 `const` 方法但此处修改的是经 mutex 保护的 `runtime_`（unique_ptr），需要确认 const 语义下 mutex 也声明为 mutable（已在头文件中）。

---

### 4.9 `CreateAndInitStageContext()`（第一个重载） — SetPkgContextInfoJson 加锁

```cpp
-    if (runtime_ && (runtime_->GetLanguage() == AbilityRuntime::Runtime::Language::JS)) {
-        static_cast<AbilityRuntime::JsRuntime&>(*runtime_).SetPkgContextInfoJson(
-            moduleInfo->moduleName, moduleInfo->hapPath, moduleInfo->packageName);
+    {
+        std::lock_guard<std::mutex> lock(runtimeMutex_);
+        if (runtime_ && (runtime_->GetLanguage() == AbilityRuntime::Runtime::Language::JS)) {
+            static_cast<AbilityRuntime::JsRuntime&>(*runtime_).SetPkgContextInfoJson(
+                moduleInfo->moduleName, moduleInfo->hapPath, moduleInfo->packageName);
+        }
     }
```

**意义**：`runtime_` 读取 + `GetLanguage()` + `static_cast` + `SetPkgContextInfoJson` 全程在锁内原子完成。原代码中如果在这几步之间 `UpdateETSRuntime` 将 runtime_ 替换为 ETS runtime（语言变为 ETS），则 `static_cast<JsRuntime&>` 会错误转型，UB。

---

### 4.10 新增 `RunStageStartupTasks()` 方法

```cpp
+bool OHOSApplication::RunStageStartupTasks(
+    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage,
+    std::shared_ptr<AbilityLocalRecord> abilityRecord,
+    const AppExecFwk::HapModuleInfo &hapModuleInfo,
+    const std::function<void(std::shared_ptr<AbilityRuntime::Context>)> &callback,
+    bool &isAsyncCallback,
+    const std::shared_ptr<AbilityRuntime::Context> &stageContext,
+    const AAFwk::Want &want)
+{
+    auto firstCallback = CreateFirstStartupCallbackForRecord(abilityStage, abilityRecord, hapModuleInfo, callback);
+    if (firstCallback != nullptr) {
+        abilityStage->RunAutoStartupTask(firstCallback, abilityRecord->GetWant(), isAsyncCallback,
+            stageContext, true);
+        if (isAsyncCallback) {
+            TAG_LOGI(AAFwkTag::APPKIT, "wait startup first");
+            return true;
+        }
+    }
+
+    TAG_LOGD(AAFwkTag::APPKIT, "no wait startup first");
+    abilityStage->LoadModule(hapModuleInfo);
+    auto secondCallback = CreateSecondStartupCallbackForRecord(abilityStage, abilityRecord, callback);
+    if (firstCallback != nullptr) {
+        abilityStage->RunAutoStartupTask(secondCallback, abilityRecord->GetWant(), isAsyncCallback,
+            abilityStage->GetContext(), false);
+        if (isAsyncCallback) {
+            TAG_LOGI(AAFwkTag::APPKIT, "wait startup second");
+            return true;
+        }
+    }
+
+    TAG_LOGD(AAFwkTag::APPKIT, "no wait startup second");
+    if (!abilityStage->IsSkipAbilityStageLifecycle()) {
+        abilityStage->OnCreate(want);
+    }
+    abilityStage->SetStageState(AbilityRuntime::StageState::READY);
+    return false;
+}
```

**逐行分析**：
- 此方法是原 `AddAbilityStage` 中段逻辑的**原样提取**，行为未变，只是换了位置。
- 末尾新增 `abilityStage->SetStageState(READY)`：启动流程全部完成（含 OnCreate）后将状态置 READY，使其可接收配置更新/内存告警等回调。
- `return true` 表示异步等待（callback 将在异步完成后回调 `AutoStartupDone`），`return false` 表示同步完成。
- 提取此方法的目的：让 `AddAbilityStage` 在早插入后可以方便地调用，并使"早插入 → 启动任务 → 置 READY"的控制流清晰可读。

---

### 4.11 `AddAbilityStage(abilityRecord)` — 早插入重构（核心改动）

这是本次提交最关键的架构变更。分步分析：

#### 4.11.1 查找改为锁内拷贝

```cpp
-    auto iterator = abilityStages_.find(moduleName);
-    if (iterator == abilityStages_.end()) {
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        auto iterator = abilityStages_.find(moduleName);
+        if (iterator != abilityStages_.end()) {
+            abilityStage = iterator->second;
+        }
+    }
+    if (abilityStage == nullptr) {
```

**意义**：
- 锁内 find，找到则拷贝 `shared_ptr` 到局部变量 `abilityStage`，锁外使用。
- 原代码的 `iterator` 在锁外持续使用（`iterator->second`），如果期间 map 被 rehash 或 erase，iterator 失效——**UB / 崩溃**。
- `if (abilityStage == nullptr)` 替代原来的 `if (iterator == end)`：因为现在用的是拷贝出来的指针，null 表示未找到。

#### 4.11.2 第二处 SetPkgContextInfoJson 加锁

```cpp
-        if (runtime_ && (runtime_->GetLanguage() == AbilityRuntime::Runtime::Language::JS)) {
-            static_cast<AbilityRuntime::JsRuntime&>(*runtime_).SetPkgContextInfoJson(
-                hapModuleInfo->moduleName, hapModuleInfo->hapPath, hapModuleInfo->packageName);
+        {
+            std::lock_guard<std::mutex> lock(runtimeMutex_);
+            if (runtime_ && (runtime_->GetLanguage() == AbilityRuntime::Runtime::Language::JS)) {
+                static_cast<AbilityRuntime::JsRuntime&>(*runtime_).SetPkgContextInfoJson(
+                    hapModuleInfo->moduleName, hapModuleInfo->hapPath, hapModuleInfo->packageName);
+            }
         }
```

**意义**：与 4.9 相同的修复，此处是 `AddAbilityStage` 内部的第二处 SetPkgContextInfoJson 调用。

#### 4.11.3 早插入 + 状态标记 + 并发抑制

```cpp
         abilityStage->Init(stageContext, weak);
+        abilityStage->SetStageState(AbilityRuntime::StageState::INITIALIZING);
         abilityStage->SetSkipAbilityStageLifecycle(abilityRecord->IsSkipAbilityStageLifecycle());
-        auto firstCallback = ...
-        ...（原内联启动逻辑）
-        abilityStages_[moduleName] = abilityStage;
-    } else {
-        abilityStage = iterator->second;
+        // Insert into the table/registry immediately after Init completes,
+        // eliminating the reentrant null-pointer window between the "placeholder nullptr → backfill":
+        // Reentrant calls from the same module will hit this valid stage and reuse it,
+        // instead of reading the nullptr placeholder.
+        bool reuseExisting = false;
+        {
+            std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+            auto result = abilityStages_.emplace(moduleName, abilityStage);
+            if (!result.second) {
+                TAG_LOGW(AAFwkTag::APPKIT, "concurrent stage insert suppressed for %{public}s",
+                    moduleName.c_str());
+                abilityStage = result.first->second;
+                reuseExisting = true;
+            }
+        }
+        if (!reuseExisting) {
+            if (RunStageStartupTasks(abilityStage, abilityRecord, *hapModuleInfo, callback, isAsyncCallback,
+                stageContext, want)) {
                 return nullptr;
+            }
     }
```

**逐行分析**：
- `SetStageState(INITIALIZING)`：Init 完成后立即标记为初始化中。
- `emplace(moduleName, abilityStage)`：**早插入**——在执行 LoadModule/RunAutoStartupTask/OnCreate 之前就将 stage 入表。原代码在全部完成后才 `abilityStages_[moduleName] = abilityStage`，这导致：
  - 重入调用 `find()` 返回 `end()` → 误判为"不存在" → 重复创建 stage。
  - 如果原代码用了"先占位 nullptr 再回填"的模式，重入会读到 nullptr。
- `emplace` 返回 `pair<iterator, bool>`：`result.second == false` 表示键已存在（并发竞争时另一线程先插入），此时 `result.first->second` 是已存在的 stage。
- `reuseExisting = true`：本线程放弃新创建的 stage，复用已存在的。**注意：复用的 stage 可能仍是 INITIALIZING 态**，本调用方直接跳过启动任务使用它。
- `if (!reuseExisting)` 才执行 `RunStageStartupTasks`：避免对同一 stage 重复执行启动流程。
- 日志 "concurrent stage insert suppressed"：记录并发竞争事件，便于诊断。

#### 4.11.4 原 else 分支删除

```cpp
-    } else {
-        abilityStage = iterator->second;
-    }
```

**意义**：原 else 分支处理"已存在则复用"。现在查找已提前在锁内完成并赋值给 `abilityStage`，无需 else。

---

### 4.12 `AutoStartupDone()` — 置 READY + 锁内写入

```cpp
     abilityStage->OnCreate(want);
-    abilityStages_[moduleName] = abilityStage;
+    abilityStage->SetStageState(AbilityRuntime::StageState::READY);
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        abilityStages_[moduleName] = abilityStage;
+    }
```

**意义**：
- `AutoStartupDone` 是异步启动完成后的回调路径。stage 已在 `AddAbilityStage` 中早插入，此处再次赋值（同一对象，幂等）。
- `SetStageState(READY)`：异步启动全部完成，标记可接收回调。
- 锁内赋值：与其他读取方互斥。

第二处 `AutoStartupDone`（hapModuleInfo 重载）同理：
```cpp
-    abilityStages_[hapModuleInfo.moduleName] = abilityStage;
+    abilityStage->SetStageState(AbilityRuntime::StageState::READY);
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        abilityStages_[hapModuleInfo.moduleName] = abilityStage;
+    }
```

---

### 4.13 `AddAbilityStage(HapModuleInfo)` — 第二个重载的早插入

```cpp
-    if (runtime_ == nullptr) {
-        TAG_LOGE(AAFwkTag::APPKIT, "null runtime_");
-        return false;
+    {
+        std::lock_guard<std::mutex> lock(runtimeMutex_);
+        if (runtime_ == nullptr) {
+            TAG_LOGE(AAFwkTag::APPKIT, "null runtime_");
+            return false;
+        }
     }
 
-    if (abilityStages_.find(hapModuleInfo.moduleName) != abilityStages_.end()) {
-        TAG_LOGE(AAFwkTag::APPKIT, "object exist");
-        return false;
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        if (abilityStages_.find(hapModuleInfo.moduleName) != abilityStages_.end()) {
+            TAG_LOGE(AAFwkTag::APPKIT, "object exist");
+            return false;
+        }
     }
```

**意义**：runtime_ 空检查 + abilityStages_ 存在性检查均加锁。原 find 与后续 insert 之间有窗口，可被并发插入。现在虽然有早插入，但此重载的检查逻辑仍保留。

```cpp
     abilityStage->Init(stageContext, weak);
+    abilityStage->SetStageState(AbilityRuntime::StageState::INITIALIZING);
+    // Insert into the table immediately after Init completes,
+    // eliminating the reentrant window between the "placeholder nullptr → backfill".
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        auto result = abilityStages_.emplace(hapModuleInfo.moduleName, abilityStage);
+        if (!result.second) {
+            TAG_LOGW(AAFwkTag::APPKIT, "concurrent stage insert suppressed for %{public}s",
+                hapModuleInfo.moduleName.c_str());
+            return false;
+        }
+    }
```

**意义**：与 4.11.3 相同的早插入模式。此重载在并发冲突时直接 `return false`（因为接口语义是 bool 返回，不像 record 重载可复用）。

```cpp
     if (!ExecuteAutoStartupTasks(abilityStage, hapModuleInfo, callback, isAsyncCallback, stageContext)) {
+        {
+            std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+            abilityStage->SetStageState(AbilityRuntime::StageState::DESTROYING);
+            abilityStages_.erase(hapModuleInfo.moduleName);
+        }
         return false;
     }
 
     TAG_LOGD(AAFwkTag::APPKIT, "no wait startup second");
     Want want;
     abilityStage->OnCreate(want);
-    abilityStages_[hapModuleInfo.moduleName] = abilityStage;
+    abilityStage->SetStageState(AbilityRuntime::StageState::READY);
     TAG_LOGI(AAFwkTag::APPKIT, "abilityStage insert and initialization");
     return true;
```

**意义**：
- `ExecuteAutoStartupTasks` 失败时：**新增清理逻辑**——置 DESTROYING 并从表 erase。原代码失败时直接 return false 但不清理已入表的 stage，导致后续调用永远 find 到一个半初始化的 stage。这是**资源泄漏 + 状态污染**修复。
- 成功路径：置 READY（stage 已在前面早插入，无需再次赋值）。

---

### 4.14 `CleanAbilityStage()` — 安全销毁

```cpp
-    auto iterator = abilityStages_.find(moduleName);
-    if (iterator != abilityStages_.end()) {
-        auto abilityStage = iterator->second;
-        if (abilityStage == nullptr) {
-            TAG_LOGE(AAFwkTag::APPKIT, "null abilityStage");
-            return;
+    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage;
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        auto iterator = abilityStages_.find(moduleName);
+        if (iterator != abilityStages_.end()) {
+            abilityStage = iterator->second;
         }
+    }
+    bool shouldDestroy = false;
+    if (abilityStage != nullptr) {
         abilityStage->RemoveAbility(token);
         if (!abilityStage->ContainsAbility() && !isCacheProcess) {
-            if (!abilityStage->IsSkipAbilityStageLifecycle()) {
-                abilityStage->OnDestroy();
+            shouldDestroy = true;
+        }
+    }
+    if (shouldDestroy) {
+        // First mark it as DESTROYING and remove it from the map (inside the lock),
+        // so that concurrent snapshot/find no longer hit the stage that is being destroyed.
+        // then call OnDestroy outside the lock,
+        // with a local shared_ptr keeping the object alive until destruction completes.
+        bool skipErase = false;
+        {
+            std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+            // Re-check ContainsAbility inside the lock:
+            // avoid mistakenly deleting a stage that has become non-empty due to a concurrent AddAbility.
+            if (abilityStage->ContainsAbility()) {
+                skipErase = true;
+            } else {
+                abilityStage->SetStageState(AbilityRuntime::StageState::DESTROYING);
+                abilityStages_.erase(moduleName);
+            }
+        }
+        if (!skipErase && !abilityStage->IsSkipAbilityStageLifecycle()) {
+            abilityStage->OnDestroy();
         }
-        abilityStages_.erase(moduleName);
     }
```

**逐行分析**：
- **第一次加锁**：find + 拷贝 shared_ptr 到局部，锁外使用。
- `RemoveAbility(token)` + `ContainsAbility()` 检查：**在锁外执行**。RemoveAbility 修改 stage 内部 ability 列表，不影响 map 结构，无需持 map 锁。
- `shouldDestroy` 标志：避免在持锁状态调用 OnDestroy（OnDestroy 可能回调 OHOSApplication 方法获取同一把锁 → 死锁）。
- **第二次加锁 + 二次检查**：
  - `abilityStage->ContainsAbility()` 在锁内重新检查：因为第一次检查（锁外）与 erase（锁内）之间，可能有并发 `AddAbility` 向该 stage 添加了 ability。如果有，则跳过 erase（`skipErase = true`），防止误删非空 stage。
  - `SetStageState(DESTROYING)` + `erase`：先标记后删除，使并发 find 能看到 DESTROYING 态（即使引用还在）。
- **锁外 OnDestroy**：局部 `shared_ptr abilityStage` 仍持有引用计数，保证对象在 OnDestroy 执行期间不被析构（erase 只移除 map 条目，不析构对象，因为 shared_ptr 引用计数 > 0）。

**原代码问题**：
- 无锁 find + erase：并发 erase 后 iterator 失效。
- 无二次检查 ContainsAbility：并发 AddAbility 可使已判空的 stage 变非空，但仍被删除。

---

### 4.15 `ScheduleAcceptWant()` — 锁内查找 + 状态守卫

```cpp
-    auto iter = abilityStages_.find(moduleName);
-    if (iter == abilityStages_.end() || iter->second == nullptr) {
+    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage;
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        auto iter = abilityStages_.find(moduleName);
+        if (iter != abilityStages_.end()) {
+            abilityStage = iter->second;
+        }
+    }
+    if (abilityStage == nullptr) {
         ...
+    }
+    if (abilityStage->GetStageState() != AbilityRuntime::StageState::READY) {
+        TAG_LOGW(AAFwkTag::APPKIT, "%{public}s stage not ready, skip OnAcceptWant", moduleName.c_str());
         std::string flag;
         callback(flag);
         return;
     }
```

**意义**：
- 锁内查找 + 拷贝。
- 新增状态守卫：非 READY 态直接回调空 flag 返回。**行为变更**：启动期间的 AcceptWant 请求被静默丢弃。

```cpp
-    if (iter->second->OnAcceptWant(want, callbackInfo, isAsync).empty()) {
+    if (abilityStage->OnAcceptWant(want, callbackInfo, isAsync).empty()) {
```

**意义**：用局部变量替代 `iter->second`（iterator 在锁外已失效风险）。

---

### 4.16 `SchedulePrepareTerminate()` — 同构修复

```cpp
-    auto iter = abilityStages_.find(moduleName);
-    if (iter == abilityStages_.end() || iter->second == nullptr) {
+    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage;
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        auto iter = abilityStages_.find(moduleName);
+        if (iter != abilityStages_.end()) {
+            abilityStage = iter->second;
+        }
+    }
+    if (abilityStage == nullptr) {
         ...
+    }
+    if (abilityStage->GetStageState() != AbilityRuntime::StageState::READY) {
+        TAG_LOGW(AAFwkTag::APPKIT, "%{public}s stage not ready, skip OnPrepareTerminate", moduleName.c_str());
+        return;
     }
```

**意义**：同 ScheduleAcceptWant，锁内查找 + 状态守卫。非 READY 直接返回。

```cpp
-    if (iter->second->IsSkipAbilityStageLifecycle() || !iter->second->OnPrepareTerminate(callbackInfo, isAsync)) {
+    if (abilityStage->IsSkipAbilityStageLifecycle() || !abilityStage->OnPrepareTerminate(callbackInfo, isAsync)) {
```

---

### 4.17 `ScheduleNewProcessRequest()` — 同构修复

```cpp
-    if (abilityStages_.empty()) {
+    std::shared_ptr<AbilityRuntime::AbilityStage> abilityStage;
+    bool isEmpty = false;
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        if (abilityStages_.empty()) {
+            isEmpty = true;
+        } else {
+            auto iter = abilityStages_.find(moduleName);
+            if (iter != abilityStages_.end()) {
+                abilityStage = iter->second;
+            }
+        }
+    }
+    std::string flag;
+    if (isEmpty) {
         ...
-    auto iter = abilityStages_.find(moduleName);
-    if (iter == abilityStages_.end() || iter->second == nullptr) {
+    if (abilityStage == nullptr) {
         ...
+    }
+    if (abilityStage->GetStageState() != AbilityRuntime::StageState::READY) {
+        TAG_LOGW(AAFwkTag::APPKIT, "%{public}s stage not ready, skip OnNewProcessRequest", moduleName.c_str());
         callback(flag);
         return;
     }
```

**意义**：
- empty() 检查也加锁（`unordered_map::empty()` 在并发写时是 UB）。
- `std::string flag` 提前声明，减少三处重复。
- 状态守卫同前。

```cpp
-    if (iter->second->OnNewProcessRequest(want, callbackInfo, isAsync).empty()) {
+    if (abilityStage->OnNewProcessRequest(want, callbackInfo, isAsync).empty()) {
```

---

### 4.18 `NotifyLoadRepairPatch / NotifyHotReloadPage / NotifyUnLoadRepairPatch` — runtime_ 加锁

```cpp
+    std::lock_guard<std::mutex> lock(runtimeMutex_);
     if (runtime_ == nullptr) {
```

**意义**：三处 QuickFix 相关方法原直接读 `runtime_` 无锁，现加锁。这三处在 QuickFix 热修复流程中调用，与 `SetRuntime`/`UpdateETSRuntime` 可并发。

---

### 4.19 `CleanEmptyAbilityStage()` — 快照 + 锁外析构

```cpp
-    for (auto it = abilityStages_.begin(); it != abilityStages_.end();) {
-        auto abilityStage = it->second;
-        if (abilityStage == nullptr) {
-            it++;
-            continue;
-        }
-        if (!abilityStage->ContainsAbility()) {
-            if (!abilityStage->IsSkipAbilityStageLifecycle()) {
-                abilityStage->OnDestroy();
-            }
-            it = abilityStages_.erase(it);
-        } else {
-            containsNonEmpty = true;
-            it++;
+    std::vector<std::string> toErase;
+    std::vector<std::shared_ptr<AbilityRuntime::AbilityStage>> toDestroy;
+    {
+        std::lock_guard<std::mutex> lock(abilityStagesMutex_);
+        for (auto it = abilityStages_.begin(); it != abilityStages_.end(); ++it) {
+            auto abilityStage = it->second;
+            if (abilityStage == nullptr) {
+                continue;
+            }
+            if (!abilityStage->ContainsAbility()) {
+                toErase.push_back(it->first);
+                toDestroy.push_back(abilityStage);
+            } else {
+                containsNonEmpty = true;
+            }
+        }
+        for (size_t i = 0; i < toErase.size(); ++i) {
+            toDestroy[i]->SetStageState(AbilityRuntime::StageState::DESTROYING);
+            abilityStages_.erase(toErase[i]);
+        }
+    }
+    for (const auto &stage : toDestroy) {
+        if (!stage->IsSkipAbilityStageLifecycle()) {
+            stage->OnDestroy();
         }
     }
```

**逐行分析**：
- `toErase`（键列表）+ `toDestroy`（shared_ptr 列表）：锁内收集待删除项。shared_ptr 增引用计数，保活。
- 锁内先 `SetStageState(DESTROYING)` 再 `erase`：使并发 find 能感知到销毁中态（虽然 erase 后 find 找不到，但快照迭代中持有的 shared_ptr 仍可通过 GetStageState 检测到 DESTROYING）。
- 锁外 `OnDestroy`：避免持锁回调死锁。
- 原代码在遍历中 erase（`it = erase(it)`）且调用 OnDestroy——既持锁又遍历中修改，且 OnDestroy 可能回调获取同一把锁，**死锁 + iterator 失效双重风险**。

---

### 4.20 `UpdateETSRuntime()` — 锁范围扩大

```cpp
+    std::lock_guard<std::mutex> lock(runtimeMutex_);
     if (runtime_ == nullptr || runtime_->GetLanguage() != AbilityRuntime::Runtime::Language::JS) {
         ...
     }
     ...
-    {
-        std::lock_guard<std::mutex> lock(runtimeMutex_);
-        etsRuntime->SetJsRuntime(runtime_);
-        runtime_ = std::move(etsRuntime);
-    }
+    etsRuntime->SetJsRuntime(runtime_);
+    runtime_ = std::move(etsRuntime);
     return true;
```

**意义**：
- 原代码只在 `SetJsRuntime + move` 处加锁，前段的 `runtime_ == nullptr` 检查 + `GetLanguage()` 判断无锁——TOCTOU。
- 现整函数加锁：检查、CreateRuntime、SetJsRuntime、move 全在锁内。
- `SetJsRuntime(runtime_)`：传入旧的 runtime_（仍有效），然后 move 替换。全程锁保护。
- **代价**：`CreateRuntime` 可能较耗时（创建 ETS 运行时），现持锁执行，会阻塞所有 runtime_ 读取方。这是安全性 vs 性能的权衡。

---

### 4.21 `InitJSLeakWatcher()` — runtime_ 加锁

```cpp
+    std::lock_guard<std::mutex> lock(runtimeMutex_);
     if (runtime_ == nullptr) {
```

**意义**：原直接读 runtime_ 无锁，现加锁。

---

## 5. `ability_lifecycle_executor.h / .cpp` — 状态原子化

### 5.1 头文件

```cpp
+#include <atomic>
 ...
-    AbilityLifecycleExecutor::LifecycleState state_ = UNINITIALIZED;
+    std::atomic<AbilityLifecycleExecutor::LifecycleState> state_ = LifecycleState::UNINITIALIZED;
```

**意义**：`state_` 从普通枚举改为 `std::atomic<枚举>`。`GetState()` 可被多线程并发调用（如生命周期查询），`DispatchLifecycleState` 可被调度线程写入。原子化消除数据竞争。

### 5.2 实现文件

```cpp
-    state_ = state;
+    state_.store(state, std::memory_order_release);
 ...
-    return state_;
+    return static_cast<int>(state_.load(std::memory_order_acquire));
```

**意义**：
- `store(release)` / `load(acquire)`：建立 happens-before，保证读取方看到新状态时也看到状态变更前的所有写入。
- `static_cast<int>`：atomic load 返回 `LifecycleState` 枚举，需转为 int（返回类型为 int）。

---

## 6. `extension_ability_thread.h / .cpp` — ContentEmbed EventRunner 竞争修复

### 6.1 头文件

```cpp
+#include <mutex>
 ...
     static std::shared_ptr<AppExecFwk::EventRunner> contentEmbedEventRunner_;
+    static std::mutex contentEmbedMutex_;
```

**意义**：为静态成员 `contentEmbedEventRunner_` 配套静态 mutex。静态成员跨线程共享，必须同步。

### 6.2 实现文件

```cpp
+std::mutex ExtensionAbilityThread::contentEmbedMutex_;
```

**意义**：静态成员定义（声明在头文件，定义在 cpp）。

```cpp
-    if (contentEmbedEventRunner_ != nullptr) {
-        runner_ = contentEmbedEventRunner_;
-        abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
-        TAG_LOGD(AAFwkTag::EXT, "Reusing existing Content Embed EventRunner");
-    } else {
-        runner_ = AppExecFwk::EventRunner::Create(abilityName + "_CONTENT_EMBED");
-        if (runner_ == nullptr) {
-            TAG_LOGE(AAFwkTag::EXT, "Failed to create Content Embed EventRunner");
-            return;
-        }
-        abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
-        if (abilityHandler_ == nullptr) {
-            TAG_LOGE(AAFwkTag::EXT, "Failed to create Content Embed AbilityHandler");
-            return;
+    {
+        std::lock_guard<std::mutex> lock(contentEmbedMutex_);
+        if (contentEmbedEventRunner_ == nullptr) {
+            contentEmbedEventRunner_ = AppExecFwk::EventRunner::Create(abilityName + "_CONTENT_EMBED");
         }
-        contentEmbedEventRunner_ = runner_;
-        TAG_LOGI(AAFwkTag::EXT, "Created new Content Embed EventRunner");
     }
+    if (contentEmbedEventRunner_ == nullptr) {
+        TAG_LOGE(AAFwkTag::EXT, "Failed to create Content Embed EventRunner");
+        return;
+    }
+    runner_ = contentEmbedEventRunner_;
+    abilityHandler_ = std::make_shared<AppExecFwk::AbilityHandler>(runner_);
+    if (abilityHandler_ == nullptr) {
+        TAG_LOGE(AAFwkTag::EXT, "Failed to create Content Embed AbilityHandler");
+        return;
+    }
+    TAG_LOGI(AAFwkTag::EXT, "Content Embed EventRunner ready");
```

**逐行分析**：
- 原代码 **check-then-create-then-assign**：先判 `contentEmbedEventRunner_ != nullptr`，若为空则 Create + 赋值。两线程同时进入判空分支 → 都创建 runner → 都赋值 → 后赋值者覆盖前者 → **前者创建的 runner 泄漏 + 其上的 AbilityHandler 绑定到已泄漏的 runner**。
- 修复：锁内 check + create + assign 原子化。锁外读取 `contentEmbedEventRunner_`（此时已保证非空）赋给 `runner_`。
- 锁只保护创建+赋值，不保护后续的 `runner_ =` 和 `abilityHandler_ =`（这些是实例成员，每个 ExtensionAbilityThread 独有，无竞争）。

---

## 7. `task_handler_client.h / .cpp` — 单例与 taskHandler 竞争修复

### 7.1 头文件

```cpp
-    static std::mutex mutex_;
-    static std::shared_ptr<TaskHandlerClient> instance_;
     std::shared_ptr<TaskHandler> taskHandler_;
+    std::mutex taskHandlerMutex_;
```

**意义**：
- 删除静态 `mutex_` 和 `instance_`：单例改用 Meyers singleton（静态局部变量），无需手动 DCLP。
- 新增实例级 `taskHandlerMutex_`：保护实例成员 `taskHandler_` 的惰性创建。

### 7.2 实现文件 — GetInstance

```cpp
-    if (instance_ == nullptr) {
-        std::lock_guard<std::mutex> lock_l(mutex_);
-        if (instance_ == nullptr) {
-            instance_ = std::make_shared<TaskHandlerClient>();
-        }
-    }
-    return instance_;
+    static auto instance = std::make_shared<TaskHandlerClient>();
+    return instance;
```

**意义**：
- DCLP（Double-Checked Locking Pattern）替换为 **Meyers singleton**（C++11 起静态局部变量初始化是线程安全的）。
- 更简洁、更安全（DCLP 在某些内存模型下有缺陷，虽然 C++11 + atomic 可修正，但 Meyers 更优雅）。
- `make_shared` 一次分配（对象 + 控制块），优于 `new` + shared_ptr 构造。

### 7.3 实现文件 — PostTask

```cpp
-    if (taskHandler_ == nullptr) {
-        if (!CreateRunner()) {
-            TAG_LOGE(AAFwkTag::ABILITY, "CreateRunner failed");
-            return false;
-        }
+    std::shared_ptr<TaskHandler> handler;
+    {
+        std::lock_guard<std::mutex> lock(taskHandlerMutex_);
+        if (taskHandler_ == nullptr) {
+            std::shared_ptr<EventRunner> runner = EventRunner::Create("TaskRunner");
+            if (runner == nullptr) {
+                TAG_LOGE(AAFwkTag::ABILITY, "null runner");
+                return false;
+            }
+            taskHandler_ = std::make_shared<TaskHandler>(runner);
+            if (taskHandler_ == nullptr) {
+                TAG_LOGE(AAFwkTag::ABILITY, "null taskHandler_");
+                return false;
+            }
+        }
+        handler = taskHandler_;
     }
 
-    bool ret = taskHandler_->PostTask(task, delayTime, EventQueue::Priority::LOW);
+    bool ret = handler->PostTask(task, delayTime, EventQueue::Priority::LOW);
```

**逐行分析**：
- 原代码：`if (taskHandler_ == nullptr)` 无锁读 → CreateRunner（内部有锁但 check 与 create 分离）→ `taskHandler_->PostTask` 无锁读。**两次 TOCTOU**。
- 修复：锁内 check + create + 拷贝 `handler = taskHandler_`，锁外 `handler->PostTask`。
- 锁外 PostTask：避免持锁调用 PostTask（PostTask 可能阻塞或回调获取同一把锁）。
- 不再调用 `CreateRunner()`：直接内联创建逻辑（因为需要在同一把锁内完成 check + create）。

### 7.4 实现文件 — CreateRunner

```cpp
+    std::lock_guard<std::mutex> lock(taskHandlerMutex_);
+    if (taskHandler_ != nullptr) {
+        return true;
+    }
+    std::shared_ptr<EventRunner> runner = EventRunner::Create("TaskRunner");
+    if (runner == nullptr) {
+        TAG_LOGE(AAFwkTag::ABILITY, "null runner");
+        return false;
+    }
+    taskHandler_ = std::make_shared<TaskHandler>(runner);
     if (taskHandler_ == nullptr) {
-        std::shared_ptr<EventRunner> runner = EventRunner::Create("TaskRunner");
-        if (runner == nullptr) {
-            TAG_LOGE(AAFwkTag::ABILITY, "null runner");
-            return false;
-        }
-        taskHandler_ = std::make_shared<TaskHandler>(runner);
-        if (taskHandler_ == nullptr) {
-            TAG_LOGE(AAFwkTag::ABILITY, "null taskHandler_");
-            return false;
-        }
+        TAG_LOGE(AAFwkTag::ABILITY, "null taskHandler_");
+        return false;
     }
     return true;
```

**意义**：
- 加锁 + 幂等检查（`if (taskHandler_ != nullptr) return true`）：防止并发重复创建。
- 逻辑与原 `CreateRunner` 一致，只是加了锁并前置幂等检查。
- 此方法现主要由需要显式预创建的场景调用（PostTask 已内联创建逻辑）。

---

## 8. `js_worker.cpp` — 全局标志原子化

```cpp
+#include <atomic>
 ...
-bool g_jsFramework = false;
-std::mutex g_mutex;
+std::atomic<bool> g_jsFramework{false};
```

**意义**：
- `g_jsFramework` 从 `bool` + `std::mutex` 改为 `std::atomic<bool>`。原子变量比 mutex 更轻量（无系统调用开销），适合单一标志的发布。
- 删除 `g_mutex`：不再需要。

```cpp
-    if (g_jsFramework && !arkNativeEngine->ExecuteJsBin(...)) {
+    if (g_jsFramework.load(std::memory_order_acquire) &&
+        !arkNativeEngine->ExecuteJsBin("/system/etc/strip.native.min.abc")) {
```

**意义**：`acquire` 读，保证看到 `true` 时也看到 SetJsFramework 之前的初始化。

```cpp
-    g_jsFramework = true;
+    g_jsFramework.store(true, std::memory_order_release);
```

**意义**：`release` 写，发布 jsFramework 已就绪的状态，与 acquire 读配对。

---

## 9. `dynamic_loader.h / .cpp` — dlopen 错误缓冲与初始化表竞争修复

### 9.1 头文件

```cpp
+#include <string>
```

**意义**：为 `std::string` 提供依赖（`g_sharedLibsSonames` 等）。可能是隐式依赖的显式化。

### 9.2 实现文件

```cpp
+#include <mutex>
 ...
-static char g_dlError[ERROR_BUF_SIZE];
+static thread_local char g_dlError[ERROR_BUF_SIZE];
```

**意义**：
- `g_dlError` 从全局静态 char 数组改为 **thread_local**。
- **原问题**：多个线程并发调用 `DynamicLoadLibrary`，dlerror 错误信息写入同一块 buffer，线程 A 的错误被线程 B 覆盖——错误归因混乱。
- `thread_local`：每个线程独立一份 buffer，互不干扰。无需锁保护（每个线程访问自己的副本）。

```cpp
+static std::mutex g_dynLoaderMutex;
```

**意义**：保护下方 `g_hasInited`（unordered_set）的并发访问。

```cpp
+    std::lock_guard<std::mutex> lock(g_dynLoaderMutex);
     if (g_hasInited.count(std::string(name))) {
         return;
     }
```

**意义**：
- `DynamicInitNamespace` 中对 `g_hasInited`（unordered_set）的 count + insert 操作加锁。
- **原问题**：unordered_set 的并发读写是 UB（可导致迭代器失效、节点损坏、崩溃）。
- check-then-insert（count 返回 0 后 insert）也需原子化，否则两线程同时判为"未初始化" → 重复初始化。

---

## 10. `assert_fault_proxy.cpp` — 简化 DCLP

```cpp
+    std::lock_guard<std::mutex> lock(dialogConnectionMutex_);
     if (dialogConnectionCallback_ == nullptr) {
-        std::lock_guard lock(dialogConnectionMutex_);
-        if (dialogConnectionCallback_ == nullptr) {
-            dialogConnectionCallback_ = new (std::nothrow) AssertDialogConnection();
-        }
+        dialogConnectionCallback_ = new (std::nothrow) AssertDialogConnection();
     }
-
     return dialogConnectionCallback_;
```

**意义**：
- 原代码是 DCLP（外层无锁检查 + 内层加锁检查）。DCLP 的正确性依赖外层读是 atomic 的，C++ 中裸读非 atomic 成员在并发下是 UB。
- 修复：**始终加锁**（无条件获取锁），正确性优先。AssertFault 弹窗连接是低频路径，性能影响可忽略。
- 简化后：锁内 check + create，一次加锁。

---

## 11. `lifecycle_deal.cpp` — 改用线程安全访问器

```cpp
-    CHECK_POINTER(abilityScheduler_);
-    abilityScheduler_->ContinueAbility(deviceId, versionCode);
+    auto abilityScheduler = GetScheduler();
+    CHECK_POINTER(abilityScheduler);
+    abilityScheduler->ContinueAbility(deviceId, versionCode);
```

**意义**：
- 原代码直接读 `abilityScheduler_` 成员（裸指针）。该成员可被并发线程通过 `SetScheduler` 修改。
- 改用 `GetScheduler()` 访问器（内部应有锁或原子读取），返回局部 `shared_ptr`/`sptr` 拷贝，保证使用期间不被置空。
- `CHECK_POINTER` 检查局部变量，避免 TOCTOU。

---

## 12. 测试文件变更分析

### 12.1 `ohos_application_test.cpp` — 访问器适配 + 新增用例

**新增测试辅助方法**（`EmplaceAbilityStage` / `IsAbilityStagesEmpty` / `GetAbilityStage` / `SetAbilityStage` / `SetRuntime` / `GetRuntimeRaw`）：

```cpp
+    static void EmplaceAbilityStage(OHOSApplication *app, const std::string &moduleName,
+        const std::shared_ptr<AbilityRuntime::AbilityStage> &stage)
+    {
+        std::lock_guard<std::mutex> lock(app->abilityStagesMutex_);
+        app->abilityStages_.emplace(moduleName, stage);
+        if (stage != nullptr) {
+            stage->SetStageState(AbilityRuntime::StageState::READY);
+        }
+    }
```

**意义**：测试用 `#define private public` 突破访问控制，但现在直接操作 `abilityStages_` 会破坏并发不变量（无锁写 map 与产品代码加锁读竞争）。因此提供加锁的访问器，使测试也能遵守并发约束。每个访问器内部加锁。`EmplaceAbilityStage` 额外置 READY，模拟产品代码中 stage 入表后的状态。

**测试用例适配**：所有原先直接赋值 `ohosApplication_->abilityStages_ = ...` / `.emplace(...)` / `.empty()` / `.find(...)` 的断言改为调用访问器。这是**机械式适配**，保证测试在加锁后仍能正确操作内部状态。

**新增用例 OnMemoryLevel_0300 / 0400**：

```cpp
+HWTEST_F(..., AppExecFwk_OHOSApplicationTest_OnMemoryLevel_0300, ...)
+{
+    ...
+    mockAbilityStage->SetStageState(AbilityRuntime::StageState::INITIALIZING);
+    EXPECT_CALL(*mockAbilityStage, OnMemoryLevel(level)).Times(0);
+    ohosApplication_->OnMemoryLevel(level);
```

**意义**：验证 INITIALIZING 态的 stage 不收到 OnMemoryLevel 回调（状态守卫生效）。0400 验证 DESTROYING 态同理。

**新增用例 CleanEmptyAbilityStage_0100**：验证空 stage 被 CleanEmptyAbilityStage 删除并触发 OnDestroy。

### 12.2 `ohos_application_first_test.cpp` — 同类适配

提供 `EmplaceAbilityStage` / `IsAbilityStagesEmpty` / `SetAbilityStages` 访问器，将直接成员访问替换为加锁调用。新增逻辑与 ohos_application_test.cpp 一致。

### 12.3 `dynamic_loader_test.cpp` — 移除错误断言

```cpp
-    char* errorBuf = const_cast<char*>(DynamicGetError());
-    if (errorBuf != nullptr) {
-        errorBuf[0] = '\0';
-    }
```

**意义**：SetUp 中清空全局 error buffer 的操作移除。因为 buffer 现在是 thread_local，每个线程独立，测试线程内无需（也不应）跨线程清空。

```cpp
-    EXPECT_STREQ(DynamicGetError(), "");
+    const char* error = DynamicGetError();
+    EXPECT_NE(error, nullptr);
```

**意义**：原先断言 DynamicGetError 返回空字符串（成功时无错误）。thread_local 改造后，成功时不保证 buffer 内容为空（可能是上一次的错误残留），改为只断言返回非空指针。**断言被削弱**——不再验证"无错误"语义。

DynamicInitNamespace 测试新增实际加载验证：

```cpp
-    EXPECT_STREQ(DynamicGetError(), "");
+    void* handle = DynamicLoadLibrary(&ns, TEST_LIB, RTLD_LAZY);
+    EXPECT_NE(handle, nullptr);
```

**意义**：用实际的 DynamicLoadLibrary 成功来验证 namespace 初始化正确，替代原先检查 error buffer 为空。这是更鲁棒的验证方式。

---

## 13. 风险评估

| # | 风险 | 严重度 | 说明 |
|---|------|--------|------|
| R1 | `runtimeMutex_` 锁内调用 `Runtime::*` 方法，依赖"不回调 SetRuntime/UpdateETSRuntime"的隐含契约 | 🔴 高 | 若未来 Runtime 实现间接触发 runtime 替换，自死锁。无编译期/运行期强制保障。 |
| R2 | `reuseExisting` 路径不等待 READY | 🟡 中 | 并发场景拿到 INITIALIZING 态 stage 直接使用，OnCreate 尚未执行，语义可能不符合调用方预期。 |
| R3 | 初始化期间丢弃 AcceptWant / PrepareTerminate / NewProcessRequest | 🟡 中 | 非 READY 态直接返回空 flag，调用方（AMS）需有重试/排队机制，否则多实例/新进程请求可能丢失。 |
| R4 | `UpdateETSRuntime` 锁范围扩大 | 🟢 低 | CreateRuntime 等耗时操作持锁执行，阻塞所有 runtime_ 读取方。ETSRuntime 切换是低频路径，影响可控。 |
| R5 | 状态守卫的 best-effort check | 🟢 低 | `GetStageState() != READY` 检查与实际调用之间仍有窗口，但 shared_ptr 保活防止 UAF，风险降为"向非 READY 对象派发回调"而非崩溃。 |
| R6 | dynamic_loader 测试断言削弱 | 🟢 低 | 移除了 `EXPECT_STREQ(DynamicGetError(), "")` 断言，未补充 thread_local 隔离性的正向测试。 |

---

## 14. 测试覆盖评估

### 已覆盖

| 路径 | 测试 |
|------|------|
| OnMemoryLevel 跳过 INITIALIZING | `OnMemoryLevel_0300` |
| OnMemoryLevel 跳过 DESTROYING | `OnMemoryLevel_0400` |
| CleanEmptyAbilityStage 删除+OnDestroy | `CleanEmptyAbilityStage_0100` |
| CleanAbilityStage 跳过 OnDestroy (skip=true) | `CleanAbilityStage_0500`（已有，适配访问器） |
| runtime_ 空指针保护 | OnForeground/OnBackground/QuickFix 相关用例（适配） |
| AddAbilityStage 正常流程 | 多个 AddAbilityStage 用例（适配） |

### 未覆盖（建议补充）

| 路径 | 风险 | 建议测试 |
|------|------|----------|
| `reuseExisting` 并发插入路径 | 中 | 多线程并发 AddAbilityStage 同一 moduleName，验证只创建一个 stage |
| `ScheduleAcceptWant` 非 READY 跳过 | 中 | 显式构造 INITIALIZING 态 stage，调用 ScheduleAcceptWant，验证回调收到空 flag |
| `ScheduleNewProcessRequest` 非 READY 跳过 | 中 | 同上 |
| `runtimeMutex_` 并发场景 | 低 | 并发 SetRuntime + OnForeground，验证无崩溃/死锁 |
| `CleanAbilityStage` 二次检查 ContainsAbility | 低 | 模拟并发 AddAbility 使 stage 变非空，验证不被误删 |
| `dynamic_loader` thread_local 隔离性 | 低 | 多线程并发 DynamicLoadLibrary，验证各自 error buffer 独立 |

---

## 附录：变更文件清单

| 文件 | 行数变化 | 性质 |
|------|----------|------|
| `ets_environment/frameworks/ets_environment/src/dynamic_loader.cpp` | +5/-1 | 🟡 内部实现 |
| `ets_environment/interfaces/inner_api/dynamic_loader.h` | +1 | 🟡 内部 API |
| `frameworks/native/ability/native/ability_lifecycle_executor.cpp` | +2/-2 | 🟢 客户端实现 |
| `frameworks/native/ability/native/extension_ability_thread.cpp` | +13/-19 | 🟢 客户端实现 |
| `frameworks/native/ability/native/task_handler_client.cpp` | +20/-35 | 🟢 客户端实现 |
| `frameworks/native/appkit/app/ohos_application.cpp` | +272/-123 | 🟢 客户端实现（核心） |
| `frameworks/native/runtime/js_worker.cpp` | +4/-5 | 🟢 客户端实现 |
| `interfaces/inner_api/ability_manager/include/task_handler_client.h` | +1/-3 | 🟡 内部 API |
| `interfaces/kits/native/ability/native/ability_lifecycle_executor.h` | +3/-1 | 🔴 公共 API |
| `interfaces/kits/native/ability/native/extension_ability_thread.h` | +3 | 🔴 公共 API |
| `interfaces/kits/native/appkit/ability_runtime/app/ability_stage.h` | +18 | 🔴 公共 API |
| `interfaces/kits/native/appkit/app/ohos_application.h` | +12 | 🔴 公共 API |
| `services/abilitymgr/src/assert_fault_proxy.cpp` | +2/-5 | 🔴 服务端实现 |
| `services/abilitymgr/src/lifecycle_deal.cpp` | +2/-2 | 🔴 服务端实现 |
| `test/unittest/appkit/ohos_application_first_test/ohos_application_first_test.cpp` | +35/-15 | 测试 |
| `test/unittest/appkit/ohos_application_test/ohos_application_test.cpp` | +204/-99 | 测试 |
| `test/unittest/dynamic_loader_test/dynamic_loader_test.cpp` | +12/-15 | 测试 |

---

*文档生成时间：2026-08-28*
*分析基于 commit `aa3692318bfecf558e973451c8e343f805a93fa3` 完整 diff*
