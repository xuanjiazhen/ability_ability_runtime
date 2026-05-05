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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H
#define OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H

#include <memory>

#include "ability_info.h"
#include "ability_token_stub.h"
#include "ipc_skeleton.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityRecord;

struct SessionInfo : public IRemoteObject::IObject {
    int32_t persistentId = 0;

    SessionInfo() = default;
    ~SessionInfo() override = default;
};

/**
 * @class Token
 * Token is identification of ability and used to interact with kit and wms.
 */
class Token : public AbilityTokenStub {
public:
    Token();
    virtual ~Token();

    static std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(sptr<IRemoteObject> token);

public:
    static std::shared_ptr<AbilityRecord> abilityRecord;
    static Want mockWant;
    static AppExecFwk::AbilityInfo mockAbilityInfo;
    static sptr<SessionInfo> mockSessionInfo;
    static int32_t mockMissionId;
    static int mockCreateModalResult;
};

/**
 * @class AbilityRecord
 * AbilityRecord records ability info and states and used to schedule ability life.
 */
class AbilityRecord : public std::enable_shared_from_this<AbilityRecord> {
public:
    AbilityRecord();
    ~AbilityRecord();

    /**
     * get ability's info.
     *
     * @return ability info.
     */
    const AppExecFwk::AbilityInfo &GetAbilityInfo() const;

    /**
     * get ability's want.
     *
     * @return ability want.
     */
    Want GetWant() const;

    /**
     * get session info.
     *
     * @return session info.
     */
    sptr<SessionInfo> GetSessionInfo() const;

    /**
     * get mission id.
     *
     * @return mission id.
     */
    int32_t GetMissionId() const;

    /**
     * create modal ui extension.
     *
     * @param want the want.
     * @return result code.
     */
    int32_t CreateModalUIExtension(const Want &want);
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H
