/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_STS_EXTENSION_COMMON_H
#define OHOS_ABILITY_RUNTIME_STS_EXTENSION_COMMON_H

#include "sts_runtime.h"
#include "configuration.h"
#include "service_extension.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class ServiceExtension;
class STSRuntime;
/**
 * @brief Basic sts extension common components.
 */
class StsExtensionCommon : public ExtensionCommon,
                            public std::enable_shared_from_this<StsExtensionCommon> {
public:
    StsExtensionCommon(STSRuntime &stsRuntime, STSNativeReference &stsObj,
        const std::shared_ptr<STSNativeReferenceWrapper> &shellContextRef);

    virtual ~StsExtensionCommon() override;

    /**
     * @brief Create StsServiceExtension.
     *
     * @param stsRuntime The runtime.
     * @param stsObj The sts object instance.
     * @return The StsServiceExtension instance.
     */
    static std::shared_ptr<StsExtensionCommon> Create(STSRuntime &jsRuntime, STSNativeReference &stsObj,
        const std::shared_ptr<STSNativeReferenceWrapper> &shellContextRef);

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration> &fullConfig) override;

    /**
     * @brief Notify current memory level.
     *
     * @param level Current memory level.
     */
    virtual void OnMemoryLevel(int level) override;

private:
    std::shared_ptr<NativeReference> shellContextRef_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_STS_EXTENSION_COMMON_H
