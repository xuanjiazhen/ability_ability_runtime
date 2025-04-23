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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_RESIDENT_PROCESS_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_RESIDENT_PROCESS_MANAGER_H

#include <list>
#include <mutex>

#include "app_scheduler.h"
#include "bundle_info.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
class ResidentProcessManager : public std::enable_shared_from_this<ResidentProcessManager> {
    DECLARE_DELAYED_SINGLETON(ResidentProcessManager)
public:
    void Init() {}
    int32_t SetResidentProcessEnabled(const std::string &bundleName, const std::string &callerName, bool updateEnable)
    {
        return 0;
    }
    void StartResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) {}
    void StartResidentProcessWithMainElement(std::vector<AppExecFwk::BundleInfo> &bundleInfos, int32_t userId) {}
    void OnAppStateChanged(const AppInfo &info) {}
    bool GetResidentBundleInfosForUser(std::vector<AppExecFwk::BundleInfo> &bundleInfos, int32_t userId)
    {
        return false;
    }
    void StartFailedResidentAbilities() {}
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
