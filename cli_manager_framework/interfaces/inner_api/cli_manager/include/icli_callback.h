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

#ifndef OHOS_ABILITY_RUNTIME_ICLI_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_ICLI_CALLBACK_H

#include <string>

#include "cli_manager_data.h"
#include "iremote_broker.h"

namespace OHOS {
namespace AbilityRuntime {

enum class CliCallbackCmd {
    ON_OUTPUT = 0,
    ON_EXIT,
    CMD_MAX
};

class ICliCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.abilityruntime.clicallback");

    virtual int32_t OnOutput(const std::string &sessionId, const ToolEvent &event) = 0;
    virtual int32_t OnExit(const std::string &sessionId, int32_t exitCode) = 0;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ICLI_CALLBACK_H
