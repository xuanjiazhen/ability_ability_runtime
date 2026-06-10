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

#include "insight_intent/ui_operation_chain_param.h"

#include "hilog_tag_wrapper.h"

namespace nlohmann {
template <typename T>
class adl_serializer;
} // namespace nlohmann

#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {

UiOperationChain UiOperationChain::ParseFromJson(const std::string& jsonStr)
{
    UiOperationChain chain;
    if (jsonStr.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "jsonStr is empty");
        return chain;
    }

    try {
        auto json = nlohmann::json::parse(jsonStr);
        if (!json.is_object()) {
            TAG_LOGE(AAFwkTag::INTENT, "json root is not object");
            return chain;
        }

        if (json.contains("windowId") && json["windowId"].is_number_integer()) {
            chain.windowId = json["windowId"].get<int32_t>();
        }
        if (json.contains("totalTimeoutMs") && json["totalTimeoutMs"].is_number_integer()) {
            chain.totalTimeoutMs = json["totalTimeoutMs"].get<int32_t>();
        }

        if (!json.contains("steps") || !json["steps"].is_array()) {
            TAG_LOGE(AAFwkTag::INTENT, "missing or invalid 'steps' field");
            return chain;
        }

        if (json.contains("waitForPageChange") && json["waitForPageChange"].is_boolean()) {
            chain.waitForPageChange = json["waitForPageChange"].get<bool>();
        }
        if (json.contains("pageChangeTimeoutMs") && json["pageChangeTimeoutMs"].is_number_integer()) {
            chain.pageChangeTimeoutMs = json["pageChangeTimeoutMs"].get<int32_t>();
        }

        for (const auto& stepJson : json["steps"]) {
            UiOperationStep step;
            if (stepJson.contains("compId") && stepJson["compId"].is_string()) {
                step.compId = stepJson["compId"].get<std::string>();
            }
            if (stepJson.contains("compType") && stepJson["compType"].is_string()) {
                step.compType = stepJson["compType"].get<std::string>();
            } else if (stepJson.contains("componentType") && stepJson["componentType"].is_string()) {
                step.compType = stepJson["componentType"].get<std::string>();
            }
            if (stepJson.contains("compName") && stepJson["compName"].is_string()) {
                step.compName = stepJson["compName"].get<std::string>();
            }
            if (stepJson.contains("compText") && stepJson["compText"].is_string()) {
                step.compText = stepJson["compText"].get<std::string>();
            } else if (stepJson.contains("contentMatcher") && stepJson["contentMatcher"].is_string()) {
                step.compText = stepJson["contentMatcher"].get<std::string>();
            }
            if (stepJson.contains("compIndex") && stepJson["compIndex"].is_number_integer()) {
                step.compIndex = stepJson["compIndex"].get<int32_t>();
            } else if (stepJson.contains("matchIndex") && stepJson["matchIndex"].is_number_integer()) {
                step.compIndex = stepJson["matchIndex"].get<int32_t>();
            }
            if (stepJson.contains("command") && stepJson["command"].is_string()) {
                step.command = stepJson["command"].get<std::string>();
            }
            if (stepJson.contains("useAsync") && stepJson["useAsync"].is_boolean()) {
                step.useAsync = stepJson["useAsync"].get<bool>();
            }
            if (stepJson.contains("timeoutMs") && stepJson["timeoutMs"].is_number_integer()) {
                step.timeoutMs = stepJson["timeoutMs"].get<int32_t>();
            }
            step.SyncLegacy();
            chain.steps.push_back(step);
        }
    } catch (const nlohmann::json::exception& e) {
        TAG_LOGE(AAFwkTag::INTENT, "JSON parse error: %{public}s", e.what());
    }

    return chain;
}

std::string UiOperationChain::ToJsonString() const
{
    auto json = nlohmann::json::object();
    json["windowId"] = windowId;
    json["totalTimeoutMs"] = totalTimeoutMs;
    json["waitForPageChange"] = waitForPageChange;
    json["pageChangeTimeoutMs"] = pageChangeTimeoutMs;

    auto stepsArr = nlohmann::json::array();
    for (const auto& step : steps) {
        auto stepObj = nlohmann::json::object();
        stepObj["compId"] = step.compId;
        stepObj["compType"] = step.compType;
        stepObj["compName"] = step.compName;
        stepObj["compText"] = step.compText;
        stepObj["compIndex"] = step.compIndex;
        stepObj["command"] = step.command;
        stepObj["useAsync"] = step.useAsync;
        stepObj["timeoutMs"] = step.timeoutMs;
        stepsArr.push_back(stepObj);
    }
    json["steps"] = stepsArr;

    try {
        return json.dump();
    } catch (const nlohmann::json::exception& e) {
        TAG_LOGE(AAFwkTag::INTENT, "JSON dump error: %{public}s", e.what());
        return "";
    }
}

bool UiOperationChain::IsValid() const
{
    return windowId >= 0 && !steps.empty();
}

} // namespace AAFwk
} // namespace OHOS
