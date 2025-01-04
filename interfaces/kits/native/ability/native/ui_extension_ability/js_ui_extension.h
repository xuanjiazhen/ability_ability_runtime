/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_H

#include "ui_extension.h"
#include "js_runtime.h"
#include "js_ui_extension_impl.h"

namespace OHOS {
namespace AbilityRuntime {
class JsUIExtension : public UIExtension {
public:
    explicit JsUIExtension(const std::unique_ptr<Runtime> &runtime);
    virtual ~JsUIExtension() override;

    /**
     * @brief Create JsUIExtension.
     *
     * @param runtime The runtime.
     * @return The JsUIExtension instance.
     */
    static JsUIExtension* Create(const std::unique_ptr<Runtime>& runtime);

    /**
     * @brief Called when this ui extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the ui extension.
     * @return Returns a pointer to the <b>sid</b> of the connected ui extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when all abilities connected to this ui extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     *
     */
    virtual void OnDisconnect(const AAFwk::Want &want) override;

private:
    napi_value CallOnConnect(const AAFwk::Want &want);

    napi_value CallOnDisconnect(const AAFwk::Want &want, bool withResult = false);

    std::shared_ptr<JsUIExtensionImpl> impl_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_H
