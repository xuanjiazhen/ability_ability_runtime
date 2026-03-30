# 应用首次启动功能时序图

## 流程概述

本文档描述应用首次启动功能的完整调用流程。

## 时序图

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
       │                    │                     │    Launched = false │                      │                      │
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
       │                    │                     │                     │                      │   (bundleName,     │
       │                    │                     │                     │                      │    userId,          │
       │                    │                     │                     │                      │    appIndex,        │
       │                    │                     │                     │                      │    true)            │
       │                    │                     │                     │                      │────────────────────>│
       │                    │                     │                     │                      │                      │
       │                    │                     │                     │                      │                      │ 9.更新状态
       │                    │                     │                     │                      │                      │ isBundleFirst
       │                    │                     │                     │                      │                      │ Launched=true
       │                    │                     │                     │                      │                      │───────────>│
       │                    │                     │                     │                      │                      │            │
       │                    │                     │                     │                      │                      │<──────────│
       │                    │                     │                     │                      │                      │            │
       │                    │                     │                     │                      │                      │ 10.返回成功│
       │                    │                     │                     │                      │                      │            │
       │                    │                     │                     │                      │<─────────────────────┤                      │
       │                    │                     │                     │                      │                      │            │
       │                    │                     │                     │                      │<─────────────────────┤                      │
       │                    │                     │                     │                      │                      │            │
       │                    │                     │                     │<────────────────────┤                      │                      │
       │                    │                     │                     │                      │                      │            │
       │                    │                     │<────────────────────┤                      │                      │                      │
       │                    │                     │                     │                      │                      │            │
       │                    │                     │                     │                      │                      │            │
       │                    │<────────────────────┴─────────────┴──────────────────────┴──────────────────────┴──────────────┘
       │                    │
       │                    │ 11.继续UIAbility启动流程
       │<───────────────────┘
```

## 时序说明

| 步骤 | 调用方 | 被调用方 | 方法/操作 | 说明 |
|------|--------|----------|-----------|------|
| 1 | UIAbility | AMS | `StartUIAbilityBySCB()` | 应用启动入口 |
| 2 | AMS | AMS | `StartUIAbilityBySCBDefaultCommon()` | 统一的UIAbility启动入口点 |
| 3 | AMS | AbilityEventUtil | `HandleBundleFirstLaunch(appInfo, userId)` | 处理首次启动逻辑 |
| 4 | AbilityEventUtil | ApplicationInfo | 检查 `isBundleFirstLaunched` | 判断是否首次启动 |
| 5 | AbilityEventUtil | Want | 创建Want对象，设置action | `COMMON_EVENT_APP_FIRST_LAUNCH` |
| 6 | AbilityEventUtil | CommonEventManager | `PublishCommonEvent(commonData)` | 发布公共事件 |
| 7 | CommonEventManager | 系统 | 发送事件到订阅者 | 事件包含：bundleName、userId、appIndex、uid |
| 8 | AbilityEventUtil | BundleMgrHelper | `SetBundleFirstLaunch(bundleName, userId, appIndex, true)` | 更新首次启动状态 |
| 9 | BundleMgrHelper | BMS | `SetBundleFirstLaunch(...)` [IPC] | 跨进程调用，更新应用状态 |
| 10 | BMS | BMS | 设置 `isBundleFirstLaunched = true` | 持久化应用状态 |
| 11 | AMS | - | 继续UIAbility启动流程 | 完成首次启动处理 |

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

## 状态转换

```
安装后首次启动:  isBundleFirstLaunched=false → 发送事件 → 更新为true
再次启动:       isBundleFirstLaunched=true  → 跳过处理
```

## 代码位置

- **入口**: `ability_manager_service.cpp:3224`
- **工具类**: `ability_event_util.cpp:59`
- **代理方法**: `bundle_mgr_helper.cpp:1177`
