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
#ifndef OHOS_ABILITY_RUNTIME_ETS_APP_SERVICE_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_ETS_APP_SERVICE_EXTENSION_CONTEXT_H

#include <memory>

#include "ability_connect_callback.h"
#include "app_service_extension_context.h"
#include "event_handler.h"
#include "ets_free_install_observer.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {

struct EtsConnectionKey {
    AAFwk::Want want;
    int32_t id = 0;
    int32_t accountId = 0;
};
struct EtsKeyCompare {
    bool operator()(const EtsConnectionKey &key1, const EtsConnectionKey &key2) const
    {
        return key1.id < key2.id;
    }
};

class EtsAppServiceExtensionContext final {
public:
    explicit EtsAppServiceExtensionContext(const std::shared_ptr<AppServiceExtensionContext> &context)
        : context_(context) {}
    ~EtsAppServiceExtensionContext() = default;

    static int32_t InsertConnection(std::shared_ptr<EtsAppServiceExtensionContext>connection,
        const AAFwk::Want &want, int32_t accountId = -1);
    static void RemoveConnection(int32_t connectId);
    static void Finalizer(ani_env *env, void *data, void *hint);
    void SetConnectionId(int32_t id);
    void RemoveConnectionObject();
    void CallEtsFailed(int32_t errorCode);
    void SetConnectionRef(ani_object connectOptionsObj);
    static void OnTerminateSelf(ani_env *env, ani_object obj, ani_object callback);
    static ani_int OnConnectAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object connectOptionsObj);
    static void OnDisconnectAbility(ani_env *env, ani_object aniObj, ani_int connectId, ani_object callback);
    static ani_object OnStartAbility(ani_env* env, ani_object aniObj, ani_object wantObj, ani_object optionsObj);
    static AppServiceExtensionContext *GetContext(ani_env *env, ani_object aniObj);
    static ani_object StartAbilityInner(ani_env* env, AAFwk::Want& want, AAFwk::StartOptions& startOptions,
        ani_object optionsObj);

private:
    std::weak_ptr<AppServiceExtensionContext> context_;
    ani_vm *etsVm_ = nullptr;
    int32_t connectionId_ = -1;
    ani_ref etsConnectionRef_ = nullptr;
};

ani_object CreateEtsAppServiceExtensionContext(ani_env *env,
    const std::shared_ptr<AppServiceExtensionContext> &context);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_SERVICE_EXTENSION_CONTEXT_H