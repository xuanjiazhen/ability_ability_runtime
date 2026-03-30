# 应用首次启动功能 UML 序列图

## 流程概述

本文档描述应用首次启动功能的完整调用流程，包括 AMS、BMS 和公共事件服务之间的交互。

## UML 序列图

```mermaid
sequenceDiagram
    autonumber
    participant UIAbility as UIAbility
    participant AMS as AbilityManagerService<br/>(能力管理服务)
    participant EventUtil as AbilityEventUtil<br/>(事件工具类)
    participant AppInfo as ApplicationInfo<br/>(应用信息)
    participant CommonEvent as CommonEventManager<br/>(公共事件管理器)
    participant BMHelper as BundleMgrHelper<br/>(Bundle管理助手)
    participant BMS as BundleMgrService<br/>(Bundle管理服务)

    Note over UIAbility,BMS: 应用首次启动流程

    UIAbility->>AMS: 1. StartUIAbilityBySCB()
    activate AMS
    AMS->>AMS: 2. StartUIAbilityBySCBDefaultCommon()
    Note right of AMS: 获取userId = callingUid / BASE_USER_RANGE
    AMS->>EventUtil: 3. HandleBundleFirstLaunch(appInfo, userId)

    EventUtil->>AppInfo: 4. 检查 isBundleFirstLaunched
    AppInfo-->>EventUtil: false (首次启动)

    EventUtil->>EventUtil: 5. 创建 Want 对象
    Note right of EventUtil: want.SetAction(<br/>COMMON_EVENT_APP_FIRST_LAUNCH)

    rect rgb(200, 220, 240)
        Note left of CommonEvent: 发送公共事件
        EventUtil->>CommonEvent: 6. PublishCommonEvent(commonData)
        Note right of CommonEvent: 事件参数:<br/>- bundleName<br/>- userId<br/>- appIndex<br/>- uid
        CommonEvent-->>EventUtil: 发送成功
    end

    rect rgb(240, 220, 200)
        Note left of BMHelper: 更新首次启动状态
        EventUtil->>BMHelper: 7. SetBundleFirstLaunch(<br/>bundleName, userId, appIndex, true)

        BMHelper->>BMS: 8. SetBundleFirstLaunch(...)<br/>[IPC 跨进程调用]

        BMS->>BMS: 9. 更新应用首次启动状态<br/>isBundleFirstLaunched = true

        BMS-->>BMHelper: 返回 ERR_OK
        BMHelper-->>EventUtil: 设置成功
    end

    EventUtil-->>AMS: 返回 true

    AMS->>AMS: 10. 继续 UIAbility 启动流程...
```

## 组件说明

| 组件 | 说明 |
|------|------|
| **UIAbility** | 用户界面能力，发起启动请求 |
| **AbilityManagerService** | 能力管理服务，统一入口点 |
| **AbilityEventUtil** | 事件工具类，封装首次启动逻辑 |
| **ApplicationInfo** | 应用信息，包含 `isBundleFirstLaunched` 字段 |
| **CommonEventManager** | 公共事件管理器，发布系统事件 |
| **BundleMgrHelper** | Bundle管理助手，本地代理 |
| **BundleMgrService** | Bundle管理服务，管理应用状态 |

## 关键数据结构

### ApplicationInfo (应用信息)
```cpp
struct ApplicationInfo {
    std::string bundleName;           // 应用包名
    int32_t appIndex;                  // 应用索引(0=普通, >0=克隆)
    bool isBundleFirstLaunched;       // 是否首次启动 ⭐
    int32_t uid;                       // 用户ID
    // ... 其他字段
};
```

### COMMON_EVENT_APP_FIRST_LAUNCH 事件参数
```cpp
Want want;
want.SetAction("usual.event.APP_FIRST_LAUNCH");
want.SetParam("bundleName", appInfo.bundleName);
want.SetParam("userId", userId);
want.SetParam("appIndex", appInfo.appIndex);
want.SetParam("uid", appInfo.uid);
```

## 代码位置

| 文件 | 行号 | 说明 |
|------|------|------|
| `ability_manager_service.cpp` | 3224 | `StartUIAbilityBySCBDefaultCommon` 入口 |
| `ability_event_util.cpp` | 59 | `HandleBundleFirstLaunch` 实现 |
| `bundle_mgr_helper.cpp` | 1177 | `SetBundleFirstLaunch` 代理方法 |

## 状态转换

```
┌─────────────────────────────────────────────────────────────────┐
│                    应用首次启动状态转换                          │
└─────────────────────────────────────────────────────────────────┘

    应用安装          首次启动           再次启动
       │                │                │
       ▼                ▼                ▼
   [未启动] ────────→ [首次启动] ────────→ [已启动]
 isBundleFirst    isBundleFirst    isBundleFirst
 Launched=false   Launched=false   Launched=true
                       │
                       ▼
                 发送事件 + 置为true
```

## 时序说明

1. **应用启动** (`T1`)
   - UIAbility 发起启动请求
   - SCB 调用 `StartUIAbilityBySCB`

2. **统一入口处理** (`T2-T3`)
   - `StartUIAbilityBySCBDefaultCommon` 作为所有 UIAbility 启动的统一入口
   - 获取 `userId = callingUid / BASE_USER_RANGE`

3. **首次启动检查** (`T4`)
   - `HandleBundleFirstLaunch` 检查 `appInfo.isBundleFirstLaunched`
   - 如果为 `true`，直接返回，不做处理

4. **发送公共事件** (`T5-T6`)
   - 创建 `Want` 对象，设置 action 为 `COMMON_EVENT_APP_FIRST_LAUNCH`
   - 添加参数：`bundleName`, `userId`, `appIndex`, `uid`
   - 调用 `CommonEventManager::PublishCommonEvent` 发布事件
   - 系统和应用可以订阅此事件

5. **更新启动状态** (`T7-T9`)
   - 调用 `BundleMgrHelper::SetBundleFirstLaunch(bundleName, userId, appIndex, true)`
   - 通过 IPC 跨进程调用 BMS 的 `SetBundleFirstLaunch` 接口
   - BMS 更新应用状态，将 `isBundleFirstLaunched` 置为 `true`

6. **完成** (`T10`)
   - 返回处理结果
   - 继续后续的 UIAbility 启动流程

## 支持场景

| 场景 | appIndex | 说明 |
|------|----------|------|
| 普通应用首次启动 | 0 | 标准应用首次安装后启动 |
| 克隆应用首次启动 | 1, 2, ... | 应用克隆的首次启动 |
| 非首次启动 | - | `isBundleFirstLaunched=true`，跳过处理 |

## 相关 Commits

- `feat: add isBundleFirstLaunched field and logging`
- `refactor: extract first launch logic to AbilityEventUtil`
- `feat: send COMMON_EVENT_APP_FIRST_LAUNCH event`
