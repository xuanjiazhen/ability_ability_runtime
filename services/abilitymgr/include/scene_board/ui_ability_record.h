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

#include <optional>

namespace OHOS {
namespace AAFwk {
class UIAbilityRecord;
using UIAbilityRecordPtr = std::shared_ptr<UIAbilityRecord>;

class UIAbilityRecord : public AbilityRecord {
public:
    UIAbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode);
    
    static std::shared_ptr<UIAbilityRecord> CreateAbilityRecord(const AbilityRequest &abilityRequest);

    AbilityRecordType GetAbilityRecordType() override;
    void Init(const AbilityRequest &abilityRequest) override;
    void ConfigLoadParam(AbilityRuntime::LoadParam &loadParam, Want &want) override;

    inline void SetKillReason(const std::string &reason)
    {
        killReason_ = reason;
    }
    inline const std::string &GetKillReason() const
    {
        return killReason_;
    }

    inline void SetIsKillPrecedeStart(bool isKillPrecedeStart)
    {
        isKillPrecedeStart_ = isKillPrecedeStart;
    }
    inline bool IsKillPrecedeStart() const
    {
        return isKillPrecedeStart_;
    }

    void SetPrelaunchFlag(bool isPrelaunch)
    {
        isPrelaunch_ = isPrelaunch;
    }
    bool GetPrelaunchFlag() const
    {
        return isPrelaunch_;
    }

    int32_t GetCollaboratorType() const
    {
        return collaboratorType_;
    }

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

    inline void SetLaunchWant(std::shared_ptr<Want> launchWant)
    {
        launchWant_ = launchWant;
    }
    inline std::shared_ptr<Want> GetLaunchWant() const
    {
        return launchWant_;
    }

    inline void SetLastWant(std::shared_ptr<Want> lastWant)
    {
        lastWant_ = lastWant;
    }
    inline bool HasLastWant() const
    {
        return lastWant_ != nullptr;
    }

    inline void SetBackgroundDrivenFlag(bool isLastWantBackgroundDriven)
    {
        isLastWantBackgroundDriven_ = isLastWantBackgroundDriven;
    }
    inline bool IsLastWantBackgroundDriven() const
    {
        return isLastWantBackgroundDriven_;
    }

    void ScheduleCollaborate(const Want &want);
    void ForegroundAbility(uint32_t sceneFlag, bool hasLastWant) override;

    void SetPromotePriority(bool promotePriority);
    bool GetPromotePriority();
    void PromotePriority();

private:
    bool isKillPrecedeStart_ = false;
    bool isPrelaunch_ = false;
    bool isPluginAbility_ = false;
    bool isHook_ = false;
    bool hookOff_ = false;
    bool isLastWantBackgroundDriven_ = false;

    int32_t collaboratorType_ = 0;
    std::optional<bool> uiAbilityProperty_;
    std::string killReason_;
    std::shared_ptr<Want> launchWant_ = nullptr;
    std::shared_ptr<Want> lastWant_ = nullptr;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_ABILITY_RECORD_H