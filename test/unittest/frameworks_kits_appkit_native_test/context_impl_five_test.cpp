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
#include <singleton.h>

#include "ability_local_record.h"
#include "application_context.h"
#include "constants.h"
#include "context.h"
#include "context_impl.h"
#include "hap_module_info.h"
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#include "mock_ability_token.h"
#include "mock_bundle_manager.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

namespace {
const mode_t MODE = 0770;
const int64_t CONTEXT_CREATE_BY_SYSTEM_APP(0x00000001);
#ifdef SUPPORT_GRAPHICS
const uint64_t INVALID_DISPLAY_ID = 500000;
#endif
const uint64_t DEFAULT_DISPLAY_ID = 0;
const float DENSITY = 1.5;
constexpr const char* DIRECTION_HORIZONTAL = "horizontal";
} // namespace

class ContextImplFiveTest : public testing::Test {
public:
    ContextImplFiveTest() : contextImpl_(nullptr)
    {}
    ~ContextImplFiveTest()
    {}
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl_ = nullptr;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ContextImplFiveTest::SetUpTestCase(void)
{}

void ContextImplFiveTest::TearDownTestCase(void)
{}

void ContextImplFiveTest::SetUp(void)
{
    contextImpl_ = std::make_shared<AbilityRuntime::ContextImpl>();
    auto config = std::make_shared<AppExecFwk::Configuration>();
    EXPECT_NE(config, nullptr);
    contextImpl_->SetConfiguration(config);
    sptr<IRemoteObject> bundleObject = new (std::nothrow) BundleMgrService();
    DelayedSingleton<SysMrgClient>::GetInstance()->RegisterSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID,
        bundleObject);
}

void ContextImplFiveTest::TearDown(void)
{}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetDatabaseDirWithCheck_001
 * @tc.name: GetDatabaseDirWithCheck
 * @tc.desc: GetDatabaseDirWithCheck test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplFiveTest, AppExecFwk_ContextImpl_GetDatabaseDirWithCheck_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetDatabaseDirWithCheck_001 start";
    ASSERT_NE(contextImpl_, nullptr);
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string dir;
    EXPECT_EQ(contextImpl->GetDatabaseDirWithCheck(true, dir), ERR_OK);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetDatabaseDirWithCheck_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetGroupDatabaseDirWithCheck_001
 * @tc.name: GetGroupDatabaseDirWithCheck
 * @tc.desc: GetGroupDatabaseDirWithCheck test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplFiveTest, AppExecFwk_ContextImpl_GetGroupDatabaseDirWithCheck_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetGroupDatabaseDirWithCheck_001 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    contextImpl->currArea_ = "el1";
    string systemDatabaseDir;
    auto res = contextImpl->GetGroupDatabaseDirWithCheck("", true, systemDatabaseDir);
    EXPECT_NE(res, ERR_OK);
    res = contextImpl->GetGroupDatabaseDirWithCheck("", false, systemDatabaseDir);
    EXPECT_NE(res, ERR_OK);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetGroupDatabaseDirWithCheck_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetGroupDatabaseDirWithCheck_001
 * @tc.name: GetGroupDatabaseDirWithCheck
 * @tc.desc: GetGroupDatabaseDirWithCheck test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplFiveTest, AppExecFwk_ContextImpl_GetGroupDatabaseDirWithCheck_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetGroupDatabaseDirWithCheck_002 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    EXPECT_NE(contextImpl, nullptr);
    contextImpl->currArea_ = "KKK";
    string systemDatabaseDir;
    auto res = contextImpl->GetGroupDatabaseDirWithCheck("Invalid_Group", true, systemDatabaseDir);
    EXPECT_NE(res, ERR_OK);
    res = contextImpl->GetGroupDatabaseDirWithCheck("Invalid_Group", false, systemDatabaseDir);
    EXPECT_NE(res, ERR_OK);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetGroupDatabaseDirWithCheck_002 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetPreferencesDirWithCheck_001
 * @tc.name: GetPreferencesDirWithCheck
 * @tc.desc: GetPreferencesDirWithCheck test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplFiveTest, AppExecFwk_ContextImpl_GetPreferencesDirWithCheck_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetPreferencesDirWithCheck_001 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string dir;
    EXPECT_EQ(contextImpl->GetPreferencesDirWithCheck(true, dir), ERR_OK);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetPreferencesDirWithCheck_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_GetAllTempDir_001
 * @tc.name: GetAllTempDir
 * @tc.desc: GetAllTempDir test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplFiveTest, AppExecFwk_ContextImpl_GetAllTempDir_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetAllTempDir_001 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::vector<std::string> tempDir1{};
    contextImpl->GetAllTempDir(tempDir1);
    EXPECT_NE(tempDir1.size(), 0);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_GetAllTempDir_001 end";
}

/**
 * @tc.number: AppExecFwk_ContextImpl_CreateDirIfNotExist_001
 * @tc.name: CreateDirIfNotExist
 * @tc.desc: CreateDirIfNotExist test
 * @tc.type: FUNC
 */
HWTEST_F(ContextImplFiveTest, AppExecFwk_ContextImpl_CreateDirIfNotExist_001, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateDirIfNotExist_001 start";
    auto contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    ASSERT_NE(contextImpl, nullptr);
    std::string dir = contextImpl->GetBaseDir() + AbilityRuntime::ContextImpl::CONTEXT_FILES;
    EXPECT_FALSE(dir.empty());
    contextImpl->CreateDirIfNotExist(dir, MODE);
    GTEST_LOG_(INFO) << "AppExecFwk_ContextImpl_CreateDirIfNotExist_001 end";
}
}  // namespace AppExecFwk
}
