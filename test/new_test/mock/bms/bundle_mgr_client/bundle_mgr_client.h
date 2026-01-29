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

#ifndef OHOS_ABILITY_RUNTIME_BUNDLE_MGR_CLIENT_H
#define OHOS_ABILITY_RUNTIME_BUNDLE_MGR_CLIENT_H

#include <memory>
#include <string>
#include "oh_mock_utils.h"
#include "mock_singleton.h"

namespace OHOS {
namespace AppExecFwk {

class BundleMgrClient {
public:
    OH_MOCK_METHOD_WITH_OUTPUT_1(int32_t, dirName, BundleMgrClient, GetDirByBundleNameAndAppIndex,
        const std::string&, int32_t, std::string& dirName);
};

// Global mock for DelayedSingleton<BundleMgrClient>::GetInstance()
OH_MOCK_GLOBAL_METHOD_WITH_DECORATOR(static, BundleMgrClient*, DelayedSingleton_BundleMgrClient_GetInstance);

}  // namespace AppExecFwk
}  // namespace OHOS

// Specialization for BundleMgrClient
template<>
class OHOS::AppExecFwk::DelayedSingleton<OHOS::AppExecFwk::BundleMgrClient> {
public:
    static OHOS::AppExecFwk::BundleMgrClient* GetInstance() {
        return OHOS::AppExecFwk::DelayedSingleton_BundleMgrClient_GetInstance();
    }
};

#endif  // OHOS_ABILITY_RUNTIME_BUNDLE_MGR_CLIENT_H
