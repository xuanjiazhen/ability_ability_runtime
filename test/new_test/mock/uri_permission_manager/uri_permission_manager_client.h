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

#ifndef OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H
#define OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H

#include "uri.h"
#include "policy_info.h"
#include "oh_mock_utils.h"

namespace OHOS {
namespace AAFwk {
class UriPermissionManagerClient {
public:
    static UriPermissionManagerClient& GetInstance()
    {
        static UriPermissionManagerClient instance;
        return instance;
    }

    ~UriPermissionManagerClient() = default;

    OH_MOCK_METHOD(int32_t, UriPermissionManagerClient, GrantUriPermission, const Uri&,
        unsigned int, const std::string, int32_t, uint32_t);

    OH_MOCK_METHOD(int32_t, UriPermissionManagerClient, GrantUriPermission, const std::vector<Uri>&,
        unsigned int, const std::string, int32_t, uint32_t);

    int32_t GrantUriPermissionPrivileged(const std::vector<Uri> &uriVec, uint32_t flag,
        const std::string &targetBundleName, int32_t appIndex = 0, uint32_t initiatorTokenId = 0,
        int32_t hideSensitiveType = 0)
    {
        return grantUriPermissionPrivilegedRet;
    }

    int32_t RevokeAllUriPermissions(const uint32_t tokenId)
    {
        return 0;
    }

    int32_t RevokeUriPermissionManually(const Uri &uri, const std::string bundleName, int32_t appIndex = 0)
    {
        return 0;
    }

    bool VerifyUriPermission(const Uri &uri, uint32_t flag, uint32_t tokenId)
    {
        return true;
    }

    OH_MOCK_METHOD(std::vector<bool>, UriPermissionManagerClient, CheckUriAuthorization,
        const std::vector<std::string>&, uint32_t, uint32_t);

    int32_t ClearPermissionTokenByMap(const uint32_t tokenId)
    {
        return 0;
    }

    int32_t GrantUriPermissionByKey(const std::string &key, uint32_t flag, uint32_t targetTokenId)
    {
        return 0;
    }

    int32_t GrantUriPermissionByKeyAsCaller(const std::string &key, uint32_t flag, uint32_t callerTokenId,
        uint32_t targetTokenId)
    {
        return 0;
    }

    int32_t Active(const std::vector<PolicyInfo> &policy, std::vector<uint32_t> &result)
    {
        return 0;
    }
public:
    static void InitMockResult()
    {
        grantUriPermissionPrivilegedRet = 0;
    }
    static int32_t grantUriPermissionPrivilegedRet;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_URI_PERMISSION_MANAGER_CLIENT_H
