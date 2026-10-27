/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_EXTENSION_CONTEXT_H

#include "ani.h"
#include "auto_fill_extension_context.h"
#include "ets_ui_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsAutoFillExtensionContext final {
public:
    explicit EtsAutoFillExtensionContext(std::shared_ptr<AutoFillExtensionContext> context)
        : context_(context) {}
    virtual ~EtsAutoFillExtensionContext() = default;

    static ani_object SetEtsAutoFillExtensionContext(ani_env *env, std::shared_ptr<AutoFillExtensionContext> context);
    static EtsAutoFillExtensionContext *GetEtsAutoFillExtensionContext(ani_env *env, ani_object object);
    static void Clean(ani_env *env, ani_object object);
    static void ReloadInModal(ani_env *env, ani_object object, ani_object customDataObj, ani_object callback);
    static ani_long ConnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
        ani_object connectOptionsObj);
    static void DisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_long connectId,
        ani_object callback);
    static void CheckConnectionAbility(ani_env *env, ani_object aniObj);
    static ani_object CreateEtsAutoFillExtensionContext(ani_env *env,
        std::shared_ptr<AutoFillExtensionContext> context);

private:
    void OnReloadInModal(ani_env *env, ani_object object, ani_object customDataObj, ani_object callback);
    ani_long OnConnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
        ani_object connectOptionsObj);
    void OnDisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj, ani_long connectId,
        ani_object callback);
    std::weak_ptr<AutoFillExtensionContext> context_;
};

class EtsAutoFillExtensionConnection : public EtsUIExtensionConnection {
public:
    explicit EtsAutoFillExtensionConnection(ani_vm *etsVm) : EtsUIExtensionConnection(etsVm) {}
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_AUTO_FILL_EXTENSION_CONTEXT_H