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

## 内存释放配置文件

内存释放配置文件是一个 JSON 格式的配置文件，用于定义需要进行 madvise 内存优化的应用白名单及其依赖库。

### 配置文件结构

```json
{
  "apps": [
    {
      "package_name": "com.alipay.mobile.client",
      "libraries": [
        {
          "name": "entry.hap",
          "type": "vma",
          "hot_page_offsets": []
        },
        {
          "name": "libALIPAYKMP.so",
          "type": "elf",
          "hot_page_offsets": []
        }
      ]
    }
  ]
}
```

### 字段说明

#### 顶层字段

- **apps** (array): 应用配置数组，包含所有需要进行内存优化的应用白名单
  - 用途：定义哪些应用在推后台时需要进行 madvise 内存释放
  - 注意：只有在此列表中的应用才会执行内存优化

#### 应用级别字段

- **package_name** (string): 应用包名
  - 用途：唯一标识一个应用
  - 匹配规则：系统会检查当前应用的包名是否与此字段匹配
  - 示例：`"com.alipay.mobile.client"`、`"com.example.myapplication"`

- **libraries** (array): 需要进行内存释放的库文件列表
  - 用途：定义该应用中哪些库文件需要优化
  - 支持的文件类型：`.hap`、`.so` 等所有文件

#### 库级别字段

- **name** (string): 库文件名称
  - 用途：指定需要优化的库文件
  - 匹配规则：使用 basename 匹配（不包含路径），大小写敏感
  - 示例：`"entry.hap"`、`"libALIPAYKMP.so"`、`"libc++.so"`

- **type** (string): 库文件类型，决定使用哪种优化接口
  - **可选值**：
    - `"vma"`: 使用通用接口（基于 `/proc/self/maps`）
      - 支持所有文件类型（`.hap`、`.so` 等）
      - 使用 `madviseUtil::madvise_general_file()` 接口
      - 性能：~0.5-1ms
    - `"elf"`: 使用 ELF 专用接口（基于 `dl_iterate_phdr`）
      - 仅支持 ELF 格式的 `.so` 文件
      - 使用 `madviseUtil::madvise_single_library()` 接口
      - 性能最优：~0.1-0.3ms
  - **选择建议**：
    - `.hap` 文件必须使用 `"vma"`
    - 标准 `.so` 动态库推荐使用 `"elf"`（性能更好）
    - 其他特殊情况使用 `"vma"`

- **hot_page_offsets** (array): 热页偏移量数组（预留字段，当前版本未使用）
  - 用途：指定不需要释放的热页（频繁访问的页面）偏移量
  - 格式：数组中的每个元素为十进制或十六进制数字
  - 示例：`[0x1000, 0x2000, 8192]`
  - 注意：当前版本此字段为空数组 `[]`，表示释放所有符合条件的内存页
  - 未来版本：支持配置热页以避免频繁访问的页面被释放导致性能下降

### 配置示例

#### 示例 1：优化 HAP 应用的主模块

```json
{
  "apps": [
    {
      "package_name": "com.example.myapp",
      "libraries": [
        {
          "name": "entry.hap",
          "type": "vma",
          "hot_page_offsets": []
        }
      ]
    }
  ]
}
```

#### 示例 2：优化大型 SO 库

```json
{
  "apps": [
    {
      "package_name": "com.example.game",
      "libraries": [
        {
          "name": "libgame.so",
          "type": "elf",
          "hot_page_offsets": []
        },
        {
          "name": "libvulkan.so",
          "type": "elf",
          "hot_page_offsets": []
        }
      ]
    }
  ]
}
```

#### 示例 3：混合优化 HAP 和 SO 文件

```json
{
  "apps": [
    {
      "package_name": "com.alipay.mobile.client",
      "libraries": [
        {
          "name": "entry.hap",
          "type": "vma",
          "hot_page_offsets": []
        },
        {
          "name": "libALIPAYKMP.so",
          "type": "elf",
          "hot_page_offsets": []
        },
        {
          "name": "libmyv8.so",
          "type": "elf",
          "hot_page_offsets": []
        }
      ]
    }
  ]
}
```

#### 示例 4：多应用配置

```json
{
  "apps": [
    {
      "package_name": "com.example.app1",
      "libraries": [
        {
          "name": "entry.hap",
          "type": "vma",
          "hot_page_offsets": []
        }
      ]
    },
    {
      "package_name": "com.example.app2",
      "libraries": [
        {
          "name": "libbiglib.so",
          "type": "elf",
          "hot_page_offsets": []
        }
      ]
    }
  ]
}
```

### 配置文件使用场景

#### 典型使用流程

1. **应用推后台**：系统检测到应用切换到后台
2. **包名匹配**：检查应用包名是否在配置文件的 `apps` 列表中
3. **库文件匹配**：如果包名匹配，则根据配置的 `libraries` 列表进行优化
4. **执行优化**：
   - `type: "elf"` → 调用 `madviseUtil::madvise_single_library()`
   - `type: "vma"` → 调用 `madviseUtil::madvise_general_file()`
5. **内存释放**：对库的只读段和可执行段应用 `MADV_DONTNEED`

### 使用建议

#### 1. 选择合适的 type

| 文件类型 | 推荐类型 | 原因 |
|---------|---------|------|
| `.hap` 文件 | `"vma"` | 必须使用 vma（非 ELF 格式） |
| 标准 `.so` 库 | `"elf"` | 性能最优（~0.1-0.3ms） |
| 特殊 `.so` 库 | `"vma"` | 兼容性更好 |

#### 2. 优化库的选择原则

**优先优化的库**：
- 大型 SO 库（> 5MB）
- V8 引擎、图形库等占用内存较大的库
- 不频繁访问的代码库

**不建议优化的库**：
- 核心系统库（`libc.so`、`libhilog.so` 等）
- 极小的库（< 100KB）
- 频繁访问的热点库

#### 3. 性能影响

- **首次访问**：优化后的页面首次访问会触发缺页中断，可能有轻微延迟（~微秒级）
- **整体收益**：应用在后台时物理内存占用显著降低（通常可节省 30-60% 的只读段内存）
- **适用场景**：内存受限设备、多任务切换频繁的场景

### 配置文件解析示例

虽然当前代码库中未包含配置文件解析器，但以下是一个通用的解析示例：

```cpp
#include <fstream>
#include <nlohmann/json.hpp>

struct LibraryConfig {
    std::string name;
    std::string type;
    std::vector<unsigned long> hot_page_offsets;
};

struct AppConfig {
    std::string package_name;
    std::vector<LibraryConfig> libraries;
};

struct MadviseConfig {
    std::vector<AppConfig> apps;
};

// 从 JSON 文件加载配置
MadviseConfig LoadConfig(const std::string& config_path) {
    std::ifstream file(config_path);
    nlohmann::json j;
    file >> j;

    MadviseConfig config = j.get<MadviseConfig>();
    return config;
}

// 使用配置进行优化
void ApplyMadviseForApp(const std::string& package_name, const MadviseConfig& config) {
    // 查找匹配的应用配置
    for (const auto& app : config.apps) {
        if (app.package_name == package_name) {
            for (const auto& lib : app.libraries) {
                if (lib.type == "elf") {
                    madviseUtil::madvise_single_library(lib.name.c_str());
                } else if (lib.type == "vma") {
                    madviseUtil::madvise_general_file(lib.name.c_str());
                }
            }
            break;
        }
    }
}
```

### 注意事项

1. **文件路径**：配置文件通常位于 `/etc/madvise_config.json` 或应用私有目录
2. **包名匹配**：必须与实际应用包名完全匹配
3. **库名匹配**：使用 basename 匹配，不包含路径，大小写敏感
4. **热页配置**：`hot_page_offsets` 为预留字段，当前版本应设置为空数组
5. **性能影响**：首次访问被释放的页面会有轻微的缺页中断开销
6. **线程安全**：如需多线程访问配置，请确保加锁保护

## 反馈与支持

如有问题或建议，欢迎反馈！
