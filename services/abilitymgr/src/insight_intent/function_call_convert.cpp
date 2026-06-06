#include "function_call_convert.h"

#include "cli_tool_mgr_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

namespace {
void AddInsightIntentOptions(FunctionInfo &func)
{
    nlohmann::json schema;
    if (!func.inputSchema.empty()) {
        schema = nlohmann::json::parse(func.inputSchema, nullptr, false);
        if (schema.is_discarded()) {
            schema = nlohmann::json();
        }
    }
    if (!schema.is_object()) {
        schema = nlohmann::json();
    }
    if (!schema.contains("type") || schema["type"] != "object") {
        schema["type"] = "object";
    }
    if (!schema.contains("properties")) {
        schema["properties"] = nlohmann::json();
    }

    nlohmann::json optionsParam;
    optionsParam["type"] = "object";
    optionsParam["required"] = nlohmann::json::array({"executeMode"});
    optionsParam["properties"]["executeMode"] = {{"type", "string"}};
    optionsParam["properties"]["uris"]["type"] = "array";
    optionsParam["properties"]["uris"]["items"] = {{"type", "string"}};
    optionsParam["properties"]["flags"] = {{"type", "string"}};

    schema["properties"]["ohos.insightIntent.options"] = optionsParam;
    func.inputSchema = schema.dump();
}
} // namespace

bool ConvertFromExtractProfile(const AbilityRuntime::ExtractInsightIntentProfileInfoVec &profileInfos,
    std::vector<FunctionInfo> &functions)
{
    for (const auto &info : profileInfos.insightIntents) {
        if (info.intentName.empty()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "empty intentName");
            continue;
        }
        FunctionInfo func;
        func.functionName = info.functionName.empty() ? info.intentName : info.functionName;
        func.bundleName = info.bundleName;
        func.description = info.displayDescription;
        func.inputSchema = info.parameters;
        func.outputSchema = info.result;
        func.functionType = FunctionType::INTENT_FUNCTION;
        functions.emplace_back(std::move(func));
    }
    return true;
}

bool ConvertFromExtractIntentInfo(const std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos,
    std::vector<FunctionInfo> &functions)
{
    for (const auto &info : intentInfos) {
        if (info.genericInfo.intentName.empty()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "empty intentName");
            continue;
        }
        FunctionInfo func;
        std::string functionName;
        if (info.genericInfo.currentType == AbilityRuntime::InfoType::Function) {
            auto &funcInfo = info.genericInfo.get<AbilityRuntime::InsightIntentFunctionInfo>();
            functionName = funcInfo.functionName;
            func.inputSchema = funcInfo.parameters;
        }
        func.functionName = functionName.empty() ? info.genericInfo.intentName : functionName;
        func.bundleName = info.genericInfo.bundleName;
        func.description = info.displayDescription;
        func.outputSchema = info.result;
        func.functionType = FunctionType::INTENT_FUNCTION;
        functions.emplace_back(std::move(func));
    }
    return true;
}

bool ConvertFromConfigIntent(const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos,
    std::vector<FunctionInfo> &functions)
{
    for (const auto &info : configInfos) {
        if (info.intentName.empty()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "empty intentName");
            continue;
        }

        FunctionInfo func;
        func.functionName = info.intentName;
        func.bundleName = info.bundleName;
        func.description = info.displayDescription;
        func.functionType = FunctionType::INTENT_FUNCTION;

        if (!info.inputParams.empty()) {
            nlohmann::json inputSchema;
            inputSchema["type"] = "object";
            nlohmann::json properties;
            for (const auto &param : info.inputParams) {
                properties[param] = {{"type", "string"}};
            }
            inputSchema["properties"] = properties;
            func.inputSchema = inputSchema.dump();
        }

        if (!info.outputParams.empty()) {
            nlohmann::json outputSchema;
            outputSchema["type"] = "object";
            nlohmann::json properties;
            for (const auto &param : info.outputParams) {
                properties[param] = {{"type", "string"}};
            }
            outputSchema["properties"] = properties;
            func.outputSchema = outputSchema.dump();
        }

        functions.emplace_back(std::move(func));
    }
    return true;
}

bool RegisterInsightIntentFunctions(
    const AbilityRuntime::ExtractInsightIntentProfileInfoVec &profileInfos,
    const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos)
{
    std::vector<FunctionInfo> functions;
    ConvertFromExtractProfile(profileInfos, functions);
    ConvertFromConfigIntent(configInfos, functions);

    if (functions.empty()) {
        return true;
    }

    for (auto &func : functions) {
        AddInsightIntentOptions(func);
    }

    auto &client = CliToolMGRClient::GetInstance();
    for (const auto &func : functions) {
        auto ret = client.RegisterFunction(func);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "register function failed: %{public}s/%{public}s, ret: %{public}d",
                func.bundleName.c_str(), func.functionName.c_str(), ret);
            return false;
        }
        TAG_LOGI(AAFwkTag::CLI_TOOL, "registered function: %{public}s/%{public}s",
            func.bundleName.c_str(), func.functionName.c_str());
    }
    return true;
}

} // namespace CliTool
} // namespace OHOS
