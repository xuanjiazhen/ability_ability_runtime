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

#ifndef OHOS_ABILITY_RUNTIME_CLI_CALLBACK_PROXY_H
#define OHOS_ABILITY_RUNTIME_CLI_CALLBACK_PROXY_H

#include "icli_callback.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace AbilityRuntime {

class CliCallbackProxy : public IRemoteProxy<ICliCallback> {
public:
    explicit CliCallbackProxy(const sptr<IRemoteObject> &impl);
    ~CliCallbackProxy() override = default;

    int32_t OnOutput(const std::string &sessionId, const ToolEvent &event) override;
    int32_t OnExit(const std::string &sessionId, int32_t exitCode) override;

private:
    static inline BrokerDelegator<CliCallbackProxy> delegator_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_CALLBACK_PROXY_H
