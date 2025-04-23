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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_URI_UTILS_H
#define MOCK_OHOS_ABILITY_RUNTIME_URI_UTILS_H

#include <string>
#include <vector>

#include "ability_record.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class UriUtils {
public:
    static UriUtils &GetInstance()
    {
        static UriUtils instance;
        return instance;
    }

    int32_t CheckNonImplicitShareFileUri(const Want &want, int32_t userId, uint32_t specifyTokenId)
    {
        return 0;
    }

#ifdef SUPPORT_UPMS
    void GrantUriPermission(Want &want, std::string targetBundleName, int32_t appIndex,
        bool isSandboxApp, uint32_t callerTokenId, int32_t collaboratorType) {}

    void CheckUriPermission(uint32_t callerTokenId, Want &want) {}

    void GrantUriPermission(const std::vector<std::string> &uriVec, int32_t flag,
        const std::string &targetBundleName, int32_t appIndex, uint32_t initiatorTokenId) {}
#endif // SUPPORT_UPMS
};
} // namespace AAFwk
} // namespace OHOS
#endif