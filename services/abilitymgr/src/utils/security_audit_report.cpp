/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "security_audit_report.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "interceptor/ability_interceptor_interface.h"
#include "want.h"
#ifdef SECURITY_GUARDE_ENABLE
#include "event_info.h"
#include "nlohmann/json.hpp"
#include "sg_collect_client.h"
#endif

namespace OHOS {
namespace AAFwk {
namespace {
#ifdef SECURITY_GUARDE_ENABLE
constexpr int64_t EDM_BLOCKED_START_EVENT_ID = 0x18000104;
constexpr const char *EVENT_VERSION = "1.0";
constexpr const char *EDM_BLOCKED_START_EVENT_ERROR_MESSAGE = "start ability blocked by EDM rule";
#endif
}

void SecurityAuditReport::ReportEdmBlockedStart(const AbilityInterceptorParam &param)
{
#ifdef SECURITY_GUARDE_ENABLE
    using namespace Security::security_guard;
    nlohmann::json content;
    content["bundleName"] = param.want.GetBundleNameRef();
    content["moduleName"] = param.want.GetModuleNameRef();
    content["abilityName"] = param.want.GetAbilityNameRef();
    content["userId"] = param.userId;
    content["appIndex"] = param.want.GetIntParam(Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    content["callerBundleName"] = param.want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    content["errorCode"] = ERR_EDM_APP_CONTROLLED;
    content["errorMessage"] = EDM_BLOCKED_START_EVENT_ERROR_MESSAGE;
    content["isPreload"] = param.IsPreloadStartAbility();
    auto sgEventInfo =
        std::make_shared<Security::SecurityGuard::EventInfo>(EDM_BLOCKED_START_EVENT_ID, EVENT_VERSION, content.dump());
    int32_t ret = Security::SecurityGuard::NativeDataCollectKit::ReportSecurityInfoAsync(sgEventInfo);
    TAG_LOGW(AAFwkTag::ABILITYMGR,
        "ReportEdmBlockedStart ret:%{public}d, bundle:%{public}s, isPreload:%{public}d",
        ret, param.want.GetBundleNameRef().c_str(), param.IsPreloadStartAbility());
#else
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ReportEdmBlockedStart sg not enable");
#endif
}
} // namespace AAFwk
} // namespace OHOS
