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

#include "task_mgr/attribution_record.h"

#include "hilog_tag_wrapper.h"
#include "task_mgr/common.h"

namespace OHOS {
namespace AppExecFwk {

void InsightAttributionRecord::Schedule(int32_t level)
{
    TAG_LOGI(AAFwkTag::APPMGR,
        "insight schedule(stub) pid: %{public}d level: %{public}s", pid, TaskPriorityToString(level));
}

void InsightAttributionRecord::Unschedule()
{
    TAG_LOGI(AAFwkTag::APPMGR, "insight unschedule(stub) pid: %{public}d", pid);
}

void CliAttributionRecord::Schedule(int32_t level)
{
    TAG_LOGI(AAFwkTag::APPMGR,
        "cli schedule(stub) pid: %{public}d level: %{public}s", pid, TaskPriorityToString(level));
}

void CliAttributionRecord::Unschedule()
{
    TAG_LOGI(AAFwkTag::APPMGR, "cli unschedule(stub) pid: %{public}d", pid);
}

void A2aAttributionRecord::Schedule(int32_t level)
{
    TAG_LOGI(AAFwkTag::APPMGR,
        "a2a schedule(stub) pid: %{public}d level: %{public}s", pid, TaskPriorityToString(level));
}

void A2aAttributionRecord::Unschedule()
{
    TAG_LOGI(AAFwkTag::APPMGR, "a2a unschedule(stub) pid: %{public}d", pid);
}

void AbilityAttributionRecord::Schedule(int32_t level)
{
    TAG_LOGI(AAFwkTag::APPMGR,
        "ability schedule(stub) pid: %{public}d level: %{public}s", pid, TaskPriorityToString(level));
}

void AbilityAttributionRecord::Unschedule()
{
    TAG_LOGI(AAFwkTag::APPMGR, "ability unschedule(stub) pid: %{public}d", pid);
}

std::shared_ptr<AttributionRecord> CreateAttributionRecord(AttributionTool tool)
{
    std::shared_ptr<AttributionRecord> record;
    switch (tool) {
        case AttributionTool::INSIGHT:
            record = std::make_shared<InsightAttributionRecord>();
            break;
        case AttributionTool::CLI:
            record = std::make_shared<CliAttributionRecord>();
            break;
        case AttributionTool::A2A:
            record = std::make_shared<A2aAttributionRecord>();
            break;
        case AttributionTool::ABILITY:
            record = std::make_shared<AbilityAttributionRecord>();
            break;
        default:
            record = std::make_shared<AttributionRecord>();
            break;
    }
    // Tag the created record with its tool so callers/tests can identify the derived type
    // without RTTI (the build uses -fno-rtti).
    record->tool = tool;
    return record;
}

const char *AttributionToolToString(AttributionTool tool)
{
    switch (tool) {
        case AttributionTool::INSIGHT:
            return "insight";
        case AttributionTool::CLI:
            return "cli";
        case AttributionTool::A2A:
            return "a2a";
        case AttributionTool::ABILITY:
            return "ability";
        default:
            return "unknown";
    }
}

AttributionTool AttributionToolFromString(const std::string &text)
{
    if (text == "insight") {
        return AttributionTool::INSIGHT;
    }
    if (text == "cli") {
        return AttributionTool::CLI;
    }
    if (text == "a2a") {
        return AttributionTool::A2A;
    }
    if (text == "ability") {
        return AttributionTool::ABILITY;
    }
    return AttributionTool::UNKNOWN;
}
}  // namespace AppExecFwk
}  // namespace OHOS
