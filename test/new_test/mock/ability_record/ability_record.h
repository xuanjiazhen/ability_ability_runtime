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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_RECORD_H

#include <string>

#include "ability_info.h"
#include "ability_state.h"
#include "ability_scheduler_interface.h"
#include "ability_connect_callback_interface.h"
#include "application_info.h"
#include "session_info.h"
#include "start_options.h"
#include "want.h"
#include "ability_start_setting.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AAFwk {
class AbilityResult {
public:
    AbilityResult() = default;
    AbilityResult(int requestCode, int resultCode, const Want &resultWant) {}
    virtual ~AbilityResult() {}
};

class SystemAbilityCallerRecord {
};

struct CallerAbilityInfo {
public:
    int32_t callerTokenId = 0;
    int32_t callerUid = 0;
    int32_t callerPid = 0;
    int32_t callerAppCloneIndex = 0;
    std::string callerNativeName;
    std::string callerBundleName;
    std::string callerAbilityName;
};

class CallerRecord {
};

enum AbilityCallType {
    INVALID_TYPE = 0,
    CALL_REQUEST_TYPE,
    START_OPTIONS_TYPE,
    START_SETTINGS_TYPE,
    START_EXTENSION_TYPE,
};

enum CollaboratorType {
    DEFAULT_TYPE = 0,
    RESERVE_TYPE,
    OTHERS_TYPE
};

struct AbilityRequest {
    bool uriReservedFlag = false;

    bool isShellCall = false;
    // ERMS embedded atomic service
    bool isQueryERMS = false;
    bool isEmbeddedAllowed = false;

    int32_t uid = 0;
    int32_t collaboratorType = CollaboratorType::DEFAULT_TYPE;
    int32_t callerTokenRecordId = -1;
    int32_t userId = -1;

    uint32_t specifyTokenId = 0;
    int callerUid = -1;         // call ability
    int requestCode = -1;
    AbilityCallType callType = AbilityCallType::INVALID_TYPE;           // call ability
    int64_t restartTime = 0;
    sptr<IRemoteObject> callerToken = nullptr;          // call ability
    sptr<IRemoteObject> asCallerSourceToken = nullptr;          // call ability
    sptr<IAbilityConnection> connect = nullptr;
    sptr<IRemoteObject> abilityInfoCallback = nullptr;
    sptr<SessionInfo> sessionInfo;
    std::shared_ptr<AbilityStartSetting> startSetting = nullptr;
    std::shared_ptr<ProcessOptions> processOptions = nullptr;
    std::shared_ptr<StartWindowOption> startWindowOption = nullptr;
    std::vector<AppExecFwk::SupportWindowMode> supportWindowModes;
    AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    AppExecFwk::ExtensionProcessMode extensionProcessMode = AppExecFwk::ExtensionProcessMode::UNDEFINED;
    std::string specifiedFlag;
    std::string customProcess;
    std::string reservedBundleName;
    std::string appId;
    std::string startTime;
    Want want;
    AppExecFwk::AbilityInfo abilityInfo;
    AppExecFwk::ApplicationInfo appInfo;

    void Voluation(const Want &srcWant, int srcRequestCode,
        const sptr<IRemoteObject> &srcCallerToken, const std::shared_ptr<AbilityStartSetting> srcStartSetting = nullptr,
        int srcCallerUid = -1) {}
};
// new version
enum ResolveResultType {
    OK_NO_REMOTE_OBJ = 0,
    OK_HAS_REMOTE_OBJ,
    NG_INNER_ERROR,
};

enum class AbilityWindowState {
    FOREGROUND = 0,
    BACKGROUND,
    TERMINATE,
    FOREGROUNDING,
    BACKGROUNDING,
    TERMINATING
};

enum class AbilityVisibilityState {
    INITIAL = 0,
    FOREGROUND_HIDE,
    FOREGROUND_SHOW,
    UNSPECIFIED,
};

struct LaunchDebugInfo {
};

enum AbilityLifeCycleState {
    ABILITY_STATE_INITIAL = 0,
    ABILITY_STATE_INACTIVE,
    ABILITY_STATE_ACTIVE,
    ABILITY_STATE_BACKGROUND,
    ABILITY_STATE_STARTED_NEW,
    ABILITY_STATE_FOREGROUND_NEW,
    ABILITY_STATE_BACKGROUND_NEW,
    ABILITY_STATE_STOPED_NEW,
    ABILITY_STATE_FOREGROUND_FAILED,
    ABILITY_STATE_BACKGROUND_FAILED,
    ABILITY_STATE_INVALID_WINDOW_MODE,
    ABILITY_STATE_WINDOW_FREEZED,
    ABILITY_STATE_DO_NOTHING,
};

struct LifeCycleStateInfo {
    uint32_t sceneFlag = 0;
};

class Token;
class AbilityRecord {
public:
    AbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode = -1) {}

    AppExecFwk::AbilityInfo GetAbilityInfo() const
    {
        AppExecFwk::AbilityInfo info;
        return info;
    }

    AppExecFwk::ApplicationInfo GetApplicationInfo() const
    {
        AppExecFwk::ApplicationInfo info;
        return info;
    }

    bool GetAbilityForegroundingFlag() const
    {
        return false;
    }

    AbilityState GetAbilityState() const
    {
        return AbilityState::INITIAL;
    }

    bool IsForeground() const
    {
        return false;
    }

    sptr<IAbilityScheduler> GetScheduler() const
    {
        return nullptr;
    }

    sptr<SessionInfo> GetSessionInfo() const
    {
        return nullptr;
    }

    sptr<Token> GetToken() const
    {
        return nullptr;
    }

    Want GetWant() const
    {
        Want want;
        return want;
    }

    void SetResult(const std::shared_ptr<AbilityResult> &result) {}

    void SendResultToCallers(bool schedulerdied = false) {}

    void SaveResultToCallers(const int resultCode, const Want *resultWant) {}

    std::shared_ptr<AbilityRecord> GetCallerRecord() const
    {
        return nullptr;
    }

    static std::string ConvertAbilityState(const AbilityState &state)
    {
        return "";
    }

    static int ConvertLifeCycleToAbilityState(const AbilityLifeCycleState &state)
    {
        return 0;
    }

    int GetRecordId() const
    {
        return 0;
    }

    void DumpAbilityInfoDone(std::vector<std::string> &infos) {}

    int64_t GetStartTime() const
    {
        return 0;
    }

    void ShareData(const int32_t &uniqueId) {}

    bool IsAbilityState(const AbilityState &state) const
    {
        return false;
    }

    int32_t GetAppIndex() const
    {
        return 0;
    }

    void ContinueAbility(const std::string &deviceId, uint32_t versionCode) {}

    void NotifyContinuationResult(int32_t result) {}

    int32_t GetMissionId() const
    {
        return 0;
    }

    int32_t GetUid()
    {
        return 0;
    }

    pid_t GetPid()
    {
        return 0;
    }

    int32_t GetOwnerMissionUserId()
    {
        return 0;
    }

    void SetSceneFlag(uint32_t sceneFlag) {}

    bool IsClearMissionFlag()
    {
        return false;
    }

    LifeCycleStateInfo lifeCycleStateInfo_;

    int64_t GetAbilityRecordId() const
    {
        return 0;
    }

    void SetLockedState(bool lockedState) {}

    int32_t CreateModalUIExtension(const Want &want)
    {
        return 0;
    }

    AppExecFwk::ElementName GetElementName() const
    {
        AppExecFwk::ElementName name;
        return name;
    }

    bool IsDebugApp() const
    {
        return false;
    }

    void OnProcessDied() {}

    std::string GetURI() const
    {
        return "";
    }

    bool IsSceneBoard() const
    {
        return false;
    }

    void GrantUriPermission(const std::vector<std::string> &uriVec, int32_t flag,
        const std::string &targetBundleName, uint32_t callerTokenId) {}

    bool GetSecurityFlag() const
    {
        return false;
    }

    void SendResult(bool isSandboxApp, uint32_t tokeId) {}

    bool IsPluginAbility()
    {
        return false;
    }
};

class IAbilityToken : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.AbilityToken");
};

class Token : public IRemoteStub<IAbilityToken> {
public:
    static std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
