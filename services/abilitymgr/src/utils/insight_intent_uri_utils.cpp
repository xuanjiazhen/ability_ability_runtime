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

#include "insight_intent_uri_utils.h"

#include <vector>

#include "insight_intent_execute_param.h"
#include "uri_utils.h"

namespace OHOS {
namespace AAFwk {
#ifdef SUPPORT_UPMS
void GrantFileEntityUriPermission(const Want &want, const std::string &targetBundleName,
    int32_t appIndex, uint32_t callerTokenId)
{
    if (!AppExecFwk::InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        return;
    }
    std::vector<std::vector<std::string>> uriGroups;
    std::vector<int32_t> flags;
    AppExecFwk::InsightIntentExecuteParam::GetFileEntityUrisAndFlags(want, uriGroups, flags);
    for (size_t i = 0; i < uriGroups.size() && i < flags.size(); i++) {
        UriUtils::GetInstance().GrantUriPermission(uriGroups[i], flags[i],
            targetBundleName, appIndex, callerTokenId);
    }
}
#endif  // SUPPORT_UPMS
}  // namespace AAFwk
}  // namespace OHOS