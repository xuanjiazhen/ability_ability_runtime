# AGENTS.md

## 1. 概述

本 AGENTS.md 用于指导开发与应用运行时管理相关特性分析、需求导入、设计、编码及验证各环节。在SDD流程开发各阶段，涉及本代码仓所有修改需要遵从本文档描述及其指向的其他更细则文档的内容，如果本文档指向的其他文档存在与本文档规定冲突内容，以本文档内容为准。本文档描述内容包括代码目录、知识体系、约束与限制三个章节，其中代码目录等内容就在本代码仓中，所引用的详细知识库内容均引自https://gitcode.com/abilitykit_public/abilitykit_sdd，在执行具体要求前，应在ability_runtime代码仓同级目录下执行git clone https://gitcode.com/abilitykit_public/abilitykit_sdd.git 下载代码仓后，在相应的代码仓中查找具体规则。

本仓库实现 OpenHarmony **元能力运行时（ability_runtime）**，核心职责是管理应用生命周期、组件调度、Extension 扩展组件管理、URI 权限管理和意图框架。最重要的架构边界是 **服务层（services/）与公共 SDK 接口层（interfaces/kits/）之间的双向解耦**——服务不可直接暴露内部实现到 SDK，SDK 不可反向依赖服务内部细节。

### 代码目录职责

| 目录                                                       | 职责                                                         | 高风险标记                                 |
| ---------------------------------------------------------- | ------------------------------------------------------------ | ------------------------------------------ |
| `services/abilitymgr/`                                     | AbilityManagerService（SA 180）：组件生命周期、启动/停止、连接、任务栈、拦截器、InsightIntent | 🔴 修改影响全局组件调度                     |
| `services/appmgr/`                                         | AppManagerService（SA 501）：应用进程生命周期、AppSpawn 协调、进程状态、子进程 | 🔴 修改影响所有应用进程                     |
| `services/uripermmgr/`                                     | UriPermissionManagerService（SA 183）：跨应用 URI 权限授予/撤销/校验 | 🔴 修改影响安全边界                         |
| `services/quickfixmgr/`                                    | QuickFixManagerService（SA 184）：应用热修复补丁管理         | 🟡 修改影响应用更新流程                     |
| `services/appdfr/`                                         | 应用 DFR：AppFreeze 管理、ANR 监听、崩溃收集                 | 🟡 修改影响故障归因                         |
| `services/common/`                                         | 服务侧工具封装：权限校验、日志封装、事件上报、HiSysEvent、XCollie | 🔴 包含 PermissionVerification 全局安全入口 |
| `interfaces/kits/native/`                                  | 公共 SDK 接口：Ability、Context、Extension、Caller/Callee、AppStartup | 🔴 公共 API 兼容性边界                      |
| `interfaces/kits/c/`                                       | C 语言公共 API                                               | 🔴 公共 API 兼容性边界                      |
| `interfaces/inner_api/`                                    | 内部组件接口：IPC 客户端/代理/桩、错误码工具                 | 🟡 内部接口，但仍需注意 IPC 兼容性          |
| `frameworks/native/ability/native/`                        | 原生框架实现：UIAbility、各 Extension、Context、InsightIntent 执行器 | 🟢 客户端侧实现，可随 SDK 接口联动修改      |
| `frameworks/native/appkit/`                                | 应用框架：Application、AbilityStage、TestRunner              | 🟢                                          |
| `frameworks/js/napi/`                                      | JavaScript NAPI 绑定                                         | 🟡 NAPI 签名是公共 API                      |
| `frameworks/ets/ani/`                                      | ArkTS ANI 绑定                                               | 🟡 ANI 签名是公共 API                       |
| `frameworks/cj/ffi/`                                       | CJ FFI 绑定                                                  | 🟡                                          |
| `frameworks/c/`                                            | C API 框架实现                                               | 🟡                                          |
| `agent_runtime_framework/`                                 | Agent 组件框架：AgentManagerService（SA 185）、AgentExtensinoAbility与AgentUIExtensionAbility | 🟢                                          |
| `tools/aa/`                                                | aa 命令行工具：Ability 调试管理                              | 🟢                                          |
| `js_environment/` / `ets_environment/` / `cj_environment/` | JS/ETS/CJ 运行时环境                                         | 🟢                                          |



## 2. 知识体系

### 2.1 Summarys: 领域概念与概述

**文档路径：**abilitykit_sdd/summarys

**文档内容**：abilitykit业务范围关键概念、概述、设计约束等基础知识。

**文档适用**：涉及abilitykit领域的相关问题、需求及业务讨论，均需要遵从本目录下文档所述相关术语、概念及要求。例如用户说"多HAP需要新增一个XXX规格"，需要先通过本目录下的文档查询多HAP相关概念。

**SDD适用**：可在Define/Specify/Design/Plan等各阶段使用，重点在Define阶段使用。

### 2.2 Standards：编码与设计规范

**文档路径：**abilitykit_sdd/standards

**文档内容**：abilitykit业务范围编码规范要求及通用设计规范要求基础知识。

**文档适用**：涉及abilitykit领域相关代码仓（主要包括ability_ability_runtime/ability_ability_base/bundlemanager_bundle_framework等代码仓）的代码修改，均需要严格遵从本文档规则。

**SDD适用**：可在Design/Plan等各阶段使用，重点在Plan阶段使用。

### 2.3 Specifications：模块场景与特性规格

**文档路径：**abilitykit_sdd/specifications

**文档内容**：abilitykit业务范围全量模块、业务场景及特性规格等基础知识。

**文档适用**：涉及abilitykit领域的相关问题、需求及业务讨论，均需要参考本目录下相关模块、场景及特性等基础知识。新增需求或业务讨论如果是或者新增模块、场景或特性，则需要关注不要与现有规格出现冲突；新增需求或业务讨论如果是对原有模块、场景或特性进行调整的，则需要和用户交互明确相关模块、场景和特性变更点。需要注意：不管是哪种情况，一旦出现specifications内容的变更（新增、删除或调整），都必须以当前用户身份向abilitykit_sdd代码仓中按照原有格式提交变更，并推送到远端请求Committer合入。

**SDD适用**：可在Define/Specify/Design/Plan等各阶段使用，重点在Specify/Design阶段使用。

### 2.4 Suits：研发工具套件

**文档路径：**abilitykit_sdd/suits

**文档内容**：abilitykit业务范围开发自测试工具套件。

**文档适用**：涉及abilitykit领域的代码在完成编码后进行测试验证阶段时使用本目录下的相关工具。

**SDD适用**：可在Plan阶段及Plan之后的阶段使用。



## 3.编码、验证、检视及上库规则

### 3.1 代码编写：Coding

**触发条件**：SDD流程中Plan阶段主动触发或用户主动要求生成代码时触发。

**执行规则**：

1）编码前严格读取abilitykit_sdd/standards所有规则后开始编码，编码及后续修改不得出现违反规则的实现。

2）所有业务代码修改都需要包含测试用例的同步编写和生成。

### 3.2 功能验证：Verify

触发条件：完成编码之后，用户要求执行功能验证时触发。

执行规则：

1）编译：代码修改完成后，需要首先进行编译。编译包括全量编译和部分编译。

注意：构建命令从 OpenHarmony 源码根目录执行，不在本子目录执行，否则会编译失败。

- 全量编译：`./build.sh --product-name <product>`
- 仅编译ability_runtime：`./build.sh --product-name <product> --build-target ability_runtime`
- 编译特定服务：`./build.sh --product-name <product> --build-target abilityms`（AMS）或 `libappms`（AppMS）

在功能验证阶段，需要优先构建模拟器镜像，编译构建完成后再启动RK版本镜像构建，全量构建按照如下顺序进行：

Step1: 

执行prebuilts,在源码根目录下执行，安装编译器及二进制工具: `bash build/prebuilts_download.sh`

Step2:根据模拟器还是RK版本选择如下一个命令执行。

模拟器：`./build.sh --product-name x86_64_virt --ccache --deps-guard=false --gn-args runtime_core_enable_codegen=false`

RK3568: `./build.sh --product-name rk3568  --ccache`

2）烧录：代码编译完成后，需要执行烧录镜像和启动运行操作。当前仅支持烧录模拟器，通过abilitykit_sdd/suits/simulator_user skill使用模拟器

3）自测试验证：验证应包括至少三部分：UT、XTS、功能验证。

UT：





### 3.3 代码检视：Review

### 3.4 代码上库：Commit



### 2.5 Review

代码检视阶段需要继续读取abilitykit_sdd/standards下的通用编码及架构设计要求规则，进行多轮代码检视。

当用户表达"检视一下代码""帮我审一下""做一次 code review""深度扫描""安全审计""查高危 bug""接口审计"等检视意图时，**不要凭记忆审查**——从代码检视工作台出发：

**入口**：[`skills/codecheck/README.md`](skills/codecheck/README.md)

该 README 是检视的导航与编排中枢，按以下顺序使用：

1. **先读 README**：了解每个子 skill 的检视维度、触发场景、输出格式，以及按场景选择的 skill 组合表。
2. **界定范围**：明确目标路径或 Kit、检视重点（通用/安全/高危 bug/API/外部输入）。未明确时向用户确认。
3. **调用子 skill**：读对应子目录的 `SKILL.md` 按其工作流执行。通用检视默认走 `deep-scan {path}`（一次拿 bug+logic+security 三维度）；接口/SDK 变更补 `api-audit`；服务侧 IPC/持久化密集区补 `external-input-audit`。
4. **合并报告**：把各 skill 产出汇总为一份 `codecheck_report_<scope>_<YYYYMMDD>.md`，跨维度去重后按 P0/P1/P1 排序。报告结构见 README 的 Step 4。

### 检视维度速查

| 维度                                      | 子 skill                | 何时用                    |
| ----------------------------------------- | ----------------------- | ------------------------- |
| 高影响缺陷（崩溃/挂死/OOM/UAF/死锁/泄漏） | `high-impact-bug-audit` | 通用检视、合入前排查      |
| 逻辑影响与一致性                          | `logic_analyzer`        | 变更影响评估、状态机/边界 |
| 安全/商用前审查                           | `security_review`       | 安全专项、商用前          |
| 外部输入→持久化健壮性                     | `external-input-audit`  | 服务侧、IPC/DB/文件密集区 |
| 对外 API 一致性                           | `api-audit`             | 接口/SDK 变更、Kit 级审计 |
| 三层编排（bug+logic+security）            | `deep-scan`             | 通用深度扫描的默认入口    |

### 检视约束

- **静态语言实现默认排除**（`frameworks/ets/ani/`、`frameworks/ets/ets/`、`frameworks/cj/ffi/`、`ets_*.cpp`、`cj_*.cpp`），除非用户明确要求包含。
- **证据要求**：每条发现可追溯到 `file:line` + 触发路径，不收无证据的代码气味项。
- **检视阶段不改源码**：只产出报告与建议；修复由用户确认后另起任务，并回到本 AGENTS.md 的 Constraints/Verification 章节执行。



## 4. 约束与限制

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

- 单元测试：`run -t UT -tp ability_runtime`
- 模块测试：`run -t UT -ts ability_caller_fw_module_test`
- Fuzz 测试：`run -t UT -ts AbilityAppDebugInfoFuzzTest`

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