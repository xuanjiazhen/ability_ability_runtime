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

#include "app_scheduler.h"

namespace OHOS {
namespace AAFwk {
namespace {
// Non-zero result mimicking the real client with the appmgr service unreachable;
// the report core drops the attribution on any non-ERR_OK result.
constexpr int32_t STUB_SERVICE_UNREACHABLE = 1;
// ERR_OK: the report core only checks against zero.
constexpr int32_t STUB_ERR_OK = 0;
}  // namespace

AppScheduler::StubState &AppScheduler::Stub()
{
    static StubState state;
    return state;
}

void AppScheduler::ResetStub()
{
    Stub() = StubState();
}

int32_t AppScheduler::AttributeProcessForAbility(const std::string &taskId, int32_t callerUid, int32_t callerPid,
    int32_t targetPid, int64_t &attributionId)
{
    (void)callerUid;
    (void)callerPid;
    (void)targetPid;
    attributionId = 0;
    StubState &state = Stub();
    state.attributeCalls++;
    if (!state.attributeSucceeds) {
        return STUB_SERVICE_UNREACHABLE;
    }
    attributionId = state.nextAttributionId++;
    return STUB_ERR_OK;
}

void AppScheduler::AttachTimeOut(const sptr<IRemoteObject> &token)
{
    (void)token;
}

void AppScheduler::BlockProcessCacheByPids(const std::vector<int32_t> &pids)
{
    (void)pids;
}

bool AppScheduler::CleanAbilityByUserRequest(const sptr<IRemoteObject> &token)
{
    (void)token;
    return false;
}

AppAbilityState AppScheduler::ConvertToAppAbilityState(const int32_t state)
{
    (void)state;
    return AppAbilityState::ABILITY_STATE_UNDEFINED;
}

int AppScheduler::GetAbilityRecordsByProcessID(const int pid, std::vector<sptr<IRemoteObject>> &tokens)
{
    (void)pid;
    (void)tokens;
    return STUB_SERVICE_UNREACHABLE;
}

void AppScheduler::GetRunningProcessInfoByPid(const pid_t pid, OHOS::AppExecFwk::RunningProcessInfo &info) const
{
    (void)pid;
    (void)info;
}

void AppScheduler::GetRunningProcessInfoByToken(const sptr<IRemoteObject> &token,
    OHOS::AppExecFwk::RunningProcessInfo &info)
{
    (void)token;
    (void)info;
}

bool AppScheduler::IsCallerKilling(const std::string &callerKey) const
{
    (void)callerKey;
    return false;
}

int32_t AppScheduler::KillProcessesByPids(const std::vector<int32_t> &pids, const std::string &reason,
    bool subProcess, bool isKillPrecedeStart)
{
    (void)pids;
    (void)reason;
    (void)subProcess;
    (void)isKillPrecedeStart;
    return STUB_SERVICE_UNREACHABLE;
}

void AppScheduler::MoveToBackground(const sptr<IRemoteObject> &token, bool isFromScreenOffBackground)
{
    (void)token;
    (void)isFromScreenOffBackground;
}

void AppScheduler::MoveToForeground(const sptr<IRemoteObject> &token,
    const AppExecFwk::UiAbilityLastCallerInfo &callerInfo)
{
    (void)token;
    (void)callerInfo;
}

void AppScheduler::NotifyLoadAbilityFinished(pid_t callingPid, pid_t targetPid, uint64_t callbackId)
{
    (void)callingPid;
    (void)targetPid;
    (void)callbackId;
}

int32_t AppScheduler::NotifyPreloadAbilityStateChanged(sptr<IRemoteObject> token, bool isPreForeground)
{
    (void)token;
    (void)isPreForeground;
    return STUB_SERVICE_UNREACHABLE;
}

void AppScheduler::PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag)
{
    (void)token;
    (void)clearMissionFlag;
}

void AppScheduler::PrepareTerminateApp(const pid_t pid, const std::string &moduleName)
{
    (void)pid;
    (void)moduleName;
}

int32_t AppScheduler::SetGameSAPrelaunch(const sptr<IRemoteObject> &token, bool isGameSAPrelaunch)
{
    (void)token;
    (void)isGameSAPrelaunch;
    return STUB_SERVICE_UNREACHABLE;
}

void AppScheduler::SetSpecifiedProcessRequestId(int32_t recordId, int32_t requestId)
{
    (void)recordId;
    (void)requestId;
}

void AppScheduler::StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    const AbilityRuntime::StartSpecifiedParam &param)
{
    (void)want;
    (void)abilityInfo;
    (void)param;
}

void AppScheduler::StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    int32_t requestId, const std::string &customProcess)
{
    (void)want;
    (void)abilityInfo;
    (void)requestId;
    (void)customProcess;
}

AppScheduler::AppScheduler() {}

AppScheduler::~AppScheduler() {}
}  // namespace AAFwk
}  // namespace OHOS
