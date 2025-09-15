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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H

#include <vector>
#include <gmock/gmock.h>

#include "bundlemgr/bundle_mgr_interface.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AppExecFwk {
class BundleMgrProxy : public IRemoteProxy<IBundleMgr> {
public:
    explicit BundleMgrProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBundleMgr>(impl)
    {}
    virtual ~BundleMgrProxy()
    {}

    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override;

    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;

    bool QueryAbilityInfo(const Want& want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo,
        const sptr<IRemoteObject> &callback) override;

    bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo& bundleInfo, int32_t userId) override;

    bool GetBundleInfos(const BundleFlag flag,
        std::vector<BundleInfo> &bundleInfos, int32_t userId = Constants::UNSPECIFIED_USERID) override;

    virtual bool ImplicitQueryInfoByPriority(const Want &want, int32_t flags, int32_t userId,
        AbilityInfo &abilityInfo, ExtensionAbilityInfo &extensionInfo) override;

    ErrCode GetBaseSharedBundleInfos(const std::string &bundleName,
        std::vector<BaseSharedBundleInfo> &baseSharedBundleInfos,
        AppExecFwk::GetDependentBundleInfoFlag flag) override;
};

class BundleMgrStub : public IRemoteStub<IBundleMgr> {
public:
    virtual ~BundleMgrStub() {}
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
};

class BundleMgrService : public BundleMgrStub {
public:
    BundleMgrService();
    ~BundleMgrService();

    bool QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo) override;
    bool QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo) override;
    bool GetApplicationInfo(
        const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo) override;
    bool GetBundleInfo(
        const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId) override;
    int GetUidByBundleName(const std::string &bundleName, int32_t userId, int32_t appCloneIndex) override;

    bool CheckWantEntity(const AAFwk::Want&, AbilityInfo&);

    virtual bool ImplicitQueryInfoByPriority(const Want &want, int32_t flags, int32_t userId,
        AbilityInfo &abilityInfo, ExtensionAbilityInfo &extensionAbilityInfo) override;

    virtual sptr<IAppControlMgr> GetAppControlProxy() override;

    ErrCode GetBaseSharedBundleInfos(const std::string &bundleName,
        std::vector<BaseSharedBundleInfo> &baseSharedBundleInfos,
        AppExecFwk::GetDependentBundleInfoFlag flag) override;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_BUNDLE_MANAGER_H
