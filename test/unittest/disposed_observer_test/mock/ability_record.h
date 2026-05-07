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

#ifndef TEST_ABILITY_RECORD_H
#define TEST_ABILITY_RECORD_H

#include <memory>
#include "ability_info.h"
#include "want.h"
#include "session_info.h"
#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {

class Token {
public:
    static std::shared_ptr<AbilityRecord> abilityRecord;
    static Want mockWant;
    static AppExecFwk::AbilityInfo mockAbilityInfo;
    static sptr<SessionInfo> mockSessionInfo;
    static int32_t mockMissionId;
    static int32_t mockCreateModalResult;

    static std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(sptr<IRemoteObject> token);
};

class AbilityRecord {
public:
    AbilityRecord() : abilityInfo_(Token::mockAbilityInfo), want_(Token::mockWant) {}
    
    const Want& GetWant() const { return want_; }
    const AppExecFwk::AbilityInfo& GetAbilityInfo() const { return abilityInfo_; }
    sptr<SessionInfo> GetSessionInfo() const { return Token::mockSessionInfo; }
    int32_t GetMissionId() const { return Token::mockMissionId; }
    
    int32_t CreateModalUIExtension(const Want& want) { return Token::mockCreateModalResult; }

private:
    AppExecFwk::AbilityInfo abilityInfo_;
    Want want_;
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // TEST_ABILITY_RECORD_H