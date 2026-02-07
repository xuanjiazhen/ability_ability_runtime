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

#ifndef OHOS_AGENT_RUNTIME_ETS_AGENT_EXTENSION_STUB_IMPL_H
#define OHOS_AGENT_RUNTIME_ETS_AGENT_EXTENSION_STUB_IMPL_H

#include "agent_receiver_stub.h"

namespace OHOS {
namespace AgentRuntime {
class EtsAgentExtension;

class EtsAgentExtensionStubImpl : public AgentReceiverStub {
public:
    explicit EtsAgentExtensionStubImpl(std::weak_ptr<EtsAgentExtension>& ext);
    virtual ~EtsAgentExtensionStubImpl();
    virtual int32_t SendData(const sptr<IRemoteObject> &hostProxy, const std::string &data) override;
    virtual int32_t Authorize(const sptr<IRemoteObject> &hostProxy, const std::string &data) override;

protected:
    std::weak_ptr<EtsAgentExtension> extension_;
};
} // namespace AgentRuntime
} // namespace OHOS
#endif //OHOS_AGENT_RUNTIME_ETS_AGENT_EXTENSION_STUB_IMPL_H
