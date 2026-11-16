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

#include "task_mgr/schedule_mapping.h"

#include "nlohmann/json.hpp"

#include "hilog_tag_wrapper.h"
#include "parameters.h"
#include "task_mgr/common.h"

namespace OHOS {
namespace AppExecFwk {
namespace {

// Reads an int32 field of a policy object without throwing (build uses -fno-exceptions).
int32_t ReadIntField(const nlohmann::json &obj, const char *key)
{
    if (obj.contains(key) && obj[key].is_number_integer()) {
        return obj[key].get<int32_t>();
    }
    return 0;
}

bool ReadBoolField(const nlohmann::json &obj, const char *key)
{
    if (obj.contains(key) && obj[key].is_boolean()) {
        return obj[key].get<bool>();
    }
    return false;
}

bool IsValidLevel(int32_t level)
{
    return level == TASK_PRIORITY_LOW || level == TASK_PRIORITY_MEDIUM || level == TASK_PRIORITY_HIGH;
}

}  // namespace

ScheduleMapping &ScheduleMapping::GetInstance()
{
    static ScheduleMapping instance;
    return instance;
}

bool ScheduleMapping::Load()
{
    std::string text = OHOS::system::GetParameter(SCHED_MAPPING_PARAM, "");
    if (text.empty()) {
        Reset();
        TAG_LOGW(AAFwkTag::APPMGR, "sched mapping not loaded (param empty), fail-safe: no control applied");
        return false;
    }
    return LoadFromString(text);
}

bool ScheduleMapping::LoadFromString(const std::string &jsonText)
{
    std::map<int32_t, SchedulePolicy> parsed;
    // allow_exceptions=false keeps parsing non-throwing under -fno-exceptions.
    auto root = nlohmann::json::parse(jsonText, nullptr, false);
    if (!root.is_discarded() && root.is_object()) {
        for (const auto &item : root.items()) {
            int32_t level = atoi(item.key().c_str());
            if (!IsValidLevel(level) || !item.value().is_object()) {
                continue;
            }
            SchedulePolicy policy;
            policy.freeze = ReadBoolField(item.value(), "freeze");
            policy.schedGroup = ReadIntField(item.value(), "schedGroup");
            policy.qos = ReadIntField(item.value(), "qos");
            policy.speed = ReadIntField(item.value(), "speed");
            parsed[level] = policy;
        }
    }
    std::lock_guard<std::mutex> lock(mappingMutex_);
    if (parsed.empty()) {
        policies_.clear();
        ready_ = false;
        TAG_LOGW(AAFwkTag::APPMGR, "sched mapping illegal or empty, fail-safe: no control applied");
        return false;
    }
    policies_ = parsed;
    ready_ = true;
    TAG_LOGI(AAFwkTag::APPMGR, "sched mapping loaded, tiers: %{public}zu", policies_.size());
    return true;
}

void ScheduleMapping::Reset()
{
    std::lock_guard<std::mutex> lock(mappingMutex_);
    policies_.clear();
    ready_ = false;
}

bool ScheduleMapping::IsReady() const
{
    std::lock_guard<std::mutex> lock(mappingMutex_);
    return ready_;
}

bool ScheduleMapping::GetPolicy(int32_t level, SchedulePolicy &policy) const
{
    std::lock_guard<std::mutex> lock(mappingMutex_);
    if (!ready_) {
        return false;
    }
    auto it = policies_.find(level);
    if (it == policies_.end()) {
        return false;
    }
    policy = it->second;
    return true;
}

}  // namespace AppExecFwk
}  // namespace OHOS
