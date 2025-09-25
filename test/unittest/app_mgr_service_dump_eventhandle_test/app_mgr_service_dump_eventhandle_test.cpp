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

#include <gtest/gtest.h>

#include "mock_ipc_skeleton.h"

#define private public
#include "app_mgr_service.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "mock_app_mgr_service_inner.h"
#include "app_mgr_service_dump_error_code.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t HIDUMPER_SERVICE_UID = 1212;
const int32_t RANDOM_SERVICE_UID = 1500;
} // namespace

class AppMgrServiceDumpEventHandleTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceDumpEventHandleTest::SetUpTestCase(void)
{}

void AppMgrServiceDumpEventHandleTest::TearDownTestCase(void)
{}

void AppMgrServiceDumpEventHandleTest::SetUp()
{}

void AppMgrServiceDumpEventHandleTest::TearDown()
{}

/*
 * @tc.number    : AppMgrServiceDumpEventHandle_0100
 * @tc.name      : AppMgrService DumpEventQueueByPid 
 * @tc.desc      : 1.Test caller permission.
 */
HWTEST_F(AppMgrServiceDumpEventHandleTest, AppMgrServiceDumpEventHandle_0100, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppMgrServiceDumpEventHandle_0100 start");
    /**
     * @tc.step: step1. construct appMgrService test suites.
     * @tc.expected: appMgrService construct success.
     */
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);

    /**
     * @tc.step: step2. set calling Uid RANDOM_SERVICE_UID.
     * @tc.expected: expect ERR_PERMISSION_DENY_ERROR
     * @tc.desc: permission check failed.
     */
    IPCSkeleton::SetCallingUid(RANDOM_SERVICE_UID);
    std::string result;
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16("");
    args.emplace_back(arg0);
    auto retCode = appMgrService->DumpEventQueueByPid(args, result);
    EXPECT_EQ(retCode, DumpErrorCode::ERR_PERMISSION_DENY_ERROR);

    /**
     * @tc.step: step3. set calling Uid HIDUMPER_SERVICE_UID.
     * @tc.expected: expect ERR_INVALID_NUM_ARGS_ERROR.
     * @tc.desc permission check passed, args check failed
     */
    IPCSkeleton::SetCallingUid(HIDUMPER_SERVICE_UID);
    retCode = appMgrService->DumpEventQueueByPid(args, result);
    EXPECT_EQ(retCode, DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR);
    TAG_LOGD(AAFwkTag::TEST, "AppMgrServiceDumpEventHandle_0100 end");
}

/*
 * @tc.number    : AppMgrServiceDumpEventHandle_0200
 * @tc.name      : AppMgrService DumpEventQueueByPid 
 * @tc.desc      : 1.Test args.
 */
HWTEST_F(AppMgrServiceDumpEventHandleTest, AppMgrServiceDumpEventHandle_0200, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppMgrServiceDumpEventHandle_0200 start");
    /**
     * @tc.step: step1. construct appMgrService test suites.
     * @tc.expected: appMgrService construct success.
     */
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);
    IPCSkeleton::SetCallingUid(HIDUMPER_SERVICE_UID);

    /**
     * @tc.step: step2. args size 1.
     * @tc.expected: ERR_INVALID_NUM_ARGS_ERROR
     */
    std::string result;
    std::vector<std::u16string> args;
    auto arg0 = Str8ToStr16("");
    args.emplace_back(arg0);
    auto retCode = appMgrService->DumpEventQueueByPid(args, result);
    EXPECT_EQ(retCode, DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR);
    args.clear();

    /**
     * @tc.step: step3. args size 2.
    * @tc.desc: expect ERR_INVALID_PID_ERROR
     * @tc.desc: args number ok, pid number illeagal
     */
    arg0 = Str8ToStr16("--eventhandle");
    auto arg1 = Str8ToStr16("");
    args.emplace_back(arg0);
    args.emplace_back(arg1);
    retCode = appMgrService->DumpEventQueueByPid(args, result);
    EXPECT_EQ(retCode, DumpErrorCode::ERR_INVALID_PID_ERROR);
    args.clear();

    /**
     * @tc.step: step4. args size 2.
    * @tc.desc: expect ERR_INVALID_PID_ERROR
     * @tc.desc: args number ok, pid number illeagal
     */
    arg0 = Str8ToStr16("--eventhandle");
    arg1 = Str8ToStr16("aaa,bbb,ccc");
    args.emplace_back(arg0);
    args.emplace_back(arg1);
    retCode = appMgrService->DumpEventQueueByPid(args, result);
    EXPECT_EQ(retCode, DumpErrorCode::ERR_INVALID_PID_ERROR);
    args.clear();
}

/*
 * @tc.number    : AppMgrServiceDumpEventHandle_0200
 * @tc.name      : AppMgrService DumpEventQueueByPid 
 * @tc.desc      : 1.Test args.
 */
HWTEST_F(AppMgrServiceDumpEventHandleTest, AppMgrServiceDumpEventHandle_0300, TestSize.Level1)
{
    TAG_LOGD(AAFwkTag::TEST, "AppMgrServiceDumpEventHandle_0300 start");
    /**
     * @tc.step: step1. construct appMgrService test suites.
     * @tc.expected: appMgrService construct success.
     */
    auto appMgrService = std::make_shared<AppMgrService>();
    EXPECT_NE(appMgrService, nullptr);
    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    EXPECT_NE(appMgrService->taskHandler_, nullptr);
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_NE(mockAppMgrServiceInner, nullptr);
    appMgrService->SetInnerService(mockAppMgrServiceInner);
    EXPECT_NE(appMgrService->appMgrServiceInner_, nullptr);
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(
        appMgrService->taskHandler_, appMgrService->appMgrServiceInner_);
    EXPECT_NE(appMgrService->eventHandler_, nullptr);
    IPCSkeleton::SetCallingUid(HIDUMPER_SERVICE_UID);
    std::string result;
    std::vector<std::u16string> args;

    /**
     * @tc.step: step2. args size 2.
     * @tc.desc: expect ERR_INVALID_PID_ERROR
     * @tc.desc: args number ok, pid number illeagal
     */
    auto arg0 = Str8ToStr16("--eventhandle");
    auto arg1 = Str8ToStr16("-1,w,2,3,4,5,6");
    args.emplace_back(arg0);
    args.emplace_back(arg1);
    auto retCode = appMgrService->DumpEventQueueByPid(args, result);
    EXPECT_EQ(retCode, DumpErrorCode::ERR_OK);
    args.clear();

    /**
     * @tc.step: step3. args size 3.
     * @tc.desc: expect ERR_INVALID_NUM_ARGS_ERROR
     * @tc.desc: args number illeage
     */
    arg0 = Str8ToStr16("--eventhandle");
    arg1 = Str8ToStr16("1,2,3");
    auto arg2 = Str8ToStr16("-wrong");
    args.emplace_back(arg0);
    args.emplace_back(arg1);
    args.emplace_back(arg2);
    retCode = appMgrService->DumpEventQueueByPid(args, result);
    EXPECT_EQ(retCode, DumpErrorCode::ERR_INVALID_NUM_ARGS_ERROR);
    args.clear();

    /**
     * @tc.step: step4. args size 4.
     * @tc.desc: expect ERR_OK
     * @tc.desc: args ok
     */
    arg0 = Str8ToStr16("--eventhandle");
    arg1 = Str8ToStr16("1,2,3");
    arg2 = Str8ToStr16("-t");
    auto arg3 = Str8ToStr16("taskname");
    args.emplace_back(arg0);
    args.emplace_back(arg1);
    args.emplace_back(arg2);
    args.emplace_back(arg3);
    retCode = appMgrService->DumpEventQueueByPid(args, result);
    EXPECT_EQ(retCode, DumpErrorCode::ERR_OK);
    args.clear();

    /**
     * @tc.step: step5. args size 4.
     * @tc.desc: expect ERR_OK
     * @tc.desc: args ok
     */
    arg0 = Str8ToStr16("--eventhandle");
    arg1 = Str8ToStr16("1,2,3");
    arg2 = Str8ToStr16("-t1111");
    arg3 = Str8ToStr16("taskname");
    args.emplace_back(arg0);
    args.emplace_back(arg1);
    args.emplace_back(arg2);
    args.emplace_back(arg3);
    retCode = appMgrService->DumpEventQueueByPid(args, result);
    EXPECT_EQ(retCode, DumpErrorCode::ERR_UNKNOWN_OPTION_ERROR);
    args.clear();
}
}  // namespace AppExecFwk
}  // namespace OHOS
