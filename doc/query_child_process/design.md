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

### 3.1 数据结构

```c
/**
 * @brief 子进程信息结构体。
 * @since TBD
 */
typedef struct OH_Ability_ChildProcessInfo {
    /** 子进程 PID */
    int32_t pid;
    /** 父进程 PID */
    int32_t hostPid;
    /** 进程名（API 内部分配，随 infos 一起通过 OH_Ability_DestroyChildProcessInfos 释放） */
    char* processName;
} OH_Ability_ChildProcessInfo;
```

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
 * @param infos 输出参数，指向子进程信息数组的指针。由 API 分配内存，
 *              调用者需通过 OH_Ability_DestroyChildProcessInfos 释放。
 *              无子进程时 *infos 为 nullptr。不能为 nullptr。
 * @param count 输出参数，返回子进程数量。不能为 nullptr。
 * @return Returns {@link NCP_NO_ERROR} if the operation is successful.
 *         Returns {@link NCP_ERR_INVALID_PARAM} if the input parameters are invalid.
 *         Returns {@link NCP_ERR_INTERNAL} if an internal error occurs.
 * @since TBD
 */
Ability_NativeChildProcess_ErrCode OH_Ability_GetChildProcesses(
    OH_Ability_ChildProcessInfo** infos,
    uint32_t* count);
```

### 3.3 接口2：查询 UIAbility 子进程

```c
/**
 * @brief 查询当前进程的 UIAbility 子进程列表。
 *
 * 包括通过 startAbility 启动，且 startOptions 中 ProcessMode
 * 指定为 NEW_PROCESS_ATTACH_TO_PARENT 创建的子进程。
 *
 * @param infos 输出参数，指向子进程信息数组的指针。由 API 分配内存，
 *              调用者需通过 OH_Ability_DestroyChildProcessInfos 释放。
 *              无子进程时 *infos 为 nullptr。不能为 nullptr。
 * @param count 输出参数，返回子进程数量。不能为 nullptr。
 * @return Returns {@link NCP_NO_ERROR} if the operation is successful.
 *         Returns {@link NCP_ERR_INVALID_PARAM} if the input parameters are invalid.
 *         Returns {@link NCP_ERR_INTERNAL} if an internal error occurs.
 * @since TBD
 */
Ability_NativeChildProcess_ErrCode OH_Ability_GetUIAbilityChildProcesses(
    OH_Ability_ChildProcessInfo** infos,
    uint32_t* count);
```

### 3.4 释放函数

```c
/**
 * @brief 释放子进程信息数组。
 *
 * @param infos 指向由 OH_Ability_GetChildProcesses 或
 *              OH_Ability_GetUIAbilityChildProcesses 分配的数组。
 *              传入 nullptr 时忽略。
 * @param count 数组元素数量。
 * @since TBD
 */
void OH_Ability_DestroyChildProcessInfos(
    OH_Ability_ChildProcessInfo* infos,
    uint32_t count);
```

**为什么需要专用释放函数（而非让调用者自行 `free`）**：

1. **混合分配器**：内部实现使用 `new[]` 分配数组、`strdup()`（底层 `malloc`）分配字符串。调用者无法安全地知道每个字段该用 `free` 还是 `delete`。
2. **嵌套指针**：`processName` 是 API 内部分配的二级指针，直接 `free(infos)` 会泄漏所有 `processName`。
3. **封装实现细节**：未来如果分配策略变化（如统一改为 `malloc`），调用者无需修改代码。
4. **同仓惯例**：ability_runtime 中涉及嵌套指针的 API（如 `OH_Ability_DestroyChildProcessConfigs`）均提供专用释放函数。

### 3.5 错误码

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

```cpp
Ability_NativeChildProcess_ErrCode OH_Ability_GetChildProcesses(
    OH_Ability_ChildProcessInfo** infos, uint32_t* count)
{
    if (infos == nullptr || count == nullptr) {
        return NCP_ERR_INVALID_PARAM;
    }
    std::vector<ChildProcessInfo> childInfos;
    int ret = ChildProcessManager::GetInstance()->GetChildProcesses(childInfos);
    if (ret != ERR_OK) {
        return NCP_ERR_INTERNAL;
    }
    return ConvertToCInfos(childInfos, infos, count);
}

Ability_NativeChildProcess_ErrCode OH_Ability_GetUIAbilityChildProcesses(
    OH_Ability_ChildProcessInfo** infos, uint32_t* count)
{
    if (infos == nullptr || count == nullptr) {
        return NCP_ERR_INVALID_PARAM;
    }
    std::vector<ChildProcessInfo> childInfos;
    int ret = ChildProcessManager::GetInstance()->GetUIAbilityChildProcesses(childInfos);
    if (ret != ERR_OK) {
        return NCP_ERR_INTERNAL;
    }
    return ConvertToCInfos(childInfos, infos, count);
}
```

转换辅助函数：

```cpp
static Ability_NativeChildProcess_ErrCode ConvertToCInfos(
    const std::vector<ChildProcessInfo>& childInfos,
    OH_Ability_ChildProcessInfo** infos, uint32_t* count)
{
    *count = static_cast<uint32_t>(childInfos.size());
    if (childInfos.empty()) {
        *infos = nullptr;
        return NCP_NO_ERROR;
    }
    *infos = new (std::nothrow) OH_Ability_ChildProcessInfo[*count];
    if (*infos == nullptr) {
        return NCP_ERR_INTERNAL;
    }
    for (uint32_t i = 0; i < *count; i++) {
        (*infos)[i].pid = childInfos[i].pid;
        (*infos)[i].hostPid = childInfos[i].hostPid;
        (*infos)[i].processName = strdup(childInfos[i].processName.c_str());
        if ((*infos)[i].processName == nullptr) {
            // strdup 失败，回滚已分配的内存
            for (uint32_t j = 0; j < i; j++) {
                free((*infos)[j].processName);
            }
            delete[] *infos;
            *infos = nullptr;
            *count = 0;
            return NCP_ERR_INTERNAL;
        }
    }
    return NCP_NO_ERROR;
}
```

释放函数：

```cpp
void OH_Ability_DestroyChildProcessInfos(
    OH_Ability_ChildProcessInfo* infos, uint32_t count)
{
    if (infos == nullptr) {
        return;
    }
    for (uint32_t i = 0; i < count; i++) {
        free(infos[i].processName);
    }
    delete[] infos;
}
```

## 7. 修改文件清单

| 层级 | 文件路径 | 操作 | 说明 |
|------|---------|------|------|
| C API | `interfaces/kits/c/ability/ability_runtime/child_process/native_child_process.h` | 修改 | 新增结构体 + 3 个函数声明 |
| C API | `frameworks/native/child_process/src/native_child_process.cpp` | 修改 | 新增 3 个函数实现 |
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
| 内存安全 | C API 采用 allocate/destroy 模式，调用者负责释放 |
| IPC 安全 | 复用现有 IPC 框架的调用者身份验证（CallingPid/CallingTokenID） |
| 线程安全 | 查询接口本身无全局可变状态（通过 IPC 调用服务端），可在多线程并发调用。返回的内存归调用者所有，不与其他调用共享 |

## 8.1 内存所有权模型

```
调用者                          API 内部
──────                          ────────
                                new[] 分配数组
                                strdup() 分配每个 processName
  infos ──────────────────────→ 调用者获得所有权
  |
  ├── infos[0].pid             (值拷贝，无需释放)
  ├── infos[0].processName ──→ 需通过 Destroy 释放
  ├── infos[1].pid             (值拷贝)
  ├── infos[1].processName ──→ 需通过 Destroy 释放
  └── ...
  OH_Ability_DestroyChildProcessInfos(infos, count) ──→ API 释放所有内存
```

## 9. 使用示例

```c
#include "native_child_process.h"

void QueryChildProcesses() {
    OH_Ability_ChildProcessInfo* infos = nullptr;
    uint32_t count = 0;

    // 查询不带页面的子进程
    int ret = OH_Ability_GetChildProcesses(&infos, &count);
    if (ret == NCP_NO_ERROR) {
        for (uint32_t i = 0; i < count; i++) {
            printf("Child[%d]: pid=%d, hostPid=%d, name=%s\n",
                i, infos[i].pid, infos[i].hostPid, infos[i].processName);
        }
        OH_Ability_DestroyChildProcessInfos(infos, count);
    }

    // 查询 UIAbility 子进程
    ret = OH_Ability_GetUIAbilityChildProcesses(&infos, &count);
    if (ret == NCP_NO_ERROR) {
        for (uint32_t i = 0; i < count; i++) {
            printf("UIAbilityChild[%d]: pid=%d, hostPid=%d, name=%s\n",
                i, infos[i].pid, infos[i].hostPid, infos[i].processName);
        }
        OH_Ability_DestroyChildProcessInfos(infos, count);
    }
}
```

## 10. 验收测试要点

1. 调用 `OH_Ability_CreateNativeChildProcess` 创建子进程后，`OH_Ability_GetChildProcesses` 可查到该子进程
2. 通过 `startAbility` + `NEW_PROCESS_ATTACH_TO_PARENT` 启动 UIAbility 子进程后，`OH_Ability_GetUIAbilityChildProcesses` 可查到
3. 子进程退出后，查询结果不再包含该子进程
4. 无子进程时，返回 `NCP_NO_ERROR`，count=0，infos=nullptr
5. 传入空指针时，返回 `NCP_ERR_INVALID_PARAM`
6. 不同 token 的进程无法查到其他应用的子进程
7. SELF_FORK 模式的子进程不出现在查询结果中
