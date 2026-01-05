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

#ifndef OHOS_ABILITY_RUNTIME_HISYSEVENT_REPORT_H
#define OHOS_ABILITY_RUNTIME_HISYSEVENT_REPORT_H

#include "hisysevent_c.h"
#include <string>

namespace OHOS {
namespace AAFwk {
class HisyseventReport {
public:
    explicit HisyseventReport(int32_t len);
    ~HisyseventReport();

    void InsertParam(const char* name, const bool value);
    void InsertParam(const char* name, const int8_t value);
    void InsertParam(const char* name, const uint8_t value);
    void InsertParam(const char* name, const int16_t value);
    void InsertParam(const char* name, const uint16_t value);
    void InsertParam(const char* name, const int32_t value);
    void InsertParam(const char* name, const uint32_t value);
    void InsertParam(const char* name, const int64_t value);
    void InsertParam(const char* name, const uint64_t value);
    void InsertParam(const char* name, const float value);
    void InsertParam(const char* name, const double value);
    void InsertParam(const char* name, std::string value);
    void InsertParam(const char* name, const char* value);
    int32_t Report(const char* domain, const char* event, HiSysEventEventType type);

private:
    void SetParamName(HiSysEventParam& param, const char* name);

    HiSysEventParam* params;
    int32_t length = 0;
    int32_t pos = 0;
};
} // namespace AAFwk
} // namespace OHOS
#endif //OHOS_ABILITY_RUNTIME_HISYSEVENT_REPORT_H