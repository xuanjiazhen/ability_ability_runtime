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

#include "ets_ui_extension_object.h"

#include <cstddef>
#include <dlfcn.h>

#include "ets_ui_extension_ffi.h"
#include "hilog_tag_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const char *ETS_FFI_LIBNAME = "libui_extension_ani.z.so";
const char *ETS_FFI_REGISTER_FUNCS = "OHOS_ETS_UIExtension_RegisterFuncs";
ETSUIExtensionFuncs *g_etsFFIFuncs = nullptr;

bool RegisterETSUIExtensionFuncs()
{
    if (g_etsFFIFuncs != nullptr) {
        return true;
    }
    auto handle = dlopen(ETS_FFI_LIBNAME, RTLD_NOW);
    if (!handle) {
        TAG_LOGE(AAFwkTag::UI_EXT, "dlopen failed %{public}s, %{public}s", ETS_FFI_LIBNAME, dlerror());
        return false;
    }
    auto symbol = dlsym(handle, ETS_FFI_REGISTER_FUNCS);
    if (!symbol) {
        TAG_LOGE(AAFwkTag::UI_EXT, "dlsym failed %{public}s, %{public}s", ETS_FFI_REGISTER_FUNCS, dlerror());
        dlclose(handle);
        return false;
    }
    auto func = reinterpret_cast<ETSUIExtensionFuncs *(*)()>(symbol);
    g_etsFFIFuncs = func();
    return true;
}
} // namespace

bool ETSUIExtensionObject::Init(AbilityHandle ability, RuntimeHandle runtime)
{
    if (!RegisterETSUIExtensionFuncs()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "RegisterETSUIExtensionFuncs failed");
        return false;
    }

    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->CreateEtsFFI == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or CreateEtsFFI");
        return false;
    }
    ffi_ = g_etsFFIFuncs->CreateEtsFFI(ability, runtime);
    return true;
}

ETSUIExtensionObject::~ETSUIExtensionObject()
{
    if (ffi_ != nullptr) {
        if (g_etsFFIFuncs == nullptr ||
            g_etsFFIFuncs->DeleteEtsFFI == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or DeleteEtsFFI");
            return;
        }
        g_etsFFIFuncs->DeleteEtsFFI(ffi_);
    }
}

bool ETSUIExtensionObject::LoadModule(const std::string &moduleName, const std::string &modulePath,
    const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->LoadModule == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or LoadModule");
        return false;
    }
    return g_etsFFIFuncs->LoadModule(ffi_, moduleName, modulePath, hapPath,
        esmodule, useCommonChunk, srcEntrance);
}

void ETSUIExtensionObject::OnStart(const AAFwk::Want &want, const AAFwk::LaunchParam &launch)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnStart == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or OnStart");
        return;
    }
    g_etsFFIFuncs->OnStart(ffi_, want, launch);
}

void ETSUIExtensionObject::OnStop()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnStop == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or OnStop");
        return;
    }
    g_etsFFIFuncs->OnStop(ffi_);
}
    
bool ETSUIExtensionObject::CallOnDestroy(AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
    bool &isAsyncCallback)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->CallOnDestroy == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or CallOnDestroy");
        return false;
    }
    return g_etsFFIFuncs->CallOnDestroy(ffi_, callbackInfo, isAsyncCallback);
}

void ETSUIExtensionObject::OnForeground()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnForeground == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or OnForeground");
        return;
    }
    g_etsFFIFuncs->OnForeground(ffi_);
}

void ETSUIExtensionObject::OnBackground()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnBackground == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or OnBackground");
        return;
    }
    g_etsFFIFuncs->OnBackground(ffi_);
}

bool ETSUIExtensionObject::OnWindowStageCreate(const AAFwk::Want &want, const std::shared_ptr<Context> &context,
    const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnWindowStageCreate == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or OnWindowStageCreate");
        return false;
    }
    return g_etsFFIFuncs->OnWindowStageCreate(ffi_, want, context, sessionInfo, uiWindow);
}

void ETSUIExtensionObject::OnSessionDestroy()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnSessionDestroy == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or OnSessionDestroy");
        return;
    }
    g_etsFFIFuncs->OnSessionDestroy(ffi_);
}

void ETSUIExtensionObject::BindContext(const std::shared_ptr<AAFwk::Want> &want,
    const std::shared_ptr<UIExtensionContext> &context)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->BindContext == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or BindContext");
        return;
    }
    g_etsFFIFuncs->BindContext(ffi_, want, context);
}

void ETSUIExtensionObject::OnAbilityResult(int requestCode, int resultCode, const AAFwk::Want &resultData)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnAbilityResult == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or OnAbilityResult");
        return;
    }
    g_etsFFIFuncs->OnAbilityResult(ffi_, requestCode, resultCode, resultData);
}

void ETSUIExtensionObject::RemoveListener(const uint64_t &componentId)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->RemoveListener == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null g_etsFFIFuncs or RemoveListener");
        return;
    }
    g_etsFFIFuncs->RemoveListener(ffi_, componentId);
}
} // namespace AbilityRuntime
} // namespace OHOS