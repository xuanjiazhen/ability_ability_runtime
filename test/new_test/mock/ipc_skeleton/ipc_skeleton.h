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

#ifndef OHOS_ABILITY_RUNTIME_IPC_SKELETON_H
#define OHOS_ABILITY_RUNTIME_IPC_SKELETON_H

#include <string>
#include <cstdint>
#include "oh_mock_utils.h"

namespace OHOS {

using pid_t = int32_t;

class IPCSkeleton {
public:
    IPCSkeleton() = default;
    ~IPCSkeleton() = default;

    // Mock GetCallingTokenID - static method returning uint32_t
    OH_MOCK_METHOD_WITH_DECORATOR(static, uint32_t, IPCSkeleton, GetCallingTokenID);

    // Mock GetCallingUid - static method returning pid_t
    OH_MOCK_METHOD_WITH_DECORATOR(static, pid_t, IPCSkeleton, GetCallingUid);

    // Mock GetCallingPid - static method returning pid_t
    OH_MOCK_METHOD_WITH_DECORATOR(static, pid_t, IPCSkeleton, GetCallingPid);

    // Mock GetCallingFullTokenID - static method returning uint64_t
    OH_MOCK_METHOD_WITH_DECORATOR(static, uint64_t, IPCSkeleton, GetCallingFullTokenID);

    // Mock GetSelfTokenID - static method returning uint64_t
    OH_MOCK_METHOD_WITH_DECORATOR(static, uint64_t, IPCSkeleton, GetSelfTokenID);

    // Mock GetCallingRealPid - static method returning pid_t
    OH_MOCK_METHOD_WITH_DECORATOR(static, pid_t, IPCSkeleton, GetCallingRealPid);

    // Mock ResetCallingIdentity - static method returning std::string
    OH_MOCK_METHOD_WITH_DECORATOR(static, std::string, IPCSkeleton, ResetCallingIdentity);

    // Mock SetCallingIdentity - static method returning bool with parameter
    OH_MOCK_METHOD_WITH_DECORATOR(static, bool, IPCSkeleton, SetCallingIdentity, const std::string&);
};

}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_IPC_SKELETON_H
