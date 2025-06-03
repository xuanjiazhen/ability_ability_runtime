/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_STATE_CALLBACK_HOST_H
#define OHOS_ABILITY_RUNTIME_APP_STATE_CALLBACK_HOST_H

#include "nocopyable.h"
#include "string_ex.h"
#include "app_mgr_constants.h"
#include "bundle_info.h"
#include "app_state_callback_stub.h"

namespace OHOS {
namespace AppExecFwk {
class AppStateCallbackHost : public AppStateCallbackStub {
public:
    AppStateCallbackHost();
    virtual ~AppStateCallbackHost();

    /**
     * AbilityMgr's request is done.
     *
     * @param token Ability token.
     * @param state Application state.
     */
    virtual ErrCode OnAbilityRequestDone(const sptr<IRemoteObject> &, const AbilityState) override;

    /**
     * Application state changed callback.
     *
     * @param appProcessData Process data
     */
    virtual ErrCode OnAppStateChanged(const AppProcessData &appProcessData) override;

    /**
     * @brief Notify application update system environment changes.
     * @param config System environment change parameters.
     * @param userId userId Designation User ID.
     */
    virtual ErrCode NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId) override;

    /**
     * @brief Notify abilityms start resident process.
     * @param bundleInfos resident process bundle infos.
     */
    virtual ErrCode NotifyStartResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) override;

    /**
     * @brief Notify abilityms start keep-alive process.
     * @param bundleInfos resident process bundle infos.
     */
    virtual ErrCode NotifyStartKeepAliveProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos) override;

    /**
     * @brief Notify abilityms app process OnRemoteDied
     * @param abilityTokens abilities in died process.
     */
    virtual ErrCode OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens) override;

    virtual ErrCode OnStartProcessFailed(const sptr<IRemoteObject> &token) override;

    /**
     * @brief Notify abilityms app process pre cache
     * @param pid process pid.
     * @param userId userId Designation User ID.
     */
    virtual ErrCode NotifyAppPreCache(int32_t pid, int32_t userId) override;

    virtual ErrCode OnCacheExitInfo(uint32_t accessTokenId, const RunningProcessInfo &exitInfo,
        const std::string &bundleName, const std::vector<std::string> &abilityNames,
        const std::vector<std::string> &uiExtensionNames) override;

private:

    DISALLOW_COPY_AND_MOVE(AppStateCallbackHost);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_STATE_CALLBACK_HOST_H
