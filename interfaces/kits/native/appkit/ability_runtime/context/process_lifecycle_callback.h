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

#ifndef OHOS_ABILITY_RUNTIME_PROCESS_LIFECYCLE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_PROCESS_LIFECYCLE_CALLBACK_H

#include <memory>
#include <set>
#include <string>

class NativeReference;
typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;

namespace OHOS {
namespace AbilityRuntime {
enum class ProcessLifecycleState : int32_t {
    CREATED = 0,
    EXITED = 1,
};

struct ProcessLifecycleData {
    int32_t pid = 0;
    std::string processName;
    int32_t uid = 0;
    ProcessLifecycleState state = ProcessLifecycleState::CREATED;
};

class ProcessLifecycleCallback {
public:
    virtual ~ProcessLifecycleCallback() {}

    /**
     * Called back when a process spawned by AMS is created.
     *
     * @param data Process lifecycle data containing pid, processName, uid and state.
     * @since 20
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     */
    virtual void OnProcessCreated(const ProcessLifecycleData &data) = 0;

    /**
     * Called back when a process spawned by AMS is exited (including crash).
     *
     * @param data Process lifecycle data containing pid, processName, uid and state.
     * @since 20
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     */
    virtual void OnProcessExited(const ProcessLifecycleData &data) = 0;
};

class JsProcessLifecycleCallback : public ProcessLifecycleCallback,
    public std::enable_shared_from_this<JsProcessLifecycleCallback> {
public:
    explicit JsProcessLifecycleCallback(napi_env env);
    virtual ~JsProcessLifecycleCallback() = default;
    void OnProcessCreated(const ProcessLifecycleData &data) override;
    void OnProcessExited(const ProcessLifecycleData &data) override;
    void Register(napi_value jsCallback);

    /**
     * @brief Unregister process lifecycle callback.
     * @param jsCallback, if jscallback is nullptr, delete all register jscallback.
     *                    or if jscallback is specified, delete prescribed jscallback.
     * @return Returns true on unregister success, others return false.
     */
    bool UnRegister(napi_value jsCallback = nullptr);
    bool IsEmpty() const;
private:
    void CallJsMethod(const std::string &methodName, const ProcessLifecycleData &data);
    napi_env env_ = nullptr;
    std::set<std::shared_ptr<NativeReference>> callbacks_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PROCESS_LIFECYCLE_CALLBACK_H
