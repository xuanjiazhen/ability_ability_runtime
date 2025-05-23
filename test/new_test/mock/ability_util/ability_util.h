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

#ifndef MOCK_ABILITY_MANAGER_SERVICE_OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
#define MOCK_ABILITY_MANAGER_SERVICE_OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H

#include <string>
#include <map>
#include "oh_mock_utils.h"
#include "mock_common.h"

namespace OHOS {
namespace AAFwk {
namespace AbilityUtil {
// mock method declare
OH_MOCK_GLOBAL_STATIC_METHOD(bool, CheckJumpInterceptorWant, Want, std::string, std::string)
OH_MOCK_GLOBAL_STATIC_METHOD(std::shared_ptr<AppExecFwk::BundleMgrHelper>, GetBundleManagerHelper)

#define CHECK_POINTER_CONTINUE(object)                         \
    if (!object) {                                             \
        continue;                                              \
    }

#define CHECK_POINTER_IS_NULLPTR(object)                       \
    if (object == nullptr) {                                   \
        return;                                                \
    }

#define CHECK_POINTER(object)                                  \
    if (!object) {                                             \
        return;                                                \
    }

#define CHECK_POINTER_LOG(object, log)                      \
    if (!object) {                                          \
        return;                                             \
    }

#define CHECK_POINTER_TAG_LOG(object, tag, log)             \
    if (!object) {                                          \
        return;                                             \
    }

#define CHECK_POINTER_AND_RETURN(object, value)                \
    if (!object) {                                             \
        return value;                                          \
    }

#define CHECK_POINTER_AND_RETURN_LOG(object, value, log)    \
    if (!object) {                                          \
        return value;                                       \
    }

#define CHECK_POINTER_RETURN_BOOL(object)                      \
    if (!object) {                                             \
        return false;                                          \
    }

#define CHECK_RET_RETURN_RET(object, log)                                            \
    if (object != ERR_OK) {                                                          \
        return object;                                                               \
    }

#define CHECK_TRUE_RETURN_RET(object, value, log)          \
    if (object) {                                          \
        return value;                                      \
    }

static int64_t GetSysTimeNs()
{
    return 0;
}

static bool IsSystemDialogAbility(const std::string &bundleName, const std::string &abilityName)
{
    return false;
}

static std::string ConvertBundleNameSingleton(const std::string &bundleName, const std::string &name,
    const std::string &moduleName, const int32_t appIndex = 0)
{
    return "";
}

static int64_t SystemTimeMillis()
{
    return 0;
}

static int64_t UTCTimeSeconds()
{
    return 0;
}

static bool IsStartFreeInstall(const Want &want)
{
    return false;
}

static bool ParseJumpInterceptorWant(Want &targetWant, const std::string callerPkg)
{
    return false;
}

static bool AddAbilityJumpRuleToBms(const std::string &callerPkg, const std::string &targetPkg,
    int32_t userId)
{
    return false;
}

#ifdef WITH_DLP
static bool HandleDlpApp(Want &want)
{
    return false;
}
#endif

static bool IsStartIncludeAtomicService(const Want &want, const int32_t userId)
{
    return false;
}

static void RemoveShowModeKey(Want &want) {}

static bool IsSceneBoard(const std::string &bundleName, const std::string &AbilityName)
{
    return false;
}

static void RemoveWindowModeKey(Want &want) {}

static void RemoveInstanceKey(Want &want) {}

static void RemoveWantKey(Want &want) {}

static int32_t CheckInstanceKey(const Want &want)
{
    return ERR_OK;
}

static void WantSetParameterWindowMode(Want &want, int32_t windowMode) {}
static void ProcessWindowMode(Want &want, uint32_t accessTokenId, int32_t windowMode) {}

static int StartAppgallery(const std::string &bundleName, const int requestCode, const int32_t userId,
    const std::string &action)
{
    return 0;
}

inline ErrCode EdmErrorType(bool isEdm)
{
    return ERR_APP_CONTROLLED;
}
}  // namespace AbilityUtil
}  // namespace AAFwk
}  // namespace OHOS
#endif
