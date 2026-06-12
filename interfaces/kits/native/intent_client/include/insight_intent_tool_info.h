/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_TOOL_INFO_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_TOOL_INFO_H

#include <string>

namespace OHOS {
namespace AAFwk {

struct InsightIntentToolInfo {
    std::string bundleName;
    std::string functionName;
    std::string description;
    std::string inputSchema;   // JSON Schema, includes ohos.insightIntent.options
    std::string outputSchema;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_TOOL_INFO_H
