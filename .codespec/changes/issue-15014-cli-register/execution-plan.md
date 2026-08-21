# 执行计划

> 将 Approved Spec 拆成可独立执行、可验证、可审查的 Task。本计划针对 CLI 工具注册（OnStart 配置加载与 KVStore 同步）既有实现进行契约固化与测试覆盖验证：实现已存在，Task 聚焦「验证既有链路符合 spec + 补齐测试缺口」。

## Plan 元数据

| 字段 | 内容 |
|------|-----|
| Plan ID | PLAN-CLI-015 |
| 关联 Feature/Bug | FEAT-CLI-015（REQ-015） |
| 关联文档 | proposal.md / design.md / spec.md |
| 复杂度 | 标准 |
| 状态 | Draft |
| Owner | 待确认 |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved（条件通过，待 Owner 转正） |
| Design | `design.md` | Approved（条件通过，待 Owner 转正） |
| Spec | `spec.md` | Approved（Draft→待转 Approved） |

## 受影响文件全量清单

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| ability_runtime | SA 生命周期 | `cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp` | 验证 | OnStart 调用 EnsureToolsLoaded |
| ability_runtime | SA 生命周期 | `cli_tool_framework/services/climgr/include/cli_tool_manager_service.h` | 只读参考 | 声明 |
| ability_runtime | 数据管理 | `cli_tool_framework/services/climgr/src/cli_tool_data_manager.cpp` | 验证 | EnsureToolsLoaded/LoadToolsFromDir/ParseToolFromJsonFile/StoreTool/SyncToolNames/CheckKvStore/RestoreKvStore |
| ability_runtime | 数据管理 | `cli_tool_framework/services/climgr/include/cli_tool_data_manager.h` | 只读参考 | 声明 |
| ability_runtime | 数据结构 | `cli_tool_framework/interfaces/cli_tool/src/tool_info.cpp` | 验证 | ParseFromJson/ValidateName/ValidateExecutablePath/ValidateRequirePermissions/ValidateEventTypes |
| ability_runtime | 数据结构 | `cli_tool_framework/interfaces/cli_tool/include/tool_info.h` | 只读参考 | 声明 |
| ability_runtime | 数据结构 | `cli_tool_framework/interfaces/cli_tool/src/sub_command_info.cpp` | 验证 | ParseFromJson/Validate |
| ability_runtime | 数据结构 | `cli_tool_framework/interfaces/cli_tool/include/sub_command_info.h` | 只读参考 | 声明 |
| ability_runtime | 配置 | `cli_tool_framework/etc/profile/aimgr.cfg` | 只读参考 | init 数据目录创建 |
| ability_runtime | 配置 | `cli_tool_framework/etc/profile/aimgr_trust.json` | 只读参考 | SA 186 注册 |
| ability_runtime | 测试 | `cli_tool_framework/test/unittest/cli_tool_data_manager_test/` | 验证/补测试 | 加载/同步/恢复用例 |
| ability_runtime | 测试 | `cli_tool_framework/test/unittest/tool_info_test/` | 验证/补测试 | 解析/校验用例 |
| ability_runtime | 测试 | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | 验证/补测试 | OnStart 调用链用例 |

**检查项：**
- [x] design.md 调用链每一层都有对应文件列出
- [x] 每个文件修改类型和职责说明明确
- [x] 无映射行的层缺失

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式 | 覆盖？ |
|----|------|------|----------|--------|
| AC-1.1 | spec.md | TASK-R-001 | 单测 | 是 |
| AC-1.2 | spec.md | TASK-R-001 | 单测 | 是 |
| AC-1.3 | spec.md | TASK-R-001 | 单测 | 是 |
| AC-1.4 | spec.md | TASK-R-001 | 单测 | 是 |
| AC-1.5 | spec.md | TASK-R-002 | 单测 | 是 |
| AC-1.6 | spec.md | TASK-R-002 | 单测 | 是 |
| AC-2.1 | spec.md | TASK-R-003 | 单测 | 是 |
| AC-2.2 | spec.md | TASK-R-003 | 单测 | 是 |
| AC-2.3 | spec.md | TASK-R-003 | 单测 | 是 |
| AC-2.4 | spec.md | TASK-R-003 | 单测 | 是 |
| AC-2.5 | spec.md | TASK-R-003 | 单测 | 是 |
| AC-2.6 | spec.md | TASK-R-003 | 单测 | 是 |
| AC-2.7 | spec.md | TASK-R-003 | 单测 | 是 |
| AC-3.1 | spec.md | TASK-R-004 | 单测 | 是 |
| AC-3.2 | spec.md | TASK-R-004 | 单测 | 是 |
| AC-3.3 | spec.md | TASK-R-005 | 单测 | 是 |
| AC-3.4 | spec.md | TASK-R-005 | 单测 | 是 |
| AC-3.5 | spec.md | TASK-R-004 | 单测 | 是 |
| AC-3.6 | spec.md | TASK-R-004 | 单测 | 是 |

## 首批实现边界

**首批必须实现：** TASK-R-001（正常加载链路）+ TASK-R-003（字段校验），构成主链与安全边界。
**可后置：** TASK-R-005（KVStore 恢复）。
**不建议延后：** TASK-R-004（增删同步）延后会导致配置变更不生效。

## 阶段计划（如适用）

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 | 正常加载+字段校验 | TASK-R-001,003 | 正常加载与校验用例通过 | `run -t UT -ts tool_info_test` |
| Phase-2 | 容错+增删同步 | TASK-R-002,004 | 容错与同步用例通过 | `run -t UT -ts cli_tool_data_manager_test` |
| Phase-3 | KVStore 恢复 | TASK-R-005 | 恢复用例通过 | `run -t UT -ts cli_tool_data_manager_test` |

## Task 粒度原则

- 每个 Task 对应一个可独立验收的最小能力闭环
- 本计划按正常加载/容错/校验/同步/恢复拆为 5 张
- 每个 Task 自包含，不依赖外部文件路径引用

## 禁止项

- [x] 没有 TBD / TODO / 占位符（「待确认」为需 Owner 决议项）
- [x] 没有"根据需要实现""酌情处理"等模糊指令
- [x] 没有跨 Task 隐式依赖
- [x] 没有要求 Agent 自行寻找未列出的上下文文件
- [x] 没有无验证方式的 AC
- [x] 没有"与 Task-N 类似""参考 Task-N 实现"等引用

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 |
|---------|------|----------|---------|----------|----------|----------|
| TASK-R-001 | 正常加载链路+幂等性验证 | cli_tool_data_manager/cli_tool_manager_service | AC-1.1,1.2,1.3,1.4 | 无 | OnStart 加载后 KVStore 含全部工具；空目录→空 KVStore；重复调用幂等 | `run -t UT -ts cli_tool_data_manager_test` |
| TASK-R-002 | 配置目录不存在+加载失败容错验证 | cli_tool_manager_service/cli_tool_data_manager | AC-1.5,1.6 | TASK-R-001 | 目录不存在返回 ERR_FILE_NOT_FOUND；加载失败 SA 继续启动 | `run -t UT -ts cli_tool_mgr_service_test` |
| TASK-R-003 | JSON 解析与字段校验验证 | tool_info/sub_command_info | AC-2.1–2.7 | TASK-R-001 | 合法 JSON 解析成功；各非法字段跳过；BOM 去除；非法 JSON 跳过 | `run -t UT -ts tool_info_test` |
| TASK-R-004 | 单文件失败容错+增删同步验证 | cli_tool_data_manager | AC-3.1,3.2,3.5,3.6 | TASK-R-001 | M 合法+K 非法→M 写入+K 跳过；旧工具删除；Put 失败不中断 | `run -t UT -ts cli_tool_data_manager_test` |
| TASK-R-005 | KVStore 不可用+数据损坏恢复验证 | cli_tool_data_manager | AC-3.3,3.4 | TASK-R-001 | 重试 5 次后 ERR_KVSTORE_NOT_READY；DATA_CORRUPTED 自动重建 | `run -t UT -ts cli_tool_data_manager_test` |

## Task 详情

### TASK-R-001: 正常加载链路+幂等性验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证 OnStart→EnsureToolsLoaded→LoadToolsFromDir 正常链路与幂等性符合 spec AC-1.1/1.2/1.3/1.4 |
| AC 映射 | AC-1.1, AC-1.2, AC-1.3, AC-1.4 |
| 前置依赖 | 无 |
| 非目标 | 不验证异常路径（属 TASK-R-002） |
| 完成判据 | 加载后 KVStore 含全部工具+AllCliToolNames 索引一致；空目录→空 KVStore；重复调用直接返回 ERR_OK |
| 停止条件 | 发现既有实现与 spec 行为不一致且无法在 Task 范围内修复 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_data_manager_test/` | 加载链路用例 |
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | OnStart 调用链用例 |

**Spec Context**

AC-1.1：OnStart 调用 EnsureToolsLoaded 加载配置到 KVStore。
AC-1.2：N 个合法文件→KVStore 含 N 个工具+索引。
AC-1.3：空目录→KVStore 空+索引空。
AC-1.4：重复调用→直接返回 ERR_OK。
R-1/R-2/R-3。

**Design Context**

OnStart→EnsureToolsLoaded：toolsLoaded_=false→CheckKvStore→LoadToolsFromDir→toolsLoaded_=true。LoadToolsFromDir：opendir→遍历 .json→ParseToolFromJsonFile→StoreTool→SyncToolNames。设计 ADR-1（OnStart 加载）、ADR-2（幂等）。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | Must：SA 生命周期→数据管理→数据结构 三层 |
| OH-ARCH-ERROR-LOG | Must：错误码 357000xx |

**Steps**

- [ ] 写失败测试：合法加载断言 KVStore 内容+索引；空目录断言空；重复调用断言幂等
- [ ] 运行测试，确认当前结果
- [ ] 若不一致，做最小实现修正
- [ ] 运行测试，确认通过
- [ ] 填写完成证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_data_manager_test` | PASS |
| 测试 | `run -t UT -ts cli_tool_mgr_service_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证正常加载链路与幂等性（AC-1.1/1.2/1.3/1.4） |
| 允许修改 | `cli_tool_data_manager.cpp`、`cli_tool_manager_service.cpp`、对应测试 |
| 允许新建 | 测试用例文件 |
| 只读参考 | `spec.md` AC-1.1–1.4、`design.md` ADR-1/ADR-2 |
| Spec 摘要 | OnStart 加载→KVStore 含全部工具+索引；空→空；幂等 |
| Design 摘要 | EnsureToolsLoaded→CheckKvStore→LoadToolsFromDir→StoreTool→SyncToolNames |
| 执行步骤 | 写失败测试→运行→最小修正→运行通过→记录证据 |
| 验证命令 | `run -t UT -ts cli_tool_data_manager_test` / 期望: PASS |
| 完成规则 | 不得修改允许范围外文件；如需扩大范围停止并修订 Plan；无 fresh evidence 不得声明完成 |

### TASK-R-002: 配置目录不存在+加载失败容错验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证配置目录不存在返回 ERR_FILE_NOT_FOUND、加载失败 SA 继续启动（AC-1.5/1.6） |
| AC 映射 | AC-1.5, AC-1.6 |
| 前置依赖 | TASK-R-001 |
| 非目标 | 不验证 KVStore 不可用（属 TASK-R-005） |
| 完成判据 | 目录不存在→ERR_FILE_NOT_FOUND；加载失败→日志告警+继续 Publish |
| 停止条件 | 容错行为与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_data_manager_test/` | 目录不存在用例 |
| Test | `cli_tool_framework/test/unittest/cli_tool_mgr_service_test/` | 加载失败继续启动用例 |

**Spec Context**

AC-1.5：目录不存在→ERR_FILE_NOT_FOUND（35700015），SA 继续启动。
AC-1.6：加载失败→日志告警+继续 Publish。
R-4/R-5。

**Design Context**

LoadToolsFromDir：opendir 返回 nullptr→ERR_FILE_NOT_FOUND。OnStart：EnsureToolsLoaded 返回非零→TAG_LOGW 告警+继续 Publish。设计 ADR-1。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：错误码 35700015 |

**Steps**

- [ ] 写失败测试：目录不存在断言 ERR_FILE_NOT_FOUND；加载失败断言 SA 继续 Publish
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_data_manager_test` | PASS |
| 测试 | `run -t UT -ts cli_tool_mgr_service_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证目录不存在+加载失败容错（AC-1.5/1.6） |
| 允许修改 | `cli_tool_data_manager.cpp`、`cli_tool_manager_service.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-1.5/1.6 + R-4/R-5 |
| Spec 摘要 | 目录不存在→35700015；加载失败→继续启动 |
| Design 摘要 | opendir 失败→ERR_FILE_NOT_FOUND；OnStart 容错 |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts cli_tool_data_manager_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-R-003: JSON 解析与字段校验验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证 ToolInfo::ParseFromJson 字段校验全路径与 BOM 处理（AC-2.1–2.7） |
| AC 映射 | AC-2.1, AC-2.2, AC-2.3, AC-2.4, AC-2.5, AC-2.6, AC-2.7 |
| 前置依赖 | TASK-R-001 |
| 非目标 | 不验证加载链路（TASK-R-001） |
| 完成判据 | 合法 JSON 解析成功；name 前缀非法/路径非绝对/权限重复/子命令缺失跳过；BOM 去除正常；非法 JSON 跳过 |
| 停止条件 | 校验逻辑与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/tool_info_test/` | 字段校验用例 |
| Test | `cli_tool_framework/test/unittest/sub_command_info_test/` | 子命令校验用例 |

**Spec Context**

AC-2.1：合法字段→解析成功。AC-2.2：name 前缀非法/后缀超长→跳过。AC-2.3：executablePath 非绝对→跳过。AC-2.4：requirePermissions 重复→跳过。AC-2.5：hasSubCommand=true 但 subcommands 缺失→跳过。AC-2.6：BOM→自动去除。AC-2.7：非法 JSON→ERR_JSON_PARSE_FAILED。
R-6/R-7/R-8/R-9。

**Design Context**

ToolInfo::ParseFromJson：name→ValidateName（ohos-/hms-+≤32后缀）；version→非空；description→非空；executablePath→ValidateExecutablePath（绝对路径）；requirePermissions→ValidateRequirePermissions（去重）；inputSchema/outputSchema→is_object；hasSubCommand=true→subcommands 须非空 object。ParseToolFromJsonFile：ifstream→BOM 检测（0xEF 0xBB 0xBF）→json::parse(nullptr,false)→root.is_object()。设计 ADR-3。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：错误码 35700014 |

**Steps**

- [ ] 写失败测试：合法解析+各非法字段+BOM+非法 JSON
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts tool_info_test` | PASS |
| 测试 | `run -t UT -ts sub_command_info_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证 JSON 解析与字段校验（AC-2.1–2.7） |
| 允许修改 | `tool_info.cpp`、`sub_command_info.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-2.1–2.7 + R-6/R-7/R-8/R-9、design ADR-3 |
| Spec 摘要 | 合法→解析成功；各非法→跳过；BOM→去除；非法 JSON→35700014 |
| Design 摘要 | ParseFromJson 字段校验+ParseToolFromJsonFile BOM+json::parse 容错 |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts tool_info_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-R-004: 单文件失败容错+增删同步验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证 M 合法+K 非法文件混合加载、旧工具删除、Put 失败不中断（AC-3.1/3.2/3.5/3.6） |
| AC 映射 | AC-3.1, AC-3.2, AC-3.5, AC-3.6 |
| 前置依赖 | TASK-R-001 |
| 非目标 | 不验证 KVStore 恢复（TASK-R-005） |
| 完成判据 | M 写入+K 跳过→ERR_OK；旧工具从 KVStore 删除；StoreTool Put 失败→不中断；SyncToolNames 索引失败→不影响已加载 |
| 停止条件 | 容错/同步与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_data_manager_test/` | 混合加载+同步用例 |

**Spec Context**

AC-3.1：M 合法+K 非法→M 写入+K 跳过→ERR_OK。AC-3.2：旧工具不在配置→从 KVStore 删除。AC-3.5：StoreTool Put 失败→不中断。AC-3.6：SyncToolNames 索引失败→不影响已加载。
R-10/R-11/R-14。

**Design Context**

LoadToolsFromDir：逐文件 ParseToolFromJsonFile→成功则 StoreTool；失败跳过。SyncToolNames：读 AllCliToolNames→set diff→delete 不在当前集合的旧工具→put 新名列表。StoreTool：Put 返回非 SUCCESS→返回 -1 记录日志。设计 ADR-3（逐文件容错）、ADR-4（diff 同步）。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：错误码/日志记录 |

**Steps**

- [ ] 写失败测试：混合加载+旧工具删除+Put 失败+索引失败
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_data_manager_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证单文件失败容错+增删同步（AC-3.1/3.2/3.5/3.6） |
| 允许修改 | `cli_tool_data_manager.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-3.1/3.2/3.5/3.6 + R-10/R-11/R-14、design ADR-3/ADR-4 |
| Spec 摘要 | 混合→M 写入+K 跳过；旧→删除；Put 失败→不中断 |
| Design 摘要 | 逐文件容错+SyncToolNames diff+StoreTool Put |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts cli_tool_data_manager_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

### TASK-R-005: KVStore 不可用+数据损坏恢复验证

| 字段 | 内容 |
|------|-----|
| 任务目标 | 验证 KVStore 初始化重试 5 次后 ERR_KVSTORE_NOT_READY、DATA_CORRUPTED 自动重建（AC-3.3/3.4） |
| AC 映射 | AC-3.3, AC-3.4 |
| 前置依赖 | TASK-R-001 |
| 非目标 | 不验证正常加载路径 |
| 完成判据 | 重试 5 次失败→ERR_KVSTORE_NOT_READY；DATA_CORRUPTED→删除+重建 |
| 停止条件 | 恢复逻辑与 spec 不一致 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Test | `cli_tool_framework/test/unittest/cli_tool_data_manager_test/` | KVStore 恢复用例 |

**Spec Context**

AC-3.3：KVStore 初始化失败→重试 5 次（100ms 间隔）→ERR_KVSTORE_NOT_READY。
AC-3.4：DATA_CORRUPTED→RestoreKvStore 删除+重建。
R-12/R-13。

**Design Context**

CheckKvStore：GetSingleKvStore 非 SUCCESS→usleep(100000)→重试，MAX_TIMES=5。RestoreKvStore：status==DATA_CORRUPTED→DeleteKvStore→GetSingleKvStore 重建。设计 ADR-5（重试 5 次）、ADR-6（损坏恢复）。

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-ERROR-LOG | Must：错误码 35700016 |

**Steps**

- [ ] 写失败测试：KVStore 初始化失败断言重试+ERR_KVSTORE_NOT_READY；DATA_CORRUPTED 断言重建
- [ ] 运行测试
- [ ] 最小修正
- [ ] 运行通过
- [ ] 记录证据

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 测试 | `run -t UT -ts cli_tool_data_manager_test` | PASS |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | 验证 KVStore 不可用+数据损坏恢复（AC-3.3/3.4） |
| 允许修改 | `cli_tool_data_manager.cpp`、对应测试 |
| 允许新建 | 测试用例 |
| 只读参考 | spec AC-3.3/3.4 + R-12/R-13、design ADR-5/ADR-6 |
| Spec 摘要 | 重试 5 次→ERR_KVSTORE_NOT_READY；DATA_CORRUPTED→重建 |
| Design 摘要 | CheckKvStore 重试+RestoreKvStore 删除重建 |
| 执行步骤 | 写失败测试→运行→修正→通过→证据 |
| 验证命令 | `run -t UT -ts cli_tool_data_manager_test` / 期望: PASS |
| 完成规则 | 不改范围外文件；无 evidence 不声明完成 |

## Plan 自审清单

- [x] 每个 P0/P1 AC 至少映射到一个 Task（AC-1.1–AC-3.6 全覆盖）
- [x] 每个 Task 文件范围明确
- [x] 每个 Task 明确前置依赖、非目标、完成判据和停止条件
- [x] 每个 Task 有验证命令
- [x] Task 粒度形成能力闭环
- [x] 没有 TBD/TODO/占位符（「待确认」为 Owner 决议项）
- [x] 没有要求 Agent 自行寻找未列出的上下文
- [x] 交接信息自包含（Handoff Summary 完整）
- [x] 每个 Task 验证在完成时立即执行并记录证据
- [x] 超 3000 行阈值的 Task 已说明不拆分理由（本计划 Task 均聚焦验证+补测试，上下文未超阈值）
