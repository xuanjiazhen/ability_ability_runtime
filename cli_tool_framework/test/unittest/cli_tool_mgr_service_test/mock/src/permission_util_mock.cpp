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

#include "cli_error_code.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace CliTool {
// Test-only toggles consumed by the mocked VerifyAccessToken below. They allow
// individual test cases to flip the permission verdict without touching real
// AccessToken machinery. Defined here (not in the test body) so every
// translation unit that includes the extern declaration in the test file links
// against the same definition.
bool g_execCliToolPermittedForTest = true;
bool g_execPublicCliToolPermittedForTest = true;

bool PermissionUtil::VerifyAccessToken(Security::AccessToken::AccessTokenID, const std::string &perm)
{
    if (perm == "ohos.permission.EXEC_CLI_TOOL") {
        return g_execCliToolPermittedForTest;
    }
    if (perm == "ohos.permission.EXEC_PUBLIC_CLI_TOOL") {
        return g_execPublicCliToolPermittedForTest;
    }
    return true;
}

bool PermissionUtil::IsSystemApp()
{
    return IPCSkeleton::GetCallingFullTokenID() == 0;
}

bool PermissionUtil::IsSystemSA()
{
    return IPCSkeleton::GetCallingTokenID() == TOKEN_NATIVE;
}

int32_t PermissionUtil::CheckSystemAndPermission(const std::string &)
{
    if (!IsSystemApp() && !IsSystemSA()) {
        return ERR_NOT_SYSTEM_APP;
    }
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
