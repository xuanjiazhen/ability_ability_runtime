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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_URI_UTILS_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_URI_UTILS_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace AAFwk {

class Want;

#ifdef SUPPORT_UPMS
/**
 * @brief Grant FileEntity URIs carried in an intent execution Want, grouped by their own flag,
 *        reusing the generic grant in UriUtils.
 * @param want The Want carrying intent custom parameters.
 * @param targetBundleName Bundle name of the grant target.
 * @param appIndex Application index of the grant target.
 * @param callerTokenId Token ID of the caller.
 */
void GrantFileEntityUriPermission(const Want &want, const std::string &targetBundleName,
    int32_t appIndex, uint32_t callerTokenId);
#endif  // SUPPORT_UPMS

}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_URI_UTILS_H