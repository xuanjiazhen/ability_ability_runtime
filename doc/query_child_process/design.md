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
┌───────────────────── C API 层 (native_child_process.h) ──────────────────┐
│  OH_Ability_GetChildProcesses()               ← 不带页面的子进程         │
│  OH_Ability_GetUIAbilityChildProcesses()      ← UIAbility 子进程         │
│  OH_Ability_DestroyChildProcessInfos()         ← 释放内存                │
└────────────────────────────┬──────────────────────────────────────────────┘
                             │
┌─────────────── C API 实现 (native_child_process.cpp) ────────────────────┐
│  调用 ChildProcessManager 单例                                           │
└────────────────────────────┬──────────────────────────────────────────────┘
                             │
┌─────────────── Client ChildProcessManager ───────────────────────────────┐
│  GetChildProcesses()                                                     │
│  GetUIAbilityChildProcesses()                                            │
└────────────────────────────┬──────────────────────────────────────────────┘
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

新增到 `interfaces/kits/c/ability/ability_runtime/child_process/native_child_process.h`。

### 3.1 数据结构（不透明指针）

采用与 `Ability_ChildProcessConfigs` 一致的不透明指针模式，结构体定义仅在 .cpp 内部暴露，头文件只做前向声明：

```c
/**
 * @brief 子进程信息集合（不透明类型）。
 *        通过 OH_Ability_GetChildProcesses / OH_Ability_GetUIAbilityChildProcesses 获取，
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

```c
/**
 * @brief 获取指定索引处子进程的 PID。
 *
 * @param infos 指向子进程信息集合的指针。不能为 nullptr。
 * @param index 子进程索引，取值范围 [0, count)。
 * @param pid 输出参数，返回子进程 PID。不能为 nullptr。
 * @return Returns {@link NCP_NO_ERROR} if the operation is successful.
 *         Returns {@link NCP_ERR_INVALID_PARAM} if the input parameters are invalid.
 * @since TBD
 */
Ability_NativeChildProcess_ErrCode OH_Ability_ChildProcessInfos_GetPid(
    Ability_ChildProcessInfos* infos, uint32_t index, int32_t* pid);

/**
 * @brief 获取指定索引处子进程的父进程 PID。
 *
 * @param infos 指向子进程信息集合的指针。不能为 nullptr。
 * @param index 子进程索引，取值范围 [0, count)。
 * @param hostPid 输出参数，返回父进程 PID。不能为 nullptr。
 * @return Returns {@link NCP_NO_ERROR} if the operation is successful.
 *         Returns {@link NCP_ERR_INVALID_PARAM} if the input parameters are invalid.
 * @since TBD
 */
Ability_NativeChildProcess_ErrCode OH_Ability_ChildProcessInfos_GetHostPid(
    Ability_ChildProcessInfos* infos, uint32_t index, int32_t* hostPid);

/**
 * @brief 获取指定索引处子进程的进程名。
 *
 * 返回的指针在调用 OH_Ability_DestroyChildProcessInfos 前有效，调用者不应释放该指针。
 *
 * @param infos 指向子进程信息集合的指针。不能为 nullptr。
 * @param index 子进程索引，取值范围 [0, count)。
 * @param processName 输出参数，返回进程名字符串指针。不能为 nullptr。
 * @return Returns {@link NCP_NO_ERROR} if the operation is successful.
 *         Returns {@link NCP_ERR_INVALID_PARAM} if the input parameters are invalid.
 * @since TBD
 */
Ability_NativeChildProcess_ErrCode OH_Ability_ChildProcessInfos_GetProcessName(
    Ability_ChildProcessInfos* infos, uint32_t index, const char** processName);
```

### 3.5 释放函数

```c
/**
 * @brief 释放子进程信息集合。
 *
 * @param infos 指向由 OH_Ability_GetChildProcesses 或
 *              OH_Ability_GetUIAbilityChildProcesses 分配的信息集合。
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

复用现有 `Ability_NativeChildProcess_ErrCode`：

| 错误码 | 值 | 说明 |
|-------|-----|------|
| `NCP_NO_ERROR` | 0 | 成功，count=0 表示无子进程 |
| `NCP_ERR_INVALID_PARAM` | 401 | infos 或 count 为 nullptr |
| `NCP_ERR_INTERNAL` | 16000050 | IPC 失败等内部错误 |

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
        return NCP_ERR_SERVICE_ERROR;
    }
    return appMgr->GetChildProcessesForSelf(infos);
}

int ChildProcessManager::GetUIAbilityChildProcesses(std::vector<ChildProcessInfo>& infos)
{
    auto appMgr = AppMgrClient::GetInstance()->GetAppMgrProxy();
    if (appMgr == nullptr) {
        return NCP_ERR_SERVICE_ERROR;
    }
    return appMgr->GetUIAbilityChildProcessesForSelf(infos);
}
```

### 6.2 C API 实现

**文件**：`frameworks/native/child_process/src/native_child_process.cpp`

**内部结构体定义**（仅在 .cpp 内部可见）：

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
```

**查询函数实现**：

```cpp
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
    return CreateAndFillInfos(childInfos, infos, count);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_GetUIAbilityChildProcesses(
    Ability_ChildProcessInfos** infos, uint32_t* count)
{
    if (infos == nullptr || count == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::vector<ChildProcessInfo> childInfos;
    int ret = ChildProcessManager::GetInstance()->GetUIAbilityChildProcesses(childInfos);
    if (ret != ERR_OK) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    return CreateAndFillInfos(childInfos, infos, count);
}
```

**创建并填充辅助函数**：

```cpp
static Ability_NativeChildProcess_ErrCode CreateAndFillInfos(
    const std::vector<ChildProcessInfo>& childInfos,
    Ability_ChildProcessInfos** infos, uint32_t* count)
{
    *count = static_cast<uint32_t>(childInfos.size());
    if (childInfos.empty()) {
        *infos = nullptr;
        return NCP_NO_ERROR;
    }
    auto* result = new (std::nothrow) Ability_ChildProcessInfos();
    if (result == nullptr) {
        return NCP_ERR_INTERNAL;
    }
    result->count = *count;
    result->items = new (std::nothrow) ChildProcessInfoItem[result->count];
    if (result->items == nullptr) {
        delete result;
        *infos = nullptr;
        *count = 0;
        return NCP_ERR_INTERNAL;
    }
    for (uint32_t i = 0; i < result->count; i++) {
        result->items[i].pid = childInfos[i].pid;
        result->items[i].hostPid = childInfos[i].hostPid;
        result->items[i].processName = strdup(childInfos[i].processName.c_str());
        if (result->items[i].processName == nullptr) {
            // strdup 失败，回滚已分配的内存
            for (uint32_t j = 0; j < i; j++) {
                free(result->items[j].processName);
            }
            delete[] result->items;
            delete result;
            *infos = nullptr;
            *count = 0;
            return NCP_ERR_INTERNAL;
        }
    }
    *infos = result;
    return NCP_NO_ERROR;
}
```

**Getter 函数实现**：

```cpp
Ability_NativeChildProcess_ErrCode OH_Ability_ChildProcessInfos_GetPid(
    Ability_ChildProcessInfos* infos, uint32_t index, int32_t* pid)
{
    if (infos == nullptr || pid == nullptr || index >= infos->count) {
        return NCP_ERR_INVALID_PARAM;
    }
    *pid = infos->items[index].pid;
    return NCP_NO_ERROR;
}

Ability_NativeChildProcess_ErrCode OH_Ability_ChildProcessInfos_GetHostPid(
    Ability_ChildProcessInfos* infos, uint32_t index, int32_t* hostPid)
{
    if (infos == nullptr || hostPid == nullptr || index >= infos->count) {
        return NCP_ERR_INVALID_PARAM;
    }
    *hostPid = infos->items[index].hostPid;
    return NCP_NO_ERROR;
}

Ability_NativeChildProcess_ErrCode OH_Ability_ChildProcessInfos_GetProcessName(
    Ability_ChildProcessInfos* infos, uint32_t index, const char** processName)
{
    if (infos == nullptr || processName == nullptr || index >= infos->count) {
        return NCP_ERR_INVALID_PARAM;
    }
    *processName = infos->items[index].processName;
    return NCP_NO_ERROR;
}
```

**释放函数**：

```cpp
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
| C API | `interfaces/kits/c/ability/ability_runtime/child_process/native_child_process.h` | 修改 | 新增不透明类型前向声明 + 5 个函数声明（1 查询 + 3 getter + 1 释放） |
| C API | `interface_sdk_c/AbilityKit/ability_runtime/application_context.h` | 修改 | 新增 `OH_AbilityRuntime_GetUIAbilityChildProcesses` 函数声明 |
| C API | `frameworks/native/child_process/src/native_child_process.cpp` | 修改 | 新增内部结构体 + 6 个函数实现 |
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
#include "native_child_process.h"

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
