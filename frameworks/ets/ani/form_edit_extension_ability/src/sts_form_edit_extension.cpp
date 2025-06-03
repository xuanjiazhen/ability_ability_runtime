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

#include "sts_form_edit_extension.h"

#include <sstream>
#include <vector>

#include "ability_info.h"
#include "ani.h"
#include "ani_common_want.h"
#include "form_provider_data.h"
#include "form_runtime/form_extension_provider_client.h"
#include "hilog_tag_wrapper.h"
#include "sts_form_edit_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {


STSFormEditExtension *STSFormEditExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    TAG_LOGI(AAFwkTag::FORM_EDIT_EXT, "call___%{public}d", runtime->GetLanguage());
    return new STSFormEditExtension(static_cast<STSRuntime &>(*runtime));
}


void STSFormEditExtension::Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
    std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token) {
    TAG_LOGI(AAFwkTag::FORM_EDIT_EXT, "Init call");
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EDIT_EXT, "null localAbilityRecord");
        return;
    }
    auto abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EDIT_EXT, "null abilityInfo");
        return;
    }
    FormEditExtension::Init(record, application, handler, token);

    std::string srcPath;
    GetSrcPath(srcPath);

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TAG_LOGD(AAFwkTag::FORM_EDIT_EXT,
        "moduleName:%{public}s,srcPath:%{public}s,"
        "compileMode :%{public}d",
        moduleName.c_str(), srcPath.c_str(), abilityInfo_->compileMode);

    BindContext(abilityInfo, record->GetWant(), moduleName, srcPath);
    TAG_LOGI(AAFwkTag::FORM_EDIT_EXT, "Init End");
}



STSFormEditExtension::STSFormEditExtension(STSRuntime &stsRuntime) : stsRuntime_(stsRuntime) {}

STSFormEditExtension::~STSFormEditExtension()
{
    TAG_LOGI(AAFwkTag::FORM_EDIT_EXT, "destructor");
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EDIT_EXT, "env null");
        return;
    }
    if (stsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EDIT_EXT, "stsAbilityObj_ null");
        return;
    }
    if (stsAbilityObj_->aniRef) {
        env->GlobalReference_Delete(stsAbilityObj_->aniRef);
    }
}


}
}
}