/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_FORM_EDIT_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_FORM_EDIT_EXTENSION_H

#include "extension_base.h"
#include "form_provider_info.h"
#include "form_state_info.h"

namespace OHOS {
namespace AbilityRuntime {
class FormEditExtensionContext;
class Runtime;
/**
 * @brief Form edit extension components.
 */
class FormEditExtension : public ExtensionBase<FormEditExtensionContext> {
public:
    FormEditExtension() = default;
    virtual ~FormEditExtension() = default;

    /**
     * @brief Create Form Edit Extension.
     *
     * @param runtime The runtime.
     * @return The FormEditExtension instance.
     */
    static FormEditExtension* Create(const std::unique_ptr<Runtime>& runtime);

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_FORM_EDIT_EXTENSION_H
