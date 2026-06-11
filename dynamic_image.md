# 将 start_window_option 对 image_framework:image_native 的依赖改为 dlopen 按需加载

## Context

当前 `start_window_option` 共享库（libstart_window_option.z.so）在 `ability_runtime_graphics && ability_runtime_start_window_options_with_pixelmap` 开启时静态依赖 `image_framework:image_native`。但实际上 PixelMap 在该库中仅用于两处 Parcel 序列化/反序列化操作，可以拆分为两个 SO，通过 dlopen 延迟加载，实现按需依赖。

代码库中已有成熟的 dlopen 桥接模式：`SubManagersHelper` 通过 `dlopen("libmission_list.z.so")` 动态加载任务管理模块（参见 `services/abilitymgr/src/sub_managers_helper.cpp:610-640`），本次方案复用该模式，并将新文件/库统一放置在 `services/common`。

## 架构

新增两个通用 SO，放在 `services/common`，命名体现"桥接"与"封装"职责：

```
SO #1: image_native_bridge  (libimage_native_bridge.z.so)  ← 给使用方直接调用
  ├── PixelMapBridge 单例类
  ├── 不依赖 image_framework:image_native
  └── dlopen → SO #2

SO #2: image_native_wrap    (libimage_native_wrap.z.so)    ← 被 dlopen
  ├── 依赖 image_framework:image_native
  ├── #include "pixel_map.h"
  └── 导出 extern "C" 函数，封装真正的 PixelMap 操作

使用方（start_window_option）:
  └── 改为依赖 image_native_bridge（替代直接依赖 image_native）
```

调用链：`start_window_option.cpp` → `PixelMapBridge`(SO#1) → dlopen/dlsym → `ImageNative_*` C 函数(SO#2) → `Media::PixelMap`

## 新增文件（3个，均位于 services/common）

### 1. `services/common/include/pixel_map_bridge.h`

桥接库（SO #1）的头文件。声明 `PixelMapBridge` 单例类与函数指针类型。**不 include `pixel_map.h`**，仅对 `OHOS::Parcel` 与 `OHOS::Media::PixelMap` 做前置声明，接口与函数指针使用类型指针（而非 `void*`），既保持桥接库零 image_native 依赖，又保证类型安全。

```cpp
#ifndef OHOS_ABILITY_RUNTIME_PIXEL_MAP_BRIDGE_H
#define OHOS_ABILITY_RUNTIME_PIXEL_MAP_BRIDGE_H

#include <mutex>

namespace OHOS {
class Parcel;
namespace Media {
class PixelMap;
}
namespace AAFwk {

// 与 SO #2 导出 C 函数一一对应的函数指针类型（类型指针，非 void*）
using ReadPixelMapFunc    = Media::PixelMap*(*)(Parcel* parcelPtr);
using WritePixelMapFunc   = bool(*)(Media::PixelMap* pixelMapPtr, Parcel* parcelPtr);
using DestroyPixelMapFunc = void(*)(Media::PixelMap* pixelMapPtr);

// 桥接单例：dlopen libimage_native_wrap.z.so 并缓存函数指针。
// 使用方（如 StartWindowOption）通过它间接访问 PixelMap，自身不链接 image_native。
class PixelMapBridge {
public:
    static PixelMapBridge& GetInstance();
    ~PixelMapBridge();

    // 从 Parcel 读出 PixelMap，所有权转移给调用方。失败返回 nullptr。
    Media::PixelMap* ReadPixelMapFromParcel(Parcel* parcelPtr);
    // 将 PixelMap 写入 Parcel。
    bool WritePixelMapToParcel(Media::PixelMap* pixelMapPtr, Parcel* parcelPtr);
    // 销毁 PixelMap（delete 操作在 SO #2 内执行，确保同一 SO 分配/释放）。
    void DestroyPixelMap(Media::PixelMap* pixelMapPtr);

private:
    PixelMapBridge();
    bool LoadLibrary();
    void UnloadLibrary();

    std::mutex mutex_;
    void* handle_ = nullptr;
    ReadPixelMapFunc readFunc_ = nullptr;
    WritePixelMapFunc writeFunc_ = nullptr;
    DestroyPixelMapFunc destroyFunc_ = nullptr;
    bool loadAttempted_ = false;
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_PIXEL_MAP_BRIDGE_H
```

### 2. `services/common/src/pixel_map_bridge.cpp`

桥接库（SO #1）实现。核心逻辑参照 `SubManagersHelper::GetMissionListWrap()`（`sub_managers_helper.cpp:610-640`）：

```cpp
#include "pixel_map_bridge.h"

#include <dlfcn.h>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* IMAGE_NATIVE_WRAP_PATH = "libimage_native_wrap.z.so";
constexpr const char* SYMBOL_READ    = "ImageNative_ReadPixelMapFromParcel";
constexpr const char* SYMBOL_WRITE   = "ImageNative_WritePixelMapToParcel";
constexpr const char* SYMBOL_DESTROY = "ImageNative_DestroyPixelMap";
}

PixelMapBridge& PixelMapBridge::GetInstance()
{
    static PixelMapBridge instance;
    return instance;
}

PixelMapBridge::PixelMapBridge() = default;

PixelMapBridge::~PixelMapBridge()
{
    UnloadLibrary();
}

void PixelMapBridge::UnloadLibrary()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (handle_ != nullptr) {
        readFunc_ = nullptr;
        writeFunc_ = nullptr;
        destroyFunc_ = nullptr;
        dlclose(handle_);
        handle_ = nullptr;
    }
}

bool PixelMapBridge::LoadLibrary()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (loadAttempted_) {
        return handle_ != nullptr;
    }
    loadAttempted_ = true;

    handle_ = dlopen(IMAGE_NATIVE_WRAP_PATH, RTLD_NOW | RTLD_GLOBAL);
    if (handle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "dlopen %{public}s failed", IMAGE_NATIVE_WRAP_PATH);
        return false;
    }

    readFunc_    = reinterpret_cast<ReadPixelMapFunc>(dlsym(handle_, SYMBOL_READ));
    writeFunc_   = reinterpret_cast<WritePixelMapFunc>(dlsym(handle_, SYMBOL_WRITE));
    destroyFunc_ = reinterpret_cast<DestroyPixelMapFunc>(dlsym(handle_, SYMBOL_DESTROY));
    if (readFunc_ == nullptr || writeFunc_ == nullptr || destroyFunc_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "dlsym pixelmap wrap symbols failed");
        dlclose(handle_);
        handle_ = nullptr;
        return false;
    }
    return true;
}

Media::PixelMap* PixelMapBridge::ReadPixelMapFromParcel(Parcel* parcelPtr)
{
    if (!LoadLibrary()) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return readFunc_(parcelPtr);
}

bool PixelMapBridge::WritePixelMapToParcel(Media::PixelMap* pixelMapPtr, Parcel* parcelPtr)
{
    if (!LoadLibrary()) {
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return writeFunc_(pixelMapPtr, parcelPtr);
}

void PixelMapBridge::DestroyPixelMap(Media::PixelMap* pixelMapPtr)
{
    if (!LoadLibrary()) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    destroyFunc_(pixelMapPtr);
}

}  // namespace AAFwk
}  // namespace OHOS
```

### 3. `services/common/src/pixel_map_wrap.cpp`

封装库（SO #2）实现，直接依赖 image_native。参照 `mission_list_manager_interface.h` 的 `extern "C" __attribute__((visibility("default")))` 导出模式。C 函数签名使用类型指针（`OHOS::Parcel*` / `OHOS::Media::PixelMap*`），与桥接头中的函数指针类型严格一致，`extern "C"` 仅影响链接（去 mangle），不影响参数为 C++ 类指针：

```cpp
#include "parcel.h"
#include "pixel_map.h"

// 从 Parcel 反序列化 PixelMap，所有权转移给调用方。
extern "C" __attribute__((visibility("default")))
OHOS::Media::PixelMap* ImageNative_ReadPixelMapFromParcel(OHOS::Parcel* parcelPtr)
{
    if (parcelPtr == nullptr) {
        return nullptr;
    }
    return parcelPtr->ReadParcelable<OHOS::Media::PixelMap>();
}

// 将 PixelMap 序列化写入 Parcel。
extern "C" __attribute__((visibility("default")))
bool ImageNative_WritePixelMapToParcel(OHOS::Media::PixelMap* pixelMapPtr, OHOS::Parcel* parcelPtr)
{
    if (pixelMapPtr == nullptr || parcelPtr == nullptr) {
        return false;
    }
    return parcelPtr->WriteParcelable(pixelMapPtr);
}

// 销毁 PixelMap。delete 在本 SO（即分配它的 SO）内执行，保证内存配对释放。
extern "C" __attribute__((visibility("default")))
void ImageNative_DestroyPixelMap(OHOS::Media::PixelMap* pixelMapPtr)
{
    if (pixelMapPtr == nullptr) {
        return;
    }
    delete pixelMapPtr;
}
```

## 修改文件（3个）

### 4. `services/common/BUILD.gn`

在文件末尾新增两个 `ohos_shared_library` 目标（沿用 common 目录 sanitize/branch_protector 等惯例）：

```gn
# SO #1: 桥接库，给使用方依赖，内部 dlopen SO #2；不依赖 image_native。
ohos_shared_library("image_native_bridge") {
  sanitize = {
    integer_overflow = true
    ubsan = true
    boundary_sanitize = true
    cfi = true
    cfi_cross_dso = true
    cfi_vcall_icall_only = true
    cfi_policy = "adaptive"
    debug = false
  }
  branch_protector_ret = "pac_ret"

  public_configs = [ ":common_config" ]
  include_dirs = [ "include" ]

  sources = [ "src/pixel_map_bridge.cpp" ]

  external_deps = [
    "c_utils:utils",
    "hilog:libhilog",
  ]

  subsystem_name = "ability"
  part_name = "ability_runtime"
}

# SO #2: 封装库，依赖 image_native，导出 C 方法供 SO #1 dlopen 调用。
ohos_shared_library("image_native_wrap") {
  sanitize = {
    integer_overflow = true
    ubsan = true
    boundary_sanitize = true
    cfi = true
    cfi_cross_dso = true
    cfi_vcall_icall_only = true
    cfi_policy = "adaptive"
    debug = false
  }
  branch_protector_ret = "pac_ret"

  include_dirs = [ "include" ]

  sources = [ "src/pixel_map_wrap.cpp" ]

  external_deps = [
    "c_utils:utils",
    "hilog:libhilog",
    "image_framework:image_native",
    "ipc:ipc_single",
  ]

  subsystem_name = "ability"
  part_name = "ability_runtime"
}
```

> 说明：`common_config` 的 visibility 已覆盖 inner_api/ability_manager，故 `start_window_option` 可依赖 `image_native_bridge` 并拿到头文件路径。

### 5. `interfaces/inner_api/ability_manager/BUILD.gn`

**`start_window_option` 目标（约 379-418 行）的 `if` 块修改：**

修改前：
```gn
if (ability_runtime_graphics &&
    ability_runtime_start_window_options_with_pixelmap) {
  defines = [ "START_WINDOW_OPTIONS_WITH_PIXELMAP" ]
  external_deps += [ "image_framework:image_native" ]
}
```

修改后：
```gn
if (ability_runtime_graphics &&
    ability_runtime_start_window_options_with_pixelmap) {
  defines = [ "START_WINDOW_OPTIONS_WITH_PIXELMAP" ]
  deps = [ "${ability_runtime_services_path}/common:image_native_bridge" ]
}
```

> 关键点：`image_framework:image_native` 从 `external_deps` 移除，改为依赖桥接库 `image_native_bridge`。`start_window_option.cpp` 不再静态链接 image_native。

### 6. `services/abilitymgr/src/start_window_option.cpp`

用桥接替换直接的 PixelMap Parcel 操作（老方式彻底移除，仅保留 dlopen 桥接）。`startWindowIcon` 字段类型（`std::shared_ptr<Media::PixelMap>`）保持不变——头文件中已是前向声明，`shared_ptr` 声明不需要完整类型。

```cpp
#include "start_window_option.h"
#include "hilog_tag_wrapper.h"

#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
#include "pixel_map_bridge.h"
#endif

namespace OHOS {
namespace AAFwk {
bool StartWindowOption::ReadFromParcel(Parcel &parcel)
{
    hasStartWindow = parcel.ReadBool();
    startWindowBackgroundColor = parcel.ReadString();
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    auto &bridge = PixelMapBridge::GetInstance();
    Media::PixelMap *rawPtr = bridge.ReadPixelMapFromParcel(&parcel);
    if (rawPtr != nullptr) {
        // 自定义 deleter：把 delete 路由回 SO #2，确保分配/释放在同一 SO，
        // 维持虚表一致与 CFI 兼容；自身不依赖 PixelMap 完整类型。
        startWindowIcon = std::shared_ptr<Media::PixelMap>(rawPtr,
            [&bridge](Media::PixelMap *p) { bridge.DestroyPixelMap(p); });
    }
#endif
    return true;
}

StartWindowOption *StartWindowOption::Unmarshalling(Parcel &parcel)
{
    // 不变
    ...
}

bool StartWindowOption::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(hasStartWindow)) { return false; }
    if (!parcel.WriteString(startWindowBackgroundColor)) { return false; }
#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP
    auto &bridge = PixelMapBridge::GetInstance();
    if (startWindowIcon != nullptr) {
        if (!bridge.WritePixelMapToParcel(startWindowIcon.get(), &parcel)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "startWindowIcon write failed");
            return false;
        }
    }
#endif
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
```

**关键设计点：** `shared_ptr` 自定义 deleter 捕获 `bridge` 引用，将 `delete` 路由回 SO #2。这保证：
- 内存分配/释放在同一 SO 中配对
- 虚函数表一致性（CFI 兼容）
- 无需修改 `startWindowIcon` 字段类型，所有下游使用方（`start_options`、`mission_ability_record` 等）零改动

## 不在本次改动范围

- `abilityms` / `abilityms_static`：它们对 `image_framework:image_native` 的依赖是**无条件的**（用于 mission 快照等其它 PixelMap 操作，见 `services/abilitymgr/BUILD.gn:265/468/797`），且其源码不直接使用 `START_WINDOW_OPTIONS_WITH_PIXELMAP`（grep 确认仅 start_window_option.cpp 与框架层使用）。故本次不动 abilityms。
- ANI / C / NAPI 框架层（`frameworks/ets|c|js`）：它们各自直接依赖 image_native（通过 `GetPixelMapFromEnvSp` / `GetInnerPixelmap` / `pixelmap_napi`），属独立目标，本次不转换。

## 线程安全

- `PixelMapBridge` 用 `std::mutex` 保护 dlopen/dlsym 与函数指针调用（参照 `SubManagersHelper::missionListWrapMutex_`）
- `loadAttempted_` 标志防止重复 dlopen
- 首次调用惰性加载，后续直接用缓存指针
- `PixelMapBridge` 是函数局部静态单例，生命周期覆盖所有 `StartWindowOption` 实例；`shared_ptr` deleter 持其引用安全

## 降级行为

| 场景 | 行为 |
|------|------|
| feature flag 关闭 | 与现状一致，PixelMap 代码编译排除 |
| flag 开启但 libimage_native_wrap.z.so 缺失 | dlopen 失败：**读** `ReadPixelMapFromParcel` 返回 nullptr → 不设 `startWindowIcon`（静默降级，不崩溃）；**写** `WritePixelMapToParcel` 返回 false → `Marshalling` 返回 false（IPC 失败，日志告警）。注：删除 `IsAvailable()` 后不再"静默跳过写"，以避免收发两端 Parcel 不对称；一致性部署下两端 lib 状态相同，不会触发该分支 |
| flag 开启且 wrap 库存在 | 正常经桥接读写 PixelMap |

## 验证方式

1. **编译验证**：

> ⚠️ 注意：`ability_runtime_start_window_options_with_pixelmap` 在 [ability_runtime.gni](../../ability_runtime.gni) 中默认 `false`，rk3568 等产品未 override。若不加 `--gn-args` 开启该 flag，`start_window_option.cpp` 的桥接代码会被 `#ifdef START_WINDOW_OPTIONS_WITH_PIXELMAP` 整体编译排除，此时 `libstart_window_option.z.so` 中查不到任何 `PixelMapBridge` 符号、也不会 NEEDED `image_native_bridge` —— 即"dlopen 路径未真正参与编译"。因此验证桥接方案时**必须显式开启该 flag**。

```bash
    cd /data/home/w00926586/repo
    # 关键：--gn-args 开启 flag，否则桥接代码被编译排除，无法真正验证
    ./build.sh --product-name rk3568 --ccache \
        --gn-args ability_runtime_start_window_options_with_pixelmap=true \
        --build-target image_native_bridge image_native_wrap start_window_option
```

2. **产物验证（编译链接层）**：构建完成后用 `readelf`/`nm` 确认依赖链与符号：
```bash
    SO=out/rk3568/lib.unstripped/ability/ability_runtime
    # start_window_option 应含 PixelMapBridge 符号（U=运行时从 bridge 解析）
    nm -C $SO/libstart_window_option.z.so | grep PixelMapBridge
    # start_window_option 应 NEEDED image_native_bridge，且不应 NEEDED image_native
    readelf -d $SO/libstart_window_option.z.so | grep NEEDED
    # wrap 库才 NEEDED image_native（image 依赖收敛到此 SO）
    readelf -d $SO/libimage_native_wrap.z.so | grep image_native
```
   预期：`start_window_option` → NEEDED `libimage_native_bridge.z.so`（不含 image_native）；`bridge` 无 image；`wrap` NEEDED `libimage_native.z.so`。

3. **单元测试**：运行已有 `ability_start_window_option_test`，验证 `StartWindowOption` Parcel 序列化/反序列化往返正确（需在设备/模拟器上执行，并确保上述 flag 开启重编）
```bash
    run -t UT -ts ability_start_window_option_test
```
4. **功能验证**：启动带 `startWindowIcon` 的 Ability，验证启动窗口图标正确显示
5. **降级验证**：删除 `libimage_native_wrap.z.so` 后启动，验证不崩溃、日志有告警
