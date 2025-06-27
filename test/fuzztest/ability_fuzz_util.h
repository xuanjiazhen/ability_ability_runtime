/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef ABILITY_FUZZ_UTIL_H
#define ABILITY_FUZZ_UTIL_H

#include <fuzzer/FuzzedDataProvider.h>
#include <map>
#include <string>
#include <vector>

#include "auto_startup_info.h"
#include "bundle_info.h"
#include "bundle_user_info.h"
#include "extract_insight_intent_profile.h"
#include "keep_alive_process_manager.h"
#include "want.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace AbilityFuzzUtil {
constexpr size_t CODE_TWO = 2;
constexpr size_t CODE_MAX = 99;
constexpr size_t STRING_MAX_LENGTH = 128;
std::vector<std::string> GenerateStringArray(FuzzedDataProvider& fdp, size_t arraySizeMax = STRING_MAX_LENGTH,
    size_t stringSize = STRING_MAX_LENGTH)
{
    std::vector<std::string> result;
    size_t arraySize = fdp.ConsumeIntegralInRange<size_t>(0, arraySizeMax);
    result.reserve(arraySize);

    for (size_t i = 0; i < arraySize; ++i) {
        std::string str = fdp.ConsumeRandomLengthString(stringSize);
        result.emplace_back(str);
    }

    return result;
}

AppExecFwk::ElementName GenerateElementName(FuzzedDataProvider& fdp, AppExecFwk::ElementName &elementName)
{
    std::string deviceId;
    std::string bundleName;
    std::string abilityName;
    std::string moduleName;
    deviceId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    AppExecFwk::ElementName name(deviceId, bundleName, abilityName, moduleName);

    return name;
}

void GetRandomExtractInsightIntentGenericInfo(FuzzedDataProvider& fdp, ExtractInsightIntentGenericInfo& info)
{
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.intentName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.displayName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.decoratorType = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
}

void GetRandomInsightIntentInfoForQuery(FuzzedDataProvider& fdp, InsightIntentInfoForQuery& info)
{
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.intentName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.domain = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.intentVersion = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.displayName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.displayDescription = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.schema = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.icon = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.llmDescription = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.intentType = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.parameters = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.result = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.keywords = GenerateStringArray(fdp);
}

void GetRandomExtractInsightIntentInfo(FuzzedDataProvider& fdp, ExtractInsightIntentInfo& info)
{
    info.decoratorFile = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.decoratorClass = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.displayDescription = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.domain = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.intentVersion = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.schema = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.icon = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.llmDescription = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.result = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.example = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.keywords = GenerateStringArray(fdp);
}

void GetRandomInsightIntentExecuteParam(FuzzedDataProvider& fdp, InsightIntentExecuteParam& info)
{
    info.executeMode_ = fdp.ConsumeIntegral<int32_t>();
    info.displayId_ = fdp.ConsumeIntegral<int32_t>();
    info.flags_ = fdp.ConsumeIntegral<int32_t>();
    info.insightIntentId_ = fdp.ConsumeIntegral<uint64_t>();
    info.bundleName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.moduleName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.abilityName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.insightIntentName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.uris_ = GenerateStringArray(fdp);
    info.decoratorType_ = fdp.ConsumeIntegral<int8_t>();
    info.srcEntrance_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.className_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.methodName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.methodParams_ = GenerateStringArray(fdp);
    info.pagePath_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.navigationId_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.navDestinationName_ = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
}

void GenerateSignatureInfo(FuzzedDataProvider& fdp, SignatureInfo &signatureInfo)
{
    signatureInfo.appId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    signatureInfo.fingerprint = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    signatureInfo.appIdentifier = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    signatureInfo.certificate = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
}

void GetRandomAutoStartupInfo(FuzzedDataProvider& fdp, AutoStartupInfo& info)
{
    info.appCloneIndex = fdp.ConsumeIntegral<int32_t>();
    info.userId = fdp.ConsumeIntegral<int32_t>();
    info.retryCount = fdp.ConsumeIntegral<int32_t>();
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.abilityTypeName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.accessTokenId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
}

void GetRandomBundleInfo(FuzzedDataProvider& fdp, BundleInfo& info)
{
    info.isNewVersion = fdp.ConsumeBool();
    info.isKeepAlive = fdp.ConsumeBool();
    info.singleton = fdp.ConsumeBool();
    info.isPreInstallApp = fdp.ConsumeBool();
    info.isNativeApp = fdp.ConsumeBool();
    info.entryInstallationFree = fdp.ConsumeBool();
    info.isDifferentName = fdp.ConsumeBool();
    info.versionCode = fdp.ConsumeIntegral<uint32_t>();
    info.minCompatibleVersionCode = fdp.ConsumeIntegral<uint32_t>();
    info.compatibleVersion = fdp.ConsumeIntegral<uint32_t>();
    info.targetVersion = fdp.ConsumeIntegral<uint32_t>();
    info.appIndex = fdp.ConsumeIntegral<int32_t>();
    info.minSdkVersion = fdp.ConsumeIntegral<int32_t>();
    info.maxSdkVersion = fdp.ConsumeIntegral<int32_t>();
    info.overlayType = fdp.ConsumeIntegral<int32_t>();
    info.installTime = fdp.ConsumeIntegral<int64_t>();
    info.updateTime = fdp.ConsumeIntegral<int64_t>();
    info.firstInstallTime = fdp.ConsumeIntegral<int64_t>();
    info.name = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.versionName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.vendor = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.releaseType = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.mainEntry = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.entryModuleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.appId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.cpuAbi = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.seInfo = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.label = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.description = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.jointUserId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    GenerateSignatureInfo(fdp, info.signatureInfo);
    info.oldAppIds = GenerateStringArray(fdp);
    info.hapModuleNames = GenerateStringArray(fdp);
    info.moduleNames = GenerateStringArray(fdp);
    info.modulePublicDirs = GenerateStringArray(fdp);
    info.moduleDirs = GenerateStringArray(fdp);
    info.moduleResPaths = GenerateStringArray(fdp);
    info.reqPermissions = GenerateStringArray(fdp);
    info.defPermissions = GenerateStringArray(fdp);
}

void GetRandomKeepAliveInfo(FuzzedDataProvider& fdp, KeepAliveInfo& info)
{
    info.userId = fdp.ConsumeIntegral<int32_t>();
    info.setterId = fdp.ConsumeIntegral<int32_t>();
    info.appType = static_cast<KeepAliveAppType>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.setter = static_cast<KeepAliveSetter>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.policy = static_cast<KeepAlivePolicy>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
}

void GetRandomKeepAliveAbilityInfo(FuzzedDataProvider& fdp, KeepAliveAbilityInfo& info)
{
    info.userId = fdp.ConsumeIntegral<int32_t>();
    info.appCloneIndex = fdp.ConsumeIntegral<int32_t>();
    info.uid = fdp.ConsumeIntegral<int32_t>();
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
}

void GetRandomKeepAliveAppInfo(FuzzedDataProvider& fdp, AppInfo& info)
{
    info.processName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.state = static_cast<AppState>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.pid = fdp.ConsumeIntegral<uint32_t>();
    info.appIndex = fdp.ConsumeIntegral<int32_t>();
    info.instanceKey = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
}

void GetRandomAbilityRequestInfo(FuzzedDataProvider& fdp, AbilityRequestInfo& info)
{
    Want want;
    sptr<IRemoteObject> callerToken = nullptr;
    GetRandomAbilityInfo(fdp, info.info);
    GetRandomResourceInfo(fdp, info.applicationInfo);
    info.startRecnet = fdp.ConsumeBool();
}

void GetRandomAbilityInfo(FuzzedDataProvider& fdp, AbilityInfo& info)
{
    info.name = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.label = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.description = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.iconPath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.labelId = fdp.ConsumeIntegral<uint32_t>();
    info.descriptionId = fdp.ConsumeIntegral<uint32_t>();
    info.iconId = fdp.ConsumeIntegral<uint32_t>();
    info.theme = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.visible = fdp.ConsumeBool();
    info.kind = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.AbilityType = static_cast<AbilityType>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.ExtensionAbilityType = static_cast<ExtensionAbilityType>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.DisplayOrientation = static_cast<DisplayOrientation>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.LaunchMode = static_cast<LaunchMode>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.srcPath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.srcLanguage = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.permissions = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.process = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.deviceTypes = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.deviceCapabilities = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.uri = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.targetAbility = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    GetRandomApplicationInfo(fdp, info.applicationInfo)
    info.isLauncherAbility = fdp.ConsumeBool();
    info.isNativeAbility = fdp.ConsumeBool();
    info.enabled = fdp.ConsumeBool();
    info.supportPipMode = fdp.ConsumeBool();
    info.formEnabled = fdp.ConsumeBool();
    info.removeMissionAfterTerminate = fdp.ConsumeBool();
    info.readPermission = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.writePermission = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.configChanges = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.formEntity = fdp.ConsumeIntegral<uint32_t>();
    info.minFormHeight = fdp.ConsumeIntegral<uint32_t>();
    info.defaultFormHeight = fdp.ConsumeIntegral<uint32_t>();
    info.minFormWidth = fdp.ConsumeIntegral<uint32_t>();
    info.defaultFormWidth = fdp.ConsumeIntegral<uint32_t>();
    info.MetaData = static_cast<MetaData>(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    info.backgroundModes = fdp.ConsumeIntegral<uint32_t>();
    info.SkillUriForAbilityAndExtension = static_cast<SkillUriForAbilityAndExtension>(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    info.package = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.applicationName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.codePath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.resourcePath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.hapPath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.srcEntrance = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.MetaData = static_cast<MetaData>(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    info.isModuleJson = fdp.ConsumeBool();
    info.isStageBasedModel = fdp.ConsumeBool();
    info.continuable = fdp.ConsumeBool();
    info.priority = fdp.ConsumeIntegral<uint32_t>();
    info.startWindow = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.startWindowIcon = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.startWindowIconId = fdp.ConsumeIntegral<uint32_t>();
    info.startWindowBackground = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.startWindowBackgroundId = fdp.ConsumeIntegral<uint32_t>();
    info.preferMultiWindowOrientation = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.excludeFromMissions = fdp.ConsumeBool();
    info.unclearableMission = fdp.ConsumeBool();
    info.excludeFromDock = fdp.ConsumeBool();
    info.recoverable = fdp.ConsumeBool();
    info.isolationProcess = fdp.ConsumeBool();
    info.LinkType = static_cast<LinkType>(fdp.ConsumeIntegralInRange<int8_t>(0, CODE_TWO));
    info.orientationId = fdp.ConsumeIntegral<uint32_t>();
    info.startWindowId = fdp.ConsumeIntegral<uint32_t>();
    info.appIndex = fdp.ConsumeIntegral<uint32_t>();
    info.SupportWindowMode = static_cast<SupportWindowMode>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.continueType = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.continueBundleNames = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.maxWindowRatio = fdp.ConsumeIntegral<uint64_t>();
    info.minWindowRatio = fdp.ConsumeIntegral<uint64_t>();
    info.maxWindowWidth = fdp.ConsumeIntegral<uint32_t>();
    info.minWindowWidth = fdp.ConsumeIntegral<uint32_t>();
    info.maxWindowHeight = fdp.ConsumeIntegral<uint32_t>();
    info.minWindowHeight = fdp.ConsumeIntegral<uint32_t>();
    GetRandomStartWindowResourceInfo(fdp, info.resourceInfo);
    info.CompileMode = static_cast<CompileMode>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.originalBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.appName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.privacyUrl = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.privacyName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.downloadUrl = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.versionName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.className = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.originalClassName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.uriPermissionMode = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.uriPermissionPath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.packageSize = fdp.ConsumeIntegral<uint32_t>();
    info.multiUserShared = fdp.ConsumeBool();
    info.grantPermission = fdp.ConsumeBool();
    info.directLaunch = fdp.ConsumeBool();
    info.AbilitySubType = static_cast<AbilitySubType>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.libPath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.deviceId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.installTime = fdp.ConsumeIntegral<uint64_t>();
    info.supportExtNames = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.supportMimeTypes = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
}

void GetRandomStartWindowResourceInfo(FuzzedDataProvider& fdp, ResourceInfo& resourceInfo)
{
    info.startWindowAppIconId = fdp.ConsumeIntegral<uint32_t>();
    info.startWindowIllustrationId = fdp.ConsumeIntegral<uint32_t>();
    info.startWindowBrandingImageId = fdp.ConsumeIntegral<uint32_t>();
    info.startWindowBackgroundColorId = fdp.ConsumeIntegral<uint32_t>();
    info.startWindowBackgroundImageId = fdp.ConsumeIntegral<uint32_t>();
    info.startWindowBackgroundImageFit = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
}

void GetRandomApplicationInfo(FuzzedDataProvider& fdp, ApplicationInfo& applicationInfo)
{
    info.name = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.versionCode = fdp.ConsumeIntegral<int32_t>();
    info.versionName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.minCompatibleVersionCode = fdp.ConsumeIntegral<int32_t>();
    info.apiCompatibleVersion = fdp.ConsumeIntegral<int32_t>();
    info.apiTargetVersion = fdp.ConsumeIntegral<int32_t>();
    info.crowdtestDeadline = fdp.ConsumeIntegral<int64_t>();
    info.iconPath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.iconId = fdp.ConsumeIntegral<int32_t>();
    GetRandomResourceInfo(fdp, info.resourceInfo);
    info.label = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.labelId = fdp.ConsumeIntegral<int32_t>();
    GetRandomResourceInfo(fdp, info.resourceInfo);
    info.description = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.descriptionId = fdp.ConsumeIntegral<int32_t>();
    GetRandomResourceInfo(fdp, info.resourceInfo);
    info.keepAlive = fdp.ConsumeBool();
    info.removable = fdp.ConsumeBool();
    info.singleton = fdp.ConsumeBool();
    info.userDataClearable = fdp.ConsumeBool();
    info.allowAppRunWhenDeviceFirstLocked = fdp.ConsumeBool();
    info.accessible = fdp.ConsumeBool();
    info.runningResourcesApply = fdp.ConsumeBool();
    info.associatedWakeUp = fdp.ConsumeBool();
    info.hideDesktopIcon = fdp.ConsumeBool();
    info.formVisibleNotify = fdp.ConsumeBool();
    info.installedForAllUser = fdp.ConsumeBool();
    info.allowEnableNotification = fdp.ConsumeBool();
    info.allowMultiProcess = fdp.ConsumeBool();
    info.gwpAsanEnabled = fdp.ConsumeBool();
    info.hasPlugin = fdp.ConsumeBool();
    info.allowCommonEvent = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.assetAccessGroups = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.resourcesApply = fdp.ConsumeIntegral<int32_t>();
    info.isSystemApp = fdp.ConsumeBool();
    info.isLauncherApp = fdp.ConsumeBool();
    info.isFreeInstallApp = fdp.ConsumeBool();
    info.asanEnabled = fdp.ConsumeBool();
    info.asanLogPath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.codePath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.dataDir = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.dataBaseDir = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.cacheDir = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.entryDir = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.apiReleaseType = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.debug = fdp.ConsumeBool();
    info.deviceId = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.distributedNotificationEnabled = fdp.ConsumeBool();
    info.entityType = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.process = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.supportedModes = fdp.ConsumeIntegral<int32_t>();
    info.vendor = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.appPrivilegeLevel = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.appDistributionType = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.appProvisionType = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.accessTokenId = fdp.ConsumeIntegral<int32_t>();
    info.applicationReservedFlag = fdp.ConsumeIntegral<int32_t>();
    info.accessTokenIdEx = fdp.ConsumeIntegral<int64_t>();
    info.enabled = fdp.ConsumeBool();
    info.appIndex = fdp.ConsumeIntegral<int64_t>();
    info.uid = fdp.ConsumeIntegral<int32_t>();
    info.maxChildProcess = fdp.ConsumeIntegral<int32_t>();
    info.applicationFlags = fdp.ConsumeIntegral<int32_t>();
    info.applicationFlags = static_cast<ApplicationInfoFlag>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.MultiAppModeData = static_cast<MultiAppModeData>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.nativeLibraryPath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.cpuAbi = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.arkNativeFilePath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.arkNativeFileAbi = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.permissions = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.moduleSourceDirs = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.moduleInfos = static_cast<ModuleInfo>(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    info.metaData = static_cast<CustomizeData>(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    info.metaData = static_cast<Metadata>(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    info.targetBundleList = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.appEnvironments = static_cast<ApplicationEnvironment>(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    info.hnpPackages = static_cast<HnpPackage>(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    info.fingerprint = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    GetRandomAppQuickFix(fdp, info.appquickfixInfo)
    info.icon = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.flags = fdp.ConsumeIntegral<int32_t>();
    info.entryModuleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.isCompressNativeLibs = fdp.ConsumeBool();
    info.signatureKey = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.multiProjects = fdp.ConsumeBool();
    info.tsanEnabled = fdp.ConsumeBool();
    info.hwasanEnabled = fdp.ConsumeBool();
    info.ubsanEnabled = fdp.ConsumeBool();
    info.cloudFileSyncEnabled = fdp.ConsumeBool();
    info.needAppDetail = fdp.ConsumeBool();
    info.appDetailAbilityLibraryPath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.targetBundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.targetPriority = fdp.ConsumeIntegral<int32_t>();
    info.overlayState = fdp.ConsumeIntegral<int32_t>();
    info.split = fdp.ConsumeBool();
    info.bundleType = static_cast<BundleType>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.compileSdkVersion = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.compileSdkType = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.organization = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.installSource = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.configuration = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
}

void GetRandomResourceInfo(FuzzedDataProvider& fdp, ResourceInfo& resourceInfo)
{
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.id = fdp.ConsumeIntegral<int32_t>();
}

void GetRandomAppQuickFix(FuzzedDataProvider& fdp, AppQuickFixInfo& appquickfixInfo)
{
    info.versionCode = fdp.ConsumeIntegral<int32_t>();
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.versionName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.id = fdp.ConsumeIntegral<int32_t>();
    GetRandomAppqfInfo(fdp, info.appqfInfo);
    GetRandomAppqfInfo(fdp, info.appqfInfo);
}

void GetRandomAppqfInfo(FuzzedDataProvider& fdp, AppqfInfo& appqfInfo)
{
    info.QuickFixType = static_cast<QuickFixType>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.versionCode = fdp.ConsumeIntegral<int32_t>();
    info.versionName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.cpuAbi = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.nativeLibraryPath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    GetRandomHqfInfo(fdp, info.hqfInfo);
}

void GetRandomHqfInfo(FuzzedDataProvider& fdp, HqfInfo& hqfInfo)
{
    info.QuickFixType = static_cast<QuickFixType>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.hapSha256 = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.hqfFilePath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.cpuAbi = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.nativeLibraryPath = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
}

void GetRandomDialogAbilityInfo(FuzzedDataProvider& fdp, DialogAbilityInfo& info)
{
    info.visible = fdp.ConsumeBool();
    info.bundleIconId = fdp.ConsumeIntegral<uint32_t>();
    info.bundleLabelId = fdp.ConsumeIntegral<uint32_t>();
    info.abilityIconId = fdp.ConsumeIntegral<uint32_t>();
    info.abilityLabelId = fdp.ConsumeIntegral<uint32_t>();
    info.appIndex = fdp.ConsumeIntegral<uint32_t>();
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.multiAppMode = static_cast<MultiAppModeData>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    std::string GetURI() const;
    bool ParseURI(const std::string &uri);
    void Split(const std::string &str, const std::string &delim, std::vector<std::string> &vec);
}

void GetRandomDialogSessionInfo(FuzzedDataProvider& fdp, DialogSessionInfo& info)
{
    GetRandomDialogAbilityInfo(fdp, info.info);
    GetRandomDialogAbilityInfo(fdp, info.info);
    AAFwk::WantParams parameters;
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override; 
    static DialogSessionInfo *Unmarshalling(Parcel &parcel);
}

void GetRandomDialogCallerInfo(FuzzedDataProvider& fdp, DialogCallerInfo& info)
{
    info.userId = fdp.ConsumeIntegral<uint32_t>();
    info.requestCode = fdp.ConsumeIntegral<uint32_t>();
    sptr<IRemoteObject> callerToken;
    Want targetWant;
    info.type = static_cast<SelectorType>(fdp.ConsumeIntegralInRange<int32_t>(0, CODE_TWO));
    info.needGrantUriPermission = fdp.ConsumeBool();
}

}  // namespace AbilityFuzzUtil
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // BMS_FUZZTEST_UTIL_H