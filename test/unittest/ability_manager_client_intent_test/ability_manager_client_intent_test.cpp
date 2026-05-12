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
#include "ability_manager_client.h"
#include "ability_manager_stub_mock_test.h"
#undef private
#undef protected

#include "hilog_tag_wrapper.h"
#include "insight_intent_host_client.h"
#include "mock_ability_connect_callback.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {

const int32_t TEST_TIMEOUT_MS = 5000;

}  // namespace

class AbilityManagerClientIntentTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityManagerClient> client_ { nullptr };
    sptr<AbilityManagerStubTestMock> mock_ { nullptr };
};

void AbilityManagerClientIntentTest::SetUpTestCase(void) {}
void AbilityManagerClientIntentTest::TearDownTestCase(void) {}
void AbilityManagerClientIntentTest::TearDown() {}

void AbilityManagerClientIntentTest::SetUp()
{
    client_ = std::make_shared<AbilityManagerClient>();
    mock_ = new AbilityManagerStubTestMock();
    client_->proxy_ = mock_;
}

/**
 * @tc.name: ExecuteIntentWithResult_0100
 * @tc.desc: Test ExecuteIntentWithResult when ExecuteIntent fails
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientIntentTest, ExecuteIntentWithResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentWithResult_0100 start");

    InsightIntentExecuteParam param;
    InsightIntentExecuteResult result;

    EXPECT_CALL(*mock_, ExecuteIntent(_, _, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_VALUE));

    ErrCode ret = client_->ExecuteIntentWithResult(param, result, TEST_TIMEOUT_MS);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentWithResult_0100 end");
}

/**
 * @tc.name: ExecuteIntentWithResult_0200
 * @tc.desc: Test ExecuteIntentWithResult success path with callback
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientIntentTest, ExecuteIntentWithResult_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentWithResult_0200 start");

    InsightIntentExecuteParam param;
    InsightIntentExecuteResult result;

    EXPECT_CALL(*mock_, ExecuteIntent(_, _, _))
        .Times(1)
        .WillOnce(Invoke([](uint64_t key, const sptr<IRemoteObject> &callerToken,
            const InsightIntentExecuteParam &param) -> int32_t {
            // Simulate async callback on a separate thread
            auto callback = AbilityRuntime::InsightIntentHostClient::GetInstance()
                ->GetInsightIntentExecute(key);
            if (callback != nullptr) {
                InsightIntentExecuteResult cbResult;
                cbResult.code = 0;
                cbResult.innerErr = 0;
                callback->ProcessInsightIntentExecute(ERR_OK, cbResult);
            }
            AbilityRuntime::InsightIntentHostClient::GetInstance()
                ->RemoveInsightIntentExecute(key);
            return ERR_OK;
        }));

    ErrCode ret = client_->ExecuteIntentWithResult(param, result, TEST_TIMEOUT_MS);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(result.innerErr, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentWithResult_0200 end");
}

/**
 * @tc.name: ExecuteIntentWithResult_0300
 * @tc.desc: Test ExecuteIntentWithResult timeout path
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientIntentTest, ExecuteIntentWithResult_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentWithResult_0300 start");

    InsightIntentExecuteParam param;
    InsightIntentExecuteResult result;

    // ExecuteIntent succeeds but no callback fires → should timeout
    EXPECT_CALL(*mock_, ExecuteIntent(_, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));

    ErrCode ret = client_->ExecuteIntentWithResult(param, result, 100); // 100ms timeout
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentWithResult_0300 end");
}

/**
 * @tc.name: ExecuteIntentWithResult_0400
 * @tc.desc: Test ExecuteIntentWithResult with error callback result
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientIntentTest, ExecuteIntentWithResult_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentWithResult_0400 start");

    InsightIntentExecuteParam param;
    InsightIntentExecuteResult result;

    EXPECT_CALL(*mock_, ExecuteIntent(_, _, _))
        .Times(1)
        .WillOnce(Invoke([](uint64_t key, const sptr<IRemoteObject> &callerToken,
            const InsightIntentExecuteParam &param) -> int32_t {
            auto callback = AbilityRuntime::InsightIntentHostClient::GetInstance()
                ->GetInsightIntentExecute(key);
            if (callback != nullptr) {
                InsightIntentExecuteResult cbResult;
                cbResult.code = -1;
                cbResult.innerErr = 0;
                callback->ProcessInsightIntentExecute(ERR_INVALID_VALUE, cbResult);
            }
            AbilityRuntime::InsightIntentHostClient::GetInstance()
                ->RemoveInsightIntentExecute(key);
            return ERR_OK;
        }));

    ErrCode ret = client_->ExecuteIntentWithResult(param, result, TEST_TIMEOUT_MS);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(result.innerErr, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentWithResult_0400 end");
}

/**
 * @tc.name: ExecuteIntentWithResult_0500
 * @tc.desc: Test ExecuteIntentWithResult with null proxy
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerClientIntentTest, ExecuteIntentWithResult_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentWithResult_0500 start");

    client_->proxy_ = nullptr;

    InsightIntentExecuteParam param;
    InsightIntentExecuteResult result;

    ErrCode ret = client_->ExecuteIntentWithResult(param, result, TEST_TIMEOUT_MS);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentWithResult_0500 end");
}

}  // namespace AAFwk
}  // namespace OHOS
