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
#include "abilitykeepaliveservice_fuzzer.h"

#define private public
#include "ability_keep_alive_service.h"
#include "ability_fuzz_util.h"
#undef private

#include "keep_alive_info.h"
#include "background_app_info.h"
#include "ability_keep_alive_data_manager.h"

#include <fuzzer/FuzzedDataProvider.h>
#include <iostream>
#include "securec.h"

namespace OHOS {
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;
namespace {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr int32_t MAX_VALID_USER_ID = 100;
constexpr int32_t TEST_SETTER_ID_USER = 100;
constexpr int32_t TEST_SETTER_ID_SYSTEM = 200;
constexpr int32_t TEST_SETTER_ID_SERVICE_EXT = 300;
constexpr int32_t MISMATCH_SETTER_ID = 999;
constexpr const char* DEFAULT_BUNDLE_NAME = "com.example.app";
constexpr const char* TEST_BUNDLE_A = "bundle.a";
constexpr const char* TEST_BUNDLE_B = "bundle.b";
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

void GenerateRandomKeepAliveInfo(FuzzedDataProvider& fdp, KeepAliveInfo& info, bool valid)
{
    if (valid) {
        info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
        if (info.bundleName.empty()) {
            info.bundleName = DEFAULT_BUNDLE_NAME;
        }
        info.userId = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALID_USER_ID);
        info.appType = fdp.ConsumeEnum<KeepAliveAppType>();
        if (info.appType == KeepAliveAppType::UNSPECIFIED) {
            info.appType = KeepAliveAppType::KEEP_ALIVE;
        }
        info.setter = fdp.ConsumeEnum<KeepAliveSetter>();
        if (info.setter == KeepAliveSetter::UNSPECIFIED) {
            info.setter = KeepAliveSetter::SELF;
        }
        info.setterId = fdp.ConsumeIntegral<int32_t>();
        info.policy = fdp.ConsumeEnum<KeepAlivePolicy>();
    } else {
        info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
        info.userId = fdp.ConsumeIntegral<int32_t>();
        info.appType = static_cast<KeepAliveAppType>(fdp.ConsumeIntegral<int32_t>());
        info.setter = static_cast<KeepAliveSetter>(fdp.ConsumeIntegral<int32_t>());
        info.setterId = fdp.ConsumeIntegral<int32_t>();
        info.policy = static_cast<KeepAlivePolicy>(fdp.ConsumeIntegral<int32_t>());
    }
}

void TestBaseApis(FuzzedDataProvider& fdp, AbilityKeepAliveService& service)
{
    KeepAliveInfo info;
    GenerateRandomKeepAliveInfo(fdp, info, true);
    std::vector<KeepAliveInfo> infoList;
    int32_t userId = info.userId;
    int32_t appType = static_cast<int32_t>(info.appType);
    service.QueryKeepAliveApplications(userId, appType, infoList);
    service.GetValidUserId(userId);
    service.IsKeepAliveApp(info.bundleName, info.userId);
    service.GetKeepAliveApplications(info.userId, infoList);
    service.QueryKeepAliveAppServiceExtensions(infoList);
    service.ClearKeepAliveAppServiceExtension(info);
    service.SetApplicationKeepAlive(info, false);
    service.SetApplicationKeepAlive(info, true);
    service.SetAppServiceExtensionKeepAlive(info, false);
    service.SetAppServiceExtensionKeepAlive(info, true);
}

void TestQueryAndIsKeepAlive(FuzzedDataProvider& fdp, AbilityKeepAliveService& service)
{
    std::vector<KeepAliveInfo> infoList;
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    int32_t appType = fdp.ConsumeIntegral<int32_t>();
    service.QueryKeepAliveApplications(userId, appType, infoList);
    service.GetKeepAliveApplications(userId, infoList);
    std::string emptyBundle = "";
    service.IsKeepAliveApp(emptyBundle, userId);
    std::string bundle = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    service.IsKeepAliveApp(bundle, userId);
    service.QueryKeepAliveAppServiceExtensions(infoList);
}

void TestSetKeepAliveTrueScenarios(AbilityKeepAliveService& service)
{
    KeepAliveInfo info;
    info.bundleName = TEST_BUNDLE_A;
    info.userId = 1;
    info.appType = KeepAliveAppType::KEEP_ALIVE;
    info.setter = KeepAliveSetter::USER;
    info.setterId = TEST_SETTER_ID_USER;
    auto& dm = AbilityKeepAliveDataManager::GetInstance();
    dm.DeleteKeepAliveData(info);
    int32_t ret = service.SetKeepAliveTrue(info);
    (void)ret;
    ret = service.SetKeepAliveTrue(info);
    KeepAliveInfo higherSetter = info;
    higherSetter.setter = KeepAliveSetter::SYSTEM;
    ret = service.SetKeepAliveTrue(higherSetter);
}

void TestCancelKeepAliveScenarios(AbilityKeepAliveService& service)
{
    KeepAliveInfo info;
    info.bundleName = TEST_BUNDLE_B;
    info.userId = 1;
    info.appType = KeepAliveAppType::KEEP_ALIVE;
    info.setter = KeepAliveSetter::SYSTEM;
    info.setterId = TEST_SETTER_ID_SYSTEM;
    auto& dm = AbilityKeepAliveDataManager::GetInstance();
    dm.DeleteKeepAliveData(info);
    int32_t ret = service.CancelKeepAlive(info);
    ret = service.SetKeepAliveTrue(info);
    ret = service.CancelKeepAlive(info);
    KeepAliveInfo lowerSetter = info;
    lowerSetter.setter = KeepAliveSetter::USER;
    ret = service.CancelKeepAlive(lowerSetter);
}

void TestSetAppServiceExtension(AbilityKeepAliveService& service)
{
    KeepAliveInfo info;
    info.bundleName = TEST_BUNDLE_A;
    info.userId = 1;
    info.appType = KeepAliveAppType::SERVICE_EXTENSION;
    info.setter = KeepAliveSetter::SYSTEM;
    info.setterId = TEST_SETTER_ID_SERVICE_EXT;
    info.policy = KeepAlivePolicy::NOT_ALLOW_CANCEL;
    auto& dm = AbilityKeepAliveDataManager::GetInstance();
    dm.DeleteKeepAliveData(info);
    service.SetAppServiceExtensionKeepAlive(info, true);
    service.SetAppServiceExtensionKeepAlive(info, false);
    KeepAliveInfo sameSetterId = info;
    sameSetterId.setterId = TEST_SETTER_ID_SERVICE_EXT;
    service.SetAppServiceExtensionKeepAlive(sameSetterId, false);
    KeepAliveInfo diffSetter = info;
    diffSetter.setter = KeepAliveSetter::USER;
    service.SetAppServiceExtensionKeepAlive(diffSetter, true);
    service.SetAppServiceExtensionKeepAlive(diffSetter, false);
    KeepAliveInfo userSetter = info;
    userSetter.setter = KeepAliveSetter::USER;
    userSetter.policy = KeepAlivePolicy::ALLOW_CANCEL;
    service.SetAppServiceExtensionKeepAlive(userSetter, true);
    service.SetAppServiceExtensionKeepAlive(userSetter, false);
}

void TestStorageFailure(AbilityKeepAliveService& service)
{
    auto& dm = AbilityKeepAliveDataManager::GetInstance();
    KeepAliveInfo info;
    info.bundleName = TEST_BUNDLE_A;
    info.userId = 1;
    {
        KvStoreGuard guard(dm, true);
        service.SetKeepAliveTrue(info);
        service.CancelKeepAlive(info);
        service.QueryKeepAliveApplications(1, 1, std::vector<KeepAliveInfo>());
        service.IsKeepAliveApp(info.bundleName, info.userId);
        service.GetKeepAliveApplications(1, std::vector<KeepAliveInfo>());
        service.SetAppServiceExtensionKeepAlive(info, true);
        service.SetAppServiceExtensionKeepAlive(info, false);
        service.QueryKeepAliveAppServiceExtensions(std::vector<KeepAliveInfo>());
        service.ClearKeepAliveAppServiceExtension(info);
    }
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    auto &abilityKeepAliveService = AbilityKeepAliveService::GetInstance();
    std::vector<KeepAliveInfo> infoList;
    FuzzedDataProvider fdp(data, size);

    KeepAliveInfo info;
    info.userId = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    info.setterId = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    info.appType = static_cast<AAFwk::KeepAliveAppType>(fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE));
    info.setter = static_cast<AAFwk::KeepAliveSetter>(fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE));
    info.policy = static_cast<AAFwk::KeepAlivePolicy>(fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE));
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);

    size_t arraySize = fdp.ConsumeIntegralInRange<size_t>(0, STRING_MAX_LENGTH);
    for (size_t i = 0; i < arraySize; ++i) {
        AppExecFwk::AbilityFuzzUtil::GetRandomKeepAliveInfo(fdp, info);
        infoList.emplace_back(info);
    }

    int32_t int32Param = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    bool boolParam = fdp.ConsumeBool();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t in32appType = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);

    abilityKeepAliveService.QueryKeepAliveApplications(int32Param, in32appType, infoList);
    abilityKeepAliveService.CancelKeepAlive(info);
    abilityKeepAliveService.GetValidUserId(int32Param);
    abilityKeepAliveService.IsKeepAliveApp(bundleName, int32Param);
    abilityKeepAliveService.GetKeepAliveApplications(int32Param, infoList);
    abilityKeepAliveService.QueryKeepAliveAppServiceExtensions(infoList);
    abilityKeepAliveService.ClearKeepAliveAppServiceExtension(info);

    Parcel parcel;
    BackgroundAppInfo backgroundAppInfo;
    backgroundAppInfo.bandleName = fdp.ConsumeRandomLengthString();
    backgroundAppInfo.appIndex = fdp.ConsumeIntegral<int32_t>();
    backgroundAppInfo.Marshalling(parcel);
    return true;
}

bool DoSomethingInterestingWithMyAPISecond(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto& service = AbilityKeepAliveService::GetInstance();
    auto& dm = AbilityKeepAliveDataManager::GetInstance();
    {
        KvStoreGuard guard(dm, false);
        TestBaseApis(fdp, service);
        TestQueryAndIsKeepAlive(fdp, service);
        TestSetKeepAliveTrueScenarios(service);
        TestCancelKeepAliveScenarios(service);
        TestSetAppServiceExtension(service);
    }
    TestStorageFailure(service);
    Parcel parcel;
    BackgroundAppInfo backgroundAppInfo;
    backgroundAppInfo.bandleName = fdp.ConsumeRandomLengthString();
    backgroundAppInfo.appIndex = fdp.ConsumeIntegral<int32_t>();
    backgroundAppInfo.Marshalling(parcel);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    OHOS::DoSomethingInterestingWithMyAPISecond(data, size);
    return 0;
}
