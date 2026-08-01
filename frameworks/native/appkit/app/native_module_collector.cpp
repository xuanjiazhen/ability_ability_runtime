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

#include "native_module_collector.h"

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* STORE_DIR_NAME = ".ohos.ability_runtime";
constexpr const char* STORE_FILE_PREFIX = "native_modules_";
constexpr const char* STORE_FILE_SUFFIX = ".v2.json";
constexpr mode_t STORE_DIR_MODE = 0700;
constexpr mode_t STORE_FILE_MODE = 0600;
constexpr uint32_t FNV1A_OFFSET_BASIS = 2166136261u;
constexpr uint32_t FNV1A_PRIME = 16777619u;
constexpr size_t HEX_HASH_LEN = 8;
} // namespace

bool NativeModuleCollector::IsEnabled()
{
    // Placeholder: the system parameter is wired later; disabled by default.
    return false;
}

std::string NativeModuleCollector::GetStoreDir(const std::string& baseDir)
{
    return baseDir + "/" + STORE_DIR_NAME;
}

std::string NativeModuleCollector::GetStorePath(const std::string& baseDir, const std::string& processName)
{
    return GetStoreDir(baseDir) + "/" + STORE_FILE_PREFIX + ProcessNameHash(processName) + STORE_FILE_SUFFIX;
}

std::string NativeModuleCollector::ProcessNameHash(const std::string& processName)
{
    uint32_t hash = FNV1A_OFFSET_BASIS;
    for (char c : processName) {
        hash ^= static_cast<uint8_t>(c);
        hash *= FNV1A_PRIME;
    }
    char buf[HEX_HASH_LEN + 1] = {0};
    (void)snprintf(buf, sizeof(buf), "%08x", hash);
    return std::string(buf);
}

bool NativeModuleCollector::EnsureDir(const std::string& dir)
{
    if (dir.empty()) {
        return false;
    }
    std::string current;
    current.reserve(dir.size());
    for (size_t i = 0; i < dir.size(); ++i) {
        current.push_back(dir[i]);
        if (dir[i] != '/' && i + 1 != dir.size()) {
            continue;
        }
        if (current == "/") {
            continue;
        }
        if (!current.empty() && current.back() == '/') {
            current.pop_back();
        }
        if (mkdir(current.c_str(), STORE_DIR_MODE) != 0 && errno != EEXIST) {
            TAG_LOGE(AAFwkTag::APPKIT, "mkdir %{public}s failed, errno %{public}d", current.c_str(), errno);
            return false;
        }
    }
    return true;
}

bool NativeModuleCollector::WriteFileWithPermission(const std::string& path, const std::string& content)
{
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, STORE_FILE_MODE);
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "open %{public}s failed, errno %{public}d", path.c_str(), errno);
        return false;
    }
    // open() mode does not tighten permissions of an already existing file
    if (fchmod(fd, STORE_FILE_MODE) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "fchmod %{public}s failed, errno %{public}d", path.c_str(), errno);
        close(fd);
        return false;
    }
    const char* data = content.data();
    size_t left = content.size();
    while (left > 0) {
        ssize_t written = write(fd, data, left);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            TAG_LOGE(AAFwkTag::APPKIT, "write %{public}s failed, errno %{public}d", path.c_str(), errno);
            close(fd);
            return false;
        }
        data += written;
        left -= static_cast<size_t>(written);
    }
    if (close(fd) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "close %{public}s failed, errno %{public}d", path.c_str(), errno);
        return false;
    }
    return true;
}

void NativeModuleCollector::SampleAtAppReady()
{
    if (!IsEnabled()) {
        return;
    }
    // Stub: sampling lands together with the merge engine.
}

void NativeModuleCollector::CollectMergeAndSave()
{
    if (!IsEnabled()) {
        return;
    }
    // Stub: merge and atomic write land together with the merge engine.
}
} // namespace AppExecFwk
} // namespace OHOS
