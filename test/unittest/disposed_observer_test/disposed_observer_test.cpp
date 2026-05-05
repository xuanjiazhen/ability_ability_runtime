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
#include <gmock/gmock.h>

#define private public
#include "disposed_observer.h"
#include "mock_my_flag.h"
#include "mock_ability_token.h"
#undef private

#include "disposed_rule_interceptor.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class DisposedObserverTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DisposedObserverTest::SetUpTestCase()
{}

void DisposedObserverTest::TearDownTestCase()
{}

void DisposedObserverTest::SetUp()
{
    // Reset mock flags before each test
    MyFlag::abilityRecord_ = nullptr;
    MyFlag::retCreateModalUIExtension_ = false;
    MyFlag::retAbilityRecordCreateModalUIExtension_ = ERR_OK;
}

void DisposedObserverTest::TearDown()
{}

/**
 * @tc.number: DisposedObserver_ExecuteUIExtension_0100
 * @tc.name: ExecuteUIExtension with null abilityRecord
 * @tc.desc: ExecuteUIExtension handles null abilityRecord gracefully
 * @tc.type: FUNC
 * @tc.require: I7XKZI
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_ExecuteUIExtension_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0100 start");

    auto interceptor = std::make_shared<DisposedRuleInterceptor>();
    AppExecFwk::DisposedRule rule;
    rule.componentType = AppExecFwk::ComponentType::UI_EXTENSION;
    rule.want = std::make_shared<Want>();
    int32_t uid = 100;

    auto observer = sptr<DisposedObserver>::MakeSptr(rule, interceptor, uid);

    // abilityRecord is null
    MyFlag::abilityRecord_ = nullptr;
    MyFlag::retCreateModalUIExtension_ = true;
    ErrCode result = observer->ExecuteUIExtension(AppExecFwk::PageStateData());
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0100 end");
}

/**
 * @tc.number: DisposedObserver_ExecuteUIExtension_0200
 * @tc.name: ExecuteUIExtension in embeddable mode
 * @tc.desc: ExecuteUIExtension sets IS_EMBEDDABLE_SERVICE for embeddable mode
 * @tc.type: FUNC
 * @tc.require: I7XKZI
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_ExecuteUIExtension_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0200 start");

    auto interceptor = std::make_shared<DisposedRuleInterceptor>();
    AppExecFwk::DisposedRule rule;
    rule.componentType = AppExecFwk::ComponentType::UI_EXTENSION;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "com.test.modal", "entry", "ModalExtension");
    rule.want->SetParam(SCREEN_MODE_KEY, EMBEDDED_FULL_SCREEN_MODE);
    int32_t uid = 100;

    auto observer = sptr<DisposedObserver>::MakeSptr(rule, interceptor, uid);

    // Create mock abilityRecord with EMBEDDED_FULL_SCREEN_MODE
    MyFlag::abilityRecord_ = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    MyFlag::retAbilityRecordCreateModalUIExtension_ = ERR_OK;

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    observer->token_ = token;

    AppExecFwk::PageStateData pageStateData;
    ErrCode result = observer->ExecuteUIExtension(pageStateData);

    // Should route to embeddable path
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0200 end");
}

/**
 * @tc.number: DisposedObserver_ExecuteUIExtension_0300
 * @tc.name: ExecuteUIExtension for PAGE type
 * @tc.desc: ExecuteUIExtension routes to AbilityRecord CreateModalUIExtension for PAGE
 * @tc.type: FUNC
 * @tc.require: I7XKZI
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_ExecuteUIExtension_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0300 start");

    auto interceptor = std::make_shared<DisposedRuleInterceptor>();
    AppExecFwk::DisposedRule rule;
    rule.componentType = AppExecFwk::ComponentType::UI_EXTENSION;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "com.test.modal", "entry", "ModalExtension");
    int32_t uid = 100;

    auto observer = sptr<DisposedObserver>::MakeSptr(rule, interceptor, uid);

    // Create mock abilityRecord with PAGE type, non-embeddable
    MyFlag::abilityRecord_ = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    MyFlag::retAbilityRecordCreateModalUIExtension_ = ERR_OK;

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    observer->token_ = token;

    AppExecFwk::PageStateData pageStateData;
    ErrCode result = observer->ExecuteUIExtension(pageStateData);

    // Should route to modal app path
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0300 end");
}

/**
 * @tc.number: DisposedObserver_ExecuteUIExtension_0400
 * @tc.name: ExecuteUIExtension for non-PAGE type
 * @tc.desc: ExecuteUIExtension routes to modal system path for non-PAGE type
 * @tc.type: FUNC
 * @tc.require: I7XKZI
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_ExecuteUIExtension_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0400 start");

    auto interceptor = std::make_shared<DisposedRuleInterceptor>();
    AppExecFwk::DisposedRule rule;
    rule.componentType = AppExecFwk::ComponentType::UI_EXTENSION;
    rule.want = std::make_shared<Want>();
    int32_t uid = 100;

    auto observer = sptr<DisposedObserver>::MakeSptr(rule, interceptor, uid);

    // Create mock abilityRecord with SERVICE type (non-PAGE)
    MyFlag::abilityRecord_ = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_->abilityInfo.type = AppExecFwk::AbilityType::SERVICE;
    MyFlag::retCreateModalUIExtension_ = true;

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    observer->token_ = token;

    AppExecFwk::PageStateData pageStateData;
    ErrCode result = observer->ExecuteUIExtension(pageStateData);

    // Should route to modal system path
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0400 end");
}

/**
 * @tc.number: DisposedObserver_ExecuteUIExtension_0500
 * @tc.name: ExecuteUIExtension CreateModalUIExtension failure
 * @tc.desc: ExecuteUIExtension returns error when CreateModalUIExtension fails
 * @tc.type: FUNC
 * @tc.require: I7XKZI
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_ExecuteUIExtension_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0500 start");

    auto interceptor = std::make_shared<DisposedRuleInterceptor>();
    AppExecFwk::DisposedRule rule;
    rule.componentType = AppExecFwk::ComponentType::UI_EXTENSION;
    rule.want = std::make_shared<Want>();
    int32_t uid = 100;

    auto observer = sptr<DisposedObserver>::MakeSptr(rule, interceptor, uid);

    MyFlag::abilityRecord_ = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    MyFlag::retAbilityRecordCreateModalUIExtension_ = ERR_INVALID_VALUE;

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    observer->token_ = token;

    AppExecFwk::PageStateData pageStateData;
    ErrCode result = observer->ExecuteUIExtension(pageStateData);

    // Should return error
    EXPECT_EQ(result, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0500 end");
}

/**
 * @tc.number: DisposedObserver_ExecuteUIExtension_0600
 * @tc.name: ExecuteUIExtension in HALF_SCREEN embeddable mode
 * @tc.desc: ExecuteUIExtension handles HALF_SCREEN embeddable mode
 * @tc.type: FUNC
 * @tc.require: I7XKZI
 */
HWTEST_F(DisposedObserverTest, DisposedObserver_ExecuteUIExtension_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0600 start");

    auto interceptor = std::make_shared<DisposedRuleInterceptor>();
    AppExecFwk::DisposedRule rule;
    rule.componentType = AppExecFwk::ComponentType::UI_EXTENSION;
    rule.want = std::make_shared<Want>();
    rule.want->SetElementName("", "com.test.modal", "entry", "ModalExtension");
    rule.want->SetParam(SCREEN_MODE_KEY, EMBEDDED_HALF_SCREEN_MODE);
    int32_t uid = 100;

    auto observer = sptr<DisposedObserver>::MakeSptr(rule, interceptor, uid);

    // Create mock abilityRecord with EMBEDDED_HALF_SCREEN_MODE
    MyFlag::abilityRecord_ = std::make_shared<AbilityRecord>();
    MyFlag::abilityRecord_->abilityInfo.type = AppExecFwk::AbilityType::PAGE;
    MyFlag::retAbilityRecordCreateModalUIExtension_ = ERR_OK;

    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    observer->token_ = token;

    AppExecFwk::PageStateData pageStateData;
    ErrCode result = observer->ExecuteUIExtension(pageStateData);

    // Should route to embeddable path
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "DisposedObserver_ExecuteUIExtension_0600 end");
}

} // namespace AAFwk
} // namespace OHOS
