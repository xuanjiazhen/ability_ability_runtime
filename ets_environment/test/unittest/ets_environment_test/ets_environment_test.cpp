/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <cstdarg>
#include <gtest/gtest.h>
#include <gtest/hwext/gtest-multithread.h>
#include <string>

#include "ohos_ets_environment_impl.h"
#include "runtime.h"
#define private public
#include "ets_environment.h"
#undef private
#include "ets_environment_impl.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace {
bool g_callbackModuleFlag;
}

namespace OHOS {
namespace EtsEnv {
const std::string TEST_ABILITY_NAME = "ContactsDataAbility";

class EtsEnvironmentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void EtsEnvironmentTest::SetUpTestCase() {}

void EtsEnvironmentTest::TearDownTestCase() {}

void EtsEnvironmentTest::SetUp() {}

void EtsEnvironmentTest::TearDown() {}

namespace {
void CallBackModuleFunc()
{
    g_callbackModuleFlag = true;
}
} // namespace

/**
 * @tc.name: LoadBootPathFile_0100
 * @tc.desc: LoadBootPathFile.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadBootPathFile_0100, TestSize.Level0)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    auto etsEnv =
        std::make_shared<ETSEnvironment>(std::make_unique<AbilityRuntime::OHOSEtsEnvironmentImpl>(eventRunner));
    ASSERT_NE(etsEnv, nullptr);
    std::string str = "LoadBootPathFile";
    bool bVal = etsEnv->LoadBootPathFile(str);
    EXPECT_EQ(bVal, true);
}
/**
 * @tc.name: LoadRuntimeApis_0100
 * @tc.desc: LoadRuntimeApis.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, LoadRuntimeApis_0100, TestSize.Level0)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    auto etsEnv =
        std::make_shared<ETSEnvironment>(std::make_unique<AbilityRuntime::OHOSEtsEnvironmentImpl>(eventRunner));
    ASSERT_NE(etsEnv, nullptr);
    bool bVal = etsEnv->LoadRuntimeApis();
    EXPECT_EQ(bVal, true);
}

/**
 * @tc.name: PostTask_0100
 * @tc.desc: PostTask
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, PostTask_0100, TestSize.Level0)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    auto etsEnv =
        std::make_shared<ETSEnvironment>(std::make_unique<AbilityRuntime::OHOSEtsEnvironmentImpl>(eventRunner));
    ASSERT_NE(etsEnv, nullptr);

    std::function<void()> task = CallBackModuleFunc;
    std::string name = "NAME";
    int64_t delayTime = 10;
    etsEnv->PostTask(task, name, delayTime);
}

/**
 * @tc.name: PostSyncTask_0100
 * @tc.desc: PostSyncTask
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, PostSyncTask_0100, TestSize.Level0)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    auto etsEnv =
        std::make_shared<ETSEnvironment>(std::make_unique<AbilityRuntime::OHOSEtsEnvironmentImpl>(eventRunner));
    ASSERT_NE(etsEnv, nullptr);

    std::function<void()> task = CallBackModuleFunc;
    std::string name = "NAME";
    etsEnv->PostSyncTask(task, name);
}

/**
 * @tc.name: RemoveTask_0100
 * @tc.desc: RemoveTask
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, RemoveTask_0100, TestSize.Level0)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    auto etsEnv =
        std::make_shared<ETSEnvironment>(std::make_unique<AbilityRuntime::OHOSEtsEnvironmentImpl>(eventRunner));
    ASSERT_NE(etsEnv, nullptr);

    std::string name = "NAME";
    etsEnv->RemoveTask(name);
}

/**
 * @tc.name: InitLoop_0100
 * @tc.desc: InitLoop
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, InitLoop_0100, TestSize.Level0)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    auto etsEnv =
        std::make_shared<ETSEnvironment>(std::make_unique<AbilityRuntime::OHOSEtsEnvironmentImpl>(eventRunner));
    ASSERT_NE(etsEnv, nullptr);

    bool bIsStage = true;
    bool bVal = etsEnv->InitLoop(bIsStage);
    EXPECT_TRUE(bVal);
}

/**
 * @tc.name: DeInitLoop_0100
 * @tc.desc: DeInitLoop
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, DeInitLoop_0100, TestSize.Level0)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    auto etsEnv =
        std::make_shared<ETSEnvironment>(std::make_unique<AbilityRuntime::OHOSEtsEnvironmentImpl>(eventRunner));
    ASSERT_NE(etsEnv, nullptr);

    etsEnv->DeInitLoop();
}

/**
 * @tc.name: ReInitUVLoop_0100
 * @tc.desc: ReInitUVLoop.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, ReInitUVLoop_0100, TestSize.Level0)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);
    auto etsEnv =
        std::make_shared<ETSEnvironment>(std::make_unique<AbilityRuntime::OHOSEtsEnvironmentImpl>(eventRunner));
    bool result = etsEnv->ReInitUVLoop();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GetAniEnv_0100
 * @tc.desc: GetAniEnv.
 * @tc.type: FUNC
 */
HWTEST_F(EtsEnvironmentTest, GetAniEnv_0100, TestSize.Level0)
{
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner = AppExecFwk::EventRunner::Create(TEST_ABILITY_NAME);

    auto etsEnv =
        std::make_shared<ETSEnvironment>(std::make_unique<AbilityRuntime::OHOSEtsEnvironmentImpl>(eventRunner));
    ETSEnvironment::VMEntry vMEntryOld = etsEnv->vmEntry_;
    ETSEnvironment::VMEntry vmEntry;
    vmEntry.ani_env = nullptr;
    etsEnv->vmEntry_ = vmEntry;
    auto result = etsEnv->GetAniEnv();
    EXPECT_EQ(result, nullptr);
    etsEnv->vmEntry_ = vMEntryOld;
}

} // namespace EtsEnv
} // namespace OHOS