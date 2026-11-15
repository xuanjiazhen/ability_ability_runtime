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

#include "abilityresidentprocessrdbsecond_fuzzer.h"

#include <charconv>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <unordered_map>
#include "hilog_tag_wrapper.h"
#include "parser_util.h"

#define private public
#include "ability_resident_process_rdb.h"
#include "rdb_data_manager.h"
#undef private

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string callerName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t callerUid = fdp.ConsumeIntegral<int32_t>();
    bool enable = fdp.ConsumeBool();

    AmsResidentProcessRdb::GetInstance().Init();
    AmsResidentProcessRdb &rdb = AmsResidentProcessRdb::GetInstance();

    rdb.VerifyConfigurationPermissions(bundleName, callerName);
    rdb.GetResidentProcessEnable(bundleName, enable);
    rdb.VerifySaConfigurationPermissions(bundleName, callerUid);
    rdb.GetResidentProcessRawData(bundleName, callerName);
    rdb.GetSaResidentProcessRawData(bundleName, callerUid);

    ResidentBundleCapability capability;
    capability.bundleName = bundleName;
    capability.keepAliveEnable = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    capability.keepAliveConfiguredList = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    capability.keepAliveSaUidList = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::unordered_map<std::string, ResidentBundleCapability> jsonMap;
    jsonMap.emplace(bundleName, capability);
    std::unordered_map<std::string, ResidentBundleCapability> dbMap;
    dbMap.emplace(callerName, capability);

    int32_t syncResult = 0;
    rdb.SyncResidentProcessData();
    rdb.BuildUpdateMap(jsonMap);
    rdb.BuildLocalMap(dbMap);
    rdb.RemoveStaleBundles(dbMap, jsonMap, syncResult);
    rdb.UpsertBundles(jsonMap, dbMap, syncResult);

    std::string storedFingerprint = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string curFingerprint = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    rdb.IsOtaUpgrade(storedFingerprint, curFingerprint);
    std::string fingerprint;
    rdb.GetOtaFingerprint(fingerprint);
    rdb.SetOtaFingerprint(fingerprint);

    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
