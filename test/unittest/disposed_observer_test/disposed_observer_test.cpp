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

#define private public
#define protected public
#include "disposed_observer.h"
#include "disposed_rule_interceptor.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "ability_record.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "mock_native_token.h"
#include "want.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AAFwk {

constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
constexpr const char* INTERCEPT_MISSION_ID = "intercept_missionId";

class MockDisposedRuleInterceptor : public DisposedRuleInterceptor {
public:
    MockDisposedRuleInterceptor() = default;
    ~MockDisposedRuleInterceptor() override = default;

    ErrCode DoProcess(AbilityInterceptorParam param) override
    {
        return ERR_OK;
    }

    MOCK_METHOD1(UnregisterObserver, void(int32_t uid));
};

class DisposedObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

    void ResetMockAbilityRecord()
    {
        Token::abilityRecord = nullptr;
        Token::mockWant = Want();
        Token::mockAbilityInfo = AppExecFwk::AbilityInfo();
        Token::mockSessionInfo = nullptr;
        Token::mockMissionId = 0;
        Token::mockCreateModalResult = ERR_OK;
    }

    void CreateDisposedRule(DisposedRule& disposedRule, ComponentType type = ComponentType::UI_EXTENSION)
    {
        disposedRule.want = std::make_shared<Want>();
        ElementName element("", "com.example.test", "TestExtension");
        disposedRule.want->SetElement(element);
        disposedRule.componentType = type;
    }
};

void DisposedObserverTest::SetUpTestCase(void)
{
    MockNativeToken::SetNativeToken();
}

void DisposedObserverTest::TearDownTestCase(void)
{}

void DisposedObserverTest::SetUp()
{
    ResetMockAbilityRecord();
}

void DisposedObserverTest::TearDown()
{
    ResetMockAbilityRecord();
}

HWTEST_F(DisposedObserverTest, OnPageShow_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0100 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    PageStateData pageStateData;
    pageStateData.uid = 200;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0100 end");
}

HWTEST_F(DisposedObserverTest, OnPageShow_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0200 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule, ComponentType::UI_ABILITY);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0200 end");
}

HWTEST_F(DisposedObserverTest, OnPageShow_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0300 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule, ComponentType::UI_EXTENSION);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0300 end");
}

HWTEST_F(DisposedObserverTest, OnAbilityStateChanged_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0100 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::ABILITY_STATE_BACKGROUND);
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);

    observer->OnAbilityStateChanged(abilityStateData);

    EXPECT_EQ(observer->token_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0100 end");
}

HWTEST_F(DisposedObserverTest, OnAbilityStateChanged_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0200 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    Token::abilityRecord = abilityRecord;
    Token::mockAbilityInfo.isStageBasedModel = true;

    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    ASSERT_NE(mockToken, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);

    observer->OnAbilityStateChanged(abilityStateData);

    EXPECT_EQ(observer->token_, mockToken);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0200 end");
}

HWTEST_F(DisposedObserverTest, OnAbilityStateChanged_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0300 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = nullptr;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);

    observer->OnAbilityStateChanged(abilityStateData);

    EXPECT_EQ(observer->token_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0300 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0100 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);
    EXPECT_EQ(observer->token_, nullptr);

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0100 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0200 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    ASSERT_NE(mockToken, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    EXPECT_EQ(observer->token_, mockToken);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    Token::abilityRecord = abilityRecord;
    Token::mockAbilityInfo.type = AppExecFwk::AbilityType::PAGE;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0200 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0300 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    ASSERT_NE(mockToken, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    Token::abilityRecord = abilityRecord;
    Token::mockWant.SetParam(AAFwk::SCREEN_MODE_KEY, AAFwk::EMBEDDED_FULL_SCREEN_MODE);
    Token::mockCreateModalResult = ERR_OK;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0300 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0400 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    ASSERT_NE(mockToken, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    Token::abilityRecord = abilityRecord;
    Token::mockWant.SetParam(AAFwk::SCREEN_MODE_KEY, AAFwk::EMBEDDED_FULL_SCREEN_MODE);
    Token::mockCreateModalResult = ERR_NO_MEMORY;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0400 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0500 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    ASSERT_NE(mockToken, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    Token::abilityRecord = abilityRecord;
    Token::mockAbilityInfo.type = AppExecFwk::AbilityType::PAGE;
    Token::mockSessionInfo = nullptr;
    Token::mockMissionId = 1001;
    Token::mockCreateModalResult = ERR_OK;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0500 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0600 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    ASSERT_NE(mockToken, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    Token::abilityRecord = abilityRecord;
    Token::mockAbilityInfo.type = AppExecFwk::AbilityType::PAGE;
    auto sessionInfo = sptr<SessionInfo>(new SessionInfo());
    sessionInfo->persistentId = 2001;
    Token::mockSessionInfo = sessionInfo;
    Token::mockCreateModalResult = ERR_OK;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0600 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0700 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    ASSERT_NE(mockToken, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    Token::abilityRecord = abilityRecord;
    Token::mockAbilityInfo.type = AppExecFwk::AbilityType::PAGE;
    Token::mockCreateModalResult = ERR_NO_MEMORY;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0700 end");
}

} // namespace AAFwk
} // namespace OHOS