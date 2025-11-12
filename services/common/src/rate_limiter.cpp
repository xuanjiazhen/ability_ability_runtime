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

#include "rate_limiter.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int64_t CLEAN_INTERVAL_MS = 60000; // 60s
constexpr int64_t EXTENSION_LIMIT_INTERVAL_MS = 1000; // 1s
const std::vector<std::pair<int32_t, std::string>> EXTENSION_TIERS = {
    {20, "TIER_20"},
    {50, "TIER_50"}, 
    {100, "TIER_100"}
};
constexpr int64_t REPORT_LIMIT_INTERVAL_MS = 5000; // 5s
constexpr int32_t REPORT_MAX_LIMIT = 1;
}

RateLimiter &RateLimiter::GetInstance()
{
    static RateLimiter instance;
    return instance;
}

RateLimiter::LimitResult RateLimiter::CheckExtensionLimit(int32_t uid)
{
    CleanCallMap();
    int64_t currentTimeMillis = CurrentTimeMillis();
    int64_t timeBefore = currentTimeMillis - EXTENSION_LIMIT_INTERVAL_MS;
    
    std::lock_guard<std::mutex> guard(extensionCallMapLock_);
    auto& timestamps = extensionCallMap_[uid];
    
    // 清理过期时间戳
    auto it = std::lower_bound(timestamps.begin(), timestamps.end(), timeBefore);
    timestamps.erase(timestamps.begin(), it);
    
    int currentCount = timestamps.size();
    LimitResult result{false, 0, ""};
    
    // 检查挡位触发状态
    {
        std::lock_guard<std::mutex> tierGuard(tierTriggerTimesLock_);
        auto& userTiers = tierTriggerTimes_[uid];
        
        // 清理该用户的过期挡位触发记录
        auto tierIt = userTiers.begin();
        while (tierIt != userTiers.end()) {
            if (tierIt->second < timeBefore) {
                tierIt = userTiers.erase(tierIt);
            } else {
                ++tierIt;
            }
        }
        
        // 从高到低检查挡位（优先触发高挡位）
        for (auto tierIt = EXTENSION_TIERS.rbegin(); tierIt != EXTENSION_TIERS.rend(); ++tierIt) {
            const auto& [limit, tierName] = *tierIt;
            
            if (currentCount >= limit) {
                // 检查这个挡位是否在当前时间窗口内已经触发过
                auto triggerIt = userTiers.find(limit);
                if (triggerIt == userTiers.end() || triggerIt->second < timeBefore) {
                    // 该挡位在当前秒内未触发过，可以触发
                    result = {true, limit, tierName};
                    userTiers[limit] = currentTimeMillis; // 记录触发时间
                    break;
                }
            }
        }
    }
    
    // 如果没有触发限流，记录当前时间
    if (!result.limited) {
        timestamps.emplace_back(currentTimeMillis);
    }
    
    return result;
}

bool RateLimiter::CheckReportLimit(int32_t uid)
{
    return CheckSingleLimit(uid, reportCallMap_, reportCallMapLock_, REPORT_LIMIT_INTERVAL_MS, REPORT_MAX_LIMIT);
}

bool RateLimiter::CheckSingleLimit(int32_t uid, std::unordered_map<int32_t, std::vector<int64_t>> &callMap,
    std::mutex &mapLock, int64_t limitInterval, int32_t maxLimit)
{
    int64_t currentTimeMillis = CurrentTimeMillis();
    int64_t timeBefore = currentTimeMillis - limitInterval;
    std::lock_guard<std::mutex> guard(mapLock);
    auto &timestamps = callMap[uid];
    
    auto it = std::lower_bound(timestamps.begin(), timestamps.end(), timeBefore);
    timestamps.erase(timestamps.begin(), it);

    if (timestamps.size() >= static_cast<size_t>(maxLimit)) {
        return true;
    }

    timestamps.emplace_back(currentTimeMillis);
    return false;
}

void RateLimiter::CleanCallMap()
{
    int64_t currentTimeMillis;
    {
        std::lock_guard<std::mutex> guard(lastCleanTimeMillisLock_);
        currentTimeMillis = CurrentTimeMillis();
        if (currentTimeMillis - lastCleanTimeMillis_ < CLEAN_INTERVAL_MS) {
            return;
        }
        lastCleanTimeMillis_ = currentTimeMillis;
    }
    CleanSingleCallMap(extensionCallMap_, extensionCallMapLock_, EXTENSION_LIMIT_INTERVAL_MS);
    CleanSingleCallMap(reportCallMap_, reportCallMapLock_, REPORT_LIMIT_INTERVAL_MS);
    CleanTierTriggerTimes(currentTimeMillis);
}

void RateLimiter::CleanTierTriggerTimes(int64_t currentTimeMillis) {
    std::lock_guard<std::mutex> guard(tierTriggerTimesLock_);
    int64_t timeBefore = currentTimeMillis - EXTENSION_LIMIT_INTERVAL_MS;
    
    auto it = tierTriggerTimes_.begin();
    while (it != tierTriggerTimes_.end()) {
        auto& userTiers = it->second;
        auto tierIt = userTiers.begin();
        while (tierIt != userTiers.end()) {
            if (tierIt->second < timeBefore) {
                tierIt = userTiers.erase(tierIt);
            } else {
                ++tierIt;
            }
        }
        
        if (userTiers.empty()) {
            it = tierTriggerTimes_.erase(it);
        } else {
            ++it;
        }
    }
}

void RateLimiter::CleanSingleCallMap(std::unordered_map<int32_t, std::vector<int64_t>> &callMap, std::mutex &mapLock,
    int64_t limitInterval)
{
    int64_t timeBefore = CurrentTimeMillis() - limitInterval;
    std::lock_guard<std::mutex> guard(mapLock);
    auto it = callMap.begin();
    while (it != callMap.end()) {
        bool allExpired = true;
        for (const auto &timestamp : it->second) {
            if (timestamp >= timeBefore) {
                allExpired = false;
                break;
            }
        }
        
        if (allExpired) {
            it = callMap.erase(it);
        } else {
            ++it;
        }
    }
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "CleanSingleCallMap end, size:%{public}zu", callMap.size());
}

int64_t RateLimiter::CurrentTimeMillis()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}
}  // namespace AAFwk
}  // namespace OHOS
