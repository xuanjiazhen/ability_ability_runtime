/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "abilitykeepalivedatamanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>

#define private public
#include "ability_keep_alive_data_manager.h"
#undef private

#include "ability_fuzz_util.h"
#include "distributed_kv_data_manager.h"
#include "keep_alive_info.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr int32_t TEST_USER_ID = 1;
constexpr int32_t MAX_VALID_USER_ID = 100;
constexpr int32_t MISMATCH_SETTER_ID = 999;
constexpr const char* DEFAULT_BUNDLE_NAME = "com.example.app";
constexpr const char* DIFFERENT_BUNDLE_NAME = "different";
}

void GenerateRandomKeepAliveInfo(FuzzedDataProvider& fdp, KeepAliveInfo& info, bool valid)
{
    if (valid) {
        info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
        if (info.bundleName.empty()) {
            info.bundleName = DEFAULT_BUNDLE_NAME;
        }
        info.userId = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALID_USER_ID);
        int32_t appTypeVal = fdp.ConsumeIntegralInRange<int32_t>(0, 2);
        info.appType = static_cast<KeepAliveAppType>(appTypeVal);
        if (info.appType == KeepAliveAppType::UNSPECIFIED) {
            info.appType = KeepAliveAppType::SYSTEM;
        }
        int32_t setterVal = fdp.ConsumeIntegralInRange<int32_t>(0, 2);
        info.setter = static_cast<KeepAliveSetter>(setterVal);
        if (info.setter == KeepAliveSetter::UNSPECIFIED) {
            info.setter = KeepAliveSetter::USER;
        }
        info.setterId = fdp.ConsumeIntegral<int32_t>();
        int32_t policyVal = fdp.ConsumeIntegralInRange<int32_t>(0, 2);
        info.policy = static_cast<KeepAlivePolicy>(policyVal);
    } else {
        info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
        info.userId = fdp.ConsumeIntegral<int32_t>();
        info.appType = static_cast<KeepAliveAppType>(fdp.ConsumeIntegral<int32_t>());
        info.setter = static_cast<KeepAliveSetter>(fdp.ConsumeIntegral<int32_t>());
        info.setterId = fdp.ConsumeIntegral<int32_t>();
        info.policy = static_cast<KeepAlivePolicy>(fdp.ConsumeIntegral<int32_t>());
    }
}

class KvStoreGuard {
public:
    explicit KvStoreGuard(AbilityKeepAliveDataManager& mgr, bool corrupt = false)
        : mgr_(mgr), originalPtr_(nullptr), locked_(false)
    {
        mgr_.kvStorePtrMutex_.lock();
        locked_ = true;
        originalPtr_ = mgr_.kvStorePtr_;
        if (corrupt) {
            mgr_.kvStorePtr_ = nullptr;
        }
    }

    ~KvStoreGuard()
    {
        if (locked_) {
            mgr_.kvStorePtr_ = originalPtr_;
            mgr_.kvStorePtrMutex_.unlock();
        }
    }

private:
    AbilityKeepAliveDataManager& mgr_;
    std::shared_ptr<DistributedKv::SingleKvStore> originalPtr_;
    bool locked_;
};

void TestRestoreAndCheck(FuzzedDataProvider& fdp, AbilityKeepAliveDataManager& mgr)
{
    std::vector<DistributedKv::Status> statusList = {
        DistributedKv::Status::DATA_CORRUPTED,
        DistributedKv::Status::SUCCESS,
        DistributedKv::Status::ERROR,
        DistributedKv::Status::NOT_FOUND
    };
    for (auto st : statusList) {
        mgr.RestoreKvStore(st);
    }
    DistributedKv::Status getStatus = mgr.GetKvStore();
    (void)getStatus;
    {
        KvStoreGuard guard(mgr, true);
        bool ret = mgr.CheckKvStore();
        (void)ret;
    }
    mgr.GetKvStore();
}

void TestInsertDelete(FuzzedDataProvider& fdp, AbilityKeepAliveDataManager& mgr)
{
    KeepAliveInfo invalidInfo;
    invalidInfo.bundleName = "";
    invalidInfo.userId = fdp.ConsumeIntegral<int32_t>();
    invalidInfo.appType = KeepAliveAppType::SYSTEM;
    invalidInfo.setter = KeepAliveSetter::SYSTEM;
    mgr.InsertKeepAliveData(invalidInfo);
    invalidInfo.bundleName = "bundle";
    invalidInfo.userId = -1;
    mgr.InsertKeepAliveData(invalidInfo);
    invalidInfo.userId = 1;
    invalidInfo.appType = KeepAliveAppType::UNSPECIFIED;
    mgr.InsertKeepAliveData(invalidInfo);
    invalidInfo.appType = KeepAliveAppType::SYSTEM;
    invalidInfo.setter = KeepAliveSetter::UNSPECIFIED;
    mgr.InsertKeepAliveData(invalidInfo);
    KeepAliveInfo validInfo;
    GenerateRandomKeepAliveInfo(fdp, validInfo, true);
    {
        KvStoreGuard guard(mgr, true);
        mgr.InsertKeepAliveData(validInfo);
    }
    mgr.InsertKeepAliveData(validInfo);
    KeepAliveInfo delInfo;
    delInfo.userId = -1;
    mgr.DeleteKeepAliveData(delInfo);
    GenerateRandomKeepAliveInfo(fdp, delInfo, true);
    mgr.DeleteKeepAliveData(delInfo);
    {
        KvStoreGuard guard(mgr, true);
        mgr.DeleteKeepAliveData(delInfo);
    }
}

void TestQuery(FuzzedDataProvider& fdp, AbilityKeepAliveDataManager& mgr)
{
    KeepAliveInfo queryInfo;
    queryInfo.bundleName = "";
    queryInfo.userId = fdp.ConsumeIntegral<int32_t>();
    mgr.QueryKeepAliveData(queryInfo);
    queryInfo.bundleName = "bundle";
    queryInfo.userId = -1;
    mgr.QueryKeepAliveData(queryInfo);
    GenerateRandomKeepAliveInfo(fdp, queryInfo, true);
    mgr.QueryKeepAliveData(queryInfo);
    {
        KvStoreGuard guard(mgr, true);
        mgr.QueryKeepAliveData(queryInfo);
    }
    KeepAliveInfo appQueryInfo;
    appQueryInfo.userId = -1;
    std::vector<KeepAliveInfo> infoList;
    mgr.QueryKeepAliveApplications(appQueryInfo, infoList);
    GenerateRandomKeepAliveInfo(fdp, appQueryInfo, true);
    mgr.QueryKeepAliveApplications(appQueryInfo, infoList);
    {
        KvStoreGuard guard(mgr, true);
        mgr.QueryKeepAliveApplications(appQueryInfo, infoList);
    }
}

void TestDeleteWithSetter(FuzzedDataProvider& fdp, AbilityKeepAliveDataManager& mgr)
{
    KeepAliveInfo setterInfo;
    GenerateRandomKeepAliveInfo(fdp, setterInfo, true);
    mgr.DeleteKeepAliveDataWithSetterId(setterInfo);
    setterInfo.userId = TEST_USER_ID;
    setterInfo.setterId = fdp.ConsumeIntegral<int32_t>();
    mgr.DeleteKeepAliveDataWithSetterId(setterInfo);
    {
        KvStoreGuard guard(mgr, true);
        mgr.DeleteKeepAliveDataWithSetterId(setterInfo);
    }
}

void TestConvertAndCompare(FuzzedDataProvider& fdp, AbilityKeepAliveDataManager& mgr)
{
    DistributedKv::Value badValue(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    KeepAliveStatus aliveStatus;
    mgr.ConvertKeepAliveStatusFromValue(badValue, aliveStatus);
    DistributedKv::Key badKey(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    mgr.ConvertKeepAliveInfoFromKey(badKey);
    KeepAliveInfo infoForConvert;
    GenerateRandomKeepAliveInfo(fdp, infoForConvert, true);
    DistributedKv::Key key = mgr.ConvertKeepAliveDataToKey(infoForConvert);
    mgr.ConvertKeepAliveInfoFromKey(key);
    DistributedKv::Value value = mgr.ConvertKeepAliveStatusToValue(infoForConvert);
    mgr.ConvertKeepAliveStatusFromValue(value, aliveStatus);
    DistributedKv::Key randomKey(fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    KeepAliveInfo compareInfo;
    GenerateRandomKeepAliveInfo(fdp, compareInfo, false);
    mgr.IsEqual(randomKey, compareInfo);
    mgr.IsEqualSetterId(randomKey, compareInfo);
    KeepAliveInfo sameInfo;
    GenerateRandomKeepAliveInfo(fdp, sameInfo, true);
    DistributedKv::Key sameKey = mgr.ConvertKeepAliveDataToKey(sameInfo);
    mgr.IsEqual(sameKey, sameInfo);
    sameInfo.bundleName = DIFFERENT_BUNDLE_NAME;
    mgr.IsEqual(sameKey, sameInfo);
    mgr.IsEqualSetterId(sameKey, sameInfo);
    sameInfo.setterId = MISMATCH_SETTER_ID;
    mgr.IsEqualSetterId(sameKey, sameInfo);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    DistributedKv::Status status;
    KeepAliveStatus aliveStatus;
    KeepAliveInfo info;
    std::vector<KeepAliveInfo> infoList;
    FuzzedDataProvider fdp(data, size);
    size_t arraySize = fdp.ConsumeIntegralInRange<size_t>(0, STRING_MAX_LENGTH);
    for (size_t i = 0; i < arraySize; ++i) {
        AbilityFuzzUtil::GetRandomKeepAliveInfo(fdp, info);
        infoList.emplace_back(info);
    }
    status = DistributedKv::Status::DATA_CORRUPTED;
    AbilityKeepAliveDataManager::GetInstance().RestoreKvStore(status);
    status = DistributedKv::Status::SUCCESS;
    AbilityKeepAliveDataManager::GetInstance().RestoreKvStore(status);
    auto kvStore = AbilityKeepAliveDataManager::GetInstance().GetKvStore();
    if (!kvStore) {
        return false;
    }
    AbilityKeepAliveDataManager::GetInstance().CheckKvStore();
    info.appType = KeepAliveAppType::UNSPECIFIED;
    AbilityKeepAliveDataManager::GetInstance().InsertKeepAliveData(info);
    AbilityKeepAliveDataManager::GetInstance().QueryKeepAliveData(info);
    AbilityKeepAliveDataManager::GetInstance().QueryKeepAliveApplications(info, infoList);
    AbilityKeepAliveDataManager::GetInstance().DeleteKeepAliveData(info);
    AbilityKeepAliveDataManager::GetInstance().DeleteKeepAliveDataWithSetterId(info);
    AbilityKeepAliveDataManager::GetInstance().ConvertKeepAliveStatusToValue(info);
    AbilityKeepAliveDataManager::GetInstance().ConvertKeepAliveDataToKey(info);
    DistributedKv::Value value;
    AbilityFuzzUtil::GetRandomKeepAliveStatus(fdp, aliveStatus);
    AbilityKeepAliveDataManager::GetInstance().ConvertKeepAliveStatusFromValue(value, aliveStatus);
    DistributedKv::Key key;
    AbilityKeepAliveDataManager::GetInstance().ConvertKeepAliveInfoFromKey(key);
    AbilityKeepAliveDataManager::GetInstance().IsEqualSetterId(key, info);
    AbilityKeepAliveDataManager::GetInstance().IsEqual(key, info);
    return true;
}

bool DoSomethingInterestingWithMyAPISecond(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto& mgr = AbilityKeepAliveDataManager::GetInstance();
    TestRestoreAndCheck(fdp, mgr);
    TestInsertDelete(fdp, mgr);
    TestQuery(fdp, mgr);
    TestDeleteWithSetter(fdp, mgr);
    TestConvertAndCompare(fdp, mgr);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    OHOS::DoSomethingInterestingWithMyAPISecond(data, size);
    return 0;
}
