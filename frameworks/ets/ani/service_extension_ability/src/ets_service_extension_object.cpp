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

#include "ets_service_extension_object.h"

#include <cstddef>
#include <dlfcn.h>

#include "ets_service_extension_ffi.h"
#include "hilog_tag_wrapper.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
const char *ETS_FFI_LIBNAME = "libservice_extension_ani.z.so";
const char *ETS_FFI_REGISTER_FUNCS = "OHOS_ETS_Service_Extension_RegisterFuncs";
ETSServiceExtensionFuncs *g_etsFFIFuncs = nullptr;

bool RegisterETSAbilityFuncs()
{
    if (g_etsFFIFuncs != nullptr) {
        return true;
    }
    auto handle = dlopen(ETS_FFI_LIBNAME, RTLD_NOW);
    if (!handle) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "dlopen failed %{public}s, %{public}s", ETS_FFI_LIBNAME, dlerror());
        return false;
    }
    auto symbol = dlsym(handle, ETS_FFI_REGISTER_FUNCS);
    if (!symbol) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "dlsym failed %{public}s, %{public}s", ETS_FFI_REGISTER_FUNCS, dlerror());
        dlclose(handle);
        return false;
    }
    auto func = reinterpret_cast<ETSServiceExtensionFuncs *(*)()>(symbol);
    g_etsFFIFuncs = func();
    return true;
}
} // namespace

bool ETSServiceExtensionObject::Init(AbilityHandle ability, RuntimeHandle runtime)
{
    if (!RegisterETSAbilityFuncs()) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "RegisterETSAbilityFuncs failed");
        return false;
    }

    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->CreateEtsFFI == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null g_etsFFIFuncs or CreateEtsFFI");
        return false;
    }
    ffi_ = g_etsFFIFuncs->CreateEtsFFI(ability, runtime);
    return true;
}

ETSServiceExtensionObject::~ETSServiceExtensionObject()
{
    if (ffi_ != nullptr) {
        if (g_etsFFIFuncs == nullptr ||
            g_etsFFIFuncs->DeleteEtsFFI == nullptr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "null g_etsFFIFuncs or DeleteEtsFFI");
            return;
        }
        g_etsFFIFuncs->DeleteEtsFFI(ffi_);
    }
}

bool ETSServiceExtensionObject::LoadModule(const std::string &moduleName, const std::string &modulePath,
    const std::string &hapPath, bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->LoadModule == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null g_etsFFIFuncs or LoadModule");
        return false;
    }
    return g_etsFFIFuncs->LoadModule(ffi_, moduleName, modulePath, hapPath,
        esmodule, useCommonChunk, srcEntrance);
}

void ETSServiceExtensionObject::OnStart(const AAFwk::Want &want)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnStart == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null g_etsFFIFuncs or OnStart");
        return;
    }
    g_etsFFIFuncs->OnStart(ffi_, want);
}

void ETSServiceExtensionObject::OnStop()
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnStop == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null g_etsFFIFuncs or OnStop");
        return;
    }
    g_etsFFIFuncs->OnStop(ffi_);
}

sptr<IRemoteObject> ETSServiceExtensionObject::OnConnect(const AAFwk::Want &want)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnConnect == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null g_etsFFIFuncs or OnConnect");
        return nullptr;
    }
    return g_etsFFIFuncs->OnConnect(ffi_, want);
}

sptr<IRemoteObject> ETSServiceExtensionObject::OnConnectCallback(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnConnectCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null g_etsFFIFuncs or OnConnectCallback");
        return nullptr;
    }
    return g_etsFFIFuncs->OnConnectCallback(ffi_, want, callbackInfo, isAsyncCallback);
}

void ETSServiceExtensionObject::OnDisconnect(const AAFwk::Want &want)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnDisconnect == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null g_etsFFIFuncs or OnDisconnect");
        return;
    }
    g_etsFFIFuncs->OnDisconnect(ffi_, want);
}

void ETSServiceExtensionObject::OnDisconnectCallback(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo, bool &isAsyncCallback)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnDisconnectCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null g_etsFFIFuncs or OnDisconnectCallback");
        return;
    }
    g_etsFFIFuncs->OnDisconnectCallback(ffi_, want, callbackInfo, isAsyncCallback);
}

void ETSServiceExtensionObject::OnCommand(const AAFwk::Want &want, int startId)
{
    if (g_etsFFIFuncs == nullptr ||
        g_etsFFIFuncs->OnCommand == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null g_etsFFIFuncs or OnCommand");
        return;
    }
    g_etsFFIFuncs->OnCommand(ffi_, want, startId);
}
} // namespace AbilityRuntime
} // namespace OHOS