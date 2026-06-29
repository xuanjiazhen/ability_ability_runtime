# Design — 应用按需释放文件页

> Stage 2 产物。复杂度：标准。状态：**Draft（待 Spec 审批）**。

## 一、架构概览

```
┌─────────────────────────────────────────────────────────────┐
│  应用层 (System App)                                         │
│   ArkTS: appMemory.releaseFilePages(...)                     │
│   JS   : appMemory.releaseFilePages(...)                     │
└───────────────┬─────────────────────────────────────────────┘
                │ (ANI binding / NAPI binding)
┌───────────────▼─────────────────────────────────────────────┐
│  Binding 层                                                  │
│   frameworks/ets/ani/app_memory/   (ANI/ArkTS)               │
│   frameworks/js/napi/app_memory/   (NAPI/JS)                 │
│   - 系统应用身份校验 (CheckCallerIsSystemApp)                 │
│   - 参数序列化 → 调用 Native 引擎                             │
└───────────────┬─────────────────────────────────────────────┘
                │
┌───────────────▼─────────────────────────────────────────────┐
│  Native 引擎层 (新增)                                         │
│   frameworks/native/ability/native/ability_runtime/          │
│     memory/  ← 新增                                          │
│       file_page_releaser.h/cpp                               │
│   - 解析本 Bundle 路径 (经 BundleMgrHelper)                   │
│   - 文件名 → 绝对路径白名单校验                                │
│   - moduleName → hap 路径 + rawfile 顶层清单                 │
│   - 异步执行 (worker / libuv napi_async)                     │
└───────────────┬─────────────────────────────────────────────┘
                │ 复用
┌───────────────▼─────────────────────────────────────────────┐
│  既有 Madvise 工具链                                          │
│   frameworks/native/ability/native/ability_runtime/madvise/  │
│     MadviseUtil::MadviseSingleLibrary   (ELF/so, dl_iterate) │
│     MadviseUtil::MadviseGeneralFiles    (hap/hsp/rawfile,VMA)│
│     VmaUtil::GetFileVmas                (/proc/self/maps)    │
└─────────────────────────────────────────────────────────────┘
```

## 二、关键设计决策 (ADR)

### ADR-1：新增独立 Manager，而非挂 Context
**决策**：新增 `appMemory` 系统模块（NAPI 模块名 `application.appMemory`，ArkTS namespace 同名），暴露两个静态方法。
**取舍**：Context（Application/Ability）语义偏向"上下文访问"，文件页释放属于进程级资源操作，挂独立模块职责更内聚；也避免污染 Context 接口树与 IDL。
**API 形态**：
```
appMemory.releaseFilePages(fileNames: Array<string>): Promise<void>
appMemory.releaseRawFilePages(moduleName: string): Promise<void>
```

### ADR-2：本 Bundle 路径强校验（白名单）
**决策**：调用方只传**文件名/相对名**；Native 引擎经 `BundleMgrHelper::GetBundleInfo` 取本 Bundle 安装根目录，将入参解析为本 Bundle 内的绝对路径；任何解析后路径不在本 Bundle 安装根/模块目录树下 → 拒绝（`ERR_INVALID_VALUE`），不执行释放。
**取舍**：杜绝路径穿越（`../`）、跨 Bundle 越权；不暴露绝对路径入参，缩小攻击面。

### ADR-3：rawfile 处理 — hap 内偏移释放（核心难点）
**背景**：hap/hsp 是 ZIP 归档，rawfile 位于 ZIP 内；进程内 mmap 的对象是 **hap 文件本身**（单一映射区域）。`/proc/self/maps` 中只能看到 hap 名，看不到 rawfile。
**决策（分层）**：
- **方案 A（首选，本期目标）**：解析 hap（ZIP central directory）获取该 moduleName 对应 hap 内 rawfile 顶层条目的 **数据偏移区间**，再结合 `VmaUtil::GetFileVmas([hap 绝对路径])` 得到的 VMA 映射，对命中 rawfile 偏移区间的 VMA 子段调用 `madvise(MADV_DONTNEED)`。
- **方案 B（降级）**：若某些设备 rawfile 被解包到磁盘缓存（`/data/.../cache/rawfile/...`），则对解包后的普通文件直接走 `MadviseGeneralFiles`。
- **探测**：实现期先在目标进程 `hidumper` + 读 `/proc/self/maps` 确认映射形态，确定 A/B 主路径；A 路径需要 ZIP 解析库（minizip/系统 zip），依赖见 ADR-5。
**取舍**：A 方案最精确（只释放 rawfile 占用的页，不影响 hap 内代码/资源），但实现复杂度高、需引入 ZIP 解析。本期接受 A 作为目标，若依赖不可得则降级 B 并在 Spec 明确。

### ADR-4：异步执行模型
**决策**：NAPI 用 `napi_create_async_work`（libuv worker 线程执行 madvise，main thread 回调 Promise）；ANI 用等价的异步任务机制。madvise 本身可能扫 `/proc/self/maps` + 多段系统调用，放 worker 避免阻塞 ArkTS 主线程。
**取舍**：与系统 API 异步惯例一致；不引入新线程池。

### ADR-5：依赖
- `bundle_framework`（已有 external_dep，`BundleMgrHelper`）：路径/module 解析。
- ZIP 解析（ADR-3 方案 A）：评估使用系统既有 zip 工具或 minizip；若 ability_runtime 无现成依赖，**优先查找 OHOS 已有的 zip 工具仓**，避免自实现。若不可得 → 降级 ADR-3 方案 B。
- 既有 `ability_madvise` 共享库：扩展不改其 API。

### ADR-6：非法项与合法项混传语义
**决策**：**全部校验通过才执行；任一项非法 → 整体返回错误，不释放任何项**（原子语义）。
**理由**：释放操作幂等且副作用仅内存，但部分成功会让调用方难以判断实际效果；原子失败更可预测。AC-6 据此细化。

### ADR-7：不加特性开关
**决策**：不加 `ability_runtime.gni` 开关、不加 `persist.sys.*` 参数。能力默认启用。
**理由**：用户澄清明确；接口需系统应用身份，影响面可控。

## 三、模块/文件清单（改动面）

| 路径 | 类型 | 说明 |
|------|------|------|
| `frameworks/native/ability/native/ability_runtime/memory/file_page_releaser.h/cpp` | 新增 | Native 引擎：路径解析、白名单校验、rawfile 偏移解析、调度 madvise |
| `frameworks/native/ability/native/ability_runtime/memory/BUILD.gn` | 新增 | 引擎目标，依赖 ability_madvise + BundleMgrHelper |
| `frameworks/native/ability/native/ability_runtime/madvise/madvise_utils.h/cpp` | 修改 | （按 ADR-3 方案 A）新增按 VMA 子区间释放的重载 / 暴露 mincore 探测 |
| `frameworks/js/napi/app_memory/app_memory_module.cpp` | 新增 | NAPI 模块注册 + `application.appMemory` |
| `frameworks/js/napi/app_memory/js_app_memory.cpp/h` | 新增 | NAPI 实现：系统应用校验 + async work |
| `frameworks/js/napi/app_memory/BUILD.gn` | 新增 | NAPI 目标 |
| `frameworks/ets/ani/app_memory/` | 新增 | ANI/ArkTS 绑定（ets_app_memory.cpp 等）+ BUILD.gn |
| `interfaces/inner_api/error_utils/` | 修改 | 若新增专属错误码（如 `ERR_ABILITY_RUNTIME_INVALID_FILE_PATH`） |
| `test/unittest/file_page_releaser_test/` | 新增 | 单元测试（白名单、路径穿越、偏移解析） |
| `bundle.json` / 各 BUILD.gn | 修改 | 注册新模块目标 |

## 四、数据流（US-1 按文件名释放）

1. App 调 `appMemory.releaseFilePages(["libfoo.so","bar.hsp"])`
2. NAPI/ANI 入口：`CheckCallerIsSystemApp()` 失败 → reject(`ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP`=202)
3. async work（worker 线程）：
   a. `ApplicationContext::GetBundleName()` → 本 bundle
   b. `BundleMgrHelper::GetBundleInfo(bundleName, GET_*)` → 安装根 + 各 hap 路径 + module 列表
   c. 对每个入参：匹配后缀(.so→`MadviseSingleLibrary` 路径；.hap/.hsp→`MadviseGeneralFiles` 路径)；解析后路径必须 `StartsWith(本 Bundle 安装根)`，否则记非法
   d. 全部合法 → 逐项执行 madvise；任一非法 → 整体 abort
4. main thread：resolve()（成功）或 reject(errCode)

## 五、数据流（US-2 按 moduleName 释放 rawfile）

1. App 调 `releaseRawFilePages("entry")`
2-3. 同上系统应用校验 + Bundle 解析，定位 moduleName 对应 hap 绝对路径
4. ADR-3 方案 A：ZIP 解析 hap → 取 rawfile 顶层条目数据偏移区间集合 R；`VmaUtil::GetFileVmas([hap])` 得 VMA 集 V；对 V 中每段与 R 求交，对交集子段 `madvise(MADV_DONTNEED)`
5. resolve/reject

## 六、安全考量

- **路径穿越**：白名单 `StartsWith(本 Bundle 安装根)`，禁止 `../`、绝对路径、符号链接逃逸。
- **越权**：仅本 Bundle；系统应用身份强制校验。
- **可写段保护**：复用既有 `MadviseGeneralFiles` 对 `r--p`/只读非可写段的过滤，不释放可写映射。
- **无新权限申请**：依靠 `system_api` 标记 + SELinux system domain。

## 七、性能影响

- madvise 为 O(映射段数) 系统调用，单次调用通常 < 数毫秒；放 worker 线程，不阻塞 UI。
- 不引入常驻内存开销。
- 首次 ZIP 解析（ADR-3 A）开销与 hap 内 rawfile 数量线性相关，可接受。

## 八、兼容性

- 纯增量 API，不改既有接口与 `MadviseWithConfigFile` 行为。
- FA/Stage 模型均可调用（系统应用）。
- 非系统应用调用立即失败，向后兼容。

## 九、测试策略

- **单元测试**：白名单边界、`../` 穿越拒绝、moduleName 不存在、rawfile 偏移区间解析（mock ZIP）、错误码映射。
- **集成验证（AC 验收）**：系统应用 Demo 调接口 → `hidumper -s 1909 -a "-p <pid>"` 或 `hidumper` smap 对比前后 `libxxx.so`/hap 区域 RSS。
- **Fuzz**：入参字符串 fuzz（已存在 fuzz 测试目录可挂载）。

## 十、开放项（进入 Spec / 实现前需确认）

- **O-1**：ADR-3 方案 A 所需 ZIP 解析依赖在 ability_runtime 是否可得（决定 A/B 主路径）。
- **O-2**：rawfile 在目标设备是否真以 hap 内偏移形式驻留文件页（需 `hidumper` 实测确认映射形态）。
- **O-3**：最终 API 命名（`appMemory` / `memoryManager` / `filePageReleaser`）—— 倾向 `appMemory`。
