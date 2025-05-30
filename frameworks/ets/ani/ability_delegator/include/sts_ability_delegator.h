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

#ifndef OHOS_ABILITY_RUNTIME_STS_ABILITY_DELEGATOR_H
#define OHOS_ABILITY_RUNTIME_STS_ABILITY_DELEGATOR_H

#include <string>
#include "ability_delegator.h"
#include "ability_monitor.h"
#include "ability_monitor_impl.h"
#include "ability_stage_monitor.h"
#include "context.h"
#include "sts_runtime.h"
#include "sts_ability_stage_monitor.h"
#include "sts_ability_delegator_registry.h"
#include "want.h"

namespace OHOS {
namespace AbilityDelegatorSts {
using namespace OHOS::AbilityRuntime;
class STSAbilityDelegator {
public:
    STSAbilityDelegator();
    ~STSAbilityDelegator();
    static void ExecuteShellCommand(ani_env* env, [[maybe_unused]]ani_object object,
        ani_string cmd, ani_double timeoutSecs, ani_object callback);

    static void FinishTestASync(ani_env* env, [[maybe_unused]]ani_object object,
        ani_string msg, ani_double code, ani_object callback);

    static ani_object CreateStsBaseContext(ani_env* aniEnv, ani_class contextClass,
        std::shared_ptr<AbilityRuntime::Context> context);

    static ani_object GetAppContext(ani_env* env, [[maybe_unused]]ani_object object, ani_class clss);

    static void PrintSync(ani_env *env, [[maybe_unused]]ani_class aniClass, ani_string msg);

    static void AddAbilityMonitorAsync(ani_env *env, [[maybe_unused]]ani_class aniClass,
        ani_object monitorObj, ani_object callback);

    static void AddAbilityMonitor(ani_env* env, [[maybe_unused]]ani_class aniClass, ani_object monitorObj);

    static void RemoveAbilityMonitorAsync(ani_env* env, [[maybe_unused]]ani_class aniClass,
        ani_object monitorObj, ani_object callback);

    static void RemoveAbilityMonitor(ani_env* env, [[maybe_unused]]ani_class aniClass, ani_object monitorObj);

    static void WaitAbilityMonitorAsync(ani_env* env, [[maybe_unused]]ani_class aniClass,
        ani_object monitorOb0, ani_double timeout, ani_object callback);

    static void AddAbilityStageMonitorAsync(ani_env* env, [[maybe_unused]]ani_class aniClass,
        ani_object stageMonitorObj, ani_object callback);

    static void AddAbilityStageMonitor(ani_env* env, [[maybe_unused]]ani_class aniClass, ani_object stageMonitorObj);

    static void RemoveAbilityStageMonitorAsync(ani_env* env, [[maybe_unused]]ani_class aniClass,
        ani_object stageMonitorObj, ani_object callback);

    static void RemoveAbilityStageMonitor(ani_env* env, [[maybe_unused]]ani_class aniClass,
        ani_object stageMonitorObj);

    static void WaitAbilityStageMonitorAsync(ani_env* env, [[maybe_unused]]ani_class aniClass,
        ani_object stageMonitorObj, ani_double timeout, ani_object callback);

    static void DoAbilityForegroundAsync(ani_env* env, [[maybe_unused]]ani_object object,
        ani_object abilityObj, ani_object callback);

    static void DoAbilityBackgroundAsync(ani_env* env, [[maybe_unused]]ani_object object,
        ani_object abilityObj, ani_object callback);

    static void PrintAsync(ani_env* env, [[maybe_unused]]ani_object object, ani_string msg, ani_object callback);

    static ani_double  GetAbilityState(ani_env* env, [[maybe_unused]]ani_object object, ani_object abilityObj);

    static void StartAbility(ani_env* env, [[maybe_unused]]ani_object object, ani_object wantObj, ani_object callback);

    static ani_ref GetCurrentTopAbilitySync(ani_env* env);

private:
    [[maybe_unused]]static void RetrieveStringFromAni(ani_env *env, ani_string string, std::string &resString);

    static ani_object WrapShellCmdResult(ani_env* env, std::unique_ptr<AppExecFwk::ShellCmdResult> result);

    static bool ParseAbilityMonitorPara(ani_env *env, ani_object monitorObj,
        std::shared_ptr<AbilityMonitorImpl> &monitorImpl);

    static bool ParseMonitorPara(ani_env *env, ani_object monitorObj,
        std::shared_ptr<AbilityMonitorImpl> &monitorImpl);

    static bool ParseMonitorParaInner(ani_env *env, ani_object monitorObj,
        std::shared_ptr<AbilityMonitorImpl> &monitorImpl);

    static bool ParseWaitAbilityMonitorPara(ani_env *env, ani_object monitorObj,
        std::shared_ptr<AbilityMonitorImpl> &monitorImpl);

    static bool ParseAbilityStageMonitorPara(ani_env *env, ani_object stageMonitorObj,
        std::shared_ptr<STSAbilityStageMonitor> &stageMonitor, bool &isExisted);

    static bool ParseStageMonitorPara(ani_env *env, ani_object stageMonitorObj,
        std::shared_ptr<STSAbilityStageMonitor> &stageMonitor, bool &isExisted);

    static bool ParseStageMonitorParaInner(ani_env *env, ani_object stageMonitorObj,
        std::shared_ptr<STSAbilityStageMonitor> &stageMonitor);

    static void AddStageMonitorRecord(ani_env *env, ani_object stageMonitorObj,
        const std::shared_ptr<STSAbilityStageMonitor> &stageMonitor);

    static void RemoveStageMonitorRecord(ani_env *env, ani_object stageMonitorObj);

    static bool ParseWaitAbilityStageMonitorPara(ani_env *env, ani_object stageMonitorObj,
        std::shared_ptr<STSAbilityStageMonitor> &stageMonitor);

    static bool ParseAbilityCommonPara(ani_env *env, ani_object abilityObj, sptr<OHOS::IRemoteObject> &remoteObject);

    static void AbilityLifecycleStateToSts(const AbilityDelegator::AbilityState &lifeState,
        AbilityLifecycleState &abilityLifeState);
};
} // namespace AbilityDelegatorSts
} // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_STS_ABILITY_DELEGATOR_H
