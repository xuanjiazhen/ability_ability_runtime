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

#include <algorithm>
#include <string>
#include <unistd.h>
#include <vector>

#include "singleton.h"

#define private public
#define protected public
#include "startup_manager.h"
#undef private
#undef protected
#include "native_module_collector.h"
#include "mock_native_module_manager.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace {
const std::string PREDICT_TEST_CACHE_DIR = "/data/local/tmp/native_module_collect_ut";

void PreparePredictPreloadDir()
{
    ::mkdir("/data/local/tmp", 0755);
    ::mkdir(PREDICT_TEST_CACHE_DIR.c_str(), 0755);
}

// Seed the predict file with the given system modules via the collector.
void SeedPredictFile(const std::vector<std::string>& modules)
{
    MockSetLoadedSystemNativeModules(modules);
    NativeModuleCollector::GetInstance().CollectAndPersist();
}
} // namespace

class PredictPreloadTaskTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override;
    void TearDown() override;
};

void PredictPreloadTaskTest::SetUp()
{
    PreparePredictPreloadDir();
    MockResetNativeModuleManagerState();
    MockSetNativeModuleManager(reinterpret_cast<NativeModuleManager*>(1));
    auto& collector = NativeModuleCollector::GetInstance();
    CollectConfig config;
    collector.Init(PREDICT_TEST_CACHE_DIR, "entry", config); // idempotent
    collector.SetEnabled(true);
    // force the predict-preload switch on (bypass the real system parameter)
    auto sm = OHOS::DelayedSingleton<StartupManager>::GetInstance();
    sm->predictPreloadSwitchInited_ = true;
    sm->predictPreloadEnabled_ = true;
}

void PredictPreloadTaskTest::TearDown() {}

/**
 * @tc.number: PredictPreloadTask_0100
 * @tc.name: RunOnNonMainThread
 * @tc.desc: AC-3: predicted system modules are preloaded off the main thread
 */
HWTEST_F(PredictPreloadTaskTest, RunOnNonMainThread_0100, Function | MediumTest | Level1)
{
    SeedPredictFile({ "mX" });

    auto sm = OHOS::DelayedSingleton<StartupManager>::GetInstance();
    ASSERT_NE(sm, nullptr);
    int callerTid = static_cast<int>(gettid());
    int32_t ret = sm->RunPredictPreloadSystemSoTask(PREDICT_TEST_CACHE_DIR, "entry");
    EXPECT_EQ(ret, OHOS::ERR_OK);

    // wait for the async preload to execute on the preload thread
    for (int i = 0; i < 100 && MockGetPreloadCalls().empty(); ++i) {
        usleep(10 * 1000);
    }
    auto calls = MockGetPreloadCalls();
    ASSERT_FALSE(calls.empty());
    EXPECT_EQ(calls.front(), "mX");
    int preloadTid = MockGetPreloadCallTid();
    EXPECT_NE(preloadTid, 0);
    EXPECT_NE(preloadTid, callerTid);
}

/**
 * @tc.number: PredictPreloadTask_0200
 * @tc.name: SingleModuleFailureContinue
 * @tc.desc: AC-7: a single module preload failure logs a fault event and continues the rest
 */
HWTEST_F(PredictPreloadTaskTest, SingleModuleFailureContinue_0200, Function | MediumTest | Level1)
{
    SeedPredictFile({ "mA", "mB", "mC" });
    MockSetPreloadFailNames({ "mB" }); // only mB fails

    auto sm = OHOS::DelayedSingleton<StartupManager>::GetInstance();
    ASSERT_NE(sm, nullptr);
    sm->DoPredictPreloadSystemSo(); // direct call bypasses the switch gate

    auto calls = MockGetPreloadCalls();
    // every predicted module was attempted; mB's failure did not stop mC
    EXPECT_EQ(calls.size(), 3u);
    EXPECT_NE(std::find(calls.begin(), calls.end(), "mA"), calls.end());
    EXPECT_NE(std::find(calls.begin(), calls.end(), "mB"), calls.end());
    EXPECT_NE(std::find(calls.begin(), calls.end(), "mC"), calls.end());
}

/**
 * @tc.number: PredictPreloadTask_0300
 * @tc.name: StoppedSkipsPreload
 * @tc.desc: AC-3: prediction preload honors the shared auto-preload stop mechanism (autoPreloadSoStopped_)
 */
HWTEST_F(PredictPreloadTaskTest, StoppedSkipsPreload_0300, Function | MediumTest | Level1)
{
    SeedPredictFile({ "mS1", "mS2" });
    auto sm = OHOS::DelayedSingleton<StartupManager>::GetInstance();
    ASSERT_NE(sm, nullptr);
    // force the shared stop flag on; prediction preload must not run
    sm->autoPreloadSoStopped_ = true;
    int32_t ret = sm->RunPredictPreloadSystemSoTask(PREDICT_TEST_CACHE_DIR, "entry");
    EXPECT_EQ(ret, OHOS::ERR_OK);
    for (int i = 0; i < 20 && MockGetPreloadCalls().empty(); ++i) {
        usleep(10 * 1000);
    }
    EXPECT_TRUE(MockGetPreloadCalls().empty());
    sm->autoPreloadSoStopped_ = false; // restore
}
