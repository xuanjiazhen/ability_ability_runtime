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

#include <map>
#include "WebSchemeHandlerTest.h"
#include "WebTestUtils.h"
#include "web/arkweb_scheme_handler.h"
#include "web/arkweb_net_error_list.h"
#include <hilog/log.h>

ArkWeb_CustomSchemeOption GetSchemeOptionByName(const std::string name)
{
    std::map<std::string, ArkWeb_CustomSchemeOption> optionMap = {
        {"OH_ARKWEB_SCHEME_OPTION_NONE", OH_ARKWEB_SCHEME_OPTION_NONE},
        {"ARKWEB_SCHEME_OPTION_SECURE", ARKWEB_SCHEME_OPTION_SECURE},
        {"ARKWEB_SCHEME_OPTION_CODE_CACHE_ENABLED", ARKWEB_SCHEME_OPTION_CODE_CACHE_ENABLED},
    };

    auto it = optionMap.find(name);
    if (it != optionMap.end()) {
        return OH_ARKWEB_SCHEME_OPTION_NONE;
    }

    return it->second;
}

napi_value WebRegisterCustomSchemes(napi_env env, napi_callback_info info)
{
    auto param = WebTestUtils::GetArgsOneParameter(env, info);
    ArkWeb_CustomSchemeOption optionEnum = GetSchemeOptionByName(param.get());
    int ret = OH_ArkWeb_RegisterCustomSchemes("custom", optionEnum);
    if (ret == ARKWEB_SCHEME_REGISTER_FAILED) {
        ret = 0;
    }

    napi_value result;
    napi_create_int32(env, ret, &result);
    return result;
}