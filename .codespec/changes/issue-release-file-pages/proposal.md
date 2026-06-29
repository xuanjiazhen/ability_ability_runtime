# Proposal — 应用按需释放文件页 (Release File Pages On Demand)

> Stage 1 产物。状态：**Baselined（已基线 2026-06-16）**。
> 复杂度级别：**标准**（单仓为主 + 新增系统 API + 复用既有 madvise 基础设施）。

## 一、问题陈述

应用按需卸载回收内存场景中，应用缺乏一种**主动、定向**释放自身文件页（file-backed pages）的能力。现有 madvise 机制（`MadviseUtil::MadviseWithConfigFile`）仅在首次退后台时按全局 JSON 配置批量释放，且无法：

1. 在运行时由应用按需指定具体 hap/hsp/so 文件释放其文件页；
2. 按模块（moduleName）释放其 rawfile 资源占用的文件页。

结果：应用无法在"已知某些文件/资源不再使用"的精确时机回收内存，只能依赖系统被动回收，内存占用居高不下。

## 二、目标

为系统应用提供两个**异步、定向**的文件页释放接口，使其能在任意时机主动降低指定文件/模块占用的文件页：

- **G1**：提供按文件名（hap/hsp/so）释放文件页的系统 API；
- **G2**：提供按 moduleName 释放该模块 rawfile 顶层文件页的系统 API；
- **G3**：复用并扩展现有 `frameworks/native/ability/native/ability_runtime/madvise/` 工具链（`MadviseGeneralFiles` / `MadviseSingleLibrary` / VMA 解析）；
- **G4**：同时提供 **ArkTS (ANI/ets)** 与 **JS/NAPI** 两套绑定；
- **G5**：仅限调用方**本 Bundle** 的文件页，安全隔离。

## 三、非目标 (N/A)

- **N1**：不提供跨 Bundle 释放能力（安全默认）；
- **N2**：不提供 Native C / CJ 绑定（本期范围外）；
- **N3**：不替换/废弃既有 `MadviseWithConfigFile` 配置式机制，二者并存；
- **N4**：不引入 GN 特性开关或运行时参数开关（默认启用）；
- **N5**：不限制前台/后台调用时机；
- **N6**：不做释放量统计返回，仅返回成功/错误码；
- **N7**：不递归释放 rawfile 子目录。

## 四、用户故事

### US-1：按文件名释放（hap/hsp/so）
**作为**一个系统应用开发者，
**我希望**在确认某些已加载的 hap/hsp/so 文件不再使用时，调用 `releaseFilePagesByFileNames(["libfoo.so", "bar.hsp"])`，
**以便**立即降低这些文件占用的文件页，回收内存。

### US-2：按模块释放 rawfile
**作为**一个系统应用开发者，
**我希望**在某个 moduleName 下的 rawfile 资源（如启动期用的引导图、迁移完成的临时数据）不再使用时，调用 `releaseRawFilePagesByModule("entry")`，
**以便**回收该模块 rawfile 顶层文件占用的文件页。

## 五、验收标准 (Acceptance Criteria)

> 所有 AC 使用 WHEN/THEN 格式，需可用 hidumper smap / 单元测试验证。

### AC-1（P0）按文件名释放 hap/hsp/so
**WHEN** 系统应用在任意生命周期阶段调用 `releaseFilePagesByFileNames(["libxxx.so"])`，且该文件属于本 Bundle；
**THEN** 接口返回成功；通过 `hidumper` 查看该进程 smap，`libxxx.so` 对应区域的文件页（`r--p` 只读映射段）RSS 显著下降。

### AC-2（P0）按 moduleName 释放 rawfile
**WHEN** 系统应用调用 `releaseRawFilePagesByModule("entry")`，且 entry 模块存在 rawfile 顶层文件；
**THEN** 接口返回成功；通过 `hidumper` 查看该进程 smap，entry 模块 hap 包内 rawfile 顶层文件对应的映射区域文件页显著下降。

### AC-3（P1）仅本 Bundle 生效
**WHEN** 调用方传入不属于本 Bundle 的文件名或解析后路径越界；
**THEN** 接口返回权限/参数错误码（`ERR_INVALID_VALUE` 或专属错误码），不释放任何文件页，并打 warning 日志。

### AC-4（P1）系统 API 鉴权
**WHEN** 非系统应用身份调用任一接口；
**THEN** 接口返回 `ERR_NOT_SYSTEM_APP`（或既有系统 API 鉴权错误码），不执行释放。

### AC-5（P2）ArkTS 与 NAPI 双绑定等价
**WHEN** 通过 ArkTS (ANI) 或 JS/NAPI 任一通道调用相同语义接口；
**THEN** 行为一致，返回值结构一致。

### AC-6（P2）不存在的文件/模块
**WHEN** 调用方传入不存在的文件名或 moduleName；
**THEN** 接口返回 `ERR_INVALID_VALUE`，不影响其它已传入且合法的条目语义（整体失败或跳过非法项，见 Spec 细化）。

## 六、范围边界

### 涉及子系统/仓
- **ability_runtime**（本仓，主）：
  - `frameworks/native/ability/native/ability_runtime/madvise/`：扩展工具链
  - `frameworks/js/napi/`：新增 NAPI 模块/绑定
  - `frameworks/ets/ani/`：新增 ANI 绑定
  - `interfaces/kits/native/` 或新增 manager 头：对外接口
  - `interfaces/inner_api/error_utils/`：可能新增错误码
  - `test/unittest/`：单元测试
- **bundle_framework**（只读依赖）：解析本 Bundle 安装路径 / moduleName → hap 路径 / rawfile 路径（仅查询，不改）。

### 不涉及
- 内核/内存子系统改动（仅用既有 madvise 系统调用）
- AppSpawn / AppManagerService 服务端逻辑
- 跨设备/分布式

## 七、关键决策（澄清记录）

| # | 决策 | 选择 | 理由 |
|---|------|------|------|
| Q1 | API 语言绑定 | ArkTS(ANI) + JS/NAPI | 覆盖 Stage/FA 主流系统应用入口；Native C / CJ 本期不做 |
| Q2 | 挂载点 | **新增独立 Manager**（如 `appMemoryManager` / `FilePageReleaseManager`） | 与 hap/so 进程级资源语义匹配，避免污染 Context 职责 |
| Q3 | 触发时机 | 任意时机异步调用（`Promise<void>`） | 需求为"应用主动释放"，不与生命周期强绑定 |
| Q4 | 跨 Bundle | 仅本 Bundle 文件页 | 安全默认，避免越权释放 |
| Q5 | hap/hsp/so 入参 | 文件名/相对名（自动解析为本 Bundle 路径） | 输入简单，与既有 madvise_config 一致 |
| Q6 | rawFile 粒度 | 按 moduleName 全量释放（仅顶层 rawfile 文件） | 一次到位、语义清晰；子目录递归本期不做 |
| Q7 | madvise 策略 | `MADV_DONTNEED` | 与既有 `MadviseGeneralFiles` 一致，内存收益直接 |
| Q8 | 返回值 | 仅成功/错误码（`Promise<void>`） | 简单，与多数系统 API 一致 |
| Q9 | 权限 | 仅系统应用身份 | 不新增专有 permission |
| Q10 | 前后台 | 前后台均可 | "主动释放"语义，不做时机限制 |
| Q11 | 特性开关 | 不加 GN/运行时开关 | 默认启用，简化交付 |

## 八、澄清记录

- **第 1 轮**（API 层 / 挂载点 / 触发时机 / 跨 Bundle）：已答 → Q1–Q4。
- **第 2 轮**（入参形式 / rawFile 粒度 / madvise 策略 / 返回值）：已答 → Q5–Q8。
- **第 3 轮**（权限 / 前后台 / rawFile 子目录 / 特性开关）：已答 → Q9–Q11。

## 九、需求基线

- **基线结论**：`通过 (Baselined 2026-06-16)`
- **Owner 确认**：用户显式批准"进入 Stage 2"
- **复杂度**：标准
- **范围**：ability_runtime 单仓（+ bundle_framework 只读查询依赖）

## 十、风险与开放项

- **R1（待 Spec 解决）**：非法项与合法项混传时整体失败 vs 跳过——倾向"整体返回错误但仍对合法项执行释放"或"全部不执行"，需在 spec 明确。
- **R2**：moduleName → hap 绝对路径解析需依赖 bundle_manager 查询接口，需确认 ability_runtime 内已有可用的 bundle 路径查询 helper（探索发现 `ability_bundle_manager_helper`）。
- **R3**：rawfile 顶层文件路径在 hap 内部的映射（hap 是 zip，其 rawfile 是否作为独立文件映射？需在 Spec 阶段核验 `/proc/self/maps` 实际映射形态）。
