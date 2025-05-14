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

#ifndef OHOS_ABILITY_RUNTIME_ETS_ENVIRONMENT_H
#define OHOS_ABILITY_RUNTIME_ETS_ENVIRONMENT_H

#include <functional>
#include <memory>
#include <string>
#include <uv.h>

#include "event_handler.h"
#include "ets_environment_impl.h"
#include "ets_envsetup.h"
#include "ani.h"
#include "napi/native_api.h"

namespace OHOS {
struct ETSRuntimeAPI;
using TaskFuncType = void (*)();
namespace EtsEnv {
class EtsEnvironmentImpl;
class ETSEnvironment final {
public:
    explicit ETSEnvironment(std::unique_ptr<EtsEnvironmentImpl> impl);

    bool IsRuntimeStarted()
    {
        return isRuntimeStarted_;
    }

    static void InitETSChipSDKNS(const std::string& path);
    static void InitETSAppNS(const std::string& path);
    static void InitETSSDKNS(const std::string& path);
    static void InitETSSysNS(const std::string& path);
    bool StartRuntime(napi_env napiEnv, std::vector<ani_option>& options);
    void StopRuntime();
    void RegisterUncaughtExceptionHandler(const ETSUncaughtExceptionInfo& handle);
    bool IsUISchedulerStarted()
    {
        return isUISchedulerStarted_;
    }
    bool StartUIScheduler();
    void StopUIScheduler();
    enum LibraryKind {
        SYSTEM,
        SDK,
        APP,
    };
    void* GetUIScheduler()
    {
        if (!isUISchedulerStarted_) {
            return nullptr;
        }
        return uiScheduler_;
    }

    void* LoadETSLibrary(const char* dlName);
    void UnLoadETSLibrary(void* handle);

    bool StartDebugger();
    bool PostTask(TaskFuncType task);
    void PostTask(const std::function<void()>& task, const std::string& name = "", int64_t delayTime = 0);
    void PostSyncTask(const std::function<void()>& task, const std::string& name);
    void RemoveTask(const std::string& name);
    bool InitLoop(bool isStage);
    void DeInitLoop();
    bool ReInitUVLoop();
    void ReInitEtsEnvImpl(std::unique_ptr<EtsEnvironmentImpl> impl);
    ani_env* GetAniEnv();
    void HandleUncaughtError();

    static const char* etsAppNSName;
    static const char* etsSDKNSName;
    static const char* etsSysNSName;
    static const char* etsChipSDKNSName;

    struct VMEntry {
        int vmKind;
        void* app;
        void* enter;
        void* emitEvent;
        ani_vm *ani_vm;
        ani_env *ani_env;
        VMEntry()
        {
            vmKind = 0;
            app = nullptr;
            enter = nullptr;
            ani_vm = nullptr;
            ani_env = nullptr;
            emitEvent = nullptr;
        }
    };

private:
    bool LoadRuntimeApis();
    bool LoadSymbolCreateVM(void* handle, ETSRuntimeAPI& apis);
    bool LoadSymbolANIGetCreatedVMs(void* handle, ETSRuntimeAPI& apis);
    bool LoadBootPathFile(std::string& bootfiles);
    void Schedule();
    std::string GetBuildId(std::string stack);
    EtsEnv::ETSErrorObject GetETSErrorObject();
    std::string GetErrorProperty(ani_error aniError, const char* property);
    static ETSRuntimeAPI lazyApis_;
    bool isRuntimeStarted_{ false };
    bool isUISchedulerStarted_{ false };
    void* uiScheduler_{ nullptr };
    VMEntry vmEntry_;
    std::unique_ptr<EtsEnvironmentImpl> impl_ = nullptr;
    ETSUncaughtExceptionInfo uncaughtExceptionInfo_;
};
} // namespace EtsEnv
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_ENVIRONMENT_H
