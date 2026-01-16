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

#include <dlfcn.h>
#include <gtest/gtest.h>
#include <cstdarg>

#include "ability_manager_errors.h"
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "app_utils.h"
#include "appfreeze_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_bundle_mgr_helper.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_parameters.h"
#include "mock_permission_verification.h"
#include "remote_client_manager.h"
#include "task_handler_wrap.h"
#include "user_record_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
namespace {
constexpr const char* CHECKPOINT_PATH = "/proc/checkpoint";
int g_fdOfCheckPoint = -1;
int32_t g_oriPid = -1;
int g_retOfIoCtl = -1;
#define CHECKPOINT_IOCTL_CHECKPOINT_ALL          _IOR('F', 0x2, int)
#define CHECKPOINT_IOCTL_RESTORE_ALL             _IOR('F', 0X3, int)
#define CHECKPOINT_IOCTL_KILL_ALL                _IOR('F', 0x8, int)
};
// redefinitions of C outer interfaces
extern "C" {
// dlsym does not depend on open so it should be safe to call it inside my interceptor
int open(const char* path, int flags, ...)
{
    static int (*realOpen)(const char *, int, ...) = NULL;
    if (realOpen == NULL) {
        realOpen = (int (*)(const char *, int, ...))dlsym(RTLD_NEXT, "open");
    }

    if (strcmp(path, CHECKPOINT_PATH) == 0) {
        return g_fdOfCheckPoint;
    }

    va_list args;
    va_start(args, flags);
    int result = realOpen(path, flags, args);
    va_end(args);
    return result;
}

int ioctl(int fd, unsigned long request, ...)
{
    static int (*realIoctl)(int, unsigned long, ...) = NULL;
    if (realIoctl == NULL) {
        realIoctl = (int (*)(int, unsigned long, ...))dlsym(RTLD_NEXT, "ioctl");
    }

    va_list args;
    va_start(args, request);
    if (fd == g_fdOfCheckPoint) {
        if (request == CHECKPOINT_IOCTL_KILL_ALL || request == CHECKPOINT_IOCTL_CHECKPOINT_ALL ||
            request == CHECKPOINT_IOCTL_RESTORE_ALL) {
            int32_t *oriPid = va_arg(args, int32_t *);
            if (oriPid != nullptr && g_retOfIoCtl != -1) {
                *oriPid = g_oriPid;
            }
            errno = g_retOfIoCtl;
            va_end(args);
            return g_retOfIoCtl;
        }
    }
    int result = realIoctl(fd, request, args);
    va_end(args);
    return result;
}
} // extern "C"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t TRUE_VALUE = 1;
constexpr int32_t FALSE_VALUE = 0;
constexpr int32_t ERR_OK = 0;
constexpr int32_t DLP_PARAMS_INDEX_VALUE_ZERO = 0;
constexpr int32_t DLP_PARAMS_INDEX_VALUE_ONE = 1;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr const char* DLP_PARAMS_INDEX = "ohos.dlp.params.index";
constexpr const char* TEST = "test";
} // namespace
class AppMgrServiceInnerSixthTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void CreateAppRunningRecordOfAppMgrServiceInnerByPid(int32_t pid,
        std::shared_ptr<AppMgrServiceInner> &appMgrServiceInner);
};

class TaskHandlerWrapTest : public TaskHandlerWrap {
public:
    explicit TaskHandlerWrapTest(const std::string& queueName) : TaskHandlerWrap(queueName) {};

    virtual ~TaskHandlerWrapTest() {};

protected:
    std::shared_ptr<InnerTaskHandle> SubmitTaskInner(
        std::function<void()>&& task, const TaskAttribute& taskAttr) override;
    bool CancelTaskInner(const std::shared_ptr<InnerTaskHandle>& taskHandle) override
    {
        return false;
    }

    void WaitTaskInner(const std::shared_ptr<InnerTaskHandle>& taskHandle) override
    {
        return;
    }

    uint64_t GetTaskCount() override
    {
        return tasks_.size();
    }
};

void AppMgrServiceInnerSixthTest::SetUpTestCase() {}

void AppMgrServiceInnerSixthTest::TearDownTestCase() {}

void AppMgrServiceInnerSixthTest::SetUp() {}

void AppMgrServiceInnerSixthTest::TearDown() {}

std::shared_ptr<InnerTaskHandle> TaskHandlerWrapTest::SubmitTaskInner(
    std::function<void()>&& task, const TaskAttribute& taskAttr)
{
    task();
    return nullptr;
}

void AppMgrServiceInnerSixthTest::CreateAppRunningRecordOfAppMgrServiceInnerByPid(int32_t pid,
    std::shared_ptr<AppMgrServiceInner> &appMgrServiceInner)
{
    std::string bundleName = "com.acts.imagetest";
    int32_t userId = 100;
    int32_t appIndex = 0;
    AppMgrServiceInner::MakeImageRequest request {
        .bundleName = bundleName,
        .userId = userId,
        .appCloneIndex = appIndex,
    };
    auto imageInfo = std::make_shared<ForkImageInfo>();
    appMgrServiceInner->imageInfoMap_.emplace(request, imageInfo);

    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = bundleName;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, 0, "PROCESS_NAME");
    appRunningRecord->GetPriorityObject()->SetPid(pid);
    appRunningRecord->SetUid(userId * BASE_USER_RANGE);
    appRunningRecord->SetAppIndex(appIndex);
    appRunningRecord->SetMakeImageState(MakeImageState::MAKE_PRELOAD_FINISH);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(pid, appRunningRecord);
}

/**
 * @tc.name: CreateAbilityInfo_001
 * @tc.desc: test CreateAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, CreateAbilityInfo_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateAbilityInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    MyFlag::flag1_ = TRUE_VALUE;
    MyFlag::flag_ = TRUE_VALUE;
    AAFwk::Want want;
    want.SetParam(DLP_PARAMS_INDEX, DLP_PARAMS_INDEX_VALUE_ZERO);
    AbilityInfo abilityInfo;
    auto ret1 = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret1, true);

    MyFlag::flag1_ = FALSE_VALUE;
    MyFlag::flag2_ = FALSE_VALUE;
    auto ret2 = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret2, false);

    MyFlag::flag2_ = TRUE_VALUE;
    auto ret3 = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret3, true);

    MyFlag::flag_ = FALSE_VALUE;
    auto ret4 = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret4, true);
    TAG_LOGI(AAFwkTag::TEST, "CreateAbilityInfo_001 end");
}

/**
 * @tc.name: CreateAbilityInfo_002
 * @tc.desc: test CreateAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, CreateAbilityInfo_002, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateAbilityInfo_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    MyFlag::flag1_ = FALSE_VALUE;
    MyFlag::flag2_ = TRUE_VALUE;
    MyFlag::flag_ = TRUE_VALUE;
    AAFwk::Want want;
    want.SetParam(DLP_PARAMS_INDEX, DLP_PARAMS_INDEX_VALUE_ONE);
    AbilityInfo abilityInfo;
    auto ret1 = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret1, true);

    MyFlag::flag2_ = FALSE_VALUE;
    MyFlag::flag_ = FALSE_VALUE;
    auto ret2 = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret2, false);
    TAG_LOGI(AAFwkTag::TEST, "CreateAbilityInfo_002 end");
}

/**
 * @tc.name: IsWaitingDebugApp_001
 * @tc.desc: test IsWaitingDebugApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, IsWaitingDebugApp_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "IsWaitingDebugApp_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "bundleName";
    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    auto ret1 = appMgrServiceInner->IsWaitingDebugApp(bundleName);
    EXPECT_EQ(ret1, false);

    appMgrServiceInner->waitingDebugBundleList_.insert(std::make_pair(bundleName, false));
    auto ret2 = appMgrServiceInner->IsWaitingDebugApp(bundleName);
    EXPECT_EQ(ret2, true);

    std::string bundleName1 = "testName";
    auto ret3 = appMgrServiceInner->IsWaitingDebugApp(bundleName1);
    EXPECT_EQ(ret3, false);
    TAG_LOGI(AAFwkTag::TEST, "IsWaitingDebugApp_001 end");
}

/**
 * @tc.name: AttachAppDebug_001
 * @tc.desc: test AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, AttachAppDebug_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachAppDebug_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    system::SetBoolParameter(TEST, false);
    std::string bundleName = "bundleName";
    bool isDebugFromLocal = false;
    auto ret1 = appMgrServiceInner->AttachAppDebug(bundleName, isDebugFromLocal);
    EXPECT_EQ(ret1, ERR_INVALID_OPERATION);

    MyFlag::flag_ = FALSE_VALUE;
    system::SetBoolParameter(TEST, true);
    auto ret2 = appMgrServiceInner->AttachAppDebug(bundleName, isDebugFromLocal);
    EXPECT_EQ(ret2, ERR_PERMISSION_DENIED);

    MyFlag::flag_ = TRUE_VALUE;
    appMgrServiceInner->appRunningManager_ = nullptr;
    auto ret3 = appMgrServiceInner->AttachAppDebug(bundleName, isDebugFromLocal);
    EXPECT_EQ(ret3, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "AttachAppDebug_001 end");
}

/**
 * @tc.name: AttachAppDebug_002
 * @tc.desc: test AttachAppDebug
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, AttachAppDebug_002, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AttachAppDebug_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();

    std::shared_ptr<ApplicationInfo> info = nullptr;
    int32_t recordId = 1;
    std::string processName = "processName";
    auto record = std::make_shared<AppRunningRecord>(info, recordId, processName);
    record->mainBundleName_ = "bundleName";
    record->isDebugApp_ = false;
    record->isAssertPause_ = false;
    record->mainUid_ = 10;
    int32_t id = 5;
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.insert(std::make_pair(id, record));
    std::string bundleName = "bundleName";
    bool isDebugFromLocal = false;
    system::SetBoolParameter(TEST, true);
    MyFlag::flag_ = TRUE_VALUE;
    auto ret = appMgrServiceInner->AttachAppDebug(bundleName, isDebugFromLocal);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AttachAppDebug_002 end");
}

/**
 * @tc.name: NotifyPageShow_001
 * @tc.desc: test NotifyPageShow
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, NotifyPageShow_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyPageShow_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    sptr<IRemoteObject> token = nullptr;
    PageStateData pageStateData;
    auto ret = appMgrServiceInner->NotifyPageShow(token, pageStateData);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "NotifyPageShow_001 end");
}

/**
 * @tc.name: NotifyPageHide_001
 * @tc.desc: test NotifyPageHide
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, NotifyPageHide_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "NotifyPageHide_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    sptr<IRemoteObject> token = nullptr;
    PageStateData pageStateData;
    auto ret = appMgrServiceInner->NotifyPageHide(token, pageStateData);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "NotifyPageHide_001 end");
}

/**
 * @tc.name: StartNativeProcessForDebugger_001
 * @tc.desc: test StartNativeProcessForDebugger
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, StartNativeProcessForDebugger_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    MyFlag::flag1_ = FALSE_VALUE;
    MyFlag::flag2_ = FALSE_VALUE;
    AAFwk::Want want;
    want.SetParam(DLP_PARAMS_INDEX, DLP_PARAMS_INDEX_VALUE_ONE);
    auto ret1 = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret1, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_001 end");
}

/**
 * @tc.name: StartNativeProcessForDebugger_002
 * @tc.desc: test StartNativeProcessForDebugger
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, StartNativeProcessForDebugger_002, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    MyFlag::flag1_ = TRUE_VALUE;
    MyFlag::getBundleInfoV9Flag_ = TRUE_VALUE;
    AAFwk::Want want;
    want.SetParam(DLP_PARAMS_INDEX, DLP_PARAMS_INDEX_VALUE_ONE);
    auto ret1 = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret1, ERR_INVALID_OPERATION);

    MyFlag::getBundleInfoV9Flag_ = FALSE_VALUE;
    MyFlag::getHapModuleInfoFlag_ = TRUE_VALUE;
    auto ret2 = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret2, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "StartNativeProcessForDebugger_002 end");
}

/**
 * @tc.name: GetRenderProcessTerminationStatus_002
 * @tc.type: FUNC
 * @tc.Function: GetRenderProcessTerminationStatus
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerSixthTest, GetRenderProcessTerminationStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRenderProcessTerminationStatus_002 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t renderPid = -1;
    int status = 0;

    appMgrServiceInner->appRunningManager_ = nullptr;
    int result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    pid_t pid = IPCSkeleton::GetCallingPid();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRunningRecord->priorityObject_->SetPid(pid);
    EXPECT_NE(appRunningRecord, nullptr);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(0, appRunningRecord);
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    appRunningRecord->renderPidSet_.emplace(renderPid);
    appMgrServiceInner->remoteClientManager_ = nullptr;
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    EXPECT_NE(appMgrServiceInner->remoteClientManager_, nullptr);
    appMgrServiceInner->remoteClientManager_->nwebSpawnClient_ = nullptr;
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    appMgrServiceInner->remoteClientManager_->nwebSpawnClient_ = std::make_shared<AppSpawnClient>();
    EXPECT_NE(appMgrServiceInner->remoteClientManager_->nwebSpawnClient_, nullptr);
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_NE(result, 0);

    appRunningRecord->renderPidSet_.clear();
    renderPid = 0;
    appRunningRecord->renderPidSet_.emplace(renderPid);
    result = appMgrServiceInner->GetRenderProcessTerminationStatus(renderPid, status);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "GetRenderProcessTerminationStatus_002 end";
}

/**
 * @tc.name: GetExceptionTimerId_002
 * @tc.type: FUNC
 * @tc.Function: GetExceptionTimerId
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerSixthTest, GetExceptionTimerId_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetExceptionTimerId_002 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
#ifdef APP_MGR_SERVICE_HICOLLIE_ENABLE
#else
    FaultData faultData;
    int32_t pid = 0;
    int32_t callerUid = 0;
    int result = appMgrServiceInner->GetExceptionTimerId(faultData, "", nullptr, pid, callerUid);
    EXPECT_NE(result, 0);
#endif

    GTEST_LOG_(INFO) << "GetExceptionTimerId_002 end";
}

/**
 * @tc.name: KillFaultApp_003
 * @tc.type: FUNC
 * @tc.Function: KillFaultApp
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerSixthTest, KillFaultApp_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KillFaultApp_003 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->taskHandler_ = nullptr;
    int pid = 0;
    std::string bundleName = "KillFaultAppTest";
    FaultData faultData;
    bool isNeedExit = true;
    int32_t result = appMgrServiceInner->KillFaultApp(pid, bundleName, faultData, isNeedExit);
    EXPECT_NE(result, 0);

    auto taskHandlerWrapTest = std::make_shared<TaskHandlerWrapTest>("");
    EXPECT_NE(taskHandlerWrapTest, nullptr);
    appMgrServiceInner->taskHandler_ = taskHandlerWrapTest;
    EXPECT_NE(appMgrServiceInner->taskHandler_, nullptr);
    bool rebultBranch = ProcessUtil::ProcessExist(pid);
    EXPECT_FALSE(rebultBranch);
    result = appMgrServiceInner->KillFaultApp(pid, bundleName, faultData, isNeedExit);
    EXPECT_EQ(result, 0);

    isNeedExit = false;
    faultData.forceExit = true;
    faultData.waitSaveState = false;
    result = appMgrServiceInner->KillFaultApp(pid, bundleName, faultData, isNeedExit);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "KillFaultApp_003 end";
}

/**
 * @tc.name: SetAppFreezeFilter_004
 * @tc.type: FUNC
 * @tc.Function: SetAppFreezeFilter
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerSixthTest, SetAppFreezeFilter_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAppFreezeFilter_004 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->appRunningManager_ = nullptr;

    int32_t pid = 0;
    IPCSkeleton::SetCallingPid(1);
    bool result = appMgrServiceInner->SetAppFreezeFilter(pid);
    EXPECT_FALSE(result);

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.clear();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    auto appRunningRecord = std::make_shared<AppRunningRecord>(nullptr, 0, "");
    appRunningRecord->priorityObject_->SetPid(pid);
    appRunningRecord->mainBundleName_ = "";
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(0, appRunningRecord);
    result = appMgrServiceInner->SetAppFreezeFilter(pid);
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "SetAppFreezeFilter_004 end";
}

/**
 * @tc.name: IsSharedBundleRunning_005
 * @tc.type: FUNC
 * @tc.Function: IsSharedBundleRunning
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerSixthTest, IsSharedBundleRunning_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsSharedBundleRunning_005 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string bundleName = "IsSharedBundleRunningTest";
    uint32_t versionCode = 1;
    appMgrServiceInner->appRunningManager_ = nullptr;
    bool result = appMgrServiceInner->IsSharedBundleRunning(bundleName, versionCode);
    EXPECT_FALSE(result);

    MyFlag::flag_ = 1;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    appMgrServiceInner->runningSharedBundleList_.clear();
    result = appMgrServiceInner->IsSharedBundleRunning(bundleName, versionCode);
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "IsSharedBundleRunning_005 end";
}

/**
 * @tc.name: IsAppRunningByBundleNameAndUserId_006
 * @tc.type: FUNC
 * @tc.Function: IsAppRunningByBundleNameAndUserId
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerSixthTest, IsAppRunningByBundleNameAndUserId_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAppRunningByBundleNameAndUserId_006 start";

    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    IPCSkeleton::SetCallingUid(1);
    int32_t userId = 0;
    bool isRunning = false;
    int32_t result = appMgrServiceInner->IsAppRunningByBundleNameAndUserId("", userId, isRunning);
    EXPECT_NE(result, 0);

    IPCSkeleton::SetCallingUid(FOUNDATION_UID);
    appMgrServiceInner->appRunningManager_ = nullptr;
    EXPECT_EQ(IPCSkeleton::GetCallingUid(), FOUNDATION_UID);
    result = appMgrServiceInner->IsAppRunningByBundleNameAndUserId("", userId, isRunning);
    EXPECT_NE(result, 0);

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    result = appMgrServiceInner->IsAppRunningByBundleNameAndUserId("", userId, isRunning);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "IsAppRunningByBundleNameAndUserId_006 end";
}

/**
 * @tc.name: UpdateConfigurationForBackgroundApp_001
 * @tc.desc: update configuration policy.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSixthTest, UpdateConfigurationForBackgroundApp_001, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateConfiguration_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::vector<BackgroundAppInfo> appInfos;
    AppExecFwk::ConfigurationPolicy policy;
    int32_t userId = -1;
    auto ret = appMgrServiceInner->UpdateConfigurationForBackgroundApp(appInfos, policy, userId);
    EXPECT_NE(ret, 0);

    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->UpdateConfigurationForBackgroundApp(appInfos, policy, userId);
    EXPECT_EQ(ret, ERR_NO_INIT);

    MyFlag::flag_ = 1;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    ret = appMgrServiceInner->UpdateConfigurationForBackgroundApp(appInfos, policy, userId);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);

    MyFlag::flag_ = 0;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    ret = appMgrServiceInner->UpdateConfigurationForBackgroundApp(appInfos, policy, userId);
    EXPECT_EQ(ret, ERR_NOT_SYSTEM_APP);

    TAG_LOGI(AAFwkTag::TEST, "UpdateConfiguration_001 end");
}

/**
 * @tc.name: AllowDebugCheck
 * @tc.desc: Test the state of AllowDebugCheck
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, AllowDebugCheck_001, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    ApplicationInfo applicationInfo;
    applicationInfo.bundleName = "testName";
    applicationInfo.appProvisionType = AppExecFwk::Constants::APP_PROVISION_TYPE_DEBUG;
    EXPECT_TRUE(appMgrServiceInner->AllowDebugCheck(applicationInfo));
    auto &appUtils = AAFwk::AppUtils::GetInstance();
    appUtils.isSupportAllowDebugPermission_.isLoaded = true;
    appUtils.isSupportAllowDebugPermission_.value = false;
    applicationInfo.appProvisionType = AppExecFwk::Constants::APP_PROVISION_TYPE_RELEASE;
    EXPECT_FALSE(appMgrServiceInner->AllowDebugCheck(applicationInfo));

    appUtils.isSupportAllowDebugPermission_.value = true;
    EXPECT_FALSE(appMgrServiceInner->AllowDebugCheck(applicationInfo));
}

/**
 * @tc.name: ReadForkAllPid_ShouldReturnEinvalWhenInputPidLessThan0
 * @tc.desc: Test the result of ReadForkAllPid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, ReadForkAllPid_ShouldReturnEinvalWhenInputPidLessThan0, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = -1;
    int32_t cmd = CHECKPOINT_IOCTL_CHECKPOINT_ALL;
    int32_t outPid = 0;
    EXPECT_EQ(appMgrServiceInner->ReadForkAllPid(pid, cmd, outPid), EINVAL);
    EXPECT_EQ(appMgrServiceInner->ReadForkAllPid(0, cmd, outPid), EINVAL);
}

/**
 * @tc.name: ReadForkAllPid_ShouldReturnEinvalWhenOpenFailed
 * @tc.desc: Test the result of ReadForkAllPid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, ReadForkAllPid_ShouldReturnEinvalWhenOpenFailed, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    int32_t cmd = CHECKPOINT_IOCTL_CHECKPOINT_ALL;
    int32_t outPid = 0;
    g_fdOfCheckPoint = -1;
    EXPECT_EQ(appMgrServiceInner->ReadForkAllPid(pid, cmd, outPid), EINVAL);
}

/**
 * @tc.name: ReadForkAllPid_ShouldReturn_1WhenIoctlFailed
 * @tc.desc: Test the result of ReadForkAllPid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, ReadForkAllPid_ShouldReturn_1WhenIoctlFailed, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    int32_t cmd = CHECKPOINT_IOCTL_CHECKPOINT_ALL;
    int32_t outPid = 0;
    g_fdOfCheckPoint = 100;
    g_retOfIoCtl = -1;
    EXPECT_EQ(appMgrServiceInner->ReadForkAllPid(pid, cmd, outPid), -1);
}

/**
 * @tc.name: ReadForkAllPid_ShouldReturnErrOkWhenIoctlSuccess
 * @tc.desc: Test the result of ReadForkAllPid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, ReadForkAllPid_ShouldReturnErrOkWhenIoctlSuccess, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    int32_t cmd = CHECKPOINT_IOCTL_CHECKPOINT_ALL;
    int32_t outPid = 0;
    g_fdOfCheckPoint = 100;
    g_retOfIoCtl = 0;
    g_oriPid = pid;
    EXPECT_EQ(appMgrServiceInner->ReadForkAllPid(pid, cmd, outPid), ERR_OK);
    EXPECT_EQ(outPid, pid);
}

/**
 * @tc.name: KillImageProcess_ShouldReturn_1WhenPidNotBiggerThan0
 * @tc.desc: Test the result of KillImageProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, KillImageProcess_ShouldReturn_1WhenPidNotBiggerThan0, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();

    EXPECT_EQ(appMgrServiceInner->KillImageProcess(0), -1);
    EXPECT_EQ(appMgrServiceInner->KillImageProcess(-1), -1);
}

/**
 * @tc.name: KillImageProcess_ShouldReturn_1WhenOpenFailed
 * @tc.desc: Test the result of KillImageProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, KillImageProcess_ShouldReturn_1WhenOpenFailed, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;

    g_fdOfCheckPoint = -1;
    EXPECT_EQ(appMgrServiceInner->KillImageProcess(pid), -1);
}

/**
 * @tc.name: KillImageProcess_ShouldReturn_1WhenIoctlFailed
 * @tc.desc: Test the result of KillImageProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, KillImageProcess_ShouldReturn_1WhenIoctlFailed, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;

    g_fdOfCheckPoint = 100;
    g_retOfIoCtl = -1;
    EXPECT_EQ(appMgrServiceInner->KillImageProcess(pid), -1);
}

/**
 * @tc.name: KillImageProcess_ShouldReturn0WhenIoctlSuccess
 * @tc.desc: Test the result of KillImageProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, KillImageProcess_ShouldReturn0WhenIoctlSuccess, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;

    g_fdOfCheckPoint = 100;
    g_retOfIoCtl = 0;
    g_oriPid = pid;
    EXPECT_EQ(appMgrServiceInner->KillImageProcess(pid), 0);
}

/**
 * @tc.name: HandleForkAll_ShouldReturn_1WhenAppRunningRecordIsNullptr
 * @tc.desc: Test the result of HandleForkAll
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, HandleForkAll_ShouldReturn_1WhenAppRunningRecordIsNullptr, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    EXPECT_EQ(appMgrServiceInner->HandleForkAll(pid), -1);
}

/**
 * @tc.name: HandleForkAll_ShouldReturn_1WhenInnerFailed
 * @tc.desc: Test the result of HandleForkAll
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, HandleForkAll_ShouldReturn_1WhenReadForkPidIsZero, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    CreateAppRunningRecordOfAppMgrServiceInnerByPid(pid, appMgrServiceInner);

    g_fdOfCheckPoint = -1;
    EXPECT_EQ(appMgrServiceInner->HandleForkAll(pid), -1);
}

/**
 * @tc.name: HandleForkAllInner_ShouldReturn_1WhenRecordImageStateNotPreloadFinish
 * @tc.desc: Test the result of HandleForkAllInner
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, HandleForkAllInner_ShouldReturnHasBeenUsedWhenRecordImageStateNotPreloadFinish,
    TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    ApplicationInfo appInfo;
    appInfo.name = "appName";
    appInfo.bundleName = "bundleName";
    auto applicationInfo = std::make_shared<ApplicationInfo>(appInfo);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(applicationInfo, 0, "PROCESS_NAME");
    appRunningRecord->GetPriorityObject()->SetPid(pid);
    appRunningRecord->SetMakeImageState(MakeImageState::MAKE_PRELOAD_START);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(pid, appRunningRecord);
    EXPECT_EQ(appMgrServiceInner->HandleForkAllInner(appRunningRecord, pid), ImageError::ERR_TEMPLATE_HAS_BEEN_USED);
}

/**
 * @tc.name: HandleForkAllInner_ShouldReturn_1WhenImageInfoNotExist
 * @tc.desc: Test the result of HandleForkAllInner
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, HandleForkAllInner_ShouldReturn_1WhenImageInfoNotExist, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    ApplicationInfo appInfo;
    appInfo.name = "appName";
    appInfo.bundleName = "bundleName";
    auto applicationInfo = std::make_shared<ApplicationInfo>(appInfo);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(applicationInfo, 0, "PROCESS_NAME");
    appRunningRecord->GetPriorityObject()->SetPid(pid);
    appRunningRecord->SetMakeImageState(MakeImageState::MAKE_PRELOAD_FINISH);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(pid, appRunningRecord);
    EXPECT_EQ(appMgrServiceInner->HandleForkAllInner(appRunningRecord, pid), ImageError::ERR_TEMPLATE_HAS_BEEN_USED);
}

/**
 * @tc.name: HandleForkAllInner_ShouldReturnInnerErrorWhenGetApplicationClientFailed
 * @tc.desc: Test the result of HandleForkAllInner
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, HandleForkAllInner_ShouldReturnInnerErrorWhenGetApplicationClientFailed,
    TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    CreateAppRunningRecordOfAppMgrServiceInnerByPid(pid, appMgrServiceInner);
    auto appRecord = appMgrServiceInner->GetAppRunningRecordByPid(pid);
    ASSERT_NE(appRecord, nullptr);
    appRecord->SetApplicationClient(nullptr);
    EXPECT_EQ(appMgrServiceInner->HandleForkAllInner(appRecord, pid), ImageError::INNER_ERR);
}

/**
 * @tc.name: HandleMakeImageTimeout_ShouldDonothingWhenTaskHandlerIsNullptr
 * @tc.desc: Test the result of HandleMakeImageTimeout
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, HandleMakeImageTimeout_ShouldDonothingWhenTaskHandlerIsNullptr, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string bundleName = "com.acts.imagetest";
    int32_t userId = 100;
    int32_t appIndex = 0;
    appMgrServiceInner->HandleMakeImageTimeout(bundleName, userId, appIndex);
    EXPECT_EQ(appMgrServiceInner->GetImageInfo(bundleName, userId, appIndex), nullptr);
}

/**
 * @tc.name: HandleMakeImageTimeout_ShouldDonothingWhenImageInfoNotExist
 * @tc.desc: Test the result of HandleMakeImageTimeout
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, HandleMakeImageTimeout_ShouldDonothingWhenImageInfoNotExist, TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string bundleName = "com.acts.imagetest";
    int32_t userId = 100;
    int32_t appIndex = 0;
    auto taskHandlerWrapTest = std::make_shared<TaskHandlerWrapTest>("");
    EXPECT_NE(taskHandlerWrapTest, nullptr);
    appMgrServiceInner->taskHandler_ = taskHandlerWrapTest;
    EXPECT_NE(appMgrServiceInner->taskHandler_, nullptr);
    appMgrServiceInner->HandleMakeImageTimeout(bundleName, userId, appIndex);
    EXPECT_EQ(appMgrServiceInner->GetImageInfo(bundleName, userId, appIndex), nullptr);
}

/**
 * @tc.name: PreloadModuleFinished_ShouldDonothingWhenImageStateNotPreloadStart
 * @tc.desc: Test the result of PreloadModuleFinished
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, PreloadModuleFinished_ShouldDonothingWhenImageStateNotPreloadStart,
    TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(applicationInfo, 0, "PROCESS_NAME");
    appRunningRecord->GetPriorityObject()->SetPid(pid);
    appRunningRecord->SetMakeImageState(MakeImageState::MAKE_IMAGE_START);

    appMgrServiceInner->PreloadModuleFinished(pid);

    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(pid, appRunningRecord);
    appMgrServiceInner->PreloadModuleFinished(pid);
    EXPECT_EQ(appRunningRecord->GetMakeImageState(), MakeImageState::MAKE_IMAGE_START);
}

/**
 * @tc.name: PreloadModuleFinished_ShouldUpdateImageStateWhenImageStateIsPreloadStart
 * @tc.desc: Test the result of PreloadModuleFinished
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, PreloadModuleFinished_ShouldUpdateImageStateWhenImageStateIsPreloadStart,
    TestSize.Level1)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appRunningRecord = std::make_shared<AppRunningRecord>(applicationInfo, 0, "PROCESS_NAME");
    appRunningRecord->GetPriorityObject()->SetPid(pid);
    appRunningRecord->SetMakeImageState(MakeImageState::MAKE_PRELOAD_START);

    appMgrServiceInner->PreloadModuleFinished(pid);

    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(pid, appRunningRecord);
    appMgrServiceInner->PreloadModuleFinished(pid);
    EXPECT_EQ(appRunningRecord->GetMakeImageState(), MakeImageState::MAKE_PRELOAD_FINISH);
}

/**
 * @tc.name: TryToUseImageInfo_ShouldReturnErrOkWhenAppRunningManagerIsNullptr
 * @tc.desc: Test the result of TryToUseImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, TryToUseImageInfo_ShouldReturnErrOkWhenAppRunningManagerIsNullptr,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturnErrOkWhenAppRunningManagerIsNullptr start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    int32_t appIndex = 0;
    std::string processName = "processName";
    std::string instanceKey = "instanceKey";
    std::string specifiedProcessFlag = "specifiedProcessFlag";
    std::string customProcessFlag = "customProcessFlag";
    std::shared_ptr<AppRunningRecord> appRecord;
    appMgrServiceInner->appRunningManager_ = nullptr;
    EXPECT_EQ(appMgrServiceInner->TryToUseImageInfo(nullptr, nullptr, nullptr, "callerKey", appIndex, processName,
        instanceKey, specifiedProcessFlag, customProcessFlag, appRecord), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturnErrOkWhenAppRunningManagerIsNullptr end");
}

/**
 * @tc.name: TryToUseImageInfo_ShouldReturnErrOkWhenImageInfoNotExist
 * @tc.desc: Test the result of TryToUseImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, TryToUseImageInfo_ShouldReturnErrOkWhenImageInfoNotExist,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturnErrOkWhenImageInfoNotExist start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    int32_t appIndex = 0;
    std::string processName = "processName";
    std::string instanceKey = "instanceKey";
    std::string specifiedProcessFlag = "specifiedProcessFlag";
    std::string customProcessFlag = "customProcessFlag";
    std::shared_ptr<AppRunningRecord> appRecord;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    EXPECT_EQ(appMgrServiceInner->TryToUseImageInfo(nullptr, appInfo, nullptr, "callerKey", appIndex, processName,
        instanceKey, specifiedProcessFlag, customProcessFlag, appRecord), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturnErrOkWhenImageInfoNotExist end");
}

/**
 * @tc.name: TryToUseImageInfo_ShouldReturnErrOkWhenImageInfoNotMatch
 * @tc.desc: Test the result of TryToUseImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, TryToUseImageInfo_ShouldReturnErrOkWhenImageInfoNotMatch,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturnErrOkWhenImageInfoNotMatch start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    int32_t appIndex = 0;
    std::string processName = "processName";
    std::string instanceKey = "instanceKey";
    std::string specifiedProcessFlag = "specifiedProcessFlag";
    std::string customProcessFlag = "customProcessFlag";
    std::shared_ptr<AppRunningRecord> appRecord;
    std::string bundleName = "com.acts.imagetest";
    int32_t userId = 100;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = bundleName;
    appInfo->uid = userId * BASE_USER_RANGE;
    PreloadRequest preloadRequest;
    appMgrServiceInner->PreAddImageInfo(bundleName, userId, appIndex, nullptr, preloadRequest);
    EXPECT_EQ(appMgrServiceInner->TryToUseImageInfo(nullptr, appInfo, nullptr, "callerKey", appIndex, processName,
        instanceKey, specifiedProcessFlag, customProcessFlag, appRecord), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturnErrOkWhenImageInfoNotMatch end");
}

/**
 * @tc.name: TryToUseImageInfo_ShouldReturn_1WhenCreateAppRunningRecordFailed
 * @tc.desc: Test the result of TryToUseImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, TryToUseImageInfo_ShouldReturn_1WhenCreateAppRunningRecordFailed,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturn_1WhenCreateAppRunningRecordFailed start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    int32_t appIndex = 0;
    std::string processName = "processName";
    std::string instanceKey = "instanceKey";
    std::string specifiedProcessFlag = "specifiedProcessFlag";
    std::string customProcessFlag = "customProcessFlag";
    std::shared_ptr<AppRunningRecord> appRecord;
    std::string bundleName = "com.acts.imagetest";
    std::string callerKey = "imageCallerKey";
    int32_t userId = 100;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = bundleName;
    appInfo->uid = userId * BASE_USER_RANGE;
    PreloadRequest preloadRequest;
    appMgrServiceInner->PreAddImageInfo(bundleName, userId, appIndex, nullptr, preloadRequest);
    auto imageInfo = appMgrServiceInner->GetImageInfo(bundleName, userId, appIndex);
    EXPECT_NE(imageInfo, nullptr);
    auto appRecord1 = std::make_shared<AppRunningRecord>(nullptr, 0, processName);
    EXPECT_NE(appRecord1, nullptr);
    appRecord1->SetInstanceKey(instanceKey);
    appRecord1->SetCustomProcessFlag(customProcessFlag);
    appRecord1->SetSpecifiedProcessFlag(specifiedProcessFlag);
    imageInfo->baseAppRecord = appRecord1;
    imageInfo->abilityInfo = std::make_shared<AbilityInfo>();
    imageInfo->want = nullptr;
    imageInfo->imagePid = pid;
    EXPECT_EQ(appMgrServiceInner->TryToUseImageInfo(nullptr, appInfo, nullptr, callerKey, appIndex, processName,
        instanceKey, specifiedProcessFlag, customProcessFlag, appRecord), -1);
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturn_1WhenCreateAppRunningRecordFailed end");
}

/**
 * @tc.name: TryToUseImageInfo_ShouldReturn_1WhenReadForkAllPidReturnLessThan0OrPidNotEqual
 * @tc.desc: Test the result of TryToUseImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, TryToUseImageInfo_ShouldReturn_1WhenReadForkAllPidReturnLessThan0OrPidNotEqual,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturn_1WhenReadForkAllPidReturnLessThan0OrPidNotEqual start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    int32_t appIndex = 0;
    std::string processName = "processName";
    std::string instanceKey = "instanceKey";
    std::string specifiedProcessFlag = "specifiedProcessFlag";
    std::string customProcessFlag = "customProcessFlag";
    std::shared_ptr<AppRunningRecord> appRecord;
    std::string bundleName = "com.acts.imagetest";
    std::string callerKey = "imageCallerKey";
    int32_t userId = 100;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = bundleName;
    appInfo->uid = userId * BASE_USER_RANGE;
    PreloadRequest preloadRequest;
    appMgrServiceInner->PreAddImageInfo(bundleName, userId, appIndex, nullptr, preloadRequest);
    auto imageInfo = appMgrServiceInner->GetImageInfo(bundleName, userId, appIndex);
    EXPECT_NE(imageInfo, nullptr);
    auto appRecord1 = std::make_shared<AppRunningRecord>(appInfo, 0, processName);
    EXPECT_NE(appRecord1, nullptr);
    appRecord1->SetInstanceKey(instanceKey);
    appRecord1->SetCustomProcessFlag(customProcessFlag);
    appRecord1->SetSpecifiedProcessFlag(specifiedProcessFlag);
    appRecord1->GetPriorityObject()->SetPid(pid);
    imageInfo->baseAppRecord = appRecord1;
    imageInfo->abilityInfo = std::make_shared<AbilityInfo>();
    imageInfo->want = nullptr;
    imageInfo->imagePid = pid;
    g_fdOfCheckPoint = -1;
    EXPECT_EQ(appMgrServiceInner->TryToUseImageInfo(nullptr, appInfo, nullptr, callerKey, appIndex, processName,
        instanceKey, specifiedProcessFlag, customProcessFlag, appRecord), -1);

    g_fdOfCheckPoint = 100;
    g_retOfIoCtl = 0;
    g_oriPid = 0;
    EXPECT_EQ(appMgrServiceInner->TryToUseImageInfo(nullptr, appInfo, nullptr, callerKey, appIndex, processName,
        instanceKey, specifiedProcessFlag, customProcessFlag, appRecord), -1);
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturn_1WhenReadForkAllPidReturnLessThan0OrPidNotEqual end");
}

/**
 * @tc.name: TryToUseImageInfo_ShouldReturnErrOkWhenPidIsEqual
 * @tc.desc: Test the result of TryToUseImageInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSixthTest, TryToUseImageInfo_ShouldReturnErrOkWhenPidIsEqual,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturnErrOkWhenPidIsEqual start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int32_t pid = 100;
    int32_t appIndex = 0;
    std::string processName = "processName";
    std::string instanceKey = "instanceKey";
    std::string specifiedProcessFlag = "specifiedProcessFlag";
    std::string customProcessFlag = "customProcessFlag";
    std::shared_ptr<AppRunningRecord> appRecord;
    std::string bundleName = "com.acts.imagetest";
    std::string callerKey = "imageCallerKey";
    int32_t userId = 100;
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    auto appInfo = std::make_shared<ApplicationInfo>();
    appInfo->bundleName = bundleName;
    appInfo->uid = userId * BASE_USER_RANGE;
    PreloadRequest preloadRequest;
    appMgrServiceInner->PreAddImageInfo(bundleName, userId, appIndex, nullptr, preloadRequest);
    auto imageInfo = appMgrServiceInner->GetImageInfo(bundleName, userId, appIndex);
    EXPECT_NE(imageInfo, nullptr);
    auto appRecord1 = std::make_shared<AppRunningRecord>(appInfo, 0, processName);
    EXPECT_NE(appRecord1, nullptr);
    appRecord1->SetInstanceKey(instanceKey);
    appRecord1->SetCustomProcessFlag(customProcessFlag);
    appRecord1->SetSpecifiedProcessFlag(specifiedProcessFlag);
    appRecord1->GetPriorityObject()->SetPid(pid);
    imageInfo->baseAppRecord = appRecord1;
    imageInfo->abilityInfo = std::make_shared<AbilityInfo>();
    imageInfo->want = nullptr;
    imageInfo->imagePid = pid;
    g_fdOfCheckPoint = 100;
    g_retOfIoCtl = 0;
    g_oriPid = pid;
    EXPECT_EQ(appMgrServiceInner->TryToUseImageInfo(nullptr, appInfo, nullptr, callerKey, appIndex, processName,
        instanceKey, specifiedProcessFlag, customProcessFlag, appRecord), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "TryToUseImageInfo_ShouldReturnErrOkWhenPidIsEqual end");
}
} // namespace AppExecFwk
} // namespace OHOS