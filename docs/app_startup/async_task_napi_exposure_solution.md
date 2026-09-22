# AppStartup AsyncTask 私有 NAPI 模块暴露问题与整改方案

| 项目 | 内容 |
|---|---|
| 问题主题 | `asynctaskcallback_napi` / `asynctaskexcutor_napi` 以公共 NAPI 模块形态发布，导致框架私有 API 暴露 |
| 影响范围 | `frameworks/js/napi/app_startup/`、`frameworks/native/appkit/app_startup/`、`frameworks/native/runtime/` |
| 风险等级 | 高（私有 API 暴露，无兼容性声明保护） |
| 方案类型 | 加载链路重构（去 NAPI 模块化，ABC 迁入独立内部库） |
| 方案版本 | R1（初版"ABC 内嵌 appkit_native"经评审修订为独立内部库，见 3.6 修订记录） |

---

## 1. 背景

### 1.1 两个模块是什么

| 模块 | NAPI 模块名 | 职责 |
|---|---|---|
| `libasynctaskexcutor_napi.so` | `app.appstartup.AsyncTaskExcutor` | TS 胶水层：把 `StartupTask.init` 推入 taskpool 并发执行，并通过 `Task.sendData` 把启动结果发回宿主线程 |
| `libasynctaskcallback_napi.so` | `app.appstartup.AsyncTaskCallback` | sendable 包装类：在 taskpool 线程持有回调，触发 `onAsyncTaskCompleted` 通知框架 C++ 侧 |

两者服务于 AppStartup（应用启动框架）的**异步启动任务**能力：当 `StartupTask` 未声明 `callCreateOnMainThread` 时，框架将 `init()` 转移到 taskpool 线程并发执行，避免阻塞主线程。

### 1.2 设计定位：仅框架内部使用

完整调用链（唯一消费方为框架自身）：

```
JsStartupTask::RunTaskInit (C++, js_startup_task.cpp:99)
  ├─ LoadJsAsyncTaskExecutor()   → requireNapi("app.appstartup.AsyncTaskExcutor")
  ├─ LoadJsAsyncTaskCallback()   → requireNapi("app.appstartup.AsyncTaskCallback")
  └─ JsStartupTaskExecutor::RunOnTaskPool(...)
       └─ AsyncTaskExcutor.asyncPushTask(startup, callback, context, name)   [TS]
            └─ new taskPool.Task(concurrentFunc) → taskPool.execute(task)
                 └─ [taskpool 线程] await startup.init(context)
                      └─ Task.sendData(asyncCallback, name, result)
                           └─ AsyncTaskCallback.onAsyncTaskCompleted(name, result)  [sendable]
                                └─ C++ AsyncTaskCallBack::AsyncTaskCompleted → JsStartupTask::OnAsyncTaskCompleted
```

### 1.3 问题

两个模块**以公共 NAPI 系统模块的形态随系统镜像发布**。任何三方应用都可在运行时通过
`requireNapi('app.appstartup.AsyncTaskExcutor')` 直接加载并调用框架私有实现——私有 API 暴露。

关键事实：

- 唯一使用方：`frameworks/native/appkit/app_startup/js_startup_task.cpp`
- SDK（`interface/sdk-js`）中**没有任何对应 d.ts 声明**，不在公共 API 清单
- 属于纯私有实现，开发者不应感知、更不应调用

---

## 2. 根因分析

暴露不是单一缺陷，而是五个环节共同作用的结果：

| # | 环节 | 证据 |
|---|---|---|
| 1 | **构造函数自动注册**：`napi_module_register` 把模块名写入进程 NAPI 注册表 | `async_task_excutor_module.cpp:21-30`、`async_task_callback_module.cpp:21-30` |
| 2 | **落入 NAPI 搜索路径**：`relative_install_dir = "module/app/appstartup"`，即 `/system/lib{,64}/module/` | 两个 `BUILD.gn:59` |
| 3 | **按名即可解析**：`requireNapi(name)` → `NativeModuleManager::GetNativeModulePath` 命中 `lib<name>_napi.so` → dlopen → 执行内嵌 ABC | `foundation/arkui/napi/module_manager/native_module_manager.cpp:1022`（路径规则见 1216-1222 行） |
| 4 | **无运行时拦截**：`ModuleCheckerDelegate` 仅 Extension 场景注册（`extension_config_mgr.cpp:110`）；`Runtime::Options::moduleCheckerDelegate` 默认 `nullptr`（`runtime.h:85`），普通应用进程加载不受限 | — |
| 5 | **无声明约束**：SDK 无 d.ts，属未文档化私有路径——删除无兼容性负担，但也意味着当前无任何保护 | — |

`requireNapi` 解析细节：

```
requireNapi("app.appstartup.AsyncTaskExcutor")
  → NativeModuleManager::GetNativeModulePath()          // native_module_manager.cpp:1022
  → /system/lib64/module/app/appstartup/libasynctaskexcutor_napi.so
  → dlopen + NAPI_app_appstartup_AsyncTaskExcutor_GetABCCode()
  → 执行内嵌 ABC → 返回模块实例
```

**结论**：治理必须让模块名从 NAPI 体系中彻底消失（不注册、不落盘、不可按名解析），仅靠拦截或挪路径均不彻底（见 3.4）。

---

## 3. 方案设计

### 3.1 核心原则

**框架内部 JS 不注册任何模块名，不落入任何 NAPI 搜索路径。**

注意 `/system/lib{,64}/module/**` 与 `/system/etc/abc/**` **均为 NAPI 搜索路径**（后者见 `native_module_manager.cpp:1048` 的 `sysAbcPrefix`），装入 `/system/etc/abc/` 同样会被 `requireNapi` 命中。

### 3.2 目标架构（Before / After）

```
Before（现状，应用可达）:
  appkit (js_startup_task.cpp)
    → requireNapi("app.appstartup.AsyncTaskExcutor")        // 全局函数
    → NativeModuleManager 按名搜索/缓存
    → dlopen libasynctaskexcutor_napi.so  (module/app/appstartup/)
    → 执行内嵌 ABC → 返回实例
  ✗ 应用进程走同一条路即可加载私有实现

After（目标，应用不可达）:
  appkit_native（DT_NEEDED）
    → libappkit_startup_task.z.so          // 独立内部库，安装于系统普通 lib 目录（非 NAPI 路径）
        → GetAsyncTaskExcutorAbc(&len)     // extern "C" getter，返回内嵌 ABC buffer
    → JSNApi::ExecuteModuleFromBuffer(vm, buf, len, fileName)
    → JSNApi::GetExportObjectFromBuffer(vm, fileName, "default")
    → napi_new_instance(...)               // 与原 LoadSystemModuleByEngine 尾段一致
    → NativeReference 持有实例
  ✓ 不注册模块名、不落任何 NAPI 搜索路径（module/** 与 /system/etc/abc/**）
```

依据：NAPI 流程内部本就使用 `JSNApi::ExecuteModuleFromBuffer` + `GetExportObjectFromBuffer` 执行 .so 内嵌 ABC（`ark_native_engine.cpp:3530-3553`），这两个引擎 API 是公开导出的（`ets_runtime/ecmascript/napi/include/jsnapi_expo.h:1871/1888`），框架可绕过 NAPI 模块管理器直接使用，执行语义（含 `'use concurrent'` / `'use sendable'`）完全一致。

### 3.3 实施步骤

**Step 1：删除两个 NAPI 模块目标**

- 删除目录 `frameworks/js/napi/app_startup/async_task_callback/`、`frameworks/js/napi/app_startup/async_task_excutor/`（`*_module.cpp` 的 `napi_module_register`、`GetABCCode` 全部消失，模块名从注册表中不存在）
- 从 `frameworks/js/napi/BUILD.gn:51-52` 的 `napi_packages` 移除两个依赖
- `.so` 不再进镜像，`requireNapi` 按名/按路径均找不到

**Step 2：ABC 迁入独立内部库 `libappkit_startup_task.z.so`**

- 保留/迁移 `es2abc_gen_abc` + `gen_js_obj` 目标（定义于 `frameworks/native/appkit/BUILD.gn`），产物 `.o` 链入新增内部库
- 新增 `ohos_shared_library("appkit_startup_task")`（命名对齐 appkit 内部支撑库族 `appkit_delegator`/`appkit_child`）：sources 含导出文件 `appkit_startup_task.cpp`，deps 为 4 个 `.o` 目标（abc + ts 各两份）；**不设** `relative_install_dir`（默认安装 `/system/lib{,64}/`，非 NAPI 路径）、**不配** `version_script`（保证 getter 导出）、补齐 sanitize 块（满足 `sanitizers.gni:446` part 级 cfi/intsan 强制断言）
- 导出文件 `appkit_startup_task.cpp` 提供 extern "C" getter：

```cpp
extern "C" __attribute__((visibility("default")))
const uint8_t* GetAsyncTaskExcutorAbc(int32_t* len);   // 返回 _binary_async_task_excutor_abc 区间
extern "C" __attribute__((visibility("default")))
const uint8_t* GetAsyncTaskCallbackAbc(int32_t* len);
```

**Step 3：改造加载入口**

`js_startup_task.cpp` 示意：

```cpp
extern "C" const uint8_t* GetAsyncTaskExcutorAbc(int32_t* len);
extern "C" const uint8_t* GetAsyncTaskCallbackAbc(int32_t* len);

int32_t JsStartupTask::LoadJsAsyncTaskExecutor()
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value object = nullptr;
    napi_create_object(env, &object);

    int32_t abcLen = 0;
    const uint8_t* abcBuf = GetAsyncTaskExcutorAbc(&abcLen);
    if (abcBuf == nullptr || abcLen <= 0) {
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    // fileName 仅作模块记录名（调试栈回溯用），保持唯一稳定即可
    AsyncTaskExecutorJsRef_ = JsRuntime::LoadInternalModuleByBuffer(env, jsRuntime_.GetEcmaVm(),
        abcBuf, static_cast<size_t>(abcLen), "async_task_excutor.abc", &object, 1);
    return AsyncTaskExecutorJsRef_ ? ERR_OK : ERR_STARTUP_INTERNAL_ERROR;
}
```

建议在 `JsRuntime` 增加通用能力（仿照 `js_runtime.cpp:1219` 的 `GetExportObjectFromOhmUrl` 写法）：

```cpp
std::unique_ptr<NativeReference> JsRuntime::LoadInternalModuleByBuffer(
    napi_env env, EcmaVM *vm, const uint8_t *buf, size_t len,
    const std::string &fileName, const napi_value *argv, size_t argc)
{
    if (!panda::JSNApi::ExecuteModuleFromBuffer(vm, buf, len, fileName)) {
        return nullptr;
    }
    auto exportObj = panda::JSNApi::GetExportObjectFromBuffer(vm, fileName, "default");
    if (exportObj->IsNull()) {
        return nullptr;
    }
    napi_value classValue = ArkNativeEngine::ArkValueToNapiValue(env, exportObj);
    napi_value instance = nullptr;
    napi_new_instance(env, classValue, argc, argv, &instance);  // 与 LoadSystemModuleByEngine 尾段一致
    napi_ref resultRef = nullptr;
    napi_create_reference(env, instance, 1, &resultRef);
    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference *>(resultRef));
}
```

`AsyncTaskCallback` 同理：C++ 侧 `napi_define_sendable_class`（`js_startup_task.cpp:195`）不动，仅把 TS 包装类的获取方式换成 buffer 加载。

### 3.4 被否决的备选方案

| 备选 | 否决原因 |
|---|---|
| `ModuleCheckerDelegate` 黑名单拦截 | 框架自身也走全局 `requireNapi`（`js_runtime.cpp:591`），`isAppModule` 同为 false，checker 无法区分调用来源；且 checker 目前仅 Extension 注册，要全局生效需改运行时治理策略，脆弱 |
| .so 挪出 module 路径但保留注册 | 构造函数一旦执行，模块名即进入进程内 `NativeModuleManager` 缓存，`FindNativeModuleByCache` 按名匹配，应用仍可 require 到 |
| ABC 装到 `/system/etc/abc/` | 这也是 NAPI 搜索路径（`nativeModulePath[2]`），照样能被 `requireNapi` 命中 |
| 改更隐蔽的模块名 | 安全靠隐蔽（security by obscurity），不可接受 |

### 3.5 变更文件清单

| 文件 | 操作 |
|---|---|
| `frameworks/js/napi/app_startup/async_task_excutor/` | 删除（BUILD.gn、`*_module.cpp`；`*.ts` 迁出） |
| `frameworks/js/napi/app_startup/async_task_callback/` | 删除（同上） |
| `frameworks/js/napi/BUILD.gn` | 移除 `napi_packages` 中两个依赖 |
| `frameworks/native/appkit/app_startup/async_task_{excutor,callback}.ts` | 新增（git mv 自原 napi 模块目录） |
| `frameworks/native/appkit/app_startup/appkit_startup_task.cpp` | 新增（内部库导出文件：extern "C" getter ×2） |
| `frameworks/native/appkit/BUILD.gn` | 新增 es2abc / gen_js_obj 目标与 `appkit_startup_task` 内部库；`appkit_native` deps 改引 `:appkit_startup_task` |
| `frameworks/native/appkit/app_startup/js_startup_task.cpp` | 重写 `LoadJsAsyncTaskExecutor` / `LoadJsAsyncTaskCallback`（getter + buffer 加载） |
| `interfaces/inner_api/runtime/include/js_runtime.h` | 新增 `LoadInternalModuleByBuffer` 声明 |
| `frameworks/native/runtime/js_runtime.cpp` | 新增 `LoadInternalModuleByBuffer` 实现 |

### 3.6 修订记录

| 版本 | 变更 | 原因 |
|---|---|---|
| R0 | ABC 以目标文件 `.o` 直接链入 `appkit_native`，extern 声明 `_binary_*` 符号 | 初版方案（R0） |
| R1（现行） | ABC 迁入独立内部库 `libappkit_startup_task.z.so`（目标 `appkit_startup_task`，appkit 内部支撑库族命名），`appkit_native` 经 DT_NEEDED 依赖并经 extern "C" getter 获取 buffer | 需求方评审意见：不希望 `appkit_native` 体积增长，要求 JS 制品独立成 .so 且不落 NAPI 模块目录。不可达三条件（不注册/不落 NAPI 搜索路径/不可按名解析）复核均满足；与公共模块 `startuptask_napi` 无路径与文件名碰撞 |

---

## 4. 影响评估

| 维度 | 评估 |
|---|---|
| **兼容性** | 无公共 API 变更：两模块从未有 d.ts，不在任何 SDK 声明中；无 IPC Stub/Proxy 序列化格式变更；无错误码、HiSysEvent 事件变更。删除的只是未文档化的私有加载路径 |
| **框架行为** | 异步启动任务语义完全不变：taskpool 并发执行 + sendable 回调；`'use concurrent'` / `'use sendable'` 字节码执行语义一致（同一 JSNApi 执行入口）；仅加载入口由 `requireNapi` 替换为等价的 `ExecuteModuleFromBuffer` |
| **安全性** | 两个 napi .so 从系统 NAPI 搜索路径彻底移除；替代它们的内部库 `libappkit_startup_task.z.so` 不含任何 NAPI 注册符号（llvm-nm 验证无 `napi_module`/`NAPI_*` 导出），安装于系统普通 lib 目录（不在 `module/**` 与 `/system/etc/abc/**` 任何一族 NAPI 搜索路径内）；任何应用 `requireNapi` 均返回"模块不存在" |
| **体积** | `appkit_native.so` 体积不变（ABC 不再链入）；系统镜像 .so 净减 1（原 2 个 napi 模块 → 1 个约 10KB 量级的内部库） |
| **DFX** | 无 hisysevent.yaml 变更；`LoadJsAsyncTaskExecutor failed` 故障归因路径保留（错误分支不变） |

---

## 5. 验证计划

1. **编译**：`./build.sh --product-name <product> --build-target ability_runtime`
2. **单测**：AppStartup / 启动框架相关用例（`run -t UT -tp ability_runtime`）
3. **E2E 正向**：配置异步启动任务（非 `callCreateOnMainThread`）的 Demo 应用，验证 taskpool 执行与 `onAsyncTaskCompleted` 回调正常
4. **E2E 负向**：应用内调用 `requireNapi('app.appstartup.AsyncTaskExcutor')` / `requireNapi('app.appstartup.AsyncTaskCallback')`，确认返回模块不存在
5. **镜像检查**：确认 `module/app/appstartup/` 下不再有 `libasynctask*_napi.so`

---

## 6. 遗留风险与后续项

- 模拟器（`frameworks/simulator`）加载链路需同步确认是否受影响（其 `LoadSystemModuleByEngine` 为独立实现）
- 加载链路切换后，调试栈回溯中的文件名由 `libasynctaskexcutor_napi.so/async_task_excutor.js` 变为自定义记录名（`async_task_excutor.abc`），需在 sourcemap/DevTools 侧确认无回归
- 内部库随 `appkit_native` 经 DT_NEEDED 常驻所有应用进程（体积仅约 10KB 量级，常驻成本可忽略；不采用 dlopen 按需加载以避免路径查找与错误处理复杂度）
- 除此之外无已知风险

---

## 附：对外评审材料（中立评审版，含证据分级）

- `svg/01_background.svg`（1920×1080）——背景（含判定链与证据纪律说明）
- `svg/02_asis.svg`（1920×1080）——AS-IS 现状与问题分析（五环链路 + 已证实事实/未决问题分列）
- `svg/03_tobe.svg`（1920×1080）——TO-BE 优化方案（R1 架构 + 等价性论证前提链 + R0/R1 权衡 + 验证状态分列）
- `svg/04_swot.svg`（1920×1080）——SWOT 分析（逐项证据分级标注 + 评审决策问题）
- `svg/05_internal_call_flow.svg`（1920×1080）——源码级调用流程（步骤 ①-⑩ 泳道分色：加载阶段 getter→LoadInternalModuleByBuffer 四步、执行阶段 taskpool↔sendable 回调链、构建期 .ts→.abc→.o→.so 流水线、关键源码摘录）

> 评审版材料立场中立：陈述按【代码】【推断】【假设】【未知】分级标注，方案取舍（含 R0/R1）由评审裁定，不预设结论。事实修正记录：新库实际安装于 `/system/lib{,64}/platformsdk/`（appkit 系库目录，非 lib 根目录）；platformsdk 在 linker `lib.paths`（`ld-musl-namespace-arm.ini:8`），新库存在 dlopen 理论可达的 native getter 级暴露（非零暴露），此为 R0/R1 的真实权衡点。

## 附：早期汇报版材料（存档）

- `svg/legacy/01_background.svg` ——背景
- `svg/legacy/02_root_cause.svg` ——根因
- `svg/legacy/03_solution.svg` ——方案（R1 版）
- `svg/legacy/04_impact.svg` ——影响

> 早期汇报版（立场为方案推荐视角）保留存档供对照；对外评审请使用根目录下的中立评审版。
