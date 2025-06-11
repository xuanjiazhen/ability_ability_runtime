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
#include <gtest/hwext/gtest-multithread.h>

#include <map>
#include <string>
#include <thread>
#include <iostream>

#define private public
#include "iability_monitor.h"
#include "ets_ability_monitor.h"
#undef private

#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;
using namespace OHOS;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {

class EtsabilityMonitorTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void EtsabilityMonitorTest::SetUpTestCase()
{
    // reset optind to 0
    optind = 0;
}

void EtsabilityMonitorTest::TearDownTestCase()
{}

void EtsabilityMonitorTest::SetUp()
{}

void EtsabilityMonitorTest::TearDown()
{}


/**
 * @tc.number: Ets_ability_Monitor_Test_0100
 * @tc.name: OnAbilityStart
 * @tc.desc: Verify the OnAbilityStart.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_0100, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    std::weak_ptr<BaseDelegatorAbilityProperty> emptyWeakPtr;
    monitor.OnAbilityStart(emptyWeakPtr);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_0200
 * @tc.name: OnAbilityStart
 * @tc.desc: Verify the OnAbilityStart.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_0200, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    auto property = std::make_shared<AppExecFwk::ETSDelegatorAbilityProperty>();
    property->object_ = std::make_shared<AbilityRuntime::ETSNativeReference>();
    monitor.OnAbilityStart(property);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_0300
 * @tc.name: OnAbilityForeground
 * @tc.desc: Verify the OnAbilityForeground.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_0300, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    std::weak_ptr<BaseDelegatorAbilityProperty> emptyWeakPtr;
    monitor.OnAbilityForeground(emptyWeakPtr);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_0400
 * @tc.name: OnAbilityForeground
 * @tc.desc: Verify the OnAbilityForeground.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_0400, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    auto property = std::make_shared<AppExecFwk::ETSDelegatorAbilityProperty>();
    property->object_ = std::make_shared<AbilityRuntime::ETSNativeReference>();
    monitor.OnAbilityForeground(property);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_0500
 * @tc.name: OnAbilityBackground
 * @tc.desc: Verify the OnAbilityBackground.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_0500, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    std::weak_ptr<BaseDelegatorAbilityProperty> emptyWeakPtr;
    monitor.OnAbilityBackground(emptyWeakPtr);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_0600
 * @tc.name: OnAbilityBackground
 * @tc.desc: Verify the OnAbilityBackground.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_0600, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    auto property = std::make_shared<AppExecFwk::ETSDelegatorAbilityProperty>();
    property->object_ = std::make_shared<AbilityRuntime::ETSNativeReference>();
    monitor.OnAbilityBackground(property);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_0700
 * @tc.name: OnAbilityStop
 * @tc.desc: Verify the OnAbilityStop.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_0700, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    std::weak_ptr<BaseDelegatorAbilityProperty> emptyWeakPtr;
    monitor.OnAbilityStop(emptyWeakPtr);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_0800
 * @tc.name: OnAbilityStop
 * @tc.desc: Verify the OnAbilityStop.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_0800, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    auto property = std::make_shared<AppExecFwk::ETSDelegatorAbilityProperty>();
    property->object_ = std::make_shared<AbilityRuntime::ETSNativeReference>();
    monitor.OnAbilityStop(property);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_0900
 * @tc.name: OnWindowStageCreate
 * @tc.desc: Verify the OnWindowStageCreate.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_0900, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    std::weak_ptr<BaseDelegatorAbilityProperty> emptyWeakPtr;
    monitor.OnWindowStageCreate(emptyWeakPtr);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_1000
 * @tc.name: OnWindowStageCreate
 * @tc.desc: Verify the OnWindowStageCreate.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_1000, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    auto property = std::make_shared<AppExecFwk::ETSDelegatorAbilityProperty>();
    property->object_ = std::make_shared<AbilityRuntime::ETSNativeReference>();
    monitor.OnWindowStageCreate(property);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_1100
 * @tc.name: OnWindowStageRestore
 * @tc.desc: Verify the OnWindowStageRestore.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_1100, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    std::weak_ptr<BaseDelegatorAbilityProperty> emptyWeakPtr;
    monitor.OnWindowStageRestore(emptyWeakPtr);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_1200
 * @tc.name: OnWindowStageRestore
 * @tc.desc: Verify the OnWindowStageRestore.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_1200, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    auto property = std::make_shared<AppExecFwk::ETSDelegatorAbilityProperty>();
    property->object_ = std::make_shared<AbilityRuntime::ETSNativeReference>();
    monitor.OnWindowStageRestore(property);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_1300
 * @tc.name: OnWindowStageDestroy
 * @tc.desc: Verify the OnWindowStageDestroy.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_1300, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    std::weak_ptr<BaseDelegatorAbilityProperty> emptyWeakPtr;
    monitor.OnWindowStageDestroy(emptyWeakPtr);
}

/**
 * @tc.number: Ets_ability_Monitor_Test_1400
 * @tc.name: OnWindowStageDestroy
 * @tc.desc: Verify the OnWindowStageDestroy.
 */
HWTEST_F(EtsabilityMonitorTest, Ets_ability_Monitor_Test_1400, Function | MediumTest | Level1)
{
    AbilityDelegatorEts::EtsAbilityMonitor monitor("TestAbility");
    auto property = std::make_shared<AppExecFwk::ETSDelegatorAbilityProperty>();
    property->object_ = std::make_shared<AbilityRuntime::ETSNativeReference>();
    monitor.OnWindowStageDestroy(property);
}

} // namespace AppExecFwk
} // namespace OHOS
