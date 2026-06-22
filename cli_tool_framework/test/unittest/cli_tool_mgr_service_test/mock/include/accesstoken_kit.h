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

#ifndef CLI_TOOL_MGR_SERVICE_TEST_ACCESSTOKEN_KIT_H
#define CLI_TOOL_MGR_SERVICE_TEST_ACCESSTOKEN_KIT_H

#include <string>
#include <sys/types.h>

namespace OHOS {
namespace Security {
namespace AccessToken {

enum ATokenTypeEnum : unsigned int {
    TOKEN_INVALID = 0,
    TOKEN_NATIVE = 1,
    TOKEN_HAP = 2,
    TOKEN_SHELL = 3,
};

enum PermissionState : int32_t {
    PERMISSION_DENIED = 0,
    PERMISSION_GRANTED = 1,
};

using AccessTokenID = uint64_t;

class AccessTokenKit {
public:
    static ATokenTypeEnum GetTokenTypeFlag(AccessTokenID tokenID);
    static int32_t VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName, bool crossIpc);
    static void DeleteToolTokenByPid(pid_t pid);
};

} // namespace AccessToken
} // namespace Security
} // namespace OHOS

#endif // CLI_TOOL_MGR_SERVICE_TEST_ACCESSTOKEN_KIT_H
