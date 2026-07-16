/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "mock_my_flag.h"

#include "native_ability_util.h"

namespace OHOS {
namespace AAFwk {
int MyFlag::flag_ = 0;
bool MyFlag::isWithNative_ = false;
unsigned int MyFlag::mockHostTokenId_ = 0;
int MyFlag::mockGetHostTokenIdRet_ = -1; // nonzero => GetHostTokenId fails (host not resolved) by default
bool MyFlag::overrideTokenPerm_ = false;
bool MyFlag::tokenPermResult_ = false;
bool MyFlag::mockParentHas_ = false;
int MyFlag::mockParentState_ = 0;
unsigned int MyFlag::mockParentAccessTokenId_ = 0;
bool MyFlag::mockHostForeground_ = false;

bool NativeAbilityMetaData::IsWithNative(const AppExecFwk::AbilityInfo& abilityInfo)
{
    return MyFlag::isWithNative_;
}
} // namespace AAFwk
} // namespace OHOS