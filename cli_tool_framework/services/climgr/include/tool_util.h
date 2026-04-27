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

#ifndef OHOS_ABILITY_RUNTIME_TOOL_UTIL_H
#define OHOS_ABILITY_RUNTIME_TOOL_UTIL_H

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include <access_token.h>
#include <iremote_object.h>
#include <functional>

namespace OHOS {
namespace AAFwk {
class WantParams;
struct IInterface;
struct IArray;
}
namespace AppExecFwk {
struct BundleInfo;
}
namespace CliTool {
class ExecToolParam;
class SessionRecord;
class ToolInfo;

using namespace OHOS::Security;

class ToolUtil {
public:
    static int32_t ValidateProperties(const ToolInfo &toolInfo, ExecToolParam &param,
        AccessToken::AccessTokenID tokenId);

    static std::string GenerateCliSessionId(const std::string &name, std::shared_ptr<SessionRecord> record);

    static bool GenerateSandboxConfig(const std::string &challenge, AccessToken::AccessTokenID tokenId,
        std::string &sandboxConfig);

    static void TransferToCmdParam(const ToolInfo &toolInfo, const AAFwk::WantParams &args, std::string &cmdLine);

private:
    static bool GetBundleInfoByTokenId(AccessToken::AccessTokenID tokenId,
        AppExecFwk::BundleInfo &bundleInfo);
    static int32_t ValidateInputSchemaProperties(const std::string &inputSchema, const AAFwk::WantParams &args);

    // Helper methods for argument mapping
    static void ApplyFlagMapping(const std::string &templates, const AAFwk::WantParams &args, std::string &cmdLine);
    static void ApplyPositionalMapping(const std::string &order, const AAFwk::WantParams &args, std::string &cmdLine);
    static void ApplyFlattenedMapping(const std::string &separator, const std::string &templates,
        const AAFwk::WantParams &args, std::string &cmdLine);
    static void ApplyJsonStringMapping(const std::string &templates, const AAFwk::WantParams &args, std::string &cmdLine);
    static void ApplyMixedMapping(const std::string &templates, const AAFwk::WantParams &args, std::string &cmdLine);
    static std::string FormatTemplate(const std::string &tmpl, const std::string &value);

    // Nested path query helper for flattened mapping
    static sptr<AAFwk::IInterface> GetParamByNestedPath(const AAFwk::WantParams &args,
        const std::string &path, const std::string &separator);

    // WantParams to JSON conversion helper for nested objects
    static std::string WantParamsToJson(const AAFwk::WantParams &wantParams);

    static void ApplyJsonStringModeToSingleParam(const std::string &key, const sptr<AAFwk::IInterface> &value,
        const nlohmann::json &templateValue, std::string &cmdLine);
    static void ApplyFlattenedModeToSingleParam(const std::string &key, const sptr<AAFwk::IInterface> &value,
        const std::string &separator, const nlohmann::json &templateValue, const AAFwk::WantParams &args,
        std::string &cmdLine);

    // Core parameter processing logic (extracted for reuse)
    static void ApplyFlagModeLogic(const sptr<AAFwk::IInterface> &value,
        const nlohmann::json &templateValue, std::string &cmdLine);
    static void ApplyJsonStringModeLogic(const std::string &key, const sptr<AAFwk::IInterface> &value,
        const std::string &tmpl, std::string &cmdLine);
    static void ApplyFlattenedModeLogic(const std::string &key, const sptr<AAFwk::IInterface> &value,
        const std::string &separator, const std::string &tmpl, const AAFwk::WantParams &args,
        std::string &cmdLine);

    // Type conversion helpers
    // GetParamStringValue: only supports basic types (bool, int, long, float, double, string)
    // GetParamArrayValue: supports single-level arrays with basic type elements
    // GetParamJsonValue: converts to JSON format (supports single-level arrays)
    // Note: Nested arrays and byte/char/short types are not supported
    static std::string GetParamStringValue(const sptr<AAFwk::IInterface> &value);
    static std::string GetParamJsonValue(const sptr<AAFwk::IInterface> &value);
    static bool GetParamBoolValue(const sptr<AAFwk::IInterface> &value, bool &result);
    static bool GetParamArrayValue(const sptr<AAFwk::IInterface> &value, std::vector<std::string> &result);

    // Low-level helper methods for code reuse
    static bool ExtractWantParams(const sptr<AAFwk::IInterface> &value, AAFwk::WantParams &wantParams);
    static std::string EscapeJsonString(const std::string &str);
    static void IterateIArray(sptr<AAFwk::IArray> arrayObj,
        std::function<void(const sptr<AAFwk::IInterface>&)> elementHandler);
    static std::string BuildJsonArrayFromIArray(sptr<AAFwk::IArray> arrayObj,
        std::function<std::string(const sptr<AAFwk::IInterface>&)> elementConverter);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_TOOL_UTIL_H
