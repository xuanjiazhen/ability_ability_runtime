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
#ifndef OHOS_ABILITY_RUNTIME_STS_UI_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_STS_UI_EXTENSION_CONTEXT_H

#include "sts_runtime.h"
#include <array>
#include <iostream>
#include <unistd.h>
#include "service_extension.h"
#include "service_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ohos_application.h"
#include "sts_free_install_observer.h"

namespace OHOS {
namespace AbilityRuntime {
class StsServiceExtensionConnection;
bool BindNativeMethods(ani_env *env, ani_class &cls);
[[maybe_unused]] static void TerminateSelfSync([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object obj,
    [[maybe_unused]] ani_object callback);
[[maybe_unused]] static void StartAbility([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object aniObj,
    ani_object wantObj, ani_object call);
[[maybe_unused]] static void StartAbilityWithOption([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object aniObj,
    ani_object wantObj, ani_object opt, ani_object call);
[[maybe_unused]] static void StartServiceExtensionAbilitySync([[maybe_unused]] ani_env *env,
    [[maybe_unused]] ani_object obj, [[maybe_unused]] ani_object wantObj, [[maybe_unused]] ani_object callbackobj);
ani_object CreateStsServiceExtensionContext(ani_env *env, std::shared_ptr<ServiceExtensionContext> context,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application);

void StsCreatExtensionContext(ani_env* aniEnv, ani_class contextClass, ani_object contextObj,
    void* applicationCtxRef, std::shared_ptr<ExtensionContext> context);

void BindExtensionInfo(ani_env* aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context, std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo);

class StsServiceExtensionContext final {
public:
    static StsServiceExtensionContext &GetInstance()
    {
        static StsServiceExtensionContext instance;
        return instance;
    }
    void StartAbilityInner([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object aniObj,
        ani_object wantObj, ani_object opt, ani_object call);
    static ServiceExtensionContext* GetAbilityContext(ani_env *env, ani_object obj);
    void AddFreeInstallObserver(ani_env *env, const AAFwk::Want &want, ani_object callback,
        ServiceExtensionContext* context);
    static bool CheckConnectionParam([[maybe_unused]]ani_env *env, ani_object connectOptionsObj,
        sptr<StsServiceExtensionConnection>& connection, AAFwk::Want& want, int32_t accountId = -1);
    static ani_int ConnectServiceExtensionAbility([[maybe_unused]]ani_env *env, [[maybe_unused]]ani_object aniObj,
        ani_object wantObj, ani_object connectOptionsObj);
    static void DisconnectServiceExtensionAbilitySync([[maybe_unused]]ani_env *env, [[maybe_unused]]ani_object aniObj,
        ani_int connectId, ani_object connectOptionsObj);
private:
    sptr<StsFreeInstallObserver> freeInstallObserver_ = nullptr;
};

class StsServiceExtensionConnection : public AbilityConnectCallback {
public:
    explicit StsServiceExtensionConnection(ani_vm *etsVm);
    ~StsServiceExtensionConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void CallStsFailed(int32_t errorCode);
    void SetConnectionId(int64_t id);
    int64_t GetConnectionId() { return connectionId_; }
    void SetConnectionRef(ani_object connectOptionsObj);
protected:
    ani_vm *etsVm_;
    int64_t connectionId_ = -1;
    ani_ref stsConnectionRef_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STS_UI_EXTENSION_CONTEXT_H