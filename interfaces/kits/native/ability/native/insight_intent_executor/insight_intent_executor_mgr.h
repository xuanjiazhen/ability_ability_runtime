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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTOR_MGR_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTOR_MGR_H

#include <string>

#include "ability_transaction_callback_info.h"
#include "insight_intent_executor.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_execute_result.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
using InsightIntentExecuteResult = AppExecFwk::InsightIntentExecuteResult;
using InsightIntentExecutorAsyncCallback = AppExecFwk::AbilityTransactionCallbackInfo<InsightIntentExecuteResult>;

class InsightIntentExecutorMgr : public std::enable_shared_from_this<InsightIntentExecutorMgr> {
    DECLARE_DELAYED_SINGLETON(InsightIntentExecutorMgr)

public:
    bool ExecuteInsightIntent(Runtime& runtime, const InsightIntentExecutorInfo &executeInfo,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback);
    static void TriggerCallbackInner(std::unique_ptr<InsightIntentExecutorAsyncCallback> callback, int32_t errCode);

    // Invoked by the reverse IPC cancel listener (on the IPC thread) when AMS
    // notifies this process that an in-flight intent was cancelled.
    void OnExecutorCanceled(uint64_t intentId, int32_t reason);

private:
    std::shared_ptr<InsightIntentExecutor> CreateExecutor(
        Runtime& runtime, InsightIntentType type, const InsightIntentExecutorInfo &executeInfo);
    void AddInsightIntentExecutor(uint64_t intentId, const std::shared_ptr<InsightIntentExecutor>& executor);
    void RemoveInsightIntentExecutor(uint64_t intentId);

    // Consume a cancellation that arrived before the executor was registered
    // (cold start race). Returns true and cancels the executor if such a
    // cancellation was pending, false otherwise.
    bool ConsumePendingCancel(uint64_t intentId, const std::shared_ptr<InsightIntentExecutor>& executor);

    std::mutex mutex_;
    std::map<uint64_t, std::shared_ptr<InsightIntentExecutor>> insightIntentExecutors_;
    std::map<uint64_t, int32_t> pendingCancel_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTOR_MGR_H
