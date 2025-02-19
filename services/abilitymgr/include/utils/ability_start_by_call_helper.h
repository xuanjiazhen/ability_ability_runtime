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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_START_BY_CALL_HELPER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_START_BY_CALL_HELPER_H

#include <iremote_broker.h>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace AbilityStartByCallHelper {
static int CheckParam(const sptr<IRemoteObject> &callerToken, const sptr<IAbilityConnection> &connect,
    std::string &errMsg)
{
    if (connect == nullptr) {
        errMsg = "connect is nullptr";
        return ERR_INVALID_VALUE;
    }

    if (connect->AsObject() == nullptr) {
        errMsg = "connect translate to object is nullptr";
        return ERR_INVALID_VALUE;
    }

    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        return ERR_OK;
    }

    if (IPCSkeleton::GetCallingPid() == getprocpid()) {
        return ERR_OK;
    }

    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenID = abilityRecord->GetApplicationInfo().accessTokenId;
    if (callingTokenId != tokenID) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "no self, no enabled, callingTokenId:%{public}u, tokenId:%{public}u", callingTokenId, tokenID);
        errMsg = "no self, no enabled";
        return CHECK_PERMISSION_FAILED;
    }

    return ERR_OK;
}
}  // namespace AbilityStartByCallHelper
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_START_BY_CALL_HELPER_H
