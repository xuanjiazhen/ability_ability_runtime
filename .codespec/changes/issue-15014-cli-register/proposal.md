---
target_release:
  id: OpenHarmony-7.0
  status: proposed
---

# 需求文档

> 一份文档，从原始需求到基线结论。本需求依据 `cli_tool_framework` 中 `OnStart()` → `EnsureToolsLoaded()` → `LoadToolsFromDir()` 配置文件加载与 KVStore 更新的既有实现反推并固化，待需求方/Owner 确认后基线。

## 一、原始需求

### 基本信息

| 字段 | 内容 |
|------|------|
| 需求ID | REQ-015 |
| 需求名称 | CLI 工具注册（cli-register）：OnStart 配置加载与 KVStore 同步 |
| 来源 | 待确认（实现已存在于 cli_tool_framework，2026 版权） |
| 提出人 | 待确认 |
| 目标发行版本 | OpenHarmony-7.0 |
| 候选 Profile | none |
| 优先级 | P1 |
| 状态 | Clarifying |

### 原始描述

**原始问题：** CLI 工具框架需要在 SA 186（aimgr）启动时，从系统配置目录读取 JSON 格式的工具注册文件，解析为 ToolInfo 结构，并更新到 KVStore 数据库中，使后续查询接口（queryToolSummaries/getToolInfoByName/queryTools）能返回最新注册的工具元数据。配置文件可能新增、修改或删除，每次 SA 启动时须将 KVStore 与配置目录同步：新增/更新的工具写入 KVStore，配置中已不存在的旧工具从 KVStore 删除。

**痛点：**

| 用户类型 | 当前痛点 | 影响 |
|----------|----------|------|
| 平台/工具开发者 | 工具注册文件变更后无自动同步机制 | 需手动操作 KVStore 或重启清除数据 |
| 系统安全/审计方 | 缺少配置文件到数据库的可追溯加载链路 | 无法审计工具注册来源 |
| 系统应用开发者 | SA 启动后查询不到最新注册的工具 | 工具发现失败 |

**期望结果：** SA 186 在 `OnStart()` 中调用 `EnsureToolsLoaded()`，从配置目录 `/system/bin/cli_tool/configs` 读取所有 `.json` 工具注册文件，经 JSON 解析与字段校验后写入 KVStore，并同步删除配置中已不存在的旧工具，确保 KVStore 始终与配置目录一致。

### 背景证据

| 证据类型 | 链接/路径 | 说明 |
|----------|-----------|------|
| 源码实现 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp:290-323` | `OnStart()` 调用 `EnsureToolsLoaded()` |
| 源码实现 | `cli_tool_framework/services/climgr/src/cli_tool_data_manager.cpp:106-167` | `EnsureToolsLoaded()` + `LoadToolsFromDir()` |
| 源码实现 | `cli_tool_framework/services/climgr/src/cli_tool_data_manager.cpp:217-260` | `ParseToolFromJsonFile()` JSON 解析+BOM 处理 |
| 源码实现 | `cli_tool_framework/services/climgr/src/cli_tool_data_manager.cpp:169-215` | `SyncToolNames()` 增删同步 |
| 源码实现 | `cli_tool_framework/services/climgr/src/cli_tool_data_manager.cpp:415-434` | `RestoreKvStore()` 数据损坏恢复 |
| 源码实现 | `cli_tool_framework/interfaces/cli_tool/src/tool_info.cpp:189-329` | `ToolInfo::ParseFromJson()` 字段校验 |
| 源码实现 | `cli_tool_framework/interfaces/cli_tool/src/tool_info.cpp:102-187` | `ValidateName()`/`ValidateExecutablePath()`/`ValidateRequirePermissions()` |
| 配置 | `cli_tool_framework/etc/profile/aimgr.cfg` | init 创建 KVStore 数据目录 `/data/service/el1/public/database/aimgr/cli_tool` |

### 初始范围

**可能包含：**
- `OnStart()` → `EnsureToolsLoaded()` 启动加载链路
- `LoadToolsFromDir()` 遍历配置目录读取 `.json` 文件
- `ParseToolFromJsonFile()` JSON 解析 + BOM 处理 + `ToolInfo::ParseFromJson()` 字段校验
- `StoreTool()` 工具写入 KVStore（key=toolName, value=JSON）
- `SyncToolNames()` 增删同步：新工具写入、旧工具删除、AllCliToolNames 索引更新
- `RestoreKvStore()` 数据损坏自动恢复
- KVStore 初始化：`CheckKvStore()` 重试 5 次（100ms 间隔）
- 错误码 35700004/35700014/35700015/35700016/35700018

**明确不包含：**
- `queryToolSummaries`/`getToolInfoByName`/`queryTools` 等查询接口（属 issue-15014-cli-query）
- `RegisterFunction`/`BatchRegisterFunctions` 等函数注册（由 foundation SA 通过 IPC 调用）
- `ExecTool`/`ExecCmd` 等执行接口
- 工具配置文件的具体内容/格式设计（仅约束解析行为）
- 跨设备工具同步

### 初始假设

| 假设 | 类型 | 验证方式 | 状态 |
|------|------|----------|------|
| 配置目录固定为 `/system/bin/cli_tool/configs` | 技术 | 源码 `DEFAULT_CONFIG_DIR` 常量 | 已验证 |
| 仅读取 `.json` 后缀文件 | 技术 | 源码 `LoadToolsFromDir` 后缀过滤 | 已验证 |
| 工具名须以 `ohos-` 或 `hms-` 开头且后缀 ≤32 字符 | 技术/安全 | 源码 `ValidateName` | 已验证 |
| 可执行路径须为绝对路径（以 `/` 开头） | 技术/安全 | 源码 `ValidateExecutablePath` | 已验证 |
| `requirePermissions` 数组须无重复项 | 技术 | 源码 `ValidateRequirePermissions` | 已验证 |
| `EnsureToolsLoaded()` 仅执行一次（原子标志 `toolsLoaded_`） | 技术 | 源码 `toolsLoaded_` 原子标志 | 已验证 |
| KVStore 初始化重试 5 次，每次间隔 100ms | 技术 | 源码 `CHECK_INTERVAL`/`MAX_TIMES` | 已验证 |
| 单个配置文件解析失败不中断整体加载 | 可靠性 | 源码 `LoadToolsFromDir` 逐文件 try-catch 语义 | 已验证 |

### 初始分级判断

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 复杂度 | 标准 | 服务启动初始化链路，涉及文件 I/O + JSON 解析 + KVStore 读写 + 增删同步 |
| 涉及仓数量 | 1 | `foundation/ability/ability_runtime` |
| 是否涉及 Public/System API | 否 | 内部实现行为，不暴露 API |
| 是否涉及安全/性能关键路径 | 是 | 工具元数据来源校验（名称前缀/路径绝对/权限去重）、KVStore 数据完整性 |
| 是否跨 SIG | 否 | 全部在 Sig-Ability 内 |

### 进入澄清条件

- [x] 原始问题和期望结果已记录
- [ ] 需求来源和责任人已明确（待确认）
- [x] 初始范围和不包含项已记录
- [x] 关键假设和待澄清问题已列出
- [x] 复杂度有判断或明确为待定

---

## 二、澄清记录

> 本需求无原始对话记录，澄清项依据实现反推。标记「已澄清（实现反推）」的项需 Owner 复核确认。

### 待澄清问题

| 编号 | 问题 | 为什么需要澄清 | 状态 |
|------|------|----------------|------|
| Q-1 | 配置目录是否固定为 `/system/bin/cli_tool/configs`？ | 决定加载路径 | 已澄清（实现反推：固定） |
| Q-2 | 是否仅加载 `.json` 后缀文件？ | 决定文件过滤规则 | 已澄清（实现反推：仅 .json） |
| Q-3 | 单个文件解析失败是否中断整体加载？ | 决定容错策略 | 已澄清（实现反推：不中断，跳过失败文件） |
| Q-4 | 工具名校验规则是否固定前缀+后缀长度？ | 决定安全约束 | 已澄清（实现反推：ohos-/hms- 前缀+≤32 后缀） |
| Q-5 | KVStore 数据损坏是否自动恢复？ | 决定可靠性行为 | 已澄清（实现反推：DATA_CORRUPTED 时删除并重建） |
| Q-6 | `EnsureToolsLoaded` 是否幂等（仅一次）？ | 决定重复调用行为 | 已澄清（实现反推：原子标志 toolsLoaded_ 保证仅一次） |
| Q-7 | 目标发行版本？ | 基线必需 | 待确认 |

### 讨论记录

| 日期 | 参与人 | 讨论主题 | 结论 | 后续动作 |
|------|--------|----------|------|----------|
| 2026-08-21 | AI 反推 | 配置加载链路 | OnStart→EnsureToolsLoaded→LoadToolsFromDir→ParseToolFromJsonFile→StoreTool→SyncToolNames | Owner 复核 |

### 功能范围确认

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 核心功能包含哪些？ | OnStart 加载配置文件→解析校验→写入 KVStore→增删同步→数据损坏恢复 | 待确认 | 已确认（反推）/待确认 |
| 明确不包含哪些？ | 查询接口/执行接口/函数注册/配置文件内容设计 | 待确认 | 已确认（反推）/待确认 |
| 是否有分期策略？ | 否 | 待确认 | 已确认（反推） |

### 方案探索

| 编号 | 方案概述 | 优势 | 风险/代价 | 选择结论 |
|------|----------|------|-----------|----------|
| A-1 | SA OnStart 从配置目录加载 JSON→解析校验→写入 KVStore→增删同步 | 集中加载、自动同步、配置变更即生效（重启后） | 启动时文件 I/O 延迟 | 推荐 |
| A-2 | 手动注册（每次工具变更手动调 IPC 注册） | 无启动延迟 | 运维成本高、易遗漏 | 放弃 |
| A-3 | 配置文件 inotify 热加载 | 实时生效 | 复杂度高、SA 可能未运行 | 放弃 |

**取舍理由：** SA 186 为 ondemand 按需拉起，每次拉起时从配置目录加载是最简可靠方案；文件 I/O 延迟在百毫秒级可接受；热加载引入复杂度且 SA 可能未运行时无法监听。

### 上下文与知识源检索日志

| 编号 | 来源 | 查询/读取内容 | 关键发现 | 可信度 | 用于 | 命中/原因 |
|------|------|---------------|----------|--------|------|-----------|
| K-1 | 源码 `cli_tool_manager_service.cpp:290-323` | `OnStart()` 调用链 | Init→EnsureToolsLoaded→Publish→DelayUnloadTask | 高 | 设计 | 命中 |
| K-2 | 源码 `cli_tool_data_manager.cpp:106-167` | `EnsureToolsLoaded`/`LoadToolsFromDir` | 懒加载原子标志；遍历 .json 文件；逐文件解析+存储 | 高 | 设计/测试 | 命中 |
| K-3 | 源码 `cli_tool_data_manager.cpp:217-260` | `ParseToolFromJsonFile` | ifstream 读取+BOM 去除+json::parse+ToolInfo::ParseFromJson | 高 | 设计/测试 | 命中 |
| K-4 | 源码 `cli_tool_data_manager.cpp:169-215` | `SyncToolNames` | 读旧名列表→diff→删旧→存新名列表到 AllCliToolNames key | 高 | 设计 | 命中 |
| K-5 | 源码 `cli_tool_data_manager.cpp:64-104` | `GetKvStore`/`CheckKvStore` | KVStore options（SINGLE_VERSION/S2/EL1/不加密）；重试 5 次 100ms 间隔 | 高 | 设计 | 命中 |
| K-6 | 源码 `cli_tool_data_manager.cpp:415-434` | `RestoreKvStore` | DATA_CORRUPTED 时 DeleteKvStore+重建 | 高 | 设计 | 命中 |
| K-7 | 源码 `tool_info.cpp:102-329` | `ValidateName`/`ParseFromJson` | 名称前缀 ohos-/hms-+≤32 后缀；路径绝对；权限去重；inputSchema/outputSchema 须 JSON object | 高 | 设计/测试 | 命中 |
| K-8 | 源码 `sub_command_info.cpp:94-166` | `SubCommandInfo::ParseFromJson` | description 非空+requirePermissions 数组+inputSchema/outputSchema JSON object | 高 | 设计 | 命中 |
| K-9 | `aimgr.cfg` | init cmds | 创建 /data/service/el1/public/database/aimgr/cli_tool 目录 | 高 | 构建 | 命中 |
| K-10 | `AGENTS.md` | 目标仓 Agent 指南 | 服务层与 SDK 层解耦；Mission 持久化数据格式跨版本约束 | 高 | 设计约束 | 命中 |

**上下文结论：**
- 高可信结论：配置加载链路、JSON 解析校验、KVStore 读写、增删同步、数据损坏恢复均已由源码固化。
- 待确认结论：目标发行版本、需求来源与责任人。
- 未使用来源及原因：未使用多仓知识库（本需求单仓内闭环）。

### 子系统影响

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 涉及哪些子系统？ | ability（ability_runtime） | 待确认 | 已确认（反推） |
| 是否需要新增子系统或部件？ | 否 | 待确认 | 已确认（反推） |

### API 变更评估

| 问题 | 回答 | 确认人 | 状态 |
|------|------|--------|------|
| 是否需要新增/修改 Public API？ | 否 | 待确认 | 已确认（反推） |
| 是否需要新增 System API？ | 否 | 待确认 | 已确认（反推） |
| 是否会废弃已有 API？ | 否 | 待确认 | 已确认（反推） |

### 兼容性与非功能需求

| 类别 | 核心问题 | 结论 | 确认人 | 状态 |
|------|----------|------|--------|------|
| 兼容性 | KVStore 数据格式变更？ | 不变更；ToolInfo JSON 序列化格式既有且不变 | 待确认 | 已确认（反推） |
| 性能 | 启动时文件 I/O 延迟？ | 配置文件数量有限（系统级），百毫秒级可接受 | 待确认 | 已确认（反推） |
| 安全 | 工具名校验/路径校验？ | 名称须 ohos-/hms- 前缀；路径须绝对路径；权限去重 | 待确认 | 已确认（反推） |
| 可靠性 | KVStore 不可用/损坏？ | 重试 5 次；DATA_CORRUPTED 自动重建；单文件失败不中断 | 待确认 | 已确认（反推） |

### 依赖与风险

| 依赖项 | 类型 | 说明 | 状态 |
|--------|------|------|------|
| `safwk`/`samgr` | 编译/运行 | SA 186 ondemand 拉起 | 已确认 |
| `kv_store` | 运行 | KVStore 数据存储 | 已确认 |
| `init` | 运行 | aimgr.cfg init 创建数据目录 | 已确认 |
| `nlohmann/json` | 编译 | JSON 解析库 | 已确认 |

| 风险 | 类型 | 影响 | 缓解措施 | 状态 |
|------|------|------|----------|------|
| 需求来源/Owner 未确认 | 进度 | 基线无法冻结 | 需求方/SIG 确认 | 待确认 |
| 目标版本未确认 | 进度 | 影响发布 | proposal.target_release 确认 | 待确认 |
| 配置目录不存在 | 可靠性 | 返回 ERR_FILE_NOT_FOUND，SA 继续启动（查询返回空列表） | 日志告警 | 已确认 |
| KVStore 长时间不可用 | 可靠性 | 重试 5 次后返回 ERR_KVSTORE_NOT_READY | 日志告警；后续查询返回 ERR_NO_INIT | 已确认 |

### AC 完整性

- [x] 每个用户故事有验收标准
- [x] AC 全部使用 WHEN/THEN 格式
- [x] 覆盖正常流程、异常流程、边界条件
- [x] AC 可测试、可度量

### 澄清结论

- [x] 功能范围已完全明确
- [x] 子系统影响已识别
- [x] API 变更已评估（无 API 变更）
- [x] 兼容性和非功能需求已确认
- [x] 依赖和风险已识别且有缓解方案
- [x] AC 完整可测试
- [x] 标准及以上复杂度已完成方案探索（A-1/A-2/A-3 + 取舍理由）

**结论:** 条件通过（实现反推完整，待 Owner/需求方确认后转「通过」）

---

## 三、需求基线

> 澄清完成后固化。manifest.md 是事实源，此处为审批结论。

### 基线信息

| 字段 | 内容 |
|------|-----|
| 基线版本 | v1.0 |
| 基线日期 | 2026-08-21 |
| Owner | 待确认 |
| 确认人 | 需求方/模块Owner/SIG代表（待确认） |
| 复杂度 | 标准 |
| Profile | none |
| 目标发行版本 | OpenHarmony-7.0 |
| 版本状态 | proposed |

### 问题陈述

CLI 工具框架缺少从配置文件到 KVStore 的自动注册同步机制。本需求固化 `OnStart()` → `EnsureToolsLoaded()` 启动加载链路，从配置目录读取 JSON 工具注册文件、解析校验后写入 KVStore，并同步删除已不存在的旧工具。

### 目标和成功指标

| 目标 | 成功指标 | 验证方式 |
|------|----------|----------|
| SA 启动时自动加载配置 | KVStore 内容与配置目录一致 | 单测 |
| 配置变更同步生效 | 新增/修改/删除工具在 SA 重启后反映到 KVStore | 单测 |
| 容错：单文件失败不中断 | 单个非法 JSON 跳过，其余正常加载 | 单测 |
| 数据损坏自动恢复 | KVStore DATA_CORRUPTED 时自动重建并重新加载 | 单测 |

### 用户故事与 AC

| Story ID | 用户故事 | 优先级 |
|----------|----------|--------|
| US-1 | 作为平台/工具开发者，我想要 SA 启动时自动从配置目录加载工具注册文件到 KVStore，以便配置变更后重启即生效 | P1 |
| US-2 | 作为系统安全方，我想要工具注册文件经严格字段校验（名称前缀/路径绝对/权限去重），以便防止非法工具注册 | P1 |
| US-3 | 作为系统可靠性方，我想要单个配置文件解析失败不中断整体加载、KVStore 损坏自动恢复，以便保障注册链路健壮性 | P1 |

> AC 共 18 条（AC-1.1–AC-3.6），详见 `spec.md`。此处不重复摘录，避免与 spec 不一致。

### 范围边界

**包含：** OnStart 配置加载链路、JSON 解析与字段校验、KVStore 写入与增删同步、数据损坏恢复。
**不包含：** 查询/执行/函数注册接口、配置文件内容设计、跨设备同步。

### 影响范围

| 子系统 | 仓库 | 模块/路径 | 当前职责 | 影响类型 | Owner |
|--------|------|-----------|----------|----------|-------|
| ability | foundation/ability/ability_runtime | cli_tool_framework/services/climgr | 服务端配置加载 | 固化契约 | 待确认 |
| ability | foundation/ability/ability_runtime | cli_tool_framework/interfaces/cli_tool | ToolInfo/SubCommandInfo 解析 | 固化契约 | 待确认 |
| ability | foundation/ability/ability_runtime | cli_tool_framework/etc/profile | init 数据目录 | 不变 | — |

### API 变更项清单

无 API 变更。本特性为内部实现行为固化。

### 不涉及项确认

| 维度 | 涉及？ | 依据 | 若涉及，进入哪个下游文档 |
|------|--------|------|--------------------------|
| 性能 | 否 | 无公开 P 指标；启动延迟百毫秒级可接受 | — |
| 安全与权限 | 是 | 工具名称前缀校验/路径绝对/权限去重 | design.md / spec.md |
| 兼容性 | 否 | KVStore 数据格式不变 | spec.md |
| API/SDK | 否 | 无 API 变更 | — |
| IPC/跨进程 | 否 | 加载在 SA 进程内完成 | — |
| 构建与部件 | 否 | 复用既有 component/parts | — |
| 国际化/无障碍 | 否 | 无界面 | — |
| 数据迁移 | 否 | KVStore 格式不变 | — |

### 变更控制

| 变更类型 | 触发条件 | 处理规则 |
|----------|----------|----------|
| 范围新增 | 新增加载路径/校验规则 | 重新评估复杂度和设计影响 |
| AC 变更 | 修改加载/同步/恢复行为 | 重新审批基线和 Spec |
| 配置目录变更 | DEFAULT_CONFIG_DIR 修改 | 影响部署，需协调 |
| KVStore 格式变更 | ToolInfo JSON 序列化变更 | 跨版本兼容性评估 |

### 进入设计/Spec 条件

- [x] 所有 P0/P1 用户故事有 AC
- [x] 每条 AC 可测试、可度量
- [x] 范围内/外已确认
- [ ] `proposal.target_release` 已确认或明确 TBD（待确认）
- [x] `manifest.profile` 已确认或明确 none
- [x] 涉及仓、模块、SIG 已识别
- [x] 不涉及项已标记 N/A
- [x] 变更控制规则已确认
- [x] 标准及以上复杂度的澄清问题已逐项关闭
- [x] 上下文与知识源检索日志已填写
- [x] 目标仓 Agent 指南已检查并记录关键约束

**基线结论:** 条件通过
