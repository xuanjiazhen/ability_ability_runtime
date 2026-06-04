# REQ-004 执行计划 — 提供查询应用自身子进程接口

> 版本: v1.0
> 日期: 2026-06-05
> 基于: design.md v1.0 / spec.md v1.0
> 需求跟踪: pilots/ability/reqs/REQ-004.md

## 一、实现概览

本执行计划按**自底向上**的依赖关系排列：先实现服务端数据查询，再实现 IPC 通道，最后实现客户端 C API 层。

```
实现顺序：服务端 → IPC 层 → 客户端 → C API 层（共享头文件 + 各 .so 实现）→ SDK 头文件
```

## 二、任务分解

### Task 1：服务端 — 新增查询方法

**文件**：
- `services/appmgr/include/app_mgr_service_inner.h` — 新增 2 个方法声明
- `services/appmgr/src/app_mgr_service_inner.cpp` — 新增 2 个方法实现

**实现内容**：

| 方法 | 数据来源 | 关键逻辑 |
|------|---------|---------|
| `GetChildProcessesForSelf` | `childProcessRecordMap_`（`shared_ptr`） | callingPid 匹配 + accessTokenId 校验 + 遍历填充 `ChildProcessInfo` |
| `GetUIAbilityChildProcessesForSelf` | `childAppRecordMap_`（`weak_ptr`） | 同上 + `weak_ptr::lock()` 存活检查 |

**宏保护**：`#ifdef SUPPORT_CHILD_PROCESS`

**关联 AC**：AC-1.1~1.5, AC-2.1~2.2, AC-3.1, AC-5.1~5.2

### Task 2：IPC 接口 — 新增虚方法

**文件**：
- `interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h` — 新增 2 个纯虚方法

**实现内容**：

```cpp
#ifdef SUPPORT_CHILD_PROCESS
virtual int GetChildProcessesForSelf(std::vector<ChildProcessInfo>& infos) = 0;
virtual int GetUIAbilityChildProcessesForSelf(std::vector<ChildProcessInfo>& infos) = 0;
#endif
```

**关联 AC**：AC-4.3（IPC 失败场景）

### Task 3：IPC Proxy — 客户端 IPC 调用

**文件**：
- `interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.h` — 新增 2 个方法声明
- `interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.cpp` — 新增 2 个方法实现

**实现内容**：
- 通过 `SendRequest` 调用服务端
- 使用 `ChildProcessInfo`（parcelable）序列化/反序列化
- 需新增 IPC 消息码（`AppMgrInterfaceCode` 枚举新增 2 项）

**关联 AC**：AC-4.3

### Task 4：IPC Stub — 服务端消息分发

**文件**：
- `services/appmgr/include/app_mgr_stub.h` — 新增 2 个方法声明
- `services/appmgr/src/app_mgr_stub.cpp` — 新增 IPC 消息分发

**实现内容**：
- 在 `OnRemoteRequest` 中新增 case 分支
- 反序列化请求，调用 `AppMgrServiceInner` 方法，序列化响应

**关联 AC**：AC-4.3

### Task 5：客户端 — ChildProcessManager 新增方法

**文件**：
- `interfaces/inner_api/child_process_manager/include/child_process_manager.h` — 新增 2 个方法声明
- `frameworks/native/ability/native/child_process_manager/child_process_manager.cpp` — 新增 2 个方法实现

**实现内容**：
- 获取 `AppMgrProxy`，调用对应 IPC 方法
- 处理 Proxy 为空的异常情况

**关联 AC**：AC-4.3

### Task 6：C API 实现 — 核心实现

实现分布在 3 个 .cpp 文件中：

**文件 1**：`frameworks/native/child_process/src/native_child_process.cpp`

| 函数 | 说明 |
|------|------|
| `OH_Ability_GetChildProcesses` | 查询不带页面的子进程，调用 `ChildProcessManager::GetChildProcesses` + `CreateAndFillChildProcessInfos` |

**文件 2**：`frameworks/c/ability_runtime/src/application_context.cpp`

| 函数 | 说明 |
|------|------|
| `OH_AbilityRuntime_GetUIAbilityChildProcesses` | 查询 UIAbility 子进程，直接调用 `AppMgrClient::GetUIAbilityChildProcessesForSelf` + `CreateAndFillChildProcessInfos` |

**文件 3**：`frameworks/native/child_process/src/child_process_info.cpp`（新增）

| 函数 | 说明 |
|------|------|
| `OH_Ability_ChildProcessInfos_GetPid` | Getter：获取 PID |
| `OH_Ability_ChildProcessInfos_GetHostPid` | Getter：获取父进程 PID |
| `OH_Ability_ChildProcessInfos_GetProcessName` | Getter：获取进程名 |
| `OH_Ability_DestroyChildProcessInfos` | 释放混合分配器内存 |

**内部头文件**：`frameworks/native/child_process/include/child_process_info_internal.h`（新增）
- 定义 `ChildProcessInfoItem` 和 `Ability_ChildProcessInfos` 结构体
- 提供 inline `CreateAndFillChildProcessInfos()` 辅助函数（`new[]` + `strdup` 混合分配器，含回滚逻辑）

**关联 AC**：AC-3.2, AC-4.1~4.2, AC-6.1~6.3

### Task 7：头文件 — 公共接口声明

**文件**：
- `interfaces/kits/c/ability/ability_runtime/child_process/child_process_info.h`（新增）— 共享类型 + getter/Destroy 声明
- `interfaces/kits/c/ability/ability_runtime/child_process/native_child_process.h` — include `child_process_info.h`，新增 `OH_Ability_GetChildProcesses`
- `interfaces/kits/c/ability_runtime/application_context.h` — include `child_process_info.h`，新增 `OH_AbilityRuntime_GetUIAbilityChildProcesses`

**实现内容**：

| 头文件 | 新增声明 |
|--------|---------|
| `child_process_info.h`（新增） | `Ability_ChildProcessInfos` 前向声明 + 3 个 Getter（返回 `int32_t`）+ `OH_Ability_DestroyChildProcessInfos` |
| `native_child_process.h` | `#include "child_process_info.h"`，移除已迁移的类型和函数声明，保留 `OH_Ability_GetChildProcesses` |
| `application_context.h` | `#include "child_process_info.h"`，新增 `OH_AbilityRuntime_GetUIAbilityChildProcesses` |

**关联 AC**：所有 AC

## 三、实现顺序与依赖

```
Task 1 (服务端) ──┐
                  ├── Task 2 (IPC 接口) ── Task 3 (Proxy) ── Task 5 (客户端) ── Task 6 (C API) ── Task 7 (头文件)
                  └── Task 4 (Stub) ──────────────────────────────────────────┘
```

**推荐实现顺序**：Task 1 → Task 2 → Task 3 + Task 4（并行）→ Task 5 → Task 6 → Task 7

## 四、文件变更清单

| # | 层级 | 文件路径 | 操作 | Task |
|---|------|---------|------|------|
| 1 | 服务端 | `services/appmgr/include/app_mgr_service_inner.h` | 修改 | Task 1 |
| 2 | 服务端 | `services/appmgr/src/app_mgr_service_inner.cpp` | 修改 | Task 1 |
| 3 | IPC 接口 | `interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h` | 修改 | Task 2 |
| 4 | IPC Proxy | `interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.h` | 修改 | Task 3 |
| 5 | IPC Proxy | `interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.cpp` | 修改 | Task 3 |
| 6 | IPC Stub | `services/appmgr/include/app_mgr_stub.h` | 修改 | Task 4 |
| 7 | IPC Stub | `services/appmgr/src/app_mgr_stub.cpp` | 修改 | Task 4 |
| 8 | 客户端 | `interfaces/inner_api/child_process_manager/include/child_process_manager.h` | 修改 | Task 5 |
| 9 | 客户端 | `frameworks/native/ability/native/child_process_manager/child_process_manager.cpp` | 修改 | Task 5 |
| 10 | C API 内部 | `frameworks/native/child_process/include/child_process_info_internal.h` | **新增** | Task 6 |
| 11 | C API 实现 | `frameworks/native/child_process/src/child_process_info.cpp` | **新增** | Task 6 |
| 12 | C API | `frameworks/native/child_process/src/native_child_process.cpp` | 修改 | Task 6 |
| 13 | C API | `frameworks/c/ability_runtime/src/application_context.cpp` | 修改 | Task 6 |
| 14 | C API 头文件 | `interfaces/kits/c/ability/ability_runtime/child_process/child_process_info.h` | **新增** | Task 7 |
| 15 | C API 头文件 | `interfaces/kits/c/ability/ability_runtime/child_process/native_child_process.h` | 修改 | Task 7 |
| 16 | C API 头文件 | `interfaces/kits/c/ability_runtime/application_context.h` | 修改 | Task 7 |
| 17 | C API 构建 | `frameworks/native/child_process/BUILD.gn` | 修改 | Task 6 |
| 18 | C API 构建 | `frameworks/c/ability_runtime/BUILD.gn` | 修改 | Task 6 |
| 19 | SDK 头文件 | `interface_sdk_c/AbilityKit/ability_runtime/child_process/child_process_info.h` | **新增** | Task 7 |
| 20 | SDK 头文件 | `interface_sdk_c/AbilityKit/ability_runtime/application_context.h` | 修改 | Task 7 |

共 **20 个文件**（新增 4 个，修改 16 个），所有新增代码在 `#ifdef SUPPORT_CHILD_PROCESS` 宏保护下。

## 五、风险与注意事项

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| IPC 消息码冲突 | 已有枚举值被占用 | 使用枚举末尾追加，避免修改已有值 |
| `strdup` 内存分配失败 | 查询结果不完整 | `CreateAndFillChildProcessInfos` 中实现回滚逻辑（design.md §6.2） |
| `weak_ptr` 并发失效 | UIAbility 子进程在遍历期间退出 | `lock()` 后立即拷贝数据，不持有 `weak_ptr` 引用 |
| `ChildProcessInfo` parcelable 兼容性 | 序列化/反序列化不一致 | 仅填充 pid/hostPid/processName，其余字段保持默认值 |

## 六、测试验证计划

| 测试类型 | 覆盖范围 | 关联 TC |
|---------|---------|---------|
| 单元测试 | 参数校验（nullptr、越界索引）、内存管理（Destroy、strdup 失败回滚） | TC-010~013, TC-016~018, TC-019~022 |
| 功能测试 | 各类子进程创建方式查询、空结果、生命周期 | TC-001~009 |
| 安全测试 | 跨 token 隔离 | TC-014~015 |
| 内存测试 | ASan/Valgrind 验证无泄漏 | TC-016 |
