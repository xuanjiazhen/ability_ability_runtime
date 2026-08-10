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

#include "amsmgrstubkill_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ams_mgr_stub.h"
#include "parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string AMSMGR_INTERFACE_TOKEN = u"ohos.appexecfwk.IAmsMgr";
constexpr uint32_t KILL_PIDS_MAX_COUNT = 50;
}

class AmsMgrStubKillFuzz : public AmsMgrStub {
public:
    AmsMgrStubKillFuzz() = default;
    ~AmsMgrStubKillFuzz() = default;

    void TerminateAbility(const sptr<IRemoteObject> &token, bool clearMissionFlag) override {}
    void UpdateAbilityState(const sptr<IRemoteObject> &token, const AbilityState state,
        bool isFromScreenOffBackground = false) override {}
    void RegisterAppStateCallback(const sptr<IAppStateCallback> &callback) override {}
    void KillProcessByAbilityToken(const sptr<IRemoteObject> &token) override {}
    int32_t SetGameSAPrelaunch(const sptr<IRemoteObject> &token, bool isGameSAPrelaunch) override { return 0; }
    void KillProcessesByUserId(int32_t userId, bool isNeedSendAppSpawnMsg = false,
        sptr<AAFwk::IUserCallback> callback = nullptr) override {}
    int32_t KillProcessesByPids(const std::vector<int32_t> &pids,
        const std::string &reason = "KillProcessesByPids", bool subProcess = false,
        bool isKillPrecedeStart = false) override { return 0; }
    int KillProcessWithAccount(const std::string &bundleName, const int accountId,
        const bool clearPageStack = false, int32_t appIndex = 0) override { return 0; }
    int32_t KillProcessesInBatch(const std::vector<int32_t> &pids) override { return 0; }
    int UpdateApplicationInfoInstalled(const std::string &bundleName, const int uid,
        const std::string &moduleName, bool isPlugin) override { return 0; }
    int KillApplication(const std::string &bundleName, bool clearPageStack = false, int32_t appIndex = 0,
        const std::string &reason = "KillApplication") override { return 0; }
    int ForceKillApplication(const std::string &bundleName, const int userId = -1, const int appIndex = 0) override
    {
        return 0;
    }
    int KillApplicationWithUserId(const std::string &bundleName,
        const int userId = -1, const int appIndex = 0) override { return 0; }
    int KillProcessesByAccessTokenId(const uint32_t accessTokenId) override { return 0; }
    int KillApplicationByUid(const std::string &bundleName, const int uid,
        const std::string& reason = "KillApplicationByUid") override { return 0; }
    void AbilityAttachTimeOut(const sptr<IRemoteObject> &token) override {}
    void PrepareTerminate(const sptr<IRemoteObject> &token, bool clearMissionFlag = false) override {}
    void GetRunningProcessInfoByToken(
        const sptr<IRemoteObject> &token, OHOS::AppExecFwk::RunningProcessInfo &info) override {}
    void SetAbilityForegroundingFlagToAppRecord(const pid_t pid) override {}
    void StartSpecifiedAbility(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0, const std::string &customProcess = "", bool isWindowStagePreload = false) override {}
    void RegisterStartSpecifiedAbilityResponse(const sptr<IStartSpecifiedAbilityResponse> &response) override {}
    void PrepareTerminateApp(const pid_t pid, const std::string &moduleName) override {}
    void StartSpecifiedProcess(const AAFwk::Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
        int32_t requestId = 0, const std::string &customProcess = "") override {}
    int GetApplicationInfoByProcessID(const int pid, AppExecFwk::ApplicationInfo &application, bool &debug) override
    {
        return 0;
    }
    int32_t NotifyAppMgrRecordExitReason(int32_t pid, int32_t reason, const std::string &exitMsg) override
    {
        return 0;
    }
    int32_t NotifyAppMgrRecordExitReasonCompability(
        int32_t pid, int32_t killId, const std::string &killMsg, const std::string &innerMsg) override { return 0; }
    int32_t GetBundleNameByPid(const int pid, std::string &bundleName, int32_t &uid) override { return 0; }
    int32_t RegisterAppDebugListener(const sptr<IAppDebugListener> &listener) override { return 0; }
    int32_t UnregisterAppDebugListener(const sptr<IAppDebugListener> &listener) override { return 0; }
    int32_t AttachAppDebug(const std::string &bundleName, bool isDebugFromLocal) override { return 0; }
    int32_t DetachAppDebug(const std::string &bundleName) override { return 0; }
    int32_t SetAppWaitingDebug(const std::string &bundleName, bool isPersist) override { return 0; }
    int32_t CancelAppWaitingDebug() override { return 0; }
    int32_t GetWaitingDebugApp(std::vector<std::string> &debugInfoList) override { return 0; }
    bool IsWaitingDebugApp(const std::string &bundleName) override { return false; }
    void ClearNonPersistWaitingDebugFlag() override {}
    int32_t RegisterAbilityDebugResponse(const sptr<IAbilityDebugResponse> &response) override { return 0; }
    bool IsAttachDebug(const std::string &bundleName) override { return false; }
    bool IsMemorySizeSufficient() override { return false; }
};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return false;
    }
    uint32_t code = FuzzUtil::GetU32Data(data);
    FuzzedDataProvider fdp(data, size);
    MessageParcel parcel;
    parcel.WriteInterfaceToken(AMSMGR_INTERFACE_TOKEN);
    uint32_t actualCode = 0;
    switch (code % 10) {
        case 0: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::TERMINATE_ABILITY);
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_PEOCESS_BY_ABILITY_TOKEN);
            parcel.WriteRemoteObject(nullptr);
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_USERID);
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 100));
            bool isWithCallback = fdp.ConsumeBool();
            parcel.WriteBool(isWithCallback);
            if (isWithCallback) {
                parcel.WriteRemoteObject(nullptr);
            }
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_BY_PIDS);
            uint32_t pidsCount = fdp.ConsumeIntegral<uint8_t>() % KILL_PIDS_MAX_COUNT + 1;
            parcel.WriteUint32(pidsCount);
            std::vector<int32_t> pids = FuzzUtil::BuildMaliciousInt32Vector(fdp);
            for (uint32_t i = 0; i < pidsCount; i++) {
                if (i < pids.size()) {
                    parcel.WriteInt32(pids[i]);
                } else {
                    parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
                }
            }
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESS_WITH_ACCOUNT);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 5: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_PROCESSES_IN_BATCH);
            uint32_t pidsCount = fdp.ConsumeIntegral<uint8_t>() % KILL_PIDS_MAX_COUNT + 1;
            parcel.WriteUint32(pidsCount);
            for (uint32_t i = 0; i < pidsCount; i++) {
                parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            }
            break;
        }
        case 6: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::BuildSpecialCharString(fdp));
            break;
        }
        case 7: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::FORCE_KILL_APPLICATION);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 100));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 8: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::KILL_APPLICATION_WITH_USER_ID);
            parcel.WriteString(FuzzUtil::BuildMaliciousBundleName(fdp));
            parcel.WriteInt32(FuzzUtil::BuildInvalidEnum(fdp, 100));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 9: {
            actualCode = static_cast<uint32_t>(IAmsMgr::Message::FORCE_KILL_APPLICATION_BY_ACCESS_TOKEN_ID);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        default:
            return false;
    }
    parcel.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    std::shared_ptr<AmsMgrStub> stub = std::make_shared<AmsMgrStubKillFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
} // namespace OHOS

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
