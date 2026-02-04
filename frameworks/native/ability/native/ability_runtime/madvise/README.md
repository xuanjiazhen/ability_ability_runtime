# Madvise 内存优化工具集

本目录包含高性能的内存优化工具，支持对 ELF 动态库和任意文件（.hap、.so 等）的只读段应用 `MADV_DONTNEED` 优化。

## 文件列表

### 核心工具
- **madvise_utils.h/cpp** - Madvise 优化核心实现
- **vma_utils.h/cpp** - VMA（虚拟内存区域）遍历和分析工具

### 文档
- **README.md** - 本文件

## 功能特性

### 1. madvise 内存优化

**支持两种接口：**

#### A. ELF 库专用接口（基于 dl_iterate_phdr，高性能）
```cpp
namespace madviseUtil {
    // 单个库
    bool madvise_single_library(const char* lib_name);

    // 多个库（批量处理，一次遍历）
    int madvise_multiple_libraries(const std::vector<std::string>& lib_names);
}
```
- 性能最优（~0.1-0.3ms）
- 仅支持 ELF 格式的 .so 文件
- 基于 dl_iterate_phdr 遍历 ELF program headers

#### B. 通用文件接口（基于 /proc/self/maps）
```cpp
namespace madviseUtil {
    // 单个文件
    bool madvise_general_file(const char* filename);

    // 多个文件（一次读取，统一处理）
    int madvise_general_files(const std::vector<std::string>& filenames);
}
```
- 支持所有文件类型（.hap、.so 等）
- 使用 VMA 工具读取 /proc/self/maps
- 一次遍历处理多个文件，性能优秀

**优化原理：**
- 对只读段（r--）和可执行段（r-x）应用 `MADV_DONTNEED`
- 内核释放对应的物理内存页
- 后续访问时自动触发缺页中断重新加载
- 代码功能不受影响，但物理内存占用显著降低

### 2. VMA 虚拟内存分析

**核心 API：**
```cpp
namespace vmaUtil {
    // 单文件
    std::vector<VMARegion> get_file_vmas(const char* filename);

    // 多文件（一次遍历）
    std::vector<VMARegion> get_file_vmas(const std::vector<std::string>& filenames);

    // 获取 PSS（物理内存占用）
    int get_file_vma_pss(const char* filename, unsigned long* pss_kb);
}
```

**VMARegion 结构：**
```cpp
namespace vmaUtil {
    struct VMARegion {
        uintptr_t start;       // 起始地址
        uintptr_t end;         // 结束地址
        uintptr_t size;        // 区域大小（字节）
        char perms[5];         // 权限（如 "r-xp"）
        unsigned long offset;  // 文件偏移量
        unsigned long inode;   // inode 号
        char pathname[256];    // 文件路径
    };
}
```

## 快速开始

### 1. madvise 优化示例

```cpp
#include "madvise/madvise_utils.h"

// 优化单个 SO 文件（使用 ELF 接口）
bool success = madviseUtil::madvise_single_library("libbiglib.so");

// 优化单个 HAP 文件（使用通用接口）
bool success = madviseUtil::madvise_general_file("entry.hap");

// 批量优化多个文件（一次读取 /proc/self/maps）
std::vector<std::string> files = {"entry.hap", "libbiglib.so"};
int count = madviseUtil::madvise_general_files(files);
```

### 2. VMA 分析示例

```cpp
#include "madvise/vma_utils.h"

// 获取文件的所有 VMA
auto regions = vmaUtil::get_file_vmas("libc.so");

// 分析内存分布
size_t exec_size = 0;
size_t readonly_size = 0;

for (const auto& region : regions) {
    if (strstr(region.perms, "r-x")) {
        exec_size += region.size;
    } else if (strstr(region.perms, "r--")) {
        readonly_size += region.size;
    }
}

printf("可执行段: %zu KB\n", exec_size / 1024);
printf("只读段: %zu KB\n", readonly_size / 1024);
```

### 3. 对比优化前后效果

```cpp
#include "madvise/madvise_utils.h"
#include "madvise/vma_utils.h"

// 获取优化前 PSS
unsigned long pss_before = 0;
vmaUtil::get_file_vma_pss("entry.hap", &pss_before);

// 执行优化
madviseUtil::madvise_general_file("entry.hap");

// 获取优化后 PSS
unsigned long pss_after = 0;
vmaUtil::get_file_vma_pss("entry.hap", &pss_after);

// 计算节省的内存
unsigned long saved = pss_before - pss_after;
double percentage = (double)saved / pss_before * 100.0;

printf("节省内存: %lu KB (%.1f%%)\n", saved, percentage);
```

## 性能对比

| 特性 | ELF 接口 | 通用接口 |
|------|---------|----------|
| **数据源** | dl_iterate_phdr | /proc/self/maps |
| **支持文件** | 仅 .so | 所有文件 |
| **性能** | 快（~0.1-0.3ms） | 中等（~0.5-1ms） |
| **批量处理** | O(M) | O(M) |
| **适用场景** | 标准动态库 | HAP 包、SO 库 |

## 架构设计

```
┌─────────────────────────────────────────┐
│  namespace madviseUtil                  │
│  Madvise 公共 API                        │
├─────────────────────────────────────────┤
│  madviseUtil::madvise_single_library()  │  ELF 库专用
│  madviseUtil::madvise_multiple_libraries() │
│  madviseUtil::madvise_general_file()    │  通用接口
│  madviseUtil::madvise_general_files()   │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│  namespace vmaUtil                      │
│  VMA 工具层                              │
├─────────────────────────────────────────┤
│  vmaUtil::get_file_vmas() - 单文件/多文件 │
│  vmaUtil::get_file_vma_pss() - 获取物理内存 │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│  底层实现                                │
├─────────────────────────────────────────┤
│  dl_iterate_phdr (ELF)                  │
│  /proc/self/maps 解析                   │
│  madvise() 系统调用                     │
└─────────────────────────────────────────┘
```


## 编译说明

### CMakeLists.txt 配置
```cmake
add_library(entry SHARED
    napi_init.cpp
    madvise/madvise_utils.cpp
    madvise/vma_utils.cpp
)
```

### Include 路径
```cpp
#include "madvise/madvise_utils.h"
#include "madvise/vma_utils.h"
```

## 注意事项

1. **线程安全**：当前实现不是线程安全的，多线程使用时需要加锁
2. **文件名匹配**：支持 basename 匹配，大小写敏感
3. **性能考虑**：频繁调用建议缓存结果
4. **权限问题**：需要 `/proc/self/{maps|smaps}` 文件系统可读
5. **OpenHarmony 兼容**：VMA 工具已针对 OpenHarmony 优化（使用 fgets 而非 mmap）

## 反馈与支持

如有问题或建议，欢迎反馈！
