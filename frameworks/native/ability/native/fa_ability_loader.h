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

#ifndef OHOS_ABILITY_RUNTIME_FA_ABILITY_LOADER_H
#define OHOS_ABILITY_RUNTIME_FA_ABILITY_LOADER_H

#include <memory>
#include <string>
#include "ability_thread.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief FA能力动态加载器
 * 用于在需要FA模型时动态加载libabilitykit_fa.so
 */
class FAAbilityLoader {
public:
    /**
     * @brief 加载FA模型库并创建FAAbilityThread
     * @param libraryPath FA库路径，默认为"libabilitykit_fa.so"
     * @return 成功返回FAAbilityThread指针，失败返回nullptr
     */
    static sptr<AbilityThread> LoadFAAbilityThread(
        const std::string& libraryPath = "libabilitykit_fa.so");

    /**
     * @brief 卸载FA模型库
     */
    static void UnloadFALibrary();

private:
    static void* faLibraryHandle_;
    static bool LoadFALibrary(const std::string& libraryPath);
    static void UnloadFALibraryInternal();
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_FA_ABILITY_LOADER_H
