/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_H

#include "extension_base.h"

struct OH_AbilityRuntime_ModularObject_ExtensionContext;
struct OH_AbilityRuntime_ModularObject_ExtensionInstance;

namespace OHOS {
namespace AbilityRuntime {
class ModularObjectExtensionContext;
class Runtime;

class ModularObjectExtension : public ExtensionBase<ModularObjectExtensionContext> {
public:
    ModularObjectExtension() = default;
    ~ModularObjectExtension() override = default;

    std::shared_ptr<ModularObjectExtensionContext> CreateAndInitContext(
        const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    void Init(const std::shared_ptr<AbilityLocalRecord> &record,
        const std::shared_ptr<OHOSApplication> &application,
        std::shared_ptr<AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    static ModularObjectExtension* Create(const std::unique_ptr<Runtime> &runtime);

    void OnStart(const AAFwk::Want &want) override;

    void OnStop() override;

    sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;

    void OnDisconnect(const AAFwk::Want &want) override;

private:
    bool LoadNativeExtensionModule();
    bool BuildCWant(const AAFwk::Want &want, AbilityBase_Want &cWant, AbilityBase_Element &element) const;
    static bool BuildElement(const AppExecFwk::ElementName &elementName, AbilityBase_Element &element);
    static void DestroyElement(AbilityBase_Element &element);

    std::shared_ptr<OH_AbilityRuntime_ModularObject_ExtensionInstance> moeInstance_;
    std::shared_ptr<OH_AbilityRuntime_ModularObject_ExtensionContext> moeContext_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_MODULAR_OBJECT_EXTENSION_H
