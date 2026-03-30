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

#include "ei_command.h"
#include "want_params.h"
#include "want_params_wrapper.h"
#include "string_wrapper.h"
#include "int_wrapper.h"
#include "bool_wrapper.h"
#include "long_wrapper.h"
#include "float_wrapper.h"
#include "double_wrapper.h"
#include "array_wrapper.h"
#include <charconv>
#include <getopt.h>
#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_host_client.h"
#include "insight_intent_info_for_query.h"
#include "insight_intent_constant.h"
#include <nlohmann/json.hpp>

namespace OHOS {
namespace AAFwk {
namespace {
const std::string SHORT_OPTIONS_EXECUTE = "hb:m:a:i:e:p:";
constexpr struct option LONG_OPTIONS_EXECUTE[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle", required_argument, nullptr, 'b'},
    {"module", required_argument, nullptr, 'm'},
    {"ability", required_argument, nullptr, 'a'},
    {"intent", required_argument, nullptr, 'i'},
    {"execute-mode", required_argument, nullptr, 'e'},
    {"param", required_argument, nullptr, 'p'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_LIST = "hf:u:";
constexpr struct option LONG_OPTIONS_LIST[] = {
    {"help", no_argument, nullptr, 'h'},
    {"flag", required_argument, nullptr, 'f'},
    {"user-id", required_argument, nullptr, 'u'},
    {nullptr, 0, nullptr, 0},
};
}  // namespace

ExecuteIntentShellCommand::ExecuteIntentShellCommand(int argc, char* argv[])
    : ShellCommand(argc, argv, TOOL_NAME)
{}

ErrCode ExecuteIntentShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", [this]() { return this->RunAsHelpCommand(); }},
        {"execute", [this]() { return this->RunAsExecuteCommand(); }},
        {"list", [this]() { return this->RunAsListCommand(); }},
    };
    return OHOS::ERR_OK;
}

ErrCode ExecuteIntentShellCommand::CreateMessageMap()
{
    return OHOS::ERR_OK;
}

ErrCode ExecuteIntentShellCommand::init()
{
    startTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    return AbilityManagerClient::GetInstance()->Connect();
}

ErrCode ExecuteIntentShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(HELP_MSG);
    return OHOS::ERR_OK;
}

ErrCode ExecuteIntentShellCommand::RunAsExecuteCommand()
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "ei execute command called");

    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string insightIntentName;
    std::string intentParamJson;
    int32_t executeMode = -1;

    int option = -1;
    int counter = 0;
    ErrCode result = OHOS::ERR_OK;

    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_EXECUTE.c_str(),
            LONG_OPTIONS_EXECUTE, nullptr);

        TAG_LOGI(AAFwkTag::AA_TOOL, "option: %{public}d, optopt: %{public}d, optind: %{public}d",
            option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            resultReceiver_.append(HELP_MSG_EXECUTE + "\n");
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                resultReceiver_.append(HELP_MSG_EXECUTE + "\n");
            }
            break;
        }

        if (option == '?') {
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
            TAG_LOGI(AAFwkTag::AA_TOOL, "'ei execute' option unknown");
            resultReceiver_.append(unknownOptionMsg);
            resultReceiver_.append(HELP_MSG_EXECUTE + "\n");
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }

        switch (option) {
            case 'h': {
                resultReceiver_.append(HELP_MSG_EXECUTE + "\n");
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'b': {
                if (optarg == nullptr) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "bundle name is null");
                    resultReceiver_.append("error: option -b requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                bundleName = optarg;
                TAG_LOGI(AAFwkTag::AA_TOOL, "bundleName: %{public}s", bundleName.c_str());
                break;
            }
            case 'm': {
                if (optarg == nullptr) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "module name is null");
                    resultReceiver_.append("error: option -m requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                moduleName = optarg;
                TAG_LOGI(AAFwkTag::AA_TOOL, "moduleName: %{public}s", moduleName.c_str());
                break;
            }
            case 'a': {
                if (optarg == nullptr) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "ability name is null");
                    resultReceiver_.append("error: option -a requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                abilityName = optarg;
                TAG_LOGI(AAFwkTag::AA_TOOL, "abilityName: %{public}s", abilityName.c_str());
                break;
            }
            case 'i': {
                if (optarg == nullptr) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "intent name is null");
                    resultReceiver_.append("error: option -i requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                insightIntentName = optarg;
                TAG_LOGI(AAFwkTag::AA_TOOL, "insightIntentName: %{public}s", insightIntentName.c_str());
                break;
            }
            case 'e': {
                if (optarg == nullptr) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "execute mode is null");
                    resultReceiver_.append("error: option -e requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                auto res = std::from_chars(optarg, optarg + strlen(optarg), executeMode);
                if (res.ec != std::errc() || res.ptr != optarg + strlen(optarg)) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "invalid execute mode: %{public}s", optarg);
                    resultReceiver_.append("error: invalid execute mode value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                break;
            }
            case 'p': {
                if (optarg == nullptr) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "intent param is null");
                    resultReceiver_.append("error: option -p requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                intentParamJson = optarg;
                TAG_LOGI(AAFwkTag::AA_TOOL, "intentParamJson: %{public}s", intentParamJson.c_str());
                break;
            }
            default: {
                break;
            }
        }

        if (result != OHOS::ERR_OK) {
            break;
        }
    }

    if (result == OHOS::ERR_OK) {
        if (bundleName.empty() || moduleName.empty() || abilityName.empty() ||
            insightIntentName.empty() || executeMode < 0) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "missing required parameters");
            resultReceiver_.append("error: missing required parameters.\n");
            resultReceiver_.append("Required: -b, -m, -a, -i, -e\n");
            resultReceiver_.append(HELP_MSG_EXECUTE + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        } else {
            InsightIntentExecuteParam param;
            param.bundleName_ = bundleName;
            param.moduleName_ = moduleName;
            param.abilityName_ = abilityName;
            param.insightIntentName_ = insightIntentName;
            param.executeMode_ = executeMode;
            if (!intentParamJson.empty()) {
                param.insightIntentParam_ = std::make_shared<WantParams>(BuildWantParamsFromJson(intentParamJson));
                TAG_LOGI(AAFwkTag::AA_TOOL, "intentParam String: %{public}s",
                    param.insightIntentParam_->ToString().c_str());
            } else {
                param.insightIntentParam_ = std::make_shared<WantParams>();
            }
            TAG_LOGI(AAFwkTag::AA_TOOL, "Executing insight intent: bundle=%{public}s, module=%{public}s, "
                "ability=%{public}s, intent=%{public}s, mode=%{public}d",
                bundleName.c_str(), moduleName.c_str(), abilityName.c_str(),
                insightIntentName.c_str(), executeMode);

            InsightIntentExecuteResult executeResult;
            auto err = AbilityManagerClient::GetInstance()->ExecuteIntentWithResult(param, executeResult, 30000);

            if (err == OHOS::ERR_OK) {
                TAG_LOGI(AAFwkTag::AA_TOOL, "execute intent successfully, innerErr=%{public}d, code=%{public}d",
                    executeResult.innerErr, executeResult.code);
                resultReceiver_.append("execute intent successfully.\n");
                resultReceiver_.append("Result:\n");
                resultReceiver_.append("  innerErr: " + std::to_string(executeResult.innerErr) + "\n");
                resultReceiver_.append("  code: " + std::to_string(executeResult.code) + "\n");
                resultReceiver_.append("  flags: " + std::to_string(executeResult.flags) + "\n");

                if (executeResult.result != nullptr) {
                    resultReceiver_.append("  result: " + executeResult.result->ToString() + "\n");
                }
                if (!executeResult.uris.empty()) {
                    resultReceiver_.append("  uris: " + std::to_string(executeResult.uris.size()) + " item(s)\n");
                    for (const auto& uri : executeResult.uris) {
                        resultReceiver_.append("    - " + uri + "\n");
                    }
                }
            } else {
                TAG_LOGE(AAFwkTag::AA_TOOL, "execute intent failed: %{public}d", err);
                resultReceiver_.append("error: failed to execute intent.\n");
                resultReceiver_.append(GetMessageFromCode(err));
                result = err;
            }
        }
    }

    return result;
}

ErrCode ExecuteIntentShellCommand::RunAsListCommand()
{
    TAG_LOGI(AAFwkTag::AA_TOOL, "ei list command called");

    int32_t flag = -1;
    int32_t userId = DEFAULT_INVAL_VALUE;
    int option = -1;
    int counter = 0;
    ErrCode result = OHOS::ERR_OK;

    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_LIST.c_str(),
            LONG_OPTIONS_LIST, nullptr);

        TAG_LOGI(AAFwkTag::AA_TOOL, "option: %{public}d, optopt: %{public}d, optind: %{public}d",
            option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            resultReceiver_.append(HELP_MSG_LIST + "\n");
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1 && strcmp(argv_[optind], cmd_.c_str()) == 0) {
                resultReceiver_.append(HELP_MSG_NO_OPTION + "\n");
                resultReceiver_.append(HELP_MSG_LIST + "\n");
            }
            break;
        }

        if (option == '?') {
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
            TAG_LOGI(AAFwkTag::AA_TOOL, "'ei list' option unknown");
            resultReceiver_.append(unknownOptionMsg);
            resultReceiver_.append(HELP_MSG_LIST + "\n");
            result = OHOS::ERR_INVALID_VALUE;
            break;
        }

        switch (option) {
            case 'h': {
                resultReceiver_.append(HELP_MSG_LIST + "\n");
                result = OHOS::ERR_INVALID_VALUE;
                break;
            }
            case 'f': {
                if (optarg == nullptr) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "flag is null");
                    resultReceiver_.append("error: option -f requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                auto res = std::from_chars(optarg, optarg + strlen(optarg), flag);
                if (res.ec != std::errc() || res.ptr != optarg + strlen(optarg)) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "invalid flag: %{public}s", optarg);
                    resultReceiver_.append("error: invalid flag value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                TAG_LOGI(AAFwkTag::AA_TOOL, "flag: %{public}d", flag);
                break;
            }
            case 'u': {
                if (optarg == nullptr) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "user id is null");
                    resultReceiver_.append("error: option -u requires a value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                auto res = std::from_chars(optarg, optarg + strlen(optarg), userId);
                if (res.ec != std::errc() || res.ptr != optarg + strlen(optarg)) {
                    TAG_LOGE(AAFwkTag::AA_TOOL, "invalid user id: %{public}s", optarg);
                    resultReceiver_.append("error: invalid user id value.\n");
                    result = OHOS::ERR_INVALID_VALUE;
                    break;
                }
                TAG_LOGI(AAFwkTag::AA_TOOL, "userId: %{public}d", userId);
                break;
            }
            default: {
                break;
            }
        }

        if (result != OHOS::ERR_OK) {
            break;
        }
    }

    if (result == OHOS::ERR_OK) {
        if (flag < 0) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "missing required parameter flag");
            resultReceiver_.append("error: missing required parameter -f.\n");
            resultReceiver_.append(HELP_MSG_LIST + "\n");
            result = OHOS::ERR_INVALID_VALUE;
        } else {
            std::vector<AbilityRuntime::InsightIntentInfoForQuery> infos;
            TAG_LOGI(AAFwkTag::AA_TOOL, "Getting all insight intent info with flag: %{public}d, userId: %{public}d",
                flag, userId);

            auto err = AbilityManagerClient::GetInstance()->GetAllInsightIntentInfo(
                static_cast<AbilityRuntime::GetInsightIntentFlag>(flag), infos, userId);

            if (err == OHOS::ERR_OK) {
                TAG_LOGI(AAFwkTag::AA_TOOL, "get all insight intent info successfully, count: %{public}zu",
                    infos.size());
                resultReceiver_.append("get all insight intent info successfully.\n");
                resultReceiver_.append("Total count: " + std::to_string(infos.size()) + "\n");

                for (size_t i = 0; i < infos.size(); i++) {
                    resultReceiver_.append("\n--- Insight Intent " + std::to_string(i + 1) + " ---\n");
                    resultReceiver_.append("BundleName: " + infos[i].bundleName + "\n");
                    resultReceiver_.append("ModuleName: " + infos[i].moduleName + "\n");
                    resultReceiver_.append("IntentName: " + infos[i].intentName + "\n");
                    resultReceiver_.append("DisplayName: " + infos[i].displayName + "\n");
                    resultReceiver_.append("Domain: " + infos[i].domain + "\n");
                    resultReceiver_.append("IntentVersion: " + infos[i].intentVersion + "\n");
                    resultReceiver_.append("IntentType: " + infos[i].intentType + "\n");
                }
            } else {
                TAG_LOGE(AAFwkTag::AA_TOOL, "get all insight intent info failed: %{public}d", err);
                resultReceiver_.append("error: failed to get all insight intent info.\n");
                resultReceiver_.append(GetMessageFromCode(err));
                result = err;
            }
        }
    }

    return result;
}

WantParams ExecuteIntentShellCommand::BuildWantParamsFromJson(const std::string& jsonStr)
{
    WantParams wantParams;

    if (jsonStr.empty()) {
        TAG_LOGW(AAFwkTag::AA_TOOL, "json string is empty");
        return wantParams;
    }

    try {
        nlohmann::json jsonObj = nlohmann::json::parse(jsonStr);

        if (!jsonObj.is_object()) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "json is not an object");
            return wantParams;
        }

        wantParams = ParseJsonObjectToWantParams(jsonObj);

        TAG_LOGI(AAFwkTag::AA_TOOL, "BuildWantParamsFromJson success, params count: %{public}d", wantParams.Size());
    } catch (const nlohmann::json::exception& e) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "json parse error: %{public}s", e.what());
    }

    return wantParams;
}

WantParams ExecuteIntentShellCommand::ParseJsonObjectToWantParams(const nlohmann::json& jsonObj)
{
    WantParams wantParams;

    if (!jsonObj.is_object()) {
        return wantParams;
    }

    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
        const std::string& key = it.key();
        const nlohmann::json& value = it.value();

        if (value.is_string()) {
            wantParams.SetParam(key, String::Box(value.get<std::string>()));
        } else if (value.is_number_integer()) {
            int64_t longValue = value.get<int64_t>();
            if (longValue > INT32_MAX || longValue < INT32_MIN) {
                wantParams.SetParam(key, Long::Box(longValue));
            } else {
                wantParams.SetParam(key, Integer::Box(static_cast<int32_t>(longValue)));
            }
        } else if (value.is_number_float()) {
            wantParams.SetParam(key, String::Box(std::to_string(value.get<double>())));
        } else if (value.is_boolean()) {
            wantParams.SetParam(key, Boolean::Box(value.get<bool>()));
        } else if (value.is_null()) {
            wantParams.SetParam(key, String::Box(""));
        } else if (value.is_object()) {
            WantParams nestedParams = ParseJsonObjectToWantParams(value);
            wantParams.SetParam(key, WantParamWrapper::Box(nestedParams));
        } else if (value.is_array()) {
            sptr<IArray> arrayObj = ParseJsonArrayToIArray(value);
            if (arrayObj != nullptr) {
                wantParams.SetParam(key, arrayObj);
            }
        } else {
            TAG_LOGW(AAFwkTag::AA_TOOL, "unsupported json type for key: %{public}s", key.c_str());
        }
    }

    return wantParams;
}

sptr<IArray> ExecuteIntentShellCommand::ParseJsonArrayToIArray(const nlohmann::json& jsonArr)
{
    if (!jsonArr.is_array()) {
        return nullptr;
    }

    std::vector<sptr<IInterface>> itemsArray;

    for (const auto& item : jsonArr) {
        if (item.is_object()) {
            WantParams itemParams = ParseJsonObjectToWantParams(item);
            itemsArray.push_back(WantParamWrapper::Box(itemParams));
        } else if (item.is_string()) {
            itemsArray.push_back(String::Box(item.get<std::string>()));
        } else if (item.is_number_integer()) {
            int64_t longValue = item.get<int64_t>();
            if (longValue > INT32_MAX || longValue < INT32_MIN) {
                itemsArray.push_back(Long::Box(longValue));
            } else {
                itemsArray.push_back(Integer::Box(static_cast<int32_t>(longValue)));
            }
        } else if (item.is_number_float()) {
            itemsArray.push_back(String::Box(std::to_string(item.get<double>())));
        } else if (item.is_boolean()) {
            itemsArray.push_back(Boolean::Box(item.get<bool>()));
        } else {
            TAG_LOGW(AAFwkTag::AA_TOOL, "unsupported array item type");
        }
    }

    if (itemsArray.empty()) {
        return nullptr;
    }

    InterfaceID arrayType = g_IID_IWantParams;
    sptr<IInterface> firstItem = itemsArray[0];

    if (IString::Query(firstItem) != nullptr) {
        arrayType = g_IID_IString;
    } else if (IInteger::Query(firstItem) != nullptr) {
        arrayType = g_IID_IInteger;
    } else if (ILong::Query(firstItem) != nullptr) {
        arrayType = g_IID_ILong;
    } else if (IBoolean::Query(firstItem) != nullptr) {
        arrayType = g_IID_IBoolean;
    } else if (IFloat::Query(firstItem) != nullptr) {
        arrayType = g_IID_IFloat;
    } else if (IDouble::Query(firstItem) != nullptr) {
        arrayType = g_IID_IDouble;
    } else if (IWantParams::Query(firstItem) != nullptr) {
        arrayType = g_IID_IWantParams;
    }

    TAG_LOGI(AAFwkTag::AA_TOOL, "array type detected, size: %{public}zu", itemsArray.size());

    sptr<IArray> arrayObj = new (std::nothrow) Array(itemsArray.size(), arrayType);
    if (arrayObj != nullptr) {
        for (size_t i = 0; i < itemsArray.size(); i++) {
            arrayObj->Set(i, itemsArray[i]);
        }
    }

    return arrayObj;
}

sptr<IAbilityManager> ExecuteIntentShellCommand::GetAbilityManagerService()
{
    auto abilityMgr = AbilityManagerClient::GetInstance();
    if (abilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "ability manager is null");
    }
    return abilityMgr;
}
}  // namespace AAFwk
}  // namespace OHOS
