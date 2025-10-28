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
#include "preload_ui_extension_host_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
sptr<PreloadUIExtensionHostClient> PreloadUIExtensionHostClient::instance_ = nullptr;
std::mutex PreloadUIExtensionHostClient::instanceMutex_;
std::once_flag PreloadUIExtensionHostClient::singletonFlag_;

sptr<PreloadUIExtensionHostClient> PreloadUIExtensionHostClient::GetInstance()
{
    std::call_once(singletonFlag_, []() {
        instance_ = new (std::nothrow) PreloadUIExtensionHostClient();
        if (instance_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null instance");
        }
    });
    return instance_;
}

int32_t PreloadUIExtensionHostClient::AddLoadedCallback(
    const std::shared_ptr<PreloadUIExtensionLoadedCallbackInterface> &callback, std::string bundleName)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
    int32_t key = ++key_;
    loadedCallbackMap_[bundleName][key] = callback;
    return key;
}

int32_t PreloadUIExtensionHostClient::AddDestroyCallback(
    const std::shared_ptr<PreloadUIExtensionDestroyCallbackInterface> &callback, std::string bundleName)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(preloadUIExtensionDestroyCallbackMutex_);
    int32_t key = ++key_;
    destroyCallbackMap_[bundleName][key] = callback;
    return key;
}

int32_t PreloadUIExtensionHostClient::RemoveLoadedCallback(int32_t key, std::string bundleName)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
    auto bundleIter = loadedCallbackMap_.find(bundleName);
    if (bundleIter != loadedCallbackMap_.end()) {
        auto callbackIter = bundleIter->second.find(key);
        if (callbackIter != bundleIter->second.end()) {
            bundleIter->second.erase(callbackIter);
            if (bundleIter->second.empty()) {
                loadedCallbackMap_.erase(bundleIter);
            }
            return ERR_OK;
        }
    }
    return ERR_INVALID_VALUE;
}

int32_t PreloadUIExtensionHostClient::RemoveDestroyCallback(int32_t key, std::string bundleName)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(preloadUIExtensionDestroyCallbackMutex_);
    auto bundleIter = destroyCallbackMap_.find(bundleName);
    if (bundleIter != destroyCallbackMap_.end()) {
        auto callbackIter = bundleIter->second.find(key);
        if (callbackIter != bundleIter->second.end()) {
            bundleIter->second.erase(callbackIter);
            if (bundleIter->second.empty()) {
                destroyCallbackMap_.erase(bundleIter);
            }
            return ERR_OK;
        }
    }
    return ERR_INVALID_VALUE;
}

void PreloadUIExtensionHostClient::RemoveAllLoadedCallback(std::string bundleName)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
    auto iter = loadedCallbackMap_.find(bundleName);
    if (iter != loadedCallbackMap_.end()) {
        loadedCallbackMap_.erase(bundleName);
    }
}

void PreloadUIExtensionHostClient::RemoveAllDestroyCallback(std::string bundleName)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(preloadUIExtensionDestroyCallbackMutex_);
    auto iter = destroyCallbackMap_.find(bundleName);
    if (iter != destroyCallbackMap_.end()) {
        destroyCallbackMap_.erase(bundleName);
    }
}

void PreloadUIExtensionHostClient::OnLoadedDone(std::string bundleName, int32_t resultCode, int32_t extensionAbilityId)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
    auto bundleIter = loadedCallbackMap_.find(bundleName);
    if (bundleIter != loadedCallbackMap_.end()) {
        for (auto& callbackPair : bundleIter->second) {
            if (callbackPair.second != nullptr) {
                callbackPair.second->ProcessOnLoadedDone(resultCode, extensionAbilityId);
            }
        }
    }
}


void PreloadUIExtensionHostClient::OnDestroyDone(std::string bundleName, int32_t resultCode, int32_t extensionAbilityId)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    std::lock_guard<std::mutex> lock(preloadUIExtensionDestroyCallbackMutex_);
    auto bundleIter = destroyCallbackMap_.find(bundleName);
    if (bundleIter != destroyCallbackMap_.end()) {
        for (auto& callbackPair : bundleIter->second) {
            if (callbackPair.second != nullptr) {
                callbackPair.second->ProcessOnDestroyDone(resultCode, extensionAbilityId);
            }
        }
    }
}
} // namespace AbilityRuntime
} // namespace OHOS