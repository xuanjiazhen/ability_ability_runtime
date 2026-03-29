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

#ifndef OHOS_ABILITY_RUNTIME_CLI_MANAGER_STUB_H
#define OHOS_ABILITY_RUNTIME_CLI_MANAGER_STUB_H

#include "icli_manager.h"
#include "iremote_stub.h"
#include "nocopyable.h"

namespace OHOS {
namespace AbilityRuntime {

/**
 * @class CliManagerStub
 * Stub class for CliManager IPC communication
 */
class CliManagerStub : public IRemoteStub<ICliManager> {
public:
    CliManagerStub();
    virtual ~CliManagerStub() = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(CliManagerStub);

    int32_t QueryToolsInner(MessageParcel &data, MessageParcel &reply);
    int32_t ExecToolInner(MessageParcel &data, MessageParcel &reply);
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MANAGER_STUB_H
