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
    std::string processName;
    uint32_t startFlags = 0;
    std::shared_ptr<AppRunningRecord> appRecord;
    int uid = 0;
    BundleInfo bundleInfo;
    std::string bundleName;
    int32_t bundleIndex = 0;
    bool appExistFlag = true;
    bool isPreload = false;
    std::string moduleName;
    std::string abilityName;
    bool strictMode = false;
    int32_t maxChildProcess = 0;
    sptr<IRemoteObject> token;
    std::shared_ptr<AAFwk::Want> want;
    ExtensionAbilityType extensionAbilityType = ExtensionAbilityType::UNSPECIFIED;
};
struct LoadAbilityParam {
    std::shared_ptr<AppRunningRecord> appRecord;
    bool isShellCall = 0;
    std::shared_ptr<ApplicationInfo> appInfo;
    std::shared_ptr<AbilityInfo> abilityInfo;
    std::string processName;
    std::string specifiedProcessFlag;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    bool appExistFlag = false;
    bool isPreload = false;
    sptr<IRemoteObject> token;
};
struct CreateStartMsgParam {
    std::string processName;
    uint32_t startFlags = 0;
    int uid = 0;
    BundleInfo bundleInfo;
    int32_t bundleIndex = 0;
    BundleType bundleType;
    std::shared_ptr<AAFwk::Want> want;
    std::string moduleName;
    std::string abilityName;
    bool strictMode = false;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_MGR_INNER_PARAM_H
