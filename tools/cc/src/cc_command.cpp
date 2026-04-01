/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "cc_command.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <getopt.h>

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent/insight_intent_info_for_query.h"
#include "insight_intent/insight_intent_constant.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t DEFAULT_USER_ID = 100;
constexpr int32_t QUERY_MODE_ALL = 1;
constexpr int32_t QUERY_MODE_BY_BUNDLE = 2;
constexpr int32_t QUERY_MODE_BY_INTENT = 3;

const std::string SHORT_OPTIONS_EXECUTE_INTENT = "hb:m:a:i:e:p:";
constexpr struct option LONG_OPTIONS_EXECUTE_INTENT[] = {
    {"help", no_argument, nullptr, 'h'},
    {"bundle", required_argument, nullptr, 'b'},
    {"module", required_argument, nullptr, 'm'},
    {"ability", required_argument, nullptr, 'a'},
    {"intent", required_argument, nullptr, 'i'},
    {"execute-mode", required_argument, nullptr, 'e'},
    {"param", required_argument, nullptr, 'p'},
    {nullptr, 0, nullptr, 0},
};

const std::string SHORT_OPTIONS_GET_INTENT = "he:f:u:b:m:i:";
constexpr struct option LONG_OPTIONS_GET_INTENT[] = {
    {"help", no_argument, nullptr, 'h'},
    {"execute-mode", required_argument, nullptr, 'e'},
    {"flag", required_argument, nullptr, 'f'},
    {"user-id", required_argument, nullptr, 'u'},
    {"bundle", required_argument, nullptr, 'b'},
    {"module", required_argument, nullptr, 'm'},
    {"intent", required_argument, nullptr, 'i'},
    {nullptr, 0, nullptr, 0},
};
}  // namespace

using ErrCode = OHOS::ErrCode;

InsightIntentShellCommand::InsightIntentShellCommand(int argc, char* argv[])
    : ShellCommand(argc, argv, TOOL_NAME)
{
    for (int i = 0; i < argc_; i++) {
        TAG_LOGI(AAFwkTag::CC_TOOL, "argv_[%{public}d]: %{public}s", i, argv_[i]);
    }
}

ErrCode InsightIntentShellCommand::CreateCommandMap()
{
    commandMap_ = {
        {"help", [this]() { return this->RunAsHelpCommand(); }},
        {"execute-intent",
            [this]() { return this->RunAsExecuteIntentCommand(); }},
        {"get-intent", [this]() { return this->RunAsGetIntentCommand(); }},
    };

    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::CreateMessageMap()
{
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::init()
{
    return AbilityManagerClient::GetInstance()->Connect();
}

ErrCode InsightIntentShellCommand::RunAsHelpCommand()
{
    resultReceiver_.append(HELP_MSG);
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::RunAsExecuteIntentCommand()
{
    TAG_LOGI(AAFwkTag::CC_TOOL, "execute-intent command called");

    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
    std::string insightIntentName;
    std::string intentParamJson;
    int32_t executeMode = -1;

    auto result = ParseExecuteIntentOptions(bundleName, moduleName, abilityName,
        insightIntentName, intentParamJson, executeMode);
    if (result != OHOS::ERR_OK) {
        return result;
    }

    result = ValidateExecuteMode(executeMode);
    if (result != OHOS::ERR_OK) {
        return result;
    }

    if (abilityName.empty() || executeMode < 0) {
        result = ResolveIntentInfo(bundleName, moduleName,
            insightIntentName, abilityName, executeMode);
        if (result != OHOS::ERR_OK) {
            return result;
        }
    }

    InsightIntentExecuteParam param;
    param.bundleName_ = bundleName;
    param.moduleName_ = moduleName;
    param.abilityName_ = abilityName;
    param.insightIntentName_ = insightIntentName;
    param.executeMode_ = executeMode;
    param.userId_ = DEFAULT_USER_ID;
    if (!intentParamJson.empty()) {
        param.insightIntentParam_ =
            std::make_shared<WantParams>(
                CcParamParser::BuildWantParamsFromJson(intentParamJson));
    } else {
        param.insightIntentParam_ = std::make_shared<WantParams>();
    }

    return ExecuteIntentWithParam(param);
}

ErrCode InsightIntentShellCommand::ParseExecuteIntentOptions(
    std::string &bundleName, std::string &moduleName,
    std::string &abilityName, std::string &insightIntentName,
    std::string &intentParamJson, int32_t &executeMode)
{
    int option = -1;
    int counter = 0;

    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_EXECUTE_INTENT.c_str(),
            LONG_OPTIONS_EXECUTE_INTENT, nullptr);

        TAG_LOGI(AAFwkTag::CC_TOOL,
            "option: %{public}d, optopt: %{public}d, optind: %{public}d",
            option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            AppendErrorEvent("missing required parameters.");
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1 && optind < argc_ &&
                strcmp(argv_[optind], cmd_.c_str()) == 0) {
                AppendErrorEvent("you must specify an option at least.");
            }
            break;
        }

        if (option == '?') {
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
            TAG_LOGI(AAFwkTag::CC_TOOL, "'ohos-claw-cc execute-intent' option unknown");
            AppendErrorEvent("unknown option.");
            return OHOS::ERR_INVALID_VALUE;
        }

        if (HandleExecuteIntentOption(option, bundleName, moduleName, abilityName,
            insightIntentName, intentParamJson, executeMode) != OHOS::ERR_OK) {
            return OHOS::ERR_INVALID_VALUE;
        }
    }

    return CheckRequiredExecuteParams(bundleName, moduleName, insightIntentName);
}

ErrCode InsightIntentShellCommand::CheckRequiredExecuteParams(
    const std::string &bundleName, const std::string &moduleName,
    const std::string &insightIntentName)
{
    if (bundleName.empty() || moduleName.empty() ||
        insightIntentName.empty()) {
        TAG_LOGE(AAFwkTag::CC_TOOL, "missing required parameters");
        AppendErrorEvent("missing required parameters. Required: -b, -m, -i");
        return OHOS::ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::HandleExecuteIntentOption(
    int option, std::string &bundleName, std::string &moduleName,
    std::string &abilityName, std::string &insightIntentName,
    std::string &intentParamJson, int32_t &executeMode)
{
    switch (option) {
        case 'h':
            resultReceiver_.append(HELP_MSG_EXECUTE_INTENT + "\n");
            return OHOS::ERR_INVALID_VALUE;
        case 'b':
            bundleName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL, "bundleName: %{public}s", bundleName.c_str());
            break;
        case 'm':
            moduleName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL, "moduleName: %{public}s", moduleName.c_str());
            break;
        case 'a':
            abilityName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL, "abilityName: %{public}s", abilityName.c_str());
            break;
        case 'i':
            insightIntentName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL,
                "insightIntentName: %{public}s", insightIntentName.c_str());
            break;
        case 'e':
            if (ParseIntOption(optarg, executeMode) != OHOS::ERR_OK) {
                AppendErrorEvent("invalid execute mode value.");
                return OHOS::ERR_INVALID_VALUE;
            }
            break;
        case 'p':
            intentParamJson = optarg;
            if (intentParamJson.size() >= 2 &&
                intentParamJson.front() == '\'' && intentParamJson.back() == '\'') {
                intentParamJson = intentParamJson.substr(1, intentParamJson.size() - 2);
            }
            TAG_LOGI(AAFwkTag::CC_TOOL,
                "intentParamJson: %{public}s", intentParamJson.c_str());
            break;
        default:
            break;
    }
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::ResolveIntentInfo(
    const std::string &bundleName, const std::string &moduleName,
    const std::string &insightIntentName, std::string &abilityName,
    int32_t &executeMode)
{
    AbilityRuntime::InsightIntentInfoForQuery queryInfo;
    auto queryErr = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByIntentName(
        AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT,
        bundleName, moduleName, insightIntentName, queryInfo);
    if (queryErr != OHOS::ERR_OK || queryInfo.intentName.empty()) {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "intentName not found: %{public}s, err: %{public}d",
            insightIntentName.c_str(), queryErr);
        AppendErrorEvent("invalid intentName, not found in database.");
        return OHOS::ERR_INVALID_VALUE;
    }

    ErrCode result = ResolveAbilityNameFromQuery(queryInfo, abilityName);
    if (result != OHOS::ERR_OK) {
        return result;
    }

    return ResolveExecuteModeFromQuery(queryInfo, executeMode);
}

ErrCode InsightIntentShellCommand::ResolveAbilityNameFromQuery(
    const AbilityRuntime::InsightIntentInfoForQuery &queryInfo,
    std::string &abilityName)
{
    if (!abilityName.empty()) {
        return OHOS::ERR_OK;
    }

    abilityName = queryInfo.uiAbilityIntentInfo.abilityName;
    if (abilityName.empty()) {
        abilityName = queryInfo.pageInfo.uiAbility;
    }
    if (abilityName.empty()) {
        abilityName = queryInfo.entryInfo.abilityName;
    }
    if (abilityName.empty()) {
        abilityName = queryInfo.formInfo.abilityName;
    }
    if (abilityName.empty()) {
        TAG_LOGE(AAFwkTag::CC_TOOL, "abilityName not found in intent info");
        AppendErrorEvent(
            "abilityName not found in intent database. "
            "Please provide -a <ability-name> explicitly.");
        return OHOS::ERR_INVALID_VALUE;
    }

    TAG_LOGI(AAFwkTag::CC_TOOL,
        "auto resolved abilityName: %{public}s", abilityName.c_str());
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::ResolveExecuteModeFromQuery(
    const AbilityRuntime::InsightIntentInfoForQuery &queryInfo,
    int32_t &executeMode)
{
    std::vector<int32_t> modes = CollectSupportedModes(queryInfo);
    if (modes.empty()) {
        TAG_LOGE(AAFwkTag::CC_TOOL, "no execute mode found in intent info");
        AppendErrorEvent(
            "no execute mode found in intent database. "
            "Please provide -e <execute-mode> explicitly.");
        return OHOS::ERR_INVALID_VALUE;
    }

    if (executeMode >= 0) {
        return OHOS::ERR_OK;
    }

    return AutoResolveExecuteMode(modes, executeMode);
}

ErrCode InsightIntentShellCommand::AutoResolveExecuteMode(
    const std::vector<int32_t> &modes, int32_t &executeMode)
{
    if (modes.size() == 1) {
        executeMode = modes[0];
        TAG_LOGI(AAFwkTag::CC_TOOL,
            "auto resolved executeMode: %{public}d", executeMode);
        return OHOS::ERR_OK;
    }

    TAG_LOGE(AAFwkTag::CC_TOOL,
        "multiple execute modes found, cannot auto-resolve");
    nlohmann::json modeArray = nlohmann::json::array();
    for (auto m : modes) {
        modeArray.emplace_back(m);
    }
    AppendErrorEvent(
        "intent supports multiple execute modes. "
        "Please provide -e <execute-mode> explicitly. Supported modes: " +
        modeArray.dump());
    return OHOS::ERR_INVALID_VALUE;
}

std::vector<int32_t> InsightIntentShellCommand::CollectSupportedModes(
    const AbilityRuntime::InsightIntentInfoForQuery &queryInfo)
{
    std::vector<int32_t> modes;
    for (auto mode : queryInfo.entryInfo.executeMode) {
        modes.push_back(static_cast<int32_t>(mode));
    }
    for (auto mode : queryInfo.uiAbilityIntentInfo.supportExecuteMode) {
        modes.push_back(static_cast<int32_t>(mode));
    }
    return modes;
}

ErrCode InsightIntentShellCommand::ExecuteIntentWithParam(
    const InsightIntentExecuteParam &param)
{
    TAG_LOGI(AAFwkTag::CC_TOOL,
        "Executing insight intent: bundle=%{public}s, module=%{public}s, "
        "ability=%{public}s, intent=%{public}s, mode=%{public}d",
        param.bundleName_.c_str(), param.moduleName_.c_str(),
        param.abilityName_.c_str(), param.insightIntentName_.c_str(),
        param.executeMode_);

    InsightIntentExecuteResult executeResult;
    auto err = AbilityManagerClient::GetInstance()->ExecuteIntentWithResult(
        const_cast<InsightIntentExecuteParam &>(param), executeResult, 30000);

    if (err == OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::CC_TOOL,
            "execute intent successfully, innerErr=%{public}d, code=%{public}d",
            executeResult.innerErr, executeResult.code);
        AppendExecuteResult(executeResult);
    } else {
        TAG_LOGE(AAFwkTag::CC_TOOL, "execute intent failed: %{public}d", err);
        AppendErrorEvent("failed to execute intent. " + GetMessageFromCode(err));
    }

    return err;
}

void InsightIntentShellCommand::AppendExecuteResult(
    const InsightIntentExecuteResult &executeResult)
{
    nlohmann::json data;
    data["innerErr"] = executeResult.innerErr;
    data["code"] = executeResult.code;
    data["flags"] = executeResult.flags;

    if (executeResult.result != nullptr) {
        data["result"] = executeResult.result->ToString();
    }
    if (!executeResult.uris.empty()) {
        data["uris"] = executeResult.uris;
    }

    AppendResultEvent(data);
}

ErrCode InsightIntentShellCommand::RunAsGetIntentCommand()
{
    TAG_LOGI(AAFwkTag::CC_TOOL, "get-intent command called");

    int32_t flag = -1;
    int32_t executeMode = -1;
    int32_t userId = DEFAULT_INVAL_VALUE;
    std::string bundleName;
    std::string moduleName;
    std::string intentName;

    auto result = ParseGetIntentOptions(flag, executeMode, userId,
        bundleName, moduleName, intentName);
    if (result != OHOS::ERR_OK) {
        return result;
    }

    return DispatchGetIntentMode(executeMode, flag, userId,
        bundleName, moduleName, intentName);
}

ErrCode InsightIntentShellCommand::ParseGetIntentOptions(
    int32_t &flag, int32_t &executeMode, int32_t &userId,
    std::string &bundleName, std::string &moduleName,
    std::string &intentName)
{
    int option = -1;
    int counter = 0;

    while (true) {
        counter++;
        option = getopt_long(argc_, argv_, SHORT_OPTIONS_GET_INTENT.c_str(),
            LONG_OPTIONS_GET_INTENT, nullptr);

        TAG_LOGI(AAFwkTag::CC_TOOL,
            "option: %{public}d, optopt: %{public}d, optind: %{public}d",
            option, optopt, optind);

        if (optind < 0 || optind > argc_) {
            AppendErrorEvent("missing required parameters.");
            return OHOS::ERR_INVALID_VALUE;
        }

        if (option == -1) {
            if (counter == 1 && optind < argc_ &&
                strcmp(argv_[optind], cmd_.c_str()) == 0) {
                AppendErrorEvent("you must specify an option at least.");
            }
            break;
        }

        if (option == '?') {
            std::string unknownOption = "";
            std::string unknownOptionMsg = GetUnknownOptionMsg(unknownOption);
            TAG_LOGI(AAFwkTag::CC_TOOL, "'ohos-claw-cc get-intent' option unknown");
            AppendErrorEvent("unknown option.");
            return OHOS::ERR_INVALID_VALUE;
        }

        if (HandleGetIntentOption(option, flag, executeMode, userId,
            bundleName, moduleName, intentName) != OHOS::ERR_OK) {
            return OHOS::ERR_INVALID_VALUE;
        }
    }

    return ValidateGetIntentParams(flag, executeMode, userId);
}

ErrCode InsightIntentShellCommand::ValidateGetIntentParams(
    int32_t flag, int32_t executeMode, int32_t &userId)
{
    if (executeMode < 0 || flag < 0) {
        TAG_LOGE(AAFwkTag::CC_TOOL, "missing required parameters -e or -f");
        AppendErrorEvent("missing required parameters -e and -f.");
        return OHOS::ERR_INVALID_VALUE;
    }

    if (userId == DEFAULT_INVAL_VALUE) {
        userId = DEFAULT_USER_ID;
    }
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::HandleGetIntentOption(
    int option, int32_t &flag, int32_t &executeMode, int32_t &userId,
    std::string &bundleName, std::string &moduleName,
    std::string &intentName)
{
    switch (option) {
        case 'h':
            resultReceiver_.append(HELP_MSG_GET_INTENT + "\n");
            return OHOS::ERR_INVALID_VALUE;
        case 'e':
            if (ParseIntOption(optarg, executeMode) != OHOS::ERR_OK) {
                AppendErrorEvent("invalid execute mode value.");
                return OHOS::ERR_INVALID_VALUE;
            }
            TAG_LOGI(AAFwkTag::CC_TOOL, "executeMode: %{public}d", executeMode);
            break;
        case 'f':
            if (ParseIntOption(optarg, flag) != OHOS::ERR_OK) {
                AppendErrorEvent("invalid flag value.");
                return OHOS::ERR_INVALID_VALUE;
            }
            TAG_LOGI(AAFwkTag::CC_TOOL, "flag: %{public}d", flag);
            break;
        case 'u':
            if (ParseIntOption(optarg, userId) != OHOS::ERR_OK) {
                AppendErrorEvent("invalid user id value.");
                return OHOS::ERR_INVALID_VALUE;
            }
            TAG_LOGI(AAFwkTag::CC_TOOL, "userId: %{public}d", userId);
            break;
        case 'b':
            bundleName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL,
                "bundleName: %{public}s", bundleName.c_str());
            break;
        case 'm':
            moduleName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL,
                "moduleName: %{public}s", moduleName.c_str());
            break;
        case 'i':
            intentName = optarg;
            TAG_LOGI(AAFwkTag::CC_TOOL,
                "intentName: %{public}s", intentName.c_str());
            break;
        default:
            break;
    }
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::DispatchGetIntentMode(
    int32_t executeMode, int32_t flag, int32_t userId,
    const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName)
{
    if (executeMode == QUERY_MODE_ALL) {
        return RunGetIntentModeAll(flag, userId);
    } else if (executeMode == QUERY_MODE_BY_BUNDLE) {
        if (bundleName.empty()) {
            TAG_LOGE(AAFwkTag::CC_TOOL, "mode 2 requires -b bundleName");
            AppendErrorEvent("mode 2 requires -b <bundle-name>.");
            return OHOS::ERR_INVALID_VALUE;
        }
        return RunGetIntentModeByBundle(flag, userId, bundleName);
    } else if (executeMode == QUERY_MODE_BY_INTENT) {
        if (bundleName.empty() || moduleName.empty() || intentName.empty()) {
            TAG_LOGE(AAFwkTag::CC_TOOL, "mode 3 requires -b, -m, -i");
            AppendErrorEvent(
                "mode 3 requires -b <bundle-name> "
                "-m <module-name> -i <intent-name>.");
            return OHOS::ERR_INVALID_VALUE;
        }
        return RunGetIntentModeByIntent(flag, userId,
            bundleName, moduleName, intentName);
    }

    TAG_LOGE(AAFwkTag::CC_TOOL,
        "invalid execute mode: %{public}d", executeMode);
    AppendErrorEvent("invalid execute mode. Must be 1, 2, or 3.");
    return OHOS::ERR_INVALID_VALUE;
}

ErrCode InsightIntentShellCommand::ParseIntOption(
    const char *arg, int32_t &value)
{
    if (arg == nullptr || *arg == '\0') {
        TAG_LOGE(AAFwkTag::CC_TOOL, "invalid integer value: empty");
        return OHOS::ERR_INVALID_VALUE;
    }

    char *end = nullptr;
    errno = 0;
    long result = strtol(arg, &end, 10);
    if (errno != 0 || end == arg || *end != '\0' ||
        result < INT32_MIN || result > INT32_MAX) {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "invalid integer value: %{public}s", arg);
        return OHOS::ERR_INVALID_VALUE;
    }
    value = static_cast<int32_t>(result);
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::ValidateExecuteMode(int32_t executeMode)
{
    if (executeMode < 0) {
        return OHOS::ERR_OK;
    }

    constexpr int32_t MAX_EXECUTE_MODE =
        static_cast<int32_t>(AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY);
    if (executeMode > MAX_EXECUTE_MODE) {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "invalid execute mode: %{public}d", executeMode);
        AppendErrorEvent("invalid execute mode, must be 0-3.");
        return OHOS::ERR_INVALID_VALUE;
    }
    return OHOS::ERR_OK;
}

ErrCode InsightIntentShellCommand::RunGetIntentModeAll(
    int32_t flag, int32_t userId)
{
    std::vector<AbilityRuntime::InsightIntentInfoForQuery> infos;
    TAG_LOGI(AAFwkTag::CC_TOOL,
        "GetAllInsightIntentInfo with flag: %{public}d, userId: %{public}d",
        flag, userId);

    auto err = AbilityManagerClient::GetInstance()->GetAllInsightIntentInfo(
        static_cast<AbilityRuntime::GetInsightIntentFlag>(flag), infos, userId);

    if (err == OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::CC_TOOL,
            "get all insight intent info successfully, count: %{public}zu",
            infos.size());
        nlohmann::json data;
        data["mode"] = "all";
        data["total"] = infos.size();
        data["intents"] = BuildIntentInfoListJson(infos, flag);
        AppendResultEvent(data);
    } else {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "get all insight intent info failed: %{public}d", err);
        AppendErrorEvent("failed to get insight intent info. " +
            GetMessageFromCode(err));
    }

    return err;
}

ErrCode InsightIntentShellCommand::RunGetIntentModeByBundle(
    int32_t flag, int32_t userId, const std::string &bundleName)
{
    std::vector<AbilityRuntime::InsightIntentInfoForQuery> infos;
    TAG_LOGI(AAFwkTag::CC_TOOL,
        "GetInsightIntentInfoByBundleName with flag: %{public}d, "
        "userId: %{public}d, bundle: %{public}s",
        flag, userId, bundleName.c_str());

    auto err = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByBundleName(
        static_cast<AbilityRuntime::GetInsightIntentFlag>(flag),
        bundleName, infos, userId);

    if (err == OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::CC_TOOL,
            "get insight intent info by bundle successfully, count: %{public}zu",
            infos.size());
        nlohmann::json data;
        data["mode"] = "by bundle";
        data["bundleName"] = bundleName;
        data["total"] = infos.size();
        data["intents"] = BuildIntentInfoListJson(infos, flag);
        AppendResultEvent(data);
    } else {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "get insight intent info by bundle failed: %{public}d", err);
        AppendErrorEvent("failed to get insight intent info by bundle. " +
            GetMessageFromCode(err));
    }

    return err;
}

ErrCode InsightIntentShellCommand::RunGetIntentModeByIntent(
    int32_t flag, int32_t userId,
    const std::string &bundleName, const std::string &moduleName,
    const std::string &intentName)
{
    AbilityRuntime::InsightIntentInfoForQuery info;
    TAG_LOGI(AAFwkTag::CC_TOOL,
        "GetInsightIntentInfoByIntentName with flag: %{public}d, "
        "userId: %{public}d, bundle: %{public}s, "
        "module: %{public}s, intent: %{public}s",
        flag, userId, bundleName.c_str(), moduleName.c_str(),
        intentName.c_str());

    auto err = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByIntentName(
        static_cast<AbilityRuntime::GetInsightIntentFlag>(flag),
        bundleName, moduleName, intentName, info, userId);

    if (err == OHOS::ERR_OK && info.intentName.empty()) {
        TAG_LOGE(AAFwkTag::CC_TOOL, "intent not found: %{public}s",
            intentName.c_str());
        AppendErrorEvent(
            "intent not found. "
            "Please check bundleName, moduleName and intentName.");
        return OHOS::ERR_INVALID_VALUE;
    }

    if (err == OHOS::ERR_OK) {
        TAG_LOGI(AAFwkTag::CC_TOOL,
            "get insight intent info by intent name successfully");
        nlohmann::json data;
        data["mode"] = "by intent";
        data["intent"] = BuildIntentInfoJson(info, flag);
        AppendResultEvent(data);
    } else {
        TAG_LOGE(AAFwkTag::CC_TOOL,
            "get insight intent info by intent name failed: %{public}d", err);
        AppendErrorEvent("failed to get insight intent info by intent name. " +
            GetMessageFromCode(err));
    }

    return err;
}

void InsightIntentShellCommand::AppendResultEvent(const nlohmann::json &data)
{
    nlohmann::json event;
    event["event"] = "result";
    event["data"] = data;
    resultReceiver_.append(event.dump());
}

void InsightIntentShellCommand::AppendErrorEvent(const std::string &message)
{
    nlohmann::json event;
    event["event"] = "error";
    event["data"]["message"] = message;
    resultReceiver_.append(event.dump());
    fprintf(stderr, "%s\n", event.dump().c_str());
}

bool InsightIntentShellCommand::ShouldAppendEntityInfo(int32_t flag) const
{
    return (flag & AbilityRuntime::GetInsightIntentFlag::GET_ENTITY_INFO) != 0;
}

bool InsightIntentShellCommand::IsFullInfo(int32_t flag) const
{
    return (flag & AbilityRuntime::GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT) != 0;
}

nlohmann::json InsightIntentShellCommand::BuildIntentInfoListJson(
    const std::vector<AbilityRuntime::InsightIntentInfoForQuery> &infos,
    int32_t flag)
{
    nlohmann::json arr = nlohmann::json::array();
    for (const auto &info : infos) {
        arr.emplace_back(BuildIntentInfoJson(info, flag));
    }
    return arr;
}

nlohmann::json InsightIntentShellCommand::BuildIntentInfoJson(
    const AbilityRuntime::InsightIntentInfoForQuery &info, int32_t flag)
{
    nlohmann::json j;
    j["bundleName"] = info.bundleName;
    j["moduleName"] = info.moduleName;
    j["intentName"] = info.intentName;
    j["displayName"] = info.displayName;
    j["intentType"] = info.intentType;
    j["developType"] = info.develoType;
    j["parameters"] = info.parameters;

    if (IsFullInfo(flag)) {
        j["domain"] = info.domain;
        j["intentVersion"] = info.intentVersion;
        j["displayDescription"] = info.displayDescription;
        j["schema"] = info.schema;
        j["icon"] = info.icon;
        j["llmDescription"] = info.llmDescription;
        if (!info.keywords.empty()) {
            j["keywords"] = info.keywords;
        }
    }

    if (!info.linkInfo.uri.empty()) {
        j["linkInfo"]["uri"] = info.linkInfo.uri;
    }

    const auto &page = info.pageInfo;
    if (!page.uiAbility.empty() || !page.pagePath.empty()) {
        j["pageInfo"]["uiAbility"] = page.uiAbility;
        j["pageInfo"]["pagePath"] = page.pagePath;
        j["pageInfo"]["navigationId"] = page.navigationId;
        j["pageInfo"]["navDestinationName"] = page.navDestinationName;
    }

    const auto &entry = info.entryInfo;
    if (!entry.abilityName.empty()) {
        j["entryInfo"]["abilityName"] = entry.abilityName;
        j["entryInfo"]["executeMode"] = CollectSupportedModesJson(info);
    }

    const auto &form = info.formInfo;
    if (!form.abilityName.empty()) {
        j["formInfo"]["abilityName"] = form.abilityName;
        j["formInfo"]["formName"] = form.formName;
    }

    if (ShouldAppendEntityInfo(flag) && !info.entities.empty()) {
        nlohmann::json entities = nlohmann::json::array();
        for (const auto &entity : info.entities) {
            entities.emplace_back(BuildEntityJson(entity));
        }
        j["entities"] = entities;
    }

    return j;
}

nlohmann::json InsightIntentShellCommand::BuildEntityJson(
    const AbilityRuntime::EntityInfoForQuery &entity)
{
    nlohmann::json j;
    j["className"] = entity.className;
    j["entityId"] = entity.entityId;
    j["entityCategory"] = entity.entityCategory;
    j["parameters"] = entity.parameters;
    j["parentClassName"] = entity.parentClassName;
    return j;
}

nlohmann::json InsightIntentShellCommand::CollectSupportedModesJson(
    const AbilityRuntime::InsightIntentInfoForQuery &queryInfo)
{
    nlohmann::json arr = nlohmann::json::array();
    for (auto mode : queryInfo.entryInfo.executeMode) {
        arr.emplace_back(static_cast<int32_t>(mode));
    }
    for (auto mode : queryInfo.uiAbilityIntentInfo.supportExecuteMode) {
        arr.emplace_back(static_cast<int32_t>(mode));
    }
    return arr;
}
}  // namespace AAFwk
}  // namespace OHOS
