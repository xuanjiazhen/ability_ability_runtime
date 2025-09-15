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

#include "mock_bundle_manager.h"

#include "mock_my_flag.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AAFwk;

sptr<IAppControlMgr> BundleMgrProxy::GetAppControlProxy()
{
    return MyFlag::mockAppControlManager_;
}

bool BundleMgrProxy::QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo)
{
    return true;
}

bool BundleMgrProxy::GetBundleInfo(
    const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
{
    return true;
}

bool BundleMgrProxy::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    return true;
}

int BundleMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    return 0;
}

BundleMgrService::BundleMgrService()
{
}

BundleMgrService::~BundleMgrService()
{
}

bool BundleMgrService::GetBundleInfo(
    const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
{
    return true;
}

bool BundleMgrService::QueryAbilityInfo(const AAFwk::Want &want, AbilityInfo &abilityInfo)
{
    return true;
}

bool BundleMgrService::QueryAbilityInfo(const Want &want, int32_t flags, int32_t userId, AbilityInfo &abilityInfo)
{
    return true;
}

bool BundleMgrService::GetApplicationInfo(
    const std::string &appName, const ApplicationFlag flag, const int userId, ApplicationInfo &appInfo)
{
    return true;
}

bool BundleMgrService::CheckWantEntity(const AAFwk::Want &want, AbilityInfo &abilityInfo)
{
    return true;
}

int BundleMgrService::GetUidByBundleName(const std::string &bundleName, int32_t userId, int32_t appCloneIndex)
{
    return 0;
}
}  // namespace AppExecFwk
}  // namespace OHOS
