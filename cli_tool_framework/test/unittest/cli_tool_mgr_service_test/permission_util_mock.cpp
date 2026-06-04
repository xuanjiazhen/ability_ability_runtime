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

#include "permission_util.h"

namespace OHOS {
namespace CliTool {
bool g_verifySystemExecPermission = true;
bool g_verifySystemQueryPermission = true;
bool g_verifyUserExecPermission = true;
bool g_verifyUserQueryPermission = true;

bool PermissionUtil::VerifyAccessToken(Security::AccessToken::AccessTokenID, const std::string &permission)
{
    if (permission == "ohos.permission.EXEC_CLI_TOOL") {
        return g_verifySystemExecPermission;
    }
    if (permission == "ohos.permission.QUERY_CLI_TOOL") {
        return g_verifySystemQueryPermission;
    }
    if (permission == "ohos.permission.EXEC_PUBLIC_CLI_TOOL") {
        return g_verifyUserExecPermission;
    }
    if (permission == "ohos.permission.QUERY_PUBLIC_CLI_TOOL") {
        return g_verifyUserQueryPermission;
    }
    return true;
}
} // namespace CliTool
} // namespace OHOS
