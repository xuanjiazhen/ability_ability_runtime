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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
#define MOCK_OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

#include <singleton.h>
#include "ability_bundle_event_callback.h"
#include "ability_manager_errors.h"
#include "bundle_info.h"
#include "ability_info.h"
#include "application_info.h"

namespace OHOS {
constexpr static int REPOLL_TIME_MICRO_SECONDS = 1000000;
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;

class IBundleEventCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.appexecfwk.BundleEventCallback");
};

class BundleMgrHelper : public std::enable_shared_from_this<BundleMgrHelper> {
public:
    void PreConnect() {}
    void ConnectTillSuccess() {}
    void SetBmsReady(bool bmsReady) {}
    ErrCode GetNameForUid(const int32_t uid, std::string &name)
    {
        return 0;
    }

    ErrCode GetNameAndIndexForUid(const int32_t uid, std::string &bundleName, int32_t &appIndex)
    {
        return 0;
    }

    bool GetBundleInfo(const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
    {
        return false;
    }

    bool GetBundleInfo(const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
    {
        return false;
    }
    ErrCode GetBundleInfoForSelf(int32_t flags, BundleInfo &bundleInfo)
    {
        return 0;
    }

    bool QueryAbilityInfo(const Want &want, AbilityInfo &abilityInfo)
    {
        return false;
    }
    bool QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo)
    {
        return false;
    }

    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo &appInfo)
    {
        return false;
    }
    bool GetApplicationInfo(const std::string &appName, int32_t flags, int32_t userId, ApplicationInfo &appInfo)
    {
        return false;
    }
    bool GetApplicationInfoWithAppIndex(const std::string &appName, int32_t appIndex,
        int32_t userId, ApplicationInfo &appInfo)
    {
        return false;
    }
    bool UnregisterBundleEventCallback(const sptr<IBundleEventCallback> &bundleEventCallback)
    {
        return false;
    }
    bool QueryExtensionAbilityInfoByUri(
        const std::string &uri, int32_t userId, ExtensionAbilityInfo &extensionAbilityInfo)
    {
        return false;
    }
    bool ImplicitQueryInfoByPriority(const Want &want, int32_t flags, int32_t userId,
        AbilityInfo &abilityInfo, ExtensionAbilityInfo &extensionInfo)
    {
        return false;
    }
    bool QueryAbilityInfoByUri(const std::string &abilityUri, int32_t userId, AbilityInfo &abilityInfo)
    {
        return false;
    }
    bool QueryAbilityInfo(const Want &want,
        int32_t flags, int32_t userId, AbilityInfo &abilityInfo, const sptr<IRemoteObject> &callBack)
    {
        return false;
    }
    bool RegisterBundleEventCallback(const sptr<IBundleEventCallback> &bundleEventCallback)
    {
        return false;
    }
    ErrCode QueryExtensionAbilityInfosOnlyWithTypeName(const std::string &extensionTypeName,
        const uint32_t flag, const int32_t userId, std::vector<ExtensionAbilityInfo> &extensionInfos)
    {
        return 0;
    }
    ErrCode GetLaunchWantForBundle(const std::string &bundleName, Want &want, int32_t userId)
    {
        return 0;
    }
    ErrCode GetCloneBundleInfo(const std::string &bundleName, int32_t flags, int32_t appCloneIndex,
        BundleInfo &bundleInfo, int32_t userId)
    {
        return 0;
    }

    ErrCode QueryCloneAbilityInfo(const ElementName &element, int32_t flags, int32_t appCloneIndex,
        AbilityInfo &abilityInfo, int32_t userId)
    {
        return 0;
    }

    ErrCode RegisterPluginEventCallback(sptr<IBundleEventCallback> pluginEventCallback)
    {
        return 0;
    }

    ErrCode UnregisterPluginEventCallback(sptr<IBundleEventCallback> pluginEventCallback)
    {
        return 0;
    }

    ErrCode GetCloneBundleInfoExt(const std::string &bundleName, uint32_t flags, int32_t appIndex,
        int32_t userId, BundleInfo &bundleInfo)
    {
        return 0;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif