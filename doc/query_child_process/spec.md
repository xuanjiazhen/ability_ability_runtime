# Spec — 提供查询应用自身子进程接口

> 查询应用自身子进程（不带页面子进程 + UIAbility 子进程）的 C 接口
> 创建日期: 2026-06-04
> 特性编号: query-child-process
> 优先级: P1

## 一、概述

为 OpenHarmony 应用提供查询自身子进程的 C 接口。当前系统支持多种创建子进程的方式（Native C API、ArkTS childProcessManager、UIAbility 跨进程），但缺少统一的查询能力。本特性新增 3 个 C API（2 个查询 + 1 个释放），通过 IPC 从 AppMgr 服务端获取子进程信息，返回 pid/hostPid/processName。

| 属性 | 值 |
|------|-----|
| 特性名称 | 提供查询应用自身子进程接口 |
| 特性编号 | query-child-process |
| 优先级 | P1 |
| 复杂度 | 简单 |
| 目标版本 | TBD |
| Profile | ability_runtime |

---

## 二、用户故事与验收标准

### US-01: 查询不带页面的子进程

**作为** 应用开发者，**我想要** 通过 C 接口查询当前进程创建的不带页面的子进程列表，**以便** 在运行时感知和管理自身创建的子进程。

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-1.1 | WHEN 调用 `OH_Ability_GetChildProcesses(&infos, &count)` 且当前进程通过 `OH_Ability_CreateNativeChildProcess` 创建了子进程 THEN 返回 `NCP_NO_ERROR`，`count >= 1`，通过 getter 可获取该子进程的 pid、hostPid、processName | 正常 | US-01 |
| AC-1.2 | WHEN 调用 `OH_Ability_GetChildProcesses(&infos, &count)` 且当前进程通过 `OH_Ability_StartNativeChildProcess` / `OH_Ability_StartNativeChildProcessWithConfigs` 创建了子进程 THEN 返回 `NCP_NO_ERROR`，查询结果包含该子进程 | 正常 | US-01 |
| AC-1.3 | WHEN 调用 `OH_Ability_GetChildProcesses(&infos, &count)` 且当前进程通过 `childProcessManager.startArkChildProcess` 创建了子进程 THEN 返回 `NCP_NO_ERROR`，查询结果包含该子进程 | 正常 | US-01 |
| AC-1.4 | WHEN 调用 `OH_Ability_GetChildProcesses(&infos, &count)` 且当前进程通过 `childProcessManager.startChildProcess`（非 SELF_FORK 模式）或 `childProcessManager.startNativeChildProcess` 创建了子进程 THEN 返回 `NCP_NO_ERROR`，查询结果包含该子进程 | 正常 | US-01 |
| AC-1.5 | WHEN 调用 `OH_Ability_GetChildProcesses` 且当前进程通过 `childProcessManager.startChildProcess`（SELF_FORK 模式）创建了子进程 THEN 查询结果不包含该子进程（SELF_FORK 不走 AppMgr，不在 childProcessRecordMap_ 中） | 边界 | US-01 |

### US-02: 查询 UIAbility 子进程

**作为** 应用开发者，**我想要** 通过 C 接口查询通过 `startAbility` + `NEW_PROCESS_ATTACH_TO_PARENT` 创建的 UIAbility 子进程，**以便** 跟踪跨进程 UIAbility 的运行状态。

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-2.1 | WHEN 调用 `OH_AbilityRuntime_GetUIAbilityChildProcesses(&infos, &count)` 且当前进程通过 `startAbility` + ProcessMode `NEW_PROCESS_ATTACH_TO_PARENT` 创建了 UIAbility 子进程 THEN 返回 `ABILITY_RUNTIME_ERROR_CODE_NO_ERROR`，`count >= 1`，通过 getter 可获取该子进程的 pid、hostPid、processName | 正常 | US-02 |
| AC-2.2 | WHEN UIAbility 子进程退出后调用 `OH_Ability_GetUIAbilityChildProcesses` THEN 查询结果不包含已退出的子进程（weak_ptr.lock() 返回 nullptr 时跳过） | 生命周期 | US-02 |

### US-03: 子进程生命周期与空结果

**作为** 应用开发者，**我想要** 查询结果实时反映子进程的存活状态，**以便** 准确掌握当前运行的子进程。

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-3.1 | WHEN 不带页面的子进程退出后调用 `OH_Ability_GetChildProcesses` THEN 查询结果不包含已退出的子进程（childProcessRecordMap_ 中记录已移除） | 生命周期 | US-03 |
| AC-3.2 | WHEN 当前进程无任何子进程时调用 `OH_Ability_GetChildProcesses(&infos, &count)` 或 `OH_AbilityRuntime_GetUIAbilityChildProcesses(&infos, &count)` THEN 返回成功，`count = 0`，`*infos = nullptr` | 边界 | US-03 |

### US-04: 参数校验与错误处理

**作为** 应用开发者，**我想要** 接口对非法参数返回明确的错误码，**以便** 在开发阶段快速定位问题。

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-4.1 | WHEN 调用查询函数时 infos 或 count 为 nullptr THEN 返回 `NCP_ERR_INVALID_PARAM`（401），不发生崩溃 | 异常 | US-04 |
| AC-4.2 | WHEN 调用 getter 函数时 infos 为 nullptr 或 index >= count THEN 返回 `NCP_ERR_INVALID_PARAM`（401），不发生崩溃 | 异常 | US-04 |
| AC-4.3 | WHEN IPC 调用失败（AppMgrProxy 为 nullptr 或 SendRequest 失败）THEN 查询函数返回 `NCP_ERR_INTERNAL`（16000050） | 异常 | US-04 |

### US-05: 安全隔离

**作为** 系统安全设计者，**我想要** 查询接口只返回调用者自身进程的子进程，**以便** 防止跨应用信息泄露。

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-5.1 | WHEN 调用者 accessTokenId 与目标 AppRunningRecord 的 accessTokenId 不匹配 THEN 服务端返回错误，不返回任何子进程信息 | 安全 | US-05 |
| AC-5.2 | WHEN 不同应用（不同 token）的进程调用查询接口 THEN 各自只能查到自己的子进程，无法查到其他应用的子进程 | 安全 | US-05 |

### US-06: 内存管理

**作为** 应用开发者，**我想要** 通过专用释放函数安全释放查询结果占用的内存，**以便** 避免内存泄漏。

| AC编号 | 验收标准 | 类型 | 关联Story |
|--------|----------|------|-----------|
| AC-6.1 | WHEN 调用 `OH_Ability_DestroyChildProcessInfos(infos)` 且 infos 非空 THEN 释放所有 processName 字符串（free）、items 数组（delete[]）和外壳对象（delete），无内存泄漏 | 正常 | US-06 |
| AC-6.2 | WHEN 调用 `OH_Ability_DestroyChildProcessInfos(nullptr)` THEN 函数直接返回，不发生崩溃 | 边界 | US-06 |
| AC-6.3 | WHEN `OH_Ability_GetChildProcesses` 内部 `strdup` 失败 THEN 已分配的内存被正确回滚（free 已分配的 processName + delete[] items + delete 外壳），返回 `NCP_ERR_INTERNAL`，`*infos = nullptr`，`*count = 0` | 异常 | US-06 |

---

## 三、业务规则

| 规则 ID | 规则描述 | 约束条件 | 关联 AC |
|---------|----------|----------|---------|
| BR-001 | 查询接口仅返回调用者自身进程（callingPid 匹配）的子进程 | 通过 IPCSkeleton::GetCallingPid() 校验 | AC-1.1 ~ AC-2.2 |
| BR-002 | 同 token 校验：accessTokenId 必须与 AppRunningRecord 的 applicationInfo.accessTokenId 一致 | 不一致时返回 ERR_INVALID_VALUE | AC-5.1, AC-5.2 |
| BR-003 | SELF_FORK 模式子进程由客户端直接 fork()，不经 AppMgr 注册，不在 childProcessRecordMap_ 中 | 天然排除，无需额外过滤逻辑 | AC-1.5 |
| BR-004 | UIAbility 子进程存储为 `weak_ptr<AppRunningRecord>`，查询时必须先 lock() 检查存活 | 已退出的子进程的 weak_ptr 返回 nullptr，跳过 | AC-2.2 |
| BR-005 | 不带页面的子进程存储为 `shared_ptr<ChildProcessRecord>`，查询时直接读取 | 已退出的子进程记录会从 map 中移除 | AC-3.1 |
| BR-006 | 无子进程时 infos 输出 nullptr、count 输出 0，返回 NCP_NO_ERROR | 空结果也是合法成功状态 | AC-3.2 |
| BR-007 | C API 采用不透明指针模式：结构体定义仅在 .cpp 内部，调用者通过 getter 函数读取属性，通过 Destroy 释放 | 调用者无法直接操作内部内存字段，确保封装安全性 | AC-6.1 |
| BR-008 | IPC 传输复用现有 ChildProcessInfo（parcelable），仅填充 pid/hostPid/processName | 其余字段保持默认值 | AC-1.1, AC-2.1 |
| BR-009 | 所有新增代码在 `#ifdef SUPPORT_CHILD_PROCESS` 宏保护下 | 不支持子进程的平台编译时排除 | — |
| BR-010 | hostPid 取自 IPCSkeleton::GetCallingPid()（即父进程 PID） | 非子进程自身的父 PID | AC-1.1, AC-2.1 |

---

## 四、异常与边界规则

| 编号 | 场景 | 触发条件 | 系统行为 | 关联 AC |
|------|------|----------|----------|---------|
| EX-001 | 查询函数 infos 为 nullptr | 调用查询函数时 infos 传入 nullptr | 返回 `NCP_ERR_INVALID_PARAM`，不崩溃 | AC-4.1 |
| EX-002 | 查询函数 count 为 nullptr | 调用查询函数时 count 传入 nullptr | 返回 `NCP_ERR_INVALID_PARAM`，不崩溃 | AC-4.1 |
| EX-003 | IPC Proxy 为空 | AppMgrProxy 未初始化 | 查询函数返回 `NCP_ERR_INTERNAL` | AC-4.3 |
| EX-004 | IPC SendRequest 失败 | IPC 通信异常 | 查询函数返回 `NCP_ERR_INTERNAL` | AC-4.3 |
| EX-005 | AppRunningRecord 不存在 | callingPid 对应的 AppRunningRecord 为 nullptr | 服务端返回 ERR_INVALID_VALUE，C API 返回 `NCP_ERR_INTERNAL` | AC-4.3 |
| EX-006 | token 不匹配 | accessTokenId 不一致 | 服务端返回 ERR_INVALID_VALUE | AC-5.1 |
| EX-007 | strdup 内存不足 | 系统内存不足 | 回滚已分配内存，返回 `NCP_ERR_INTERNAL`，*infos = nullptr | AC-6.3 |
| EX-008 | new[] 内存不足 | 系统内存不足 | 返回 `NCP_ERR_INTERNAL` | AC-6.3 |
| EX-009 | Destroy 传入 nullptr | infos 为 nullptr | 直接返回，不操作 | AC-6.2 |
| EX-011 | Getter 传入无效索引 | index >= count | 返回 `NCP_ERR_INVALID_PARAM` | AC-4.1 |
| EX-012 | Getter 传入 nullptr infos | infos 为 nullptr | 返回 `NCP_ERR_INVALID_PARAM` | AC-4.1 |
| EX-013 | Getter 传入 nullptr 输出参数 | pid/hostPid/processName 为 nullptr | 返回 `NCP_ERR_INVALID_PARAM` | AC-4.1 |
| EX-010 | UIAbility 子进程 weak_ptr 失效 | 子进程在 IPC 传输期间退出 | lock() 返回 nullptr，跳过该条目 | AC-2.2 |

---

## 五、错误码定义

| 错误码 ID | 错误码值 | 含义 | 关联 AC |
|-----------|----------|------|---------|
| NCP_NO_ERROR | 0 | 成功 | AC-1.1 ~ AC-3.2 |
| NCP_ERR_INVALID_PARAM | 401 | infos 或 count 参数为 nullptr | AC-4.1, AC-4.2 |
| NCP_ERR_INTERNAL | 16000050 | IPC 失败、服务端错误、内存不足等内部错误 | AC-4.3, AC-6.3 |

---

## 六、接口变更分析

### 新增接口

| 接口名称 | 开放级别 | 参数概要 | 返回值 | 错误码 | 关联 AC |
|----------|----------|----------|--------|--------|---------|
| `OH_Ability_GetChildProcesses` | C API (Public) | `Ability_ChildProcessInfos** infos, uint32_t* count` | `Ability_NativeChildProcess_ErrCode` | NCP_NO_ERROR / NCP_ERR_INVALID_PARAM / NCP_ERR_INTERNAL | AC-1.1 ~ AC-1.5, AC-3.1, AC-3.2, AC-4.1 |
| `OH_AbilityRuntime_GetUIAbilityChildProcesses` | C API (Public) | `Ability_ChildProcessInfos** infos, uint32_t* count` | `AbilityRuntime_ErrorCode` | ABILITY_RUNTIME_ERROR_CODE_NO_ERROR / ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID / ABILITY_RUNTIME_ERROR_CODE_INTERNAL | AC-2.1, AC-2.2, AC-3.2, AC-4.1 |
| `OH_Ability_ChildProcessInfos_GetPid` | C API (Public) | `Ability_ChildProcessInfos* infos, uint32_t index, int32_t* pid` | `Ability_NativeChildProcess_ErrCode` | NCP_NO_ERROR / NCP_ERR_INVALID_PARAM | AC-1.1 |
| `OH_Ability_ChildProcessInfos_GetHostPid` | C API (Public) | `Ability_ChildProcessInfos* infos, uint32_t index, int32_t* hostPid` | `Ability_NativeChildProcess_ErrCode` | NCP_NO_ERROR / NCP_ERR_INVALID_PARAM | AC-1.1 |
| `OH_Ability_ChildProcessInfos_GetProcessName` | C API (Public) | `Ability_ChildProcessInfos* infos, uint32_t index, const char** processName` | `Ability_NativeChildProcess_ErrCode` | NCP_NO_ERROR / NCP_ERR_INVALID_PARAM | AC-1.1 |
| `OH_Ability_DestroyChildProcessInfos` | C API (Public) | `Ability_ChildProcessInfos* infos` | `void` | — | AC-6.1, AC-6.2 |

### 新增数据结构

| 结构体名称 | 说明 | 可见性 | 关联 AC |
|-----------|------|--------|---------|
| `Ability_ChildProcessInfos` | 子进程信息集合（不透明类型，定义仅在 .cpp 内部） | 头文件仅前向声明 | AC-1.1 |

### 变更/废弃接口

无。本次仅新增接口，不修改已有接口行为。

---

## 七、兼容性声明

- **已有 API 行为变更:** 否。新增独立接口，不影响任何已有 API 行为
- **配置文件格式变更:** 否
- **数据存储格式变更:** 否
- **编译宏保护:** 所有新增代码在 `#ifdef SUPPORT_CHILD_PROCESS` 下，不支持子进程的平台无影响
- **IPC 兼容性:** 新增 IPC 方法编号，不影响已有 IPC 消息处理

---

## 八、验证映射

| ID | 规格项 | 验证方式 | 验证命令/方法 | 状态 |
|----|--------|---------|---------------|------|
| V-001 | AC-1.1 ~ AC-1.4（不带页面的子进程查询） | 功能测试 | 分别通过各创建方式创建子进程后调用 OH_Ability_GetChildProcesses | 待实现 |
| V-002 | AC-1.5（SELF_FORK 排除） | 功能测试 | 通过 SELF_FORK 模式创建子进程后验证查询结果不包含 | 待实现 |
| V-003 | AC-2.1 ~ AC-2.2（UIAbility 子进程查询） | 集成测试 | startAbility + NEW_PROCESS_ATTACH_TO_PARENT 后查询；退出后查询 | 待实现 |
| V-004 | AC-3.1 ~ AC-3.2（生命周期与空结果） | 功能测试 | 子进程退出后查询；无子进程时查询 | 待实现 |
| V-005 | AC-4.1 ~ AC-4.3（参数校验与错误处理） | 单元测试 | nullptr 参数、IPC 失败场景、getter 越界索引 | 待实现 |
| V-006 | AC-5.1 ~ AC-5.2（安全隔离） | 安全测试 | 不同 token 应用互相查询 | 待实现 |
| V-007 | AC-6.1 ~ AC-6.3（内存管理） | 内存测试 | Destroy 释放验证、nullptr 安全、strdup 失败回滚 | 待实现 |

### 测试用例详情

#### 不带页面的子进程查询测试

| 用例 ID | 用例名称 | 前置条件 | 操作步骤 | 预期结果 |
|---------|----------|----------|----------|----------|
| TC-001 | 查询 CreateNativeChildProcess 创建的子进程 | 已通过 OH_Ability_CreateNativeChildProcess 创建子进程 | 调用 OH_Ability_GetChildProcesses，通过 getter 获取属性 | 返回 NCP_NO_ERROR，count >= 1，包含该子进程 pid/hostPid/processName |
| TC-002 | 查询 StartNativeChildProcess 创建的子进程 | 已通过 OH_Ability_StartNativeChildProcess 创建子进程 | 调用 OH_Ability_GetChildProcesses | 返回 NCP_NO_ERROR，包含该子进程 |
| TC-003 | 查询 ArkTS 子进程 | 已通过 startArkChildProcess 创建子进程 | 调用 OH_Ability_GetChildProcesses | 返回 NCP_NO_ERROR，包含该子进程 |
| TC-004 | 查询非 SELF_FORK 的 startChildProcess | 已通过 startChildProcess（非 SELF_FORK）创建子进程 | 调用 OH_Ability_GetChildProcesses | 返回 NCP_NO_ERROR，包含该子进程 |
| TC-005 | SELF_FORK 子进程排除 | 已通过 startChildProcess（SELF_FORK）创建子进程 | 调用 OH_Ability_GetChildProcesses | 该子进程不在结果中 |

#### UIAbility 子进程查询测试

| 用例 ID | 用例名称 | 前置条件 | 操作步骤 | 预期结果 |
|---------|----------|----------|----------|----------|
| TC-006 | 查询 UIAbility 子进程 | 已通过 startAbility + NEW_PROCESS_ATTACH_TO_PARENT 创建子进程 | 调用 OH_Ability_GetUIAbilityChildProcesses | 返回 NCP_NO_ERROR，包含该子进程 |
| TC-007 | UIAbility 子进程退出后不可查 | TC-006 前提下子进程退出 | 调用 OH_Ability_GetUIAbilityChildProcesses | count 中不包含已退出的子进程 |

#### 生命周期与空结果测试

| 用例 ID | 用例名称 | 前置条件 | 操作步骤 | 预期结果 |
|---------|----------|----------|----------|----------|
| TC-008 | 不带页面子进程退出后不可查 | 创建并退出子进程 | 调用 OH_Ability_GetChildProcesses | count 中不包含已退出的子进程 |
| TC-009 | 无子进程时返回空 | 进程未创建任何子进程 | 调用 OH_Ability_GetChildProcesses 和 OH_Ability_GetUIAbilityChildProcesses | 均返回 NCP_NO_ERROR，count=0，infos=nullptr |

#### 参数校验测试

| 用例 ID | 用例名称 | 前置条件 | 操作步骤 | 预期结果 |
|---------|----------|----------|----------|----------|
| TC-010 | 查询函数 infos 为 nullptr | 无 | OH_Ability_GetChildProcesses(nullptr, &count) | 返回 NCP_ERR_INVALID_PARAM |
| TC-011 | 查询函数 count 为 nullptr | 已创建 infos | OH_Ability_GetChildProcesses(infos, nullptr) | 返回 NCP_ERR_INVALID_PARAM |
| TC-012 | UIAbility 查询 infos 为 nullptr | 无 | OH_Ability_GetUIAbilityChildProcesses(nullptr, &count) | 返回 NCP_ERR_INVALID_PARAM |
| TC-013 | UIAbility 查询 count 为 nullptr | 已创建 infos | OH_Ability_GetUIAbilityChildProcesses(infos, nullptr) | 返回 NCP_ERR_INVALID_PARAM |

#### Getter 函数参数校验测试

| 用例 ID | 用例名称 | 前置条件 | 操作步骤 | 预期结果 |
|---------|----------|----------|----------|----------|
| TC-019 | GetPid 传入 nullptr infos | 无 | OH_Ability_ChildProcessInfos_GetPid(nullptr, 0, &pid) | 返回 NCP_ERR_INVALID_PARAM |
| TC-020 | GetPid 传入 nullptr 输出参数 | 已查询获取 infos | OH_Ability_ChildProcessInfos_GetPid(infos, 0, nullptr) | 返回 NCP_ERR_INVALID_PARAM |
| TC-021 | GetPid 传入越界索引 | 已查询获取 infos（count=N） | OH_Ability_ChildProcessInfos_GetPid(infos, N, &pid) | 返回 NCP_ERR_INVALID_PARAM |
| TC-022 | GetProcessName 传入 nullptr 输出参数 | 已查询获取 infos | OH_Ability_ChildProcessInfos_GetProcessName(infos, 0, nullptr) | 返回 NCP_ERR_INVALID_PARAM |

#### 安全隔离测试

| 用例 ID | 用例名称 | 前置条件 | 操作步骤 | 预期结果 |
|---------|----------|----------|----------|----------|
| TC-014 | 不同 token 应用不可互查 | 应用 A 有子进程 | 从应用 B 调用查询接口 | 应用 B 查不到应用 A 的子进程 |
| TC-015 | token 不匹配返回错误 | token 校验不一致 | 发起查询请求 | 服务端返回 ERR_INVALID_VALUE |

#### 内存管理测试

| 用例 ID | 用例名称 | 前置条件 | 操作步骤 | 预期结果 |
|---------|----------|----------|----------|----------|
| TC-016 | Destroy 正常释放 | 查询返回非空 infos | 调用 OH_Ability_DestroyChildProcessInfos(infos) | 无内存泄漏（ASan/Valgrind 验证） |
| TC-017 | Destroy 传入 nullptr | 无 | 调用 OH_Ability_DestroyChildProcessInfos(nullptr) | 直接返回，不崩溃 |
| TC-018 | 查询结果多次 Destroy | 已查询获取 infos | 连续调用两次 Destroy | 第二次为 UB（文档标注不可重复释放） |

---

## 九、代码映射

| AC 编号 | 验收标准摘要 | 预期实现模块/文件 | 实际实现文件 | 关联 Task | 验证状态 |
|---------|-------------|------------------|-------------|-----------|---------|
| AC-1.1 | CreateNativeChildProcess 子进程可查 | native_child_process.cpp / app_mgr_service_inner.cpp | | | |
| AC-1.2 | StartNativeChildProcess 子进程可查 | native_child_process.cpp / app_mgr_service_inner.cpp | | | |
| AC-1.3 | ArkTS 子进程可查 | native_child_process.cpp / app_mgr_service_inner.cpp | | | |
| AC-1.4 | 非 SELF_FORK startChildProcess 可查 | native_child_process.cpp / app_mgr_service_inner.cpp | | | |
| AC-1.5 | SELF_FORK 子进程排除 | 天然排除（无代码变更） | | | |
| AC-2.1 | UIAbility 子进程可查 | native_child_process.cpp / app_mgr_service_inner.cpp | | | |
| AC-2.2 | UIAbility 子进程退出后不可查 | app_mgr_service_inner.cpp（weak_ptr.lock 检查） | | | |
| AC-3.1 | 不带页面子进程退出后不可查 | app_mgr_service_inner.cpp | | | |
| AC-3.2 | 无子进程返回空 | native_child_process.cpp（ConvertToCInfos） | | | |
| AC-4.1 | GetChildProcesses 参数校验 | native_child_process.cpp（nullptr 检查） | | | |
| AC-4.2 | GetUIAbilityChildProcesses 参数校验 | native_child_process.cpp（nullptr 检查） | | | |
| AC-4.3 | IPC 失败返回内部错误 | native_child_process.cpp / child_process_manager.cpp | | | |
| AC-5.1 | token 不匹配拒绝查询 | app_mgr_service_inner.cpp（accessTokenId 校验） | | | |
| AC-5.2 | 跨应用隔离 | app_mgr_service_inner.cpp（callingPid + tokenId） | | | |
| AC-6.1 | Destroy 正确释放混合分配器内存 | native_child_process.cpp（Destroy 函数） | | | |
| AC-6.2 | Destroy nullptr 安全 | native_child_process.cpp | | | |
| AC-6.3 | strdup 失败回滚 | native_child_process.cpp（ConvertToCInfos 错误路径） | | | |

---

## 十、术语表

| 术语 | 定义 |
|------|------|
| `OH_Ability_GetChildProcesses` | 查询当前进程创建的不带页面的子进程列表 |
| `OH_Ability_GetUIAbilityChildProcesses` | 查询当前进程创建的 UIAbility 子进程列表 |
| `OH_Ability_DestroyChildProcessInfos` | 释放查询接口返回的子进程信息数组 |
| `OH_Ability_ChildProcessInfo` | 子进程信息结构体，包含 pid/hostPid/processName |
| `childProcessRecordMap_` | AppRunningRecord 中存储不带页面子进程的 map，key 为 pid_t，value 为 shared_ptr<ChildProcessRecord> |
| `childAppRecordMap_` | AppRunningRecord 中存储 UIAbility 子进程的 map，key 为 pid_t，value 为 weak_ptr<AppRunningRecord> |
| `NEW_PROCESS_ATTACH_TO_PARENT` | ProcessMode 枚举值，表示在新的进程中启动 Ability 并绑定到父进程 |
| `SELF_FORK` | 子进程模式之一，客户端直接 fork() 创建，不经 AppMgr 注册 |
| `Ability_NativeChildProcess_ErrCode` | 子进程相关 C API 的错误码枚举 |
| `SUPPORT_CHILD_PROCESS` | 条件编译宏，控制子进程相关功能的编译 |
