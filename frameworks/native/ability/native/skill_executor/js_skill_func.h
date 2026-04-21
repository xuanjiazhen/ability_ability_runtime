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

#ifndef OHOS_ABILITY_RUNTIME_JS_SKILL_FUNC_H
#define OHOS_ABILITY_RUNTIME_JS_SKILL_FUNC_H

#include <string>

#include "js_runtime.h"
#include "skill_executor.h"

namespace OHOS {
namespace AbilityRuntime {

class JsSkillFunc : public SkillExecutor {
public:
    static std::shared_ptr<JsSkillFunc> Create(JsRuntime &runtime);
    explicit JsSkillFunc(JsRuntime &runtime);
    ~JsSkillFunc() override;

    bool Init(const SkillExecutorInfo &skillInfo) override;
    bool ExecuteSkill(
        std::shared_ptr<SkillExecuteParam> skillParam,
        std::unique_ptr<SkillExecutorAsyncCallback> callback,
        bool &isAsync) override;

private:
    bool LoadModule(const SkillExecutorInfo &skillInfo);
    bool CallExecuteSkill(const std::shared_ptr<SkillExecuteParam> &skillParam, bool &isAsync);
    void ReplyFailed();
    void ReplySucceeded(const std::string &resultStr);
    bool HandleResult(napi_value result, bool &isAsync);

    static void OnPromiseResolve(napi_env env, napi_callback_info info);
    static void OnPromiseReject(napi_env env, napi_callback_info info);

    JsRuntime &runtime_;
    std::unique_ptr<SkillExecutorAsyncCallback> callback_;
    napi_ref moduleRef_ = nullptr;
};

} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_SKILL_FUNC_H
