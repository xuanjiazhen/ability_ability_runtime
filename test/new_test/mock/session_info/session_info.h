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

#ifndef MOCK_OHOS_ABILITY_SESSION_INFO_H
#define MOCK_OHOS_ABILITY_SESSION_INFO_H

#include <typeinfo>
#include <unistd.h>

#include "ability_info.h"
#include "iremote_object.h"
#include "parcel.h"
#include "session_info_constants.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityStartSetting;

class SessionInfo : public Parcelable {
public:
    bool Marshalling(Parcel& parcel) const override
    {
        return false;
    }
    static SessionInfo* Unmarshalling(Parcel& parcel)
    {
        return nullptr;
    }

    sptr<IRemoteObject> sessionToken = nullptr;
    sptr<IRemoteObject> callerSession = nullptr;
    sptr<IRemoteObject> callerToken = nullptr;
    sptr<IRemoteObject> parentToken = nullptr;

    int32_t persistentId = 0;
    uint32_t hostWindowId = 0;

    int32_t resultCode = -1;
    int32_t requestCode = -1;
    std::string errorReason;
    int32_t errorCode = -1;
    int64_t uiAbilityId = 0;
    std::shared_ptr<AbilityStartSetting> startSetting = nullptr;
    Want want;
    int32_t userId = -1;
    bool isNewWant = true;
    bool isClearSession = false;
    uint32_t callingTokenId = 0;
    int32_t collaboratorType = 0;
    bool isSkipErms = false;
    UIExtensionUsage uiExtensionUsage = UIExtensionUsage::MODAL;
    uint64_t uiExtensionComponentId = 0;
    uint64_t displayId = 0;
    int32_t orientation = 0;
    std::string instanceKey = "";
    bool isMinimizedDuringFreeInstall = false;
    std::string sessionName;
};
} // namespace AAFwk
} // namespace OHOS
#endif
