# CLI启动沙箱分身应用完整调用链

## 概述

本文档详细说明了从CLI执行`ohos-aa start --claw-index`命令到启动分身UIAbility的完整调用链。

## 完整调用链

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                          CLI命令层                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│ 1. 用户执行:                                                               │
│    ohos-aa start --abilityname EntryAbility --bundlename com.example.app   │
│                --claw-index 1 --creator-bundle com.creator.app               │
│                                                                             │
│ 2. ClawAaShellCommand::RunAsStartAbility()                                  │
│    ├─ MakeWantFromCmd() 解析命令行参数                                       │
│    │  ├─ 解析 --claw-index → want.SetParam(PARAM_APP_CLONE_INDEX_KEY, 1)  │
│    │  └─ 解析 --creator-bundle → want.SetParam("creatorBundleName", ...)  │
│    └─ AbilityManagerClient::GetInstance()->StartAbility(want)               │
└─────────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────────┐
│                          IPC通信层                                          │
├─────────────────────────────────────────────────────────────────────────────┤
│ 3. AbilityManagerClient::StartAbility()                                    │
│    └─ abms->StartAbility(want) [通过IPC]                                     │
│                                                                             │
│ 4. AbilityManagerProxy::StartAbility() [Proxy端]                             │
│    └─ remote_->SendRequest(AbilityManagerInterfaceCode::START_ABILITY)       │
└─────────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────────┐
│                          AMS服务层                                          │
├─────────────────────────────────────────────────────────────────────────────┤
│ 5. AbilityManagerStub::StartAbilityInner() [Stub端]                         │
│    └─ AbilityManagerService::StartAbility()                                  │
└─────────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────────┐
│                    AbilityManagerService核心处理                             │
├─────────────────────────────────────────────────────────────────────────────┤
│ 6. AbilityManagerService::StartAbility()                                    │
│    ├─ 检查 forbid start                                                      │
│    ├─ 检查 develop mode                                                     │
│    ├─ 构建 StartAbilityWrapParam                                            │
│    └─ StartAbilityWrap(startAbilityWrapParam)                               │
└─────────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────────┐
│                    StartAbilityWrap → StartAbilityInner                      │
├─────────────────────────────────────────────────────────────────────────────┤
│ 7. AbilityManagerService::StartAbilityInner()                              │
│    ├─ ProcessSandboxCloneLaunch()  ◄─── 沙箱分身应用处理入口              │
│    │  ├─ 检查 PARAM_APP_CLONE_INDEX_KEY                                     │
│    │  ├─ 验证 clawCloneIndex 范围                                            │
│    │  ├─ GetCreatorBundleNameForSandboxClone()  ◄─── 权限检查               │
│    │  │  ├─ 获取 callerBundleName (从环境变量)                                │
│    │  │  ├─ 获取 inputCreatorBundleName (从CLI参数)                           │
│    │  │  └─ VerifyAccessToken(MANAGE_CLI_SANDBOX_BUNDLE)                     │
│    │  │     ├─ 有权限 → 使用 inputCreatorBundleName                           │
│    │  │     └─ 无权限 → 使用 callerBundleName                                │
│    │  ├─ QueryAbilityInfo()  ◄─── 查询分身Ability信息                       │
│    │  │  └─ BundleMgrHelper::QueryAbilityInfo(want, flags, userId, info)    │
│    │  └─ 更新 want.SetModuleName(abilityInfo.moduleName)                     │
│    │                                                                         │
│    ├─ StartAbilityUtils::SetTargetCloneIndexInSameBundle()                  │
│    ├─ StartAbilityUtils::GetAppIndex()  ◄─── 获取appIndex                     │
│    ├─ CheckMultiInstanceAndAppClone()  ◄─── 检查多实例和分身                 │
│    ├─ StartAbilityInfoWrap threadLocalInfo()  ◄─── 创建AbilityInfo             │
│    └─ interceptorExecuter_->DoProcess()  ◄─── 拦截器处理                     │
└─────────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────────┐
│                    拦截器处理链                                               │
├─────────────────────────────────────────────────────────────────────────────┤
│ 8. AbilityInterceptorExecuter::DoProcess()                                 │
│    ├─ StartOtherAppInterceptor                                               │
│    ├─ EcologicalRuleInterceptor                                             │
│    ├─ KioskInterceptor                                                      │
│    ├─ ScreenUnlockInterceptor                                               │
│    ├─ ControlInterceptor                                                    │
│    ├─ DisposedRuleInterceptor                                              │
│    ├─ BlockAllAppStartInterceptor                                           │
│    └─ ExtensionControlInterceptor                                          │
└─────────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────────┐
│                    GenerateAbilityRequest                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│ 9. GenerateAbilityRequest()                                                 │
│    ├─ StartAbilityInfo::CreateStartAbilityInfo()  ◄─── 创建AbilityInfo      │
│    │  └─ 查询分身应用的 AbilityInfo (appIndex已在Want中设置)                  │
│    ├─ 构建 AbilityRequest                                                   │
│    └─ 设置 abilityInfo, appInfo, uid 等                                      │
└─────────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────────┐
│                    启动UIAbility                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│ 10. 根据Ability类型分发给不同的Manager                                       │
│     │                                                                       │
│     ├─ [UIAbility] → UIAbilityLifecycleManager::StartAbility()              │
│     │  ├─ 创建 AbilityRecord                                               │
│     │  ├─ 设置 appIndex (从Want中获取)                                       │
│     │  ├─ LoadAbility()                                                     │
│     │  ├─ 发起应用进程启动 (通过AppManagerService)                           │
│     │  │  └─ AppSpawn → 启动带appIndex的分身应用进程                         │
│     │  └─ 连接 AbilityScheduler                                              │
│     │                                                                       │
│     └─ [Extension] → CommonExtensionManager / UIExtensionAbilityManager       │
└─────────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────────┐
│                    应用进程启动 (AppManagerService)                          │
├─────────────────────────────────────────────────────────────────────────────┤
│ 11. AppManagerService::StartProcess()                                       │
│     ├─ 根据 appIndex 启动分身应用进程                                        │
│     │  └─ 进程名格式: bundleName:appIndex                                    │
│     ├─ 创建 AppRunningRecord                                                │
│     └─ 设置 appIndex 到 AppInfo                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                    ↓
┌─────────────────────────────────────────────────────────────────────────────┐
│                    UIAbility生命周期                                        │
├─────────────────────────────────────────────────────────────────────────────┤
│ 12. UIAbilityImpl (在应用进程中)                                            │
│     ├─ onCreate()   ◄─── 分身应用启动                                        │
│     ├─ onForeground()                                                        │
│     └─ 显示分身应用的UI                                                      │
└─────────────────────────────────────────────────────────────────────────────┘

## 关键数据结构

### Want参数传递
```cpp
// CLI设置
want.SetParam(Want::PARAM_APP_CLONE_INDEX_KEY, clawIndex);  // 分身索引
want.SetParam("creatorBundleName", inputCreatorBundleName);    // 创建方包名(不可信)
want.SetParam("callerBundleName", callerBundleName);          // 调用方包名(可信，来自环境变量)

// AMS处理后
want.SetModuleName(abilityInfo.moduleName);  // 从BMS查询的module name
```

### 权限检查流程
```cpp
bool hasPermission = Security::AccessToken::AccessTokenKit::VerifyAccessToken(
    callerTokenId, "ohos.permission.MANAGE_CLI_SANDBOX_BUNDLE", false);

if (hasPermission) {
    creatorBundleName = inputCreatorBundleName;  // 使用CLI传入的创建方包名
} else {
    creatorBundleName = callerBundleName;       // 使用调用方自己的包名
}
```

### BMS查询
```cpp
// 使用现有的QueryAbilityInfo接口
BundleMgrHelper::QueryAbilityInfo(want, 
    AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, 
    userId, abilityInfo);

// abilityInfo中包含分身应用的完整信息
// abilityInfo.applicationInfo.appIndex 即为分身索引
```

## 关键文件位置

| 功能 | 文件 |
|------|------|
| CLI参数解析 | [tools/ohos-aa/src/ohos_aa_command.cpp:913-935](tools/ohos-aa/src/ohos_aa_command.cpp) |
| AMS入口 | [services/abilitymgr/src/ability_manager_service.cpp:688](services/abilitymgr/src/ability_manager_service.cpp) |
| 分身处理入口 | [services/abilitymgr/src/ability_manager_service.cpp:6716](services/abilitymgr/src/ability_manager_service.cpp) |
| 权限检查 | [services/abilitymgr/src/ability_manager_service.cpp:6659](services/abilitymgr/src/ability_manager_service.cpp) |
| BMS查询 | [services/abilitymgr/src/ability_manager_service.cpp:6742](services/abilitymgr/src/ability_manager_service.cpp) |
| UIAbility启动 | [services/abilitymgr/src/scene_board/ui_ability_lifecycle_manager.cpp](services/abilitymgr/src/scene_board/) |

## 错误处理

| 错误场景 | 错误码 | 说明 |
|---------|-------|------|
| clawIndex参数缺失 | ERR_OK | 不是分身启动，正常流程 |
| clawIndex范围无效 | ERR_APP_CLONE_INDEX_INVALID | 索引小于0或大于MAX_APP_CLONE_INDEX |
| 创建方包名获取失败 | ERR_INVALID_VALUE | 权限检查和包名获取失败 |
| BMS查询失败 | RESOLVE_ABILITY_ERR | QueryAbilityInfo返回失败 |
| 权限不足 | CHECK_PERMISSION_FAILED | VerifyAccessToken返回PERMISSION_DENIED |

## 注意事项

1. **appIndex传递**: `PARAM_APP_CLONE_INDEX_KEY`参数在整个流程中保持传递，最终用于：
   - BMS查询分身应用信息
   - AppManagerService启动分身进程
   - AbilityRecord中记录分身索引

2. **权限隔离**: 无`MANAGE_CLI_SANDBOX_BUNDLE`权限时，强制使用调用方自己的包名作为创建方，防止恶意应用指定创建方

3. **兼容性**: 流程与普通应用启动完全兼容，仅在检测到`PARAM_APP_CLONE_INDEX_KEY`时进行分身特殊处理
