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

#ifndef VMA_UTILS_H
#define VMA_UTILS_H

#include <cstdint>
#include <cstddef>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
namespace VmaUtil {

/* 常量定义 */
constexpr size_t VMA_MAX_PATH_NAME = 256;   /**< 最大路径名长度 */
constexpr size_t VMA_MAX_PERMS = 5;         /**< 最大权限字符串长度（如 "r-xp"） */

/**
 * @brief VMA（虚拟内存区域）信息结构
 */
struct VMARegion {
    uintptr_t start;                    /**< 起始地址 */
    uintptr_t end;                      /**< 结束地址 */
    uintptr_t size;                     /**< 区域大小（字节） */
    char perms[VMA_MAX_PERMS];          /**< 权限字符串（如 "r-xp"） */
    unsigned long offset;               /**< 文件偏移量 */
    unsigned long inode;                /**< inode 号 */
    char pathname[VMA_MAX_PATH_NAME];   /**< 文件路径或区域名称 */
};

/**
 * @brief 获取指定文件的所有 VMA 区域
 *
 * @param filename 目标文件名
 * @return std::vector<VMARegion> VMA 区域列表
 *
 * @example
 * auto regions = vma::GetFileVmas("libc.so");
 * for (const auto& region : regions) {
 *     printf("%08lx-%08lx %s\n", region.start, region.end, region.perms);
 * }
 */
std::vector<VMARegion> GetFileVmas(const char* filename);

/**
 * @brief 获取多个指定文件的所有 VMA 区域
 *
 * @param filenames 目标文件名列表
 * @return std::vector<VMARegion> 所有匹配的 VMA 区域列表
 *
 * @note 一次遍历即可获取多个文件的 VMA，性能优于多次调用单文件版本
 *
 * @example
 * std::vector<std::string> files = {"libc.so", "libm.so"};
 * auto regions = vma::GetFileVmas(files);
 * for (const auto& region : regions) {
 *     printf("%08lx-%08lx %s %s\n", region.start, region.end, region.perms, region.pathname);
 * }
 */
std::vector<VMARegion> GetFileVmas(const std::vector<std::string>& filenames);

/**
 * @brief 获取指定文件的 PSS（比例共享大小）
 *
 * @param filename 目标文件名
 * @param pssKb 输出参数，PSS 值（KB）
 * @return int 成功返回 0，失败返回 -1
 *
 * @note 此函数会读取 /proc/self/smaps，性能开销较大
 */
int GetFileVmaPss(const char* filename, unsigned long* pssKb);

} // namespace VmaUtil
} // namespace AbilityRuntime
} // namespace OHOS

#endif // VMA_UTILS_H
