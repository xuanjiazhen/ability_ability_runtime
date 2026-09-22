# 特性规格：AppStartup AsyncTask 私有 NAPI 模块暴露治理

## 1. 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | AppStartup AsyncTask 私有 NAPI 模块暴露治理 |
| 特性编号 | FEAT-ASYNTASK-NAPI-EXPOSURE |
| 所属 Epic | 无 |
| 优先级 | P1 |
| 目标版本 | OpenHarmony-Master（引用 proposal.target_release） |
| SIG 归属 | sig-apability |
| 状态 | Approved |
| 复杂度 | 标准 |

**一句话规格**：`app.appstartup.AsyncTaskExcutor` 与 `app.appstartup.AsyncTaskCallback` 两个框架私有实现从系统 NAPI 模块体系中彻底移除（不注册、不落盘、不可按名/按路径加载），AppStartup 异步启动任务对外行为保持完全不变。

## 本次变更范围（Delta）

| 类型 | 内容 | 说明 |
|------|------|------|
| REMOVED | 系统镜像中的 `libasynctaskexcutor_napi.so`、`libasynctaskcallback_napi.so` 及其 NAPI 模块名注册 | 两模块无任何 SDK 声明（d.ts），删除属私有路径治理，无兼容性迁移指引需要 |
| MODIFIED | 框架内部对上述两份 TS 实现的加载方式（requireNapi → 进程内直接加载） | 对外不可观测；异步启动任务行为不变 |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `proposal.md` | Approved（Baselined） |

## 用户故事

### US-1: 三方应用无法加载框架私有模块

**作为** 应用开发者（含潜在误用者），
**我想要** 即使我主动调用 `requireNapi('app.appstartup.AsyncTaskExcutor')` 或 `requireNapi('app.appstartup.AsyncTaskCallback')`，
**以便** ——系统应当拒绝该加载，使我无法触达框架私有实现（这是治理要求，非能力要求）。

### US-2: 异步启动任务体验不变

**作为** 使用 AppStartup 的应用开发者，
**我想要** 我配置的异步 StartupTask（未声明 `callCreateOnMainThread`）在启动时仍于 taskpool 线程并发执行，结果照常回传，
**以便** 我的启动性能收益与变更前完全一致。

### US-3: API 治理合规

**作为** API 治理/系统开发者，
**我想要** 系统镜像中不再存在未声明即发布的 NAPI 模块制品，
**以便** 私有 API 扫描/审计不再报出该暴露项。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 可观察表面 | 类型 |
|--------|----------|--------------|------|
| AC-1 | WHEN 任意应用在运行时调用 `requireNapi('app.appstartup.AsyncTaskExcutor')` 或 `requireNapi('app.appstartup.AsyncTaskCallback')` THEN 返回模块不存在/加载失败，不获得任何可调用的模块导出对象 | InnerAPI（进程内 NAPI 模块边界——requireNapi 加载结果对象） | 异常（负向治理） |
| AC-2 | WHEN 构建系统镜像并检查 NAPI 模块安装目录 THEN `/system/lib{,64}/module/app/appstartup/` 下不存在 `libasynctaskexcutor_napi.so` 与 `libasynctaskcallback_napi.so`（目录中仅保留 startuptask/startupManager/startuplistener/startupconfigentry 四个公共模块） | InnerAPI（系统镜像制品可查询结果） | 正常 |
| AC-3 | WHEN 应用配置异步 StartupTask（非 `callCreateOnMainThread`）并触发 AppStartup 执行 THEN `init()` 在 taskpool 线程执行，执行结果经回调送达框架并驱动后续依赖任务/监听器，整体行为与变更前一致（含 `'use concurrent'`/`'use sendable'` 语义） | Public API（@ohos.app.appstartup.startupManager 启动结果回调链） | 正常 |
| AC-4 | WHEN 应用 StartupTask 声明 `callCreateOnMainThread` THEN 任务在主线程执行，行为与变更前一致（本次未触碰该路径） | Public API（StartupTask 主线程执行行为） | 回归 |
| AC-5 | WHEN 框架内部加载两份 TS 实现失败 THEN 沿用既有错误处理：返回 `ERR_STARTUP_INTERNAL_ERROR` 并上报 `STARTUP_TASK_ERROR` 故障事件（事件名/参数与 hisysevent.yaml 现有定义一致） | InnerAPI（RunTaskInit 返回值 + HiSysEvent 事件） | 异常 |
| AC-6 | WHEN 执行 ability_runtime 编译与 AppStartup 相关单测 THEN 编译通过、既有用例全部通过 | InnerAPI（构建系统与测试入口可查询结果） | 工程 |

## 规则定义

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 异常 | 应用 JS 调用 `requireNapi("app.appstartup.AsyncTaskExcutor")` 或 `requireNapi("app.appstartup.AsyncTaskCallback")` | 模块管理器按名/按路径均解析失败，返回模块不存在信息对象；不抛未捕获异常、不影响宿主应用其他模块加载 | 两个模块名均须不可达；`/system/etc/abc/` 路径同样不得存在可被命中的同名 ABC | AC-1 |
| R-2 | 行为 | 系统镜像构建完成 | `module/app/appstartup/` 下仅有四个公共模块 .so；新增产物中不存在任何承载这两个模块名的可安装 .so | 镜像级检查 | AC-2 |
| R-3 | 行为 | 异步 StartupTask 触发执行 | taskpool 并发执行 `init(context)`；结果对象经 sendable 回调送回主线程并分发给依赖方/监听器；执行语义（并发函数、sendable 传值）与变更前一致 | 框架内部加载失败时走 R-5 | AC-3 |
| R-4 | 行为 | StartupTask 声明 `callCreateOnMainThread=true` | 主线程执行路径不走新加载链路，行为与变更前一致 | 本次不修改该路径任何代码 | AC-4 |
| R-5 | 异常 | 框架内部加载 TS 实现失败（buffer 无效/导出缺失/实例化失败） | 返回 `ERR_STARTUP_INTERNAL_ERROR`，上报 `STARTUP_TASK_ERROR`（HISYSEVENT_FAULT），不 crash | 事件定义不变更 hisysevent.yaml | AC-5 |
| R-6 | 恢复 | 加载失败的启动任务 | 既有 StartupFramework 错误处理路径照常（结果回调携带错误，依赖任务收错误结果），本变更不改变恢复语义 | 恢复路径与变更前一致 | AC-5 |

## 验收追溯

| AC | 关联规则 | 可观察表面 |
|----|----------|--------------|
| AC-1 | R-1 | InnerAPI：requireNapi 返回的加载结果对象 |
| AC-2 | R-2 | InnerAPI：镜像 NAPI 模块目录制品清单 |
| AC-3 | R-3 | Public API：startupManager 启动结果回调链 |
| AC-4 | R-4 | Public API：StartupTask 主线程执行行为 |
| AC-5 | R-5, R-6 | InnerAPI：RunTaskInit 返回值 + STARTUP_TASK_ERROR 事件 |
| AC-6 | — | InnerAPI：构建/测试命令输出 |

## 验证映射

> 每个 AC 至少关联一个可执行测试入口。AC→Task 映射由 execution-plan 维护；实际执行证据由 evidence/reviews 维护。

| 编号 | 对应规格项 | 测试入口 | 验证方式 | Red 条件（实现前失败信号） | 通过标准 |
|------|------------|----------|----------|---------------------------|----------|
| VM-1 | R-1 / AC-1 | 静态：`grep -rn "nm_modname.*AsyncTask" --include="*.cpp" frameworks/`；设备：应用内 `requireNapi('app.appstartup.AsyncTaskExcutor')` | 静态检查 + 人工（设备） | 实现前 grep 命中 2 处注册源（async_task_*_module.cpp），设备 requireNapi 返回模块实例可调用 | grep 无输出；设备返回模块不存在信息对象 |
| VM-2 | R-2 / AC-2 | 构建后检查 `out/<product>/packages/phone/system/lib{,64}/module/app/appstartup/` | 静态检查（制品） | 实现前目录含 libasynctaskexcutor_napi.z.so 与 libasynctaskcallback_napi.z.so | 目录仅 4 个公共模块 .so，无 libasynctask* |
| VM-3 | R-3 / AC-3 | AppStartup 单测（startup_manager_test 等）+ Demo 应用异步任务 E2E | 单测 + 集成 | 实现前异步任务链路（requireNapi 加载 excutor/callback）为基线行为；本变更后加载入口改变，若执行语义不等价则单测/回调失败 | init 在 taskpool 执行、onAsyncTaskCompleted 回调送达、依赖任务获得结果 |
| VM-4 | R-4 / AC-4 | AppStartup 既有单测全量回归（主线程路径用例零修改） | 单测回归 | 主线程路径若被误触碰则既有用例失败 | 既有用例全部通过 |
| VM-5 | R-5, R-6 / AC-5 | 单测错误分支（buffer 无效/导出缺失）+ HiSysEvent 上报检查 | 单测 + 事件检查 | 实现前 LoadJsAsyncTaskExecutor 恒返回 ERR_OK 不判空；注入失败无事件上报 | 失败返回 ERR_STARTUP_INTERNAL_ERROR（数值 16000050）并上报 STARTUP_TASK_ERROR |
| VM-6 | — / AC-6 | `./build.sh --product-name <product> --build-target ability_runtime` + `run -t UT -tp ability_runtime` | 编译 + 单测 | 构建图含被删目标时编译失败；单测对被删目标有依赖时失败 | 编译通过、既有用例全部通过 |

> 本环境无设备/完整构建条件时的降级执行：VM-1/VM-2 以构建图（build.ninja）与源码 grep 替代制品检查，VM-6 以 gn 解析 + 受影响文件真实工具链编译替代全量构建；降级证据见 evidence/reviews/verification.md，设备/CI 补验为遗留项。

## API 变更分析

### 新增 API

N/A：本次为私有加载路径治理，不新增任何 Public/System API。

### 变更/废弃 API

N/A：两模块从未有 d.ts 声明（interface/sdk-js 无对应文件），不在任何 API 清单中，无既有调用方可迁移；新增 InnerAPI（`JsRuntime::LoadInternalModuleByBuffer`、`GetAsyncTaskExcutorAbc`/`GetAsyncTaskCallbackAbc`）为部件内部接口，不面向应用。

### API 与错误码事实

| 事实项 | 事实类型 | 开放级别 | 精确签名/数值 | 触发条件 | 对外行为 | 事实来源（文件:行/符号或 New 依据） | 关联 AC |
|--------|----------|----------|---------------|----------|----------|--------------------------------------|---------|
| ERR_STARTUP_INTERNAL_ERROR | Existing Error Code | InnerAPI | 16000050（枚举值） | 框架内部加载/执行启动任务失败 | RunTaskInit 返回 16000050 并触发既有失败回调，同时上报 STARTUP_TASK_ERROR 事件（事件名/参数与 hisysevent.yaml 现有定义一致，无新增） | `interfaces/kits/native/appkit/app_startup/startup_utils.h:27` | AC-5 |
| JsRuntime::LoadInternalModuleByBuffer | New API | InnerAPI | `static std::unique_ptr<NativeReference> LoadInternalModuleByBuffer(napi_env env, panda::ecmascript::EcmaVM* vm, const uint8_t* buf, size_t len, const std::string& fileName, const napi_value* argv, size_t argc)` | 框架内部从进程内 buffer 加载非注册模块 | 成功返回持有实例的 NativeReference；任一失败环节返回 nullptr（含 TAG_LOGE 日志） | New: design.md D2 / TASK-2 | AC-3, AC-5 |
| GetAsyncTaskExcutorAbc / GetAsyncTaskCallbackAbc | New API | InnerAPI | `extern "C" const uint8_t* GetAsyncTask{Excutor,Callback}Abc(int32_t* len)` | appkit_native 调用获取内嵌 ABC buffer | 返回 buffer 指针，len 输出字节数（excutor.abc=2212B / callback.abc=1212B，编译期确定） | New: design.md D1 / TASK-1 | AC-3 |

## Spec 自审清单

- [x] 每个 AC 触发条件可复现、预期行为可观测（返回值/制品存在性/事件）
- [x] 规则无重叠冲突；R-3/R-4 路径互斥（异步 vs 主线程声明）
- [x] 不包含内部实现细节（类名/方法名/调用链在 design.md）
- [x] 兼容性口径明确：REMOVED 项无 SDK 声明、无迁移指引需要
- [x] N/A 维度继承 proposal（公共 API/IPC/FA/第三方依赖等均为 N/A）
- [x] 每个 AC 有验证方式且映射 Task
