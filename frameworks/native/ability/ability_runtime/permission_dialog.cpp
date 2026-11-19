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

#include "permission_dialog.h"

#include <string>
#include <vector>

#include "ability_manager_errors.h"
#include "array_wrapper.h"
#include "hilog_tag_wrapper.h"
#include "want.h"
#include "want_params_wrapper.h"
#ifdef SUPPORT_SCREEN
#include "ui_content.h"
#endif // SUPPORT_SCREEN

namespace OHOS {
namespace AbilityRuntime {
namespace {
// Privacy-center consent dialog (client-side modal UIExtension, sys/commonUI).
constexpr const char* SYSTEM_CONSENT_BUNDLE_NAME = "com.huawei.hmos.security.privacycenter";
constexpr const char* SYSTEM_CONSENT_ABILITY_NAME = "CollaborationAccessAbility";
constexpr const char* UI_EXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
constexpr const char* UI_EXTENSION_TYPE_VALUE = "sys/commonUI";
// Want params handed to the consent dialog so it can render "caller X requests
// to invoke capability Y". Keys follow the privacy-center contract and are all
// prefixed with "ohos.hmos.security.privacycenter.".
constexpr const char* CONSENT_PARAM_CALLER_BUNDLE_NAME = "ohos.hmos.security.privacycenter.callerBundleName";
constexpr const char* CONSENT_PARAM_TARGET_BUNDLE_NAME = "ohos.hmos.security.privacycenter.targetBundleNameList";
// App clone index of the caller / target list; invokePublicFunction only targets
// the default (non-cloned) instance, so the single entry is 0.
constexpr const char* CONSENT_PARAM_CALLER_APP_INDEX = "ohos.hmos.security.privacycenter.callerAppIndex";
constexpr const char* CONSENT_PARAM_TARGET_APP_INDEX = "ohos.hmos.security.privacycenter.targetAppIndexList";
// Intent execute record id returned by the server-side pre-check; the consent
// dialog uses it to query the invoking caller bundle via
// getInsightIntentCallerBundleName.
constexpr const char* CONSENT_PARAM_INTENT_ID = "ohos.hmos.security.privacycenter.intentId";
// Result key the dialog writes back: a single-element WantParams array whose
// resultCode field carries the final decision (see ParseGrantResult).
constexpr const char* CONSENT_PARAM_GRANT_RESULT = "ohos.hmos.security.privacycenter.grantResult";
// Field inside the grantResult WantParams; 0 = granted, otherwise declined.
constexpr const char* CONSENT_RESULT_CODE = "resultCode";
constexpr int32_t CONSENT_RESULT_GRANTED = 0;

// Extract the grant result from the dialog's returned Want. The privacy center
// writes grantResult as a WantParams array with one element (dump: "W1{{...}}")
// whose resultCode field is a typed int, so plain GetBoolParam always sees the
// default. Fail-closed: anything we cannot parse counts as declined.
bool ParseGrantResult(const AAFwk::Want &result)
{
    const auto &allParams = result.GetParams().GetParams();
    auto it = allParams.find(CONSENT_PARAM_GRANT_RESULT);
    if (it == allParams.end() || it->second == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: param missing");
        return false;
    }
    AAFwk::IInterface *raw = it->second.GetRefPtr();
    AAFwk::WantParams grantParams;
    AAFwk::IArray *array = AAFwk::IArray::Query(raw);
    if (array != nullptr) {
        long length = 0;
        if (array->GetLength(length) != ERR_OK || length <= 0) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: empty array, length=%{public}ld", length);
            return false;
        }
        sptr<AAFwk::IInterface> element;
        if (array->Get(0, element) != ERR_OK || element == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: get element 0 failed");
            return false;
        }
        AAFwk::IWantParams *inner = AAFwk::IWantParams::Query(element.GetRefPtr());
        if (inner == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: element 0 is not WantParams");
            return false;
        }
        grantParams = AAFwk::WantParamWrapper::Unbox(inner);
    } else {
        AAFwk::IWantParams *inner = AAFwk::IWantParams::Query(raw);
        if (inner == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: unexpected param type");
            return false;
        }
        grantParams = AAFwk::WantParamWrapper::Unbox(inner);
    }
    if (grantParams.GetParams().find(CONSENT_RESULT_CODE) == grantParams.GetParams().end()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "grant result: resultCode missing");
        return false;
    }
    int32_t resultCode = grantParams.GetIntParam(CONSENT_RESULT_CODE, -1);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "grant result: resultCode=%{public}d", resultCode);
    return resultCode == CONSENT_RESULT_GRANTED;
}
} // namespace

#ifdef SUPPORT_SCREEN
ErrCode StartPermissionDialog(const std::shared_ptr<AbilityContext> &context,
    const std::string &targetBundleName, int32_t intentId,
    const std::shared_ptr<PermissionDialogCallback> &callback)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "permission dialog: null context");
        return ERR_INVALID_VALUE;
    }
    auto uiContent = context->GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "permission dialog: null uiContent");
        return ERR_INVALID_VALUE;
    }
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "permission dialog: null callback");
        return ERR_INVALID_VALUE;
    }

    AAFwk::Want want;
    want.SetElementName(SYSTEM_CONSENT_BUNDLE_NAME, SYSTEM_CONSENT_ABILITY_NAME);
    want.SetParam(UI_EXTENSION_TYPE_KEY, std::string(UI_EXTENSION_TYPE_VALUE));
    want.SetParam(CONSENT_PARAM_CALLER_BUNDLE_NAME, context->GetBundleName());
    want.SetParam(CONSENT_PARAM_TARGET_BUNDLE_NAME, std::vector<std::string> { targetBundleName });
    want.SetParam(CONSENT_PARAM_CALLER_APP_INDEX, 0);
    want.SetParam(CONSENT_PARAM_TARGET_APP_INDEX, std::vector<int> { 0 });
    want.SetParam(CONSENT_PARAM_INTENT_ID, intentId);

    Ace::ModalUIExtensionCallbacks callbacks;
    callbacks.onResult = [callback](int32_t code, const AAFwk::Want &result) {
        callback->OnResult(ParseGrantResult(result));
    };
    callbacks.onError = [callback](int32_t code, const std::string &name, const std::string &message) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "permission dialog error: %{public}d, %{public}s, %{public}s", code,
            name.c_str(), message.c_str());
        callback->OnResult(false);
    };
    callbacks.onRelease = [callback](int32_t code) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "permission dialog released without result: %{public}d", code);
        callback->OnResult(false);
    };

    Ace::ModalUIExtensionConfig config;
    config.isProhibitBack = true;

    int32_t sessionId = uiContent->CreateModalUIExtension(want, callbacks, config);
    if (sessionId == 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "permission dialog: create failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}
#endif // SUPPORT_SCREEN
} // namespace AbilityRuntime
} // namespace OHOS