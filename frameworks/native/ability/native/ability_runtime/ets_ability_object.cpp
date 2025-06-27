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

#include "ets_ability_object.h"

#include <cstddef>
#include <dlfcn.h>

#include "ets_ability_ffi.h"
#include "hilog_tag_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const char *ETS_FFI_LIBNAME = "libui_ability_ani.z.so";
const char *ETS_FFI_REGISTER_FUNCS = "OHOS_ETS_Ability_RegisterFuncs";
ETSAbilityFuncs *g_etsFFIFuncs = nullptr;

bool RegisterETSAbilityFuncs()
{
    if (g_etsFFIFuncs != nullptr) {
        return true;
    }
    auto handle = dlopen(ETS_FFI_LIBNAME, RTLD_NOW);
    if (!handle) {
        TAG_LOGE(AAFwkTag::UIABILITY, "dlopen failed %{public}s, %{public}s", ETS_FFI_LIBNAME, dlerror());
        return false;
    }
    auto symbol = dlsym(handle, ETS_FFI_REGISTER_FUNCS);
    if (!symbol) {
        TAG_LOGE(AAFwkTag::UIABILITY, "dlsym failed %{public}s, %{public}s", ETS_FFI_REGISTER_FUNCS, dlerror());
        dlclose(handle);
        return false;
    }
    auto func = reinterpret_cast<ETSAbilityFuncs *(*)()>(symbol);
    g_etsFFIFuncs = func();
    return true;
}
} // namespace

bool ETSAbilityObject::Init(AbilityHandle ability, RuntimeHandle runtime)
{
    if (!RegisterETSAbilityFuncs()) {
        TAG_LOGE(AAFwkTag::UIABILITY, "RegisterETSAbilityFuncs failed");
        return false;
    }

    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->CreateEtsFFI == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or CreateEtsFFI");
        return false;
    }
    ffi_ = g_etsFFIFuncs->CreateEtsFFI(ability, runtime);
    return true;
}

ETSAbilityObject::~ETSAbilityObject()
{
    if (ffi_ != nullptr) {
        if (g_etsFFIFuncs == nullptr ||
            g_etsFFIFuncs->DeleteEtsFFI == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or DeleteEtsFFI");
            return;
        }
        g_etsFFIFuncs->DeleteEtsFFI(ffi_);
    }
}

bool ETSAbilityObject::LoadModule(const std::string &moduleName, const std::string &modulePath,
    const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->LoadModule == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or LoadModule");
        return false;
    }
    return g_etsFFIFuncs->LoadModule(ffi_, moduleName, modulePath, hapPath,
        esmodule, useCommonChunk, srcEntrance);
}

bool ETSAbilityObject::BindNativeMethods()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->BindNativeMethods == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or BindNativeMethods");
        return false;
    }
    return g_etsFFIFuncs->BindNativeMethods(ffi_);
}

void ETSAbilityObject::CreateEtsContext(int32_t screenMode,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    const std::shared_ptr<AbilityRuntime::AbilityContext> &abilityContext)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->CreateEtsContext == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or CreateEtsContext");
        return;
    }
    g_etsFFIFuncs->CreateEtsContext(ffi_, screenMode, application, abilityContext);
}

void ETSAbilityObject::OnStart(const AAFwk::Want &want, const AAFwk::LaunchParam &launch)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnStart == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or OnStart");
        return;
    }
    g_etsFFIFuncs->OnStart(ffi_, want, launch);
}

void ETSAbilityObject::OnStop()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnStop == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or OnStop");
        return;
    }
    g_etsFFIFuncs->OnStop(ffi_);
}
    
bool ETSAbilityObject::CallOnDestroy(AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
    bool &isAsyncCallback)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->CallOnDestroy == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or CallOnDestroy");
        return false;
    }
    return g_etsFFIFuncs->CallOnDestroy(ffi_, callbackInfo, isAsyncCallback);
}

void ETSAbilityObject::OnWindowStageCreate()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnWindowStageCreate == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or OnWindowStageCreate");
        return;
    }
    g_etsFFIFuncs->OnWindowStageCreate(ffi_);
}

void ETSAbilityObject::OnWindowStageDestroy()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnWindowStageDestroy == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or OnWindowStageDestroy");
        return;
    }
    g_etsFFIFuncs->OnWindowStageDestroy(ffi_);
}

void ETSAbilityObject::OnForeground(const AAFwk::Want &want)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnForeground == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or OnForeground");
        return;
    }
    g_etsFFIFuncs->OnForeground(ffi_, want);
}

void ETSAbilityObject::OnBackground()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnBackground == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or OnBackground");
        return;
    }
    g_etsFFIFuncs->OnBackground(ffi_);
}

void ETSAbilityObject::OnNewWant(const AAFwk::Want &want, const AAFwk::LaunchParam &launch)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnNewWant == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or OnNewWant");
        return;
    }
    g_etsFFIFuncs->OnNewWant(ffi_, want, launch);
}

std::weak_ptr<EtsEnv::ETSNativeReference> ETSAbilityObject::GetAbilityObject()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->GetAbilityObject == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null g_etsFFIFuncs or GetAbilityObject");
        return {};
    }
    return g_etsFFIFuncs->GetAbilityObject(ffi_);
}
} // namespace AbilityRuntime
} // namespace OHOS