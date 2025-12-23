/*
 * Copyright (c) 2025  Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_PAGE_CONFIG_MANAGER_H
#define OHOS_ABILITY_RUNTIME_PAGE_CONFIG_MANAGER_H

#include <mutex>
#include <string>

#include "nocopyable.h"
#include "window.h"

namespace OHOS::AbilityRuntime {
class PageConfigManager final : public NoCopyable {
public:
    static PageConfigManager &GetInstance();

    void Initialize(std::string configJson, const sptr<Rosen::Window>& window);

    int32_t NotifyPageChanged(char* targetPageName, uint32_t windowId);

private:
    PageConfigManager() = default;
    ~PageConfigManager();

    void LoadPageConfigSo();

    mutable std::mutex g_loadsoMutex;
    void* pageConfigSo_ = nullptr;
    bool isInitialized_ = false;
};
} // namespace AbilityRuntime

#endif // OHOS_ABILITY_RUNTIME_PAGE_CONFIG_MANAGER_H
