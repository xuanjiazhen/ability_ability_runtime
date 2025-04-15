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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_INFO_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_INFO_H

#include <string>
#include <vector>

#include "parcel.h"
#include "insight_intent_constant.h"
#include "extra_insight_intent_profile.h"

namespace OHOS {
namespace AbilityRuntime {
using ExtraInsightIntentGenericInfo = AAFwk::ExtraInsightIntentGenericInfo;
using ExtraInsightIntentInfo = AAFwk::ExtraInsightIntentInfo;
// using InsightIntentExecuteMode = AbilityRuntime::InsightIntentExecuteMode;

struct JsLinkInfo : public Parcelable {
    std::string uri;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static JsLinkInfo *Unmarshalling(Parcel &parcel);
};

struct JsPageInfo : public Parcelable  {
    std::string uiAbility;
    std::string pageRouterName;
    std::string navigationId;
    std::string navDestination;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static JsPageInfo *Unmarshalling(Parcel &parcel);
};

struct JsEntryInfo : public Parcelable  {
    std::string abilityName;
    InsightIntentExecuteMode executeMode;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static JsEntryInfo *Unmarshalling(Parcel &parcel);
};

struct JsFunctionInfo : public Parcelable  {
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static JsFunctionInfo *Unmarshalling(Parcel &parcel);
};

struct JsFormInfo : public Parcelable  {
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static JsFormInfo *Unmarshalling(Parcel &parcel);
};

struct JsIntentParamInfo : public Parcelable  {
    std::string paramName;
    std::string paramDescription;
    std::string paramType;
    bool isRequired;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static JsIntentParamInfo *Unmarshalling(Parcel &parcel);
};

struct JsInsightIntentInfo : public Parcelable {
    std::string bundleName;
    std::string moduleName;
    std::string intentName;
    std::string domain;
    std::string intentVersion;
    std::string displayName;
    std::string displayDescription;
    std::string schema;
    std::string icon;
    std::string llmDescription;
    std::string intentType;
    std::vector<std::string> keywords;
    std::vector<JsIntentParamInfo> params;
    JsLinkInfo linkInfo;
    JsPageInfo pageInfo;
    JsEntryInfo entryInfo;
    JsFunctionInfo functionInfo;
    JsFormInfo formInfo;

    bool ExtraInsightIntentGenericInfo2JsInsightIntentInfo(ExtraInsightIntentGenericInfo &summaryInfo);
    bool ExtraInsightIntentInfo2JsInsightIntentInfo(ExtraInsightIntentInfo &fullInfo);
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static JsInsightIntentInfo *Unmarshalling(Parcel &parcel);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_INFO_H
