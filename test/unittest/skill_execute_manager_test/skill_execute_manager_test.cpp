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

#include "hilog_tag_wrapper.h"
#include "skill/skill_execute_manager.h"
#include "skill/skill_execute_callback_interface.h"
#include "skill/skill_execute_result.h"
#include "mock_ability_token.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {

const std::string TARGET_BUNDLE = "com.example.target";
const std::string CALLER_BUNDLE = "com.example.caller";
const std::string EXTERNAL_REQUEST_CODE = "cli_ohos-arkTSScript_123_456";

}  // namespace

class MockSkillExecuteCallback : public SkillExecuteCallbackStub {
public:
    MockSkillExecuteCallback() = default;
    ~MockSkillExecuteCallback() override = default;

    void OnExecuteDone(const std::string &requestCode, int32_t resultCode,
        const AppExecFwk::SkillExecuteResult &result) override
    {
        lastRequestCode_ = requestCode;
        lastResultCode_ = resultCode;
        lastResult_ = result;
        callCount_++;
    }

    std::string lastRequestCode_;
    int32_t lastResultCode_ = -1;
    AppExecFwk::SkillExecuteResult lastResult_;
    int32_t callCount_ = 0;
};

class SkillExecuteManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SkillExecuteManagerTest::SetUpTestCase(void) {}
void SkillExecuteManagerTest::TearDownTestCase(void) {}
void SkillExecuteManagerTest::SetUp() {}
void SkillExecuteManagerTest::TearDown() {}

/**
 * @tc.name: SkillExecuteManager_CreateExecuteRecord_0100
 * @tc.desc: Test CreateExecuteRecord with auto-generated requestCode
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, SkillExecuteManager_CreateExecuteRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateExecuteRecord_0100 start");

    auto &mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    auto callback = sptr<MockSkillExecuteCallback>::MakeSptr();

    auto requestCode = mgr->CreateExecuteRecord(
        nullptr, TARGET_BUNDLE, CALLER_BUNDLE, 100u, callback);
    EXPECT_FALSE(requestCode.empty());

    TAG_LOGI(AAFwkTag::TEST, "CreateExecuteRecord_0100 end");
}

/**
 * @tc.name: SkillExecuteManager_CreateExecuteRecord_0200
 * @tc.desc: Test CreateExecuteRecord with external requestCode
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, SkillExecuteManager_CreateExecuteRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateExecuteRecord_0200 start");

    auto &mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    auto callback = sptr<MockSkillExecuteCallback>::MakeSptr();

    auto requestCode = mgr->CreateExecuteRecord(
        nullptr, TARGET_BUNDLE, CALLER_BUNDLE, 100u, callback, EXTERNAL_REQUEST_CODE);
    EXPECT_EQ(requestCode, EXTERNAL_REQUEST_CODE);

    TAG_LOGI(AAFwkTag::TEST, "CreateExecuteRecord_0200 end");
}

/**
 * @tc.name: SkillExecuteManager_ExecuteSkillDone_0100
 * @tc.desc: Test ExecuteSkillDone with matching bundleName
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, SkillExecuteManager_ExecuteSkillDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0100 start");

    auto &mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    auto callback = sptr<MockSkillExecuteCallback>::MakeSptr();

    auto requestCode = mgr->CreateExecuteRecord(
        nullptr, TARGET_BUNDLE, CALLER_BUNDLE, 100u, callback);

    AppExecFwk::SkillExecuteResult result;
    result.code = 0;
    result.result = std::make_shared<AAFwk::WantParams>();
    result.result->SetParam("answer", AAFwk::String::Box("done"));

    int32_t ret = mgr->ExecuteSkillDone(requestCode, 0, result, TARGET_BUNDLE);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(callback->callCount_, 1);
    EXPECT_EQ(callback->lastRequestCode_, requestCode);
    EXPECT_EQ(callback->lastResultCode_, 0);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0100 end");
}

/**
 * @tc.name: SkillExecuteManager_ExecuteSkillDone_0200
 * @tc.desc: Test ExecuteSkillDone with mismatched bundleName
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, SkillExecuteManager_ExecuteSkillDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0200 start");

    auto &mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    auto callback = sptr<MockSkillExecuteCallback>::MakeSptr();

    auto requestCode = mgr->CreateExecuteRecord(
        nullptr, TARGET_BUNDLE, CALLER_BUNDLE, 100u, callback);

    AppExecFwk::SkillExecuteResult result;
    result.code = 0;

    int32_t ret = mgr->ExecuteSkillDone(requestCode, 0, result, "com.example.wrong");
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_EQ(callback->callCount_, 0);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0200 end");
}

/**
 * @tc.name: SkillExecuteManager_ExecuteSkillDone_0300
 * @tc.desc: Test ExecuteSkillDone with non-existent requestCode
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, SkillExecuteManager_ExecuteSkillDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0300 start");

    auto &mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();

    AppExecFwk::SkillExecuteResult result;
    result.code = 0;

    int32_t ret = mgr->ExecuteSkillDone("non_existent_code", 0, result, TARGET_BUNDLE);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0300 end");
}

/**
 * @tc.name: SkillExecuteManager_ExecuteSkillDone_0400
 * @tc.desc: Test ExecuteSkillDone with uris in result
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, SkillExecuteManager_ExecuteSkillDone_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0400 start");

    auto &mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();
    auto callback = sptr<MockSkillExecuteCallback>::MakeSptr();

    auto requestCode = mgr->CreateExecuteRecord(
        nullptr, TARGET_BUNDLE, CALLER_BUNDLE, 100u, callback);

    AppExecFwk::SkillExecuteResult result;
    result.code = 0;
    result.uris = {"file://test/doc.txt"};
    result.flags = 1;

    int32_t ret = mgr->ExecuteSkillDone(requestCode, 0, result, TARGET_BUNDLE);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(callback->callCount_, 1);
    EXPECT_EQ(callback->lastResult_.uris.size(), 1u);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0400 end");
}

/**
 * @tc.name: SkillExecuteManager_CreateExecuteRecord_0300
 * @tc.desc: Test CreateExecuteRecord with null callback
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteManagerTest, SkillExecuteManager_CreateExecuteRecord_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateExecuteRecord_0300 start");

    auto &mgr = DelayedSingleton<SkillExecuteManager>::GetInstance();

    auto requestCode = mgr->CreateExecuteRecord(
        nullptr, TARGET_BUNDLE, CALLER_BUNDLE, 100u, nullptr);
    EXPECT_FALSE(requestCode.empty());

    // ExecuteSkillDone with null callback should still succeed
    AppExecFwk::SkillExecuteResult result;
    result.code = 0;
    int32_t ret = mgr->ExecuteSkillDone(requestCode, 0, result, TARGET_BUNDLE);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CreateExecuteRecord_0300 end");
}

}  // namespace AAFwk
}  // namespace OHOS
