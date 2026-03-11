/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "modular_object_extension_manager.h"

#include <atomic>
#include <mutex>
#include <new>
#include <string>
#include <unordered_map>

#include "ability_business_error_utils.h"
#include "ability_connect_callback.h"
#include "ability_manager_errors.h"
#include "connect_options_impl.h"
#include "connection_manager.h"
#include "hilog_tag_wrapper.h"
#include "ipc_inner_object.h"
#include "native_extension/context_impl.h"
#include "native_extension/modular_object_extension_common.h"
#include "securec.h"
#include "want_manager.h"
#include "want_utils.h"

using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace {
struct OH_AbilityRuntime_ModularObject_ExtensionInfo {
    OH_AbilityRuntime_LaunchMode launchMode = OH_AbilityRuntime_LaunchMode_InProccess;
    OH_AbilityRuntime_ProcessMode processMode = OH_AbilityRuntime_ProcessMode_Bundle;
    OH_AbilityRuntime_ThreadMode threadMode = OH_AbilityRuntime_ThreadMode_Bundle;
    AbilityBase_Element element {};
    bool isSelfDisabled = false;
    bool isSystemDisabled = false;
};

struct MoeConnectionRecord {
    sptr<IRemoteObject> token;
    Want want;
    sptr<AbilityConnectCallback> callback;
};

std::atomic<int64_t> g_nextConnectionId(1);
std::mutex g_connectionMutex;
std::unordered_map<int64_t, MoeConnectionRecord> g_connectionRecords;

void RemoveConnectionRecord(int64_t connectionId)
{
    std::lock_guard<std::mutex> guard(g_connectionMutex);
    g_connectionRecords.erase(connectionId);
}

AbilityRuntime_ErrorCode ConvertConnectBusinessErrorCode(int32_t errCode)
{
    switch (errCode) {
        case AAFwk::ABILITY_VISIBLE_FALSE_DENY_REQUEST:
            return ABILITY_RUNTIME_ERROR_CODE_VISIBILITY_VERIFICATION_FAILED;
        case AAFwk::ERR_STATIC_CFG_PERMISSION:
            return ABILITY_RUNTIME_ERROR_CODE_STATIC_CFG_PERMISSION;
        case AAFwk::ERR_CROSS_USER:
            return ABILITY_RUNTIME_ERROR_CODE_CROSS_USER_OPERATION;
        case AAFwk::ERR_CHECK_CALL_FROM_BACKGROUND_FAILED:
            return ABILITY_RUNTIME_ERROR_CODE_NO_RUNNING_ABILITIES_WITH_UI;
        case AAFwk::ERR_FREQ_START_ABILITY:
            return ABILITY_RUNTIME_ERROR_CODE_UPPER_RATE_LIMIT;
        case AAFwk::ERR_REACH_UPPER_LIMIT:
        case AAFwk::ERR_UPPER_LIMIT:
            return ABILITY_RUNTIME_ERROR_CODE_UPPER_CONNECTION_NUMBER_LIMIT;
        default:
            return ConvertToCommonBusinessErrorCode(errCode);
    }
}

void NotifyFailed(const std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> &state, int32_t businessErrorCode)
{
    if (state == nullptr) {
        return;
    }
    OH_AbilityRuntime_ConnectOptions_OnFailedCallback callback = nullptr;
    OH_AbilityRuntime_ConnectOptions *owner = nullptr;
    {
        std::lock_guard<std::mutex> guard(state->mutex);
        if (!state->alive) {
            return;
        }
        callback = state->onFailedCallback;
        owner = state->owner;
    }
    if (callback != nullptr) {
        callback(owner, businessErrorCode);
    }
}

bool CopyToCString(const std::string &src, char *&dst)
{
    dst = new (std::nothrow) char[src.size() + 1];
    if (dst == nullptr) {
        return false;
    }
    if (strcpy_s(dst, src.size() + 1, src.c_str()) != EOK) {
        delete[] dst;
        dst = nullptr;
        return false;
    }
    return true;
}

bool BuildElement(const AppExecFwk::ElementName &elementName, AbilityBase_Element &element)
{
    element.bundleName = nullptr;
    element.moduleName = nullptr;
    element.abilityName = nullptr;
    if (!CopyToCString(elementName.GetBundleName(), element.bundleName)) {
        return false;
    }
    if (!CopyToCString(elementName.GetModuleName(), element.moduleName)) {
        delete[] element.bundleName;
        element.bundleName = nullptr;
        return false;
    }
    if (!CopyToCString(elementName.GetAbilityName(), element.abilityName)) {
        delete[] element.bundleName;
        delete[] element.moduleName;
        element.bundleName = nullptr;
        element.moduleName = nullptr;
        return false;
    }
    return true;
}

void DestroyElement(AbilityBase_Element &element)
{
    delete[] element.bundleName;
    delete[] element.moduleName;
    delete[] element.abilityName;
    element.bundleName = nullptr;
    element.moduleName = nullptr;
    element.abilityName = nullptr;
}

AbilityRuntime_ErrorCode TransformWant(AbilityBase_Want *want, Want &abilityWant)
{
    auto ret = CheckWant(want);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid want");
        return ret;
    }
    auto errCode = CWantManager::TransformToWant(*want, false, abilityWant);
    if (errCode != ABILITY_BASE_ERROR_CODE_NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPKIT, "transform want failed: %{public}d", errCode);
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode CheckContextAndToken(AbilityRuntime_ContextHandle context, sptr<IRemoteObject> &token)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto contextPtr = context->context.lock();
    if (contextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context not exist");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    token = contextPtr->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null token");
        return ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

class MoeConnectionCallback : public AbilityConnectCallback {
public:
    MoeConnectionCallback(int64_t connectionId, const std::shared_ptr<OH_AbilityRuntime_ConnectOptionsState> &state)
        : connectionId_(connectionId), state_(state)
    {}

    ~MoeConnectionCallback() override = default;

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode) override
    {
        if (resultCode != ERR_OK || remoteObject == nullptr) {
            auto businessErrorCode = (resultCode == ERR_OK) ?
                ABILITY_RUNTIME_ERROR_CODE_INTERNAL : ConvertConnectBusinessErrorCode(resultCode);
            NotifyFailed(state_.lock(), businessErrorCode);
            RemoveConnectionRecord(connectionId_);
            return;
        }

        auto state = state_.lock();
        if (state == nullptr) {
            return;
        }

        OH_AbilityRuntime_ConnectOptions_OnConnectCallback callback = nullptr;
        OH_AbilityRuntime_ConnectOptions *owner = nullptr;
        {
            std::lock_guard<std::mutex> guard(state->mutex);
            if (!state->alive) {
                return;
            }
            callback = state->onConnectCallback;
            owner = state->owner;
        }
        if (callback == nullptr) {
            return;
        }

        AbilityBase_Element cElement;
        if (!BuildElement(element, cElement)) {
            NotifyFailed(state, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
            return;
        }
        OHIPCRemoteProxy *proxy = CreateIPCRemoteProxy(remoteObject);
        if (proxy == nullptr) {
            DestroyElement(cElement);
            NotifyFailed(state, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
            return;
        }
        callback(owner, &cElement, proxy);
        DestroyElement(cElement);
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        auto state = state_.lock();
        if (resultCode != ERR_OK) {
            NotifyFailed(state, ConvertConnectBusinessErrorCode(resultCode));
            RemoveConnectionRecord(connectionId_);
            return;
        }

        if (state != nullptr) {
            OH_AbilityRuntime_ConnectOptions_OnDisconnectCallback callback = nullptr;
            OH_AbilityRuntime_ConnectOptions *owner = nullptr;
            {
                std::lock_guard<std::mutex> guard(state->mutex);
                if (state->alive) {
                    callback = state->onDisconnectCallback;
                    owner = state->owner;
                }
            }

            if (callback != nullptr) {
                AbilityBase_Element cElement;
                if (BuildElement(element, cElement)) {
                    callback(owner, &cElement);
                    DestroyElement(cElement);
                } else {
                    NotifyFailed(state, ABILITY_RUNTIME_ERROR_CODE_INTERNAL);
                }
            }
        }
        RemoveConnectionRecord(connectionId_);
    }

private:
    int64_t connectionId_ = 0;
    std::weak_ptr<OH_AbilityRuntime_ConnectOptionsState> state_;
};
} // namespace

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_Errcode OH_AbilityRuntime_ConnectModularObjectExtensionAbility(
    AbilityRuntime_ContextHandle context, AbilityBase_Want *want,
    OH_AbilityRuntime_ConnectOptions *connectOptions, int64_t *connectionId)
{
    if (connectOptions == nullptr || connectionId == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (connectOptions->state == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null connectOptions state");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    {
        std::lock_guard<std::mutex> guard(connectOptions->state->mutex);
        if (!connectOptions->state->alive) {
            TAG_LOGE(AAFwkTag::APPKIT, "connect options already destroyed");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
    }

    Want abilityWant;
    auto ret = TransformWant(want, abilityWant);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    sptr<IRemoteObject> token;
    ret = CheckContextAndToken(context, token);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    int64_t connectId = g_nextConnectionId.fetch_add(1);
    auto callback = sptr<MoeConnectionCallback>::MakeSptr(connectId, connectOptions->state);
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create callback");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    int32_t innerRet = ConnectionManager::GetInstance().ConnectExtensionAbilityWithExtensionType(
        token, abilityWant, callback, GetModularObjectExtensionType());
    if (innerRet != ERR_OK) {
        auto businessErrorCode = ConvertConnectBusinessErrorCode(innerRet);
        NotifyFailed(connectOptions->state, businessErrorCode);
        return businessErrorCode;
    }

    {
        std::lock_guard<std::mutex> guard(g_connectionMutex);
        g_connectionRecords.emplace(connectId, MoeConnectionRecord{ token, abilityWant, callback });
    }
    *connectionId = connectId;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_Errcode OH_AbilityRuntime_DisconnectModularObjectExtensionAbility(
    AbilityRuntime_ContextHandle context, int64_t connectionId)
{
    sptr<IRemoteObject> token;
    auto ret = CheckContextAndToken(context, token);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    Want want;
    sptr<AbilityConnectCallback> callback = nullptr;
    {
        std::lock_guard<std::mutex> guard(g_connectionMutex);
        auto iter = g_connectionRecords.find(connectionId);
        if (iter == g_connectionRecords.end()) {
            TAG_LOGE(AAFwkTag::APPKIT, "connection id not found");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        if (iter->second.token != token) {
            TAG_LOGE(AAFwkTag::APPKIT, "connection owner mismatched");
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        want = iter->second.want;
        callback = iter->second.callback;
    }

    int32_t innerRet = ConnectionManager::GetInstance().DisconnectAbility(token, want, callback);
    auto businessErrorCode = ConvertConnectBusinessErrorCode(innerRet);
    if (innerRet == ERR_OK || innerRet == AAFwk::CONNECTION_NOT_EXIST) {
        RemoveConnectionRecord(connectionId);
    }
    return businessErrorCode;
}

AbilityRuntime_Errcode OH_AbilityRuntime_DestroyExtensionInfo(OH_AbilityRuntime_MoeInfoHandle extensionInfo)
{
    if (extensionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null extensionInfo");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    DestroyElement(extensionInfo->element);
    delete extensionInfo;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_Errcode OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_LaunchMode *launchMode)
{
    if (extensionInfo == nullptr || launchMode == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *launchMode = extensionInfo->launchMode;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_Errcode OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_ProcessMode *processMode)
{
    if (extensionInfo == nullptr || processMode == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *processMode = extensionInfo->processMode;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_Errcode OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_ThreadMode *threadMode)
{
    if (extensionInfo == nullptr || threadMode == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *threadMode = extensionInfo->threadMode;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_Errcode OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, AbilityBase_Element *element)
{
    if (extensionInfo == nullptr || element == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *element = extensionInfo->element;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_Errcode OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, bool *isSelfDisabled, bool *isSystemDisabled)
{
    if (extensionInfo == nullptr || isSelfDisabled == nullptr || isSystemDisabled == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *isSelfDisabled = extensionInfo->isSelfDisabled;
    *isSystemDisabled = extensionInfo->isSystemDisabled;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_Errcode OH_AbilityRuntime_QuerySelfModularObjectExtensionInfo(
    OH_AbilityRuntime_MoeInfoHandle *extensionInfos, size_t *count)
{
    if (extensionInfos == nullptr || count == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid params");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *extensionInfos = nullptr;
    *count = 0;
    return ABILITY_RUNTIME_ERROR_CODE_NOT_SUPPORTED;
}

AbilityRuntime_Errcode OH_AbilityRuntime_SetSelfModularObjectDisableState(char *abilityName, bool isDisable)
{
    (void)abilityName;
    (void)isDisable;
    return ABILITY_RUNTIME_ERROR_CODE_NOT_SUPPORTED;
}

#ifdef __cplusplus
} // extern "C"
#endif
