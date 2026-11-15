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

#include "abilitykeepalivedatamanagersecond_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <fuzzer/FuzzedDataProvider.h>

#define private public
#include "ability_keep_alive_data_manager.h"
#undef private

#include "distributed_kv_data_manager.h"
#include "keep_alive_info.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 64;
constexpr int32_t APP_TYPE_MIN = static_cast<int32_t>(KeepAliveAppType::UNSPECIFIED);
constexpr int32_t APP_TYPE_MAX = static_cast<int32_t>(KeepAliveAppType::SYSTEM);
constexpr int32_t SETTER_MIN = static_cast<int32_t>(KeepAliveSetter::UNSPECIFIED);
constexpr int32_t SETTER_MAX = static_cast<int32_t>(KeepAliveSetter::USER);
constexpr int32_t POLICY_MIN = static_cast<int32_t>(KeepAlivePolicy::UNSPECIFIED);
constexpr int32_t POLICY_MAX = static_cast<int32_t>(KeepAlivePolicy::ALLOW_CANCEL);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    auto &mgr = AbilityKeepAliveDataManager::GetInstance();

    // IsRecoverableStatus only compares a status enum and never touches the kv store.
    (void)mgr.IsRecoverableStatus(DistributedKv::Status::DATA_CORRUPTED);
    (void)mgr.IsRecoverableStatus(DistributedKv::Status::DB_CANT_OPEN);
    (void)mgr.IsRecoverableStatus(DistributedKv::Status::DB_ERROR);
    (void)mgr.IsRecoverableStatus(DistributedKv::Status::INVALID_QUERY_FORMAT);
    (void)mgr.IsRecoverableStatus(DistributedKv::Status::STORE_NOT_OPEN);
    (void)mgr.IsRecoverableStatus(DistributedKv::Status::SUCCESS);

    KeepAliveInfo info;
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    // bundleName is serialized into a JSON key by ConvertKeepAliveDataToKey; the raw
    // fuzz bytes may be invalid UTF-8, which makes nlohmann::json::dump() throw and
    // abort the process. Keep it 7-bit ASCII so the string is always valid UTF-8.
    for (auto &ch : info.bundleName) {
        ch = static_cast<char>(static_cast<unsigned char>(ch) & 0x7F);
    }
    info.userId = fdp.ConsumeIntegral<int32_t>();
    info.setterId = fdp.ConsumeIntegral<int32_t>();
    info.appType = static_cast<KeepAliveAppType>(fdp.ConsumeIntegralInRange<int32_t>(APP_TYPE_MIN, APP_TYPE_MAX));
    info.setter = static_cast<KeepAliveSetter>(fdp.ConsumeIntegralInRange<int32_t>(SETTER_MIN, SETTER_MAX));
    info.policy = static_cast<KeepAlivePolicy>(fdp.ConsumeIntegralInRange<int32_t>(POLICY_MIN, POLICY_MAX));

    // Convert helpers are pure (de)serialization and never reach the kv store backend,
    // so they are safe inside a fuzz process that has no distributed KV service.
    DistributedKv::Value value = mgr.ConvertKeepAliveStatusToValue(info);
    DistributedKv::Key key = mgr.ConvertKeepAliveDataToKey(info);

    KeepAliveStatus status;
    mgr.ConvertKeepAliveStatusFromValue(value, status);
    (void)mgr.ConvertKeepAliveInfoFromKey(key);
    (void)mgr.IsEqual(key, info);
    (void)mgr.IsEqualSetterId(key, info);

    // Feed malformed blobs so the JSON parse-defence branches are exercised too.
    std::string junk = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    DistributedKv::Value junkValue(junk);
    DistributedKv::Key junkKey(junk);
    KeepAliveStatus junkStatus;
    mgr.ConvertKeepAliveStatusFromValue(junkValue, junkStatus);
    (void)mgr.ConvertKeepAliveInfoFromKey(junkKey);
    (void)mgr.IsEqual(junkKey, info);
    (void)mgr.IsEqualSetterId(junkKey, info);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}