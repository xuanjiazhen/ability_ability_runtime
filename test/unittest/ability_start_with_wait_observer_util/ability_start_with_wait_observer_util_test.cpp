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

#define private public
#include "ability_start_with_wait_observer_utils.h"
#undef private
 
#include "ability_start_with_wait_observer_proxy.h"
#include "ability_start_with_wait_observer_stub.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;
 
namespace OHOS {
namespace AAFwk {

class MockAbilityStartWithWaitObserver : public AbilityStartWithWaitObserverStub {
public:
    int32_t StartAbilityWithWait(const AbilityStartWithWaitObserverData &abilityStartWithWaitData) override
    {
        return 0;
    }
};
class AbilityStartWithWaitObserverUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityStartWithWaitObserverUtilsTest::SetUpTestCase(void)
{}

void AbilityStartWithWaitObserverUtilsTest::TearDownTestCase(void)
{}

void AbilityStartWithWaitObserverUtilsTest::SetUp()
{}

void AbilityStartWithWaitObserverUtilsTest::TearDown()
{}

/**
 * @tc.name: AbilityStartWithWaitObserverUtilsTest_0100
 * @tc.desc: Ability stage basic func test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStartWithWaitObserverUtilsTest, AbilityStartWithWaitObserverUtilsTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    sptr<IAbilityStartWithWaitObserver> observer;
    AbilityStartWithWaitObserverUtils::GetInstance().Regist(want, observer);
    EXPECT_EQ(want.GetIntParam(AbilityUtil::OBSERVER_TOKEN_KEY, -1), -1);

    sptr<IRemoteObject> observerStub = nullptr;
    auto observer2 = sptr<AbilityStartWithWaitObserverProxy>::MakeSptr(observerStub);
    AbilityStartWithWaitObserverUtils::GetInstance().Regist(want, observer2);
    EXPECT_EQ(want.GetIntParam(AbilityUtil::OBSERVER_TOKEN_KEY, -1), -1);
}

/**
 * @tc.name: AbilityStartWithWaitObserverUtilsTest_0200
 * @tc.desc: Ability stage basic func test.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(AbilityStartWithWaitObserverUtilsTest, AbilityStartWithWaitObserverUtilsTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    Want want;
    sptr<IRemoteObject> observerStub = sptr<MockAbilityStartWithWaitObserver>::MakeSptr();
    sptr<AbilityStartWithWaitObserverProxy> observer = sptr<AbilityStartWithWaitObserverProxy>::MakeSptr(observerStub);
    AbilityStartWithWaitObserverUtils::GetInstance().Regist(want, observer);
    EXPECT_EQ(want.GetIntParam(AbilityUtil::OBSERVER_TOKEN_KEY, -1), 0);
    
    // dulipate regist same observer, failed
    AbilityStartWithWaitObserverUtils::GetInstance().Regist(want, observer);
    EXPECT_EQ(want.GetIntParam(AbilityUtil::OBSERVER_TOKEN_KEY, -1), 0);

    // observerStub = sptr<MockAbilityStartWithWaitObserver>::MakeSptr();
    // observer = sptr<AbilityStartWithWaitObserverProxy>::MakeSptr(observerStub);
    // AbilityStartWithWaitObserverUtils::GetInstance().Regist(want, observer);
    // EXPECT_EQ(want.GetIntParam(AbilityUtil::OBSERVER_TOKEN_KEY, -1), 1);
}
} // namespace AAFwk
} // namespace OHOS