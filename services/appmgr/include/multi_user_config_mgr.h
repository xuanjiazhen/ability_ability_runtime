/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MULTI_USER_CONFIG_MGR_H
#define OHOS_ABILITY_RUNTIME_MULTI_USER_CONFIG_MGR_H

#include <map>
#include <mutex>
#include "configuration.h"
#include <memory>

namespace OHOS {
namespace AppExecFwk {

class MultiUserConfigurationMgr {
public:
    void Insert(const int32_t userId, const Configuration& config);

    std::shared_ptr<AppExecFwk::Configuration> GetConfigurationByUserId(const int32_t userId);

    void CheckAndUpdateConfigurationByUserId(
        const int32_t userId, const Configuration& config, std::vector<std::string>& changeKeyV);
    
    void InitConfiguration(std::shared_ptr<AppExecFwk::Configuration> config);
    std::shared_ptr<AppExecFwk::Configuration> GetConfigurationByUserIdAndUpdateGlobalConfiguration(const int32_t userId);
    void HandleConfiguration(const int32_t userId, const Configuration& config, std::vector<std::string>& changeKeyV);
    std::shared_ptr<AppExecFwk::Configuration> GetGlobalConfiguration();

private:
    std::map<int32_t, Configuration> multiUserConfiguration_;
    std::mutex multiUserConfigurationMutex_;

    std::shared_ptr<AppExecFwk::Configuration> globalConfiguration_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_MULTI_USER_CONFIG_MGR_H
