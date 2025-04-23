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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_EVENT_REPORT_H
#define MOCK_OHOS_ABILITY_RUNTIME_EVENT_REPORT_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
class HiSysEvent {
public:
    enum EventType {
        FAULT = 1,
        STATISTIC = 2,
        SECURITY = 3,
        BEHAVIOR = 4
    };
    class Domain {
    public:
        static constexpr char AAFWK[] = "AAFWK";
    };
};
}

using HiSysEventType = HiviewDFX::HiSysEvent::EventType;
using HiSysEvent = HiviewDFX::HiSysEvent;

#define HiSysEventWrite(domain, eventName, type, ...) \

namespace AAFwk {
struct EventInfo {
    bool isPreload = false;
    int32_t pid = -1;
    int32_t userId = -1;
    int32_t extensionType = -1;
    int32_t abilityNumber = 0;
    int32_t abilityType = -1;
    int32_t callerUid = -1;
    int32_t exitResult = -1;
    int32_t bundleType = -1;
    int32_t startType = 0;
    int32_t startReason = 0;
    int32_t appUid = -1;
    int32_t appIndex = -1;
    int32_t errCode = -1;
    int32_t persistentId = 0;
    int32_t callerState = -1;
    int32_t processType = -1;
    int32_t callerPid = -1;
    int64_t duration = 0;
    int32_t reason = -1;
    int32_t subReason = -1;
    int32_t exitReason = -1;
    int32_t preloadMode = 0;
    uint32_t versionCode = 0;
    uint32_t callerVersionCode = -1;
    int64_t time = 0;
    std::string versionName;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string processName;
    std::string callerProcessName;
    std::string callerBundleName;
    std::string callerVersionName;
    std::string uri;
    std::string lifeCycleState;
    std::string errReason;
    std::string lifeCycle;
    std::string intentName;
};

enum class EventName {
    START_ABILITY_ERROR = 0,
    TERMINATE_ABILITY_ERROR,
    START_EXTENSION_ERROR,
    STOP_EXTENSION_ERROR,
    CONNECT_SERVICE_ERROR,
    DISCONNECT_SERVICE_ERROR,
    UI_SERVICE_EXTENSION_ERROR,
    EXECUTE_INSIGHT_INTENT_ERROR,
    START_ABILITY,
    TERMINATE_ABILITY,
    CLOSE_ABILITY,
    ABILITY_ONFOREGROUND,
    ABILITY_ONBACKGROUND,
    ABILITY_ONACTIVE,
    ABILITY_ONINACTIVE,
    START_ABILITY_BY_APP_LINKING,
    START_SERVICE,
    STOP_SERVICE,
    CONNECT_SERVICE,
    DISCONNECT_SERVICE,
    START_ABILITY_OTHER_EXTENSION,
    APP_ATTACH,
    APP_LAUNCH,
    APP_FOREGROUND,
    APP_BACKGROUND,
    APP_TERMINATE,
    PROCESS_START,
    PROCESS_EXIT,
    DRAWN_COMPLETED,
    APP_STARTUP_TYPE,
    UI_EXTENSION_ERROR,
    PROCESS_START_FAILED,
    GRANT_URI_PERMISSION,
    FA_SHOW_ON_LOCK,
    START_PRIVATE_ABILITY,
    RESTART_PROCESS_BY_SAME_APP,
    START_STANDARD_ABILITIES,
    CREATE_ATOMIC_SERVICE_PROCESS,
    ATOMIC_SERVICE_DRAWN_COMPLETE,
    SHARE_UNPRIVILEGED_FILE_URI
};

class EventReport {
public:
    static void SendAppEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo) {}
    static void SendAbilityEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo) {}
    static void SendExtensionEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo) {}
    static void SendStartAbilityOtherExtensionEvent(const EventName &eventName, const EventInfo &eventInfo) {}
    static void SendExecuteIntentEvent(const EventName &eventName, HiSysEventType type, const EventInfo &eventInfo) {}
};
}  // namespace AAFWK
}  // namespace OHOS
#endif
