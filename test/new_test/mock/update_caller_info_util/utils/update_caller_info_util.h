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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_UPDATE_CALLER_INFO_UTIL_H
#define MOCK_OHOS_ABILITY_RUNTIME_UPDATE_CALLER_INFO_UTIL_H

#include <string>
#include "want.h"

namespace OHOS {
namespace AAFwk {
class UpdateCallerInfoUtil {
public:
    static UpdateCallerInfoUtil &GetInstance()
    {
        static UpdateCallerInfoUtil instance;
        return instance;
    }

    ~UpdateCallerInfoUtil() = default;

    void UpdateAsCallerSourceInfo(Want& want, sptr<IRemoteObject> asCallerSourceToken,
        sptr<IRemoteObject> callerToken) {}
    void UpdateCallerInfo(Want& want, const sptr<IRemoteObject> &callerToken) {}
    void UpdateBackToCallerFlag(const sptr<IRemoteObject> &callerToken, Want &want,
        int32_t requestCode, bool backFlag) {}
    void UpdateCallerInfoFromToken(Want& want, const sptr<IRemoteObject> &token) {}
    void UpdateDmsCallerInfo(Want& want, const sptr<IRemoteObject> &callerToken) {}
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
