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
#include "assert_fault_callback_death_mgr.h"
#include "assert_fault_proxy.h"
#include "app_scheduler.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AAFwk {
class AssertFaultCallbackDeathMgrTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AssertFaultCallbackDeathMgrTest::SetUpTestCase(void)
{}
void AssertFaultCallbackDeathMgrTest::TearDownTestCase(void)
{}
void AssertFaultCallbackDeathMgrTest::SetUp()
{}
void AssertFaultCallbackDeathMgrTest::TearDown()
{}

/**
 * @tc.number: AddAssertFaultCallback_0100
 * @tc.name: AddAssertFaultCallback
 * @tc.desc: AddAssert Fault Callback
 */
HWTEST_F(AssertFaultCallbackDeathMgrTest, AddAssertFaultCallback_0100, TestSize.Level1)
{
    auto assertFaultCallbackDeathMgr = new AbilityRuntime::AssertFaultCallbackDeathMgr();
    EXPECT_NE(assertFaultCallbackDeathMgr, nullptr);

    sptr<IRemoteObject> remote;
    AbilityRuntime::AssertFaultCallbackDeathMgr::CallbackTask callback;
    assertFaultCallbackDeathMgr->AddAssertFaultCallback(remote, callback);

    delete assertFaultCallbackDeathMgr;
}

/**
 * @tc.number: RemoveAssertFaultCallback_0100
 * @tc.name: RemoveAssertFaultCallback
 * @tc.desc: Remove Assert Fault Callback
 */
HWTEST_F(AssertFaultCallbackDeathMgrTest, RemoveAssertFaultCallback_0100, TestSize.Level1)
{
    auto assertFaultCallbackDeathMgr = new AbilityRuntime::AssertFaultCallbackDeathMgr();
    EXPECT_NE(assertFaultCallbackDeathMgr, nullptr);

    wptr<IRemoteObject> remote;
    EXPECT_NE(remote, nullptr);
    AbilityRuntime::AssertFaultCallbackDeathMgr::CallbackTask callback;
    assertFaultCallbackDeathMgr->RemoveAssertFaultCallback(remote, false);
    assertFaultCallbackDeathMgr->RemoveAssertFaultCallback(remote, true);

    delete assertFaultCallbackDeathMgr;
}

/**
 * @tc.number: CallAssertFaultCallback_0100
 * @tc.name: CallAssertFaultCallback
 * @tc.desc: Call Assert Fault Callback
 */
HWTEST_F(AssertFaultCallbackDeathMgrTest, CallAssertFaultCallback_0100, TestSize.Level1)
{
    auto assertFaultCallbackDeathMgr = new AbilityRuntime::AssertFaultCallbackDeathMgr();
    EXPECT_NE(assertFaultCallbackDeathMgr, nullptr);

    uint64_t assertFaultSessionId = 0;
    AAFwk::UserStatus status = ASSERT_TERMINATE;
    assertFaultCallbackDeathMgr->CallAssertFaultCallback(assertFaultSessionId, status);

    delete assertFaultCallbackDeathMgr;
}
}  // namespace AAFwk
}  // namespace OHOS
