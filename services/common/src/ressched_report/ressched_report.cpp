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

#include <dlfcn.h>

#include "ressched_report.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *RES_SCHED_CLIENT_SO = "libressched_client.z.so";
}

ReportSyncEventFunc LoadResSchedReportSyncEventFunc()
{
    auto handle = dlopen(RES_SCHED_CLIENT_SO, RTLD_LAZY);
    if (handle == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "dlopen failed!");
        return nullptr;
    }
    auto func = reinterpret_cast<ReportSyncEventFunc>(dlsym(handle, "ReportSyncEvent"));
    if (func == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "dlsym failed!");
        dlclose(handle);
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "dlsym success.");
    return func;
}

ResSchedReport& ResSchedReport::GetInstance()
{
    static ResSchedReport instance;
    return instance;
}

ResSchedReport::ResSchedReport()
{
    reportSyncEventFunc_ = LoadResSchedReportSyncEventFunc();
}

int32_t ResSchedReport::ResSchedReportSyncEvent(const uint32_t resType, const int64_t value,
    const std::unordered_map<std::string, std::string> &payload, std::unordered_map<std::string, std::string> &reply)
{
    if (reportSyncEventFunc_ == nullptr) {
        reportSyncEventFunc_ = LoadResSchedReportSyncEventFunc();
    }
    if (!reportSyncEventFunc_) {
        TAG_LOGE(AAFwkTag::APPKIT, "func is null");
        return -1;
    }
    return reportSyncEventFunc_(resType, value, payload, reply);
}

}  // namespace AbilityRuntime
}  // namespace OHOS