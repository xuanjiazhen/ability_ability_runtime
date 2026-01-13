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

#ifndef OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_H
#define OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_H

#include "agent_extension.h"
#include "configuration.h"

namespace OHOS {
namespace AbilityRuntime {
class AgentExtension;
class JsRuntime;
class JsUIExtensionBase;
/**
 * @brief Basic agent extension components.
 */
class JsAgentExtension : public AgentExtension, public std::enable_shared_from_this<JsAgentExtension> {
public:
    explicit JsAgentExtension(const std::unique_ptr<Runtime> &runtime);
    virtual ~JsAgentExtension() override;

    /**
     * @brief Create JsAgentExtension.
     *
     * @param runtime The runtime.
     * @return The JsAgentExtension instance.
     */
    static JsAgentExtension *Create(const std::unique_ptr<Runtime> &runtime);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_JS_AGENT_EXTENSION_H
