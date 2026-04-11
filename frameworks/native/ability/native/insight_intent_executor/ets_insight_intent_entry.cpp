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

#include "ets_insight_intent_entry.h"

#include <sstream>

#include "ability_transaction_callback_info.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "ets_insight_intent_context.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "insight_intent_execute_result.h"
#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define ETS_EXPORT __attribute__((visibility("default")))

namespace OHOS::AbilityRuntime {
using State = EtsInsightIntentUtils::State;

namespace {
constexpr const char *ENTRY_EXECUTOR_CLASS_NAME =
    "@ohos.app.ability.InsightIntentEntryExecutor.InsightIntentEntryExecutor";
constexpr const char *EXECUTE_MODE_ENUM_NAME = "@ohos.app.ability.insightIntent.insightIntent.ExecuteMode";
constexpr const char *CHECK_PROMISE_SIGNATURE = "Y:z";
constexpr const char *CALL_PROMISE_SIGNATURE = "C{std.core.Promise}:";
} // namespace

InsightIntentExecutor *EtsInsightIntentEntry::Create(Runtime &runtime)
{
    return new (std::nothrow) EtsInsightIntentEntry(static_cast<ETSRuntime &>(runtime));
}

EtsInsightIntentEntry::EtsInsightIntentEntry(ETSRuntime &runtime) : runtime_(runtime) {}

EtsInsightIntentEntry::~EtsInsightIntentEntry()
{
    state_ = State::DESTROYED;
    EtsInsightIntentUtils::DeleteReference(runtime_, contextObj_);
    EtsInsightIntentUtils::DeleteReference(runtime_, etsObj_);
}

std::unique_ptr<AppExecFwk::ETSNativeReference> EtsInsightIntentEntry::LoadEtsCode(
    const InsightIntentExecutorInfo &insightIntentInfo, ETSRuntime &runtime)
{
    auto executeParam = insightIntentInfo.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return nullptr;
    }

    std::string moduleName(executeParam->moduleName_);
    std::string srcPath(executeParam->moduleName_);

    // Convert @normalized format to original path
    std::string srcEntrance = executeParam->srcEntrance_;
    const std::string normalizedPrefix = "@normalized:N&&&";
    if (srcEntrance.find(normalizedPrefix) == 0) {
        srcEntrance = srcEntrance.substr(normalizedPrefix.length());
        if (!srcEntrance.empty() && srcEntrance.back() == '&') {
            srcEntrance.pop_back();
        }
    }
    srcEntrance = srcEntrance + ':' + insightIntentInfo.decoratorClass;
    TAG_LOGE(AAFwkTag::INTENT, "testTag srcEntrance: %{public}s", srcEntrance.c_str());
    TAG_LOGE(AAFwkTag::INTENT, "testTag executeParam->srcEntrance_: %{public}s", executeParam->srcEntrance_.c_str());
    return runtime.LoadModule(
        moduleName, srcPath, insightIntentInfo.hapPath, insightIntentInfo.esmodule, false, srcEntrance);
}

bool EtsInsightIntentEntry::Init(const InsightIntentExecutorInfo &insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "EtsInsightIntentEntry::Init called");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    InsightIntentExecutor::Init(insightIntentInfo);

    if (insightIntentInfo.executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null execute param");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto *env = runtime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    if (etsObj_ == nullptr) {
        etsObj_ = EtsInsightIntentEntry::LoadEtsCode(insightIntentInfo, runtime_);
        if (etsObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null etsObj_");
            STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
        }
    }

    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null Context");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    contextCpp_ = std::make_shared<EtsInsightIntentContext>(context);
    contextObj_ = CreateEtsInsightIntentContext(env, contextCpp_.get());
    if (contextObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null contextObj_");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    ani_status status = env->Object_SetFieldByName_Ref(etsObj_->aniObj, "context", contextObj_->aniRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "set context failed status: %{public}d", status);
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    ani_class executorClass = nullptr;
    status = env->FindClass(ENTRY_EXECUTOR_CLASS_NAME, &executorClass);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "find executor class failed status: %{public}d", status);
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    std::array functions = {
        ani_native_function { "nativeOnExecuteResult", nullptr, reinterpret_cast<void *>(OnExecuteResult) },
    };
    status = env->Class_BindNativeMethods(executorClass, functions.data(), functions.size());
    if (status != ANI_OK && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::INTENT, "bind native methods failed status: %{public}d", status);
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    status = env->Object_SetFieldByName_Long(etsObj_->aniObj, "nativeExecutor", reinterpret_cast<ani_long>(this));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "set nativeExecutor failed status: %{public}d", status);
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    context->SetExecuteMode(insightIntentInfo.executeParam->executeMode_);
    return true;
}

bool EtsInsightIntentEntry::HandleExecuteIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam,
    void *pageLoader, std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, bool &isAsync)
{
    TAG_LOGE(AAFwkTag::INTENT, "EtsInsightIntentEntry::HandleExecuteIntent called");
    STATE_PATTERN_NAIVE_ACCEPT(State::INITIALIZED, false);
    state_ = State::EXECUTING;

    if (callback == nullptr || callback->IsEmpty()) {
        TAG_LOGE(AAFwkTag::INTENT, "null callback");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    if (executeParam == nullptr || executeParam->insightIntentParam_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute param");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto *env = runtime_.GetAniEnv();
    if (env == nullptr || etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid runtime state");
        EtsInsightIntentUtils::ReplyFailed(callback.release());
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    callback_ = std::move(callback);
    auto aniWantParams = AppExecFwk::WrapWantParams(env, *executeParam->insightIntentParam_);
    if (aniWantParams == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "WrapWantParams failed");
        return ExecuteIntentCheckError();
    }

    TAG_LOGE(AAFwkTag::INTENT, "testTag executeParam->executeMode_ is %{public}d", executeParam->executeMode_);
    ani_enum_item executeModeItem {};
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToEts(
        env, EXECUTE_MODE_ENUM_NAME, executeParam->executeMode_, executeModeItem);
    ani_status status = env->Object_SetPropertyByName_Ref(etsObj_->aniObj, "executeMode", executeModeItem);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "set executeMode failed, status: %{public}d", status);
        return ExecuteIntentCheckError();
    }
    if (pageLoader != nullptr) {
        auto mode = static_cast<InsightIntentExecuteMode>(executeParam->executeMode_);
        const char *propertyName =
            mode == InsightIntentExecuteMode::UIEXTENSION_ABILITY ? "uiExtensionSession" : "windowStage";
        if (env->Object_SetPropertyByName_Ref(etsObj_->aniObj, propertyName, reinterpret_cast<ani_ref>(pageLoader)) !=
            ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "set page loader failed");
            return ExecuteIntentCheckError();
        }
    }

    ani_ref result = EtsInsightIntentUtils::CallObjectMethod(runtime_, etsObj_, true, "onExecute", nullptr);
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "onExecute failed");
        return ExecuteIntentCheckError();
    }

    ani_method method = nullptr;
    if (env->Class_FindMethod(etsObj_->aniCls, "checkPromise", CHECK_PROMISE_SIGNATURE, &method) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "find checkPromise failed");
        return ExecuteIntentCheckError();
    }
    ani_boolean isPromise = ANI_FALSE;
    if (env->Object_CallMethod_Boolean(etsObj_->aniObj, method, &isPromise, reinterpret_cast<ani_object>(result)) !=
        ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "call checkPromise failed");
        return ExecuteIntentCheckError();
    }
    isAsync_ = static_cast<bool>(isPromise);
    isAsync = isAsync_;
    if (isAsync_) {
        TAG_LOGE(AAFwkTag::INTENT, "EtsInsightIntentEntry::HandleExecuteIntent isAsync_");
        if (env->Class_FindMethod(etsObj_->aniCls, "callPromise", CALL_PROMISE_SIGNATURE, &method) != ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "find callPromise failed");
            return ExecuteIntentCheckError();
        }
        ani_boolean errorExists;
        // ADD_LOG替换成自己的日志函数和宏
        TAG_LOGE(AAFwkTag::INTENT, "ETSRUNTIME loadClassMethod start");
        env->ExistUnhandledError(&errorExists);
        if ((status = env->Object_CallMethod_Void(etsObj_->aniObj, method, reinterpret_cast<ani_object>(result))) !=
            ANI_OK) {
            TAG_LOGE(AAFwkTag::INTENT, "callPromise failed, status: %{public}d", status);
            std::ostringstream buffer;
            std::streambuf *oldStderr = std::cerr.rdbuf(buffer.rdbuf());
            status = env->DescribeError();
            std::cerr.rdbuf(oldStderr);
            std::string output = buffer.str();
            TAG_LOGE(AAFwkTag::INTENT, "loadClassMethod failed, output: %{public}s", output.c_str());
            env->ExistUnhandledError(&errorExists);
            return ExecuteIntentCheckError();
        }
        return true;
    }
    TAG_LOGE(AAFwkTag::INTENT, "EtsInsightIntentEntry::HandleExecuteIntent !isAsync_");
    return HandleResultReturnedFromEtsFunc(env, result, false);
}

void EtsInsightIntentEntry::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    state_ = State::INVALID;
    EtsInsightIntentUtils::ReplyFailed(callback_.release(), innerErr);
}

void EtsInsightIntentEntry::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    state_ = State::EXECUTATION_DONE;
    TAG_LOGE(AAFwkTag::INTENT, "EtsInsightIntentEntry::ReplySucceededInner called");
    EtsInsightIntentUtils::ReplySucceeded(callback_.release(), resultCpp);
}

bool EtsInsightIntentEntry::ExecuteIntentCheckError()
{
    ReplyFailedInner();
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}

bool EtsInsightIntentEntry::HandleResultReturnedFromEtsFunc(ani_env *env, ani_ref result, bool isAsync)
{
    TAG_LOGE(AAFwkTag::INTENT, "testTag HandleResultReturnedFromEtsFunc 1");
    if (env == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid env or result");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    TAG_LOGE(AAFwkTag::INTENT, "testTag HandleResultReturnedFromEtsFunc 2");
    auto resultCpp = EtsInsightIntentUtils::GetResultFromEts(env, result);
    if (resultCpp == nullptr) {
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    TAG_LOGE(AAFwkTag::INTENT, "testTag HandleResultReturnedFromEtsFunc 3");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    TAG_LOGE(AAFwkTag::INTENT, "testTag HandleResultReturnedFromEtsFunc 4");
    if (context->GetDelayReturnMode() == InsightIntentReturnMode::FUNCTION) {
        resultCpp->isNeedDelayResult = true;
    } else {
        InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(context->GetIntentId());
    }

    if (isAsync) {
        EtsInsightIntentUtils::ReplySucceeded(callback_.release(), resultCpp);
        return true;
    }
    TAG_LOGE(AAFwkTag::INTENT, "testTag HandleResultReturnedFromEtsFunc 5");
    ReplySucceededInner(resultCpp);
    return true;
}

void EtsInsightIntentEntry::OnExecuteResult(ani_env *env, ani_object aniObj, ani_object result)
{
    TAG_LOGE(AAFwkTag::INTENT, "EtsInsightIntentEntry::OnExecuteResult called");
    if (env == nullptr || aniObj == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid onExecute result callback input");
        return;
    }
    TAG_LOGE(AAFwkTag::INTENT, "EtsInsightIntentEntry::OnExecuteResult 1");
    ani_long nativeExecutor = 0;
    if (env->Object_GetFieldByName_Long(aniObj, "nativeExecutor", &nativeExecutor) != ANI_OK) {
        TAG_LOGE(AAFwkTag::INTENT, "get nativeExecutor failed");
        return;
    }
    TAG_LOGE(AAFwkTag::INTENT, "EtsInsightIntentEntry::OnExecuteResult 2");
    auto *executor = reinterpret_cast<EtsInsightIntentEntry *>(nativeExecutor);
    if (executor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null native executor");
        return;
    }
    TAG_LOGE(AAFwkTag::INTENT, "EtsInsightIntentEntry::OnExecuteResult 3");
    executor->HandleResultReturnedFromEtsFunc(env, result, true);
}
} // namespace OHOS::AbilityRuntime

ETS_EXPORT extern "C" OHOS::AbilityRuntime::InsightIntentExecutor *OHOS_ETS_Insight_Intent_Entry_Create(
    OHOS::AbilityRuntime::Runtime &runtime)
{
    return OHOS::AbilityRuntime::EtsInsightIntentEntry::Create(runtime);
}
