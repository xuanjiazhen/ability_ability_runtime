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

#ifndef MOCK_BUNDLE_MGR_CLIENT_H
#define MOCK_BUNDLE_MGR_CLIENT_H

#include "oh_mock_utils.h"
#include <string>
#include <vector>

namespace OHOS {
namespace AppExecFwk {

struct BundleInfo {
    std::string name;
    std::string label;
    int32_t uid;
    uint32_t accessTokenId;
    bool isSystemApp;
};

struct ApplicationInfo {
    std::string name;
    std::string bundleName;
    bool isSystemApp;
};

class MockBundleMgrClient {
public:
    OH_MOCK_METHOD(bool, MockBundleMgrClient, GetBundleInfo, const std::string& bundleName, int32_t flag,
                   BundleInfo& bundleInfo);

    OH_MOCK_METHOD(bool, MockBundleMgrClient, GetBundleInfo, uint32_t accessTokenId, BundleInfo& bundleInfo);

    OH_MOCK_METHOD(bool, MockBundleMgrClient, GetBundleNameForUid, const int uid, std::string& bundleName);

    OH_MOCK_METHOD(int32_t, MockBundleMgrClient, GetUidByBundleName, const std::string& bundleName, int32_t userId);

    OH_MOCK_METHOD(bool, MockBundleMgrClient, GetApplicationInfo, const std::string& appName, int32_t flag,
                   int32_t userId, ApplicationInfo& appInfo);

    OH_MOCK_METHOD(bool, MockBundleMgrClient, GetBundleInfosByMetaData, const std::string& metaData,
                   std::vector<BundleInfo>& bundleInfos);

    OH_MOCK_METHOD(bool, MockBundleMgrClient, QueryExtensionAbilityInfos, const Want& want, int32_t flag,
                   int32_t userId, std::vector<ExtensionAbilityInfo>& extensionInfos);
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // MOCK_BUNDLE_MGR_CLIENT_H
