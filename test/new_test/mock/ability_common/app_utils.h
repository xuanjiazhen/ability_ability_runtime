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

#ifndef OHOS_ABILITY_RUNTIME_APP_UTILS_H
#define OHOS_ABILITY_RUNTIME_APP_UTILS_H

#include <mutex>
#include <string>

#include "nocopyable.h"
#include "oh_mock_utils.h"

namespace OHOS {
namespace AAFwk {

/**
 * @class AppUtils
 * provides app utils.
 */
class AppUtils {
public:
    static AppUtils &GetInstance()
    {
        static AppUtils utils;
        return utils;
    }
    ~AppUtils() {}

    OH_MOCK_METHOD(bool, AppUtils, IsLauncher, const std::string&);
    OH_MOCK_METHOD(bool, AppUtils, IsLauncherAbility, const std::string&);
    OH_MOCK_METHOD(bool, AppUtils, IsInheritWindowSplitScreenMode);
    OH_MOCK_METHOD(bool, AppUtils, IsSupportAncoApp);
    OH_MOCK_METHOD(bool, AppUtils, GetTimeoutUnitTimeRatio);
    OH_MOCK_METHOD(bool, AppUtils, IsSelectorDialogDefaultPossion);
    OH_MOCK_METHOD(bool, AppUtils, IsStartSpecifiedProcess);
    OH_MOCK_METHOD(bool, AppUtils, IsUseMultiRenderProcess);
    OH_MOCK_METHOD(bool, AppUtils, IsLimitMaximumOfRenderProcess);
    OH_MOCK_METHOD(bool, AppUtils, IsGrantPersistUriPermission);
    OH_MOCK_METHOD(bool, AppUtils, IsSupportGrantUriPermission);
    OH_MOCK_METHOD(bool, AppUtils, IsStartOptionsWithAnimation);
    OH_MOCK_METHOD(bool, AppUtils, IsMultiProcessModel);
    OH_MOCK_METHOD(bool, AppUtils, AllowChildProcessInMultiProcessFeatureApp);
    OH_MOCK_METHOD(bool, AppUtils, IsStartOptionsWithProcessOptions);
    OH_MOCK_METHOD(bool, AppUtils, EnableMoveUIAbilityToBackgroundApi);
    OH_MOCK_METHOD(bool, AppUtils, IsLaunchEmbededUIAbility);
    OH_MOCK_METHOD(bool, AppUtils, IsSupportNativeChildProcess);
    OH_MOCK_METHOD(bool, AppUtils, IsSupportMultiInstance);
    OH_MOCK_METHOD(bool, AppUtils, IsAllowNativeChildProcess, const std::string&);
    OH_MOCK_METHOD(bool, AppUtils, GetLimitMaximumExtensionsPerProc);
    OH_MOCK_METHOD(bool, AppUtils, IsBigMemoryUnrelatedKeepAliveProc, const std::string&);
    OH_MOCK_METHOD(bool, AppUtils, IsRequireBigMemoryProcess, const std::string&);
    OH_MOCK_METHOD(bool, AppUtils, GetLimitMaximumExtensionsPerDevice);
    OH_MOCK_METHOD(bool, AppUtils, GetCacheExtensionTypeList);
    OH_MOCK_METHOD(std::string, AppUtils, GetAncoAppIdentifiers);
    OH_MOCK_METHOD(bool, AppUtils, IsAllowStartAbilityWithoutCallerToken, const std::string&, const std::string&);
    OH_MOCK_METHOD(std::string, AppUtils, GetBrokerDelegateBundleName);
    OH_MOCK_METHOD(int32_t, AppUtils, GetCollaboratorBrokerUID);
    OH_MOCK_METHOD(int32_t, AppUtils, GetCollaboratorBrokerReserveUID);
    OH_MOCK_METHOD(int32_t, AppUtils, MaxChildProcess);
    OH_MOCK_METHOD(int32_t, AppUtils, MaxMultiProcessFeatureChildProcess);
    OH_MOCK_METHOD(std::string, AppUtils, GetMigrateClientBundleName);
    OH_MOCK_METHOD(bool, AppUtils, IsConnectSupportCrossUser);
    OH_MOCK_METHOD(bool, AppUtils, IsSupportAppServiceExtension);
    OH_MOCK_METHOD(bool, AppUtils, IsPrepareTerminateEnabled);
    OH_MOCK_METHOD(bool, AppUtils, IsCacheExtensionAbilityByList);
    OH_MOCK_METHOD(bool, AppUtils, IsSystemReasonMessage, const std::string&);
    OH_MOCK_METHOD(bool, AppUtils, InResidentWhiteList, const std::string&);
    OH_MOCK_METHOD(const std::vector<std::string>&, AppUtils, GetResidentWhiteList);

    bool IsAllowResidentInExtremeMemory(const std::string& bundleName, const std::string& abilityName = "")
    {
        return false;
    }

private:
    AppUtils() {}
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_UTILS_H