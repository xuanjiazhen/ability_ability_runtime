# AGENTS.md

## 1. Code map

本 AGENTS.md 适用于 ability_runtime 仓库根目录。子目录 `services/abilitymgr/` 和 `services/uripermmgr/` 各有独立的 CLAUDE.md，提供更细粒度的架构和开发指引。

本仓库实现 OpenHarmony **元能力运行时（ability_runtime）**，核心职责是管理应用生命周期、组件调度、Extension 扩展组件管理、URI 权限管理和意图框架。最重要的架构边界是 **服务层（services/）与公共 SDK 接口层（interfaces/kits/）之间的双向解耦**——服务不可直接暴露内部实现到 SDK，SDK 不可反向依赖服务内部细节。

### 目录职责

| 目录 | 职责 | 高风险标记 |
|------|------|-----------|
| `services/abilitymgr/` | AbilityManagerService（SA 180）：组件生命周期、启动/停止、连接、任务栈、拦截器、InsightIntent | 🔴 修改影响全局组件调度 |
| `services/appmgr/` | AppManagerService（SA 501）：应用进程生命周期、AppSpawn 协调、进程状态、子进程 | 🔴 修改影响所有应用进程 |
| `services/uripermmgr/` | UriPermissionManagerService（SA 183）：跨应用 URI 权限授予/撤销/校验 | 🔴 修改影响安全边界 |
| `services/quickfixmgr/` | QuickFixManagerService（SA 184）：应用热修复补丁管理 | 🟡 修改影响应用更新流程 |
| `services/appdfr/` | 应用 DFR：AppFreeze 管理、ANR 监听、崩溃收集 | 🟡 修改影响故障归因 |
| `services/common/` | 服务侧工具封装：权限校验、日志封装、事件上报、HiSysEvent、XCollie | 🔴 包含 PermissionVerification 全局安全入口 |
| `interfaces/kits/native/` | 公共 SDK 接口：Ability、Context、Extension、Caller/Callee、AppStartup | 🔴 公共 API 兼容性边界 |
| `interfaces/kits/c/` | C 语言公共 API | 🔴 公共 API 兼容性边界 |
| `interfaces/inner_api/` | 内部组件接口：IPC 客户端/代理/桩、错误码工具 | 🟡 内部接口，但仍需注意 IPC 兼容性 |
| `frameworks/native/ability/native/` | 原生框架实现：UIAbility、各 Extension、Context、InsightIntent 执行器 | 🟢 客户端侧实现，可随 SDK 接口联动修改 |
| `frameworks/native/appkit/` | 应用框架：Application、AbilityStage、TestRunner | 🟢 |
| `frameworks/js/napi/` | JavaScript NAPI 绑定 | 🟡 NAPI 签名是公共 API |
| `frameworks/ets/ani/` | ArkTS ANI 绑定 | 🟡 ANI 签名是公共 API |
| `frameworks/cj/ffi/` | CJ FFI 绑定 | 🟡 |
| `frameworks/c/` | C API 框架实现 | 🟡 |
| `agent_runtime_framework/` | Agent 组件框架：AgentManagerService（SA 185）、AgentExtensinoAbility与AgentUIExtensionAbility | 🟢 |
| `tools/aa/` | aa 命令行工具：Ability 调试管理 | 🟢 |
| `js_environment/` / `ets_environment/` / `cj_environment/` | JS/ETS/CJ 运行时环境 | 🟢 |

### Where to look

| 任务类型 | 先看 |
|---|---|
| Ability 生命周期（启动/停止/前台/后台） | `services/abilitymgr/` → `services/abilitymgr/CLAUDE.md` |
| 应用进程管理（启动/杀死/预加载/ANR） | `services/appmgr/` → `app_mgr_service.cpp`、`app_running_manager.cpp` |
| URI 权限（授予/撤销/校验） | `services/uripermmgr/` → `services/uripermmgr/CLAUDE.md` |
| 拦截器相关（启动被拦截/生态规则/Kiosk） | `services/abilitymgr/src/interceptor/` |
| 任务栈/Mission 管理 | `services/abilitymgr/src/mission/` → `mission_list_manager.cpp` |
| InsightIntent（意图识别/分发/执行） | `services/abilitymgr/src/insight_intent/` + `frameworks/native/ability/native/insight_intent_executor/` |
| 可靠性设计/开发（故障模式、资源限制风险） | `skills/Reliability/skill.md`（Want/InsightIntent 模块） |
| 新增 Extension 类型 | `interfaces/kits/native/` → `frameworks/native/ability/native/` → `frameworks/ets/ani/` → `frameworks/js/napi/` → `services/abilitymgr/` |
| 修改公共 SDK API 签名 | `interfaces/kits/native/` + `interfaces/kits/c/` → 需兼容性评估 |
| NAPI 绑定修改 | `frameworks/js/napi/<module>/` → 对应 `nativecommon` 共享组件 |
| ANI 绑定修改 | `frameworks/ets/ani/<module>/` → 对应 `ani_common/` 共享组件 |
| 子进程相关 | `frameworks/native/child_process/` + `services/appmgr/src/child_process_record.cpp` |
| 日志/DFX 修改 | `services/common/include/hilog_tag_wrapper.h` + `hisysevent.yaml` |
| 错误码修改 | `interfaces/inner_api/error_utils/include/ability_runtime_error_util.h` + `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.h` |
| 权限校验逻辑 | `services/common/include/permission_verification.h` |
| QuickFix 热修复 | `services/quickfixmgr/src/quick_fix_manager_service.cpp` |
| AppFreeze/崩溃收集 | `services/appdfr/src/appfreeze_manager.cpp` + `application_anr_listener.cpp` |
| Agent 组件框架 | `agent_runtime_framework/services/agentmgr/` |
| 构建配置/特性开关 | `ability_runtime.gni` |
| 系统能力注册 | `services/sa_profile/` |

## 2. Knowledge routing

遇到问题先定位场景，再读对应文档。以下文档包含完整领域概念和操作指引，不是可选背景阅读。

### Task-based routing

| 任务类别 | 先读 |
|---|---|
| 公共 API 或 SDK 行为变更 | `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.h`（错误码兼容性边界）+ `interfaces/inner_api/error_utils/include/ability_runtime_error_util.h` |
| AbilityManagerService 架构或子管理器变更 | `services/abilitymgr/CLAUDE.md` |
| URI 权限/安全/认证变更 | `services/uripermmgr/CLAUDE.md` + `services/common/include/permission_verification.h` |
| DFX/日志/故障归因变更 | `services/common/include/hilog_tag_wrapper.h` + `hisysevent.yaml` |
| 构建特性开关变更 | `ability_runtime.gni` |
| 应用进程管理/AppSpawn 协调变更 | `services/appmgr/` → `app_mgr_service.h`、`app_spawn_client.h` |
| 拦截器新增或修改 | `services/abilitymgr/CLAUDE.md` 中的 Interceptor Framework 章节 |
| InsightIntent 意图框架变更 | `services/abilitymgr/src/insight_intent/` 目录结构 + `核心功能特性总结.md` 意图框架章节 |
| 元能力代码设计/开发的可靠性风险识别（故障模式、资源限制） | `skills/Reliability/skill.md`（元能力可靠性skill，覆盖 Want/InsightIntent 模块） |

### Path-based routing

| 修改路径 | 先读 |
|---|---|
| `services/abilitymgr/` | `services/abilitymgr/CLAUDE.md` |
| `services/uripermmgr/` | `services/uripermmgr/CLAUDE.md` |
| `services/common/` | `services/common/include/permission_verification.h` + `services/common/include/hilog_tag_wrapper.h` |
| `interfaces/kits/` | `interfaces/kits/native/ability/native/ability_business_error/ability_business_error.h`（公共 API 兼容性） |
| `interfaces/inner_api/error_utils/` | `interfaces/inner_api/error_utils/include/ability_runtime_error_util.h` |
| `frameworks/js/napi/` | 对应模块目录的 `native_module.cpp`（NAPI 模块注册入口） |
| `frameworks/ets/ani/` | 对应模块目录的 ANI 注册入口 + `ani_common/` 共享工具 |
| `services/appdfr/` | `services/appmgr/CLAUDE.md`（如存在）或 `services/appdfr/` 目录头文件 |
| `services/abilitymgr/src/insight_intent/` | `skills/Reliability/skill.md`（InsightIntent 可靠性子skill）+ `services/abilitymgr/CLAUDE.md` |

### Vocabulary-based routing

当任务、issue、日志、API 名称或变更文件包含以下术语时，在规划前先读对应文档：

| 术语 | 风险提示 | 先读 |
|---|---|---|
| UIAbility / Ability | 不是通用"能力"，而是 OpenHarmony 有界面的应用组件核心抽象，有严格生命周期状态机 | `services/abilitymgr/CLAUDE.md` |
| Extension / ExtensionAbility | 不是通用"扩展"，而是 OpenHarmony 特定的无界面服务组件体系（10+ 类型各有独立生命周期） | `services/abilitymgr/CLAUDE.md` |
| Want / WantAgent | 不是通用"意图"，而是 OpenHarmony 组件间调度的结构化描述对象，包含 bundle/module/ability 元数据 | `interfaces/inner_api/wantagent/` |
| Mission / 任务栈 | 不是通用"任务"，而是 OpenHarmony 管理多 Ability 实例的栈结构，有持久化和跨设备同步 | `services/abilitymgr/CLAUDE.md` 中 Mission 章节 |
| AppSpawn / 应用孵化 | 不是通用"进程创建"，而是 OpenHarmony 专属的应用进程孵化服务，有安全沙箱和参数注入机制 | `services/appmgr/` → `app_spawn_client.h` |
| URI Permission / UriPerm | 不是通用"文件权限"，而是跨应用临时 URI 访问授权，涉及 Media/Docs/Sandbox 三种分发策略 | `services/uripermmgr/CLAUDE.md` |
| InsightIntent / 意图框架 | 不是通用"AI 意图"，而是 OpenHarmony 系统级意图标准体系，连接应用内业务功能 | 意图框架章节 |
| Kiosk / 展台模式 | 不是通用"锁定"，而是系统级设备锁定运行模式，涉及拦截器链和启动控制 | `services/abilitymgr/CLAUDE.md` |
| Interceptor / 拦截器 | 不是通用"中间件"，而是 AbilityManagerService 的启动拦截链，10+ 拦截器按顺序执行 | `services/abilitymgr/CLAUDE.md` 中 Interceptor 章节 |
| DFX / HiLog / HiSysEvent / XCollie | 不是通用"日志"，而是 OpenHarmony 故障归因体系，事件定义在 hisysevent.yaml 是硬性约束 | `hisysevent.yaml` + `hilog_tag_wrapper.h` |
| FreeInstall / 免安装 | 不是通用"按需加载"，而是 OpenHarmony 原子化服务免安装机制，涉及 Bundle 和分发协调 | `services/abilitymgr/src/free_install_manager.cpp` |
| KeepAlive / 保活 | 不是通用"后台运行"，而是系统级关键进程保活机制，有独立数据管理 | `services/abilitymgr/src/keep_alive/` |
| SA / SystemAbility / 系统能力 | 不是通用"微服务"，而是 OpenHarmony 通过 samgr 注册的系统能力，有 SA ID、进程绑定和按需启动配置 | `services/sa_profile/` |
| NAPI / ANI / CJ FFI | 三种多语言绑定机制，共享 nativecommon/ani_common 组件，签名变更都是公共 API 变更 | `frameworks/js/napi/` + `frameworks/ets/ani/ani_common/` |
| PermissionVerification | 不是通用"权限检查"，而是 ability_runtime 的全局安全入口，所有组件操作都经过此单例 | `services/common/include/permission_verification.h` |
| QuickFix / 热修复 | 不是通用"热更新"，而是 OpenHarmony 应用级补丁机制，有 Apply/Revert 和版本管理 | `services/quickfixmgr/` |
| AutoFill / 自动填充 | 不是通用"表单填充"，而是跨应用隐私保护的自动填充扩展，有独立的 Extension 和 Manager | `ability_runtime.gni` 中 `ability_runtime_auto_fill` |
| 故障模式 / 资源限制 / 可靠性设计 / FaultMode | 元能力可靠性风险库，覆盖 Want（序列化/fd传递）和 InsightIntent（执行记录/IPC回传）的已知故障模式和资源限制规格 | `skills/Reliability/skill.md` |

在规划中声明：
- 任务类别
- 已读知识文档
- 发现的约束
- 是否需要使用 Skill 或子目录 CLAUDE.md

## 3. Constraints and boundaries

### Architecture/domain invariants

- 公共 SDK 接口（`interfaces/kits/`）表达稳定的能力意图，不暴露服务内部实现细节。
- 权限校验必须在能力入口点执行：所有 Ability/Extension 的启动、连接、数据操作均经过 `PermissionVerification` 单例。
- 服务层（`services/`）与框架层（`frameworks/`）通过 `interfaces/inner_api/` 的 IPC 接口解耦，不直接引用对方内部头文件。
- `services/common/` 是服务层共享基础设施，不依赖特定子服务业务逻辑。
- DFX 事件定义（`hisysevent.yaml`）是故障归因的硬性合约，事件名、参数类型、级别不可随意修改。
- `ability_runtime.gni` 中的特性标志控制编译时行为，默认值变更影响所有下游产品。
- FA 模型和 Stage 模型共存：修改任一模型的行为时，必须确认不影响另一模型。
- Mission 持久化数据格式（RDB）是跨版本兼容性约束，不可破坏已写入的数据结构。

### Do not

- 不要在 `interfaces/kits/` 中修改公共 API 签名、错误码值、生命周期语义，除非任务明确要求且有兼容性评估。
- 不要在 `services/abilitymgr/` 或 `services/appmgr/` 中绕过 `PermissionVerification` 的权限校验。
- 不要修改 IPC Stub/Proxy 的序列化格式——它影响跨版本和跨进程兼容性。
- 不要修改 `hisysevent.yaml` 中已发布事件的参数类型或删除参数——影响线上故障归因。
- 不要修改 `ability_runtime_error_util.h` 和 `ability_business_error.h` 中已发布的错误码数值——它们是公共合约。
- 不要为通过测试而删除日志、HiSysEvent 事件、错误码或诊断信息。
- 不要在 `services/abilitymgr/` 中直接调用 AppManagerService 内部方法——应通过 `interfaces/inner_api/app_manager/` 的 IPC 接口。
- 不要在 `frameworks/native/` 中直接调用 `services/` 的内部头文件——应通过 `interfaces/inner_api/` 的客户端接口。
- 不要跳过 BUILD.gn 更新——添加源文件后必须更新对应 BUILD.gn 和 `.gni` 源文件列表。
- 不要新增第三方依赖——必须先在 `bundle.json` 中声明并通过 License 审查。

### Ask before

- 修改 `interfaces/kits/` 中任何已有 API 的签名或语义。
- 涉及安全/权限/信任的行为变更（特别是 URI 权限、AppSpawn 安全参数、PermissionVerification 规则）。
- 涉及 IPC 协议兼容性或序列化格式变更。
- 涉及 Mission 持久化数据格式变更——影响跨版本数据兼容性。
- 新增或修改第三方依赖、License。
- 删除或重命名公共 API。
- 修改 `ability_runtime.gni` 中特性标志的默认值。
- 修改 `services/sa_profile/` 中 SA 注册配置（SA ID、进程名、库路径）。
- 修改 `hisysevent.yaml` 中已发布事件的定义。
- 修改 FA 模型相关代码（API 8 及更早版本的兼容性约束）。
- 修改 AppSpawn 协调逻辑（影响应用进程安全沙箱）。
- 修改 AppFreeze 或 ANR 检测逻辑（影响线上故障归因）。

### 反模式（不要这样做）

- ❌ 在 `services/abilitymgr/` 中直接调用 AppMgrService 内部方法（应通过 IPC 接口）
- ❌ 在 `frameworks/` 中直接引用 `services/` 内部头文件（应通过 `interfaces/inner_api/`）
- ❌ 不经兼容性评估修改公共 API 签名或错误码值（应先评估影响范围）
- ❌ 为通过测试删除 HiSysEvent 事件或日志（应修复根因）
- ❌ 绕过 PermissionVerification 权限校验（应确认权限规则）
- ❌ 修改 IPC Stub/Proxy 序列化格式但不更新版本协商（应同时更新兼容性处理）
- ❌ 添加源文件但不更新 BUILD.gn（应同步更新构建定义）
- ❌ 修改特性标志默认值但不通知产品配置团队（应先协调）

## 4. Verification

构建命令从 OpenHarmony 源码根目录执行，不在本子目录执行。

### Minimum checks

- 编译验证：`./build.sh --product-name <product> --build-target ability_runtime`
- 编译特定服务：`./build.sh --product-name <product> --build-target abilityms`（AMS）或 `libappms`（AppMS）
- 单元测试：`run -t UT -tp ability_runtime`
- 模块测试：`run -t UT -ts ability_caller_fw_module_test`
- Fuzz 测试：`run -t UT -ts AbilityAppDebugInfoFuzzTest`

### Task-specific checks

| 变更类型 | 最小验证 |
|---|---|
| 修改 AbilityManagerService 内部实现 | 编译通过 + `ability_manager_service_first_test` 及后续编号单测 |
| 修改 AppManagerService 内部实现 | 编译通过 + `app_manager_service_test` 相关单测 |
| 修改 URI 权限逻辑 | 编译通过 + `uri_permission_manager_test` + `uri_permission_impl_test` |
| 修改拦截器 | 编译通过 + `ability_connect_manager_test` + 相关拦截器单测 |
| 新增/修改公共 API（interfaces/kits/） | 编译通过 + 全量单测 + NAPI/ANI/CJ 各语言绑定编译 + 兼容性评估 + `ability_business_error.h` 错误码一致性检查 |
| 新增/修改 NAPI 绑定 | 编译通过 + 对应 NAPI 模块单测 |
| 新增/修改 ANI 绑定 | 编译通过 + 对应 ANI 模块单测 |
| 修改 DFX/日志 | 编译通过 + `hisysevent.yaml` 事件定义未被破坏 + 相关 DFR 单测 |
| 修改特性标志 | 编译通过 + 受影响模块单测 + 检查 `ability_runtime.gni` 与 `bundle.json` features 一致性 |
| 新增 Extension 类型 | 编译通过 + 按步骤清单（7 步）全部完成 + 各层绑定编译 + 服务端管理单测 |
| 仅测试变更 | 运行变更的测试 + 至少一个相邻相关测试 |

### Done definition

任务完成仅当：
1. 请求的行为已实现。
2. 相关编译/测试/lint/兼容性检查已运行，或已说明无法运行的原因。
3. 最终回复包含：变更摘要、变更文件列表、验证命令和结果、兼容性/权限/DFX 影响评估（如相关）。
4. 不包含无关的格式化、重构或附带变更。
5. 如涉及公共 API 变更，已标注兼容性影响。

### Final response format

完成任务时，回复应包含：
- 变更摘要
- 变更文件列表
- 验证命令和结果
- 兼容性、权限、DFX 或跨设备影响（如相关）
- 遗留风险或后续项

## 5. 代码检视

当用户表达"检视一下代码""帮我审一下""做一次 code review""深度扫描""安全审计""查高危 bug""接口审计"等检视意图时，**不要凭记忆审查**——从代码检视工作台出发：

**入口**：[`skills/codecheck/README.md`](skills/codecheck/README.md)

该 README 是检视的导航与编排中枢，按以下顺序使用：

1. **先读 README**：了解每个子 skill 的检视维度、触发场景、输出格式，以及按场景选择的 skill 组合表。
2. **界定范围**：明确目标路径或 Kit、检视重点（通用/安全/高危 bug/API/外部输入）。未明确时向用户确认。
3. **调用子 skill**：读对应子目录的 `SKILL.md` 按其工作流执行。通用检视默认走 `deep-scan {path}`（一次拿 bug+logic+security 三维度）；接口/SDK 变更补 `api-audit`；服务侧 IPC/持久化密集区补 `external-input-audit`。
4. **合并报告**：把各 skill 产出汇总为一份 `codecheck_report_<scope>_<YYYYMMDD>.md`，跨维度去重后按 P0/P1/P1 排序。报告结构见 README 的 Step 4。

### 检视维度速查

| 维度 | 子 skill | 何时用 |
|------|---------|--------|
| 高影响缺陷（崩溃/挂死/OOM/UAF/死锁/泄漏） | `high-impact-bug-audit` | 通用检视、合入前排查 |
| 逻辑影响与一致性 | `logic_analyzer` | 变更影响评估、状态机/边界 |
| 安全/商用前审查 | `security_review` | 安全专项、商用前 |
| 外部输入→持久化健壮性 | `external-input-audit` | 服务侧、IPC/DB/文件密集区 |
| 对外 API 一致性 | `api-audit` | 接口/SDK 变更、Kit 级审计 |
| 三层编排（bug+logic+security） | `deep-scan` | 通用深度扫描的默认入口 |

### 检视约束

- **静态语言实现默认排除**（`frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/`、`ets_*.cpp`、`cj_*.cpp`），除非用户明确要求包含。
- **证据要求**：每条发现可追溯到 `file:line` + 触发路径，不收无证据的代码气味项。
- **检视阶段不改源码**：只产出报告与建议；修复由用户确认后另起任务，并回到本 AGENTS.md 的 Constraints/Verification 章节执行。

## 6. AI 开发操作规范（必须遵守）

以下规则来源于实际开发中踩过的坑，每次改动必须严格遵守。

### 6.1 提交前必须检查分支和状态

**每次 commit/amend/push 之前，必须执行以下命令并确认结果：**

```bash
git branch --show-current    # 确认当前分支是目标分支
git status --short           # 确认改动文件列表符合预期
git log --oneline -3         # 确认最近提交是自己的改动
```

**原因**：曾在 `feature/subspawn-support` 分支上误执行了 `git commit --amend`，把无关分支的提交消息覆盖了，导致改动丢失。多分支并行开发时，AI 可能停留在上一次操作遗留的分支上。

**修复步骤**：如果发现 commit 到了错误分支，立即执行 `git reset --hard HEAD~1` 恢复，然后切回正确分支重新操作。

### 6.2 新增/修改函数必须检查 BUILD.gn 链接

**每次新增函数或新增文件后，必须检查以下三个维度：**

| 检查项 | 怎么查 | 影响范围 |
|---|---|---|
| 源文件是否在 sources 列表 | `grep "文件名" <target>/BUILD.gn` 或 `grep "文件名" <target>/*.gni` | 新增 .cpp 文件必须加入对应 target 的 sources |
| include 路径是否在 include_dirs | `grep "头文件目录" <target>/BUILD.gn` | 新增 .h 文件所在目录必须在消费者 target 的 include_dirs |
| 外部依赖是否在 external_deps | `grep "库名" <target>/BUILD.gn` | 使用新库时必须加入 external_deps，且确认是无条件还是有条件（`if (xxx_enable)`） |

**Mock 文件同步检查**：如果修改了被 mock 的类（如 `AppScheduler`），必须检查所有使用该 mock 的测试 target 的 mock .cpp 文件是否也实现了新方法。

**查找受影响的 mock**：
```bash
# 找出编译该源文件的所有测试 target
grep -rn "源文件名" test/*/BUILD.gn
# 找出使用 mock 版本的 target
grep -rn "mock_源文件名" test/*/BUILD.gn
```

**原因**：曾在 `AppScheduler` 上新增了 `CheckProcessAvailable` 方法，但 7 个测试 target 使用的 mock `AppScheduler` 没有该方法，导致链接失败。修复了 2 个 mock 文件，另外 5 个不需要改（因为它们不编译 `ability_connect_manager.cpp`）。

### 6.3 日志级别规范

| 级别 | 何时用 | 例子 |
|---|---|---|
| `TAG_LOGE` | 错误场景：IPC 失败、超时、null 检查失败、鉴权失败 | `QueryProcessStateWithCallback failed` |
| `TAG_LOGW` | 原有代码就是 W 级别的（不改） | `SetProcessPrepareExit appRecord null` |
| `TAG_LOGI` | 原有代码就是 I 级别的（不改） | `ui_ability_lifecycle_manager` 的 SetProcessPrepareExit |
| `TAG_LOGD` | 我们新增的正常流程日志 | `QueryProcessStateWithCallback begin/sent` |

**原则**：新增日志根据语义选级别；原有日志保持原级别不改。不要把自己新增的 W/I 级别日志改成 D。

### 6.4 sptr 对象创建规范

**使用 `MakeSptr` 而非 `new`**，并必须判空：

```cpp
// 正确
sptr<MyClass> callback = sptr<MyClass>::MakeSptr();
if (callback == nullptr) {
    TAG_LOGE(...);
    return;  // 或 return 默认值
}

// 错误：new 可能失败导致空指针崩溃
sptr<MyClass> callback = new MyClass();
```

### 6.5 IPC 新增方法全链路检查清单

在 `IAmsMgr`（或类似 IPC 接口）新增方法时，必须同时修改以下所有层，缺一层就会链接失败或编译报错：

| 层 | 文件 | 检查内容 |
|---|---|---|
| 接口定义 | `ams_mgr_interface.h` | 方法声明 + IPC code 枚举值 |
| Proxy | `ams_mgr_proxy.h/.cpp` | 方法 override + IPC 发送实现 |
| Stub | `ams_mgr_stub.h/.cpp` | dispatch case + handler 实现 |
| Scheduler | `ams_mgr_scheduler.h/.cpp` | 方法 override + 鉴权 + 委托给 inner |
| Client | `app_mgr_client.h/.cpp` | 方法声明 + 获取 proxy 调用 |
| AMS Scheduler | `app_scheduler.h/.cpp` | passthrough 方法 |
| 调用方 | 业务代码 | 通过 `DelayedSingleton<AppScheduler>::GetInstance()->方法名()` |

### 6.6 锁持有期间调 IPC 的注意事项

在持有锁（如 `serialMutex_`）期间调用同步 IPC（如 `CheckProcessAvailable` 内部的 `WaitForResult`），会阻塞其他需要该锁的线程。修改前必须：

1. 确认该锁保护了哪些函数（`grep "锁名" <文件>.cpp`）
2. 评估阻塞时长对其他调用方的影响
3. 如果阻塞时间长（如 condition_variable 等待 100ms），考虑将 IPC 调用移出锁外（但需评估竞态风险）

### 6.7 跨进程调用链验证

修改涉及跨进程调用时，用以下命令验证每一层的签名和调用关系：

```bash
# 检查方法在所有层的签名是否一致
grep -rn "方法名" services/ interfaces/ --include="*.h" --include="*.cpp" | grep -v test

# 检查 IPC code 枚举值在 proxy 和 stub 中是否一致
grep -rn "IPC_CODE" interfaces/ --include="*.cpp"

# 确认调用链上下游类型匹配（IPC 传基本类型，inner 接收复杂类型时需要转换）
grep -n "方法签名" <每一层的文件>
```

### 6.8 测试用例同步更新

每次修改函数行为后，必须同步更新测试用例：

| 改动类型 | 测试更新 |
|---|---|
| 新增函数 | 新增对应用例（正常路径 + 边界条件 + 异常路径） |
| 删除函数内部逻辑 | 检查已有用例的 EXPECT 是否仍然通过 |
| 修改返回值语义 | 更新用例的期望值 |
| 新增类 | 新增类的生命周期测试（构造、回调、超时等） |

**测试 BUILD.gn 也需要检查**：测试目录的 BUILD.gn 可能需要加 `include_dirs` 和 `external_deps`。

### 6.9 修改 include / 基类前必须验证依赖存在

**改 `#include` 或继承基类之前，必须先确认被引用的头文件/类在当前工作空间的依赖仓中存在：**

```bash
# 确认头文件存在
find <依赖仓路径> -name "要引用的头文件名"
# 确认类存在
grep -rn "class 类名" <依赖仓路径> --include="*.h"
```

**原因**：曾根据 memmgr 团队的口头改动说明，将基类从 `MemoryObserverStub` 改为 `ProcessStateCallbackStub`，但该类在当前工作空间的 memmgr 仓中不存在（对方的改动尚未合入），导致编译报错"file not found"。

**核心原则**：口头/文档描述 ≠ 代码现状。修改任何跨仓依赖（include、基类、接口签名）之前，必须在当前工作空间中实际验证依赖存在，不能仅凭对方的描述就改代码。

### 6.10 依赖仓变更合入时机确认

当依赖仓（如 memmgr）的团队告知"有新接口/有重构"时，必须确认：

| 确认项 | 方法 |
|---|---|
| 新接口是否已在当前工作空间合入 | `ls` / `find` 查看依赖仓的实际文件 |
| 新接口的签名是否与描述一致 | `grep -rn "方法名" <依赖仓>` |
| 依赖仓的分支是否与描述一致 | `git -C <依赖仓> branch --show-current` + `git log --oneline -3` |

**如果依赖仓的实际代码与描述不符**：以实际代码为准，不以描述为准。改完后同步告知对方存在差异。