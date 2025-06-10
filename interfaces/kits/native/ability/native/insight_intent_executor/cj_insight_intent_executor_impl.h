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

#ifndef OHOS_ABILITY_RUNTIME_CJ_INSIGHT_INTENT_EXECUTOR_IMPL_H
#define OHOS_ABILITY_RUNTIME_CJ_INSIGHT_INTENT_EXECUTOR_IMPL_H

#include "cj_insight_intent_executor.h"
#include "cj_insight_intent_executor_impl_object.h"

namespace OHOS::Rosen {
    class CJWindowStageImpl;
}

namespace OHOS {
namespace AppExecFwk {
    struct InsightIntentExecuteResult;
} // namespace AAFwk
namespace AbilityRuntime {

class CJInsightIntentExecutorImpl final : public CJInsightIntentExecutor {
public:
    static std::shared_ptr<CJInsightIntentExecutorImpl> Create();
    enum class State {
        INVALID,
        CREATED,
        INITIALIZED,
        EXECUTING,
        EXECUTATION_DONE,
        DESTROYED
    };
private:
    explicit CJInsightIntentExecutorImpl();
public:
    CJInsightIntentExecutorImpl(const CJInsightIntentExecutorImpl&) = delete;
    CJInsightIntentExecutorImpl(const CJInsightIntentExecutorImpl&&) = delete;
    CJInsightIntentExecutorImpl& operator=(const CJInsightIntentExecutorImpl&) = delete;
    CJInsightIntentExecutorImpl& operator=(const CJInsightIntentExecutorImpl&&) = delete;
    ~CJInsightIntentExecutorImpl() override;

    /**
     * @brief Init the intent executor and intent context.
     *
     * @param
     */
    bool Init(const CJInsightIntentExecutorInfo& insightIntentInfo) override;

    /**
     * @brief Handling the life cycle execute intent.
     *
     * @param
     *
     */
    bool HandleExecuteIntent(
        InsightIntentExecuteMode mode,
        const std::string& name,
        const AAFwk::WantParams& param,
        Rosen::CJWindowStageImpl* pageLoader,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
        bool& isAsync) override;

    inline State GetState() const
    {
        return state_;
    }

private:
    
    static void ReplyFailed(InsightIntentExecutorAsyncCallback* callback,
        InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED);
    static void ReplySucceeded(InsightIntentExecutorAsyncCallback* callback,
        std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);
    void ReplyFailedInner(InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_EXECUTE_REPLY_FAILED);
    void ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);
    bool ExecuteIntentCheckError();


    static bool CheckParametersUIAbilityForeground(Rosen::CJWindowStageImpl* windowStage);
    bool ExecuteInsightIntentUIAbilityForeground(
        const std::string& name,
        const AAFwk::WantParams& param,
        Rosen::CJWindowStageImpl* windowStage);

    // static bool CheckParametersUIAbilityBackground();
    // bool ExecuteInsightIntentUIAbilityBackground(
    //     const std::string& name,
    //     const AAFwk::WantParams& param);


    // Runtime& runtime_;
    State state_ = State::CREATED;
    CJInsightIntentExecutorImplObj cjObj_;
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback_;
    bool isAsync_ = false;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_CJ_INSIGHT_INTENT_EXECUTOR_IMPL_H
