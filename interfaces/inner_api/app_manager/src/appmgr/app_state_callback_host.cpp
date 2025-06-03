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

#include "app_state_callback_host.h"

#include "appexecfwk_errors.h"
#include "configuration.h"
#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
AppStateCallbackHost::AppStateCallbackHost() {}

AppStateCallbackHost::~AppStateCallbackHost() {}

ErrCode AppStateCallbackHost::OnAbilityRequestDone(const sptr<IRemoteObject> &, const AbilityState&)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return ERR_OK;
}

ErrCode AppStateCallbackHost::OnAppStateChanged(const AppProcessData &appProcessData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return ERR_OK;
}

ErrCode AppStateCallbackHost::NotifyAppPreCache(int32_t pid, int32_t userId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return ERR_OK;
}

ErrCode AppStateCallbackHost::NotifyConfigurationChange(const AppExecFwk::Configuration &config, int32_t userId)
{
    return ERR_OK;
}

ErrCode AppStateCallbackHost::NotifyStartResidentProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return ERR_OK;
}

ErrCode AppStateCallbackHost::NotifyStartKeepAliveProcess(const std::vector<AppExecFwk::BundleInfo> &bundleInfos)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return ERR_OK;
}

ErrCode AppStateCallbackHost::OnAppRemoteDied(const std::vector<sptr<IRemoteObject>> &abilityTokens)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return ERR_OK;
}

ErrCode AppStateCallbackHost::OnStartProcessFailed(const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return ERR_OK;
}

ErrCode AppStateCallbackHost::OnCacheExitInfo(uint32_t accessTokenId, const RunningProcessInfo &exitInfo,
    const std::string &bundleName, const std::vector<std::string> &abilityNames,
    const std::vector<std::string> &uiExtensionNames)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    return ERR_OK;
}
}  // namespace AppExecFwk
}  // namespace OHOS
