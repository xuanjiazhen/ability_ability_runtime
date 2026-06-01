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

#ifndef OHOS_ABILITY_RUNTIME_SKILL_CALLBACK_STUB_LITE_H
#define OHOS_ABILITY_RUNTIME_SKILL_CALLBACK_STUB_LITE_H

#include <string>

#include "ipc_object_stub.h"
#include "skill_execute_types.h"

namespace OHOS {
namespace CliTool {

class SkillCallbackLite : public IPCObjectStub {
public:
    SkillCallbackLite() : IPCObjectStub(u"ohos.AAFwk.SkillExecuteCallback") {}
    ~SkillCallbackLite() override = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data,
        MessageParcel &reply, MessageOption &option) override;

    virtual void OnExecuteDone(const std::string &requestCode, int32_t resultCode,
        const AppExecFwk::SkillExecuteResult &result) = 0;

private:
    int32_t OnExecuteDoneInner(MessageParcel &data, MessageParcel &reply);

    static constexpr uint32_t ON_SKILL_EXECUTE_DONE = 1;
};

} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_SKILL_CALLBACK_STUB_LITE_H
