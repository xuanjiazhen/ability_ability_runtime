/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_ETS_ABILITY_FFI_H
#define OHOS_ABILITY_RUNTIME_ETS_ABILITY_FFI_H

#include <mutex>

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AppExecFwk {
class OHOSApplication;
template <typename> class AbilityTransactionCallbackInfo;
}
namespace AAFwk {
class Want;
struct LaunchParam;
}
namespace AbilityRuntime {
class AbilityContext;
}
namespace EtsEnv {
struct ETSNativeReference;
}
} // namespace OHOS

using FFIHandle = void *;
using AbilityHandle = void *;
using RuntimeHandle = void *;

extern "C" {
struct ETSAbilityFuncs {
    FFIHandle (*CreateEtsFFI)(AbilityHandle ability, RuntimeHandle runtime) = nullptr;
    void (*DeleteEtsFFI)(FFIHandle ffi) = nullptr;
    bool (*LoadModule)(FFIHandle ffi, const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance) = nullptr;
    bool (*BindNativeMethods)(FFIHandle ffi) = nullptr;
    bool (*CreateEtsContext)(FFIHandle ffi, int32_t screenMode,
        const std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> &abilityContext) = nullptr;
    void (*OnStart)(FFIHandle ffi, const OHOS::AAFwk::Want &want, const OHOS::AAFwk::LaunchParam &launch) = nullptr;
    void (*OnStop)(FFIHandle ffi) = nullptr;
    bool (*CallOnDestroy)(FFIHandle ffi, OHOS::AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
        bool &isAsyncCallback) = nullptr;
    void (*OnWindowStageCreate)(FFIHandle ffi) = nullptr;
    void (*OnWindowStageDestroy)(FFIHandle ffi) = nullptr;
    void (*OnForeground)(FFIHandle ffi, const OHOS::AAFwk::Want &want) = nullptr;
    void (*OnBackground)(FFIHandle ffi) = nullptr;
    void (*OnNewWant)(FFIHandle ffi, const OHOS::AAFwk::Want &want, const OHOS::AAFwk::LaunchParam &launch) = nullptr;
    std::weak_ptr<OHOS::EtsEnv::ETSNativeReference> (*GetAbilityObject)(FFIHandle ffi) = nullptr;
};
}

class __ani_ref;
using ani_ref = __ani_ref *;
class __ani_object;
using ani_object = __ani_object *;

namespace OHOS {
namespace EtsEnv {
struct ETSNativeReference;
}

namespace AbilityRuntime {
class EtsUIAbility;
class ETSRuntime;

class EtsAbilityFFI {
public:
    EtsAbilityFFI(AbilityHandle ability, RuntimeHandle runtime);
    ~EtsAbilityFFI();

    static ETSAbilityFuncs *RegisterFuncs();
    static FFIHandle FFICreate(AbilityHandle ability, RuntimeHandle runtime);
    static void FFIDelete(FFIHandle ffi);
    static bool FFILoadModule(FFIHandle ffi, const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance);
    static bool FFIBindNativeMethods(FFIHandle ffi);
    static bool FFICreateEtsContext(FFIHandle ffi, int32_t screenMode,
        const std::shared_ptr<OHOS::AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<OHOS::AbilityRuntime::AbilityContext> &abilityContext);
    static void FFIOnStart(FFIHandle ffi, const OHOS::AAFwk::Want &want,
        const OHOS::AAFwk::LaunchParam &launch);
    static void FFIOnStop(FFIHandle ffi);
    static bool FFICallOnDestroy(FFIHandle ffi, OHOS::AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
        bool &isAsyncCallback);
    static void FFIOnWindowStageCreate(FFIHandle ffi);
    static void FFIOnWindowStageDestroy(FFIHandle ffi);
    static void FFIOnForeground(FFIHandle ffi, const OHOS::AAFwk::Want &want);
    static void FFIOnBackground(FFIHandle ffi);
    static void FFIOnNewWant(FFIHandle ffi, const OHOS::AAFwk::Want &want, const OHOS::AAFwk::LaunchParam &launch);
    static std::weak_ptr<OHOS::EtsEnv::ETSNativeReference> FFIGetAbilityObject(FFIHandle ffi);

    bool LoadModule(const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance);
    bool BindNativeMethods();
    bool CreateEtsContext(int32_t screenMode, const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContex);
    void OnStart(const AAFwk::Want &want, const AAFwk::LaunchParam &launch);
    void OnStop();
    bool CallOnDestroy(AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo, bool &isAsyncCallback);
    void OnWindowStageCreate();
    void OnWindowStageDestroy();
    void OnForeground(const AAFwk::Want &want);
    void OnBackground();
    void OnNewWant(const AAFwk::Want &want, const AAFwk::LaunchParam &launch);
    std::weak_ptr<OHOS::EtsEnv::ETSNativeReference> GetAbilityObject();

#ifdef SUPPORT_SCREEN
private:
    void UpdateEtsWindowStage(ani_ref windowStage);
    std::shared_ptr<EtsEnv::ETSNativeReference> etsWindowStageObj_;
#endif

private:
    bool CallObjectMethod(bool withResult, const char *name, const char *signature, ...);
    ani_object CreateAppWindowStage();
    AbilityRuntime::EtsUIAbility *ability_ = nullptr;
    AbilityRuntime::ETSRuntime *runtime_ = nullptr;
    std::shared_ptr<EtsEnv::ETSNativeReference> shellContextRef_;
    std::shared_ptr<EtsEnv::ETSNativeReference> etsAbilityObj_;
    static std::once_flag singletonFlag_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_ABILITY_FFI_H
