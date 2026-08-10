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

#include "abilitymanagerstubstart_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "ability_manager_stub.h"
#include "message_parcel.h"
#include "securec.h"
#include "session_info.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string ABILITYMGR_INTERFACE_TOKEN = u"ohos.aafwk.AbilityManager";
}

class AbilityManagerStubStartFuzz : public AbilityManagerStub {
public:
    AbilityManagerStubStartFuzz() = default;
    ~AbilityManagerStubStartFuzz() = default;

    int StartAbility(const Want &want, int32_t userId, int requestCode, uint64_t specifiedFullTokenId) override
    {
        return 0;
    }

    int StartAbility(const Want &want, const sptr<IRemoteObject> &callerToken, int32_t userId,
        int requestCode, uint64_t specifiedFullTokenId) override
    {
        return 0;
    }

    int StartAbilityWithSpecifyTokenId(const Want &want, const sptr<IRemoteObject> &callerToken,
        uint32_t specifyTokenId, int32_t userId, int requestCode) override
    {
        return 0;
    }

    int32_t StartAbilityByInsightIntent(const Want &want, const sptr<IRemoteObject> &callerToken,
        uint64_t intentId, int32_t userId) override
    {
        return 0;
    }

    int StartAbility(const Want &want, const AbilityStartSetting &abilityStartSetting,
        const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode) override
    {
        return 0;
    }

    int StartAbility(const Want &want, const StartOptions &startOptions,
        const sptr<IRemoteObject> &callerToken, int32_t userId, int requestCode) override
    {
        return 0;
    }

    int StartAbilityByCall(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t accountId, bool isSilent,
        bool promotePriority, bool isVisible, uint64_t specifiedFullTokenId) override
    {
        return 0;
    }

    int TerminateAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant) override
    {
        return 0;
    }

    int CloseAbility(const sptr<IRemoteObject> &token, int resultCode, const Want *resultWant) override
    {
        return 0;
    }

    int MinimizeAbility(const sptr<IRemoteObject> &token, bool fromUser) override
    {
        return 0;
    }

    int32_t ConnectAbility(const Want &want, const sptr<IAbilityConnection> &connect,
        const sptr<IRemoteObject> &callerToken, int32_t userId) override
    {
        return 0;
    }

    int DisconnectAbility(sptr<IAbilityConnection> connect) override
    {
        return 0;
    }

    sptr<IAbilityScheduler> AcquireDataAbility(const Uri &uri, bool tryBind,
        const sptr<IRemoteObject> &callerToken) override
    {
        return nullptr;
    }

    int ReleaseDataAbility(sptr<IAbilityScheduler> dataAbilityScheduler,
        const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }

    int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler,
        const sptr<IRemoteObject> &token) override
    {
        return 0;
    }

    int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state, const PacMap &saveData) override
    {
        return 0;
    }

    int ScheduleConnectAbilityDone(const sptr<IRemoteObject> &token,
        const sptr<IRemoteObject> &remoteObject) override
    {
        return 0;
    }

    int ScheduleDisconnectAbilityDone(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }

    int ScheduleCommandAbilityDone(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }

    int ScheduleCommandAbilityWindowDone(const sptr<IRemoteObject> &token,
        const sptr<SessionInfo> &sessionInfo, WindowCommand winCmd, AbilityCommand abilityCmd) override
    {
        return 0;
    }

    void DumpState(const std::string &args, std::vector<std::string> &state) override {}

    void DumpSysState(const std::string &args, std::vector<std::string> &info,
        bool isClient, bool isUserID, int UserID) override {}

    int StopServiceAbility(const Want &want, int32_t userId, const sptr<IRemoteObject> &token) override
    {
        return 0;
    }

    int KillProcess(const std::string &bundleName, bool clearPageStack, int32_t appIndex,
        const std::string &reason) override
    {
        return 0;
    }

    int32_t UninstallApp(const std::string &bundleName, int32_t uid, int32_t appIndex) override
    {
        return 0;
    }

    sptr<IWantSender> GetWantSender(const WantSenderInfo &wantSenderInfo,
        const sptr<IRemoteObject> &callerToken, int32_t uid) override
    {
        return nullptr;
    }

    int SendWantSender(sptr<IWantSender> target, SenderInfo &senderInfo) override
    {
        return 0;
    }

    void CancelWantSender(const sptr<IWantSender> &sender) override {}

    int GetPendingWantUid(const sptr<IWantSender> &target) override
    {
        return 0;
    }

    int GetPendingWantUserId(const sptr<IWantSender> &target) override
    {
        return 0;
    }

    std::string GetPendingWantBundleName(const sptr<IWantSender> &target) override
    {
        return "";
    }

    int GetPendingWantCode(const sptr<IWantSender> &target) override
    {
        return 0;
    }

    int GetPendingWantType(const sptr<IWantSender> &target) override
    {
        return 0;
    }

    void RegisterCancelListener(const sptr<IWantSender> &sender,
        const sptr<IWantReceiver> &receiver) override {}

    void UnregisterCancelListener(const sptr<IWantSender> &sender,
        const sptr<IWantReceiver> &receiver) override {}

    int GetPendingRequestWant(const sptr<IWantSender> &target, std::shared_ptr<Want> &want) override
    {
        return 0;
    }

    int GetWantSenderInfo(const sptr<IWantSender> &target, std::shared_ptr<WantSenderInfo> &info) override
    {
        return 0;
    }

    int ContinueMission(const std::string &srcDeviceId, const std::string &dstDeviceId,
        int32_t missionId, const sptr<IRemoteObject> &callBack, AAFwk::WantParams &wantParams) override
    {
        return 0;
    }

    int ContinueAbility(const std::string &deviceId, int32_t missionId, uint32_t versionCode) override
    {
        return 0;
    }

    int StartContinuation(const Want &want, const sptr<IRemoteObject> &abilityToken, int32_t status) override
    {
        return 0;
    }

    void NotifyCompleteContinuation(const std::string &deviceId, int32_t sessionId, bool isSuccess) override {}

    int NotifyContinuationResult(int32_t missionId, int32_t result) override
    {
        return 0;
    }

    int LockMissionForCleanup(int32_t missionId) override
    {
        return 0;
    }

    int UnlockMissionForCleanup(int32_t missionId) override
    {
        return 0;
    }

    int RegisterMissionListener(const sptr<IMissionListener> &listener) override
    {
        return 0;
    }

    int UnRegisterMissionListener(const sptr<IMissionListener> &listener) override
    {
        return 0;
    }

    int GetMissionInfos(const std::string &deviceId, int32_t numMax,
        std::vector<MissionInfo> &missionInfos) override
    {
        return 0;
    }

    int GetMissionInfo(const std::string &deviceId, int32_t missionId, MissionInfo &missionInfo) override
    {
        return 0;
    }

    int CleanMission(int32_t missionId) override
    {
        return 0;
    }

    int CleanAllMissions() override
    {
        return 0;
    }

    int MoveMissionToFront(int32_t missionId) override
    {
        return 0;
    }

    int MoveMissionToFront(int32_t missionId, const StartOptions &startOptions) override
    {
        return 0;
    }

    int MoveMissionsToForeground(const std::vector<int32_t> &missionIds, int32_t topMissionId) override
    {
        return 0;
    }

    int MoveMissionsToBackground(const std::vector<int32_t> &missionIds, std::vector<int32_t> &result) override
    {
        return 0;
    }

    int ReleaseCall(const sptr<IAbilityConnection> &connect, const AppExecFwk::ElementName &element) override
    {
        return 0;
    }

    int StartUser(int userId, uint64_t displayId, sptr<IUserCallback> callback, bool isAppRecovery) override
    {
        return 0;
    }

    int StopUser(int userId, const sptr<IUserCallback> &callback) override
    {
        return 0;
    }

    int GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info) override
    {
        return 0;
    }

    int GetExtensionRunningInfos(int upperLimit, std::vector<ExtensionRunningInfo> &info) override
    {
        return 0;
    }

    int GetProcessRunningInfos(std::vector<AppExecFwk::RunningProcessInfo> &info) override
    {
        return 0;
    }

    int StartSyncRemoteMissions(const std::string &devId, bool fixConflict, int64_t tag) override
    {
        return 0;
    }

    int StopSyncRemoteMissions(const std::string &devId) override
    {
        return 0;
    }

    int RegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override
    {
        return 0;
    }

    int UnRegisterMissionListener(const std::string &deviceId,
        const sptr<IRemoteMissionListener> &listener) override
    {
        return 0;
    }

    bool IsRunningInStabilityTest() override
    {
        return false;
    }

    int RegisterSnapshotHandler(const sptr<ISnapshotHandler> &handler) override
    {
        return 0;
    }

    int StartUserTest(const Want &want, const sptr<IRemoteObject> &observer) override
    {
        return 0;
    }

    int FinishUserTest(const std::string &msg, const int64_t &resultCode, const std::string &bundleName) override
    {
        return 0;
    }

    int GetTopAbility(sptr<IRemoteObject> &token) override
    {
        return 0;
    }

    int DelegatorDoAbilityForeground(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }

    int DelegatorDoAbilityBackground(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }

    int32_t GetMissionIdByToken(const sptr<IRemoteObject> &token) override
    {
        return 0;
    }

    void GetAbilityTokenByCalleeObj(const sptr<IRemoteObject> &callStub, sptr<IRemoteObject> &token) override {}

    int32_t IsValidMissionIds(const std::vector<int32_t> &missionIds,
        std::vector<MissionValidResult> &results) override
    {
        return 0;
    }

    int32_t ReportDrawnCompleted(const sptr<IRemoteObject> &callerToken) override
    {
        return 0;
    }

    int32_t RegisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) override
    {
        return 0;
    }

    int32_t UnregisterAppDebugListener(sptr<AppExecFwk::IAppDebugListener> listener) override
    {
        return 0;
    }

    int32_t AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal) override
    {
        return 0;
    }

    int32_t DetachAppDebug(const std::string &bundleName, bool isDebugFromLocal) override
    {
        return 0;
    }

    int32_t ExecuteIntent(uint64_t key, const sptr<IRemoteObject> &callerToken,
        const InsightIntentExecuteParam &param) override
    {
        return 0;
    }

    int32_t ExecuteInsightIntentDone(const sptr<IRemoteObject> &token, uint64_t intentId,
        const InsightIntentExecuteResult &result) override
    {
        return 0;
    }

    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag,
        bool isHiddenStart) override
    {
        return 0;
    }

    int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag) override
    {
        return 0;
    }

    int32_t GetForegroundUIAbilities(std::vector<AppExecFwk::AbilityStateData> &list) override
    {
        return 0;
    }

    int SetAbilityController(const sptr<AppExecFwk::IAbilityController> &abilityController,
        bool imAStabilityTest) override
    {
        return 0;
    }

    int32_t AcquireShareData(const int32_t &missionId,
        const sptr<IAcquireShareDataCallback> &shareData) override
    {
        return 0;
    }

    int32_t ShareDataDone(const sptr<IRemoteObject> &token, const int32_t &resultCode,
        const int32_t &uniqueId, WantParams &wantParam) override
    {
        return 0;
    }

    int GetMissionSnapshot(const std::string &deviceId, int32_t missionId,
        MissionSnapshot &snapshot, bool isLowResolution) override
    {
        return 0;
    }

#ifdef SUPPORT_SCREEN
    int SetMissionLabel(const sptr<IRemoteObject> &abilityToken, const std::string &label) override
    {
        return 0;
    }

    int SetMissionIcon(const sptr<IRemoteObject> &token,
        const std::shared_ptr<OHOS::Media::PixelMap> &icon) override
    {
        return 0;
    }

    void CompleteFirstFrameDrawing(const sptr<IRemoteObject> &abilityToken) override {}
#endif

#ifdef ABILITY_COMMAND_FOR_TEST
    int ForceTimeoutForTest(const std::string &abilityName, const std::string &state) override
    {
        return 0;
    }
#endif
};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    uint32_t code = FuzzUtil::GetU32Data(data);
    FuzzedDataProvider fdp(data + FuzzUtil::U32_AT_SIZE, size - FuzzUtil::U32_AT_SIZE);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY);

    parcel.WriteInterfaceToken(ABILITYMGR_INTERFACE_TOKEN);

    switch (code % 6) {
        case 0: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_UI_EXTENSION_ABILITY);
            bool hasSession = fdp.ConsumeBool();
            parcel.WriteBool(hasSession);
            if (hasSession) {
                SessionInfo sessionInfo;
                parcel.WriteParcelable(&sessionInfo);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_EXTENSION_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(fdp.ConsumeIntegral<int32_t>());
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_FOR_RESULT_AS_CALLER);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_CALL_ABILITY);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteRemoteObject(nullptr);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(AbilityManagerInterfaceCode::START_ABILITY_AS_CALLER_BY_TOKEN);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            bool hasCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasCaller);
            if (hasCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            bool hasAsCaller = fdp.ConsumeBool();
            parcel.WriteBool(hasAsCaller);
            if (hasAsCaller) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<AbilityManagerStub> stub = std::make_shared<AbilityManagerStubStartFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    if (size < OHOS::FuzzUtil::U32_AT_SIZE) {
        return 0;
    }

    char *ch = static_cast<char *>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}
