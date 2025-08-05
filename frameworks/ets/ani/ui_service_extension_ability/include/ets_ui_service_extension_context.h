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

#ifndef OHOS_ABILITY_RUNTIME_ETS_UI_SERVICE_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ETS_UI_SERVICE_EXTENSION_CONTEXT_H

#include <memory>

#include "ability_connect_callback.h"
#include "event_handler.h"
#include "sts_free_install_observer.h"
#include "sts_runtime.h"
#include "ui_service_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
ani_object CreateEtsUIServiceExtensionContext(ani_env *env, std::shared_ptr<UIServiceExtensionContext> context);

class EtsUIServiceExtensionConnection : public AbilityConnectCallback {
public:
    explicit EtsUIServiceExtensionConnection(ani_env *env);
    virtual ~EtsUIServiceExtensionConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void SetEtsConnectionObject(ani_object etsConnectionObject);
    void RemoveConnectionObject();
    void CallEtsFailed(int32_t errorCode);
    void SetConnectionId(int64_t id);
    int64_t GetConnectionId();
private:
    ani_env* env_ = nullptr;
    std::unique_ptr<STSNativeReference> etsConnectionObject_ = nullptr;
    int64_t connectionId_ = -1;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ETS_UI_SERVICE_EXTENSION_CONTEXT_H