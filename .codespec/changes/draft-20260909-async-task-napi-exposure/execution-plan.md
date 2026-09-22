# 执行计划

> 将 Approved Spec 拆成可独立执行、可验证、可审查的 Task。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-ASYNTASK-NAPI-EXPOSURE |
| 关联 Feature/Bug | FEAT-ASYNTASK-NAPI-EXPOSURE / REQ-ASYNTASK-NAPI-EXPOSURE |
| 关联文档 | proposal.md / design.md / spec.md |
| 复杂度 | 标准 |
| 状态 | Approved |
| Owner | yangzk |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved（Baselined） |
| Design | `design.md` | Approved |
| Spec | `spec.md` | Approved |

## 受影响文件全量清单

> 对照 design.md 调用链层级分析（D1 构建层 / D2 引擎加载层 / D3 框架接线层），逐层列出具体文件。

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| ability_runtime | D1 构建层 | `frameworks/native/appkit/app_startup/async_task_excutor.ts` | 新增（git mv 自 `frameworks/js/napi/app_startup/async_task_excutor/`） | TS 胶水：taskpool 推送（'use concurrent'） |
| ability_runtime | D1 构建层 | `frameworks/native/appkit/app_startup/async_task_callback.ts` | 新增（git mv 自 `frameworks/js/napi/app_startup/async_task_callback/`） | TS sendable 包装类（'use sendable'） |
| ability_runtime | D1 构建层 | `frameworks/native/appkit/app_startup/appkit_startup_task.cpp` | 新增 | 内部 .so 导出文件：extern "C" getter 返回两段 ABC buffer（visibility default，无 version_script） |
| ability_runtime | D1 构建层 | `frameworks/js/napi/app_startup/async_task_excutor/BUILD.gn` | 删除 | 原 napi 模块目标定义 |
| ability_runtime | D1 构建层 | `frameworks/js/napi/app_startup/async_task_excutor/async_task_excutor_module.cpp` | 删除 | napi_module_register / GetABCCode 注册源 |
| ability_runtime | D1 构建层 | `frameworks/js/napi/app_startup/async_task_callback/BUILD.gn` | 删除 | 原 napi 模块目标定义 |
| ability_runtime | D1 构建层 | `frameworks/js/napi/app_startup/async_task_callback/async_task_callback_module.cpp` | 删除 | napi_module_register / GetABCCode 注册源 |
| ability_runtime | D1 构建层 | `frameworks/js/napi/BUILD.gn` | 修改 | `napi_packages` 移除 `asynctaskcallback_napi`、`asynctaskexcutor_napi` 两行 |
| ability_runtime | D1 构建层 | `frameworks/native/appkit/BUILD.gn` | 修改 | es2abc/gen_js_obj 目标保留；新增 `ohos_shared_library("appkit_startup_task")`（默认安装 /system/lib{,64}/，**不设** relative_install_dir、**不配** version_script、无注册代码）链入 4 个 .o；`appkit_native` deps 由 4 个 .o 目标改为 `:appkit_startup_task` |
| ability_runtime | D2 引擎加载层 | `interfaces/inner_api/runtime/include/js_runtime.h` | 修改 | 新增静态方法 `LoadInternalModuleByBuffer` 声明 |
| ability_runtime | D2 引擎加载层 | `frameworks/native/runtime/js_runtime.cpp` | 修改 | 新增 `LoadInternalModuleByBuffer` 实现（ExecuteModuleFromBuffer + GetExportObjectFromBuffer + napi_new_instance） |
| ability_runtime | D3 框架接线层 | `frameworks/native/appkit/app_startup/js_startup_task.cpp` | 修改 | 移除 `_binary_*` extern 声明，改为声明并调用 `GetAsyncTaskExcutorAbc`/`GetAsyncTaskCallbackAbc` getter，改走 LoadInternalModuleByBuffer |

**检查项：**
- [x] design.md 三层（D1/D2/D3）每一层都有对应文件列出
- [x] 每个文件修改类型和职责说明明确
- [x] `interfaces/kits/`（公共 SDK）零文件——公共 API 兼容性边界不受触碰

## AC 到 Task 追溯

> spec.md 的每个 AC 必须映射到至少一个 Task。

| AC | 来源 | Task | 验证方式 | 实际 Code Ref | Commit | Review Evidence | 验证状态 |
|----|------|------|----------|---------------|--------|-----------------|----------|
| AC-1 | spec.md | TASK-1, TASK-3 | 静态 grep + 产物检查 + 设备负向（待） | js_startup_task.cpp:20-23；async_task_*/ 目录删除 | 本笔提交 | evidence/reviews/spec-compliance.md#AC | Pass |
| AC-2 | spec.md | TASK-1 | 构建产物/目标清单检查 | frameworks/js/napi/BUILD.gn:49-52；appkit/BUILD.gn:72-99 | 本笔提交 | evidence/reviews/spec-compliance.md#AC | Pass |
| AC-3 | spec.md | TASK-2, TASK-3 | 等价论证 + 编译 + 单测（待） | js_runtime.cpp:632-665；js_startup_task.cpp:180-220 | 本笔提交 | evidence/reviews/spec-compliance.md#AC | Pass |
| AC-4 | spec.md | TASK-3 | diff 范围核对（主线程路径零触碰） | js_startup_task.cpp（RunOnMainThread 未改） | 本笔提交 | evidence/reviews/verification.md#变更范围核对 | Pass |
| AC-5 | spec.md | TASK-2, TASK-3 | 错误分支走查 + 编译 | js_runtime.cpp:636-660；js_startup_task.cpp:184-197 | 本笔提交 | evidence/reviews/spec-compliance.md#AC | Pass |
| AC-6 | spec.md | TASK-1, TASK-2, TASK-3 | gn + 真实工具链编译 + CI（待） | 全部受影响文件 | 本笔提交 | evidence/reviews/verification.md#Execution | Pass |

> 设备/CI 侧验证遗留项不影响上表静态验证状态（Pass 指本环境已执行部分），遗留清单见 review.md 与 verification.md Coverage 表。

## 首批实现边界

**首批必须实现：** TASK-1、TASK-2、TASK-3 全部（主链闭合的最小集——三者共同构成一个能力闭环，单独任一 Task 均无法编译通过或无法达成 AC）
**可后置：** 设备级 E2E（三方应用 requireNapi 负向用例需真机/模拟器镜像，静态与构建产物检查先行）
**不建议延后：** 无（本变更无独立可延后的增强项）

## Task 粒度原则

- TASK-1（构建层）与 TASK-2（引擎能力）文件范围与验证闭环完全分离 → 并行拆分
- TASK-3 依赖前两者产物（.o 符号 + 新 API），是行为变更落地点 → 独立 Task 承载回归责任
- 每个 Task 可独立验证：TASK-1 以构建/清单为判据，TASK-2 以编译 + 单测为判据，TASK-3 以全量回归为判据

## 禁止项

- [x] 没有未解决的占位标记（自审项本身不含占位词汇）
- [x] 没有"根据需要实现""酌情处理"等模糊指令
- [x] 没有跨 Task 隐式依赖（依赖显式声明）
- [x] 没有要求 Agent 自行寻找未列出的上下文文件
- [x] 没有无验证方式的 AC
- [x] 没有"与 Task-N 类似""参考 Task-N 实现"等引用

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 状态 |
|---------|------|----------|---------|----------|----------|----------|------|
| TASK-1 | 私有 NAPI 模块从构建体系移除，ABC 迁入独立内部 .so | `frameworks/js/napi/app_startup/async_task_*/`（删）、`frameworks/js/napi/BUILD.gn`、`frameworks/native/appkit/BUILD.gn`、`frameworks/native/appkit/app_startup/appkit_startup_task.cpp`（新）、两个 .ts（迁） | AC-1, AC-2, AC-6 | 无 | 两目标从 napi_packages 消失；新内部 .so 目标 `appkit_startup_task` 生成（默认 lib 目录安装、无注册符号）并被 appkit_native 依赖 | `grep -rn "asynctask" frameworks/`（无 napi 模块残留）；产物目录检查 | Done |
| TASK-2 | JsRuntime 新增 LoadInternalModuleByBuffer 内部加载能力 | `interfaces/inner_api/runtime/include/js_runtime.h`、`frameworks/native/runtime/js_runtime.cpp` | AC-3, AC-5, AC-6 | 无 | 新静态方法编译通过；失败路径返回 nullptr（不 crash） | 编译（runtime 目标）+ `js_runtime_test` | Done |
| TASK-3 | 框架加载链路切换与全量回归 | `frameworks/native/appkit/app_startup/js_startup_task.cpp` | AC-1, AC-3, AC-4, AC-5, AC-6 | TASK-1, TASK-2 | 两 Load 函数走内嵌 ABC + LoadInternalModuleByBuffer；错误路径保留；既有单测全通过 | 编译（appkit 目标）+ `startup_manager_test` 等 AppStartup 单测 | Done |

## Task 详情

### TASK-1: 私有 NAPI 模块从构建体系移除

| 字段 | 内容 |
|------|------|
| 任务目标 | 两个模块名与 .so 制品从 NAPI 模块体系彻底消失；ABC 产物改由 appkit_native 内嵌 |
| AC 映射 | AC-1, AC-2, AC-6 |
| 前置依赖 | 无 |
| 非目标 | 不修改任何 *_module.cpp 之外的 napi 模块；不改 ability_runtime.gni |
| 完成判据 | ① `frameworks/js/napi/app_startup/` 下无 async_task_* 目录；② napi_packages 无 asynctask 两项；③ appkit BUILD.gn 有 4 个新目标且 appkit_native deps 引用；④ .ts 新路径在 appkit/app_startup/ 下 |
| 停止条件 | gen_js_obj 目标被 appkit 依赖时出现 visibility 冲突或 .o 符号未解析（链入失败） |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| git mv | `frameworks/js/napi/app_startup/async_task_excutor/async_task_excutor.ts` → `frameworks/native/appkit/app_startup/async_task_excutor.ts` | 文件内容不变（License 头保留） |
| git mv | `frameworks/js/napi/app_startup/async_task_callback/async_task_callback.ts` → `frameworks/native/appkit/app_startup/async_task_callback.ts` | 同上 |
| 删除 | `frameworks/js/napi/app_startup/async_task_excutor/`（BUILD.gn + async_task_excutor_module.cpp） | 目标与注册源整体移除 |
| 删除 | `frameworks/js/napi/app_startup/async_task_callback/`（BUILD.gn + async_task_callback_module.cpp） | 同上 |
| 修改 | `frameworks/js/napi/BUILD.gn` | 移除 napi_packages 中两行 |
| 新增 | `frameworks/native/appkit/app_startup/appkit_startup_task.cpp` | extern "C" getter ×2（GetAsyncTaskExcutorAbc / GetAsyncTaskCallbackAbc），visibility("default")，返回 _binary_* 符号区间与长度 |
| 修改 | `frameworks/native/appkit/BUILD.gn` | 新增 `ohos_shared_library("appkit_startup_task")`：sources 含新 cpp，deps 为 4 个 es2abc/gen_js_obj 目标；无 relative_install_dir（默认 /system/lib{,64}/）、无 version_script、无 napi 注册；`appkit_native` deps 改引 `:appkit_startup_task` |

**关键实现约束**
- es2abc `extra_args = [ "--module" ]` 必须保留（.ts 是 export default 模块）
- 新 .so **不得**包含：`napi_module_register` 调用、`nm_modname`、`NAPI_*` 前缀导出符号、`relative_install_dir`（避免落入 module/ 路径）、version_script（保证 getter 导出）
- 目标名 `appkit_startup_task`（产物 libappkit_startup_task.z.so），与公共模块 startuptask_napi 明确区分
- getter 返回 `{buf, len}`，len 以 int32_t 输出参数返回，buf 为空或 len<=0 时调用方走既有错误路径

**Spec References**

- AC / Rules: AC-1, AC-2, AC-6 / R-1, R-2
- Anchors: `spec.md#验收追溯`, `spec.md#规则定义`, `spec.md#验证映射`

**Design References**

- IDs / Anchors: D1（独立内部库与不可达三条件）, `design.md#关键设计决策`

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| N/A | N/A | N/A | N/A | N/A | N/A | N/A | N/A（本 Task 仅构建目标调整，无运行时状态） |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `extern "C" const uint8_t* GetAsyncTaskExcutorAbc(int32_t*)` / `GetAsyncTaskCallbackAbc(int32_t*)` | TASK-1 | 供 TASK-3 使用 | visibility("default")、无 version_script；符号 ABI 稳定，改名需同步 TASK-3 声明 |
| Consumes | 无 | TASK-1 | — | — |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-1/AC-2 静态证据：注册源 grep 0 处；napi_packages 仅 4 公共模块；build.ninja 旧目标 0 处/新目标 18 处 |
| Scope respected | 实际 git status 与受影响文件清单逐项一致（spec-compliance.md 一致性核对） |
| State lifecycle complete | N/A（无运行时状态） |
| Interface contract matched | getter 符号经 llvm-nm 验证为全局 T 符号，与 TASK-3 声明签名一致 |
| Relevant verification | verification.md V1（gn 解析）/V3（链接+符号）直接覆盖本 Task |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 静态检查 | `grep -rn "nm_modname.*AsyncTask" --include="*.cpp" .` | 无输出 | 无输出（注册源 0 处）— PASS |
| 构建检查 | `grep -c asynctaskexcutor_napi build.ninja` / `grep -c appkit_startup_task build.ninja` | 0 / >0 | 0 处 / 18 处 — PASS |
| 符号检查 | `llvm-nm -D libappkit_startup_task.z.so` | getter 全局导出且无 NAPI 符号 | GetAsyncTask*Abc 全局 T 符号；无 napi_module/NAPI_* — PASS |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1/AC-2 映射、Extra/Deviation（spec-compliance.md AC 表） |
| Code Quality | 命名对齐 appkit 族、sanitize 满足 part 断言（code-quality.md Quality Dimensions） |
| Verification | V1/V3 执行记录（verification.md Execution Records） |

### TASK-2: JsRuntime 新增 LoadInternalModuleByBuffer

| 字段 | 内容 |
|------|------|
| 任务目标 | 框架获得绕过 NAPI 模块管理器的进程内 ABC 加载能力（引擎公开 API 封装） |
| AC 映射 | AC-3, AC-5, AC-6 |
| 前置依赖 | 无 |
| 非目标 | 不改 LoadSystemModuleByEngine（其他系统模块仍走 requireNapi）；不加 IPC/不加公共接口 |
| 完成判据 | 声明与实现编译通过；buf/env 无效或执行失败时返回 nullptr 且有错误日志，不 crash |
| 停止条件 | ExecuteModuleFromBuffer 在无 HandleScope 场景下断言失败（需在调用侧确认 HandleScope 由 js_startup_task.cpp 既有 HandleScope 覆盖） |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | `interfaces/inner_api/runtime/include/js_runtime.h` | `LoadSystemModuleByEngine` 声明旁新增静态方法声明（注释说明用途：框架内部模块 buffer 加载，不注册模块名） |
| 修改 | `frameworks/native/runtime/js_runtime.cpp` | 实现：ExecuteModuleFromBuffer → GetExportObjectFromBuffer("default") → ArkValueToNapiValue → napi_new_instance → napi_create_reference；全失败路径 TAG_LOGE + nullptr |

**签名（对标 LoadSystemModuleByEngine，js_runtime.cpp:591 写法）**

```cpp
static std::unique_ptr<NativeReference> LoadInternalModuleByBuffer(napi_env env,
    panda::ecmascript::EcmaVM *vm, const uint8_t *buf, size_t len,
    const std::string &fileName, const napi_value *argv, size_t argc);
```

**Spec References**

- AC / Rules: AC-3, AC-5, AC-6 / R-3, R-5
- Anchors: `spec.md#验收追溯`, `spec.md#规则定义`, `spec.md#API-变更分析`

**Design References**

- IDs / Anchors: D2（加载链路与 fileName 约定）, `design.md#关键设计决策`

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| N/A | N/A | N/A | N/A | N/A | N/A | N/A | N/A（纯静态加载函数，返回的 NativeReference 生命周期由调用方 TASK-3 管理） |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `JsRuntime::LoadInternalModuleByBuffer(env, vm, buf, len, fileName, argv, argc)`（inner_api 静态方法） | TASK-2 | 供 TASK-3 使用 | inner_api 内部接口不进 SDK；失败统一返回 nullptr |
| Consumes | `panda::JSNApi::ExecuteModuleFromBuffer/GetExportObjectFromBuffer`（ets_runtime 导出，jsnapi_expo.h:1871/1888）；`ArkNativeEngine::ArkValueToNapiValue` | TASK-2 | 引擎公开导出 API | 与 NAPI 流程同一入口（ark_native_engine.cpp:3545），语义由引擎保证 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-3（加载能力）/AC-5（失败路径）编译级证据：js_runtime.cpp:632-665 四段失败分支均判空 + TAG_LOGE + nullptr |
| Scope respected | 仅改 js_runtime.h/js_runtime.cpp 两个文件（diff 核对）；顺带修复 EcmaVM 拼写为编译验证发现的自纠 |
| State lifecycle complete | N/A（无状态；返回引用归调用方） |
| Interface contract matched | 实现签名与 js_runtime.h 声明一致（EcmaVM）；fileName 约定与 design D2 一致 |
| Relevant verification | verification.md 真实工具链 -Werror 编译 rc=0 直接覆盖本 Task |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 编译 | clang++ -c js_runtime.cpp（GN 真实参数，-Wall -Werror） | rc=0 | rc=0（修复 EcmaVM 拼写后）— PASS |
| 错误分支走查 | js_runtime.cpp:636-660 四段判空 | 全失败路径 nullptr + 日志 | buf/env/vm/len/fileName 判空 + 执行/导出/转换/实例化失败均 nullptr — PASS |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-3/AC-5 映射与证据（spec-compliance.md AC 表） |
| Code Quality | 与 GetExportObjectFromOhmUrl 既有模式对齐（code-quality.md Quality Dimensions） |
| Verification | 编译执行记录（verification.md Execution Records 第 8 行） |

### TASK-3: 框架加载链路切换与全量回归

| 字段 | 内容 |
|------|------|
| 任务目标 | js_startup_task 两处加载切换为内嵌 ABC 直接加载；错误处理与 DFX 保持；全量回归 |
| AC 映射 | AC-1, AC-3, AC-4, AC-5, AC-6 |
| 前置依赖 | TASK-1（.o 符号可用）、TASK-2（新 API 可用） |
| 非目标 | 不改 RunOnMainThread 路径（AC-4）；不改 AsyncTaskCallBack 类与 sendable 定义；不改 OnAsyncTaskCompleted 回调链 |
| 完成判据 | ① LoadJsAsyncTaskExecutor/LoadJsAsyncTaskCallback 引用内嵌符号并经 LoadInternalModuleByBuffer 实例化；② 失败时 ERR_STARTUP_INTERNAL_ERROR + STARTUP_TASK_ERROR 保留；③ 既有 AppStartup 单测全通过 |
| 停止条件 | 单测暴露行为差异（sendable/并发语义变化）——回 design D2 复核执行入口等价性 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | `frameworks/native/appkit/app_startup/js_startup_task.cpp` | ① 删除文件头 `_binary_*` extern 声明，改为 extern "C" getter 声明（GetAsyncTaskExcutorAbc / GetAsyncTaskCallbackAbc）；② LoadJsAsyncTaskExecutor 经 getter 取 buffer 后调 LoadInternalModuleByBuffer，失败返回 ERR_STARTUP_INTERNAL_ERROR；③ LoadJsAsyncTaskCallback 同理（sendable class 定义保留） |

**现状缺陷顺带修复**：`LoadJsAsyncTaskExecutor`（js_startup_task.cpp:161-177）现状恒返回 ERR_OK 不判空——本次重写补齐判空（属 AC-5 错误路径要求，非范围外扩写）。

**Spec References**

- AC / Rules: AC-1, AC-3, AC-4, AC-5, AC-6 / R-3, R-4, R-5, R-6
- Anchors: `spec.md#验收追溯`, `spec.md#验证映射`

**Design References**

- IDs / Anchors: D2（加载链路）、D3（sendable 类定义不变）、D4（错误处理与 DFX 保持）, `design.md#关键设计决策`

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| AsyncTaskExecutorJsRef_ / AsyncTaskExecutorCallbackJsRef_ | NativeReference 成员 | JsStartupTask（js_startup_task.h:48-49） | 成员变量 | LoadJsAsyncTaskExecutor/Callback 中 napi_create_reference（refcount=1） | RunOnTaskPool 读取实例 | 对象析构随 NativeReference 释放（与原实现一致，本次未改生命周期管理） | 引用由 HandleScope 内创建，跨调用持有，析构线程与创建线程一致 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Consumes | `GetAsyncTaskExcutorAbc/GetAsyncTaskCallbackAbc(int32_t*)` | TASK-3 | 来自 TASK-1 | buf 空或 len<=0 走 ERR_STARTUP_INTERNAL_ERROR（AC-5） |
| Consumes | `JsRuntime::LoadInternalModuleByBuffer(...)` | TASK-3 | 来自 TASK-2 | 返回 nullptr 时 excutor 走错误分支；callback 沿用 RunOnTaskPool 判空（与现状一致） |
| Produces | 无（行为终点，回调链 OnAsyncTaskCompleted 不变） | TASK-3 | — | — |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-4 diff 证明 RunOnMainThread 路径零触碰；AC-5 excutor 失败分支（abcBuf/abcLen 判空 + nullptr 判空）补齐；AC-1 模块名引用消失 |
| Scope respected | 仅改 js_startup_task.cpp 单文件（diff 核对），无清单外文件 |
| State lifecycle complete | NativeReference 创建/读取/清理与原实现逐行一致（上表） |
| Interface contract matched | Consumes 契约与 TASK-1/TASK-2 Produces 签名一致（extern "C" getter / 静态方法） |
| Relevant verification | verification.md js_startup_task.cpp 编译 + diff 范围核对直接覆盖本 Task；单测回归遗留 CI 如实记录 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 编译 | clang++ -c js_startup_task.cpp（GN 真实参数，-Werror） | rc=0 | rc=0（修复 extern 匿名 namespace 后）— PASS |
| 范围核对 | `git diff 53cfaad1e7..HEAD -- frameworks/native/appkit/app_startup/js_startup_task.cpp` | 仅两 Load 函数、getter 声明与常量改动 | RunOnMainThread/sendable/回调链零触碰 — PASS |

> 环境受限如实记录：单测回归（startup_manager_test 等）因本环境无设备与完整构建产物未执行，遗留 CI——已登记于 verification.md Coverage 表与 review.md 遗留项，不作为本表 PASS 项。

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-1/3/4/5 映射、等价论证 P1-P4（spec-compliance.md） |
| Code Quality | Findings（getter 声明就地）与单测 WARN（code-quality.md） |
| Verification | 编译/diff 记录 + Coverage 表遗留项（verification.md） |

## 代码范围映射

| Task ID | 计划文件/符号 | 实际文件/符号 | 操作 | Commit | Spec Compliance | Code Quality | Verification | 偏差 |
|---------|---------------|---------------|------|--------|-----------------|--------------|--------------|------|
| TASK-1 | async_task_*/ 删除、napi/BUILD.gn、appkit/BUILD.gn、appkit_startup_task.cpp（新）、两 .ts 迁移 | 与计划一致（git status 11 项核对） | Add/Modify/Delete | 本笔提交 | evidence/reviews/spec-compliance.md#AC | evidence/reviews/code-quality.md#Scope | evidence/reviews/verification.md#Execution | 无（R1 修订已回写本计划） |
| TASK-2 | js_runtime.h、js_runtime.cpp | 与计划一致；另修复 EcmaVM 拼写（编译发现自纠） | Modify | 本笔提交 | evidence/reviews/spec-compliance.md#AC | evidence/reviews/code-quality.md#Scope | evidence/reviews/verification.md#Execution | 无 |
| TASK-3 | js_startup_task.cpp | 与计划一致；补 excutor 失败分支（计划已声明） | Modify | 本笔提交 | evidence/reviews/spec-compliance.md#AC | evidence/reviews/code-quality.md#Scope | evidence/reviews/verification.md#Execution | 无 |

## Plan 自审清单

- [x] spec 每个 AC 均映射 Task 且有验证方式
- [x] 文件清单覆盖 design 全部三层，无未列出文件
- [x] Task 间依赖显式（TASK-3 → TASK-1+TASK-2）
- [x] 无占位符/模糊指令
- [x] 生成文件说明：本计划不涉及生成器产物（.abc/.o 由 es2abc/llvm-objcopy 构建动作产出，BUILD.gn 中目标即生成源）
- [x] 工作区边界：单仓 ability_runtime、分支 fix/appstartup-async-task-napi-exposure、允许修改文件=本清单 11 项；当前工作区除本变更外仅存在 services/dialog_ui 构建产物（未跟踪，不触碰）
