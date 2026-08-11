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

#include "appstatecallbackhost_fuzzer.h"
#include "fuzz_util.h"
#include "attack_vectors.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#include "app_state_callback_host.h"
#include "app_process_data.h"
#include "bundle_info.h"
#include "configuration.h"
#include "iapp_state_callback.h"
#include "message_parcel.h"
#include "securec.h"

using namespace OHOS::AppExecFwk;
using namespace OHOS;

namespace OHOS {
namespace {
const std::u16string APPSTATECALLBACK_INTERFACE_TOKEN = u"ohos.appexecfwk.AppStateCallback";
}

class AppStateCallbackHostFuzz : public AppStateCallbackHost {};

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    uint32_t code = FuzzUtil::GetU32Data(data);
    FuzzedDataProvider fdp(data + FuzzUtil::U32_AT_SIZE, size - FuzzUtil::U32_AT_SIZE);
    MessageParcel parcel;
    MessageParcel reply;
    MessageOption option;
    uint32_t actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_STATE_CHANGED);

    parcel.WriteInterfaceToken(APPSTATECALLBACK_INTERFACE_TOKEN);

    switch (code % 5) {
        case 0: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_APP_STATE_CHANGED);
            AppProcessData processData;
            parcel.WriteParcelable(&processData);
            break;
        }
        case 1: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_ABILITY_REQUEST_DONE);
            bool flag = fdp.ConsumeBool();
            parcel.WriteBool(flag);
            if (flag) {
                parcel.WriteRemoteObject(nullptr);
            }
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 2: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_CONFIG_CHANGE);
            Configuration config;
            parcel.WriteParcelable(&config);
            parcel.WriteInt32(FuzzUtil::BuildIntegerOverflow(fdp));
            break;
        }
        case 3: {
            actualCode = static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_START_RESIDENT_PROCESS);
            int32_t infoSize = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteInt32(infoSize);
            for (int32_t i = 0; i < infoSize; i++) {
                BundleInfo bundleInfo;
                parcel.WriteParcelable(&bundleInfo);
            }
            break;
        }
        case 4: {
            actualCode =
                static_cast<uint32_t>(IAppStateCallback::Message::TRANSACT_ON_NOTIFY_START_KEEP_ALIVE_PROCESS);
            int32_t infoSize = static_cast<int32_t>(fdp.ConsumeIntegral<uint8_t>() % 4);
            parcel.WriteInt32(infoSize);
            for (int32_t i = 0; i < infoSize; i++) {
                BundleInfo bundleInfo;
                parcel.WriteParcelable(&bundleInfo);
            }
            break;
        }
        default:
            break;
    }

    parcel.RewindRead(0);
    std::shared_ptr<AppStateCallbackHost> appStateCallbackHost = std::make_shared<AppStateCallbackHostFuzz>();
    appStateCallbackHost->OnRemoteRequest(actualCode, parcel, reply, option);
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
