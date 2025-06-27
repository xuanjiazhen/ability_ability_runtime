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

#include "ets_stage_object.h"

#include <cstddef>
#include <dlfcn.h>

#include "ets_ability_stage_ffi.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const char *ETS_FFI_LIBNAME = "libability_stage_ani.z.so";
const char *ETS_FFI_REGISTER_FUNCS = "OHOS_ETS_Ability_Stage_RegisterFuncs";
ETSAbilityStageFuncs *g_etsFFIFuncs = nullptr;

bool RegisterETSAbilityStageFuncs()
{
    if (g_etsFFIFuncs != nullptr) {
        return true;
    }
    auto handle = dlopen(ETS_FFI_LIBNAME, RTLD_NOW);
    if (!handle) {
        TAG_LOGE(AAFwkTag::APPKIT, "dlopen failed %{public}s, %{public}s", ETS_FFI_LIBNAME, dlerror());
        return false;
    }
    auto symbol = dlsym(handle, ETS_FFI_REGISTER_FUNCS);
    if (!symbol) {
        TAG_LOGE(AAFwkTag::APPKIT, "dlsym failed %{public}s, %{public}s", ETS_FFI_REGISTER_FUNCS, dlerror());
        dlclose(handle);
        return false;
    }
    auto func = reinterpret_cast<ETSAbilityStageFuncs *(*)()>(symbol);
    g_etsFFIFuncs = func();
    return true;
}
} // namespace

ETSAbilityStageObject::~ETSAbilityStageObject()
{
    if (ffi_ != nullptr) {
        if (g_etsFFIFuncs == nullptr ||
            g_etsFFIFuncs->DeleteEtsFFI == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null g_etsFFIFuncs or DeleteEtsFFI");
            return;
        }
        g_etsFFIFuncs->DeleteEtsFFI(ffi_);
    }
}

bool ETSAbilityStageObject::Init(StageHandle ability, RuntimeHandle runtime)
{
    if (!RegisterETSAbilityStageFuncs()) {
        TAG_LOGE(AAFwkTag::APPKIT, "RegisterETSAbilityStageFuncs failed");
        return false;
    }

    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->CreateEtsFFI == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null g_etsFFIFuncs or CreateEtsFFI");
        return false;
    }
    ffi_ = g_etsFFIFuncs->CreateEtsFFI(ability, runtime);
    return true;

}

bool ETSAbilityStageObject::LoadModule(const std::string &moduleName, const std::string &modulePath,
    const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->LoadModule == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null g_etsFFIFuncs or LoadModule");
        return false;
    }
    return g_etsFFIFuncs->LoadModule(ffi_, moduleName, modulePath, hapPath,
        esmodule, useCommonChunk, srcEntrance);
}

void ETSAbilityStageObject::OnCreate()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnCreate == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null g_etsFFIFuncs or OnStart");
        return;
    }
    g_etsFFIFuncs->OnCreate(ffi_);
}

void ETSAbilityStageObject::OnDestroy()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnDestroy == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null g_etsFFIFuncs or OnDestroy");
        return;
    }
    g_etsFFIFuncs->OnDestroy(ffi_);
}

void ETSAbilityStageObject::OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration> &fullConfig,
    const AppExecFwk::Configuration &configuration)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnConfigurationUpdated == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null g_etsFFIFuncs or OnConfigurationUpdated");
        return;
    }
    g_etsFFIFuncs->OnConfigurationUpdated(ffi_, fullConfig, configuration);
}

void ETSAbilityStageObject::SetEtsAbilityStage(const std::shared_ptr<Context> &context)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->SetEtsAbilityStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null g_etsFFIFuncs or SetEtsAbilityStage");
        return;
    }
    g_etsFFIFuncs->SetEtsAbilityStage(ffi_, context);
}
} // namespace AbilityRuntime
} // namespace OHOS