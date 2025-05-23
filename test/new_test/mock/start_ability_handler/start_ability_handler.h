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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_START_ABILITY_HANDLER_H
#define MOCK_OHOS_ABILITY_RUNTIME_START_ABILITY_HANDLER_H
#include <memory>
#include <optional>

#include "ability_record.h"
#include "event_report.h"
#include "want.h"
#include "refbase.h"

namespace OHOS {
namespace AAFwk {
struct StartAbilityParams {
    StartAbilityParams(Want &reqWant) : want(reqWant) {}
    bool isStartAsCaller = false;
    int32_t userId = -1;
    int requestCode = 0;
    sptr<IRemoteObject> callerToken;
    sptr<IRemoteObject> asCallerSourceToken;
    const StartOptions* startOptions = nullptr;
    Want &want;

    int32_t GetValidUserId()
    {
        return 0;
    }
    void SetValidUserId(int32_t value) {}

    std::shared_ptr<AbilityRecord> GetCallerRecord()
    {
        return nullptr;
    }

    EventInfo BuildEventInfo()
    {
        EventInfo info;
        return info;
    }
};

class StartAbilityHandler {
public:
    StartAbilityHandler() {}
    virtual ~StartAbilityHandler() {}
    virtual bool MatchStartRequest(StartAbilityParams &params)
    {
        return false;
    }
    virtual int HandleStartRequest(StartAbilityParams &params)
    {
        return 0;
    }
    virtual int GetPriority()
    {
        return 0;
    }
    virtual std::string GetHandlerName()
    {
        return "";
    }
};
} // namespace AAFwk
} // namespace OHOS
#endif