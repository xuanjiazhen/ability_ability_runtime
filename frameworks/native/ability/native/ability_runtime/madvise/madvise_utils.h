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

#ifndef MADVISE_UTILS_H
#define MADVISE_UTILS_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
namespace MadviseUtil {

/**
 * @brief 对指定的 SO 库应用 madvise MADV_DONTNEED 优化
 *
 * @param libName SO 库名称（如 "libbiglib.so"）
 * @return true 成功应用 madvise
 * @return false 应用失败
 */
bool MadviseSingleLibrary(const char* libName);

/**
 * @brief 对多个 SO 库批量应用 madvise MADV_DONTNEED 优化
 *
 * @param libNames SO 库名称列表
 * @return int 成功优化的库数量
 */
int MadviseMultipleLibraries(const std::vector<std::string>& libNames);

/**
 * @brief 通用 madvise 接口 - 单个文件（支持 .hap、.so 等所有文件）
 *
 * @param filename 目标文件名
 * @return true 至少有一个段成功应用 madvise
 * @return false 应用失败
 *
 * @note 使用 vmaUtil::GetFileVmas 从 /proc/self/maps 获取 VMA，然后应用 madvise
 */
bool MadviseGeneralFile(const char* filename);

/**
 * @brief 通用 madvise 接口 - 多个文件
 *
 * @param filenames 目标文件名列表
 * @return int 成功应用 madvise 的文件数量
 *
 * @note 一次遍历获取所有文件的 VMA，然后应用 madvise
 */
int MadviseGeneralFiles(const std::vector<std::string>& filenames);

/**
 * @brief 使用配置文件对指定应用应用 madvise 优化
 *
 * @param packageName 应用包名
 * @return int 成功优化的库数量，失败返回 -1
 *
 * @note 此函数会从默认配置文件路径读取配置，然后应用 madvise 优化
 *       配置文件路径: /system/etc/ams_madvise_config.json
 *       - type: ELF -> 调用 MadviseSingleLibrary()
 *       - type: VMA -> 调用 MadviseGeneralFile()
 */
int MadviseWithConfigFile(const char* packageName);

/**
 * @brief 自动优化当前进程占用页面最多的前 10 个文件
 *
 * @return int 成功优化的文件数量
 *
 * @note 此函数会扫描 /proc/self/maps，统计各文件占用的页面数，
 *       对页面数最多的前 10 个文件应用 madvise MADV_DONTNEED：
 *       - 文件名以 .so 结尾：使用 ELF 方法（dl_iterate_phdr）
 *       - 其他文件：使用 VMA 方法（/proc/self/maps）
 */
int MadviseTopFiles();

std::string GetConfigPath();

} // namespace MadviseUtil
} // namespace AbilityRuntime
} // namespace OHOS

#endif // MADVISE_UTILS_H
