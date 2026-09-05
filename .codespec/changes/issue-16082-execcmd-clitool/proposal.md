---
target_release: 7.1
change_type: enhancement
issue: "16082"
author: ""
date: "2026-09-03"
status: Draft
---

# Proposal

## 背景与问题

`cliManager.execCmd(cmd, options)` 当前仅支持 shell 命令执行：将 `cmd` 字符串透传给 `/bin/sh -c "<cmd>"`，在沙箱中执行。这意味着：

- 只能执行 shell 命令（echo、ls 等），无法调用已注册的 CLI 工具（ohos-aa、ohos-hdc 等）
- LLM 输出的工具命令字符串（如 `"ohos-aa start --bundleName com.xxx --abilityName EntryAbility"`）需要先手动解析为结构化参数再走 `execTool`，多一层转换
- `execCmd`（shell）和 `execTool`（结构化）是两条独立路径，调用方需要判断用哪个接口

## 用户场景与业务触发

### US-1: LLM 工具命令直通执行
- **角色**：上层 AI/LLM 业务调用方
- **业务触发**：LLM 生成工具命令字符串（如 `ohos-aa start --bundleName ...`）后需经 cliManager 执行
- **业务价值**：调用方无需自行解析为结构化参数，execCmd 一个接口统一覆盖 shell 命令与工具命令两类场景，降低集成成本

## 初始分级判断

> 变更类型见 frontmatter `change_type`（new-feature/enhancement/optimization/bugfix/refactor/deprecation）。
> L0/简单变更：非目标、影响范围等章节可简化为一句话；L1+ 建议完整填写所有表格。

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 变更类型 | enhancement | 在既有 execCmd 上新增可选参数与行为分支（工具命令模式） |
| 复杂度 | 标准 | 跨接口/IPC/NAPI/客户端/服务端多层，但复用既有 execTool 校验与沙箱链路 |
| 涉及仓数量 | 2 | ability_runtime（实现）+ interface_sdk-js（.d.ts 公共 API 声明） |
| 是否涉及 Public/System API | 是 | ExecCmdOptions 新增两个可选 System API 属性 |
| 是否涉及安全/性能关键路径 | 是 | 命令执行路径，涉及权限校验、沙箱、跨进程 IPC |
| 是否跨 SIG | 否 | 均在 CLI 工具框架范围内 |

## 目标

- 为 `execCmd` 增加工具命令模式：当 `isShellCommand=false` 时，`cmd` 被视为拼接好的工具命令字符串，经过与 `execTool` 相同的校验和执行流程
- 统一入口：`execCmd` 一个接口覆盖 shell 命令和工具命令两种场景
- 复用现有 `execTool` 的校验、沙箱、进程管理链路，不引入额外安全风险
- 次要类型：兼容性（IPC 向后兼容与公共 API 可选属性默认值不变）

## 非目标

- 不修改既有 shell 路径（`isShellCommand=true` 默认行为不变）
- 不支持短标志（`-b`）、前缀参数、条件标志——列入后续迭代
- 不变更 `execTool` 接口签名
- 不新增第三方依赖
- 不修改 `ICliToolManager.idl` 的 ExecCmd 接口签名（ExecCmdParam 已是 sequenceable）

## Agent Scope Guard

> 约束 AI/Agent 的探索和修改边界。若需要突破边界，先更新本 proposal 并获得确认。

| 维度 | 范围/限制 | 需人工确认的触发条件 |
|------|-----------|----------------------|
| 允许仓库 | ability_runtime（cli_tool_framework）、interface_sdk-js（.d.ts 中英） | 跨仓 .d.ts 修改需人工确认 |
| 允许模块/目录 | cli_tool_framework/{interfaces/cli_tool, services/climgr, frameworks/js/napi/cli_tool_manager} | |
| 禁止修改项 | ICliToolManager.idl 的 ExecCmd 签名；既有 shell 路径行为；hisysevent 既有事件定义；公共错误码数值；ExecTool 接口 | |
| 外部依赖/网络访问 | 无新增 | |

## 成功标准

> 每条标准写**系统级能力达成**（可观察、可量化）。**禁内部实现**（注册表/指针/状态机）+ **禁接口细节**（签名/参数/错误码——属 spec AC）。例：✅"焦点变化可被应用层感知"，❌"onFocus(focusSource=Tab) 触发"（接口细节）、❌"焦点注册表存在节点"（内部实现）。"提升性能"不可验证，"P95 延迟 < 100ms"可验证。

| 标准 | 可观察指标 | 验证方式 |
|------|-----------|---------|
| execCmd 工具命令模式可调用已注册 CLI 工具并完成执行 | 工具命令被执行，会话结果可查询 | 集成测试：调用已注册工具，会话产生预期结果 |
| 默认行为（shell 模式）与改动前一致 | 既有 shell 命令调用全部回归通过 | 回归测试 |
| 非法工具名（含路径分隔符）被拦截 | 含 `/` 或 `.` 的工具名返回校验失败且不创建进程 | 安全测试 |
| 新增能力对既有 IPC 客户端向后兼容 | 旧客户端行为不变、不崩溃 | IPC 兼容性测试 |

## 影响范围

| 子系统 | 仓库 | 模块/路径 | 影响类型 |
|--------|------|-----------|---------|
| CLI 工具框架 | ability_runtime | cli_tool_framework/interfaces/cli_tool（exec_cmd_param、cli_tool_mgr_client） | 接口/IPC 数据层 |
| CLI 工具框架 | ability_runtime | cli_tool_framework/services/climgr（cli_tool_manager_service、tool_util） | 服务端实现 |
| CLI 工具框架 | ability_runtime | cli_tool_framework/frameworks/js/napi/cli_tool_manager（js_cli_manager、js_cli_manager_utils） | NAPI 绑定 |
| 公共 API | interface_sdk-js | api/@ohos.app.cli.cliManager.d.ts（中英） | 公共 API 声明 |

## 假设与开放问题

### 假设
- ExecToolParam 已支持 challenge 透传（参考 execTool 既有实现）
- 已注册 CLI 工具的 schema 可驱动 ParseToolCommand 类型恢复
- 工具命令字符串仅使用长标志语法，短标志/前缀参数为后续迭代

### 开放问题
- challenge 字段来源（access token manager）的获取时机与 execTool 是否完全一致，需 spec/design 确认
- toolName 白名单字符集 `[a-zA-Z0-9-_]` 是否覆盖所有已注册工具名，需核对工具注册表
- ParseToolCommand 对带空格的参数值（如 `--desc "a b"`）是否需引号支持，待 spec 确认

## 资源开销审视

> 三维状态：`required` / `review-required` / `not-applicable` / `waived`。测量、预算与证据 schema 由子系统 `odk_resource_gate` 负责。
> 存储(ROM) 主语：交付进 OS 版本镜像的产物 + 运行后 data 分区下的大体量持久化；阈值归子系统 gate。

| 维度 | 状态 | 信号/依据 | 确认人/理由/范围 |
|------|------|-----------|-----------------|
| 性能 | not-applicable | CLI 执行为按需低频路径，非热路径；工具命令模式复用 execTool 既有校验/沙箱/进程链路；ParseToolCommand 为单次 O(n) 分词，无每帧/每事件开销 | 确认人=CLI工具框架负责人（待确认）; 理由=按需执行非热路径且复用既有链路; 范围=cli_tool_framework |
| 内存(RAM) | not-applicable | 无新增常驻结构；命令字符串与解析结果为单次调用内临时对象，调用结束即释放 | 确认人=CLI工具框架负责人（待确认）; 理由=无常驻增量; 范围=cli_tool_framework |
| 存储(ROM) | not-applicable | 仅在既有 .so 内增量代码（ParseToolCommand/ExecCmdToolMode），无新增镜像产物或大体量预置数据 | 确认人=CLI工具框架负责人（待确认）; 理由=增量代码不构成镜像产物关注点; 范围=cli_tool_framework |

## 不涉及项确认

> 逐维度主动判断"涉及/不涉及"并填写依据；不允许留空；标"不涉及"必须写明理由。
> 「性能」行为摘要（过渡字段，后续以「资源开销审视」为准并废弃本行独立语义）：只由上方「性能」状态推导——`required` / `review-required` 为「是」，`not-applicable` / `waived` 为「否」；RAM/ROM 状态不改变本行语义。

| 维度 | 是否涉及 | 依据 |
|------|---------|------|
| 性能 | 否 | 资源开销审视「性能」= not-applicable：按需低频路径，非热路径，复用既有链路 |
| 安全/权限 | 是 | 命令执行路径跨信任边界（IPC/Binder）、复用权限校验链路、处理 challenge 认证标识、toolName 白名单防注入；命中安全判据 |
| 兼容性 | 是 | IPC Parcel 末尾追加字段需向后兼容；公共 API 新增可选属性需默认行为不变 |
| API/SDK | 是 | ExecCmdOptions 新增 isShellCommand、challenge 两个可选 System API 属性 |
| IPC/跨进程 | 是 | ExecCmdParam Marshalling/Unmarshalling 变更，跨进程 CLI 工具管理 IPC |
| 构建/组件 | 否 | 仅修改既有源文件，无新增构建目标/组件/第三方依赖 |
| 国际化/无障碍 | 否 | 无 UI 文本、无用户可感知字符串，命令解析不涉及国际化 |
| 数据迁移 | 否 | 无持久化数据格式变更；IPC Parcel 为运行时兼容，向后兼容 |

## API 设计属性

> 本章节定义 API 设计的基本属性，后续 spec 和 implement 阶段将基于这些信息进行设计。
> 如不涉及 API/SDK 设计，填写"不涉及"并说明理由（如纯内部实现变更/纯配置变更/纯文档变更等）。

### API 范围声明

| 属性 | 值 | 说明 |
|------|-----|------|
| **涉及 Kit** | AbilityKit | 模块路径 @ohos.app.cli.cliManager，隶属 @ohos.app.* 元能力体系 |
| **API 类型** | system | System API，仅对系统应用开放；涉及沙箱执行与 access token challenge |
| **编程语言** | ArkTS | 通过 NAPI 暴露 @ohos.app.cli.cliManager 模块，生成 .d.ts |
| **ArkTS 静态类型 API** | 是 | 支持 @ohos.* 静态类型模块导入 |

> **填写说明**：
> - **涉及 Kit**：参考 OpenHarmony Kit 列表，填写完整的 Kit 名称
> - **API 类型**：根据 API 的访问级别填写，system API 需要特殊权限
> - **编程语言**：ArkTS API 生成 `.d.ts` 文件，C API 生成 `.h` 文件
> - **ArkTS 静态类型 API**：静态类型 API 使用 `@ohos.*` 模块导入，动态类型使用命名空间导入
>
> **注**：跨平台支持、元服务支持、卡片支持等属性在 spec.md `## API 规格定义` 中填写。

### API 使用场景探索（可选）

> 如有复杂的 API 使用场景，可在此处记录开发者场景和用户画像，帮助后续设计阶段理解上下文。

**开发者场景**：
- 场景 1：LLM 业务调用方直接输出工具命令字符串，经 execCmd 执行，无需调用方解析为结构化参数
- 场景 2：统一入口需求——同一接口按需切换 shell 命令与工具命令两种模式

**用户画像**：
- 画像 1：AI/Agent 业务集成开发者，需要将 LLM 生成的工具命令透传给 CLI 框架执行
