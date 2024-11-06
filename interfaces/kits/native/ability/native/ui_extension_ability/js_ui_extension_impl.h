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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_IMPL_H
#define OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_IMPL_H

#include "js_ui_extension_base.h"
#include "runtime.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
class JsUIExtensionImpl : public JsUIExtensionBase {
public:
    explicit JsUIExtensionImpl(const std::unique_ptr<Runtime> &runtime);
    virtual ~JsUIExtensionImpl() override;

    sptr<IRemoteObject> OnConnect(const AAFwk::Want &want);
    void OnDisconnect(const AAFwk::Want &want);

private:
    napi_value CallOnConnect(const AAFwk::Want &want);
    napi_value CallOnDisconnect(const AAFwk::Want &want, bool withResult = false);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_IMPL_H
