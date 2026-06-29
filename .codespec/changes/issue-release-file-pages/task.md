# Task Spec — file_page_releaser + 绑定

> Stage 3 产物。覆盖 execution-plan T-0~T-7 的实现规格。

## T-0 开放项实测

**目标**：定 ADR-3 A/B 主路径。
**步骤**：
1. 选一个已安装系统应用（如本仓 demo），`cat /proc/self/maps | grep -E '\.hap|\.hsp'`，确认 hap 是否整体 mmap（单一映射区域）。
2. 在 hap 包内放 rawfile，重新加载后查 maps：rawfile 是否以 hap 偏移存在 / 是否解包到 `/data/.../cache/`。
3. 查 `bundle.json` 与 `external_deps`：ability_runtime 是否已依赖 zip/minizip 类库；不可得则记为方案 B。
**产出**：结论写入 review.md；若需修订 AC-3 措辞则同步 spec.md。

## T-1 Native 引擎 file_page_releaser

**文件**：`frameworks/native/ability/native/ability_runtime/memory/file_page_releaser.{h,cpp}` + `BUILD.gn`

**接口（建议）**：
```cpp
namespace OHOS::AbilityRuntime::FilePageReleaser {
// 供 binding 调用；返回 0 成功，非 0 错误码（对齐 error_utils）
int32_t ReleaseByFileNames(const std::string& bundleName,
                           const std::vector<std::string>& fileNames);
int32_t ReleaseRawFileByModule(const std::string& bundleName,
                               const std::string& moduleName);
}
```

**内部职责**：
1. 经 `BundleMgrHelper` 查本 Bundle 安装根 + 各模块 hap 路径 + module 列表。
2. **白名单校验函数** `IsWithinBundle(path, bundleRoot)`：路径标准化后 `StartsWith(bundleRoot)`，拒绝 `../`、绝对非本 bundle 路径、符号链接逃逸（`realpath` 解析后复校）。
3. 入参分类：`.so` → 收集库名；`.hap/.hsp` → 收集 hap 路径；其它 → 标记非法。
4. **原子性**：先全部校验，任一非法直接返回 `401`，不调用任何 madvise。
5. 合法后：`.so` 调 `MadviseSingleLibrary`；hap/rawfile 按 T-0 结论调相应 madvise。
6. 日志：`TAG_LOGI` 成功摘要、`TAG_LOGW` 非法项与权限拒绝、`TAG_LOGE` 系统调用失败（使用 AAFwkLogTag 既有 LABEL）。

## T-2 madvise 工具链扩展

**文件**：`frameworks/native/ability/native/ability_runtime/madvise/madvise_utils.{h,cpp}`、`vma_utils.{h,cpp}`

**按 T-0 结论**：
- 方案 A：新增 `MadviseFilePagesAtOffsets(filePath, spanVector)`，先 `GetFileVmas` 取 VMA，对 VMA 段与给定 `[offsetStart,offsetEnd)` 求交，对命中子段 `madvise(MADV_DONTNEED)`。需 `vma_utils` 已正确解析 `offset`（参考 MADVISE_COLD_PAGE_ENHANCEMENT_PLAN 的 offset 修复）。
- 方案 B：复用 `MadviseGeneralFiles` 即可，无新增。
- rawfile 偏移解析封装于 `file_page_releaser`（解 ZIP central directory，仅顶层条目，跳过目录前缀含 `/` 的）。

**约束**：仅对 `r--p`/`---p` 中只读非可写段释放；保持 `MADV_DONTNEED`。

## T-3 NAPI 绑定

**文件**：`frameworks/js/napi/app_memory/{app_memory_module.cpp,js_app_memory.{h,cpp},BUILD.gn}`

**要点**：
1. 模块注册镜像 `app_manager_module.cpp`：`nm_modname="application.appMemory"`，`__attribute__((constructor))`。
2. `JsAppMemoryInit` 导出 `releaseFilePages` / `releaseRawFilePages`（`BindNativeFunction`）。
3. 入口第一步：`permission_verification` `CheckCallerIsSystemApp()` 失败 → `napi_throw` 错误码 `202`，返回 undefined。
4. 参数校验：数组非空/元素为 string/无空串/moduleName 非空，否则 `401`。
5. 用 `napi_create_async_work` 在 worker 执行 `FilePageReleaser::*`，主线程 resolve/reject（错误码经 `BusinessError` 转换）。

## T-4 ANI 绑定

**文件**：`frameworks/ets/ani/app_memory/{ets_app_memory.cpp,...,BUILD.gn}`

**要点**：镜像 `ets_app_manager.cpp` 模式：`ANI_Constructor` + `kitFunctions[]`（`NativeReleaseFilePages`/`NativeReleaseRawFilePages`）；同样做系统应用校验与异步。

## T-5 注册与配置

- `bundle.json`：登记 NAPI/ANI 新模块产物与 external_deps（zip 依赖若方案 A）。
- 相关 BUILD.gn 引入新目标。
- 确认接口声明在合适 `interfaces/kits/` 头（若有对外 native 头需求，否则仅 binding 内部用）。
- 错误码：复用既有 202/401；若新增专属码（如 `ERR_ABILITY_RUNTIME_INVALID_FILE_PATH`）在 `ability_runtime_error_util.h` 登记。

## T-6 单元测试

**文件**：`test/unittest/file_page_releaser_test/file_page_releaser_test.cpp` + BUILD.gn

**用例**：
- 白名单：合法本 bundle 路径通过；`../` 穿越 reject；绝对非本 bundle reject；`realpath` 逃逸 reject。
- 后缀分类：.so/.hap/.hsp 通过；.txt/空串 reject。
- 原子语义：混合合法非法 → 整体 reject 且不调用 madvise（mock `MadviseSingleLibrary` 验证未被调用）。
- moduleName 不存在 reject。
- rawfile 偏移解析（方案 A）：mock ZIP 字节流，验证只取顶层条目偏移。
- 错误码映射：非系统应用→202、参数→401。

## T-7 集成验证

- 构建系统应用 Demo（含 libfoo.so、entry.hap、rawfile 顶层文件）。
- 对 AC-1/2/3：调用前后 `hidumper` smap 抓 RSS/Pss，证明指定文件/模块映射区域文件页显著下降。**保存日志/截图作为 review 证据**。
- AC-4/5/6/7：用错误入参/非系统应用 demo 验证错误码。
- AC-8：ArkTS 与 NAPI 各跑一次。
- AC-9：前台/后台各跑一次。

## 完成定义 (DoD)

- [ ] T-0 结论记录于 review.md
- [ ] T-1~T-5 代码合入，本地构建通过
- [ ] T-6 单测全绿
- [ ] T-7 AC-1~AC-9 全部有证据
- [ ] review.md 全项打勾 + 验证证据齐
- [ ] CLAUDE.md 约束遵守（GN 构建、HiLog、error_utils）
