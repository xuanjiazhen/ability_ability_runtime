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

#include "ccm_util.h"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <limits.h>
#include <unistd.h>

#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"
#include "parameters.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr const char* MAX_CLI_QUANTITY = "const.sys.cli.limit_maximum_concurrency_quantity";
constexpr const char* PUBLIC_CLI_TOOLS_CONFIG_PATH = "/etc/ability_runtime/public_cli_tools.json";
constexpr const char* KEY_PUBLIC_CLI_TOOLS = "publicCliTools";

bool LoadJsonFile(const std::string &filePath, nlohmann::json &jsonBuf)
{
    if (access(filePath.c_str(), F_OK) != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "public cli tool config not exist");
        return false;
    }

    char path[PATH_MAX] = {0};
    if (realpath(filePath.c_str(), path) == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "public cli tool config realpath failed, errno: %{public}d", errno);
        return false;
    }

    std::fstream in;
    in.open(path, std::ios_base::in);
    if (!in.is_open()) {
        char errBuf[256] = {'\0'};
        strerror_r(errno, errBuf, sizeof(errBuf));
        TAG_LOGW(AAFwkTag::CLI_TOOL, "public cli tool config open failed: %{public}s", errBuf);
        return false;
    }

    jsonBuf = nlohmann::json::parse(in, nullptr, false);
    in.close();
    if (jsonBuf.is_discarded()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "public cli tool config parse failed");
        return false;
    }
    return true;
}
}

CcmUtil &CcmUtil::GetInstance()
{
    static CcmUtil util;
    return util;
}

int32_t CcmUtil::GetCliConcurrencyLimit()
{
    if (!maxCliQuantity_.isLoaded) {
        maxCliQuantity_.value =
            system::GetIntParameter<int32_t>(MAX_CLI_QUANTITY, DEFAULT_MAX_CLI_QUANTITY);
        maxCliQuantity_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::CLI_TOOL, "MaxCLiQuantity: %{public}d", maxCliQuantity_.value);
    return maxCliQuantity_.value;
}

std::set<std::string> CcmUtil::GetPublicCliToolNames()
{
    std::lock_guard<std::mutex> lock(publicCliToolNamesMutex_);
    if (publicCliToolNames_.isLoaded) {
        return publicCliToolNames_.value;
    }

    publicCliToolNames_.value.clear();
    nlohmann::json object;
    if (!LoadJsonFile(PUBLIC_CLI_TOOLS_CONFIG_PATH, object)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "load public cli tool config failed");
        publicCliToolNames_.isLoaded = true;
        return publicCliToolNames_.value;
    }
    ParsePublicCliToolNames(object, publicCliToolNames_.value);
    publicCliToolNames_.isLoaded = true;
    return publicCliToolNames_.value;
}

bool CcmUtil::ParsePublicCliToolNames(const nlohmann::json &object, std::set<std::string> &publicNames) const
{
    publicNames.clear();
    if (!object.contains(KEY_PUBLIC_CLI_TOOLS) || !object.at(KEY_PUBLIC_CLI_TOOLS).is_array()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "public cli tool config key invalid");
        return false;
    }

    for (const auto &item : object.at(KEY_PUBLIC_CLI_TOOLS)) {
        if (!item.is_string()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "public cli tool name invalid");
            continue;
        }
        std::string name = item.get<std::string>();
        if (name.empty()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "public cli tool name empty");
            continue;
        }
        publicNames.insert(std::move(name));
    }
    return true;
}
} // namespace CliTool
} // namespace OHOS
