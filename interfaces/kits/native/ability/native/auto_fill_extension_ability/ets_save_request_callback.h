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

#ifndef OHOS_ABILITY_RUNTIME_ETS_SAVE_REQUEST_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_ETS_SAVE_REQUEST_CALLBACK_H

#include "ets_auto_fill_extension_util.h"
#include "session_info.h"
#include "sts_runtime.h"
#include "want.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsSaveRequestCallback {
public:
    EtsSaveRequestCallback() = default;
    virtual ~EtsSaveRequestCallback() = default;

    static EtsSaveRequestCallback &GetInstance();
    static ani_object SetSessionInfoAndWindow(ani_env *env, const sptr<AAFwk::SessionInfo> &sessionInfo,
        const sptr<Rosen::Window> &uiWindow);
    static void SetField(ani_env *env, ani_object object, ani_class cls, ani_field field,
        const sptr<AAFwk::SessionInfo> &sessionInfo, const sptr<Rosen::Window> &uiWindow);
    static sptr<AAFwk::SessionInfo> GetSessionInfo(ani_env *env, ani_object object);
    static sptr<Rosen::Window> GetWindow(ani_env *env, ani_object object);
    static ani_object CreateEtsSaveRequestCallback(ani_env *env, const sptr<AAFwk::SessionInfo> &sessionInfo,
        const sptr<Rosen::Window> &uiWindow);
    static void SaveRequestSuccess(ani_env *env, ani_object object);
    static void SaveRequestFailed(ani_env *env, ani_object object);

private:
    void OnSaveRequestSuccess(ani_env *env, ani_object object);
    void OnSaveRequestFailed(ani_env *env, ani_object object);
    void SendResultCodeAndViewData(ani_env *env, ani_object object,
        const EtsAutoFillExtensionUtil::AutoFillResultCode &resultCode);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_SAVE_REQUEST_CALLBACK_H