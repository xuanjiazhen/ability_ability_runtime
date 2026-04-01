/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "native_thread_info.h"

#include "hilog_tag_wrapper.h"
#include "module_manager/native_module_manager.h"
#include "native_module_util.h"

namespace OHOS {
namespace AppExecFwk {
constexpr const char* DEFAULT_NAMESPACE = "default";
NativeThreadInfo::~NativeThreadInfo()
{
    // Clean up resources
    if (nativeThread_.joinable()) {
        TAG_LOGW(AAFwkTag::ABILITY, "Native thread is still running, forcing detach");
        nativeThread_.detach();
    }

    if (moduleHandle_ != nullptr) {
        dlclose(moduleHandle_);
        moduleHandle_ = nullptr;
        TAG_LOGI(AAFwkTag::ABILITY, "Native module unloaded");
    }
}

bool NativeThreadInfo::LoadNativeModule(const AAFwk::NativeModuleMetaData& metaData,
    const std::string &bundleName, const std::string &moduleName)
{
    if (metaData.nativeModuleSource.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Native module source is empty");
        return false;
    }

    if (metaData.nativeModuleFunc.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Native module function is empty");
        return false;
    }

    // Load the dynamic library
    std::string bundleModuleName = bundleName + "/" + moduleName;
    moduleHandle_ = OpenNativeLibrary(bundleModuleName, metaData.nativeModuleSource);
    if (moduleHandle_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to load native module: %{public}s-%{public}s",
            bundleModuleName.c_str(), metaData.nativeModuleSource.c_str());
        return false;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "Native module loaded: %{public}s", metaData.nativeModuleSource.c_str());

    dlerror(); // clear error
    // Find the OHMain function
    OHMain_ = reinterpret_cast<OHMainType>(dlsym(moduleHandle_, metaData.nativeModuleFunc.c_str()));
    if (OHMain_ == nullptr) {
        char* error = dlerror();
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to find OHMain function: %{public}s, error: %{public}s",
            metaData.nativeModuleFunc.c_str(), error ? error : "null");
        dlclose(moduleHandle_);
        moduleHandle_ = nullptr;
        return false;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "OHMain function found: %{public}s", metaData.nativeModuleFunc.c_str());

    // Find the PostAbility function (optional but recommended)
    postAbilityFunc_ = reinterpret_cast<PostAbilityType>(dlsym(moduleHandle_, "PostAbility"));
    if (postAbilityFunc_ == nullptr) {
        char* error = dlerror();
        TAG_LOGW(AAFwkTag::ABILITY, "PostAbility function not found (optional): %{public}s",
            error ? error : "null");
    } else {
        TAG_LOGI(AAFwkTag::ABILITY, "PostAbility function found");
    }

    return true;
}

void NativeThreadInfo::RunMain()
{
    if (OHMain_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "OHMain function is null, cannot run native thread");
        return;
    }

    // Capture OHMain_ by value to avoid capturing this
    OHMainType mainFunc = OHMain_;
    nativeThread_ = std::thread([mainFunc]() {
        TAG_LOGI(AAFwkTag::ABILITY, "Native thread started");

        // Call the main function
        mainFunc();

        TAG_LOGI(AAFwkTag::ABILITY, "Native thread finished");
    });

    // Detach the thread to allow it to run independently
    // The thread will be cleaned up in the destructor
    TAG_LOGI(AAFwkTag::ABILITY, "Native thread created and detached");
}

void NativeThreadInfo::PostAbility(const NativeAbilityWrapper* nativeAbilityWrapper)
{
    if (nativeAbilityWrapper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "NativeAbilityWrapper is null");
        return;
    }

    if (postAbilityFunc_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "PostAbility function is null");
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "Posting ability to native thread: %{public}s (instanceId: %{public}d)",
        nativeAbilityWrapper->abilityName.c_str(), nativeAbilityWrapper->instanceId);

    // Call the PostAbility function from the native module
    postAbilityFunc_(nativeAbilityWrapper);
}

LIBHANDLE NativeThreadInfo::OpenNativeLibrary(const std::string& bundleModuleName, const std::string& fileName)
{
    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager == nullptr) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "null moduleManager");
        return nullptr;
    }
    std::string namespaceName;
    if (!moduleManager->GetLdNamespaceName(bundleModuleName, namespaceName)) {
        if (!moduleManager->GetLdNamespaceName(DEFAULT_NAMESPACE, namespaceName)) {
            TAG_LOGE(AAFwkTag::JSRUNTIME, "GetLdNamespaceName failed");
            return nullptr;
        }
    }
    Dl_namespace ns;
    if (dlns_get(namespaceName.data(), &ns) != 0) {
        TAG_LOGE(AAFwkTag::JSRUNTIME, "namespaceName not found: %{public}s", namespaceName.c_str());
        return nullptr;
    }
    LIBHANDLE nativeHandle = nullptr;
    nativeHandle = dlopen_ns(&ns, fileName.c_str(), RTLD_LAZY);
    return nativeHandle;
}
} // namespace AppExecFwk
} // namespace OHOS
