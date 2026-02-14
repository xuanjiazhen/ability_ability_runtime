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

#include "madvise_utils.h"

#include "cJSON.h"
#include <algorithm>
#include <chrono>
#include <libgen.h>
#include <cstring>
#include <dlfcn.h>
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <unordered_set>
#include <string>
#include <vector>

#include "config_policy_utils.h"
#include "hilog_tag_wrapper.h"
#include "vma_utils.h"

namespace {
constexpr const char* MADVISE_CONFIG_DEFAULT_FILE_PATH = "/system/etc/ams_madvise_config.json";
constexpr const char* MADVISE_CONFIG_FILE_PATH = "/etc/ams_madvise_config.json";

constexpr int PERMS_INDEX_0 = 0;
constexpr int PERMS_INDEX_1 = 1;
constexpr int PERMS_INDEX_2 = 2;
constexpr int PERMS_INDEX_3 = 3;
constexpr size_t SO_SUFFIX_LEN = 3;
constexpr size_t LINE_BUFFER_SIZE = 1024;
constexpr size_t PATH_BUFFER_SIZE = 256;
constexpr int PARSED_MIN_FIELDS = 6;
constexpr size_t TOP_FILE_COUNT = 30;

/**
 * @brief 库文件类型
 */
enum class LibraryType {
    UNKNOWN = 0,
    ELF,
    VMA
};

/**
 * @brief 单个库的配置
 */
struct LibraryConfig {
    std::string name;
    LibraryType type;
    std::vector<unsigned long> hotPageOffsets;  // 预留字段，当前未使用
};

/**
 * @brief 应用配置
 */
struct AppConfig {
    std::string packageName;
    std::vector<LibraryConfig> libraries;
};

/**
 * @brief Madvise 配置
 */
struct MadviseConfig {
    std::vector<AppConfig> apps;

    bool IsValid() const
    {
        return !apps.empty();
    }

    const AppConfig* FindApp(const char* packageName) const
    {
        if (!packageName || strlen(packageName) == 0) {
            return nullptr;
        }
        for (const auto& app : apps) {
            if (app.packageName == packageName) {
                return &app;
            }
        }
        return nullptr;
    }
};

/**
 * @brief 字符串转 LibraryType
 */
LibraryType ParseLibraryType(const std::string& typeStr)
{
    if (typeStr == "elf") {
        return LibraryType::ELF;
    } else if (typeStr == "vma") {
        return LibraryType::VMA;
    }
    return LibraryType::UNKNOWN;
}

/**
 * @brief 从默认路径加载配置文件
 */
MadviseConfig LoadConfigFromFile()
{
    MadviseConfig config;
    std::string configPath = OHOS::AbilityRuntime::MadviseUtil::GetConfigPath();
    // 读取文件内容
    FILE* file = fopen(configPath.c_str(), "r");
    if (!file) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to open config file: %{public}s", configPath.c_str());
        return config;
    }
    // 获取文件大小
    if (fseek(file, 0, SEEK_END) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to seek file end: %{public}s", configPath.c_str());
        if (fclose(file) != 0) {
            TAG_LOGE(AAFwkTag::ABILITY, "Failed to close file");
        }
        return config;
    }
    long fileSize = ftell(file);
    if (fseek(file, 0, SEEK_SET) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to seek file begin: %{public}s", configPath.c_str());
        if (fclose(file) != 0) {
            TAG_LOGE(AAFwkTag::ABILITY, "Failed to close file");
        }
        return config;
    }
    if (fileSize <= 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Config file is empty: %{public}s", configPath.c_str());
        if (fclose(file) != 0) {
            TAG_LOGE(AAFwkTag::ABILITY, "Failed to close file");
        }
        return config;
    }
    // 读取文件内容
    std::string content;
    content.resize(fileSize);
    size_t readSize = fread(&content[0], 1, fileSize, file);
    if (fclose(file) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to close file");
    }
    if (readSize != static_cast<size_t>(fileSize)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to read config file: %{public}s", configPath.c_str());
        return config;
    }
    // 使用 cJSON 解析 JSON
    cJSON* root = cJSON_Parse(content.c_str());
    if (!root) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to parse JSON config: %{public}s", configPath.c_str());
        return config;
    }
    // 获取 apps 数组
    cJSON* appsArray = cJSON_GetObjectItemCaseSensitive(root, "apps");
    if (!cJSON_IsArray(appsArray)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid config format: 'apps' not found or not an array");
        cJSON_Delete(root);
        return config;
    }
    // 遍历 apps 数组
    cJSON* appItem = nullptr;
    cJSON_ArrayForEach(appItem, appsArray) {
        AppConfig appConfig;
        // 获取 package_name
        cJSON* packageNameItem = cJSON_GetObjectItemCaseSensitive(appItem, "package_name");
        if (cJSON_IsString(packageNameItem) && packageNameItem->valuestring) {
            appConfig.packageName = packageNameItem->valuestring;
        }
        // 获取 libraries 数组
        cJSON* libsArray = cJSON_GetObjectItemCaseSensitive(appItem, "libraries");
        if (cJSON_IsArray(libsArray)) {
            cJSON* libItem = nullptr;
            cJSON_ArrayForEach(libItem, libsArray) {
                LibraryConfig libConfig;
                // 获取 name
                cJSON* nameItem = cJSON_GetObjectItemCaseSensitive(libItem, "name");
                if (cJSON_IsString(nameItem) && nameItem->valuestring) {
                    libConfig.name = nameItem->valuestring;
                }
                // 获取 type
                cJSON* typeItem = cJSON_GetObjectItemCaseSensitive(libItem, "type");
                if (cJSON_IsString(typeItem) && typeItem->valuestring) {
                    libConfig.type = ParseLibraryType(typeItem->valuestring);
                }
                // hot_page_offsets 字段存在但当前未使用，直接跳过
                // cJSON 会在下面 Delete 时自动释放
                // 只添加有效的库配置
                if (!libConfig.name.empty() && libConfig.type != LibraryType::UNKNOWN) {
                    appConfig.libraries.push_back(libConfig);
                }
            }
        }
        // 只添加有效的应用配置
        if (!appConfig.packageName.empty() && !appConfig.libraries.empty()) {
            config.apps.push_back(appConfig);
        }
    }

    cJSON_Delete(root);
    TAG_LOGI(AAFwkTag::UIABILITY, "Loaded madvise config from %{public}s: %{public}zu apps",
        configPath.c_str(), config.apps.size());
    return config;
}

/**
 * @brief 使用配置对指定应用应用 madvise 优化
 */
int ApplyMadviseWithConfig(const char* packageName, const MadviseConfig& config)
{
    if (!packageName || strlen(packageName) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid package name: null or empty");
        return 0;
    }
    // 查找匹配的应用配置
    const AppConfig* appConfig = config.FindApp(packageName);
    if (!appConfig) {
        TAG_LOGI(AAFwkTag::UIABILITY, "No config found for package: %{public}s", packageName);
        return 0;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "Applying madvise for package: %{public}s, libraries: %{public}zu",
        packageName, appConfig->libraries.size());
    int successCount = 0;
    // 遍历库列表进行优化
    for (const auto& libConfig : appConfig->libraries) {
        bool libSuccess = false;
        if (libConfig.type == LibraryType::ELF) {
            // 使用 ELF 专用接口
            TAG_LOGI(AAFwkTag::UIABILITY, "Applying ELF madvise for: %{public}s", libConfig.name.c_str());
            libSuccess = OHOS::AbilityRuntime::MadviseUtil::MadviseSingleLibrary(libConfig.name.c_str());
        } else if (libConfig.type == LibraryType::VMA) {
            // 使用通用接口
            TAG_LOGI(AAFwkTag::UIABILITY, "Applying VMA madvise for: %{public}s", libConfig.name.c_str());
            libSuccess = OHOS::AbilityRuntime::MadviseUtil::MadviseGeneralFile(libConfig.name.c_str());
        } else {
            TAG_LOGW(AAFwkTag::ABILITY, "Unknown library type for: %{public}s", libConfig.name.c_str());
            continue;
        }
        if (libSuccess) {
            successCount++;
        }
    }
    TAG_LOGI(AAFwkTag::UIABILITY,
        "Madvise completed for package %{public}s: %{public}d/%{public}zu libraries succeeded",
        packageName, successCount, appConfig->libraries.size());
    return successCount;
}

} // anonymous namespace

// madvise 回调函数的数据结构（单个库）
struct MadviseData {
    int successCount;
    int failCount;
    const char* targetLibName;
};

// madvise 回调函数的数据结构（多个库）
struct MadviseDataMultiple {
    int successCount;  // 成功优化的库数量
    int failCount;     // 失败的库数量
    std::unordered_set<std::string>* targetLibNames;  // 目标库名称集合
    std::unordered_set<std::string> processedLibs;      // 已处理的库（避免重复计数）
};

/**
 * @brief dl_iterate_phdr 回调函数，对目标库应用 madvise
 */
static int MadvisePhdrCallback(struct dl_phdr_info *info, size_t size, void *data)
{
    MadviseData* madviseData = static_cast<MadviseData*>(data);
    // 获取当前库名
    const char* currentLibName = info->dlpi_name;
    if (!currentLibName || strlen(currentLibName) == 0) {
        currentLibName = "unknown";  // 处理主程序无名的情况
    }
    // 检查是否是目标库（使用模糊匹配）
    if (strstr(currentLibName, madviseData->targetLibName) == nullptr) {
        return 0;  // 不是目标库，跳过
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "Found target library: %{public}s", currentLibName);
    // 遍历所有程序头
    for (int i = 0; i < info->dlpi_phnum; i++) {
        const ElfW(Phdr) *phdr = &info->dlpi_phdr[i];
        // 只处理 LOAD 段(实际加载到内存的段)
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        // 检查段权限
        bool isReadable = (phdr->p_flags & PF_R);
        bool isWritable = (phdr->p_flags & PF_W);
        // 跳过可写段
        if (!isReadable || isWritable) {
            continue;
        }
        void* startAddr = reinterpret_cast<void*>(info->dlpi_addr + phdr->p_vaddr);
        size_t len = phdr->p_memsz;
        // 对齐到页边界
        size_t pageSize = getpagesize();
        void* alignedStart = reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(startAddr) & ~(pageSize - 1)
        );
        size_t alignedLen = (reinterpret_cast<uintptr_t>(startAddr) + len +
                            pageSize - 1) & ~(pageSize - 1);
        alignedLen -= reinterpret_cast<uintptr_t>(alignedStart);

        // 获取段权限描述
        char perms[4] = "---";
        if (phdr->p_flags & PF_R) {
            perms[PERMS_INDEX_0] = 'r';
        }
        if (phdr->p_flags & PF_W) {
            perms[PERMS_INDEX_1] = 'w';
        }
        if (phdr->p_flags & PF_X) {
            perms[PERMS_INDEX_2] = 'x';
        }
        perms[PERMS_INDEX_3] = '\0';
        TAG_LOGI(AAFwkTag::UIABILITY,
            "madvise: lib=%{public}s, perms=%{public}s, addr=%{public}p, len=%{public}zu, alignedLen=%{public}zu",
            info->dlpi_name ? info->dlpi_name : "unknown",
            perms,
            startAddr,
            len,
            alignedLen);
        // 调用 madvise，建议内核这些内存不需要
        int result = madvise(alignedStart, alignedLen, MADV_DONTNEED);
        if (result == 0) {
            TAG_LOGI(AAFwkTag::UIABILITY, "madvise success: addr=%{public}p, len=%{public}zu",
                alignedStart, alignedLen);
            madviseData->successCount++;
        } else {
            TAG_LOGE(AAFwkTag::ABILITY, "madvise failed: addr=%{public}p, len=%{public}zu, errno=%{public}d",
                alignedStart, alignedLen, errno);
            madviseData->failCount++;
        }
    }
    return 0;
}

/**
 * @brief dl_iterate_phdr 回调函数，批量处理多个目标库
 */
static int MadvisePhdrCallbackMultiple(struct dl_phdr_info *info, size_t size, void *data)
{
    MadviseDataMultiple* madviseData = static_cast<MadviseDataMultiple*>(data);
    // 获取当前库名
    const char* currentLibName = info->dlpi_name;
    if (!currentLibName || strlen(currentLibName) == 0) {
        return 0;  // 跳过主程序
    }
    // 提取基名称（去掉路径）
    const char* baseName = strrchr(currentLibName, '/');
    baseName = (baseName == nullptr) ? currentLibName : (baseName + 1);
    // 检查是否在目标库列表中
    if (madviseData->targetLibNames->find(baseName) == madviseData->targetLibNames->end()) {
        return 0;  // 不是目标库，跳过
    }
    // 检查是否已经处理过
    if (madviseData->processedLibs.find(baseName) != madviseData->processedLibs.end()) {
        return 0;  // 已处理过，跳过
    }
    // 标记为已处理
    madviseData->processedLibs.insert(baseName);
    TAG_LOGI(AAFwkTag::UIABILITY, "Processing target library: %{public}s", currentLibName);
    int libSuccessCount = 0;
    int libFailCount = 0;
    // 遍历所有程序头
    for (int i = 0; i < info->dlpi_phnum; i++) {
        const ElfW(Phdr) *phdr = &info->dlpi_phdr[i];
        // 只处理 LOAD 段(实际加载到内存的段)
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        // 检查段权限
        bool isReadable = (phdr->p_flags & PF_R);
        bool isWritable = (phdr->p_flags & PF_W);
        // 跳过可写段
        if (!isReadable || isWritable) {
            continue;
        }
        void* startAddr = reinterpret_cast<void*>(info->dlpi_addr + phdr->p_vaddr);
        size_t len = phdr->p_memsz;
        // 对齐到页边界
        size_t pageSize = getpagesize();
        void* alignedStart = reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(startAddr) & ~(pageSize - 1)
        );
        size_t alignedLen = (reinterpret_cast<uintptr_t>(startAddr) + len +
                            pageSize - 1) & ~(pageSize - 1);
        alignedLen -= reinterpret_cast<uintptr_t>(alignedStart);

        // 获取段权限描述
        char perms[4] = "---";
        if (phdr->p_flags & PF_R) {
            perms[PERMS_INDEX_0] = 'r';
        }
        if (phdr->p_flags & PF_W) {
            perms[PERMS_INDEX_1] = 'w';
        }
        if (phdr->p_flags & PF_X) {
            perms[PERMS_INDEX_2] = 'x';
        }
        perms[PERMS_INDEX_3] = '\0';
        TAG_LOGI(AAFwkTag::UIABILITY,
            "madvise: lib=%{public}s, perms=%{public}s, addr=%{public}p, len=%{public}zu, alignedLen=%{public}zu",
            info->dlpi_name ? info->dlpi_name : "unknown",
            perms,
            startAddr,
            len,
            alignedLen);
        // 调用 madvise，建议内核这些内存不需要
        int result = madvise(alignedStart, alignedLen, MADV_DONTNEED);
        if (result == 0) {
            TAG_LOGI(AAFwkTag::UIABILITY, "madvise success: addr=%{public}p, len=%{public}zu",
                alignedStart, alignedLen);
            libSuccessCount++;
        } else {
            TAG_LOGE(AAFwkTag::ABILITY, "madvise failed: addr=%{public}p, len=%{public}zu, errno=%{public}d",
                alignedStart, alignedLen, errno);
            libFailCount++;
        }
    }
    // 如果至少有一个段成功，则该库优化成功
    if (libSuccessCount > 0) {
        madviseData->successCount++;
        TAG_LOGI(AAFwkTag::UIABILITY, "Library %{public}s optimization completed: %{public}d segments succeeded",
            baseName, libSuccessCount);
    } else {
        madviseData->failCount++;
        TAG_LOGE(AAFwkTag::ABILITY, "Library %{public}s optimization failed", baseName);
    }
    return 0;
}

namespace OHOS {
namespace AbilityRuntime {
namespace MadviseUtil {

bool MadviseSingleLibrary(const char* libName)
{
    if (!libName || strlen(libName) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid library name: empty string");
        return false;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "MadviseSingleLibrary called for library=%{public}s", libName);
    // 初始化数据结构
    MadviseData madviseData = {0, 0, libName};
    // 使用 dl_iterate_phdr 遍历所有加载的段并对目标库调用 madvise
    dl_iterate_phdr(MadvisePhdrCallback, &madviseData);
    // 打印统计结果
    TAG_LOGI(AAFwkTag::UIABILITY,
        "madvise completed for lib=%{public}s: successCount=%{public}d, failCount=%{public}d",
        libName, madviseData.successCount, madviseData.failCount);
    // 返回是否至少有一个段成功应用了 madvise
    return (madviseData.successCount > 0);
}

int MadviseMultipleLibraries(const std::vector<std::string>& libNames)
{
    if (libNames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Library name list is empty");
        return 0;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "MadviseMultipleLibraries called for %{public}zu libraries", libNames.size());
    // 创建目标库集合
    std::unordered_set<std::string> targetLibSet(libNames.begin(), libNames.end());
    // 初始化数据结构
    MadviseDataMultiple madviseData = {0, 0, &targetLibSet, {}};
    // 使用 dl_iterate_phdr 一次遍历所有加载的库
    dl_iterate_phdr(MadvisePhdrCallbackMultiple, &madviseData);
    // 打印统计结果
    TAG_LOGI(AAFwkTag::UIABILITY, "MadviseMultipleLibraries completed: %{public}d succeeded, %{public}d failed",
        madviseData.successCount, madviseData.failCount);
    return madviseData.successCount;
}

/**
 * @brief 内部辅助函数：对单个 VMA 段应用 madvise
 *
 * @param region VMA 区域
 * @param pageSize 页大小
 * @param filename 文件名（用于日志）
 * @return int 0=失败/跳过，1=成功
 */
static int ApplyMadviseToRegion(const OHOS::AbilityRuntime::VmaUtil::VMARegion& region, size_t pageSize, const char* filename)
{
    // 检查权限：只优化只读段(r--)和可执行段(r-x)
    bool isReadonly = (strstr(region.perms, "r--") != nullptr);
    bool isExecutable = (strstr(region.perms, "r-x") != nullptr);
    // 跳过可写段和其他不需要优化的段
    if (!isReadonly && !isExecutable) {
        return 0;
    }
    // 计算对齐后的地址和长度
    void* startAddr = reinterpret_cast<void*>(region.start);
    void* alignedStart = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(startAddr) & ~(pageSize - 1)
    );
    void* endAddr = reinterpret_cast<void*>(region.end);
    size_t alignedLen = (reinterpret_cast<uintptr_t>(endAddr) + pageSize - 1) & ~(pageSize - 1);
    alignedLen -= reinterpret_cast<uintptr_t>(alignedStart);

    TAG_LOGI(AAFwkTag::UIABILITY,
        "madvise: file=%{public}s, perms=%{public}s, addr=%{public}p, len=%{public}zu, alignedLen=%{public}zu",
        filename ? filename : "unknown",
        region.perms,
        startAddr,
        region.size,
        alignedLen);
    // 调用 madvise，建议内核这些内存不需要
    int result = madvise(alignedStart, alignedLen, MADV_DONTNEED);
    if (result == 0) {
        TAG_LOGI(AAFwkTag::UIABILITY, "madvise success: addr=%{public}p, len=%{public}zu",
            alignedStart, alignedLen);
        return 1;
    } else {
        TAG_LOGE(AAFwkTag::ABILITY, "madvise failed: addr=%{public}p, len=%{public}zu, errno=%{public}d",
            alignedStart, alignedLen, errno);
        return 0;
    }
}

/**
 * @brief 内部辅助函数：处理 VMA 列表（单文件和多文件版本共享）
 *
 * @param regions VMA 区域列表
 * @return int 成功应用 madvise 的段数量
 */
static int ProcessVmaRegions(const std::vector<OHOS::AbilityRuntime::VmaUtil::VMARegion>& regions)
{
    if (regions.empty()) {
        return 0;
    }
    int successCount = 0;
    size_t pageSize = getpagesize();
    // 遍历所有 VMA 段，应用 madvise
    for (const auto& region : regions) {
        successCount += ApplyMadviseToRegion(region, pageSize, region.pathname);
    }
    return successCount;
}

/**
 * @brief 通用 madvise 接口 - 单个文件版本（支持 .hap、.so 等所有文件）
 *
 * @param filename 目标文件名
 * @return bool 是否至少有一个段成功应用了 madvise
 *
 * @note 使用 GetFileVmas 从 /proc/self/maps 获取 VMA，然后应用 madvise
 */
bool MadviseGeneralFile(const char* filename)
{
    if (!filename || strlen(filename) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid filename: empty string");
        return false;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "MadviseGeneralFile called for file=%{public}s", filename);
    // 使用 GetFileVmas 获取所有匹配的 VMA 段
    std::vector<OHOS::AbilityRuntime::VmaUtil::VMARegion> regions = OHOS::AbilityRuntime::VmaUtil::GetFileVmas(filename);
    if (regions.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "No VMA regions found for %{public}s", filename);
        return false;
    }
    // 处理 VMA 列表（与多文件版本共享逻辑）
    int successCount = ProcessVmaRegions(regions);
    TAG_LOGI(AAFwkTag::UIABILITY, "MadviseGeneralFile completed for %{public}s: successCount=%{public}d",
        filename, successCount);
    return (successCount > 0);
}

/**
 * @brief 通用 madvise 接口 - 多个文件版本
 *
 * @param filenames 目标文件名列表
 * @return int 成功应用 madvise 的段数量
 *
 * @note 使用 GetFileVmas 一次获取所有文件的 VMA，然后统一处理
 */
int MadviseGeneralFiles(const std::vector<std::string>& filenames)
{
    if (filenames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Filename list is empty");
        return 0;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "MadviseGeneralFiles called for %{public}zu files", filenames.size());
    // 使用 GetFileVmas 一次获取所有文件的 VMA（避免多次读取 /proc/self/maps）
    std::vector<OHOS::AbilityRuntime::VmaUtil::VMARegion> regions = OHOS::AbilityRuntime::VmaUtil::GetFileVmas(filenames);
    if (regions.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "No VMA regions found for any of the %{public}zu files", filenames.size());
        return 0;
    }
    TAG_LOGI(AAFwkTag::UIABILITY, "Found %{public}zu total VMA regions", regions.size());
    // 处理 VMA 列表（与单文件版本共享逻辑）
    int successCount = ProcessVmaRegions(regions);
    TAG_LOGI(AAFwkTag::UIABILITY, "MadviseGeneralFiles completed: %{public}d segments optimized", successCount);
    return successCount;
}

int MadviseWithConfigFile(const char* packageName)
{
    if (!packageName || strlen(packageName) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid package name: null or empty");
        return -1;
    }
    // 加载配置文件
    MadviseConfig config = LoadConfigFromFile();
    if (!config.IsValid()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to load valid config from: %{public}s", MADVISE_CONFIG_FILE_PATH);
        return -1;
    }
    // 应用配置
    return ApplyMadviseWithConfig(packageName, config);
}

/**
 * @brief 文件页面统计结构
 */
struct FilePageStats {
    std::string filename;
    size_t pageCount;

    bool operator>(const FilePageStats& other) const
    {
        return pageCount > other.pageCount;
    }
};

/**
 * @brief 检查文件是否以 .so 结尾
 */
static inline bool IsSoFile(const char* filename)
{
    if (!filename) {
        return false;
    }
    size_t len = strlen(filename);
    return len > SO_SUFFIX_LEN && strcmp(filename + len - SO_SUFFIX_LEN, ".so") == 0;
}

/**
 * @brief 从 /proc/self/maps 读取并统计文件页面占用
 */
static std::vector<FilePageStats> CollectFilePageStats()
{
    std::vector<FilePageStats> stats;
    FILE* fp = fopen("/proc/self/maps", "r");
    if (!fp) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to open /proc/self/maps");
        return stats;
    }
    char line[LINE_BUFFER_SIZE];
    size_t pageSize = getpagesize();
    while (fgets(line, sizeof(line), fp) != nullptr) {
        unsigned long start = 0;
        unsigned long end = 0;
        char perms[5] = {0};
        unsigned long offset = 0;
        unsigned long inode = 0;
        char pathname[PATH_BUFFER_SIZE] = {0};
        // 解析行: start-end perms offset dev inode pathname
        int parsed = sscanf_s(line, "%lx-%lx %4s %lx %*x:%*x %lu %255s",
            &start, &end, perms, sizeof(perms), &offset, &inode, pathname, PATH_BUFFER_SIZE);
        // 只处理有文件路径的行，且 pathname 不为空
        if (parsed >= PARSED_MIN_FIELDS && strlen(pathname) > 0 && pathname[0] == '/') {
            // 跳过特殊文件和已删除文件
            if (strstr(pathname, "[heap]") || strstr(pathname, "[stack]") ||
                strstr(pathname, "[vvar]") || strstr(pathname, "[vdso]") ||
                strstr(pathname, "(deleted)")) {
                continue;
            }
            // 只统计可读但不可写的段（代码段、只读数据段）
            bool isReadable = (perms[PERMS_INDEX_0] == 'r');
            bool isWritable = (perms[PERMS_INDEX_1] == 'w');
            if (!isReadable || isWritable) {
                continue;
            }
            size_t size = end - start;
            if (pageSize == 0) {
                TAG_LOGE(AAFwkTag::ABILITY, "Page size is zero");
                continue;
            }
            size_t pages = (size + pageSize - 1) / pageSize;
            // 查找是否已存在该文件
            bool found = false;
            for (auto& stat : stats) {
                if (stat.filename == pathname) {
                    stat.pageCount += pages;
                    found = true;
                    break;
                }
            }
            if (!found) {
                stats.push_back({pathname, pages});
            }
        }
    }
    if (fclose(fp) != 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to close file");
    }
    return stats;
}

int MadviseTopFiles()
{
    auto startTime = std::chrono::high_resolution_clock::now();
    // 收集文件页面统计
    std::vector<FilePageStats> stats = CollectFilePageStats();
    if (stats.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "No file stats collected");
        return 0;
    }
    // 按页面数降序排序
    std::sort(stats.begin(), stats.end(), std::greater<FilePageStats>());
    // 取前 N 个
    size_t topN = (stats.size() < TOP_FILE_COUNT) ? stats.size() : TOP_FILE_COUNT;
    std::vector<std::string> topSoFiles;
    std::vector<std::string> topVmaFiles;
    for (size_t i = 0; i < topN; i++) {
        const std::string& filename = stats[i].filename;
        TAG_LOGI(AAFwkTag::ABILITY, "Top %{public}zu: %{public}s (%{public}zu pages)",
            i + 1, filename.c_str(), stats[i].pageCount);
        // 根据文件类型分类
        if (IsSoFile(filename.c_str())) {
            topSoFiles.push_back(filename);
        } else {
            topVmaFiles.push_back(filename);
        }
    }
    // 使用 ELF 方法处理 .so 文件
    int soSuccess = 0;
    if (!topSoFiles.empty()) {
        for (const auto& soFile : topSoFiles) {
            // 获取文件名（不含路径）
            const char* baseName = strrchr(soFile.c_str(), '/');
            const char* libName = baseName ? baseName + 1 : soFile.c_str();
            if (MadviseSingleLibrary(libName)) {
                soSuccess++;
            }
        }
    }
    // 使用 VMA 方法处理其他文件
    int vmaSuccess = MadviseGeneralFiles(topVmaFiles);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    TAG_LOGI(AAFwkTag::ABILITY,
        "Madvise top files: %{public}d .so files, %{public}d other files optimized, time cost: %{public}lld ms",
        soSuccess, vmaSuccess, static_cast<long long>(duration.count()));
    return soSuccess + vmaSuccess;
}

std::string GetConfigPath()
{
    char buf[MAX_PATH_LEN] = { 0 };
    char *configPath = GetOneCfgFile(MADVISE_CONFIG_FILE_PATH, buf, MAX_PATH_LEN);
    if (configPath == nullptr || configPath[0] == '\0' || strlen(configPath) > MAX_PATH_LEN) {
        return MADVISE_CONFIG_DEFAULT_FILE_PATH;
    }
    return configPath;
}

} // namespace MadviseUtil
} // namespace AbilityRuntime
} // namespace OHOS
