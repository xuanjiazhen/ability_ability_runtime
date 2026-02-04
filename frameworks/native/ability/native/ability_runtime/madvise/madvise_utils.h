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

} // namespace madviseUtil

#endif // MADVISE_UTILS_H
