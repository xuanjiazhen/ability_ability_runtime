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

#ifndef OHOS_ABILITY_RUNTIME_APP_MGR_INNER_PARAM_H
#define OHOS_ABILITY_RUNTIME_APP_MGR_INNER_PARAM_H

#include "app_running_record.h"
#include "bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
struct StartProcessParam {
    const std::string &processName;
    uint32_t startFlags;
    std::shared_ptr<AppRunningRecord> appRecord;
    const int uid;
    const BundleInfo &bundleInfo;
    const std::string &bundleName;
    const int32_t bundleIndex;
    bool appExistFlag = true;
    bool isPreload = false;
    const std::string &moduleName;
    const std::string &abilityName;
    bool strictMode = false;
    int32_t maxChildProcess = 0;
    sptr<IRemoteObject> token = nullptr;
    std::shared_ptr<AAFwk::Want> want = nullptr;
    ExtensionAbilityType ExtensionAbilityType = ExtensionAbilityType::UNSPECIFIED;
};
struct LoadAbilityParam {
    const std::shared_ptr<AppRunningRecord> appRecord;
    bool isShellCall;
    std::shared_ptr<ApplicationInfo> appInfo;
    std::shared_ptr<AbilityInfo> abilityInfo;
    const std::string &processName;
    const std::string &specifiedProcessFlag;
    const BundleInfo &bundleInfo;
    const HapModuleInfo &hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    bool appExistFlag;
    bool isPreload;
    sptr<IRemoteObject> token = nullptr;
};
struct CreateStartMsgParam {
    const std::string &processName;
    uint32_t startFlags;
    const int uid;
    const BundleInfo &bundleInfo;
    const int32_t bundleIndex;
    BundleType bundleType;
    AppSpawnStartMsg &startMsg;
    std::shared_ptr<AAFwk::Want> want;
    const std::string &moduleName;
    const std::string &abilityName;
    bool strictMode = false;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_MGR_INNER_PARAM_H
