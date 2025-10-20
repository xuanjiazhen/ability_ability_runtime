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
#include "ets_startup_config.h"

#include "ani_common_util.h"
#include "ani_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char STARTUP_CONFIG_CLASS_NAME* = "L@ohos/app/appstartup/StartupConfig/StartupConfigImpl";
constexpr const char STARTUP_CONFIG_ENTRY_CLASS_NAME* = "L@ohos/app/appstartup/StartupConfigEntry/StartupConfigEntry";
constexpr const char STARTUP_CONFIG_ENTRY_SIGNATURE_ON_REQUEST_CUSTOM_MATCH_RULE* =
    "L@ohos/app/ability/Want/Want;:Lstd/core/String;";
    constexpr const char *STARTUP_LISTEN_SIGNATURE_ON_COMPLETED = "L@ohos/base/BusinessError;:V";
constexpr int32_t DEFAULT_AWAIT_TIMEOUT_MS = 10000;
}
ETSStartupConfig::ETSStartupConfig(ani_env env) : StartupConfig(), env_(env)
{}

ETSStartupConfig::~ETSStartupConfig() = default;

int32_t ETSStartupConfig::Init(ani_ref &configEntry, std::shared_ptr<AAFwk::Want> want)
{
    if (configEntry == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null configEntry");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    if (env_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env_");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    ani_status status = ANI_ERROR;
    ani_ref config = nullptr;
    if ((status = env_->Object_CallMethodByName_Ref(reinterpret_cast<ani_object>(configEntry), "onConfig",
        ":V", &config)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "Object_CallMethodByName_Ref onConfig failed");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    InitAwaitTimeout(env_, reinterpret_cast<ani_object>(config));
    InitListener(env_, reinterpret_cast<ani_object>(config));
    InitCustomization(env_, reinterpret_cast<ani_object>(configEntry), want);
    return ERR_OK;
}

void ETSStartupConfig::Init(ani_object config)
{
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    InitAwaitTimeout(env_, config);
    InitListener(env_, config);
    return ERR_OK;
}

void ETSStartupConfig::InitAwaitTimeout(ani_env env, ani_object config)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return;
    }
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return;
    }


    int32_t awaitTimeoutNum = DEFAULT_AWAIT_TIMEOUT_MS;
    if (!GetIntPropertyValue(env, config, "timeoutMs", awaitTimeoutNum)) {
        TAG_LOGE(AAFwkTag::STARTUP, "timeoutMs, GetPropertyRef failed");
        return;
    }

    if (awaitTimeoutNum <= 0) {
        TAG_LOGE(AAFwkTag::STARTUP, "invalid argc");
        awaitTimeoutNum = DEFAULT_AWAIT_TIMEOUT_MS;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "set awaitTimeoutMs to %{public}d", awaitTimeoutNum);
    awaitTimeoutMs_ = awaitTimeoutNum;
}

void ETSStartupConfig::InitListener(ani_env env, ani_object config)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return;
    }
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return;
    }
    ani_ref listener = nullptr;
    if (!GetRefProperty(env, config, "startupListener", listener)) {
        TAG_LOGE(AAFwkTag::STARTUP, "startupListener GetPropertyRef failed");
        return;
    }

    OnCompletedCallbackFunc onCompletedCallback =
    [env, listener](const std::shared_ptr<StartupTaskResult> &result) {
        if (env == nullptr || listener == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "null env or listener");
            return;
        }

        ani_object resultValue = ETSStartupConfig::BuildResult(env, result);

        ani_status status = ANI_ERROR;
        if ((status = env->Object_CallMethodByName_Void(reinterpret_cast<ani_object>(listener),
            "onCompleted", STARTUP_LISTEN_SIGNATURE_ON_COMPLETED,
            resultValue) != ANI_OK)) {
            TAG_LOGE(AAFwkTag::STARTUP, "Function_Call onCompleted failed, %{public}d", status);
        }
    };
    listener_ = std::make_shared<StartupListener>(onCompletedCallback);
}

void ETSStartupConfig::InitCustomization(ani_env env, ani_object configEntry, std::shared_ptr<AAFwk::Want> want)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return;
    }
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return;
    }
    if (!want) {
        TAG_LOGD(AAFwkTag::STARTUP, "want is null");
        return;
    }
    ani_object wantAniObj = nullptr;
    wantAniObj = WrapWant(env, *want);

    ani_status status = ANI_ERROR;
    ani_ref callResult = nullptr;
    if ((status = env_->Object_CallMethodByName_Ref(reinterpret_cast<ani_object>(configEntry),
        "onRequestCustomMatchRule", STARTUP_CONFIG_ENTRY_SIGNATURE_ON_REQUEST_CUSTOM_MATCH_RULE,
        &callResult, wantAniObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "Object_CallMethodByName_Ref onRequestCustomMatchRule failed");
        return;
    }
    if (callResult == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return;
    }

    std::string customization = ""; 
    if (!GetStdString(env, reinterpret_cast<ani_string>(callResult), customization)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetStdString failed");
        return;
    }
    customization_ = customization;
}

ani_object ETSStartupConfig::BuildResult(ani_env env, const std::shared_ptr<StartupTaskResult> &result)
{
    if (result == nullptr) {
        return EtsErrorUtil::CreateError(env, ERR_STARTUP_INTERNAL_ERROR,
            StartupUtils::GetErrorMessage(ERR_STARTUP_INTERNAL_ERROR));
    }
    ani_ref undefRef = nullptr;
    if (result->GetResultCode() != ERR_OK) {
        return EtsErrorUtil::CreateError(env, result->GetResultCode(), result->GetResultMessage());
    }
    return nullptr;
}
}
}