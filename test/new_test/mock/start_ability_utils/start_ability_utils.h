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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H
#define MOCK_OHOS_ABILITY_RUNTIME_START_ABILITY_UTILS_H

#include <memory>
#include <string>

#include "ability_info.h"
#include "extension_ability_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

struct StartAbilityInfo {
    static std::shared_ptr<StartAbilityInfo> CreateStartAbilityInfo(const Want &want, int32_t userId,
        int32_t appIndex, sptr<IRemoteObject> callerToken)
    {
        return nullptr;
    }

    static std::shared_ptr<StartAbilityInfo> CreateStartExtensionInfo(const Want &want, int32_t userId,
        int32_t appIndex)
    {
        return nullptr;
    }

    std::string GetAppBundleName() const
    {
        return "";
    }
    int32_t status = ERR_OK;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ExtensionProcessMode extensionProcessMode = AppExecFwk::ExtensionProcessMode::UNDEFINED;
    std::string customProcess;
};

struct StartAbilityUtils {
    static bool GetAppIndex(const Want &want, sptr<IRemoteObject> callerToken, int32_t &appIndex)
    {
        return false;
    }
    static bool GetApplicationInfo(const std::string &bundleName, int32_t userId,
        AppExecFwk::ApplicationInfo &appInfo)
    {
        return false;
    }
    static bool GetCallerAbilityInfo(const sptr<IRemoteObject> &callerToken,
        AppExecFwk::AbilityInfo &abilityInfo)
    {
        return false;
    }
    static int32_t CheckAppProvisionMode(const Want& want, int32_t userId)
    {
        return 0;
    }
    static int32_t CheckAppProvisionMode(const std::string& bundleName, int32_t userId)
    {
        return 0;
    }
    static std::vector<int32_t> GetCloneAppIndexes(const std::string &bundleName, int32_t userId)
    {
        std::vector<int32_t> ret;
        return ret;
    }

    static bool IsCallFromAncoShellOrBroker(const sptr<IRemoteObject> &callerToken)
    {
        return false;
    }
    static bool ermsSupportBackToCallerFlag;
    static std::shared_ptr<StartAbilityInfo> startAbilityInfo;
    static bool skipErms;
};

bool StartAbilityUtils::ermsSupportBackToCallerFlag = false;
bool StartAbilityUtils::skipErms = false;
std::shared_ptr<StartAbilityInfo> StartAbilityUtils::startAbilityInfo = nullptr;

struct StartAbilityInfoWrap {
    StartAbilityInfoWrap(const Want &want, int32_t validUserId, int32_t appIndex,
        const sptr<IRemoteObject> &callerToken, bool isExtension = false) {}
    StartAbilityInfoWrap() {}
    ~StartAbilityInfoWrap() {}
    void SetStartAbilityInfo(const AppExecFwk::AbilityInfo& abilityInfo) {}
};
}
}
#endif