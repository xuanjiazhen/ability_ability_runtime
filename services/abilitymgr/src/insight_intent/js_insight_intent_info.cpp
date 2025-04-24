/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_insight_intent_info.h"

#include "string_wrapper.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
#define READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(type, parcel, data)            \
    do {                                                                    \
        if (!(parcel).Read##type(data)) {                                   \
            TAG_LOGE(AAFwkTag::INTENT, "read %{public}s type fail", #type); \
            return false;                                                   \
        }                                                                   \
    } while (0)

#define WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(type, parcel, data)           \
    do {                                                                    \
        if (!(parcel).Write##type(data)) {                                  \
            TAG_LOGE(AAFwkTag::INTENT, "write %{public}s type fail", #type);\
            return false;                                                   \
        }                                                                   \
    } while (0)
} // namespace
template<typename T>
void ReadParcelableInfo(T &parceable, Parcel &parcel)
{
    std::unique_ptr<T> info(parcel.ReadParcelable<T>());
    if (!info) {
        TAG_LOGE(AAFwkTag::INTENT, "failed, info is nullptr");
    }
    parceable = *info;
}

bool JsLinkInfo::ReadFromParcel(Parcel &parcel)
{
    uri = Str16ToStr8(parcel.ReadString16());
    return true;
}    

bool JsLinkInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(uri))) {
        return false;
    }
    return true;
}

JsLinkInfo *JsLinkInfo::Unmarshalling(Parcel &parcel)
{
    JsLinkInfo *info = new (std::nothrow) JsLinkInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool JsPageInfo::ReadFromParcel(Parcel &parcel)
{
    uiAbility = Str16ToStr8(parcel.ReadString16());
    pageRouterName = Str16ToStr8(parcel.ReadString16());
    navigationId = Str16ToStr8(parcel.ReadString16());
    navDestination = Str16ToStr8(parcel.ReadString16());
    return true;
}

bool JsPageInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(uiAbility))) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(pageRouterName))) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(navigationId))) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(navDestination))) {
        return false;
    }
    return true;
}

JsPageInfo *JsPageInfo::Unmarshalling(Parcel &parcel)
{
    JsPageInfo *info = new (std::nothrow) JsPageInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool JsEntryInfo::ReadFromParcel(Parcel &parcel)
{
    abilityName = Str16ToStr8(parcel.ReadString16());
    executeMode = static_cast<InsightIntentExecuteMode>(parcel.ReadUint32());
    return true;
}

bool JsEntryInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(abilityName))) {
        return false;
    }
    if (!parcel.WriteUint32(static_cast<uint32_t>(executeMode))) {
        return false;
    }
    return true;
}

JsEntryInfo *JsEntryInfo::Unmarshalling(Parcel &parcel)
{
    JsEntryInfo *info = new (std::nothrow) JsEntryInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool JsFunctionInfo::ReadFromParcel(Parcel &parcel)
{
    return true;
}    

bool JsFunctionInfo::Marshalling(Parcel &parcel) const
{
    return true;
}

JsFunctionInfo *JsFunctionInfo::Unmarshalling(Parcel &parcel)
{
    JsFunctionInfo *info = new (std::nothrow) JsFunctionInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool JsFormInfo::ReadFromParcel(Parcel &parcel)
{
    return true;
}    

bool JsFormInfo::Marshalling(Parcel &parcel) const
{
    return true;
}

JsFormInfo *JsFormInfo::Unmarshalling(Parcel &parcel)
{
    JsFormInfo *info = new (std::nothrow) JsFormInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool JsIntentParamInfo::ReadFromParcel(Parcel &parcel)
{
    paramName = Str16ToStr8(parcel.ReadString16());
    paramDescription = Str16ToStr8(parcel.ReadString16());
    paramType = Str16ToStr8(parcel.ReadString16());
    paramName = Str16ToStr8(parcel.ReadString16());
    isRequired = parcel.ReadBool();
    return true;
}

bool JsIntentParamInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(paramName))) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(paramDescription))) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(paramType))) {
        return false;
    }
    if (!parcel.WriteBool(isRequired)) {
        return false;
    }
    return true;
}

JsIntentParamInfo *JsIntentParamInfo::Unmarshalling(Parcel &parcel)
{
    JsIntentParamInfo *info = new (std::nothrow) JsIntentParamInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool JsInsightIntentInfo::ExtraInsightIntentGenericInfo2JsInsightIntentInfo(ExtraInsightIntentGenericInfo &summaryInfo)
{
    bundleName = (summaryInfo.bundleName == nullptr) ? summaryInfo.bundleName : "";
    moduleName = (summaryInfo.moduleName == nullptr) ? summaryInfo.moduleName : "";
    intentName = (summaryInfo.intentName == nullptr) ? summaryInfo.intentName : "";
    displayName = (summaryInfo.displayName == nullptr) ? summaryInfo.displayName : "";
    intentType = (summaryInfo.decoratorType == nullptr) ? summaryInfo.decoratorType : "";
    return true;
}

bool JsInsightIntentInfo::ExtraInsightIntentInfo2JsInsightIntentInfo(ExtraInsightIntentInfo &fullInfo)
{
    if (fullInfo.genericInfo == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "failed, ExtraInsightIntentInfo is nullptr");
        return false;
    }
    ExtraInsightIntentGenericInfo2JsInsightIntentInfo(*(fullInfo.genericInfo));
    domain = (fullInfo.domain == nullptr) ? fullInfo.domain : "";
    intentVersion = (fullInfo.intentVersion == nullptr) ? fullInfo.intentVersion : "";
    displayDescription = (fullInfo.displayDescription == nullptr) ? fullInfo.displayDescription : "";
    schema = (fullInfo.schema == nullptr) ? fullInfo.schema : "";
    icon = (fullInfo.icon == nullptr) ? fullInfo.icon : "";
    llmDescription = (fullInfo.llmDescription == nullptr) ? fullInfo.llmDescription : "";

    for (auto &keyword : fullInfo.keywords) {
        keywords.emplace_back(keyword);
    }
    return true;
}

bool JsInsightIntentInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName = Str16ToStr8(parcel.ReadString16());
    moduleName = Str16ToStr8(parcel.ReadString16());
    intentName = Str16ToStr8(parcel.ReadString16());
    domain = Str16ToStr8(parcel.ReadString16());
    intentVersion = Str16ToStr8(parcel.ReadString16());
    displayName = Str16ToStr8(parcel.ReadString16());
    displayDescription = Str16ToStr8(parcel.ReadString16());
    schema = Str16ToStr8(parcel.ReadString16());
    icon = Str16ToStr8(parcel.ReadString16());
    llmDescription = Str16ToStr8(parcel.ReadString16());
    intentType = Str16ToStr8(parcel.ReadString16());
    parcel.ReadStringVector(&keywords);
    int32_t infoSize = parcel.ReadInt32();
    params.clear();
    for (int32_t i = 0; i < infoSize; i++) {
        auto info = parcel.ReadParcelable<JsIntentParamInfo>();
        if (info == nullptr) {
            return false;
        }
        params.emplace_back(*info);
    }
    ReadParcelableInfo<JsLinkInfo>(linkInfo, parcel);
    ReadParcelableInfo<JsPageInfo>(pageInfo, parcel);
    ReadParcelableInfo<JsEntryInfo>(entryInfo, parcel);
    return true;
}

bool JsInsightIntentInfo::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(bundleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(moduleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(intentName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(domain));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(intentVersion));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(displayName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(displayDescription));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(schema));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(icon));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(llmDescription));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(intentType));
    if (!parcel.WriteStringVector(keywords)) {
        return false;
    }
    if (!parcel.WriteInt32(params.size())) {
        return false;
    }
    for (auto &param: params) {
        if (!parcel.WriteParcelable(&param)) {
            return false;
        }
    }
    if (!parcel.WriteParcelable(&linkInfo)) {
        return false;
    }
    if (!parcel.WriteParcelable(&pageInfo)) {
        return false;
    }
    if (!parcel.WriteParcelable(&entryInfo)) {
        return false;
    }
    return true;
}

JsInsightIntentInfo *JsInsightIntentInfo::Unmarshalling(Parcel &parcel)
{
    JsInsightIntentInfo *info = new (std::nothrow) JsInsightIntentInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}
} // namespace AbilityRuntime
} // namespace OHOS
 