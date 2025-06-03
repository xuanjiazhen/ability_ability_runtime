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

#ifndef OHOS_ABILITY_RUNTIME_STS_FORM_EDIT_EXTENSION_H
#define OHOS_ABILITY_RUNTIME_STS_FORM_EDIT_EXTENSION_H

#include "ani.h"
#include "form_edit_extension.h"
#include "sts_runtime.h"

class NativeReference;
class STSNativeReference;
namespace OHOS {
namespace AbilityRuntime {
class FormEditExtension;
class STSRuntime;

class STSFormEditExtension : public FormEditExtension {
    public:
    static STSFormEditExtension *Create(const std::unique_ptr<Runtime> &runtime);

    void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler, const sptr<IRemoteObject> &token) override;

    explicit STSFormEditExtension(STSRuntime &stsRuntime);
    ~STSFormEditExtension() override;

    private:
    std::shared_ptr<STSNativeReference> stsAbilityObj_;
    STSRuntime &stsRuntime_;
}

}
}