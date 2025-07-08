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

#include <cstring>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <gtest/gtest.h>

#include "config_policy_utils.h"
#include "mock_config_policy_utils.h"

namespace OHOS {
namespace AbilityRuntime {
MockConfigPolicyUtils& MockConfigPolicyUtils::GetInstance()
{
    static MockConfigPolicyUtils instance;
    return instance;
}

MockConfigPolicyUtils::~MockConfigPolicyUtils()
{
    std::error_code ec;
    if (std::filesystem::exists(oneCfgFilePath_, ec) &&
        std::filesystem::is_regular_file(oneCfgFilePath_, ec) &&
        std::filesystem::remove(oneCfgFilePath_, ec)) {
        GTEST_LOG_(INFO) << "SetOneCfgFilePath remove " << oneCfgFilePath_;
        oneCfgFilePath_.clear();
    }
}

const char* MockConfigPolicyUtils::GetOneCfgFilePath()
{
    return oneCfgFilePathIsNull_ ? nullptr : oneCfgFilePath_.c_str();
}

void MockConfigPolicyUtils::SetOneCfgFilePathIsNull()
{
    std::error_code ec;
    if (std::filesystem::exists(oneCfgFilePath_, ec) &&
        std::filesystem::is_regular_file(oneCfgFilePath_, ec) &&
        std::filesystem::remove(oneCfgFilePath_, ec)) {
        GTEST_LOG_(INFO) << "SetOneCfgFilePath remove " << oneCfgFilePath_;
    }
    oneCfgFilePath_.clear();
    oneCfgFilePathIsNull_ = true;
}

void MockConfigPolicyUtils::SetOneCfgFilePath(std::string fileName, bool isCreate, bool isRoot)
{
    std::error_code ec;
    if (std::filesystem::exists(oneCfgFilePath_, ec) &&
        std::filesystem::is_regular_file(oneCfgFilePath_, ec) &&
        std::filesystem::remove(oneCfgFilePath_, ec)) {
        GTEST_LOG_(INFO) << "SetOneCfgFilePath remove " << oneCfgFilePath_;
    }

    oneCfgFilePathIsNull_ = false;
    if (fileName.empty()) {
        oneCfgFilePath_ = fileName;
        GTEST_LOG_(INFO) << "SetOneCfgFilePath fileName is empty";
        return;
    }

    oneCfgFilePath_.clear();
    if (isRoot) {
        oneCfgFilePath_.append("/").append(fileName);
    } else {
        std::filesystem::path currentDir = std::filesystem::current_path(ec);
        oneCfgFilePath_.append(currentDir).append("/").append(fileName);
    }

    if (isCreate) {
        std::ofstream file(oneCfgFilePath_);
        GTEST_LOG_(INFO) << "SetOneCfgFilePath create file [" << oneCfgFilePath_ << "] is " <<
            (file.is_open() ? "success" : "failure");
    }
}

bool MockConfigPolicyUtils::GetRealPathStatus()
{
    return realPathIsNull_;
}

void MockConfigPolicyUtils::SetRealPathIsNull(bool isNull)
{
    realPathIsNull_ = isNull;
}
}
}

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif // __cplusplus
char *GetOneCfgFile(const char *pathSuffix, char *buf, unsigned int bufLength)
{
    if (buf == nullptr || bufLength == 0) {
        GTEST_LOG_(ERROR) << "GetOneCfgFile failed: buffer is nullptr or length is 0";
        return nullptr;
    }

    const char* path = OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().GetOneCfgFilePath();
    GTEST_LOG_(INFO) << "GetOneCfgFile called. Raw path: " <<
        (path != nullptr ? path : "nullptr");

    if (path == nullptr) {
        GTEST_LOG_(ERROR) << "GetOneCfgFile failed: path is nullptr";
        return nullptr;
    }

    if (snprintf(buf, bufLength - 1, "%s", path) < 0) {
        GTEST_LOG_(ERROR) << "GetOneCfgFile failed: snprintf internal error";
        return nullptr;
    }

    buf[bufLength - 1] = '\0';
    return buf;
}

char *realpath(const char *path, char *resolved_path)
{
    if (resolved_path == nullptr) {
        GTEST_LOG_(ERROR) << "realpath failed: resolved_path is nullptr";
        return nullptr;
    }

    auto flag = OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().GetRealPathStatus();
    if (flag) {
        GTEST_LOG_(INFO) << "realpath Mock, Return nullptr";
        return nullptr;
    }

    const char* cfgPath = OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().GetOneCfgFilePath();
    if (cfgPath == nullptr) {
        GTEST_LOG_(ERROR) << "GetOneCfgFile failed: path is nullptr";
        return nullptr;
    }

    if (snprintf(resolved_path, PATH_MAX - 1, "%s", cfgPath) < 0) {
        GTEST_LOG_(ERROR) << "GetOneCfgFile failed: snprintf internal error";
        return nullptr;
    }

    resolved_path[PATH_MAX - 1] = '\0';
    return resolved_path;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif // __cplusplus