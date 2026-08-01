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

#ifndef OHOS_ABILITY_RUNTIME_NATIVE_MODULE_COLLECTOR_H
#define OHOS_ABILITY_RUNTIME_NATIVE_MODULE_COLLECTOR_H

#include <string>

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Collects native modules imported by the application during startup and
 * persists them into the application sandbox cache directory.
 *
 * The collector is gated by a system parameter (disabled by default). When the
 * switch is absent or disabled, every entry point is a no-op with zero file I/O.
 *
 * Store file layout:
 *   <cacheDir>/.ohos.ability_runtime/native_modules_<processHash8>.v2.json
 * The dot-prefixed hidden directory marks the file as system-managed so it never
 * collides with developer files. processHash8 isolates files between processes
 * of the same application. The file permission is 0600.
 */
class NativeModuleCollector {
public:
    /**
     * @brief Whether collection is enabled.
     *
     * Placeholder: always returns false until the system parameter is wired
     * (tracked separately). The result is cached per process once wired.
     */
    static bool IsEnabled();

    /**
     * @brief Build the store file path under the given base directory.
     *
     * @param baseDir application cache directory, e.g. /data/storage/el2/base/cache
     * @param processName current process name, used for the hash suffix
     * @return full store file path, e.g.
     *         <baseDir>/.ohos.ability_runtime/native_modules_a1b2c3d4.v2.json
     */
    static std::string GetStorePath(const std::string& baseDir, const std::string& processName);

    /**
     * @brief Build the store directory under the given base directory.
     */
    static std::string GetStoreDir(const std::string& baseDir);

    /**
     * @brief 8-char lowercase hex FNV-1a (32-bit) hash of the process name.
     *
     * Used only for file naming isolation between processes, not for security.
     */
    static std::string ProcessNameHash(const std::string& processName);

    /**
     * @brief Recursively create the directory if it does not exist.
     * @return true if the directory exists after the call
     */
    static bool EnsureDir(const std::string& dir);

    /**
     * @brief Write content to a file with permission 0600.
     *
     * Plain (non-atomic) write is sufficient here; the atomic tmp+rename
     * strategy lands together with the merge engine.
     *
     * @return true on success
     */
    static bool WriteFileWithPermission(const std::string& path, const std::string& content);

    /**
     * @brief Startup checkpoint 1: sample loaded modules in memory (no disk write).
     *
     * Stub: no-op until the merge engine lands.
     */
    static void SampleAtAppReady();

    /**
     * @brief Startup checkpoint 2: sample remaining modules, merge scores and
     * write the store file at most once per launch.
     *
     * Stub: no-op until the merge engine lands.
     */
    static void CollectMergeAndSave();
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_NATIVE_MODULE_COLLECTOR_H
