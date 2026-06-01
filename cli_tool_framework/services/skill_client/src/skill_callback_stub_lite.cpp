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

#include "skill_callback_stub_lite.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t MAX_URI_COUNT = 500;
} // namespace

int32_t SkillCallbackLite::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != u"ohos.AAFwk.SkillExecuteCallback") {
        return ERR_INVALID_STATE;
    }
    if (code == ON_SKILL_EXECUTE_DONE) {
        return OnExecuteDoneInner(data, reply);
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t SkillCallbackLite::OnExecuteDoneInner(MessageParcel &data, MessageParcel &reply)
{
    std::string requestCode = data.ReadString();
    int32_t resultCode = data.ReadInt32();

    AppExecFwk::SkillExecuteResult result;
    result.code = data.ReadInt32();
    auto *wantParams = data.ReadParcelable<AAFwk::WantParams>();
    if (wantParams != nullptr) {
        result.result = std::shared_ptr<AAFwk::WantParams>(wantParams);
    }
    int32_t uriCount = data.ReadInt32();
    if (uriCount < 0 || uriCount > MAX_URI_COUNT) {
        return ERR_INVALID_VALUE;
    }
    for (int32_t i = 0; i < uriCount; i++) {
        result.uris.push_back(Str16ToStr8(data.ReadString16()));
    }
    result.flags = data.ReadUint32();

    OnExecuteDone(requestCode, resultCode, result);
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
