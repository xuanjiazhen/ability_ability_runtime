/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "multi_user_config_mgr.h"
namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr int32_t USER0 = 0;
}

MultiUserConfigurationMgr::MultiUserConfigurationMgr()
    : globalConfiguration_(std::make_shared<AppExecFwk::Configuration>())
{}

void MultiUserConfigurationMgr::InitConfiguration(std::shared_ptr<AppExecFwk::Configuration> config)
{
    globalConfiguration_ = config;
}

std::shared_ptr<AppExecFwk::Configuration> MultiUserConfigurationMgr::GetConfigurationByUserId(const int32_t userId)
{
    std::lock_guard<std::mutex> guard(multiUserConfigurationMutex_);
    auto it = multiUserConfiguration_.find(userId);
    if (it != multiUserConfiguration_.end()) {
        return std::make_shared<AppExecFwk::Configuration>((it->second));
    } else {
        return std::make_shared<AppExecFwk::Configuration>(*globalConfiguration_);
    }
}

void MultiUserConfigurationMgr::HandleConfiguration(
    const int32_t userId, const Configuration& config, std::vector<std::string>& changeKeyV)
{
    if (userId == -1) {
        globalConfiguration_->CompareDifferent(changeKeyV, config);
        if (changeKeyV.size() != 0) {
            globalConfiguration_->Merge(changeKeyV, config);
        }
        // update all user config
        {
            std::lock_guard<std::mutex> guard(multiUserConfigurationMutex_);
            for (auto& userConfig : multiUserConfiguration_) {
                std::vector<std::string> diffVe;
                userConfig.second.CompareDifferent(diffVe, config);
                if (diffVe.size() != 0) {
                    userConfig.second.Merge(diffVe, config);
                }
            }
        }
    } else {
        std::lock_guard<std::mutex> guard(multiUserConfigurationMutex_);
        auto it = multiUserConfiguration_.find(userId);
        if (it != multiUserConfiguration_.end()) {
            it->second.CompareDifferent(changeKeyV, config);
            if (changeKeyV.size() != 0) {
                it->second.Merge(changeKeyV, config);
            }
        } else {
            Configuration userConfig = *globalConfiguration_;
            userConfig.CompareDifferent(changeKeyV, config);
            if (changeKeyV.size() != 0) {
                userConfig.Merge(changeKeyV, config);
            }
            multiUserConfiguration_[userId] = userConfig;
        }

        if (userId != USER0) {
            multiUserConfiguration_[USER0] = multiUserConfiguration_[userId];
        }
    }
}
} // namespace AppExecFwk
} // namespace OHOS
