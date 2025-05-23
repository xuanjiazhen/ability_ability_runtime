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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_KEEP_ALIVE_PROCESS_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_KEEP_ALIVE_PROCESS_MANAGER_H

#include <functional>

#include "app_scheduler.h"
#include "bundle_info.h"
#include "ffrt.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
class KeepAliveProcessManager {
public:
    static KeepAliveProcessManager &GetInstance()
    {
        static KeepAliveProcessManager instance;
        return instance;
    }

    int32_t SetApplicationKeepAlive(const std::string &bundleName, int32_t userId, bool updateEnable,
        bool isByEDM, bool isInner)
    {
        return 0;
    }

    int32_t QueryKeepAliveApplications(int32_t appType, int32_t userId, std::vector<KeepAliveInfo> &infoList,
        bool isByEDM)
    {
        return 0;
    }

    void StartKeepAliveProcessWithMainElement(std::vector<AppExecFwk::BundleInfo> &bundleInfos, int32_t userId) {}

    void OnAppStateChanged(const AppInfo &info) {}

    bool IsKeepAliveBundle(const std::string &bundleName, int32_t userId)
    {
        return false;
    }

    bool GetKeepAliveBundleInfosForUser(std::vector<AppExecFwk::BundleInfo> &bundleInfos, int32_t userId)
    {
        return false;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
