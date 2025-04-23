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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MISSION_INFO_H
#define MOCK_OHOS_ABILITY_RUNTIME_MISSION_INFO_H

#include <string>

#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
enum ContinueState {
    CONTINUESTATE_UNKNOWN = -1,
    CONTINUESTATE_ACTIVE = 0,
    CONTINUESTATE_INACTIVE = 1,
    CONTINUESTATE_MAX
};

struct MissionInfo {
    int32_t id = 0;
    Want want;
};

struct MissionValidResult {};
}  // namespace AAFwk
}  // namespace OHOS
#endif
