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

#include "vma_utils.h"
#include "hilog_tag_wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <vector>
#include <unordered_set>
#include <string>

namespace {
/**
 * @brief VMA 遍历回调函数类型
 *
 * @param region 当前 VMA 区域
 * @param userdata 用户数据
 * @return int 返回 0 继续遍历，返回非 0 停止遍历
 */
typedef int (*vma_callback)(const vmaUtil::VMARegion* region, void* userdata);

/**
 * @brief 高性能十六进制字符串转整数
 *
 * @param str 十六进制字符串
 * @param len 字符串长度
 * @return uintptr_t 转换后的数值
 */
static constexpr uintptr_t hex_to_uintptr(const char* str, size_t len) {
    uintptr_t value = 0;
    for (size_t i = 0; i < len; i++) {
        value <<= 4;
        char c = str[i];
        if (c >= '0' && c <= '9') {
            value |= (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            value |= (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            value |= (c - 'A' + 10);
        }
    }
    return value;
}

/**
 * @brief 高性能十进制字符串转整数
 *
 * @param str 十进制字符串
 * @return unsigned long 转换后的数值
 */
static constexpr unsigned long dec_to_ulong(const char* str) {
    unsigned long value = 0;
    while (*str >= '0' && *str <= '9') {
        value = value * 10 + (*str - '0');
        str++;
    }
    return value;
}

/**
 * @brief 快速跳过空白字符
 *
 * @param ptr 当前指针
 * @return const char* 跳过空白后的指针
 */
static constexpr const char* skip_whitespace(const char* ptr) {
    while (*ptr == ' ' || *ptr == '\t') {
        ptr++;
    }
    return ptr;
}

/**
 * @brief 快速字符串匹配（检查 filename 是否在 path 中）
 *
 * @param path 完整路径
 * @param filename 目标文件名
 * @return true 匹配成功
 * @return false 匹配失败
 */
static constexpr bool is_filename_match(const char* path, const char* filename) {
    if (!path || !filename) {
        return false;
    }

    // 如果 filename 是路径，直接比较
    if (strchr(filename, '/') != nullptr) {
        return strstr(path, filename) != nullptr;
    }

    // 否则提取 basename 比较
    const char* last_slash = strrchr(path, '/');
    const char* basename = (last_slash == nullptr) ? path : (last_slash + 1);
    return strcmp(basename, filename) == 0;
}

/**
 * @brief 解析 /proc/self/maps 的一行
 *
 * @param line 行内容
 * @param line_len 行长度
 * @param region 输出参数，解析结果
 * @return true 解析成功
 * @return false 解析失败
 */
static bool parse_maps_line(const char* line, size_t line_len, vmaUtil::VMARegion* region) {
    const char* ptr = line;

    // 1. 解析地址范围：40000000-40020000
    const char* dash = strchr(ptr, '-');
    if (!dash) return false;

    region->start = hex_to_uintptr(ptr, dash - ptr);
    ptr = dash + 1;

    const char* space = strchr(ptr, ' ');
    if (!space) return false;
    region->end = hex_to_uintptr(ptr, space - ptr);
    region->size = region->end - region->start;
    ptr = space + 1;

    // 2. 解析权限：r-xp
    ptr = skip_whitespace(ptr);
    if (ptr[0] == '\0' || ptr[1] == '\0' || ptr[2] == '\0' || ptr[3] == '\0') return false;
    region->perms[0] = ptr[0];
    region->perms[1] = ptr[1];
    region->perms[2] = ptr[2];
    region->perms[3] = ptr[3];
    region->perms[4] = '\0';
    ptr += 4;

    // 3. 跳过偏移量：00000
    ptr = skip_whitespace(ptr);
    ptr = strchr(ptr, ' ');
    if (!ptr) return false;
    ptr++;

    // 4. 跳过设备：08:01
    ptr = skip_whitespace(ptr);
    ptr = strchr(ptr, ' ');
    if (!ptr) return false;
    ptr++;

    // 5. 解析 inode：1234567
    ptr = skip_whitespace(ptr);
    region->inode = dec_to_ulong(ptr);
    ptr = strchr(ptr, ' ');
    if (!ptr) return false;
    ptr++;

    // 6. 解析路径（可选）
    ptr = skip_whitespace(ptr);
    if (*ptr != '\0' && *ptr != '\n') {
        strncpy(region->pathname, ptr, sizeof(region->pathname) - 1);
        region->pathname[sizeof(region->pathname) - 1] = '\0';

        // 去除换行符
        char* newline = strchr(region->pathname, '\n');
        if (newline) {
            *newline = '\0';
        }
    } else {
        region->pathname[0] = '\0';
    }

    return true;
}

/**
 * @brief 读取并解析 /proc/self/maps（兼容 OpenHarmony）
 *
 * @param filename 目标文件名
 * @param callback 回调函数
 * @param userdata 用户数据
 * @return int 找到的 VMA 数量
 */
static int iterate_maps_internal(const char* filename, vma_callback callback, void* userdata) {
    const char* maps_path = "/proc/self/maps";
    FILE* fp = fopen(maps_path, "r");
    if (!fp) {
        TAG_LOGE(OHOS::AAFwk::AAFwkLogTag::DEFAULT, "Failed to open %{public}s: %{public}d", maps_path, errno);
        return -1;
    }

    int count = 0;
    char line[1024];  // 足够大的缓冲区存储一行

    while (fgets(line, sizeof(line), fp)) {
        // 解析一行
        vmaUtil::VMARegion region;
        size_t line_len = strlen(line);

        if (parse_maps_line(line, line_len, &region)) {
            // 检查是否匹配目标文件
            // 如果 filename 为空字符串，匹配所有 VMA（由回调函数决定是否收集）
            bool match = (strlen(filename) == 0) || is_filename_match(region.pathname, filename);
            if (match) {
                count++;
                if (callback && callback(&region, userdata) != 0) {
                    break;  // 回调要求停止
                }
            }
        }
    }

    fclose(fp);

    return count;
}

} // anonymous namespace

// ========== 公共 API 实现 ==========

namespace vmaUtil {

std::vector<VMARegion> get_file_vmas(const char* filename) {
    if (!filename || strlen(filename) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid filename: null or empty");
        return {};
    }

    TAG_LOGI(AAFwkTag::UIABILITY, "get_file_vmas: searching for %{public}s", filename);

    // 使用 vector 收集匹配的 VMA
    std::vector<VMARegion> vma_list;

    // 定义收集回调函数
    auto collect_callback = [](const VMARegion* region, void* userdata) -> int {
        std::vector<VMARegion>* list = static_cast<std::vector<VMARegion>*>(userdata);
        list->push_back(*region);
        return 0;  // 继续遍历
    };

    // 一次遍历完成收集
    iterate_maps_internal(filename, collect_callback, &vma_list);

    TAG_LOGI(AAFwkTag::UIABILITY, "get_file_vmas: found %{public}zu VMAs for %{public}s", vma_list.size(), filename);

    return vma_list;  // RVO (Return Value Optimization) 避免拷贝
}

/**
 * @brief 多文件版本的回调数据结构
 */
struct MultiFileCallbackData {
    std::unordered_set<std::string> target_filenames;  // 目标文件名集合
    std::vector<VMARegion>* vma_list;                   // 收集到的 VMA 列表
};

std::vector<VMARegion> get_file_vmas(const std::vector<std::string>& filenames) {
    if (filenames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Filename list is empty");
        return {};
    }

    TAG_LOGI(AAFwkTag::UIABILITY, "get_file_vmas: searching for %{public}zu files", filenames.size());

    // 使用 unordered_set 存储目标文件名，提高查找效率
    std::unordered_set<std::string> target_set(filenames.begin(), filenames.end());

    // 使用 vector 收集匹配的 VMA
    std::vector<VMARegion> vma_list;

    // 定义多文件收集回调函数
    auto multi_file_callback = [](const VMARegion* region, void* userdata) -> int {
        MultiFileCallbackData* data = static_cast<MultiFileCallbackData*>(userdata);

        // 检查当前 VMA 是否匹配任一目标文件
        const char* pathname = region->pathname;
        if (!pathname || strlen(pathname) == 0) {
            return 0;  // 跳过无名 VMA
        }

        // 提取 basename
        const char* basename = strrchr(pathname, '/');
        basename = (basename == nullptr) ? pathname : (basename + 1);

        // 检查是否在目标文件集合中
        if (data->target_filenames.find(basename) != data->target_filenames.end()) {
            data->vma_list->push_back(*region);
        }

        return 0;  // 继续遍历
    };

    // 准备回调数据
    MultiFileCallbackData callback_data = {target_set, &vma_list};

    // 一次遍历完成收集
    iterate_maps_internal("", multi_file_callback, &callback_data);

    TAG_LOGI(AAFwkTag::UIABILITY, "get_file_vmas: found %{public}zu VMAs for %{public}zu files",
        vma_list.size(), filenames.size());

    return vma_list;  // RVO (Return Value Optimization) 避免拷贝
}

int get_file_vma_pss(const char* filename, unsigned long* pss_kb) {
    if (!filename || !pss_kb) {
        return -1;
    }

    const char* smaps_path = "/proc/self/smaps";
    FILE* fp = fopen(smaps_path, "r");
    if (!fp) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to open %{public}s: %{public}d", smaps_path, errno);
        return -1;
    }

    char line[1024];
    unsigned long total_pss = 0;
    bool in_target = false;

    while (fgets(line, sizeof(line), fp)) {
        // VMA 头行
        if (strchr(line, '-') && strchr(line, 'r')) {
            char pathname[VMA_MAX_PATH_NAME];
            if (sscanf(line, "%*lx-%*lx %*4s %*lx %*s %*lu %255s", pathname) >= 1) {
                in_target = is_filename_match(pathname, filename);
            }
        }
        // 解析 Pss 行
        else if (in_target && strncmp(line, "Pss:", 4) == 0) {
            unsigned long pss;
            if (sscanf(line, "Pss: %lu kB", &pss) == 1) {
                total_pss += pss;
            }
        }
    }

    fclose(fp);

    *pss_kb = total_pss;

    TAG_LOGI(AAFwkTag::UIABILITY, "Total PSS for %{public}s: %{public}lu kB", filename, total_pss);
    return 0;
}

} // namespace vmaUtil
