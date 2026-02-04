#include "madvise_utils.h"
#include "vma_utils.h"
#include "hilog_tag_wrapper.h"
#include <libgen.h>
#include <cstring>
#include <dlfcn.h>
#include <link.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <unordered_set>
#include <string>
#include <vector>

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

} // namespace madviseUtil
