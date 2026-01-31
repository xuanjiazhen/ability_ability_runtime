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
#include "oh_mock_utils.h"
#include "singleton.h"

namespace OHOS {
namespace AppExecFwk {

class BundleMgrHelper : public DelayedSingleton<BundleMgrHelper> {
public:
    OH_MOCK_METHOD(bool, BundleMgrHelper, GetApplicationInfo, const std::string&, int32_t, int32_t, ApplicationInfo&);

    OH_MOCK_METHOD(bool, BundleMgrHelper, GetBundleInfo, const std::string&, int32_t, BundleInfo&, int32_t);

    OH_MOCK_METHOD(int32_t, BundleMgrHelper, GetCloneBundleInfo, const std::string&, int32_t, int32_t, BundleInfo&, int32_t);

    OH_MOCK_METHOD(int32_t, BundleMgrHelper, GetSandboxBundleInfo, const std::string&, int32_t, int32_t, BundleInfo&);

    OH_MOCK_METHOD(std::string, BundleMgrHelper, GetAppIdByBundleName, const std::string&, int32_t);
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_HELPER_H
