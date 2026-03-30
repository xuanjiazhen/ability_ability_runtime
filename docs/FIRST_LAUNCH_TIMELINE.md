# 应用首次启动功能时序图

## 流程概述

本文档描述应用首次启动功能的完整调用流程。

## 时序图

### 完整时序图（包含分支判断）

```
┌─────────────┐      ┌──────────────┐      ┌─────────────┐      ┌────────────────┐      ┌──────────────┐      ┌───────────┐
│  UIAbility  │      │      AMS     │      │ AbilityEvent │      │   CommonEvent  │      │BundleMgrHelper│      │    BMS    │
│             │      │              │      │    Util      │      │    Manager     │      │              │      │           │
└──────┬──────┘      └──────┬───────┘      └──────┬───────┘      └────────┬───────┘      └──────┬───────┘      └─────┬─────┘
       │                    │                     │                     │                      │                      │
       │ 1.StartUIAbility    │                     │                     │                      │                      │
       │───────────────────>│                     │                     │                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │ 2.StartAbilityInner   │                     │                      │                      │
       │                    │    (统一入口点)        │                     │                      │                      │
       │                    │────────────────────>│                     │                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │ 3.HandleBundleFirstLaunch                      │                      │
       │                    │                     │    (appInfo, userId)  │                      │                      │
       │                    │                     │────────────────────>│                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │ 4.检查 isBundleFirst │                      │                      │
       │                    │                     │    Launched          │                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │ [alt: isBundleFirstLaunched]                   │                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │   [false: 首次启动]   │                      │                      │
       │                    │                     │─────────────────────┴──────────────────────┴──────────────────────┘
       │                    │                     │                     │                      │                      │
       │                    │                     │ 5.创建Want对象       │                      │                      │
       │                    │                     │   设置action = APP_   │                      │                      │
       │                    │                     │   FIRST_LAUNCH       │                      │                      │
       │                    │                     │────────────────────>│                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │ 6.PublishCommonEvent │                      │                      │
       │                    │                     │   (commonData)      │                      │                      │
       │                    │                     │────────────────────>│                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │                     │ 7.发送事件             │                      │
       │                    │                     │                     │─────────────────────>│                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │                     │                      │ 8.SetBundleFirstLaunch
       │                    │                     │<────────────────────┴─────────────────────┤   (bundleName,     │
       │                    │                     │                     │                      │    userId,          │
       │                    │                     │                     │                      │    appIndex,        │
       │                    │                     │ 9a.返回 true        │                      │    true)            │
       │                    │                     │                     │                      │────────────────────>│
       │                    │                     │                     │                      │                      │
       │                    │                     │                     │                      │                      │ 10.更新状态
       │                    │                     │                     │                      │                      │ isBundleFirst
       │                    │                     │                     │                      │                      │ Launched=true
       │                    │                     │                     │                      │                      │───────────>│
       │                    │                     │                     │                      │                      │            │
       │                    │                     │                     │                      │                      │<──────────│
       │                    │                     │                     │                      │                      │            │
       │                    │                     │                     │                      │                      │ 11.返回成功│
       │                    │                     │                     │                      │                      │            │
       │                    │                     │                     │                      │<─────────────────────┤                      │
       │                    │                     │                     │                      │                      │            │
       │                    │                     │<────────────────────┤                      │<─────────────────────┤                      │
       │                    │                     │                     │                      │                      │            │
       │                    │                     │   [true: 非首次启动]  │                      │                      │
       │                    │                     │────────────────────────────────────────────────────────────────────────────
       │                    │                     │                     │                      │                      │
       │                    │                     │ 9b.直接返回 true     │                      │                      │
       │                    │                     │  (跳过所有后续操作)   │                      │                      │
       │                    │                     │  - 不发送公共事件     │                      │                      │
       │                    │                     │  - 不更新状态         │                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │<────────────────────┴─────────────────────┴──────────────────────┴──────────────────────┘
       │                    │
       │                    │ [end alt]
       │                    │
       │                    │ 12.继续UIAbility启动流程
       │<───────────────────┘
```

### 分支时序图 A：首次启动 (isBundleFirstLaunched = false)

```
┌─────────────┐      ┌──────────────┐      ┌─────────────┐      ┌────────────────┐      ┌──────────────┐      ┌───────────┐
│  UIAbility  │      │      AMS     │      │ AbilityEvent │      │   CommonEvent  │      │BundleMgrHelper│      │    BMS    │
│             │      │              │      │    Util      │      │    Manager     │      │              │      │           │
└──────┬──────┘      └──────┬───────┘      └──────┬───────┘      └────────┬───────┘      └──────┬───────┘      └─────┬─────┘
       │                    │                     │                     │                      │                      │
       │ StartUIAbility     │                     │                     │                      │                      │
       │───────────────────>│                     │                     │                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │ StartAbilityInner    │                     │                      │                      │
       │                    │────────────────────>│                     │                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │ HandleBundleFirstLaunch                     │                      │
       │                    │                     │   (appInfo, userId)   │                      │                      │
       │                    │                     │────────────────────>│                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │ 检查: isBundleFirst  │                      │                      │
       │                    │                     │   Launched = false   │                      │                      │
       │                    │                     │   ✓ 继续执行          │                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │ 创建Want             │                      │                      │
       │                    │                     │ action=APP_FIRST_    │                      │                      │
       │                    │                     │ LAUNCH               │                      │                      │
       │                    │                     │────────────────────>│                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │ PublishCommonEvent   │                      │                      │
       │                    │                     │────────────────────>│                      │                      │
       │                    │                     │                     │                      │                      │
       │                    │                     │                     │ 发送事件               │                      │
       │                    │                     │                     │─────────────────────>│                      │
       │                    │                     │<────────────────────┴─────────────────────┤                      │
       │                    │                     │                     │                      │ SetBundleFirstLaunch
       │                    │                     │                     │                      │ (... , true)         │
       │                    │                     │                     │                      │────────────────────>│
       │                    │                     │                     │                      │                      │
       │                    │                     │                     │                      │                      │ 更新状态
       │                    │                     │                     │                      │                      │ isBundleFirst
       │                    │                     │                     │                      │                      │ Launched=true
       │                    │                     │                     │                      │                      │───────────>│
       │                    │                     │                     │                      │                      │            │
       │                    │                     │                     │                      │                      │<──────────│
       │                    │                     │                     │                      │<─────────────────────┤                      │
       │                    │                     │<────────────────────┴─────────────────────┴──────────────────────┘
       │                    │                     │
       │                    │<────────────────────┘
       │                    │
       │                    │ 继续UIAbility启动流程
       │<───────────────────┘
```

### 分支时序图 B：非首次启动 (isBundleFirstLaunched = true)

```
┌─────────────┐      ┌──────────────┐      ┌─────────────┐
│  UIAbility  │      │      AMS     │      │ AbilityEvent │
│             │      │              │      │    Util      │
└──────┬──────┘      └──────┬───────┘      └──────┬───────┘
       │                    │                     │
       │ StartUIAbility     │                     │
       │───────────────────>│                     │
       │                    │                     │
       │                    │ StartAbilityInner    │
       │                    │────────────────────>│
       │                    │                     │
       │                    │                     │ HandleBundleFirstLaunch
       │                    │                     │   (appInfo, userId)
       │                    │                     │────────────────────>│
       │                    │                     │                     │
       │                    │                     │ 检查: isBundleFirst  │
       │                    │                     │   Launched = true    │
       │                    │                     │   ✓ 直接返回 true     │
       │                    │                     │                     │
       │                    │                     │ ⚠ 跳过后续操作：       │
       │                    │                     │  - 不发送公共事件     │
       │                    │                     │  - 不更新状态         │
       │                    │                     │  - 不调用 BMS        │
       │                    │                     │                     │
       │                    │                     │ 返回 true           │
       │                    │                     │<────────────────────┘
       │                    │                     │
       │                    │<────────────────────┘
       │                    │
       │                    │ 继续UIAbility启动流程
       │<───────────────────┘
```

## 时序说明

### 场景A：首次启动 (isBundleFirstLaunched = false)

| 步骤 | 调用方 | 被调用方 | 方法/操作 | 说明 |
|------|--------|----------|-----------|------|
| 1 | UIAbility | AMS | `StartUIAbilityBySCB()` | 应用启动入口 |
| 2 | AMS | AMS | `StartAbilityInner()` | 统一的UIAbility启动入口点 |
| 3 | AMS | AbilityEventUtil | `HandleBundleFirstLaunch(appInfo, userId)` | 处理首次启动逻辑 |
| 4 | AbilityEventUtil | ApplicationInfo | 检查 `isBundleFirstLaunched = false` | 判断为首次启动，继续处理 |
| 5 | AbilityEventUtil | Want | 创建Want对象 | 设置action = `COMMON_EVENT_APP_FIRST_LAUNCH` |
| 6 | AbilityEventUtil | CommonEventManager | `PublishCommonEvent(commonData)` | 发布公共事件 |
| 7 | CommonEventManager | 系统 | 发送事件到订阅者 | 事件包含：bundleName、userId、appIndex、uid |
| 8 | AbilityEventUtil | BundleMgrHelper | `SetBundleFirstLaunch(bundleName, userId, appIndex, true)` | 更新首次启动状态 |
| 9 | BundleMgrHelper | BMS | `SetBundleFirstLaunch(...)` [IPC] | 跨进程调用，更新应用状态 |
| 10 | BMS | BMS | 设置 `isBundleFirstLaunched = true` | 持久化应用状态 |
| 11 | AbilityEventUtil | AMS | 返回 `true` | 首次启动处理完成 |
| 12 | AMS | - | 继续UIAbility启动流程 | 完成首次启动处理后继续 |

### 场景B：非首次启动 (isBundleFirstLaunched = true)

| 步骤 | 调用方 | 被调用方 | 方法/操作 | 说明 |
|------|--------|----------|-----------|------|
| 1 | UIAbility | AMS | `StartUIAbilityBySCB()` | 应用启动入口 |
| 2 | AMS | AMS | `StartAbilityInner()` | 统一的UIAbility启动入口点 |
| 3 | AMS | AbilityEventUtil | `HandleBundleFirstLaunch(appInfo, userId)` | 处理首次启动逻辑 |
| 4 | AbilityEventUtil | ApplicationInfo | 检查 `isBundleFirstLaunched = true` | **判断为已启动，直接返回** |
| 5 | AbilityEventUtil | - | **直接返回 `true`** | ⭐ **不执行任何后续操作** |
|   | - | - | - | - 不发送公共事件 |
|   | - | - | - | - 不调用 BundleMgrHelper |
|   | - | - | - | - 不更新 BMS 状态 |
|   | - | - | - | - 函数立即返回 |
| 6 | AMS | - | 继续UIAbility启动流程 | 跳过首次启动处理，直接继续 |

## 关键数据

### ApplicationInfo 字段
```cpp
struct ApplicationInfo {
    std::string bundleName;           // 应用包名
    int32_t appIndex;                  // 应用索引（0=普通, >0=克隆）
    bool isBundleFirstLaunched;       // 是否首次启动 ⭐
    int32_t uid;                       // 用户ID
};
```

### 公共事件参数
| 参数名 | 类型 | 示例 |
|--------|------|------|
| bundleName | String | "com.example.app" |
| userId | Int | 100 |
| appIndex | Int | 0 |
| uid | Int | 20010000 |

## 状态转换与分支处理

```
安装后首次启动 (isBundleFirstLaunched = false):
  ┌─────────────────────────────────────────────────────────────────┐
  │ 1. HandleBundleFirstLaunch(appInfo, userId)                     │
  │ 2. 检查 isBundleFirstLaunched = false                           │
  │ 3. 条件判断：if (appInfo.isBundleFirstLaunched) → false         │
  │ 4. 跳过 if 块，继续执行后续代码                                 │
  │ 5. 创建 Want (action = APP_FIRST_LAUNCH)                        │
  │ 6. PublishCommonEvent(commonData)                               │
  │ 7. SetBundleFirstLaunch(..., true)                              │
  │ 8. 返回 true                                                   │
  └─────────────────────────────────────────────────────────────────┘

再次启动 (isBundleFirstLaunched = true):
  ┌─────────────────────────────────────────────────────────────────┐
  │ 1. HandleBundleFirstLaunch(appInfo, userId)                     │
  │ 2. 检查 isBundleFirstLaunched = true                            │
  │ 3. 条件判断：if (appInfo.isBundleFirstLaunched) → true          │
  │ 4. 进入 if 块                                                   │
  │ 5. 执行 return true                                            │
  │ 6. ⭐ 函数立即返回，不执行后续任何代码                           │
  │    - 不发送公共事件                                             │
  │    - 不调用 SetBundleFirstLaunch                                │
  │    - 不更新状态                                                 │
  └─────────────────────────────────────────────────────────────────┘
```

### HandleBundleFirstLaunch 代码逻辑

```cpp
bool AbilityEventUtil::HandleBundleFirstLaunch(
    const AppExecFwk::ApplicationInfo &appInfo, int32_t userId)
{
    // ⭐ 分支1: 已经启动过 → 直接返回（不往下执行）
    if (appInfo.isBundleFirstLaunched) {
        return true;  // ⚠ 这里 return 后，后面的代码都不会执行
    }

    // 分支2: 首次启动 → 继续执行以下代码
    // 1. 发送 COMMON_EVENT_APP_FIRST_LAUNCH 事件
    // 2. 调用 SetBundleFirstLaunch 更新状态为 true

    return true;
}
```

### 关键区别说明

| 对比项 | 首次启动 (false) | 非首次启动 (true) |
|--------|-----------------|-------------------|
| if 条件 | `false`，跳过 if 块 | `true`，进入 if 块 |
| 执行路径 | 继续执行后续代码 | 立即 return，函数结束 |
| 发送公共事件 | ✓ 发送 | ✗ 不发送 |
| 调用 BMS 更新状态 | ✓ 调用 | ✗ 不调用 |
| 状态变化 | false → true | 保持 true |

## 代码位置

- **入口**: `ability_manager_service.cpp:3224`
- **工具类**: `ability_event_util.cpp:59`
- **代理方法**: `bundle_mgr_helper.cpp:1177`
