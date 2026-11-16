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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_SCHEDULE_MAPPING_H
#define OHOS_APP_EXECFWK_TASK_MGR_SCHEDULE_MAPPING_H

#include <cstdint>
#include <map>
#include <mutex>
#include <string>

namespace OHOS {
namespace AppExecFwk {

// System parameter carrying the priority->policy mapping (JSON, finalized by the perf/power
// review board). External so mapping changes need no interface/architecture change (ADR-3).
constexpr const char *SCHED_MAPPING_PARAM = "persist.sys.appms.taskmanager.sched_mapping";

// One policy tier of the mapping: the freeze/schedGroup/qos/speed combination of a level.
struct SchedulePolicy {
    bool freeze = false;
    int32_t schedGroup = 0;
    int32_t qos = 0;
    int32_t speed = 0;
};

/**
 * @brief Loads and validates the priority->policy mapping from the system parameter.
 *        When the parameter is missing or illegal the mapping stays not-ready (fail-safe):
 *        no control is applied and an alert log is recorded (R-6 / INV-5).
 */
class ScheduleMapping {
public:
    static ScheduleMapping &GetInstance();

    // Loads the mapping from the system parameter. Returns true when a valid mapping is ready.
    bool Load();
    // Parses a mapping JSON text (parameterized entry used by Load and by tests). On illegal
    // or empty content the mapping becomes not-ready (fail-safe). Returns the ready state.
    bool LoadFromString(const std::string &jsonText);
    // Clears the mapping, returning to the fail-safe (not-ready) state.
    void Reset();
    bool IsReady() const;
    // Fills the policy of a level (100/200/300). Returns false when not ready or unknown level.
    bool GetPolicy(int32_t level, SchedulePolicy &policy) const;

private:
    ScheduleMapping() = default;
    ~ScheduleMapping() = default;
    ScheduleMapping(const ScheduleMapping &) = delete;
    ScheduleMapping &operator=(const ScheduleMapping &) = delete;

    mutable std::mutex mappingMutex_;
    std::map<int32_t, SchedulePolicy> policies_;
    bool ready_ = false;
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_APP_EXECFWK_TASK_MGR_SCHEDULE_MAPPING_H
