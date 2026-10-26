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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_OS_ACCOUNT_MANAGER_WRAPPER_H
#define MOCK_OHOS_ABILITY_RUNTIME_OS_ACCOUNT_MANAGER_WRAPPER_H

#include <cstdint>
#include <string>
#include <vector>

#include "errors.h"
#include "oh_mock_utils.h"

namespace OHOS {
namespace AppExecFwk {
inline thread_local std::vector<int32_t> g_getForegroundDisplayUserIds;

class OsAccountManagerWrapper {
public:
    static ErrCode GetOsAccountLocalIdFromUid(const int32_t uid, int32_t &id)
    {
        return 0;
    }

    static ErrCode GetForegroundOsAccountDisplayId(const int32_t userId, uint64_t &displayId)
    {
        g_getForegroundDisplayUserIds.push_back(userId);
        std::string key = "OsAccountManagerWrapper_GetForegroundOsAccountDisplayId_"
            "const int32_t userId, uint64_t& displayId";
        if (g_mockMap.find(key) == g_mockMap.end()) {
            g_mockMap[key] = {};
        }
        ErrCode temp{};
        ResultWrap tempRet(temp);
        auto it = g_mockMap.find(key);
        if (it != g_mockMap.end()) {
            std::vector<ResultWrap> expectRets = it->second;
            if (!expectRets.empty()) {
                tempRet = expectRets[0];
                expectRets.erase(expectRets.begin());
            }
            g_mockMap[key] = expectRets;
        }
        if (g_RefMap.find(key) == g_RefMap.end()) {
            g_RefMap[key] = {};
        }
        uint64_t tempOp{};
        ResultWrap tempRef(tempOp);
        auto it1 = g_RefMap.find(key);
        if (it1 != g_RefMap.end()) {
            std::vector<std::vector<ResultWrap>> expectRefs = it1->second;
            if (!expectRefs.empty()) {
                tempRef = expectRefs[0][0];
                expectRefs.erase(expectRefs.begin());
            }
            g_RefMap[key] = expectRefs;
        }
        displayId = tempRef.Get<decltype(tempOp)>();
        return tempRet.Get<ErrCode>();
    }

    OH_MOCK_METHOD_WITH_DECORATOR_AND_OUTPUT_1(static, ErrCode, isOsAccountExists, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif