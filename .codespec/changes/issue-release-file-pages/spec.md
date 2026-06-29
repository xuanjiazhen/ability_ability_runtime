# Spec — 应用按需释放文件页

> Stage 2 产物。复杂度：标准。状态：**Approved（2026-06-16）**。
> 设计依据见同目录 `design.md`。

## 一、API 规格（System API）

模块名（建议）：`appMemory`（NAPI: `application.appMemory`；ArkTS namespace: `appMemory`）。
标注 `@systemapi`，仅系统应用可调用。

### 1.1 releaseFilePages

```
releaseFilePages(fileNames: Array<string>): Promise<void>
```

- **入参** `fileNames`：本 Bundle 内文件相对名清单。每项为以下之一：
  - `.so`：本 Bundle 已加载的动态库名（如 `"libfoo.so"`），按 `MadviseSingleLibrary`（dl_iterate_phdr）定位。
  - `.hap` / `.hsp`：本 Bundle 安装的包文件名（如 `"entry.hap"`），按 `MadviseGeneralFiles`（VMA）定位。
  - 其它后缀：拒绝。
- **返回**：`Promise<void>` —— resolve 表示已对所有合法项执行释放；reject 携带错误码。
- **语义（原子）**：所有项校验通过才执行；任一项非法（不在本 Bundle / 后缀不支持 / 路径穿越）→ 整体 reject，不释放任何项（ADR-6）。
- **时机**：前台/后台均可调用。
- **错误码**：
  - `202 ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP`：非系统应用。
  - `401 ERR_INVALID_VALUE`（参数非法通用码）：入参为空数组、含空串、含路径分隔符 `../`、解析后不在本 Bundle 树内、后缀不支持、文件不存在。
  - `16000001` 系列（复用既有通用错误）：内部失败（madvise 系统调用失败）。

### 1.2 releaseRawFilePages

```
releaseRawFilePages(moduleName: string): Promise<void>
```

- **入参** `moduleName`：本 Bundle 内的模块名（module.json5 的 `name`）。
- **行为**：
  - 解析该模块对应 hap 绝对路径；
  - **仅释放该 hap 内 rawfile 顶层文件**对应文件页（不递归子目录，ADR-Q6）；
  - 主路径（ADR-3 方案 A）：ZIP 解析 hap 取 rawfile 顶层条目偏移区间，对 hap 的 VMA 命中区间 `madvise(MADV_DONTNEED)`。
  - 降级（ADR-3 方案 B）：若 rawfile 已解包到磁盘，对解包后的普通文件走 `MadviseGeneralFiles`。
- **返回**：`Promise<void>`，同上。
- **错误码**：`202`、`401`（moduleName 为空/不存在/越界）、内部失败码。
- **时机**：前台/后台均可调用。

## 二、可接受标准 (AC) — WHEN/THEN 可测

### AC-1 (P0) 按文件名释放 .so
**WHEN** 系统应用调用 `releaseFilePages(["libxxx.so"])`，`libxxx.so` 属本 Bundle 且已映射；
**THEN** Promise resolve；`hidumper` 该进程 smap 中 `libxxx.so` 的 `r--p` 段 RSS/Pss 显著下降（>50% 可回收部分）。

### AC-2 (P0) 按文件名释放 .hap/.hsp
**WHEN** 系统应用调用 `releaseFilePages(["entry.hap"])`，该 hap 属本 Bundle 且已映射；
**THEN** Promise resolve；smap 中该 hap 的只读映射段 RSS 显著下降。

### AC-3 (P0) 按 moduleName 释放 rawfile
**WHEN** 系统应用调用 `releaseRawFilePages("entry")`，entry 模块 hap 内 rawfile 顶层有文件；
**THEN** Promise resolve；smap 中 entry.hap 映射区域对应 rawfile 偏移区间的文件页显著下降（通过 mincore 或 RSS 差异验证）。

### AC-4 (P1) 仅本 Bundle
**WHEN** 入参解析后路径不在本 Bundle 安装根下（含 `../` 穿越、绝对路径、他人 bundle 文件）；
**THEN** Promise reject(401)，不释放任何项，TAG_LOGW 告警。

### AC-5 (P1) 系统 API 鉴权
**WHEN** 非系统应用调用任一接口；
**THEN** Promise reject(202)，不执行释放。

### AC-6 (P1) 原子语义
**WHEN** `releaseFilePages` 入参同时含合法项与非法项；
**THEN** 整体 reject(401)，**合法项也不释放**（ADR-6 原子失败）。

### AC-7 (P1) 不存在的 moduleName / 文件
**WHEN** 入参对应文件/模块在本 Bundle 中不存在；
**THEN** reject(401)。

### AC-8 (P2) ArkTS 与 NAPI 等价
**WHEN** 分别经 ANI / NAPI 通道对相同合法入参调用；
**THEN** 行为一致、返回结构一致（均 resolve/reject 同错误码）。

### AC-9 (P2) 前后台均可
**WHEN** 应用在前台 / 后台分别调用合法入参；
**THEN** 均正常 resolve 并释放文件页（无时机限制）。

## 三、行为约束规则

- **R1**：所有路径解析必须以 `ApplicationContext::GetBundleName()` 得到的本 Bundle 名为根，经 `BundleMgrHelper` 查询；禁止信任调用方提供的绝对路径。
- **R2**：白名单校验在 `file_page_releaser` 中统一进行，binding 层不重复实现。
- **R3**：只对只读、非可写 VMA 段执行 `MADV_DONTNEED`（复用 `MadviseGeneralFiles` 既有过滤）。
- **R4**：rawfile 释放仅限顶层文件，不递归子目录（含子目录的条目偏移跳过）。
- **R5**：原子语义 — 单次调用任一项非法则整体不执行（ADR-6）。
- **R6**：不写文件、不跨进程、不修改 hap 内容（madvise 仅影响本进程页缓存）。

## 四、上下文收集（复用既有设施）

- 复用 `MadviseUtil::MadviseSingleLibrary`（.so）/ `MadviseGeneralFiles`（.hap/.hsp/rawfile 解包态）。
- 复用 `VmaUtil::GetFileVmas`（`/proc/self/maps` 解析）。
- 新增（按 ADR-3 方案 A）：rawfile 偏移解析能力（ZIP central directory 解析），封装于 `file_page_releaser`。
- 系统应用校验：复用 `permission_verification.h` 的 `CheckCallerIsSystemApp()`。
- 错误码：复用 `interfaces/inner_api/error_utils/include/ability_runtime_error_util.h` 的 `ERR_ABILITY_RUNTIME_NOT_SYSTEM_APP`(202) 及通用 401。

## 五、不涉及项 (N/A)

- 跨 Bundle 释放、Native C / CJ 绑定、rawfile 子目录递归、释放量统计返回、GN/运行时特性开关、前后台时机限制 —— 同 proposal §三。

## 六、待 Spec 阶段确认的开放项（来自 design.md）

- **O-1 / O-2**：ADR-3 方案 A 的 ZIP 解析依赖可得性 + rawfile 映射形态实测 → 决定 A/B 主路径。**约束**：进入 Stage 3 前，需通过一次 `hidumper`/`/proc/self/maps` 实测确认；若方案 A 依赖不可得，明确降级方案 B 并据此修订 AC-3 的验证措辞。
- **O-3**：API 命名定稿（`appMemory` 倾向）。

## 七、示例用法（ArkTS）

```ts
import { appMemory } from 'kit.AbilityKit'; // 或专属 kit，Stage 3 定稿

// 释放 so 与 hap 文件页
appMemory.releaseFilePages(['libfoo.so', 'entry.hap'])
  .then(() => console.info('released'))
  .catch((e: BusinessError) => console.error(`code=${e.code}`));

// 释放 entry 模块 rawfile 顶层文件页
appMemory.releaseRawFilePages('entry')
  .catch((e: BusinessError) => console.error(`code=${e.code}`));
```
