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
#include "interceptor/disposed_rule_interceptor.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "mock_my_flag.h"
#include "modal_system_ui_extension.h"
#include "ability_record.h"
#include "want.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS::Rosen;

namespace OHOS {
namespace AAFwk {

constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
constexpr const char* INTERCEPT_MISSION_ID = "intercept_missionId";

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

    void ResetMockAbilityRecord()
    {
        MyFlag::abilityRecord_ = nullptr;
        MyFlag::retAbilityRecordCreateModalUIExtension_ = ERR_OK;
        MyFlag::retCreateModalUIExtension_ = true;
        MyFlag::startAbilityRet_ = ERR_OK;
    }

    void CreateDisposedRule(AppExecFwk::DisposedRule& disposedRule, 
                           AppExecFwk::ComponentType type = AppExecFwk::ComponentType::UI_EXTENSION)
    {
        disposedRule.want = std::make_shared<Want>();
        ElementName element("", "com.example.test", "TestExtension");
        disposedRule.want->SetElement(element);
        disposedRule.componentType = type;
    }
};

void DisposedObserverTest::SetUpTestCase(void)
{}

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

HWTEST_F(DisposedObserverTest, OnAbilityStateChanged_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_001 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_BACKGROUND);
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);

    observer->OnAbilityStateChanged(abilityStateData);

    EXPECT_EQ(observer->token_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_001 end");
}

HWTEST_F(DisposedObserverTest, OnAbilityStateChanged_002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_002 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_ = abilityRecord;
    abilityRecord->abilityInfo.isStageBasedModel = false;

    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    ASSERT_NE(mockToken, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnAbilityStateChanged(abilityStateData);

    EXPECT_EQ(observer->token_, mockToken);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_002 end");
}

HWTEST_F(DisposedObserverTest, OnAbilityStateChanged_003, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_003 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = nullptr;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);

    observer->OnAbilityStateChanged(abilityStateData);

    EXPECT_EQ(observer->token_, nullptr);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_003 end");
}

HWTEST_F(DisposedObserverTest, OnAbilityStateChanged_004, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_004 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_ = abilityRecord;
    abilityRecord->abilityInfo.isStageBasedModel = true;

    sptr<IRemoteObject> mockToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    ASSERT_NE(mockToken, nullptr);

    AbilityStateData abilityStateData;
    abilityStateData.abilityState = static_cast<int32_t>(AppExecFwk::AbilityState::ABILITY_STATE_FOREGROUND);
    abilityStateData.token = mockToken;
    abilityStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);

    observer->OnAbilityStateChanged(abilityStateData);

    EXPECT_EQ(observer->token_, mockToken);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged_004 end");
}

HWTEST_F(DisposedObserverTest, OnPageShow_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_001 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    PageStateData pageStateData;
    pageStateData.uid = 200;

    EXPECT_CALL(*interceptor, UnregisterObserver(testing::_)).Times(0);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_001 end");
}

HWTEST_F(DisposedObserverTest, OnPageShow_002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_002 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule, AppExecFwk::ComponentType::UI_ABILITY);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    MyFlag::startAbilityRet_ = ERR_OK;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_002 end");
}

HWTEST_F(DisposedObserverTest, OnPageShow_003, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_003 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule, AppExecFwk::ComponentType::UI_ABILITY);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    MyFlag::startAbilityRet_ = ERR_NO_MEMORY;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_003 end");
}

HWTEST_F(DisposedObserverTest, OnPageShow_004, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_004 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule, AppExecFwk::ComponentType::UI_EXTENSION);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    EXPECT_CALL(*interceptor, UnregisterObserver(observerUid)).Times(1);

    observer->OnPageShow(pageStateData);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnPageShow_004 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_001 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    MyFlag::abilityRecord_ = nullptr;
    MyFlag::retCreateModalUIExtension_ = true;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    ErrCode ret = observer->ExecuteUIExtension(pageStateData);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_001 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_002 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    MyFlag::abilityRecord_ = nullptr;
    MyFlag::retCreateModalUIExtension_ = false;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    ErrCode ret = observer->ExecuteUIExtension(pageStateData);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_002 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_003, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_003 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_ = abilityRecord;
    abilityRecord->abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    MyFlag::retCreateModalUIExtension_ = true;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    ErrCode ret = observer->ExecuteUIExtension(pageStateData);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_003 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_004, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_004 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_ = abilityRecord;
    abilityRecord->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    MyFlag::retAbilityRecordCreateModalUIExtension_ = ERR_OK;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    ErrCode ret = observer->ExecuteUIExtension(pageStateData);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_004 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_005, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_005 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_ = abilityRecord;
    abilityRecord->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    MyFlag::retAbilityRecordCreateModalUIExtension_ = ERR_NO_MEMORY;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    ErrCode ret = observer->ExecuteUIExtension(pageStateData);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_005 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_006, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_006 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_ = abilityRecord;
    Want want;
    want.SetParam(AAFwk::SCREEN_MODE_KEY, AAFwk::EMBEDDED_FULL_SCREEN_MODE);
    abilityRecord->want_ = want;
    MyFlag::retAbilityRecordCreateModalUIExtension_ = ERR_OK;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    ErrCode ret = observer->ExecuteUIExtension(pageStateData);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_006 end");
}

HWTEST_F(DisposedObserverTest, ExecuteUIExtension_007, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_007 start");

    auto interceptor = std::make_shared<MockDisposedRuleInterceptor>();
    ASSERT_NE(interceptor, nullptr);

    AppExecFwk::DisposedRule disposedRule;
    CreateDisposedRule(disposedRule);

    int32_t observerUid = 100;
    auto observer = std::make_shared<DisposedObserver>(disposedRule, interceptor, observerUid);
    ASSERT_NE(observer, nullptr);

    auto abilityRecord = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_ = abilityRecord;
    Want want;
    want.SetParam(AAFwk::SCREEN_MODE_KEY, AAFwk::EMBEDDED_FULL_SCREEN_MODE);
    abilityRecord->want_ = want;
    MyFlag::retAbilityRecordCreateModalUIExtension_ = ERR_NO_MEMORY;

    PageStateData pageStateData;
    pageStateData.uid = observerUid;

    ErrCode ret = observer->ExecuteUIExtension(pageStateData);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteUIExtension_007 end");
}

} // namespace AAFwk
} // namespace OHOS