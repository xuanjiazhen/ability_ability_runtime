/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <mutex>

#define private public
#include "exit_resident_process_manager.h"
#undef private

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "in_process_call_wrapper.h"
#include "remote_client_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class WindowVisibilityChangedListenerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void WindowVisibilityChangedListenerTest::SetUpTestCase()
{}

void WindowVisibilityChangedListenerTest::TearDownTestCase()
{}

void WindowVisibilityChangedListenerTest::SetUp()
{}

void WindowVisibilityChangedListenerTest::TearDown()
{}

/**
 * @tc.name: IsMemorySizeSufficent_001
 * @tc.desc: Verify that the IsMemorySizeSufficent interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(WindowVisibilityChangedListenerTest, IsMemorySizeSufficent_001, TestSize.Level1)
{
    auto exitResidentProcessManager = std::make_shared<ExitResidentProcessManager>();
    exitResidentProcessManager->currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_SUFFICENT;
    EXPECT_EQ(exitResidentProcessManager->IsMemorySizeSufficent(), true);
}

/**
 * @tc.name: RecordExitResidentBundleName_001
 * @tc.desc: Verify that the RecordExitResidentBundleName interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(WindowVisibilityChangedListenerTest, RecordExitResidentBundleName_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    auto exitResidentProcessManager = std::make_shared<ExitResidentProcessManager>();
    exitResidentProcessManager->currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_SUFFICENT;
    EXPECT_EQ(exitResidentProcessManager->RecordExitResidentBundleName(bundleName), false);
}

/**
 * @tc.name: RecordExitResidentBundleName_002
 * @tc.desc: Verify that the RecordExitResidentBundleName interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(WindowVisibilityChangedListenerTest, RecordExitResidentBundleName_002, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    auto exitResidentProcessManager = std::make_shared<ExitResidentProcessManager>();
    exitResidentProcessManager->currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_INSUFFICENT;
    exitResidentProcessManager->exitResidentBundleNames_.emplace_back(bundleName);
    EXPECT_EQ(exitResidentProcessManager->RecordExitResidentBundleName(bundleName), true);
}

/**
 * @tc.name: RecordExitResidentBundleDependedOnWeb_001
 * @tc.desc: Verify that the RecordExitResidentBundleDependedOnWeb interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(WindowVisibilityChangedListenerTest, RecordExitResidentBundleDependedOnWeb_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    auto exitResidentProcessManager = std::make_shared<ExitResidentProcessManager>();
    exitResidentProcessManager->RecordExitResidentBundleDependedOnWeb(bundleName);
    std::string flag = *exitResidentProcessManager->exitResidentBundlesDependedOnWeb_.begin();
    EXPECT_EQ(flag, bundleName);
}

/**
 * @tc.name: HandleMemorySizeInSufficent_001
 * @tc.desc: Verify that the HandleMemorySizeInSufficent interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(WindowVisibilityChangedListenerTest, HandleMemorySizeInSufficent_001, TestSize.Level1)
{
    auto exitResidentProcessManager = std::make_shared<ExitResidentProcessManager>();
    exitResidentProcessManager->currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_INSUFFICENT;
    exitResidentProcessManager->HandleMemorySizeInSufficent();
    EXPECT_EQ(exitResidentProcessManager->HandleMemorySizeInSufficent(),
        AAFwk::ERR_NATIVE_MEMORY_SIZE_STATE_UNCHANGED);
}

/**
 * @tc.name: HandleMemorySizeInSufficent_002
 * @tc.desc: Verify that the HandleMemorySizeInSufficent interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(WindowVisibilityChangedListenerTest, HandleMemorySizeInSufficent_002, TestSize.Level1)
{
    auto exitResidentProcessManager = std::make_shared<ExitResidentProcessManager>();
    exitResidentProcessManager->currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_SUFFICENT;
    EXPECT_EQ(exitResidentProcessManager->HandleMemorySizeInSufficent(), ERR_OK);
}

/**
 * @tc.name: HandleMemorySizeSufficent_001
 * @tc.desc: Verify that the HandleMemorySizeSufficent interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(WindowVisibilityChangedListenerTest, HandleMemorySizeSufficent_001, TestSize.Level1)
{
    std::vector<std::string> bundleNames;
    auto exitResidentProcessManager = std::make_shared<ExitResidentProcessManager>();
    exitResidentProcessManager->currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_SUFFICENT;
    EXPECT_EQ(exitResidentProcessManager->HandleMemorySizeSufficent(bundleNames),
        AAFwk::ERR_NATIVE_MEMORY_SIZE_STATE_UNCHANGED);
}

/**
 * @tc.name: HandleMemorySizeSufficent_002
 * @tc.desc: Verify that the HandleMemorySizeSufficent interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(WindowVisibilityChangedListenerTest, HandleMemorySizeSufficent_002, TestSize.Level1)
{
    std::vector<std::string> bundleNames;
    auto exitResidentProcessManager = std::make_shared<ExitResidentProcessManager>();
    exitResidentProcessManager->currentMemorySizeState_ = MemorySizeState::MEMORY_SIZE_INSUFFICENT;
    EXPECT_EQ(exitResidentProcessManager->HandleMemorySizeSufficent(bundleNames), ERR_OK);
}

/**
 * @tc.name: HandleExitResidentBundleDependedOnWeb_001
 * @tc.desc: Verify that the HandleExitResidentBundleDependedOnWeb interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(WindowVisibilityChangedListenerTest, HandleExitResidentBundleDependedOnWeb_001, TestSize.Level1)
{
    std::vector<std::string> bundleNames;
    auto exitResidentProcessManager = std::make_shared<ExitResidentProcessManager>();
    exitResidentProcessManager->HandleExitResidentBundleDependedOnWeb(bundleNames);
    auto flag = exitResidentProcessManager->exitResidentBundlesDependedOnWeb_;
    EXPECT_EQ(flag.empty(), true);
}

/**
 * @tc.name: QueryExitBundleInfos_001
 * @tc.desc: Verify that the QueryExitBundleInfos interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(WindowVisibilityChangedListenerTest, QueryExitBundleInfos_001, TestSize.Level1)
{
    std::vector<std::string> exitBundleNames;
    std::vector<AppExecFwk::BundleInfo> exitBundleInfos;
    auto exitResidentProcessManager = std::make_shared<ExitResidentProcessManager>();
    std::shared_ptr<RemoteClientManager> remoteClientManager = std::make_shared<RemoteClientManager>();
    auto bundleMgrHelper = remoteClientManager->GetBundleManagerHelper();
    exitResidentProcessManager->QueryExitBundleInfos(exitBundleNames, exitBundleInfos);
    EXPECT_NE(remoteClientManager, nullptr);
    EXPECT_NE(bundleMgrHelper, nullptr);
}

/**
 * @tc.name: IsKilledForUpgradeWeb_001
 * @tc.desc: Verify that the IsKilledForUpgradeWeb interface calls normally
 * @tc.type: FUNC
 */
HWTEST_F(WindowVisibilityChangedListenerTest, IsKilledForUpgradeWeb_001, TestSize.Level1)
{
    std::string bundleName = "bundleName";
    auto exitResidentProcessManager = std::make_shared<ExitResidentProcessManager>();
    // exitResidentProcessManager->exitResidentBundlesDependedOnWeb_;
    EXPECT_EQ(exitResidentProcessManager->IsKilledForUpgradeWeb(bundleName), false);
}
} // namespace AppExecFwk
} // namespace OHOS
