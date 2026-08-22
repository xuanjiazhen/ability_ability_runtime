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

#include <set>
#include <string>

namespace OHOS {
namespace CliTool {
// Test-only deny-list: permissions listed here fail verification, everything else is granted
std::set<std::string> g_mockDeniedPermissions;

bool PermissionUtil::VerifyAccessToken(Security::AccessToken::AccessTokenID,
    const std::string &requirePermission)
{
    return g_mockDeniedPermissions.find(requirePermission) == g_mockDeniedPermissions.end();
}

} // namespace CliTool
} // namespace OHOS
