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

#include "multiuserconfigmgrfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

#define private public
#include "multi_user_config_mgr.h"
#undef private

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
} // namespace

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto multiUserConfigurationMgr = std::make_shared<AppExecFwk::MultiUserConfigurationMgr>();
    if (multiUserConfigurationMgr == nullptr) {
        return false;
    }
    std::shared_ptr<AppExecFwk::Configuration> configurationPtr = std::make_shared<AppExecFwk::Configuration>();
    if (configurationPtr == nullptr) {
        return false;
    }
    configurationPtr->AddItem(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH),
        fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));

    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::vector<std::string> changeKeyV;
    bool isNotifyUser0 = false;

    multiUserConfigurationMgr->GetConfigurationByUserId(userId);
    multiUserConfigurationMgr->InitConfiguration(configurationPtr);
    multiUserConfigurationMgr->HandleConfiguration(userId, *configurationPtr, changeKeyV, isNotifyUser0);
    multiUserConfigurationMgr->UpdateMultiUserConfiguration(*configurationPtr);
    multiUserConfigurationMgr->UpdateMultiUserConfigurationForGlobal(*configurationPtr);
    multiUserConfigurationMgr->SetOrUpdateConfigByUserId(userId, *configurationPtr, changeKeyV);
    multiUserConfigurationMgr->GetForegroundOsAccountLocalId();
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
