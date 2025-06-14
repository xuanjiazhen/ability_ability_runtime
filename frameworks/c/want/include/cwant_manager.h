/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "cwant.h"
#include <map>
#include <string>
#include "hilog_tag_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want.h"
#include "want_params_wrapper.h"

using namespace OHOS::AAFwk;

namespace OHOS {
namespace AbilityRuntime {
class CWantManager {
public:
    static Want_ErrorCode TransformToWant(AbilityRuntime_Want cwant, Want &want, bool byDup)
    {
        want.SetElementName("", cwant.element.bundleName, cwant.element.abilityName, cwant.element.moduleName);
        WantParams wantParams;
        wantParams.SetParam(Want::PARAM_MODULE_NAME, String::Box(cwant.element.moduleName));
        for (auto it : cwant.params) {
            TAG_LOGI(AAFwkTag::APPKIT, "params key: %{public}s, value: %{public}s", it.first.c_str(), it.second.c_str());
            wantParams.SetParam(it.first, String::Box(it.second));
        }

        for (auto it : cwant.fds) {
            TAG_LOGI(AAFwkTag::APPKIT, "fd key: %{public}s, value: %{public}d", it.first.c_str(), it.second);
            WantParams wp;
            wp.SetParam(TYPE_PROPERTY, String::Box(FD));
            int32_t new_fd = byDup ? dup(it.second) : it.second;
            wp.SetParam(VALUE_PROPERTY, Integer::Box(new_fd));
            OHOS::sptr<IWantParams> pWantParams = WantParamWrapper::Box(wp);
            if (pWantParams != nullptr) {
                wantParams.SetParam(it.first, pWantParams);
            }
        }
        want.SetParams(wantParams);
        return WANT_ERROR_CODE_NO_ERROR;
    }
}
}
}