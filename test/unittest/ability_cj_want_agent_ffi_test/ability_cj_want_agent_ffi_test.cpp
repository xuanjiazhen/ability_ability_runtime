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

#include "cj_want_agent_ffi.h"
#include "want_agent_helper.h"
#include "want_agent_info.h"
#include "trigger_info.h"
#include "want_agent.h"
#include "pending_want.h"
#include "want_sender_interface.h"
#include "want_agent_client.h"
#include "message_parcel.h"
#include "cj_want_agent_utils.h"
#include "want_sender_info.h"
#include "errors.h"
#include "want.h"
#include "ffi_remote_data.h"
#include "context_impl.h"
#include "cj_ability_runtime_error.h"
#include "cj_common_ffi.h"

#include <memory>

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS{
namespace AbilityRuntime{
class CjWantAgentFfiTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void testcall();
};
void CjWantAgentFfiTest::SetUpTestCase(){}
void CjWantAgentFfiTest::TearDownTestCase(){}
void CjWantAgentFfiTest::SetUp(){}
void CjWantAgentFfiTest::TearDown(){}
void CjWantAgentFfiTest::testcall(){}

/**
 * @tc.name: CjWantAgentFfiTestOnGetBundleName_0010
 * @tc.desc: CjWantAgentFfiTest test for OnGetBundleName.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestOnGetBundleName_0010, TestSize.Level1)
{
    auto pendingWant = std::make_shared<OHOS::AbilityRuntime::WantAgent::PendingWant>();
    std::shared_ptr<WantAgent::WantAgent> wantAgent = std::make_shared<WantAgent::WantAgent>(pendingWant);
    auto cjWantAgent = std::make_shared<FfiWantAgent::CJWantAgent>(wantAgent);
    int32_t err = -1;
    int32_t *errCode_ = &err;
    std::string bundleName = cjWantAgent->OnGetBundleName(errCode_);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    EXPECT_EQ(bundleName, "");
}

/**
 * @tc.name: CjWantAgentFfiTestOnGetUid_0010
 * @tc.desc: CjWantAgentFfiTest test for OnGetUid.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestOnGetUid_0010, TestSize.Level1)
{
    auto pendingWant = std::make_shared<OHOS::AbilityRuntime::WantAgent::PendingWant>();
    std::shared_ptr<WantAgent::WantAgent> wantAgent = std::make_shared<WantAgent::WantAgent>(pendingWant);
    auto cjWantAgent = std::make_shared<FfiWantAgent::CJWantAgent>(wantAgent);
    int err = -2;
    int* errCode_ = &err;
    int32_t uid_ = cjWantAgent->OnGetUid(errCode_);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_WANTAGENT);
    EXPECT_EQ(uid_, -1);
}

/**
 * @tc.name: CjWantAgentFfiTestOnCancel_0010
 * @tc.desc: CjWantAgentFfiTest test for OnCancel.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestOnCancel_0010, TestSize.Level1)
{
    auto pendingWant = std::make_shared<OHOS::AbilityRuntime::WantAgent::PendingWant>();
    std::shared_ptr<WantAgent::WantAgent> wantAgent = std::make_shared<WantAgent::WantAgent>(pendingWant);
    auto cjWantAgent = std::make_shared<FfiWantAgent::CJWantAgent>(wantAgent);
    int32_t err = -1;
    int32_t *errCode_ = &err;
    cjWantAgent->OnCancel(errCode_);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CjWantAgentFfiTestOnTrigger_0010
 * @tc.desc: CjWantAgentFfiTest test for OnTrigger.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestOnTrigger_0010, TestSize.Level1)
{
    FfiWantAgent::CJTriggerInfo cjtriggerInfo{
        .hasWant = false,
        .extraInfos = strdup("extroinfo"),
        .permission = strdup("Permission"),
        .code = 1
    };
    std::function<void(FfiWantAgent::CJCompleteData)> callback_ =
        [](FfiWantAgent::CJCompleteData) {};
    auto pendingWant = std::make_shared<OHOS::AbilityRuntime::WantAgent::PendingWant>();
    std::shared_ptr<WantAgent::WantAgent> wantAgent = std::make_shared<WantAgent::WantAgent>(pendingWant);
    auto cjWantAgent = std::make_shared<FfiWantAgent::CJWantAgent>(wantAgent);
    int32_t err = -1;
    int32_t *errCode_ = &err;
    cjWantAgent->OnTrigger(cjtriggerInfo, callback_, errCode_);
    EXPECT_EQ(*errCode_, NO_ERROR);
    free(cjtriggerInfo.extraInfos);
    free(cjtriggerInfo.permission);
}

/**
 * @tc.name: CjWantAgentFfiTestOnGetOperationType_0010
 * @tc.desc: CjWantAgentFfiTest test for OnGetOperationType.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestOnGetOperationType_0010, TestSize.Level1)
{
    auto pendingWant = std::make_shared<OHOS::AbilityRuntime::WantAgent::PendingWant>();
    std::shared_ptr<WantAgent::WantAgent> wantAgent = std::make_shared<WantAgent::WantAgent>(pendingWant);
    auto cjWantAgent = std::make_shared<FfiWantAgent::CJWantAgent>(wantAgent);
    int32_t err = -1;
    int32_t *errCode_ = &err;
    int oprationType = 0;
    *errCode_ = cjWantAgent->OnGetOperationType(errCode_);
    EXPECT_EQ(*errCode_, NO_ERROR);
    EXPECT_NE(oprationType, -1);
}

/**
 * @tc.name: CjWantAgentFfiTestOnEqual_0010
 * @tc.desc: CjWantAgentFfiTest test for OnEqual.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestOnEqual_0010, TestSize.Level1)
{
    int32_t err =-1;
    int32_t *errCode_ = &err;
    auto second_ = std::make_shared<WantAgent::WantAgent>();
    auto pendingWant_ = std::make_shared<OHOS::AbilityRuntime::WantAgent::PendingWant>();
    auto wantAgent_ = std::make_shared<WantAgent::WantAgent>(pendingWant_);
    auto cjWantAgent = std::make_shared<FfiWantAgent::CJWantAgent>(wantAgent_);
    bool result_ = cjWantAgent->OnEqual(second_, errCode_);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    EXPECT_EQ(result_, false);
}

/**
 * @tc.name: CjWantAgentFfiTestOnSendFinished_0010
 * @tc.desc: CjWantAgentFfiTest test for OnSendFinished.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestOnSendFinished_0010, TestSize.Level1)
{
    auto cjtriggerCompletecb = std::make_shared<FfiWantAgent::CJTriggerCompleteCallBack>();
    AAFwk::Want want_;
    int resultCode_ = 1;
    std::string resultData_ = "resultData";
    AAFwk::WantParams resultExtras_;
    cjtriggerCompletecb->SetWantAgentInstance(1);
    FfiWantAgent::CJCompleteData mydata_;
    auto cb_ = [&mydata_](const FfiWantAgent::CJCompleteData &data) -> void {
        mydata_.finalCode = data.finalCode;
    };
    cjtriggerCompletecb->SetCallbackInfo(cb_);
    cjtriggerCompletecb->OnSendFinished(want_, resultCode_, resultData_, resultExtras_);
    EXPECT_EQ(mydata_.finalCode, resultCode_);
}

/**
 * @tc.name: CjWantAgentFfiTestFfiWantAgentGetWantAgent_0010
 * @tc.desc: CjWantAgentFfiTest test for FfiWantAgentGetWantAgent.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestFfiWantAgentGetWantAgent_0010, TestSize.Level1)
{
    FfiWantAgent::CJWantAgentInfo cjWantAgentInfo{
        .actionFlags = {.head = nullptr, .size = 0},
        .actionType = 1,
        .extraInfos = strdup("extroinfo"),
        .requestCode = 1,
        .wants = {.head = nullptr,.size = 0}
    };
    int32_t err = -1;
    int32_t *errCode_ = &err;
    int64_t ret_ = FfiWantAgent::FfiWantAgentGetWantAgent(cjWantAgentInfo, errCode_);
    EXPECT_EQ(ret_, -1);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    free(cjWantAgentInfo.extraInfos);
}

/**
 * @tc.name: CjWantAgentFfiTestFfiWantAgentGetBoundleName_0010
 * @tc.desc: CjWantAgentFfiTest test for FfiWantAgentGetBoundleName.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestFfiWantAgentGetBoundleName_0010, TestSize.Level1)
{
    int64_t cjWantAgent_ = -1;
    int32_t err = 0;
    int32_t *errCode_ = &err;
    char* name_ = FfiWantAgent::FfiWantAgentGetBoundleName(cjWantAgent_, errCode_);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    EXPECT_EQ(name_, NULL);
}

/**
 * @tc.name: CjWantAgentFfiTestFfiWantAgentGetUid
 * @tc.desc: CjWantAgentFfiTest test for FfiWantAgentGetUid.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestFfiWantAgentGetUid_0010, TestSize.Level1)
{
    int64_t cjWantAgent_ = -1;
    int32_t err = 0;
    int32_t *errCode_ = &err;
    int32_t ret_ = FfiWantAgent::FfiWantAgentGetUid(cjWantAgent_, errCode_);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    EXPECT_EQ(ret_, -1);
}

/**
 * @tc.name: CjWantAgentFfiTestFfiWantAgentCancel_0010
 * @tc.desc: CjWantAgentFfiTest test for FfiWantAgentCancel.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestFfiWantAgentCancel_0010, TestSize.Level1)
{
    int64_t cjWantAgent_ = -1;
    int32_t err = 0;
    int32_t *errCode_ = &err;
    FfiWantAgent::FfiWantAgentCancel(cjWantAgent_, errCode_);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CjWantAgentFfiTestFfiWantAgentTrigger_0010
 * @tc.desc: CjWantAgentFfiTest test for FfiWantAgentTrigger.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestFfiWantAgentTrigger_0010, TestSize.Level1)
{
    int64_t cjWantAgent_ = -1;
    int32_t err = -1;
    int32_t *errCode_ = &err;
    void (*callback_)(FfiWantAgent::CJCompleteData) = [](FfiWantAgent::CJCompleteData data){};
    FfiWantAgent::CJTriggerInfo triggerInfo{
        .code = 0,
        .extraInfos = nullptr,
        .hasWant = false,
        .permission = nullptr,
        .want = nullptr
    };
    FfiWantAgent::FfiWantAgentTrigger(cjWantAgent_, triggerInfo, callback_, errCode_);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CjWantAgentFfiTestFfiWantAgentOnGetOperationType_0010
 * @tc.desc: CjWantAgentFfiTest test for FfiWantAgentOnGetOperationType_0010.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestFfiWantAgentOnGetOperationType_0010, TestSize.Level1)
{
    int64_t cjWantAgent = -1;
    int32_t err = -1;
    int32_t *errCode_ = &err;
    int32_t ret_ = FfiWantAgent::FfiWantAgentGetOperationType(cjWantAgent, errCode_);
    EXPECT_EQ(ret_,-1);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CjWantAgentFfiTestFfiWantAgentGetOperationType_0010
 * @tc.desc: CjWantAgentFfiTest test for FfiWantAgentGetOperationType.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestFfiWantAgentGetOperationType_0010, TestSize.Level1)
{
    ino64_t cjWantAgent = -1;
    int32_t err = -1;
    int32_t *errCode_ = &err;
    int32_t ret_ = FfiWantAgent::FfiWantAgentGetOperationType(cjWantAgent, errCode_);
    EXPECT_EQ(ret_,-1);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CjWantAgentFfiTestFfiWantAgentEqual_0010
 * @tc.desc: CjWantAgentFfiTest test for FfiWantAgentEqual.
 * @tc.type: FUNC
 */
HWTEST_F(CjWantAgentFfiTest, CjWantAgentFfiTestFfiWantAgentEqual_0010, TestSize.Level1)
{
    int64_t cjWantAgent = -1;
    int64_t cjWantAgent2 = -1;
    int32_t err = -1;
    int32_t *errCode_ = &err;
    bool ret_ = FfiWantAgent::FfiWantAgentEqual(cjWantAgent, cjWantAgent2, errCode_);
    EXPECT_EQ(ret_, false);
    EXPECT_EQ(*errCode_, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}
}// namespace AbilityRuntime
}// namespace OHOS