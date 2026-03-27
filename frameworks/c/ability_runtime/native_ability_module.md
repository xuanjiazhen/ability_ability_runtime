# NativeThread方案

# 总体方案

1. 创建UIAbilityRecord的时候，解析AbilityInfo的metadata信息。

matedata中获取这些字段信息：

- ohos.ability.withNativeModule，为bool类型，默认值false，无该配置或者错误配置，值都是false；

- ohos.ability.startPhase，这几种值是合法的， pre_window, pre_foreground, foreground， 默认值pre_window，非法值则取pre_window，每一种值代表一个启动阶段

- ohos.ability.nativeModuleSource，字符串类型，表示动态库路径

- ohos.ability.nativeModuleFun，字符串类型，表示入口函数名称

只有withNativeModule为true时，其他几个配置项才有意义，才需要解析处理；当withNativeModule为true时，解析完整的配置，此时创建 NativeModuleMetaData 的智能指针，保存到UIAbilityRecord中。

NativeModuleMetaData 定义参考下面的文件：

interfaces/inner_api/ability_manager/include/native_module_util.h

```C++
#ifndef OHOS_ABILITY_RUNTIME_NOTIVE_MODULE_UTIL_H
#define OHOS_ABILITY_RUNTIME_NOTIVE_MODULE_UTIL_H

#include "ability_info.h"

namespace OHOS {
namespace AAFwk {
enum class StartupPhase : uint8_t {
    PRE_WINDOW, // onCreate
    PRE_FOREGROUND, // onWindowStage create
    FOREGROUND, // onForeground
};

struct NativeModuleMetaData {
public:
    static void InitData(const AppExecFwk::AbilityInfo &abilityInfo, NativeModuleMetaData &data);
    bool withNativeModule = false;
    StartupPhase startupPhase = StartupPhase::PRE_WINDOW;
    std::string nativeModuleSource;
    std::string nativeModuleFunc;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NOTIVE_MODULE_UTIL_H
```



这里解析出来的信息，下面启动过程中，ability attach到AMS服务的时候，会用到。



1. 在LaunchAbility流程中，也需要解析abilityInfo的metadata信息，加载动态库。

LaunchAbility，是应用启动的一个过程。应用启动过程经过下面的调用链：

- AMS触发loadAbility流程，AbilityRecord::LoadAbility

- 通过调用AppScheduler::LoadAbility，传递到appms

- appms使用ffrt异步任务，调用AppMgrServiceInner::LoadAbility

- appms首先创建应用进程进程，等进程启动后会执行AppRunningRecord::LaunchAbility

- luanchAbility经过ipc把流程传递到应用进程（application）

- 应用进程收到ipc请求后，会切换到主线程执行MainThread::HandleLaunchAbility

- 这里考虑UIAbility，在AbilityThread::AbilityThreadMain创建UIAbilityThread对象

- UIAbilityThread对象准备好以后，通过AbilityManagerClient::GetInstance()->AttachAbilityThread发送ipc请求到ams，即attach过程

- 对于UIAbility，AMS中会分发到UIAbilityLifecycleManager::AttachAbilityThread进行处理

LaunchAbility到应用进程中执行，执行到 UIAbilityThread::AttachInner 时， 考虑在AttachAbilityThread前解析metadata，abilityInfo信息可以从AbilityLocalRecord获取。解析完metadata，根据配置信息，加载so，从so中查找主函数nativeModuleFun；此外，还要从so中找到另外一个函数PostAbility。

解析好的信息使用 NativeThreadInfo 存储，参考这个文件中的定义， interfaces/kits/native/appkit/app/native_thread_info.h

```C++
#ifndef OHOS_ABILITY_RUNTIME_NATIVE_THREAD_INFO_H
#define OHOS_ABILITY_RUNTIME_NATIVE_THREAD_INFO_H

#include <memory>
#include <thread>

#include "native_module_util.h"
#include "napi/native_api.h"

#ifdef __cplusplus
extern "C" {
#endif
struct NativeAbilityWrapper {
    int32_t instanceId = 0;
    std::string abilityName;
    napi_env env = nullptr;
    napi_value jsAbilityObj_ = nullptr;
};

typedef void(*PostAbilityType)(const NativeAbilityWrapper* nativeAbilityWrapper);
typedef void(*OHMainType)();
#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace AppExecFwk {
class NativeThreadInfo {
public:
    NativeThreadInfo(NativeThreadInfo &) = delete;
    void operator=(NativeThreadInfo &) = delete;
    void RunMain();
    void PostAbility(const NativeAbilityWrapper* nativeAbilityWrapper);

private:
    void *moduleHandle_ = nullptr;
    OHMainType OHMain_ = nullptr;
    PostAbilityType postAbilityFunc_ = nullptr;
    std::thread nativeThread_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NATIVE_THREAD_INFO_H
```

使用智能指针创建 NativeThreadInfo ，信息保存到 ApplicationContext 中。注意 UIAbilityThread::AttachInner 中可以获取到 OHOSApplication 对象，OHOSApplication 内部存储了 ApplicationContext。（理论上可以不存储，ApplicationContext是单例）

此外，要在UIAbilityImpl中存储一份NativeModuleMetaData。（UIAbilityThread的成员变量abilityImpl_）



1. 创建线程执行，执行入口函数。

当ApplicationContext中设置好NativeThreadInfo后，则执行其RunMain方法，在该方法中创建新线程，在新线程里执行so的入口函数OHMain_。



1. foreground流程修改。

LaunchAbility流程在应用进程结束时，会发起AttachAbilityThread调用，把自己的通信对象发给AMS服务。流程可以描述如下：

- 应用进程，UIAbilityThread::AttachInner中调用 AbilityManagerClient::GetInstance()->AttachAbilityThread ，这里通过IPC把消息发送到AMS

- AMS服务侧，通过AbilityManagerService::AttachAbilityThread处理该请求

- AMS把请求分发给UIAbilityLifecycleManager::AttachAbilityThread 进行处理

attach完成后，ams拿到应用中ability的通信对象，此时调度执行foreground流程。大致流程参考：

- UIAbilityLifecycleManager::AttachAbilityThread这里调用 DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground

- 走到 AppMgrServiceInner::UpdateAbilityState， AppRunningRecord::AbilityForeground 然后通过ipc调度到应用

- MainThread::ScheduleForegroundApplication，MainThread::HandleForegroundApplication

- 应用调度结束后，调用appMgr_->ApplicationForegrounded，通过ipc回到服务侧

- AppMgrServiceInner::ApplicationForegrounded，AppRunningRecord::PopForegroundingAbilityTokens，ModuleRunningRecord::OnAbilityStateChanged，AppMgrServiceInner::OnAbilityStateChanged

- AbilityManagerService::OnAbilityRequestDone，UIAbilityLifecycleManager::OnAbilityRequestDone

- AbilityRecord::ForegroundAbility，LifecycleDeal::ForegroundNew，abilityScheduler->ScheduleAbilityTransaction，这里会发消息到应用侧

- 应用收到消息，进行处理 UIAbilityThread::ScheduleAbilityTransaction，UIAbilityThread::HandleAbilityTransaction，UIAbilityImpl::HandleAbilityTransaction

- UIAbilityImpl::Start，JsUIAbility::OnStart，然后调用应用的 onCreate 生命周期

执行完应用 onCreate 生命周期后，判断是nativeModule，需要创建 NativeAbilityWrapper 对象（使用智能指针），保存到ApplicationContext中（可以参考applicationContext->DispatchOnAbilityCreate），保存方式按照map<instanceId, std::shared_ptr<NativeAbilityWrapper >>保存。

instanceId使用abilityRecordId。

保存对象后，还要找到前面存储在这里的 NativeThreadInfo ，调用 PostAbility 方法。



1. 判断 startPhase ，来确定接下来需要走哪些生命周期。

回到 UIAbilityImpl::HandleAbilityTransaction

- 如果 startPhase 值为 PRE_WINDOW，则不执行 AbilityTransaction ，直接调用 AbilityTransactionCallback

- 如果 startPhase 值为 PRE_FOREGROUND，UIAbilityImpl::Foreground 中，不调用ability_->OnForeground

- 如果 startPhase  值为 FOREGROUND，则保持逻辑不变



1. ability生命周期调度完成后，AMS服务处理。

生命周期执行结束后，应用会调用AbilityManagerClient::GetInstance()->AbilityTransitionDone发送ipc消息到AMS，AbilityManagerService::AbilityTransitionDone处理后，再分发到UIAbilityLifecycleManager::AbilityTransactionDone，UIAbilityLifecycleManager::DispatchForeground

AMS此时需要判断两个值，startPhase值以及abilityState的前值（保存在AbilityRecord中的值）：

- 如果 startPhase 是 FOREGROUND，则一切都不变

- 如果 startPhase 是 其他值，abilityState前值是 INITIAL，则新增一个状态值，更新到AbilityRecord中，表示已经执行了onCreate，但是没有完成foreground，并且不执行CompleteForegroundSuccess



1. 新增native api

参考interfaces\kits\c\ability_runtime下面的实现方式，增加下面几个API

- AbilityRuntime_ErrorCode OH_AbilityRuntime_GetAbilityInstanceId(const NativeAbilityWrapper* nativeAbilityWrapper, int32_t* instanceId);

- AbilityRuntime_ErrorCode OH_AbilityRuntime_GetAbilityName(const NativeAbilityWrapper* nativeAbilityWrapper, char* buff, const int32_t bufferSize, int32_t* writeLength);

- AbilityRuntime_ErrorCode OH_AbilityRuntime_GetEnv(const NativeAbilityWrapper* nativeAbilityWrapper, napi_env* env);

其中 OH_AbilityRuntime_GetAbilityName 方法，如果 buff 为空，writeLength 不为空，只返回abilityName的大小。



1. JsApplicationContextUtils 中新增方法 getUIAbilityByInstanceId



1. JsAbilityContext 中新增方法 startSelf。

参考JsAbilityContext中其他API，调用链大致为：

- js调用绑定的native方法 startSelf

- JsAbilityContext::OnStartSelf

- AbilityContextImpl  需要携带 token 作为参数

- AbilityManagerClient

- 跨ipc到 ams服务侧  AbilityManagerService

- 分发消息 到 UIAbilityLifecycleManager
