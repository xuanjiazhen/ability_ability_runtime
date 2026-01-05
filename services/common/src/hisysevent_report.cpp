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

#include "hisysevent_report.h"

#include <cstring>
#include "securec.h"
#include <cstdlib>
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
HisyseventReport::HisyseventReport(int32_t len)
{
    this->params = new HiSysEventParam[len];
    this->length = len;
}

HisyseventReport::~HisyseventReport()
{
    delete[] params;
}

void HisyseventReport::InsertParam(const char* name, const bool value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_BOOL,
        .v = { .b = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::InsertParam(const char* name, const int8_t value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_INT8,
        .v = { .i8 = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::InsertParam(const char* name, const uint8_t value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_UINT8,
        .v = { .ui8 = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::InsertParam(const char* name, const int16_t value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_INT16,
        .v = { .i16 = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::InsertParam(const char* name, const uint16_t value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_UINT16,
        .v = { .ui16 = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::InsertParam(const char* name, const int32_t value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_INT32,
        .v = { .i32 = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::InsertParam(const char* name, const uint32_t value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_UINT32,
        .v = { .ui32 = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::InsertParam(const char* name, const int64_t value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_INT64,
        .v = { .i64 = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::InsertParam(const char* name, const uint64_t value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_UINT64,
        .v = { .ui64 = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::InsertParam(const char* name, const float value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_FLOAT,
        .v = { .f = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::InsertParam(const char* name, const double value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_DOUBLE,
        .v = { .d = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::InsertParam(const char* name, std::string value)
{
    this->InsertParam(name, const_cast<char *>(value.c_str()));
}

void HisyseventReport::InsertParam(const char* name, const char* value)
{
    if (length <= pos) {
        TAG_LOGE(AAFwkTag::DEFAULT, "param is full");
    }
    HiSysEventParam param = {
        .t = HISYSEVENT_STRING,
        .v = { .s = value },
        .arraySize = 0,
    };
    SetParamName(param, name);
    params[pos++] = param;
}

void HisyseventReport::SetParamName(HiSysEventParam& param, const char* name)
{
    int32_t ret = strcpy_s(param.name, sizeof(param.name), name);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::DEFAULT, "SetParamName err %{public}d", ret);
    }
}

int32_t HisyseventReport::Report(const char* domain, const char* event, HiSysEventEventType type)
{
    if (params == nullptr || len == 0) {
        TAG_LOGE(AAFwkTag::DEFAULT, "invalid report");
    }
    return OH_HiSysEvent_Write(domain, event, type, params, pos);
}
} // namespace AAFwk
} // namespace OHOS