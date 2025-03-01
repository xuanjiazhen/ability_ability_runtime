/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_STS_UI_EXTENSION_CONTENT_SESSION_H
#define OHOS_ABILITY_RUNTIME_STS_UI_EXTENSION_CONTENT_SESSION_H

#include "session_info.h"
#include "start_options.h"
#include "window.h"
#include "sts_runtime.h"

namespace OHOS {
namespace AbilityRuntime {

class StsUIExtensionContentSession {
private:
    class CallbackWrapper;
public:
    StsUIExtensionContentSession();
    // StsUIExtensionContentSession(sptr<AAFwk::SessionInfo> sessionInfo,
    //     sptr<Rosen::Window> uiWindow, std::weak_ptr<AbilityRuntime::Context>& context,
    //     std::shared_ptr<AbilityResultListeners>& abilityResultListeners);
    // StsUIExtensionContentSession(sptr<AAFwk::SessionInfo> sessionInfo,
    //     sptr<Rosen::Window> uiWindow);
    virtual ~StsUIExtensionContentSession() = default;
    static ani_object CreateStsUIExtensionContentSession(ani_env* env);

    void TerminateSelf();

protected:

private:
    sptr<AAFwk::SessionInfo> sessionInfo_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_STS_UI_EXTENSION_CONTENT_SESSION_H
