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

#ifndef MOCK_OHOS_AAFWK_UI_EXTENSION_UTILS_H
#define MOCK_OHOS_AAFWK_UI_EXTENSION_UTILS_H

#include <unordered_set>

#include "extension_ability_info.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int EDM_SA_UID = 3057;
}
namespace UIExtensionUtils {
inline bool IsUIExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return false;
}

inline bool IsSystemUIExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return false;
}

inline bool IsSystemCallerNeeded(const AppExecFwk::ExtensionAbilityType type)
{
    return false;
}

inline bool IsPublicForEmbedded(const AppExecFwk::ExtensionAbilityType type)
{
    return false;
}

inline bool IsPublicForConstrainedEmbedded(const AppExecFwk::ExtensionAbilityType type)
{
    return false;
}

inline bool IsOnlyForModal(const AppExecFwk::ExtensionAbilityType type)
{
    return false;
}

inline bool IsEnterpriseAdmin(const AppExecFwk::ExtensionAbilityType type)
{
    return false;
}

inline bool IsWindowExtension(const AppExecFwk::ExtensionAbilityType type)
{
    return false;
}
} // namespace UIExtensionUtils
} // namespace AAFwk
} // namespace OHOS
#endif
