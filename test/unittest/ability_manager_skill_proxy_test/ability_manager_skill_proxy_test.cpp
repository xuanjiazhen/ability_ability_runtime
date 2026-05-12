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

// NOLINTNEXTLINE(g.nam.03-cpp): Redefining C++ keyword 'private' for unit testing
#define private public
#include "ability_manager_proxy.h"
#undef private

#include "ability_manager_stub_mock.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "string_wrapper.h"
#include "want_params.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
namespace {

const std::string BUNDLE_NAME = "com.example.test";
const std::string MODULE_NAME = "entry";
const std::string SKILL_NAME = "testSkill";
const std::string SCRIPT_PATH = "ets/skill/TestSkill";
const std::string FUNC_NAME = "handleSkill";
const std::string REQUEST_CODE = "req_123";

}  // namespace

class AbilityManagerSkillProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerProxy> proxy_ { nullptr };
    sptr<AbilityManagerStubMock> mock_ { nullptr };
};

void AbilityManagerSkillProxyTest::SetUpTestCase(void) {}
void AbilityManagerSkillProxyTest::TearDownTestCase(void) {}
void AbilityManagerSkillProxyTest::TearDown() {}

void AbilityManagerSkillProxyTest::SetUp()
{
    mock_ = new AbilityManagerStubMock();
    proxy_ = std::make_shared<AbilityManagerProxy>(mock_);
}

/**
 * @tc.name: AbilityManagerSkillProxy_ExecuteInAppSkillWithTokenId_0100
 * @tc.desc: Test ExecuteInAppSkillWithTokenId proxy sends correct IPC code
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillProxyTest,
    AbilityManagerSkillProxy_ExecuteInAppSkillWithTokenId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenId_0100 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    AppExecFwk::SkillExecuteRequest request;
    request.callerTokenId = 100;
    request.bundleName = BUNDLE_NAME;
    request.moduleName = MODULE_NAME;
    request.skillName = SKILL_NAME;
    request.scriptPath = SCRIPT_PATH;
    request.functionName = FUNC_NAME;
    request.skillArgs = std::make_shared<AAFwk::WantParams>();

    int32_t res = proxy_->ExecuteInAppSkillWithTokenId(request, nullptr);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(mock_->code_,
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL_WITH_TOKEN_ID));

    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenId_0100 end");
}

/**
 * @tc.name: AbilityManagerSkillProxy_ExecuteInAppSkillWithTokenId_0200
 * @tc.desc: Test ExecuteInAppSkillWithTokenId proxy with SendRequest error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillProxyTest,
    AbilityManagerSkillProxy_ExecuteInAppSkillWithTokenId_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenId_0200 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));

    AppExecFwk::SkillExecuteRequest request;
    request.callerTokenId = 100;
    request.bundleName = BUNDLE_NAME;
    request.skillName = SKILL_NAME;
    request.skillArgs = nullptr;

    int32_t res = proxy_->ExecuteInAppSkillWithTokenId(request, nullptr);
    EXPECT_NE(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenId_0200 end");
}

/**
 * @tc.name: AbilityManagerSkillProxy_ExecuteSkillDone_0100
 * @tc.desc: Test ExecuteSkillDone proxy sends correct IPC code
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillProxyTest,
    AbilityManagerSkillProxy_ExecuteSkillDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0100 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    auto token = new MockAbilityToken();
    AppExecFwk::SkillExecuteResult result;
    result.code = 0;
    result.result = std::make_shared<AAFwk::WantParams>();

    int32_t res = proxy_->ExecuteSkillDone(token->AsObject(), REQUEST_CODE, 0, result);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(mock_->code_,
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_SKILL_DONE_WITH_TOKEN));

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0100 end");
}

/**
 * @tc.name: AbilityManagerSkillProxy_QuerySkillType_0100
 * @tc.desc: Test QuerySkillType proxy sends correct IPC code
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillProxyTest,
    AbilityManagerSkillProxy_QuerySkillType_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QuerySkillType_0100 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    int32_t skillType = 0;
    int32_t res = proxy_->QuerySkillType(BUNDLE_NAME, MODULE_NAME, SKILL_NAME, skillType);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(mock_->code_,
        static_cast<uint32_t>(AbilityManagerInterfaceCode::QUERY_SKILL_TYPE));

    TAG_LOGI(AAFwkTag::TEST, "QuerySkillType_0100 end");
}

/**
 * @tc.name: AbilityManagerSkillProxy_ExecuteInAppSkillWithTokenId_0300
 * @tc.desc: Test ExecuteInAppSkillWithTokenId with null skillArgs
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillProxyTest,
    AbilityManagerSkillProxy_ExecuteInAppSkillWithTokenId_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenId_0300 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    AppExecFwk::SkillExecuteRequest request;
    request.callerTokenId = 100;
    request.bundleName = BUNDLE_NAME;
    request.skillName = SKILL_NAME;
    request.skillArgs = nullptr; // null skillArgs

    int32_t res = proxy_->ExecuteInAppSkillWithTokenId(request, nullptr);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenId_0300 end");
}

/**
 * @tc.name: AbilityManagerSkillProxy_ExecuteInAppSkillWithTokenId_0400
 * @tc.desc: Test ExecuteInAppSkillWithTokenId with skillArgs containing params
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillProxyTest,
    AbilityManagerSkillProxy_ExecuteInAppSkillWithTokenId_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenId_0400 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("arg1", AAFwk::String::Box("value1"));
    skillArgs->SetParam("arg2", AAFwk::Integer::Box(42));

    AppExecFwk::SkillExecuteRequest request;
    request.callerTokenId = 200;
    request.bundleName = BUNDLE_NAME;
    request.moduleName = MODULE_NAME;
    request.skillName = SKILL_NAME;
    request.scriptPath = SCRIPT_PATH;
    request.functionName = FUNC_NAME;
    request.skillArgs = skillArgs;

    int32_t res = proxy_->ExecuteInAppSkillWithTokenId(request, nullptr);
    EXPECT_EQ(res, NO_ERROR);
    EXPECT_EQ(mock_->code_,
        static_cast<uint32_t>(AbilityManagerInterfaceCode::EXECUTE_IN_APP_SKILL_WITH_TOKEN_ID));

    TAG_LOGI(AAFwkTag::TEST, "ExecuteInAppSkillWithTokenId_0400 end");
}

/**
 * @tc.name: AbilityManagerSkillProxy_ExecuteSkillDone_0200
 * @tc.desc: Test ExecuteSkillDone with error result
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillProxyTest,
    AbilityManagerSkillProxy_ExecuteSkillDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0200 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeSendRequest));

    auto token = new MockAbilityToken();
    AppExecFwk::SkillExecuteResult result;
    result.code = -1;
    result.result = nullptr;

    int32_t res = proxy_->ExecuteSkillDone(token->AsObject(), REQUEST_CODE, -1, result);
    EXPECT_EQ(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteSkillDone_0200 end");
}

/**
 * @tc.name: AbilityManagerSkillProxy_QuerySkillType_0200
 * @tc.desc: Test QuerySkillType with SendRequest error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerSkillProxyTest,
    AbilityManagerSkillProxy_QuerySkillType_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "QuerySkillType_0200 start");

    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &AbilityManagerStubMock::InvokeErrorSendRequest));

    int32_t skillType = 0;
    int32_t res = proxy_->QuerySkillType(BUNDLE_NAME, MODULE_NAME, SKILL_NAME, skillType);
    EXPECT_NE(res, NO_ERROR);

    TAG_LOGI(AAFwkTag::TEST, "QuerySkillType_0200 end");
}

}  // namespace AAFwk
}  // namespace OHOS
