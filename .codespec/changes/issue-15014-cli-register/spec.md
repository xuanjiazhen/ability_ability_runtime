# 特性规格

> 本规格仅覆盖 CLI 工具注册（OnStart 配置加载与 KVStore 同步）的内部实现行为。范围限定：`OnStart()` → `EnsureToolsLoaded()` → `LoadToolsFromDir()` → `ParseToolFromJsonFile()` → `StoreTool()` → `SyncToolNames()` 链路，以及 KVStore 初始化与数据损坏恢复。查询/执行/函数注册等接口不在本规格范围内。
>
> **输入文档说明：** 本规格依据 `cli_tool_framework` 中配置加载链路的既有实现反推固化。待 `proposal.md` 基线化后，以下字段应回填 proposal 值。

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | CLI 工具注册（cli-register） |
| 特性编号 | FEAT-CLI-015 |
| 所属 Epic | REQ-015 |
| 优先级 | P1 |
| 目标版本 | 参见 proposal.md（待基线） |
| SIG 归属 | Sig-Ability |
| 状态 | Draft |
| 复杂度 | 标准 |

## 本次变更范围（Delta）

本特性为既有实现固化（lineage: consolidation）。配置加载链路已存在于 `cli_tool_framework`，本规格固化其行为契约。

| 类型 | 内容 | 说明 |
|------|------|------|
| FIXED | OnStart → EnsureToolsLoaded 启动加载链路 | SA 启动时从配置目录加载工具到 KVStore |
| FIXED | LoadToolsFromDir 遍历 .json 文件 | 仅读取 .json 后缀文件 |
| FIXED | ParseToolFromJsonFile JSON 解析+BOM 处理 | UTF-8 BOM 自动去除；json::parse 容错 |
| FIXED | ToolInfo::ParseFromJson 字段校验 | name/version/description/executablePath/requirePermissions/inputSchema/outputSchema/subcommands 校验规则 |
| FIXED | StoreTool KVStore 写入 | key=toolName, value=ToolInfo JSON |
| FIXED | SyncToolNames 增删同步 | 旧名列表 diff 删除已移除工具；更新 AllCliToolNames 索引 |
| FIXED | CheckKvStore 重试初始化 | 5 次重试，100ms 间隔 |
| FIXED | RestoreKvStore 数据损坏恢复 | DATA_CORRUPTED 时删除+重建 KVStore |

## 输入文档

| 文档 | 路径 | 状态 |
|------|------|------|
| Requirement | `REQ-015`（未在工作区检索到） | Pending |
| Proposal | `proposal.md`（本 change 目录） | Conditional Pass |

> 需求基线、不涉及项、受影响子系统与仓库详见 proposal.md。

## 用户故事

### US-1: SA 启动时自动加载配置到 KVStore

**作为** 平台/工具开发者,
**我想要** SA 186 在 `OnStart()` 时自动从配置目录加载工具注册文件到 KVStore,
**以便** 配置文件变更后 SA 重启即生效。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-1.1 | WHEN SA 186 `OnStart()` 执行 THEN 调用 `EnsureToolsLoaded()` 从配置目录 `/system/bin/cli_tool/configs` 加载工具注册文件到 KVStore，加载完成后 SA 继续启动并 Publish | 正常 |
| AC-1.2 | WHEN 配置目录中存在 N 个合法 `.json` 文件 THEN 加载完成后 KVStore 中包含这 N 个工具的完整 ToolInfo，`AllCliToolNames` 索引包含这 N 个工具名 | 正常 |
| AC-1.3 | WHEN 配置目录中不存在 `.json` 文件或目录为空 THEN 加载完成且 KVStore 中无工具（`AllCliToolNames` 为空数组），SA 正常启动 | 边界 |
| AC-1.4 | WHEN `EnsureToolsLoaded()` 已成功执行过（`toolsLoaded_=true`）THEN 再次调用直接返回 ERR_OK，不重复加载 | 边界 |
| AC-1.5 | WHEN 配置目录不存在（`opendir` 失败）THEN `LoadToolsFromDir` 返回 ERR_FILE_NOT_FOUND（35700015），`EnsureToolsLoaded` 返回该码，SA 继续启动并 Publish | 异常 |
| AC-1.6 | WHEN `OnStart()` 中 `EnsureToolsLoaded()` 返回非零 THEN SA 记录日志告警但继续启动（不因加载失败阻止 Publish） | 异常 |

### US-2: 工具注册文件经严格字段校验

**作为** 系统安全方,
**我想要** 工具注册 JSON 文件经严格字段校验后才能写入 KVStore,
**以便** 防止非法工具注册。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-2.1 | WHEN JSON 文件包含合法的 `name`（以 `ohos-` 或 `hms-` 开头，后缀 1-32 字符）、非空 `version`、非空 `description`、绝对路径 `executablePath`（以 `/` 开头）、无重复项的 `requirePermissions` 数组、JSON object 类型的 `inputSchema` 和 `outputSchema` THEN 文件解析成功，ToolInfo 写入 KVStore | 正常 |
| AC-2.2 | WHEN `name` 不以 `ohos-` 或 `hms-` 开头，或后缀为空/超过 32 字符 THEN `ParseFromJson` 返回 false，该文件跳过不写入 KVStore | 异常 |
| AC-2.3 | WHEN `executablePath` 不以 `/` 开头或为空 THEN `ParseFromJson` 返回 false，该文件跳过 | 异常 |
| AC-2.4 | WHEN `requirePermissions` 数组含重复项 THEN `ParseFromJson` 返回 false，该文件跳过 | 异常 |
| AC-2.5 | WHEN `hasSubCommand=true` 但 `subcommands` 字段缺失/非 object/为空 THEN `ParseFromJson` 返回 false，该文件跳过 | 异常 |
| AC-2.6 | WHEN JSON 文件含 UTF-8 BOM 头（0xEF 0xBB 0xBF）THEN BOM 被自动去除后正常解析 | 边界 |
| AC-2.7 | WHEN JSON 文件内容非法（`json::parse` 返回 discarded）或根节点非 object THEN `ParseToolFromJsonFile` 返回 ERR_JSON_PARSE_FAILED（35700014），该文件跳过 | 异常 |

### US-3: 单文件失败不中断+数据损坏恢复

**作为** 系统可靠性方,
**我想要** 单个配置文件解析失败不中断整体加载，且 KVStore 数据损坏时自动恢复,
**以便** 保障注册链路健壮性。

**验收标准（AC, Acceptance Criteria）：**

| AC编号 | 验收标准 | 类型 |
|--------|----------|------|
| AC-3.1 | WHEN 配置目录中有 M 个合法文件和 K 个非法文件 THEN 合法的 M 个工具写入 KVStore，非法的 K 个文件被跳过并记录日志，`LoadToolsFromDir` 返回 ERR_OK | 正常 |
| AC-3.2 | WHEN 配置目录中之前注册的工具 A 不再存在于当前配置目录 THEN `SyncToolNames` 从 KVStore 删除工具 A 的记录，`AllCliToolNames` 更新为当前工具列表 | 正常 |
| AC-3.3 | WHEN KVStore 初始化时 `GetSingleKvStore` 返回非 SUCCESS THEN `CheckKvStore` 重试最多 5 次（每次间隔 100ms），5 次均失败则返回 false，`EnsureToolsLoaded` 返回 ERR_KVSTORE_NOT_READY（35700016） | 异常 |
| AC-3.4 | WHEN KVStore 操作返回 DATA_CORRUPTED THEN `RestoreKvStore` 删除旧 KVStore 并重新创建，后续操作使用新 KVStore | 异常 |
| AC-3.5 | WHEN `StoreTool` 写入 KVStore 失败（Put 返回非 SUCCESS）THEN 该工具不写入，记录日志，后续文件继续加载 | 异常 |
| AC-3.6 | WHEN `SyncToolNames` 写入 `AllCliToolNames` 索引失败 THEN 返回 -1 并记录日志，但不影响已加载的工具 | 异常 |

## 验收追溯

| AC | 关联规则 | 关联 Task | 验证方式 | 证据 |
|----|----------|-----------|----------|------|
| AC-1.1 | R-1 | TASK-R-001 | 单测 | `cli_tool_mgr_service_test/` |
| AC-1.2 | R-2 | TASK-R-001 | 单测 | `cli_tool_data_manager_test/` |
| AC-1.3 | R-2 | TASK-R-001 | 单测 | `cli_tool_data_manager_test/` |
| AC-1.4 | R-3 | TASK-R-001 | 单测 | `cli_tool_data_manager_test/` |
| AC-1.5 | R-4 | TASK-R-002 | 单测 | `cli_tool_data_manager_test/` |
| AC-1.6 | R-5 | TASK-R-002 | 单测 | `cli_tool_mgr_service_test/` |
| AC-2.1 | R-6 | TASK-R-003 | 单测 | `tool_info_test/` |
| AC-2.2 | R-7 | TASK-R-003 | 单测 | `tool_info_test/` |
| AC-2.3 | R-7 | TASK-R-003 | 单测 | `tool_info_test/` |
| AC-2.4 | R-7 | TASK-R-003 | 单测 | `tool_info_test/` |
| AC-2.5 | R-7 | TASK-R-003 | 单测 | `tool_info_test/` |
| AC-2.6 | R-8 | TASK-R-003 | 单测 | `tool_info_test/` |
| AC-2.7 | R-9 | TASK-R-003 | 单测 | `tool_info_test/` |
| AC-3.1 | R-10 | TASK-R-004 | 单测 | `cli_tool_data_manager_test/` |
| AC-3.2 | R-11 | TASK-R-004 | 单测 | `cli_tool_data_manager_test/` |
| AC-3.3 | R-12 | TASK-R-005 | 单测 | `cli_tool_data_manager_test/` |
| AC-3.4 | R-13 | TASK-R-005 | 单测 | `cli_tool_data_manager_test/` |
| AC-3.5 | R-14 | TASK-R-004 | 单测 | `cli_tool_data_manager_test/` |
| AC-3.6 | R-14 | TASK-R-004 | 单测 | `cli_tool_data_manager_test/` |

## 规则定义

| 规则ID | 类型 | 触发条件 | 预期行为 | 边界/约束 | 关联AC |
|--------|------|----------|----------|-----------|--------|
| R-1 | 行为 | SA OnStart 执行 | 调用 EnsureToolsLoaded 从配置目录加载工具到 KVStore，完成后继续 Publish | 加载在 Publish 前执行 | AC-1.1 |
| R-2 | 行为 | 配置目录有 N 个合法 .json 文件 | 逐文件解析+存储到 KVStore，更新 AllCliToolNames 索引 | 空目录→KVStore 空+索引空 | AC-1.2, AC-1.3 |
| R-3 | 行为 | EnsureToolsLoaded 已执行过 | toolsLoaded_=true 时直接返回 ERR_OK，不重复加载 | 原子标志保证幂等 | AC-1.4 |
| R-4 | 异常 | 配置目录不存在 | opendir 返回 nullptr→返回 ERR_FILE_NOT_FOUND | SA 继续启动 | AC-1.5 |
| R-5 | 异常 | EnsureToolsLoaded 返回非零 | OnStart 记录日志告警但继续 Publish | 不因加载失败阻止启动 | AC-1.6 |
| R-6 | 行为 | JSON 文件所有字段合法 | ParseFromJson 返回 true→StoreTool 写入 KVStore | 必填字段：name/version/description/executablePath/requirePermissions/inputSchema/outputSchema | AC-2.1 |
| R-7 | 异常 | name 前缀非法/后缀超长/路径非绝对/权限重复/子命令缺失 | ParseFromJson 返回 false→该文件跳过 | 名称须 ohos-/hms-+≤32 后缀；路径须绝对；权限须去重；hasSubCommand=true 时 subcommands 须非空 object | AC-2.2–2.5 |
| R-8 | 边界 | JSON 文件含 UTF-8 BOM | BOM（0xEF 0xBB 0xBF）被自动去除后正常解析 | 仅检测 3 字节 BOM | AC-2.6 |
| R-9 | 异常 | JSON 内容非法或根非 object | ParseToolFromJsonFile 返回 ERR_JSON_PARSE_FAILED→该文件跳过 | json::parse 容错（nullptr, false） | AC-2.7 |
| R-10 | 行为 | 配置目录有 M 合法+K 非法文件 | M 个写入 KVStore，K 个跳过并记录日志，LoadToolsFromDir 返回 ERR_OK | 逐文件独立处理 | AC-3.1 |
| R-11 | 行为 | 旧工具名不在当前配置目录中 | SyncToolNames 从 KVStore 删除该工具记录，更新 AllCliToolNames | diff 旧名集合 vs 当前名集合 | AC-3.2 |
| R-12 | 异常 | KVStore 初始化失败 | CheckKvStore 重试 5 次（100ms 间隔），均失败返回 false→ERR_KVSTORE_NOT_READY | MAX_TIMES=5; CHECK_INTERVAL=100000us | AC-3.3 |
| R-13 | 异常 | KVStore 操作返回 DATA_CORRUPTED | RestoreKvStore 删除旧 KVStore+重建 | DeleteKvStore+GetSingleKvStore | AC-3.4 |
| R-14 | 异常 | StoreTool Put 失败/SyncToolNames 索引写入失败 | 该操作失败记录日志，不影响其他工具加载 | StoreTool 返回 -1；SyncToolNames 返回 -1 | AC-3.5, AC-3.6 |

## 验证映射

| 编号 | 对应规格项 | 验证方式 | 验证重点 |
|------|------------|----------|----------|
| VM-1 | R-1, R-2, R-3 / AC-1.1–1.4 | 单测 | 正常加载链路与幂等性 |
| VM-2 | R-4, R-5 / AC-1.5–1.6 | 单测 | 配置目录不存在与加载失败容错 |
| VM-3 | R-6–R-9 / AC-2.1–2.7 | 单测 | JSON 解析与字段校验全路径 |
| VM-4 | R-10, R-11, R-14 / AC-3.1–3.2, 3.5–3.6 | 单测 | 单文件失败容错与增删同步 |
| VM-5 | R-12, R-13 / AC-3.3–3.4 | 单测 | KVStore 不可用与数据损坏恢复 |

## API 变更分析

无 API 变更。本特性为内部实现行为固化。

## 接口规格

### 配置文件格式约束

| 字段 | 类型 | 必填 | 约束条件 |
|------|------|------|---------|
| name | string | 是 | 以 `ohos-` 或 `hms-` 开头；后缀 1-32 字符 |
| version | string | 是 | 非空 |
| description | string | 是 | 非空 |
| executablePath | string | 是 | 绝对路径（以 `/` 开头） |
| requirePermissions | string[] | 是 | 数组；项须为 string；无重复项；空数组合法 |
| inputSchema | JSON object | 是 | JSON 对象类型 |
| outputSchema | JSON object | 是 | JSON 对象类型 |
| eventSchemas | JSON object | 否 | 如存在须为 JSON 对象 |
| eventTypes | string[] | 否 | 如存在须为 string 数组；无重复项 |
| hasSubCommand | boolean | 否 | 默认 false |
| subcommands | JSON object | 条件必填 | hasSubCommand=true 时必填且非空 |
| isLockScreenExecutionAllowed | boolean | 否 | 默认 false |

### 行为场景

| # | 触发条件 | 预期行为 | 关联 AC |
|---|----------|----------|---------|
| 1 | SA OnStart + 配置目录有合法 .json | 加载到 KVStore + 更新索引 | AC-1.1, 1.2 |
| 2 | 配置目录为空/无 .json | KVStore 空 + 索引空 | AC-1.3 |
| 3 | EnsureToolsLoaded 已执行 | 直接返回 ERR_OK | AC-1.4 |
| 4 | 配置目录不存在 | 返回 ERR_FILE_NOT_FOUND | AC-1.5 |
| 5 | 加载失败 | SA 记录日志继续启动 | AC-1.6 |
| 6 | 合法 JSON 文件 | ParseFromJson→StoreTool | AC-2.1 |
| 7 | name 前缀非法/后缀超长 | 跳过该文件 | AC-2.2 |
| 8 | executablePath 非绝对 | 跳过该文件 | AC-2.3 |
| 9 | requirePermissions 有重复 | 跳过该文件 | AC-2.4 |
| 10 | hasSubCommand=true 但 subcommands 缺失 | 跳过该文件 | AC-2.5 |
| 11 | JSON 含 BOM | BOM 去除后正常解析 | AC-2.6 |
| 12 | JSON 非法/根非 object | 返回 ERR_JSON_PARSE_FAILED 跳过 | AC-2.7 |
| 13 | M 合法+K 非法文件 | M 写入+K 跳过，返回 ERR_OK | AC-3.1 |
| 14 | 旧工具不在配置目录 | 从 KVStore 删除 | AC-3.2 |
| 15 | KVStore 初始化失败 | 重试 5 次后 ERR_KVSTORE_NOT_READY | AC-3.3 |
| 16 | KVStore DATA_CORRUPTED | 删除+重建 | AC-3.4 |
| 17 | StoreTool Put 失败 | 记录日志，继续加载 | AC-3.5 |
| 18 | SyncToolNames 索引写入失败 | 返回 -1 记录日志 | AC-3.6 |

## 兼容性声明

- **已有行为变更:** 否。本特性固化既有实现。
- **配置文件格式变更:** 否。ToolInfo JSON 字段集合不变。
- **数据存储格式变更:** 否。KVStore key=toolName, value=ToolInfo JSON dump。
- **最低支持版本:** 参见 proposal.md（待基线）。

## 架构约束

| 关键约束 | 约束说明 | 影响 AC |
|----------|----------|---------|
| 配置目录固定为 `/system/bin/cli_tool/configs` | 系统级只读分区 | AC-1.1 |
| 工具名须 ohos-/hms- 前缀 | 防止非系统工具注册 | AC-2.2 |
| 可执行路径须绝对路径 | 防止相对路径注入 | AC-2.3 |
| KVStore 在 aimgr 进程沙箱内 | 应用不可直接访问 | AC-1.2 |
| 懒加载原子标志 toolsLoaded_ | 保证幂等 | AC-1.4 |

> 架构规则适用性及设计方案见 design.md。

## 非功能性需求

| 类型 | 指标/阈值 | 验证方式 | 证据 |
|------|-----------|----------|------|
| 可靠性 | 单文件解析失败不中断整体加载 | 单测 | `cli_tool_data_manager_test/` |
| 可靠性 | KVStore DATA_CORRUPTED 自动恢复 | 单测 | `cli_tool_data_manager_test/` |
| 安全 | 工具名/路径/权限字段校验 | 单测 | `tool_info_test/` |
| 可测试性 | 加载链路可 mock KVStore 验证 | 单测 | `cli_tool_data_manager_test/` |

## 多设备适配声明

| 设备类型 | 行为差异 | 规格/约束 | 验证方式 | 证据 |
|----------|----------|-----------|----------|------|
| 手机 | 无差异 | — | 集成 | `cli_tool_framework/test/` |
| 平板 | 无差异 | — | 集成 | `cli_tool_framework/test/` |

> 注册行为与设备形态无关。

## 全局特性影响

| 特性 | 适用？ | 结论 | 关联场景 |
|------|--------|------|----------|
| 无障碍 | 否 | 无界面 | — |
| 多用户 | 否 | KVStore 在 el1/public | — |
| 版本升级 | 否 | KVStore 格式不变 | — |

## Spec 自审清单

- [x] 无"待定""TBD""TODO"等占位符
- [x] 所有 AC 使用 WHEN/THEN 格式，可独立测试
- [x] 范围边界明确
- [x] 无语义模糊表述
- [x] AC 与规则表交叉一致
- [x] 规则表每条通过 5 项质量检查

## context-references

```yaml
context-queries:
  - repo: "openharmony/ability_runtime"
    query: "cli_tool_framework CliToolDataManager EnsureToolsLoaded LoadToolsFromDir 实现"
  - repo: "openharmony/ability_runtime"
    query: "ToolInfo ParseFromJson ValidateName ValidateExecutablePath 字段校验规则"
```

**关键文档：**
- `cli_tool_framework/services/climgr/src/cli_tool_data_manager.cpp`（配置加载与 KVStore 管理）
- `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp:290-323`（OnStart 调用链）
- `cli_tool_framework/interfaces/cli_tool/src/tool_info.cpp`（ToolInfo 解析与校验）
- `cli_tool_framework/interfaces/cli_tool/src/sub_command_info.cpp`（SubCommandInfo 解析与校验）
- `cli_tool_framework/etc/profile/aimgr.cfg`（init 数据目录创建）
