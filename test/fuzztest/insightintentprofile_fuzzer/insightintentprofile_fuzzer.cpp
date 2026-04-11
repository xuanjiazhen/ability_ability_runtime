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

#include "insightintentprofile_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <nlohmann/json.hpp>

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

    // Test TransformTo with various JSON strings
    std::string profileStr = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::vector<InsightIntentInfo> infos;

    auto result = InsightIntentProfile::TransformTo(profileStr, infos);

    // Test with valid JSON structure
    json validJson = {
        {"intentName", fdp.ConsumeRandomLengthString(64)},
        {"domain", fdp.ConsumeRandomLengthString(64)},
        {"intentVersion", fdp.ConsumeRandomLengthString(16)},
        {"srcEntry", fdp.ConsumeRandomLengthString(64)},
        {"displayName", fdp.ConsumeRandomLengthString(128)},
        {"icon", fdp.ConsumeRandomLengthString(256)},
        {"displayDescription", fdp.ConsumeRandomLengthString(512)},
        {"bundleName", fdp.ConsumeRandomLengthString(128)},
        {"moduleName", fdp.ConsumeRandomLengthString(64)},
        {"arkTSMode", fdp.ConsumeRandomLengthString(16)},
        {"uiAbility", {
            {"ability", fdp.ConsumeRandomLengthString(64)},
            {"executeMode", json::array({
                fdp.ConsumeRandomLengthString(16),
                fdp.ConsumeRandomLengthString(16)
            })}
        }},
        {"inputParams", json::array({
            fdp.ConsumeRandomLengthString(32),
            fdp.ConsumeRandomLengthString(32)
        })},
        {"outputParams", json::array({
            fdp.ConsumeRandomLengthString(32)
        })},
        {"keywords", json::array({
            fdp.ConsumeRandomLengthString(32),
            fdp.ConsumeRandomLengthString(32),
            fdp.ConsumeRandomLengthString(32)
        })}
    };

    std::string validJsonStr = validJson.dump();
    std::vector<InsightIntentInfo> validInfos;
    InsightIntentProfile::TransformTo(validJsonStr, validInfos);

    // Test ToJson with fuzzed data
    if (!validInfos.empty()) {
        json jsonObject;
        InsightIntentProfile::ToJson(validInfos[0], jsonObject);
    }

    // Test with array JSON structure
    json arrayJson = {
        {"insightIntents", json::array({
            {
                {"intentName", fdp.ConsumeRandomLengthString(64)},
                {"domain", fdp.ConsumeRandomLengthString(64)},
                {"bundleName", fdp.ConsumeRandomLengthString(128)}
            },
            {
                {"intentName", fdp.ConsumeRandomLengthString(64)},
                {"domain", fdp.ConsumeRandomLengthString(64)},
                {"bundleName", fdp.ConsumeRandomLengthString(128)}
            }
        })}
    };

    std::string arrayJsonStr = arrayJson.dump();
    std::vector<InsightIntentInfo> arrayInfos;
    InsightIntentProfile::TransformTo(arrayJsonStr, arrayInfos);

    // Test with malformed JSON
    std::string malformedJson = "{invalid json" + fdp.ConsumeRemainingBytesAsString();
    std::vector<InsightIntentInfo> malformedInfos;
    InsightIntentProfile::TransformTo(malformedJson, malformedInfos);

    // Test with empty JSON
    std::vector<InsightIntentInfo> emptyInfos;
    InsightIntentProfile::TransformTo("", emptyInfos);
    InsightIntentProfile::TransformTo("{}", emptyInfos);
    InsightIntentProfile::TransformTo("[]", emptyInfos);

    // Test ToJson with fuzzed InsightIntentInfo
    InsightIntentInfo fuzzInfo;
    fuzzInfo.intentName = fdp.ConsumeRandomLengthString(64);
    fuzzInfo.intentDomain = fdp.ConsumeRandomLengthString(64);
    fuzzInfo.intentVersion = fdp.ConsumeRandomLengthString(16);
    fuzzInfo.srcEntry = fdp.ConsumeRandomLengthString(64);
    fuzzInfo.arkTSMode = fdp.ConsumeRandomLengthString(16);
    fuzzInfo.displayName = fdp.ConsumeRandomLengthString(128);
    fuzzInfo.icon = fdp.ConsumeRandomLengthString(256);
    fuzzInfo.displayDescription = fdp.ConsumeRandomLengthString(512);
    fuzzInfo.bundleName = fdp.ConsumeRandomLengthString(128);
    fuzzInfo.moduleName = fdp.ConsumeRandomLengthString(64);

    // Add random input/output params
    int32_t paramCount = fdp.ConsumeIntegralInRange(0, 10);
    for (int32_t i = 0; i < paramCount; i++) {
        fuzzInfo.inputParams.push_back(fdp.ConsumeRandomLengthString(32));
        fuzzInfo.outputParams.push_back(fdp.ConsumeRandomLengthString(32));
        fuzzInfo.keywords.push_back(fdp.ConsumeRandomLengthString(32));
    }

    // Add random execute modes
    fuzzInfo.uiAbilityIntentInfo.abilityName = fdp.ConsumeRandomLengthString(64);
    int32_t executeModeCount = fdp.ConsumeIntegralInRange(0, 5);
    for (int32_t i = 0; i < executeModeCount; i++) {
        fuzzInfo.uiAbilityIntentInfo.supportExecuteMode.push_back(
            static_cast<ExecuteMode>(fdp.ConsumeIntegralInRange(0, 4))
        );
    }

    json fuzzJsonObject;
    InsightIntentProfile::ToJson(fuzzInfo, fuzzJsonObject);

    // Test with all UIAbility/Extension types
    std::vector<std::string> intentTypes = {
        "UIAbility",
        "UIExtension",
        "ServiceExtension",
        "DataAbility",
        "Form",
        "",
        fdp.ConsumeRandomLengthString(32)
    };

    for (auto type : intentTypes) {
        json typeJson = {
            {"intentName", fdp.ConsumeRandomLengthString(64)},
            {"type", type},
            {"bundleName", fdp.ConsumeRandomLengthString(128)},
            {"abilityName", fdp.ConsumeRandomLengthString(64)},
            {"executeMode", json::array({
                "foreground", "background", "uiextension"
            })}
        };
        std::string typeJsonStr = typeJson.dump();
        std::vector<InsightIntentInfo> typeInfos;
        InsightIntentProfile::TransformTo(typeJsonStr, typeInfos);
    }

    // Test with special characters in all string fields
    json specialJson = {
        {"intentName", "$(malicious)"},
        {"domain", "../../etc/passwd"},
        {"intentVersion", "'; DROP TABLE; --"},
        {"srcEntry", "<script>alert(1)</script>"},
        {"displayName", "\x00\x01\x02"},
        {"icon", "😀🎉🚀"},
        {"displayDescription", "测试中文\n\r\t"},
        {"bundleName", "$(whoami)"},
        {"moduleName", "`id`"},
        {"arkTSMode", "${HOME}"},
        {"keywords", json::array({
            "../../etc/passwd",
            "$(evil)",
            "<script>",
            "';--",
            "\n\r\t\x00"
        })}
    };

    std::string specialJsonStr = specialJson.dump();
    std::vector<InsightIntentInfo> specialInfos;
    InsightIntentProfile::TransformTo(specialJsonStr, specialInfos);

    // Test with extremely large arrays
    json largeArrayJson = {
        {"intentName", "test"},
        {"inputParams", json::array()},
        {"outputParams", json::array()},
        {"keywords", json::array()}
    };

    // Add up to 1000 items to each array
    int32_t largeCount = fdp.ConsumeIntegralInRange(0, 1000);
    for (int32_t i = 0; i < largeCount; i++) {
        largeArrayJson["inputParams"].push_back(fdp.ConsumeRandomLengthString(32));
        largeArrayJson["outputParams"].push_back(fdp.ConsumeRandomLengthString(32));
        largeArrayJson["keywords"].push_back(fdp.ConsumeRandomLengthString(32));
    }

    std::string largeArrayJsonStr = largeArrayJson.dump();
    std::vector<InsightIntentInfo> largeArrayInfos;
    InsightIntentProfile::TransformTo(largeArrayJsonStr, largeArrayInfos);

    // Test with deeply nested JSON
    json nestedJson = {
        {"intentName", "nested"},
        {"nested", {
            {"level1", {
                {"level2", {
                    {"level3", {
                        {"level4", {
                            {"level5", fdp.ConsumeRandomLengthString(64)}
                        }}
                    }}
                }}
            }}
        }}
    };

    std::string nestedJsonStr = nestedJson.dump();
    std::vector<InsightIntentInfo> nestedInfos;
    InsightIntentProfile::TransformTo(nestedJsonStr, nestedInfos);

    // Test with null values
    json nullJson = {
        {"intentName", nullptr},
        {"domain", nullptr},
        {"intentVersion", nullptr},
        {"srcEntry", nullptr},
        {"displayName", nullptr},
        {"icon", nullptr},
        {"displayDescription", nullptr},
        {"bundleName", nullptr},
        {"moduleName", nullptr},
        {"arkTSMode", nullptr}
    };

    std::string nullJsonStr = nullJson.dump();
    std::vector<InsightIntentInfo> nullInfos;
    InsightIntentProfile::TransformTo(nullJsonStr, nullInfos);

    // Test with mixed types
    json mixedJson = {
        {"intentName", 12345},
        {"domain", true},
        {"intentVersion", false},
        {"srcEntry", json::array()},
        {"displayName", 3.14159},
        {"icon", nullptr},
        {"displayDescription", {{}}},
        {"bundleName", json::object()},
        {"moduleName", "normal"}
    };

    std::string mixedJsonStr = mixedJson.dump();
    std::vector<InsightIntentInfo> mixedInfos;
    InsightIntentProfile::TransformTo(mixedJsonStr, mixedInfos);

    // Test with duplicate keys (JSON standard behavior)
    json duplicateJson = {
        {"intentName", "first"},
        {"intentName", "second"},
        {"bundleName", "third"},
        {"bundleName", "fourth"}
    };

    std::string duplicateJsonStr = duplicateJson.dump();
    std::vector<InsightIntentInfo> duplicateInfos;
    InsightIntentProfile::TransformTo(duplicateJsonStr, duplicateInfos);

    // Test ToJson with empty InsightIntentInfo
    InsightIntentInfo emptyInfo;
    json emptyJsonObject;
    InsightIntentProfile::ToJson(emptyInfo, emptyJsonObject);

    // Test ToJson with extreme string lengths
    InsightIntentInfo extremeInfo;
    extremeInfo.intentName = std::string(10000, 'A');
    extremeInfo.displayName = std::string(20000, 'B');
    extremeInfo.displayDescription = std::string(50000, 'C');
    extremeInfo.icon = std::string(100000, 'D');

    for (int i = 0; i < 1000; i++) {
        extremeInfo.inputParams.push_back(std::string(100, 'P'));
        extremeInfo.outputParams.push_back(std::string(100, 'Q'));
        extremeInfo.keywords.push_back(std::string(100, 'K'));
    }

    json extremeJsonObject;
    InsightIntentProfile::ToJson(extremeInfo, extremeJsonObject);

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
