/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "cj_want_ffi.h"

#include <cstring>
#include <string>
#include <vector>

#include "cj_utils_ffi.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want.h"
#include "want_params.h"
#include "want_params_wrapper.h"
#include "hilog_tag_wrapper.h"

using OHOS::AAFwk::Want;
using OHOS::AppExecFwk::ElementName;

static const char* FD = "FD";
static const char* TYPE_PROPERTY = "type";
static const char* VALUE_PROPERTY = "value";

static bool ParseStringToInt32(const std::string &str, int32_t &result)
{
    if (str.empty()) {
        return false;
    }

    int32_t value = 0;
    size_t processed = 0;
    bool isNegative = false;
    bool hasDigit = false;

    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            value = value * 10 + (str[i] - '0');
            processed++;
            hasDigit = true;
        } else if (str[i] == '-' && i == 0) {
            isNegative = true;
        } else if (str[i] != ' ' && str[i] != '\t') {
            return false;
        }
    }

    if (!hasDigit) {
        return false;
    }

    result = isNegative ? -value : value;
    return true;
}

static bool ParseFdKey(const std::string &json, size_t &pos, std::string &key, size_t &colonPos)
{
    size_t keyStart = json.find('"', pos);
    if (keyStart == std::string::npos) {
        return false;
    }
    
    size_t keyEnd = json.find('"', keyStart + 1);
    if (keyEnd == std::string::npos) {
        return false;
    }
    
    key = json.substr(keyStart + 1, keyEnd - keyStart - 1);
    colonPos = json.find(':', keyEnd);
    
    return colonPos != std::string::npos;
}

static bool ParseFdValue(const std::string &json, size_t colonPos, size_t &valueEnd, int32_t &fdValue)
{
    size_t valueStart = colonPos + 1;
    while (valueStart < json.size() && (json[valueStart] == ' ' || json[valueStart] == '\t')) {
        valueStart++;
    }
    
    valueEnd = json.find_first_of(",}", valueStart);
    if (valueEnd == std::string::npos) {
        return false;
    }
    
    std::string valueStr = json.substr(valueStart, valueEnd - valueStart);
    return ParseStringToInt32(valueStr, fdValue);
}

static void InjectFdsToWantParams(const std::string &fdsJson, OHOS::AAFwk::WantParams &wantParams)
{
    if (fdsJson.empty() || fdsJson == "{}") {
        return;
    }

    std::string json = fdsJson;
    if (json.front() == '{') {
        json = json.substr(1);
    }
    if (json.back() == '}') {
        json.pop_back();
    }

    size_t pos = 0;
    while (pos < json.size()) {
        std::string key;
        size_t colonPos = 0;
        
        if (!ParseFdKey(json, pos, key, colonPos)) {
            break;
        }

        size_t valueEnd = 0;
        int32_t fdValue = 0;
        
        if (ParseFdValue(json, colonPos, valueEnd, fdValue)) {
            OHOS::AAFwk::WantParams fdParam;
            fdParam.SetParam(TYPE_PROPERTY, OHOS::AAFwk::String::Box(FD));
            fdParam.SetParam(VALUE_PROPERTY, OHOS::AAFwk::Integer::Box(fdValue));
            wantParams.SetParam(key, OHOS::AAFwk::WantParamWrapper::Box(fdParam));
        }
        
        pos = valueEnd + 1;
    }
}

static std::string ExtractFdsFromWantParams(const OHOS::AAFwk::WantParams &wantParams)
{
    auto paramList = wantParams.GetParams();
    std::string result = "{";
    bool first = true;
    for (auto it = paramList.begin(); it != paramList.end(); it++) {
        auto value = wantParams.GetParam(it->first);
        OHOS::AAFwk::IWantParams *o = OHOS::AAFwk::IWantParams::Query(value);
        if (o == nullptr) {
            continue;
        }
        OHOS::AAFwk::WantParams wp = OHOS::AAFwk::WantParamWrapper::Unbox(o);
        auto valueMap = wp.GetParams();
        if (valueMap.size() != 2) {
            continue;
        }
        auto typeIt = valueMap.find(TYPE_PROPERTY);
        if (typeIt == valueMap.end()) {
            continue;
        }
        OHOS::AAFwk::IString *strValue = OHOS::AAFwk::IString::Query(typeIt->second);
        if (strValue == nullptr) {
            continue;
        }
        std::string typeString = OHOS::AAFwk::String::Unbox(strValue);
        if (typeString != FD) {
            continue;
        }
        auto valueIt = valueMap.find(VALUE_PROPERTY);
        if (valueIt == valueMap.end()) {
            continue;
        }
        OHOS::AAFwk::IInteger *intValue = OHOS::AAFwk::IInteger::Query(valueIt->second);
        if (intValue == nullptr) {
            continue;
        }
        int32_t fdValue = OHOS::AAFwk::Integer::Unbox(intValue);
        if (!first) {
            result += ",";
        }
        result += "\"" + it->first + "\":" + std::to_string(fdValue);
        first = false;
    }
    result += "}";
    return result;
}

// Attention: The function does not handle entities.
WantHandle FFICJWantCreateWithWantInfo(CJWantParams params)
{
    Want* want = new (std::nothrow) Want();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null want");
        return nullptr;
    }

    auto element = reinterpret_cast<ElementName*>(params.elementName);
    want->SetElement(*element);
    want->SetFlags(params.flags);
    want->SetUri(params.uri);
    want->SetAction(params.action);
    want->SetType(params.wantType);
    want->SetParams(OHOS::AAFwk::WantParamWrapper::ParseWantParamsWithBrackets(params.parameters));

    return want;
}

WantHandle FFICJWantCreateWithWantInfoV2(CJWantParamsV2 params)
{
    Want* want = new (std::nothrow) Want();
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null want");
        return nullptr;
    }

    auto element = reinterpret_cast<ElementName*>(params.elementName);
    want->SetElement(*element);
    want->SetFlags(params.flags);
    want->SetUri(params.uri);
    want->SetAction(params.action);
    want->SetType(params.wantType);
    
    auto wantParams = OHOS::AAFwk::WantParamWrapper::ParseWantParamsWithBrackets(params.parameters);
    
    // Inject fds to WantParams
    if (params.fds != nullptr) {
        std::string fdsStr(params.fds);
        InjectFdsToWantParams(fdsStr, wantParams);
    }
    
    want->SetParams(wantParams);

    return want;
}

void FFICJWantDelete(WantHandle want)
{
    if (want == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null want");
        return;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    delete actualWant;
    actualWant = nullptr;
}

CJWantParams* FFICJWantGetWantInfo(WantHandle want)
{
    CJWantParams* buffer = static_cast<CJWantParams*>(malloc(sizeof(CJWantParams)));
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null buffer");
        return nullptr;
    }

    auto actualWant = reinterpret_cast<Want*>(want);
    auto element = actualWant->GetElement();
    ElementNameHandle elementName = new ElementName(
        element.GetDeviceID(), element.GetBundleName(), element.GetAbilityName(), element.GetModuleName());
    if (elementName == nullptr) {
        free(buffer);
        TAG_LOGE(AAFwkTag::DEFAULT, "element name null");
        return nullptr;
    }
    buffer->elementName = elementName;
    buffer->flags = actualWant->GetFlags();
    buffer->uri = CreateCStringFromString(actualWant->GetUriString());
    buffer->action = CreateCStringFromString(actualWant->GetAction());
    buffer->wantType = CreateCStringFromString(actualWant->GetType());
    buffer->entities = const_cast<std::vector<std::string>*>(&(actualWant->GetEntities())); // reference vector<String>
    buffer->parameters = CreateCStringFromString(OHOS::AAFwk::WantParamWrapper(actualWant->GetParams()).ToString());
    return buffer;
}

void FFICJWantParamsDelete(CJWantParams* params)
{
    if (params == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "argc null");
        return;
    }
    auto actualElementName = reinterpret_cast<ElementName*>(params->elementName);
    delete actualElementName;
    actualElementName = nullptr;

    free(static_cast<void*>(params->uri));
    free(static_cast<void*>(params->action));
    free(static_cast<void*>(params->wantType));
    free(static_cast<void*>(params->parameters));
    // Entities are reference, do not free.
    free(static_cast<void*>(params));
}

void FFICJWantAddEntity(WantHandle want, const char* entity)
{
    if (want == nullptr || entity == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Want or entity null");
        return;
    }
    auto actualWant = reinterpret_cast<Want*>(want);
    actualWant->AddEntity(entity);
}

WantHandle FFICJWantParseUri(const char* uri)
{
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "Uri null");
        return nullptr;
    }
    return Want::ParseUri(uri);
}

CJWantParamsV2* FFICJWantGetWantInfoV2(WantHandle want)
{
    CJWantParamsV2* buffer = static_cast<CJWantParamsV2*>(malloc(sizeof(CJWantParamsV2)));
    if (buffer == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null buffer");
        return nullptr;
    }

    auto actualWant = reinterpret_cast<Want*>(want);
    auto element = actualWant->GetElement();
    ElementNameHandle elementName = new ElementName(
        element.GetDeviceID(), element.GetBundleName(), element.GetAbilityName(), element.GetModuleName());
    if (elementName == nullptr) {
        free(buffer);
        TAG_LOGE(AAFwkTag::DEFAULT, "element name null");
        return nullptr;
    }
    buffer->elementName = elementName;
    buffer->flags = actualWant->GetFlags();
    buffer->uri = CreateCStringFromString(actualWant->GetUriString());
    buffer->action = CreateCStringFromString(actualWant->GetAction());
    buffer->wantType = CreateCStringFromString(actualWant->GetType());
    buffer->entities = const_cast<std::vector<std::string>*>(&(actualWant->GetEntities()));
    buffer->parameters = CreateCStringFromString(OHOS::AAFwk::WantParamWrapper(actualWant->GetParams()).ToString());
    buffer->fds = CreateCStringFromString(ExtractFdsFromWantParams(actualWant->GetParams()));
    return buffer;
}

void FFICJWantParamsDeleteV2(CJWantParamsV2* params)
{
    if (params == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "argc null");
        return;
    }
    auto actualElementName = reinterpret_cast<ElementName*>(params->elementName);
    delete actualElementName;
    actualElementName = nullptr;

    free(static_cast<void*>(params->uri));
    free(static_cast<void*>(params->action));
    free(static_cast<void*>(params->wantType));
    free(static_cast<void*>(params->parameters));
    free(static_cast<void*>(params->fds));
    free(static_cast<void*>(params));
}
