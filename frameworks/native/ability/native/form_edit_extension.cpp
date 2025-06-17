/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "form_edit_extension.h"

#include "configuration_utils.h"
#include "formedit__extension_context.h"
#include "form_runtime/js_form_edit_extension.h"
#include "sts_form_edit_extension.h"
#include "hilog_tag_wrapper.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
FormEditExtension* FormEditExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new FormEditExtension();
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "call %{public}d", runtime->GetLanguage());
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsFormEditExtension::Create(runtime);
        case Runtime::Language::STS:
            return STSFormEditExtension::Create(runtime);
        default:
            return new FormEditExtension();
    }
}

void FormEditExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<FormEditExtensionContext>::Init(record, application, handler, token);
    TAG_LOGD(AAFwkTag::FORM_EXT, "init");
}
} // namespace AbilityRuntime
} // namespace OHOS
