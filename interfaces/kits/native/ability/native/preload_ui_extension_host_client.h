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

#ifndef OHOS_ABILITY_RUNTIME_PRELOAD_UI_EXTENSION_HOST_CLIENT_H
#define OHOS_ABILITY_RUNTIME_PRELOAD_UI_EXTENSION_HOST_CLIENT_H

#include <map>
#include <memory>
#include <mutex>
#include "preload_ui_extension_destroy_callback_interface.h"
#include "preload_ui_extension_loaded_callback_interface.h"
#include "preload_ui_extension_callback_stub.h"

namespace OHOS {
namespace AbilityRuntime {
class PreloadUIExtensionHostClient : public AAFwk::PreloadUIExtensionCallbackStub {
public:
    PreloadUIExtensionHostClient() = default;
    virtual ~PreloadUIExtensionHostClient() = default;

    static sptr<PreloadUIExtensionHostClient> GetInstance();

    int32_t AddLoadedCallback(const std::shared_ptr<PreloadUIExtensionLoadedCallbackInterface> &callback, std::string bundleName);

    int32_t AddDestroyCallback(
        const std::shared_ptr<PreloadUIExtensionDestroyCallbackInterface> &callback, std::string bundleName);

    int32_t RemoveLoadedCallback(int32_t key, std::string bundleName);

    int32_t RemoveDestroyCallback(int32_t key, std::string bundleName);

    void RemoveAllLoadedCallback(std::string bundleName);

    void RemoveAllDestroyCallback(std::string bundleName);

    void OnLoadedDone(std::string bundleName, int32_t resultCode, int32_t extensionAbilityId) override;
    
    void OnDestroyDone(std::string bundleName, int32_t resultCode, int32_t extensionAbilityId) override;

private:
    static std::mutex instanceMutex_;
    static std::once_flag singletonFlag_;
    int32_t key_ = 0;
    static sptr<PreloadUIExtensionHostClient> instance_;
    mutable std::mutex preloadUIExtensionLoadedCallbackMutex_;
    mutable std::mutex preloadUIExtensionDestroyCallbackMutex_;
    std::map<std::string, std::map<int32_t, std::shared_ptr<PreloadUIExtensionLoadedCallbackInterface>>> loadedCallbackMap_;
    std::map<std::string, std::map<int32_t, std::shared_ptr<PreloadUIExtensionDestroyCallbackInterface>>> destroyCallbackMap_;
    DISALLOW_COPY_AND_MOVE(PreloadUIExtensionHostClient);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PRELOAD_UI_EXTENSION_HOST_CLIENT_H