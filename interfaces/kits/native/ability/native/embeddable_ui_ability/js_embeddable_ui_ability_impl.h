
/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_EMBEDDABLE_UI_ABILITY_IMPL_H
#define OHOS_ABILITY_RUNTIME_JS_EMBEDDABLE_UI_ABILITY_IMPL_H

#include "js_ui_extension_base.h"
#include "launch_param.h"
#include "runtime.h"
#include "session_info.h"
#include "ui_extension_context.h"
#include "want.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
class JsEmbeddableUIAbilityImpl : public JsUIExtensionBase {
public:
    explicit JsEmbeddableUIAbilityImpl(const std::unique_ptr<Runtime> &runtime);
    virtual ~JsEmbeddableUIAbilityImpl() override;

protected:
    void OnStart(const AAFwk::Want &want, AAFwk::LaunchParam &launchParam, sptr<AAFwk::SessionInfo> sessionInfo)
        override;

    void BindContext(const std::shared_ptr<AAFwk::Want> want) override;

    bool HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> sessionInfo) override;

    void DestroyWindow(const sptr<AAFwk::SessionInfo> sessionInfo) override;

private:
    std::unique_ptr<NativeReference> CreateAppWindowStage(sptr<Rosen::Window> uiWindow,
        sptr<AAFwk::SessionInfo> sessionInfo);
    sptr<Rosen::Window> CreateUIWindow(const std::shared_ptr<UIExtensionContext> context,
        const sptr<AAFwk::SessionInfo> &sessionInfo);

    int32_t screenMode_ = AAFwk::IDLE_SCREEN_MODE;
    std::shared_ptr<int32_t> screenModePtr_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_EMBEDDABLE_UI_ABILITY_IMPL_H
