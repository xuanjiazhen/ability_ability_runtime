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

static char* CopyString(const std::string& source)
{
    if (source.empty()) {
        return nullptr;
    }
    char* result = static_cast<char*>(malloc(source.length() + 1));
    if (result != nullptr) {
        std::copy(source.begin(), source.end(), result);
        result[source.length()] = '\0';
    }
    return result;
}

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_ErrorCode OH_AbilityRuntime_DestroyExtensionInfo(OH_AbilityRuntime_MoeInfoHandle extensionInfo)
{
    if (!extensionInfo) {
        TAG_LOGE(AAFwkTag::APPKIT, "null extensionInfo");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    delete extensionInfo;
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
    element->bundleName = CopyString(extensionInfo->bundleName);
    element->moduleName = CopyString(extensionInfo->moduleName);
    element->abilityName = CopyString(extensionInfo->abilityName);
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

static OH_AbilityRuntime_MoeInfoHandle CreateExtensionInfo(const OHOS::AAFwk::ModularObjectExtensionInfo& source)
{
    auto info = new (std::nothrow) OH_AbilityRuntime_ModularObject_ExtensionInfo();
    if (!info) {
        return nullptr;
    }

    info->bundleName = source.bundleName;
    info->moduleName = source.moduleName;
    info->abilityName = source.abilityName;
    info->appIndex = source.appIndex;
    info->launchMode = static_cast<OH_AbilityRuntime_LaunchMode>(source.launchMode);
    info->processMode = static_cast<OH_AbilityRuntime_ProcessMode>(source.processMode);
    info->threadMode = static_cast<OH_AbilityRuntime_ThreadMode>(source.threadMode);
    info->isSelfDisabled = source.isSelfDisabled;
    info->isSystemDisabled = source.isSystemDisabled;

    return info;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_QuerySelfModularObjectExtensionInfo(
    OH_AbilityRuntime_MoeInfoHandle *extensionInfos, size_t *count)
{
    if (!extensionInfos || !count) {
        TAG_LOGE(AAFwkTag::APPKIT, "null parameter");
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    // 后续调用真实业务功能获取信息，以下为保留模拟数据
    OHOS::AAFwk::ModularObjectExtensionInfo info1;
    info1.bundleName = "bundleName1";
    info1.moduleName = "moduleName1";
    info1.abilityName = "abilityName1";
    info1.launchMode = OHOS::AAFwk::LaunchMode::IN_PROCESS;
    
    OHOS::AAFwk::ModularObjectExtensionInfo info2;
    info2.bundleName = "bundleName2";
    info2.moduleName = "moduleName2";
    info2.abilityName = "abilityName2";
    info2.launchMode = OHOS::AAFwk::LaunchMode::CROSS_PROCESS;
    info2.isSelfDisabled = true;
    info2.isSystemDisabled = true;

    size_t resultCount = 2;
    OH_AbilityRuntime_MoeInfoHandle *infos = static_cast<OH_AbilityRuntime_MoeInfoHandle *>(
        malloc(resultCount * sizeof(OH_AbilityRuntime_MoeInfoHandle)));
    if (!infos) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    infos[0] = CreateExtensionInfo(info1);
    infos[1] = CreateExtensionInfo(info2);

    if (!infos[0] || !infos[1]) {
        OH_AbilityRuntime_DestroyExtensionInfo(infos[0]);
        OH_AbilityRuntime_DestroyExtensionInfo(infos[1]);
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    extensionInfos = infos;
    *count = resultCount;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_SetSelfModularObjectDisableState(char *abilityName, bool isDisable)
{
    if (!abilityName) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

#ifdef __cplusplus
}  // extern "C"
#endif