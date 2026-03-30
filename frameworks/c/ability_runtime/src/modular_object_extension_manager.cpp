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

#include "ability_runtime/modular_object_extension_manager.h"

#include "modular_object_extension_manager.h"

#include <atomic>
#include <mutex>
#include <new>
#include <string>
#include <unordered_map>

#include "ability_business_error_utils.h"
#include "ability_connect_callback.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "ability_manager/include/modular_object_extension_info.h"
#include "connect_options_impl.h"
#include "connection_manager.h"
#include "hilog_tag_wrapper.h"
#include "ipc_inner_object.h"
#include "modular_object_extension_types.h"
#include "native_extension/context_impl.h"
#include "securec.h"
#include "want_manager.h"
#include "want_utils.h"

using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace {
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
        sptr<IRemoteObject> remoteObjectCopy = remoteObject;
        OHIPCRemoteProxy *proxy = CreateIPCRemoteProxy(remoteObjectCopy);
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

struct OH_AbilityRuntime_ModularObject_AllExtensionInfos {
    std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> allMoeInfos;
};

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_ErrorCode OH_AbilityRuntime_ConnectModularObjectExtensionAbility(
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

AbilityRuntime_ErrorCode OH_AbilityRuntime_DisconnectModularObjectExtensionAbility(
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

AbilityRuntime_ErrorCode OH_AbilityRuntime_DestroyAllExtensionInfos(
    OH_AbilityRuntime_AllMoeInfosHandle *allExtensionInfos)
{
    if (!allExtensionInfos || !*allExtensionInfos) {
        TAG_LOGD(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    delete *allExtensionInfos;
    *allExtensionInfos = nullptr;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_LaunchMode *launchMode)
{
    if (!extensionInfo || !launchMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto info = reinterpret_cast<OHOS::AAFwk::ModularObjectExtensionInfo*>(extensionInfo);
    *launchMode = static_cast<OH_AbilityRuntime_LaunchMode>(info->launchMode);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_ProcessMode *processMode)
{
    if (!extensionInfo || !processMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto info = reinterpret_cast<OHOS::AAFwk::ModularObjectExtensionInfo*>(extensionInfo);
    *processMode = static_cast<OH_AbilityRuntime_ProcessMode>(info->processMode);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_ThreadMode *threadMode)
{
    if (!extensionInfo || !threadMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto info = reinterpret_cast<OHOS::AAFwk::ModularObjectExtensionInfo*>(extensionInfo);
    *threadMode = static_cast<OH_AbilityRuntime_ThreadMode>(info->threadMode);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, AbilityBase_Element *element)
{
    if (!extensionInfo || !element) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto info = reinterpret_cast<OHOS::AAFwk::ModularObjectExtensionInfo*>(extensionInfo);

    char* newBundleName = strdup(info->bundleName.c_str());
    if (!newBundleName) {
        TAG_LOGE(AAFwkTag::APPKIT, "strdup bundleName failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    char* newModuleName = strdup(info->moduleName.c_str());
    if (!newModuleName) {
        free(newBundleName);
        TAG_LOGE(AAFwkTag::APPKIT, "strdup moduleName failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    char* newAbilityName = strdup(info->abilityName.c_str());
    if (!newAbilityName) {
        free(newBundleName);
        free(newModuleName);
        TAG_LOGE(AAFwkTag::APPKIT, "strdup abilityName failed");
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    element->bundleName = newBundleName;
    element->moduleName = newModuleName;
    element->abilityName = newAbilityName;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, bool *isDisabled)
{
    if (!extensionInfo || !isDisabled) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto info = reinterpret_cast<OHOS::AAFwk::ModularObjectExtensionInfo*>(extensionInfo);
    *isDisabled = info->isDisabled;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_QuerySelfModularObjectExtensionInfos(
    OH_AbilityRuntime_AllMoeInfosHandle *allExtensionInfos)
{
    if (!allExtensionInfos) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    std::unique_ptr<OH_AbilityRuntime_ModularObject_AllExtensionInfos> infos =
        std::make_unique<OH_AbilityRuntime_ModularObject_AllExtensionInfos>();
    std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> dataList;
    auto ret = OHOS::AAFwk::AbilityManagerClient::GetInstance()->QuerySelfModularObjectExtensionInfos(dataList);
    if (ret != OHOS::ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "get modular object extension info inner error: %{public}d", ret);
        return ConvertToCommonBusinessErrorCode(ret);
    }
    infos->allMoeInfos = dataList;
    *allExtensionInfos = infos.release();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetCountFromAllMoeInfos(
    OH_AbilityRuntime_AllMoeInfosHandle allExtensionInfos, size_t *count)
{
    if (!allExtensionInfos || !count) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *count = allExtensionInfos->allMoeInfos.size();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetMoeInfoByIndex(
    OH_AbilityRuntime_AllMoeInfosHandle allExtensionInfos, size_t index,
    OH_AbilityRuntime_MoeInfoHandle *extensionInfo)
{
    if (!allExtensionInfos || !extensionInfo) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (index >= allExtensionInfos->allMoeInfos.size()) {
        TAG_LOGE(AAFwkTag::APPKIT, "index out of range");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *extensionInfo = reinterpret_cast<OH_AbilityRuntime_MoeInfoHandle>(&(allExtensionInfos->allMoeInfos[index]));
    if (*extensionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get extension info for index %zu", index);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

#ifdef __cplusplus
}  // extern "C"
#endif
