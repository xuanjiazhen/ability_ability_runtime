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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H

#include "ability_manager_errors.h"
#include "ability_record.h"
#include "app_mgr_util.h"
#include "app_scheduler.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "mock_my_status.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
namespace AbilityUtil {
#ifdef WITH_DLP
constexpr const char* DLP_PARAMS_SANDBOX = "ohos.dlp.params.sandbox";
#endif // WITH_DLP

#define CHECK_POINTER_CONTINUE(object)                         \
    if (!object) {                                             \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr"); \
        continue;                                              \
    }

#define CHECK_POINTER_IS_NULLPTR(object)                       \
    if (object == nullptr) {                                   \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr"); \
        return;                                                \
    }

#define CHECK_POINTER(object)                                  \
    if (!object) {                                             \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr"); \
        return;                                                \
    }

#define CHECK_POINTER_LOG(object, log)                      \
    if (!object) {                                          \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s:", log); \
        return;                                             \
    }

#define CHECK_POINTER_TAG_LOG(object, tag, log)             \
    if (!object) {                                          \
        TAG_LOGE(tag, "%{public}s:", log);                  \
        return;                                             \
    }

#define CHECK_POINTER_AND_RETURN(object, value)                \
    if (!object) {                                             \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr"); \
        return value;                                          \
    }

#define CHECK_POINTER_AND_RETURN_LOG(object, value, log)    \
    if (!object) {                                          \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s:", log); \
        return value;                                       \
    }

#define CHECK_POINTER_RETURN_BOOL(object)                      \
    if (!object) {                                             \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "nullptr"); \
        return false;                                          \
    }

#define CHECK_RET_RETURN_RET(object, log)                                            \
    if (object != ERR_OK) {                                                          \
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s, ret : %{public}d", log, object); \
        return object;                                                               \
    }

#define CHECK_TRUE_RETURN_RET(object, value, log)          \
    if (object) {                                          \
        TAG_LOGW(AAFwkTag::ABILITYMGR, "%{public}s", log); \
        return value;                                      \
    }
[[maybe_unused]] static bool IsStartIncludeAtomicService(const Want &want, const int32_t userId)
{
    return false;
}

[[maybe_unused]] static bool ParseJumpInterceptorWant(Want &targetWant, const std::string callerPkg)
{
    return true;
}

[[maybe_unused]] static int StartAppgallery(const std::string &bundleName, const int requestCode, const int32_t userId,
    const std::string &action)
{
    return 0;
}

[[maybe_unused]] static bool IsSystemDialogAbility(const std::string &bundleName, const std::string &abilityName)
{
    return false;
}

[[maybe_unused]] static void WantSetParameterWindowMode(Want &want, int32_t windowMode)
{
}

[[maybe_unused]] static int64_t SystemTimeMillis()
{
    return 0;
}

[[maybe_unused]] static std::string ConvertBundleNameSingleton(const std::string &bundleName, const std::string &name,
    const std::string &moduleName, const int32_t appIndex = 0)
{
    return "";
}

[[maybe_unused]] static int64_t UTCTimeSeconds()
{
    return 0;
}

[[maybe_unused]] static bool IsSceneBoard(const std::string &bundleName, const std::string &AbilityName)
{
    return false;
}

[[maybe_unused]] static bool IsStartFreeInstall(const Want &want)
{
    return false;
}

[[maybe_unused]] static std::shared_ptr<AppExecFwk::BundleMgrHelper> GetBundleManagerHelper()
{
    return MyStatus::GetInstance().bundleMgrHelper_;
}

[[maybe_unused]] static bool CheckJumpInterceptorWant(const Want &targetWant, std::string &callerPkg,
    std::string &targetPkg)
{
    return false;
}

[[maybe_unused]] static bool AddAbilityJumpRuleToBms(const std::string &callerPkg, const std::string &targetPkg,
    int32_t userId)
{
    return true;
}

#ifdef WITH_DLP
[[maybe_unused]] static bool HandleDlpApp(Want &want)
{
    return true;
}
#endif // WITH_DLP

[[maybe_unused]] static void RemoveShowModeKey(Want &want)
{
}

[[maybe_unused]] static void RemoveWindowModeKey(Want &want)
{
}

[[maybe_unused]] static void RemoveInstanceKey(Want &want)
{
}

[[maybe_unused]] static void RemoveWantKey(Want &want)
{
}

[[maybe_unused]] static int32_t CheckInstanceKey(const Want &want)
{
    return ERR_OK;
}

[[maybe_unused]] static void ProcessWindowMode(Want &want, uint32_t accessTokenId, int32_t windowMode)
{
}

inline ErrCode EdmErrorType(bool isEdm)
{
    if (isEdm) {
        return ERR_EDM_APP_CONTROLLED;
    }
    return ERR_APP_CONTROLLED;
}
}  // namespace AbilityUtil
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
