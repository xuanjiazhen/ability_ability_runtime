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

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include <nlohmann/json.hpp>

#include "native_module_collector.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;
using OHOS::AppExecFwk::CollectedModule;
using OHOS::AppExecFwk::ModuleStat;
using OHOS::AppExecFwk::NativeModuleCollector;
using OHOS::AppExecFwk::PredictedModule;
using OHOS::AppExecFwk::StoreMeta;

namespace {
constexpr mode_t STORE_FILE_MODE = 0600;

std::string TempBaseDir(const std::string& caseName)
{
    return (std::filesystem::temp_directory_path() / ("native_module_collector_" + caseName)).string();
}

void RemoveDirRecursively(const std::string& dir)
{
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

mode_t FilePermission(const std::string& path)
{
    struct stat st {};
    if (stat(path.c_str(), &st) != 0) {
        return 0;
    }
    return st.st_mode & 0777;
}

CollectedModule MakeModule(const std::string& name, bool isApp = false, const std::string& relPath = "")
{
    CollectedModule m;
    m.moduleName = name;
    m.relativePath = relPath;
    m.isAppModule = isApp;
    return m;
}

ModuleStat MakeStat(float score, uint32_t lastSeen)
{
    ModuleStat s;
    s.score = score;
    s.lastSeenLaunch = lastSeen;
    return s;
}

constexpr char PARAM_ENABLE[] = "ccm.ability.native_module_collect.enable";
constexpr char PARAM_ALPHA[] = "ccm.ability.native_module_collect.alpha";
constexpr char PARAM_THRESHOLD[] = "ccm.ability.native_module_collect.predict_threshold";
} // namespace

class NativeModuleCollectorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        NativeModuleCollector::ResetStateForTest();
        NativeModuleCollector::SetEnabledForTest(false);
        NativeModuleCollector::ResetParamCacheForTest();
    }
    void TearDown() override
    {
        NativeModuleCollector::SetEnabledForTest(false);
        NativeModuleCollector::ResetStateForTest();
    }
};

// ---------------- store path / naming ----------------

HWTEST_F(NativeModuleCollectorTest, GetStorePath_Format_001, TestSize.Level1)
{
    std::string path = NativeModuleCollector::GetStorePath("/data/storage/el2/base/cache", "com.example.app");
    const std::string prefix = "/data/storage/el2/base/cache/.ohos.ability_runtime/native_modules_";
    EXPECT_EQ(path.compare(0, prefix.size(), prefix), 0);
    const std::string suffix = ".v2.json";
    EXPECT_EQ(path.compare(path.size() - suffix.size(), suffix.size(), suffix), 0);
    std::string hash = path.substr(prefix.size(), path.size() - prefix.size() - suffix.size());
    EXPECT_EQ(hash.size(), 8u);
    for (char c : hash) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }
}

HWTEST_F(NativeModuleCollectorTest, ProcessNameHash_DistinctPerProcess_002, TestSize.Level1)
{
    std::string main = NativeModuleCollector::ProcessNameHash("com.example.app");
    std::string remote = NativeModuleCollector::ProcessNameHash("com.example.app:remote");
    EXPECT_NE(main, remote);
    EXPECT_NE(NativeModuleCollector::GetStorePath("/cache", "com.example.app"),
              NativeModuleCollector::GetStorePath("/cache", "com.example.app:remote"));
}

HWTEST_F(NativeModuleCollectorTest, EnsureDir_NestedAndIdempotent_003, TestSize.Level1)
{
    std::string base = TempBaseDir("ensuredir");
    RemoveDirRecursively(base);
    std::string nested = base + "/a/b/.ohos.ability_runtime";
    EXPECT_TRUE(NativeModuleCollector::EnsureDir(nested));
    EXPECT_TRUE(std::filesystem::is_directory(nested));
    EXPECT_TRUE(NativeModuleCollector::EnsureDir(nested)); // idempotent
    EXPECT_FALSE(NativeModuleCollector::EnsureDir(""));
    RemoveDirRecursively(base);
}

HWTEST_F(NativeModuleCollectorTest, WriteFileWithPermission_ContentAndMode_004, TestSize.Level1)
{
    std::string base = TempBaseDir("writefile");
    RemoveDirRecursively(base);
    ASSERT_TRUE(NativeModuleCollector::EnsureDir(base));
    std::string file = base + "/native_modules_a1b2c3d4.v2.json";
    const std::string content = "{\"version\":2}";
    EXPECT_TRUE(NativeModuleCollector::WriteFileWithPermission(file, content));
    EXPECT_EQ(FilePermission(file), STORE_FILE_MODE);
    ASSERT_EQ(chmod(file.c_str(), 0644), 0); // loosen, then rewrite must tighten back
    EXPECT_TRUE(NativeModuleCollector::WriteFileWithPermission(file, content));
    EXPECT_EQ(FilePermission(file), STORE_FILE_MODE);
    RemoveDirRecursively(base);
}

// ---------------- switch gating ----------------

HWTEST_F(NativeModuleCollectorTest, IsEnabled_DisabledNoFileIO_005, TestSize.Level1)
{
    EXPECT_FALSE(NativeModuleCollector::IsEnabled());
    std::string base = TempBaseDir("disabled");
    RemoveDirRecursively(base);
    ASSERT_TRUE(NativeModuleCollector::EnsureDir(base));
    NativeModuleCollector::AccumulateModules({ MakeModule("ohos.hilog") }, 0);
    EXPECT_FALSE(NativeModuleCollector::SaveNow(base, "com.example.app", 100));
    EXPECT_TRUE(std::filesystem::is_empty(base));
    RemoveDirRecursively(base);
}

// ---------------- EWMA merge ----------------

HWTEST_F(NativeModuleCollectorTest, MergeRound_ColdStart_006, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    NativeModuleCollector::MergeRound({ MakeModule("ohos.hilog"), MakeModule("ohos.window") }, stats, 1, true);
    ASSERT_EQ(stats.size(), 2u);
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 1.0f);
    EXPECT_EQ(stats["ohos.hilog"].lastSeenLaunch, 1u);
}

HWTEST_F(NativeModuleCollectorTest, MergeRound_HitReward_007, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    stats["ohos.hilog"] = MakeStat(0.5f, 1u);
    NativeModuleCollector::MergeRound({ MakeModule("ohos.hilog") }, stats, 2, false);
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 0.4f + 0.6f * 0.5f); // 0.7
    NativeModuleCollector::MergeRound({ MakeModule("ohos.hilog") }, stats, 3, false);
    EXPECT_NEAR(stats["ohos.hilog"].score, 0.82f, 1e-6);
    // brand-new module in steady state starts below the predict threshold
    NativeModuleCollector::MergeRound({ MakeModule("ohos.window") }, stats, 4, false);
    EXPECT_FLOAT_EQ(stats["ohos.window"].score, 0.4f);
}

HWTEST_F(NativeModuleCollectorTest, MergeRound_MissDecay_008, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    stats["ohos.hilog"] = MakeStat(1.0f, 1u);
    NativeModuleCollector::MergeRound({ MakeModule("ohos.window") }, stats, 2, false);
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 0.6f); // decayed, lastSeenLaunch unchanged
    EXPECT_EQ(stats["ohos.hilog"].lastSeenLaunch, 1u);
}

HWTEST_F(NativeModuleCollectorTest, ComputePredicted_Threshold_009, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    stats["in"] = MakeStat(0.5f, 1u);
    stats["out"] = MakeStat(0.499f, 1u);
    EXPECT_EQ(NativeModuleCollector::ComputePredicted(stats).size(), 1u);
}

HWTEST_F(NativeModuleCollectorTest, EvictStats_LowScoreAndLongUnseen_010, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    stats["decayed"] = MakeStat(0.04f, 10u);
    stats["stale"] = MakeStat(0.5f, 1u);   // 17 - 1 = 16 rounds unseen
    stats["border"] = MakeStat(0.5f, 2u);  // 15 rounds, stays
    NativeModuleCollector::EvictStats(stats, 17);
    EXPECT_EQ(stats.count("decayed"), 0u);
    EXPECT_EQ(stats.count("stale"), 0u);
    EXPECT_EQ(stats.count("border"), 1u);
}

HWTEST_F(NativeModuleCollectorTest, EvictStats_CapLimit_011, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    for (int i = 0; i < 1024; ++i) {
        stats["mod_" + std::to_string(i)] = MakeStat(0.5f, 1u);
    }
    stats["lowest"] = MakeStat(0.06f, 1u);
    NativeModuleCollector::EvictStats(stats, 1);
    EXPECT_EQ(stats.size(), 1024u);
    EXPECT_EQ(stats.count("lowest"), 0u);
}

// ---------------- load / rebuild ----------------

HWTEST_F(NativeModuleCollectorTest, LoadStats_RejectInvalid_012, TestSize.Level1)
{
    std::string base = TempBaseDir("loadstats");
    RemoveDirRecursively(base);
    ASSERT_TRUE(NativeModuleCollector::EnsureDir(base));
    std::map<std::string, ModuleStat> stats;
    StoreMeta meta;
    std::string err;
    EXPECT_FALSE(NativeModuleCollector::LoadStats(base + "/missing.json", stats, meta, err));
    std::string corrupted = base + "/corrupted.json";
    ASSERT_TRUE(NativeModuleCollector::WriteFileWithPermission(corrupted, "not a json{{{"));
    EXPECT_FALSE(NativeModuleCollector::LoadStats(corrupted, stats, meta, err));
    std::string wrongVer = base + "/v1.json";
    ASSERT_TRUE(NativeModuleCollector::WriteFileWithPermission(wrongVer,
        "{\"version\":1,\"appVersionCode\":1,\"launchIndex\":1,\"modules\":[]}"));
    EXPECT_FALSE(NativeModuleCollector::LoadStats(wrongVer, stats, meta, err));
    RemoveDirRecursively(base);
}

HWTEST_F(NativeModuleCollectorTest, MergeAndSave_RoundTripAndRelativePath_013, TestSize.Level1)
{
    std::string base = TempBaseDir("roundtrip");
    RemoveDirRecursively(base);
    std::string store = NativeModuleCollector::GetStorePath(base, "com.example.app");
    // only system modules are persisted; relativePath is captured and replayed
    // MergeAndSave trusts its input; app-module filtering happens at the
    // AccumulateModules entry (covered by AccumulateModules_DropsAppModules_017),
    // so here only system modules are fed in.
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, 100,
        { MakeModule("ohos.hilog", false, "@app/feature"),
          MakeModule("ohos.window") }));
    EXPECT_EQ(FilePermission(store), STORE_FILE_MODE);

    std::map<std::string, ModuleStat> stats;
    StoreMeta meta;
    std::string err;
    ASSERT_TRUE(NativeModuleCollector::LoadStats(store, stats, meta, err));
    EXPECT_EQ(meta.launchIndex, 1u);
    EXPECT_EQ(meta.appVersionCode, 100);
    EXPECT_EQ(stats.size(), 2u);
    EXPECT_EQ(stats["ohos.hilog"].relativePath, "@app/feature"); // relativePath round-trips
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 1.0f);

    // no "predicted" field in the file (computed on demand)
    std::ifstream in(store);
    nlohmann::json root = nlohmann::json::parse(in, nullptr, false, true);
    ASSERT_TRUE(root.is_object());
    EXPECT_FALSE(root.contains("predicted"));
    EXPECT_FALSE(root.contains("bundleName")); // redundant identity fields removed

    // second round: only ohos.hilog hits, ohos.window decays
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, 100, { MakeModule("ohos.hilog", false, "@app/feature") }));
    ASSERT_TRUE(NativeModuleCollector::LoadStats(store, stats, meta, err));
    EXPECT_EQ(meta.launchIndex, 2u);
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 1.0f);
    EXPECT_FLOAT_EQ(stats["ohos.window"].score, 0.6f);
    RemoveDirRecursively(base);
}

HWTEST_F(NativeModuleCollectorTest, MergeAndSave_AppUpgradeRebuild_014, TestSize.Level1)
{
    std::string base = TempBaseDir("upgrade");
    RemoveDirRecursively(base);
    std::string store = NativeModuleCollector::GetStorePath(base, "com.example.app");
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, 100, { MakeModule("ohos.hilog") }));
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, 100, { MakeModule("ohos.hilog") }));
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, 200, { MakeModule("ohos.window") })); // upgrade
    std::map<std::string, ModuleStat> stats;
    StoreMeta meta;
    std::string err;
    ASSERT_TRUE(NativeModuleCollector::LoadStats(store, stats, meta, err));
    EXPECT_EQ(meta.launchIndex, 1u); // rebuilt
    EXPECT_EQ(meta.appVersionCode, 200);
    EXPECT_EQ(stats.count("ohos.hilog"), 0u);
    EXPECT_FLOAT_EQ(stats["ohos.window"].score, 1.0f);
    RemoveDirRecursively(base);
}

HWTEST_F(NativeModuleCollectorTest, WriteAtomic_SuccessAndFailure_015, TestSize.Level1)
{
    std::string base = TempBaseDir("atomic");
    RemoveDirRecursively(base);
    ASSERT_TRUE(NativeModuleCollector::EnsureDir(base));
    std::string file = base + "/native_modules_a1b2c3d4.v2.json";
    EXPECT_TRUE(NativeModuleCollector::WriteAtomic(file, "{\"version\":2}"));
    EXPECT_EQ(FilePermission(file), STORE_FILE_MODE);
    EXPECT_FALSE(std::filesystem::exists(file + ".tmp"));
    EXPECT_FALSE(NativeModuleCollector::WriteAtomic(base + "/no/such/dir/f.json", "{}"));
    EXPECT_FALSE(std::filesystem::exists(base + "/no/such/dir/f.json"));
    RemoveDirRecursively(base);
}

// ---------------- accumulation: dedup / preload / app filter / relativePath ----------------

HWTEST_F(NativeModuleCollectorTest, AccumulateModules_DedupAndPreloadFilter_016, TestSize.Level1)
{
    NativeModuleCollector::SetPreloadModuleKeys({ "ohos.preload" });
    NativeModuleCollector::AccumulateModules(
        { MakeModule("ohos.preload"), MakeModule("ohos.hilog", false, "@app/x") }, 0);
    EXPECT_EQ(NativeModuleCollector::GetSampledCountForTest(), 1u);
    // duplicate refreshes relativePath only
    NativeModuleCollector::AccumulateModules({ MakeModule("ohos.hilog", false, "@app/y") }, 1);
    EXPECT_EQ(NativeModuleCollector::GetSampledCountForTest(), 1u);
    CollectedModule out;
    ASSERT_TRUE(NativeModuleCollector::GetSampledModuleForTest("ohos.hilog", out));
    EXPECT_EQ(out.relativePath, "@app/y");
}

HWTEST_F(NativeModuleCollectorTest, AccumulateModules_DropsAppModules_017, TestSize.Level1)
{
    NativeModuleCollector::AccumulateModules(
        { MakeModule("ohos.hilog"), MakeModule("entry", true), MakeModule("libapp.so", true) }, 0);
    EXPECT_EQ(NativeModuleCollector::GetSampledCountForTest(), 1u);
    CollectedModule out;
    ASSERT_TRUE(NativeModuleCollector::GetSampledModuleForTest("ohos.hilog", out));
    EXPECT_FALSE(out.isAppModule);
    EXPECT_FALSE(NativeModuleCollector::GetSampledModuleForTest("entry", out));
}

HWTEST_F(NativeModuleCollectorTest, SaveNow_AtMostOnce_018, TestSize.Level1)
{
    NativeModuleCollector::SetEnabledForTest(true);
    NativeModuleCollector::SetAsyncSaveForTest(false);
    std::string base = TempBaseDir("saveonce");
    RemoveDirRecursively(base);
    NativeModuleCollector::AccumulateModules({ MakeModule("ohos.hilog") }, 0);
    EXPECT_TRUE(NativeModuleCollector::SaveNow(base, "com.example.app", 100));
    EXPECT_TRUE(std::filesystem::exists(NativeModuleCollector::GetStorePath(base, "com.example.app")));
    EXPECT_FALSE(NativeModuleCollector::SaveNow(base, "com.example.app", 100)); // once per launch
    RemoveDirRecursively(base);
}

HWTEST_F(NativeModuleCollectorTest, SaveNow_AsyncDispatched_019, TestSize.Level1)
{
    NativeModuleCollector::SetEnabledForTest(true);
    NativeModuleCollector::SetAsyncSaveForTest(true);
    std::string base = TempBaseDir("saveasync");
    RemoveDirRecursively(base);
    NativeModuleCollector::AccumulateModules({ MakeModule("ohos.hilog") }, 0);
    // async path: reports success without blocking; content verified by 018 (sync)
    EXPECT_TRUE(NativeModuleCollector::SaveNow(base, "com.example.app", 100));
    RemoveDirRecursively(base);
}

// ---------------- consumer-side predicted preload ----------------

HWTEST_F(NativeModuleCollectorTest, LoadPredictedSystemModules_Filter_020, TestSize.Level1)
{
    std::string base = TempBaseDir("consumer");
    RemoveDirRecursively(base);
    std::string store = NativeModuleCollector::GetStorePath(base, "com.example.app");
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, 100,
        { MakeModule("ohos.hilog", false, "@app/a"), MakeModule("ohos.window") }));
    auto predicted = NativeModuleCollector::LoadPredictedSystemModules(store);
    ASSERT_EQ(predicted.size(), 2u);
    bool foundHilog = false;
    for (const auto& p : predicted) {
        if (p.moduleName == "ohos.hilog") {
            EXPECT_EQ(p.relativePath, "@app/a");
            foundHilog = true;
        }
    }
    EXPECT_TRUE(foundHilog);
    // ohos.window misses: 1.0 -> 0.6 (still >= 0.5, stays predicted)
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, 100, { MakeModule("ohos.hilog", false, "@app/a") }));
    predicted = NativeModuleCollector::LoadPredictedSystemModules(store);
    EXPECT_EQ(predicted.size(), 2u);
    // another miss: 0.6 -> 0.36 (< 0.5, drops out); only ohos.hilog stays predicted
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, 100, { MakeModule("ohos.hilog", false, "@app/a") }));
    predicted = NativeModuleCollector::LoadPredictedSystemModules(store);
    EXPECT_EQ(predicted.size(), 1u);
    RemoveDirRecursively(base);
}

HWTEST_F(NativeModuleCollectorTest, LoadPredictedSystemModules_MissingOrCorrupted_021, TestSize.Level1)
{
    std::string base = TempBaseDir("consumerbad");
    RemoveDirRecursively(base);
    ASSERT_TRUE(NativeModuleCollector::EnsureDir(base));
    EXPECT_TRUE(NativeModuleCollector::LoadPredictedSystemModules(base + "/missing.json").empty());
    std::string corrupted = base + "/corrupted.json";
    ASSERT_TRUE(NativeModuleCollector::WriteFileWithPermission(corrupted, "{broken"));
    EXPECT_TRUE(NativeModuleCollector::LoadPredictedSystemModules(corrupted).empty());
    RemoveDirRecursively(base);
}

// ---------------- anti self-feedback (consumer-preloaded modules never rewarded) ----------------

HWTEST_F(NativeModuleCollectorTest, MergeRound_ConsumerPreloadedDecays_022, TestSize.Level1)
{
    // steady state: module in consumerPreloads is not rewarded; left unseen -> decays
    std::map<std::string, ModuleStat> stats;
    stats["ohos.hilog"] = MakeStat(0.5f, 1u);
    NativeModuleCollector::MergeRound({ MakeModule("ohos.hilog") }, stats, 2, false, { "ohos.hilog" });
    EXPECT_LE(stats["ohos.hilog"].score, 0.5f);          // not rewarded
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 0.3f);    // decayed as unused (0.5*(1-0.4))
    EXPECT_EQ(stats["ohos.hilog"].lastSeenLaunch, 1u);   // not marked as hit
}

HWTEST_F(NativeModuleCollectorTest, MergeRound_NotConsumerPreloaded_Hit_023, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    NativeModuleCollector::MergeRound({ MakeModule("ohos.window") }, stats, 1, false, { "ohos.hilog" });
    EXPECT_FLOAT_EQ(stats["ohos.window"].score, 0.4f); // normal new-module reward
    EXPECT_EQ(stats["ohos.window"].lastSeenLaunch, 1u);
}

HWTEST_F(NativeModuleCollectorTest, MergeRound_ColdStartConsumerPreloadNotHit_024, TestSize.Level1)
{
    // cold start: a consumer-preloaded module must never become a hit (R-9)
    std::map<std::string, ModuleStat> stats;
    NativeModuleCollector::MergeRound({ MakeModule("ohos.hilog") }, stats, 1, true, { "ohos.hilog" });
    ASSERT_EQ(stats.count("ohos.hilog"), 1u);
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 0.0f); // not assigned 1.0; decayed from 0
    EXPECT_EQ(stats["ohos.hilog"].lastSeenLaunch, 0u);
}

HWTEST_F(NativeModuleCollectorTest, SaveNow_ConsumerPreloadNotPredicted_025, TestSize.Level1)
{
    NativeModuleCollector::SetEnabledForTest(true);
    NativeModuleCollector::SetAsyncSaveForTest(false);
    std::string base = TempBaseDir("selffeedback");
    RemoveDirRecursively(base);
    std::string store = NativeModuleCollector::GetStorePath(base, "com.example.app");
    // consumer preloads ohos.hilog; even so it must not enter the predicted set
    NativeModuleCollector::RecordConsumerPreload("ohos.hilog");
    NativeModuleCollector::AccumulateModules({ MakeModule("ohos.hilog") }, 0);
    EXPECT_TRUE(NativeModuleCollector::SaveNow(base, "com.example.app", 100));
    EXPECT_TRUE(NativeModuleCollector::LoadPredictedSystemModules(store).empty());
    RemoveDirRecursively(base);
}

// ---------------- system parameter (CCM) channel ----------------

HWTEST_F(NativeModuleCollectorTest, Params_DefaultsWhenAbsent_026, TestSize.Level1)
{
    EXPECT_FALSE(NativeModuleCollector::IsEnabled());
    const auto params = NativeModuleCollector::GetParams();
    EXPECT_FLOAT_EQ(params.alpha, 0.4f);
    EXPECT_FLOAT_EQ(params.predictThreshold, 0.5f);
    EXPECT_FLOAT_EQ(params.evictScore, 0.05f);
    EXPECT_EQ(params.evictRounds, 16u);
    EXPECT_EQ(params.checkpoint2DelayMs, 5000u);
    EXPECT_FALSE(params.reportHitRate);
}

HWTEST_F(NativeModuleCollectorTest, Params_OverrideAndInvalid_027, TestSize.Level1)
{
    if (!OHOS::system::SetParameter(PARAM_ENABLE, "") ||
        !OHOS::system::SetParameter(PARAM_ALPHA, "") ||
        !OHOS::system::SetParameter(PARAM_THRESHOLD, "")) {
        GTEST_SKIP() << "SetParameter not permitted in this environment";
    }
    // EXPECT so the restore block always runs even on failure
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_ENABLE, "true"));
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_ALPHA, "0.7"));
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_THRESHOLD, "abc")); // invalid
    NativeModuleCollector::ResetParamCacheForTest();
    EXPECT_TRUE(NativeModuleCollector::IsEnabled());
    const auto params = NativeModuleCollector::GetParams();
    EXPECT_FLOAT_EQ(params.alpha, 0.7f);
    EXPECT_FLOAT_EQ(params.predictThreshold, 0.5f); // invalid -> default
    // restore
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_ENABLE, "false"));
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_ALPHA, ""));
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_THRESHOLD, ""));
    NativeModuleCollector::ResetParamCacheForTest();
    EXPECT_FALSE(NativeModuleCollector::IsEnabled());
}

// ---------------- hit-rate metrics ----------------

HWTEST_F(NativeModuleCollectorTest, ComputeHitRateMetrics_028, TestSize.Level1)
{
    std::vector<std::string> predicted = { "a", "b", "c" };
    std::vector<CollectedModule> round = { MakeModule("b"), MakeModule("c"), MakeModule("d") };
    auto metrics = NativeModuleCollector::ComputeHitRateMetrics(predicted, round);
    EXPECT_EQ(metrics.hitCount, 2u);
    EXPECT_FLOAT_EQ(metrics.hitRate, 2.0f / 3.0f);
    EXPECT_FLOAT_EQ(metrics.falseRate, 1.0f / 3.0f);
    EXPECT_FLOAT_EQ(NativeModuleCollector::ComputeHitRateMetrics(predicted, {}).hitRate, 1.0f);
    EXPECT_FLOAT_EQ(NativeModuleCollector::ComputeHitRateMetrics({}, round).falseRate, 0.0f);
}
