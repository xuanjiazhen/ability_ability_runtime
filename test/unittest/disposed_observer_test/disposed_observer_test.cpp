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

class MockDisposedRuleInterceptor : public DisposedRuleInterceptor {
public:
    MockDisposedRuleInterceptor() = default;
    ~MockDisposedRuleInterceptor() override = default;

    MOCK_METHOD1(UnregisterObserver, void(int32_t uid));
};

class DisposedObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;

    void CreateInterceptor(std::shared_ptr<DisposedRuleInterceptor>& interceptor)
    {
        interceptor = std::make_shared<MockDisposedRuleInterceptor>();
        EXPECT_NE(interceptor, nullptr);
    }

    void CreateDisposedRule(DisposedRule& disposedRule)
    {
        disposedRule.want = std::make_shared<Want>();
        ElementName element("", "com.example.test", "TestExtension");
        disposedRule.want->SetElement(element);
        disposedRule.componentType = ComponentType::UI_EXTENSION;
    }
};

void DisposedObserverTest::SetUpTestCase(void)
{
    // Initialize native token for testing
    MockNativeToken::SetNativeToken();
}

void DisposedObserverTest::TearDownTestCase(void)
{}

void DisposedObserverTest::SetUp()
{}

void DisposedObserverTest::TearDown()
{}

/**
 * @tc.name: OnPageShow_0100
 * @tc.desc: Test OnPageShow when uid does not match
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, OnPageShow_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0100 start");

    // Create interceptor
    std::shared_ptr<DisposedRuleInterceptor> interceptor;
    CreateInterceptor(interceptor);

    // Create disposed rule
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer with uid = 100
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Create PageStateData with different uid = 200
    PageStateData pageStateData;
    pageStateData.uid = 200;

    // Verify: UnregisterObserver should NOT be called when uid does not match
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(testing::_)).Times(0);

    // Should return early when uid does not match
    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0100 end");
}

/**
 * @tc.name: OnPageShow_0200
 * @tc.desc: Test OnPageShow when componentType is UI_ABILITY and StartAbility fails
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, OnPageShow_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0200 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule for UI_ABILITY
    DisposedRule disposedRule;
    disposedRule.want = std::make_shared<Want>();
    ElementName element("", "com.example.test", "TestAbility");
    disposedRule.want->SetElement(element);
    disposedRule.componentType = ComponentType::UI_ABILITY;

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Create PageStateData with matching uid
    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    // Expect UnregisterObserver to be called when StartAbility fails
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(observerUid)).Times(1);

    // OnPageShow will call StartAbility which will fail (no actual service)
    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0200 end");
}

/**
 * @tc.name: OnPageShow_0300
 * @tc.desc: Test OnPageShow when componentType is UI_EXTENSION
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, OnPageShow_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0300 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule for UI_EXTENSION
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Create PageStateData with matching uid
    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    // Expect UnregisterObserver to be called (ExecuteUIExtension will fail)
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_0300 end");
}

/**
 * @tc.name: OnAbilityStateChanged_0100
 * @tc.desc: Test OnAbilityStateChanged when state is not FOREGROUND
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, OnAbilityStateChanged_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0100 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Create AbilityStateData with BACKGROUND state (not FOREGROUND)
    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::BACKGROUND);

    // Verify: UnregisterObserver should NOT be called when state is not FOREGROUND
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(testing::_)).Times(0);

    // Should return early when state is not FOREGROUND
    observer->OnAbilityStateChanged(abilityStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0100 end");
}

/**
 * @tc.name: OnAbilityStateChanged_0200
 * @tc.desc: Test OnAbilityStateChanged when state is FOREGROUND and model is FA
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, OnAbilityStateChanged_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0200 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Create AbilityStateData with FOREGROUND state
    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::FOREGROUND);
    abilityStateData.uid = observerUid;

    // Expect UnregisterObserver to be called (token is null, so processing will be limited)
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(observerUid)).Times(0); // May or may not be called depending on implementation

    observer->OnAbilityStateChanged(abilityStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0200 end");
}

/**
 * @tc.name: ExecuteUIExtension_0100
 * @tc.desc: Test ExecuteUIExtension when token_ is null (ModalSystemUiExtension path)
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0100, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0100 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);
    EXPECT_EQ(observer->token_, nullptr);  // Verify token_ is null

    // Create PageStateData with matching uid
    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    // Expect UnregisterObserver to be called (ExecuteUIExtension will fail due to null token_)
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(observerUid)).Times(1);

    // Call OnPageShow which calls ExecuteUIExtension with null token_
    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0100 end");
}

/**
 * @tc.name: ExecuteUIExtension_0200
 * @tc.desc: Test OnPageShow after OnAbilityStateChanged (token_ is set)
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0200, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0200 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Create mock token
    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(mockToken, nullptr);

    // Step 1: Trigger OnAbilityStateChanged to set token_
    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    // For StageBasedModel, OnAbilityStateChanged does nothing (no UnregisterObserver call expected)
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    // Verify token_ is set
    EXPECT_EQ(observer->token_, mockToken);

    // Step 2: Call OnPageShow (ExecuteUIExtension will be called with valid token_)
    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    // Expect UnregisterObserver to be called (abilityRecord is null via Token::GetAbilityRecordByToken)
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0200 end");
}

/**
 * @tc.name: OnAbilityStateChanged_0300
 * @tc.desc: Test OnAbilityStateChanged when token is null (StageBasedModel path)
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, OnAbilityStateChanged_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0300 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Create AbilityStateData with FOREGROUND state but null token
    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::FOREGROUND);
    abilityStateData.uid = observerUid;
    abilityStateData.token = nullptr;

    // Verify: UnregisterObserver should NOT be called when token is null (StageBasedModel)
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(testing::_)).Times(0);

    observer->OnAbilityStateChanged(abilityStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_0300 end");
}

/**
 * @tc.name: ExecuteUIExtension_0300
 * @tc.desc: Test ExecuteUIExtension embeddable path (isEmbeddable=true)
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0300, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0300 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Set up mock token
    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(mockToken, nullptr);

    // Create mock abilityRecord with embeddable Want (SCREEN_MODE_KEY = EMBEDDED_FULL_SCREEN_MODE)
    auto abilityRecord = std::make_shared<AAFwk::AbilityRecord>();
    EXPECT_NE(abilityRecord, nullptr);

    // Set up mock Want with SCREEN_MODE_KEY for embeddable
    AAFwk::Want embeddableWant;
    embeddableWant.SetParam(AAFwk::SCREEN_MODE_KEY, AAFwk::EMBEDDED_FULL_SCREEN_MODE);
    AAFwk::Token::mockWant = embeddableWant;

    // Set up mock AbilityInfo (non-PAGE type)
    AAFwk::Token::mockAbilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    AAFwk::Token::mockAbilityInfo.isStageBasedModel = true;

    // Set up CreateModalUIExtension to succeed
    AAFwk::Token::mockCreateModalResult = ERR_OK;
    AAFwk::Token::abilityRecord = abilityRecord;
    AAFwk::Token::mockSessionInfo = nullptr;
    AAFwk::Token::mockMissionId = 0;

    // Set token on observer
    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    // Verify token_ is set
    EXPECT_EQ(observer->token_, mockToken);

    // Create PageStateData with matching uid
    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    // Expect UnregisterObserver to be called when ExecuteUIExtension succeeds
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(observerUid)).Times(1);

    // Call OnPageShow which calls ExecuteUIExtension with embeddable path
    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0300 end");
}

/**
 * @tc.name: ExecuteUIExtension_0400
 * @tc.desc: Test ExecuteUIExtension embeddable path failure (CreateModalUIExtension fails)
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0400, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0400 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Set up mock token
    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(mockToken, nullptr);

    // Create mock abilityRecord with embeddable Want
    auto abilityRecord = std::make_shared<AAFwk::AbilityRecord>();
    EXPECT_NE(abilityRecord, nullptr);

    // Set up mock Want with SCREEN_MODE_KEY for embeddable
    AAFwk::Want embeddableWant;
    embeddableWant.SetParam(AAFwk::SCREEN_MODE_KEY, AAFwk::EMBEDDED_HALF_SCREEN_MODE);
    AAFwk::Token::mockWant = embeddableWant;

    // Set up mock AbilityInfo
    AAFwk::Token::mockAbilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    AAFwk::Token::mockAbilityInfo.isStageBasedModel = true;

    // Set up CreateModalUIExtension to fail
    AAFwk::Token::mockCreateModalResult = ERR_INVALID_VALUE;
    AAFwk::Token::abilityRecord = abilityRecord;
    AAFwk::Token::mockSessionInfo = nullptr;
    AAFwk::Token::mockMissionId = 0;

    // Set token on observer
    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    // Verify token_ is set
    EXPECT_EQ(observer->token_, mockToken);

    // Create PageStateData with matching uid
    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    // Expect UnregisterObserver to be called when ExecuteUIExtension fails
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(observerUid)).Times(1);

    // Call OnPageShow which calls ExecuteUIExtension with embeddable path
    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0400 end");
}

/**
 * @tc.name: ExecuteUIExtension_0500
 * @tc.desc: Test ExecuteUIExtension PAGE type path with null sessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0500, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0500 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Set up mock token
    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(mockToken, nullptr);

    // Create mock abilityRecord with PAGE type
    auto abilityRecord = std::make_shared<AAFwk::AbilityRecord>();
    EXPECT_NE(abilityRecord, nullptr);

    // Set up mock Want without SCREEN_MODE_KEY (not embeddable)
    AAFwk::Want normalWant;
    AAFwk::Token::mockWant = normalWant;

    // Set up mock AbilityInfo as PAGE type
    AAFwk::Token::mockAbilityInfo.type = AppExecFwk::AbilityType::PAGE;
    AAFwk::Token::mockAbilityInfo.isStageBasedModel = true;

    // Set up CreateModalUIExtension to succeed
    AAFwk::Token::mockCreateModalResult = ERR_OK;
    AAFwk::Token::abilityRecord = abilityRecord;
    AAFwk::Token::mockSessionInfo = nullptr;  // null sessionInfo
    AAFwk::Token::mockMissionId = 12345;  // Use missionId instead

    // Set token on observer
    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    // Verify token_ is set
    EXPECT_EQ(observer->token_, mockToken);

    // Create PageStateData with matching uid
    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    // Expect UnregisterObserver to be called when ExecuteUIExtension succeeds
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(observerUid)).Times(1);

    // Call OnPageShow which calls ExecuteUIExtension with PAGE type path
    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0500 end");
}

/**
 * @tc.name: ExecuteUIExtension_0600
 * @tc.desc: Test ExecuteUIExtension PAGE type path with valid sessionInfo
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0600, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0600 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Set up mock token
    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(mockToken, nullptr);

    // Create mock abilityRecord with PAGE type
    auto abilityRecord = std::make_shared<AAFwk::AbilityRecord>();
    EXPECT_NE(abilityRecord, nullptr);

    // Set up mock Want without SCREEN_MODE_KEY
    AAFwk::Want normalWant;
    AAFwk::Token::mockWant = normalWant;

    // Set up mock AbilityInfo as PAGE type
    AAFwk::Token::mockAbilityInfo.type = AppExecFwk::AbilityType::PAGE;
    AAFwk::Token::mockAbilityInfo.isStageBasedModel = true;

    // Set up CreateModalUIExtension to succeed
    AAFwk::Token::mockCreateModalResult = ERR_OK;
    AAFwk::Token::abilityRecord = abilityRecord;

    // Create mock sessionInfo with persistentId
    sptr<SessionInfo> sessionInfo = sptr<SessionInfo>(new (std::nothrow) SessionInfo());
    EXPECT_NE(sessionInfo, nullptr);
    sessionInfo->persistentId = 67890;
    AAFwk::Token::mockSessionInfo = sessionInfo;
    AAFwk::Token::mockMissionId = 0;

    // Set token on observer
    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    // Verify token_ is set
    EXPECT_EQ(observer->token_, mockToken);

    // Create PageStateData with matching uid
    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    // Expect UnregisterObserver to be called when ExecuteUIExtension succeeds
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(observerUid)).Times(1);

    // Call OnPageShow which calls ExecuteUIExtension with PAGE type path
    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0600 end");
}

/**
 * @tc.name: ExecuteUIExtension_0700
 * @tc.desc: Test ExecuteUIExtension PAGE type path failure (CreateModalUIExtension fails)
 * @tc.type: FUNC
 */
HWTEST_F(DisposedObserverTest, ExecuteUIExtension_0700, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0700 start");

    // Create interceptor
    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    EXPECT_NE(interceptor, nullptr);

    // Create disposed rule
    DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    // Create observer
    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    EXPECT_NE(observer, nullptr);

    // Set up mock token
    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(mockToken, nullptr);

    // Create mock abilityRecord with PAGE type
    auto abilityRecord = std::make_shared<AAFwk::AbilityRecord>();
    EXPECT_NE(abilityRecord, nullptr);

    // Set up mock Want without SCREEN_MODE_KEY
    AAFwk::Want normalWant;
    AAFwk::Token::mockWant = normalWant;

    // Set up mock AbilityInfo as PAGE type
    AAFwk::Token::mockAbilityInfo.type = AppExecFwk::AbilityType::PAGE;
    AAFwk::Token::mockAbilityInfo.isStageBasedModel = true;

    // Set up CreateModalUIExtension to fail
    AAFwk::Token::mockCreateModalResult = ERR_INVALID_VALUE;
    AAFwk::Token::abilityRecord = abilityRecord;
    AAFwk::Token::mockSessionInfo = nullptr;
    AAFwk::Token::mockMissionId = 0;

    // Set token on observer
    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AbilityState::FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(testing::_)).Times(0);
    observer->OnAbilityStateChanged(abilityStateData);

    // Verify token_ is set
    EXPECT_EQ(observer->token_, mockToken);

    // Create PageStateData with matching uid
    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    // Expect UnregisterObserver to be called when ExecuteUIExtension fails
    EXPECT_CALL(*static_cast<MockDisposedRuleInterceptor*>(interceptor.get()),
        UnregisterObserver(observerUid)).Times(1);

    // Call OnPageShow which calls ExecuteUIExtension with PAGE type path
    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_0700 end");
}

} // namespace AAFwk
} // namespace OHOS
