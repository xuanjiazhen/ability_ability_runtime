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
#include <unistd.h>
#include <vector>

#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_verification.h"
#include "skill_execute_param.h"

namespace OHOS {
namespace AAFwk {
namespace {
// Third-party invisible parameters: system-internal keys that must not be set or
// perceived by third-party apps. key is the parameter name, value is a short
// description (used for logs and self-documentation). To protect a new key, just
// append one more { key, description } entry below.
const std::map<std::string, std::string> THIRD_PARTY_INVISIBLE_KEYS = {
    { AppExecFwk::SKILL_EXECUTE_PARAM_BUNDLE_NAME, "skill execution bundle name" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_MODULE_NAME, "skill execution module name" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_SKILL_NAME, "skill execution skill name" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_SCRIPT_PATH, "skill execution script path" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_FUNCTION_NAME, "skill execution function name" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_ARGS_KEYS, "skill execution args keys" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT, "skill execution src entries count" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_HAP_PATH, "skill execution hap path" },
    { AppExecFwk::SKILL_EXECUTE_PARAM_REQUEST_CODE, "skill execution request code" },
};

// Dynamic skill params are constructed at runtime as "<prefix><name>", so they
// cannot be listed as exact keys above and are stripped by prefix instead.
const std::vector<std::string> THIRD_PARTY_INVISIBLE_KEY_PREFIXES = {
    AppExecFwk::SKILL_EXECUTE_PARAM_ARGS_PREFIX,
    AppExecFwk::SKILL_EXECUTE_PARAM_SRC_ENTRY_PREFIX,
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
    // Skip filtering when there is no cross-process IPC caller (e.g. local
    // deserialization from persistent storage or file recovery): GetCallingPid()
    // then equals the current pid, and IsSystemCaller() would misjudge the caller
    // and wrongly strip system-internal params kept by AMS itself.
    if (IPCSkeleton::GetCallingPid() == getpid()) {
        return;
    }
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
    auto keySet = params.KeySet();
    for (const auto &key : keySet) {
        for (const auto &prefix : THIRD_PARTY_INVISIBLE_KEY_PREFIXES) {
            if (key.rfind(prefix, 0) == 0) {
                TAG_LOGI(AAFwkTag::ABILITYMGR,
                    "strip system-internal param %{public}s from non-system caller", key.c_str());
                params.Remove(key);
                break;
            }
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