# 任务规格：UIExtension sessionInfo 支持更新

> ## ⚠ 代码检视结论（2026-07-09 深度检视后修订）
>
> 对照当前 ability_runtime 代码树逐文件验证，本 spec 原描述的大部分工作**已在代码中落地**（按 Revision 2 路线）。重新定位如下：
>
> | 任务 | 代码现状 | 实际位置 |
> |------|----------|----------|
> | T0a（ability_base SessionInfo） | ✅ 已实现 | `session_info.h:44-48,120` / `session_info.cpp:400-424,525-533` |
> | T1（ExtensionRecordManager 迁移分支 + 两个 helper + stableId 回填） | ✅ 已实现 | `extension_record_manager.cpp:218-239,319-324,326-351`；`.h:202,204` |
> | T2（uiExtensionMap_ 按 persistentId 清理旧映射） | ❌ **未实现（唯一真待做）** | 插入点 `ui_extension_ability_manager.cpp:671→672` |
> | T3（UIExtension 基类 OnCommandWindow 迁移检测 + HandleSessionUpdate 虚方法 + GetEffectiveComponentId） | ✅ 已实现 | `ui_extension.cpp:223-229`；`ui_extension.h:138,35-41` |
> | T4（JsUIExtension::HandleSessionUpdate 重写，Rev2：GetEffectiveComponentId，无 key 滚动） | ✅ 已实现 | `js_ui_extension.cpp:687-720`；`js_ui_extension.h:188` |
> | T5（DoUpdateExtensionParent + jsExtensionWindowRefs_ + env_ + napi_ref 跟踪） | ✅ 已实现，**但 DoUpdateExtensionParent 仍用 napi 方式（correction-2 重构未做）** | `js_ui_extension_content_session.cpp:1290-1321`；`.h:92,137,138` |
> | Correction-2（DoUpdateExtensionParent：napi → C++ 直调 UpdateExtensionParent(WindowOption) + 加 uiWindow_->UpdateExtensionParent） | ✅ **已实现** | 改写 `js_ui_extension_content_session.cpp:1290+` |
>
> **结论**：本 spec 已从"待实现计划"重定位为"已实现基线 + 增量"。T1/T3/T4/T5 已实现，Correction-2 已实现（2026-07-09，参照 window_manager PR #19373 API 设计，窗口侧代码开发中，当前供穿刺验证参照），T2 因 sessionToken 迁移时不变而已废弃。**活跃任务仅 T6 单元测试（等闭环终稿后补充）**。
> 已知 spec 正文遗留：ADR-7/T4 4.4 原写 Revision 1 key 滚动，已修订为 Revision 2（见各节"⚠ Rev2 修订"）。

## 跨仓前置依赖（不在本次范围，记录供对齐）

### T0a: ability_base — SessionInfo 新增 originalCallerInfo ✅ 已实现

**仓**：ability_base（本仓内）
**文件**：`interfaces/kits/native/session_info/include/session_info.h` + `src/session_info.cpp`
**实现内容**：
1. 新增 `OriginalCallerInfo` 结构体（`callerToken` + `persistentId`），定义在 `SessionInfo` 类之前
2. `SessionInfo` 类新增成员 `OriginalCallerInfo originalCallerInfo {}`
3. 新增 `DoMarshallingEight`：序列化 callerToken（nullable remote object）+ persistentId
4. 新增 `ReadParcelThree`：反序列化 callerToken + persistentId
5. `Marshalling()` 末尾追加 `DoMarshallingEight` 调用；`Unmarshalling()` 末尾追加 `ReadParcelThree` 调用
**状态**：已完成，T1/T3 可直接消费

### T0b: window_manager — Rosen::Window / JsExtensionWindow C++ UpdateExtensionParent 接口

**仓**：window_manager
**内容**：`Rosen::Window` 与 `Rosen::JsExtensionWindow` 新增 C++ 接口 `UpdateExtensionParent(sptr<WindowOption>)`，用于更新已创建窗口/proxy 的 parent 绑定。**无需 napi 暴露**，ability_runtime 直接 C++ 调用。参数 `WindowOption` 由 window_manager 定义（含 newHostWindowId 等）。
**对齐要求**：T5 的 `DoUpdateExtensionParent` 依赖此 C++ 接口；PR #19373 已具备

---

## T1: ExtensionRecordManager 迁移匹配 ✅ 已实现

> **代码检视**：本任务全部步骤已在代码中落地。下方步骤保留为"对照说明"，**勿重复注入**（会引发重复定义编译错误）。实际位置：迁移分支 `extension_record_manager.cpp:218-239`、`IsMigrationUpdateRequest` :319-324、`GetAbilityRecordByPersistentId` :326-351、声明 `.h:202,204`；并已含 Revision 2 的 stableId 回填（:223-231）。注：原 spec 引用的旧行号（~217/237/247/264-293）因迁移分支先在而整体 +24 偏移，实际为 241/261/271/288-317。

**依赖**：T0a
**文件**：
- `services/abilitymgr/src/extension_record/extension_record_manager.cpp`
- `services/abilitymgr/include/extension_record/extension_record_manager.h`

### 步骤

#### 1.1 新增 `IsMigrationUpdateRequest` 私有方法声明（.h）

在 `ExtensionRecordManager` 类的 private 区域新增：

```cpp
bool IsMigrationUpdateRequest(const sptr<AAFwk::SessionInfo> &sessionInfo) const;
```

#### 1.2 新增 `GetAbilityRecordByPersistentId` 私有方法声明（.h）

```cpp
std::shared_ptr<AAFwk::BaseExtensionRecord>
GetAbilityRecordByPersistentId(int32_t persistentId);
```

#### 1.3 实现 `IsMigrationUpdateRequest`（.cpp）

```cpp
bool ExtensionRecordManager::IsMigrationUpdateRequest(
    const sptr<AAFwk::SessionInfo> &sessionInfo) const
{
    return sessionInfo != nullptr &&
           sessionInfo->originalCallerInfo.callerToken != nullptr &&
           sessionInfo->originalCallerInfo.persistentId != 0;
}
```

#### 1.4 实现 `GetAbilityRecordByPersistentId`（.cpp）

参考现有 `GetAbilityRecordBySessionInfo`（行 264-293）的遍历结构，改为按 persistentId 匹配：

```cpp
std::shared_ptr<AAFwk::BaseExtensionRecord>
ExtensionRecordManager::GetAbilityRecordByPersistentId(int32_t persistentId)
{
    if (persistentId == 0) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &it : extensionRecords_) {
        if (it.second == nullptr) continue;
        auto abilityRecord = it.second->abilityRecord_;
        if (abilityRecord == nullptr) continue;
        auto recordSessionInfo = abilityRecord->GetSessionInfo();
        if (recordSessionInfo == nullptr) continue;
        if (recordSessionInfo->persistentId == persistentId) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "found by persistentId: %{public}d", persistentId);
            return abilityRecord;
        }
    }
    return nullptr;
}
```

#### 1.5 在 `GetOrCreateExtensionRecord` 新增迁移分支（.cpp，行 216 后）

在现有 `CHECK_POINTER_AND_RETURN`（行 216）之后、`GetAbilityRecordBySessionInfo`（行 217）之前插入：

```cpp
// [新增] 迁移更新分支
if (IsMigrationUpdateRequest(abilityRequest.sessionInfo)) {
    abilityRecord = GetAbilityRecordByPersistentId(abilityRequest.sessionInfo->persistentId);
    if (abilityRecord != nullptr) {
        isLoaded = true;
        TAG_LOGI(AAFwkTag::ABILITYMGR, "migration update: found record by persistentId: %{public}d",
            abilityRequest.sessionInfo->persistentId);
        return ERR_OK;
    }
    TAG_LOGW(AAFwkTag::ABILITYMGR, "migration update: record not found for persistentId: %{public}d, fallback",
        abilityRequest.sessionInfo->persistentId);
    // 容错降级：继续走现有逻辑
}
```

### 验证
- 单元测试：构造 sessionInfo 带 originalCallerInfo，验证迁移分支命中 + 返回 isLoaded=true
- 回归测试：现有 extension_record_manager_test 全部通过

---

## T2: UIExtensionAbilityManager map 清理 ❌ 已废弃（sessionToken 迁移时不变）

> **⚠ 2026-07-09 修订（基于需求方澄清）**：**迁移后 sessionToken 不发生变化**。
> `uiExtensionMap_` 以 sessionToken(remoteObj) 为 key（`ui_extension_ability_manager.cpp:669`）。
> 由于 sessionToken 迁移前后是同一对象，现有行 672 `uiExtensionMap_[remoteObj] = {targetService, abilityRequest.sessionInfo}`
> 已是对**同一 key 的 value 原地更新**（新 sessionInfo，含新 callerToken）——天然正确，无需清理旧映射。
> 又因同一 sessionToken 的 DeathRecipient 已注册，行 674 `AddUIExtWindowDeathRecipient` 对重复注册幂等返回（`:811-814` "recipient added before"）。
> `GetUIExtensionBySessionInfo`（`:367-396`）按 sessionToken 查找（`:375`）并校验 callerToken 一致性（`:386-387`）：
> 迁移后 map value 已更新为新 callerToken，故用新 sessionInfo 查询能正确命中原 record。
>
> **结论**：原 T2（按 persistentId 清理旧 sessionToken 映射）的前提「sessionToken 变化导致旧映射泄漏」**不成立**，T2 **废弃，不再实现**。原 ADR-3 同步作废（见 design.md）。
>
> 历史代码检视记录（已作废，仅留档）：原插入点 `ui_extension_ability_manager.cpp:671→672`；`RemoveUIExtWindowDeathRecipient` 已存在（`.h:176`）。spec 引用行号 613/646/666/668-676/701/705 经检视全部精确命中。

---

## T3: UIExtension 基类迁移检测 ✅ 已实现 → ⚠ 方案 B 修订（2026-07-09，移除 OnCommandWindow 短路）

> **⚠ 2026-07-09 方案 B 修订**：移除 `OnCommandWindow` 的迁移短路（原 `HandleSessionUpdate` + `OnCommandWindowDone` + return）。moveTo 现流经正常 `WIN_CMD_FOREGROUND` → `ForegroundWindow` → `HandleSessionCreate` else 分支（天然不 onSessionCreate）。`HandleSessionUpdate` 虚方法从基类及全部变体移除。详见 design.md ADR-10/ADR-11。
> **原代码检视（已废弃）**：~~`HandleSessionUpdate` 虚方法 `ui_extension.h:138`；`OnCommandWindow` 迁移检测 `ui_extension.cpp:223-229`~~。这些已移除。`GetEffectiveComponentId` helper `ui_extension.h:35-41` 保留不变。
> **⚠ 补充（CJ/base/ETS 变体）**：原 `js_ui_extension_base.cpp` / `cj_ui_extension_base.cpp` / `ets_ui_extension_base.cpp` 的 OnCommandWindow 短路也已移除（方案 B）。base 变体的 `HandleSessionUpdate` 原为空桩（no-op），移除后流经 ForegroundWindow（UpdateExtensionConfig + Show）反优于原 no-op。

**依赖**：T0a
**文件**：
- `frameworks/native/ability/native/ui_extension_ability/ui_extension.cpp` + `ui_extension.h`（移除短路 + HandleSessionUpdate 虚方法）
- 3 个 base 变体 cpp/h（移除短路 + HandleSessionUpdate 声明）

### 步骤

#### 3.1 ~~新增 `HandleSessionUpdate` 虚方法声明（.h）~~ — **方案 B 移除**

#### 3.2 ~~在 `OnCommandWindow` 新增迁移检测（.cpp）~~ — **方案 B 移除**（moveTo 流经正常 ForegroundWindow 路径，迁移更新在 T4 HandleSessionCreate else 分支内联）

### 验证
- 单元测试：sessionInfo 带 originalCallerInfo 时，OnCommandWindow 调用 HandleSessionUpdate 而非 ForegroundWindow

---

## T4: JsUIExtension 迁移更新 ✅ 方案 B 修订（2026-07-09，HandleSessionCreate else 分支内联）

> **⚠ 2026-07-09 方案 B 修订**：移除独立 `HandleSessionUpdate` 重写方法。迁移更新内联进 `JsUIExtension::HandleSessionCreate` else 分支（`js_ui_extension.cpp:676+`）：检测 `originalCallerInfo.callerToken != nullptr` 时，经 `contentSessions_[compId]` → `napi_unwrap` 取 ContentSession 指针调 `DoUpdateExtensionParent`。moveTo 流经正常 `ForegroundWindow` → `HandleSessionCreate` else 分支（天然不 onSessionCreate）+ `Show()`，不再短路。`HandleSessionUpdate` override 声明 `.h:188` 已移除。
> **原代码检视（已废弃）**：~~`js_ui_extension.cpp:687-720` HandleSessionUpdate 重写~~ 已删除。

**依赖**：T3
**文件**：
- `frameworks/native/ability/native/ui_extension_ability/js_ui_extension.cpp`
- 对应头文件（`interfaces/kits/native/ability/native/ui_extension_ability/js_ui_extension.h`）

### 步骤

#### 4.1 ~~`HandleSessionUpdate` override 声明（.h）~~ — **方案 B 移除**

#### 4.2 `HandleSessionCreate` else 分支内联迁移更新（.cpp，方案 B，:676+）

```cpp
} else {  // window 已存在（前台到前台 / moveTo）
    auto uiWindow = uiWindowMap_[compId];
    if (uiWindow == nullptr) { ... return false; }
    uiWindow->UpdateExtensionConfig(sharedWant);
    // moveTo 迁移（originalCallerInfo 标识）：增量更新窗口 parent 绑定
    if (sessionInfo->originalCallerInfo.callerToken != nullptr) {
        auto sessionIt = contentSessions_.find(compId);
        if (sessionIt != contentSessions_.end() && sessionIt->second != nullptr) {
            HandleScope handleScope(jsRuntime_);
            napi_env env = jsRuntime_.GetNapiEnv();
            napi_value contentSession = sessionIt->second->GetNapiValue();
            void* nativeObject = nullptr;
            napi_unwrap(env, contentSession, &nativeObject);
            auto* contentSessionPtr = static_cast<JsUIExtensionContentSession*>(nativeObject);
            if (contentSessionPtr != nullptr) {
                contentSessionPtr->DoUpdateExtensionParent(sessionInfo);
            }
        }
    }
}
```

#### 4.3 ~~实现 `HandleSessionUpdate`（.cpp）~~ — **方案 B 移除**

### 验证
- 单元测试：moveTo 流经 ForegroundWindow，HandleSessionCreate else 分支命中（不 onSessionCreate），`DoUpdateExtensionParent` 被调用，`Show()` 被调用

---

## T5: JsUIExtensionContentSession UpdateExtensionParent + proxy 跟踪 ✅ 已实现（含 Correction-2）

> **代码检视**：成员与跟踪已落地——`jsExtensionWindowRefs_` `.h:137`、`env_` `.h:138`；`env_` 在 `CreateJsUIExtensionContentSession` 两处 napi_wrap 前存入（`.cpp:1052,1088`）；`OnGetUIExtensionHostWindowProxy`/`OnGetUIExtensionWindowProxy` 已创建 proxy 后 `napi_create_reference` 并 push（`.cpp:314-316,346-348`）。
> **⚠ 归属更正**：`OnGet*Proxy` 在 `JsUIExtensionContentSession`（本文件），**不在 `JsUIExtension`**（原 spec T4 误置）。
> **✅ Correction-2 已实现（2026-07-09）**：`DoUpdateExtensionParent`（`.cpp:1290+`）已从 napi 方式重构为 C++ 直调——构造 `sptr<Rosen::WindowOption>`（SetParentId/SetRealParentId/SetParentWindowType/SetDisplayId 取自 sessionInfo）→ 调 `uiWindow_->UpdateExtensionParent(option)` → 遍历 `jsExtensionWindowRefs_` 经 `napi_unwrap` 取 `Rosen::JsExtensionWindow*` 直调 C++ `UpdateExtensionParent(option)`。依赖 window_manager PR #19373 提供的 C++ 接口 `Window::UpdateExtensionParent(WindowOption)` / `JsExtensionWindow::UpdateExtensionParent(WindowOption)`（窗口侧代码开发中，当前供穿刺验证参照，未合入本树）。已补 `#include "window_option.h"`。

**依赖**：T4
**文件**：
- `frameworks/native/ability/native/ui_extension_base/js_ui_extension_content_session.cpp`
- 对应头文件（`interfaces/kits/native/ability/native/ui_extension_base/js_ui_extension_content_session.h`）

### 步骤

#### 5.1 `jsExtensionWindowRefs_` + `env_` 成员（.h）✅ 已实现（`.h:137,138`）

```cpp
std::vector<napi_ref> jsExtensionWindowRefs_;
napi_env env_ = nullptr;
```

#### 5.2 `OnGetUIExtensionHostWindowProxy` / `OnGetUIExtensionWindowProxy` 记录 proxy 引用（.cpp）✅ 已实现（`.cpp:314-316,346-348`）

```cpp
// [新增] 记录 proxy 引用，供迁移时更新
napi_value proxyValue = value->GetNapiValue();
if (proxyValue != nullptr) {
    napi_ref ref = nullptr;
    napi_create_reference(env, proxyValue, 1, &ref);
    jsExtensionWindowRefs_.push_back(ref);
}
return handleEscape.Escape(proxyValue);
```

#### 5.3 在 `CreateJsUIExtensionContentSession`（两处）存 `env_`（.cpp）✅ 已实现（`.cpp:1052,1088`）

```cpp
std::unique_ptr<JsUIExtensionContentSession> jsSession = ...;
jsSession->env_ = env;  // [新增] napi_wrap 前存入
napi_wrap(env, object, jsSession.release(), Finalizer, nullptr, nullptr);
```

#### 5.4 `DoUpdateExtensionParent` — ❌ Correction-2 待重构（声明已实现 `.h:92`，实现 `.cpp:1290-1321` 需改写）

> **现状**（`.cpp:1290-1321`）：仅 `sessionInfo_ = sessionInfo` + 遍历 `jsExtensionWindowRefs_` 调 napi `updateExtensionParent`（0 参，带 TODO，`method==nullptr` 时空转），**不调 `uiWindow_->UpdateExtensionParent`**。
> **目标**（按下文改写）：加 `uiWindow_->UpdateExtensionParent(WindowOption)` + napi_unwrap→C++ `jsExtWin->UpdateExtensionParent(WindowOption)`；补 `window_option.h` include。

```cpp
// .h
void DoUpdateExtensionParent(const sptr<AAFwk::SessionInfo> &sessionInfo);

// .cpp — 内部 C++ 方法，非 napi 接口
void JsUIExtensionContentSession::DoUpdateExtensionParent(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    if (sessionInfo == nullptr) return;
    // a. 更新 sessionInfo_（uiWindow_ 对象复用不变，ADR-8）
    sessionInfo_ = sessionInfo;
    // 构造 WindowOption（newHostWindowId = sessionInfo_->hostWindowId 等，入参定义以 window_manager 为准）
    sptr<Rosen::WindowOption> option = ...;
    // b. uiWindow_ 直接调 C++ 接口
    if (uiWindow_ != nullptr) {
        uiWindow_->UpdateExtensionParent(option);
    }
    // c. 遍历老 proxy，napi_unwrap 拿 JsExtensionWindow* 直接调 C++ 接口（非 napi 方法调用）
    if (env_ == nullptr) return;
    for (auto &ref : jsExtensionWindowRefs_) {
        napi_value proxy = nullptr;
        napi_get_reference_value(env_, ref, &proxy);
        if (proxy == nullptr) continue;
        Rosen::JsExtensionWindow* jsExtWin = nullptr;
        napi_unwrap(env_, proxy, reinterpret_cast<void**>(&jsExtWin));
        if (jsExtWin != nullptr) {
            jsExtWin->UpdateExtensionParent(option);
        }
    }
}
```

**设计要点**：
- `DoUpdateExtensionParent` 是内部 C++ 方法（非 napi 接口），由 `JsUIExtension::HandleSessionUpdate` 通过 `napi_unwrap` 拿指针直接调用
- **uiWindow_ 对象复用不变**（ADR-8）：不重建窗口对象，但需调 `uiWindow_->UpdateExtensionParent(WindowOption)` 更新 parent 绑定
- 老 proxy 保留并更新（应用缓存了）：经 `napi_unwrap` 取 JsExtensionWindow C++ 指针，直接调 `UpdateExtensionParent(WindowOption)`，**无需 napi 暴露**
- 新 proxy 不主动创建——应用下次调 `getUIExtension*Proxy` 时自然用新 `sessionInfo_->hostWindowId` 创建

**跨仓依赖**：
- window_manager T0b 提供 C++ 接口 `Window::UpdateExtensionParent(WindowOption)` 与 `JsExtensionWindow::UpdateExtensionParent(WindowOption)`（PR #19373 已具备），**无需 napi 暴露**
- ability_runtime 直接 C++ 调用；WindowOption 入参构造以 window_manager 定义为准

#### 5.5 析构清理 proxy 引用 — 已知 minor 泄漏（暂不实现）

`Finalizer` 中 `env` 可能已失效，无法安全调 `napi_delete_reference`。当前 `jsExtensionWindowRefs_` 的 napi_ref 不在析构时主动释放（vector 析构只销毁容器，不调 napi_delete_reference）。

proxy 对象本身由应用层 JS 引用持有，GC 会回收 proxy 对象，napi_ref 引用计数归零后自然释放。残余风险：napi_ref 本身的小内存泄漏，可接受。后续可考虑改用 weak ref 机制。

### 验证
- 单元测试：OnGet*Proxy 后 jsExtensionWindowRefs_ 非空
- 单元测试：DoUpdateExtensionParent 后 sessionInfo_ 更新（uiWindow_ 对象复用不变）
- 单元测试：DoUpdateExtensionParent 后调 uiWindow_ 与各老 proxy 的 C++ UpdateExtensionParent(WindowOption)

---

## T6: 单元测试

**依赖**：T1-T5
**文件**：
- `test/unittest/` 下对应测试目录

### 服务侧测试用例

| 用例 | 覆盖 AC | 前置条件 | 预期 |
|------|---------|----------|------|
| IsMigrationUpdateRequest_true | AC1 | originalCallerInfo 非默认值 | 返回 true |
| IsMigrationUpdateRequest_false | AC8 | originalCallerInfo 默认值 | 返回 false |
| GetAbilityRecordByPersistentId_found | AC2 | record 存在且 persistentId 匹配 | 返回 record |
| GetAbilityRecordByPersistentId_notFound | AC9 | 无匹配 record | 返回 nullptr |
| Migration_foundOriginalRecord | AC3,AC4 | 迁移分支命中 | isLoaded=true，不 LoadAbility |
| Migration_mapCleanup | AC7 | 迁移场景 | uiExtensionMap_ 旧映射清理、新映射建立 |
| NoMigration_walksOriginalPath | AC8 | 无 originalCallerInfo | 走现有 GetAbilityRecordBySessionInfo |

### 应用侧测试用例

| 用例 | 覆盖 AC | 前置条件 | 预期 |
|------|---------|----------|------|
| OnCommandWindow_migrationDetected | AC5 | originalCallerInfo 非空 + FOREGROUND | 调 HandleSessionUpdate，不调 ForegroundWindow |
| HandleSessionUpdate_findsByStableCompId | AC5a | `contentSessions_` 有 stable X 匹配项 | `GetEffectiveComponentId` 返回稳定 X，`contentSessions_[X]` 直接命中（无 key 滚动），`DoUpdateExtensionParent` 被调用 |
| HandleSessionUpdate_notFound | — | 索引无匹配 | 记警告日志，不 crash（已知问题：不 fallback，窗口丢失） |
| DoUpdateExtensionParent_updatesSessionInfo | AC5a | 调用 DoUpdateExtensionParent | sessionInfo_ 更新，uiWindow_ 对象复用不变 |
| DoUpdateExtensionParent_updatesProxies | AC5a | jsExtensionWindowRefs_ 非空 | 调 uiWindow_ + 各老 proxy 的 C++ UpdateExtensionParent(WindowOption) |

### 回归测试

- 现有 `extension_record_manager_test` 全部通过
- 现有 `ui_extension_ability_manager_test` 全部通过
- 现有 `ability_manager_service_*_test` UIExtension 相关用例全部通过
