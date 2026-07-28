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

#include <gtest/gtest.h>

#define private public
#define protected public
#include "skill_execute_manager.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "bundle_skill/skill_info.h"
#include "hilog_tag_wrapper.h"
#include "skill/skill_execute_result.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TEST_BUNDLE_NAME = "com.test.bundle";
const std::string TEST_MODULE_NAME = "entry";
const std::string TEST_SKILL_NAME = "PlayMusic";
const std::string TEST_ABILITY_NAME = "EntryAbility";
const std::string TEST_EXTERNAL_CODE = "external_req_001";
} // namespace

class SkillExecuteManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<SkillExecuteManager> manager_;
};

void SkillExecuteManagerTest::SetUpTestCase(void) {}
void SkillExecuteManagerTest::TearDownTestCase(void) {}

void SkillExecuteManagerTest::SetUp()
{
    manager_ = DelayedSingleton<SkillExecuteManager>::GetInstance();
    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    manager_->records_.clear();
    manager_->seqToRequestCodeMap_.clear();
    manager_->requestCodeSeq_ = 0;
}

void SkillExecuteManagerTest::TearDown()
{
    if (manager_ == nullptr) {
        return;
    }
    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    manager_->records_.clear();
    manager_->seqToRequestCodeMap_.clear();
}

/**
 * @tc.name: GenerateSkillWant_0100
 * @tc.desc: Test GenerateSkillWant returns ERR_OK when skillInfo has abilityName
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, GenerateSkillWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateSkillWant_0100 begin");

    AppExecFwk::SkillInfo skillInfo;
    skillInfo.bundleName = TEST_BUNDLE_NAME;
    skillInfo.moduleName = TEST_MODULE_NAME;
    skillInfo.skillName = TEST_SKILL_NAME;
    skillInfo.abilityName = TEST_ABILITY_NAME;

    Want want;
    int32_t userId = 100;
    AppExecFwk::ExtensionAbilityType targetType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto ret = manager_->GenerateSkillWant(skillInfo, want, userId, "req_001", targetType);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(want.GetBundle(), TEST_BUNDLE_NAME);

    TAG_LOGI(AAFwkTag::TEST, "GenerateSkillWant_0100 end");
}

/**
 * @tc.name: GenerateSkillWant_0200
 * @tc.desc: Test GenerateSkillWant returns ERR_INVALID_VALUE when abilityName empty and BMS unavailable
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, GenerateSkillWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateSkillWant_0200 begin");

    AppExecFwk::SkillInfo skillInfo;
    skillInfo.bundleName = TEST_BUNDLE_NAME;
    skillInfo.moduleName = TEST_MODULE_NAME;
    skillInfo.skillName = TEST_SKILL_NAME;

    Want want;
    int32_t userId = 100;
    AppExecFwk::ExtensionAbilityType targetType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto ret = manager_->GenerateSkillWant(skillInfo, want, userId, "req_002", targetType);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "GenerateSkillWant_0200 end");
}

/**
 * @tc.name: QuerySkillInfo_0100
 * @tc.desc: Test QuerySkillInfo returns ERR_INVALID_VALUE when SkillManager proxy unavailable
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, QuerySkillInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QuerySkillInfo_0100 begin");

    AppExecFwk::SkillInfo skillInfo;
    auto ret = manager_->QuerySkillInfo(TEST_BUNDLE_NAME, TEST_MODULE_NAME, TEST_SKILL_NAME, 100, skillInfo);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "QuerySkillInfo_0100 end");
}

/**
 * @tc.name: CheckSkillPermission_0100
 * @tc.desc: Test CheckSkillPermission returns ERR_NOT_SYSTEM_APP when caller is not SA
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CheckSkillPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckSkillPermission_0100 begin");

    AppExecFwk::SkillInfo skillInfo;
    auto ret = manager_->CheckSkillPermission(skillInfo, 0);

    EXPECT_EQ(ret, ERR_NOT_SYSTEM_APP);

    TAG_LOGI(AAFwkTag::TEST, "CheckSkillPermission_0100 end");
}

/**
 * @tc.name: CreateExecuteRecord_0100
 * @tc.desc: Test CreateExecuteRecord returns numeric requestCode when externalRequestCode empty
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateExecuteRecord_0100 begin");

    auto requestCode = manager_->CreateExecuteRecord(nullptr, TEST_BUNDLE_NAME, "caller", 0, nullptr);

    EXPECT_FALSE(requestCode.empty());
    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_EQ(manager_->records_.count(requestCode), 1u);

    TAG_LOGI(AAFwkTag::TEST, "CreateExecuteRecord_0100 end");
}

/**
 * @tc.name: CreateExecuteRecord_0200
 * @tc.desc: Test CreateExecuteRecord returns externalRequestCode when provided
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateExecuteRecord_0200 begin");

    auto requestCode = manager_->CreateExecuteRecord(
        nullptr, TEST_BUNDLE_NAME, "caller", 0, nullptr, TEST_EXTERNAL_CODE);

    EXPECT_EQ(requestCode, TEST_EXTERNAL_CODE);
    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_EQ(manager_->records_.count(requestCode), 1u);

    TAG_LOGI(AAFwkTag::TEST, "CreateExecuteRecord_0200 end");
}

/**
 * @tc.name: ExecuteSkillDone_0100
 * @tc.desc: Test ExecuteSkillDone returns ERR_CODE_INVALID_ID when record not found
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0100 begin");

    AppExecFwk::SkillExecuteResult result;
    auto ret = manager_->ExecuteSkillDone("nonexistent", 0, result, TEST_BUNDLE_NAME);

    EXPECT_EQ(ret, ERR_CODE_INVALID_ID);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0100 end");
}

/**
 * @tc.name: OnTimeout_0100
 * @tc.desc: Test OnTimeout returns silently when seq not found
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnTimeout_0100 begin");

    manager_->OnTimeout(99999);

    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_TRUE(manager_->records_.empty());

    TAG_LOGI(AAFwkTag::TEST, "OnTimeout_0100 end");
}

/**
 * @tc.name: OnLaunchCompleted_0100
 * @tc.desc: Test OnLaunchCompleted returns silently when record not found
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnLaunchCompleted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnLaunchCompleted_0100 begin");

    manager_->OnLaunchCompleted("nonexistent");

    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_TRUE(manager_->records_.empty());

    TAG_LOGI(AAFwkTag::TEST, "OnLaunchCompleted_0100 end");
}

/**
 * @tc.name: OnLaunchFailed_0100
 * @tc.desc: Test OnLaunchFailed returns silently when record not found
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnLaunchFailed_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnLaunchFailed_0100 begin");

    manager_->OnLaunchFailed("nonexistent", ERR_INVALID_VALUE);

    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_TRUE(manager_->records_.empty());

    TAG_LOGI(AAFwkTag::TEST, "OnLaunchFailed_0100 end");
}

/**
 * @tc.name: OnTargetProcessDied_0100
 * @tc.desc: Test OnTargetProcessDied returns silently when no record hit
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTargetProcessDied_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnTargetProcessDied_0100 begin");

    manager_->OnTargetProcessDied(TEST_BUNDLE_NAME, 12345);

    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_TRUE(manager_->records_.empty());

    TAG_LOGI(AAFwkTag::TEST, "OnTargetProcessDied_0100 end");
}

/**
 * @tc.name: ExecuteSkillDone_0200
 * @tc.desc: Test ExecuteSkillDone returns ERR_INVALID_VALUE when callerBundle mismatches target
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0200 begin");

    auto requestCode = manager_->CreateExecuteRecord(nullptr, TEST_BUNDLE_NAME, "caller", 0, nullptr);
    AppExecFwk::SkillExecuteResult result;
    auto ret = manager_->ExecuteSkillDone(requestCode, 0, result, "mismatch_caller");

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_EQ(manager_->records_.count(requestCode), 1u);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0200 end");
}

/**
 * @tc.name: ExecuteSkillDone_0300
 * @tc.desc: Test ExecuteSkillDone returns ERR_INVALID_VALUE when state is not EXECUTING
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0300 begin");

    auto requestCode = manager_->CreateExecuteRecord(nullptr, TEST_BUNDLE_NAME, "caller", 0, nullptr);
    {
        std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
        auto it = manager_->records_.find(requestCode);
        ASSERT_NE(it, manager_->records_.end());
        it->second->state = SkillExecuteState::EXECUTE_DONE;
    }

    AppExecFwk::SkillExecuteResult result;
    auto ret = manager_->ExecuteSkillDone(requestCode, 0, result, TEST_BUNDLE_NAME);

    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    std::lock_guard<ffrt::mutex> guard2(manager_->mutex_);
    EXPECT_EQ(manager_->records_.count(requestCode), 1u);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0300 end");
}

/**
 * @tc.name: ExecuteSkillDone_0400
 * @tc.desc: Test ExecuteSkillDone returns ERR_OK and removes record when state is EXECUTING and callback null
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0400 begin");

    auto requestCode = manager_->CreateExecuteRecord(nullptr, TEST_BUNDLE_NAME, "caller", 0, nullptr);
    AppExecFwk::SkillExecuteResult result;
    auto ret = manager_->ExecuteSkillDone(requestCode, 0, result, TEST_BUNDLE_NAME);

    EXPECT_EQ(ret, ERR_OK);
    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_EQ(manager_->records_.count(requestCode), 0u);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0400 end");
}

/**
 * @tc.name: OnTimeout_0200
 * @tc.desc: Test OnTimeout returns silently when seq exists but record is gone
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnTimeout_0200 begin");

    {
        std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
        manager_->seqToRequestCodeMap_[88888] = "ghost_code";
    }
    manager_->OnTimeout(88888);

    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_EQ(manager_->seqToRequestCodeMap_.count(88888), 0u);
    EXPECT_TRUE(manager_->records_.empty());

    TAG_LOGI(AAFwkTag::TEST, "OnTimeout_0200 end");
}

/**
 * @tc.name: OnTimeout_0300
 * @tc.desc: Test OnTimeout keeps record when state is not EXECUTING
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTimeout_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnTimeout_0300 begin");

    auto requestCode = manager_->CreateExecuteRecord(nullptr, TEST_BUNDLE_NAME, "caller", 0, nullptr);
    uint64_t seq = 0;
    {
        std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
        auto it = manager_->records_.find(requestCode);
        ASSERT_NE(it, manager_->records_.end());
        it->second->state = SkillExecuteState::EXECUTE_DONE;
        seq = it->second->requestCodeSeq;
    }
    manager_->OnTimeout(static_cast<int64_t>(seq));

    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_EQ(manager_->records_.count(requestCode), 1u);

    TAG_LOGI(AAFwkTag::TEST, "OnTimeout_0300 end");
}

/**
 * @tc.name: OnLaunchCompleted_0200
 * @tc.desc: Test OnLaunchCompleted updates targetPid and keeps record for async result
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnLaunchCompleted_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnLaunchCompleted_0200 begin");

    auto requestCode = manager_->CreateExecuteRecord(nullptr, TEST_BUNDLE_NAME, "caller", 0, nullptr);
    manager_->OnLaunchCompleted(requestCode);

    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    auto it = manager_->records_.find(requestCode);
    ASSERT_NE(it, manager_->records_.end());
    EXPECT_EQ(it->second->state, SkillExecuteState::EXECUTING);

    TAG_LOGI(AAFwkTag::TEST, "OnLaunchCompleted_0200 end");
}

/**
 * @tc.name: OnLaunchFailed_0200
 * @tc.desc: Test OnLaunchFailed removes record without callback when state is not EXECUTING
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnLaunchFailed_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnLaunchFailed_0200 begin");

    auto requestCode = manager_->CreateExecuteRecord(nullptr, TEST_BUNDLE_NAME, "caller", 0, nullptr);
    {
        std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
        auto it = manager_->records_.find(requestCode);
        ASSERT_NE(it, manager_->records_.end());
        it->second->state = SkillExecuteState::EXECUTE_DONE;
    }
    manager_->OnLaunchFailed(requestCode, ERR_INVALID_VALUE);

    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_EQ(manager_->records_.count(requestCode), 0u);

    TAG_LOGI(AAFwkTag::TEST, "OnLaunchFailed_0200 end");
}

/**
 * @tc.name: OnLaunchFailed_0300
 * @tc.desc: Test OnLaunchFailed removes record and skips callback when callback is null
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnLaunchFailed_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnLaunchFailed_0300 begin");

    auto requestCode = manager_->CreateExecuteRecord(nullptr, TEST_BUNDLE_NAME, "caller", 0, nullptr);
    manager_->OnLaunchFailed(requestCode, ERR_INVALID_VALUE);

    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_EQ(manager_->records_.count(requestCode), 0u);

    TAG_LOGI(AAFwkTag::TEST, "OnLaunchFailed_0300 end");
}

/**
 * @tc.name: OnTargetProcessDied_0200
 * @tc.desc: Test OnTargetProcessDied removes hit record when bundle and pid match
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnTargetProcessDied_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnTargetProcessDied_0200 begin");

    auto requestCode = manager_->CreateExecuteRecord(nullptr, TEST_BUNDLE_NAME, "caller", 0, nullptr);
    {
        std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
        auto it = manager_->records_.find(requestCode);
        ASSERT_NE(it, manager_->records_.end());
        it->second->targetPid = 54321;
    }
    manager_->OnTargetProcessDied(TEST_BUNDLE_NAME, 54321);

    std::lock_guard<ffrt::mutex> guard(manager_->mutex_);
    EXPECT_EQ(manager_->records_.count(requestCode), 0u);

    TAG_LOGI(AAFwkTag::TEST, "OnTargetProcessDied_0200 end");
}
} // namespace AAFwk
} // namespace OHOS
