# REQ-004 提供查询应用自身子进程接口 — 设计文档

> 版本: v1.0
> 日期: 2026-06-03
> 作者: @SKY2001
> 需求跟踪: pilots/ability/reqs/REQ-004.md

## 1. 需求概述

提供查询应用自身子进程的 C 接口，包括两类子进程：

1. **UIAbility 子进程**：通过 `startAbility` 启动，`startOptions` 中 `ProcessMode` 指定 `NEW_PROCESS_ATTACH_TO_PARENT` 创建
2. **不带页面的子进程**：通过以下接口创建：
   - `OH_Ability_CreateNativeChildProcess`
   - `OH_Ability_CreateNativeChildProcessWithConfigs`
   - `OH_Ability_StartNativeChildProcess`
   - `OH_Ability_StartNativeChildProcessWithConfigs`
   - `childProcessManager.startChildProcess`（不包括 SELF_FORK 模式）
   - `childProcessManager.startArkChildProcess`
   - `childProcessManager.startNativeChildProcess`

**接口方案**：两个接口，分别查询两类子进程。

**返回信息**：pid、hostPid、processName。

**调用方式**：同步。

**权限**：同 token 校验，无需额外权限。

## 2. 架构设计

```
┌─────────── 共享公共头 (child_process_info.h) ────────────────────────────┐
│  Ability_ChildProcessInfos 不透明类型前向声明                             │
│  OH_Ability_ChildProcessInfos_GetPid / GetHostPid / GetProcessName      │
│  OH_Ability_DestroyChildProcessInfos                                    │
└─────────────────────────────────────────────────────────────────────────┘
         ↑ include                    ↑ include
┌─── native_child_process.h ───┐  ┌─── application_context.h ───────────┐
│  OH_Ability_GetChildProcesses│  │  OH_AbilityRuntime_                 │
│     ← 不带页面的子进程       │  │  GetUIAbilityChildProcesses         │
└──────────────┬───────────────┘  │     ← UIAbility 子进程             │
               │                  └──────────────┬──────────────────────┘
               │                                  │
┌──────────────┴────── C API 实现 ────────────────┴──────────────────────┐
│  native_child_process.cpp:                                              │
│    OH_Ability_GetChildProcesses → ChildProcessManager.GetChildProcesses │
│  application_context.cpp:                                               │
│    OH_AbilityRuntime_GetUIAbilityChildProcesses                        │
│      → AppMgrClient.GetUIAbilityChildProcessesForSelf                  │
│  child_process_info.cpp:                                                │
│    GetPid / GetHostPid / GetProcessName / Destroy 实现                  │
│  child_process_info_internal.h (inline):                                │
│    CreateAndFillChildProcessInfos 创建辅助函数                           │
└─────────────────────────────────────────────────────────────────────────┘
               │                                  │
┌──────────────┴─── Client 层 ───────────────────┴───────────────────────┐
│  ChildProcessManager.GetChildProcesses()                                │
│  ChildProcessManager.GetUIAbilityChildProcesses()                      │
│  AppMgrClient.GetUIAbilityChildProcessesForSelf()                      │
└────────────────────────────┬────────────────────────────────────────────┘
                             │  IPC (AppMgrProxy → AppMgrStub)
┌─────────────── AppMgrServiceInner (服务端) ──────────────────────────────┐
│  GetChildProcessesForSelf()                                              │
│    → 查询 AppRunningRecord::childProcessRecordMap_                      │
│  GetUIAbilityChildProcessesForSelf()                                     │
│    → 查询 AppRunningRecord::childAppRecordMap_                          │
└──────────────────────────────────────────────────────────────────────────┘
```

### 关键数据结构映射

| 子进程类型 | 服务端存储位置 | 数据结构 |
|-----------|--------------|---------|
| 不带页面的子进程 | `AppRunningRecord::childProcessRecordMap_` | `map<pid_t, shared_ptr<ChildProcessRecord>>` |
| UIAbility 子进程 | `AppRunningRecord::childAppRecordMap_` | `map<pid_t, weak_ptr<AppRunningRecord>>` |

### SELF_FORK 排除说明

SELF_FORK 模式的子进程由客户端直接 `fork()`，不走 AppMgr 服务端，不在 `childProcessRecordMap_` 中，因此天然被排除，无需额外过滤。

## 3. C API 定义

声明分布在三个公共头文件中：

| 头文件 | 声明内容 |
|-------|---------|
| `interfaces/kits/c/ability/ability_runtime/child_process/child_process_info.h` | 不透明类型 `Ability_ChildProcessInfos` + 3 个 getter + 1 个 Destroy 函数 |
| `interfaces/kits/c/ability/ability_runtime/child_process/native_child_process.h` | `OH_Ability_GetChildProcesses`（查询不带页面的子进程） |
| `interfaces/kits/c/ability_runtime/application_context.h` | `OH_AbilityRuntime_GetUIAbilityChildProcesses`（查询 UIAbility 子进程） |

`native_child_process.h` 和 `application_context.h` 均 `#include "child_process_info.h"` 获取共享类型。

### 3.1 数据结构（不透明指针）

声明于 `child_process_info.h`。采用与 `Ability_ChildProcessConfigs` 一致的不透明指针模式，结构体定义仅在内部头文件 `child_process_info_internal.h` 中暴露，公共头文件只做前向声明：

```c
/**
 * @brief 子进程信息集合（不透明类型）。
 *        通过 OH_Ability_GetChildProcesses / OH_AbilityRuntime_GetUIAbilityChildProcesses 获取，
 *        通过 OH_Ability_ChildProcessInfos_GetXxx 系列函数读取属性，
 *        通过 OH_Ability_DestroyChildProcessInfos 释放。
 * @since TBD
 */
typedef struct Ability_ChildProcessInfos Ability_ChildProcessInfos;
```

**为什么不暴露结构体字段**：

1. **封装实现细节**：内部使用混合分配器（`new[]` + `strdup()`），调用者无需关心内存分配策略，未来可自由切换分配方案。
2. **同仓惯例**：`Ability_ChildProcessConfigs` 等同仓 C API 均采用不透明指针模式，保持一致性。
3. **ABI 稳定**：结构体字段变更不会破坏二进制兼容性。
4. **安全性**：调用者无法直接修改返回数据或误用指针字段。

### 3.2 接口1：查询不带页面的子进程

声明于 `native_child_process.h`。

```c
/**
 * @brief 查询当前进程的不带页面的子进程列表。
 *
 * 包括通过以下方式创建的子进程：
 *   - OH_Ability_CreateNativeChildProcess / OH_Ability_CreateNativeChildProcessWithConfigs
 *   - OH_Ability_StartNativeChildProcess / OH_Ability_StartNativeChildProcessWithConfigs
 *   - childProcessManager.startChildProcess（非 SELF_FORK 模式）
 *   - childProcessManager.startArkChildProcess
 *   - childProcessManager.startNativeChildProcess
 *
 * @param infos 输出参数，指向子进程信息集合的指针。由 API 内部分配，
 *              调用者需通过 OH_Ability_DestroyChildProcessInfos 释放。
 *              无子进程时 *infos 为 nullptr。不能为 nullptr。
 * @param count 输出参数，返回子进程数量。不能为 nullptr。
 * @return Returns {@link NCP_NO_ERROR} if the operation is successful.
 *         Returns {@link NCP_ERR_INVALID_PARAM} if the input parameters are invalid.
 *         Returns {@link NCP_ERR_INTERNAL} if an internal error occurs.
 * @since TBD
 */
Ability_NativeChildProcess_ErrCode OH_Ability_GetChildProcesses(
    Ability_ChildProcessInfos** infos,
    uint32_t* count);
```

### 3.3 接口2：查询 UIAbility 子进程

声明于 `interfaces/kits/c/ability_runtime/application_context.h`，遵循该文件的接口格式规范。

```c
/**
 * @brief 查询当前进程的 UIAbility 子进程列表。
 *
 * 包括通过 startAbility 启动，且 startOptions 中 ProcessMode
 * 指定为 NEW_PROCESS_ATTACH_TO_PARENT 创建的子进程。
 *
 * @param infos 输出参数，指向子进程信息集合的指针。由 API 内部分配，
 *      调用者需通过 {@link OH_Ability_DestroyChildProcessInfos} 释放。
 *      无子进程时 *infos 为 nullptr。不能为 nullptr。
 * @param count 输出参数，返回子进程数量。不能为 nullptr。
 * @return One of the following execution results:
 *      **ABILITY_RUNTIME_ERROR_CODE_NO_ERROR**: The operation is successful.
 *      **ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID**: The passed-in value of **infos**
 *      or **count** is null.
 *      **ABILITY_RUNTIME_ERROR_CODE_INTERNAL**: An internal error occurs.
 * @since TBD
 */
AbilityRuntime_ErrorCode OH_AbilityRuntime_GetUIAbilityChildProcesses(
    Ability_ChildProcessInfos** infos,
    uint32_t* count);
```

### 3.4 属性获取函数

声明于 `child_process_info.h`。返回类型使用 `int32_t`（0 = 成功，401 = 参数无效），避免对 `Ability_NativeChildProcess_ErrCode` 枚举的依赖。

```c
/**
 * @brief 获取指定索引处子进程的 PID。
 *
 * @param infos 指向子进程信息集合的指针。不能为 nullptr。
 * @param index 子进程索引，取值范围 [0, count)。
 * @param pid 输出参数，返回子进程 PID。不能为 nullptr。
 * @return Returns 0 if the operation is successful.
 *         Returns 401 if the input parameters are invalid.
 * @since TBD
 */
int32_t OH_Ability_ChildProcessInfos_GetPid(
    Ability_ChildProcessInfos* infos, uint32_t index, int32_t* pid);

/**
 * @brief 获取指定索引处子进程的父进程 PID。
 *
 * @param infos 指向子进程信息集合的指针。不能为 nullptr。
 * @param index 子进程索引，取值范围 [0, count)。
 * @param hostPid 输出参数，返回父进程 PID。不能为 nullptr。
 * @return Returns 0 if the operation is successful.
 *         Returns 401 if the input parameters are invalid.
 * @since TBD
 */
int32_t OH_Ability_ChildProcessInfos_GetHostPid(
    Ability_ChildProcessInfos* infos, uint32_t index, int32_t* hostPid);

/**
 * @brief 获取指定索引处子进程的进程名。
 *
 * 返回的指针在调用 OH_Ability_DestroyChildProcessInfos 前有效，调用者不应释放该指针。
 *
 * @param infos 指向子进程信息集合的指针。不能为 nullptr。
 * @param index 子进程索引，取值范围 [0, count)。
 * @param processName 输出参数，返回进程名字符串指针。不能为 nullptr。
 * @return Returns 0 if the operation is successful.
 *         Returns 401 if the input parameters are invalid.
 * @since TBD
 */
int32_t OH_Ability_ChildProcessInfos_GetProcessName(
    Ability_ChildProcessInfos* infos, uint32_t index, const char** processName);
```

### 3.5 释放函数

声明于 `child_process_info.h`。

```c
/**
 * @brief 释放子进程信息集合。
 *
 * @param infos 指向由 OH_Ability_GetChildProcesses 或
 *              OH_AbilityRuntime_GetUIAbilityChildProcesses 分配的信息集合。
 *              传入 nullptr 时忽略。
 * @since TBD
 */
void OH_Ability_DestroyChildProcessInfos(Ability_ChildProcessInfos* infos);
```

**为什么不暴露结构体而使用不透明指针 + getter**：

1. **封装实现细节**：内部使用混合分配器（`new[]` + `strdup()`），调用者无需关心内存分配策略，未来可自由切换分配方案。
2. **同仓惯例**：`Ability_ChildProcessConfigs` 等同仓 C API 均采用不透明指针 + getter/setter 模式，保持一致性。
3. **ABI 稳定**：内部结构体字段变更不会影响已编译的调用者代码。
4. **安全性**：调用者无法直接修改返回数据或误操作内部指针字段。

### 3.6 错误码

查询接口复用各自所在头文件的现有错误码：

| 函数 | 错误码类型 | 来源 |
|-----|-----------|------|
| `OH_Ability_GetChildProcesses` | `Ability_NativeChildProcess_ErrCode`（`native_child_process.h`） | NCP_NO_ERROR / NCP_ERR_INVALID_PARAM / NCP_ERR_INTERNAL |
| `OH_AbilityRuntime_GetUIAbilityChildProcesses` | `AbilityRuntime_ErrorCode`（`ability_runtime_common.h`） | ABILITY_RUNTIME_ERROR_CODE_NO_ERROR / PARAM_INVALID / INTERNAL |
| getter / Destroy | `int32_t`（`child_process_info.h`） | 0 / 401 |

## 4. 服务端实现

### 4.1 GetChildProcessesForSelf

**位置**：`services/appmgr/src/app_mgr_service_inner.cpp`

**伪代码**：

```cpp
int32_t AppMgrServiceInner::GetChildProcessesForSelf(std::vector<ChildProcessInfo>& infos)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(callingPid);
    if (appRecord == nullptr) {
        return ERR_INVALID_VALUE;
    }
    // 同 token 校验
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (applicationInfo == nullptr || callingTokenId != applicationInfo->accessTokenId) {
        return ERR_INVALID_VALUE;
    }
    // 遍历 childProcessRecordMap_
    auto childProcessRecordMap = appRecord->GetChildProcessRecordMap();
    for (const auto& iter : childProcessRecordMap) {
        auto childRecord = iter.second;
        if (childRecord == nullptr) {
            continue;
        }
        ChildProcessInfo info;
        info.pid = childRecord->GetPid();
        info.hostPid = callingPid;
        info.processName = childRecord->GetProcessName();
        infos.emplace_back(info);
    }
    return ERR_OK;
}
```

**数据来源**：`AppRunningRecord::childProcessRecordMap_`

### 4.2 GetUIAbilityChildProcessesForSelf

**位置**：`services/appmgr/src/app_mgr_service_inner.cpp`

**伪代码**：

```cpp
int32_t AppMgrServiceInner::GetUIAbilityChildProcessesForSelf(std::vector<ChildProcessInfo>& infos)
{
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    auto appRecord = GetAppRunningRecordByPid(callingPid);
    if (appRecord == nullptr) {
        return ERR_INVALID_VALUE;
    }
    // 同 token 校验
    auto callingTokenId = IPCSkeleton::GetCallingTokenID();
    auto applicationInfo = appRecord->GetApplicationInfo();
    if (applicationInfo == nullptr || callingTokenId != applicationInfo->accessTokenId) {
        return ERR_INVALID_VALUE;
    }
    // 遍历 childAppRecordMap_（UIAbility 子进程）
    auto childAppRecordMap = appRecord->GetChildAppRecordMap();
    for (const auto& iter : childAppRecordMap) {
        auto childAppRecord = iter.second.lock();
        if (childAppRecord == nullptr) {
            continue;  // weak_ptr 已失效，子进程已退出
        }
        ChildProcessInfo info;
        info.pid = childAppRecord->GetPid();
        info.hostPid = callingPid;
        info.processName = childAppRecord->GetProcessName();
        infos.emplace_back(info);
    }
    return ERR_OK;
}
```

**数据来源**：`AppRunningRecord::childAppRecordMap_`

**注意事项**：
- `childAppRecordMap_` 存储 `weak_ptr<AppRunningRecord>`，必须先 `lock()` 检查存活
- UIAbility 子进程的 hostPid 取自调用者 PID（即父进程 PID）

## 5. IPC 层

### 5.1 IAppMgr 接口新增

**文件**：`interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h`

```cpp
#ifdef SUPPORT_CHILD_PROCESS
virtual int GetChildProcessesForSelf(std::vector<ChildProcessInfo>& infos) = 0;
virtual int GetUIAbilityChildProcessesForSelf(std::vector<ChildProcessInfo>& infos) = 0;
#endif
```

### 5.2 AppMgrProxy

**文件**：`interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.cpp`

新增两个方法，通过 IPC `SendRequest` 调用服务端，序列化/反序列化使用 `ChildProcessInfo`（已有 parcelable 支持）。

### 5.3 AppMgrStub

**文件**：`services/appmgr/src/app_mgr_stub.cpp`

新增两个方法的 IPC 消息分发。

### 5.4 IPC 传输复用

复用现有 `ChildProcessInfo`（parcelable），仅填充 pid/hostPid/processName 三个字段，其余字段保持默认值。C API 层只暴露这三个字段。

## 6. 客户端实现

### 6.1 ChildProcessManager

**文件**：`frameworks/native/ability/native/child_process_manager/child_process_manager.cpp`

```cpp
int ChildProcessManager::GetChildProcesses(std::vector<ChildProcessInfo>& infos)
{
    auto appMgr = AppMgrClient::GetInstance()->GetAppMgrProxy();
    if (appMgr == nullptr) {
        return ERR_GET_APP_MGR_FAILED;
    }
    return appMgr->GetChildProcessesForSelf(infos);
}

int ChildProcessManager::GetUIAbilityChildProcesses(std::vector<ChildProcessInfo>& infos)
{
    auto appMgr = AppMgrClient::GetInstance()->GetAppMgrProxy();
    if (appMgr == nullptr) {
        return ERR_GET_APP_MGR_FAILED;
    }
    return appMgr->GetUIAbilityChildProcessesForSelf(infos);
}
```

### 6.2 内部结构体与共享创建函数

**文件**：`frameworks/native/child_process/include/child_process_info_internal.h`

内部头文件定义结构体实际布局和 inline 创建辅助函数，供 `libchild_process.so` 和 `libability_runtime.so` 共用：

```cpp
struct ChildProcessInfoItem {
    int32_t pid;
    int32_t hostPid;
    char* processName;  // strdup'd，随 Ability_ChildProcessInfos 一起释放
};

struct Ability_ChildProcessInfos {
    ChildProcessInfoItem* items;
    uint32_t count;
};

inline bool CreateAndFillChildProcessInfos(
    const std::vector<OHOS::AppExecFwk::ChildProcessInfo>& childInfos,
    Ability_ChildProcessInfos** infos, uint32_t* count)
{
    *count = static_cast<uint32_t>(childInfos.size());
    if (childInfos.empty()) {
        *infos = nullptr;
        return true;
    }
    auto* result = new (std::nothrow) Ability_ChildProcessInfos();
    // ... 分配 items 数组，strdup processName，失败时回滚
    *infos = result;
    return true;
}
```

**为什么使用 `inline`**：`CreateAndFillChildProcessInfos` 被 `libchild_process.so` 和 `libability_runtime.so` 两个共享库调用。若声明为 `static`，编译器会在每个编译单元产生独立的副本，触发 `-Wunused-function` 警告；若声明为普通函数，version_script 过滤非 `OH_*` 符号导致链接失败。`inline` 让每个 .so 内含独立副本且无符号导出问题。

### 6.3 C API 实现 — libchild_process.so

**文件**：`frameworks/native/child_process/src/native_child_process.cpp`

```cpp
#include "child_process_info_internal.h"

Ability_NativeChildProcess_ErrCode OH_Ability_GetChildProcesses(
    Ability_ChildProcessInfos** infos, uint32_t* count)
{
    if (infos == nullptr || count == nullptr) {
        return NCP_ERR_INVALID_PARAM;
    }
    std::vector<ChildProcessInfo> childInfos;
    int ret = ChildProcessManager::GetInstance()->GetChildProcesses(childInfos);
    if (ret != ERR_OK) {
        return NCP_ERR_INTERNAL;
    }
    if (!CreateAndFillChildProcessInfos(childInfos, infos, count)) {
        return NCP_ERR_INTERNAL;
    }
    return NCP_NO_ERROR;
}
```

### 6.4 C API 实现 — libability_runtime.so

**文件**：`frameworks/c/ability_runtime/src/application_context.cpp`

```cpp
#ifdef SUPPORT_CHILD_PROCESS
#include "child_process_info_internal.h"
#include "app_mgr_client.h"

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetUIAbilityChildProcesses(
    Ability_ChildProcessInfos** infos, uint32_t* count)
{
    if (infos == nullptr || count == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::vector<OHOS::AppExecFwk::ChildProcessInfo> childInfos;
    int ret = OHOS::AppExecFwk::AppMgrClient::GetInstance()
        ->GetUIAbilityChildProcessesForSelf(childInfos);
    if (ret != ERR_OK) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    if (!CreateAndFillChildProcessInfos(childInfos, infos, count)) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
#endif
```

**为什么不经过 ChildProcessManager**：`ChildProcessManager` 头文件包含 `runtime.h` 等仅在 `libchild_process.so` 编译环境中可用的依赖。`application_context.cpp` 编译进 `libability_runtime.so`，直接使用 `AppMgrClient` 避免引入不必要依赖。

### 6.5 getter / Destroy 实现

**文件**：`frameworks/native/child_process/src/child_process_info.cpp`

编译进 `libchild_process.so`，声明于 `child_process_info.h`。

```cpp
int32_t OH_Ability_ChildProcessInfos_GetPid(
    Ability_ChildProcessInfos* infos, uint32_t index, int32_t* pid)
{
    if (infos == nullptr || pid == nullptr || index >= infos->count) {
        return 401;
    }
    *pid = infos->items[index].pid;
    return 0;
}

// GetHostPid / GetProcessName 同理，返回 int32_t (0 / 401)

void OH_Ability_DestroyChildProcessInfos(Ability_ChildProcessInfos* infos)
{
    if (infos == nullptr) {
        return;
    }
    for (uint32_t i = 0; i < infos->count; i++) {
        free(infos->items[i].processName);
    }
    delete[] infos->items;
    delete infos;
}
```

## 7. 修改文件清单

| 层级 | 文件路径 | 操作 | 说明 |
|------|---------|------|------|
| C API（公共） | `interfaces/kits/c/ability/ability_runtime/child_process/child_process_info.h` | 新增 | 不透明类型 `Ability_ChildProcessInfos` 前向声明 + 3 个 getter + 1 个 Destroy 函数声明 |
| C API（公共） | `interfaces/kits/c/ability/ability_runtime/child_process/native_child_process.h` | 修改 | include `child_process_info.h`，移除已迁移的类型声明和 getter/Destroy 声明，保留 `OH_Ability_GetChildProcesses` |
| C API（公共） | `interfaces/kits/c/ability_runtime/application_context.h` | 修改 | include `child_process_info.h`，新增 `OH_AbilityRuntime_GetUIAbilityChildProcesses` 函数声明 |
| C API（内部） | `frameworks/native/child_process/include/child_process_info_internal.h` | 新增 | 内部结构体定义（`ChildProcessInfoItem`、`Ability_ChildProcessInfos`）+ inline `CreateAndFillChildProcessInfos` 创建辅助函数 |
| C API（实现） | `frameworks/native/child_process/src/child_process_info.cpp` | 新增 | 4 个 getter/Destroy 函数实现 |
| C API（实现） | `frameworks/native/child_process/src/native_child_process.cpp` | 修改 | 移除重复的内部结构体和 getter/Destroy 实现，使用共享的 `child_process_info_internal.h` |
| C API（实现） | `frameworks/c/ability_runtime/src/application_context.cpp` | 修改 | 新增 `OH_AbilityRuntime_GetUIAbilityChildProcesses` 实现，使用共享的 `child_process_info_internal.h` |
| C API（构建） | `frameworks/native/child_process/BUILD.gn` | 修改 | 新增 `child_process_info.cpp` 源文件 |
| C API（构建） | `frameworks/c/ability_runtime/BUILD.gn` | 修改 | 新增 `SUPPORT_CHILD_PROCESS` 宏和 internal header include 路径 |
| IPC 接口 | `interfaces/inner_api/app_manager/include/appmgr/app_mgr_interface.h` | 修改 | 新增 2 个虚方法 |
| IPC Proxy | `interfaces/inner_api/app_manager/src/appmgr/app_mgr_proxy.cpp` | 修改 | 新增 2 个 IPC 调用实现 |
| IPC Stub | `services/appmgr/src/app_mgr_stub.cpp` / `.h` | 修改 | 新增 2 个 IPC 消息分发 |
| 服务端 | `services/appmgr/include/app_mgr_service_inner.h` | 修改 | 新增 2 个方法声明 |
| 服务端 | `services/appmgr/src/app_mgr_service_inner.cpp` | 修改 | 新增 2 个方法实现 |
| 客户端 | `interfaces/inner_api/child_process_manager/include/child_process_manager.h` | 修改 | 新增 2 个方法声明 |
| 客户端 | `frameworks/native/ability/native/child_process_manager/child_process_manager.cpp` | 修改 | 新增 2 个方法实现 |

所有新增代码均在 `#ifdef SUPPORT_CHILD_PROCESS` 宏保护下。

## 8. 安全性分析

| 安全维度 | 设计考量 |
|---------|---------|
| 权限控制 | 同 token 校验（accessTokenId 比对），无需额外系统权限 |
| 越权防护 | 仅返回调用者自身进程的子进程，不暴露其他应用信息 |
| 内存安全 | C API 采用不透明指针模式，调用者通过 getter 读取属性、通过 Destroy 释放，无法直接操作内部内存 |
| IPC 安全 | 复用现有 IPC 框架的调用者身份验证（CallingPid/CallingTokenID） |
| 线程安全 | 查询接口本身无全局可变状态（通过 IPC 调用服务端），可在多线程并发调用。返回的内存归调用者所有，不与其他调用共享 |

## 8.1 内存所有权模型

```
调用者                                          API 内部
──────                                          ────────
  Ability_ChildProcessInfos* infos = nullptr;
  OH_Ability_GetChildProcesses(&infos, &count)
    ─────────────────────────────────────────→ new Ability_ChildProcessInfos { items, count }
                                              new[] ChildProcessInfoItem[count]
                                              strdup() 分配每个 processName
                                              *infos = result
  |
  ├── OH_Ability_ChildProcessInfos_GetPid(infos, i, &pid)
  │     → 值拷贝到 pid，无需释放
  ├── OH_Ability_ChildProcessInfos_GetHostPid(infos, i, &hostPid)
  │     → 值拷贝到 hostPid，无需释放
  ├── OH_Ability_ChildProcessInfos_GetProcessName(infos, i, &name)
  │     → 返回内部指针，Destroy 前有效，调用者不应释放
  └── ...
  OH_Ability_DestroyChildProcessInfos(infos) ──→ API 释放所有内存（items + processName + 外壳）
```

## 9. 使用示例

```c
#include "child_process_info.h"         // getter / Destroy 函数
#include "native_child_process.h"       // OH_Ability_GetChildProcesses
#include "application_context.h"        // OH_AbilityRuntime_GetUIAbilityChildProcesses

void QueryChildProcesses() {
    Ability_ChildProcessInfos* infos = nullptr;
    uint32_t count = 0;

    // 查询不带页面的子进程
    int ret = OH_Ability_GetChildProcesses(&infos, &count);
    if (ret == NCP_NO_ERROR) {
        for (uint32_t i = 0; i < count; i++) {
            int32_t pid = 0;
            int32_t hostPid = 0;
            const char* processName = nullptr;
            OH_Ability_ChildProcessInfos_GetPid(infos, i, &pid);
            OH_Ability_ChildProcessInfos_GetHostPid(infos, i, &hostPid);
            OH_Ability_ChildProcessInfos_GetProcessName(infos, i, &processName);
            printf("Child[%d]: pid=%d, hostPid=%d, name=%s\n",
                i, pid, hostPid, processName);
        }
        OH_Ability_DestroyChildProcessInfos(infos);
    }

    // 查询 UIAbility 子进程（声明在 application_context.h）
    AbilityRuntime_ErrorCode ret2 = OH_AbilityRuntime_GetUIAbilityChildProcesses(&infos, &count);
    if (ret2 == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        for (uint32_t i = 0; i < count; i++) {
            int32_t pid = 0;
            int32_t hostPid = 0;
            const char* processName = nullptr;
            OH_Ability_ChildProcessInfos_GetPid(infos, i, &pid);
            OH_Ability_ChildProcessInfos_GetHostPid(infos, i, &hostPid);
            OH_Ability_ChildProcessInfos_GetProcessName(infos, i, &processName);
            printf("UIAbilityChild[%d]: pid=%d, hostPid=%d, name=%s\n",
                i, pid, hostPid, processName);
        }
        OH_Ability_DestroyChildProcessInfos(infos);
    }
}
```

## 10. 验收测试要点

1. 调用 `OH_Ability_CreateNativeChildProcess` 创建子进程后，`OH_Ability_GetChildProcesses` 返回 `NCP_NO_ERROR` 且 `count >= 1`
2. 通过 `startAbility` + `NEW_PROCESS_ATTACH_TO_PARENT` 启动 UIAbility 子进程后，`OH_Ability_GetUIAbilityChildProcesses` 返回 `NCP_NO_ERROR` 且 `count >= 1`
3. 子进程退出后，查询结果的 `count` 不再包含该子进程
4. 无子进程时，返回 `NCP_NO_ERROR`，`count = 0`，`*infos = nullptr`
5. `infos` 或 `count` 为 nullptr 时，返回 `NCP_ERR_INVALID_PARAM`
6. 不同 token 的进程无法查到其他应用的子进程
7. SELF_FORK 模式的子进程不出现在查询结果中
