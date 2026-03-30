/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ui_extension_context.h"

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "bool_wrapper.h"
#include "configuration.h"
#include "configuration_convertor.h"
#include "connection_manager.h"
#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "string_wrapper.h"
#include "ui_content.h"

namespace OHOS {
namespace AbilityRuntime {
const size_t UIExtensionContext::CONTEXT_TYPE_ID(std::hash<const char*> {} ("UIExtensionContext"));
int UIExtensionContext::ILLEGAL_REQUEST_CODE(-1);
constexpr const char* REQUEST_COMPONENT_TERMINATE_KEY = "ohos.param.key.requestComponentTerminate";
constexpr const char* UIEXTENSION_TARGET_TYPE_KEY = "ability.want.params.uiExtensionTargetType";
constexpr const char* FLAG_AUTH_READ_URI_PERMISSION = "ability.want.params.uriPermissionFlag";
constexpr int32_t TERMINATE_SELF_ANIMATION_TIMEOUT_MS = 2000;

namespace {
bool IsEmbeddableStart(int32_t screenMode)
{
    return screenMode == AAFwk::EMBEDDED_FULL_SCREEN_MODE ||
        screenMode == AAFwk::EMBEDDED_HALF_SCREEN_MODE;
}
}

ErrCode UIExtensionContext::StartAbility(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "begin, ability:%{public}s", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::StartAbility(const AAFwk::Want &want, int requestCode) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "begin, requestCode:%{public}d", requestCode);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::StartAbility(const AAFwk::Want &want, const AAFwk::StartOptions &startOptions) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "begin, ability:%{public}s", want.GetElement().GetAbilityName().c_str());
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_,
        ILLEGAL_REQUEST_CODE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::StartUIAbilitiesInSplitWindowMode(int32_t primaryWindowId,
    const AAFwk::Want &secondaryWant) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartUIAbilitiesInSplitWindowMode(
        primaryWindowId, secondaryWant, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
    }
    return err;
}

ErrCode UIExtensionContext::StartUIServiceExtension(const AAFwk::Want& want, int32_t accountId) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "Start UIServiceExtension begin");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::UI_SERVICE);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
    }
    return err;
}

bool UIExtensionContext::CheckAndSetPendingTerminate()
{
    std::lock_guard<std::mutex> lock(terminateSelfMutex_);
    if (pendingAnimationTerminate_) {
        TAG_LOGW(AAFwkTag::UI_EXT, "Already pending animation terminate, skip");
        return false;
    }
    pendingAnimationTerminate_ = true;
    return true;
}

bool UIExtensionContext::TryGetAnimationCallback(TerminateSelfWithAnimationCallback &callback)
{
    std::lock_guard<std::mutex> lock(terminateSelfMutex_);
    if (terminateSelfWithAnimationCallback_ != nullptr) {
        std::swap(callback, terminateSelfWithAnimationCallback_);
        return true;
    }
    return false;
}

bool UIExtensionContext::TryGetAnimationCallbackWithId(TerminateSelfWithAnimationCallbackWithId &callback)
{
    std::lock_guard<std::mutex> lock(terminateSelfMutex_);
    if (terminateSelfWithAnimationCallbackWithId_ != nullptr) {
        std::swap(callback, terminateSelfWithAnimationCallbackWithId_);
        return true;
    }
    return false;
}

ErrCode UIExtensionContext::GetOrCreateEventHandler(std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    std::lock_guard<std::mutex> lock(terminateSelfMutex_);
    if (eventHandler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create("UIExtensionTerminateRunner");
        if (runner == nullptr) {
            TAG_LOGW(AAFwkTag::UI_EXT, "Failed to create EventRunner, fallback without timeout");
            return ERR_INVALID_VALUE;
        }
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
        TAG_LOGI(AAFwkTag::UI_EXT, "Created independent EventRunner for timeout task");
    }
    handler = eventHandler_;
    return ERR_OK;
}

void UIExtensionContext::ExecuteTerminationWithTimeout(const sptr<IRemoteObject> &token, TerminateRequestId requestId)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "Animation timeout, execute termination logic, requestId=%{public}d", requestId);

    // ✅ Find request data from map and check if already terminated
    std::lock_guard<std::mutex> lock(terminateRequestMutex_);
    auto it = terminateRequests_.find(requestId);
    if (it == terminateRequests_.end()) {
        TAG_LOGW(AAFwkTag::UI_EXT, "Request not found for requestId=%{public}d, already completed", requestId);
        return;
    }

    TerminateRequestInfo &info = it->second;

    // ✅ Check if this specific request is already terminated
    if (info.isTerminated) {
        TAG_LOGI(AAFwkTag::UI_EXT, "Request %{public}d already terminated, skip duplicate execution", requestId);
        return;
    }

    // ✅ Mark this request as terminated
    info.isTerminated = true;

    // ✅ Transfer result for TerminateSelfWithResult (if transfer fails, cleanup and return)
    if (!info.isTerminateSelfWithoutResult) {
        ErrCode transferErr = TransferAbilityResultToWindow(info.resultCode, info.want);
        if (transferErr != ERR_OK) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Transfer result failed, err=%{public}d, cleanup and return", transferErr);

            // ✅ Notify error and clean up
            TerminateSelfResultCallback callback = std::move(info.callback);
            terminateRequests_.erase(it);

            // Release lock before calling callback and cleanup
            lock.unlock();

            // ✅ Remove only this request's timeout task
            std::lock_guard<std::mutex> lock2(terminateSelfMutex_);
            if (eventHandler_ != nullptr) {
                eventHandler_->RemoveTask("UIExtensionTerminateTimeout_" + std::to_string(requestId));
                TAG_LOGI(AAFwkTag::UI_EXT, "Timeout task removed for requestId=%{public}d", requestId);
            }
            if (callback) {
                callback(transferErr);
            }
            return;  // ❌ 不调用 TerminateAbility
        }
    }

    // ✅ Transfer succeeded or is TerminateSelf, proceed with termination
    TerminateSelfResultCallback callback = std::move(info.callback);
    terminateRequests_.erase(it);

    // Release lock before executing termination
    lock.unlock();

    // ✅ Execute termination
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token, -1, nullptr);
    if (err == ERR_OK) {
        isTerminated_.store(true);
    } else {
        TAG_LOGE(AAFwkTag::UI_EXT, "Termination failed, err=%{public}d", err);
    }

    // ✅ Notify callback
    if (callback) {
        callback(err);
    }
}

ErrCode UIExtensionContext::HandleTerminateWithAnimation(TerminateRequestId requestId)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "Handle terminate with animation, requestId=%{public}d", requestId);

    // Only check if already terminated (object destroyed case)
    if (isTerminated_.load()) {
        TAG_LOGI(AAFwkTag::UI_EXT, "Already terminated, skip animation");
        return ERR_OK;
    }

    // Get ArkUI callback
    TerminateSelfWithAnimationCallbackWithId callback;
    if (!TryGetAnimationCallbackWithId(callback)) {
        TAG_LOGW(AAFwkTag::UI_EXT, "Callback not registered, fallback to normal terminate");
        return TerminateSelfInner(requestId);
    }

    // ✅ Reuse existing EventHandler (or create on first use)
    std::shared_ptr<AppExecFwk::EventHandler> handler;
    if (GetOrCreateEventHandler(handler) != ERR_OK) {
        TAG_LOGW(AAFwkTag::UI_EXT, "Failed to get/create EventHandler, fallback without timeout");
        callback(requestId);
        return TerminateSelfInner(requestId);
    }

    // Create timeout task with unique name for each requestId
    auto timeoutTask = [weakContext = weak_from_this(), token = token_, requestId]() {
        auto context = std::static_pointer_cast<UIExtensionContext>(weakContext.lock());
        if (context != nullptr) {
            context->ExecuteTerminationWithTimeout(token, requestId);
        }
    };

    if (!handler->PostTask(timeoutTask, "UIExtensionTerminateTimeout_" + std::to_string(requestId),
        TERMINATE_SELF_ANIMATION_TIMEOUT_MS)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to post timeout task, fallback to normal terminate");
        callback(requestId);
        return TerminateSelfInner(requestId);
    }

    // Call ArkUI callback with requestId
    callback(requestId);
    TAG_LOGI(AAFwkTag::UI_EXT, "Callback executed with requestId=%{public}d, waiting for ArkUI", requestId);
    return ERR_OK;
}

ErrCode UIExtensionContext::TerminateSelf()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelf begin");

    if (IsEmbeddableStart(screenMode_)) {
        // Check if already terminated to prevent duplicate termination in embedded mode
        if (isTerminated_.load()) {
            TAG_LOGI(AAFwkTag::UI_EXT, "Already terminated, skip");
            return ERR_OK;
        }

        // Store TerminateSelf request info in map
        {
            std::lock_guard<std::mutex> lock(terminateRequestMutex_);
            TerminateRequestInfo info;
            info.isTerminateSelfWithoutResult = true;
            terminateRequests_[0] = std::move(info);
        }

        ErrCode err = HandleTerminateWithAnimation(0);

        // Clean up on failure
        if (err != ERR_OK) {
            std::lock_guard<std::mutex> lock(terminateRequestMutex_);
            terminateRequests_.erase(0);
        }

        return err;
    }

    return TerminateSelfInner(-1);  // Non-embedded mode: requestId = -1
}

ErrCode UIExtensionContext::TransferAbilityResultToWindow(int32_t resultCode, const AAFwk::Want &want)
{
    auto token = GetToken();
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TransferAbilityResultForExtension(
        token, resultCode, want);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "TransferAbilityResultForExtension failed, err = %{public}d", err);
        return err;
    }

#ifdef SUPPORT_SCREEN
    sptr<Rosen::Window> uiWindow = GetWindow();
    if (!uiWindow) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    auto ret = uiWindow->TransferAbilityResult(resultCode, want);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "TransferAbilityResult to window failed, ret = %{public}d", ret);
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
#endif // SUPPORT_SCREEN

    return ERR_OK;
}

void UIExtensionContext::NotifyPendingCallbackById(TerminateRequestId requestId, ErrCode err)
{
    std::lock_guard<std::mutex> lock(terminateRequestMutex_);
    auto it = terminateRequests_.find(requestId);
    if (it != terminateRequests_.end()) {
        if (it->second.callback) {
            it->second.callback(err);
        }
        terminateRequests_.erase(it);
        TAG_LOGI(AAFwkTag::UI_EXT, "Notified callback for requestId=%{public}d, err=%{public}d", requestId, err);
    } else {
        TAG_LOGW(AAFwkTag::UI_EXT, "Callback not found for requestId=%{public}d", requestId);
    }
}

void UIExtensionContext::CleanupAnimationResources()
{
    std::lock_guard<std::mutex> lock(terminateSelfMutex_);
    if (eventHandler_ != nullptr) {
        // For old interface path (requestId == -1), remove all tasks
        // For new interface (embedded mode), specific request tasks are removed directly in the failure handling
        eventHandler_->RemoveAllTasks();
        TAG_LOGI(AAFwkTag::UI_EXT, "All timeout tasks removed");
    }
    terminateSelfWithAnimationCallback_ = nullptr;
    terminateSelfWithAnimationCallbackWithId_ = nullptr;
}

ErrCode UIExtensionContext::TransferPendingResult(TerminateRequestId requestId)
{
    std::lock_guard<std::mutex> lock(terminateRequestMutex_);
    auto it = terminateRequests_.find(requestId);
    if (it == terminateRequests_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Request not found for requestId=%{public}d", requestId);
        return ERR_INVALID_VALUE;
    }

    TerminateRequestInfo &info = it->second;
    return TransferAbilityResultToWindow(info.resultCode, info.want);
}

bool UIExtensionContext::HasTerminateRequest(TerminateRequestId requestId)
{
    std::lock_guard<std::mutex> lock(terminateRequestMutex_);
    return terminateRequests_.find(requestId) != terminateRequests_.end();
}

void UIExtensionContext::TerminateSelfWithResult(int32_t resultCode, const AAFwk::Want &want,
    TerminateSelfResultCallback callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfWithResult begin");

    // Non-embedded mode: execute synchronously and call callback
    if (!IsEmbeddableStart(screenMode_)) {
        TAG_LOGI(AAFwkTag::UI_EXT, "Non-embedded mode: transfer result synchronously");

        ErrCode err = TransferAbilityResultToWindow(resultCode, want);
        if (err != ERR_OK) {
            if (callback) {
                callback(err);
            }
            return;
        }

        // Execute termination and call callback
        SetTerminating(true);
        err = TerminateSelf();

        if (callback) {
            callback(err);
        }
        return;
    }

    // Embedded mode: generate requestId and call new interface
    TAG_LOGI(AAFwkTag::UI_EXT, "Embedded mode: generate requestId and trigger animation");

    // Generate unique requestId
    TerminateRequestId requestId = GenerateTerminateRequestId();

    // Call new interface with requestId
    TerminateSelfWithResultForResult(resultCode, want, requestId, std::move(callback));
}

void UIExtensionContext::TerminateSelfWithResultForResult(int32_t resultCode, const AAFwk::Want &want,
    TerminateRequestId requestId, TerminateSelfResultCallback callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfWithResultForResult begin, requestId=%{public}d", requestId);

    // Store complete request info in map
    {
        std::lock_guard<std::mutex> lock(terminateRequestMutex_);
        TerminateRequestInfo info(resultCode, want, std::move(callback), false);
        terminateRequests_[requestId] = std::move(info);
    }

    SetTerminating(true);

    // Trigger animation
    ErrCode err = HandleTerminateWithAnimation(requestId);

    // If triggering animation fails, fallback to direct termination
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Failed to trigger animation, fallback to direct termination, err=%{public}d", err);
        TerminateSelfInner(requestId);
    }
}

ErrCode UIExtensionContext::TerminateSelfInner(TerminateRequestId requestId)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfInner begin, requestId=%{public}d", requestId);

    if (IsEmbeddableStart(screenMode_)) {
        // Non-embedded mode or requestId is -1: use original logic
        if (requestId == -1) {
            // Old interface call (non-embedded mode)
            CleanupAnimationResources();

            ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);
            if (err == ERR_OK) {
                isTerminated_.store(true);
            }
            // Notify pending callback if any
            NotifyPendingCallbackById(-1, err);
            return err;
        }

        // Embedded mode: use requestId logic

        // ✅ Find request data from map and check if already terminated
        std::lock_guard<std::mutex> lock(terminateRequestMutex_);
        auto it = terminateRequests_.find(requestId);
        if (it == terminateRequests_.end()) {
            TAG_LOGW(AAFwkTag::UI_EXT, "Request not found for requestId=%{public}d, already completed", requestId);
            return ERR_INVALID_VALUE;
        }

        TerminateRequestInfo &info = it->second;

        // ✅ Check if this specific request is already terminated
        if (info.isTerminated) {
            TAG_LOGI(AAFwkTag::UI_EXT, "Request %{public}d already terminated, skip duplicate execution", requestId);
            return ERR_OK;
        }

        // ✅ Mark this request as terminated
        info.isTerminated = true;

        // ✅ Transfer result for TerminateSelfWithResult (if transfer fails, cancel timeout and return)
        if (!info.isTerminateSelfWithoutResult) {
            ErrCode transferErr = TransferAbilityResultToWindow(info.resultCode, info.want);
            if (transferErr != ERR_OK) {
                TAG_LOGE(AAFwkTag::UI_EXT, "Transfer result failed, err=%{public}d, cancel timeout and return", transferErr);

                // ✅ Cancel timeout task and notify callback with error
                TerminateSelfResultCallback callback = std::move(info.callback);
                terminateRequests_.erase(it);

                // Release lock before calling callback and cleanup
                lock.unlock();

                // ✅ Remove only this request's timeout task
                std::lock_guard<std::mutex> lock2(terminateSelfMutex_);
                if (eventHandler_ != nullptr) {
                    eventHandler_->RemoveTask("UIExtensionTerminateTimeout_" + std::to_string(requestId));
                    TAG_LOGI(AAFwkTag::UI_EXT, "Timeout task removed for requestId=%{public}d", requestId);
                }
                if (callback) {
                    callback(transferErr);
                }
                return transferErr;  // ❌ 不调用 TerminateAbility
            }
        }

        // ✅ Transfer succeeded, save callback and clean up request
        TerminateSelfResultCallback callback = std::move(info.callback);
        terminateRequests_.erase(it);

        // Release lock before executing termination
        lock.unlock();
    }

    // ✅ Execute termination (only reached if result transfer succeeded or is TerminateSelf)
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->TerminateAbility(token_, -1, nullptr);

    if (IsEmbeddableStart(screenMode_)) {
        if (err == ERR_OK) {
            isTerminated_.store(true);
        }

        // ✅ Notify callback with termination error
        if (callback) {
            callback(err);
        }
    }

    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "TerminateAbility failed, err=%{public}d", err);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfInner end");
    return err;
}

ErrCode UIExtensionContext::TerminateSelfWithAnimation(
    TerminateSelfWithAnimationCallback &&callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfWithAnimation called");

    if (!IsEmbeddableStart(screenMode_)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Not embeddable mode, registration not allowed");
        return ERR_INVALID_OPERATION;
    }

    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Callback is null");
        return ERR_INVALID_VALUE;
    }

    {
        std::lock_guard<std::mutex> lock(terminateSelfMutex_);
        if (terminateSelfWithAnimationCallback_ != nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "Callback already registered, duplicate registration not allowed");
            return ERR_INVALID_OPERATION;
        }

        terminateSelfWithAnimationCallback_ = std::move(callback);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfWithAnimation success");
    return ERR_OK;
}

ErrCode UIExtensionContext::ConnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin, ability:%{public}s",
        want.GetElement().GetAbilityName().c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectAbility(token_, want, connectCallback);
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtensionContext::ConnectAbility ErrorCode = %{public}d", ret);
    return ret;
}

ErrCode UIExtensionContext::ConnectUIServiceExtensionAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin, ability:%{public}s",
        want.GetElement().GetAbilityName().c_str());
    ErrCode ret =
        ConnectionManager::GetInstance().ConnectUIServiceExtensionAbility(token_, want, connectCallback);
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtensionContext::ConnectUIServiceExtensionAbility ErrorCode = %{public}d", ret);
    return ret;
}

ErrCode UIExtensionContext::DisconnectAbility(
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback) const
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    ErrCode ret =
        ConnectionManager::GetInstance().DisconnectAbility(token_, want, connectCallback);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", ret);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return ret;
}

ErrCode UIExtensionContext::StartServiceExtensionAbility(const AAFwk::Want& want, int32_t accountId)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "Start service extension %{public}s/%{public}s, accountId: %{public}d",
        want.GetElement().GetBundleName().c_str(), want.GetElement().GetAbilityName().c_str(), accountId);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->StartExtensionAbility(
        want, token_, accountId, AppExecFwk::ExtensionAbilityType::SERVICE);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Start service extension failed %{public}d", ret);
    }
    return ret;
}

ErrCode UIExtensionContext::StartUIAbilities(const std::vector<AAFwk::Want> &wantList, const std::string &requestKey)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "call StartUIAbilities");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartUIAbilities(wantList, requestKey, token_);
    return err;
}

ErrCode UIExtensionContext::StartAbilityForResult(const AAFwk::Want &want, int requestCode, RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, token_, requestCode);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret=%{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return err;
}

void UIExtensionContext::InsertResultCallbackTask(int requestCode, RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    }
}

void UIExtensionContext::RemoveResultCallbackTask(int requestCode)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.erase(requestCode);
    }
}

ErrCode UIExtensionContext::StartAbilityForResult(
    const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbility(want, startOptions, token_, requestCode);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret=%{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
    return err;
}

ErrCode UIExtensionContext::StartAbilityForResultAsCaller(const AAFwk::Want &want, int requestCode, RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityForResultAsCaller(want, token_, requestCode);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "End");
    return err;
}

ErrCode UIExtensionContext::StartAbilityForResultAsCaller(
    const AAFwk::Want &want, const AAFwk::StartOptions &startOptions, int requestCode, RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    {
        std::lock_guard<std::mutex> lock(mutexlock_);
        resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    }
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->StartAbilityForResultAsCaller(
        want, startOptions, token_, requestCode);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "End");
    return err;
}

ErrCode UIExtensionContext::ReportDrawnCompleted()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->ReportDrawnCompleted(token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret=%{public}d", err);
    }
    return err;
}

void UIExtensionContext::OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    std::lock_guard<std::mutex> lock(mutexlock_);
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, false);
        }
        resultCallbacks_.erase(requestCode);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

AppExecFwk::AbilityType UIExtensionContext::GetAbilityInfoType() const
{
    std::shared_ptr<AppExecFwk::AbilityInfo> info = GetAbilityInfo();
    if (info == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null info");
        return AppExecFwk::AbilityType::UNKNOWN;
    }

    return info->type;
}

void UIExtensionContext::OnAbilityResultInner(int requestCode, int resultCode, const AAFwk::Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "begin");
    std::lock_guard<std::mutex> lock(mutexlock_);
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, resultData, true);
        }
        resultCallbacks_.erase(requestCode);
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "end");
}

int32_t UIExtensionContext::GetScreenMode() const
{
    return screenMode_;
}

void UIExtensionContext::SetScreenMode(int32_t screenMode)
{
    screenMode_ = screenMode;
}

int UIExtensionContext::GenerateCurRequestCode()
{
    std::lock_guard lock(requestCodeMutex_);
    curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
    return curRequestCode_;
}

TerminateRequestId UIExtensionContext::GenerateTerminateRequestId()
{
    std::lock_guard<std::mutex> lock(terminateRequestIdMutex_);
    curTerminateRequestId_ = (curTerminateRequestId_ == INT_MAX) ? 1 : (curTerminateRequestId_ + 1);
    return curTerminateRequestId_;
}

#ifdef SUPPORT_SCREEN
void UIExtensionContext::SetWindow(sptr<Rosen::Window> window)
{
    window_ = window;
}
sptr<Rosen::Window> UIExtensionContext::GetWindow()
{
    return window_;
}
Ace::UIContent* UIExtensionContext::GetUIContent()
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    if (window_ == nullptr) {
        return nullptr;
    }
    return window_->GetUIContent();
}
#endif // SUPPORT_SCREEN
ErrCode UIExtensionContext::OpenAtomicService(AAFwk::Want& want, const AAFwk::StartOptions &options, int requestCode,
    RuntimeTask &&task)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    resultCallbacks_.insert(make_pair(requestCode, std::move(task)));
    ErrCode err = AAFwk::AbilityManagerClient::GetInstance()->OpenAtomicService(want, options, token_, requestCode);
    if (err != ERR_OK && err != AAFwk::START_ABILITY_WAITING) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", err);
        OnAbilityResultInner(requestCode, err, want);
    }
    return err;
}

ErrCode UIExtensionContext::AddFreeInstallObserver(const sptr<IFreeInstallObserver> &observer)
{
    ErrCode ret = AAFwk::AbilityManagerClient::GetInstance()->AddFreeInstallObserver(token_, observer);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "ret = %{public}d", ret);
    }
    return ret;
}

ErrCode UIExtensionContext::OpenLink(const AAFwk::Want &want, int requestCode, bool hideFailureTipDialog)
{
    return AAFwk::AbilityManagerClient::GetInstance()->OpenLink(want, token_, -1, requestCode, hideFailureTipDialog);
}

std::shared_ptr<Global::Resource::ResourceManager> UIExtensionContext::GetResourceManager() const
{
    if (abilityResourceMgr_) {
        return abilityResourceMgr_;
    }

    return ContextImpl::GetResourceManager();
}

void UIExtensionContext::SetAbilityResourceManager(
    std::shared_ptr<Global::Resource::ResourceManager> abilityResourceMgr)
{
    abilityResourceMgr_ = abilityResourceMgr;
}

void UIExtensionContext::RegisterAbilityConfigUpdateCallback(AbilityConfigUpdateCallback abilityConfigUpdateCallback)
{
    abilityConfigUpdateCallback_ = abilityConfigUpdateCallback;
}

std::shared_ptr<AppExecFwk::Configuration> UIExtensionContext::GetAbilityConfiguration() const
{
    return abilityConfiguration_;
}

void UIExtensionContext::SetAbilityConfiguration(const AppExecFwk::Configuration &config)
{
    if (!abilityConfiguration_) {
        abilityConfiguration_ = std::make_shared<AppExecFwk::Configuration>(config);
        TAG_LOGI(AAFwkTag::CONTEXT, "abilityConfiguration: %{public}s", abilityConfiguration_->GetName().c_str());
        return;
    }
    std::vector<std::string> changeKeyV;
    abilityConfiguration_->CompareDifferent(changeKeyV, config);
    if (!changeKeyV.empty()) {
        abilityConfiguration_->Merge(changeKeyV, config);
    }
    TAG_LOGI(AAFwkTag::CONTEXT, "abilityConfiguration: %{public}s", abilityConfiguration_->GetName().c_str());
}

void UIExtensionContext::SetAbilityColorMode(int32_t colorMode)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "SetAbilityColorMode colorMode: %{public}d", colorMode);
    if (colorMode < -1 || colorMode > 1) {
        TAG_LOGE(AAFwkTag::CONTEXT, "colorMode error");
        return;
    }
    AppExecFwk::Configuration config;

    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, AppExecFwk::GetColorModeStr(colorMode));
    config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP,
        AppExecFwk::ConfigurationInner::IS_SET_BY_APP);
    if (!abilityConfigUpdateCallback_) {
        TAG_LOGE(AAFwkTag::CONTEXT, "abilityConfigUpdateCallback_ nullptr");
        return;
    }
    abilityConfigUpdateCallback_(config);
}

void UIExtensionContext::RequestComponentTerminate()
{
    TAG_LOGD(AAFwkTag::CONTEXT, "RequestComponentTerminate called");
    if (!window_) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null window_");
        return;
    }
    AAFwk::WantParams params;
    params.SetParam(REQUEST_COMPONENT_TERMINATE_KEY, AAFwk::Boolean::Box(true));
    auto ret = window_->TransferExtensionData(params);
    if (ret != Rosen::WMError::WM_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "transfer extension data failed, ret:%{public}d", ret);
    }
}

bool UIExtensionContext::IsTerminating()
{
    return isTerminating_;
}

void UIExtensionContext::SetTerminating(bool state)
{
    isTerminating_ = state;
}

ErrCode UIExtensionContext::StartAbilityByType(const std::string &type,
    AAFwk::WantParams &wantParam, const std::shared_ptr<JsUIExtensionCallback> &uiExtensionCallbacks)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbilityByType begin.");
    if (uiExtensionCallbacks == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiExtensionCallbacks");
        return ERR_INVALID_VALUE;
    }
    auto uiContent = GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null uiContent");
        return ERR_INVALID_VALUE;
    }
    wantParam.SetParam(UIEXTENSION_TARGET_TYPE_KEY, AAFwk::String::Box(type));
    AAFwk::Want want;
    want.SetParams(wantParam);
    if (wantParam.HasParam(FLAG_AUTH_READ_URI_PERMISSION)) {
        int32_t flag = wantParam.GetIntParam(FLAG_AUTH_READ_URI_PERMISSION, 0);
        want.SetFlags(flag);
        wantParam.Remove(FLAG_AUTH_READ_URI_PERMISSION);
    }

    OHOS::Ace::ModalUIExtensionCallbacks callback;
    OHOS::Ace::ModalUIExtensionConfig config;
    callback.onError = [uiExtensionCallbacks](int32_t arg, const std::string &str1, const std::string &str2) {
        uiExtensionCallbacks->OnError(arg);
    };
    callback.onRelease = [uiExtensionCallbacks](int32_t arg) {
        uiExtensionCallbacks->OnRelease(arg);
    };
    callback.onResult = [uiExtensionCallbacks](int32_t arg1, const OHOS::AAFwk::Want arg2) {
        uiExtensionCallbacks->OnResult(arg1, arg2);
    };

    int32_t sessionId = uiContent->CreateModalUIExtension(want, callback, config);
    if (sessionId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "createModalUIExtension fail");
        return ERR_INVALID_VALUE;
    }
    uiExtensionCallbacks->SetUIContent(uiContent);
    uiExtensionCallbacks->SetSessionId(sessionId);
    return ERR_OK;
}

ErrCode UIExtensionContext::AddCompletionHandlerForAtomicService(const std::string &requestId,
    OnAtomicRequestSuccess onRequestSucc, OnAtomicRequestFailure onRequestFail, const std::string &appId)
{
    if (onRequestSucc == nullptr || onRequestFail == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "either func is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard lock(onRequestResultMutex_);
    for (auto iter = onAtomicRequestResults_.begin(); iter != onAtomicRequestResults_.end(); iter++) {
        if ((*iter)->requestId_ == requestId) {
            TAG_LOGI(AAFwkTag::UI_EXT, "requestId=%{public}s already exists", requestId.c_str());
            return ERR_OK;
        }
    }
    onAtomicRequestResults_.emplace_back(std::make_shared<OnAtomicRequestResult>(
        requestId, appId, onRequestSucc, onRequestFail));
    return ERR_OK;
}

void UIExtensionContext::OnRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message)
{
    std::shared_ptr<OnAtomicRequestResult> atomicResult = nullptr;
    {
        std::lock_guard<std::mutex> lock(onRequestResultMutex_);
        for (auto iter = onAtomicRequestResults_.begin(); iter != onAtomicRequestResults_.end(); iter++) {
            if ((*iter)->requestId_ == requestId) {
                atomicResult = *iter;
                onAtomicRequestResults_.erase(iter);
                break;
            }
        }
    }

    if (atomicResult != nullptr) {
        TAG_LOGI(AAFwkTag::CONTEXT, "requestId=%{public}s, call onRequestSuccess", requestId.c_str());
        atomicResult->onRequestSuccess_(atomicResult->appId_);
        return;
    }

    OnOpenLinkRequestSuccess(requestId, element, message);
}

void UIExtensionContext::OnRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message, int32_t resultCode)
{
    std::shared_ptr<OnAtomicRequestResult> atomicResult = nullptr;
    {
        std::lock_guard lock(onRequestResultMutex_);
        for (auto iter = onAtomicRequestResults_.begin(); iter != onAtomicRequestResults_.end(); iter++) {
            if ((*iter)->requestId_ == requestId) {
                atomicResult = *iter;
                onAtomicRequestResults_.erase(iter);
                break;
            }
        }
    }

    if (atomicResult != nullptr) {
        TAG_LOGI(AAFwkTag::UI_EXT, "requestId=%{public}s, call onRequestFailure", requestId.c_str());
        int32_t failureCode = 0;
        std::string failureMessage;
        GetFailureInfoByMessage(message, failureCode, failureMessage, resultCode);
        atomicResult->onRequestFailure_(atomicResult->appId_, failureCode, failureMessage);
        return;
    }

    OnOpenLinkRequestFailure(requestId, element, message);
}

ErrCode UIExtensionContext::AddCompletionHandlerForOpenLink(const std::string &requestId,
    OnRequestResult onRequestSucc, OnRequestResult onRequestFail)
{
    if (onRequestSucc == nullptr || onRequestFail == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "either func is null");
        return ERR_INVALID_VALUE;
    }
    std::lock_guard lock(onOpenLinkRequestResultMutex_);
    for (auto iter = onOpenLinkRequestResults_.begin(); iter != onOpenLinkRequestResults_.end(); iter++) {
        if ((*iter)->requestId_ == requestId) {
            TAG_LOGW(AAFwkTag::UI_EXT, "requestId=%{public}s already exists", requestId.c_str());
            return ERR_OK;
        }
    }
    onOpenLinkRequestResults_.emplace_back(std::make_shared<AAFwk::OnOpenLinkRequestResult>(
        requestId, onRequestSucc, onRequestFail));
    return ERR_OK;
}

void UIExtensionContext::OnOpenLinkRequestSuccess(const std::string &requestId,
    const AppExecFwk::ElementName &element, const std::string &message)
{
    std::shared_ptr<AAFwk::OnOpenLinkRequestResult> openLinkResult = nullptr;
    {
        std::lock_guard lock(onOpenLinkRequestResultMutex_);
        for (auto iter = onOpenLinkRequestResults_.begin(); iter != onOpenLinkRequestResults_.end(); iter++) {
            if ((*iter)->requestId_ == requestId) {
                openLinkResult = *iter;
                onOpenLinkRequestResults_.erase(iter);
                break;
            }
        }
    }
    if (openLinkResult != nullptr) {
        TAG_LOGI(AAFwkTag::UI_EXT, "requestId=%{public}s, call onRequestSuccess", requestId.c_str());
        openLinkResult->onRequestSuccess_(element, message);
        return;
    }

    TAG_LOGE(AAFwkTag::UI_EXT, "requestId=%{public}s not exist", requestId.c_str());
}

void UIExtensionContext::OnOpenLinkRequestFailure(const std::string &requestId,
    const AppExecFwk::ElementName &element, const std::string &message)
{
    if (requestId.empty()) {
        return;
    }
    std::shared_ptr<AAFwk::OnOpenLinkRequestResult> openLinkResult = nullptr;
    {
        std::lock_guard lock(onOpenLinkRequestResultMutex_);
        for (auto iter = onOpenLinkRequestResults_.begin(); iter != onOpenLinkRequestResults_.end(); iter++) {
            if ((*iter)->requestId_ == requestId) {
                openLinkResult = *iter;
                onOpenLinkRequestResults_.erase(iter);
                break;
            }
        }
    }
    if (openLinkResult != nullptr) {
        TAG_LOGI(AAFwkTag::UI_EXT, "requestId=%{public}s, call onRequestFailure", requestId.c_str());
        openLinkResult->onRequestFailure_(element, message);
        return;
    }

    TAG_LOGE(AAFwkTag::UI_EXT, "requestId=%{public}s not exist", requestId.c_str());
}

void UIExtensionContext::GetFailureInfoByMessage(
    const std::string &message, int32_t &failureCode, std::string &failureMessage, int32_t resultCode)
{
    if (resultCode == USER_CANCEL) {
        failureCode = static_cast<int32_t>(FailureCode::FAILURE_CODE_USER_CANCEL);
        failureMessage = "The user canceled this startup";
    } else if (message.find("User refused redirection") != std::string::npos) {
        failureCode = static_cast<int32_t>(FailureCode::FAILURE_CODE_USER_REFUSE);
        failureMessage = "User refused redirection";
    } else {
        failureCode = static_cast<int32_t>(FailureCode::FAILURE_CODE_SYSTEM_MALFUNCTION);
        failureMessage = "A system error occurred";
    }
}

int32_t UIExtensionContext::curRequestCode_ = 0;
std::mutex UIExtensionContext::requestCodeMutex_;
TerminateRequestId UIExtensionContext::curTerminateRequestId_ = 0;
std::mutex UIExtensionContext::terminateRequestIdMutex_;
}  // namespace AbilityRuntime
}  // namespace OHOS
