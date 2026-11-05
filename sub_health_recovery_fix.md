# sub_health_recovery 与 errormanager 行为不一致问题修复

## 1. 问题描述

### 现象

在使能了 `const.dfx.sub_health_recovery.enable == true` 的设备上，某应用发生 JS error 时**必定触发应用恢复（重启）**；而在未使能该参数的机型上，同一应用发生 JS error 时可以通过 errormanager catch 住异常，**应用正常运行不闪退**。两种机型行为不一致。

### 复现条件

- 设备系统参数 `const.dfx.sub_health_recovery.enable` 设为 `true`
- 应用未调用 `appRecovery.enableAppRecovery()`（即 `AppRecovery::isEnable_ == false`）
- 应用通过 `errorManager.on("error", observer)` 注册了错误观察者
- 应用发生 JS error 并被 observer 的 `onUnhandledException` 回调 catch 住

### 关键日志

```
11-03 09:36:48.451 13635 13635 I C01317/com.ezviz.videogo/AppKit: [application_data_manager.cpp:212] main thread
11-03 09:36:48.451 13635 13635 I C01324/com.ezviz.videogo/JsNapi: [js_error_manager.cpp:345] call func: onUnhandledException
```

## 2. 根因分析

### 2.1 强制恢复的入口

`AppRecovery::ScheduleRecoverApp` 中，当应用未主动使能 appRecovery（`isEnable_ == false`）时，会检查系统参数 `const.dfx.sub_health_recovery.enable`：

```cpp
// app_recovery.cpp ScheduleRecoverApp
if (!isEnable_) {
    if (OHOS::system::GetParameter("const.dfx.sub_health_recovery.enable", "") == "true") {
        return DoRecoverMainApp(reason);  // 强制恢复
    }
    return false;  // 未使能则不处理
}
```

- **使能设备**：执行 `DoRecoverMainApp` → 强制恢复（重启）
- **未使能设备**：返回 false → 不处理 → 应用通过 errormanager catch 住异常后正常运行

### 2.2 errormanager 旧版 API 的注册路径

`errorManager.on("error", observer)` 存在新旧两种 API 路径：

| API 调用形式 | 路由函数 | 加入 observerList | 调用 AddErrorObserver（设置 errorObserver_） |
|---|---|---|---|
| `errorManager.on("error", observer)` | `OnOnOld` | 是 | **否** |
| `errorManager.on("errorEvent", observer)` | `OnOnNew` | 是 | 是 |

应用使用旧版 `errorManager.on("error", observer)`，走 `OnOnOld` 路径，仅将 observer 加入 `observerList`，**不调用 `AddErrorObserver`**，因此 `ApplicationDataManager::errorObserver_` 保持为 null。当 `errorObserver_` 为 null 时，`NotifyUnhandledException`/`NotifyExceptionObject` 会调用 `TryRecoverApp` → `ScheduleRecoverApp`，最终走到 `DoRecoverMainApp`。

### 2.2b 实际触发路径：DEVELOPER_REQUEST

经日志确认，应用在 errormanager 的 `onUnhandledException` 回调中主动调用了 `appRecovery.restartApp()`，该 API 以 `DEVELOPER_REQUEST` reason 调用 `ScheduleRecoverApp`：

```
app_recovery_api.cpp:191  AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::DEVELOPER_REQUEST);
```

- **使能设备**：`isEnable_==false` + `sub_health_recovery.enable==true` → 执行 `DoRecoverMainApp` → 强制恢复（重启）
- **未使能设备**：`isEnable_==false` + `sub_health_recovery.enable!=true` → 返回 false → 应用正常运行

因此修复不能仅针对 `JS_ERROR`/`CJ_ERROR`，需要对所有非 `APP_FREEZE` 的 reason 检查应用是否已注册 errormanager 回调。

### 2.3 已有的判断机制

`ApplicationDataManager` 已有 `GetHasOnErrorCallback()` 方法，通过 `RegisterHasOnErrorCallback` 注册了 `IsErrorObserverListNotEmpty` 回调，覆盖了所有 errormanager 注册路径：

```cpp
// NAPI (js_error_manager.cpp)
static bool IsErrorObserverListNotEmpty()
{
    return IsobserverListNotEmpty()       // errorManager.on("error", observer) — 旧版 API
        || IsGlobalObserverListNotEmpty() // errorManager.on("globalErrorOccurred", handler)
        || isSetHandler;                  // errorManager.setDefaultErrorHandler(handler)
}

// ANI (error_manager_ani.cpp)
static bool IsErrorObserverListNotEmpty()
{
    return g_defaultHandler.ref != nullptr; // errorManager.on("error", handler)
}
```

## 3. 修复方案

### 3.1 核心思路

在 `ScheduleRecoverApp` 中判断 `sub_health_recovery` 使能的位置，增加对 `ApplicationDataManager::GetHasOnErrorCallback()` 的检查。当应用已通过 errormanager 注册了错误处理器时，跳过 `DoRecoverMainApp`，返回 false。

### 3.2 具体修改

修改两个文件：

**文件1：`frameworks/native/appkit/app/application_data_manager.cpp`**

在已有 namespace 内（`NotifyAppFault` 方法之后、namespace 闭合之前）新增 C++ 桥接函数：

```cpp
// Cross-library bridge for uiabilitykit_native (app_recovery.cpp) to check
// whether the app has registered an error handler via errormanager.
// Uses C++ linkage so the symbol is exported by libappkit_native.map via
// the *ApplicationDataManager* wildcard pattern.
bool ApplicationDataManagerHasOnErrorCallback()
{
    return ApplicationDataManager::GetInstance().GetHasOnErrorCallback();
}
```

**文件2：`frameworks/native/ability/native/recovery/app_recovery.cpp`**

1. 新增弱符号声明和检查函数（在已有的 `namespace OHOS::AppExecFwk` 内）：

```cpp
// Weak declaration: resolved at runtime from appkit_native (loaded later).
// The symbol is a C++ free function whose mangled name contains
// "ApplicationDataManager", matched by libappkit_native.map's wildcard.
bool ApplicationDataManagerHasOnErrorCallback() __attribute__((weak));

static bool CheckHasOnErrorCallback()
{
    if (&ApplicationDataManagerHasOnErrorCallback == nullptr) {
        TAG_LOGI(AAFwkTag::RECOVERY, "ApplicationDataManagerHasOnErrorCallback weak symbol is null");
        return false;
    }
    bool result = ApplicationDataManagerHasOnErrorCallback();
    TAG_LOGI(AAFwkTag::RECOVERY, "CheckHasOnErrorCallback result:%{public}d", result);
    return result;
}
```

2. 修改 `ScheduleRecoverApp` 中的 sub_health_recovery 判断：

```cpp
if (!isEnable_) {
    if (OHOS::system::GetParameter("const.dfx.sub_health_recovery.enable", "") == "true") {
        if (reason != StateReason::APP_FREEZE && CheckHasOnErrorCallback()) {
            TAG_LOGI(AAFwkTag::RECOVERY,
                "app has error handler, skip sub_health_recovery for reason:%{public}d",
                static_cast<int>(reason));
            return false;
        }
        TAG_LOGI(AAFwkTag::RECOVERY,
            "DoRecoverMainApp for reason:%{public}d, isEnable_:%{public}d",
            static_cast<int>(reason), static_cast<int>(isEnable_));
        return DoRecoverMainApp(reason);
    }
    return false;
}
```

### 3.3 依赖说明

存在循环依赖：`appkit_native` 依赖 `uiabilitykit_native`（appkit/BUILD.gn line 187），因此 `uiabilitykit_native` 不能反向依赖 `appkit_native`，否则构成循环依赖。LTO 链接也会在链接时要求所有符号已定义，共享库的"允许未定义符号"特性在 LTO 下不生效。

解决方案：使用 C++ 链接 + `__attribute__((weak))` 弱符号机制打破循环依赖：
- `appkit_native`（`application_data_manager.cpp`）导出一个 C++ 自由函数 `ApplicationDataManagerHasOnErrorCallback()`，内部调用 `GetHasOnErrorCallback()`。其 mangled name 包含 `ApplicationDataManager`，被 `libappkit_native.map` 中 `extern "C++"` 块的 `*ApplicationDataManager*` 通配符自动匹配导出，无需修改 map 文件
- `uiabilitykit_native`（`app_recovery.cpp`）以弱符号声明引用该函数，链接时弱符号未解析不报错，运行时 `appkit_native` 加载后动态链接器将强符号覆盖弱符号
- `CheckHasOnErrorCallback()` 在调用前检查弱符号地址是否为 null，确保 `appkit_native` 未加载时安全返回 false

## 4. 修复后行为对照

| 场景 | 修复前（sub_health_recovery 使能） | 修复前（未使能） | 修复后（sub_health_recovery 使能） |
|------|------|------|------|
| 应用用 errormanager catch 住 JS error | **强制恢复（错误）** | 应用正常运行 | 应用正常运行（一致） |
| 应用在 errormanager 回调中调用 restartApp() | **强制恢复（错误）** | 不重启，应用正常运行 | 应用正常运行（一致） |
| 应用未用 errormanager，发生 JS error | 强制恢复 | 应用闪退 | 强制恢复（不变） |
| 应用发生 APP_FREEZE | 强制恢复 | 不处理 | 强制恢复（不变） |
| 应用主动使能了 appRecovery | 按 appRecovery 逻辑 | 按 appRecovery 逻辑 | 按 appRecovery 逻辑（不变） |

## 5. 影响评估

- **修改范围**：修改 `app_recovery.cpp` 和 `application_data_manager.cpp` 两个文件，不涉及头文件和构建配置修改。
- **复用已有机制**：使用 `GetHasOnErrorCallback()`，该方法已在 errormanager 初始化时通过 `RegisterHasOnErrorCallback` 注册了回调，NAPI 和 ANI 两条路径均已覆盖。
- **范围限制**：对所有非 `APP_FREEZE` 的 reason 生效（包括 `JS_ERROR`、`CJ_ERROR`、`DEVELOPER_REQUEST`），`APP_FREEZE` 恢复逻辑不受影响。
- **兼容性**：不修改任何公共 API 签名，不修改 IPC 协议。
- **行为一致性**：使能 sub_health_recovery 的设备上，应用通过 errormanager catch 异常后不再被强制恢复，与未使能设备行为一致。
- **DFX**：新增 TAG_LOGI 日志记录跳过原因，不修改 hisysevent 事件定义。

## 6. 验证方法

```bash
# 编译验证
./build.sh --product-name <product> --build-target ability_runtime

# 单元测试
run -t UT -tp ability_runtime
```

验证场景：
1. 使能 `const.dfx.sub_health_recovery.enable=true`，应用通过 `errorManager.on("error", observer)` catch JS error → 应用不闪退、不恢复
2. 使能 `const.dfx.sub_health_recovery.enable=true`，应用未注册 errormanager，发生 JS error → 强制恢复
3. 使能 `const.dfx.sub_health_recovery.enable=true`，应用发生 APP_FREEZE → 强制恢复

## 7. 测试用例

### 7.1 修改文件

仅修改 `test/unittest/app_recovery_test/app_recovery_test.cpp`，在文件末尾（`IsEtsAPP_Test_001` 之后、namespace 闭合之前）新增测试用例。

### 7.2 新增测试用例

```cpp
/**
 * @tc.name:  ScheduleRecoverApp_008
 * @tc.desc:  ScheduleRecoverApp when isEnable_ is false with JS_ERROR reason.
 *            ShouldRecoverApp returns true (ALWAYS_RESTART), but
 *            sub_health_recovery.enable is not "true" in test env, returns false.
 * @tc.type: FUNC
 * @tc.require: I7UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleRecoverApp_008, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = false;
    AppRecovery::GetInstance().restartFlag_ = RestartFlag::ALWAYS_RESTART;
    bool ret = AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::JS_ERROR);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ScheduleRecoverApp_009
 * @tc.desc:  ScheduleRecoverApp when isEnable_ is false with CJ_ERROR reason.
 *            ShouldRecoverApp returns true (ALWAYS_RESTART), but
 *            sub_health_recovery.enable is not "true" in test env, returns false.
 * @tc.type: FUNC
 * @tc.require: I7UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleRecoverApp_009, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = false;
    AppRecovery::GetInstance().restartFlag_ = RestartFlag::ALWAYS_RESTART;
    bool ret = AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::CJ_ERROR);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name:  ScheduleRecoverApp_010
 * @tc.desc:  ScheduleRecoverApp with APP_FREEZE reason when isEnable_ is false.
 *            ShouldRecoverApp returns true for APP_FREEZE with ALWAYS_RESTART,
 *            but sub_health_recovery.enable is not "true" in test env, returns false.
 * @tc.type: FUNC
 * @tc.require: I7UL6H
 */
HWTEST_F(AppRecoveryUnitTest, ScheduleRecoverApp_010, TestSize.Level1)
{
    AppRecovery::GetInstance().isEnable_ = false;
    AppRecovery::GetInstance().restartFlag_ = RestartFlag::ALWAYS_RESTART;
    bool ret = AppRecovery::GetInstance().ScheduleRecoverApp(StateReason::APP_FREEZE);
    EXPECT_FALSE(ret);
}
```

### 7.3 测试说明

- `ScheduleRecoverApp_008/009/010`：单元测试环境 `const.dfx.sub_health_recovery.enable` 系统参数不为 `"true"`，`ScheduleRecoverApp` 在 `isEnable_==false` 时直接返回 false，覆盖了 `JS_ERROR`、`CJ_ERROR`、`APP_FREEZE` 三种 reason 路径
- 注意：`CheckHasOnErrorCallback()` 是 `app_recovery.cpp` 中的 `static` 函数，测试文件无法直接调用，因此不为其单独编写用例
- 完整的 `sub_health_recovery` 使能 + errormanager 注册场景需在设备上验证（见第 6 节验证场景）
