---
phase_status: approved
approval:
  status: approved
  approver: "yangzk"
  evidence: "需求方在会话内对根因分析、方案选型（含 R1 修订）逐轮确认并要求实施；方案文档已评审合入（随本笔提交）；澄清记录 8 项全闭环；分支历史 squash 为单笔提交时同步刷新本摘要（基线内容无实质变更）"
  approved_at: "2026-09-11"
  baseline_digest: "308dee23440035bb6ab632ad2c9c024ae2978dafb88f646fd5ca6007e2eef286"
target_release:
  id: OpenHarmony-Master
  status: committed
---

# 需求文档

> AppStartup AsyncTask 私有 NAPI 模块暴露治理。一份文档，从原始需求到基线结论。

## 需求输入

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | REQ-ASYNTASK-NAPI-EXPOSURE |
| 需求名称 | 消除 asynctaskcallback_napi / asynctaskexcutor_napi 私有 API 暴露 |
| 来源 | API 治理专项（私有 API 暴露问题） |
| 提出人 | yangzk |
| 目标发行版本 | OpenHarmony-Master |
| 候选 Profile | none（ability_runtime 不属于 arkui/arkweb/arkgraphic/arkdata 任一 profile 的 repos 集合） |
| 优先级 | P1 |
| 状态 | Approved |
| Define 阶段状态 | Approved |

### 原始描述

**原始问题（需求方原话）：** "asynctaskcallback_napi 和 asynctaskexcutor_napi 只被框架侧使用，而开发者实际不应该使用，但是现在以 napi 暴露出去了，导致私有 api 暴露。我们应该怎么实现以避免这种私有 api 暴露"

**痛点：**

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 系统开发者（API 治理） | 两个框架内部模块以公共 NAPI 系统模块形态随镜像发布，任何三方应用可 `requireNapi('app.appstartup.AsyncTaskExcutor')` 加载 | 私有 API 暴露，违反 API 治理约束；未声明的私有路径被依赖后形成兼容性负担 |
| 应用开发者 | 可访问未文档化的框架内部实现 | 误用框架私有实现，无稳定性承诺 |
| 框架维护者 | 私有实现被外部依赖的风险 | 后续重构私有实现可能破坏三方应用 |

**期望结果：** 两个模块从 NAPI 公共模块体系中彻底移除（不可按名 require、不可按路径加载），同时框架自身异步启动任务能力行为完全不变。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 根因与方案分析 | `docs/app_startup/async_task_napi_exposure_solution.md`（随本笔提交） | 完整五环暴露链路分析、四备选方案对比，需求方已评审并合入 |
| 暴露链路代码证据 | `foundation/arkui/napi/module_manager/native_module_manager.cpp:1022-1233` | requireNapi 按名解析 `lib<name>_napi.so` 的路径规则 |
| SDK 声明缺失 | `interface/sdk-js/api/` 无 AsyncTask 相关 d.ts | 两模块从未有公共/内部 API 声明 |

### 初始范围

**可能包含：**
- 删除两个 NAPI 模块目标与注册代码
- ABC 字节码内嵌框架 .so 并直接加载
- 框架加载入口重写

**明确不包含：**
- AppStartup 公共 API（`@ohos.app.appstartup.*`）任何变更
- 其他 ABC 型 NAPI 模块（如 `application.Context` 系列）的治理

### 初始假设

| 假设 | 类型 | 验证方式 | 状态 |
|------|------|----------|------|
| 两模块无任何外部消费方（仅 js_startup_task.cpp） | 技术 | 全仓 grep `app.appstartup.AsyncTask` | 已验证（仅框架一处消费） |
| `JSNApi::ExecuteModuleFromBuffer` 与 NAPI 流程执行语义一致 | 技术 | NAPI 流程源码 ark_native_engine.cpp:3545 使用同一 API | 已验证 |
| 模拟器不链接 appkit_native，不受影响 | 技术 | frameworks/simulator 无 appkit_native 依赖 | 已验证 |
| 两模块无 d.ts，删除无 SDK 兼容性负担 | 兼容性 | interface/sdk-js 检索 | 已验证 |

### 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 公共/System API 变更 | 否 | 两模块无 d.ts 声明，不在 API 清单 |
| 跨仓变更 | 否 | 全部变更在 ability_runtime 单仓 |
| 安全/隐私维度 | 是（风险降低型） | 消除私有暴露面，不引入新攻击面；非高风险变更，不触发 threat-model |
| 复杂度 | 标准 | 单仓、构建架构调整 + 加载机制重写，无新公共接口 |

### Agent Scope Guard

> 只约束需求探索和上下文收集阶段可访问的仓库、模块与知识源，不维护实现文件清单。发现需要越界时必须停止并重新确认基线；精确代码文件范围由 execution-plan.md / task.md 持有。

| 维度 | 允许范围/限制 | 越界触发条件 | 处理规则 |
|------|---------------|--------------|----------|
| 允许检索仓库 | OpenHarmony master 工作区（本仓 ability_runtime 及其依赖仓只读：arkui/napi、arkcompiler/ets_runtime、interface/sdk-js、build） | 需要访问未列仓库 | 停止探索，补充影响分析并重新确认基线 |
| 允许模块/目录 | ability_runtime 全仓；依赖仓仅限模块加载/NAPI 注册/构建模板相关目录 | 需要扩大模块/目录 | 更新范围与影响范围，经 Owner 确认后继续 |
| 禁止访问/修改 | services/dialog_ui 构建产物（无关工作区）；任何凭据/私有目录；生成物不手工修改 | 命中禁止项 | 标记 Blocked，不读取、不修改 |
| 外部网络/私有知识源 | N/A：本需求全部上下文来自本地源码与本地构建产物，未使用外部网络或 DeepWiki | 需要新增权限或未授权来源 | 请求人工授权；未授权前不得使用 |

---

## 二、澄清记录

> 澄清通过需求方与本会话的前置分析对话完成，需求方对根因分析与方案结论明确认可（要求落地为文档、SVG 并要求按 SDD 流程实施修复）。

| # | 模糊点 | 澄清结论 |
|---|--------|----------|
| 1 | 暴露的根因是什么 | 五环链路共同作用：构造函数自动注册（napi_module_register）+ 安装到 NAPI 搜索路径（module/app/appstartup）+ requireNapi 按名可解析（GetNativeModulePath 命中 lib*_napi.so）+ 普通应用无 ModuleCheckerDelegate 拦截（runtime.h:85 默认 nullptr，仅 Extension 场景注册）+ SDK 无 d.ts 声明（纯私有路径无保护） |
| 2 | 交付边界（方案选型） | **去 NAPI 模块化 + ABC 迁入独立内部库直接加载（R1 修订）**：删除两个 napi 模块目标与注册代码；es2abc 产物 .o 与 extern "C" getter 链入新增内部库 `libappkit_startup_task.z.so`（普通 lib 目录，非 NAPI 路径），`appkit_native` 经 DT_NEEDED 依赖；框架经 `JSNApi::ExecuteModuleFromBuffer` + `GetExportObjectFromBuffer` + `napi_new_instance` 加载，与原 NAPI 流程执行语义一致（初版 R0 曾定"直接链入 appkit_native"，评审后按"独立 .so 且不落 NAPI 目录"意见修订） |
| 3 | 备选方案为何否决 | ① ModuleChecker 黑名单：框架自身也走全局 requireNapi，isAppModule 同为 false，无法区分调用来源，且 checker 仅 Extension 注册，全局生效需改运行时治理策略；② 挪路径保留注册：构造函数执行后模块名进入进程 NativeModuleManager 缓存，按名仍可命中；③ 装入 /system/etc/abc：同为 NAPI 搜索路径（nativeModulePath[2]）；④ 改隐蔽模块名：安全靠隐蔽，不可接受 |
| 4 | .ts 胶水文件归属 | 从 `frameworks/js/napi/app_startup/` 迁移至 `frameworks/native/appkit/app_startup/`（不再是 NAPI 模块，归框架内部实现所有） |
| 5 | AsyncTaskCallback 的 sendable 类是否改动 | C++ 侧 `napi_define_sendable_class`（js_startup_task.cpp:195）保持不变，仅 TS 包装类获取方式由 requireNapi 换为 buffer 加载 |
| 6 | 是否新增公共接口 | 否。`JsRuntime::LoadInternalModuleByBuffer` 为 inner_api 内部静态方法，不进 SDK |
| 7 | 模拟器影响 | frameworks/simulator 不链接 appkit_native，其 JsRuntime 为独立实现，不受影响（N/A） |
| 8 | 变更前的正向行为基线 | 异步 StartupTask（非 callCreateOnMainThread）：init 在 taskpool 执行，结果经 sendable AsyncTaskCallback 回传框架，回调触发 JsStartupTask::OnAsyncTaskCompleted；此行为必须逐项保持 |

---

## 三、需求基线

### 范围

- **包含**：
  - 删除 `asynctaskexcutor_napi` / `asynctaskcallback_napi` 两个 `ohos_shared_library` 目标及其 `*_module.cpp`（napi_module_register、GetABCCode 导出符号随 .so 从镜像消失）
  - `frameworks/js/napi/BUILD.gn` 的 `napi_packages` 移除上述两依赖
  - 两个 .ts 胶水文件迁移至 `frameworks/native/appkit/app_startup/`，es2abc + gen_js_obj 目标定义迁入 `frameworks/native/appkit/BUILD.gn` 并链入 `appkit_native`
  - `JsRuntime` 新增内部静态方法 `LoadInternalModuleByBuffer`（对标 `LoadSystemModuleByEngine` 写法）
  - `js_startup_task.cpp` 重写 `LoadJsAsyncTaskExecutor` / `LoadJsAsyncTaskCallback` 走 buffer 加载
  - 回归验证：编译 + AppStartup 单测 + 负向（requireNapi 不可达）验证

- **排除**：
  - AppStartup 公共 API（`@ohos.app.appstartup.*` 五个 d.ts）任何签名/语义变更
  - 其他 ABC 型 NAPI 模块（`application.Context` 等有内部 d.ts 声明的模块）的治理
  - `napi_module_register` / NativeModuleManager 运行时机制本身的修改
  - FA 模型、错误码、HiSysEvent 事件的变更

### 涉及子系统/仓

| 仓 | 子系统/路径 | 说明 |
|----|------------|------|
| ability_runtime | `frameworks/js/napi/app_startup/` | 删除两个 NAPI 模块目录（.ts 迁出） |
| ability_runtime | `frameworks/native/appkit/` | ABC 目标定义迁入 + `js_startup_task.cpp` 加载重写 |
| ability_runtime | `frameworks/native/runtime/` + `interfaces/inner_api/runtime/` | `JsRuntime::LoadInternalModuleByBuffer` 声明与实现 |

### 不涉及项（N/A 维度）

| 维度 | 结论 |
|------|------|
| 公共 SDK API 变更 | N/A（两模块无 d.ts） |
| IPC/序列化格式变更 | N/A |
| Mission 持久化数据格式 | N/A |
| FA 模型 | N/A |
| 第三方依赖 | N/A |
| hisysevent.yaml | N/A（事件定义不变，上报分支保留） |
| 特性标志（ability_runtime.gni） | N/A |
| SA 注册配置 | N/A |
| 跨仓/跨 SIG 协调 | N/A |

### 复杂度级别

**标准** — 单仓（ability_runtime）缺陷治理：构建目标结构调整 + 框架加载链路重写；无新公共接口、无跨仓依赖、无高风险安全路径（变更为收窄暴露面的风险降低型）。

### 验收标准（P0，追溯至"期望结果"）

- **成功标准 1（不可达）**：三方应用运行时 `requireNapi('app.appstartup.AsyncTaskExcutor')` / `requireNapi('app.appstartup.AsyncTaskCallback')` 无法获得模块实例
- **成功标准 2（镜像移除）**：系统镜像 `/system/lib{,64}/module/app/appstartup/` 下不存在 `libasynctask*_napi.so`
- **成功标准 3（行为不变）**：异步启动任务（taskpool 执行 + sendable 回调）行为与变更前一致；主线程路径（callCreateOnMainThread）不受影响
- **成功标准 4（工程不回归）**：ability_runtime 编译通过，AppStartup 相关单测通过

### 基线结论

**通过** — 澄清项全部关闭（见澄清记录），范围与 N/A 维度明确，成功标准可验证。

| 字段 | 内容 |
|------|------|
| 决策 | Baselined |
| 审批 | yangzk（需求方，会话内对根因分析、方案选型、SVG 汇报材料逐轮确认并要求实施） |
| 证据 | `docs/app_startup/async_task_napi_exposure_solution.md`（随本笔提交）；本会话澄清记录 8 项全闭环 |
| 下游 | spec.md |
| 重检范围 | 无 |
