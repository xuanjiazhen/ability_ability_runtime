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

namespace madviseUtil {

/**
 * @brief 对指定的 SO 库应用 madvise MADV_DONTNEED 优化
 *
 * @param lib_name SO 库名称（如 "libbiglib.so"）
 * @return true 成功应用 madvise
 * @return false 应用失败
 */
bool madvise_single_library(const char* lib_name);

/**
 * @brief 对多个 SO 库批量应用 madvise MADV_DONTNEED 优化
 *
 * @param lib_names SO 库名称列表
 * @return int 成功优化的库数量
 */
int madvise_multiple_libraries(const std::vector<std::string>& lib_names);

/**
 * @brief 通用 madvise 接口 - 单个文件（支持 .hap、.so 等所有文件）
 *
 * @param filename 目标文件名
 * @return true 至少有一个段成功应用 madvise
 * @return false 应用失败
 *
 * @note 使用 vmaUtil::get_file_vmas 从 /proc/self/maps 获取 VMA，然后应用 madvise
 */
bool madvise_general_file(const char* filename);

/**
 * @brief 通用 madvise 接口 - 多个文件
 *
 * @param filenames 目标文件名列表
 * @return int 成功应用 madvise 的文件数量
 *
 * @note 一次遍历获取所有文件的 VMA，然后应用 madvise
 */
int madvise_general_files(const std::vector<std::string>& filenames);

/**
 * @brief 使用配置文件对指定应用应用 madvise 优化
 *
 * @param package_name 应用包名
 * @return int 成功优化的库数量，失败返回 -1
 *
 * @note 此函数会从默认配置文件路径读取配置，然后应用 madvise 优化
 *       配置文件路径: /system/etc/madvise_config.json
 *       - type: ELF -> 调用 madvise_single_library()
 *       - type: VMA -> 调用 madvise_general_file()
 */
int madvise_with_config_file(const char* package_name);

} // namespace madviseUtil

#endif // MADVISE_UTILS_H
