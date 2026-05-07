# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 在此代码库中工作时提供指导。

## 概述

这是 OpenHarmony 的 **ability_runtime** 组件（元能力运行时）。它提供核心组件框架，用于管理应用生命周期、组件调度和扩展组件管理。

### 开发者框架

**UIAbility 组件**：
- 提供应用界面的核心组件
- 支持多实例、多窗口模式
- 完整的生命周期管理（onStart、onForeground、onBackground、onStop）

**Extension 扩展组件**：
- ServiceExtension：后台服务扩展
- UIExtension：UI 扩展组件
- UIServiceExtension：UI 服务扩展
- AppServiceExtension：应用服务扩展
- DataShareExtension：数据共享扩展

**Uri 授权**：
- 跨应用 URI 访问权限管理
- URI 权限授予和撤销
- 安全的跨进程数据访问

**意图框架 (InsightIntent)**：
- 系统级意图标准体系，连接应用内的业务功能
- 智能意图识别和分发，支持意图匹配和路由
- 跨应用意图调度，实现应用间智能协同
- 帮助开发者将应用内的业务功能智能化
- 支持配置和装饰器两种方式开发意图

**启动框架**：
- 应用启动流程管理
- 启动模式和启动规则控制
- 冷启动、热启动优化

**自动填充框架**：
- 表单自动填充支持
- 跨应用数据填充

**子进程**：
- 子进程创建和管理
- 父子进程通信
- 进程隔离和资源控制

**应用上下文环境**：
- 提供应用级别和组件级别的上下文
- 支持获取应用信息、资源访问、权限管理
- 提供应用级数据共享和通信能力

**系统环境变化监听**：
- 配置变化监听（系统语言、主题、方向等）
- 内存级别变化监听
- 系统状态变化监听

**组件生命周期监听**：
- UIAbility 生命周期状态变化监听
- Extension 组件生命周期状态变化监听
- 进程生命周期监听

**任务管理**：
- 任务栈管理
- 多任务切换
- 任务持久化和恢复

### 系统服务

**组件管理服务**：
- 管理 UIAbility 和 Extension 组件生命周期
- 组件启动/停止、连接管理
- 任务栈和任务列表管理
- 与应用管理服务协调进行进程管理

**应用管理**：
- 应用进程生命周期管理
- 与 AppSpawn 服务协调应用孵化
- 应用状态管理（前台、后台）
- 进程状态上报、子进程管理、进程预加载
- 应用恢复和重启

**快速修复**：
- 应用热修复能力
- 补丁管理和应用
- 无需重启的修复机制

## 构建系统

本项目使用 **GN (Generate Ninja)** 作为构建系统。

### 构建命令

```bash
# 构建整个 ability_runtime 组件
./build.sh --product-name <product> --build-target ability_runtime

# 构建特定目标：以abilityms为例
./build.sh --product-name <product> --build-target abilityms

# 构建全量tdd测试用例
./build.sh --product-name <product> --build-target ability_runtime_test

# 构建特定TDD用例：以 ability_manager_service_first_test 为例
./build.sh --product-name <product> --build-target ability_manager_service_first_test
```

### 清理构建

```bash
./build.sh --product-name <product> --build-target ability_runtime --clean
```

## 测试

### 单元测试

单元测试位于 `test/unittest/` 并按组件组织。

```bash
# 运行所有单元测试
run -t UT -tp ability_runtime

# 运行特定测试（示例：ability_manager_service_first_test 测试）
run -t UT -ts ability_manager_service_first_test
```

### 模块测试

位于 `test/moduletest/`，以 ability_caller_fw_module_test 为例：

```bash
run -t UT -ts ability_caller_fw_module_test
```

### Fuzz 测试

位于 `test/fuzztest/`, 以 AbilityAppDebugInfoFuzzTest 为例：

```bash
run -t UT -ts AbilityAppDebugInfoFuzzTest
```

## 架构

### 目录结构

```
ability_runtime/
├── frameworks/           # 框架实现
│   ├── native/          # C++ 原生框架（ability context，extensions）
│   ├── js/napi/         # JavaScript NAPI 绑定
│   ├── ets/             # ETS/ArkTS 绑定（ani - arkts native interface）
│   ├── cj/              # CJ (C-based JSON) FFI 绑定
│   └── c/               # ability_runtime 的 C API
├── interfaces/          # 公共接口
│   ├── inner_api/       # 内部组件接口（IPC、客户端）
│   └── kits/            # 公共 SDK 接口（native、C）
├── services/            # 系统服务
│   ├── abilitymgr/      # AbilityManagerService
│   ├── appmgr/          # AppManagerService
│   ├── dataobsmgr/      # DataObserverManager
│   ├── uripermmgr/      # UriPermissionManager
│   ├── quickfixmgr/     # QuickFix 管理器
│   └── dialog_ui/       # 系统对话框 HAP
├── agent_runtime_framework/  # Agent 组件框架
├── service_router_framework/ # 服务路由框架
├── js_environment/      # JS 运行时环境
├── ets_environment/     # ETS 运行时环境
├── cj_environment/      # CJ 运行时环境
├── tools/aa/            # "aa" 命令行工具
└── test/                # 测试（unittest、moduletest、fuzztest）
```

### 核心组件

#### AbilityManagerService
位于 `services/abilitymgr/`，这是核心系统能力，负责：
- 启动和停止组件
- 管理组件生命周期（onStart、onForeground、onBackground、onStop）
- 处理组件连接（connectAbility、disconnectAbility）
- 管理任务栈和任务列表
- 与 AppManagerService 协调进行进程管理

主要子管理器：
- `AbilityConnectManager`：Extension 扩展组件连接和生命周期管理
- `DataAbilityManager`：Data Ability 管理
- `MissionListManager`：任务/任务栈管理（需要图形支持）
- `PendingWantManager`：待定意图/Want 管理
- `FreeInstallManager`：免安装（按需）组件支持
- `KioskManager`：Kiosk 模式管理
- `AutoStartupService`：自动启动组件管理

#### AppManagerService
位于 `services/appmgr/`，负责管理：
- 应用进程生命周期
- 与 AppSpawn 服务协调应用孵化
- 应用状态（前台、后台）
- 进程状态上报、子进程管理、进程预加载
- 应用恢复和重启

#### 框架层

**Native 框架** (`frameworks/native/`)：
- `ability/native/`：Ability、AbilityContext、extensions 实现
- `appkit/`：Application、AbilityStage、TestRunner
- `child_process/`：子进程管理

**NAPI 层** (`frameworks/js/napi/`)：
- JavaScript/TypeScript 应用的绑定
- 模块：ability、abilityManager、appManager、context、wantAgent 等

**ANI 层** (`frameworks/ets/` 和 `frameworks/ets/ets/`)：
- ArkTS 原生接口绑定
- ETS 特定的性能优化实现

**CJ 层** (`frameworks/cj/`)：
- 面向云的语言 (CJ) FFI 绑定
- 与 Ark 运行时的互操作

### 两种应用模型

此代码库同时支持 FA（Feature Ability）和 Stage 模型：

**FA 模型**（API 8 及更早版本）：
- 使用 `config.json` 进行模块配置
- PageAbility、ServiceAbility、DataAbility、FormAbility
- 每个 Ability 有自己独立的 JS VM 实例

**Stage 模型**（API 9+）：
- 使用 `module.json5` 进行模块配置
- Ability (UIAbility)、ExtensionAbility 系列
- 每个进程共享 JS VM 实例
- 更好地支持复杂应用和分布式场景

## 开发说明

### 添加新的 Ability 或 Extension

1. 在 `interfaces/kits/native/` 中定义扩展类型
2. 在 `frameworks/native/ability/native/` 中实现
3. 在 `frameworks/ets/ani/` 中添加 ANI 绑定
4. 在 `frameworks/js/napi/` 中添加 NAPI 绑定
5. 在 `services/abilitymgr/` 中添加服务端管理
6. 更新 BUILD.gn 文件
7. 在 `test/unittest/` 中添加测试

### 特性标志

特性标志在 `ability_runtime.gni` 中定义：
- `ability_runtime_auto_fill`：自动填充扩展支持
- `ability_runtime_child_process`：子进程支持
- `ability_runtime_ui_service_extension`：UI 服务扩展
- `ability_runtime_photo_editor_extension`：照片编辑器扩展
- `ability_runtime_graphics`：图形依赖特性（任务栈）
- `ability_runtime_screenlock_enable`：锁屏集成

添加条件特性时请检查这些标志。

### 错误处理

- 使用在 `interfaces/inner_api/error_utils/` 中定义的 `ERR_*` 错误码
- 常见错误：`ERR_OK`、`ERR_NO_INIT`、`ERR_INVALID_VALUE`、`ERR_INVALID_CALLING`
- 业务错误在 `frameworks/native/ability/native/ability_business_error.h` 中

### 日志记录

使用 HiLog 进行日志记录，已封装在hilog_tag_wrapper.h文件中。
```cpp
#include "hilog_tag_wrapper.h"

// LABEL定义在AAFwkLogTag， 如AAFwkLogTag::ABILITY
TAG_LOGI(LABEL, "Message: %{public}d", value);
```

### aa 命令

`aa` 工具（`tools/aa/`）是 Ability 管理命令行工具，用于调试和管理 Ability 组件。

#### 常用命令

```bash
# 启动 Ability
aa start -a <ability-name> -b <bundle-name> [-D]

# 查询并输出系统信息
aa dump -a

# 强制停止应用进程
aa force-stop <bundle-name>
```

## 配置

### 应用模块配置

- FA 模型：`entry/src/main/config.json`
- Stage 模型：`entry/src/main/module.json5`

### 系统服务配置

- 系统服务配置文件：`services/sa_profile/`

## 依赖项

主要依赖项（在 `bundle.json` 中定义）：
- `ability_base`：Ability 基础工具
- `bundle_framework`：Bundle 管理
- `eventhandler`：事件处理
- `ipc`：IPC/RPC 通信
- `samgr`：系统能力管理器
- `hilog`：日志记录
- `window_manager`：窗口管理
- `ace_engine`：ArkUI 引擎
- `ets_runtime`：ETS 运行时
- `napi`：NAPI 框架

## Skill 执行框架

### 架构概览

Skill 执行框架允许调用方（如小艺 claw）通过 `executeSkill` 接口调度目标 App 执行特定函数，并通过 IPC callback 异步获取执行结果。

**数据流**：
```
调用方 App → NAPI executeSkill → IPC ExecuteInAppSkill(callback) → AMS
  → SkillExecuteManager(CreateExecuteRecord) → 调度目标 Ability
  → 目标 App funcName 执行 → completeArkTSScriptInApp → AMS ExecuteSkillDone
  → callback.OnExecuteDone → 调用方 Promise resolve(ExecuteResult)
```

### 关键文件

| 层级 | 文件 | 职责 |
|------|------|------|
| IPC 接口 | `interfaces/inner_api/ability_manager/include/skill/skill_execute_callback.h` | ISkillExecuteCallback 回调接口 |
| IPC Stub | `services/abilitymgr/include/skill/skill_execute_callback_stub.h` + `.cpp` | 回调 Stub 实现 |
| IPC Proxy | `services/abilitymgr/include/skill/skill_execute_callback_proxy.h` + `.cpp` | 回调 Proxy（TF_ASYNC） |
| AMS 管理 | `services/abilitymgr/include/skill/skill_execute_manager.h` + `.cpp` | 执行记录管理、权限检查、Want 生成 |
| 执行记录 | `services/abilitymgr/include/skill/skill_execute_record.h` | SkillExecuteRecord 结构体 |
| 参数序列化 | `services/abilitymgr/include/skill/skill_execute_param.h` + `.cpp` | Want 参数读写 |
| 结果结构 | `services/abilitymgr/include/skill/skill_execute_result.h` | SkillExecuteResult 结构体 |
| NAPI 入口（调用方） | `cli_tool_framework/frameworks/js/napi/skill_driver/src/js_skill_driver.cpp` | execSkillTool + SkillExecuteCallbackImpl |
| NAPI 入口（目标方） | `cli_tool_framework/frameworks/js/napi/script_manager/src/js_script_manager.cpp` | completeArkTSScriptInApp |
| 目标方 UIAbility | `frameworks/native/ability/native/ability_runtime/js_ui_ability.cpp` | ExecuteSkill 处理 |
| 目标方 ServiceExtension | `frameworks/native/ability/native/js_service_extension.cpp` | ExecuteSkill 处理 |
| AMS 入口 | `services/abilitymgr/src/ability_manager_service.cpp` | ExecuteInAppSkill 调度 |

### 已修复的陷阱

1. **双 NapiAsyncTask Bug**: `CreateAsyncTaskWithLastParam` 和 `NapiAsyncTask::Schedule` 各创建了一个 task，导致返回的 Promise 和实际 resolve 的 Promise 不是同一个对象。修复：使用单个 deferred Promise。

2. **Binder 线程调用 napi**: `OnExecuteDone` 在 binder 线程触发，不能直接调用 napi。修复：使用 `JsCliEventHandlerManager::PostTask()` 投递到 JS 主线程。

3. **WantParams API**: `want.Params()` 不存在，正确 API 是 `want.GetParams()`（返回 `const WantParams&`）。非 const 操作需要值拷贝 → 修改 → `want.SetParams()` 写回。

4. **DeathRecipient 不可用 lambda 构造**: `IRemoteObject::DeathRecipient` 有纯虚函数 `OnRemoteDied`，不能直接用 lambda。需创建 `CallerDeathRecipient` 具体子类。

5. **SkillType 枚举**: `SkillInfo.skillType` 是 `enum class SkillType`，不能隐式转为 `int32_t`，需 `static_cast<int32_t>()`。

6. **sptr 隐式转换**: `sptr<IInterface>` 有 `operator T*()` 可隐式转为裸指针，`SetParam(key, sptr_val)` 无需 `.GetRefPtr()`。

### 需要重建的 .so 文件

| .so 文件 | 构建目标 | 涉及修改 |
|----------|----------|----------|
| `libabilityms.so` | abilityms | skill_execute_manager, skill_execute_callback_proxy, skill_execute_callback_stub, ability_manager_service, ability_manager_stub |
| `libability_manager.so` | ability_manager | skill_execute_callback_stub, ability_manager_proxy, ability_manager_client |
| `libcli_tool_manager.z.so` | cli_tool_manager | js_cli_manager (原有 CLI 接口) |
| `libskilldriver_napi.z.so` | skilldriver_napi | js_skill_driver (execSkillTool) |
| `libscriptmanager_napi.z.so` | scriptmanager_napi | js_script_manager (completeArkTSScriptInApp) |
| `libace.compatible.z.so` | ace_compatible | js_ui_ability (ExecuteSkill 处理) |
| `libability_extension.z.so` | 详见 framework native BUILD | js_service_extension (ExecuteSkill 处理) |
| `libcli_tool_napi.z.so` | cli_tool_napi | aa 命令 exec-skill 入口 |

### executeSkill 支持的参数类型

`ExtractSkillArgs` 在 `js_skill_driver.cpp` 中将 JS 参数转为 WantParams：
- `napi_string` → `AAFwk::String::Box(str)`
- `napi_number`（整数） → `AAFwk::Integer::Box(intVal)`
- `napi_number`（浮点） → `AAFwk::Double::Box(dblVal)`
- `napi_boolean` → `AAFwk::Bool::Box(boolVal)`

## Code Audit: without-token ExecuteSkillDone removal (2026-05-04)

### 修改内容

1. 移除了 without-token 的 `ExecuteSkillDone` 重载（3参数版本：requestCode, resultCode, result）
2. 移除了 `JsUIAbility::ReportSkillDone` 和 `JsServiceExtension::ReportSkillDone`
3. 移除了 IPC code `EXECUTE_IN_APP_SKILL_DONE = 6170`

### 审计结果

**代码一致性：通过**

- IPC 枚举：`EXECUTE_IN_APP_SKILL_DONE = 6170` 已完全移除，保留值 6169/6171/6172 无冲突
- 接口层：`IAbilityManager` 只保留 `ExecuteSkillDone(token, requestCode, resultCode, result)` 带默认实现
- Proxy：只有带 token 的 `ExecuteSkillDone` 实现，使用 `EXECUTE_SKILL_DONE_WITH_TOKEN = 6172`
- Stub：只有 `ExecuteSkillDoneWithTokenInner`，switch case 正确路由
- Client：`AbilityManagerClient::ExecuteSkillDone` 签名为 `sptr<IRemoteObject> token`（按值，非 override，安全）
- Service：只有带 token 版本，从 token 获取 abilityRecord 后提取 bundleName 传给 SkillExecuteManager
- SkillExecuteManager：`ExecuteSkillDone(requestCode, resultCode, result, callerBundleName)`，callerBundleName 始终非空

**目标方文件：通过**

- `js_ui_ability.h/.cpp`：ReportSkillDone 已完全移除，ExecuteSkill 保留，无死代码
- `js_service_extension.h/.cpp`：ReportSkillDone 已完全移除，ExecuteSkill 保留，无死代码

**测试 Mock 文件（11个）：通过**

所有 mock 文件只包含 `MOCK_METHOD4(ExecuteSkillDone, ...)` 带签名 `(const sptr<IRemoteObject>&, const std::string&, int32_t, const SkillExecuteResult&)`，没有残留的旧 MOCK_METHOD3

**已确认的预存问题（非本次修改引入）**

- 枚举值冲突：`RECORD_APP_WITH_REASON_BY_USERID = 6010` 和 `REGISTER_SESSION_HANDLER = 6010` 共用同一值
- 枚举间隔：6170 被跳过（有意为之，不影响功能）
- `napi_value result` 在 `ExecuteSkill` 中被 `napi_call_function` 写入但未被读取（设计如此，future use）

**文档文件未更新（低优先级）**

- `skill.md` 和 `execute_skill.md` 仍引用旧的 3 参数 `ExecuteSkillDone` 和 `EXECUTE_IN_APP_SKILL_DONE`

## NAPI 模块拆分 (2026-05-04)

### 概述

Skill 相关的 NAPI 接口从 `js_cli_manager.cpp`（cliManager 模块）拆分为独立模块，对应各自的 `.d.ts` 定义：

| 模块 | 模块名 | .so 文件 | 接口 | 权限 |
|------|--------|----------|------|------|
| cliManager | `app.cli.cliManager` | `libclimanager_napi.z.so` | execTool, subscribeSession 等 | @systemapi |
| skillDriver | `app.ability.skillDriver` | `libskilldriver_napi.z.so` | execSkillTool | @systemapi |
| scriptManager | `app.ability.scriptManager` | `libscriptmanager_napi.z.so` | completeArkTSScriptInApp, completeArkTSScript | @atomicservice |

### 目录结构

```
cli_tool_framework/frameworks/js/napi/
├── cli_tool_manager/          (精简后，仅保留 CLI 原有接口)
│   ├── BUILD.gn               (新增 cli_manager_error_utils_src, js_cli_event_handler_manager_src source_set)
│   ├── include/js_cli_manager.h
│   └── src/js_cli_manager.cpp
├── skill_driver/              (新建，调用方接口)
│   ├── BUILD.gn
│   ├── include/js_skill_driver.h
│   └── src/js_skill_driver.cpp, skill_driver_module.cpp
└── script_manager/            (新建，目标方接口)
    ├── BUILD.gn
    ├── include/js_script_manager.h
    └── src/js_script_manager.cpp, script_manager_module.cpp
```

### 共享代码

- `cli_manager_error_utils.h/.cpp`：错误码映射，通过 `cli_manager_error_utils_src` source_set 共享
- `js_cli_event_handler_manager.h/.cpp`：事件线程投递，通过 `js_cli_event_handler_manager_src` source_set 共享
- `napi_common_skill_execute.h/.cpp`：SkillExecuteResult 序列化，已在 napi_common 库中

## 遗留任务

### TODO-1: Skill 执行超时机制

**状态**: 已完成

**问题**: `SkillExecuteManager` 缺少服务端超时保护。目标方不调 `completeArkTSScriptInApp` 时，record 永远处于 `EXECUTING` 状态，无自动清理。

**方案**: 复用 AMS EventHandler 超时框架（与 InsightIntent 一致），超时倍数 `SKILL_EXECUTE_TIMEOUT_MULTIPLE = INSIGHT_INTENT_TIMEOUT_MULTIPLE`。

**已修改文件**（8 个）:
- `global_constant.h` — 添加 `SKILL_EXECUTE_TIMEOUT_MULTIPLE`（SUPPORT_ASAN 和非 ASAN 两处）
- `ability_manager_service.h` — 添加 `SKILL_EXECUTE_TIMEOUT_MSG = 12` + `HandleSkillExecuteTimeOut` 声明
- `ability_manager_service.cpp` — `HandleSkillExecuteTimeOut` 实现，委托给 `SkillExecuteManager::OnTimeout`
- `ability_event_handler.h/.cpp` — 添加 `ProcessSkillExecuteTimeOut` + switch case
- `skill_execute_record.h` — 添加 `TIMED_OUT` 状态 + `requestCodeSeq` 字段
- `skill_execute_manager.h/.cpp` — `PostSkillExecuteTimeout`、`RemoveSkillExecuteTimeoutLocked`、`OnTimeout` 实现

### TODO-2: 调用方 DispatchExecuteSkill 异步化

**状态**: 已完成

已将 `js_skill_driver.cpp` 的 `DispatchExecuteSkill`（`QuerySkillType` + `ExecuteInAppSkill` 同步 IPC）从 JS 主线程移到 `NapiAsyncTask::Schedule` worker 线程，避免阻塞 JS 事件循环。
