#ifndef OHOS_ABILITY_RUNTIME_FUNCTION_CALL_CONVERT_H
#define OHOS_ABILITY_RUNTIME_FUNCTION_CALL_CONVERT_H

#include <vector>

#include "function_info.h"
#include "extract_insight_intent_profile.h"
#include "insight_intent_profile.h"

namespace OHOS {
namespace CliTool {

bool ConvertFromExtractProfile(const AbilityRuntime::ExtractInsightIntentProfileInfoVec &profileInfos,
    std::vector<FunctionInfo> &functions);

bool ConvertFromExtractIntentInfo(const std::vector<AbilityRuntime::ExtractInsightIntentInfo> &intentInfos,
    std::vector<FunctionInfo> &functions);

bool ConvertFromConfigIntent(const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos,
    std::vector<FunctionInfo> &functions);

bool RegisterInsightIntentFunctions(
    const AbilityRuntime::ExtractInsightIntentProfileInfoVec &profileInfos,
    const std::vector<AbilityRuntime::InsightIntentInfo> &configInfos,
    const std::string &bundleName = "");

bool UnregisterInsightIntentFunctions(const std::string &bundleName);

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_FUNCTION_CALL_CONVERT_H
