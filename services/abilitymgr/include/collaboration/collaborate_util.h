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

#ifndef OHOS_ABILITY_RUNTIME_COLLABORATE_UTIL_H
#define OHOS_ABILITY_RUNTIME_COLLABORATE_UTIL_H

#include <string>
#include <unordered_set>

#include "ability_config.h"

namespace OHOS {
namespace AAFwk {
namespace CollaborateUtil {
constexpr const char* SYSTEM_BASIC = "system_basic";
constexpr const char* SYSTEM_CORE = "system_core";
constexpr const char* DEFAULT_DEVICE_ID = "";

#define CHECK_POINTER_CONTINUE(object)                         \
    if (!object) {                                             \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr"); \
        continue;                                              \
    }

static bool IsCollaborator(const std::string &bundleName)
{
	bool collaboratorBundle = callerBundleName == AppUtils::GetInstance().GetBrokerDelegateBundleName();
	bool isBrokerUid = IPCSkeleton::GetCallingUid() == AppUtils::GetInstance().GetCollaboratorBrokerUID();
	return (collaboratorBundle && AppUtils::GetInstance().IsSupportAncoApp()) || isBrokerUid;
}
}  // namespace CollaborateUtil
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_COLLABORATE_UTIL_H