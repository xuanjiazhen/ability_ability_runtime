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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_FREEZE_UTIL_H
#define MOCK_OHOS_ABILITY_RUNTIME_FREEZE_UTIL_H

#include <list>
#include <mutex>
#include <unordered_map>

#include "iremote_object.h"

namespace OHOS::AbilityRuntime {
class FreezeUtil {
public:
    static FreezeUtil& GetInstance()
    {
        static FreezeUtil instance;
        return instance;
    }

    void AddLifecycleEvent(sptr<IRemoteObject> token, const std::string &entry) {}
    void DeleteLifecycleEvent(sptr<IRemoteObject> token) {}
    void DeleteAppLifecycleEvent(pid_t pid) {}
};
}  // namespace OHOS::AbilityRuntime
#endif