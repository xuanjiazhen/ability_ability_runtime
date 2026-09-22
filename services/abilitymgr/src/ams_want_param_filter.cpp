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

#include "ams_want_param_filter.h"

#include <map>
#include <string>

#include "hilog_tag_wrapper.h"
#include "permission_verification.h"

namespace OHOS {
namespace AAFwk {
namespace {
// Third-party invisible parameters: system-internal keys that must not be set or
// perceived by third-party apps. key is the parameter name, value is a short
// description (used for logs and self-documentation). To protect a new key, just
// append one more { key, description } entry below.
const std::map<std::string, std::string> THIRD_PARTY_INVISIBLE_KEYS = {
    { "specifyTokenId", "specified caller token id, system-only" },
    { "skillCallerTokenId", "skill caller token id, system-internal" },
    { "debugApp", "debug launch flag, system-only" },
};
}

AMSWantParamFilter &AMSWantParamFilter::GetInstance()
{
    static AMSWantParamFilter instance;
    return instance;
}

void AMSWantParamFilter::InstallFilter()
{
    WantParams::AddParamFilter("AMS", &AMSWantParamFilter::FilterCallback);
}

void AMSWantParamFilter::FilterCallback(WantParams &params)
{
    // System callers pass through untouched; only third-party callers are stripped.
    if (AMSWantParamFilter::GetInstance().IsSystemCaller()) {
        return;
    }
    for (const auto &entry : THIRD_PARTY_INVISIBLE_KEYS) {
        if (params.HasParam(entry.first)) {
            TAG_LOGI(AAFwkTag::ABILITYMGR,
                "strip system-internal param %{public}s from non-system caller: %{public}s",
                entry.first.c_str(), entry.second.c_str());
            params.Remove(entry.first);
        }
    }
}

bool AMSWantParamFilter::IsSystemCaller() const
{
    return PermissionVerification::GetInstance()->IsSACall() ||
        PermissionVerification::GetInstance()->IsSystemAppCall();
}

} // namespace AAFwk
} // namespace OHOS