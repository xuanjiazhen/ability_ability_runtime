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

#ifndef OHOS_ABILITY_RUNTIME_ETS_SERVICE_EXTENSION_FFI_H
#define OHOS_ABILITY_RUNTIME_ETS_SERVICE_EXTENSION_FFI_H

#include <string>

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
class IRemoteObject;
template <typename T> class sptr;
namespace AppExecFwk {
class OHOSApplication;
template <typename> class AbilityTransactionCallbackInfo;
}

namespace AAFwk {
class Want;
struct LaunchParam;
}
} // namespace OHOS

using FFIHandle = void *;
using AbilityHandle = void *;
using RuntimeHandle = void *;

extern "C" {
struct ETSServiceExtensionFuncs {
    FFIHandle (*CreateEtsFFI)(AbilityHandle ability, RuntimeHandle runtime) = nullptr;
    void (*DeleteEtsFFI)(FFIHandle ffi) = nullptr;
    bool (*LoadModule)(FFIHandle ffi, const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance) = nullptr;
    void (*OnStart)(FFIHandle ffi, const OHOS::AAFwk::Want &want) = nullptr;
    void (*OnStop)(FFIHandle ffi) = nullptr;
    OHOS::sptr<OHOS::IRemoteObject> (*OnConnect)(FFIHandle ffi, const OHOS::AAFwk::Want &want) = nullptr;
    OHOS::sptr<OHOS::IRemoteObject> (*OnConnectCallback)(FFIHandle ffi, const OHOS::AAFwk::Want &want,
        OHOS::AppExecFwk::AbilityTransactionCallbackInfo<OHOS::sptr<OHOS::IRemoteObject>> *callbackInfo,
        bool &isAsyncCallback) = nullptr;
    void (*OnDisconnect)(FFIHandle ffi, const OHOS::AAFwk::Want &want) = nullptr;
    void (*OnDisconnectCallback)(FFIHandle ffi, const OHOS::AAFwk::Want &want,
        OHOS::AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
        bool &isAsyncCallback) = nullptr;
    void (*OnCommand)(FFIHandle ffi, const OHOS::AAFwk::Want &want, int startId) = nullptr;
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
class EtsServiceExtension;
class ETSRuntime;

class EtsServiceExtensionFFI {
public:
    EtsServiceExtensionFFI(AbilityHandle ability, RuntimeHandle runtime);
    ~EtsServiceExtensionFFI();

    static ETSServiceExtensionFuncs *RegisterFuncs();
    static FFIHandle FFICreate(AbilityHandle ability, RuntimeHandle runtime);
    static void FFIDelete(FFIHandle ffi);
    static bool FFILoadModule(FFIHandle ffi, const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance);
    static void FFIOnStart(FFIHandle ffi, const OHOS::AAFwk::Want &want);
    static void FFIOnStop(FFIHandle ffi);
    static OHOS::sptr<OHOS::IRemoteObject> FFIOnConnect(FFIHandle ffi, const OHOS::AAFwk::Want &want);
    static OHOS::sptr<OHOS::IRemoteObject> FFIOnConnectCallback(FFIHandle ffi, const OHOS::AAFwk::Want &want,
        OHOS::AppExecFwk::AbilityTransactionCallbackInfo<OHOS::sptr<OHOS::IRemoteObject>> *callbackInfo,
        bool &isAsyncCallback);
    static void FFIOnDisconnect(FFIHandle ffi, const OHOS::AAFwk::Want &want);
    static void FFIOnDisconnectCallback(FFIHandle ffi, const OHOS::AAFwk::Want &want,
        OHOS::AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
        bool &isAsyncCallback);
    static void FFIOnCommand(FFIHandle ffi, const OHOS::AAFwk::Want &want, int startId);
    static void DisconnectPromiseCallback(ani_env *env, ani_object aniObj);
    static void ConnectPromiseCallback(ani_env *env, ani_object aniObj, ani_object obj);

    bool LoadModule(const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance);
    void OnStart(const AAFwk::Want &want);
    void OnStop();
    sptr<IRemoteObject> OnConnect(const OHOS::AAFwk::Want &want);
    sptr<IRemoteObject> OnConnectCallback(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback);
    void OnDisconnect(const OHOS::AAFwk::Want &want);
    void OnDisconnectCallback(const AAFwk::Want &want, AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
        bool &isAsyncCallback);
    void OnCommand(const AAFwk::Want &want, int startId);

private:
    ani_ref CallObjectMethod(bool withResult, const char *name, const char *signature, ...);
    sptr<IRemoteObject> OnConnectInner(ani_env *env, ani_object &aniRemoteobj, bool &isAsyncCallback);
    AbilityRuntime::EtsServiceExtension *ability_ = nullptr;
    AbilityRuntime::ETSRuntime *runtime_ = nullptr;
    std::shared_ptr<EtsEnv::ETSNativeReference> etsAbilityObj_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_SERVICE_EXTENSION_FFI_H
