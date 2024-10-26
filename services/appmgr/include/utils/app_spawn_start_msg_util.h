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

#ifndef OHOS_ABILITY_RUNTIME_APP_SPAWN_START_MSG_UTIL_H
#define OHOS_ABILITY_RUNTIME_APP_SPAWN_START_MSG_UTIL_H

#include "app_running_record.h"
#include "app_spawn_client.h"
#include "application_info.h"
#include "bundle_info.h"
#include "remote_client_manager.h"
#include "task_handler_wrap.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class AppSpawnStartMsgUtil {
public:
    /**
     * Constructor.
     */
    explicit AppSpawnStartMsgUtil();

    /**
     * Build message body for app spawn
     */
    static int32_t CreateStartMsgInner(std::shared_ptr<RemoteClientManager> remoteClientManager,
        const std::string &processName, uint32_t startFlags, const int uid,
        const BundleInfo &bundleInfo, const int32_t bundleIndex, BundleType bundleType, AppSpawnStartMsg &startMsg,
        std::shared_ptr<AAFwk::Want> want, const std::string &moduleName = "", const std::string &abilityName = "",
        bool strictMode = false);

    static void PresetMaxChildProcess(std::shared_ptr<AppRunningRecord> appRecord, int32_t &maxChildProcess);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APP_SPAWN_START_MSG_UTIL_H
