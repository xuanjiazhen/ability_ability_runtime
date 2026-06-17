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

#include "abilityappfreezemanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <set>
#include <vector>
#include <iostream>
#include <fstream>

#define private public
#include "appfreeze_manager.h"
#undef private

#include "fault_data.h"

using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t INPUT_ZERO = 0;
constexpr size_t INPUT_ONE = 1;
constexpr size_t INPUT_TWO = 2;
constexpr size_t INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr int32_t MIN_PID = 0;
constexpr int32_t MAX_PID = 10000;
constexpr int32_t TEST_USER_ID = 1;
constexpr int32_t INVALID_KILL_ID = -2;
constexpr const char* DEFAULT_BUNDLE = "com.example.app";
constexpr const char* TEST_EVENT_NAME = "THREAD_BLOCK_3S";
constexpr const char* TEST_EVENT_INPUT = "APP_INPUT_BLOCK";
constexpr const char* TEST_EVENT_LIFECYCLE_HALF = "LIFECYCLE_HALF_TIMEOUT";
constexpr const char* TEST_EVENT_LIFECYCLE_TIMEOUT = "LIFECYCLE_TIMEOUT";
}

uint32_t GetU32Data(const char* ptr)
{
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) | (ptr[INPUT_TWO] << OFFSET_TWO) |
        ptr[INPUT_THREE];
}

void TestAppfreezeHandle(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    FaultData faultData;
    AppfreezeManager::AppInfo appInfo;
    faultData.errorObject.name = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.errorObject.message = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.pid = fdp.ConsumeIntegral<int32_t>();
    faultData.tid = fdp.ConsumeIntegral<int32_t>();
    faultData.isInForeground = fdp.ConsumeBool();
    appInfo.pid = faultData.pid;
    appInfo.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    appInfo.uid = fdp.ConsumeIntegral<int32_t>();
    freeze->AppfreezeHandle(faultData, appInfo);
    freeze->AppfreezeHandleWithStack(faultData, appInfo);
}

void TestLifecycleTimeoutHandle(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    AppfreezeManager::ParamInfo info;
    info.typeId = fdp.ConsumeIntegral<int>();
    info.pid = fdp.ConsumeIntegral<int32_t>();
    info.eventName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    info.msg = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->LifecycleTimeoutHandle(info);
}

void TestIsHandleAppfreeze(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    std::string bundle = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->IsHandleAppfreeze(bundle);
    freeze->IsHandleAppfreeze("");
}

void TestProcessDebug(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    std::string bundle = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->IsProcessDebug(pid, bundle);
    freeze->CancelAppFreezeDetect(pid, bundle);
    freeze->ResetAppfreezeState(pid, bundle);
    freeze->IsValidFreezeFilter(pid, bundle);
    freeze->RemoveDeathProcess(bundle);
}

void TestStackOperations(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    std::string stack = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->FindStackByPid(stack, pid);
    freeze->CatcherStacktrace(pid);
    std::string faultType = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->CatchJsonStacktrace(pid, faultType);
    freeze->DeleteStack(pid);
}

void TestBinderPeerPids(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    std::string stack;
    AppfreezeManager::ParseBinderParam params;
    params.pid = fdp.ConsumeIntegral<int32_t>();
    params.eventPid = fdp.ConsumeIntegral<int32_t>();
    params.eventTid = fdp.ConsumeIntegral<int32_t>();
    std::set<int> asyncPids;
    AppfreezeManager::TerminalBinder terminalBinder;
    freeze->GetBinderPeerPids(stack, params, asyncPids, terminalBinder);
}

void TestAcquireAndNotify(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    FaultData faultData;
    AppfreezeManager::AppInfo appInfo;
    faultData.errorObject.name = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.errorObject.message = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.pid = fdp.ConsumeIntegral<int32_t>();
    faultData.tid = fdp.ConsumeIntegral<int32_t>();
    faultData.isInForeground = fdp.ConsumeBool();
    appInfo.pid = faultData.pid;
    appInfo.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    appInfo.uid = fdp.ConsumeIntegral<int32_t>();
    std::string memoryContent = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->AcquireStack(faultData, appInfo, memoryContent);
    std::string binderInfo = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->NotifyANR(faultData, appInfo, binderInfo, memoryContent);
}

void TestFreezeState(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    std::string key = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    int state = fdp.ConsumeIntegral<int>();
    std::string eventName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->SetFreezeState(key, state, eventName);
    freeze->GetReportTimes(key);
    freeze->GetLastOccurTime(key);
    freeze->IsNeedIgnoreFreezeEvent(key, eventName);
    freeze->CheckAppfreezeHappend(key, eventName);
}

void TestCpuAndPerf(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    FaultData faultData;
    AppfreezeManager::AppInfo appInfo;
    faultData.errorObject.name = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    appInfo.pid = fdp.ConsumeIntegral<int32_t>();
    appInfo.uid = fdp.ConsumeIntegral<int32_t>();
    appInfo.bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->InitWarningCpuInfo(faultData, appInfo);
    freeze->PerfStart(faultData.errorObject.name);
    std::string path = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->GetFirstLine(path);
}

void TestKillThread(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    int state = fdp.ConsumeIntegral<int>();
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->InsertKillThread(state, pid, uid, bundleName);
    freeze->CheckThreadKilled(pid, uid, bundleName);
    freeze->IsSkipDetect(pid, uid, bundleName, TEST_EVENT_NAME);
}

void TestKillReason(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    int killId = fdp.ConsumeIntegral<int>();
    int32_t pid = fdp.ConsumeIntegral<int32_t>();
    std::string killMsg = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->GetProcessKillReason(killId, pid, killMsg);
    freeze->GetProcessKillReason(INVALID_KILL_ID, pid, killMsg);
    freeze->GetFreezeExitReason(TEST_EVENT_NAME);
    freeze->GetFreezeExitReason(TEST_EVENT_INPUT);
    freeze->GetFreezeExitReason(TEST_EVENT_LIFECYCLE_HALF);
    freeze->GetFreezeExitReason(TEST_EVENT_LIFECYCLE_TIMEOUT);
}

void TestExcludedPid(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    bool isAdd = fdp.ConsumeBool();
    int32_t targetPid = fdp.ConsumeIntegral<int32_t>();
    int32_t profilerPid = fdp.ConsumeIntegral<int32_t>();
    freeze->UpdateFreezeExcludedPid(isAdd, targetPid, profilerPid);
    freeze->IsFreezeExcludedPid(targetPid);
}

void TestMisc(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    FaultData faultData;
    faultData.errorObject.name = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    faultData.isInForeground = fdp.ConsumeBool();
    freeze->CheckInBackGround(faultData);
    freeze->CheckNeedRecordAppRunningUnquieId(faultData.errorObject.name);
    freeze->IsBetaVersion();
    freeze->DestroyInstance();
    freeze = AppfreezeManager::GetInstance();
    freeze->RegisterAppKillTime(fdp.ConsumeIntegral<int32_t>(), fdp.ConsumeIntegral<uint64_t>());
    freeze->ReportAppFreezeSysEvents(fdp.ConsumeIntegral<int32_t>(), fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH));
    std::string fileName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    std::string content = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->WriteToFile(fileName, content);
    freeze->CollectFreezeSysMemory(content);
    freeze->GetAppfreezeInfoPath(faultData, AppfreezeManager::AppInfo{});
}

void TestJsonAndBinderParser(FuzzedDataProvider& fdp)
{
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return;
    }
    std::string line = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    freeze->GetFileToList(line);
    freeze->StrSplit(line, fdp.ConsumeIntegral<uint16_t>());
    std::ifstream fin;
    std::string stack;
    std::set<int> asyncPids;
    std::map<uint32_t, uint32_t> asyncBinderMap;
    std::vector<std::pair<uint32_t, uint64_t>> freeAsyncSpacePairs;
    freeze->BinderParser(fin, stack, asyncPids);
    freeze->BinderLineParser(fin, stack, asyncBinderMap, freeAsyncSpacePairs);
    freeze->ParseDecToHex(fdp.ConsumeIntegral<uint64_t>());
    freeze->GetHitraceInfo();
}

bool OriginalFuzzTest(const char* data, size_t size)
{
    FaultData faultData;
    AppfreezeManager::AppInfo appInfo;
    faultData.errorObject.name = data;
    faultData.errorObject.message = data;
    faultData.errorObject.stack = data;
    faultData.pid = static_cast<int32_t>(GetU32Data(data));
    faultData.tid = static_cast<int32_t>(GetU32Data(data));
    faultData.eventId = static_cast<int32_t>(GetU32Data(data));
    faultData.schedTime = static_cast<uint64_t>(GetU32Data(data));
    faultData.detectTime = static_cast<uint64_t>(GetU32Data(data));
    faultData.appfreezeInfo = data;
    appInfo.pid = static_cast<int32_t>(GetU32Data(data));
    appInfo.uid = static_cast<int32_t>(GetU32Data(data));
    appInfo.bundleName = data;
    auto freeze = AppfreezeManager::GetInstance();
    if (!freeze) {
        return false;
    }
    freeze->AppfreezeHandle(faultData, appInfo);
    freeze->AppfreezeHandleWithStack(faultData, appInfo);
    AppfreezeManager::ParamInfo info;
    info.typeId = static_cast<int>(GetU32Data(data));
    info.pid = static_cast<int32_t>(GetU32Data(data));
    info.eventName = data;
    info.bundleName = data;
    info.msg = data;
    freeze->LifecycleTimeoutHandle(info);
    freeze->WriteToFile(data, data);
    std::string bundle = data;
    freeze->IsHandleAppfreeze(bundle);
    int32_t pid = static_cast<int32_t>(GetU32Data(data));
    freeze->IsProcessDebug(pid, bundle);
    freeze->CancelAppFreezeDetect(pid, bundle);
    freeze->ResetAppfreezeState(pid, bundle);
    freeze->IsValidFreezeFilter(pid, bundle);
    freeze->RemoveDeathProcess(bundle);
    std::string stack = data;
    freeze->FindStackByPid(stack, pid);
    freeze->CatcherStacktrace(pid);
    std::string faultType = data;
    freeze->CatchJsonStacktrace(pid, faultType);
    freeze->DeleteStack(pid);
    AppfreezeManager::ParseBinderParam params;
    params.pid = static_cast<int32_t>(GetU32Data(data));
    params.eventPid = static_cast<int32_t>(GetU32Data(data));
    params.eventTid = static_cast<int32_t>(GetU32Data(data));
    std::set<int> asyncPids;
    AppfreezeManager::TerminalBinder terminalBinder;
    freeze->GetBinderPeerPids(stack, params, asyncPids, terminalBinder);
    std::string memoryContent = data;
    freeze->AcquireStack(faultData, appInfo, memoryContent);
    std::string binderInfo = data;
    freeze->NotifyANR(faultData, appInfo, binderInfo, memoryContent);
    std::string key = data;
    std::string eventName = data;
    int state = static_cast<int>(GetU32Data(data));
    freeze->SetFreezeState(key, state, eventName);
    freeze->GetReportTimes(key);
    freeze->GetLastOccurTime(key);
    return true;
}

bool ModifiedFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    TestAppfreezeHandle(fdp);
    TestLifecycleTimeoutHandle(fdp);
    TestIsHandleAppfreeze(fdp);
    TestProcessDebug(fdp);
    TestStackOperations(fdp);
    TestBinderPeerPids(fdp);
    TestAcquireAndNotify(fdp);
    TestFreezeState(fdp);
    TestCpuAndPerf(fdp);
    TestKillThread(fdp);
    TestKillReason(fdp);
    TestExcludedPid(fdp);
    TestMisc(fdp);
    TestJsonAndBinderParser(fdp);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }
    char* ch = static_cast<char*>(malloc(size + 1));
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
    OHOS::OriginalFuzzTest(ch, size);
    free(ch);
    ch = nullptr;

    OHOS::ModifiedFuzzTest(data, size);
    return 0;
}
