# 架构设计：AppStartup AsyncTask 私有 NAPI 模块暴露治理

> 对应 spec.md 的 AC-1 ~ AC-6。本设计的完整前置分析已由需求方评审合入：`docs/app_startup/async_task_napi_exposure_solution.md`（随本笔提交）。

## 需求基线

继承 proposal.md 基线：两模块从 NAPI 体系彻底移除（AC-1、AC-2），异步启动任务行为不变（AC-3、AC-4、AC-5），工程不回归（AC-6）。范围：ability_runtime 单仓；N/A 维度见 proposal。

## 上下文和现状

### 现状加载链路（Before）

```
JsStartupTask::RunTaskInit (frameworks/native/appkit/app_startup/js_startup_task.cpp:99)
  ├─ LoadJsAsyncTaskExecutor()  → JsRuntime::LoadSystemModuleByEngine("app.appstartup.AsyncTaskExcutor")
  ├─ LoadJsAsyncTaskCallback()  → JsRuntime::LoadSystemModuleByEngine("app.appstartup.AsyncTaskCallback")
  │     └─ LoadSystemModuleByEngine = 全局 requireNapi（frameworks/native/runtime/js_runtime.cpp:591）
  │           └─ NativeModuleManager 按名解析 → dlopen libasynctask*_napi.so → 执行内嵌 ABC
  └─ JsStartupTaskExecutor::RunOnTaskPool(asyncTaskExcutor 实例, asyncTaskCallback 实例, ...)
```

### 暴露链路（根因，五环）

1. **注册**：`async_task_{excutor,callback}_module.cpp:21-30` 构造函数 `napi_module_register`，模块名入进程注册表
2. **安装**：两 BUILD.gn `relative_install_dir = "module/app/appstartup"` → `/system/lib{,64}/module/`（NAPI 搜索路径）
3. **解析**：`NativeModuleManager::GetNativeModulePath`（`foundation/arkui/napi/module_manager/native_module_manager.cpp:1022`，规则 1216-1222）按名命中 `lib<name>_napi.so` → dlopen → `NAPI_..._GetABCCode()` → 执行内嵌 ABC
4. **无拦截**：`ModuleCheckerDelegate` 仅 Extension 场景注册（`extension_config_mgr.cpp:110`）；`Runtime::Options::moduleCheckerDelegate` 默认 nullptr（`runtime.h:85`）
5. **无声明**：interface/sdk-js 无 AsyncTask d.ts → 纯私有路径，无保护也无兼容负担

### 关键现状事实（已验证）

| 事实 | 证据 |
|------|------|
| 唯一消费方是 `js_startup_task.cpp` | 全仓 grep `app.appstartup.AsyncTask` |
| NAPI 流程执行 ABC 的真实入口是 `JSNApi::ExecuteModuleFromBuffer` + `GetExportObjectFromBuffer` | `foundation/arkui/napi/native_engine/impl/ark/ark_native_engine.cpp:3530-3553`（LoadArkModule） |
| 上述两 API 为 ets_runtime 公开导出 | `arkcompiler/ets_runtime/ecmascript/napi/include/jsnapi_expo.h:1871/1888` |
| Local<ObjectRef> → napi_value 转换先例 | `JsRuntime::GetExportObjectFromOhmUrl`（`frameworks/native/runtime/js_runtime.cpp:1219`，`ArkNativeEngine::ArkValueToNapiValue`） |
| `gen_js_obj` 产出 source_set，.o 可链入任意依赖方 | `build/ohos/ace/ace.gni:24-67`；ohos.gni 在 support_jsapi 时自动引入 |
| `js_startup_task.cpp` 仅编译进 `appkit_native` 目标 | 全仓 BUILD.gn 检索 |
| `appkit_native` 已依赖 `ets_runtime:libark_jsruntime` 与 `napi:ace_napi` | `frameworks/native/appkit/BUILD.gn:210/236` |
| 模拟器不链接 appkit_native | frameworks/simulator 无该依赖 |
| es2abc 模块编译参数 `--module` 保留即可（.ts 为 export default 模块） | 现有两 BUILD.gn |

## 关键设计决策

### D1: 去 NAPI 模块化 + ABC 迁入独立内部库（对应 AC-1、AC-2）【R1 修订】

**决策**：删除两个 `ohos_shared_library` napi 模块目标与 `*_module.cpp`；`.ts` 迁至 `frameworks/native/appkit/app_startup/`；es2abc 产物经 `gen_js_obj` 生成 .o，与新写的导出文件 `appkit_startup_task.cpp` 一起链入**新的内部共享库** `libappkit_startup_task.z.so`（安装到系统普通 lib 目录 `/system/lib{,64}/`，**非** NAPI 模块目录）；`appkit_native` 经 DT_NEEDED 依赖并通过导出 getter 函数获取 ABC buffer。

**R1 修订原因（需求方评审意见）**：初版方案（R0）将 ABC 直接链入 appkit_native，需求方不希望 appkit_native 体积增长，要求 JS 制品独立成 .so 供 appkit_native 调用，且不放置在 NAPI 模块相关目录。

**不可达性论证（三条件仍全部满足）**：
1. **不注册**：新 .so 不含 `napi_module_register`/`nm_modname`/`NAPI_*_GetABCCode` 任何注册符号，模块名不存在于进程注册表
2. **不落 NAPI 搜索路径**：`NativeModuleManager::GetNativeModulePath` 仅搜索 `/system/lib{,64}/module/**`（sysPrefix）、应用 lib 路径（isAppModule）与 `/system/etc/abc/**`（sysAbcPrefix）；新 .so 装在 `/system/lib{,64}/`（ohos_shared_library 默认安装目录，无 relative_install_dir），不在任何一族搜索路径内
3. **不可按名解析**：requireNapi 按名→路径映射仅命中 `lib<name>_napi.so`/`lib<name>.so`（module 目录下）或 `<name>.abc`（etc/abc 下）；`appkit_startup_task` 不对应任何模块名

**接口设计**：getter 函数（显式 `visibility("default")`，不配 version_script——version_script 为逐目标可选配置，见 appkit BUILD.gn:397；与旧 napi 模块导出 GetABCCode 同机制）：

```cpp
// appkit_startup_task.cpp（新 .so 内，extern "C"）
extern "C" __attribute__((visibility("default")))
const uint8_t* GetAsyncTaskExcutorAbc(int32_t* len);
extern "C" __attribute__((visibility("default")))
const uint8_t* GetAsyncTaskCallbackAbc(int32_t* len);
```

**权衡（DT_NEEDED vs dlopen 按需加载）**：DT_NEEDED 使新 .so 随 appkit_native 进入所有应用进程，但体积仅约数 KB（两个 ABC 共约 3.4KB + 源文本），常驻成本可忽略；dlopen 按需加载可避免常驻但引入路径查找/错误处理复杂度，不采用。

**理由**：相比初版（ABC 内嵌 appkit_native），appkit_native 体积不增长、JS 制品独立可单独维护；镜像 .so 数量与初版方案相比 +1（仍比原始状态 -1：原始 2 个 napi .so → 现在 1 个内部 .so）。

**否决备选**（详见方案文档 3.4）：ModuleChecker 黑名单（无法区分调用来源）/ 挪路径保留注册（进程缓存按名仍可命中）/ 装入 /system/etc/abc（同为 NAPI 搜索路径）/ 改隐蔽名（安全靠隐蔽）。

**约束**：AGENTS.md 架构边界——frameworks 层经 `interfaces/inner_api` 与引擎交互；本次新增的 `ExecuteModuleFromBuffer` 调用与 `js_runtime.cpp` 现有直接使用 `panda::JSNApi`（如 `ExecuteSecureWithOhmUrl`）的模式一致，不引入新的越层依赖。

### D2: 加载链路 = ExecuteModuleFromBuffer + GetExportObjectFromBuffer + napi_new_instance（对应 AC-3）

**决策**：新增 `JsRuntime::LoadInternalModuleByBuffer(vm, env, buf, len, fileName, argv, argc)` 静态方法；`LoadJsAsyncTaskExecutor` / `LoadJsAsyncTaskCallback` 改为：

```
appkit_native --DT_NEEDED--> libappkit_startup_task.z.so
  → GetAsyncTaskExcutorAbc(&len) / GetAsyncTaskCallbackAbc(&len)   // 进程内取内嵌 ABC buffer
  → JSNApi::ExecuteModuleFromBuffer(vm, buf, len, fileName)     // 与 NAPI 流程同一执行入口
  → JSNApi::GetExportObjectFromBuffer(vm, fileName, "default")  // 取 export default 类
  → ArkNativeEngine::ArkValueToNapiValue → napi_new_instance(argv) // 与 LoadSystemModuleByEngine 尾段一致
  → napi_create_reference 持有（原有生命周期管理不变）
```

**理由**：与原 NAPI 流程（LoadArkModule → 相同两 API → requireNapi 返回类 → napi_new_instance）执行语义完全一致，`'use concurrent'`（async_task_excutor.ts）与 `'use sendable'`（async_task_callback.ts）字节码行为不变。

**fileName 约定**：仅作模块记录名（栈回溯/source map 定位用），取 `async_task_excutor.abc` / `async_task_callback.abc`，进程内唯一且稳定。原 NAPI 流程记录名为 `libasynctaskexcutor_napi.z.so/asynctaskexcutor.js`，变化仅影响调试栈显示，需在回归时确认无功能性影响。

### D3: sendable 类定义位置不变（对应 AC-3）

`AsyncTaskCallBack::AsyncTaskCompleted` 与 `napi_define_sendable_class`（js_startup_task.cpp:190-196）保持 C++ 侧定义不变；仅 TS 包装类（`__impl__` 持有 sendable 实例）的获取方式由 requireNapi 换为 buffer 加载。回调链路 `onAsyncTaskCompleted → JsStartupTask::OnAsyncTaskCompleted` 逐行不变。

### D4: 错误处理与 DFX 保持（对应 AC-5）

`LoadJsAsyncTaskExecutor` 失败时沿用 `ERR_STARTUP_INTERNAL_ERROR` + `STARTUP_TASK_ERROR`（HISYSEVENT_FAULT）上报（js_startup_task.cpp:113-119），hisysevent.yaml 无变更。`LoadJsAsyncTaskCallback` 现状无独立错误分支（返回 void），保持现状；新增 `LoadInternalModuleByBuffer` 返回 nullptr 时由调用方判空走既有错误路径。

### D5: Task 拆分（对应 AC-6）

按"能力闭环"拆 3 个 Task（R1 修订后 TASK-1 含新内部库 `appkit_startup_task` 的创建）：
- TASK-1 构建层：删模块、迁 .ts、新增内部 .so 目标（getter 导出）、改两处 BUILD.gn（AC-1、AC-2 的制品面）
- TASK-2 运行时能力：`LoadInternalModuleByBuffer`（AC-3 的引擎侧支撑；R1 修订下本 Task 无变化）
- TASK-3 框架接线：`js_startup_task.cpp` 改走 getter + buffer 加载 + 回归（AC-3/4/5 的行为面）

依赖：TASK-3 依赖 TASK-1、TASK-2；TASK-1 与 TASK-2 可并行。

## 后续 Task 拆分

| Task | 目标 | AC 映射 | 前置依赖 |
|------|------|---------|----------|
| TASK-1 | 私有 NAPI 模块从构建体系移除，ABC 迁入独立内部 .so（.ts 迁移 + 新目标 + napi_packages 清理） | AC-1, AC-2, AC-6 | 无 |
| TASK-2 | JsRuntime 新增 LoadInternalModuleByBuffer 内部加载能力 | AC-3, AC-5, AC-6 | 无 |
| TASK-3 | js_startup_task 加载链路切换（getter 取 buffer）+ 全量回归验证 | AC-1, AC-3, AC-4, AC-5, AC-6 | TASK-1, TASK-2 |

> 详细文件清单、验证命令、完成判据见 execution-plan.md。
