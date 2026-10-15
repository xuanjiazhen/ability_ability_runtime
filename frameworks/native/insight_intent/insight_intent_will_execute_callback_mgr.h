/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_WILL_EXECUTE_CALLBACK_MGR_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_WILL_EXECUTE_CALLBACK_MGR_H

#include <functional>
#include <mutex>

#include "insight_intent_will_execute_callback_stub.h"

namespace OHOS {
namespace AbilityRuntime {
using InsightIntentExecutionInfo = AppExecFwk::InsightIntentExecutionInfo;

class InsightIntentWillExecuteCallbackMgr : public AAFwk::InsightIntentWillExecuteCallbackStub,
    public std::enable_shared_from_this<InsightIntentWillExecuteCallbackMgr> {
public:
    InsightIntentWillExecuteCallbackMgr() = default;
    ~InsightIntentWillExecuteCallbackMgr() override = default;

    static sptr<InsightIntentWillExecuteCallbackMgr> GetInstance();

    int32_t Register(const std::function<void(const InsightIntentExecutionInfo &)> &callback);
    int32_t Unregister();
    void OnWillInsightIntentExecute(const InsightIntentExecutionInfo &info) override;

private:
    static std::mutex instanceMutex_;
    static sptr<InsightIntentWillExecuteCallbackMgr> instance_;

    std::mutex callbackMutex_;
    std::function<void(const InsightIntentExecutionInfo &)> callback_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_WILL_EXECUTE_CALLBACK_MGR_H