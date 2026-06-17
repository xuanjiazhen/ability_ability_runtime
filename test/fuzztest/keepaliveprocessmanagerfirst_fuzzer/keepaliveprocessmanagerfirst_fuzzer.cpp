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

#include "keepaliveprocessmanagerfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>
#include "keep_alive_process_manager.h"
#include "ability_fuzz_util.h"
#include "ability_keep_alive_service.h"
#include "ability_util.h"
#include "app_mgr_client.h"
#include "parameters.h"
#include "permission_verification.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr int32_t MAX_VALID_USER_ID = 100;
constexpr int32_t TEST_UID = 1000;
constexpr int32_t TEST_USER_ID = 1;
constexpr int32_t TEST_APP_TYPE = 1;
constexpr int32_t TEST_PID_VALUE = 12345;
constexpr const char* DEFAULT_BUNDLE_NAME = "com.example.app";
}

void TestSetApplicationKeepAlive(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    if (bundleName.empty()) {
        bundleName = DEFAULT_BUNDLE_NAME;
    }
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    bool updateEnable = fdp.ConsumeBool();
    bool isByEDM = fdp.ConsumeBool();
    bool isInner = fdp.ConsumeBool();
    mgr.SetApplicationKeepAlive(bundleName, userId, updateEnable, isByEDM, isInner);
    mgr.SetApplicationKeepAlive("", userId, updateEnable, isByEDM, isInner);
}

void TestSetAppServiceExtensionKeepAlive(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    if (bundleName.empty()) {
        bundleName = DEFAULT_BUNDLE_NAME;
    }
    bool updateEnable = fdp.ConsumeBool();
    bool isByEDM = fdp.ConsumeBool();
    bool isAllowUserToCancel = fdp.ConsumeBool();
    mgr.SetAppServiceExtensionKeepAlive(bundleName, updateEnable, isByEDM, isAllowUserToCancel);
    mgr.SetAppServiceExtensionKeepAlive("", updateEnable, isByEDM, isAllowUserToCancel);
}

void TestQueryKeepAliveApplications(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    int32_t appType = fdp.ConsumeIntegral<int32_t>();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = fdp.ConsumeBool();
    mgr.QueryKeepAliveApplications(appType, userId, infoList, isByEDM);
}

void TestQueryKeepAliveAppServiceExtensions(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    std::vector<KeepAliveInfo> infoList;
    bool isByEDM = fdp.ConsumeBool();
    mgr.QueryKeepAliveAppServiceExtensions(infoList, isByEDM);
}

void TestStartKeepAliveProcessWithMainElement(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    std::vector<BundleInfo> bundleInfos;
    size_t count = fdp.ConsumeIntegralInRange<size_t>(0, 5);
    for (size_t i = 0; i < count; ++i) {
        BundleInfo info;
        AbilityFuzzUtil::GetRandomBundleInfo(fdp, info);
        bundleInfos.push_back(info);
    }
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    mgr.StartKeepAliveProcessWithMainElement(bundleInfos, userId);
}

void TestStartKeepAliveAppServiceExtension(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    std::vector<BundleInfo> bundleInfos;
    size_t count = fdp.ConsumeIntegralInRange<size_t>(0, 5);
    for (size_t i = 0; i < count; ++i) {
        BundleInfo info;
        AbilityFuzzUtil::GetRandomBundleInfo(fdp, info);
        bundleInfos.push_back(info);
    }
    mgr.StartKeepAliveAppServiceExtension(bundleInfos);
}

void TestAfterStartKeepAliveApp(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    uint32_t token = fdp.ConsumeIntegral<uint32_t>();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    bool isMulti = fdp.ConsumeBool();
    mgr.AfterStartKeepAliveApp(bundleName, token, uid, userId, isMulti);
}

void TestOnAppStateChanged(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    AppInfo info;
    info.state = fdp.ConsumeEnum<AppState>();
    info.pid = fdp.ConsumeIntegral<int32_t>();
    mgr.OnAppStateChanged(info);
    info.state = AppState::BEGIN;
    info.pid = -1;
    mgr.OnAppStateChanged(info);
    info.pid = TEST_PID_VALUE;
    mgr.OnAppStateChanged(info);
}

void TestSaveRestartAfterUpgrade(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    mgr.SaveAppSeriviceRestartAfterUpgrade(bundleName, uid);
    mgr.SaveKeepAliveAppRestartAfterUpgrade(bundleName, uid);
    mgr.CheckNeedRestartAfterUpgrade(uid);
    mgr.KeepAliveCheckNeedRestartAfterUpgrade(uid);
    mgr.KeepAliveIsRestartAfterUpdate(uid);
    mgr.AddNeedRestartKeepAliveUid(uid);
    std::vector<BundleInfo> bundleInfos;
    mgr.FilterNeedRestartKeepAliveBundleInfos(bundleInfos);
}

void TestRemoveCheckStatusBarTask(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    bool cancel = fdp.ConsumeBool();
    mgr.RemoveCheckStatusBarTask(uid, cancel);
}

void TestIsKeepAliveBundle(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    mgr.IsKeepAliveBundle(bundleName, userId);
}

void TestGetKeepAliveBundleInfosForUser(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    std::vector<BundleInfo> bundleInfos;
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    mgr.GetKeepAliveBundleInfosForUser(bundleInfos, userId);
}

void TestStartKeepAliveMainAbility(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    KeepAliveAbilityInfo info;
    AbilityFuzzUtil::GetRandomKeepAliveAbilityInfo(fdp, info);
    mgr.StartKeepAliveMainAbility(info);
}

void TestStartKeepAliveAppServiceExtensionInner(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    KeepAliveAbilityInfo info;
    AbilityFuzzUtil::GetRandomKeepAliveAbilityInfo(fdp, info);
    mgr.StartKeepAliveAppServiceExtensionInner(info);
}

void TestClearKeepAliveAppServiceExtension(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    mgr.ClearKeepAliveAppServiceExtension(userId);
}

void TestStartKeepAliveAfterAppUpgrade(FuzzedDataProvider& fdp)
{
    auto& mgr = KeepAliveProcessManager::GetInstance();
    std::vector<BundleInfo> bundleInfos;
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    mgr.StartKeepAliveAfterAppUpgrade(bundleInfos, uid);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    BundleInfo info;
    int32_t userId;
    FuzzedDataProvider fdp(data, size);
    size_t arraySize = fdp.ConsumeIntegralInRange<size_t>(0, STRING_MAX_LENGTH);
    for (size_t i = 0; i < arraySize; ++i) {
        AbilityFuzzUtil::GetRandomBundleInfo(fdp, info);
        bundleInfos.emplace_back(info);
    }
    userId = fdp.ConsumeIntegral<int32_t>();
    KeepAliveProcessManager::GetInstance().StartKeepAliveProcessWithMainElement(bundleInfos, userId);
    return true;
}

bool DoSomethingInterestingWithMyAPISecond(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    TestSetApplicationKeepAlive(fdp);
    TestSetAppServiceExtensionKeepAlive(fdp);
    TestQueryKeepAliveApplications(fdp);
    TestQueryKeepAliveAppServiceExtensions(fdp);
    TestStartKeepAliveProcessWithMainElement(fdp);
    TestStartKeepAliveAppServiceExtension(fdp);
    TestAfterStartKeepAliveApp(fdp);
    TestOnAppStateChanged(fdp);
    TestSaveRestartAfterUpgrade(fdp);
    TestRemoveCheckStatusBarTask(fdp);
    TestIsKeepAliveBundle(fdp);
    TestGetKeepAliveBundleInfosForUser(fdp);
    TestStartKeepAliveMainAbility(fdp);
    TestStartKeepAliveAppServiceExtensionInner(fdp);
    TestClearKeepAliveAppServiceExtension(fdp);
    TestStartKeepAliveAfterAppUpgrade(fdp);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    OHOS::DoSomethingInterestingWithMyAPISecond(data, size);
    return 0;
}
