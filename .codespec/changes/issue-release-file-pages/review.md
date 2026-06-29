# Review — 应用按需释放文件页

> Stage 3 统一审查。复杂度：标准。

## 一、规范符合性

| 项 | 结果 | 证据 |
|----|------|------|
| 遵循 CLAUDE.md 构建（GN）/日志（HiLog TAG_LOG*）/错误码（error_utils） | 通过 | BUILD.gn 用 `ohos_shared_library`；代码用 `TAG_LOGI/W/E` + `AAFwkTag::ABILITY`；错误码复用 202/401 |
| 复用既有 madvise 工具链（ADR-3） | 通过 | `MadviseSingleLibrary`/`MadviseGeneralFiles` 复用；新增 `MadviseFilePagesAtOffsets` 复用 `VmaUtil::GetFileVmas` |
| API 形态对齐 spec（两个 Promise<void> 系统接口） | 通过 | NAPI `releaseFilePages`/`releaseRawFilePages`；ANI `nativeReleaseFilePages`/`nativeReleaseRawFilePages` |
| 仅本 Bundle（ADR-2 安全） | 通过 | 入参禁止路径分隔符；hap 需匹配 `GetBundleInfoForSelf` 返回的 `hapModuleInfos[].hapPath` basename；module/hap 路径来自调用方自身 |
| 原子语义（ADR-6） | 通过 | `ReleaseByFileNames` 先全量校验再执行，任一非法返回 401 |
| 前后台均可 / 不加特性开关 | 通过 | 无生命周期判断、无 gni 开关 |

## 二、任务完成状态

| Task | 状态 | 说明 |
|------|------|------|
| T-0 rawfile 映射形态 + ZIP 依赖实测 | **通过（2026-06-29）** | 用户设备实测确认 rawfile 以 hap 整包 mmap 形态存在（无磁盘解包），方案 A 成立，当前 ZIP 中央目录解析 + offset 求交实现可用 |
| T-1 Native 引擎 file_page_releaser | **通过** | `frameworks/native/.../memory/file_page_releaser.{h,cpp}` + BUILD.gn；编译 + 单测 + 设备验证通过 |
| T-2 madvise 扩展 | **通过** | `MadviseFilePagesAtOffsets` + VMA offset 解析修复；编译 + 单测通过 |
| T-3 NAPI 绑定 | **通过** | `frameworks/js/napi/app_memory/*`；编译通过，设备实测 AC-1/3 RSS 下降符合预期 |
| T-4 ANI 绑定 | **In-repo 半边完成，跨仓 TS 声明落位中（O-4）** | `frameworks/ets/ani/app_memory/*`；用户确认可在 interface/sdk 仓落位 SDK 声明 |
| T-5 构建登记 | 完成 | `frameworks/js/napi/BUILD.gn`、`frameworks/ets/ani/BUILD.gn` 两个聚合 group 已加条目；bundle.json 无显式 target 列表（自动发现） |
| T-6 单元测试 | **通过（2026-06-29）** | 用户本地 `run -t UT` 全绿：`ability_madvise_utils_test` + `file_page_releaser_test` |
| T-7 集成验证（hidumper smap） | **通过（2026-06-29）** | 用户设备 AC-1~AC-9 全部通过：releaseFilePages/releaseRawFilePages 后 smap RSS 显著下降；参数校验类抛码正确 |

## 三、未验证声明（诚实边界）

**编译验证（已通过）**：用户本地构建编译通过（2026-06-29），覆盖全部新增/修改源码，含此前标注的风险点（`runtime` 共享库提供 BundleMgrHelper 符号、ANI 签名串、NAPI 包含链）。

**单元测试（已通过）**：用户本地运行 `run -t UT -ts ability_madvise_utils_test` 与 `run -t UT -ts file_page_releaser_test`，全部用例通过（2026-06-29）。覆盖 `MadviseFilePagesAtOffsets` 空入参/真实文件 in-span vs out-span、`ReleaseByFileNames` 空列表/路径分隔符/后缀/原子校验、`ReleaseRawFileByModule` 空 moduleName/分隔符拒绝。

**T-0 实测（已通过）**：用户设备确认 rawfile 以 hap 整包 mmap 形态存在（无磁盘解包），方案 A 成立，自包含 ZIP 中央目录解析实现可用（2026-06-29）。

**AC-1~AC-9 hidumper 实测（已通过）**：用户设备验证 `releaseFilePages`/`releaseRawFilePages` 调用后目标文件 hap RSS 显著下降；参数校验类（202/401）抛码正确；原子语义成立（2026-06-29）。

**待办（不阻塞本仓合入）**：
- O-4：`handoff/appMemory.d.ts` 在 interface/sdk-js 仓落位 + AsyncCallbackWrapper 胶水 + `@since` 版本号分配（用户确认可落位，由 SDK 团队执行）。落位前 ANI `FindNamespace` 失败已降级处理，NAPI 通道不受影响。

**"通过"证据齐**：编译通过 + 单测全绿 + T-0 确认 + AC hidumper 对比留存。

## 四、阻塞与开放项

- **O-1/O-2（T-0）**：rawfile 映射形态 + ZIP 依赖。**已确认方案 A 成立**（设备实测，2026-06-29），无需切方案 B。
- **O-4（ANI 跨仓）**：**交付件已产出**（`.codespec/.../handoff/appMemory.d.ts` + `handoff/README.md`），含公共 API 声明与 native 绑定契约，供 interface/sdk-js 仓落位。用户确认可落位；落位 + AsyncCallbackWrapper 胶水 + `@since` 版本号由 SDK 团队执行，不阻塞本仓合入。落位前 ANI `FindNamespace` 失败已降级，NAPI 通道不受影响。
- **ZIP 解析逻辑复核**（本轮）：EOCD 回扫、CD 遍历、local header 偏移解析、span 与 VMA offset 求交均正确；越界检查完备；无死循环（每轮 pos 至少 +46）；stored/deflated 条目均适用。未发现缺陷。

## 五、改动文件清单

新增：
- `frameworks/native/ability/native/ability_runtime/memory/file_page_releaser.h`
- `frameworks/native/ability/native/ability_runtime/memory/file_page_releaser.cpp`
- `frameworks/native/ability/native/ability_runtime/memory/BUILD.gn`
- `frameworks/js/napi/app_memory/{app_memory_module.cpp,js_app_memory.h,js_app_memory.cpp,BUILD.gn}`
- `frameworks/ets/ani/app_memory/{include/ets_app_memory.h,src/ets_app_memory.cpp,BUILD.gn}`
- `test/unittest/file_page_releaser_test/{file_page_releaser_test.cpp,BUILD.gn}`

修改：
- `frameworks/native/ability/native/ability_runtime/madvise/madvise_utils.h`（+FileOffsetSpan, MadviseFilePagesAtOffsets）
- `frameworks/native/ability/native/ability_runtime/madvise/madvise_utils.cpp`（实现 + 还原 MadviseWithConfigFile）
- `frameworks/native/ability/native/ability_runtime/madvise/vma_utils.cpp`（offset 解析修复）
- `frameworks/js/napi/BUILD.gn`、`frameworks/ets/ani/BUILD.gn`（聚合 group 登记）
- `test/unittest/ability_madvise_utils_test/madvise_utils_test.cpp`（+offset 用例）

## 六、结论

**通过 / Approved**：代码层面按 design/spec 落地完整；编译通过、单测全绿、T-0 设备实测确认方案 A 成立、AC-1~AC-9 hidumper 验证全部通过（2026-06-29）。**满足合入门禁**（HARD-GATE: review.md 全项打勾 + 验证证据齐 = 通过）。

唯一遗留：O-4 跨仓 SDK 声明（`interface/sdk-js` 仓）由 SDK 团队落位，不阻塞本仓代码合入；落位前 ANI 通道降级，NAPI 通道正常。
