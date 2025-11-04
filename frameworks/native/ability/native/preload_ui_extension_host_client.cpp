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
            TAG_LOGE(AAFwkTag::UI_EXT, "null instance");
        }
    });
    return instance_;
}

int32_t PreloadUIExtensionHostClient::AddLoadedCallback(
    const std::shared_ptr<PreloadUIExtensionLoadedCallbackInterface> &callback, std::string bundleName,
    int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
    auto pidAndBundleName = std::make_pair(callerPid, bundleName);
    int32_t key = ++key_;
    loadedCallbackMap_[pidAndBundleName][key] = callback;
    return key;
}

int32_t PreloadUIExtensionHostClient::AddDestroyCallback(
    const std::shared_ptr<PreloadUIExtensionDestroyCallbackInterface> &callback, std::string bundleName,
    int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionDestroyCallbackMutex_);
    auto pidAndBundleName = std::make_pair(callerPid, bundleName);
    int32_t key = ++key_;
    destroyCallbackMap_[pidAndBundleName][key] = callback;
    return key;
}

int32_t PreloadUIExtensionHostClient::RemoveLoadedCallback(int32_t key, std::string bundleName, int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
    auto pidAndBundleName = std::make_pair(callerPid, bundleName);
    auto outerIter = loadedCallbackMap_.find(pidAndBundleName);
    if (outerIter == loadedCallbackMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "No callback map found for PID: %{public}d, bundle: %{public}s", callerPid,
            bundleName.c_str());
        return ERR_INVALID_VALUE;
    }
    auto &callbackMap = outerIter->second;
    auto callbackIter = callbackMap.find(key);
    if (callbackIter == callbackMap.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "No callback found with key: %{public}d", key);
        return ERR_INVALID_VALUE;
    }
    callbackMap.erase(callbackIter);
    if (callbackMap.empty()) {
        loadedCallbackMap_.erase(outerIter);
        TAG_LOGD(AAFwkTag::UI_EXT, "Removed empty callback map for PID: %{public}d, bundle: %{public}s", callerPid,
            bundleName.c_str());
    }
    return ERR_OK;
}

int32_t PreloadUIExtensionHostClient::RemoveDestroyCallback(int32_t key, std::string bundleName, int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionDestroyCallbackMutex_);
    auto pidAndBundleName = std::make_pair(callerPid, bundleName);
    auto outerIter = destroyCallbackMap_.find(pidAndBundleName);
    if (outerIter == destroyCallbackMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "No callback map found for PID: %{public}d, bundle: %{public}s", callerPid,
            bundleName.c_str());
        return ERR_INVALID_VALUE;
    }
    auto &callbackMap = outerIter->second;
    auto callbackIter = callbackMap.find(key);
    if (callbackIter == callbackMap.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "No callback found with key: %{public}d", key);
        return ERR_INVALID_VALUE;
    }
    callbackMap.erase(callbackIter);
    if (callbackMap.empty()) {
        destroyCallbackMap_.erase(outerIter);
        TAG_LOGE(AAFwkTag::UI_EXT, "Removed empty callback map for PID: %{public}d, bundle: %{public}s", callerPid,
            bundleName.c_str());
    }
    return ERR_OK;
}

void PreloadUIExtensionHostClient::RemoveAllLoadedCallback(std::string bundleName, int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
    auto pidAndBundleName = std::make_pair(callerPid, bundleName);
    auto outerIter = loadedCallbackMap_.find(pidAndBundleName);
    if (outerIter == loadedCallbackMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "No callbacks found for PID: %{public}d, bundle: %{public}s", callerPid,
            bundleName.c_str());
        return;
    }
    loadedCallbackMap_.erase(outerIter);
}

void PreloadUIExtensionHostClient::RemoveAllDestroyCallback(std::string bundleName, int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(preloadUIExtensionDestroyCallbackMutex_);
    auto pidAndBundleName = std::make_pair(callerPid, bundleName);
    auto outerIter = destroyCallbackMap_.find(pidAndBundleName);
    if (outerIter == destroyCallbackMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "No callbacks found for PID: %{public}d, bundle: %{public}s", callerPid,
            bundleName.c_str());
        return;
    }
    destroyCallbackMap_.erase(outerIter);
}

void PreloadUIExtensionHostClient::OnLoadedDone(int32_t hostPid, std::string bundleName, int32_t extensionAbilityId)
{
    std::vector<std::shared_ptr<PreloadUIExtensionLoadedCallbackInterface>> callbacksToInvoke;
    {
        std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(hostPid, bundleName);
        auto iter = loadedCallbackMap_.find(key);
        if (iter != loadedCallbackMap_.end()) {
            for (auto &callbackPair : iter->second) {
                if (callbackPair.second != nullptr) {
                    callbacksToInvoke.push_back(callbackPair.second);
                }
            }
        }
    }
    for (auto &callback : callbacksToInvoke) {
        if (callback != nullptr) {
            callback->ProcessOnLoadedDone(extensionAbilityId);
        }
    }
}

void PreloadUIExtensionHostClient::OnDestroyDone(int32_t hostPid, std::string bundleName, int32_t extensionAbilityId)
{
    std::vector<std::shared_ptr<PreloadUIExtensionDestroyCallbackInterface>> callbacksToInvoke;
    {
        std::lock_guard<std::mutex> lock(preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(hostPid, bundleName);
        auto iter = destroyCallbackMap_.find(key);
        if (iter != destroyCallbackMap_.end()) {
            for (auto &callbackPair : iter->second) {
                if (callbackPair.second != nullptr) {
                    callbacksToInvoke.push_back(callbackPair.second);
                }
            }
        }
    }
    for (auto &callback : callbacksToInvoke) {
        if (callback != nullptr) {
            callback->ProcessOnDestroyDone(extensionAbilityId);
        }
    }
}
} // namespace AbilityRuntime
} // namespace OHOS