/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_ui_extension.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
namespace OHOS {
namespace AbilityRuntime {
JsUIExtension* JsUIExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new (std::nothrow) JsUIExtension(runtime);
}

JsUIExtension::JsUIExtension(const std::unique_ptr<Runtime>& runtime)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "JsUIExtension constructor");
    impl_ = std::make_shared<JsUIExtensionImpl>(runtime);
    SetUIExtensionBaseImpl(impl_);
}

JsUIExtension::~JsUIExtension()
{
    TAG_LOGI(AAFwkTag::UI_EXT, "JsUIExtension destructor");
}

sptr<IRemoteObject> JsUIExtension::OnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    if (impl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnConnect, impl invalid");
        return nullptr;
    }

    return impl_->OnConnect(want);
}

void JsUIExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnDisconnect(want);
    if (impl_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "OnDisconnect, impl invalid");
        return;
    }

    impl_->OnDisconnect(want);
}
} // namespace AbilityRuntime
} // namespace OHOS