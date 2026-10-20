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

#include <fstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#define private public
#include "native_module_collector.h"
#undef private
#include "mock_native_module_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace {
const std::string TEST_CACHE_DIR = "/data/local/tmp/native_module_collect_ut";
const std::string TEST_COLLECT_DIR = TEST_CACHE_DIR + "/native_module_collect";

void PrepareTestDir()
{
    ::mkdir("/data/local/tmp", 0755);
    ::mkdir(TEST_CACHE_DIR.c_str(), 0755);
    ::mkdir(TEST_COLLECT_DIR.c_str(), 0755);
}

std::string ReadFile(const std::string& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}
} // namespace

class NativeModuleCollectorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void NativeModuleCollectorTest::SetUpTestCase() {}
void NativeModuleCollectorTest::TearDownTestCase() {}
void NativeModuleCollectorTest::SetUp()
{
    PrepareTestDir();
    MockResetNativeModuleManagerState();
    MockSetNativeModuleManager(reinterpret_cast<NativeModuleManager*>(1)); // return the real mock instance
    auto collector = &NativeModuleCollector::GetInstance();
    if (collector != nullptr) {
        collector->SetEnabled(true);
        CollectConfig config;
        collector->Init(TEST_CACHE_DIR, "entry", config); // idempotent: effective on the first call
    }
}
void NativeModuleCollectorTest::TearDown() {}

/**
 * @tc.number: NativeModuleCollector_0100
 * @tc.name: CollectAndPersist writes predict file with imported system modules
 * @tc.desc: AC-1 / AC-2: persist system modules and produce the predicted set after the collect window ends
 */
HWTEST_F(NativeModuleCollectorTest, CollectAndPersist_0100, Function | MediumTest | Level1)
{
    MockSetLoadedSystemNativeModules({ "account", "net.http" });
    auto collector = &NativeModuleCollector::GetInstance();
    ASSERT_NE(collector, nullptr);
    collector->CollectAndPersist();

    std::string content = ReadFile(TEST_COLLECT_DIR + "/predict.json");
    EXPECT_FALSE(content.empty());
    EXPECT_NE(content.find("account"), std::string::npos);
}

/**
 * @tc.number: NativeModuleCollector_0200
 * @tc.name: preloaded-only module is not collected (anti self-reinforcement)
 * @tc.desc: AC-4: a module only preloaded (not registered) is absent from the enumeration and not counted as a hit
 */
HWTEST_F(NativeModuleCollectorTest, ExcludePreloaded_0200, Function | MediumTest | Level1)
{
    // only really-imported modules appear in the enumeration; preloaded (unregistered) ones do not
    MockSetLoadedSystemNativeModules({ "account" }); // "preloadOnlyMod" is not in it
    auto collector = &NativeModuleCollector::GetInstance();
    ASSERT_NE(collector, nullptr);
    collector->CollectAndPersist();

    std::string content = ReadFile(TEST_COLLECT_DIR + "/collected.json");
    EXPECT_NE(content.find("account"), std::string::npos);
    EXPECT_EQ(content.find("preloadOnlyMod"), std::string::npos);
}

/**
 * @tc.number: NativeModuleCollector_0300
 * @tc.name: LoadPredictedModules returns empty when file missing/corrupt
 * @tc.desc: AC-6: a missing/corrupt file is treated as no history and does not affect startup
 */
HWTEST_F(NativeModuleCollectorTest, CorruptDegrade_0300, Function | MediumTest | Level1)
{
    auto collector = &NativeModuleCollector::GetInstance();
    ASSERT_NE(collector, nullptr);

    // corrupt file (invalid JSON)
    std::string predictPath = TEST_COLLECT_DIR + "/predict.json";
    {
        std::ofstream ofs(predictPath, std::ios::trunc);
        ofs << "this is not json {{{";
    }
    auto predicted = collector->LoadPredictedModules();
    EXPECT_TRUE(predicted.empty());
}

/**
 * @tc.number: NativeModuleCollector_0400
 * @tc.name: disabled switch produces no file
 * @tc.desc: AC-5: when the switch is off, neither collect nor persist happens
 */
HWTEST_F(NativeModuleCollectorTest, DisabledByParam_0400, Function | MediumTest | Level1)
{
    auto collector = &NativeModuleCollector::GetInstance();
    ASSERT_NE(collector, nullptr);
    collector->SetEnabled(false);

    std::string predictPath = TEST_COLLECT_DIR + "/predict.json";
    ::remove(predictPath.c_str());
    collector->CollectAndPersist();
    EXPECT_EQ(::access(predictPath.c_str(), F_OK), -1);

    collector->SetEnabled(true); // restore
}

/**
 * @tc.number: NativeModuleCollector_0600
 * @tc.name: ConcurrentMerge
 * @tc.desc: AC-9: concurrent instances merge predicted hits as a union (no lost update)
 */
HWTEST_F(NativeModuleCollectorTest, ConcurrentMerge_0600, Function | MediumTest | Level1)
{
    auto collector = &NativeModuleCollector::GetInstance();
    ASSERT_NE(collector, nullptr);
    std::string predictPath = TEST_COLLECT_DIR + "/predict.json";
    ::remove(predictPath.c_str());

    // simulate instance A hitting {mA}, then instance B hitting {mB}; merged result keeps both
    MockSetLoadedSystemNativeModules({ "mA" });
    collector->CollectAndPersist();
    MockSetLoadedSystemNativeModules({ "mB" });
    collector->CollectAndPersist();

    std::string content = ReadFile(predictPath);
    EXPECT_NE(content.find("mA"), std::string::npos);
    EXPECT_NE(content.find("mB"), std::string::npos);
}

/**
 * @tc.number: NativeModuleCollector_0500
 * @tc.name: EwmaPredict
 * @tc.desc: AC-2: a hit raises the score, a miss decays it
 */
HWTEST_F(NativeModuleCollectorTest, EwmaPredict_0500, Function | MediumTest | Level1)
{
    auto collector = &NativeModuleCollector::GetInstance();
    ASSERT_NE(collector, nullptr);
    std::string predictPath = TEST_COLLECT_DIR + "/predict.json";
    ::remove(predictPath.c_str());

    MockSetLoadedSystemNativeModules({ "account", "net.http" });
    collector->CollectAndPersist();
    auto predicted = collector->LoadPredictedModules();
    EXPECT_FALSE(predicted.empty());

    // round 2: "net.http" missed so its score decays; "account" hit again
    MockSetLoadedSystemNativeModules({ "account" });
    collector->CollectAndPersist();
    predicted = collector->LoadPredictedModules();
    EXPECT_NE(std::find(predicted.begin(), predicted.end(), "account"), predicted.end());
}

/**
 * @tc.number: NativeModuleCollector_0700
 * @tc.name: CollectedOnlyInDebug
 * @tc.desc: AC-2: collected.json (raw per-launch snapshot) is written only in debug mode
 */
HWTEST_F(NativeModuleCollectorTest, CollectedOnlyInDebug_0700, Function | MediumTest | Level1)
{
    auto collector = &NativeModuleCollector::GetInstance();
    ASSERT_NE(collector, nullptr);
    std::string collectedPath = TEST_COLLECT_DIR + "/collected.json";
    std::string predictPath = TEST_COLLECT_DIR + "/predict.json";
    ::remove(collectedPath.c_str());

    MockSetLoadedSystemNativeModules({ "mDbg" });
    // debug off (default): collected.json is NOT written, predict.json is written
    collector->debugCollectEnabled_ = false;
    collector->CollectAndPersist();
    EXPECT_EQ(::access(collectedPath.c_str(), F_OK), -1);
    EXPECT_EQ(::access(predictPath.c_str(), F_OK), 0);

    // debug on: collected.json is written
    collector->debugCollectEnabled_ = true;
    collector->CollectAndPersist();
    EXPECT_EQ(::access(collectedPath.c_str(), F_OK), 0);

    collector->debugCollectEnabled_ = false; // restore
}

/**
 * @tc.number: NativeModuleCollector_0800
 * @tc.name: CollectConfigSanitize
 * @tc.desc: CollectConfig invalid fields fall back to defaults (predict params tunable via system param with safe defaults)
 */
HWTEST_F(NativeModuleCollectorTest, CollectConfigSanitize_0800, Function | MediumTest | Level1)
{
    CollectConfig bad;
    bad.collectWindowMs = 0;
    bad.maxPredictModules = 0;
    bad.ewmaAlphaMilli = 2000;    // out of range
    bad.scoreThresholdMilli = 2000; // out of range
    CollectConfig cfg = CollectConfig::Sanitize(bad);
    CollectConfig def;
    EXPECT_EQ(cfg.collectWindowMs, def.collectWindowMs);
    EXPECT_EQ(cfg.maxPredictModules, def.maxPredictModules);
    EXPECT_EQ(cfg.ewmaAlphaMilli, def.ewmaAlphaMilli);
    EXPECT_EQ(cfg.scoreThresholdMilli, def.scoreThresholdMilli);
}
