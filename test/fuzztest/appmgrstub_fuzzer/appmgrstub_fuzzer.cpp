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

#include "appmgrstub_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "app_mgr_stub.h"
#include "app_mgr_stub_mock.h"
#include "bundle_info.h"
#include "configuration.h"
#include "message_parcel.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string APPMGR_INTERFACE_TOKEN = u"ohos.appexecfwk.AppMgr";
}

class AppMgrStubFuzz : public AppMgrStubFuzzBase {};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    FUZZ_EXTRACT_CODE(data, size);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_TERMINATED);

    parcel.WriteInterfaceToken(APPMGR_INTERFACE_TOKEN);

    switch (code % 10) {
        case 0:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_APPLICATION_TERMINATED);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 1:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_ABILITY_CLEANED);
            parcel.WriteRemoteObject(nullptr);
            break;
        case 2:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_CLEAR_UP_APPLICATION_DATA);
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 256));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            OHOS::FuzzUtil::WriteUntrustedCallerData(parcel, fdp);
            break;
        case 3: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::STARTUP_RESIDENT_PROCESS);
            int32_t infoSize = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteInt32(infoSize);
            for (int32_t i = 0; i < infoSize; i++) {
                AppExecFwk::BundleInfo bundleInfo;
                parcel.WriteParcelable(&bundleInfo);
            }
            break;
        }
        case 4: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::UPDATE_CONFIGURATION);
            OHOS::FuzzUtil::WriteMaliciousConfiguration(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 5:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_ATTACH_APPLICATION);
            parcel.WriteRemoteObject(nullptr);
            break;
        case 6:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_ADD_ABILITY_STAGE_INFO_DONE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 7:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::APP_NOTIFY_MEMORY_LEVEL);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        case 8:
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::SCHEDULE_ACCEPT_WANT_DONE);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteString(FuzzUtil::BuildSqlInjectionString(fdp));
            break;
        case 9: {
            actualCode = static_cast<uint32_t>(AppMgrInterfaceCode::MAKE_IMAGE);
            FuzzUtil::WriteMaliciousWant(parcel, fdp);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            bool flag = fdp.ConsumeBool();
            parcel.WriteBool(flag);
            if (flag) {
                parcel.WriteRemoteObject(nullptr);
            }
            OHOS::FuzzUtil::WriteMaliciousFaultData(parcel, fdp);
            std::vector<uint8_t> corruptData = OHOS::FuzzUtil::BuildNonIpcMemoryCorruption(fdp, 256);
            parcel.WriteString(std::string(corruptData.begin(), corruptData.end()));
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            parcel.WriteString(FuzzUtil::GenStrcpyOverflowString(fdp, 128));
            break;
        }
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<AppMgrStub> appMgrStub = std::make_shared<AppMgrStubFuzz>();
    appMgrStub->OnRemoteRequest(actualCode, parcel, reply, option);
    return true;
}
} // namespace OHOS

FUZZ_ENTRY_IMPL(OHOS::DoSomethingInterestingWithMyAPI)
