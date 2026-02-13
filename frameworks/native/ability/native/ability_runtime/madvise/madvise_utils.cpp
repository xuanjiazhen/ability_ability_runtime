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
#include "vma_utils.h"
#include "hilog_tag_wrapper.h"
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

namespace {
constexpr const char* MADVISE_CONFIG_DEFAULT_FILE_PATH = "/system/etc/ams_madvise_config.json";
constexpr const char* MADVISE_CONFIG_FILE_PATH = "/etc/ams_madvise_config.json";

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
    std::vector<unsigned long> hot_page_offsets;  // 预留字段，当前未使用
};

/**
 * @brief 应用配置
 */
struct AppConfig {
    std::string package_name;
    std::vector<LibraryConfig> libraries;
};

/**
 * @brief Madvise 配置
 */
struct MadviseConfig {
    std::vector<AppConfig> apps;

    bool IsValid() const {
        return !apps.empty();
    }

    const AppConfig* FindApp(const char* package_name) const {
        if (!package_name || strlen(package_name) == 0) {
            return nullptr;
        }
        for (const auto& app : apps) {
            if (app.package_name == package_name) {
                return &app;
            }
        }
        return nullptr;
    }
};

/**
 * @brief 字符串转 LibraryType
 */
LibraryType ParseLibraryType(const std::string& typeStr) {
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
    const auto config_path = get_config_path();

    // 读取文件内容
    FILE* file = fopen(config_path.c_str(), "r");
    if (!file) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to open config file: %{public}s", config_path.c_str());
        return config;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Config file is empty: %{public}s", config_path.c_str());
        fclose(file);
        return config;
    }

    // 读取文件内容
    std::string content;
    content.resize(file_size);
    size_t read_size = fread(&content[0], 1, file_size, file);
    fclose(file);

    if (read_size != static_cast<size_t>(file_size)) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to read config file: %{public}s", config_path.c_str());
        return config;
    }

    // 使用 cJSON 解析 JSON
    cJSON* root = cJSON_Parse(content.c_str());
    if (!root) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to parse JSON config: %{public}s", config_path.c_str());
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
            appConfig.package_name = packageNameItem->valuestring;
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
        if (!appConfig.package_name.empty() && !appConfig.libraries.empty()) {
            config.apps.push_back(appConfig);
        }
    }

    cJSON_Delete(root);

    TAG_LOGI(AAFwkTag::UIABILITY, "Loaded madvise config from %{public}s: %{public}zu apps",
        config_path.c_str(), config.apps.size());

    return config;
}

/**
 * @brief 使用配置对指定应用应用 madvise 优化
 */
int ApplyMadviseWithConfig(const char* package_name, const MadviseConfig& config)
{
    if (!package_name || strlen(package_name) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid package name: null or empty");
        return 0;
    }

    // 查找匹配的应用配置
    const AppConfig* appConfig = config.FindApp(package_name);
    if (!appConfig) {
        TAG_LOGI(AAFwkTag::UIABILITY, "No config found for package: %{public}s", package_name);
        return 0;
    }

    TAG_LOGI(AAFwkTag::UIABILITY, "Applying madvise for package: %{public}s, libraries: %{public}zu",
        package_name, appConfig->libraries.size());

    int success_count = 0;

    // 遍历库列表进行优化
    for (const auto& libConfig : appConfig->libraries) {
        bool libSuccess = false;

        if (libConfig.type == LibraryType::ELF) {
            // 使用 ELF 专用接口
            TAG_LOGI(AAFwkTag::UIABILITY, "Applying ELF madvise for: %{public}s", libConfig.name.c_str());
            libSuccess = madviseUtil::madvise_single_library(libConfig.name.c_str());
        } else if (libConfig.type == LibraryType::VMA) {
            // 使用通用接口
            TAG_LOGI(AAFwkTag::UIABILITY, "Applying VMA madvise for: %{public}s", libConfig.name.c_str());
            libSuccess = madviseUtil::madvise_general_file(libConfig.name.c_str());
        } else {
            TAG_LOGW(AAFwkTag::ABILITY, "Unknown library type for: %{public}s", libConfig.name.c_str());
            continue;
        }

        if (libSuccess) {
            success_count++;
        }
    }

    TAG_LOGI(AAFwkTag::UIABILITY, "Madvise completed for package %{public}s: %{public}d/%{public}zu libraries succeeded",
        package_name, success_count, appConfig->libraries.size());

    return success_count;
}

} // anonymous namespace

// madvise 回调函数的数据结构（单个库）
struct MadviseData {
    int success_count;
    int fail_count;
    const char* target_lib_name;
};

// madvise 回调函数的数据结构（多个库）
struct MadviseDataMultiple {
    int success_count;  // 成功优化的库数量
    int fail_count;     // 失败的库数量
    std::unordered_set<std::string>* target_lib_names;  // 目标库名称集合
    std::unordered_set<std::string> processed_libs;      // 已处理的库（避免重复计数）
};

/**
 * @brief dl_iterate_phdr 回调函数，对目标库应用 madvise
 */
static int madvise_phdr_callback(struct dl_phdr_info *info, size_t size, void *data)
{
    MadviseData* madvise_data = static_cast<MadviseData*>(data);

    // 获取当前库名
    const char* current_lib_name = info->dlpi_name;
    if (!current_lib_name || strlen(current_lib_name) == 0) {
        current_lib_name = "unknown";  // 处理主程序无名的情况
    }

    // 检查是否是目标库（使用模糊匹配）
    if (strstr(current_lib_name, madvise_data->target_lib_name) == nullptr) {
        return 0;  // 不是目标库，跳过
    }

    TAG_LOGI(AAFwkTag::UIABILITY, "Found target library: %{public}s", current_lib_name);

    // 遍历所有程序头
    for (int i = 0; i < info->dlpi_phnum; i++) {
        const ElfW(Phdr) *phdr = &info->dlpi_phdr[i];

        // 只处理 LOAD 段(实际加载到内存的段)
        if (phdr->p_type != PT_LOAD) {
            continue;
        }

        // 检查段权限
        bool is_readable = (phdr->p_flags & PF_R);
        bool is_writable = (phdr->p_flags & PF_W);

        // 跳过可写段
        if (!is_readable || is_writable) {
            continue;
        }

        void* start_addr = reinterpret_cast<void*>(info->dlpi_addr + phdr->p_vaddr);
        size_t len = phdr->p_memsz;

        // 对齐到页边界
        size_t page_size = getpagesize();
        void* aligned_start = reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(start_addr) & ~(page_size - 1)
        );
        size_t aligned_len = (reinterpret_cast<uintptr_t>(start_addr) + len +
                            page_size - 1) & ~(page_size - 1);
        aligned_len -= reinterpret_cast<uintptr_t>(aligned_start);

        // 获取段权限描述
        char perms[4] = "---";
        if (phdr->p_flags & PF_R) perms[0] = 'r';
        if (phdr->p_flags & PF_W) perms[1] = 'w';
        if (phdr->p_flags & PF_X) perms[2] = 'x';
        perms[3] = '\0';

        TAG_LOGI(AAFwkTag::UIABILITY, "madvise: lib=%{public}s, perms=%{public}s, addr=%{public}p, len=%{public}zu, aligned_len=%{public}zu",
            info->dlpi_name ? info->dlpi_name : "unknown",
            perms,
            start_addr,
            len,
            aligned_len);

        // 调用 madvise，建议内核这些内存不需要
        int result = madvise(aligned_start, aligned_len, MADV_DONTNEED);

        if (result == 0) {
            TAG_LOGI(AAFwkTag::UIABILITY, "madvise success: addr=%{public}p, len=%{public}zu", aligned_start, aligned_len);
            madvise_data->success_count++;
        } else {
            TAG_LOGE(AAFwkTag::ABILITY, "madvise failed: addr=%{public}p, len=%{public}zu, errno=%{public}d",
                aligned_start, aligned_len, errno);
            madvise_data->fail_count++;
        }
    }

    return 0;
}

/**
 * @brief dl_iterate_phdr 回调函数，批量处理多个目标库
 */
static int madvise_phdr_callback_multiple(struct dl_phdr_info *info, size_t size, void *data)
{
    MadviseDataMultiple* madvise_data = static_cast<MadviseDataMultiple*>(data);

    // 获取当前库名
    const char* current_lib_name = info->dlpi_name;
    if (!current_lib_name || strlen(current_lib_name) == 0) {
        return 0;  // 跳过主程序
    }

    // 提取基名称（去掉路径）
    const char* base_name = strrchr(current_lib_name, '/');
    base_name = (base_name == nullptr) ? current_lib_name : (base_name + 1);

    // 检查是否在目标库列表中
    if (madvise_data->target_lib_names->find(base_name) == madvise_data->target_lib_names->end()) {
        return 0;  // 不是目标库，跳过
    }

    // 检查是否已经处理过
    if (madvise_data->processed_libs.find(base_name) != madvise_data->processed_libs.end()) {
        return 0;  // 已处理过，跳过
    }

    // 标记为已处理
    madvise_data->processed_libs.insert(base_name);

    TAG_LOGI(AAFwkTag::UIABILITY, "Processing target library: %{public}s", current_lib_name);

    int lib_success_count = 0;
    int lib_fail_count = 0;

    // 遍历所有程序头
    for (int i = 0; i < info->dlpi_phnum; i++) {
        const ElfW(Phdr) *phdr = &info->dlpi_phdr[i];

        // 只处理 LOAD 段(实际加载到内存的段)
        if (phdr->p_type != PT_LOAD) {
            continue;
        }

        // 检查段权限
        bool is_readable = (phdr->p_flags & PF_R);
        bool is_writable = (phdr->p_flags & PF_W);

        // 跳过可写段
        if (!is_readable || is_writable) {
            continue;
        }

        void* start_addr = reinterpret_cast<void*>(info->dlpi_addr + phdr->p_vaddr);
        size_t len = phdr->p_memsz;

        // 对齐到页边界
        size_t page_size = getpagesize();
        void* aligned_start = reinterpret_cast<void*>(
            reinterpret_cast<uintptr_t>(start_addr) & ~(page_size - 1)
        );
        size_t aligned_len = (reinterpret_cast<uintptr_t>(start_addr) + len +
                            page_size - 1) & ~(page_size - 1);
        aligned_len -= reinterpret_cast<uintptr_t>(aligned_start);

        // 获取段权限描述
        char perms[4] = "---";
        if (phdr->p_flags & PF_R) perms[0] = 'r';
        if (phdr->p_flags & PF_W) perms[1] = 'w';
        if (phdr->p_flags & PF_X) perms[2] = 'x';
        perms[3] = '\0';

        TAG_LOGI(AAFwkTag::UIABILITY, "madvise: lib=%{public}s, perms=%{public}s, addr=%{public}p, len=%{public}zu, aligned_len=%{public}zu",
            info->dlpi_name ? info->dlpi_name : "unknown",
            perms,
            start_addr,
            len,
            aligned_len);

        // 调用 madvise，建议内核这些内存不需要
        int result = madvise(aligned_start, aligned_len, MADV_DONTNEED);

        if (result == 0) {
            TAG_LOGI(AAFwkTag::UIABILITY, "madvise success: addr=%{public}p, len=%{public}zu", aligned_start, aligned_len);
            lib_success_count++;
        } else {
            TAG_LOGE(AAFwkTag::ABILITY, "madvise failed: addr=%{public}p, len=%{public}zu, errno=%{public}d",
                aligned_start, aligned_len, errno);
            lib_fail_count++;
        }
    }

    // 如果至少有一个段成功，则该库优化成功
    if (lib_success_count > 0) {
        madvise_data->success_count++;
        TAG_LOGI(AAFwkTag::UIABILITY, "Library %{public}s optimization completed: %{public}d segments succeeded",
            base_name, lib_success_count);
    } else {
        madvise_data->fail_count++;
        TAG_LOGE(AAFwkTag::ABILITY, "Library %{public}s optimization failed", base_name);
    }

    return 0;
}

namespace madviseUtil {

bool madvise_single_library(const char* lib_name)
{
    if (!lib_name || strlen(lib_name) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid library name: empty string");
        return false;
    }

    TAG_LOGI(AAFwkTag::UIABILITY, "madvise_single_library called for library=%{public}s", lib_name);

    // 初始化数据结构
    MadviseData madvise_data = {0, 0, lib_name};

    // 使用 dl_iterate_phdr 遍历所有加载的段并对目标库调用 madvise
    dl_iterate_phdr(madvise_phdr_callback, &madvise_data);

    // 打印统计结果
    TAG_LOGI(AAFwkTag::UIABILITY, "madvise completed for lib=%{public}s: success_count=%{public}d, fail_count=%{public}d",
        lib_name, madvise_data.success_count, madvise_data.fail_count);

    // 返回是否至少有一个段成功应用了 madvise
    return (madvise_data.success_count > 0);
}

int madvise_multiple_libraries(const std::vector<std::string>& lib_names)
{
    if (lib_names.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Library name list is empty");
        return 0;
    }

    TAG_LOGI(AAFwkTag::UIABILITY, "madvise_multiple_libraries called for %{public}zu libraries", lib_names.size());

    // 创建目标库集合
    std::unordered_set<std::string> target_lib_set(lib_names.begin(), lib_names.end());

    // 初始化数据结构
    MadviseDataMultiple madvise_data = {0, 0, &target_lib_set, {}};

    // 使用 dl_iterate_phdr 一次遍历所有加载的库
    dl_iterate_phdr(madvise_phdr_callback_multiple, &madvise_data);

    // 打印统计结果
    TAG_LOGI(AAFwkTag::UIABILITY, "madvise_multiple_libraries completed: %{public}d succeeded, %{public}d failed",
        madvise_data.success_count, madvise_data.fail_count);

    return madvise_data.success_count;
}

/**
 * @brief 内部辅助函数：对单个 VMA 段应用 madvise
 *
 * @param region VMA 区域
 * @param page_size 页大小
 * @param filename 文件名（用于日志）
 * @return int 0=失败/跳过，1=成功
 */
static int apply_madvise_to_region(const vmaUtil::VMARegion& region, size_t page_size, const char* filename)
{
    // 检查权限：只优化只读段(r--)和可执行段(r-x)
    bool is_readonly = (strstr(region.perms, "r--") != nullptr);
    bool is_executable = (strstr(region.perms, "r-x") != nullptr);

    // 跳过可写段和其他不需要优化的段
    if (!is_readonly && !is_executable) {
        return 0;
    }

    // 计算对齐后的地址和长度
    void* start_addr = reinterpret_cast<void*>(region.start);
    void* aligned_start = reinterpret_cast<void*>(
        reinterpret_cast<uintptr_t>(start_addr) & ~(page_size - 1)
    );
    void* end_addr = reinterpret_cast<void*>(region.end);
    size_t aligned_len = (reinterpret_cast<uintptr_t>(end_addr) + page_size - 1) & ~(page_size - 1);
    aligned_len -= reinterpret_cast<uintptr_t>(aligned_start);

    TAG_LOGI(AAFwkTag::UIABILITY, "madvise: file=%{public}s, perms=%{public}s, addr=%{public}p, len=%{public}zu, aligned_len=%{public}zu",
        filename ? filename : "unknown",
        region.perms,
        start_addr,
        region.size,
        aligned_len);

    // 调用 madvise，建议内核这些内存不需要
    int result = madvise(aligned_start, aligned_len, MADV_DONTNEED);

    if (result == 0) {
        TAG_LOGI(AAFwkTag::UIABILITY, "madvise success: addr=%{public}p, len=%{public}zu", aligned_start, aligned_len);
        return 1;
    } else {
        TAG_LOGE(AAFwkTag::ABILITY, "madvise failed: addr=%{public}p, len=%{public}zu, errno=%{public}d",
            aligned_start, aligned_len, errno);
        return 0;
    }
}

/**
 * @brief 内部辅助函数：处理 VMA 列表（单文件和多文件版本共享）
 *
 * @param regions VMA 区域列表
 * @return int 成功应用 madvise 的段数量
 */
static int process_vma_regions(const std::vector<vmaUtil::VMARegion>& regions)
{
    if (regions.empty()) {
        return 0;
    }

    int success_count = 0;
    size_t page_size = getpagesize();

    // 遍历所有 VMA 段，应用 madvise
    for (const auto& region : regions) {
        success_count += apply_madvise_to_region(region, page_size, region.pathname);
    }

    return success_count;
}

/**
 * @brief 通用 madvise 接口 - 单个文件版本（支持 .hap、.so 等所有文件）
 *
 * @param filename 目标文件名
 * @return bool 是否至少有一个段成功应用了 madvise
 *
 * @note 使用 get_file_vmas 从 /proc/self/maps 获取 VMA，然后应用 madvise
 */
bool madvise_general_file(const char* filename)
{
    if (!filename || strlen(filename) == 0) {
        TAG_LOGE(AAFwkTag::ABILITY, "Invalid filename: empty string");
        return false;
    }

    TAG_LOGI(AAFwkTag::UIABILITY, "madvise_general_file called for file=%{public}s", filename);

    // 使用 get_file_vmas 获取所有匹配的 VMA 段
    std::vector<vmaUtil::VMARegion> regions = vmaUtil::get_file_vmas(filename);

    if (regions.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "No VMA regions found for %{public}s", filename);
        return false;
    }

    // 处理 VMA 列表（与多文件版本共享逻辑）
    int success_count = process_vma_regions(regions);

    TAG_LOGI(AAFwkTag::UIABILITY, "madvise_general_file completed for %{public}s: success_count=%{public}d",
        filename, success_count);

    return (success_count > 0);
}

/**
 * @brief 通用 madvise 接口 - 多个文件版本
 *
 * @param filenames 目标文件名列表
 * @return int 成功应用 madvise 的段数量
 *
 * @note 使用 get_file_vmas 一次获取所有文件的 VMA，然后统一处理
 */
int madvise_general_files(const std::vector<std::string>& filenames)
{
    if (filenames.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Filename list is empty");
        return 0;
    }

    TAG_LOGI(AAFwkTag::UIABILITY, "madvise_general_files called for %{public}zu files", filenames.size());

    // 使用 get_file_vmas 一次获取所有文件的 VMA（避免多次读取 /proc/self/maps）
    std::vector<vmaUtil::VMARegion> regions = vmaUtil::get_file_vmas(filenames);

    if (regions.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "No VMA regions found for any of the %{public}zu files", filenames.size());
        return 0;
    }

    TAG_LOGI(AAFwkTag::UIABILITY, "Found %{public}zu total VMA regions", regions.size());

    // 处理 VMA 列表（与单文件版本共享逻辑）
    int success_count = process_vma_regions(regions);

    TAG_LOGI(AAFwkTag::UIABILITY, "madvise_general_files completed: %{public}d segments optimized", success_count);

    return success_count;
}

int madvise_with_config_file(const char* package_name)
{
    if (!package_name || strlen(package_name) == 0) {
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
    return ApplyMadviseWithConfig(package_name, config);
}

/**
 * @brief 文件页面统计结构
 */
struct FilePageStats {
    std::string filename;
    size_t page_count;

    bool operator>(const FilePageStats& other) const {
        return page_count > other.page_count;
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
    return len > 3 && strcmp(filename + len - 3, ".so") == 0;
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

    char line[1024];
    size_t page_size = getpagesize();

    while (fgets(line, sizeof(line), fp)) {
        unsigned long start, end;
        char perms[5];
        unsigned long offset, inode;
        char pathname[256];

        // 解析行: start-end perms offset dev inode pathname
        int parsed = sscanf(line, "%lx-%lx %4s %lx %*x:%*x %lu %255s",
            &start, &end, perms, &offset, &inode, pathname);

        // 只处理有文件路径的行，且 pathname 不为空
        if (parsed >= 6 && strlen(pathname) > 0 && pathname[0] == '/') {
            // 跳过特殊文件和已删除文件
            if (strstr(pathname, "[heap]") || strstr(pathname, "[stack]") ||
                strstr(pathname, "[vvar]") || strstr(pathname, "[vdso]") ||
                strstr(pathname, "(deleted)")) {
                continue;
            }

            // 只统计可读但不可写的段（代码段、只读数据段）
            bool is_readable = (perms[0] == 'r');
            bool is_writable = (perms[1] == 'w');
            if (!is_readable || is_writable) {
                continue;
            }

            size_t size = end - start;
            size_t pages = (size + page_size - 1) / page_size;

            // 查找是否已存在该文件
            bool found = false;
            for (auto& stat : stats) {
                if (stat.filename == pathname) {
                    stat.page_count += pages;
                    found = true;
                    break;
                }
            }

            if (!found) {
                stats.push_back({pathname, pages});
            }
        }
    }

    fclose(fp);
    return stats;
}

int madvise_top_files()
{
    auto start_time = std::chrono::high_resolution_clock::now();

    // 收集文件页面统计
    std::vector<FilePageStats> stats = CollectFilePageStats();
    if (stats.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "No file stats collected");
        return 0;
    }

    // 按页面数降序排序
    std::sort(stats.begin(), stats.end(), std::greater<FilePageStats>());

    // 取前 10 个
    size_t top_n = (stats.size() < 30) ? stats.size() : 30;
    std::vector<std::string> top_so_files;
    std::vector<std::string> top_vma_files;

    for (size_t i = 0; i < top_n; i++) {
        const std::string& filename = stats[i].filename;
        TAG_LOGI(AAFwkTag::ABILITY, "Top %{public}zu: %{public}s (%{public}zu pages)",
            i + 1, filename.c_str(), stats[i].page_count);

        // 根据文件类型分类
        if (IsSoFile(filename.c_str())) {
            top_so_files.push_back(filename);
        } else {
            top_vma_files.push_back(filename);
        }
    }

    // 使用 ELF 方法处理 .so 文件
    int so_success = 0;
    if (!top_so_files.empty()) {
        for (const auto& so_file : top_so_files) {
            // 获取文件名（不含路径）
            const char* basename = strrchr(so_file.c_str(), '/');
            const char* lib_name = basename ? basename + 1 : so_file.c_str();

            if (madvise_single_library(lib_name)) {
                so_success++;
            }
        }
    }

    // 使用 VMA 方法处理其他文件
    int vma_success = madvise_general_files(top_vma_files);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    TAG_LOGI(AAFwkTag::ABILITY, "Madvise top files: %{public}d .so files, %{public}d other files optimized, time cost: %{public}lld ms",
        so_success, vma_success, static_cast<long long>(duration.count()));

    return so_success + vma_success;
}

std::string get_config_path() const
{
    char buf[MAX_PATH_LEN] = { 0 };
    char *configPath = GetOneCfgFile(MADVISE_CONFIG_FILE_PATH, buf, MAX_PATH_LEN);
    if (configPath == nullptr || configPath[0] == '\0' || strlen(configPath) > MAX_PATH_LEN) {
        return MADVISE_CONFIG_DEFAULT_FILE_PATH;
    }
    return configPath;
}
} // namespace madviseUtil
