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

#ifndef OHOS_ABILITY_RUNTIME_RESSCHED_REPORT_H
#define OHOS_ABILITY_RUNTIME_RESSCHED_REPORT_H

#include <unordered_map>

namespace OHOS {
namespace AbilityRuntime {
namespace ResType {
enum : uint32_t {
    RES_TYPE_CTRL_FORKALL_IMAGE_INTERFACE = 511,
};
} // ResType

namespace ResValue {
enum : int64_t {
    SET_SUPPORT_MIRROR_PROCESS = 0,
    REBUILD_IMAGE = 1,
};
} // ResValue
using ReportSyncEventFunc = int32_t (*)(const uint32_t resType, const int64_t value,
    const std::unordered_map<std::string, std::string> &payload, std::unordered_map<std::string, std::string> &reply);
ReportSyncEventFunc LoadResSchedReportSyncEventFunc();
class ResSchedReport final {
public:
    static ResSchedReport& GetInstance();

    int32_t ResSchedReportSyncEvent(const uint32_t resType, const int64_t value,
        const std::unordered_map<std::string, std::string> &payload,
        std::unordered_map<std::string, std::string> &reply);
private:
    ResSchedReport();
    ~ResSchedReport() {}
    ReportSyncEventFunc reportSyncEventFunc_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_RESSCHED_REPORT_H