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

#include "extension_query_event_util.h"

#include <string>

#include "event_report.h"
#include "extension_ability_info.h"

namespace OHOS {
namespace AAFwk {
bool NeedReportExtensionQueryMultiResult(
    const std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos)
{
    if (extensionInfos.size() <= 1) {
        return false;
    }
    if (extensionInfos[0].applicationInfo.isSystemApp) {
        return false;
    }
    for (const auto &info : extensionInfos) {
        if (info.applicationInfo.isSystemApp) {
            return true;
        }
    }
    return false;
}

void ReportExtensionQueryMultiResult(
    const std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos, bool isSandbox,
    int32_t callingUid, const std::string &callerBundleName)
{
    if (extensionInfos.size() <= 1) {
        return;
    }
    std::string bundleNames;
    std::string abilityNames;
    for (const auto &info : extensionInfos) {
        if (!bundleNames.empty()) {
            bundleNames += "-";
            abilityNames += "-";
        }
        bundleNames += info.bundleName;
        abilityNames += info.name;
    }
    EventInfo eventInfo;
    eventInfo.moduleName = isSandbox ? "GetSandboxExtAbilityInfos" : "QueryExtensionAbilityInfos";
    eventInfo.bundleName = bundleNames;
    eventInfo.abilityName = abilityNames;
    eventInfo.extensionType = static_cast<int32_t>(extensionInfos[0].type);
    eventInfo.callerBundleName = callerBundleName.empty() ? std::to_string(callingUid) : callerBundleName;
    EventReport::SendStartAbilityOtherExtensionEvent(
        EventName::START_ABILITY_OTHER_EXTENSION, eventInfo);
}
}
}
