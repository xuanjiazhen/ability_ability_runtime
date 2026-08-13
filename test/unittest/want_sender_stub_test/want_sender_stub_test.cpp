/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "iremote_proxy.h"
#include "want_sender_stub_impl_mock.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class WantSenderStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void WriteInterfaceToken(MessageParcel& data);
    sptr<WantSenderStubImplMock> stub_{ nullptr };
};

void WantSenderStubTest::SetUpTestCase(void)
{}
void WantSenderStubTest::TearDownTestCase(void)
{}
void WantSenderStubTest::TearDown()
{}

void WantSenderStubTest::SetUp()
{
    stub_ = new WantSenderStubImplMock();
}

void WantSenderStubTest::WriteInterfaceToken(MessageParcel& data)
{
    data.WriteInterfaceToken(WantSenderStubImplMock::GetDescriptor());
}

/*
 * Feature: WantSenderStub
 * Function: OnRemoteRequest
 * SubFunction: NA
 * FunctionPoints: WantSenderStub OnRemoteRequest
 * EnvConditions: The code which not exist
 * CaseDescription: Verify that on remote request is abnormal
 */
HWTEST_F(WantSenderStubTest, WantSenderStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    WriteInterfaceToken(data);
    int res = stub_->OnRemoteRequest(10000, data, reply, option);

    EXPECT_EQ(res, IPC_STUB_UNKNOW_TRANS_ERR);
}
}  // namespace AAFwk
}  // namespace OHOS
