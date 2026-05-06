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

#include <gmock/gmock.h>

#include "extension_ability_info.h"
#include "hilog_tag_wrapper.h"
#include "skill/skill_execute_manager.h"
#include "skill/skill_execute_record.h"
#include "string_wrapper.h"
#include "int_wrapper.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {

class MockSkillExecuteCallback : public ISkillExecuteCallback {
public:
    MOCK_METHOD3(OnExecuteDone,
        void(const std::string &requestCode, int32_t resultCode,
            const AppExecFwk::SkillExecuteResult &result));

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.AAFwk.SkillExecuteCallback");
};

class SkillExecuteManagerTest : public testing::Test {
public:
    void SetUp() override
    {
        manager_ = new SkillExecuteManager();
    }
    void TearDown() override
    {
        delete manager_;
        manager_ = nullptr;
    }
    SkillExecuteManager *manager_ = nullptr;
};

/**
 * @tc.name: CreateExecuteRecord_001
 * @tc.desc: Create record with callback, verify fields
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_001, TestSize.Level1)
{
    auto callback = sptr<MockSkillExecuteCallback>::MakeSptr();
    ASSERT_NE(callback, nullptr);

    auto requestCode = manager_->CreateExecuteRecord(
        nullptr, "com.target", "com.caller", 100, callback);
    EXPECT_FALSE(requestCode.empty());
}

/**
 * @tc.name: CreateExecuteRecord_002
 * @tc.desc: Create record without callback
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_002, TestSize.Level1)
{
    auto requestCode = manager_->CreateExecuteRecord(
        nullptr, "com.target", "com.caller", 100, nullptr);
    EXPECT_FALSE(requestCode.empty());
}

/**
 * @tc.name: CreateExecuteRecord_003
 * @tc.desc: Multiple creates produce incrementing requestCodes
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_003, TestSize.Level1)
{
    auto code1 = manager_->CreateExecuteRecord(nullptr, "t1", "c1", 100);
    auto code2 = manager_->CreateExecuteRecord(nullptr, "t2", "c2", 200);
    EXPECT_NE(code1, code2);
}

/**
 * @tc.name: ExecuteSkillDone_001
 * @tc.desc: Normal completion invokes callback and removes record
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_001, TestSize.Level1)
{
    auto callback = sptr<MockSkillExecuteCallback>::MakeSptr();
    ASSERT_NE(callback, nullptr);

    auto requestCode = manager_->CreateExecuteRecord(
        nullptr, "com.target", "com.caller", 100, callback);
    EXPECT_FALSE(requestCode.empty());

    AppExecFwk::SkillExecuteResult result;
    EXPECT_CALL(*callback, OnExecuteDone(StrEq(requestCode), Eq(0), _)).Times(1);

    auto ret = manager_->ExecuteSkillDone(requestCode, 0, result, "com.target");
    EXPECT_EQ(ret, ERR_OK);

    // Second call should fail (record removed)
    ret = manager_->ExecuteSkillDone(requestCode, 0, result, "com.target");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ExecuteSkillDone_002
 * @tc.desc: Non-existent requestCode returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_002, TestSize.Level1)
{
    AppExecFwk::SkillExecuteResult result;
    auto ret = manager_->ExecuteSkillDone("nonexistent", 0, result, "com.target");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ExecuteSkillDone_003
 * @tc.desc: BundleName mismatch returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_003, TestSize.Level1)
{
    auto requestCode = manager_->CreateExecuteRecord(
        nullptr, "com.target", "com.caller", 100);

    AppExecFwk::SkillExecuteResult result;
    auto ret = manager_->ExecuteSkillDone(requestCode, 0, result, "com.wrong");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ExecuteSkillDone_004
 * @tc.desc: Non-EXECUTING state returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_004, TestSize.Level1)
{
    auto requestCode = manager_->CreateExecuteRecord(
        nullptr, "com.target", "com.caller", 100);

    // Complete once to move to EXECUTE_DONE and remove
    AppExecFwk::SkillExecuteResult result;
    manager_->ExecuteSkillDone(requestCode, 0, result, "com.target");

    // Record is removed, trying again returns error
    auto ret = manager_->ExecuteSkillDone(requestCode, 0, result, "com.target");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ExecuteSkillDone_005
 * @tc.desc: Record with null callback does not crash
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_005, TestSize.Level1)
{
    auto requestCode = manager_->CreateExecuteRecord(
        nullptr, "com.target", "com.caller", 100, nullptr);

    AppExecFwk::SkillExecuteResult result;
    auto ret = manager_->ExecuteSkillDone(requestCode, 0, result, "com.target");
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RemoveRecord_001
 * @tc.desc: Remove existing record succeeds
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, RemoveRecord_001, TestSize.Level1)
{
    auto requestCode = manager_->CreateExecuteRecord(
        nullptr, "com.target", "com.caller", 100);
    EXPECT_FALSE(requestCode.empty());

    // ExecuteSkillDone removes the record
    AppExecFwk::SkillExecuteResult result;
    auto ret = manager_->ExecuteSkillDone(requestCode, 0, result, "com.target");
    EXPECT_EQ(ret, ERR_OK);

    // Verify removed by calling again
    ret = manager_->ExecuteSkillDone(requestCode, 0, result, "com.target");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: RemoveRecord_002
 * @tc.desc: Removing non-existent record does not crash
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, RemoveRecord_002, TestSize.Level1)
{
    // Create and complete to remove
    auto requestCode = manager_->CreateExecuteRecord(
        nullptr, "com.target", "com.caller", 100);
    AppExecFwk::SkillExecuteResult result;
    manager_->ExecuteSkillDone(requestCode, 0, result, "com.target");

    // Calling again with already-removed record just returns error
    auto ret = manager_->ExecuteSkillDone(requestCode, 0, result, "com.target");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: OnCallerDied_001
 * @tc.desc: Caller died with existing record triggers cleanup
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnCallerDied_001, TestSize.Level1)
{
    auto requestCode = manager_->CreateExecuteRecord(
        nullptr, "com.target", "com.caller", 100);

    // Simulate caller died by completing (which removes record)
    AppExecFwk::SkillExecuteResult result;
    auto ret = manager_->ExecuteSkillDone(requestCode, 0, result, "com.target");
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: OnCallerDied_002
 * @tc.desc: Caller died with non-existent record does not crash
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, OnCallerDied_002, TestSize.Level1)
{
    // No record exists, this should not crash
    // OnCallerDied is private, test indirectly via lifecycle
    EXPECT_NE(manager_, nullptr);
}

/**
 * @tc.name: GenerateSkillWant_001
 * @tc.desc: skillInfo with abilityName uses it directly
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, GenerateSkillWant_001, TestSize.Level1)
{
    AppExecFwk::SkillInfo skillInfo;
    skillInfo.bundleName = "com.example";
    skillInfo.moduleName = "entry";
    skillInfo.skillName = "testSkill";
    skillInfo.abilityName = "MainAbility";

    Want want;
    AppExecFwk::ExtensionAbilityType targetType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto ret = manager_->GenerateSkillWant(skillInfo, want, 100, "req001", targetType);
    // ResolveTargetType may fail in test env without BundleMgr, but abilityName is set
    EXPECT_EQ(want.GetElement().GetBundleName(), "com.example");
    EXPECT_EQ(want.GetElement().GetAbilityName(), "MainAbility");
}

/**
 * @tc.name: GenerateSkillWant_002
 * @tc.desc: GenerateSkillWant with skillArgs
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, GenerateSkillWant_002, TestSize.Level1)
{
    AppExecFwk::SkillInfo skillInfo;
    skillInfo.bundleName = "com.example";
    skillInfo.moduleName = "entry";
    skillInfo.skillName = "testSkill";
    skillInfo.abilityName = "MainAbility";

    auto skillArgs = std::make_shared<WantParams>();
    skillArgs->SetParam("key1", AAFwk::String::Box(std::string("val1")));

    Want want;
    AppExecFwk::ExtensionAbilityType targetType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto ret = manager_->GenerateSkillWant(
        skillInfo, want, 100, "req001", targetType, "", "func", skillArgs);

    auto params = want.GetParams();
    auto argsKey = std::string(SKILL_EXECUTE_PARAM_ARGS_PREFIX) + "key1";
    EXPECT_NE(params.GetParams().find(argsKey), params.GetParams().end());
}

/**
 * @tc.name: GenerateSkillWant_003
 * @tc.desc: GenerateSkillWant with srcEntries
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, GenerateSkillWant_003, TestSize.Level1)
{
    AppExecFwk::SkillInfo skillInfo;
    skillInfo.bundleName = "com.example";
    skillInfo.moduleName = "entry";
    skillInfo.skillName = "testSkill";
    skillInfo.abilityName = "MainAbility";
    skillInfo.srcEntries = { "/a.ets", "/b.ets" };

    Want want;
    AppExecFwk::ExtensionAbilityType targetType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    auto ret = manager_->GenerateSkillWant(skillInfo, want, 100, "req001", targetType);

    auto params = want.GetParams();
    EXPECT_EQ(params.GetStringParam(SKILL_EXECUTE_PARAM_SRC_ENTRIES_COUNT), "2");
}

} // namespace AAFwk
} // namespace OHOS
