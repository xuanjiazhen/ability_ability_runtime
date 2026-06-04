# REQ-004 实现总结 — 提供查询应用自身子进程接口

> 日期: 2026-06-06
> 基于: execution_plan.md v1.0 / design.md v1.0 / spec.md v1.0

## 变更概览

共变更 **20 个文件**（新增 4 个，修改 16 个），所有新增代码在 `#ifdef SUPPORT_CHILD_PROCESS` 宏保护下。

| # | 层级 | 文件路径 | 变更说明 |
|---|------|---------|---------|
| 1 | 服务端 | `services/appmgr/include/app_mgr_service_inner.h` | +2 方法声明 |
| 2 | 服务端 | `services/appmgr/src/app_mgr_service_inner.cpp` | +2 方法实现（GetChildProcessesForSelf / GetUIAbilityChildProcessesForSelf） |
| 3 | IPC 枚举 | `interfaces/inner_api/app_manager/include/appmgr/app_mgr_ipc_interface_code.h` | +2 枚举值（139/140） |
| 4 | IPC 接口 | `interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h` | +2 虚方法（默认实现） |
| 5 | IPC Proxy | `interfaces/inner_api/app_manager/include/appmgr/app_mgr_proxy.h` | +2 override 声明 |
| 6 | IPC Proxy | `interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.cpp` | +2 IPC 调用实现 |
| 7 | IPC Stub | `interfaces/inner_api/app_manager/include/appmgr/app_mgr_stub.h` | +2 handler 声明 |
| 8 | IPC Stub | `interfaces/inner_api/app_manager/src/appmgr/app_mgr_stub.cpp` | +2 case 分支 + 2 handler 实现 |
| 9 | 客户端 | `interfaces/inner_api/app_manager/include/appmgr/app_mgr_client.h` | +2 包装方法声明 |
| 10 | 客户端 | `interfaces/inner_api/app_manager/src/appmgr/app_mgr_client.cpp` | +2 包装方法实现 |
| 11 | 客户端 | `interfaces/inner_api/child_process_manager/include/child_process_manager.h` | +2 方法声明 |
| 12 | 客户端 | `frameworks/native/ability/native/child_process_manager/child_process_manager.cpp` | +2 方法实现（AppMgrClient 模式） |
| 13 | C API 内部 | `frameworks/native/child_process/include/child_process_info_internal.h` | **新增** 内部结构体定义 + inline CreateAndFillChildProcessInfos |
| 14 | C API 实现 | `frameworks/native/child_process/src/child_process_info.cpp` | **新增** 4 个 getter/Destroy 函数实现 |
| 15 | C API | `frameworks/native/child_process/src/native_child_process.cpp` | 移除重复结构体和 getter/Destroy，使用共享 child_process_info_internal.h |
| 16 | C API | `frameworks/c/ability_runtime/src/application_context.cpp` | +OH_AbilityRuntime_GetUIAbilityChildProcesses 实现（AppMgrClient 直调） |
| 17 | C API 头文件 | `interfaces/kits/c/.../child_process/child_process_info.h` | **新增** 共享公共头文件（不透明类型 + getter/Destroy 声明） |
| 18 | C API 头文件 | `interfaces/kits/c/.../child_process/native_child_process.h` | include child_process_info.h，移除已迁移声明 |
| 19 | C API 头文件 | `interfaces/kits/c/ability_runtime/application_context.h` | include child_process_info.h，+1 函数声明 |
| 20 | C API 构建 | `frameworks/native/child_process/BUILD.gn` | +child_process_info.cpp 源文件 |
| 21 | C API 构建 | `frameworks/c/ability_runtime/BUILD.gn` | +SUPPORT_CHILD_PROCESS 宏和 include 路径 |
| 22 | SDK 头文件 | `interface_sdk_c/.../child_process/child_process_info.h` | **新增** SDK 版本公共头文件 |
| 23 | SDK 头文件 | `interface_sdk_c/.../application_context.h` | 更新 include 路径 |

## 实现要点

### 服务端
- **GetChildProcessesForSelf**：通过 `childProcessRecordMap_`（`shared_ptr`）遍历，获取锁保护副本后迭代
- **GetUIAbilityChildProcessesForSelf**：通过 `childAppRecordMap_`（`weak_ptr`）遍历，`lock()` 检查存活
- 安全：`IPCSkeleton::GetCallingPid()` + `GetCallingTokenID()` 双重校验
- 仅填充 pid/hostPid/processName，其余字段保持默认值

### IPC 层
- 新增枚举码 `GET_CHILD_PROCESSES_FOR_SELF = 139`、`GET_UIABILITY_CHILD_PROCESSES_FOR_SELF = 140`
- 复用 `GetParcelableInfos<ChildProcessInfo>` 进行序列化/反序列化
- Stub handler 遵循 `HandleGetAllChildrenProcesses` 模式

### 客户端
- 使用 `DelayedSingleton<AppMgrClient>::GetInstance()` 模式
- `AppMgrClient` 新增 `GetChildProcessesForSelf` / `GetUIAbilityChildProcessesForSelf` 包装方法

### C API 层
- **共享公共头文件 `child_process_info.h`**：声明不透明类型 `Ability_ChildProcessInfos` + 3 个 getter（返回 `int32_t`）+ 1 个 Destroy，被 `native_child_process.h` 和 `application_context.h` 同时 include
- **内部头文件 `child_process_info_internal.h`**：定义 `ChildProcessInfoItem` 和 `Ability_ChildProcessInfos` 结构体，提供 inline `CreateAndFillChildProcessInfos()` 辅助函数（避免跨 .so 符号导出问题）
- **实现分布**：
  - `native_child_process.cpp`（libchild_process.so）：`OH_Ability_GetChildProcesses`，通过 `ChildProcessManager` 调用 IPC
  - `application_context.cpp`（libability_runtime.so）：`OH_AbilityRuntime_GetUIAbilityChildProcesses`，直接使用 `AppMgrClient` 调用 IPC
  - `child_process_info.cpp`（libchild_process.so）：4 个 getter/Destroy 函数
- 混合分配器：`new[]` 数组 + `strdup()` 字符串，`OH_Ability_DestroyChildProcessInfos` 统一释放
- `strdup` 失败回滚：释放已分配的所有 processName + items + 外壳对象
- 3 个 getter 函数：GetPid / GetHostPid / GetProcessName，带 nullptr 和越界检查
- `OH_AbilityRuntime_GetUIAbilityChildProcesses` 返回 `AbilityRuntime_ErrorCode`，遵循 application_context.h 规范

## AC 覆盖映射

| AC | 实现位置 |
|----|---------|
| AC-1.1~1.4 | app_mgr_service_inner.cpp GetChildProcessesForSelf |
| AC-1.5 | 天然排除（SELF_FORK 不走 AppMgr） |
| AC-2.1~2.2 | app_mgr_service_inner.cpp GetUIAbilityChildProcessesForSelf（weak_ptr.lock 检查） |
| AC-3.1~3.2 | 服务端遍历 map 副本 + child_process_info_internal.h CreateAndFillChildProcessInfos（空结果返回 nullptr） |
| AC-4.1 | native_child_process.cpp 参数校验（GetChildProcesses） |
| AC-4.2 | application_context.cpp 参数校验（GetUIAbilityChildProcesses） |
| AC-4.3 | native_child_process.cpp / application_context.cpp IPC 错误处理 + child_process_manager.cpp |
| AC-5.1~5.2 | 服务端 accessTokenId 校验 |
| AC-6.1~6.2 | child_process_info.cpp Destroy 函数 |
| AC-6.3 | child_process_info_internal.h CreateAndFillChildProcessInfos 回滚逻辑 |
