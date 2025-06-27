/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "sts_extension_common.h"

#include "hilog_tag_wrapper.h"
#include "js_extension_context.h"
#include "sts_runtime.h"
#include "js_runtime_utils.h"


namespace OHOS {
namespace AbilityRuntime {

using namespace OHOS::AppExecFwk;

std::shared_ptr<StsExtensionCommon> StsExtensionCommon::Create(STSRuntime &stsRuntime, STSNativeReference &stsObj,
    const std::shared_ptr<STSNativeReferenceWrapper> &shellContextRef)
{
    return std::make_shared<StsExtensionCommon>(stsRuntime, stsObj, shellContextRef);
}

StsExtensionCommon::StsExtensionCommon(STSRuntime &stsRuntime, STSNativeReference &stsObj,
    const std::shared_ptr<STSNativeReferenceWrapper> &shellContextRef)
    : shellContextRef_(shellContextRef) {}

StsExtensionCommon::~StsExtensionCommon()
{
}

void StsExtensionCommon::OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration> &fullConfig)
{
    TAG_LOGI(AAFwkTag::EXT, "StsExtensionCommon::OnConfigurationUpdated Called");
}

void StsExtensionCommon::OnMemoryLevel(int level)
{
    TAG_LOGD(AAFwkTag::EXT, "StsExtensionCommon::OnMemoryLevel Called");
}

}
}
