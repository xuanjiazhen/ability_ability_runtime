/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <securec.h>

#include "cj_ability_stage.h"
#include "cj_runtime.h"
#include "hilog_wrapper.h"
#include "runtime.h"
#include "cj_ability_stage_object.h"
#include "cj_ability_delegator_registry.h"
#include "cj_application_context.h"
#include "application_context.h"
#include "runner_runtime/cj_test_runner.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FFI;
using namespace OHOS::AbilityDelegatorCJ;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;
using namespace OHOS::RunnerRuntime;
namespace OHOS {
namespace AbilityRuntime {
namespace {
const std::string KEY_TEST_BUNDLE_NAME = "-p";
const std::string VALUE_TEST_BUNDLE_NAME = "com.example.myapplication";
const std::string KEY_TEST_RUNNER_CLASS = "-s unittest";
const std::string VALUE_TEST_RUNNER_CLASS = "JSUserTestRunner";
const std::string KEY_TEST_CASE = "-s class";
const std::string VALUE_TEST_CASE = "ohos.acts.aafwk.ability.test.ConstructorTest#testDataAbilityOtherFunction0010";
const std::string KEY_TEST_WAIT_TIMEOUT = "-w";
const std::string VALUE_TEST_WAIT_TIMEOUT = "50";
const std::string KEY_TEST_DEBUG { "-D" };
const std::string VALUE_TEST_DEBUG { "true" };
} // namespace
class CjAbilityStageTest : public testing::Test {
public:
    CjAbilityStageTest()
    {}
    ~CjAbilityStageTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
    std::unique_ptr<AbilityRuntime::CJAbilityStage> cjAbilityStage_ = nullptr;
    std::unique_ptr<AbilityRuntime::CJAbilityStage> initCjAbilityStage_ = nullptr;
};

void (*g_registerFunc)(CJAbilityStageFuncs* result) = [](CJAbilityStageFuncs *result) {
        result->LoadAbilityStage = [](const char *moduleName) -> int64_t { return moduleName[0] == '0' ? 0 : 1; };
        result->ReleaseAbilityStage = [](int64_t handle) {};
        result->AbilityStageOnCreate = [](int64_t handle) {};
        result->AbilityStageOnAcceptWant = [](int64_t handle, OHOS::AAFwk::Want *want) -> char* {
            std::string str = "Hello, world!";
            char* cstr = new char[str.length() + 1];
            if (memcpy_s(cstr, str.length() + 1, str.c_str(), str.size()) != EOK) {
                delete[] cstr;
                return nullptr;
            }
            return cstr;
            };
        result->AbilityStageOnConfigurationUpdated = [](int64_t id, CJConfiguration configuration) {};
        result->AbilityStageOnMemoryLevel = [](int64_t id, int32_t level) {};
    };
void CjAbilityStageTest::SetUpTestCase(void)
{}

void CjAbilityStageTest::TearDownTestCase(void)
{}

void CjAbilityStageTest::SetUp(void)
{
    // Construct a common ability delegator firstly.
    std::map<std::string, std::string> paras;
    paras.emplace(KEY_TEST_BUNDLE_NAME, VALUE_TEST_BUNDLE_NAME);
    paras.emplace(KEY_TEST_RUNNER_CLASS, VALUE_TEST_RUNNER_CLASS);
    paras.emplace(KEY_TEST_CASE, VALUE_TEST_CASE);
    paras.emplace(KEY_TEST_WAIT_TIMEOUT, VALUE_TEST_WAIT_TIMEOUT);
    paras.emplace(KEY_TEST_DEBUG, VALUE_TEST_DEBUG);

    Want want;
    for (auto para : paras) {
        want.SetParam(para.first, para.second);
    }

    auto delegatorArgs_ = std::make_shared<AbilityDelegatorArgs>(want);
    AbilityRuntime::Runtime::Options options;
    BundleInfo bundleInfo;
    auto testRunner = CJTestRunner::Create(AbilityRuntime::Runtime::Create(options), delegatorArgs_, bundleInfo);
    auto commonDelegator_ = std::make_shared<CJAbilityDelegatorImpl>(
        std::make_shared<AbilityRuntime::ContextImpl>(), std::move(testRunner), nullptr);
    CJAbilityDelegatorRegistry::RegisterInstance(commonDelegator_, delegatorArgs_);
    
    RegisterCJAbilityStageFuncs(g_registerFunc);
    std::string moduleName = "0";
    auto proxy = CJAbilityStageObject::LoadModule(moduleName);
    cjAbilityStage_ = std::make_unique<AbilityRuntime::CJAbilityStage>(proxy);
    auto cjProxy = CJAbilityStageObject::LoadModule("1");
    initCjAbilityStage_ = std::make_unique<AbilityRuntime::CJAbilityStage>(cjProxy);
}

void CjAbilityStageTest::TearDown(void)
{}

/**
 * @tc.name: CjAbilityStageTestCreate_001
 * @tc.desc: CjAbilityStageTest test for Create.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestCreate_001, TestSize.Level0)
{
    AppExecFwk::HapModuleInfo hapModuleInfo;
    std::unique_ptr<Runtime> nullRuntime = nullptr;
    auto cjStage = CJAbilityStage::Create(nullRuntime, hapModuleInfo);
    Runtime::Options options;
    auto runtime = Runtime::Create(options);
    EXPECT_TRUE(runtime != nullptr);
    auto cjAbilityStage = CJAbilityStage::Create(runtime, hapModuleInfo);
    EXPECT_TRUE(cjAbilityStage == nullptr);
}

/**
 * @tc.name: CjAbilityStageTestOnCreate_001
 * @tc.desc: CjAbilityStageTest test for OnCreate.
 * @tc.type: FUNC
*/
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestOnCreate_001, TestSize.Level0)
{
    AAFwk::Want want;
    cjAbilityStage_->OnCreate(want);
    EXPECT_NE(cjAbilityStage_, nullptr);
    initCjAbilityStage_->OnCreate(want);
    EXPECT_NE(initCjAbilityStage_, nullptr);
}

/**
 * @tc.name: CjAbilityStageTestOnAcceptWant_001
 * @tc.desc: CjAbilityStageTest test for OnAcceptWant.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestOnAcceptWant_001, TestSize.Level0)
{
    AAFwk::Want want;
    auto info = cjAbilityStage_->OnAcceptWant(want);
    EXPECT_TRUE(info == "");
    auto ret = initCjAbilityStage_->OnAcceptWant(want);
    EXPECT_TRUE(ret != "");
}

/**
 * @tc.name: CjAbilityStageTestOnMemoryLevel_001
 * @tc.desc: CjAbilityStageTest test for OnMemoryLevel.
 * @tc.type: FUNC
 */
HWTEST_F(CjAbilityStageTest, CjAbilityStageTestOnMemoryLevel_001, TestSize.Level0)
{
    int level = 1;
    cjAbilityStage_->OnMemoryLevel(level);
    EXPECT_NE(cjAbilityStage_, nullptr);
    initCjAbilityStage_->OnMemoryLevel(level);
    EXPECT_NE(initCjAbilityStage_, nullptr);
}

}  // namespace AbilityRuntime
}  // namespace OHOS
