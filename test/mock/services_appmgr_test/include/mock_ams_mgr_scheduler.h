/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

 #ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_AMS_MGR_SCHEDULER_H
 #define MOCK_OHOS_ABILITY_RUNTIME_MOCK_AMS_MGR_SCHEDULER_H
 
 #include "gmock/gmock.h"
 #include "ams_mgr_scheduler.h"
 
 namespace OHOS {
 namespace AbilityRuntime {
 struct LoadParam;
 }
 namespace AppExecFwk {
 class MockAmsMgrScheduler : public AmsMgrStub {
 public:
     MOCK_METHOD4(LoadAbility,
         ErrCode(const std::shared_ptr<AbilityInfo>& abilityInfo, const std::shared_ptr<ApplicationInfo>& appInfo,
             const std::shared_ptr<AAFwk::Want>& want, const std::shared_ptr<AbilityRuntime::LoadParam>& loadParam));
     MOCK_METHOD2(TerminateAbility, ErrCode(const sptr<IRemoteObject>& token, bool clearMissionFlag));
     MOCK_METHOD2(UpdateAbilityState, ErrCode(const sptr<IRemoteObject> &token, int32_t state));
     MOCK_METHOD0(Reset, void());
     MOCK_METHOD1(KillProcessByAbilityToken, ErrCode(const sptr<IRemoteObject>& token));
     MOCK_METHOD3(KillProcessesByUserId, ErrCode(int32_t userId, bool isNeedSendAppSpawnMsg,
         const sptr<AAFwk::IUserCallback>& callback));
     MOCK_METHOD4(KillProcessWithAccount, ErrCode(const std::string&, const int, const bool clearPageStack, int32_t));
     MOCK_METHOD1(KillProcessesInBatch, ErrCode(const std::vector<int32_t> &pids));
     MOCK_METHOD4(UpdateApplicationInfoInstalled, int(const std::string&, const int uid, const std::string&, bool));
     MOCK_METHOD3(ForceKillApplication, ErrCode(const std::string& appName, const int userId, const int appIndex));
     MOCK_METHOD3(KillApplication, ErrCode(const std::string& bundleName, const bool clearPageStack, int32_t appIndex));
     MOCK_METHOD1(KillProcessesByAccessTokenId, ErrCode(const uint32_t accessTokenId));
     MOCK_METHOD3(KillApplicationByUid, ErrCode(const std::string&, const int uid, const std::string&));
     MOCK_METHOD0(IsReady, bool());
     MOCK_METHOD1(AbilityAttachTimeOut, ErrCode(const sptr<IRemoteObject>& token));
     MOCK_METHOD2(PrepareTerminate, ErrCode(const sptr<IRemoteObject>& token, bool clearMissionFlag));
     MOCK_METHOD2(PrepareTerminateApp, ErrCode(const pid_t, const std::string &moduleName));
     MOCK_METHOD2(GetRunningProcessInfoByToken,
         ErrCode(const sptr<IRemoteObject>& token, OHOS::AppExecFwk::RunningProcessInfo& info));
     MOCK_METHOD1(SetAbilityForegroundingFlagToAppRecord, ErrCode(const pid_t pid));
     MOCK_METHOD3(StartSpecifiedAbility, ErrCode(const AAFwk::Want&, const AppExecFwk::AbilityInfo&, int32_t));
     MOCK_METHOD3(StartSpecifiedProcess, ErrCode(const AAFwk::Want&, const AppExecFwk::AbilityInfo&, int32_t));
     MOCK_METHOD1(RegisterStartSpecifiedAbilityResponse, ErrCode(const sptr<IStartSpecifiedAbilityResponse>& response));
     MOCK_METHOD3(GetApplicationInfoByProcessID, ErrCode(const int pid, AppExecFwk::ApplicationInfo& application,
         bool& debug));
     MOCK_METHOD3(NotifyAppMgrRecordExitReason, ErrCode(int32_t pid, int32_t reason, const std::string &exitMsg));
     MOCK_METHOD3(GetBundleNameByPid, ErrCode(const int pid, std::string &bundleName, int32_t &uid));
     MOCK_METHOD1(RegisterAppDebugListener, ErrCode(const sptr<IAppDebugListener> &listener));
     MOCK_METHOD1(UnregisterAppDebugListener, ErrCode(const sptr<IAppDebugListener> &listener));
     MOCK_METHOD2(AttachAppDebug, ErrCode(const std::string &bundleName, bool isDebugFromLocal));
     MOCK_METHOD1(DetachAppDebug, ErrCode(const std::string &bundleName));
     MOCK_METHOD1(RegisterAbilityDebugResponse, ErrCode(const sptr<IAbilityDebugResponse> &response));
     MOCK_METHOD2(IsAttachDebug, ErrCode(const std::string &bundleName, bool&));
     MOCK_METHOD2(SetAppWaitingDebug, ErrCode(const std::string &bundleName, bool isPersist));
     MOCK_METHOD0(CancelAppWaitingDebug, ErrCode());
     MOCK_METHOD1(GetWaitingDebugApp, ErrCode(std::vector<std::string> &debugInfoList));
     MOCK_METHOD2(IsWaitingDebugApp, ErrCode(const std::string &bundleName, bool&));
     MOCK_METHOD0(ClearNonPersistWaitingDebugFlag, ErrCode());
     MOCK_METHOD1(IsMemorySizeSufficent, ErrCode(bool& funcResult));
     MOCK_METHOD3(KillProcessesByPids, ErrCode(const std::vector<int32_t>&, const std::string&, bool));
     MOCK_METHOD2(UpdateExtensionState, ErrCode(const sptr<IRemoteObject>&, ExtensionState));
     MOCK_METHOD2(AttachPidToParent, ErrCode(const sptr<IRemoteObject>&, const sptr<IRemoteObject>&));
     MOCK_METHOD2(KillApplicationSelf, ErrCode(bool clearPageStack, const std::string&));
     MOCK_METHOD2(SetEnableStartProcessFlagByUserId, ErrCode(int32_t, bool));
     MOCK_METHOD3(SetKeepAliveEnableState, ErrCode(const std::string&, bool, int32_t));
     MOCK_METHOD3(SetKeepAliveDkv, ErrCode(const std::string &, bool, int32_t));
     MOCK_METHOD1(ClearProcessByToken, ErrCode(const sptr<IRemoteObject>&));
     MOCK_METHOD1(IsNoRequireBigMemory, ErrCode(bool&));
     MOCK_METHOD1(AttachedToStatusBar, ErrCode(const sptr<IRemoteObject>&));
     MOCK_METHOD1(BlockProcessCacheByPids, ErrCode(const std::vector<int32_t> &pids));
     MOCK_METHOD2(IsKilledForUpgradeWeb, ErrCode(const std::string&, bool&));
     MOCK_METHOD2(CleanAbilityByUserRequest, ErrCode(const sptr<IRemoteObject>&, bool&));
     MOCK_METHOD2(IsProcessContainsOnlyUIAbility,  ErrCode(int32_t, bool&));
     MOCK_METHOD2(IsProcessAttached, ErrCode(const sptr<IRemoteObject>&, bool&));
     MOCK_METHOD2(IsCallerKilling, ErrCode(const std::string&, bool&));
 
     MockAmsMgrScheduler() : AmsMgrStub() {};
     virtual ~MockAmsMgrScheduler() {};
     virtual ErrCode RegisterAppStateCallback(const sptr<IAppStateCallback>& callback) override
     {
         callback->OnAbilityRequestDone(nullptr, AbilityState::ABILITY_STATE_BACKGROUND);
         AppProcessData appProcessData;
         callback->OnAppStateChanged(appProcessData);
         return ERR_OK;
     }
 
     MOCK_METHOD1(SetCurrentUserId, ErrCode(const int32_t userId));
 
     MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel&, MessageParcel&, MessageOption&));
 
     int InvokeSendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
     {
         code_ = code;
 
         return 0;
     }
 
     int code_ = 0;
 };
 }  // namespace AppExecFwk
 }  // namespace OHOS
 
 #endif
 