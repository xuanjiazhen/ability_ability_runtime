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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_WANT_UTILS_H
#define MOCK_OHOS_ABILITY_RUNTIME_WANT_UTILS_H

#include "want.h"

namespace OHOS {
namespace AAFwk {
class WantUtils final {
public:
    static int32_t ConvertToExplicitWant(Want& want)
    {
        return 0;
    }

    static int32_t GetCallerBundleName(std::string &callerBundleName)
    {
        return 0;
    }

    static bool IsAtomicServiceUrl(const Want& want)
    {
        return false;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
