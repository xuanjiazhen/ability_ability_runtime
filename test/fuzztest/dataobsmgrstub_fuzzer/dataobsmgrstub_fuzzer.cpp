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

#include "dataobsmgrstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "dataobs_mgr_stub.h"
#include "dataobs_mgr_errors.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AAFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string DATAOBS_MGR_INTERFACE_TOKEN = u"ohos.aafwk.DataObsMgr";
}

class DataObsMgrStubFuzz : public DataObsManagerStub {
public:
    DataObsMgrStubFuzz() = default;
    virtual ~DataObsMgrStubFuzz() = default;
    int RegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver, int32_t userId,
        DataObsOption opt) override
    {
        return SUCCESS;
    }
    int RegisterObserverFromExtension(const Uri &uri, sptr<IDataAbilityObserver> dataObserver, int32_t userId,
        DataObsOption opt) override
    {
        return SUCCESS;
    }
    int UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver, int32_t userId,
        DataObsOption opt) override
    {
        return SUCCESS;
    }
    int NotifyChange(const Uri &uri, int32_t userId, DataObsOption opt) override
    {
        return SUCCESS;
    }
    int NotifyChangeFromExtension(const Uri &uri, int32_t userId, DataObsOption opt) override
    {
        return SUCCESS;
    }
    Status RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver, bool isDescendants,
        DataObsOption opt) override
    {
        return SUCCESS;
    }
    Status UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
        DataObsOption opt) override
    {
        return SUCCESS;
    }
    Status UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver, DataObsOption opt) override
    {
        return SUCCESS;
    }
    Status NotifyChangeExt(const ChangeInfo &changeInfo, DataObsOption opt) override
    {
        return SUCCESS;
    }
    Status NotifyProcessObserver(const std::string &key, const sptr<IRemoteObject> observer) override
    {
        return SUCCESS;
    }
};

void WriteMaliciousDataObsOption(MessageParcel &parcel, FuzzedDataProvider &fdp)
{
    parcel.WriteBool(fdp.ConsumeBool());
    parcel.WriteUint32(fdp.ConsumeIntegral<uint32_t>());
    parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
    parcel.WriteBool(fdp.ConsumeBool());
    parcel.WriteUint64(fdp.ConsumeIntegral<uint64_t>());
}

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IDataObsMgr::REGISTER_OBSERVER);

    parcel.WriteInterfaceToken(DATAOBS_MGR_INTERFACE_TOKEN);

    switch (code % 5) {
        case 0:
            actualCode = static_cast<uint32_t>(IDataObsMgr::REGISTER_OBSERVER);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            WriteMaliciousDataObsOption(parcel, fdp);
            break;
        case 1:
            actualCode = static_cast<uint32_t>(IDataObsMgr::REGISTER_OBSERVER_FROM_EXTENSION);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            WriteMaliciousDataObsOption(parcel, fdp);
            break;
        case 2:
            actualCode = static_cast<uint32_t>(IDataObsMgr::UNREGISTER_OBSERVER);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteRemoteObject(nullptr);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        case 3:
            actualCode = static_cast<uint32_t>(IDataObsMgr::NOTIFY_CHANGE);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteBool(fdp.ConsumeBool());
            break;
        case 4:
            actualCode = static_cast<uint32_t>(IDataObsMgr::NOTIFY_CHANGE_FROM_EXTENSION);
            parcel.WriteString(FuzzUtil::BuildUriAttackString(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            WriteMaliciousDataObsOption(parcel, fdp);
            break;
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<DataObsManagerStub> stub = std::make_shared<DataObsMgrStubFuzz>();
    stub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
} // namespace OHOS

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
