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

#include "multi_user_config_mgr.h"
namespace OHOS {
namespace AppExecFwk {
namespace {
    constexpr int32_t USER0 = 0;
}

void MultiUserConfigurationMgr::InitConfiguration(std::shared_ptr<AppExecFwk::Configuration> config)
{
    std::lock_guard<std::mutex> guard(multiUserConfigurationMutex_);
    globalConfiguration_ = config;
}

void MultiUserConfigurationMgr::Insert(const int32_t userId, const Configuration& config)
{
    std::lock_guard<std::mutex> guard(multiUserConfigurationMutex_);
    auto it = multiUserConfiguration_.find(userId);
    if (it != multiUserConfiguration_.end()) {
        std::vector<std::string> diffVe;
        it->second.CompareDifferent(diffVe, config);
        it->second.Merge(diffVe, config);
    } else {
        multiUserConfiguration_[userId] = config;
    }
}

std::shared_ptr<AppExecFwk::Configuration> MultiUserConfigurationMgr::GetGlobalConfiguration()
{
    return globalConfiguration_;
}

std::shared_ptr<AppExecFwk::Configuration> MultiUserConfigurationMgr::GetConfigurationByUserId(const int32_t userId)
{
    std::lock_guard<std::mutex> guard(multiUserConfigurationMutex_);
    Configuration userConfig;
    auto it = multiUserConfiguration_.find(userId);
    if (it != multiUserConfiguration_.end()) {
        userConfig = it->second;
    }

    std::vector<std::string> diffVe;
    userConfig.CompareDifferent(diffVe, *globalConfiguration_);
    userConfig.Merge(diffVe, *globalConfiguration_);

    return std::make_shared<AppExecFwk::Configuration>(userConfig);
}

std::shared_ptr<AppExecFwk::Configuration> MultiUserConfigurationMgr::GetConfigurationByUserIdAndUpdateGlobalConfiguration(const int32_t userId)
{
    std::lock_guard<std::mutex> guard(multiUserConfigurationMutex_);
    Configuration userConfig;
    auto it = multiUserConfiguration_.find(userId);
    if (it != multiUserConfiguration_.end()) {
        userConfig = it->second;
    }

    std::vector<std::string> diffVe;
    globalConfiguration_.CompareDifferent(diffVe, userConfig);
    globalConfiguration_.Merge(diffVe, userConfig);

    return globalConfiguration_;
}

void MultiUserConfigurationMgr::HandleConfiguration(const int32_t userId, const Configuration& config, std::vector<std::string>& changeKeyV)
{
    if (userId == -1) {
        {
            globalConfiguration_.CompareDifferent(changeKeyV, config);
            if(changeKeyV.size() != 0) {
                globalConfiguration_.Merge(changeKeyV, config);
            }
        }

        // compatible
        Configuration configTmp;
        if (!config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE).empty()) {
            configTmp.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE,
                config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE));
        }
        if (!config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE).empty()) {
            configTmp.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE,
                config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE));
        }
        if (!config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE).empty()) {
            configTmp.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE,
                config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_WEIGHT_SCALE));
        }
        this->Insert(GetUserIdByUid(IPCSkeleton::GetCallingUid()), configTmp);
        
    } else {
        CheckAndUpdateConfigurationByUserId(userId, config, changeKeyV);
        if(changeKeyV.size() != 0) {
                globalConfiguration_.Merge(changeKeyV, config);
            }
    }

}

void MultiUserConfigurationMgr::CheckAndUpdateConfigurationByUserId(
    const int32_t userId, const Configuration& config, std::vector<std::string>& changeKeyV)
{
    std::lock_guard<std::mutex> guard(multiUserConfigurationMutex_);

    auto it = multiUserConfiguration_.find(userId);
    if (it != multiUserConfiguration_.end()) {
        it->second.CompareDifferent(changeKeyV, config);
        it->second.Merge(changeKeyV, config);
    } else {
        multiUserConfiguration_[userId] = config;
        Configuration().CompareDifferent(changeKeyV, config);
    }

    if (userId != USER0) {
        auto ite = multiUserConfiguration_.find(USER0);
        if (ite != multiUserConfiguration_.end()) {
            std::vector<std::string> diffKeyV;
            ite->second.CompareDifferent(diffKeyV, config);
            ite->second.Merge(diffKeyV, config);
        } else {
            multiUserConfiguration_[USER0] = config;
        }
    }
}

}  // namespace AppExecFwk
}  // namespace OHOS
