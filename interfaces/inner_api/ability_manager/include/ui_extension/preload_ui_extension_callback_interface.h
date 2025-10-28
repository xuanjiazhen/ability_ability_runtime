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

#ifndef OHOS_ABILITY_RUNTIME_PRELOAD_UI_EXTENSION_CALLBACK_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_PRELOAD_UI_EXTENSION_CALLBACK_INTERFACE_H

#include "iremote_broker.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class IInsightIntentExecuteCallback
 * IInsightIntentExecuteCallback is used to notify caller ability that intent execute is complete.
 */
class IPreloadUIExtensionCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.AAFwk.IPreloadUIExtensionCallback");

    virtual void OnLoadedDone(std::string bundleName, int32_t resultCode, int32_t extensionAbilityId) = 0;
    virtual void OnDestroyDone(std::string bundleName, int32_t resultCode, int32_t extensionAbilityId) = 0;

    enum {
        ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE = 1,
        ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE = 2,
    };
};
} // namespace AAFwk
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_CALLBACK_INTERFACE_H
