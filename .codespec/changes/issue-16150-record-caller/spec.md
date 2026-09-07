# Spec

## 概述

| 属性 | 值 |
|------|-----|
| 特性名称 | 终止/记录接口调用方归因信息记录（Record Caller Info） |
| 特性编号 | 16150 |
| 优先级 | P1 |
| 复杂度 | 简单（引用 proposal.md 分级判断：单仓、单模块集中、内部记录扩展） |
| 目标版本 | 7.1 |

## 用户故事或场景

### US-1: 终止调用方归因

**作为** 可靠性维护人员 / 大数据分析平台, **我想要** 系统组件通过终止/记录接口终止应用时，终止原因数据中自动携带调用方 pid 和 processName, **以便** 定位是哪个系统组件发起的终止，支撑大数据按调用方维度进行二级拆解分类.

**验收标准：**

> 使用 Given/When/Then 格式。Then 描述通过三层接口边界（本特性无 public/system API，观测点为 inner API 返回值 + 终止原因数据读回 + 日志）可观测的结果。
> 注：终止原因数据指 AbilityManagerService 终止原因持久化数据（KV 存储，appId=`app_exit_reason_storage`、storeId=`app_exit_reason_infos`），可由终止原因数据读回接口观测。

- **AC-1.1:** Given 目标应用进程存在，调用方为具备 SA kill 权限（或 shell）的系统组件 When 通过 IPC 调用 `KillAppWithReason(pid, exitReason)` 且终止成功（返回 0） Then 新生成的终止原因记录中包含调用方 pid（=IPC 实际调用方进程 pid）与调用方 processName（=调用方进程名，可能为空），终止原因数据读回可观测；接口返回值与终止行为与现状一致
- **AC-1.2:** Given 目标 bundle 存在，调用方具备权限 When 通过 IPC 调用 `KillBundleWithReason(bundleName, userId, appIndex, exitReason)` 且终止成功 Then 本次为该 bundle 生成的每条终止原因记录均携带同一调用方 pid 与 processName（processName 可能为空）；接口返回值与终止行为与现状一致
- **AC-1.3:** Given 调用方具备权限（含调用方即目标应用自身、callingUid==uid 的自记录场景） When 通过 IPC 调用 `RecordAppWithReason(pid, uid, exitReason)` Then 新生成的终止原因记录中包含调用方 pid 与 processName（processName 可能为空）；接口返回值与现状一致
- **AC-1.4:** Given 调用方进程信息反查失败（进程已退出、反查异常等） When 调用上述任一接口成功 Then 调用方 pid 正常记录，processName 记录为空字符串，接口返回值不受影响
- **AC-1.5:** Given 调用方不具备权限（非 SA、非 shell、且非自记录场景） When 调用上述任一接口 Then 返回 `ERR_PERMISSION_DENIED`（值 1），不产生新的终止原因记录（行为与现状一致）
- **AC-1.6:** Given 存量终止原因数据（本特性上线前写入，不含调用方字段） When 读回该记录 Then 读取成功、无错误，调用方 pid/processName 表现为空（缺省值），其余既有字段读取结果不变
- **AC-1.7:** Given 终止被既有保护规则拦截（前台保护、启动期保护、保活等） When 调用 `KillAppWithReason`/`KillBundleWithReason` 返回既有错误码 Then 不新增终止原因记录（含调用方信息），行为与现状一致
- **AC-1.8:** Given 两个不同调用方并发调用三个接口 When 各自调用完成 Then 每条生成的终止原因记录中的调用方 pid 与该次调用的实际调用方一致，无串扰
- **AC-1.9:** Given 现网/测试设备正常运行，系统组件发起终止/记录操作 When 按周期抽样检查终止原因数据 Then 含调用方 pid 的记录占比 ≥95%（反查失败的少数场景允许仅 pid 无 processName）

## 业务规则

> 业务规则是**约束声明**（非验收标准）；详细内部实现展开移 design.md「状态归属与不变量」。

| 规则 ID | 规则描述 | 约束条件 | 关联 AC |
|---------|----------|----------|---------|
| BR-1 | 调用方 pid 由服务端从 IPC 调用上下文自动获取，接口签名与 IPC 序列化格式不变，调用方无需任何修改 | 自动获取；禁止要求调用方传参 | AC-1.1~1.3 |
| BR-2 | 三个接口产生的终止原因记录均须携带调用方 pid 与 processName；processName 允许为空 | 覆盖 KillAppWithReason / KillBundleWithReason / RecordAppWithReason 的全部记录生成路径 | AC-1.1~1.4 |
| BR-3 | 调用方信息获取/记录失败不得影响终止与记录主流程及接口返回值 | 容错记录；失败仅日志 | AC-1.4, AC-1.5 |
| BR-4 | 调用方 processName 通过既有运行进程信息按 pid 反查获得，不新增对外查询能力 | 复用现有进程信息查询通道 | AC-1.1~1.4 |
| BR-5 | 存量数据无调用方字段时按空值兼容，不迁移、不报错 | 向后兼容读取 | AC-1.6 |
| BR-6 | 服务进程内调用（in-process）时，调用方 pid 记为 ability_runtime 服务所在进程自身 pid | 进程内调用无独立调用方 | AC-1.3 |
| BR-7 | 调用方信息与终止原因数据同存储、同生命周期，随既有删除逻辑一并清除 | 不引入独立存储与独立清理 | AC-1.1~1.6 |

## 异常与边界规则

| 编号 | 场景 | 触发条件 | 系统行为 | 关联 AC |
|------|------|----------|----------|---------|
| E-1 | 调用方进程名反查失败 | 调用方进程已退出或进程信息查询失败 | processName 记为空字符串，pid 正常记录，主流程不阻塞、返回值不变 | AC-1.4 |
| E-2 | 进程内调用 | ability_runtime 服务进程内部直接调用（非跨进程） | 调用方 pid 记为服务自身进程 pid | AC-1.3 |
| E-3 | 并发调用 | 多个调用方并发调用三个接口 | 每条记录的调用方信息与该次请求的 IPC 调用方严格对应，互不串扰 | AC-1.8 |
| E-4 | 终止原因数据写入失败 | KV 写入异常 | 沿用既有终止原因数据写入失败处理，不新增额外处理分支 | AC-1.1~1.3 |
| E-5 | 终止原因非法 | `IsExitReasonValid` 校验失败 | 返回 `ERR_INVALID_VALUE`（值 22）；非法 killId 记录路径是否携带调用方信息遵循既有记录路径的统一规则（BR-2），行为细节由 design 明确 | — |

## 错误码定义

> 本特性**不新增错误码**，接口返回值语义与现状一致。下表列出三个接口相关既有错误码（代码事实核对结果）：

| 错误码 ID | 错误码值 | 含义 | 关联 AC |
|-----------|----------|------|---------|
| ERR_OK | 0 | 成功 | AC-1.1~1.3 |
| ERR_PERMISSION_DENIED | 1 | 权限校验失败（定义于 commonlibrary c_utils `errors.h`：BASE_ERR_OFFSET + EPERM） | AC-1.5 |
| ERR_INVALID_VALUE | 22 | 终止原因非法（定义于 commonlibrary c_utils `errors.h`：BASE_ERR_OFFSET + EINVAL） | E-5 |
| ERR_NULL_OBJECT | 7 | 内部对象为空（定义于 `ipc_types.h`） | —（既有） |
| ERR_NULL_APP_EXIT_REASON_HELPER | 2097288 | 终止原因记录对象为空（定义于 `ability_manager_errors.h`） | —（既有） |
| ERR_KILL_APP_WHILE_STARTING | 2097331 | 启动期保护，不终止（定义于 `ability_manager_errors.h`） | AC-1.7 |
| ERR_KILL_APP_WHILE_FOREGROUND | 2097339 | 前台保护，不终止（定义于 `ability_manager_errors.h`） | AC-1.7 |
| KILL_PROCESS_FAILED / GET_BUNDLE_INFO_FAILED / KILL_PROCESS_KEEP_ALIVE | 既有枚举值（`ability_manager_errors.h` 位置编号，本特性不改动） | 终止失败 / 获取包信息失败 / 保活进程不终止 | AC-1.7 |

## API 规格定义

不涉及。理由：proposal.md `API/SDK` 维度为「否」——本特性为内部维测记录扩展，不新增/修改任何 public/system API，`interfaces/kits` 无变更；三个终止/记录接口为 AbilityManagerService 的 inner API 且签名不变。

## 接口变更分析

### 新增接口

无。

### 变更/废弃接口

| 接口名称 | 开放级别 | 变更类型 | 影响场景 | 迁移指引 | 关联 AC |
|----------|----------|----------|----------|----------|---------|
| `KillAppWithReason(pid, exitReason)` | Inner | 行为扩展（签名不变，记录内容新增调用方信息） | 系统组件按 pid 终止应用 | 无需迁移 | AC-1.1, AC-1.4~1.8 |
| `KillBundleWithReason(bundleName, userId, appIndex, exitReason)` | Inner | 行为扩展（签名不变，记录内容新增调用方信息） | 系统组件按包名终止应用 | 无需迁移 | AC-1.2, AC-1.4~1.8 |
| `RecordAppWithReason(pid, uid, exitReason)` | Inner | 行为扩展（签名不变，记录内容新增调用方信息） | 系统组件/应用自身记录退出原因 | 无需迁移 | AC-1.3, AC-1.4~1.8 |

## 兼容性声明

- **已有 API 行为变更:** 否（三个接口签名与返回值语义不变，仅记录内容增加）
- **配置文件格式变更:** 否
- **数据存储格式变更:** 是（终止原因 KV JSON 值新增调用方 pid / processName 字段；向后兼容——存量数据缺字段按空值处理，不做数据迁移）

## 验证映射

| AC | 关联规则 | 验证方式 | 证据 |
|----|----------|----------|------|
| AC-1.1 | BR-1, BR-2, BR-7 | UT（`KillAppWithReason` 路径，mock IPC 调用方，终止原因数据读回断言） | 单测报告 |
| AC-1.2 | BR-1, BR-2 | UT（`KillBundleWithReason` 路径，同上） | 单测报告 |
| AC-1.3 | BR-1, BR-2, BR-6 | UT（`RecordAppWithReason` 路径，含自记录场景） | 单测报告 |
| AC-1.4 | BR-3, BR-4 | UT（mock 进程信息反查失败） | 单测报告 |
| AC-1.5 | BR-3 | UT（mock 权限校验失败，断言返回值与无新增记录） | 单测报告 |
| AC-1.6 | BR-5 | UT（构造不含调用方字段的旧格式 KV 值，读回断言） | 单测报告 |
| AC-1.7 | — | UT（前台/启动期/保活拦截路径，断言无新增记录） | 单测报告 |
| AC-1.8 | BR-1 | UT 并发用例（双调用方并发，逐条断言调用方归属） | 单测报告 |
| AC-1.9 | BR-2 | 现网/测试设备抽样统计（日志 + 终止原因数据抽样） | 抽样统计报告 |
| E-5 | — | UT（非法终止原因路径） | 单测报告 |

## 测试设计提示

| AC | 测试类型 | 测试文件 | 测试名称 | 输入/触发 | 期望输出/错误 | Red 条件 |
|----|----------|----------|----------|-----------|---------------|----------|
| AC-1.1 | UT | test/unittest/ability_manager_service_*_test（Kill 相关 suite）或 test/unittest/app_exit_reason_helper_test | KillAppWithReasonRecordsCallerInfo | mock 具备权限的 IPC 调用方 + 有效进程，调用 KillAppWithReason | 返回 0；读回记录含调用方 pid/processName | 实现前记录生成路径未采集调用方信息，读回断言失败 |
| AC-1.2 | UT | 同上 | KillBundleWithReasonRecordsCallerInfo | mock 调用方 + 有效 bundle，调用 KillBundleWithReason | 返回 0；每条记录携带同一调用方信息 | 同上 |
| AC-1.3 | UT | test/unittest/app_exit_reason_helper_test | RecordAppWithReasonRecordsCallerInfo | mock 调用方（含 callingUid==uid 自记录），调用 RecordAppWithReason | 返回 0；记录含调用方信息 | 同上 |
| AC-1.4 | UT | test/unittest/app_exit_reason_helper_test | RecordCallerInfoWhenProcessNameLookupFailed | mock 调用方进程信息反查失败 | 返回 0；pid 有值、processName 为空 | 实现前无该容错路径，断言失败 |
| AC-1.5 | UT | test/unittest/ability_manager_service_*_test | KillWithReasonPermissionDeniedNoRecord | mock 无权限调用方 | 返回 1（ERR_PERMISSION_DENIED）；无新增记录 | 实现前已有该返回行为，需补"无新增记录"断言（记录能力未实现前整体为 Red） |
| AC-1.6 | UT | test/unittest/app_exit_reason_data_manager_test | ReadLegacyExitReasonWithoutCallerFields | 写入不含调用方字段的旧格式 KV 值后读回 | 读取成功；调用方字段为空；既有字段正确 | 实现前读回结构无调用方字段，兼容解析断言失败 |
| AC-1.7 | UT | test/unittest/ability_manager_service_*_test | KillBlockedNoNewRecord | mock 前台/启动期/保活拦截 | 返回既有错误码（2097339/2097331/KILL_PROCESS_KEEP_ALIVE）；无新增记录 | 需实现后回归保证不被破坏（防回归断言） |
| AC-1.8 | UT | test/unittest/app_exit_reason_helper_test | ConcurrentCallersRecordCorrectly | 两个不同 mock 调用方并发调用 | 每条记录调用方 pid 与实际调用方一致 | 实现前无调用方记录，断言失败 |
| AC-1.9 | 现网/设备抽样 | 不适用（人工/工具抽样） | — | 设备运行期间系统组件终止事件 | 含调用方 pid 的记录占比 ≥95% | 实现前记录中无调用方字段，占比为 0 |

> AC → 实现文件 + Task + 验证状态的映射见 `execution-plan.md`「AC 到 Task 追溯」+「代码范围映射」。
