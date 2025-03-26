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
#define protected public
#include "ability_manager_service.h"
#include "sub_managers_helper.h"
#undef private
#undef protected
#include "ability_manager_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class AbilityManagerServiceMockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
    
void AbilityManagerServiceMockTest::SetUpTestCase() {}
    
void AbilityManagerServiceMockTest::TearDownTestCase() {}
    
void AbilityManagerServiceMockTest::SetUp() {}
    
void AbilityManagerServiceMockTest::TearDown() {}

/**
 * @tc.name: ExecuteIntent_0100
 * @tc.desc: Test  StartAbilityWithInsightIntent error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntent_0100, TestSize.Level1)
{
    uint64_t key = 0;
    sptr<IRemoteObject> callerToken;
    InsightIntentExecuteParam param;
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    auto abilityMs = std::make_shared<AbilityManagerService>();
    auto ret = abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: PreloadUIExtensionAbilityInner_0100
 * @tc.desc: Test  PreloadUIExtensionAbilityInner error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, PreloadUIExtensionAbilityInner_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);
    std::string hostBundleName = "com.ohos.uiextensionuser";
    //abilityMs->subManagersHelper_->connectManagers_.clear();
    auto result = abilityMs->PreloadUIExtensionAbilityInner(providerWant, hostBundleName, DEFAULT_INVAL_VALUE);
    EXPECT_NE(result, ERR_OK);
}
}
}