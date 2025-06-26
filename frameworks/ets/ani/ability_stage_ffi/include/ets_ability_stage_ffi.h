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

#ifndef OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_FFI_H
#define OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_FFI_H

#include <string>

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AppExecFwk {
class OHOSApplication;
class Configuration;
}

namespace AAFwk {
class Want;
struct LaunchParam;
}

namespace AbilityRuntime {
class Context;
}
} // namespace OHOS

using FFIHandle = void *;
using StageHandle = void *;
using RuntimeHandle = void *;

extern "C" {
struct ETSAbilityStageFuncs {
    FFIHandle (*CreateEtsFFI)(StageHandle stage, RuntimeHandle runtime) = nullptr;
    void (*DeleteEtsFFI)(FFIHandle ffi) = nullptr;
    bool (*LoadModule)(FFIHandle ffi, const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance) = nullptr;
    void (*OnCreate)(FFIHandle ffi) = nullptr;
    void (*OnDestroy)(FFIHandle ffi) = nullptr;
    void (*OnConfigurationUpdated)(FFIHandle ffi, const std::shared_ptr<OHOS::AppExecFwk::Configuration> &fullConfig,
        const OHOS::AppExecFwk::Configuration &configuration) = nullptr;
    void (*SetEtsAbilityStage)(FFIHandle ffi, const std::shared_ptr<OHOS::AbilityRuntime::Context> &context) = nullptr;
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
class EtsAbilityStage;
class ETSRuntime;
class Context;

class EtsAbilityStageFFI {
public:
    EtsAbilityStageFFI(StageHandle stage, RuntimeHandle runtime);
    ~EtsAbilityStageFFI() {};

    static ETSAbilityStageFuncs *RegisterFuncs();
    static FFIHandle FFICreate(StageHandle stage, RuntimeHandle runtime);
    static void FFIDelete(FFIHandle ffi);
    static bool FFILoadModule(FFIHandle ffi, const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance);
    static void FFIOnCreate(FFIHandle ffi);
    static void FFIOnDestroy(FFIHandle ffi);
    static void FFIOnConfigurationUpdated(FFIHandle ffi,
        const std::shared_ptr<OHOS::AppExecFwk::Configuration> &fullConfig,
        const OHOS::AppExecFwk::Configuration &configuration);
    static void FFISetEtsAbilityStage(FFIHandle ffi, const std::shared_ptr<OHOS::AbilityRuntime::Context> &context);

    bool LoadModule(const std::string &moduleName, const std::string &modulePath,
        const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance);
    void OnCreate();
    void OnDestroy();
    void OnConfigurationUpdated(const std::shared_ptr<OHOS::AppExecFwk::Configuration> &fullConfig,
        const OHOS::AppExecFwk::Configuration &configuration);
    void SetEtsAbilityStage(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context);

private:
    bool CallObjectMethod(bool withResult, const char* name, const char* signature, ...) const;

    AbilityRuntime::EtsAbilityStage *stage_ = nullptr;
    AbilityRuntime::ETSRuntime *runtime_ = nullptr;
    std::shared_ptr<EtsEnv::ETSNativeReference> etsAbilityStageObj_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_ABILITY_STAGE_FFI_H
