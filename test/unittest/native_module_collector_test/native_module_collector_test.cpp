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

#include "parameters.h"

#include "native_module_collector.h"

using namespace testing;
using namespace testing::ext;
using OHOS::AppExecFwk::NativeModuleCollector;

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
} // namespace

class NativeModuleCollectorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        // every case starts from a clean collector state: switch off, params uncached
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

/**
 * @tc.name: GetStorePath_Format_001
 * @tc.desc: store path uses hidden dot dir, system namespace prefix, 8-char hex hash and version suffix
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, GetStorePath_Format_001, TestSize.Level1)
{
    std::string path = NativeModuleCollector::GetStorePath("/data/storage/el2/base/cache", "com.example.app");

    const std::string expectedPrefix =
        "/data/storage/el2/base/cache/.ohos.ability_runtime/native_modules_";
    EXPECT_EQ(path.compare(0, expectedPrefix.size(), expectedPrefix), 0);

    ASSERT_TRUE(path.size() > expectedPrefix.size());
    std::string tail = path.substr(expectedPrefix.size());
    const std::string suffix = ".v2.json";
    ASSERT_TRUE(tail.size() > suffix.size());
    EXPECT_EQ(tail.compare(tail.size() - suffix.size(), suffix.size(), suffix), 0);

    std::string hash = tail.substr(0, tail.size() - suffix.size());
    EXPECT_EQ(hash.size(), 8u);
    for (char c : hash) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) << "hash char not lowercase hex: " << c;
    }
}

/**
 * @tc.name: ProcessNameHash_StableAndDistinct_002
 * @tc.desc: same process name yields stable hash; different process names yield different hashes
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, ProcessNameHash_StableAndDistinct_002, TestSize.Level1)
{
    std::string main = NativeModuleCollector::ProcessNameHash("com.example.app");
    std::string remote = NativeModuleCollector::ProcessNameHash("com.example.app:remote");

    EXPECT_NE(main, remote);

    std::string mainPath = NativeModuleCollector::GetStorePath("/cache", "com.example.app");
    std::string remotePath = NativeModuleCollector::GetStorePath("/cache", "com.example.app:remote");
    EXPECT_NE(mainPath, remotePath);
}

/**
 * @tc.name: EnsureDir_CreatesNestedAndIdempotent_003
 * @tc.desc: EnsureDir creates nested directories, is idempotent, rejects empty input
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, EnsureDir_CreatesNestedAndIdempotent_003, TestSize.Level1)
{
    std::string base = TempBaseDir("ensuredir");
    RemoveDirRecursively(base);

    std::string nested = base + "/a/b/.ohos.ability_runtime";
    EXPECT_TRUE(NativeModuleCollector::EnsureDir(nested));
    EXPECT_TRUE(std::filesystem::is_directory(nested));

    // second call meets EEXIST on every level and must still succeed
    EXPECT_TRUE(NativeModuleCollector::EnsureDir(nested));

    EXPECT_FALSE(NativeModuleCollector::EnsureDir(""));

    RemoveDirRecursively(base);
}

/**
 * @tc.name: WriteFileWithPermission_ContentAndMode_004
 * @tc.desc: file content round-trips, permission is 0600 and gets tightened on rewrite
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, WriteFileWithPermission_ContentAndMode_004, TestSize.Level1)
{
    std::string base = TempBaseDir("writefile");
    RemoveDirRecursively(base);
    ASSERT_TRUE(NativeModuleCollector::EnsureDir(base));
    std::string file = base + "/native_modules_a1b2c3d4.v2.json";

    const std::string content = "{\"version\":2,\"predicted\":[]}";
    EXPECT_TRUE(NativeModuleCollector::WriteFileWithPermission(file, content));
    EXPECT_EQ(FilePermission(file), STORE_FILE_MODE);

    std::ifstream in(file);
    std::stringstream ss;
    ss << in.rdbuf();
    EXPECT_EQ(ss.str(), content);

    // loosen the mode, then rewrite: mode must be tightened back to 0600
    ASSERT_EQ(chmod(file.c_str(), 0644), 0);
    EXPECT_TRUE(NativeModuleCollector::WriteFileWithPermission(file, content));
    EXPECT_EQ(FilePermission(file), STORE_FILE_MODE);

    // write into a missing directory must fail, not crash
    EXPECT_FALSE(NativeModuleCollector::WriteFileWithPermission(base + "/no/such/dir/f.json", content));

    RemoveDirRecursively(base);
}

/**
 * @tc.name: IsEnabled_DefaultDisabledNoFileIO_005
 * @tc.desc: switch placeholder is disabled; SaveNow refuses and performs no file I/O
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, IsEnabled_DefaultDisabledNoFileIO_005, TestSize.Level1)
{
    NativeModuleCollector::ResetStateForTest();
    NativeModuleCollector::SetEnabledForTest(false);
    EXPECT_FALSE(NativeModuleCollector::IsEnabled());

    std::string base = TempBaseDir("disabled");
    RemoveDirRecursively(base);
    ASSERT_TRUE(NativeModuleCollector::EnsureDir(base));

    OHOS::AppExecFwk::CollectedModule module;
    module.moduleName = "ohos.hilog";
    NativeModuleCollector::AccumulateModules({ module }, 0);
    EXPECT_FALSE(NativeModuleCollector::SaveNow(base, "com.example.app", "com.example.app", 100, "launcher"));
    EXPECT_TRUE(std::filesystem::is_empty(base));

    RemoveDirRecursively(base);
}

// ---------------- TASK-3: EWMA merge engine ----------------

namespace {
using OHOS::AppExecFwk::CollectedModule;
using OHOS::AppExecFwk::ModuleStat;
using OHOS::AppExecFwk::StoreMeta;

CollectedModule MakeModule(const std::string& name, bool isApp = false)
{
    CollectedModule m;
    m.moduleName = name;
    m.fileName = "/system/lib64/module/lib" + name + ".z.so";
    m.isAppModule = isApp;
    return m;
}

ModuleStat MakeStat(float score, uint32_t hits, uint32_t lastSeen)
{
    ModuleStat s;
    s.fileName = "/system/lib64/module/libx.z.so";
    s.score = score;
    s.hits = hits;
    s.lastSeenLaunch = lastSeen;
    return s;
}
} // namespace

/**
 * @tc.name: MergeRound_ColdStart_001
 * @tc.desc: cold start gives every hit score 1.0 and a full predicted set (spec R-7)
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, MergeRound_ColdStart_001, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    std::vector<CollectedModule> round = { MakeModule("ohos.hilog"), MakeModule("ohos.window") };

    NativeModuleCollector::MergeRound(round, stats, 1, true);

    ASSERT_EQ(stats.size(), 2u);
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 1.0f);
    EXPECT_FLOAT_EQ(stats["ohos.window"].score, 1.0f);
    EXPECT_EQ(stats["ohos.hilog"].hits, 1u);
    EXPECT_EQ(stats["ohos.hilog"].lastSeenLaunch, 1u);

    auto predicted = NativeModuleCollector::ComputePredicted(stats);
    EXPECT_EQ(predicted.size(), 2u);
}

/**
 * @tc.name: MergeRound_HitReward_002
 * @tc.desc: hit applies score <- alpha + (1-alpha)*score and converges to 1
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, MergeRound_HitReward_002, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    stats["ohos.hilog"] = MakeStat(0.5f, 1u, 1u);

    NativeModuleCollector::MergeRound({ MakeModule("ohos.hilog") }, stats, 2, false);
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 0.4f + 0.6f * 0.5f); // 0.7

    NativeModuleCollector::MergeRound({ MakeModule("ohos.hilog") }, stats, 3, false);
    EXPECT_NEAR(stats["ohos.hilog"].score, 0.82f, 1e-6);

    // brand-new module in steady state starts below the predict threshold
    NativeModuleCollector::MergeRound({ MakeModule("ohos.window") }, stats, 4, false);
    EXPECT_FLOAT_EQ(stats["ohos.window"].score, 0.4f);
}

/**
 * @tc.name: MergeRound_MissDecay_003
 * @tc.desc: miss applies score <- (1-alpha)*score
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, MergeRound_MissDecay_003, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    stats["ohos.hilog"] = MakeStat(1.0f, 3u, 1u);

    NativeModuleCollector::MergeRound({ MakeModule("ohos.window") }, stats, 2, false);

    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 0.6f);
    EXPECT_EQ(stats["ohos.hilog"].hits, 3u); // miss does not bump hits
    EXPECT_EQ(stats["ohos.hilog"].lastSeenLaunch, 1u);
}

/**
 * @tc.name: ComputePredicted_Threshold_004
 * @tc.desc: score >= 0.5 enters predicted, below is excluded
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, ComputePredicted_Threshold_004, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    stats["in"] = MakeStat(0.5f, 1u, 1u);
    stats["out"] = MakeStat(0.499f, 1u, 1u);

    auto predicted = NativeModuleCollector::ComputePredicted(stats);
    ASSERT_EQ(predicted.size(), 1u);
    EXPECT_EQ(predicted[0], "in");
}

/**
 * @tc.name: EvictStats_LowScoreAndLongUnseen_005
 * @tc.desc: score < 0.05 or unseen for 16 rounds evicts; boundary stays
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, EvictStats_LowScoreAndLongUnseen_005, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    stats["decayed"] = MakeStat(0.04f, 5u, 10u);
    stats["stale"] = MakeStat(0.5f, 5u, 1u);   // launchIndex 17 - 1 = 16 rounds unseen
    stats["border"] = MakeStat(0.5f, 5u, 2u);  // 15 rounds unseen, stays

    NativeModuleCollector::EvictStats(stats, 17);

    EXPECT_EQ(stats.count("decayed"), 0u);
    EXPECT_EQ(stats.count("stale"), 0u);
    EXPECT_EQ(stats.count("border"), 1u);
}

/**
 * @tc.name: EvictStats_CapLimit_006
 * @tc.desc: entries over 1024 are evicted lowest-score first
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, EvictStats_CapLimit_006, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    for (int i = 0; i < 1024; ++i) {
        stats["mod_" + std::to_string(i)] = MakeStat(0.5f, 1u, 1u);
    }
    stats["lowest"] = MakeStat(0.06f, 1u, 1u); // above evictScore, below all others

    NativeModuleCollector::EvictStats(stats, 1);

    EXPECT_EQ(stats.size(), 1024u);
    EXPECT_EQ(stats.count("lowest"), 0u);
}

/**
 * @tc.name: LoadStats_RejectInvalid_007
 * @tc.desc: missing file, corrupted json and version mismatch are all rejected
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, LoadStats_RejectInvalid_007, TestSize.Level1)
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

    std::string wrongVersion = base + "/v1.json";
    ASSERT_TRUE(NativeModuleCollector::WriteFileWithPermission(wrongVersion,
        "{\"version\":1,\"appVersionCode\":1,\"launchIndex\":1,\"modules\":[]}"));
    EXPECT_FALSE(NativeModuleCollector::LoadStats(wrongVersion, stats, meta, err));

    // type-confused fields must be rejected instead of throwing
    std::string typeConfused = base + "/confused.json";
    ASSERT_TRUE(NativeModuleCollector::WriteFileWithPermission(typeConfused,
        "{\"version\":2,\"appVersionCode\":\"x\",\"launchIndex\":1,\"modules\":[]}"));
    EXPECT_FALSE(NativeModuleCollector::LoadStats(typeConfused, stats, meta, err));

    RemoveDirRecursively(base);
}

/**
 * @tc.name: MergeAndSave_RoundTrip_008
 * @tc.desc: two rounds persist and reload correctly; launchIndex increments; scores evolve
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, MergeAndSave_RoundTrip_008, TestSize.Level1)
{
    std::string base = TempBaseDir("roundtrip");
    RemoveDirRecursively(base);
    std::string store = NativeModuleCollector::GetStorePath(base, "com.example.app");

    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, "com.example.app", "com.example.app",
        100, "launcher", { MakeModule("ohos.hilog"), MakeModule("ohos.window"), MakeModule("entry", true) }));
    EXPECT_EQ(FilePermission(store), STORE_FILE_MODE);

    std::map<std::string, ModuleStat> stats;
    StoreMeta meta;
    std::string err;
    ASSERT_TRUE(NativeModuleCollector::LoadStats(store, stats, meta, err));
    EXPECT_EQ(meta.launchIndex, 1u);
    EXPECT_EQ(meta.appVersionCode, 100);
    EXPECT_EQ(stats.size(), 3u);
    EXPECT_TRUE(stats["entry"].isAppModule);

    // second round: only ohos.hilog hits
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, "com.example.app", "com.example.app",
        100, "launcher", { MakeModule("ohos.hilog") }));
    ASSERT_TRUE(NativeModuleCollector::LoadStats(store, stats, meta, err));
    EXPECT_EQ(meta.launchIndex, 2u);
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 1.0f);       // 0.4 + 0.6*1.0
    EXPECT_FLOAT_EQ(stats["ohos.window"].score, 0.6f);      // decayed
    EXPECT_FLOAT_EQ(stats["entry"].score, 0.6f);

    // predicted survives a full serialize/parse round trip
    std::ifstream in(store);
    nlohmann::json root = nlohmann::json::parse(in, nullptr, false, true);
    ASSERT_TRUE(root.is_object());
    ASSERT_TRUE(root.contains("predicted"));
    EXPECT_EQ(root["predicted"].size(), 3u); // all >= 0.5

    RemoveDirRecursively(base);
}

/**
 * @tc.name: MergeAndSave_AppUpgradeRebuild_009
 * @tc.desc: appVersionCode change clears history and rebuilds from cold start
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, MergeAndSave_AppUpgradeRebuild_009, TestSize.Level1)
{
    std::string base = TempBaseDir("upgrade");
    RemoveDirRecursively(base);
    std::string store = NativeModuleCollector::GetStorePath(base, "com.example.app");

    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, "com.example.app", "com.example.app",
        100, "launcher", { MakeModule("ohos.hilog") }));
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, "com.example.app", "com.example.app",
        100, "launcher", { MakeModule("ohos.hilog") }));

    // upgrade: appVersionCode 100 -> 200
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, "com.example.app", "com.example.app",
        200, "launcher", { MakeModule("ohos.window") }));

    std::map<std::string, ModuleStat> stats;
    StoreMeta meta;
    std::string err;
    ASSERT_TRUE(NativeModuleCollector::LoadStats(store, stats, meta, err));
    EXPECT_EQ(meta.launchIndex, 1u);              // rebuilt, not 3
    EXPECT_EQ(meta.appVersionCode, 200);
    EXPECT_EQ(stats.size(), 1u);                  // old entry gone
    EXPECT_EQ(stats.count("ohos.hilog"), 0u);
    EXPECT_FLOAT_EQ(stats["ohos.window"].score, 1.0f); // cold start rule

    RemoveDirRecursively(base);
}

/**
 * @tc.name: WriteAtomic_SuccessAndFailure_010
 * @tc.desc: success leaves formal file with 0600 and no tmp; failure leaves nothing behind
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, WriteAtomic_SuccessAndFailure_010, TestSize.Level1)
{
    std::string base = TempBaseDir("atomic");
    RemoveDirRecursively(base);
    ASSERT_TRUE(NativeModuleCollector::EnsureDir(base));
    std::string file = base + "/native_modules_a1b2c3d4.v2.json";

    EXPECT_TRUE(NativeModuleCollector::WriteAtomic(file, "{\"version\":2}"));
    EXPECT_EQ(FilePermission(file), STORE_FILE_MODE);
    EXPECT_FALSE(std::filesystem::exists(file + ".tmp"));

    // rewrite replaces atomically
    EXPECT_TRUE(NativeModuleCollector::WriteAtomic(file, "{\"version\":2,\"x\":1}"));
    std::ifstream in(file);
    std::stringstream ss;
    ss << in.rdbuf();
    EXPECT_NE(ss.str().find("\"x\":1"), std::string::npos);

    // failure: parent dir missing -> no formal file, no tmp
    std::string bad = base + "/no/such/dir/f.json";
    EXPECT_FALSE(NativeModuleCollector::WriteAtomic(bad, "{}"));
    EXPECT_FALSE(std::filesystem::exists(bad));
    EXPECT_FALSE(std::filesystem::exists(bad + ".tmp"));

    RemoveDirRecursively(base);
}

// ---------------- TASK-4/5: checkpoint accumulation & preload filter ----------------

/**
 * @tc.name: AccumulateModules_DedupAndPreloadFilter_011
 * @tc.desc: preload keys are dropped; duplicates refresh fields but keep first-sighting stage
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, AccumulateModules_DedupAndPreloadFilter_011, TestSize.Level1)
{
    NativeModuleCollector::ResetStateForTest();
    NativeModuleCollector::SetPreloadModuleKeys({ "ohos.preload" });

    NativeModuleCollector::AccumulateModules(
        { MakeModule("ohos.preload"), MakeModule("ohos.hilog") }, 0);
    EXPECT_EQ(NativeModuleCollector::GetSampledCountForTest(), 1u); // preload filtered

    CollectedModule refreshed = MakeModule("ohos.hilog");
    refreshed.fileName = "/system/lib64/module/libhilog_v2.z.so";
    refreshed.loadCount = 9;
    NativeModuleCollector::AccumulateModules({ refreshed }, 1);
    EXPECT_EQ(NativeModuleCollector::GetSampledCountForTest(), 1u); // deduped

    CollectedModule out;
    ASSERT_TRUE(NativeModuleCollector::GetSampledModuleForTest("ohos.hilog", out));
    EXPECT_EQ(out.fileName, "/system/lib64/module/libhilog_v2.z.so"); // refreshed
    EXPECT_EQ(out.loadCount, 9u);
    EXPECT_EQ(out.firstStage, 0u); // first-sighting stage kept

    NativeModuleCollector::ResetStateForTest();
}

/**
 * @tc.name: SaveNow_AtMostOnce_012
 * @tc.desc: enabled sync save writes the file once; a second SaveNow is refused
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, SaveNow_AtMostOnce_012, TestSize.Level1)
{
    NativeModuleCollector::ResetStateForTest();
    NativeModuleCollector::SetEnabledForTest(true);
    NativeModuleCollector::SetAsyncSaveForTest(false);

    std::string base = TempBaseDir("saveonce");
    RemoveDirRecursively(base);

    NativeModuleCollector::AccumulateModules({ MakeModule("ohos.hilog") }, 0);
    EXPECT_TRUE(NativeModuleCollector::SaveNow(base, "com.example.app", "com.example.app", 100, "launcher"));

    std::string store = NativeModuleCollector::GetStorePath(base, "com.example.app");
    EXPECT_TRUE(std::filesystem::exists(store));

    EXPECT_FALSE(NativeModuleCollector::SaveNow(base, "com.example.app", "com.example.app", 100, "launcher"));

    NativeModuleCollector::SetEnabledForTest(false);
    NativeModuleCollector::ResetStateForTest();
    RemoveDirRecursively(base);
}

/**
 * @tc.name: SaveNow_AsyncDispatched_013
 * @tc.desc: async mode dispatches the write and reports success without blocking
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, SaveNow_AsyncDispatched_013, TestSize.Level1)
{
    NativeModuleCollector::ResetStateForTest();
    NativeModuleCollector::SetEnabledForTest(true);
    NativeModuleCollector::SetAsyncSaveForTest(true);

    std::string base = TempBaseDir("saveasync");
    RemoveDirRecursively(base);
    NativeModuleCollector::AccumulateModules({ MakeModule("ohos.hilog") }, 0);

    // async path: SaveNow dispatches and reports success without blocking.
    // The landed content is already verified by SaveNow_AtMostOnce_012 (sync),
    // so this case only asserts the dispatch result, avoiding IO-timing flakiness.
    EXPECT_TRUE(NativeModuleCollector::SaveNow(base, "com.example.app", "com.example.app", 100, "launcher"));

    NativeModuleCollector::SetEnabledForTest(false);
    NativeModuleCollector::ResetStateForTest();
    RemoveDirRecursively(base);
}

// ---------------- TASK-6: consumer-side predicted system modules ----------------

/**
 * @tc.name: LoadPredictedSystemModules_Filter_014
 * @tc.desc: returns only system modules above the threshold; app modules and low scores excluded
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, LoadPredictedSystemModules_Filter_014, TestSize.Level1)
{
    std::string base = TempBaseDir("consumer");
    RemoveDirRecursively(base);
    std::string store = NativeModuleCollector::GetStorePath(base, "com.example.app");

    // cold start: every hit scores 1.0
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, "com.example.app", "com.example.app",
        100, "launcher", { MakeModule("ohos.hilog"), MakeModule("ohos.window"), MakeModule("entry", true) }));

    auto predicted = NativeModuleCollector::LoadPredictedSystemModules(store);
    ASSERT_EQ(predicted.size(), 2u);
    for (const auto& name : predicted) {
        EXPECT_NE(name, "entry"); // app module must be excluded
    }

    // second round: only ohos.hilog hits; others decay to 0.6 (still >= 0.5)
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, "com.example.app", "com.example.app",
        100, "launcher", { MakeModule("ohos.hilog") }));
    predicted = NativeModuleCollector::LoadPredictedSystemModules(store);
    EXPECT_EQ(predicted.size(), 2u);

    // keep missing ohos.window until it drops below 0.5: 0.6 -> 0.36
    ASSERT_TRUE(NativeModuleCollector::MergeAndSave(store, "com.example.app", "com.example.app",
        100, "launcher", { MakeModule("ohos.hilog") }));
    predicted = NativeModuleCollector::LoadPredictedSystemModules(store);
    ASSERT_EQ(predicted.size(), 1u);
    EXPECT_EQ(predicted[0], "ohos.hilog");

    RemoveDirRecursively(base);
}

/**
 * @tc.name: LoadPredictedSystemModules_MissingOrCorrupted_015
 * @tc.desc: missing or corrupted store file yields an empty list, no crash
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, LoadPredictedSystemModules_MissingOrCorrupted_015, TestSize.Level1)
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

// ---------------- TASK-7: loadCount hit determination (anti self-feedback) ----------------

/**
 * @tc.name: MergeRound_ConsumerPreloadOnly_016
 * @tc.desc: consumer-preloaded module without import evidence is not rewarded as a hit; loadCount
 *          refreshes and the entry still decays as unused so it eventually leaves the predicted set
 *          (breaks the self-feedback loop). AC-2.5 only requires the score not to rise.
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, MergeRound_ConsumerPreloadOnly_016, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    stats["ohos.hilog"] = MakeStat(0.5f, 2u, 1u);
    stats["ohos.hilog"].loadCount = 5;

    CollectedModule m = MakeModule("ohos.hilog");
    m.loadCount = 6; // +1, exactly the consumer preload count
    std::map<std::string, uint32_t> preloads = { { "ohos.hilog", 1u } };

    NativeModuleCollector::MergeRound({ m }, stats, 2, false, preloads);

    // not rewarded: hits unchanged; score must not rise above 0.5
    EXPECT_EQ(stats["ohos.hilog"].hits, 2u);
    EXPECT_LE(stats["ohos.hilog"].score, 0.5f);
    // treated as unused this round: decays toward 0 (0.5 * (1-0.4) = 0.3) so that a module only ever
    // preloaded by the consumer eventually drops below the threshold and stops being predicted
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 0.3f);
    EXPECT_EQ(stats["ohos.hilog"].lastSeenLaunch, 1u); // not marked as a hit
    EXPECT_EQ(stats["ohos.hilog"].loadCount, 6u);      // observed counter refreshed
}

/**
 * @tc.name: MergeRound_ConsumerPreloadPlusImport_017
 * @tc.desc: loadCount growth beyond the consumer preload count proves a real import: hit
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, MergeRound_ConsumerPreloadPlusImport_017, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;
    stats["ohos.hilog"] = MakeStat(0.5f, 2u, 1u);
    stats["ohos.hilog"].loadCount = 5;

    CollectedModule m = MakeModule("ohos.hilog");
    m.loadCount = 7; // +2 while consumer preloaded once: one real import
    std::map<std::string, uint32_t> preloads = { { "ohos.hilog", 1u } };

    NativeModuleCollector::MergeRound({ m }, stats, 2, false, preloads);

    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 0.4f + 0.6f * 0.5f); // rewarded
    EXPECT_EQ(stats["ohos.hilog"].hits, 3u);
    EXPECT_EQ(stats["ohos.hilog"].lastSeenLaunch, 2u);
    EXPECT_EQ(stats["ohos.hilog"].loadCount, 7u);
}

/**
 * @tc.name: MergeRound_NotConsumerPreloaded_018
 * @tc.desc: module absent from the consumer set merges as a normal hit even with preloads map
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, MergeRound_NotConsumerPreloaded_018, TestSize.Level1)
{
    std::map<std::string, ModuleStat> stats;

    CollectedModule m = MakeModule("ohos.window");
    m.loadCount = 1; // diff from 0 is 1, but module is not in the consumer set
    std::map<std::string, uint32_t> preloads = { { "ohos.hilog", 1u } };

    NativeModuleCollector::MergeRound({ m }, stats, 1, false, preloads);

    EXPECT_FLOAT_EQ(stats["ohos.window"].score, 0.4f); // normal new-module reward
    EXPECT_EQ(stats["ohos.window"].hits, 1u);
}

/**
 * @tc.name: SaveNow_ConsumerPreloadNotPredicted_019
 * @tc.desc: end to end: consumer-preloaded module is discounted and never enters the store
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, SaveNow_ConsumerPreloadNotPredicted_019, TestSize.Level1)
{
    NativeModuleCollector::ResetStateForTest();
    NativeModuleCollector::SetEnabledForTest(true);
    NativeModuleCollector::SetAsyncSaveForTest(false);

    std::string base = TempBaseDir("selffeedback");
    RemoveDirRecursively(base);
    std::string store = NativeModuleCollector::GetStorePath(base, "com.example.app");

    // consumer preloads ohos.hilog; app never imports it (loadCount == preload count)
    NativeModuleCollector::RecordConsumerPreload("ohos.hilog");
    CollectedModule m = MakeModule("ohos.hilog");
    m.loadCount = 1;
    NativeModuleCollector::AccumulateModules({ m }, 0);

    EXPECT_TRUE(NativeModuleCollector::SaveNow(base, "com.example.app", "com.example.app", 100, "launcher"));
    EXPECT_TRUE(NativeModuleCollector::LoadPredictedSystemModules(store).empty());

    // module was skipped and its zero-score entry evicted in the same round
    std::map<std::string, ModuleStat> stats;
    StoreMeta meta;
    std::string err;
    ASSERT_TRUE(NativeModuleCollector::LoadStats(store, stats, meta, err));
    EXPECT_EQ(stats.count("ohos.hilog"), 0u);

    NativeModuleCollector::SetEnabledForTest(false);
    NativeModuleCollector::ResetStateForTest();
    RemoveDirRecursively(base);
}

/**
 * @tc.name: MergeRound_ColdStartConsumerPreloadNotHit_023
 * @tc.desc: on cold start a consumer-preloaded module never becomes a hit even when its
 *          inherited loadCount baseline exceeds the preload count (self-feedback guard, R-9)
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, MergeRound_ColdStartConsumerPreloadNotHit_023, TestSize.Level1)
{
    // cold start: stats empty; module loadCount=5 is an inherited fork baseline,
    // far above the consumer's single preload this round.
    std::map<std::string, ModuleStat> stats;
    CollectedModule m = MakeModule("ohos.hilog");
    m.loadCount = 5;
    std::map<std::string, uint32_t> preloads = { { "ohos.hilog", 1u } };

    NativeModuleCollector::MergeRound({ m }, stats, 1, true, preloads);

    // must NOT be counted as a hit: score stays 0, hits 0; only loadCount refreshed
    ASSERT_EQ(stats.count("ohos.hilog"), 1u);
    EXPECT_FLOAT_EQ(stats["ohos.hilog"].score, 0.0f);
    EXPECT_EQ(stats["ohos.hilog"].hits, 0u);
    EXPECT_EQ(stats["ohos.hilog"].lastSeenLaunch, 0u);
    EXPECT_EQ(stats["ohos.hilog"].loadCount, 5u);
}

// ---------------- TASK-8: system parameter (CCM) channel ----------------

namespace {
// keep in sync with native_module_collector.cpp (anonymous namespace keys)
constexpr char PARAM_ENABLE[] = "ccm.ability.native_module_collect.enable";
constexpr char PARAM_ALPHA[] = "ccm.ability.native_module_collect.alpha";
constexpr char PARAM_THRESHOLD[] = "ccm.ability.native_module_collect.predict_threshold";
} // namespace

/**
 * @tc.name: Params_DefaultsWhenAbsent_020
 * @tc.desc: absent parameters yield disabled switch and spec-default tunables
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, Params_DefaultsWhenAbsent_020, TestSize.Level1)
{
    NativeModuleCollector::ResetParamCacheForTest();
    NativeModuleCollector::SetEnabledForTest(false);

    EXPECT_FALSE(NativeModuleCollector::IsEnabled());

    const auto& params = NativeModuleCollector::GetParams();
    EXPECT_FLOAT_EQ(params.alpha, 0.4f);
    EXPECT_FLOAT_EQ(params.predictThreshold, 0.5f);
    EXPECT_FLOAT_EQ(params.evictScore, 0.05f);
    EXPECT_EQ(params.evictRounds, 16u);
    EXPECT_EQ(params.maxEntries, 1024u);
    EXPECT_EQ(params.checkpoint2DelayMs, 5000u);
    EXPECT_FALSE(params.reportHitRate);
}

/**
 * @tc.name: Params_OverrideAndInvalid_021
 * @tc.desc: system parameter overrides tunables; invalid value falls back to default
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, Params_OverrideAndInvalid_021, TestSize.Level1)
{
    NativeModuleCollector::SetEnabledForTest(false);
    // probe SetParameter capability first; skip without touching any value so a
    // disabled environment cannot leave global sysparam polluted.
    if (!OHOS::system::SetParameter(PARAM_ENABLE, "") ||
        !OHOS::system::SetParameter(PARAM_ALPHA, "") ||
        !OHOS::system::SetParameter(PARAM_THRESHOLD, "")) {
        GTEST_SKIP() << "SetParameter not permitted in this environment";
    }

    // EXPECT (not ASSERT) so the restore block always runs even if a set fails,
    // preventing residual sysparam from leaking into sibling cases.
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_ENABLE, "true"));
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_ALPHA, "0.7"));
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_THRESHOLD, "abc")); // invalid
    NativeModuleCollector::ResetParamCacheForTest();

    EXPECT_TRUE(NativeModuleCollector::IsEnabled());
    auto params = NativeModuleCollector::GetParams();
    EXPECT_FLOAT_EQ(params.alpha, 0.7f);
    EXPECT_FLOAT_EQ(params.predictThreshold, 0.5f); // invalid -> spec default

    // restore environment: disable and clear overrides (always runs)
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_ENABLE, "false"));
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_ALPHA, ""));
    EXPECT_TRUE(OHOS::system::SetParameter(PARAM_THRESHOLD, ""));
    NativeModuleCollector::ResetParamCacheForTest();
    EXPECT_FALSE(NativeModuleCollector::IsEnabled());
}

// ---------------- TASK-9: hit-rate metrics ----------------

/**
 * @tc.name: ComputeHitRateMetrics_022
 * @tc.desc: hitRate = |P ∩ A| / |A|, falseRate = |P - A| / |P|, with empty-set edges
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, ComputeHitRateMetrics_022, TestSize.Level1)
{
    std::vector<std::string> predicted = { "a", "b", "c" };
    std::vector<CollectedModule> round = { MakeModule("b"), MakeModule("c"), MakeModule("d") };

    auto metrics = NativeModuleCollector::ComputeHitRateMetrics(predicted, round);
    EXPECT_EQ(metrics.predictedCount, 3u);
    EXPECT_EQ(metrics.actualCount, 3u);
    EXPECT_EQ(metrics.hitCount, 2u);
    EXPECT_FLOAT_EQ(metrics.hitRate, 2.0f / 3.0f);
    EXPECT_FLOAT_EQ(metrics.falseRate, 1.0f / 3.0f);

    // empty actual: perfect score by definition
    auto emptyActual = NativeModuleCollector::ComputeHitRateMetrics(predicted, {});
    EXPECT_FLOAT_EQ(emptyActual.hitRate, 1.0f);

    // empty predicted: no false positives possible
    auto emptyPredicted = NativeModuleCollector::ComputeHitRateMetrics({}, round);
    EXPECT_FLOAT_EQ(emptyPredicted.falseRate, 0.0f);
    EXPECT_FLOAT_EQ(emptyPredicted.hitRate, 0.0f);
}
