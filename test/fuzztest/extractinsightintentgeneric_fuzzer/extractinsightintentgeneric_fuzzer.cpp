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

#include "extractinsightintentgeneric_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <nlohmann/json.hpp>

#include "extract_insight_intent_profile.h"
#include "insight_intent_profile.h"

using namespace OHOS::AbilityRuntime;
using json = nlohmann::json;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 512;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    if (size < 20) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    // Test LinkInfo variant
    InsightIntentLinkInfo linkInfo;
    linkInfo.uri = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    linkInfo.parameters = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    int32_t mappingCount = fdp.ConsumeIntegralInRange(0, 10);
    for (int32_t i = 0; i < mappingCount; i++) {
        LinkIntentParamMapping mapping;
        mapping.paramName = fdp.ConsumeRandomLengthString(64);
        mapping.paramMappingName = fdp.ConsumeRandomLengthString(64);
        mapping.paramCategory = fdp.ConsumeRandomLengthString(64);
        linkInfo.paramMapping.push_back(mapping);
    }

    // Test PageInfo variant
    InsightIntentPageInfo pageInfo;
    pageInfo.uiAbility = fdp.ConsumeRandomLengthString(128);
    pageInfo.pagePath = fdp.ConsumeRandomLengthString(256);
    pageInfo.navigationId = fdp.ConsumeRandomLengthString(128);
    pageInfo.navDestinationName = fdp.ConsumeRandomLengthString(128);
    pageInfo.parameters = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    // Test EntryInfo variant
    InsightIntentEntryInfo entryInfo;
    entryInfo.abilityName = fdp.ConsumeRandomLengthString(128);

    int32_t modeCount = fdp.ConsumeIntegralInRange(0, 5);
    for (int32_t i = 0; i < modeCount; i++) {
        entryInfo.executeMode.push_back(
            static_cast<ExecuteMode>(fdp.ConsumeIntegralInRange(0, 4))
        );
    }
    entryInfo.parameters = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    // Test FunctionInfo variant
    InsightIntentFunctionInfo functionInfo;
    functionInfo.functionName = fdp.ConsumeRandomLengthString(128);

    int32_t paramCount = fdp.ConsumeIntegralInRange(0, 10);
    for (int32_t i = 0; i < paramCount; i++) {
        functionInfo.functionParams.push_back(fdp.ConsumeRandomLengthString(64));
    }
    functionInfo.parameters = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    // Test FormInfo variant
    InsightIntentFormInfo formInfo;
    formInfo.abilityName = fdp.ConsumeRandomLengthString(128);
    formInfo.formName = fdp.ConsumeRandomLengthString(128);
    formInfo.parameters = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    // Test ExtractInsightIntentGenericInfo with all variants
    ExtractInsightIntentGenericInfo genericInfo;
    genericInfo.bundleName = fdp.ConsumeRandomLengthString(128);
    genericInfo.moduleName = fdp.ConsumeRandomLengthString(64);
    genericInfo.intentName = fdp.ConsumeRandomLengthString(64);
    genericInfo.displayName = fdp.ConsumeRandomLengthString(128);
    genericInfo.decoratorType = fdp.ConsumeRandomLengthString(64);

    // Test setting different data types
    std::vector<InfoType> types = {
        InfoType::Link, InfoType::Page, InfoType::Entry,
        InfoType::Function, InfoType::Form, InfoType::None
    };

    for (auto type : types) {
        ExtractInsightIntentGenericInfo testInfo;
        testInfo.bundleName = fdp.ConsumeRandomLengthString(128);
        testInfo.moduleName = fdp.ConsumeRandomLengthString(64);
        testInfo.intentName = fdp.ConsumeRandomLengthString(64);
        testInfo.displayName = fdp.ConsumeRandomLengthString(128);
        testInfo.decoratorType = fdp.ConsumeRandomLengthString(64);

        switch (type) {
            case InfoType::Link:
                testInfo.set<InsightIntentLinkInfo>();
                testInfo.get<InsightIntentLinkInfo>() = linkInfo;
                break;
            case InfoType::Page:
                testInfo.set<InsightIntentPageInfo>();
                testInfo.get<InsightIntentPageInfo>() = pageInfo;
                break;
            case InfoType::Entry:
                testInfo.set<InsightIntentEntryInfo>();
                testInfo.get<InsightIntentEntryInfo>() = entryInfo;
                break;
            case InfoType::Function:
                testInfo.set<InsightIntentFunctionInfo>();
                testInfo.get<InsightIntentFunctionInfo>() = functionInfo;
                break;
            case InfoType::Form:
                testInfo.set<InsightIntentFormInfo>();
                testInfo.get<InsightIntentFormInfo>() = formInfo;
                break;
            default:
                break;
        }
    }

    // Test ExtractInsightIntentInfo with entities
    ExtractInsightIntentInfo intentInfo;
    intentInfo.decoratorFile = fdp.ConsumeRandomLengthString(256);
    intentInfo.decoratorClass = fdp.ConsumeRandomLengthString(128);
    intentInfo.displayDescription = fdp.ConsumeRandomLengthString(512);
    intentInfo.domain = fdp.ConsumeRandomLengthString(128);
    intentInfo.intentVersion = fdp.ConsumeRandomLengthString(32);
    intentInfo.schema = fdp.ConsumeRandomLengthString(512);
    intentInfo.icon = fdp.ConsumeRandomLengthString(256);
    intentInfo.llmDescription = fdp.ConsumeRandomLengthString(512);
    intentInfo.result = fdp.ConsumeRandomLengthString(512);
    intentInfo.example = fdp.ConsumeRandomLengthString(512);

    // Add entities
    int32_t entityCount = fdp.ConsumeIntegralInRange(0, 10);
    for (int32_t i = 0; i < entityCount; i++) {
        InsightIntentEntityInfo entity;
        entity.decoratorFile = fdp.ConsumeRandomLengthString(256);
        entity.className = fdp.ConsumeRandomLengthString(128);
        entity.decoratorType = fdp.ConsumeRandomLengthString(64);
        entity.entityId = fdp.ConsumeRandomLengthString(64);
        entity.entityCategory = fdp.ConsumeRandomLengthString(64);
        entity.parameters = fdp.ConsumeRandomLengthString(512);
        entity.parentClassName = fdp.ConsumeRandomLengthString(128);
        intentInfo.entities.push_back(entity);
    }

    // Add keywords
    int32_t keywordCount = fdp.ConsumeIntegralInRange(0, 20);
    for (int32_t i = 0; i < keywordCount; i++) {
        intentInfo.keywords.push_back(fdp.ConsumeRandomLengthString(64));
    }

    // Test TransformTo with JSON
    json testJson = {
        {"insightIntents", json::array({
            {
                {"decoratorFile", fdp.ConsumeRandomLengthString(256)},
                {"decoratorClass", fdp.ConsumeRandomLengthString(128)},
                {"decoratorType", INSIGHT_INTENTS_DECORATOR_TYPE_LINK},
                {"bundleName", fdp.ConsumeRandomLengthString(128)},
                {"moduleName", fdp.ConsumeRandomLengthString(64)},
                {"intentName", fdp.ConsumeRandomLengthString(64)},
                {"displayName", fdp.ConsumeRandomLengthString(128)},
                {"domain", fdp.ConsumeRandomLengthString(128)},
                {"intentVersion", fdp.ConsumeRandomLengthString(32)},
                {"uri", fdp.ConsumeRandomLengthString(256)},
                {"parameters", fdp.ConsumeRandomLengthString(512)},
                {"entities", json::array()}
            },
            {
                {"decoratorFile", fdp.ConsumeRandomLengthString(256)},
                {"decoratorClass", fdp.ConsumeRandomLengthString(128)},
                {"decoratorType", INSIGHT_INTENTS_DECORATOR_TYPE_PAGE},
                {"bundleName", fdp.ConsumeRandomLengthString(128)},
                {"moduleName", fdp.ConsumeRandomLengthString(64)},
                {"intentName", fdp.ConsumeRandomLengthString(64)},
                {"uiAbility", fdp.ConsumeRandomLengthString(128)},
                {"pagePath", fdp.ConsumeRandomLengthString(256)},
                {"navigationId", fdp.ConsumeRandomLengthString(128)},
                {"navDestinationName", fdp.ConsumeRandomLengthString(128)},
                {"entities", json::array()}
            }
        })}
    };

    std::string jsonStr = testJson.dump();
    ExtractInsightIntentProfileInfoVec profileInfos;
    ExtractInsightIntentProfile::TransformTo(jsonStr, profileInfos);

    // Test ToJson
    ExtractInsightIntentProfileInfo profileInfo;
    profileInfo.bundleName = fdp.ConsumeRandomLengthString(128);
    profileInfo.moduleName = fdp.ConsumeRandomLengthString(64);
    profileInfo.intentName = fdp.ConsumeRandomLengthString(64);
    profileInfo.decoratorType = INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION;
    profileInfo.functionName = fdp.ConsumeRandomLengthString(128);

    json jsonObject;
    ExtractInsightIntentProfile::ToJson(profileInfo, jsonObject);

    // Test ProfileInfoFormat
    ExtractInsightIntentInfo formattedInfo;
    ExtractInsightIntentProfile::ProfileInfoFormat(profileInfo, formattedInfo);

    // Test with malformed JSON
    std::string malformedJson = "{invalid" + fdp.ConsumeRemainingBytesAsString();
    ExtractInsightIntentProfileInfoVec malformedInfos;
    ExtractInsightIntentProfile::TransformTo(malformedJson, malformedInfos);

    // Test with extreme JSON sizes
    std::vector<std::string> extremeJsons = {
        "{}",
        "[]",
        "{\"a\":\"b\"}",
        std::string(10000, '{'),
        std::string(10000, '['),
        "{\"nested\":" + std::string(1000, '{') + "}}",
        "{\"array\":" + std::string(1000, '[') + "]}",
        fdp.ConsumeRandomLengthString(10000)
    };

    for (auto testJsonStr : extremeJsons) {
        ExtractInsightIntentProfileInfoVec testInfos;
        ExtractInsightIntentProfile::TransformTo(testJsonStr, testInfos);
    }

    // Test InsightIntentProfile TransformTo
    std::vector<InsightIntentInfo> configInfos;
    InsightIntentProfile::TransformTo(jsonStr, configInfos);

    // Test with special characters in JSON
    json specialJson = {
        {"insightIntents", json::array({
            {
                {"intentName", "$(whoami)"},
                {"domain", "../../etc/passwd"},
                {"displayName", "<script>alert(1)</script>"},
                {"icon", "\x00\x01\x02"},
                {"displayDescription", "'; DROP TABLE users; --"},
                {"keywords", json::array({"😀", "🎉", "测试", "\n\r\t"})}
            }
        })}
    };

    std::string specialJsonStr = specialJson.dump();
    ExtractInsightIntentProfileInfoVec specialInfos;
    ExtractInsightIntentProfile::TransformTo(specialJsonStr, specialInfos);

    // Test with all decorator types
    std::vector<std::string> decoratorTypes = {
        INSIGHT_INTENTS_DECORATOR_TYPE_LINK,
        INSIGHT_INTENTS_DECORATOR_TYPE_PAGE,
        INSIGHT_INTENTS_DECORATOR_TYPE_ENTRY,
        INSIGHT_INTENTS_DECORATOR_TYPE_FUNCTION,
        INSIGHT_INTENTS_DECORATOR_TYPE_FORM,
        "@InvalidDecorator",
        "",
        fdp.ConsumeRandomLengthString(64)
    };

    for (auto decoType : decoratorTypes) {
        json decoJson = {
            {"decoratorType", decoType},
            {"intentName", fdp.ConsumeRandomLengthString(64)},
            {"bundleName", fdp.ConsumeRandomLengthString(128)}
        };
        std::string decoJsonStr = decoJson.dump();
        ExtractInsightIntentProfileInfoVec decoInfos;
        ExtractInsightIntentProfile::TransformTo(decoJsonStr, decoInfos);
    }

    return true;
}

} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
