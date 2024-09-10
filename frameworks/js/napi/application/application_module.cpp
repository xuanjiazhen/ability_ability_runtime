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

<<<<<<<< HEAD:frameworks/js/napi/application/application_module.cpp
#include "js_application.h"
#include "napi/native_api.h"

static napi_module _module = {
    .nm_version = 0,
    .nm_register_func = OHOS::AbilityRuntime::ApplicationInit,
    .nm_modname = "app.ability.application",
};

extern "C" __attribute__((constructor)) void NAPI_app_ability_application_AutoRegister(void)
{
    napi_module_register(&_module);
}
========
#ifndef OHOS_ABILITY_RUNTIME_WANT_UTILS_H
#define OHOS_ABILITY_RUNTIME_WANT_UTILS_H

#include "want.h"

#ifdef APP_DOMAIN_VERIFY_ENABLED
#include "ag_convert_callback_impl.h"
#include "app_domain_verify_mgr_client.h"
#endif

namespace OHOS {
namespace AAFwk {
class WantUtils final {
public:
    static int32_t ConvertToExplicitWant(Want& want);
    static int32_t GetCallerBundleName(std::string &callerBundleName);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_WANT_UTILS_H
>>>>>>>> upstream/master:services/abilitymgr/include/utils/want_utils.h
