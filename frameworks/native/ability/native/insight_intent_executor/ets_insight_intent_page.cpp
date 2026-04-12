/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "ets_insight_intent_page.h"

#include "ability_transaction_callback_info.h"
#include "ets_insight_intent_context.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_result.h"

#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define ETS_EXPORT __attribute__((visibility("default")))

namespace OHOS::AbilityRuntime {
using State = EtsInsightIntentUtils::State;

InsightIntentExecutor *EtsInsightIntentPage::Create(Runtime &runtime)
{
    return new (std::nothrow) EtsInsightIntentPage(static_cast<ETSRuntime &>(runtime));
}

EtsInsightIntentPage::EtsInsightIntentPage(ETSRuntime &runtime) : runtime_(runtime)
{}

EtsInsightIntentPage::~EtsInsightIntentPage()
{
    state_ = State::DESTROYED;
    EtsInsightIntentUtils::DeleteReference(runtime_, etsObj_);
}

std::unique_ptr<AppExecFwk::ETSNativeReference> EtsInsightIntentPage::LoadEtsCode(
    const InsightIntentExecutorInfo &insightIntentInfo, ETSRuntime &runtime)
{
    auto executeParam = insightIntentInfo.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return nullptr;
    }

    std::string moduleName(executeParam->moduleName_);
    std::string srcPath(executeParam->moduleName_);
    return runtime.LoadModule(moduleName, srcPath, insightIntentInfo.hapPath, insightIntentInfo.esmodule,
        false, executeParam->srcEntrance_);
}

bool EtsInsightIntentPage::Init(const InsightIntentExecutorInfo &insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "EtsInsightIntentPage::Init called");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    InsightIntentExecutor::Init(insightIntentInfo);

    if (insightIntentInfo.executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null execute param");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    etsObj_ = EtsInsightIntentPage::LoadEtsCode(insightIntentInfo, runtime_);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null etsObj_");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    return true;
}

bool EtsInsightIntentPage::HandleExecuteIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam,
    void *pageLoader, std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, bool &isAsync)
{
    (void)executeParam;
    (void)pageLoader;
    STATE_PATTERN_NAIVE_ACCEPT(State::INITIALIZED, false);
    state_ = State::EXECUTING;

    if (callback == nullptr || callback->IsEmpty()) {
        TAG_LOGE(AAFwkTag::INTENT, "null callback");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    callback_ = std::move(callback);
    isAsync = false;
    auto result = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    ReplySucceededInner(result);
    return true;
}

void EtsInsightIntentPage::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    state_ = State::INVALID;
    EtsInsightIntentUtils::ReplyFailed(callback_.release(), innerErr);
}

void EtsInsightIntentPage::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    state_ = State::EXECUTATION_DONE;
    EtsInsightIntentUtils::ReplySucceeded(callback_.release(), resultCpp);
}

bool EtsInsightIntentPage::ExecuteIntentCheckError()
{
    ReplyFailedInner();
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}
} // namespace OHOS::AbilityRuntime

ETS_EXPORT extern "C" OHOS::AbilityRuntime::InsightIntentExecutor *OHOS_ETS_Insight_Intent_Page_Create(
    OHOS::AbilityRuntime::Runtime &runtime)
{
    return OHOS::AbilityRuntime::EtsInsightIntentPage::Create(runtime);
}
