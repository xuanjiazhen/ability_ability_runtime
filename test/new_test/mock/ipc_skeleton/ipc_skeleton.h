/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_IPC_IPC_SKELETON_H
#define MOCK_OHOS_IPC_IPC_SKELETON_H

#include "iremote_object.h"

namespace OHOS {
class IPCSkeleton {
public:
    static pid_t GetCallingPid()
    {
        return 0;
    }

    static pid_t GetCallingUid()
    {
        return 0;
    }

    static uint32_t GetCallingTokenID()
    {
        return 0;
    }

    static uint64_t GetSelfTokenID()
    {
        return 0;
    }

    static std::string ResetCallingIdentity()
    {
        return "";
    }

    static bool SetCallingIdentity(std::string &identity, bool flag = false)
    {
        return false;
    }
};
} // namespace OHOS
#endif
