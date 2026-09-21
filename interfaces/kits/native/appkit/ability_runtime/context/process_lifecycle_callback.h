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

#ifndef OHOS_ABILITY_RUNTIME_PROCESS_LIFECYCLE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_PROCESS_LIFECYCLE_CALLBACK_H

#include <cstdint>
#include <memory>
#include <string>

class NativeReference;
typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;

namespace OHOS {
namespace AbilityRuntime {
struct ProcessLifecycleInfo {
    int32_t pid = 0;
    std::string processName;
    int32_t uid = 0;
    std::string state; // "CREATED" or "EXITED"
};

class ProcessLifecycleCallback {
public:
    virtual ~ProcessLifecycleCallback() {}

    virtual void OnProcessCreated(const ProcessLifecycleInfo &info) = 0;
    virtual void OnProcessExited(const ProcessLifecycleInfo &info) = 0;
};

class JsProcessLifecycleCallback : public ProcessLifecycleCallback,
    public std::enable_shared_from_this<JsProcessLifecycleCallback> {
public:
    explicit JsProcessLifecycleCallback(napi_env env, napi_value callback);
    virtual ~JsProcessLifecycleCallback() = default;

    void OnProcessCreated(const ProcessLifecycleInfo &info) override;
    void OnProcessExited(const ProcessLifecycleInfo &info) override;

private:
    void CallJsMethod(const ProcessLifecycleInfo &info, const std::string &methodName);
    napi_env env_ = nullptr;
    std::shared_ptr<NativeReference> callback_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_PROCESS_LIFECYCLE_CALLBACK_H
