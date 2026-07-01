# 任务规格：嵌入式元服务隐私弹框改为模应用体验

## T1: Want 常量定义

### 目标
新增本次需求的 Want 参数常量，供 AMS 与应用进程协议使用。

### 修改文件
| 文件 | 修改 |
|------|------|
| `services/abilitymgr/src/disposed_observer.cpp` | 在匿名命名空间新增常量（与现有 `IS_EMBEDDABLE_SERVICE` 并列） |

### 实现要点

在 `disposed_observer.cpp` 文件顶部匿名命名空间内，现有常量旁新增：

```cpp
constexpr const char* EMBEDDABLE_PRIVACY_MODAL_FLAG = "ohos.want.param.embeddablePrivacyModal";
```

`EMBEDDABLE_PRIVACY_EXIT_KEY`（退出 key）定义在 T4 的 `embeddable_privacy_modal_callback.cpp` 内部（仅 Callback 使用，不需要跨文件共享）。

`IS_EMBEDDABLE_SERVICE`（`ohos.param.isCallerEmbeddableUIExtension`）已存在于 `disposed_observer.cpp:29`，本次**复用现有常量**，不重新定义。

### 验收
- 编译通过
- 不修改任何现有代码

---

## T2: Inner API — IAbilityScheduler + Proxy + Stub

### 目标
新增独立的 IPC 接口 `CreateEmbeddablePrivacyUIExtension`，不复用 `CreateModalUIExtension`。

### 修改文件
| 文件 | 修改 |
|------|------|
| `interfaces/inner_api/ability_manager/include/ability_scheduler_interface.h` | 新增虚方法 + 新增 enum 项 |
| `services/abilitymgr/include/ability_scheduler_proxy.h` | 新增方法声明 |
| `services/abilitymgr/src/ability_scheduler_proxy.cpp` | 新增 Proxy 实现（借鉴 `CreateModalUIExtension` 行 1171-1193） |
| `services/abilitymgr/include/ability_scheduler_stub.h` | 新增方法声明 |
| `services/abilitymgr/src/ability_scheduler_stub.cpp` | 新增 switch case + Inner 实现 |

### 实现要点

#### 2.1 ability_scheduler_interface.h

**虚方法**（在现有 `CreateModalUIExtension` 旁，行 295 后）：
```cpp
virtual int CreateEmbeddablePrivacyUIExtension(const Want &want) = 0;
```

**IPC code**（enum 末尾，行 432 `SCHEDULE_ABILITIES_REQUEST_DONE,` 后追加）：
```cpp
SCHEDULE_ABILITIES_REQUEST_DONE,

CREATE_EMBEDDABLE_PRIVACY_UI_EXTENSION,
```

#### 2.2 ability_scheduler_proxy.h / .cpp

声明（行 324 旁）：
```cpp
int32_t CreateEmbeddablePrivacyUIExtension(const Want &want) override;
```

实现（借鉴 `CreateModalUIExtension`，使用新 IPC code）：
```cpp
int32_t AbilitySchedulerProxy::CreateEmbeddablePrivacyUIExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilitySchedulerProxy::CreateEmbeddablePrivacyUIExtension start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface fail");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write wants fail");
        return INNER_ERR;
    }
    int32_t err = SendTransactCmd(IAbilityScheduler::CREATE_EMBEDDABLE_PRIVACY_UI_EXTENSION, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return err;
    }
    return reply.ReadInt32();
}
```

#### 2.3 ability_scheduler_stub.h / .cpp

声明（行 73 旁）：
```cpp
int CreateEmbeddablePrivacyUIExtensionInner(MessageParcel &data, MessageParcel &reply);
```

`OnRemoteRequestInnerThird` switch case 末尾（行 165-166 之间，`SCHEDULE_ABILITIES_REQUEST_DONE` case 之后）追加：
```cpp
case CREATE_EMBEDDABLE_PRIVACY_UI_EXTENSION:
    return CreateEmbeddablePrivacyUIExtensionInner(data, reply);
```

Inner 实现（借鉴 `CreateModalUIExtensionInner`，行 741-754）：
```cpp
int AbilitySchedulerStub::CreateEmbeddablePrivacyUIExtensionInner(MessageParcel &data, MessageParcel &reply)
{
    std::shared_ptr<Want> want(data.ReadParcelable<Want>());
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null want");
        return ERR_INVALID_VALUE;
    }
    int ret = CreateEmbeddablePrivacyUIExtension(*want);
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 ret");
        return ERR_INVALID_VALUE;
    }
    return NO_ERROR;
}
```

### 验收
- IPC code 追加在 enum 末尾，不插入中间
- Proxy 序列化与现有 `CreateModalUIExtension` 一致
- Stub switch case 在末尾追加
- AC3 验证点：新 IPC code 独立

---

## T3: AMS 入口 — AbilityRecord + DisposedObserver

### 目标
新增 AMS 侧入口：DisposedObserver 识别嵌入式场景并触发新链路；AbilityRecord 调用新 IPC。

### 修改文件
| 文件 | 修改 |
|------|------|
| `services/abilitymgr/include/ability_record.h` | 新增 `CreateEmbeddablePrivacyUIExtension` 声明 |
| `services/abilitymgr/src/ability_record.cpp` | 新增方法实现 |
| `services/abilitymgr/include/disposed_observer.h` | 新增私有方法 `ExecuteEmbeddablePrivacyUIExtension` 声明 |
| `services/abilitymgr/src/disposed_observer.cpp` | 新增私有方法实现 + 在 `ExecuteUIExtension` 开头新增分支0 |

### 实现要点

#### 3.1 ability_record.h / .cpp

声明（行 657 旁）：
```cpp
int32_t CreateEmbeddablePrivacyUIExtension(const Want &want);
```

实现（在 `CreateModalUIExtension` 旁，借鉴结构）：
```cpp
int32_t AbilityRecord::CreateEmbeddablePrivacyUIExtension(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CreateEmbeddablePrivacyUIExtension call");
    CHECK_POINTER_AND_RETURN(scheduler_, INNER_ERR);
    return scheduler_->CreateEmbeddablePrivacyUIExtension(want);
}
```

#### 3.2 disposed_observer.h

在 `private` 区（行 48 旁）新增：
```cpp
ErrCode ExecuteEmbeddablePrivacyUIExtension();
```

#### 3.3 disposed_observer.cpp

**新增分支0**（在 `ExecuteUIExtension` 函数开头，行 161 `auto abilityRecord = ...` 之后，行 164 `bool isEmbeddable = false;` 之前）：

```cpp
ErrCode DisposedObserver::ExecuteUIExtension(const AppExecFwk::PageStateData &pageStateData)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token_);
    Want want = *disposedRule_.want;

    // ===== 新增分支0：嵌入式隐私弹框模应用独立路径（本次需求） =====
    bool isPrivacyModalEmbeddable = false;
    if (abilityRecord != nullptr) {
        const auto& abilityWant = abilityRecord->GetWant();
        if (abilityWant.HasParameter(AAFwk::SCREEN_MODE_KEY)) {
            int32_t screenMode = abilityWant.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
            isPrivacyModalEmbeddable = IsEmbeddableStart(screenMode);
        }
    }
    if (isPrivacyModalEmbeddable) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Handle embeddable privacy modal (new path)");
        ErrCode ret = ExecuteEmbeddablePrivacyUIExtension();
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "ExecuteEmbeddablePrivacyUIExtension failed");
        }
        return ret;
    }
    // ===== 新增分支0 结束 =====

    // 现有三条分支代码逐行不变 ↓
    bool isEmbeddable = false;
    // ... [现有代码保持不变] ...
}
```

**新增私有方法**（在 `ExecuteUIExtension` 旁）：
```cpp
ErrCode DisposedObserver::ExecuteEmbeddablePrivacyUIExtension()
{
    auto abilityRecord = Token::GetAbilityRecordByToken(token_);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord");
        return ERR_INVALID_VALUE;
    }
    Want want = *disposedRule_.want;
    want.SetParam(IS_EMBEDDABLE_SERVICE, true);
    want.SetParam(EMBEDDABLE_PRIVACY_MODAL_FLAG, true);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Trigger embeddable privacy modal (new independent path)");
    return abilityRecord->CreateEmbeddablePrivacyUIExtension(want);
}
```

### 验收
- 现有 `ExecuteUIExtension` 三条分支代码逐行不变（`git diff` 验证）
- 新增分支0 在 `bool isEmbeddable = false;` 之前
- AC1、AC2、AC3、AC7 验证点

---

## T4: 新类 — EmbeddablePrivacyModalCallback

### 目标
新增独立的 ModalCallback 类，处理新链路的回调（含新退出 key）。

### 修改文件
| 文件 | 类型 |
|------|------|
| `interfaces/kits/native/ability/native/ui_extension_base/embeddable_privacy_modal_callback.h` | 新建 |
| `frameworks/native/ability/native/ui_extension_base/embeddable_privacy_modal_callback.cpp` | 新建 |

### 实现要点

#### 4.1 embeddable_privacy_modal_callback.h

借鉴 `ui_extension_modal_callback.h` 结构，类名改为 `EmbeddablePrivacyModalCallback`：

```cpp
#ifndef OHOS_ABILITY_RUNTIME_EMBEDDABLE_PRIVACY_MODAL_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_EMBEDDABLE_PRIVACY_MODAL_CALLBACK_H

#include "hilog_tag_wrapper.h"
#include <memory>

#ifdef SUPPORT_SCREEN
#include "ui_content.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AbilityRuntime {

class UIExtensionContext;

class EmbeddablePrivacyModalCallback {
public:
    EmbeddablePrivacyModalCallback() = default;
    ~EmbeddablePrivacyModalCallback() = default;

    void SetSessionId(int32_t sessionId) { sessionId_ = sessionId; }

#ifdef SUPPORT_SCREEN
    void SetUIContent(Ace::UIContent* uiContent) { uiContent_ = uiContent; }
#endif // SUPPORT_SCREEN

    void SetUIExtensionContext(const std::weak_ptr<UIExtensionContext>& context) { contextWeak_ = context; }

    void OnRelease();
    void OnError();
    void OnDestroy();
    void OnReceive(const AAFwk::WantParams& data);

private:
    int32_t sessionId_ = 0;
#ifdef SUPPORT_SCREEN
    Ace::UIContent* uiContent_ = nullptr;
#endif // SUPPORT_SCREEN
    std::weak_ptr<UIExtensionContext> contextWeak_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_EMBEDDABLE_PRIVACY_MODAL_CALLBACK_H
```

#### 4.2 embeddable_privacy_modal_callback.cpp

借鉴 `ui_extension_modal_callback.cpp`，使用**新退出 key** `ohos.param.exitEmbeddablePrivacyUIExtension`：

```cpp
#include "embeddable_privacy_modal_callback.h"
#include "ui_extension_context.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* EMBEDDABLE_PRIVACY_EXIT = "ohos.param.exitEmbeddablePrivacyUIExtension";
}

void EmbeddablePrivacyModalCallback::OnRelease()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "EmbeddablePrivacyModalCallback::OnRelease, sessionId: %{public}d", sessionId_);
    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnRelease");
        return;
    }
    context->EraseUIExtension(sessionId_);
    TAG_LOGI(AAFwkTag::UI_EXT, "Erased embeddable privacy modal on release: %{public}d", sessionId_);
#ifdef SUPPORT_SCREEN
    if (uiContent_ != nullptr) {
        uiContent_->CloseModalUIExtension(sessionId_);
        TAG_LOGI(AAFwkTag::UI_EXT, "Closed embeddable privacy modal: %{public}d", sessionId_);
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiContent_ in OnRelease");
    }
#endif // SUPPORT_SCREEN
}

void EmbeddablePrivacyModalCallback::OnError()
{
    TAG_LOGE(AAFwkTag::UI_EXT, "EmbeddablePrivacyModalCallback::OnError, sessionId: %{public}d", sessionId_);
    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnError");
        return;
    }
    context->EraseUIExtension(sessionId_);
#ifdef SUPPORT_SCREEN
    if (uiContent_ != nullptr) {
        uiContent_->CloseModalUIExtension(sessionId_);
        TAG_LOGI(AAFwkTag::UI_EXT, "Closed embeddable privacy modal on error: %{public}d", sessionId_);
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiContent_ in OnError");
    }
#endif // SUPPORT_SCREEN
}

void EmbeddablePrivacyModalCallback::OnDestroy()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "EmbeddablePrivacyModalCallback::OnDestroy, sessionId: %{public}d", sessionId_);
    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnDestroy");
        return;
    }
    context->EraseUIExtension(sessionId_);
    TAG_LOGI(AAFwkTag::UI_EXT, "Erased embeddable privacy modal on destroy: %{public}d", sessionId_);
}

void EmbeddablePrivacyModalCallback::OnReceive(const AAFwk::WantParams& data)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "EmbeddablePrivacyModalCallback::OnReceive, sessionId: %{public}d", sessionId_);
    auto context = contextWeak_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Context already destroyed in OnReceive");
        return;
    }
    if (data.HasParam(EMBEDDABLE_PRIVACY_EXIT)) {
        bool shouldExit = data.GetIntParam(EMBEDDABLE_PRIVACY_EXIT, 0);
        if (shouldExit == 1) {
            TAG_LOGI(AAFwkTag::UI_EXT, "Embeddable privacy modal notified exit, sessionId: %{public}d", sessionId_);
            context->TerminateSelfWithAnimation(nullptr);
            return;
        }
    }
}

} // namespace AbilityRuntime
} // namespace OHOS
```

### 验收
- 类名 `EmbeddablePrivacyModalCallback` 与现有 `UIExtensionModalCallback` 明确区分
- 退出 key `ohos.param.exitEmbeddablePrivacyUIExtension` 与现有 `ohos.param.exitEmbeddableUIExtension` 区分
- AC6、AC14 验证点

---

## T5: 进程内 C++ — UIExtension + UIExtensionContext

### 目标
新增应用进程内 C++ 方法，接收 IPC 调用并创建模应用。

### 修改文件
| 文件 | 修改 |
|------|------|
| `interfaces/kits/native/ability/native/ui_extension_ability/ui_extension.h` | 新增虚方法声明 |
| `frameworks/native/ability/native/ui_extension_ability/ui_extension.cpp` | 新增方法实现 |
| `interfaces/kits/native/ability/native/ui_extension_base/ui_extension_context.h` | 新增 `CreateEmbeddablePrivacyUIExtensionWithApp` 声明 + `SetupEmbeddablePrivacyModalCallbacks` 声明 |
| `frameworks/native/ability/native/ui_extension_base/ui_extension_context.cpp` | 新增方法实现 |

### 实现要点

#### 5.1 ui_extension.h

在 `CreateModalUIExtension` 声明旁（行 100）新增：
```cpp
/**
 * @brief Create embeddable privacy modal UIExtension (new independent path).
 * @param want The want of the embeddable privacy modal UIExtension to create.
 * @return Returns ERR_OK on success, error code on failure.
 */
virtual int CreateEmbeddablePrivacyUIExtension(const AAFwk::Want &want);
```

#### 5.2 ui_extension.cpp

借鉴 `CreateModalUIExtension`（行 413-447），调用新 Context 方法：

```cpp
int UIExtension::CreateEmbeddablePrivacyUIExtension(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension::CreateEmbeddablePrivacyUIExtension call");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is null");
        return ERR_INVALID_VALUE;
    }
    if (handler_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null handler_");
        return ERR_INVALID_VALUE;
    }
    auto uiExtensionContext = Context::ConvertTo<UIExtensionContext>(context);
    if (uiExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "uiExtensionContext is null");
        return ERR_INVALID_VALUE;
    }
    std::weak_ptr<UIExtensionContext> uiExtensionContextWptr = uiExtensionContext;
    auto task = [uiExtensionContextWptr, want]() {
        std::shared_ptr<UIExtensionContext> uiExtensionContextSptr = uiExtensionContextWptr.lock();
        if (uiExtensionContextSptr == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiExtensionContext in task");
            return;
        }
        (void)uiExtensionContextSptr->CreateEmbeddablePrivacyUIExtensionWithApp(want);
    };
    if (!handler_->PostTask(task, "UIExtension:CreateEmbeddablePrivacyUIExtensionWithApp")) {
        TAG_LOGE(AAFwkTag::UI_EXT, "PostTask failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}
```

#### 5.3 ui_extension_context.h

在 `CreateModalUIExtensionWithApp` 声明旁（行 267）新增：
```cpp
/**
 * @brief Create embeddable privacy modal UIExtension with app (new independent path).
 * @param want The want of the embeddable privacy modal UIExtension to create.
 * @return Returns ERR_OK on success, error code on failure.
 */
ErrCode CreateEmbeddablePrivacyUIExtensionWithApp(const AAFwk::Want &want);
```

在 private 区（`SetupModalCallbacks` 旁）新增：
```cpp
Ace::ModalUIExtensionCallbacks SetupEmbeddablePrivacyModalCallbacks(
    std::shared_ptr<EmbeddablePrivacyModalCallback> modalCallback);
```

同时在 include 区新增：
```cpp
#include "embeddable_privacy_modal_callback.h"
```

#### 5.4 ui_extension_context.cpp

**实现 `CreateEmbeddablePrivacyUIExtensionWithApp`**（借鉴 `CreateModalUIExtensionWithApp`，行 984-1039，实例化 `EmbeddablePrivacyModalCallback`）：

```cpp
ErrCode UIExtensionContext::CreateEmbeddablePrivacyUIExtensionWithApp(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtensionContext::CreateEmbeddablePrivacyUIExtensionWithApp call");
    auto uiContent = GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiContent");
        return ERR_INVALID_VALUE;
    }
    if (IsUIExtensionExist(want) && !want.GetBoolParam(USE_GLOBAL_UICONTENT, false)) {
        TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension already exists");
        return ERR_OK;
    }
    auto modalCallback = std::make_shared<EmbeddablePrivacyModalCallback>();
    Ace::ModalUIExtensionCallbacks callback = SetupEmbeddablePrivacyModalCallbacks(modalCallback);
    Ace::ModalUIExtensionConfig config;
    config.prohibitedRemoveByRouter = true;
    if (want.GetBoolParam(DISPOSED_PROHIBIT_BACK, false)) {
        config.isProhibitBack = true;
    }
    if (want.HasParameter(IS_WINDOWMODE_FOLLOWHOST)) {
        config.isWindowModeFollowHost = want.GetBoolParam(IS_WINDOWMODE_FOLLOWHOST, false);
    }
    int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "CreateEmbeddablePrivacyModalUIExtension failed");
        return ERR_INVALID_VALUE;
    }
    modalCallback->SetSessionId(sessionId);
    modalCallback->SetUIContent(uiContent);
    auto contextWeak = std::static_pointer_cast<UIExtensionContext>(shared_from_this());
    modalCallback->SetUIExtensionContext(contextWeak);
    {
        std::lock_guard<std::mutex> lock(uiExtensionMutex_);
        uiExtensionMap_.emplace(sessionId, want);
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "Created embeddable privacy modal UIExtension, sessionId: %{public}d", sessionId);
    return ERR_OK;
}

Ace::ModalUIExtensionCallbacks UIExtensionContext::SetupEmbeddablePrivacyModalCallbacks(
    std::shared_ptr<EmbeddablePrivacyModalCallback> modalCallback)
{
    Ace::ModalUIExtensionCallbacks callback;
    callback.onError = [modalCallback](int32_t code, const std::string &str1, const std::string &str2) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Embeddable privacy modal error: %{public}d, msg: %{public}s", code, str2.c_str());
        modalCallback->OnError();
    };
    callback.onRelease = [modalCallback](int32_t code) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Embeddable privacy modal release: %{public}d", code);
        modalCallback->OnRelease();
    };
    callback.onResult = [modalCallback](int32_t code, const AAFwk::Want &resultWant) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Embeddable privacy modal result: %{public}d", code);
    };
    callback.onDestroy = [modalCallback]() {
        TAG_LOGD(AAFwkTag::UI_EXT, "Embeddable privacy modal destroy");
        modalCallback->OnDestroy();
    };
    callback.onReceive = [modalCallback](const AAFwk::WantParams& data) {
        TAG_LOGD(AAFwkTag::UI_EXT, "Embeddable privacy modal onReceive");
        modalCallback->OnReceive(data);
    };
    return callback;
}
```

### 验收
- 现有 `UIExtension::CreateModalUIExtension` / `CreateModalUIExtensionWithApp` / `SetupModalCallbacks` 逐行不变
- 新方法命名带 `EmbeddablePrivacy` 前缀
- AC4、AC5 验证点

---

## T6: BUILD.gn 更新

### 目标
将新建的 `embeddable_privacy_modal_callback.cpp` 加入构建。

### 修改文件
| 文件 | 修改 |
|------|------|
| `frameworks/native/ability/native/BUILD.gn` | sources 列表新增一行 |

### 实现要点

在 `frameworks/native/ability/native/BUILD.gn:2506`（现有 `ui_extension_modal_callback.cpp` 行）之后追加：
```gn
"${ability_runtime_native_path}/ability/native/ui_extension_base/embeddable_privacy_modal_callback.cpp",
```

### 验收
- 编译通过
- 新文件被正确包含在构建产物中

---

## T7: 单元测试

### 目标
覆盖新链路的关键场景，验证行为正确性。

### 修改文件
| 文件 | 类型 |
|------|------|
| `test/unittest/disposed_observer_test/disposed_observer_test.cpp` | 扩展（新增用例） |
| `test/unittest/embeddable_privacy_modal_callback_test/embeddable_privacy_modal_callback_test.cpp` | 新建 |
| `test/unittest/embeddable_privacy_modal_callback_test/BUILD.gn` | 新建 |

### 实现要点

#### 7.1 disposed_observer_test 扩展

新增至少 3 个用例：
- `DisposedObserver_OnPageShow_EmbeddablePrivacy_0100`：嵌入式场景触发新分支0
- `DisposedObserver_OnPageShow_EmbeddablePrivacy_0200`：非嵌入式场景不触发新分支0
- `DisposedObserver_ExecuteEmbeddablePrivacyUIExtension_0100`：Want 参数正确传递

#### 7.2 embeddable_privacy_modal_callback_test 新建

参考 `test/unittest/ui_extension_modal_callback_test/` 结构，覆盖：
- `OnReceive_0100`：退出 key=1 触发 TerminateSelfWithAnimation
- `OnReceive_0200`：退出 key=0 不触发
- `OnReceive_0300`：无退出 key 不触发
- `OnRelease_0100`：EraseUIExtension + CloseModalUIExtension 调用
- `OnError_0100`：同 OnRelease
- `OnDestroy_0100`：仅 EraseUIExtension
- `OnReceive_ContextDestroyed_0100`：weak_ptr lock 失败不 crash

### 验收
- 新增测试用例全部通过
- AC11、AC12（部分）验证点

---

## T8: 验证

### 目标
全量编译 + 回归测试 + 新增测试 + AC 对照。

### 执行步骤

```bash
# 1. 全量编译
./build.sh --product-name <product> --build-target ability_runtime

# 2. 现有测试回归（AC2、AC8、R1-R4）
run -t UT -ts disposed_observer_test
run -t UT -ts ui_extension_modal_callback_test
run -t UT -ts ability_manager_service_first_test

# 3. 新增测试（AC11）
run -t UT -ts embeddable_privacy_modal_callback_test

# 4. git diff 检查现有代码无修改
git diff services/abilitymgr/src/disposed_observer.cpp  # 仅新增，无现有行修改
git diff frameworks/native/ability/native/ui_extension_base/ui_extension_context.cpp  # 仅新增，无现有行修改
```

### AC 对照检查清单

| AC | 验证方法 |
|----|---------|
| AC1 | 单元测试 T7.1 + 日志 "Handle embeddable privacy modal (new path)" |
| AC2 | `git diff` + 现有测试回归全通过 |
| AC3 | 单元测试 T7.1 验证 IPC code + Want 参数 |
| AC4 | 单元测试 T7.1 + 进程内方法签名检查 |
| AC5 | 手动集成测试（I1） |
| AC6 | 单元测试 T7.2 OnReceive_0100 |
| AC7 | 单元测试 T7.1 Want 参数检查 |
| AC8 | 现有测试回归 |
| AC9 | 代码审查（Proxy 错误返回路径） |
| AC10 | 单元测试 T7.2（如有 Context 测试） |
| AC11 | 新增单元测试全通过 |
| AC12 | 代码审查（TAG_LOG 关键节点） |
| AC13 | 同 AC12 |
| AC14 | 命名审查（`EmbeddablePrivacy` 前缀） |

### 验收
- 所有 AC 通过
- 现有测试无回归
- `git diff` 验证现有函数体逐行不变
