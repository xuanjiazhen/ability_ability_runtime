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

#include "cj_insight_intent_executor_impl_object.h"
#include "hilog_tag_wrapper.h"
#include <want_params.h>
#include "securec.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

using WindowStagePtr = void*;

struct CJInsightIntentExecutorFuncs {
    int64_t (*createCjInsightIntentExecutor)(const char* name);
    int64_t (*cjInsightIntentExecutorOnExecuteInUIAbilityForegroundMode)(
        int64_t id, const char* name, const char* param, WindowStagePtr cjWindowStage);
};
} // namespace AbilityRuntime
} // namespace OHOS

namespace {
static OHOS::AbilityRuntime::CJInsightIntentExecutorFuncs g_cjFuncs {};
static const int32_t CJ_OBJECT_ERR_CODE = -1;
} // namespace

namespace OHOS {
namespace AbilityRuntime {

char* CreateCStringFromString(const std::string& source)
{
    if (source.size() == 0) {
        return nullptr;
    }
    size_t length = source.size() + 1;
    auto res = static_cast<char*>(malloc(length));
    if (res == nullptr) {
        TAG_LOGE(AAFwkTag::DEFAULT, "null res");
        return nullptr;
    }
    if (strcpy_s(res, length, source.c_str()) != 0) {
        free(res);
        TAG_LOGE(AAFwkTag::DEFAULT, "Strcpy failed");
        return nullptr;
    }
    return res;
}


int32_t CJInsightIntentExecutorImplObj::Init(const std::string& abilityName)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");

    if (g_cjFuncs.createCjInsightIntentExecutor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "createCjInsightIntentExecutor is not registered");
        return CJ_OBJECT_ERR_CODE;
    }

    cjID_ = g_cjFuncs.createCjInsightIntentExecutor(abilityName.c_str());
    if (cjID_ == 0) {
        TAG_LOGE(AAFwkTag::INTENT,
            "Failed to Init CJUIExtensionObject. CJExtAbility: %{public}s is not registered", abilityName.c_str());
        return CJ_OBJECT_ERR_CODE;
    }

    return 0;
}

int64_t CJInsightIntentExecutorImplObj::onExecuteInUIAbilityForegroundMode(const std::string& name,
    const AAFwk::WantParams &wantParams,
    OHOS::Rosen::CJWindowStageImpl* cjWindowStage)
{
    if (g_cjFuncs.cjInsightIntentExecutorOnExecuteInUIAbilityForegroundMode == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "cjInsightIntentExecutorOnExecuteInUIAbilityForegroundMode is not registered");
        return -1;
    }
    auto nameCStr = CreateCStringFromString(name);
    auto paramsCStr = CreateCStringFromString(OHOS::AAFwk::WantParamWrapper(wantParams).ToString());
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(cjWindowStage);
    auto ret = g_cjFuncs.cjInsightIntentExecutorOnExecuteInUIAbilityForegroundMode(cjID_, nameCStr, paramsCStr, windowStage);
    free(nameCStr);
    free(paramsCStr);
    return ret;
}

extern "C" {
CJ_EXPORT void FFIRegisterCJInsightIntentExecutorFuncs(void (*registerFunc)(CJInsightIntentExecutorFuncs*))
{
    TAG_LOGD(AAFwkTag::INTENT, "FFIRegisterCJExtAbilityFuncs start");
    if (g_cjFuncs.createCjInsightIntentExecutor != nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "Repeated registration for cj functions of CJInsightIntentExecutor");
        return;
    }

    if (registerFunc == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "FFIRegisterCJInsightIntentExecutorFuncs failed, registerFunc is nullptr");
        return;
    }

    registerFunc(&g_cjFuncs);
    TAG_LOGD(AAFwkTag::INTENT, "FFIRegisterCJInsightIntentExecutorFuncs end");
}
} // extern "C"
} // namespace AbilityRuntime
} // namespace OHOS
