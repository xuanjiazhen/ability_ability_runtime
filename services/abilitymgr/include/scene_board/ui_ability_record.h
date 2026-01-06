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

#ifndef OHOS_ABILITY_RUNTIME_UI_ABILITY_RECORD_H
#define OHOS_ABILITY_RUNTIME_UI_ABILITY_RECORD_H

#include "ability_record.h"

namespace OHOS {
namespace AAFwk {
class UIAbilityRecord;
using UIAbilityRecordPtr = std::shared_ptr<UIAbilityRecord>;

class UIAbilityRecord : public AbilityRecord {
public:
    UIAbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode);
    
    static UIAbilityRecordPtr CreateAbilityRecord(const AbilityRequest &abilityRequest);
    static UIAbilityRecordPtr FromBaseRecord(std::shared_ptr<AbilityRecord> abilityRecord);
    static UIAbilityRecordPtr FromAbilityToken(sptr<IRemoteObject> token);

    AbilityRecordType GetAbilityRecordType() override;
    void OnProcessDied() override;

    void SetExitReasonLoaded()
    {
        exitReasonLoaded_ = true;
    }
    bool IsExitReasonLoaded() const
    {
        return exitReasonLoaded_;
    }

    inline void SetIsKillPrecedeStart(bool isKillPrecedeStart)
    {
        isKillPrecedeStart_.store(isKillPrecedeStart);
    }
    inline bool IsKillPrecedeStart()
    {
        return isKillPrecedeStart_.load();
    }

    bool GetPrelaunchFlag() const
    {
        return isPrelaunch_;
    }
    void SetPrelaunchFlag(bool isPrelaunch)
    {
        isPrelaunch_ = isPrelaunch;
    }

    void ScheduleCollaborate(const Want &want);

    bool IsHook () const
    {
        return isHook_;
    }
    inline void SetIsHook(bool isHook)
    {
        isHook_ = isHook;
    }

    bool GetHookOff () const
    {
        return hookOff_;
    }
    inline void SetHookOff(bool hookOff)
    {
        hookOff_ = hookOff;
    }

    inline void SetBackgroundDrivenFlag(bool isLastWantBackgroundDriven)
    {
        isLastWantBackgroundDriven_.store(isLastWantBackgroundDriven);
    }
    inline bool IsLastWantBackgroundDriven() const
    {
        return isLastWantBackgroundDriven_.load();
    }

    inline void SetLastWant(std::shared_ptr<Want> lastWant)
    {
        lastWant_ = lastWant;
    }
    inline bool HasLastWant() const
    {
        return lastWant_ != nullptr;
    }

    void ForegroundAbility(uint32_t sceneFlag, bool hasLastWant);
    /**
     * prepare terminate ability.
     *
     * @param isSCBCall, if the call is from SCB.
     * @return Returns true on stop terminating; returns false on terminate.
     */
    bool PrepareTerminateAbility(bool isSCBCall);

    /**
     * prepare terminate ability done.
     *
     * @param isTerminate, the result of the onPrepareToTerminate/onPrepareToTerminateAsync.
     */
    void PrepareTerminateAbilityDone(bool isTerminate);
    void CancelPrepareTerminate();
private:
    bool exitReasonLoaded_ = false;
    bool hookOff_ = false;
    std::atomic_bool isKillPrecedeStart_ = false;
    std::atomic_bool isLastWantBackgroundDriven_ = false;

    std::mutex isPrepareTerminateAbilityMutex_;
    std::condition_variable isPrepareTerminateAbilityCv_;
    std::atomic_bool isPrepareTerminateAbilityCalled_ = false;
    std::atomic_bool isPrepareTerminateAbilityDone_ = false;
    bool isPrepareTerminate_ = false;

    std::shared_ptr<Want> lastWant_;

    std::mutex collaborateWantLock_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_ABILITY_RECORD_H