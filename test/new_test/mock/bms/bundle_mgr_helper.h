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

#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H

#include <string>
#include "bundle_info.h"
#include "application_info.h"
#include "oh_mock_utils.h"
#include "singleton.h"

namespace OHOS {
namespace AppExecFwk {

class BundleMgrHelper : public DelayedSingleton<BundleMgrHelper> {
public:
    // GetApplicationInfo - returns bool, output param: ApplicationInfo& appInfo
    OH_MOCK_METHOD_WITH_OUTPUT_1(bool, appInfo, BundleMgrHelper,
        GetApplicationInfo, const std::string&, int32_t, int32_t, ApplicationInfo& appInfo);

    // GetBundleInfo - returns bool, output param: BundleInfo& bundleInfo
    OH_MOCK_METHOD_WITH_OUTPUT_1(bool, bundleInfo, BundleMgrHelper,
        GetBundleInfo, const std::string&, int32_t, BundleInfo& bundleInfo, int32_t);

    // GetCloneBundleInfo - returns ErrCode (int32_t), output param: BundleInfo& bundleInfo
    OH_MOCK_METHOD_WITH_OUTPUT_1(int32_t, bundleInfo, BundleMgrHelper,
        GetCloneBundleInfo, const std::string&, int32_t, int32_t, BundleInfo& bundleInfo, int32_t);

    // GetSandboxBundleInfo - returns ErrCode (int32_t), output param: BundleInfo& bundleInfo
    OH_MOCK_METHOD_WITH_OUTPUT_1(int32_t, bundleInfo, BundleMgrHelper,
        GetSandboxBundleInfo, const std::string&, int32_t, int32_t, BundleInfo& bundleInfo);

    // GetAppIdByBundleName - returns std::string, no output param
    OH_MOCK_METHOD(std::string, BundleMgrHelper, GetAppIdByBundleName, const std::string&, int32_t);
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
