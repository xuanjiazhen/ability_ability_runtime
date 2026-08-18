/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_INTERFACE_H

#include "ability_info.h"
#include "ability_manager_errors.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
struct AbilityInterceptorParam {
    Want want;
    int32_t requestCode = 0;
    int32_t userId = 0;
    bool isWithUI = false;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;  // resolved target abilityInfo passed to the interceptor
    bool isStartAsCaller = false;
    int32_t appIndex = 0;
    std::function<bool(void)> shouldBlockAllAppStartFunc_;
    bool isTargetPlugin = false; //Is it pulling up a plugin
    std::string hostBundleName;
    const StartOptions* startOptions = nullptr;
    bool fromConnect = false;
};

// Fluent builder: setters mutate an internal AbilityInterceptorParam directly, Build() moves it out.
// Defined after AbilityInterceptorParam so it can hold one by value.
class InterceptorParamBuilder {
public:
    InterceptorParamBuilder(const Want &want, int32_t requestCode, int32_t userId)
    {
        param_.want = want;
        param_.requestCode = requestCode;
        param_.userId = userId;
    }
    ~InterceptorParamBuilder() = default;
    InterceptorParamBuilder &WithUI(bool isWithUI) { param_.isWithUI = isWithUI; return *this; }
    InterceptorParamBuilder &CallerToken(const sptr<IRemoteObject> &callerToken) { param_.callerToken = callerToken; return *this; }
    InterceptorParamBuilder &ShouldBlockAllAppStart(const std::function<bool(void)> &func)
    { param_.shouldBlockAllAppStartFunc_ = func; return *this; }
    InterceptorParamBuilder &AbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
    { param_.abilityInfo = abilityInfo; return *this; }
    InterceptorParamBuilder &StartAsCaller(bool isStartAsCaller) { param_.isStartAsCaller = isStartAsCaller; return *this; }
    InterceptorParamBuilder &AppIndex(int32_t appIndex) { param_.appIndex = appIndex; return *this; }
    InterceptorParamBuilder &WithStartOptions(const StartOptions *startOptions) { param_.startOptions = startOptions; return *this; }
    InterceptorParamBuilder &HostBundleName(const std::string &hostBundleName) { param_.hostBundleName = hostBundleName; return *this; }
    InterceptorParamBuilder &IsTargetPlugin(bool isTargetPlugin) { param_.isTargetPlugin = isTargetPlugin; return *this; }
    InterceptorParamBuilder &FromConnect(bool fromConnect) { param_.fromConnect = fromConnect; return *this; }
    AbilityInterceptorParam Build() { return std::move(param_); }
private:
    AbilityInterceptorParam param_;
};

/**
 * @class IAbilityInterceptor
 * IAbilityInterceptor is used to intercept a different type of start request.
 */
class IAbilityInterceptor {
public:
    virtual ~IAbilityInterceptor() = default;

    /**
     * Excute interception processing.
     */
    virtual ErrCode DoProcess(const AbilityInterceptorParam &param) = 0;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_INTERFACE_H