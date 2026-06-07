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

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "skill/skill_execute_callback_stub.h"
#include "skill_execute_manager.h"
#include "skill_execute_record.h"
#include "string_wrapper.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TEST_BUNDLE = "com.test.bundle";
const std::string TEST_MODULE = "entry";
const std::string TEST_SKILL = "PlayMusic";
const std::string TEST_ABILITY = "MainAbility";
const std::string TEST_CALLER_BUNDLE = "com.test.caller";
const std::string TEST_SCRIPT = "./ets/entry/PlayMusic.ts";
const std::string TEST_FUNC = "executePlay";

class MockCallback : public SkillExecuteCallbackStub {
public:
    MOCK_METHOD3(OnExecuteDone,
        void(const std::string &requestCode, int32_t resultCode,
            const AppExecFwk::SkillExecuteResult &result));
};
} // namespace

class SkillExecuteManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SkillExecuteManagerTest::SetUpTestCase()
{}

void SkillExecuteManagerTest::TearDownTestCase()
{}

void SkillExecuteManagerTest::SetUp()
{}

void SkillExecuteManagerTest::TearDown()
{}

// ---- CancelExecuteRecord tests ----

/**
 * @tc.name: CancelExecuteRecord_0100
 * @tc.desc: Test CancelExecuteRecord with existing record removes it.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CancelExecuteRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    auto callback = sptr<MockCallback>(new MockCallback());
    ASSERT_NE(callback, nullptr);

    auto requestCode = mgr->CreateExecuteRecord(
        nullptr, TEST_BUNDLE, TEST_CALLER_BUNDLE, 100, callback);
    EXPECT_FALSE(requestCode.empty());

    mgr->CancelExecuteRecord(requestCode);
    AppExecFwk::SkillExecuteResult result;
    auto ret = mgr->ExecuteSkillDone(requestCode, 0, result, TEST_BUNDLE);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CancelExecuteRecord_0200
 * @tc.desc: Test CancelExecuteRecord with non-existent requestCode does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CancelExecuteRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    mgr->CancelExecuteRecord("non_existent_code");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

// ---- GenerateSkillWant tests (new signature) ----

/**
 * @tc.name: GenerateSkillWant_0100
 * @tc.desc: Test GenerateSkillWant with valid skill info generates correct Want.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, GenerateSkillWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::SkillInfo skillInfo;
    skillInfo.bundleName = TEST_BUNDLE;
    skillInfo.moduleName = TEST_MODULE;
    skillInfo.skillName = TEST_SKILL;
    skillInfo.abilityName = TEST_ABILITY;

    Want want;
    auto ret = DelayedSingleton<SkillExecuteManager>::GetInstance()->GenerateSkillWant(
        skillInfo, want, "req_001", TEST_SCRIPT, TEST_FUNC);
    EXPECT_EQ(ret, ERR_OK);

    auto element = want.GetElement();
    EXPECT_EQ(element.GetBundleName(), TEST_BUNDLE);
    EXPECT_EQ(element.GetAbilityName(), TEST_ABILITY);
    EXPECT_EQ(element.GetModuleName(), TEST_MODULE);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateSkillWant_0200
 * @tc.desc: Test GenerateSkillWant with empty abilityName sets empty element.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, GenerateSkillWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::SkillInfo skillInfo;
    skillInfo.bundleName = TEST_BUNDLE;
    skillInfo.moduleName = TEST_MODULE;
    skillInfo.skillName = TEST_SKILL;

    Want want;
    auto ret = DelayedSingleton<SkillExecuteManager>::GetInstance()->GenerateSkillWant(
        skillInfo, want, "req_002");
    EXPECT_EQ(ret, ERR_OK);

    auto element = want.GetElement();
    EXPECT_EQ(element.GetAbilityName(), "");
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateSkillWant_0300
 * @tc.desc: Test GenerateSkillWant with skill args.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, GenerateSkillWant_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::SkillInfo skillInfo;
    skillInfo.bundleName = TEST_BUNDLE;
    skillInfo.moduleName = TEST_MODULE;
    skillInfo.skillName = TEST_SKILL;
    skillInfo.abilityName = TEST_ABILITY;

    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("key1", AAFwk::String::Box("value1"));

    Want want;
    auto ret = DelayedSingleton<SkillExecuteManager>::GetInstance()->GenerateSkillWant(
        skillInfo, want, "req_003", TEST_SCRIPT, TEST_FUNC, skillArgs);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(want.HasParameter("ohos.skill.executeParam.skillName"));
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

// ---- CreateExecuteRecord tests ----

/**
 * @tc.name: CreateExecuteRecord_0100
 * @tc.desc: Test CreateExecuteRecord generates unique requestCodes.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    auto code1 = mgr->CreateExecuteRecord(
        nullptr, TEST_BUNDLE, TEST_CALLER_BUNDLE, 100);
    auto code2 = mgr->CreateExecuteRecord(
        nullptr, TEST_BUNDLE, TEST_CALLER_BUNDLE, 100);
    EXPECT_FALSE(code1.empty());
    EXPECT_FALSE(code2.empty());
    EXPECT_NE(code1, code2);
    mgr->CancelExecuteRecord(code1);
    mgr->CancelExecuteRecord(code2);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CreateExecuteRecord_0200
 * @tc.desc: Test CreateExecuteRecord with externalRequestCode uses it.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    std::string externalCode = "ext_req_001";
    auto code = mgr->CreateExecuteRecord(
        nullptr, TEST_BUNDLE, TEST_CALLER_BUNDLE, 100, nullptr, externalCode);
    EXPECT_EQ(code, externalCode);
    mgr->CancelExecuteRecord(code);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: CreateExecuteRecord_0300
 * @tc.desc: Test CreateExecuteRecord with callback sets callbackDeathRecipient.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, CreateExecuteRecord_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    auto callback = sptr<MockCallback>(new MockCallback());
    ASSERT_NE(callback, nullptr);

    auto code = mgr->CreateExecuteRecord(
        nullptr, TEST_BUNDLE, TEST_CALLER_BUNDLE, 100, callback);
    EXPECT_FALSE(code.empty());
    mgr->CancelExecuteRecord(code);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

// ---- ExecuteSkillDone tests ----

/**
 * @tc.name: ExecuteSkillDone_0100
 * @tc.desc: Test ExecuteSkillDone with non-existent requestCode returns error.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::SkillExecuteResult result;
    auto ret = DelayedSingleton<SkillExecuteManager>::GetInstance()->ExecuteSkillDone(
        "non_existent", 0, result, TEST_BUNDLE);
    EXPECT_EQ(ret, ERR_CODE_INVALID_ID);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteSkillDone_0200
 * @tc.desc: Test ExecuteSkillDone with mismatched bundleName returns error.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    auto code = mgr->CreateExecuteRecord(
        nullptr, TEST_BUNDLE, TEST_CALLER_BUNDLE, 100);

    AppExecFwk::SkillExecuteResult result;
    auto ret = mgr->ExecuteSkillDone(code, 0, result, "wrong.bundle");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    mgr->CancelExecuteRecord(code);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: ExecuteSkillDone_0300
 * @tc.desc: Test ExecuteSkillDone with matching bundleName invokes callback.
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, ExecuteSkillDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    auto mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    auto callback = sptr<MockCallback>(new MockCallback());
    ASSERT_NE(callback, nullptr);

    auto code = mgr->CreateExecuteRecord(
        nullptr, TEST_BUNDLE, TEST_CALLER_BUNDLE, 100, callback);

    AppExecFwk::SkillExecuteResult result;
    result.code = 0;
    EXPECT_CALL(*callback, OnExecuteDone(code, 0, _)).Times(1);
    auto ret = mgr->ExecuteSkillDone(code, 0, result, TEST_BUNDLE);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

} // namespace AAFwk
} // namespace OHOS
