/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_DRIVER_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_DRIVER_UTILS_H

#include "insight_intent_execute_result.h"
#include "js_insight_intent_info.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsExecuteResult(napi_env env, const AppExecFwk::InsightIntentExecuteResult &result);
napi_value CreateJsWantParams(napi_env env, const AAFwk::WantParams &wantParams);
napi_value CreateJsLinkInfo(napi_env env, const JsLinkInfo &info);
napi_value CreateJsPageInfo(napi_env env, const JsPageInfo &info);
napi_value CreateJsEntryInfo(napi_env env, const JsEntryInfo &info);
napi_value CreateJsFunctionInfo(napi_env env, const JsFunctionInfo &info);
napi_value CreateJsFormInfo(napi_env env, constJsFormInfo &info);
napi_value CreateJsInsightIntentInfo(napi_env env, const JsInsightIntentInfo &info);
napi_value CreateJsInsightIntentInfoArray(napi_env env, const std::vector<JsInsightIntentInfo> &infos);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_DRIVER_UTILS_H
