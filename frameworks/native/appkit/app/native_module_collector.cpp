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

#include "native_module_collector.h"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <mutex>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include <nlohmann/json.hpp>

#include "hilog_tag_wrapper.h"
#include "hisysevent_report.h"
#include "parameters.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* STORE_DIR_NAME = ".ohos.ability_runtime";
constexpr const char* STORE_FILE_PREFIX = "native_modules_";
constexpr const char* STORE_FILE_SUFFIX = ".v2.json";
constexpr const char* STORE_TMP_SUFFIX = ".tmp";
constexpr mode_t STORE_DIR_MODE = 0700;
constexpr mode_t STORE_FILE_MODE = 0600;
constexpr uint32_t FNV1A_OFFSET_BASIS = 2166136261u;
constexpr uint32_t FNV1A_PRIME = 16777619u;
constexpr size_t HEX_HASH_LEN = 8;
constexpr int32_t STORE_FILE_VERSION = 2;
constexpr size_t MAX_PARSE_ENTRIES = 4096;     /* reject abnormally large files */
constexpr size_t MAX_MODULE_NAME_LEN = 256;
constexpr size_t MAX_FILE_NAME_LEN = 4096;
constexpr size_t MAX_BUNDLE_NAME_LEN = 256;
constexpr float SCORE_PRECISION = 1000.0f;     /* serialize score with 3 decimals */

// system parameter keys; the CCM channel maps its switches to these sysparam
// keys (exact naming to be confirmed with the CCM owner, RSK-3)
constexpr char PARAM_ENABLE[] = "ccm.ability.native_module_collect.enable";
constexpr char PARAM_ALPHA[] = "ccm.ability.native_module_collect.alpha";
constexpr char PARAM_PREDICT_THRESHOLD[] = "ccm.ability.native_module_collect.predict_threshold";
constexpr char PARAM_EVICT_SCORE[] = "ccm.ability.native_module_collect.evict_score";
constexpr char PARAM_EVICT_ROUNDS[] = "ccm.ability.native_module_collect.evict_rounds";
constexpr char PARAM_CHECKPOINT2_DELAY_MS[] = "ccm.ability.native_module_collect.checkpoint2_delay_ms";
constexpr char PARAM_REPORT_HITRATE[] = "ccm.ability.native_module_collect.report_hitrate";

struct ParamCache {
    std::mutex mutex;
    bool loaded = false;
    bool enabled = false;
    CollectorParams params;
};
ParamCache g_paramCache;

float GetFloatParameter(const std::string& key, float def, float minValue, float maxValue)
{
    std::string value = OHOS::system::GetParameter(key, "");
    if (value.empty()) {
        return def;
    }
    char* end = nullptr;
    float parsed = std::strtof(value.c_str(), &end);
    if (end == value.c_str() || *end != '\0' || std::isnan(parsed)) {
        TAG_LOGW(AAFwkTag::APPKIT, "invalid param %{public}s=%{public}s, use default",
            key.c_str(), value.c_str());
        return def;
    }
    return std::min(std::max(parsed, minValue), maxValue);
}

CollectorParams LoadParamsFromSystem()
{
    CollectorParams params;
    params.alpha = GetFloatParameter(PARAM_ALPHA, params.alpha, 0.05f, 0.95f);
    params.predictThreshold = GetFloatParameter(PARAM_PREDICT_THRESHOLD, params.predictThreshold, 0.05f, 0.95f);
    params.evictScore = GetFloatParameter(PARAM_EVICT_SCORE, params.evictScore, 0.001f, 0.5f);
    params.evictRounds = OHOS::system::GetIntParameter<uint32_t>(PARAM_EVICT_ROUNDS, params.evictRounds, 1u, 1000u);
    params.checkpoint2DelayMs =
        OHOS::system::GetIntParameter<uint32_t>(PARAM_CHECKPOINT2_DELAY_MS, params.checkpoint2DelayMs, 0u, 60000u);
    params.reportHitRate = OHOS::system::GetBoolParameter(PARAM_REPORT_HITRATE, false);
    return params;
}

// read the switch and tunables at most once per process (spec R-2: cached per launch)
void EnsureParamsLoaded()
{
    std::lock_guard<std::mutex> lock(g_paramCache.mutex);
    if (g_paramCache.loaded) {
        return;
    }
    g_paramCache.enabled = OHOS::system::GetBoolParameter(PARAM_ENABLE, false);
    g_paramCache.params = LoadParamsFromSystem();
    g_paramCache.loaded = true;
}

struct CollectorState {
    std::mutex mutex;
    std::map<std::string, CollectedModule> sampled;
    std::set<std::string> preloadKeys;
    std::map<std::string, uint32_t> consumerPreloads; // this round, by the startup consumer
    bool saveDispatched = false;
    bool asyncSave = true;
};
CollectorState g_state;
std::atomic<bool> g_enabledForTest { false };

float ClampScore(double score)
{
    if (std::isnan(score) || score < 0.0) {
        return 0.0f;
    }
    if (score > 1.0) {
        return 1.0f;
    }
    return static_cast<float>(score);
}

int64_t NowMillis()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

bool ParseStatEntry(const nlohmann::json& item, std::string& moduleName, ModuleStat& stat)
{
    if (!item.is_object()) {
        return false;
    }
    // OpenHarmony is built with -fno-exceptions; use non-throwing access only.
    auto readStr = [](const nlohmann::json& node, std::string& out) {
        if (!node.is_string()) {
            return false;
        }
        out = node.get_ref<const std::string&>();
        return true;
    };
    const auto name = item.find("moduleName");
    if (name == item.end() || !readStr(*name, moduleName) || moduleName.empty() ||
        moduleName.size() > MAX_MODULE_NAME_LEN) {
        return false;
    }
    const auto fileName = item.find("fileName");
    if (fileName == item.end() || !readStr(*fileName, stat.fileName) ||
        stat.fileName.size() > MAX_FILE_NAME_LEN) {
        return false;
    }
    const auto score = item.find("score");
    const auto hits = item.find("hits");
    const auto lastSeen = item.find("lastSeenLaunch");
    const auto firstStage = item.find("firstStage");
    const auto isApp = item.find("isAppModule");
    if (score == item.end() || !score->is_number() ||
        hits == item.end() || !hits->is_number_unsigned() ||
        lastSeen == item.end() || !lastSeen->is_number_unsigned() ||
        firstStage == item.end() || !firstStage->is_number_unsigned() ||
        isApp == item.end() || !isApp->is_boolean()) {
        return false;
    }
    stat.isAppModule = isApp->get<bool>();
    stat.score = ClampScore(score->get<double>());
    stat.hits = hits->get<uint32_t>();
    stat.lastSeenLaunch = lastSeen->get<uint32_t>();
    stat.firstStage = firstStage->get<uint8_t>() > 1 ? 1 : firstStage->get<uint8_t>();
    const auto loadCount = item.find("loadCount");
    stat.loadCount = (loadCount != item.end() && loadCount->is_number_unsigned())
        ? loadCount->get<uint32_t>() : 0u;
    return true;
}
} // namespace

bool NativeModuleCollector::IsEnabled()
{
    // test override wins; otherwise the cached system parameter decides
    // (absent/invalid parameter => disabled, spec R-2)
    if (g_enabledForTest.load()) {
        return true;
    }
    EnsureParamsLoaded();
    std::lock_guard<std::mutex> lock(g_paramCache.mutex);
    return g_paramCache.enabled;
}

std::string NativeModuleCollector::GetStoreDir(const std::string& baseDir)
{
    return baseDir + "/" + STORE_DIR_NAME;
}

std::string NativeModuleCollector::GetStorePath(const std::string& baseDir, const std::string& processName)
{
    return GetStoreDir(baseDir) + "/" + STORE_FILE_PREFIX + ProcessNameHash(processName) + STORE_FILE_SUFFIX;
}

std::string NativeModuleCollector::ProcessNameHash(const std::string& processName)
{
    uint32_t hash = FNV1A_OFFSET_BASIS;
    for (char c : processName) {
        hash ^= static_cast<uint8_t>(c);
        hash *= FNV1A_PRIME;
    }
    char buf[HEX_HASH_LEN + 1] = {0};
    (void)snprintf(buf, sizeof(buf), "%08x", hash);
    return std::string(buf);
}

bool NativeModuleCollector::EnsureDir(const std::string& dir)
{
    if (dir.empty()) {
        return false;
    }
    std::string current;
    current.reserve(dir.size());
    for (size_t i = 0; i < dir.size(); ++i) {
        current.push_back(dir[i]);
        if (dir[i] != '/' && i + 1 != dir.size()) {
            continue;
        }
        if (current == "/") {
            continue;
        }
        if (!current.empty() && current.back() == '/') {
            current.pop_back();
        }
        if (mkdir(current.c_str(), STORE_DIR_MODE) != 0 && errno != EEXIST) {
            TAG_LOGE(AAFwkTag::APPKIT, "mkdir %{public}s failed, errno %{public}d", current.c_str(), errno);
            return false;
        }
    }
    return true;
}

bool NativeModuleCollector::WriteFileWithPermission(const std::string& path, const std::string& content)
{
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, STORE_FILE_MODE);
    if (fd < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "open %{public}s failed, errno %{public}d", path.c_str(), errno);
        return false;
    }
    // open() mode does not tighten permissions of an already existing file
    if (fchmod(fd, STORE_FILE_MODE) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "fchmod %{public}s failed, errno %{public}d", path.c_str(), errno);
        close(fd);
        return false;
    }
    const char* data = content.data();
    size_t left = content.size();
    while (left > 0) {
        ssize_t written = write(fd, data, left);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            TAG_LOGE(AAFwkTag::APPKIT, "write %{public}s failed, errno %{public}d", path.c_str(), errno);
            close(fd);
            return false;
        }
        data += written;
        left -= static_cast<size_t>(written);
    }
    if (close(fd) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "close %{public}s failed, errno %{public}d", path.c_str(), errno);
        return false;
    }
    return true;
}

void NativeModuleCollector::AccumulateModules(const std::vector<CollectedModule>& modules, uint8_t firstStage)
{
    std::lock_guard<std::mutex> lock(g_state.mutex);
    for (const auto& module : modules) {
        if (module.moduleName.empty() || g_state.preloadKeys.count(module.moduleName) != 0) {
            continue;
        }
        auto [it, inserted] = g_state.sampled.emplace(module.moduleName, module);
        if (inserted) {
            it->second.firstStage = firstStage;
        } else {
            // duplicate sighting: refresh identity fields, keep the first-sighting stage
            it->second.fileName = module.fileName;
            it->second.isAppModule = module.isAppModule;
            it->second.loadCount = module.loadCount;
        }
    }
}

bool NativeModuleCollector::SaveNow(const std::string& cacheDir, const std::string& bundleName,
    const std::string& processName, int64_t appVersionCode, const std::string& scenario)
{
    if (!IsEnabled()) {
        return false;
    }
    std::vector<CollectedModule> round;
    std::map<std::string, uint32_t> consumerPreloads;
    bool asyncSave;
    {
        std::lock_guard<std::mutex> lock(g_state.mutex);
        if (g_state.saveDispatched) {
            return false; // at most once per launch
        }
        g_state.saveDispatched = true;
        round.reserve(g_state.sampled.size());
        for (const auto& [name, module] : g_state.sampled) {
            round.push_back(module);
        }
        consumerPreloads = g_state.consumerPreloads;
        asyncSave = g_state.asyncSave;
    }
    std::string storePath = GetStorePath(cacheDir, processName);
    if (asyncSave) {
        std::thread([storePath, bundleName, processName, appVersionCode, scenario, round,
            consumerPreloads]() {
            (void)MergeAndSave(storePath, bundleName, processName, appVersionCode, scenario, round,
                consumerPreloads);
        }).detach();
        return true;
    }
    return MergeAndSave(storePath, bundleName, processName, appVersionCode, scenario, round,
        consumerPreloads);
}

void NativeModuleCollector::SetPreloadModuleKeys(std::set<std::string> keys)
{
    std::lock_guard<std::mutex> lock(g_state.mutex);
    g_state.preloadKeys = std::move(keys);
}

std::set<std::string> NativeModuleCollector::GetPreloadModuleKeys()
{
    std::lock_guard<std::mutex> lock(g_state.mutex);
    return g_state.preloadKeys;
}

void NativeModuleCollector::SetEnabledForTest(bool enabled)
{
    g_enabledForTest.store(enabled);
}

void NativeModuleCollector::SetAsyncSaveForTest(bool async)
{
    std::lock_guard<std::mutex> lock(g_state.mutex);
    g_state.asyncSave = async;
}

void NativeModuleCollector::ResetStateForTest()
{
    std::lock_guard<std::mutex> lock(g_state.mutex);
    g_state.sampled.clear();
    g_state.preloadKeys.clear();
    g_state.consumerPreloads.clear();
    g_state.saveDispatched = false;
    g_state.asyncSave = true;
}

size_t NativeModuleCollector::GetSampledCountForTest()
{
    std::lock_guard<std::mutex> lock(g_state.mutex);
    return g_state.sampled.size();
}

bool NativeModuleCollector::GetSampledModuleForTest(const std::string& moduleName, CollectedModule& out)
{
    std::lock_guard<std::mutex> lock(g_state.mutex);
    auto it = g_state.sampled.find(moduleName);
    if (it == g_state.sampled.end()) {
        return false;
    }
    out = it->second;
    return true;
}

const CollectorParams& NativeModuleCollector::GetParams()
{
    EnsureParamsLoaded();
    return g_paramCache.params;
}

void NativeModuleCollector::ResetParamCacheForTest()
{
    std::lock_guard<std::mutex> lock(g_paramCache.mutex);
    g_paramCache.loaded = false;
}

bool NativeModuleCollector::LoadStats(const std::string& path, std::map<std::string, ModuleStat>& stats,
    StoreMeta& meta, std::string& errInfo)
{
    stats.clear();
    std::ifstream in(path);
    if (!in.good()) {
        errInfo = "file missing or unreadable";
        return false;
    }
    // OpenHarmony is built with -fno-exceptions: parse without throwing.
    // The third arg `false` disables exceptions; a malformed file yields a null json.
    nlohmann::json root = nlohmann::json::parse(in, nullptr, false, true);
    if (!root.is_object()) {
        errInfo = root.is_null() ? "json parse failed" : "root is not an object";
        return false;
    }
    const auto version = root.find("version");
    if (version == root.end() || !version->is_number_integer() ||
        version->get<int32_t>() != STORE_FILE_VERSION) {
        errInfo = "version mismatch";
        return false;
    }
    const auto appVersion = root.find("appVersionCode");
    const auto launchIndex = root.find("launchIndex");
    const auto modules = root.find("modules");
    if (appVersion == root.end() || !appVersion->is_number_integer() ||
        launchIndex == root.end() || !launchIndex->is_number_unsigned() ||
        modules == root.end() || !modules->is_array()) {
        errInfo = "meta fields invalid";
        return false;
    }
    if (modules->size() > MAX_PARSE_ENTRIES) {
        errInfo = "entries over parse cap";
        return false;
    }
    auto readMetaStr = [&root](const char* key, std::string& out) {
        const auto it = root.find(key);
        if (it == root.end() || !it->is_string()) {
            return false;
        }
        out = it->get_ref<const std::string&>();
        return true;
    };
    std::string bundleName;
    std::string processName;
    if (!readMetaStr("bundleName", bundleName) ||
        bundleName.size() > MAX_BUNDLE_NAME_LEN ||
        !readMetaStr("processName", processName) ||
        processName.size() > MAX_MODULE_NAME_LEN) {
        errInfo = "meta fields over length cap";
        return false;
    }
    std::map<std::string, ModuleStat> loaded;
    for (const auto& item : *modules) {
        std::string moduleName;
        ModuleStat stat;
        if (!ParseStatEntry(item, moduleName, stat)) {
            errInfo = "module entry invalid";
            return false;
        }
        loaded[moduleName] = stat;
    }
    meta.bundleName = bundleName;
    meta.processName = processName;
    meta.appVersionCode = appVersion->get<int64_t>();
    meta.launchIndex = launchIndex->get<uint32_t>();
    const auto scenario = root.find("scenario");
    meta.scenario = (scenario != root.end() && scenario->is_string())
        ? scenario->get_ref<const std::string&>() : "unknown";
    stats = std::move(loaded);
    return true;
}

void NativeModuleCollector::MergeRound(const std::vector<CollectedModule>& round,
    std::map<std::string, ModuleStat>& stats, uint32_t launchIndex, bool coldStart,
    const std::map<std::string, uint32_t>& consumerPreloads)
{
    const CollectorParams& params = GetParams();
    for (const auto& module : round) {
        if (module.moduleName.empty()) {
            continue;
        }
        ModuleStat& stat = stats[module.moduleName];
        if (stat.lastSeenLaunch == launchIndex) {
            continue; // duplicate entry in the same round: reward at most once
        }
        auto preloadIt = consumerPreloads.find(module.moduleName);
        if (preloadIt != consumerPreloads.end() && module.loadCount >= stat.loadCount &&
            (module.loadCount - stat.loadCount) <= preloadIt->second) {
            // only consumer-preloaded this round, no evidence of a real app import
            // (spec R-9): refresh the observed counter, leave score/hits untouched
            stat.loadCount = module.loadCount;
            continue;
        }
        if (coldStart && stat.hits == 0 && stat.lastSeenLaunch == 0) {
            stat.score = 1.0f; // first round is fully trusted (spec R-7)
        } else {
            stat.score = params.alpha + (1.0f - params.alpha) * stat.score;
        }
        stat.fileName = module.fileName;
        stat.isAppModule = module.isAppModule;
        stat.loadCount = module.loadCount;
        if (stat.hits == 0 && stat.lastSeenLaunch == 0) {
            stat.firstStage = module.firstStage;
        }
        stat.hits++;
        stat.lastSeenLaunch = launchIndex;
    }
    for (auto& [name, stat] : stats) {
        bool hitThisRound = (stat.lastSeenLaunch == launchIndex);
        if (!hitThisRound) {
            stat.score = (1.0f - params.alpha) * stat.score;
        }
    }
}

void NativeModuleCollector::EvictStats(std::map<std::string, ModuleStat>& stats, uint32_t launchIndex)
{
    const CollectorParams& params = GetParams();
    for (auto it = stats.begin(); it != stats.end();) {
        bool decayedOut = it->second.score < params.evictScore;
        bool longUnseen = (launchIndex >= it->second.lastSeenLaunch) &&
            (launchIndex - it->second.lastSeenLaunch >= params.evictRounds);
        if (decayedOut || longUnseen) {
            it = stats.erase(it);
        } else {
            ++it;
        }
    }
    if (stats.size() <= params.maxEntries) {
        return;
    }
    std::vector<std::pair<float, std::string>> byScore;
    byScore.reserve(stats.size());
    for (const auto& [name, stat] : stats) {
        byScore.emplace_back(stat.score, name);
    }
    // ascending: lowest score first, name as tie-breaker for determinism
    std::sort(byScore.begin(), byScore.end());
    size_t toRemove = stats.size() - params.maxEntries;
    for (size_t i = 0; i < toRemove; ++i) {
        stats.erase(byScore[i].second);
    }
}

std::vector<std::string> NativeModuleCollector::ComputePredicted(
    const std::map<std::string, ModuleStat>& stats)
{
    const CollectorParams& params = GetParams();
    std::vector<std::string> predicted;
    for (const auto& [name, stat] : stats) {
        if (stat.score >= params.predictThreshold) {
            predicted.push_back(name);
        }
    }
    return predicted;
}

std::string NativeModuleCollector::SerializeStats(const StoreMeta& meta,
    const std::map<std::string, ModuleStat>& stats)
{
    nlohmann::json root;
    root["version"] = STORE_FILE_VERSION;
    root["bundleName"] = meta.bundleName;
    root["processName"] = meta.processName;
    root["appVersionCode"] = meta.appVersionCode;
    root["launchIndex"] = meta.launchIndex;
    root["updatedAt"] = NowMillis();
    root["scenario"] = meta.scenario;
    root["predicted"] = ComputePredicted(stats);
    nlohmann::json modules = nlohmann::json::array();
    for (const auto& [name, stat] : stats) {
        nlohmann::json item;
        item["moduleName"] = name;
        item["fileName"] = stat.fileName;
        item["isAppModule"] = stat.isAppModule;
        item["score"] = std::round(static_cast<double>(stat.score) * SCORE_PRECISION) / SCORE_PRECISION;
        item["hits"] = stat.hits;
        item["lastSeenLaunch"] = stat.lastSeenLaunch;
        item["firstStage"] = stat.firstStage; // numeric 0/1, must round-trip with ParseStatEntry
        item["loadCount"] = stat.loadCount;
        modules.push_back(std::move(item));
    }
    root["modules"] = std::move(modules);
    return root.dump();
}

bool NativeModuleCollector::WriteAtomic(const std::string& path, const std::string& content)
{
    std::string tmpPath = path + STORE_TMP_SUFFIX;
    if (!WriteFileWithPermission(tmpPath, content)) {
        return false;
    }
    if (rename(tmpPath.c_str(), path.c_str()) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "rename to %{public}s failed, errno %{public}d", path.c_str(), errno);
        (void)remove(tmpPath.c_str());
        return false;
    }
    // rename() replaces the destination atomically; the mode of tmp (0600) is kept
    return true;
}

bool NativeModuleCollector::MergeAndSave(const std::string& storePath, const std::string& bundleName,
    const std::string& processName, int64_t appVersionCode, const std::string& scenario,
    const std::vector<CollectedModule>& round, const std::map<std::string, uint32_t>& consumerPreloads)
{
    std::map<std::string, ModuleStat> stats;
    StoreMeta meta;
    std::string errInfo;
    bool hasHistory = LoadStats(storePath, stats, meta, errInfo);
    if (!hasHistory) {
        TAG_LOGI(AAFwkTag::APPKIT, "rebuild stats: %{public}s", errInfo.c_str());
        stats.clear();
        meta = StoreMeta {};
    } else if (meta.appVersionCode != appVersionCode) {
        TAG_LOGI(AAFwkTag::APPKIT, "rebuild stats: app upgraded");
        hasHistory = false;
        stats.clear();
        meta = StoreMeta {};
    }
    bool coldStart = !hasHistory;
    // snapshot of the previous prediction, for beta hit-rate metrics
    std::vector<std::string> predictedPrev;
    if (hasHistory && GetParams().reportHitRate) {
        predictedPrev = ComputePredicted(stats);
    }
    meta.bundleName = bundleName;
    meta.processName = processName;
    meta.appVersionCode = appVersionCode;
    meta.scenario = scenario.empty() ? "unknown" : scenario;
    meta.launchIndex++;

    MergeRound(round, stats, meta.launchIndex, coldStart, consumerPreloads);
    EvictStats(stats, meta.launchIndex);

    size_t slashPos = storePath.find_last_of('/');
    if (slashPos == std::string::npos || slashPos == 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid store path: %{public}s", storePath.c_str());
        return false;
    }
    std::string dir = storePath.substr(0, slashPos);
    if (!EnsureDir(dir)) {
        TAG_LOGE(AAFwkTag::APPKIT, "ensure dir failed: %{public}s", dir.c_str());
        return false;
    }
    bool written = WriteAtomic(storePath, SerializeStats(meta, stats));
    if (written && hasHistory && GetParams().reportHitRate) {
        HitRateMetrics metrics = ComputeHitRateMetrics(predictedPrev, round);
        TAG_LOGI(AAFwkTag::APPKIT,
            "hitRate %{public}.3f falseRate %{public}.3f predicted %{public}zu actual %{public}zu",
            metrics.hitRate, metrics.falseRate, metrics.predictedCount, metrics.actualCount);
        // beta only: report prediction quality for parameter calibration (gated by
        // ccm.ability.native_module_collect.report_hitrate, off in production)
        AAFwk::HisyseventReport report(5);
        report.InsertParam("HIT_RATE", metrics.hitRate);
        report.InsertParam("FALSE_RATE", metrics.falseRate);
        report.InsertParam("PREDICTED_COUNT", static_cast<uint64_t>(metrics.predictedCount));
        report.InsertParam("ACTUAL_COUNT", static_cast<uint64_t>(metrics.actualCount));
        report.InsertParam("HIT_COUNT", static_cast<uint64_t>(metrics.hitCount));
        (void)report.Report("AAFWK", "NATIVE_MODULE_COLLECT_HITRATE", HISYSEVENT_STATISTIC);
    }
    return written;
}

NativeModuleCollector::HitRateMetrics NativeModuleCollector::ComputeHitRateMetrics(
    const std::vector<std::string>& predictedPrev, const std::vector<CollectedModule>& round)
{
    HitRateMetrics metrics;
    metrics.predictedCount = predictedPrev.size();
    metrics.actualCount = round.size();
    std::set<std::string> actual;
    for (const auto& module : round) {
        actual.insert(module.moduleName);
    }
    for (const auto& name : predictedPrev) {
        if (actual.count(name) != 0) {
            metrics.hitCount++;
        }
    }
    metrics.hitRate = metrics.actualCount == 0
        ? 1.0f : static_cast<float>(metrics.hitCount) / static_cast<float>(metrics.actualCount);
    size_t falseCount = metrics.predictedCount - metrics.hitCount;
    metrics.falseRate = metrics.predictedCount == 0
        ? 0.0f : static_cast<float>(falseCount) / static_cast<float>(metrics.predictedCount);
    return metrics;
}

std::vector<std::string> NativeModuleCollector::LoadPredictedSystemModules(const std::string& storePath)
{
    std::vector<std::string> result;
    std::map<std::string, ModuleStat> stats;
    StoreMeta meta;
    std::string errInfo;
    if (!LoadStats(storePath, stats, meta, errInfo)) {
        TAG_LOGI(AAFwkTag::APPKIT, "no usable store file: %{public}s", errInfo.c_str());
        return result;
    }
    const CollectorParams& params = GetParams();
    for (const auto& [name, stat] : stats) {
        if (!stat.isAppModule && stat.score >= params.predictThreshold) {
            result.push_back(name);
        }
    }
    return result;
}

void NativeModuleCollector::RecordConsumerPreload(const std::string& moduleName)
{
    if (moduleName.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_state.mutex);
    g_state.consumerPreloads[moduleName]++;
}
} // namespace AppExecFwk
} // namespace OHOS
