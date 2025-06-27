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

#ifndef OHOS_ABILITY_RUNTIME_ETS_SERVICE_EXTENSION_OBJECT_H
#define OHOS_ABILITY_RUNTIME_ETS_SERVICE_EXTENSION_OBJECT_H

#include <memory>

#include "ets_runtime.h"

using FFIHandle = void *;
using AbilityHandle = void *;
using RuntimeHandle = void *;

namespace OHOS {
class IRemoteObject;
template <typename T> class sptr;
namespace AppExecFwk {
class OHOSApplication;
template <typename> class AbilityTransactionCallbackInfo;
}

namespace AAFwk {
class Want;
struct LaunchParam;
}

namespace AbilityRuntime {
class AbilityContext;
class ETSServiceExtensionObject {
public:
    explicit ETSServiceExtensionObject() {}
    ~ETSServiceExtensionObject();

    bool Init(AbilityHandle ability, RuntimeHandle runtime);
    bool LoadModule(const std::string &moduleName, const std::string &modulePath, const std::string &hapPath,
        bool esmodule, bool useCommonChunk, const std::string &srcEntrance);
    void OnStart(const AAFwk::Want &want);
    void OnStop();
    sptr<IRemoteObject> OnConnect(const AAFwk::Want &want);
    sptr<IRemoteObject> OnConnectCallback(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback);
    void OnDisconnect(const AAFwk::Want &want);
    void OnDisconnectCallback(const AAFwk::Want &want,
        AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo, bool &isAsyncCallback);
    void OnCommand(const AAFwk::Want &want, int startId);

private:
    FFIHandle ffi_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ETS_SERVICE_EXTENSION_OBJECT_H
