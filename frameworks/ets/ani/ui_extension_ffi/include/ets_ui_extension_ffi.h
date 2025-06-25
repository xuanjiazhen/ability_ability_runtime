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

#ifndef OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_FFI_H
#define OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_FFI_H

#include <string>

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
template <typename T> class sptr;
namespace AppExecFwk {
class OHOSApplication;
template <typename> class AbilityTransactionCallbackInfo;
class Configuration;
}

namespace AAFwk {
class Want;
struct LaunchParam;
class SessionInfo;
}

namespace Rosen {
class Window;
}

namespace AbilityRuntime {
class Context;
class UIExtensionContext;
}
} // namespace OHOS

using FFIHandle = void *;
using AbilityHandle = void *;
using RuntimeHandle = void *;

extern "C" {
struct ETSUIExtensionFuncs {
    FFIHandle (*CreateEtsFFI)(AbilityHandle ability, RuntimeHandle runtime) = nullptr;
    void (*DeleteEtsFFI)(FFIHandle ffi) = nullptr;
    bool (*LoadModule)(FFIHandle ffi, const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance) = nullptr;
    void (*OnStart)(FFIHandle ffi, const OHOS::AAFwk::Want &want, const OHOS::AAFwk::LaunchParam &launch) = nullptr;
    void (*OnStop)(FFIHandle ffi) = nullptr;
    bool (*CallOnDestroy)(FFIHandle ffi, OHOS::AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
        bool &isAsyncCallback) = nullptr;
    void (*OnForeground)(FFIHandle ffi) = nullptr;
    void (*OnBackground)(FFIHandle ffi) = nullptr;
    bool (*OnWindowStageCreate)(FFIHandle ffi, const OHOS::AAFwk::Want &want,
        const std::shared_ptr<OHOS::AbilityRuntime::Context> &context,
        const OHOS::sptr<OHOS::AAFwk::SessionInfo> &sessionInfo,
        const OHOS::sptr<OHOS::Rosen::Window> &uiWindow) = nullptr;
    void (*OnSessionDestroy)(FFIHandle ffi) = nullptr;
    void (*BindContext)(FFIHandle ffi, const std::shared_ptr<OHOS::AAFwk::Want> &want,
        const std::shared_ptr<OHOS::AbilityRuntime::UIExtensionContext> &context) = nullptr;
    void (*OnAbilityResult)(FFIHandle ffi, int requestCode, int resultCode,
        const OHOS::AAFwk::Want &resultData) = nullptr;
    void (*RemoveListener)(FFIHandle ffi, const uint64_t &componentId) = nullptr;
};
}

class __ani_ref;
using ani_ref = __ani_ref *;
class __ani_object;
using ani_object = __ani_object *;
struct __ani_env;
using ani_env = __ani_env;

namespace OHOS {
namespace EtsEnv {
struct ETSNativeReference;
}

namespace AbilityRuntime {
class ETSRuntime;
class UIExtension;
class EtsUIExtension;
class EtsAbilityResultListener;
class EtsAbilityResultListeners;
class EtsUIExtensionContentSession;
class EtsUIExtensionFFI {
public:
    EtsUIExtensionFFI(AbilityHandle ability, RuntimeHandle runtime);
    ~EtsUIExtensionFFI();

    static ETSUIExtensionFuncs *RegisterFuncs();
    static FFIHandle FFICreate(AbilityHandle ability, RuntimeHandle runtime);
    static void FFIDelete(FFIHandle ffi);
    static bool FFILoadModule(FFIHandle ffi, const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance);
    static void FFIOnStart(FFIHandle ffi, const OHOS::AAFwk::Want &want, const OHOS::AAFwk::LaunchParam &launch);
    static void FFIOnStop(FFIHandle ffi);
    static bool FFICallOnDestroy(FFIHandle ffi, OHOS::AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
        bool &isAsyncCallback);
    static void FFIOnForeground(FFIHandle ffi);
    static void FFIOnBackground(FFIHandle ffi);
    static bool FFIOnWindowStageCreate(FFIHandle ffi, const AAFwk::Want &want,
        const std::shared_ptr<OHOS::AbilityRuntime::Context> &context,
        const OHOS::sptr<OHOS::AAFwk::SessionInfo> &sessionInfo,
        const OHOS::sptr<OHOS::Rosen::Window> &uiWindow);
    static void FFIOnSessionDestroy(FFIHandle ffi);
    static void FFIBindContext(FFIHandle ffi, const std::shared_ptr<OHOS::AAFwk::Want> &want,
        const std::shared_ptr<UIExtensionContext> &context);
    static void FFIOnAbilityResult(FFIHandle ffi, int requestCode, int resultCode, const OHOS::AAFwk::Want &resultData);
    static void FFIRemoveListener(FFIHandle ffi, const uint64_t &componentId);

    bool LoadModule(const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance);
    void OnStart(const AAFwk::Want &want, const AAFwk::LaunchParam &launch);
    void OnStop();
    bool CallOnDestroy(AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo, bool &isAsyncCallback);
    void OnForeground();
    void OnBackground();
    bool OnWindowStageCreate(const AAFwk::Want &want, const std::shared_ptr<OHOS::AbilityRuntime::Context> &context,
        const OHOS::sptr<OHOS::AAFwk::SessionInfo> &sessionInfo, const OHOS::sptr<OHOS::Rosen::Window> &uiWindow);
    void OnSessionDestroy();
    void BindContext(const std::shared_ptr<OHOS::AAFwk::Want> &want,
        const std::shared_ptr<UIExtensionContext> &context);
    void OnAbilityResult(int requestCode, int resultCode, const OHOS::AAFwk::Want &resultData);
    void RemoveListener(const uint64_t &componentId);

private:

    static void PromiseCallback(ani_env* env, ani_object aniObj);
    ani_object CreateETSContext(ani_env *env,
        std::shared_ptr<UIExtensionContext> context, int32_t screenMode);
    bool CallObjectMethod(bool withResult, const char *name, const char *signature, ...);
    AbilityRuntime::EtsUIExtension *ability_ = nullptr;
    AbilityRuntime::ETSRuntime *runtime_ = nullptr;
    std::shared_ptr<EtsEnv::ETSNativeReference> etsUIExtensionObj_ = nullptr;
    std::shared_ptr<EtsEnv::ETSNativeReference> shellContextRef_ = nullptr;
    ani_ref contentSession_ = nullptr;
    std::shared_ptr<EtsAbilityResultListeners> abilityResultListeners_ = nullptr;
    std::shared_ptr<EtsUIExtensionContentSession> etsUiExtContentSession_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_UI_EXTENSION_FFI_H
