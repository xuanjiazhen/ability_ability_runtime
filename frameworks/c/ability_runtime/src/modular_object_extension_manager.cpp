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

#include "ability_business_error_utils.h"
#include "ability_manager_client.h"
#include "ability_manager/include/modular_object_extension_info.h"
#include "hilog_tag_wrapper.h"
#include "want_manager.h"

struct OH_AbilityRuntime_ModularObject_ExtensionInfo {
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;

    int32_t appIndex = 0;
    OH_AbilityRuntime_LaunchMode launchMode = OH_ABILITY_RUNTIME_LAUNCH_MODE_IN_PROCESS;
    OH_AbilityRuntime_ProcessMode processMode = OH_ABILITY_RUNTIME_PROCESS_MODE_BUNDLE;
    OH_AbilityRuntime_ThreadMode threadMode = OH_ABILITY_RUNTIME_THREAD_MODE_BUNDLE;
    bool isSelfDisabled = false;
    bool isSystemDisabled = false;
};

struct OH_AbilityRuntime_ModularObject_AllExtensionInfos {
    /**
     * List of all devices
     */
    std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> allMoeInfos;

    size_t count;
};

namespace {
OH_AbilityRuntime_MoeInfoHandle CreateExtensionInfo(const OHOS::AAFwk::ModularObjectExtensionInfo &source)
{
    auto info = std::make_unique<OH_AbilityRuntime_ModularObject_ExtensionInfo>();
    info->bundleName = source.bundleName;
    info->moduleName = source.moduleName;
    info->abilityName = source.abilityName;
    info->appIndex = source.appIndex;
    info->launchMode = static_cast<OH_AbilityRuntime_LaunchMode>(source.launchMode);
    info->processMode = static_cast<OH_AbilityRuntime_ProcessMode>(source.processMode);
    info->threadMode = static_cast<OH_AbilityRuntime_ThreadMode>(source.threadMode);
    info->isSelfDisabled = source.isSelfDisabled;
    info->isSystemDisabled = source.isSystemDisabled;
    return info.release();
}
} // namespace

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_ErrorCode OH_AbilityRuntime_DestroyExtensionInfo(OH_AbilityRuntime_MoeInfoHandle *extensionInfo)
{
    if (extensionInfo == nullptr || *extensionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null extensionInfo");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    delete *extensionInfo;
    *extensionInfo = nullptr;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_LaunchMode *launchMode)
{
    if (!extensionInfo || !launchMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *launchMode = static_cast<OH_AbilityRuntime_LaunchMode>(extensionInfo->launchMode);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_ProcessMode *processMode)
{
    if (!extensionInfo || !processMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *processMode = static_cast<OH_AbilityRuntime_ProcessMode>(extensionInfo->processMode);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, OH_AbilityRuntime_ThreadMode *threadMode)
{
    if (!extensionInfo || !threadMode) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *threadMode = static_cast<OH_AbilityRuntime_ThreadMode>(extensionInfo->threadMode);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, AbilityBase_Element *element)
{
    if (!extensionInfo || !element) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    element->bundleName = const_cast<char *>(extensionInfo->bundleName.c_str());
    element->moduleName = const_cast<char *>(extensionInfo->moduleName.c_str());
    element->abilityName = const_cast<char *>(extensionInfo->abilityName.c_str());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(
    OH_AbilityRuntime_MoeInfoHandle extensionInfo, bool *isSelfDisabled, bool *isSystemDisabled)
{
    if (!extensionInfo || !isSelfDisabled || !isSystemDisabled) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *isSelfDisabled = extensionInfo->isSelfDisabled;
    *isSystemDisabled = extensionInfo->isSystemDisabled;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_QuerySelfModularObjectExtensionInfos(
    OH_AbilityRuntime_AllMoeInfosHandle *allExtensionInfos)
{
    if (!allExtensionInfos) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (*allExtensionInfos != nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "handle is not null, to avoid memory leak it should be null");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    auto abilityManagerClient = OHOS::AAFwk::AbilityManagerClient::GetInstance();
    if (!abilityManagerClient) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    std::unique_ptr<OH_AbilityRuntime_ModularObject_AllExtensionInfos> infos =
        std::make_unique<OH_AbilityRuntime_ModularObject_AllExtensionInfos>();
    std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> dataList;
    auto ret = abilityManagerClient->QueryAllModularObjectExtensionInfo(dataList);
    if (ret != OHOS::ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "get modular object extension info inner error: %{public}d", ret);
        TAG_LOGE(AAFwkTag::APPKIT, "get modular object extension info error: %{public}d", ConvertToCommonBusinessErrorCode(ret));
        // 后续调用真实业务功能获取信息，以下为保留模拟数据
        OHOS::AAFwk::ModularObjectExtensionInfo info1;
        info1.bundleName = "bundleName1";
        info1.moduleName = "moduleName1";
        info1.abilityName = "abilityName1";
        info1.launchMode = OHOS::AAFwk::LaunchMode::IN_PROCESS;
        info1.processMode = OHOS::AAFwk::ModularProcessMode::BUNDLE;
        info1.threadMode = OHOS::AAFwk::ThreadMode::TYPE;
        info1.appIndex = 0;
        dataList.push_back(info1);
        
        OHOS::AAFwk::ModularObjectExtensionInfo info2;
        info2.bundleName = "bundleName2";
        info2.moduleName = "moduleName2";
        info2.abilityName = "abilityName2";
        info2.launchMode = OHOS::AAFwk::LaunchMode::CROSS_PROCESS;
        info2.processMode = OHOS::AAFwk::ModularProcessMode::TYPE;
        info2.threadMode = OHOS::AAFwk::ThreadMode::INSTANCE;
        info2.appIndex = 1;
        info2.isSelfDisabled = true;
        info2.isSystemDisabled = true;
        dataList.push_back(info2);
        // return ConvertToCommonBusinessErrorCode(ret);

    }
    infos->allMoeInfos = dataList;
    infos->count = dataList.size();
    *allExtensionInfos = infos.release();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_DestroyAllExtensionInfos(OH_AbilityRuntime_AllMoeInfosHandle *extensionInfo)
{
    if (!extensionInfo || !*extensionInfo) {
        TAG_LOGD(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    delete *extensionInfo;
    *extensionInfo = nullptr;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetCountFromAllMoeInfos(
    OH_AbilityRuntime_AllMoeInfosHandle allExtensionInfos, size_t *count)
{
    if (!allExtensionInfos || !count) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *count = allExtensionInfos->count;
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
    if (index >= allExtensionInfos->count) {
        TAG_LOGE(AAFwkTag::APPKIT, "index out of range");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *extensionInfo = CreateExtensionInfo(allExtensionInfos->allMoeInfos[index]);
    if (*extensionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create extension info for index %d", index);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

#ifdef __cplusplus
}  // extern "C"
#endif
