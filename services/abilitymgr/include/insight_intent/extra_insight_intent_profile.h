/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_EXTRA_INSIGHT_INTENT_PROFILE_H
#define OHOS_ABILITY_RUNTIME_EXTRA_INSIGHT_INTENT_PROFILE_H

#include <string>
#include <vector>

#include "insight_intent_execute_param.h"

namespace OHOS {
namespace AbilityRuntime {
using ExecuteMode = AppExecFwk::ExecuteMode;

struct InsightIntentParam {
    char *paramName; // 必选
    bool isRequired; // 必选
    char *paramSchema; // schema解析后
};

struct LinkIntentParamMapping {
    char *paramName; // 必选
    char *paramMappingName; // 可选
    char *paramCategory; // 可选
};

struct LinkInfo {
    char *uri; // 必选
    std::vector<LinkIntentParamMapping> paramMapping {}; // 可选
    // std::vector<InsightIntentParam> params {}; // 可选，schema解析
    char *parameters;
};

struct PageInfo {
    char *uiAbility; // 可选
    char *pageRouteName; // 必选
    char *navigationId; // 必选
    char *navDestination; // 必选
    // std::vector<InsightIntentParam> params {}; // 可选，schema解析
    char *parameters;
};

struct EntryInfo {
    char *abilityName; // 必选
    std::vector<ExecuteMode> executeMode {}; // 必选
    // std::vector<InsightIntentParam> params {}; // 可选，schema解析
    char *parameters;
};

struct FunctionInfo {
    char *functionName; // 必选
    std::vector<std::string> functionParams;
    // std::vector<InsightIntentParam> params {}; // 可选，schema解析
    char *parameters;
};

struct FormInfo {
    // std::vector<InsightIntentParam> params {}; // 可选，schema解析
    char *parameters;
};

// 简要信息
struct ExtraInsightIntentGenericInfo {
    char *bundleName; // 必选
    char *moduleName; // 必选
    char *intentName; // 必选
    char *displayName; // 必选
    char *decoratorType; // 必选
    union {
        LinkInfo linkInfo;
        PageInfo pageInfo;
        EntryInfo entryInfo;
        FunctionInfo functionInfo;
        FormInfo formInfo;
    };
};

// 全量信息
struct ExtraInsightIntentInfo {
    char *decoratorFile; // 必选
    char *decoratorClass; // 必选
    char *displayDescription; // 可选
    char *domain; // 必选
    char *intentVersion; // 必选
    char *schema; // 可选
    char *icon; // 可选
    char *llmDescription; // 可选
    std::vector<std::string> keywords; // 可选
    ExtraInsightIntentGenericInfo genericInfo;
};

struct LinkIntentParamProfileMapping {
    std::string paramName; // 必选
    std::string paramMappingName; // 可选
    std::string paramCategory; // 可选
};
// json parse
struct ExtraInsightIntentProfileInfo {
    std::string decoratorFile; // 必选
    std::string decoratorClass; // 必选
    std::string decoratorType; // 必选
    std::string bundleName; // 必选
    std::string moduleName; // 必选
    std::string intentName; // 必选
    std::string domain; // 必选
    std::string intentVersion; // 必选
    std::string displayName; // 必选
    std::string displayDescription; // 可选
    std::string schema; // 可选
    std::string icon; // 可选
    std::string llmDescription; // 可选
    std::vector<std::string> keywords; // 可选
    std::string parameters; // json schema模块解析
    //LinkInfo linkInfo;
    std::string uri;
    std::vector<LinkIntentParamProfileMapping> paramMapping {};
    // PageInfo pageInfo;
    std::string uiAbility; // 可选
    std::string pageRouteName; // 必选
    std::string navigationId; // 可选
    std::string navDestination; // 可选
    // EntryInfo entryInfo;
    std::string abilityName; // 必选
    std::vector<std::string> executeMode {}; // 必选
    // FunctionInfo functionInfo;
    std::string functionName; // 必选
    std::vector<std::string> functionParams; // 函数参数名列表，可选
    // FormInfo formInfo;
};

struct ExtraInsightIntentProfileInfoVec {
    std::vector<ExtraInsightIntentProfileInfo> insightIntents {};
};

class ExtraInsightIntentProfile {
public:
    static bool TransformTo(const std::string &profileStr, ExtraInsightIntentProfileInfoVec &infos);
    static bool ToJson(const ExtraInsightIntentProfileInfo &info, nlohmann::json &jsonObject);
    static bool ProfileInfoFormat(const ExtraInsightIntentProfileInfo &insightIntent, ExtraInsightIntentInfo &info);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_EXTRA_INSIGHT_INTENT_PROFILE_H
