/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "mock_bundle_mgr_helper.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t TOKENID_LENGTH = 4;
bool IsDigitString(std::string name)
{
    return true;
}
}

BundleMgrHelper::BundleMgrHelper() {}

BundleMgrHelper::~BundleMgrHelper()
{}

void BundleMgrHelper::OnDeath()
{}

ErrCode BundleMgrHelper::GetSandboxBundleInfo(
    const std::string &bundleName, int32_t appIndex, int32_t userId, BundleInfo &info)
{
    return ERR_OK;
}

bool BundleMgrHelper::GetBundleInfo(const std::string &bundleName, const BundleFlag flags,
    BundleInfo &bundleInfo, int32_t userId)
{
    if (bundleName.size() < TOKENID_LENGTH) {
        return false;
    }

    if (bundleName == "com.test.test1") {
        bundleInfo.isKeepAlive = true;
        bundleInfo.applicationInfo.process = "com.test.test1";
        return true;
    }

    return true;
}

bool BundleMgrHelper::GetApplicationInfo(const std::string &appName, uint32_t flags,
    int32_t userId, ApplicationInfo &appInfo)
{
    return true;
}

std::string BundleMgrHelper::GetAppIdByBundleName(const std::string &bundleName, const int32_t userId)
{
    auto appId = bundleName + "_appId";
    return appId;
}

ErrCode BundleMgrHelper::GetCloneBundleInfo(const std::string &bundleName, int32_t flags, int32_t appCloneIndex,
    BundleInfo &bundleInfo, int32_t userId)
{
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS