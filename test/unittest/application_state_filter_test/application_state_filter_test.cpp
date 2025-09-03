/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "application_state_filter.h"
#undef private
#undef protected

#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "mock_app_mgr_service.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t INVAID_ID = 120;
}  // namespace

class ApplicationStateFilterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<MockAppMgrService> mockAppMgrService_;

    void WriteInterfaceToken(MessageParcel& data);
};

void ApplicationStateFilterTest::SetUpTestCase(void)
{}

void ApplicationStateFilterTest::TearDownTestCase(void)
{}

void ApplicationStateFilterTest::SetUp()
{
    GTEST_LOG_(INFO) << "ApplicationStateFilterTest::SetUp()";

    mockAppMgrService_ = new MockAppMgrService();
}

void ApplicationStateFilterTest::TearDown()
{}

std::unordered_map<ApplicationState, FilterAppStateType> APPLICATION_STATE_TO_FILTER_MAP = {
    {ApplicationState::APP_STATE_CREATE, FilterAppStateType::CREATE},
    {ApplicationState::APP_STATE_FOREGROUND, FilterAppStateType::FOREGROUND},
    {ApplicationState::APP_STATE_BACKGROUND, FilterAppStateType::BACKGROUND},
    {ApplicationState::APP_STATE_TERMINATED, FilterAppStateType::DESTROY},
    {ApplicationState::APP_STATE_READY, FilterAppStateType::READY},
    {ApplicationState::APP_STATE_FOCUS, FilterAppStateType::FOCUS},
    {ApplicationState::APP_STATE_END, FilterAppStateType::END},
    {ApplicationState::APP_STATE_SET_COLD_START, FilterAppStateType::SET_COLD_START},
    {ApplicationState::APP_STATE_CACHED, FilterAppStateType::CACHED},
    {ApplicationState::APP_STATE_PRE_FOREGROUND, FilterAppStateType::PRE_FOREGROUND},
};

std::unordered_map<AppProcessState, FilterProcessStateType> APP_PROCESS_STATE_TO_FILTER_MAP = {
    {AppProcessState::APP_STATE_CREATE, FilterProcessStateType::CREATE},
    {AppProcessState::APP_STATE_FOREGROUND, FilterProcessStateType::FOREGROUND},
    {AppProcessState::APP_STATE_BACKGROUND, FilterProcessStateType::BACKGROUND},
    {AppProcessState::APP_STATE_TERMINATED, FilterProcessStateType::DESTROY},
    {AppProcessState::APP_STATE_READY, FilterProcessStateType::READY},
    {AppProcessState::APP_STATE_FOCUS, FilterProcessStateType::FOCUS},
    {AppProcessState::APP_STATE_END, FilterProcessStateType::END},
    {AppProcessState::APP_STATE_CACHED, FilterProcessStateType::CACHED},
    {AppProcessState::APP_STATE_PRE_FOREGROUND, FilterProcessStateType::PRE_FOREGROUND},
};

std::unordered_map<AbilityState, FilterAbilityStateType> ABILITY_STATE_TO_FILTER_MAP = {
    {AbilityState::ABILITY_STATE_CREATE, FilterAbilityStateType::CREATE},
    {AbilityState::ABILITY_STATE_FOREGROUND, FilterAbilityStateType::FOREGROUND},
    {AbilityState::ABILITY_STATE_BACKGROUND, FilterAbilityStateType::BACKGROUND},
    {AbilityState::ABILITY_STATE_TERMINATED, FilterAbilityStateType::DESTROY},
    {AbilityState::ABILITY_STATE_READY, FilterAbilityStateType::READY},
    {AbilityState::ABILITY_STATE_FOCUS, FilterAbilityStateType::FOCUS},
    {AbilityState::ABILITY_STATE_END, FilterAbilityStateType::END},
    {AbilityState::ABILITY_STATE_CONNECTED, FilterAbilityStateType::CONNECTED},
    {AbilityState::ABILITY_STATE_DISCONNECTED, FilterAbilityStateType::DISCONNECTED},
};

std::unordered_map<BundleType, FilterBundleType> BUNDLE_TYPE_TO_FILTER_MAP = {
    {BundleType::APP, FilterBundleType::APP},
    {BundleType::ATOMIC_SERVICE, FilterBundleType::ATOMIC_SERVICE},
    {BundleType::SHARED, FilterBundleType::SHARED},
    {BundleType::APP_SERVICE_FWK, FilterBundleType::APP_SERVICE_FWK},
    {BundleType::APP_PLUGIN, FilterBundleType::APP_PLUGIN},
};

/**
 * @tc.name: ApplicationStateFilter_GetFilterTypeFromApplicationState_001
 * @tc.desc: GetFilterTypeFromApplicationState
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationStateFilterTest, ApplicationStateFilter_GetFilterTypeFromApplicationState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromApplicationState_001 start";
    auto result = GetFilterTypeFromApplicationState(ApplicationState::APP_STATE_CREATE);
    EXPECT_EQ(result, FilterAppStateType::CREATE);
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromApplicationState_001 end";
}

/**
 * @tc.name: ApplicationStateFilter_GetFilterTypeFromApplicationState_002
 * @tc.desc: GetFilterTypeFromApplicationState
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationStateFilterTest, ApplicationStateFilter_GetFilterTypeFromApplicationState_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromApplicationState_002 start";
    auto result = GetFilterTypeFromApplicationState(static_cast<ApplicationState>(INVAID_ID));
    EXPECT_EQ(result, FilterAppStateType::NONE);
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromApplicationState_002 end";
}

/**
 * @tc.name: ApplicationStateFilter_GetFilterTypeFromAppProcessState_001
 * @tc.desc: GetFilterTypeFromAppProcessState
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationStateFilterTest, ApplicationStateFilter_GetFilterTypeFromAppProcessState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromAppProcessState_001 start";
    auto result = GetFilterTypeFromAppProcessState(ApplicationState::APP_STATE_CREATE);
    EXPECT_EQ(result, FilterAppStateType::CREATE);
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromAppProcessState_001 end";
}

/**
 * @tc.name: ApplicationStateFilter_GetFilterTypeFromAppProcessState_002
 * @tc.desc: GetFilterTypeFromAppProcessState
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationStateFilterTest, ApplicationStateFilter_GetFilterTypeFromAppProcessState_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromAppProcessState_002 start";
    auto result = GetFilterTypeFromAppProcessState(static_cast<ApplicationState>(INVAID_ID));
    EXPECT_EQ(result, FilterAppStateType::NONE);
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromAppProcessState_002 end";
}

/**
 * @tc.name: ApplicationStateFilter_GetFilterTypeFromAbilityState_001
 * @tc.desc: GetFilterTypeFromAbilityState
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationStateFilterTest, ApplicationStateFilter_GetFilterTypeFromAbilityState_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromAbilityState_001 start";
    auto result = GetFilterTypeFromAbilityState(ApplicationState::APP_STATE_CREATE);
    EXPECT_EQ(result, FilterAppStateType::CREATE);
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromAbilityState_001 end";
}

/**
 * @tc.name: ApplicationStateFilter_GetFilterTypeFromAbilityState_002
 * @tc.desc: GetFilterTypeFromAbilityState
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationStateFilterTest, ApplicationStateFilter_GetFilterTypeFromAbilityState_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromAbilityState_002 start";
    auto result = GetFilterTypeFromAbilityState(static_cast<ApplicationState>(INVAID_ID));
    EXPECT_EQ(result, FilterAppStateType::NONE);
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromAbilityState_002 end";
}

/**
 * @tc.name: ApplicationStateFilter_GetFilterTypeFromBundleType_001
 * @tc.desc: GetFilterTypeFromBundleType
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationStateFilterTest, ApplicationStateFilter_GetFilterTypeFromBundleType_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromBundleType_001 start";
    auto result = GetFilterTypeFromBundleType(ApplicationState::APP_STATE_CREATE);
    EXPECT_EQ(result, FilterAppStateType::CREATE);
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromBundleType_001 end";
}

/**
 * @tc.name: ApplicationStateFilter_GetFilterTypeFromBundleType_002
 * @tc.desc: GetFilterTypeFromBundleType
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ApplicationStateFilterTest, ApplicationStateFilter_GetFilterTypeFromBundleType_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromBundleType_002 start";
    auto result = GetFilterTypeFromBundleType(static_cast<ApplicationState>(INVAID_ID));
    EXPECT_EQ(result, FilterAppStateType::NONE);
    GTEST_LOG_(INFO) << "ApplicationStateFilter_GetFilterTypeFromBundleType_002 end";
}

} // namespace AppExecFwk
} // namespace OHOS
