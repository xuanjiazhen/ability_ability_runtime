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

#ifndef OHOS_ABILITY_RUNTIME_NATIVE_THREAD_INFO_H
#define OHOS_ABILITY_RUNTIME_NATIVE_THREAD_INFO_H

#include <memory>
#include <string>
#include <thread>

typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;

#ifdef __cplusplus
extern "C" {
#endif
struct NativeAbilityWrapper {
    int32_t instanceId = 0;
    std::string abilityName;
    napi_env env = nullptr;
    napi_value jsAbilityObj_ = nullptr;
};

typedef void(*PostAbilityType)(const NativeAbilityWrapper* nativeAbilityWrapper);
typedef void(*OHMainType)();
#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace AAFwk {
struct NativeModuleMetaData;
}  // namespace AAFwk

namespace AppExecFwk {
class NativeThreadInfo {
public:
    NativeThreadInfo() = default;
    ~NativeThreadInfo();

    // Disable copy and move
    NativeThreadInfo(NativeThreadInfo &) = delete;
    NativeThreadInfo& operator=(NativeThreadInfo &) = delete;
    NativeThreadInfo(NativeThreadInfo &&) = delete;
    NativeThreadInfo& operator=(NativeThreadInfo &&) = delete;

    /**
     * @brief Load native module from the specified metadata.
     * @param metaData The native module metadata containing library path and function names.
     * @param bundleName Indicates native library path bundleName.
     * @param moduleName Indicates native library path moduleName.
     * @return true if loading succeeded, false otherwise.
     */
    bool LoadNativeModule(const AAFwk::NativeModuleMetaData& metaData, const std::string &bundleName,
        const std::string &moduleName);

    /**
     * @brief Run the main function in a new native thread.
     */
    void RunMain();

    /**
     * @brief Post the ability wrapper to the native thread.
     * @param nativeAbilityWrapper The pointer to the native ability wrapper.
     */
    void PostAbility(const NativeAbilityWrapper* nativeAbilityWrapper);

private:
    void *moduleHandle_ = nullptr;
    OHMainType OHMain_ = nullptr;
    PostAbilityType postAbilityFunc_ = nullptr;
    std::thread nativeThread_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NATIVE_THREAD_INFO_H