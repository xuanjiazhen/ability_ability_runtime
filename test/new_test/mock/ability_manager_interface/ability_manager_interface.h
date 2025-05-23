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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_INTERFACE_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_MANAGER_INTERFACE_H

#include <iremote_broker.h>
#include <list>
#include <vector>

#include "ability_connect_callback_interface.h"
#include "ability_running_info.h"
#include "ability_scheduler_interface.h"
#include "ability_start_setting.h"
#include "ability_state.h"
#include "ability_state_data.h"
#include "app_debug_listener_interface.h"
#include "auto_startup_info.h"
#include "dms_continueInfo.h"
#include "exit_reason.h"
#include "extension_running_info.h"
#include "free_install_observer_interface.h"
#include "iability_controller.h"
#include "iability_manager_collaborator.h"
#include "iacquire_share_data_callback_interface.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "iprepare_terminate_callback_interface.h"
#include "keep_alive_info.h"
#include "mission_info.h"
#include "query_erms_observer_interface.h"
#include "running_process_info.h"
#include "start_options.h"
#include "user_callback.h"
#include "ui_extension_ability_connect_info.h"
#include "uri.h"
#include "want.h"
#include "want_receiver_interface.h"
#include "want_sender_info.h"
#include "want_sender_interface.h"
#include "dialog_session_info.h"
#include "window_config.h"
#include "intent_exemption_info.h"
#include "ui_extension_window_command.h"
#ifdef SUPPORT_SCREEN
#include "window_manager_service_handler.h"
#endif
#include "ihidden_start_observer.h"
#include "ui_extension_session_info.h"
#include "inner_mission_info.h"
#include "mock_common.h"

namespace OHOS {
namespace AbilityRuntime {
class IStatusBarDelegate;
struct UIExtensionHostInfo {
    AppExecFwk::ElementName elementName_;
    std::string sessionName;
};
}

namespace AAFwk {
class IRemoteMissionListener : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.RemoteMissionListener");
};

class IRemoteOnListener : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.RemoteOnListener");
};

using KeepAliveInfo = AbilityRuntime::KeepAliveInfo;
using AutoStartupInfo = AbilityRuntime::AutoStartupInfo;
using InsightIntentExecuteParam = AppExecFwk::InsightIntentExecuteParam;
using InsightIntentExecuteResult = AppExecFwk::InsightIntentExecuteResult;
using UIExtensionAbilityConnectInfo = AbilityRuntime::UIExtensionAbilityConnectInfo;
using UIExtensionHostInfo = AbilityRuntime::UIExtensionHostInfo;
using UIExtensionSessionInfo = AbilityRuntime::UIExtensionSessionInfo;
#ifdef SUPPORT_SCREEN
using IAbilityFirstFrameStateObserver = AppExecFwk::IAbilityFirstFrameStateObserver;
#endif
using AtomicServiceStartupRule = AbilityRuntime::AtomicServiceStartupRule;
using PacMap = AppExecFwk::PacMap;

constexpr const char* ABILITY_MANAGER_SERVICE_NAME = "AbilityManagerService";
const int DEFAULT_INVAL_VALUE = -1;
const int DELAY_LOCAL_FREE_INSTALL_TIMEOUT = 40000;
const int DELAY_REMOTE_FREE_INSTALL_TIMEOUT = 30000 + DELAY_LOCAL_FREE_INSTALL_TIMEOUT;
constexpr const char* FROM_REMOTE_KEY = "freeInstallFromRemote";
class IAbilityManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.AbilityManager")

    virtual int StartSelfUIAbility(const Want &want)
    {
        return 0;
    }

    virtual int StartSelfUIAbilityWithStartOptions(const Want &want, const StartOptions &options)
    {
        return 0;
    }

    virtual int StartAbility(
        const Want &want,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    virtual int StartAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    virtual int StartAbilityWithSpecifyTokenId(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint32_t specifyTokenId,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    virtual int32_t StartAbilityByInsightIntent(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint64_t intentId,
        int32_t userId = DEFAULT_INVAL_VALUE) = 0;

    virtual int StartAbility(
        const Want &want,
        const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    virtual int StartAbility(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE) = 0;

    virtual int StartAbilityAsCaller(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int StartAbilityAsCaller(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        sptr<IRemoteObject> asCallerSourceToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int StartAbilityForResultAsCaller(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int StartAbilityForResultAsCaller(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        int requestCode = DEFAULT_INVAL_VALUE,
        int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int StartAbilityByUIContentSession(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        const sptr<SessionInfo> &sessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int StartAbilityByUIContentSession(
        const Want &want,
        const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken,
        const sptr<SessionInfo> &sessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE,
        int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int StartAbilityOnlyUIAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        uint32_t specifyTokenId)
    {
        return 0;
    }

    virtual int32_t StartExtensionAbility(
        const Want &want,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED)
    {
        return 0;
    }

    virtual int RequestModalUIExtension(const Want &want)
    {
        return 0;
    }

    virtual int PreloadUIExtensionAbility(const Want &want, std::string &hostBundleName,
        int32_t userId = DEFAULT_INVAL_VALUE, int32_t hostPid = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int ChangeAbilityVisibility(sptr<IRemoteObject> token, bool isShow)
    {
        return 0;
    }

        virtual int ChangeUIAbilityVisibilityBySCB(sptr<SessionInfo> sessionInfo, bool isShow)
    {
        return 0;
    }

    virtual int StartUIExtensionAbility(
        const sptr<SessionInfo> &extensionSessionInfo,
        int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int StartUIAbilityBySCB(sptr<SessionInfo> sessionInfo, bool &isColdStart, uint32_t sceneFlag = 0)
    {
        return 0;
    }

    virtual int StopExtensionAbility(
        const Want& want,
        const sptr<IRemoteObject>& callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType extensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED)
    {
        return 0;
    }

    virtual int GetAppMemorySize()
    {
        return 0;
    }

    virtual bool IsRamConstrainedDevice()
    {
        return false;
    }

    virtual AppExecFwk::ElementName GetTopAbility(bool isNeedLocalDeviceId = true)
    {
        return {};
    }

    virtual AppExecFwk::ElementName GetElementNameByToken(sptr<IRemoteObject> token,
        bool isNeedLocalDeviceId = true)
    {
        return {};
    }

    virtual int TerminateAbility(
        const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant = nullptr) = 0;

    virtual int BackToCallerAbilityWithResult(const sptr<IRemoteObject> &token, int resultCode,
        const Want *resultWant, int64_t callerRequestCode)
    {
        return 0;
    };

    virtual int32_t TerminateUIServiceExtensionAbility(const sptr<IRemoteObject> &token)
    {
        return 0;
    }

    virtual int TerminateUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
        int resultCode, const Want *resultWant = nullptr)
    {
        return 0;
    }

    virtual int CloseUIExtensionAbilityBySCB(const sptr<IRemoteObject> token)
    {
        return 0;
    }

    virtual int CloseUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool isUserRequestedExit,
        uint32_t sceneFlag = 0)
    {
        return 0;
    }

    virtual int SendResultToAbility(int requestCode, int resultCode, Want &resultWant)
    {
        return 0;
    }

    virtual int MoveAbilityToBackground(const sptr<IRemoteObject> &token)
    {
        return 0;
    };

    virtual int32_t MoveUIAbilityToBackground(const sptr<IRemoteObject> token)
    {
        return 0;
    };

    virtual int CloseAbility(const sptr<IRemoteObject> &token, int resultCode = DEFAULT_INVAL_VALUE,
        const Want *resultWant = nullptr) = 0;

    virtual int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser = false) = 0;

    virtual int MinimizeUIExtensionAbility(const sptr<SessionInfo> &extensionSessionInfo,
        bool fromUser = false)
    {
        return 0;
    };

    virtual int MinimizeUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool fromUser = false,
        uint32_t sceneFlag = 0)
    {
        return 0;
    };

    virtual int32_t ConnectAbility(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE) = 0;

    virtual int32_t ConnectAbilityCommon(
        const Want &want,
        const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken,
        AppExecFwk::ExtensionAbilityType extensionType,
        int32_t userId = DEFAULT_INVAL_VALUE,
        bool isQueryExtensionOnly = false)
    {
        return 0;
    }

    virtual int ConnectUIExtensionAbility(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<SessionInfo> &sessionInfo, int32_t userId = DEFAULT_INVAL_VALUE,
        sptr<UIExtensionAbilityConnectInfo> connectInfo = nullptr)
    {
        return 0;
    }

    virtual int DisconnectAbility(sptr<IAbilityConnection> connect) = 0;

    virtual sptr<IAbilityScheduler> AcquireDataAbility(
        const Uri &uri, bool tryBind, const sptr<IRemoteObject> &callerToken) = 0;

    virtual int ReleaseDataAbility(
        sptr<IAbilityScheduler> dataAbilityScheduler, const sptr<IRemoteObject> &callerToken) = 0;

    virtual int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token) = 0;

    virtual int AbilityTransitionDone(const sptr<IRemoteObject> &token,
        int state, const AppExecFwk::PacMap &saveData) = 0;

    virtual int AbilityWindowConfigTransitionDone(
        const sptr<IRemoteObject> &token, const WindowConfig &windowConfig)
        {
            return 0;
        }

    virtual int ScheduleConnectAbilityDone(
        const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &remoteObject) = 0;

    virtual int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token) = 0;

    virtual int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token) = 0;

    virtual int ScheduleCommandAbilityWindowDone(
        const sptr<IRemoteObject> &token,
        const sptr<AAFwk::SessionInfo> &sessionInfo,
        AAFwk::WindowCommand winCmd,
        AAFwk::AbilityCommand abilityCmd) = 0;

    virtual void DumpState(const std::string &args, std::vector<std::string> &state) = 0;
    virtual void DumpSysState(
        const std::string& args, std::vector<std::string>& state, bool isClient, bool isUserID, int UserID) = 0;

    virtual int StopServiceAbility(const Want &want, int32_t userId = DEFAULT_INVAL_VALUE,
        const sptr<IRemoteObject> &token = nullptr) = 0;

    virtual int KillProcess(const std::string &bundleName, bool clearPageStack = false, int32_t appIndex = 0) = 0;

    #ifdef ABILITY_COMMAND_FOR_TEST
    virtual int ForceTimeoutForTest(const std::string &abilityName, const std::string &state) = 0;
    #endif

    virtual int UninstallApp(const std::string &bundleName, int32_t uid)
    {
        return 0;
    }

    virtual int32_t UninstallApp(const std::string &bundleName, int32_t uid, int32_t appIndex)
    {
        return 0;
    }

    virtual int32_t UpgradeApp(const std::string &bundleName, const int32_t uid, const std::string &exitMsg,
        int32_t appIndex = 0)
    {
        return 0;
    }

    virtual sptr<IWantSender> GetWantSender(
        const WantSenderInfo &wantSenderInfo, const sptr<IRemoteObject> &callerToken, int32_t uid = -1) = 0;

    virtual int SendWantSender(sptr<IWantSender> target, const SenderInfo &senderInfo) = 0;

    virtual void CancelWantSender(const sptr<IWantSender> &sender) = 0;

    virtual int GetPendingWantUid(const sptr<IWantSender> &target) = 0;

    virtual int GetPendingWantUserId(const sptr<IWantSender> &target) = 0;

    virtual std::string GetPendingWantBundleName(const sptr<IWantSender> &target) = 0;

    virtual int GetPendingWantCode(const sptr<IWantSender> &target) = 0;

    virtual int GetPendingWantType(const sptr<IWantSender> &target) = 0;

    virtual void RegisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) = 0;

    virtual void UnregisterCancelListener(const sptr<IWantSender> &sender, const sptr<IWantReceiver> &receiver) = 0;

    virtual int GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want) = 0;

    virtual int GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info) = 0;

    virtual int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId, int32_t missionId,
        const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) = 0;

    virtual int ContinueMission(AAFwk::ContinueMissionInfo continueMissionInfo, const sptr<IRemoteObject> &callback)
    {
        return 0;
    }

    virtual int ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode) = 0;

    virtual int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) = 0;

    virtual void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) = 0;

    virtual int NotifyContinuationResult(int32_t missionId, int32_t result) = 0;

    virtual int LockMissionForCleanup(int32_t missionId) = 0;

    virtual int UnlockMissionForCleanup(int32_t missionId) = 0;

    virtual void SetLockedState(int32_t sessionId, bool lockedState)
    {
        return;
    }

    virtual int RegisterMissionListener(const sptr<IMissionListener> &listener) = 0;

    virtual int UnRegisterMissionListener(const sptr<IMissionListener> &listener) = 0;

    virtual int GetMissionInfos(
        const std::string &deviceId, int32_t numMax, std::vector<MissionInfo> &missionInfos) = 0;

    virtual int GetMissionInfo(const std::string &deviceId, int32_t missionId, MissionInfo &missionInfo) = 0;

    virtual int GetMissionSnapshot(const std::string& deviceId, int32_t missionId,
        MissionSnapshot& snapshot, bool isLowResolution) = 0;

    virtual int CleanMission(int32_t missionId) = 0;

    virtual int CleanAllMissions() = 0;

    virtual int MoveMissionToFront(int32_t missionId) = 0;

    virtual int MoveMissionToFront(int32_t missionId, const StartOptions &startOptions) = 0;

    virtual int MoveMissionsToForeground(const std::vector<int32_t>& missionIds, int32_t topMissionId)
    {
        return 0;
    }

    virtual int MoveMissionsToBackground(const std::vector<int32_t>& missionIds, std::vector<int32_t>& result)
    {
        return 0;
    }

    virtual int RegisterSessionHandler(const sptr<IRemoteObject> &object)
    {
        return 0;
    }

    virtual int StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t accountId = DEFAULT_INVAL_VALUE) = 0;

    virtual int StartAbilityByCallWithErrMsg(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t accountId, std::string &errMsg)
    {
        return 0;
    };

    virtual void CallRequestDone(const sptr<IRemoteObject> &token, const sptr<IRemoteObject> &callStub) {};

    virtual int ReleaseCall(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element) = 0;

    virtual int StartUser(int userId, sptr<IUserCallback> callback, bool isAppRecovery = false) = 0;

    virtual int StopUser(int userId, const sptr<IUserCallback> &callback) = 0;

    virtual int LogoutUser(int32_t userId, sptr<IUserCallback> callback = nullptr)
    {
        return 0;
    }

    virtual int SetMissionContinueState(const sptr<IRemoteObject> &token, const AAFwk::ContinueState &state)
    {
        return 0;
    };

#ifdef SUPPORT_SCREEN
    virtual int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) = 0;

    virtual int SetMissionIcon(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &icon) = 0;

    virtual void UpdateMissionSnapShot(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &pixelMap) {};

    virtual int RegisterWindowManagerServiceHandler(const sptr<IWindowManagerServiceHandler>& handler,
        bool animationEnabled)
    {
        return 0;
    }

    virtual void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) = 0;

    virtual void CompleteFirstFrameDrawing(int32_t sessionId)
    {}

    virtual int PrepareTerminateAbility(const sptr<IRemoteObject> &token, sptr<IPrepareTerminateCallback> &callback)
    {
        return 0;
    }

    virtual int GetDialogSessionInfo(const std::string &dialogSessionId, sptr<DialogSessionInfo> &dialogSessionInfo)
    {
        return 0;
    }

    virtual int SendDialogResult(const Want &want, const std::string &dialogSessionId, bool isAllow)
    {
        return 0;
    }

    virtual int32_t RegisterAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer,
        const std::string &targetBundleName)
    {
        return 0;
    }

    virtual int32_t UnregisterAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer)
    {
        return 0;
    }

#endif
    virtual int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) = 0;

    virtual int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) = 0;

    virtual int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) = 0;

    virtual int StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag) = 0;

    virtual int StopSyncRemoteMissions(const std::string &devId) = 0;

    virtual int RegisterMissionListener(const std::string &deviceId, const sptr<IRemoteMissionListener> &listener) = 0;

    virtual int RegisterOnListener(const std::string &type, const sptr<IRemoteOnListener> &listener)
    {
        return 0;
    }

    virtual int RegisterOffListener(const std::string &type, const sptr<IRemoteOnListener> &listener)
    {
        return 0;
    }

    virtual int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) = 0;

    virtual int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) = 0;

    virtual bool IsRunningInStabilityTest() = 0;

    virtual int RegisterSnapshotHandler(const sptr<ISnapshotHandler>& handler) = 0;

    virtual int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer) = 0;

    virtual int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName) = 0;

    virtual int GetTopAbility(sptr<IRemoteObject> &token) = 0;

    virtual int CheckUIExtensionIsFocused(uint32_t uiExtensionTokenId, bool& isFocused)
    {
        return 0;
    }

    virtual int DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token) = 0;

    virtual int DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token) = 0;

    virtual int DoAbilityForeground(const sptr<IRemoteObject> &token, uint32_t flag) = 0;

    virtual int DoAbilityBackground(const sptr<IRemoteObject> &token, uint32_t flag) = 0;

    virtual int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token) = 0;

    virtual void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token) = 0;

    virtual int DumpAbilityInfoDone(std::vector<std::string> &infos, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }

    virtual int FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
        int32_t userId, int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int AddFreeInstallObserver(const sptr<IRemoteObject> &callerToken,
        const sptr<AbilityRuntime::IFreeInstallObserver> &observer)
    {
        return 0;
    }

    virtual void EnableRecoverAbility(const sptr<IRemoteObject>& token) {};
    virtual void SubmitSaveRecoveryInfo(const sptr<IRemoteObject>& token) {};
    virtual void ScheduleRecoverAbility(const sptr<IRemoteObject> &token, int32_t reason,
        const Want *want = nullptr) {};

    virtual void ScheduleClearRecoveryPageStack() {};

    virtual int32_t IsValidMissionIds(
        const std::vector<int32_t> &missionIds, std::vector<MissionValidResult> &results) = 0;

    virtual int VerifyPermission(const std::string &permission, int pid, int uid)
    {
        return 0;
    }

    virtual int32_t RequestDialogService(const Want &want, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }

    virtual int32_t ReportDrawnCompleted(const sptr<IRemoteObject> &callerToken) = 0;

    virtual int32_t AcquireShareData(const int32_t &missionId, const sptr<IAcquireShareDataCallback> &shareData)
    {
        return 0;
    }

    virtual int32_t ShareDataDone(const sptr<IRemoteObject>& token,
        const int32_t &resultCode, const int32_t &uniqueId, WantParams &wantParam)
    {
        return 0;
    }

    virtual int32_t ForceExitApp(const int32_t pid, const ExitReason &exitReason)
    {
        return 0;
    }

    virtual int32_t RecordAppExitReason(const ExitReason &exitReason)
    {
        return 0;
    }

    virtual int32_t RecordProcessExitReason(const int32_t pid, const ExitReason &exitReason)
    {
        return 0;
    }

    virtual int32_t RecordProcessExitReason(int32_t pid, int32_t uid, const ExitReason &exitReason)
    {
        return 0;
    }

    virtual void SetRootSceneSession(const sptr<IRemoteObject> &rootSceneSession) {}

    virtual void CallUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isColdStart) {}

    virtual void StartSpecifiedAbilityBySCB(const Want &want) {};

    virtual int32_t NotifySaveAsResult(const Want &want, int resultCode, int requestCode)
    {
        return 0;
    }

    virtual int32_t SetSessionManagerService(const sptr<IRemoteObject> &sessionManagerService)
    {
        return 0;
    }

    virtual int32_t RegisterIAbilityManagerCollaborator(int32_t type, const sptr<IAbilityManagerCollaborator> &impl)
    {
        return 0;
    }

    virtual int32_t UnregisterIAbilityManagerCollaborator(int32_t type)
    {
        return 0;
    }

    virtual int32_t RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate)
    {
        return 0;
    }

    virtual int32_t KillProcessWithPrepareTerminate(const std::vector<int32_t>& pids)
    {
        return 0;
    }

    virtual int32_t KillProcessWithReason(int32_t pid, const ExitReason &reason)
    {
        return 0;
    }

    virtual int32_t RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
    {
        return 0;
    }

    virtual int32_t UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
    {
        return 0;
    }

    virtual int32_t SetApplicationAutoStartup(const AutoStartupInfo &info)
    {
        return 0;
    }

    virtual int32_t CancelApplicationAutoStartup(const AutoStartupInfo &info)
    {
        return 0;
    }

    virtual int32_t QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList)
    {
        return 0;
    }

    virtual int PrepareTerminateAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool &isPrepareTerminate)
    {
        return 0;
    }

    virtual int32_t RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) = 0;

    virtual int32_t UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) = 0;

    virtual int32_t AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal) = 0;

    virtual int32_t DetachAppDebug(const std::string &bundleName, bool isDebugFromLocal) = 0;

    virtual int32_t ExecuteIntent(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const InsightIntentExecuteParam &param) = 0;

    virtual bool IsAbilityControllerStart(const Want &want)
    {
        return true;
    }

    virtual int32_t ExecuteInsightIntentDone(const sptr<IRemoteObject> &token, uint64_t intentId,
        const InsightIntentExecuteResult &result) = 0;

    virtual int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) = 0;

    virtual int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) = 0;

    virtual int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list) = 0;

    virtual int32_t OpenFile(const Uri& uri, uint32_t flag)
    {
        return 0;
    }

    virtual int32_t UpdateSessionInfoBySCB(std::list<SessionInfo> &sessionInfos, int32_t userId,
        std::vector<int32_t> &sessionIds)
    {
        return 0;
    }

    virtual int32_t RestartApp(const AAFwk::Want &want, bool isAppRecovery = false)
    {
        return 0;
    }

    virtual int32_t GetUIExtensionRootHostInfo(const sptr<IRemoteObject> token, UIExtensionHostInfo &hostInfo,
        int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int32_t GetUIExtensionSessionInfo(const sptr<IRemoteObject> token,
        UIExtensionSessionInfo &uiExtensionSessionInfo, int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int32_t OpenLink(const Want& want, sptr<IRemoteObject> callerToken,
        int32_t userId = DEFAULT_INVAL_VALUE, int requestCode = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int32_t OpenAtomicService(Want& want, const StartOptions &options, sptr<IRemoteObject> callerToken,
        int32_t requestCode = DEFAULT_INVAL_VALUE, int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual int32_t SetResidentProcessEnabled(const std::string &bundleName, bool enable)
    {
        return 0;
    }

    virtual bool IsEmbeddedOpenAllowed(sptr<IRemoteObject> callerToken, const std::string &appId)
    {
        return true;
    }

    virtual int32_t RequestAssertFaultDialog(const sptr<IRemoteObject> &callback, const AAFwk::WantParams &wantParams)
    {
        return -1;
    }

    virtual int32_t NotifyDebugAssertResult(uint64_t assertFaultSessionId, AAFwk::UserStatus userStatus)
    {
        return -1;
    }

    virtual int32_t StartShortcut(const Want &want, const StartOptions &startOptions)
    {
        return 0;
    }

    virtual int32_t GetAbilityStateByPersistentId(int32_t persistentId, bool &state)
    {
        return 0;
    }

    virtual int32_t TransferAbilityResultForExtension(const sptr<IRemoteObject> &callerToken, int32_t resultCode,
        const Want &want)
    {
        return 0;
    }

    virtual void NotifyFrozenProcessByRSS(const std::vector<int32_t> &pidList, int32_t uid)
    {
        return;
    }

    virtual int32_t CleanUIAbilityBySCB(const sptr<SessionInfo> &sessionInfo, bool isUserRequestedExit,
        uint32_t sceneFlag = 0)
    {
        return 0;
    }

    virtual int32_t PreStartMission(const std::string& bundleName, const std::string& moduleName,
        const std::string& abilityName, const std::string& startTime)
    {
        return 0;
    }

    virtual int32_t TerminateMission(int32_t missionId)
    {
        return 0;
    }

    virtual int32_t BlockAllAppStart(bool flag)
    {
        return 0;
    }

    virtual int32_t UpdateAssociateConfigList(const std::map<std::string, std::list<std::string>>& configs,
        const std::list<std::string>& exportConfigs, int32_t flag)
    {
        return 0;
    }

    virtual int32_t SetApplicationKeepAlive(const std::string &bundleName, int32_t userId, bool flag)
    {
        return 0;
    }

    virtual int32_t QueryKeepAliveApplications(int32_t appType, int32_t userId, std::vector<KeepAliveInfo> &list)
    {
        return 0;
    }

    virtual int32_t SetApplicationKeepAliveByEDM(const std::string &bundleName, int32_t userId, bool flag)
    {
        return 0;
    }

    virtual int32_t QueryKeepAliveApplicationsByEDM(int32_t appType, int32_t userId, std::vector<KeepAliveInfo> &list)
    {
        return 0;
    }

    virtual int32_t GetAllIntentExemptionInfo(std::vector<AppExecFwk::IntentExemptionInfo> &info)
    {
        return 0;
    }

    virtual int32_t AddQueryERMSObserver(sptr<IRemoteObject> callerToken,
        sptr<AbilityRuntime::IQueryERMSObserver> observer)
    {
        return 0;
    }

    virtual int32_t QueryAtomicServiceStartupRule(sptr<IRemoteObject> callerToken,
        const std::string &appId, const std::string &startTime, AtomicServiceStartupRule &rule)
    {
        return 0;
    }

    virtual void PrepareTerminateAbilityDone(const sptr<IRemoteObject> &token, bool isTerminate)
    {}

    virtual void KillProcessWithPrepareTerminateDone(const std::string &moduleName,
        int32_t prepareTermination, bool isExist)
    {}

    virtual int32_t KillProcessForPermissionUpdate(uint32_t accessTokenId)
    {
        return 0;
    }

    virtual int32_t RegisterHiddenStartObserver(const sptr<IHiddenStartObserver> &observer)
    {
        return 0;
    }

    virtual int32_t UnregisterHiddenStartObserver(const sptr<IHiddenStartObserver> &observer)
    {
        return 0;
    }

    virtual int32_t QueryPreLoadUIExtensionRecord(const AppExecFwk::ElementName &element,
                                                  const std::string &moduleName,
                                                  const std::string &hostBundleName,
                                                  int32_t &recordNum,
                                                  int32_t userId = DEFAULT_INVAL_VALUE)
    {
        return 0;
    }

    virtual ErrCode RevokeDelegator(sptr<IRemoteObject> token)
    {
        return 0;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif