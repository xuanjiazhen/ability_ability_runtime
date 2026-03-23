# Stage 模型按需加载 FA 模块设计文档（动态 `dlopen`）

## 1. 背景

当前 `ability_runtime` 同时兼容 Stage 与 FA 两类应用模型。目标场景是：
- Stage 模型应用进程启动时，不预加载 FA 模型相关 `.so` 与 NAPI 模块；
- 通过动态 `dlopen` 按需加载（仅在确有调用时加载），从而降低内存占用并改善启动性能。

## 2. 现状分析（代码依据）

### 2.1 已有模型识别能力

`MainThread::HandleLaunchApplication` 已拿到模型标识：
- `entryHapModuleInfo.isStageBasedModel`（`frameworks/native/appkit/app/main_thread.cpp:1604`）

### 2.2 现有库加载行为过于“全量”

`LoadAbilityLibrary` 当前会扫描目录并对所有 `.so` 执行 `dlopen`：
- 扫描：`ScanDir` + `CheckFileType(.so)`（`main_thread.cpp:3277-3446`）
- 加载：`dlopen(resolvedPath, RTLD_NOW | RTLD_GLOBAL)`（`main_thread.cpp:3314`）

说明：当前逻辑没有“模型维度过滤”，只要在扫描目录里命中 `.so` 就会加载。

### 2.3 FA 模块的典型库与模块名

FA 相关 JS NAPI 目标（构建层）：
- `frameworks/js/napi/featureAbility/BUILD.gn` -> `featureability`
- `frameworks/js/napi/particleAbility/BUILD.gn` -> `particleability`
- `frameworks/js/napi/feature_ability/BUILD.gn` -> `featureability_napi`

FA 模块注册名（运行时 require 名称）：
- `ability.featureAbility`（`featureAbility/native_module.cpp`）
- `ability.particleAbility`（`particleAbility/native_module.cpp`）
- `FeatureAbility`（`feature_ability/feature_ability_module.cpp`）

### 2.4 运行时已有 `requireNapi` 调用入口

`JsRuntime` 内通过 `requireNapi` 加载系统模块：
- `LoadSystemModule` / `LoadSystemModuleByEngine`（`frameworks/native/runtime/js_runtime.cpp:589-628, 1142-1169`）

这为“按模块名触发懒加载 `dlopen`”提供了天然切入点。

## 3. 目标与非目标

## 3.1 目标

1. Stage 模型进程启动时不加载 FA 相关库。  
2. FA 模型行为保持不变（兼容优先）。  
3. Stage 模型误用 FA API 时可控（默认禁用，支持可选兼容开关）。  
4. 具备可灰度、可观测、可回退能力。

## 3.2 非目标

1. 不移除 FA 能力本身。  
2. 不改变对外 API 定义。  
3. 本期不做跨子系统 ABI 调整。

## 4. 方案总览

引入“模型感知 + 模块感知”的双层加载策略：

1. **启动期过滤（MainThread）**
- Stage 模型：过滤掉 FA so，不进入启动期全量 `dlopen`。
- FA 模型：保持现有行为。

2. **运行时按需加载（JsRuntime）**
- 在 `requireNapi(moduleName)` 前增加轻量守卫：
  - 若 `moduleName` 属于 FA 且模型为 Stage：默认拒绝（返回明确错误）。
  - 若启用兼容开关：先 `dlopen(FA so)`，再重试 `requireNapi`。

3. **集中映射表**
- 建立 `moduleName -> soName` 映射，统一管理 FA 模块归属，避免散落判断。

## 5. 详细设计

### 5.1 模型态传递

当前 `Runtime::Options` 已有 `isStageModel` 字段（默认 true）。建议显式赋值：
- 在 `HandleLaunchApplication` 创建 runtime options 时设置：`options.isStageModel = isStageBased;`

目的：避免后续逻辑依赖默认值，提升可读性与可靠性。

### 5.2 启动期 so 过滤（MainThread）

#### 5.2.1 新增分类函数

新增函数（建议放 `main_thread.cpp` 匿名命名空间）：
- `bool IsFAModelSo(const std::string &soName)`
- 规则匹配：
  - `libfeatureability.z.so`
  - `libfeatureability_napi.z.so`
  - `libparticleability.z.so`
  - （按实际产物保留 `*.so` / `*.z.so` 双后缀兼容）

#### 5.2.2 修改加载入口

在 `LoadAbilityLibrary` / `LoadNativeLibrary` 中加入策略参数：
- `AppModelType modelType`（`STAGE` / `FA`）
- Stage 时跳过 `IsFAModelSo == true` 的条目；FA 时不变。

#### 5.2.3 观测信息

增加日志和统计：
- `skip_fa_so_count`
- `skip_fa_so_names`
- 上报到 HiSysEvent（debug/统计域）

### 5.3 运行时懒加载（JsRuntime requireNapi 前置）

#### 5.3.1 模块映射

新增静态映射（建议放 `js_runtime.cpp`）：

- `ability.featureAbility -> libfeatureability.z.so`
- `ability.particleAbility -> libparticleability.z.so`
- `FeatureAbility -> libfeatureability_napi.z.so`

#### 5.3.2 懒加载流程

在 `LoadSystemModule` / `LoadSystemModuleByEngine` 里：

1. 先判断 `moduleName` 是否 FA 模块。  
2. 若非 FA：原逻辑。  
3. 若是 FA：
- `isStageModel == true` 且兼容开关关闭 -> 直接返回 `ERROR_NOT_SUPPORTED_IN_STAGE_MODEL`。
- `isStageModel == true` 且兼容开关开启 -> `EnsureFaModuleLoaded(soName)` 后再 `requireNapi`。
- `isStageModel == false`（FA 模型）-> 保持兼容（可选：同样走 `EnsureFaModuleLoaded` 提前兜底）。

#### 5.3.3 `EnsureFaModuleLoaded`

- 使用 `std::once_flag + unordered_map<string, void*>` 防重复加载。  
- `dlopen(path, RTLD_NOW | RTLD_GLOBAL)` 成功后缓存 handle。  
- 失败时返回错误并上报统计。

#### 5.3.4 开关设计

建议新增系统参数：
- `persist.sys.abilityruntime.stage_fa_lazyload=true`（总开关）
- `persist.sys.abilityruntime.stage_allow_fa_compat=false`（Stage 下 FA 兼容）

默认策略建议：
- `stage_fa_lazyload=true`
- `stage_allow_fa_compat=false`

### 5.4 错误码与行为约定

新增业务错误码：
- `ERR_MODULE_NOT_SUPPORTED_IN_STAGE`（Stage 使用 FA API）
- `ERR_MODULE_DLOPEN_FAILED`

JS 层抛错信息建议：
- `"module ability.featureAbility is not supported in Stage model"`

### 5.5 线程安全与生命周期

1. 懒加载函数必须线程安全（并发 require 场景）。  
2. so handle 默认进程级常驻，不执行 `dlclose`（避免析构顺序风险）。  
3. 重复加载应无副作用（幂等）。

## 6. 代码改造点（建议清单）

### 6.1 启动层
- `frameworks/native/appkit/app/main_thread.cpp`
- `interfaces/kits/native/appkit/app/main_thread.h`

改造内容：
- 新增模型感知加载策略
- `LoadAbilityLibrary` / `LoadNativeLibrary` 增加过滤
- 增加跳过统计打点

### 6.2 运行时层
- `frameworks/native/runtime/js_runtime.cpp`
- `interfaces/inner_api/runtime/include/js_runtime.h`

改造内容：
- 增加模块->so 映射
- 增加 `EnsureFaModuleLoaded`
- 在 `LoadSystemModule*` 中接入守卫逻辑
- 通过 options 记录 `isStageModel`

### 6.3 公共常量与错误
- 建议在 runtime/common 错误定义处新增错误码与文本

## 7. 兼容性与风险

### 7.1 风险点

1. Stage 应用历史上误用 FA API，可能在开启严格策略后报错。  
2. so 名称/安装路径在不同产品变体下可能不一致。  
3. 某些三方插件可能间接依赖 FA 模块符号。

### 7.2 缓解

1. 提供 `stage_allow_fa_compat` 开关用于灰度过渡。  
2. 模块映射支持多候选路径和后缀。  
3. 增加上线前扫描：统计 Stage 应用是否实际请求过 FA 模块。

## 8. 测试与验收

### 8.1 功能测试

1. Stage 应用不调用 FA API：进程正常运行，且 `/proc/<pid>/maps` 不含 FA so。  
2. Stage 应用调用 FA API：
- 严格模式：返回预期错误码。  
- 兼容模式：懒加载成功后调用可用。  
3. FA 应用回归：行为不变。

### 8.2 稳定性测试

1. 并发 `requireNapi` 压测（确保无重复 dlopen 崩溃）。  
2. 长稳运行（24h+）验证无句柄异常和内存持续泄漏。  
3. 关键场景回归：启动、前后台切换、补丁加载。

### 8.3 性能测试

对比前后指标：
- 启动阶段 `dlopen` 次数
- 冷启动耗时（P50/P95）
- 首帧时延
- 进程 PSS/RSS（启动后 10s、稳定态 5min）

## 9. 收益预估（含假设）

## 9.1 假设前提

1. Stage 应用不需要 FA API（主流场景）。  
2. 当前 Stage 进程存在 FA so 被预加载的情况（由目录扫描触发）。  
3. FA 相关 so 典型集合：
- `libfeatureability.z.so`
- `libfeatureability_napi.z.so`
- `libparticleability.z.so`

## 9.2 单进程收益（估算）

| 指标 | 保守估计 | 中位估计 | 激进估计 |
|---|---:|---:|---:|
| 进程 PSS 降低 | 0.8 MB | 1.4 MB | 2.4 MB |
| 进程 RSS 降低 | 1.5 MB | 2.8 MB | 4.5 MB |
| 冷启动耗时下降 | 10~20 ms | 20~45 ms | 45~80 ms |

说明：
- PSS 下降取决于 so 实际映射段与共享比例；
- 启动时延下降取决于 `dlopen + relocation + ctor` 路径占比。

## 9.3 系统级收益（估算）

以“同一时刻 20 个 Stage 进程活跃”为例：
- 总 PSS 节省约 `16 MB ~ 48 MB`。

在内存压力场景下，可明显降低被 LMK 回收概率，并提升后台驻留质量。

## 10. 实施计划

### Phase 1（1 周）
- 增加埋点与只读统计（不改变行为）
- 产出 Stage 进程 FA so 实际加载画像

### Phase 2（1~2 周）
- 启动期过滤上线（灰度）
- 运行时守卫上线，默认兼容模式开启

### Phase 3（1 周）
- 关闭 Stage 下 FA 兼容开关（严格模式）
- 全量观察并沉淀回归基线

## 11. 回退策略

1. 运行时开关即时回退：
- `stage_fa_lazyload=false` 或 `stage_allow_fa_compat=true`

2. 代码路径保留旧逻辑：
- 若懒加载失败，按开关选择“降级兼容”或“快速失败”。

---

该方案的核心收益来自“把模型不需要的库从启动热路径拿掉”，并通过 `requireNapi` 前置守卫实现真正按需加载。对 Stage 模型应用，预期能稳定带来内存下降与冷启动收益。
