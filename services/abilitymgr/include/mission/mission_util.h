/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MISSION_UTIL_H
#define OHOS_ABILITY_RUNTIME_MISSION_UTIL_H

#include "ability_record.h"
#include "event_report.h"

namespace OHOS::AbilityRuntime {
namespace MissionUtil {
// NANOSECONDS mean 10^9 nano second
constexpr int64_t NANOSECONDS = 1000000000;
// MICROSECONDS mean 10^6 milli second
constexpr int64_t MICROSECONDS = 1000000;
constexpr int64_t SEC_TO_MILLISEC = 1000;
constexpr int64_t MAX_TIME_BUFF = 64; // 64 : for example 2021-05-27-01-01-01
constexpr int32_t DECIMAL_BASE = 10;

static FreezeUtil::TimeoutState MsgId2State(uint32_t msgId)
{
    if (msgId == AbilityManagerService::LOAD_TIMEOUT_MSG) {
        return FreezeUtil::TimeoutState::LOAD;
    } else if (msgId == AbilityManagerService::FOREGROUND_TIMEOUT_MSG) {
        return FreezeUtil::TimeoutState::FOREGROUND;
    } else if (msgId == AbilityManagerService::BACKGROUND_TIMEOUT_MSG) {
        return FreezeUtil::TimeoutState::BACKGROUND;
    }
    return FreezeUtil::TimeoutState::UNKNOWN;
}

static bool GetContentAndTypeId(uint32_t msgId, std::string &msgContent, int &typeId) const
{
    switch (msgId) {
        case AbilityManagerService::LOAD_TIMEOUT_MSG:
            msgContent += "load timeout";
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AbilityManagerService::ACTIVE_TIMEOUT_MSG:
            msgContent += "active timeout";
            break;
        case AbilityManagerService::INACTIVE_TIMEOUT_MSG:
            msgContent += "inactive timeout";
            break;
        case AbilityManagerService::FOREGROUND_TIMEOUT_MSG:
            msgContent += "foreground timeout";
            typeId = AppExecFwk::AppfreezeManager::TypeAttribute::CRITICAL_TIMEOUT;
            break;
        case AbilityManagerService::BACKGROUND_TIMEOUT_MSG:
            msgContent += "background timeout";
            break;
        case AbilityManagerService::TERMINATE_TIMEOUT_MSG:
            msgContent += "terminate timeout";
            break;
        default:
            return false;
    }
    return true;
}

static std::string GetMissionName(const AbilityRequest &abilityRequest) const
{
    int32_t appIndex = 0;
    (void)AbilityRuntime::StartupUtil::GetAppIndex(abilityRequest.want, appIndex);
    return AbilityUtil::ConvertBundleNameSingleton(abilityRequest.abilityInfo.bundleName,
        abilityRequest.abilityInfo.name, abilityRequest.abilityInfo.moduleName, appIndex);
}

static int32_t CallType2StartMethod(int32_t callType)
{
    switch (callType) {
        case AbilityCallType::INVALID_TYPE:
            return static_cast<int32_t>(StartMethod::START_NORMAL);
        case AbilityCallType::CALL_REQUEST_TYPE:
            return static_cast<int32_t>(StartMethod::START_CALL);
        default:
            break;
    }
    return -1;
}

static bool CallTypeFilter(int32_t callType)
{
    switch (callType) {
        case AbilityCallType::CALL_REQUEST_TYPE:
            return true;
        default:
            break;
    }
    return false;
}

static void SendKeyEvent(const AbilityRequest &abilityRequest)
{
    auto abilityInfo = abilityRequest.abilityInfo;
    EventInfo eventInfo;
    eventInfo.abilityName = abilityInfo.name;
    eventInfo.bundleName = abilityInfo.bundleName;
    eventInfo.moduleName = abilityInfo.moduleName;
    EventReport::SendKeyEvent(EventName::START_PRIVATE_ABILITY, HiSysEventType::BEHAVIOR, eventInfo);
}
}  // namespace MissionUtil
}  // namespace OHOS::AbilityRuntime
#endif  // OHOS_ABILITY_RUNTIME_MISSION_UTIL_H