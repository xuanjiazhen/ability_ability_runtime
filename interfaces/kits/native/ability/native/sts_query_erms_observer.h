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

#ifndef OHOS_ABILITY_RUNTIME_STS_QUERY_ERMS_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_STS_QUERY_ERMS_OBSERVER_H

#include <memory>
#include <string>
#include <vector>

#include "sts_runtime.h"
#include "sts_error_utils.h"
#include "query_erms_observer_stub.h"
namespace OHOS {
namespace AbilityRuntime {
struct StsQueryERMSObserverObject {
    std::string appId;
    std::string startTime;
    ani_object callBack;
};

class StsQueryERMSObserver : public QueryERMSObserverStub {
public:
    explicit StsQueryERMSObserver(ani_vm *etsVm);
    ~StsQueryERMSObserver();
    void OnQueryFinished(const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule, int32_t resultCode) override;
    void AddStsObserverObject(ani_env *env, const std::string &appId, const std::string &startTime, ani_object callBack);
private:
    void CallCallback(ani_object callback, const AtomicServiceStartupRule &rule, int32_t resultCode);
    void HandleOnQueryFinished(const std::string &appId, const std::string &startTime,
        const AtomicServiceStartupRule &rule, int32_t resultCode);
    napi_value CreateStsAtomicServiceStartupRule(napi_env env, const AbilityRuntime::AtomicServiceStartupRule &rule);
    ani_vm *etsVm_;
    std::mutex stsObserverObjectListLock_;
    std::vector<StsQueryERMSObserverObject> stsObserverObjectList_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_STS_QUERY_ERMS_OBSERVER_H