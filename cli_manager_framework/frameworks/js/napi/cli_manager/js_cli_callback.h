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

#ifndef OHOS_ABILITY_RUNTIME_JS_CLI_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_CLI_CALLBACK_H

#include <memory>

#include "cli_manager_data.h"
#include "native_engine/native_reference.h"

namespace OHOS {
namespace AbilityRuntime {

class CliCallbackNapi final : public std::enable_shared_from_this<CliCallbackNapi> {
public:
    CliCallbackNapi(napi_env env, napi_value jsCallback);
    ~CliCallbackNapi();

    bool IsValid() const;
    int32_t DispatchEvent(const ToolEvent &event);

private:
    void CallJs(const ToolEvent &event);
    void FreeNativeReference(std::unique_ptr<NativeReference> &reference);

    napi_env env_ = nullptr;
    std::unique_ptr<NativeReference> callback_ = nullptr;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_CLI_CALLBACK_H
