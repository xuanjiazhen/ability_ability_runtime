# 应用首次启动功能技术文档

## 1. 功能概述

应用首次启动功能用于检测和记录应用在安装后的首次启动事件，通过发送 `COMMON_EVENT_APP_FIRST_LAUNCH` 公共事件通知系统和其他应用。

### 1.1 核心特性

- **统一入口点**：所有 UIAbility 启动都通过 `StartUIAbilityBySCBDefaultCommon` 处理
- **事件通知**：首次启动时发送 `COMMON_EVENT_APP_FIRST_LAUNCH` 事件
- **状态记录**：通过 `SetBundleFirstLaunch` 更新应用首次启动状态
- **克隆应用支持**：支持应用克隆场景（appIndex > 0）

---

## 2. 调用流程

### 2.1 完整调用链

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          应用首次启动流程                                     │
└─────────────────────────────────────────────────────────────────────────────┘

  应用/桌面启动
       │
       ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│ SCB (Scene Board)                                                           │
│  StartUIAbilityBySCB()                                                      │
└──────────────────────────────────────────────────────────────────────────────┘
       │
       ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│ AbilityManagerService                                                        │
│  StartUIAbilityBySCBDefaultCommon()                                         │
│  - 统一的 UIAbility 启动入口                                                 │
│  - 处理应用启动、桌面点击、其他启动方式                                        │
└──────────────────────────────────────────────────────────────────────────────┘
       │
       ▼
┌──────────────────────────────────────────────────────────────────────────────┐
│ AbilityEventUtil                                                            │
│  HandleBundleFirstLaunch(appInfo, userId)                                  │
│  - 检查 isBundleFirstLaunched 标志                                          │
│  - 发送 COMMON_EVENT_APP_FIRST_LAUNCH 事件                                   │
│  - 调用 SetBundleFirstLaunch 更新状态                                        │
└──────────────────────────────────────────────────────────────────────────────┘
       │
       ├─────────────────────────────────────────────────────────────────────┐
       │                                                                      │
       ▼                                                                      ▼
┌──────────────────────────────┐                                ┌──────────────────────────────┐
│ CommonEventManager           │                                │ BundleMgrHelper               │
│ PublishCommonEvent()          │                                │ SetBundleFirstLaunch()        │
│ - 发送公共事件                │                                │ - 更新首次启动状态              │
└──────────────────────────────┘                                └──────────────────────────────┘
```

### 2.2 代码位置

| 文件 | 路径 |
|------|------|
| **入口函数** | `services/abilitymgr/src/ability_manager_service.cpp:3224` |
| **工具类** | `services/abilitymgr/src/utils/ability_event_util.cpp:56` |
| **头文件** | `services/abilitymgr/include/utils/ability_event_util.h:38` |

---

## 3. 关键代码

### 3.1 StartUIAbilityBySCBDefaultCommon (入口)

**文件**: `services/abilitymgr/src/ability_manager_service.cpp`

```cpp
int32_t AbilityManagerService::StartUIAbilityBySCBDefaultCommon(
    AbilityRequest &abilityRequest,
    sptr<SessionInfo> sessionInfo,
    AbilityRuntime::StartParamsBySCB &params,
    bool &isColdStart)
{
    // 获取 userId
    int32_t userId = IPCSkeleton::GetCallingUid() / BASE_USER_RANGE;

    // 处理首次启动逻辑
    AbilityEventUtil::HandleBundleFirstLaunch(abilityRequest.appInfo, userId);

    // 继续处理 UIAbility 启动...
    auto abilityInfo = abilityRequest.abilityInfo;
    // ...
}
```

### 3.2 AbilityEventUtil::HandleBundleFirstLaunch (核心逻辑)

**文件**: `services/abilitymgr/src/utils/ability_event_util.cpp`

```cpp
bool AbilityEventUtil::HandleBundleFirstLaunch(
    const AppExecFwk::ApplicationInfo &appInfo,
    int32_t userId)
{
    // 如果已经启动过，直接返回
    if (appInfo.isBundleFirstLaunched) {
        return true;
    }

    // 1. 发布 COMMON_EVENT_APP_FIRST_LAUNCH 事件
    AAFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_APP_FIRST_LAUNCH);
    want.SetParam("bundleName", appInfo.bundleName);
    want.SetParam("userId", userId);
    want.SetParam("appIndex", appInfo.appIndex);
    want.SetParam("uid", appInfo.uid);

    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);

    // 2. 更新首次启动状态
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    bundleMgrHelper->SetBundleFirstLaunch(
        appInfo.bundleName, userId, appInfo.appIndex, true);

    return true;
}
```

---

## 4. 公共事件说明

### 4.1 事件名称

```
usual.event.APP_FIRST_LAUNCH
```

### 4.2 事件参数

| 参数名 | 类型 | 说明 | 示例值 |
|--------|------|------|--------|
| `bundleName` | String | 应用包名 | `"com.example.app"` |
| `userId` | Int | 用户ID | `100` |
| `appIndex` | Int | 应用索引（0为普通应用，>0为克隆应用） | `0` |
| `uid` | Int | 应用UID | `20010000` |

### 4.3 事件订阅示例

```javascript
// 订阅首次启动事件
import commonEventManager from '@ohos.commonEventManager';

let subscribeInfo = {
    events: [
        "usual.event.APP_FIRST_LAUNCH"
    ]
};

commonEventManager.createSubscriber(subscribeInfo, (err, subscriber) => {
    if (!err) {
        commonEventManager.subscribe(subscriber, (err, data) => {
            console.log('App First Launch:');
            console.log('  bundleName: ' + data.data.parameters.bundleName);
            console.log('  userId: ' + data.data.parameters.userId);
            console.log('  appIndex: ' + data.data.parameters.appIndex);
            console.log('  uid: ' + data.data.parameters.uid);
        });
    }
});
```

---

## 5. 测试场景

### 5.1 场景1：普通应用首次启动

**前置条件**: 应用已安装，`isBundleFirstLaunched = false`

**执行步骤**:
1. 从桌面点击应用图标
2. 系统调用 `StartUIAbilityBySCB`
3. `StartUIAbilityBySCBDefaultCommon` 被调用
4. `HandleBundleFirstLaunch` 检测到首次启动
5. 发送 `COMMON_EVENT_APP_FIRST_LAUNCH` 事件
6. 调用 `SetBundleFirstLaunch(true)` 更新状态

**预期结果**:
- 事件成功发送，包含正确的参数
- `isBundleFirstLaunched` 被设置为 `true`
- 应用正常启动

### 5.2 场景2：应用非首次启动

**前置条件**: 应用已启动过，`isBundleFirstLaunched = true`

**执行步骤**:
1. 再次启动应用
2. `HandleBundleFirstLaunch` 检测到已启动
3. 直接返回，不发送事件

**预期结果**:
- 不发送首次启动事件
- 应用正常启动

### 5.3 场景3：克隆应用首次启动

**前置条件**: 克隆应用已安装，`appIndex = 1`, `isBundleFirstLaunched = false`

**执行步骤**:
1. 启动克隆应用
2. `HandleBundleFirstLaunch` 处理克隆应用
3. 发送事件，`appIndex = 1`

**预期结果**:
- 事件成功发送，`appIndex` 正确为 `1`
- 克隆应用的首次启动状态独立于原应用

---

## 6. 日志说明

### 6.1 关键日志

```cpp
// 首次启动检测日志
TAG_LOGI(AAFwkTag::ABILITYMGR, "HandleBundleFirstLaunch: bundleName=%{public}s, userId=%{public}d, appIndex=%{public}d",
    appInfo.bundleName.c_str(), userId, appInfo.appIndex);

// 事件发送失败日志
TAG_LOGE(AAFwkTag::ABILITYMGR, "PublishCommonEvent COMMON_EVENT_APP_FIRST_LAUNCH failed");

// SetBundleFirstLaunch 失败日志
TAG_LOGW(AAFwkTag::ABILITYMGR, "SetBundleFirstLaunch failed: bundleName=%{public}s, err=%{public}d",
    appInfo.bundleName.c_str(), ret);
```

### 6.2 日志过滤

```bash
# 查看首次启动相关日志
hilog -x ability_runtime | grep -i "FirstLaunch"
hilog -x ability_runtime | grep -i "APP_FIRST_LAUNCH"
```

---

## 7. 相关接口

### 7.1 BundleMgrHelper::SetBundleFirstLaunch

**接口定义**:
```cpp
ErrCode SetBundleFirstLaunch(
    const std::string &bundleName,  // 应用包名
    int32_t userId,                  // 用户ID
    int32_t appIndex,                // 应用索引
    bool isBundleFirstLaunched       // 是否首次启动
);
```

### 7.2 ApplicationInfo::isBundleFirstLaunched

**字段说明**:
- 类型: `bool`
- 默认值: `false`
- 说明: 标识应用是否已首次启动过

---

## 8. 版本历史

| 版本 | 日期 | 变更说明 |
|------|------|----------|
| 1.0 | 2026-03-30 | 初始版本，实现首次启动事件通知功能 |

---

## 9. 附录

### 9.1 相关Commit

- `feat: add isBundleFirstLaunched field and logging for first launch event`
- `refactor: extract first launch logic to AbilityEventUtil::HandleBundleFirstLaunch`
- `feat: send COMMON_EVENT_APP_FIRST_LAUNCH event on app first launch`

### 9.2 MR链接

https://gitcode.com/openharmony/ability_ability_runtime/merge_requests/18813
