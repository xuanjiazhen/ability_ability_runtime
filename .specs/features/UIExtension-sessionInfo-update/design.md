# 设计文档：UIExtension sessionInfo 支持更新

## 0. 修订历史

### Revision 2 — 稳定 componentId 方案（当前实现）

**背景**：Revision 1 的 key 滚动方案（`persistentIdToComponentId_` + `HandleSessionUpdate` 换 key）在所有变体（JS/ETS/CJ/PhotoEditor）中维护成本高、易出 bug。

**核心改动**：在 `OriginalCallerInfo` 中新增稳定的 `uiExtensionComponentId`（= 首创时 AMS 锁定的原始值 X，永不变化），下游统一用 effectiveKey 索引，完全移除 key 滚动逻辑。

**具体变化**：

1. **ability_base 结构体扩展**（ADR-2 修订）：
   - `OriginalCallerInfo` 新增 `uint64_t uiExtensionComponentId = 0`
   - 序列化/反序列化：`DoMarshallingEight` / `ReadParcelThree` 末尾追加（兼容旧端，默认 0）

2. **AMS 回填稳定 X**（`extension_record_manager.cpp` `GetOrCreateExtensionRecord` 迁移分支）：
   - 定位到 record 后，在 `SetSessionInfo` **之前**，将 record 的原始 `uiExtensionComponentId`（即 X）回填到 `abilityRequest.sessionInfo->originalCallerInfo.uiExtensionComponentId`
   - 首次迁移取 `recordSessionInfo->uiExtensionComponentId`（= X）；后续迁移透传已有值
   - 保证下发给 provider 的 sessionInfo 始终携带 X

3. **provider 统一 effectiveKey**（全部变体 JS/ETS/CJ/PhotoEditor）：
   ```cpp
   inline uint64_t GetEffectiveComponentId(const sptr<SessionInfo> &si) {
       if (si && si->originalCallerInfo.uiExtensionComponentId != 0) {
           return si->originalCallerInfo.uiExtensionComponentId;  // 迁移后：X
       }
       return si ? si->uiExtensionComponentId : 0;  // 首创：X（同一个值）
   }
   ```
   - **首创时** originalCallerInfo=0 → 取 `uiExtensionComponentId`(=X)
   - **迁移后** 取 `originalCallerInfo.uiExtensionComponentId`(=X)
   - 两者恒等，map key 永不变化

4. **移除内容**：
   - `persistentIdToComponentId_` 成员（`js_ui_extension.h`）— **完全移除**
   - `HandleSessionUpdate` key 滚动逻辑（`js_ui_extension.cpp` 708-724 行）— **完全移除**，简化为仅调 `DoUpdateExtensionParent`
   - `HandleSessionCreate` 中的 `persistentIdToComponentId_` 写入 — **移除**
   - `DestroyWindow` 中的 `persistentIdToComponentId_.erase` — **移除**

5. **全部变体支持迁移检测**：
   - JS 变体 `UIExtension::OnCommandWindow`（基类，`ui_extension.cpp:224`）— 已有
   - ETS 变体 `EtsUIExtensionBase::OnCommandWindow` — **新增**（`originalCallerInfo.callerToken != nullptr && winCmd == FOREGROUND` → 调 `HandleSessionUpdate`）
   - CJ 变体 `CJUIExtensionBase::OnCommandWindow` — **新增**（同上）
   - JS base 变体 `JsUIExtensionBase::OnCommandWindow`（PhotoEditor 等）— **新增**（同上）

**ADR-3 修订（uiExtensionMap_ 清理）**：保留 persistentId 查找不变。理由：`uiExtensionMap_` 以 sessionToken 为 key，迁移时 persistentId 对首次迁移定位旧 sessionToken 最稳健（鸡生蛋约束：迁移分支用 `GetAbilityRecordByPersistentId`，与 map 清理逻辑一致）。

**X 持久性保证**：窗口侧 `Rosen::SessionInfo sessionInfo_` 是 `Session` 基类持久成员（`session.h:1104`），迁移时 `EditSessionInfo()` 原地更新（`session.cpp:685`），不重建对象。`originalCallerInfo.uiExtensionComponentId` 一旦设置即随持久对象存在于所有后续操作。

---

## 1. 架构上下文

### 1.1 现有链路（保持不动）

```
应用侧调用 StartUIExtensionAbility
  ↓
AbilityManagerService::StartUIExtensionAbility          [ability_manager_service.cpp:4800]
  │  生成 uiExtensionComponentId = (callerRecordId << 32) | persistentId   [:4942]
  │  委托给 UIExtensionAbilityManager
  ↓
UIExtensionAbilityManager::StartAbilityLocked            [ui_extension_ability_manager.cpp:613]
  │
  ├── GetOrCreateExtensionRecord(abilityRequest, ...)     [:646]
  │     ↓
  │   ExtensionRecordManager::GetOrCreateExtensionRecord  [extension_record_manager.cpp:213]
  │     │
  │     ├── GetAbilityRecordBySessionInfo(sessionInfo)    [:217]  ← 按 uiExtensionComponentId 匹配
  │     │     └─ 找到 → isLoaded=true, return             [:218-221]
  │     │     └─ 未找到 → 继续
  │     │
  │     ├── IsPreloadExtensionRecord(...)                 [:237]
  │     │     └─ 命中预加载 → Update + isLoaded=true
  │     │
  │     └── GetOrCreateExtensionRecordInner(...)          [:247]
  │           └─ 创建新 record
  │
  ├── targetService->SetSessionInfo(sessionInfo)          [:666]  ← 无条件更新
  ├── uiExtensionMap_[sessionToken] = {targetService, ...} [:668-676]  ← 无条件更新
  │
  ├── if (!isLoadedAbility)
  │     └── LoadAbility(targetService)                    [:701]  ← 首次加载
  └── else
        └── DoForegroundUIExtension(targetService, ...)   [:705]
              └── CommandAbilityWindow → ScheduleCommandAbilityWindow  ← IPC 传 sessionInfo 到提供方
```

### 1.2 迁移场景的核心问题

**关键前提（需求方确认）**：窗口迁移时 **persistentId 不变**，只有 callerToken（对应 Ability 窗口信息）变化。

```
首次创建 UEC（窗口 A，Ability 实例 1）：
  sessionInfo.callerToken = tokenA
  sessionInfo.persistentId = P              ← 不变
  uiExtensionComponentId = (recordId1 << 32) | P

迁移到窗口 B（Ability 实例 2）：
  sessionInfo.callerToken = tokenB          ← 变了
  sessionInfo.persistentId = P              ← 不变
  uiExtensionComponentId = (recordId2 << 32) | P  ← 变了（高32位变）
  sessionInfo.originalCallerInfo = { callerToken: tokenA, persistentId: P }  ← 迁移标识

  → GetAbilityRecordBySessionInfo 用新 uiExtensionComponentId 匹配
  → 原 record 存储的是旧 uiExtensionComponentId
  → 匹配失败 → 走"创建新 record"路径 ← 错误！应该更新原 record

  解决：新增分支按 persistentId P 匹配（P 不变），定位到原 record 再更新 callerToken
```

### 1.3 本次设计后链路（新增迁移更新分支 + 应用侧迁移感知）

```
[AMS 服务侧]
UIExtensionAbilityManager::StartAbilityLocked
  │
  ├── GetOrCreateExtensionRecord(abilityRequest, ...)
  │     ↓
  │   ExtensionRecordManager::GetOrCreateExtensionRecord
  │     │
  │     ├── [新增分支0] IsMigrationUpdateRequest?              ← originalCallerInfo.callerToken≠空
  │     │     ↓ Yes
  │     │   GetAbilityRecordByPersistentId(persistentId)       ← persistentId 不变，直接匹配
  │     │     ├── 找到原 record → isLoaded=true, return        ← 迁移更新成功
  │     │     └── 未找到 → 记警告日志，继续走原逻辑（容错）
  │     │
  │     ├── [现有] GetAbilityRecordBySessionInfo               ← 不变
  │     ├── [现有] IsPreloadExtensionRecord                    ← 不变
  │     └── [现有] GetOrCreateExtensionRecordInner             ← 不变
  │
  ├── SetSessionInfo(新 sessionInfo)                           ← callerToken 更新为新值
  ├── [新增] 清理 uiExtensionMap_ 旧映射 (按 persistentId 查找)
  ├── uiExtensionMap_[newSessionToken] = {record, sessionInfo} ← 建立新映射
  │
  └── DoForegroundUIExtension → CommandAbilityWindow(WIN_CMD_FOREGROUND)
        ↓ ScheduleCommandAbilityWindow (IPC)

[应用侧 — UIExtension 提供方进程]
ExtensionAbilityThread → ExtensionImpl → UIExtension::OnCommandWindow
  │
  ├── [新增] 迁移检测: originalCallerInfo.callerToken ≠ nullptr?
  │     ↓ Yes
  │   HandleSessionUpdate(want, sessionInfo)     ← 新增虚方法，JsUIExtension 重写
  │   return                                      ← 不进入现有 ForegroundWindow
  │
  └── [现有] 非 migration: ForegroundWindow / BackgroundWindow / DestroyWindow ← 不变

JsUIExtension::HandleSessionUpdate (重写，Rev2)
  ├── compId = GetEffectiveComponentId(sessionInfo)     ← 稳定 X，迁移前后不变
  ├── contentSessions_.find(compId) 直接命中            ← 无需 erase/re-emplace key
  │   (uiWindowMap_ 同以 stable X 为 key，对象复用不变，见 ADR-8)
  └── napi_unwrap 拿 ContentSession C++ 指针 → DoUpdateExtensionParent(sessionInfo)
```

### 1.4 关键约束

- `GetAbilityRecordBySessionInfo` 按 `uiExtensionComponentId` 匹配的主路径逻辑**不变**
- 迁移更新分支是 `GetOrCreateExtensionRecord` 内的**新增前置分支**（非修改现有分支）
- 现有 preload / create-new 路径**逐行不变**
- 现有所有测试继续通过

### 1.5 sessionInfo 迁移时变化字段清单（window_manager 侧）

> 来源：window_manager PR #19373 + ability_base 代码检视。涉及**两个不同的 SessionInfo 结构体**，勿混淆：
> - **Rosen::SessionInfo**（window_manager 侧，`Session` 基类持久成员）：`ExtensionSessionManager::UpdateExtensionSessionParent` 通过 `EditSessionInfo()` 原地更新下列字段，再由 `SetAbilitySessionInfo` 构造 **AAFwk::SessionInfo** 发给 AMS。
> - **AAFwk::SessionInfo**（ability_base，AMS 实际消费/透传）：字段名与 Rosen 侧**不同**（见下表"AAFwk 对应字段"列）。
>
> AMS 侧**不关心、不解析**这些字段的业务语义，仅在 `SetSessionInfo` / `DoForegroundUIExtension` → `ScheduleCommandAbilityWindow` 链路中**透传**给提供方进程；提供方 `DoUpdateExtensionParent` 用 `sessionInfo_ = sessionInfo` 整体指针替换隐式覆盖全部字段。下列出仅为跨仓对齐与审计。

| Rosen::SessionInfo 字段（window_manager 侧 EditSessionInfo 更新） | AAFwk::SessionInfo 对应字段（ability_base，AMS 透传） | 迁移时变化 | AMS 是否解析 |
|------|------|-----------|--------------|
| `callerToken_` | `callerToken`（session_info.h:59） | = 新窗口 callerToken | 是（originalCallerInfo 标识 + record 按 persistentId 匹配） |
| `rootToken_` | `parentToken`（:60） | = 新 parentToken | 否，透传 |
| `realParentId_` | `realHostWindowId`（:65） | = 新 realHostWindowId | 否，透传 |
| `parentWindowType_` | `parentWindowType`（:66） | = 新 parentWindowType | 否，透传 |
| `config_.displayId_` | `displayId`（:101，**扁平成员，无 config_ 子结构**） | = 新 displayId | 否，透传 |
| `config_.density_` | `density`（:102） | = 新 density | 否，透传 |
| `config_.orientation_` | `orientation`（:103） | = 新 orientation | 否，透传 |
| —（`abilitySessionInfo->hostWindowId` 直赋值） | `hostWindowId`（:64） | = newHostWindowId | 否，透传（提供方用于 CreateJsExtensionWindow / UpdateExtensionParent） |
| —（AMS 侧生成） | `uiExtensionComponentId`（:100） | callerRecordId 变 → 整体变；另 `originalCallerInfo.uiExtensionComponentId`=稳定 X（回填） | 是，AMS 生成 `(callerRecordId<<32)|persistentId`，作 uiExtensionMap_ key；X 用于 GetEffectiveComponentId |
| — | `persistentId`（:63） | 不变 | 是，AMS 按 persistentId 定位原 record |
| — | `originalCallerInfo`（:120，3 字段：callerToken/persistentId/uiExtensionComponentId） | 迁移标识 + 稳定 X 回填 | 是，IsMigrationUpdateRequest 判断 + GetEffectiveComponentId |
| — | `uiWindow_`（Rosen::Window 对象，content session 持有） | 对象复用不变（不重建，仅调 UpdateExtensionParent 更新 property） | — |

> **检视注**：AAFwk::SessionInfo 无 `rootToken`/`realParentId`/`config_` 字段；Rosen→AAFwk 的字段映射发生在 window_manager 的 `SetAbilitySessionInfo`。`DoMarshallingEight`/`ReadParcelThree` 实际序列化 `originalCallerInfo` 的**3 个字段**（callerToken + persistentId + uiExtensionComponentId），非 2 个。

---

## 2. 设计决策 (ADR)

### ADR-1: 迁移更新分支注入点 — GetOrCreateExtensionRecord 新增前置分支

**决策**：在 `ExtensionRecordManager::GetOrCreateExtensionRecord`（`extension_record_manager.cpp:213`）函数体最前面，在现有 `GetAbilityRecordBySessionInfo` 调用（行 217）之前，新增迁移更新分支。

```cpp
int32_t ExtensionRecordManager::GetOrCreateExtensionRecord(...) {
    CHECK_POINTER_AND_RETURN(abilityRequest.sessionInfo, ERR_INVALID_VALUE);

    // [新增] 迁移更新分支：检查 originalCallerInfo（迁移标识）
    if (IsMigrationUpdateRequest(abilityRequest.sessionInfo)) {
        // persistentId 迁移时不变，直接用当前 sessionInfo 的 persistentId 匹配
        abilityRecord = GetAbilityRecordByPersistentId(abilityRequest.sessionInfo->persistentId);
        if (abilityRecord != nullptr) {
            isLoaded = true;
            TAG_LOGI(AAFwkTag::ABILITYMGR, "migration update: found record by persistentId");
            return ERR_OK;
        }
        TAG_LOGW(AAFwkTag::ABILITYMGR, "migration update: record not found, fallback");
        // 未找到原 record，继续走现有逻辑（容错）
    }

    // [现有] 按 uiExtensionComponentId 匹配
    abilityRecord = GetAbilityRecordBySessionInfo(abilityRequest.sessionInfo);
    if (abilityRecord != nullptr) {
        isLoaded = true;
        return ERR_OK;
    }
    // ... 现有 preload / create-new 逻辑不变
}
```

**理由**：
- 迁移更新分支在最前面，命中后直接 return，不影响现有逻辑
- 未找到原 record 时**容错降级**到现有逻辑（创建新 record），保证不 crash
- `isLoaded=true` 确保 `StartAbilityLocked` 走 `DoForegroundUIExtension` 路径（而非 `LoadAbility`），不创建新 Ability 实例

**替代方案**：
- 在 `StartAbilityLocked` 中分流（在调用 `GetOrCreateExtensionRecord` 之前）— 需要在两个地方维护迁移逻辑，改动面更大
- 在 `ability_manager_service.cpp::StartUIExtensionAbility` 中分流 — 太靠前，sessionInfo 尚未完整构造

### ADR-2: 匹配策略 — 按 persistentId 匹配（迁移时 persistentId 不变）

> **⚠ Revision 2 修订**：本节描述的 persistentId 匹配策略**保持不变**。新增内容：`OriginalCallerInfo` 新增稳定 `uiExtensionComponentId` 字段（= 首创时 AMS 锁定的原始 X），下游统一用 `GetEffectiveComponentId()` 索引。`persistentIdToComponentId_` 映射和 `HandleSessionUpdate` key 滚动逻辑**完全移除**。全部变体（JS/ETS/CJ/PhotoEditor）支持迁移。详见本文档「Revision 2」节。

**关键前提（需求方确认）**：窗口迁移时 **persistentId 不变**，只有 callerToken（对应 Ability 窗口信息）变化。

因此 `uiExtensionComponentId = (callerRecordId << 32) | persistentId` 会变（高 32 位 callerRecordId 变），但 record 的 `sessionInfo.persistentId` 始终不变。

**决策**：在 `ExtensionRecordManager` 新增 `GetAbilityRecordByPersistentId`，直接用当前 sessionInfo 的 `persistentId` 匹配已存在的 extension record。`originalCallerInfo.callerToken` 仅作为**迁移标识**（非匹配键）。

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

**originalCallerInfo 各字段作用**：

| 字段 | 作用 | 说明 |
|------|------|------|
| `originalCallerInfo.callerToken` | **迁移标识** | 非空 = 这是一次迁移更新请求（ADR-6 `IsMigrationUpdateRequest` 判断依据） |
| `originalCallerInfo.persistentId` | **一致性校验** | 应等于当前 sessionInfo.persistentId（迁移时不变）；可用于校验，非匹配键 |

**为什么不能复用 `GetAbilityRecordBySessionInfo`**：
- 该函数按 `uiExtensionComponentId` 匹配
- 迁移后 `callerRecordId` 变化 → `uiExtensionComponentId` 变化 → 新 ID 与原 record 存储的老 ID 不同 → 匹配失败

**替代方案（已否决）**：
- 按 `originalCallerInfo.callerToken + persistentId` 匹配 — 多余，persistentId 已足够唯一标识 session
- 在 `originalCallerInfo` 中存储原始 `uiExtensionComponentId` 复用现有函数 — 扩展结构超出 SR 定义

### ADR-3: uiExtensionMap_ 旧映射清理 — ⚠ 已废弃（sessionToken 迁移时不变）

> **⚠ 2026-07-09 修订（基于需求方澄清）**：**迁移后 sessionToken 不发生变化**。
> `uiExtensionMap_` 以 sessionToken（remoteObj）为 key（`ui_extension_ability_manager.cpp:669`）。由于迁移前后 sessionToken 是同一对象，现有行 672 `uiExtensionMap_[remoteObj] = {targetService, abilityRequest.sessionInfo}` 已是对**同一 key 的 value 原地更新**（新 sessionInfo，含新 callerToken）——天然正确，无需清理旧映射。又因同一 sessionToken 的 DeathRecipient 已注册，行 674 `AddUIExtWindowDeathRecipient` 对重复注册幂等返回（`:811-814` "recipient added before"）。`GetUIExtensionBySessionInfo`（`:367-396`）按 sessionToken 查找（`:375`）并校验 callerToken 一致性（`:386-387`）：迁移后 map value 已更新为新 callerToken，故用新 sessionInfo 查询能正确命中原 record。
>
> **结论**：本 ADR 原"按 persistentId 清理旧 sessionToken 映射"的前提「sessionToken 变化导致旧映射泄漏」**不成立**，ADR-3 **废弃**。下方原决策仅留档。
>
> **原决策（已废弃）**：在 `StartAbilityLocked`（`ui_extension_ability_manager.cpp:668-676`）更新 `uiExtensionMap_` 时，如果检测到 `originalCallerInfo.callerToken` 非空（迁移场景），**先按 persistentId 查找并清理旧 sessionToken 映射，再建立新映射**。由于 persistentId 迁移时不变，可通过 persistentId 在 `uiExtensionMap_` 中定位旧映射条目。

```cpp
// 在 uiExtensionMap_ 更新处新增（行 668 之前）
if (abilityRequest.sessionInfo &&
    abilityRequest.sessionInfo->originalCallerInfo.callerToken != nullptr) {
    // 迁移场景：按 persistentId 查找并清理旧 sessionToken 映射
    auto persistentId = abilityRequest.sessionInfo->persistentId;
    std::lock_guard guard(uiExtensionMapMutex_);
    for (auto it = uiExtensionMap_.begin(); it != uiExtensionMap_.end(); ++it) {
        auto &savedSessionInfo = it->second.second;
        if (savedSessionInfo && savedSessionInfo->persistentId == persistentId) {
            uiExtensionMap_.erase(it);
            TAG_LOGD(AAFwkTag::ABILITYMGR, "migration: erased old sessionToken for persistentId: %{public}d",
                persistentId);
            break;
        }
    }
}
```

**理由**：
- 迁移后旧 sessionToken 已失效，保留会导致 `GetUIExtensionBySessionInfo` 可能返回过期映射
- persistentId 不变，可直接用作查找旧映射的依据，无需扩展 originalCallerInfo 结构
- 不依赖 `originalCallerInfo.oldSessionToken`，避免跨仓结构体扩展

**替代方案（已否决）**：
- 在 originalCallerInfo 中新增 oldSessionToken 字段 — 扩展 SR 定义的结构体，增加跨仓对齐成本
- 不清理旧映射，依赖 sessionToken DeathRecipient — 旧 sessionToken 不会自然死亡（仍被旧窗口持有），导致映射泄漏

### ADR-4: 多次迁移策略 — persistentId 不变，天然支持多次迁移

**关键简化**：由于 persistentId 迁移时不变，且 AMS 侧按 persistentId 匹配 record（ADR-2），多次迁移无需特殊策略。

```
初始状态：record.sessionInfo = { callerToken: A, persistentId: P }

第 1 次迁移 A → B：
  入参 sessionInfo = { callerToken: B, persistentId: P,
                       originalCallerInfo: { callerToken: A, persistentId: P } }
  → 按 persistentId P 匹配到 record ✓
  → 更新 record.sessionInfo.callerToken = B（persistentId 不变）

第 2 次迁移 B → C：
  入参 sessionInfo = { callerToken: C, persistentId: P,
                       originalCallerInfo: { callerToken: B, persistentId: P } }
  → 按 persistentId P 匹配到 record ✓（persistentId 从未改变）
  → 更新 record.sessionInfo.callerToken = C
```

**结论**：originalCallerInfo.callerToken 记录"上一次的调用方"，用于迁移标识和审计追溯；实际匹配始终依赖 persistentId，天然支持任意次数迁移。

**跨领域约定**：window_manager 侧发起迁移时，originalCallerInfo.callerToken 填入当前（迁移前）的 callerToken，persistentId 填入当前 persistentId（与目标相同）。

### ADR-5: 并发保护 — 复用现有 mutex_ + 新增原子性保证

**决策**：
1. `GetAbilityRecordByPersistentId` 遍历匹配时持有 `ExtensionRecordManager::mutex_`（与 `GetAbilityRecordBySessionInfo` 一致）
2. `StartAbilityLocked` 中的 SetSessionInfo + uiExtensionMap_ 更新 + 旧映射清理需保证顺序性，在 `UIExtensionAbilityManager` 层面加锁保护

**理由**：
- `SetSessionInfo` 内部已有 `sessionLock_` 保护（`ability_record.cpp:2437`）
- `uiExtensionMap_` 已有 `uiExtensionMapMutex_` 保护
- 但"定位原 record + 更新 sessionInfo + 更新 map"是跨多个锁的复合操作
- 在 `UIExtensionAbilityManager` 层面，`StartAbilityLocked` 已通过调用链串行化，同一 sessionInfo 不会并发进入

**残余风险**：不同 sessionInfo 并发操作同一 record（如同时迁移到两个不同窗口）。缓解：迁移操作由应用侧发起（window_manager），应用层应保证不并发迁移同一 UEC。本次不在 ability_runtime 层增加额外防并发机制（YAGNI），但在 review 时验证。

### ADR-6: 迁移标识判断函数 — IsMigrationUpdateRequest

**决策**：新增内联判断函数，封装"originalCallerInfo 是否为迁移更新请求"的判断逻辑。

```cpp
bool ExtensionRecordManager::IsMigrationUpdateRequest(
    const sptr<AAFwk::SessionInfo> &sessionInfo) const
{
    return sessionInfo != nullptr &&
           sessionInfo->originalCallerInfo.callerToken != nullptr &&
           sessionInfo->originalCallerInfo.persistentId != 0;
}
```

**理由**：
- 集中判断逻辑，避免多处重复 `callerToken != nullptr && persistentId != 0`
- 与 SR 第 3 条一致："如果 originalCallerInfo 里的 callerToken 和 persistentId 不为默认值，标识 SessionInfo 需要更新"
- persistentId 检查保留：虽然迁移时 persistentId 不变，但校验其非默认值可防止异常 sessionInfo 误触发迁移分支

### ADR-7: 应用侧 sessionInfo 更新传递链路 — OnCommandWindow 迁移感知 + JsUIExtensionContentSession 调窗口新接口

**问题**：AMS 侧更新 record 的 sessionInfo 后，需将新 sessionInfo 传递到 UIExtension 提供方进程的 window 子系统。现有 IPC 链路已能传递 sessionInfo，但应用侧缺少"更新已创建窗口 session 绑定"的处理。

**现有完整 IPC 链路（代码调研结论）**：

```
[AMS 服务侧]
LifecycleDeal::CommandAbilityWindow                      [lifecycle_deal.cpp:105]
  → AbilitySchedulerProxy::ScheduleCommandAbilityWindow   [ability_scheduler_proxy.cpp:227]
    序列化: Want + SessionInfo + WindowCommand
    IPC code: SCHEDULE_ABILITY_COMMAND_WINDOW (TF_ASYNC)
      ↓ [IPC 跨进程]
[应用侧]
AbilitySchedulerStub::CommandAbilityWindowInner          [ability_scheduler_stub.cpp:261]
  → ExtensionAbilityThread::ScheduleCommandAbilityWindow  [extension_ability_thread.cpp:626]
    PostTask 到 abilityHandler_ 事件循环
    → HandleCommandExtensionWindow                         [extension_ability_thread.cpp:444]
      → ExtensionImpl::CommandExtensionWindow              [extension_impl.cpp:468]
        → UIExtension::OnCommandWindow                      [ui_extension.cpp:212]
          switch(winCmd):
            WIN_CMD_FOREGROUND → ForegroundWindow()         [ui_extension.cpp:135]
              → HandleSessionCreate(want, sessionInfo)       ← JsUIExtension 重写
                → JsUIExtensionContentSession::Create(...)   ← 创建 content session
                → JsExtensionWindow::CreateJsExtensionWindow ← 创建 JS 窗口对象
                → contentSessions_[componentId] = session    ← 以 componentId 为 key 存储
                → uiWindowMap_[componentId] = window
            WIN_CMD_BACKGROUND → BackgroundWindow()
            WIN_CMD_DESTROY    → DestroyWindow()
```

**应用侧窗口与 content session 的管理结构（代码调研结论）**：

| 结构 | 位置 | key | 说明 |
|------|------|-----|------|
| `UIExtension::uiWindowMap_` | `ui_extension.h` | `uiExtensionComponentId` | `uint64_t → sptr<Rosen::Window>` |
| `JsUIExtension::contentSessions_` | `js_ui_extension.h:199` | `uiExtensionComponentId` | `uint64_t → shared_ptr<NativeReference>` (JS content session) |
| `JsUIExtensionContentSession::sessionInfo_` | `js_ui_extension_content_session.h:124` | — | `sptr<SessionInfo>`，创建后从不更新 |
| `JsUIExtensionContentSession::uiWindow_` | `js_ui_extension_content_session.h:125` | — | `sptr<Rosen::Window>`，通过 `CreateJsExtensionWindow` 创建 |

**迁移时的核心问题**（⚠ Rev2 修订）：
1. ~~`contentSessions_` 和 `uiWindowMap_` 均以 `uiExtensionComponentId` 为 key — 迁移后 componentId 变了，用新 ID 查找为空~~ — **Rev2 已解决**：AMS 在迁移分支回填 `originalCallerInfo.uiExtensionComponentId = X`（首创时锁定的稳定值），`GetEffectiveComponentId(sessionInfo)` 迁移前后恒为 X，`contentSessions_[X]`/`uiWindowMap_[X]` 直接命中，**无需换 key**。
2. `JsUIExtensionContentSession::sessionInfo_` 创建后从不更新 — 仍成立，由 `DoUpdateExtensionParent` 整体指针替换。
3. 窗口的 session 绑定需要通过窗口的新接口更新（window_manager 需提供新能力）

**决策**：迁移感知分两层处理——基类 `UIExtension::OnCommandWindow` 检测迁移并分流，`JsUIExtension` 重写 `HandleSessionUpdate` 执行窗口更新。

```
[第一层：UIExtension 基类 — 迁移检测与分流]

UIExtension::OnCommandWindow(want, sessionInfo, winCmd)
  │
  ├── [新增] if (IsMigration(sessionInfo) && winCmd == FOREGROUND)
  │     ↓
  │   HandleSessionUpdate(want, sessionInfo)   ← 新增虚方法，JsUIExtension 重写
  │   return                                    ← 不进入现有 ForegroundWindow
  │
  └── [现有] switch(winCmd) { ForegroundWindow / BackgroundWindow / DestroyWindow }  ← 不变


[第二层：JsUIExtension — 按 stable compId 定位 + 委派 ContentSession（Rev2，无 key 滚动）]

JsUIExtension::HandleSessionUpdate(want, sessionInfo)   ← 重写基类虚方法
  │
  ├── 1. compId = GetEffectiveComponentId(sessionInfo)   ← 稳定 X，迁移前后不变
  ├── 2. contentSessions_.find(compId) 直接命中（无需 erase/re-emplace）
  └── 3. napi_unwrap 拿 ContentSession C++ 指针 → DoUpdateExtensionParent(sessionInfo)
         ← 窗口 parent 更新全部由 ContentSession 完成（见第三层）

> Rev2：不操作 `persistentIdToComponentId_`（已废弃，代码中不存在）；不 erase/re-emplace
> `contentSessions_`/`uiWindowMap_` 的 key。`uiWindowMap_` 同样以 stable X 为 key，无需换。


[第三层：JsUIExtensionContentSession — sessionInfo_ + proxy 更新]

JsUIExtensionContentSession::DoUpdateExtensionParent(sessionInfo)   ← 内部 C++ 方法，非 napi 接口
  │
  ├── a. 更新 sessionInfo_ 指针（hostWindowId、componentId 等随之变化）
  │     uiWindow_ 对象复用不变（ADR-8），但需调其 UpdateExtensionParent 更新 parent 绑定（见 b）
  │
  └── b. 更新 uiWindow_ 与已创建的 JsExtensionWindow proxy 列表
        → 直接调用 uiWindow_->UpdateExtensionParent(WindowOption)（C++ 接口，非 napi）
        → 遍历 jsExtensionWindowRefs_（应用通过 getUIExtensionHostWindowProxy
           /getUIExtensionWindowProxy 获取的 proxy，被应用持有）
           napi_unwrap 拿 JsExtensionWindow* → 直接调 C++ UpdateExtensionParent(WindowOption)
        → 无需 window_manager 暴露 napi 方法
        → 新 proxy 不主动创建：应用下次调 getUIExtension*Proxy 时，
           因 sessionInfo_->hostWindowId 已更新，自然用新值 CreateJsExtensionWindow
```

**JsExtensionWindow proxy 更新机制（详细说明）**：

每个 JsExtensionWindow proxy 是 `Rosen::JsExtensionWindow::CreateJsExtensionWindow(env, uiWindow_, hostWindowId)` 创建的 napi 对象，内部持有：
- `sptr<Rosen::Window> uiWindow_` — 底层窗口对象引用
- `uint32_t hostWindowId` — 宿主窗口 ID（值类型，迁移后过期）

迁移后需要更新每个 proxy 的内部状态。window_manager PR #19373 已提供 C++ 接口 `Window::UpdateExtensionParent(WindowOption)` 与 `JsExtensionWindow::UpdateExtensionParent(WindowOption)`，ability_runtime **直接 C++ 调用，无需 napi 暴露**。

**方案：uiWindow_ + 遍历保留的老 proxy，直接调 C++ UpdateExtensionParent(WindowOption)**

```cpp
void JsUIExtensionContentSession::DoUpdateExtensionParent(const sptr<SessionInfo> &sessionInfo)
{
    if (sessionInfo == nullptr) return;
    // a. 更新 sessionInfo_（uiWindow_ 对象复用不变，见 ADR-8）
    sessionInfo_ = sessionInfo;

    // 构造 WindowOption（newHostWindowId = sessionInfo_->hostWindowId 等，入参定义以 window_manager 为准）
    sptr<Rosen::WindowOption> option = ...;

    // b. uiWindow_ 直接调 C++ 接口
    if (uiWindow_ != nullptr) {
        uiWindow_->UpdateExtensionParent(option);
    }

    // c. 遍历老 proxy，napi_unwrap 拿 JsExtensionWindow* 直接调 C++ 接口（非 napi 方法调用）
    for (auto &ref : jsExtensionWindowRefs_) {
        napi_value proxy = nullptr;
        napi_get_reference_value(env_, ref, &proxy);
        if (proxy == nullptr) continue;  // proxy 已被 GC 回收
        Rosen::JsExtensionWindow* jsExtWin = nullptr;
        napi_unwrap(env_, proxy, reinterpret_cast<void**>(&jsExtWin));
        if (jsExtWin != nullptr) {
            jsExtWin->UpdateExtensionParent(option);
        }
    }
}
```

**env_ 成员**：`JsUIExtensionContentSession` 新增 `napi_env env_` 成员，在 `CreateJsUIExtensionContentSession` 中 `napi_wrap` 之前存入，供 `DoUpdateExtensionParent` 遍历 proxy 时 `napi_unwrap` 取 JsExtensionWindow C++ 指针使用。

**跨仓依赖**：
- window_manager T0b 提供 C++ 接口 `Window::UpdateExtensionParent(WindowOption)` 与 `JsExtensionWindow::UpdateExtensionParent(WindowOption)`（PR #19373 已具备）
- **无需 napi 暴露**：ability_runtime 直接 C++ 调用，不经 napi 方法表
- `JsExtensionWindow::UpdateExtensionParent` 内部更新 proxy 的 `hostWindowId_` 并调用底层 `windowImpl->UpdateExtensionParent(option)`
```

**JsExtensionWindow proxy 跟踪机制**（⚠ 代码检视：已实现）：

`OnGetUIExtensionHostWindowProxy` / `OnGetUIExtensionWindowProxy`（位于 `JsUIExtensionContentSession`）每次调 `CreateJsExtensionWindow` 创建新 proxy 返回应用，并**已记录 `napi_ref` 到 `jsExtensionWindowRefs_`**（`.cpp:314-316,346-348`）。迁移时遍历更新这些 proxy 的 parent。

**决策**：`JsUIExtensionContentSession` 的 `std::vector<napi_ref> jsExtensionWindowRefs_` 成员，在每次创建 proxy 时记录引用，迁移时遍历更新。

```cpp
// OnGetUIExtensionHostWindowProxy / OnGetUIExtensionWindowProxy 修改（创建 proxy 时记录）
napi_value jsExtensionWindow =
    Rosen::JsExtensionWindow::CreateJsExtensionWindow(env, uiWindow_, sessionInfo_->hostWindowId);
// [新增] 记录 proxy 引用，供迁移时更新
napi_ref ref = nullptr;
napi_create_reference(env, jsExtensionWindow, 1, &ref);
jsExtensionWindowRefs_.push_back(ref);
```

**新增虚方法（基类声明 + JsUIExtension 重写）**：

```cpp
// UIExtension 基类 (ui_extension.h)
virtual void HandleSessionUpdate(const Want &want, const sptr<SessionInfo> &sessionInfo) {}
// 默认空实现：非 JsUIExtension 的 UIExtension 派生类不处理迁移（保持现有行为）

// JsUIExtension 重写 (js_ui_extension.h)
void HandleSessionUpdate(const Want &want, const sptr<SessionInfo> &sessionInfo) override;
```

**新增应用侧方法清单**：

| 方法 | 位置 | 说明 |
|------|------|------|
| `UIExtension::HandleSessionUpdate(want, sessionInfo)` | `ui_extension.h / .cpp` | 新增虚方法，基类默认空实现 |
| `JsUIExtension::HandleSessionUpdate(want, sessionInfo)` | `js_ui_extension.h / .cpp` | 重写（Rev2）：`compId = GetEffectiveComponentId(sessionInfo)`（稳定 X）→ `contentSessions_.find(compId)` 直接命中（**无 key 滚动**）→ `napi_unwrap` 拿 ContentSession 指针调 `DoUpdateExtensionParent` |
| `JsUIExtensionContentSession::DoUpdateExtensionParent(sessionInfo)` | `js_ui_extension_content_session.h / .cpp` | 内部 C++ 方法（非 napi）：更新 sessionInfo_ + 调 `uiWindow_->UpdateExtensionParent(WindowOption)` + 遍历 jsExtensionWindowRefs_ 经 napi_unwrap 调老 proxy 的 C++ `UpdateExtensionParent(WindowOption)`。**无需 napi 暴露**；uiWindow_ 对象复用不变（ADR-8） |
| `JsUIExtensionContentSession::jsExtensionWindowRefs_` | 同上 | 新增成员：`vector<napi_ref>` 跟踪应用持有的老 JsExtensionWindow proxy |
| `JsUIExtensionContentSession::env_` | 同上 | 新增成员：`napi_env`，CreateJsUIExtensionContentSession 时存入，供 DoUpdateExtensionParent 使用 |
| ~~`JsUIExtension::persistentIdToComponentId_`~~ | ~~`js_ui_extension.h`~~ | **Rev2 废弃**：原 Rev1 索引成员，代码中不存在；Rev2 用 `GetEffectiveComponentId` 替代，无需此 map |
| `OnGetUIExtensionHostWindowProxy` / `OnGetUIExtensionWindowProxy` | `js_ui_extension_content_session.h / .cpp`（**非 js_ui_extension**） | 修改：创建 proxy 后记录 napi_ref 到 `jsExtensionWindowRefs_` |

**窗口子系统更新（跨仓依赖）**：
- window_manager T0b 提供 C++ 接口 `Window::UpdateExtensionParent(WindowOption)` 与 `JsExtensionWindow::UpdateExtensionParent(WindowOption)`（PR #19373 已具备），**无需 napi 暴露**
- `JsExtensionWindow::UpdateExtensionParent` 内部更新 proxy 的 `hostWindowId_` 并调底层 `windowImpl->UpdateExtensionParent(option)`
- **uiWindow_（Rosen::Window）对象迁移时复用不变**（不重建，ADR-8），但需调用其 `UpdateExtensionParent(WindowOption)` 更新 parent 绑定
- 新 proxy 不主动创建：应用下次调 `getUIExtension*Proxy` 时，因 `sessionInfo_->hostWindowId` 已更新，自然用新值 `CreateJsExtensionWindow`
- 本次 ability_runtime 侧负责：迁移检测 + sessionInfo 传递 + content session / map key 更新 + uiWindow_/老 proxy 的 UpdateExtensionParent C++ 调用
- window_manager 侧负责：`Window::UpdateExtensionParent` / `JsExtensionWindow::UpdateExtensionParent` C++ 接口实现

**替代方案（已否决）**：
- 新增 `WIN_CMD_UPDATE_SESSION` WindowCommand — 需新增 IPC code，违反 SR"使用老接口加新参数"原则
- 在 `ForegroundWindow` 中直接处理 — 迁移逻辑与首次创建逻辑混合，降低可维护性
- 在 `UIExtension` 基类直接操作 contentSessions_ — `contentSessions_` 是 `JsUIExtension` 的成员，基类无法访问
- 不跟踪 proxy，让应用自行重新获取 — 违反兼容性，应用持有的旧 proxy 已失效

### ADR-8: uiWindow_ 迁移时对象复用不变，但需调 UpdateExtensionParent 更新 parent 绑定

> **⚠ 修订**：原决策"不调用 uiWindow_ 的任何更新方法"已废弃。依 window_manager PR #19373，`Window::UpdateExtensionParent(WindowOption)` 与 `JsExtensionWindow::UpdateExtensionParent(WindowOption)` 均为 C++ 接口，ability_runtime 直接调用，无需 napi 暴露。

**决策**：迁移时 `Rosen::Window`（uiWindow_）**对象复用不变**（不重建，window sptr 不变），但需调用 `uiWindow_->UpdateExtensionParent(WindowOption)` 更新 parent 绑定；应用持有的老 JsExtensionWindow proxy 同样通过 C++ 直接调 `UpdateExtensionParent(WindowOption)` 更新其内部 `hostWindowId_`。

**理由**：
- 窗口对象本身不重建（PR 中 `WindowExtensionSessionImpl::UpdateExtensionParent` 只改 property，不重建窗口对象），window sptr 不变
- 但 parent 绑定（parentId/realParentId/callingSessionId/parentWindowType）必须显式更新，需调 `UpdateExtensionParent(WindowOption)` 触发；window_manager 不会仅凭 sessionInfo 变化"自然感知"，需 ability_runtime 显式调用
- `JsUIExtension::HandleSessionUpdate` 换 `uiWindowMap_` 的 key（oldCompId → newCompId），window sptr 不变
- `DoUpdateExtensionParent` 更新 `sessionInfo_` 后，调 `uiWindow_->UpdateExtensionParent(option)` + 遍历老 proxy 调 `UpdateExtensionParent(option)`
- 后续应用调 `getUIExtension*Proxy` 时，`CreateJsExtensionWindow(env, uiWindow_, sessionInfo_->hostWindowId)` 传入的 `uiWindow_` 仍是同一对象，`hostWindowId` 从更新后的 `sessionInfo_` 取新值

### ADR-9: 迁移前后台时序约束

**决策**：迁移期间旧 session 不能触发 `WIN_CMD_DESTROY`，否则 `HandleSessionUpdate` 找不到旧记录。

**时序分析**：
```
旧 session (oldCompId)              新 session (newCompId)
─────────────────────              ──────────────────────
1. WIN_CMD_BACKGROUND
   → BackgroundWindow(oldCompId)
   → uiWindow->Hide()              ❶ 窗口保活：UIExtension::BackgroundWindow
                                     只 Hide + foregroundWindows_.erase，
                                     不删 uiWindowMap_（ui_extension.cpp:156-178）

2. (旧 session 不能 Destroy!)       ← 关键约束

                                    3. WIN_CMD_FOREGROUND (带 originalCallerInfo)
                                       → OnCommandWindow 迁移检测
                                       → HandleSessionUpdate（Rev2：按 stable X 直接命中，不换 key）
                                       ❷ DoUpdateExtensionParent 更新 sessionInfo_ + uiWindow_/proxy parent
```

**关键代码事实**（⚠ Rev2 + 空桩澄清）：
- `UIExtension::BackgroundWindow`（基类 `ui_extension.cpp:156`）：只 Hide，**不动 uiWindowMap_** → 窗口在迁移期间存活
- `UIExtension::DestroyWindow`（基类 `ui_extension.cpp:180`）是**空桩**（仅日志）；真销毁是其子类重写 `JsUIExtension::DestroyWindow`（`js_ui_extension.cpp:777`）→ erase `uiWindowMap_` + FreeNativeReference
- Rev2：`HandleSessionUpdate` 用 `GetEffectiveComponentId`（稳定 X）直接 `contentSessions_.find(X)`，**不做 key 滚动**，故无 `persistentIdToComponentId_.find` 依赖

**残余风险**：若 window_manager 侧时序为「旧 session Destroy → 新 session Foreground」，则 `JsUIExtension::DestroyWindow` 已 erase `contentSessions_[X]`/`uiWindowMap_[X]`，随后 `HandleSessionUpdate` 中 `contentSessions_.find(X)` miss → return，不 fallback 到正常 ForegroundWindow 路径 → 窗口丢失。缓解：依赖 window_manager 保证迁移时旧 session 不 Destroy（跨仓时序约定）；ability_runtime 侧可考虑补 fallback（见风险表）。

### ADR-10: UEC 迁移两种模式（moveTo vs removeChild+addChild）

**背景（2026-07-09 需求方补充）**：UEC 窗口迁移有两种模式，语义不同：

| 模式 | ability 前后台 | session 生命周期 | sessionToken | 应用侧应触发 |
|------|--------------|----------------|------------|------------|
| **moveTo** | 不发生（前台→前台） | 不触发 onSessionCreate/onSessionDestroy | 不变 | 仅 `UpdateExtensionParent(WindowOption)` |
| **removeChild + addChild** | 发生 | 触发 onSessionDestroy + onSessionCreate | 变（新 session） | 走正常 WIN_CMD_DESTROY + WIN_CMD_FOREGROUND（新建）生命周期 |

**关键区分**：moveTo 携带 `originalCallerInfo`（迁移标识），sessionToken 不变；removeChild+addChild 是旧 session 销毁 + 新 session 创建，sessionToken 变化，**不携带 originalCallerInfo**（走正常 destroy+create 路径）。当前设计以 `originalCallerInfo.callerToken != nullptr` 作为 moveTo 标识，removeChild+addChild 天然流经现有 destroy+create 生命周期，二者不混淆。

**待 window_manager 对齐**：(1) removeChild+addChild 确认不携带 originalCallerInfo；(2) moveTo 的 winCmd 确认恒为 WIN_CMD_FOREGROUND。

### ADR-11: 应用侧迁移感知 — 方案 B（流经 ForegroundWindow，不短路）⚠ 2026-07-09 修订

> **⚠ 修订（取代原 ADR-7 的 OnCommandWindow 短路方案）**：原 ADR-7 在 `OnCommandWindow` 检测 `originalCallerInfo` 后短路（`HandleSessionUpdate` + `OnCommandWindowDone` + return，跳过 ForegroundWindow）。经分析此方案有两个问题：(1) `HandleSessionUpdate` 命名误导，听起来像走 session 生命周期（sessionCreate），实际只做 `UpdateExtensionParent`；(2) 短路跳过 ForegroundWindow 的 `Show()`，moveTo 后窗口是否需重新 Show 存疑。

**关键洞察**：修改前"前台到前台"（已存在 session 的 WIN_CMD_FOREGROUND）本就不触发 onSessionCreate——`HandleSessionCreate` 的 else 分支（`js_ui_extension.cpp:676`）对已存在 session 只做 `UpdateExtensionConfig`，**不调 onSessionCreate**。因此"避免 onSessionCreate"无需靠短路实现。

**决策（方案 B）**：移除 `OnCommandWindow` 的迁移短路，让 moveTo 流经正常 WIN_CMD_FOREGROUND → `ForegroundWindow` → `HandleSessionCreate` else 分支（天然不 onSessionCreate + `Show()`）。在 `HandleSessionCreate` else 分支内，检测 `originalCallerInfo.callerToken != nullptr` 时增量调用 `DoUpdateExtensionParent(sessionInfo)`（即 Correction-2 已实现的 `uiWindow_->UpdateExtensionParent` + proxy 遍历更新）。

```
moveTo 流程（方案 B）：
OnCommandWindow(WIN_CMD_FOREGROUND, originalCallerInfo 非空)
  → Extension::OnCommandWindow (空桩)
  → switch(WIN_CMD_FOREGROUND) → ForegroundWindow
      → HandleSessionCreate
          if (window 不存在): [首创路径，moveTo 不会走到]
          else (window 已存在):              ← moveTo 命中此分支
              uiWindow->UpdateExtensionConfig(sharedWant)
              if (originalCallerInfo.callerToken != nullptr):  ← 增量
                  contentSessions_[compId] → napi_unwrap → DoUpdateExtensionParent
                      → uiWindow_->UpdateExtensionParent(WindowOption)
                      → 遍历 jsExtensionWindowRefs_ → UpdateExtensionParent
      → uiWindow->Show()                       ← 不再跳过
      → foregroundWindows_.emplace(compId)
  → OnCommandWindowDone (上报 ABILITY_CMD_FOREGROUND)
```

**移除项**：`HandleSessionUpdate` 虚方法（基类 `ui_extension.h` + 5 变体声明/实现全部移除）——方案 B 不再需要独立迁移分流方法，迁移更新内联进 `HandleSessionCreate` else 分支。仅 `JsUIExtension` 有 `DoUpdateExtensionParent`（content session 持有 uiWindow_ + proxy refs）；base 变体（JsUIExtensionBase/CJ/Ets）无 proxy 跟踪机制，其 `HandleSessionUpdate` 原本即为空桩（no-op），移除短路后流经 ForegroundWindow（UpdateExtensionConfig + Show）反优于原 no-op。

**优点**：路径统一、命名误导消除、Show() 不跳过、复用现有"已存在 session"处理。**代价**：moveTo 多做一次 UpdateExtensionConfig（无害）+ Show（幂等）。

### ADR-12: AMS 自主检测迁移 + 自主赋值 originalCallerInfo（2026-07-09）⚠ 取代原"窗口侧塞 originalCallerInfo"设计

> **⚠ 2026-07-09 修订（取代 ADR-1/ADR-6 的 IsMigrationUpdateRequest 门禁）**：原设计依赖 window_manager 在 moveTo 时向 sessionInfo 塞 `originalCallerInfo.callerToken`（非空）作为迁移标识，AMS 用 `IsMigrationUpdateRequest` 读取此标识做门禁。经分析，AMS 完全可从**自身状态**推断迁移，无需窗口侧塞 originalCallerInfo。

**关键洞察**：同进程迁移支持**同 Ability**（caller 不变、仅 hostWindowId 变）和**多 Ability**（caller 变、uiExtensionComponentId 变）两种，AMS 均可从自身状态判定，不需要 originalCallerInfo 作为窗口侧信号：

| 场景 | by uiExtensionComponentId | by persistentId | hostWindowId 变更 | 结论 |
|------|--------------------------|----------------|----------------|------|
| 首次创建 | miss | miss | — | 新建 |
| 同 session 再前台（同 caller、同窗口） | hit | hit | 不变 | 正常再前台（不迁移） |
| **同 Ability moveTo**（caller 不变、窗口换） | hit（componentId 不变） | hit | **变** | **迁移**（step1 hostWindowId 子检查） |
| **多 Ability moveTo**（caller 变） | miss（componentId 变） | hit（persistentId 不变） | 变 | **迁移**（step2 persistentId fallback） |
| removeChild+addChild | miss（旧 record 已 destroy 移除） | miss | — | 新建（走正常 destroy+create） |

**决策**：`GetOrCreateExtensionRecord` 匹配顺序改为"uiExtensionComponentId-first → persistentId-fallback"，覆盖同 Ability + 多 Ability 两类同进程迁移：
1. `GetAbilityRecordBySessionInfo`（by uiExtensionComponentId）hit → **同 Ability moveTo 子检查**：若 record 旧 sessionInfo 的 hostWindowId ≠ 当前 sessionInfo 的 hostWindowId（窗口换了、caller 没变）→ 同 Ability moveTo → AMS 自主赋值 originalCallerInfo，return；若 hostWindowId 不变 → 正常再前台，return
2. miss → `GetAbilityRecordByPersistentId`（by persistentId）hit → **多 Ability moveTo**：caller 变 → componentId 变 → step1 miss、persistentId 不变 → step2 hit。AMS 自主赋值 originalCallerInfo（callerToken 取自 record 旧 sessionInfo、persistentId = 当前、uiExtensionComponentId = 稳定 X），return
3. miss → preload / create-new

**originalCallerInfo 的来源转移**：从"窗口侧塞"变为"AMS 从 record 旧 sessionInfo 自主赋值"。provider 侧 `HandleSessionCreate` else 分支检查 `originalCallerInfo.callerToken != nullptr` 的逻辑**不变**——只是 originalCallerInfo 的填充责任从 window_manager 转移到 AMS（provider 不关心谁塞的）。

**移除项**：`IsMigrationUpdateRequest`（原 ADR-6）从 .h/.cpp 彻底移除——迁移检测不再依赖 originalCallerInfo 门禁，改为 persistentId fallback 隐式判定。

**收益**：
1. **消除跨仓依赖**：window_manager 不需在 moveTo 时塞 originalCallerInfo，契约简化为"发新 sessionInfo（新 callerToken + 同 persistentId）即可"。
2. **AMS 单一真相源**：迁移检测 + originalCallerInfo 填充均在 AMS，自洽，鲁棒（窗口侧漏塞不再导致误创新 record 泄漏）。
3. **moveTo / removeChild+addChild 天然区分**：由"旧 record 是否仍在 extensionRecords_"决定（moveTo 不 destroy → 命中迁移；removeChild+add 已 destroy → 不命中 → 走新建）。**消解了 ADR-10 待 window_manager 对齐"removeChild+add 是否携带 originalCallerInfo"的未闭合点**——不再需要对齐。
4. **多次迁移天然支持**：persistentId 始终不变，AMS 每次按 persistentId 命中，旧 callerToken 从 record 当前 sessionInfo 取（即上一次迁移后的 callerToken）。

**ability_base 字段保留**：`SessionInfo::originalCallerInfo`（callerToken + persistentId + uiExtensionComponentId）仍保留——provider 侧仍读它触发 `DoUpdateExtensionParent`；只是填充者从窗口侧变为 AMS。T0a 的 ability_base 结构体 + Marshalling 不变。

**代价**：迁移时多一次 `GetAbilityRecordBySessionInfo`（miss）调用。迁移低频（用户拖窗），可忽略。

---

## 3. 不涉及项确认

| 项 | 说明 |
|----|------|
| 修改 `GetAbilityRecordBySessionInfo` 现有匹配逻辑 | 不涉及。新增并行函数 `GetAbilityRecordByPersistentId`，现有函数逐行不变 |
| 修改 `GetOrCreateExtensionRecordInner` | 不涉及。迁移更新分支命中后直接 return，不进入 Inner 创建逻辑 |
| 修改 `IsPreloadExtensionRecord` | 不涉及。迁移更新分支在前，命中后不进入预加载检查 |
| 修改 `ability_manager_service.cpp::StartUIExtensionAbility` 的 uiExtensionComponentId 生成 | 不涉及。ID 生成逻辑不变，迁移后新 ID 自然生成 |
| 修改 `UIExtension::ForegroundWindow` / `BackgroundWindow` 现有逻辑 | 不涉及。迁移更新在 `OnCommandWindow` 新增前置判断，命中后 return，不进入现有 switch |
| ability_base 的 SessionInfo 结构体新增 originalCallerInfo | **已在本仓实现（T0a）**：`session_info.h` 新增 `OriginalCallerInfo` 结构体 + 成员；`session_info.cpp` 新增 `DoMarshallingEight` + `ReadParcelThree` |
| window_manager 的窗口 session 更新 API | 不涉及（独立 proposal）。本仓负责调用入口和 sessionInfo 传递 |
| arkui 的 ArkUI 组件接口 | 不涉及（独立 proposal） |
| 跨进程 / 分布式迁移 | 不涉及 |
| 新增 IPC 接口 | 不涉及。复用现有 `StartUIExtensionAbility` + `ScheduleCommandAbilityWindow` IPC 入口，仅通过 sessionInfo 携带新字段 |

---

## 4. 修改物清单

### 4.0 跨仓依赖 — ability_base（已在本仓实现）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `ability_base/.../session_info.h` | 代码新增 | 新增 `OriginalCallerInfo` 结构体（callerToken + persistentId）；新增 `originalCallerInfo` 成员；新增 `DoMarshallingEight` / `ReadParcelThree` 声明 |
| `ability_base/.../session_info.cpp` | 代码新增 | `Marshalling()` 末尾调 `DoMarshallingEight`；新增 `DoMarshallingEight`（序列化 callerToken + persistentId）；`Unmarshalling()` 末尾调 `ReadParcelThree`；新增 `ReadParcelThree` |

### 4.1 服务侧 — ExtensionRecordManager（services/abilitymgr/src/extension_record/）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `extension_record_manager.cpp` | 代码新增 | `GetOrCreateExtensionRecord` 函数体最前面新增迁移更新分支（ADR-1）；新增 `GetAbilityRecordByPersistentId` 实现（ADR-2）；新增 `IsMigrationUpdateRequest` 判断函数（ADR-6） |
| `../../include/extension_record/extension_record_manager.h` | 代码新增 | 新增 `GetAbilityRecordByPersistentId` 私有方法声明；新增 `IsMigrationUpdateRequest` 私有方法声明 |

### 4.2 服务侧 — UIExtensionAbilityManager（services/abilitymgr/src/ui_extension/）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `ui_extension_ability_manager.cpp` | 代码新增 | `StartAbilityLocked` 中 `SetSessionInfo` 之后、`uiExtensionMap_` 更新之前，新增迁移场景旧映射清理（ADR-3）：按 persistentId 查找 + `RemoveUIExtWindowDeathRecipient` + erase |

### 4.3 应用侧 — UIExtension 框架（frameworks/native/ability/native/）

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `ui_extension_ability/ui_extension.cpp` | 代码新增 | `OnCommandWindow` 新增迁移检测：if originalCallerInfo.callerToken 非空 && WIN_CMD_FOREGROUND → 调 `HandleSessionUpdate` + `OnCommandWindowDone` + return |
| 对应 `ui_extension.h` 头文件 | 代码新增 | 新增 `virtual void HandleSessionUpdate(want, sessionInfo)` 虚方法声明（默认空实现） |
| `ui_extension_ability/js_ui_extension.cpp` | 代码新增 | 重写 `HandleSessionUpdate`（Rev2）：`compId = GetEffectiveComponentId(sessionInfo)`（稳定 X）→ `contentSessions_.find(compId)` 直接命中（**无 key 滚动**）→ `napi_unwrap` 拿 ContentSession 指针调 `DoUpdateExtensionParent`。**不**操作 `persistentIdToComponentId_`（Rev2 废弃） |
| 对应 `js_ui_extension.h` 头文件 | 代码新增 | 新增 `HandleSessionUpdate` override 声明。**不**新增 `persistentIdToComponentId_`（Rev2 废弃） |
| `ui_extension_base/js_ui_extension_content_session.cpp` | 代码新增 + 修改 | 新增 `DoUpdateExtensionParent(sessionInfo)` 内部方法（更新 sessionInfo_ + 调 `uiWindow_->UpdateExtensionParent(WindowOption)` + 遍历 jsExtensionWindowRefs_ 经 napi_unwrap 调老 proxy 的 C++ `UpdateExtensionParent(WindowOption)`，**无需 napi 暴露**；uiWindow_ 对象复用不变（ADR-8）；**修改** `OnGetUIExtensionHostWindowProxy` / `OnGetUIExtensionWindowProxy`：创建 proxy 后记录 napi_ref；`CreateJsUIExtensionContentSession`（两处）napi_wrap 前存 `env_` |
| 对应 `js_ui_extension_content_session.h` 头文件 | 代码新增 | 新增 `DoUpdateExtensionParent` 方法声明；新增 `std::vector<napi_ref> jsExtensionWindowRefs_` 成员；新增 `napi_env env_` 成员 |

### 4.4 单元测试

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| `test/unittest/extension_record_manager_test/`（若存在）或同等测试目录 | 代码新增 | 新增用例覆盖：IsMigrationUpdateRequest 判断、按 persistentId 定位原 record、迁移更新返回 isLoaded=true、未找到原 record 容错降级 |
| `test/unittest/ui_extension_ability_manager_test/`（若存在）或同等 | 代码新增 | 新增用例覆盖：迁移场景 uiExtensionMap_ 按 persistentId 清理旧映射 |
| `test/unittest/ui_extension_test/`（若存在）或同等 | 代码新增 | 新增用例覆盖：OnCommandWindow 迁移感知、HandleSessionUpdate 窗口查找、UpdateWindowMapKey |

### 4.5 构建配置

| 文件 | 类型 | 修改内容 |
|------|------|----------|
| 相关 `BUILD.gn` / `abilitymgr.gni` | 代码新增 | 若新增独立测试目录，更新源文件列表 |

---

## 5. 架构图

```
              ┌─────────────────────────────────────────────────┐
              │  应用发起方 (window_manager / arkui)              │
              │  发起 UEC 窗口迁移请求                             │
              │  sessionInfo = {                                  │
              │    callerToken: 新窗口B,    ← 变了                │
              │    persistentId: P,        ← 不变                 │
              │    originalCallerInfo: { callerToken: 旧窗口A,   │ ← 迁移标识
              │                         persistentId: P }        │
              │  }                                                │
              └──────────────────────┬────────────────────────────┘
                                     ↓ StartUIExtensionAbility (IPC)
    ════════════════════════════════════════════════════════════════
    ║  AMS 服务侧 (services/abilitymgr)                           ║
    ╚══════════════════════════════════════════════════════════════╝
              ┌─────────────────────────────────────────────────┐
              │  AbilityManagerService::StartUIExtensionAbility  │
              │  生成 uiExtensionComponentId = (newRecordId<<32)|P│
              └──────────────────────┬────────────────────────────┘
                                     ↓
              ┌─────────────────────────────────────────────────┐
              │  UIExtensionAbilityManager::StartAbilityLocked    │
              │                                                  │
              │  GetOrCreateExtensionRecord(abilityRequest)       │
              │    ↓                                             │
              │  ┌──────────────────────────────────────────┐   │
              │  │ ExtensionRecordManager::                  │   │
              │  │   GetOrCreateExtensionRecord              │   │
              │  │                                          │   │
              │  │ ┌─[新增分支0]─────────────────────────┐  │   │
              │  │ │ IsMigrationUpdateRequest?            │  │   │
              │  │ │  (originalCallerInfo.callerToken≠空) │  │   │
              │  │ │  ↓ Yes                               │  │   │
              │  │ │ GetAbilityRecordByPersistentId(P)    │  │   │
              │  │ │  ↓ Found 原 record                  │  │   │
              │  │ │ isLoaded=true, return               │  │   │
              │  │ └──────────────────────────────────────┘  │   │
              │  │                                          │   │
              │  │ [现有] GetAbilityRecordBySessionInfo      │   │
              │  │ [现有] IsPreloadExtensionRecord           │   │
              │  │ [现有] GetOrCreateExtensionRecordInner    │   │
              │  └──────────────────────────────────────────┘   │
              │                                          │       │
              │  SetSessionInfo(新 sessionInfo)           │ ← callerToken 更新为 B
              │  [新增] 清理 uiExtensionMap_ 旧映射       │   (按 persistentId P 查找)
              │  uiExtensionMap_[新sessionToken] = ...    │       │
              │                                          │       │
              │  DoForegroundUIExtension(原 record, 新 req)       │
              │    → CommandAbilityWindow(WIN_CMD_FOREGROUND)     │
              └──────────────────────┬────────────────────────────┘
                                     ↓ ScheduleCommandAbilityWindow (IPC)
    ════════════════════════════════════════════════════════════════
    ║  应用侧 — UIExtension 提供方进程 (frameworks/native)        ║
    ╚══════════════════════════════════════════════════════════════╝
              ┌─────────────────────────────────────────────────┐
              │  AbilitySchedulerStub → ExtensionAbilityThread   │
              │  → ExtensionImpl → UIExtension::OnCommandWindow  │
              │                                                  │
              │  ┌─[新增] 迁移检测 ─────────────────────────┐   │
              │  │ originalCallerInfo.callerToken != nullptr?│   │
              │  │  ↓ Yes                                   │   │
              │  │ HandleSessionUpdate(want, sessionInfo)   │   │
              │  │   ← 虚方法，JsUIExtension 重写            │   │
              │  │ return                                  │   │
              │  └──────────────────────────────────────────┘   │
              │                      ↓ (JsUIExtension 重写)      │
              │  ┌──────────────────────────────────────────┐   │
              │  │ JsUIExtension::HandleSessionUpdate       │   │
              │  │   (Rev2: 无 key 滚动)                   │   │
              │  │  1. compId=GetEffectiveComponentId (X)   │   │
              │  │  2. contentSessions_.find(X) 直接命中    │   │
              │  │     (uiWindowMap_ 同以 X 为 key,         │   │
              │  │      对象复用不变 ADR-8)                 │   │
              │  │  3. napi_unwrap → DoUpdateExtensionParent│   │
              │  └──────────────────┬───────────────────────┘   │
              │                     ↓                           │
              │  ┌──────────────────────────────────────────┐   │
              │  │ ContentSession::DoUpdateExtensionParent  │   │
              │  │  a. 更新 sessionInfo_                    │   │
              │  │     (uiWindow_ 对象复用不变 ADR-8)       │   │
              │  │  b. uiWindow_+老proxy 直接调 C++         │   │
              │  │     UpdateExtensionParent(WindowOption)  │   │
              │  │     (无需 napi;新proxy等下次 get*Proxy)   │   │
              │  └──────────────────────────────────────────┘   │
              │                                                  │
              │  [现有] 非 migration: ForegroundWindow 等不变    │
              └──────────────────────┬────────────────────────────┘
                                     ↓
              ┌─────────────────────────────────────────────────┐
              │  window 子系统 (Rosen::Window)                   │
              │  窗口 session 绑定更新到新窗口 B                  │
              │  UEC 在窗口 B 呈现                               │
              └─────────────────────────────────────────────────┘
```

---

## 6. 时序图（迁移场景：UEC 从窗口 A 迁移到窗口 B，persistentId P 不变）

```
应用发起方          AMS 服务侧              应用侧 UIExtension提供方
(window_mgr)        (ERM/UIEAM)             (OnCommandWindow)
    │                   │                           │
 发起迁移请求           │                           │
 sessionInfo={         │                           │
  callerToken:B,       │                           │
  persistentId:P,      │ ← P 不变                  │
  originalCallerInfo:{ │                           │
   callerToken:A,      │ ← 迁移标识                │
   persistentId:P}     │                           │
 }                     │                           │
    │──────────────────>│                           │
    │                   │ StartUIExtensionAbility   │
    │                   │ 生成新 uiExtensionComponentId
    │                   │ =(newRecordId<<32)|P      │
    │                   │ StartAbilityLocked        │
    │                   │──┐                        │
    │                   │  │ GetOrCreateExtensionRecord
    │                   │<─┘                        │
    │                   │ IsMigrationUpdateRequest? │
    │                   │  ↓ Yes                    │
    │                   │ GetAbilityRecordBy        │
    │                   │   PersistentId(P)         │
    │                   │  ↓ Found 原 record        │
    │                   │ isLoaded=true             │
    │                   │                           │
    │                   │ SetSessionInfo(新)        │
    │                   │  → callerToken 更新为 B   │
    │                   │                           │
    │                   │ 清理 uiExtensionMap_      │
    │                   │  (按 P 查找旧映射并 erase)│
    │                   │ 建立 uiExtensionMap_      │
    │                   │  [新sessionToken] = record│
    │                   │                           │
    │                   │ DoForegroundUIExtension   │
    │                   │──┐                        │
    │                   │  │ CommandAbilityWindow   │
    │                   │<─┘                        │
    │                   │──────────────────────────>│
    │                   │   ScheduleCommandAbilityWindow
    │                   │   (want, sessionInfo,     │
    │                   │    WIN_CMD_FOREGROUND)    │
    │                   │                           │
    │                   │                    OnCommandWindow
    │                   │                    originalCallerInfo≠空?
    │                   │                     ↓ Yes (迁移检测)
    │                   │                    HandleSessionUpdate()
    │                   │                     ← JsUIExtension 重写
    │                   │                     │
    │                   │                     ├ 1. compId=GetEffectiveComponentId(X)
    │                   │                     ├ 2. contentSessions_.find(X) 直接命中
    │                   │                     │   (无 key 滚动; uiWindow 对象复用不变)
    │                   │                     └ 3. napi_unwrap → DoUpdateExtensionParent()
    │                   │                       │
    │                   │                       ├ a. 更新 sessionInfo_
    │                   │                       │   (uiWindow_ 对象复用不变)
    │                   │                       └ b. uiWindow_ + 遍历 jsExtensionWindowRefs_
    │                   │                           直接调 C++ UpdateExtensionParent(WindowOption)
    │                   │                           (无需 napi；新 proxy 等下次 get*Proxy)
    │                   │                           │
    │                   │                           │ 老 proxy 已更新
    │                   │                           │ UEC 在窗口 B 呈现
    │                   │                           │
    │<──────────────────│                           │
    │ 迁移成功          │                           │
```

---

## 7. 关键风险与缓解

| 风险 | 缓解措施 |
|------|----------|
| ~~`originalCallerInfo` 字段由 ability_base 独立添加，若 ability_base 未同步合入则编译失败~~ | **已解决**：T0a 已在本仓（ability_base）实现 originalCallerInfo + Marshalling |
| ~~window_manager 的 `JsExtensionWindow::updateExtensionParent` napi 方法（T0b）未就绪~~ | **已解决**：Correction-2 已实现（2026-07-09），`DoUpdateExtensionParent` 从 napi 方式重构为 C++ 直调——构造 `sptr<Rosen::WindowOption>` → 调 `uiWindow_->UpdateExtensionParent(option)` + 遍历 `jsExtensionWindowRefs_` 经 `napi_unwrap` 取 `Rosen::JsExtensionWindow*` 直调 C++ `UpdateExtensionParent(option)`。依赖 window_manager PR #19373 的 C++ 接口 `Window::UpdateExtensionParent(WindowOption)` / `JsExtensionWindow::UpdateExtensionParent(WindowOption)`（窗口侧代码开发中，当前供穿刺验证参照） |
| **迁移期间旧 session 先 Destroy 导致 HandleSessionUpdate 找不到旧记录**（ADR-9） | 依赖 window_manager 保证迁移时旧 session 不 Destroy（跨仓时序约定）；Rev2 下 `HandleSessionUpdate` 用 `GetEffectiveComponentId`（稳定 X）→ `contentSessions_.find(X)`，若旧 session 已被 `JsUIExtension::DestroyWindow` erase 则 miss → 记 warning + return；**已知问题**：当前 return 后不 fallback 到正常 ForegroundWindow 路径，窗口可能丢失——需后续补 fallback |
| persistentId 在迁移时实际发生变化（与需求方确认矛盾） | 需求已确认 persistentId 不变；若实测发现变化，ADR-2 匹配策略需回退为 originalCallerInfo.callerToken + persistentId 组合匹配 |
| 并发迁移同一 UEC 到多个窗口导致 record sessionInfo 竞态 | 应用层保证不并发迁移（ADR-5）；本仓不加额外防并发；review 时验证 |
| 原 record 已销毁（如提供方进程已死亡）导致匹配失败 | 服务侧容错降级到现有逻辑（创建新 record），记 TAG_LOGW 日志；不 crash |
| `GetAbilityRecordByPersistentId` 遍历性能（extensionRecords_ 较大时） | 迁移是低频操作（用户拖拽窗口），遍历开销可接受；如后续有性能要求可增加 persistentId → record 的索引 map |
| 应用侧 `uiWindowMap_` key 更新期间窗口操作并发 | `uiWindowMutex_` 保护 map 操作；key 更新在锁范围内原子完成（ADR-7） |
| `jsExtensionWindowRefs_` napi_ref 泄漏（析构时未 delete_reference） | 已知 minor 泄漏：`Finalizer` 中 env 可能失效，无法安全调 `napi_delete_reference`；proxy 对象本身由应用持有，GC 会回收；后续可考虑 weak ref 机制 |
| **两条 parent 变更通知通路时序交错**（待窗口侧确认） | window_manager PR #19373 引入 `NotifyHostOnParentChanged`（`Businesscode::NOTIFY_HOST_ON_PARENT_CHANGED`，host 侧反向感知）与本特性 `ScheduleCommandAbilityWindow(WIN_CMD_FOREGROUND)`（提供方进程感知）是**两条平行通路**；若二者交错可能产生竞态（如 host 已切 parent 而提供方 sessionInfo_ 尚未更新，或反向）。**待 window_manager 侧确认二者时序保证与 ability_runtime 是否需额外同步**；暂定不额外同步，review 时验证 |
