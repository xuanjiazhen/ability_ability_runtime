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
#include "ability_manager_service.h"
#undef private
#undef protected

#include "ability_manager_errors.h"
#include "start_ability_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
const int32_t USER_ID_U100 = 100;
const int REQUESTCODE = 10;
const std::string APP_LINKING_ONLY = "appLinkingOnly";
}  // namespace

class AbilityManagerServiceEmulatorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityManagerServiceEmulatorTest::SetUpTestCase() {}

void AbilityManagerServiceEmulatorTest::TearDownTestCase() {}

void AbilityManagerServiceEmulatorTest::SetUp() {}

void AbilityManagerServiceEmulatorTest::TearDown() {}

/*
 * Feature: AbilityManagerService
 * Function: OpenAtomicService
 * FunctionPoints: AbilityManagerService OpenAtomicService on EMULATOR_BUILD
 * CaseDescription: atomic service startup is rejected with ERR_EMULATOR_NOT_SUPPORTED.
 */
HWTEST_F(AbilityManagerServiceEmulatorTest, OpenAtomicService_Emulator_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    StartOptions startOptions;
    auto result = abilityMs->OpenAtomicService(want, startOptions, nullptr, 1, USER_ID_U100);
    EXPECT_EQ(result, ERR_EMULATOR_NOT_SUPPORTED);
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLinkInner
 * FunctionPoints: AbilityManagerService OpenLinkInner on EMULATOR_BUILD
 * CaseDescription: app linking (appLinkingOnly) is rejected with ERR_EMULATOR_NOT_SUPPORTED.
 */
HWTEST_F(AbilityManagerServiceEmulatorTest, OpenLinkInner_Emulator_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetParam(APP_LINKING_ONLY, true);
    auto result = abilityMs->OpenLinkInner(want, nullptr, USER_ID_U100, REQUESTCODE, true);
    EXPECT_EQ(result, ERR_EMULATOR_NOT_SUPPORTED);
}

/*
 * Feature: AbilityManagerService
 * Function: OpenLinkFreeInstallAtomicService
 * FunctionPoints: AbilityManagerService OpenLinkFreeInstallAtomicService on EMULATOR_BUILD
 * CaseDescription: installation-free atomic service is rejected with ERR_EMULATOR_NOT_SUPPORTED.
 */
HWTEST_F(AbilityManagerServiceEmulatorTest, OpenLinkFreeInstallAtomicService_Emulator_001, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    StartAbilityWrapParam param = {
        .want = want,
        .requestCode = REQUESTCODE,
        .userId = USER_ID_U100,
    };
    auto result = abilityMs->OpenLinkFreeInstallAtomicService(param, want);
    EXPECT_EQ(result, ERR_EMULATOR_NOT_SUPPORTED);
}
}  // namespace AAFwk
}  // namespace OHOS