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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <cerrno>
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
typedef int (*VmaCallback)(const OHOS::AbilityRuntime::VmaUtil::VMARegion* region, void* userdata);

constexpr int HEX_BASE = 16;
constexpr int DEC_BASE = 10;
constexpr int PERMS_INDEX_0 = 0;
constexpr int PERMS_INDEX_1 = 1;
constexpr int PERMS_INDEX_2 = 2;
constexpr int PERMS_INDEX_3 = 3;
constexpr int PERMS_LEN = 4;
constexpr int PSS_PREFIX_LEN = 4;
constexpr size_t LINE_BUFFER_SIZE = 1024;

/**
 * @brief 高性能十六进制字符串转整数
 *
 * @param str 十六进制字符串
 * @param len 字符串长度
 * @return uintptr_t 转换后的数值
 */
static constexpr uintptr_t HexToUintptr(const char* str, size_t len)
{
    uintptr_t value = 0;
    for (size_t i = 0; i < len; i++) {
        value <<= HEX_BASE;
        char c = str[i];
        if (c >= '0' && c <= '9') {
            value |= (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            value |= (c - 'a' + DEC_BASE);
        } else if (c >= 'A' && c <= 'F') {
            value |= (c - 'A' + DEC_BASE);
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
static constexpr unsigned long DecToUlong(const char* str)
{
    unsigned long value = 0;
    while (*str >= '0' && *str <= '9') {
        value = value * DEC_BASE + (*str - '0');
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
static constexpr const char* SkipWhitespace(const char* ptr)
{
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
static constexpr bool IsFilenameMatch(const char* path, const char* filename)
{
    if (!path || !filename) {
        return false;
    }
    // 如果 filename 是路径，直接比较
    if (strchr(filename, '/') != nullptr) {
        return strstr(path, filename) != nullptr;
    }
    // 否则提取 basename 比较
    const char* lastSlash = strrchr(path, '/');
    const char* basename = (lastSlash == nullptr) ? path : (lastSlash + 1);
    return strcmp(basename, filename) == 0;
}

/**
 * @brief 解析 /proc/self/maps 的一行
 *
 * @param line 行内容
 * @param lineLen 行长度
 * @param region 输出参数，解析结果
 * @return true 解析成功
 * @return false 解析失败
 */
static bool ParseMapsLine(const char* line, size_t lineLen, OHOS::AbilityRuntime::VmaUtil::VMARegion* region)
{
    const char* ptr = line;

    // 1. 解析地址范围：40000000-40020000
    const char* dash = strchr(ptr, '-');
    if (!dash) {
        return false;
    }

    region->start = HexToUintptr(ptr, dash - ptr);
    ptr = dash + 1;

    const char* space = strchr(ptr, ' ');
    if (!space) {
        return false;
    }
    region->end = HexToUintptr(ptr, space - ptr);
    region->size = region->end - region->start;
    ptr = space + 1;

    // 2. 解析权限：r-xp
    ptr = SkipWhitespace(ptr);
    if (ptr[PERMS_INDEX_0] == '\0' || ptr[PERMS_INDEX_1] == '\0' ||
        ptr[PERMS_INDEX_2] == '\0' || ptr[PERMS_INDEX_3] == '\0') {
        return false;
    }
    region->perms[PERMS_INDEX_0] = ptr[PERMS_INDEX_0];
    region->perms[PERMS_INDEX_1] = ptr[PERMS_INDEX_1];
    region->perms[PERMS_INDEX_2] = ptr[PERMS_INDEX_2];
    region->perms[PERMS_INDEX_3] = ptr[PERMS_INDEX_3];
    region->perms[OHOS::AbilityRuntime::VmaUtil::VMA_MAX_PERMS - 1] = '\0';
    ptr += PERMS_LEN;

    // 3. 跳过偏移量：00000
    ptr = SkipWhitespace(ptr);
    ptr = strchr(ptr, ' ');
    if (!ptr) {
        return false;
    }
    ptr++;

    // 4. 跳过设备：08:01
    ptr = SkipWhitespace(ptr);
    ptr = strchr(ptr, ' ');
    if (!ptr) {
        return false;
    }
    ptr++;

    // 5. 解析 inode：1234567
    ptr = SkipWhitespace(ptr);
    region->inode = DecToUlong(ptr);
    ptr = strchr(ptr, ' ');
    if (!ptr) {
        return false;
    }
    ptr++;

    // 6. 解析路径（可选）
    ptr = SkipWhitespace(ptr);
    if (*ptr != '\0' && *ptr != '\n') {
        errno_t err = strncpy_s(region->pathname, sizeof(region->pathname), ptr, sizeof(region->pathname) - 1);
        if (err != EOK) {
            region->pathname[0] = '\0';
        } else {
            // 去除换行符
            char* newline = strchr(region->pathname, '\n');
            if (newline) {
                *newline = '\0';
            }
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
static int IterateMapsInternal(const char* filename, VmaCallback callback, void* userdata)
{
    const char* mapsPath = "/proc/self/maps";
    FILE* fp = fopen(mapsPath, "r");
    if (!fp) {
        TAG_LOGE(OHOS::AAFwk::AAFwkLogTag::DEFAULT, "Failed to open %{public}s: %{public}d", mapsPath, errno);
        return -1;
    }
    int count = 0;
    char line[LINE_BUFFER_SIZE];
    while (fgets(line, sizeof(line), fp) != nullptr) {
        OHOS::AbilityRuntime::VmaUtil::VMARegion region;
        size_t lineLen = strlen(line);
        if (ParseMapsLine(line, lineLen, &region)) {
            // 检查是否匹配目标文件
            // 如果 filename 为空字符串，匹配所有 VMA（由回调函数决定是否收集）
            bool match = (strlen(filename) == 0) || IsFilenameMatch(region.pathname, filename);
            if (match) {
                count++;
                if (callback && callback(&region, userdata) != 0) {
                    break;
                }
            }
        }
    }
    if (fclose(fp) != 0) {
        TAG_LOGE(OHOS::AAFwk::AAFwkLogTag::DEFAULT, "Failed to close file: %{public}d", errno);
    }
    return count;
}
} // anonymous namespace

namespace OHOS {
namespace AbilityRuntime {
namespace VmaUtil {
std::vector<VMARegion> GetFileVmas(const char* filename)
{
    if (!filename || strlen(filename) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid filename: null or empty");
        return {};
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "GetFileVmas: searching for %{public}s", filename);
    std::vector<VMARegion> vmaList;
    auto collectCallback = [](const VMARegion* region, void* userdata) -> int {
        std::vector<VMARegion>* list = static_cast<std::vector<VMARegion>*>(userdata);
        list->push_back(*region);
        return 0;
    };
    IterateMapsInternal(filename, collectCallback, &vmaList);
    TAG_LOGI(AAFwkTag::UIABILITY, "GetFileVmas: found %{public}zu VMAs for %{public}s",
        vmaList.size(), filename);
    return vmaList;  // RVO (Return Value Optimization) 避免拷贝
}

/**
 * @brief 多文件版本的回调数据结构
 */
struct MultiFileCallbackData {
    std::unordered_set<std::string> targetFilenames;  // 目标文件名集合
    std::vector<VMARegion>* vmaList;                   // 收集到的 VMA 列表
};

std::vector<VMARegion> GetFileVmas(const std::vector<std::string>& filenames)
{
    if (filenames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Filename list is empty");
        return {};
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "GetFileVmas: searching for %{public}zu files", filenames.size());
    // 使用 unordered_set 存储目标文件名，提高查找效率
    std::unordered_set<std::string> targetSet(filenames.begin(), filenames.end());
    // 使用 vector 收集匹配的 VMA
    std::vector<VMARegion> vmaList;
    // 定义多文件收集回调函数
    auto multiFileCallback = [](const VMARegion* region, void* userdata) -> int {
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
        if (data->targetFilenames.find(basename) != data->targetFilenames.end()) {
            data->vmaList->push_back(*region);
        }
        return 0;  // 继续遍历
    };
    // 准备回调数据
    MultiFileCallbackData callbackData = {targetSet, &vmaList};
    // 一次遍历完成收集
    IterateMapsInternal("", multiFileCallback, &callbackData);
    TAG_LOGI(AAFwkTag::UIABILITY, "GetFileVmas: found %{public}zu VMAs for %{public}zu files",
        vmaList.size(), filenames.size());

    return vmaList;  // RVO (Return Value Optimization) 避免拷贝
}

int GetFileVmaPss(const char* filename, unsigned long* pssKb)
{
    if (!filename || !pssKb) {
        return -1;
    }
    const char* smapsPath = "/proc/self/smaps";
    FILE* fp = fopen(smapsPath, "r");
    if (!fp) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to open %{public}s: %{public}d", smapsPath, errno);
        return -1;
    }
    char line[LINE_BUFFER_SIZE];
    unsigned long totalPss = 0;
    bool inTarget = false;
    while (fgets(line, sizeof(line), fp)) {
        // VMA 头行
        if (strchr(line, '-') && strchr(line, 'r')) {
            char pathname[VMA_MAX_PATH_NAME];
            int parsed = sscanf_s(line, "%*lx-%*lx %*4s %*lx %*s %*lu %255s", pathname, VMA_MAX_PATH_NAME);
            if (parsed >= 1) {
                inTarget = IsFilenameMatch(pathname, filename);
            }
        }
        // 解析 Pss 行
        else if (inTarget && strncmp(line, "Pss:", PSS_PREFIX_LEN) == 0) {
            unsigned long pss = 0;
            int parsed = sscanf_s(line, "Pss: %lu kB", &pss);
            if (parsed == 1) {
                totalPss += pss;
            }
        }
    }

    if (fclose(fp) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to close file: %{public}d", errno);
    }
    *pssKb = totalPss;
    TAG_LOGI(AAFwkTag::UIABILITY, "Total PSS for %{public}s: %{public}lu kB", filename, totalPss);
    return 0;
}

} // namespace VmaUtil
} // namespace AbilityRuntime
} // namespace OHOS
