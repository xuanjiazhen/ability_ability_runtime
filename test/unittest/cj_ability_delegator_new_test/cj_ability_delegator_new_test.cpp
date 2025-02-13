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

#include "ability_delegator_registry.h"
#include "application_context.h"
#include "cj_ability_delegator.h"
#include "cj_application_context.h"
#include "gtest/gtest.h"
#include "runner_runtime/cj_test_runner.h"

using namespace testing::ext;
namespace OHOS {
namespace AbilityDelegatorCJ {
constexpr uint64_t RETURN_VALUE1 = 401;
constexpr uint64_t RETURN_VALUE2 = 16000100;
extern std::map<int64_t, std::shared_ptr<CJAbilityMonitor>> g_monitorRecord;
extern std::map<int64_t, std::shared_ptr<CJAbilityStageMonitor>> g_stageMonitorRecord;
extern std::map<int64_t, sptr<OHOS::IRemoteObject>> g_abilityRecord;
class Token;
std::shared_ptr<CJAbilityMonitor> ParseMonitorPara(
    int64_t monitorId, const std::string& abilityName, const std::string& moduleName, bool& isExisted);
std::shared_ptr<CJAbilityStageMonitor> ParseStageMonitorPara(
    int64_t stageMonitorId, const std::string& moduleName, const std::string& srcEntrance, bool& isExisted);

class CjAbilityDelegatorNewTest : public testing::Test {
public:
    CjAbilityDelegatorNewTest() {}
    ~CjAbilityDelegatorNewTest() {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    std::shared_ptr<CJAbilityDelegator> cjDelegator = nullptr;
    std::shared_ptr<AppExecFwk::CJAbilityDelegatorImpl> abilityDelegatorImpl_ = nullptr;
    std::shared_ptr<AppExecFwk::AbilityDelegatorArgs> delegatorArgs_ = nullptr;
};

void CjAbilityDelegatorNewTest::SetUpTestCase() {}

void CjAbilityDelegatorNewTest::TearDownTestCase() {}

void CjAbilityDelegatorNewTest::SetUp()
{
    AAFwk::Want want;
    delegatorArgs_ = std::make_shared<AbilityDelegatorArgs>(want);
    AbilityRuntime::Runtime::Options options;
    BundleInfo bundleInfo;
    auto testRunner =
        RunnerRuntime::CJTestRunner::Create(AbilityRuntime::Runtime::Create(options), delegatorArgs_, bundleInfo);
    abilityDelegatorImpl_ = std::make_shared<CJAbilityDelegatorImpl>(
        std::make_shared<AbilityRuntime::ContextImpl>(), std::move(testRunner), nullptr);

    // creat CJAbilityDelegator
    cjDelegator = std::make_shared<CJAbilityDelegator>(abilityDelegatorImpl_);
}

void CjAbilityDelegatorNewTest::TearDown()
{
    delegatorArgs_.reset();
    abilityDelegatorImpl_.reset();
    cjDelegator.reset();
    g_monitorRecord.clear();
    g_stageMonitorRecord.clear();
    g_abilityRecord.clear();
}

/**
 * @tc.name: ConstructorFunctionTest_001
 * @tc.desc: ConstructorFunction test
c
 */
HWTEST_F(CjAbilityDelegatorNewTest, ConstructorFunctionTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConstructorFunctionTest_001 start";

    std::shared_ptr<AppExecFwk::ACJDelegatorAbilityProperty> property = nullptr;
    g_abilityRecord.emplace(0, nullptr);
    cjDelegator->delegator_->clearFunc_(property);
    EXPECT_EQ(g_abilityRecord.size(), 1);

    property = std::make_shared<AppExecFwk::ACJDelegatorAbilityProperty>();
    property->token_ = nullptr;
    cjDelegator->delegator_->clearFunc_(property);
    EXPECT_EQ(g_abilityRecord.size(), 0);

    AAFwk::Want want;
    EXPECT_NE(cjDelegator->StartAbility(want), 0);

    GTEST_LOG_(INFO) << "ConstructorFunctionTest_001 end";
}

/**
 * @tc.name: AddOrRemoveAbilityMonitorTest_001
 * @tc.desc: AddOrRemoveAbilityMonitor test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, AddOrRemoveAbilityMonitorTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOrRemoveAbilityMonitorTest_001 start";

    std::shared_ptr<CJAbilityMonitor> abilityMonitor = std::make_shared<CJAbilityMonitor>("", nullptr);
    EXPECT_NE(abilityMonitor, nullptr);
    cjDelegator->AddAbilityMonitor(abilityMonitor);
    EXPECT_EQ(cjDelegator->delegator_->abilityMonitors_.size(), 1);
    std::shared_ptr<CJAbilityStageMonitor> stageMonitor = std::make_shared<CJAbilityStageMonitor>("", "", 0);
    EXPECT_NE(stageMonitor, nullptr);
    cjDelegator->AddAbilityStageMonitor(stageMonitor);
    EXPECT_EQ(cjDelegator->delegator_->abilityStageMonitors_.size(), 1);

    cjDelegator->RemoveAbilityMonitor(abilityMonitor);
    EXPECT_EQ(cjDelegator->delegator_->abilityMonitors_.size(), 0);
    cjDelegator->RemoveAbilityStageMonitor(stageMonitor);
    EXPECT_EQ(cjDelegator->delegator_->abilityStageMonitors_.size(), 0);

    GTEST_LOG_(INFO) << "AddOrRemoveAbilityMonitorTest_001 end";
}

/**
 * @tc.name: WaitAbilityMonitorTest_001
 * @tc.desc: WaitAbilityMonitor test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, WaitAbilityMonitorTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WaitAbilityMonitorTest_001 start";

    int64_t timeoutMs = 1;
    std::shared_ptr<CJAbilityMonitor> abilityMonitor = std::make_shared<CJAbilityMonitor>("", nullptr);
    EXPECT_NE(abilityMonitor, nullptr);
    abilityMonitor->matchedAbility_ = nullptr;
    EXPECT_EQ(cjDelegator->WaitAbilityMonitor(abilityMonitor), nullptr);
    EXPECT_EQ(cjDelegator->WaitAbilityMonitor(abilityMonitor, timeoutMs), nullptr);
    std::shared_ptr<CJAbilityStageMonitor> stageMonitor = std::make_shared<CJAbilityStageMonitor>("", "", 0);
    EXPECT_NE(stageMonitor, nullptr);
    stageMonitor->matchedAbilityStage_ = nullptr;
    EXPECT_EQ(cjDelegator->WaitAbilityStageMonitor(stageMonitor), nullptr);
    EXPECT_EQ(cjDelegator->WaitAbilityStageMonitor(stageMonitor, timeoutMs), nullptr);

    abilityMonitor->matchedAbility_ = std::make_shared<ACJDelegatorAbilityProperty>();
    EXPECT_NE(abilityMonitor->matchedAbility_, nullptr);
    EXPECT_NE(cjDelegator->WaitAbilityMonitor(abilityMonitor), nullptr);
    EXPECT_NE(cjDelegator->WaitAbilityMonitor(abilityMonitor, timeoutMs), nullptr);
    stageMonitor->matchedAbilityStage_ = std::make_shared<CJDelegatorAbilityStageProperty>();
    EXPECT_NE(stageMonitor->matchedAbilityStage_, nullptr);
    EXPECT_NE(cjDelegator->WaitAbilityStageMonitor(stageMonitor), nullptr);
    EXPECT_NE(cjDelegator->WaitAbilityStageMonitor(stageMonitor, timeoutMs), nullptr);

    GTEST_LOG_(INFO) << "WaitAbilityMonitorTest_001 end";
}

/**
 * @tc.name: ParseMonitorParaTest_001
 * @tc.desc: ParseMonitorPara test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, ParseMonitorParaTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParseMonitorParaTest_001 start";

    std::string abilityName = "";
    std::string moduleName = "";
    bool isExisted = false;
    g_monitorRecord.emplace(0, nullptr);
    std::shared_ptr<CJAbilityMonitor> returnDataTest1 = ParseMonitorPara(0, abilityName, moduleName, isExisted);
    EXPECT_EQ(returnDataTest1, nullptr);

    isExisted = false;
    returnDataTest1 = ParseMonitorPara(1, abilityName, moduleName, isExisted);
    EXPECT_NE(returnDataTest1, nullptr);

    moduleName = "testParseMonitorPara";
    returnDataTest1 = ParseMonitorPara(1, abilityName, moduleName, isExisted);
    EXPECT_NE(returnDataTest1, nullptr);

    g_stageMonitorRecord.emplace(0, nullptr);
    std::shared_ptr<CJAbilityStageMonitor> returnDataTest2 = ParseStageMonitorPara(0, "", "", isExisted);
    EXPECT_EQ(returnDataTest2, nullptr);

    isExisted = false;
    returnDataTest2 = ParseStageMonitorPara(1, "", "", isExisted);
    EXPECT_NE(returnDataTest2, nullptr);

    GTEST_LOG_(INFO) << "ParseMonitorParaTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorDoAbilityForegroundTest_001
 * @tc.desc: FFIAbilityDelegatorDoAbilityForeground test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorDoAbilityForegroundTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorDoAbilityForegroundTest_001 start";

    int64_t id = 0;
    int64_t abilityId = 0;
    bool ret = true;
    EXPECT_EQ(FFIAbilityDelegatorDoAbilityForeground(id, abilityId, &ret), RETURN_VALUE2);
    EXPECT_FALSE(ret);

    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    abilityId = id;
    EXPECT_EQ(FFIAbilityDelegatorDoAbilityForeground(id, abilityId, nullptr), RETURN_VALUE2);

    ret = true;
    g_abilityRecord.emplace(abilityId, nullptr);
    EXPECT_EQ(FFIAbilityDelegatorDoAbilityForeground(id, abilityId, &ret), RETURN_VALUE1);
    EXPECT_FALSE(ret);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorDoAbilityForegroundTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorDoAbilityBackgroundTest_001
 * @tc.desc: FFIAbilityDelegatorDoAbilityBackground test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorDoAbilityBackgroundTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorDoAbilityBackgroundTest_001 start";

    int64_t id = 0;
    int64_t abilityId = 0;
    EXPECT_EQ(FFIAbilityDelegatorDoAbilityBackground(id, abilityId, nullptr), RETURN_VALUE2);

    bool ret = true;
    EXPECT_EQ(FFIAbilityDelegatorDoAbilityBackground(id, abilityId, &ret), RETURN_VALUE2);
    EXPECT_FALSE(ret);

    ret = true;
    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    abilityId = id;
    g_abilityRecord.emplace(abilityId, nullptr);
    EXPECT_EQ(FFIAbilityDelegatorDoAbilityBackground(id, abilityId, &ret), RETURN_VALUE1);
    EXPECT_FALSE(ret);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorDoAbilityBackgroundTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorGetCurrentTopAbilityTest_001
 * @tc.desc: FFIAbilityDelegatorGetCurrentTopAbility test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorGetCurrentTopAbilityTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorGetCurrentTopAbilityTest_001 start";

    int64_t id = 0;
    int64_t abilityId = 0;
    EXPECT_EQ(FFIAbilityDelegatorGetCurrentTopAbility(id, &abilityId), RETURN_VALUE2);

    id = 1;
    abilityId = 1;
    EXPECT_EQ(FFIAbilityDelegatorGetCurrentTopAbility(id, &abilityId), RETURN_VALUE2);
    EXPECT_EQ(abilityId, 0);

    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    abilityId = id;
    g_abilityRecord.emplace(abilityId, nullptr);
    EXPECT_EQ(FFIAbilityDelegatorGetCurrentTopAbility(id, &abilityId), RETURN_VALUE2);
    EXPECT_EQ(abilityId, 0);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorGetCurrentTopAbilityTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorPrintTest_001
 * @tc.desc: FFIAbilityDelegatorPrint test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorPrintTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorPrintTest_001 start";

    int64_t id = 0;
    std::string msgStr = "test";
    EXPECT_EQ(FFIAbilityDelegatorPrint(id, msgStr.c_str()), RETURN_VALUE2);

    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    EXPECT_EQ(FFIAbilityDelegatorPrint(id, msgStr.c_str()), 0);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorPrintTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorGetAbilityStateTest_001
 * @tc.desc: FIAbilityDelegatorGetAbilityState test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorGetAbilityStateTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorGetAbilityStateTest_001 start";

    int64_t id = 0;
    int64_t abilityId = 0;
    int64_t state = 0;
    EXPECT_EQ(FFIAbilityDelegatorGetAbilityState(id, abilityId, &state), RETURN_VALUE2);

    state = 1;
    EXPECT_EQ(FFIAbilityDelegatorGetAbilityState(id, abilityId, &state), RETURN_VALUE2);
    EXPECT_EQ(state, -1);

    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    abilityId = id;
    g_abilityRecord.emplace(abilityId, nullptr);
    EXPECT_EQ(FFIAbilityDelegatorGetAbilityState(id, abilityId, &state), RETURN_VALUE1);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorGetAbilityStateTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorAddAbilityMonitorTest_001
 * @tc.desc: FFIAbilityDelegatorAddAbilityMonitor test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorAddAbilityMonitorTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorAddAbilityMonitorTest_001 start";

    int64_t id = 0;
    int64_t monitorId = 0;
    std::string abilityName = "test";
    std::string moduleName = "test";
    EXPECT_EQ(
        FFIAbilityDelegatorAddAbilityMonitor(id, monitorId, abilityName.c_str(), moduleName.c_str()), RETURN_VALUE2);

    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    monitorId = id;
    g_monitorRecord.emplace(monitorId, nullptr);
    EXPECT_EQ(
        FFIAbilityDelegatorAddAbilityMonitor(id, monitorId, abilityName.c_str(), moduleName.c_str()), RETURN_VALUE1);

    g_monitorRecord.clear();
    EXPECT_EQ(FFIAbilityDelegatorAddAbilityMonitor(id, monitorId, abilityName.c_str(), moduleName.c_str()), 0);
    EXPECT_EQ(g_monitorRecord.size(), 1);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorAddAbilityMonitorTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorRemoveAbilityMonitorTest_001
 * @tc.desc: FFIAbilityDelegatorRemoveAbilityMonitor test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorRemoveAbilityMonitorTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorRemoveAbilityMonitorTest_001 start";

    int64_t id = 0;
    int64_t monitorId = 0;
    std::string abilityName = "test";
    std::string moduleName = "test";
    EXPECT_EQ(
        FFIAbilityDelegatorRemoveAbilityMonitor(id, monitorId, abilityName.c_str(), moduleName.c_str()), RETURN_VALUE2);

    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    monitorId = id;
    g_monitorRecord.emplace(monitorId, nullptr);
    EXPECT_EQ(
        FFIAbilityDelegatorRemoveAbilityMonitor(id, monitorId, abilityName.c_str(), moduleName.c_str()), RETURN_VALUE1);

    g_monitorRecord.at(monitorId) = std::make_shared<CJAbilityMonitor>("", nullptr);
    EXPECT_EQ(FFIAbilityDelegatorRemoveAbilityMonitor(id, monitorId, abilityName.c_str(), moduleName.c_str()), 0);
    EXPECT_EQ(g_monitorRecord.size(), 0);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorRemoveAbilityMonitorTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorWaitAbilityMonitorTest_001
 * @tc.desc: FFIAbilityDelegatorWaitAbilityMonitor test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorWaitAbilityMonitorTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorWaitAbilityMonitorTest_001 start";

    int64_t id = 0;
    int64_t monitorId = 0;
    std::string abilityName = "test";
    std::string moduleName = "test";
    CJAbilityInfo abilityInfo;
    abilityInfo.abilityName = abilityName.c_str();
    abilityInfo.moduleName = moduleName.c_str();
    int64_t abilityId = 0;
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityMonitor(id, monitorId, abilityInfo, &abilityId), RETURN_VALUE2);

    abilityId = 1;
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityMonitor(id, monitorId, abilityInfo, &abilityId), RETURN_VALUE2);
    EXPECT_EQ(abilityId, 0);

    abilityId = 1;
    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    monitorId = id;
    g_monitorRecord.emplace(monitorId, nullptr);
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityMonitor(id, monitorId, abilityInfo, &abilityId), RETURN_VALUE1);
    EXPECT_EQ(abilityId, 0);

    abilityId = 1;
    g_monitorRecord.clear();
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityMonitor(id, monitorId, abilityInfo, &abilityId), RETURN_VALUE2);
    EXPECT_EQ(g_monitorRecord.size(), 1);
    EXPECT_EQ(abilityId, 0);

    abilityId = 1;
    auto cjMonitor = std::make_shared<CJAbilityMonitor>("", nullptr);
    cjMonitor->matchedAbility_ = std::make_shared<ACJDelegatorAbilityProperty>();
    g_monitorRecord.at(monitorId) = cjMonitor;
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityMonitor(id, monitorId, abilityInfo, &abilityId), 0);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorWaitAbilityMonitorTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorAddAbilityStageMonitorTest_001
 * @tc.desc: FFIAbilityDelegatorAddAbilityStageMonitor test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorAddAbilityStageMonitorTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorAddAbilityStageMonitorTest_001 start";

    int64_t id = 0;
    int64_t monitorId = 0;
    std::string srcEntrance = "test";
    std::string moduleName = "test";
    EXPECT_EQ(FFIAbilityDelegatorAddAbilityStageMonitor(id, monitorId, moduleName.c_str(), srcEntrance.c_str()),
        RETURN_VALUE2);

    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    monitorId = id;
    g_stageMonitorRecord.emplace(monitorId, nullptr);
    EXPECT_EQ(FFIAbilityDelegatorAddAbilityStageMonitor(id, monitorId, moduleName.c_str(), srcEntrance.c_str()),
        RETURN_VALUE1);

    g_stageMonitorRecord.clear();
    EXPECT_EQ(FFIAbilityDelegatorAddAbilityStageMonitor(id, monitorId, moduleName.c_str(), srcEntrance.c_str()), 0);
    EXPECT_EQ(g_stageMonitorRecord.size(), 1);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorAddAbilityStageMonitorTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorRemoveAbilityStageMonitorTest_001
 * @tc.desc: FFIAbilityDelegatorRemoveAbilityStageMonitor test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorRemoveAbilityStageMonitorTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorRemoveAbilityStageMonitorTest_001 start";

    int64_t id = 0;
    int64_t monitorId = 0;
    std::string srcEntrance = "test";
    std::string moduleName = "test";
    EXPECT_EQ(FFIAbilityDelegatorRemoveAbilityStageMonitor(id, monitorId, moduleName.c_str(), srcEntrance.c_str()),
        RETURN_VALUE2);

    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    monitorId = id;
    g_stageMonitorRecord.emplace(monitorId, nullptr);
    EXPECT_EQ(FFIAbilityDelegatorRemoveAbilityStageMonitor(id, monitorId, moduleName.c_str(), srcEntrance.c_str()),
        RETURN_VALUE1);

    g_stageMonitorRecord.at(monitorId) = std::make_shared<CJAbilityStageMonitor>("", "", 0);
    EXPECT_EQ(FFIAbilityDelegatorRemoveAbilityStageMonitor(id, monitorId, moduleName.c_str(), srcEntrance.c_str()), 0);
    EXPECT_EQ(g_stageMonitorRecord.size(), 0);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorRemoveAbilityStageMonitorTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorWaitAbilityStageMonitorTest_001
 * @tc.desc: FFIAbilityDelegatorWaitAbilityStageMonitor test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorWaitAbilityStageMonitorTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorWaitAbilityStageMonitorTest_001 start";

    int64_t id = 0;
    int64_t stageMonitorId = 0;
    std::string srcEntrance = "test";
    std::string moduleName = "test";
    CJAbilityStageInfo abilityStageInfo;
    abilityStageInfo.moduleName = srcEntrance.c_str();
    abilityStageInfo.srcEntrance = moduleName.c_str();
    int64_t abilityStageId = 0;
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityStageMonitor(id, stageMonitorId, abilityStageInfo, &abilityStageId),
        RETURN_VALUE2);

    abilityStageId = 1;
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityStageMonitor(id, stageMonitorId, abilityStageInfo, &abilityStageId),
        RETURN_VALUE2);

    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    stageMonitorId = id;
    g_stageMonitorRecord.emplace(stageMonitorId, nullptr);
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityStageMonitor(id, stageMonitorId, abilityStageInfo, &abilityStageId),
        RETURN_VALUE1);
    EXPECT_EQ(abilityStageId, 0);

    abilityStageId = 1;
    g_stageMonitorRecord.clear();
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityStageMonitor(id, stageMonitorId, abilityStageInfo, &abilityStageId),
        RETURN_VALUE2);
    EXPECT_EQ(g_stageMonitorRecord.size(), 1);
    EXPECT_EQ(abilityStageId, 0);

    abilityStageId = 1;
    auto cjMonitor = std::make_shared<CJAbilityStageMonitor>("", "", 0);
    cjMonitor->matchedAbilityStage_ = std::make_shared<CJDelegatorAbilityStageProperty>();
    g_stageMonitorRecord.at(stageMonitorId) = cjMonitor;
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityStageMonitor(id, stageMonitorId, abilityStageInfo, &abilityStageId), 0);

    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorWaitAbilityStageMonitorTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeoutTest_001
 * @tc.desc: FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeout test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeoutTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeoutTest_001 start";

    int64_t id = 0;
    int64_t stageMonitorId = 0;
    std::string srcEntrance = "test";
    std::string moduleName = "test";
    CJAbilityStageInfo abilityStageInfo;
    abilityStageInfo.moduleName = srcEntrance.c_str();
    abilityStageInfo.srcEntrance = moduleName.c_str();
    int64_t abilityStageId = 0;
    int64_t timeout = 1;

    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeout(
                  id, stageMonitorId, abilityStageInfo, timeout, &abilityStageId),
        RETURN_VALUE2);

    abilityStageId = 1;
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeout(
                  id, stageMonitorId, abilityStageInfo, timeout, &abilityStageId),
        RETURN_VALUE2);

    auto cjDelegatorTest = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegatorTest, nullptr);
    id = cjDelegatorTest->GetID();
    stageMonitorId = id;
    g_stageMonitorRecord.emplace(stageMonitorId, nullptr);
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeout(
                  id, stageMonitorId, abilityStageInfo, timeout, &abilityStageId),
        RETURN_VALUE1);
    EXPECT_EQ(abilityStageId, 0);

    abilityStageId = 1;
    g_stageMonitorRecord.clear();
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeout(
                  id, stageMonitorId, abilityStageInfo, timeout, &abilityStageId),
        RETURN_VALUE2);
    EXPECT_EQ(g_stageMonitorRecord.size(), 1);
    EXPECT_EQ(abilityStageId, 0);

    abilityStageId = 1;
    auto cjMonitor = std::make_shared<CJAbilityStageMonitor>("", "", 0);
    cjMonitor->matchedAbilityStage_ = std::make_shared<CJDelegatorAbilityStageProperty>();
    g_stageMonitorRecord.at(stageMonitorId) = cjMonitor;
    EXPECT_EQ(FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeout(
                  id, stageMonitorId, abilityStageInfo, timeout, &abilityStageId),
        0);

    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorWaitAbilityStageMonitorWithTimeoutTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorRegistryGetAbilityDelegatorTest_001
 * @tc.desc: FFIAbilityDelegatorRegistryGetAbilityDelegator test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorRegistryGetAbilityDelegatorTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorRegistryGetAbilityDelegatorTest_001 start";

    EXPECT_EQ(FFIAbilityDelegatorRegistryGetAbilityDelegator(), 1);

    std::shared_ptr<IAbilityDelegator> delegator = std::make_shared<CJAbilityDelegatorImpl>(nullptr, nullptr, nullptr);
    EXPECT_NE(delegator, nullptr);
    AppExecFwk::AbilityDelegatorRegistry::RegisterInstance(delegator, nullptr);
    int64_t id = FFIAbilityDelegatorRegistryGetAbilityDelegator();
    EXPECT_NE(id, 1);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorRegistryGetAbilityDelegatorTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorStartAbilityTest_001
 * @tc.desc: FFIAbilityDelegatorStartAbility test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorStartAbilityTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorStartAbilityTest_001 start";

    int64_t id = 0;
    AAFwk::Want want;
    WantHandle wantHandle = reinterpret_cast<WantHandle>(&want);
    EXPECT_EQ(FFIAbilityDelegatorStartAbility(id, wantHandle), 1);

    auto cjDelegator = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegator, nullptr);
    id = cjDelegator->GetID();
    EXPECT_NE(FFIAbilityDelegatorStartAbility(id, wantHandle), 1);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorStartAbilityTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorExecuteShellCommandTest_001
 * @tc.desc: FFIAbilityDelegatorExecuteShellCommand test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorExecuteShellCommandTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorExecuteShellCommandTest_001 start";

    int64_t id = 0;
    std::string cmd = "test";
    int64_t timeoutSec = 1;
    EXPECT_EQ(FFIAbilityDelegatorExecuteShellCommand(id, cmd.c_str(), timeoutSec), 1);

    auto cjDelegator = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegator, nullptr);
    id = cjDelegator->GetID();
    EXPECT_NE(FFIAbilityDelegatorExecuteShellCommand(id, cmd.c_str(), timeoutSec), 1);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorExecuteShellCommandTest_001 end";
}

/**
 * @tc.name: FFIGetExitCodeTest_001
 * @tc.desc: FFIGetExitCode test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIGetExitCodeTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIGetExitCodeTest_001 start";

    int64_t id = 0;
    EXPECT_EQ(FFIGetExitCode(id), 1);

    int32_t exitCode = 0;
    std::string stdResult = "test";
    std::shared_ptr<AppExecFwk::ShellCmdResult> shellCmdResult =
        std::make_shared<AppExecFwk::ShellCmdResult>(exitCode, stdResult);
    EXPECT_NE(shellCmdResult, nullptr);
    auto cjShellCmdResult = FFI::FFIData::Create<CJShellCmdResult>(shellCmdResult);
    EXPECT_NE(cjShellCmdResult, nullptr);
    id = cjShellCmdResult->GetID();
    EXPECT_EQ(FFIGetExitCode(id), 0);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIGetExitCodeTest_001 end";
}

/**
 * @tc.name: FFIGetStdResultTest_001
 * @tc.desc: FFIGetStdResult test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIGetStdResultTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIGetStdResultTest_001 start";

    int64_t id = 0;
    EXPECT_EQ(FFIGetStdResult(id), nullptr);

    int32_t exitCode = 0;
    std::string stdResult = "test";
    std::shared_ptr<AppExecFwk::ShellCmdResult> shellCmdResult =
        std::make_shared<AppExecFwk::ShellCmdResult>(exitCode, stdResult);
    EXPECT_NE(shellCmdResult, nullptr);
    auto cjShellCmdResult = FFI::FFIData::Create<CJShellCmdResult>(shellCmdResult);
    EXPECT_NE(cjShellCmdResult, nullptr);
    id = cjShellCmdResult->GetID();
    EXPECT_EQ(FFIGetStdResult(id), stdResult);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIGetStdResultTest_001 end";
}

/**
 * @tc.name: FFIDumpTest_001
 * @tc.desc: FFIDump test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIDumpTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIDumpTest_001 start";

    int64_t id = 0;
    EXPECT_EQ(FFIDump(id), nullptr);

    int32_t exitCode = 0;
    std::string stdResult = "test";
    std::shared_ptr<AppExecFwk::ShellCmdResult> shellCmdResult =
        std::make_shared<AppExecFwk::ShellCmdResult>(exitCode, stdResult);
    EXPECT_NE(shellCmdResult, nullptr);
    auto cjShellCmdResult = FFI::FFIData::Create<CJShellCmdResult>(shellCmdResult);
    EXPECT_NE(cjShellCmdResult, nullptr);
    id = cjShellCmdResult->GetID();
    EXPECT_NE(FFIDump(id), nullptr);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIDumpTest_001 end";
}

/**
 * @tc.name: FFIAbilityDelegatorApplicationContextTest_001
 * @tc.desc: FFIAbilityDelegatorApplicationContext test
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityDelegatorNewTest, FFIAbilityDelegatorApplicationContextTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFIAbilityDelegatorApplicationContextTest_001 start";

    int64_t id = 0;
    EXPECT_EQ(FFIAbilityDelegatorApplicationContext(id), 1);

    auto cjDelegator = FFI::FFIData::Create<CJAbilityDelegator>(abilityDelegatorImpl_);
    EXPECT_NE(cjDelegator, nullptr);
    id = cjDelegator->GetID();
    EXPECT_NE(FFIAbilityDelegatorApplicationContext(id), 1);
    FFI::FFIData::Release(id);

    GTEST_LOG_(INFO) << "FFIAbilityDelegatorApplicationContextTest_001 end";
}
} // namespace AbilityDelegatorCJ
} // namespace OHOS