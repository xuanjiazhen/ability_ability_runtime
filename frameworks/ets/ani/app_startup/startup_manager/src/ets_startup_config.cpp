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
#include "ets_runtime_utils.h"
#include "hilog_tag_wrapper.h"
#include "sts_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {

EtsStartupConfig::EtsStartupConfig(ani_env *env) : StartupConfig(), env_(env)
{
    
}

EtsStartupConfig::~EtsStartupConfig() = default;

int32_t EtsStartupConfig::Init(std::unique_ptr<STSNativeReference> &configEntryRef)
{
    if (configEntryRef == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null configEntry");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    ani_ref configEntry = configEntryRef->aniRef;
    ani_boolean isUndefined = true;
    ani_ref onConfig = nullptr;
    if (!GetFieldRef(env, configEntry, "onConfig", onConfig, isUndefined)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetFieldRef failed");
        return;
    }

    if (isUndefined) {
        TAG_LOGD(AAFwkTag::STARTUP, "onConfig undefined");
        return;
    }

    ani_ref config;
    std::vector<ani_ref> argv = { };
    status = env->FunctionalObject_Call(static_cast<ani_fn_object>(onConfig), 0, argv, &config);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FunctionalObject_Call status: %{public}d", status);
        return;
    }

    InitAwaitTimeout(env_, config);
    InitListener(env_, config);
    return ERR_OK;
}

int32_t EtsStartupConfig::Init(ani_object config)
{
    if (config == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null config");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    InitAwaitTimeout(env_, config);
    InitListener(env_, config);
    return ERR_OK;
}

void EtsStartupConfig::InitAwaitTimeout(ani_env *env, ani_object config)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return;
    }

    int32_t awaitTimeoutNum = DEFAULT_AWAIT_TIMEOUT_MS;
    if (IsExistsProperty(env, config, "timeoutMs")) {
        double awaitTimeoutD = 0.0;
        if (!GetDoublePropertyObject(env, config, "timeoutMs", awaitTimeoutD)) {
            TAG_LOGE(AAFwkTag::STARTUP, "GetDoublePropertyObject failed");
            return;
        }
        awaitTimeoutNum = static_cast<int32_t>(awaitTimeoutD);
    }

    if (awaitTimeoutNum <= 0) {
        TAG_LOGE(AAFwkTag::STARTUP, "invalid argc");
        awaitTimeoutNum = DEFAULT_AWAIT_TIMEOUT_MS;
    }
    TAG_LOGD(AAFwkTag::STARTUP, "set awaitTimeoutMs to %{public}d", awaitTimeoutNum);
    awaitTimeoutMs_ = awaitTimeoutNum;
}

void EtsStartupConfig::InitListener(ani_env *env, ani_object config)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return;
    }

    if (!IsExistsProperty(env, config, "startupListener")) {
        TAG_LOGD(AAFwkTag::STARTUP, "null startupListener");
        return;
    }

    ani_ref listener = nullptr;
    if (!GetRefProperty(env, config, "startupListener", listener)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetRefProperty failed");
        return;
    }

    ani_boolean isUndefined = true;
    ani_ref onCompleted = nullptr;
    if (!GetFieldRef(env, listener, "onCompleted", onCompleted, isUndefined)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetFieldRef failed");
        return;
    }

    if (isUndefined) {
        TAG_LOGD(AAFwkTag::STARTUP, "onCompleted undefined");
        return;
    }

    OnCompletedCallbackFunc onCompletedCallback =
        [env, listener](const std::shared_ptr<StartupTaskResult> &result) {
            if (env == nullptr || listener == nullptr) {
                TAG_LOGE(AAFwkTag::STARTUP, "null env or listener");
                return;
            }

            ani_boolean isUndefined = true;
            ani_ref onCompleted = nullptr;
            if (!GetFieldRef(env, listener, "onCompleted", onCompleted, isUndefined)) {
                TAG_LOGE(AAFwkTag::STARTUP, "GetFieldRef failed");
                return;
            }
        
            if (!isUndefined) {
                TAG_LOGE(AAFwkTag::STARTUP, "onCompleted undefined");
                return;
            }
            ani_object resultObj = EtsStartupConfig::BuildResult(env, result);
            ani_ref fnReturn;
            std::vector<ani_ref> argv = { resultObj };
            status = env->FunctionalObject_Call(static_cast<ani_fn_object>(onCompleted), 1, argv, &fnReturn);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "FunctionalObject_Call status: %{public}d", status);
                 return;
            }
        }
    listener_ = std::make_shared<StartupListener>(onCompletedCallback);
}

ani_object EtsStartupConfig::BuildResult(ani_env *env, const std::shared_ptr<StartupTaskResult> &result)
{
    if (result == nullptr) {
        return CreateStsError(env, ERR_STARTUP_INTERNAL_ERROR,
            StartupUtils::GetErrorMessage(ERR_STARTUP_INTERNAL_ERROR));
    }
    if (result->GetResultCode() != ERR_OK) {
        return CreateStsError(env, result->GetResultCode(), result->GetResultMessage());
    }
    return CreateEtsNull(env);
}
} // namespace AbilityRuntime
} // namespace OHOS
