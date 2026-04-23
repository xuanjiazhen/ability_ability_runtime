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

#include "tool_util.h"

#include <atomic>
#include <chrono>
#include <fcntl.h>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <vector>

#include "accesstoken_kit.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace CliTool {

ToolUtil &ToolUtil::GetInstance()
{
    static ToolUtil instance;
    return instance;
}

bool ToolUtil::ValidateInputSchemaProperties(const std::string &inputSchema,
    const std::string &toolName, const std::string &subcommand,
    const std::map<std::string, std::string> &args)
{
    if (inputSchema.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "inputSchema is empty");
        return false;
    }

    nlohmann::json schema = nlohmann::json::parse(inputSchema);
    if (!schema.contains("properties") || !schema["properties"].is_object()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "properties not found or invalid");
        return false;
    }

    auto properties = schema["properties"];

    if (!properties.contains(toolName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "toolName not found in properties");
        return false;
    }

    if (!subcommand.empty() && !properties.contains(subcommand)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "subcommand not found in properties");
        return false;
    }

    for (auto &[key, vlue] : args) {
        if (!properties.contains(key)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "args key not found in properties");
            return false;
        }
    }
    return true;
}

bool ToolUtil::VerifySystemApp()
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Called");
    uint64_t accessTokenIdEx = IPCSkeleton::GetCallingFullTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(accessTokenIdEx)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "non-system app calling system api");
        return false;
    }
    return true;
}

bool ToolUtil::VerifyCallingPermission(const std::string &permissionName)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Verify: %{public}s", permissionName.c_str());
    OHOS::Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    int32_t ret = OHOS::Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret != OHOS::Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "PERMISSION_DENIED: %{public}s", permissionName.c_str());
        return false;
    }
    return true;
}

std::string ToolUtil::GenerateCliSessionId(const std::string &name, int64_t startTime, int32_t counter) const
{
    return name + "_" + std::to_string(startTime) + "_" + std::to_string(counter);
}

std::shared_ptr<SessionRecord> ToolUtil::CreateSessionRecord(const ExecToolParam &param, int32_t counter)
{   
    auto record = std::make_shared<SessionRecord>();
    if (record == nullptr) {
        return nullptr;
    }
    int32_t timeoutMs = param.options.timeout;
    record->startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->sessionId = GenerateCliSessionId(param.toolName, record->startTime, counter);
    record->toolName = param.toolName;
    record->timeoutMs = timeoutMs;
    record->SetState(SessionState::RUNNING);
    record->SetBackground(param.options.background);
    return record;
}

bool ToolUtil::GenerateSandboxConfig(const std::string &challenge, std::string &sandboxConfig) const
{
    AppExecFwk::BundleInfo bundleInfo;
    if (!ToolUtil::GetInstance().GetBundleInfoByTokenId(bundleInfo)) {
        return false;
    }

    nlohmann::json config;
    config["callerTokenId"] = IPCSkeleton::GetCallingFullTokenID();
    config["challenge"] = challenge;
    config["uid"] = IPCSkeleton::GetCallingUid();
    config["callerPid"] = IPCSkeleton::GetCallingPid();
    config["gid"] = bundleInfo.gid;
    config["appId"] = bundleInfo.appId;
    sandboxConfig = config.dump();
    return true;
}

bool ToolUtil::GetBundleInfoByTokenId(AppExecFwk::BundleInfo &bundleInfo) const
{
    OHOS::Security::AccessToken::AccessTokenID tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_HAP) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "caller is not hap");
        return false;
    }
    Security::AccessToken::HapTokenInfo hapInfo;
    auto ret = Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo);
    if (ret != Security::AccessToken::AccessTokenKitRet::RET_SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetHapTokenInfo failed, ret:%{public}d", ret);
        return false;
    }

    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "bundlerMgrHelper is invalid");
        return false;
    }
    auto flag = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION);
    if (bundleMgrHelper->GetCloneBundleInfo(hapInfo.bundleName, flag, hapInfo.instIndex, bundleInfo, hapInfo.userID) !=
        ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Fail to get bundle info");
        return false;
    }
    return true;
}

} // namespace CliTool
} // namespace OHOS
