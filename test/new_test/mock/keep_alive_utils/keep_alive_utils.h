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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_KEEP_ALIVE_UTILS_H
#define MOCK_OHOS_ABILITY_RUNTIME_KEEP_ALIVE_UTILS_H

#include "bundle_info.h"

namespace OHOS {
namespace AAFwk {
enum class KeepAliveType : int32_t {
    UNSPECIFIED = -1,
    RESIDENT_PROCESS = 0,
    THIRD_PARTY = 1,
};

class KeepAliveUtils final {
public:
    static bool IsKeepAliveBundle(const AppExecFwk::BundleInfo &bundleInfo, int32_t userId, KeepAliveType &type)
    {
        return false;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
