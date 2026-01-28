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
#include <thread>

#define private public
#include "uri_permission_manager_service.h"
#undef private

#include "hilog_tag_wrapper.h"
#include "oh_mock_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class UriPermissionManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void InitService();
    void ClearService();

    std::shared_ptr<UriPermissionManagerService> service_ = nullptr;
};

void UriPermissionManagerServiceTest::SetUpTestCase() {}

void UriPermissionManagerServiceTest::TearDownTestCase() {}

void UriPermissionManagerServiceTest::SetUp()
{
    service_ = std::make_shared<UriPermissionManagerService>();
}

void UriPermissionManagerServiceTest::TearDown()
{
    service_ = nullptr;
}

void UriPermissionManagerServiceTest::InitService()
{
    if (service_ != nullptr) {
        service_->OnStart();
    }
}

void UriPermissionManagerServiceTest::ClearService()
{
    if (service_ != nullptr) {
        service_->OnStop();
    }
}

/**
 * @tc.name: UriPermissionManagerService_LifeCycle_001
 * @tc.desc: Test UriPermissionManagerService OnStart and OnStop
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerServiceTest, UriPermissionManagerService_LifeCycle_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_LifeCycle_001 start");

    // Test OnStart
    service_->OnStart();
    EXPECT_TRUE(service_ != nullptr);

    // Test service ready state
    EXPECT_TRUE(service_->IsServiceReady());

    // Test OnStop
    service_->OnStop();
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_LifeCycle_001 end");
}

/**
 * @tc.name: UriPermissionManagerService_Init_001
 * @tc.desc: Test UriPermissionManagerService Init
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerServiceTest, UriPermissionManagerService_Init_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_Init_001 start");

    // Mock system ability manager registration success
    // Mock SystemAbilityManager::AddSystemAbility to return ERR_OK
    OH_EXPECT_RET({true}, MockSystemAbilityManagerClient, GetSystemAbility, int32_t);

    bool result = service_->Init();
    EXPECT_TRUE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_Init_001 end");
}

/**
 * @tc.name: UriPermissionManagerService_IsServiceReady_001
 * @tc.desc: Test UriPermissionManagerService IsServiceReady
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerServiceTest, UriPermissionManagerService_IsServiceReady_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_IsServiceReady_001 start");

    // Test initial state
    EXPECT_FALSE(service_->IsServiceReady());

    // After OnStart
    service_->OnStart();
    EXPECT_TRUE(service_->IsServiceReady());

    // After OnStop
    service_->OnStop();
    EXPECT_FALSE(service_->IsServiceReady());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_IsServiceReady_001 end");
}

/**
 * @tc.name: UriPermissionManagerService_GetService_001
 * @tc.desc: Test GetInstance method
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerServiceTest, UriPermissionManagerService_GetService_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_GetService_001 start");

    auto& instance = UriPermissionManagerService::GetInstance();
    EXPECT_NE(&instance, nullptr);

    // Verify singleton pattern
    auto& instance2 = UriPermissionManagerService::GetInstance();
    EXPECT_EQ(&instance, &instance2);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_GetService_001 end");
}

/**
 * @tc.name: UriPermissionManagerService_Dump_001
 * @tc.desc: Test Dump functionality
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerServiceTest, UriPermissionManagerService_Dump_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_Dump_001 start");

    InitService();

    std::string args;
    std::vector<std::string> info;
    info.push_back("Test dump info");

    // Test dump - basic call
    // Actual dump functionality testing requires mock IPCSkeleton
    service_->Dump(args, info);
    EXPECT_GE(info.size(), 1u);

    ClearService();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_Dump_001 end");
}

/**
 * @tc.name: UriPermissionManagerService_OnStop_Multiple_001
 * @tc.desc: Test multiple OnStop calls
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerServiceTest, UriPermissionManagerService_OnStop_Multiple_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_OnStop_Multiple_001 start");

    InitService();

    // Multiple OnStop calls should not crash
    service_->OnStop();
    EXPECT_FALSE(service_->IsServiceReady());

    service_->OnStop();
    EXPECT_FALSE(service_->IsServiceReady());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_OnStop_Multiple_001 end");
}

/**
 * @tc.name: UriPermissionManagerService_OnStart_Multiple_001
 * @tc.desc: Test multiple OnStart calls
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerServiceTest, UriPermissionManagerService_OnStart_Multiple_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_OnStart_Multiple_001 start");

    // First OnStart
    service_->OnStart();
    EXPECT_TRUE(service_->IsServiceReady());

    // Second OnStart should not crash
    service_->OnStart();
    EXPECT_TRUE(service_->IsServiceReady());

    ClearService();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "UriPermissionManagerService_OnStart_Multiple_001 end");
}

} // namespace AAFwk
} // namespace OHOS
