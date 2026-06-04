# REQ-004 提供查询应用自身子进程接口 — Proposal

> 版本: v1.0
> 日期: 2026-06-03
> 作者: @SKY2001
> 需求跟踪: pilots/ability/reqs/REQ-004.md

## 1. 需求背景

生态需要查询应用自身的子进程。当前 OpenHarmony 提供了多种创建子进程的方式，但缺少一个统一的查询能力，应用无法感知自身创建了哪些子进程。

## 2. 需求描述

提供查询应用自身子进程的能力，包括下面两类：

### 2.1 UIAbility 子进程

通过 `startAbility` 启动，`startOptions` 里的 `ProcessMode` 指定 `NEW_PROCESS_ATTACH_TO_PARENT` 创建的子进程。

### 2.2 不带页面的子进程

通过以下接口创建的不带页面的进程：

- `OH_Ability_CreateNativeChildProcess`
- `OH_Ability_CreateNativeChildProcessWithConfigs`
- `OH_Ability_StartNativeChildProcess`
- `OH_Ability_StartNativeChildProcessWithConfigs`
- `childProcessManager.startChildProcess`（不包括 SELF_FORK 模式）
- `childProcessManager.startArkChildProcess`
- `childProcessManager.startNativeChildProcess`

### 2.3 接口约束

- 诉求是 **C 接口**
- 通过**两个接口**分别提供两类子进程的查询能力
- 查询接口为**同步**调用

## 3. 输入/处理/输出

| 项目 | 描述 |
|------|------|
| **输入** | 调用查询接口 |
| **处理** | 查询应用自身子进程 |
| **输出** | 返回查询结果（pid、hostPid、processName） |

## 4. 验收场景和标准

| AC 编号 | 验收场景 | 验收标准 |
|---------|---------|---------|
| AC-1.1 | 查询不带页面的子进程 | 调用 `OH_Ability_GetChildProcesses` 可查询到通过 `OH_Ability_CreateNativeChildProcess` 创建的子进程 |
| AC-1.2 | 查询不带页面的子进程（带参数） | 调用 `OH_Ability_GetChildProcesses` 可查询到通过 `OH_Ability_StartNativeChildProcess` 创建的子进程 |
| AC-1.3 | 查询 ArkTS 子进程 | 调用 `OH_Ability_GetChildProcesses` 可查询到通过 `childProcessManager.startArkChildProcess` 创建的子进程 |
| AC-1.4 | 查询 UIAbility 子进程 | 调用 `OH_Ability_GetUIAbilityChildProcesses` 可查询到通过 `startAbility` + `NEW_PROCESS_ATTACH_TO_PARENT` 创建的子进程 |
| AC-1.5 | SELF_FORK 模式排除 | `OH_Ability_GetChildProcesses` 的查询结果不包含 `childProcessManager.startChildProcess`（SELF_FORK 模式）创建的子进程 |
| AC-2.1 | 子进程退出后更新 | 子进程退出后，查询结果不再包含该子进程 |
| AC-2.2 | 无子进程时返回空 | 无子进程时，接口返回成功，count=0 |
| AC-3.1 | 空指针参数校验 | 传入空指针时，返回 `NCP_ERR_INVALID_PARAM` |
| AC-3.2 | 同 token 校验 | 不同 token 的进程无法查到其他应用的子进程 |
| AC-3.3 | 内存释放 | 调用 `OH_Ability_DestroyChildProcessInfos` 后无内存泄漏 |

## 5. 约束与假设

- 查询接口仅返回调用者自身进程的子进程
- 仅需同 token 校验，无需额外系统权限
- 子进程信息返回 pid、hostPid、processName 三个字段
