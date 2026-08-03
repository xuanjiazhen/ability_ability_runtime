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
#include <cmath>
#include <cstdint>
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

// libbegetutil only instantiates GetIntParameter for {u,}int{8,16,32,64}_t, but some
// toolchains link only the signed small-width variants; int32_t is always available.
uint32_t GetUint32Parameter(const std::string& key, uint32_t def, uint32_t minVal, uint32_t maxVal)
{
    int64_t raw = OHOS::system::GetIntParameter<int64_t>(
        key, static_cast<int64_t>(def), static_cast<int64_t>(minVal), static_cast<int64_t>(maxVal));
    return static_cast<uint32_t>(raw);
}

CollectorParams LoadParamsFromSystem()
{
    CollectorParams params;
    params.alpha = GetFloatParameter(PARAM_ALPHA, params.alpha, 0.05f, 0.95f);
    params.predictThreshold = GetFloatParameter(PARAM_PREDICT_THRESHOLD, params.predictThreshold, 0.05f, 0.95f);
    params.evictScore = GetFloatParameter(PARAM_EVICT_SCORE, params.evictScore, 0.001f, 0.5f);
    params.evictRounds = GetUint32Parameter(PARAM_EVICT_ROUNDS, params.evictRounds, 1u, 1000u);
    params.checkpoint2DelayMs = GetUint32Parameter(PARAM_CHECKPOINT2_DELAY_MS, params.checkpoint2DelayMs, 0u, 60000u);
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
    TAG_LOGI(AAFwkTag::APPKIT,
        "params loaded: enable=%{public}d, alpha=%{public}.3f, threshold=%{public}.3f, "
        "evictScore=%{public}.3f, evictRounds=%{public}u, cp2DelayMs=%{public}u, reportHitRate=%{public}d",
        g_paramCache.enabled, g_paramCache.params.alpha, g_paramCache.params.predictThreshold,
        g_paramCache.params.evictScore, g_paramCache.params.evictRounds,
        g_paramCache.params.checkpoint2DelayMs, g_paramCache.params.reportHitRate);
}

struct CollectorState {
    std::mutex mutex;
    std::map<std::string, CollectedModule> sampled;
    std::set<std::string> preloadKeys;
    std::set<std::string> consumerPreloads; // moduleNames preloaded by the consumer this round
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

// OpenHarmony is built with -fno-exceptions: read json fields with find()+type
// guards only, never throwing accessors.
bool ReadStatNumericFields(const nlohmann::json& item, ModuleStat& stat)
{
    const auto score = item.find("score");
    const auto lastSeen = item.find("lastSeenLaunch");
    if (score == item.end() || !score->is_number() ||
        lastSeen == item.end() || !lastSeen->is_number_unsigned()) {
        return false;
    }
    stat.score = ClampScore(score->get<double>());
    stat.lastSeenLaunch = lastSeen->get<uint32_t>();
    return true;
}

bool ParseStatEntry(const nlohmann::json& item, std::string& moduleName, ModuleStat& stat)
{
    if (!item.is_object()) {
        return false;
    }
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
    const auto relPath = item.find("relativePath");
    if (relPath != item.end() && relPath->is_string()) {
        stat.relativePath = relPath->get_ref<const std::string&>();
        if (stat.relativePath.size() > MAX_FILE_NAME_LEN) {
            return false;
        }
    }
    return ReadStatNumericFields(item, stat);
}

// ---- store-file loaders (split out to keep LoadStats under complexity/size limits) ----

bool ValidateStoreVersion(const nlohmann::json& root, std::string& errInfo)
{
    const auto version = root.find("version");
    if (version == root.end() || !version->is_number_integer() ||
        version->get<int32_t>() != STORE_FILE_VERSION) {
        errInfo = "version mismatch";
        return false;
    }
    return true;
}

bool ReadStoreMeta(const nlohmann::json& root, StoreMeta& meta, std::string& errInfo)
{
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
    meta.appVersionCode = appVersion->get<int64_t>();
    meta.launchIndex = launchIndex->get<uint32_t>();
    return true;
}

bool ReadStoreModules(const nlohmann::json& modules, std::map<std::string, ModuleStat>& loaded,
    std::string& errInfo)
{
    for (const auto& item : modules) {
        std::string moduleName;
        ModuleStat stat;
        if (!ParseStatEntry(item, moduleName, stat)) {
            errInfo = "module entry invalid";
            return false;
        }
        loaded[moduleName] = stat;
    }
    return true;
}

// ---- merge helpers (split out to keep MergeAndSave under size limits) ----

// Load history and decide cold-start; rebuilds (discard + reset) on missing /
// corrupted / app-upgrade. Fills predictedPrev only when reporting is on and
// history is usable. Returns true on cold start.
bool PrepareMergeHistory(const std::string& storePath, int64_t appVersionCode,
    std::map<std::string, ModuleStat>& stats, StoreMeta& meta,
    std::vector<std::string>& predictedPrev)
{
    std::string errInfo;
    bool hasHistory = NativeModuleCollector::LoadStats(storePath, stats, meta, errInfo);
    if (!hasHistory) {
        TAG_LOGI(AAFwkTag::APPKIT, "rebuild stats: %{public}s", errInfo.c_str());
        stats.clear();
        meta = StoreMeta {};
        return true;
    }
    if (meta.appVersionCode != appVersionCode) {
        TAG_LOGI(AAFwkTag::APPKIT, "rebuild stats: app upgraded");
        stats.clear();
        meta = StoreMeta {};
        return true;
    }
    if (NativeModuleCollector::GetParams().reportHitRate) {
        predictedPrev = NativeModuleCollector::ComputePredicted(stats);
    }
    return false;
}

void ReportHitRateIfNeeded(bool written, bool coldStart, const std::vector<std::string>& predictedPrev,
    const std::vector<CollectedModule>& round)
{
    if (!written || coldStart || !NativeModuleCollector::GetParams().reportHitRate) {
        return;
    }
    auto metrics = NativeModuleCollector::ComputeHitRateMetrics(predictedPrev, round);
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
    // format as fixed-width lowercase hex without sprintf (avoids the banned
    // snprintf; output is exactly HEX_HASH_LEN chars)
    static constexpr char HEX[] = "0123456789abcdef";
    std::string result(HEX_HASH_LEN, '0');
    for (size_t i = 0; i < HEX_HASH_LEN; ++i) {
        result[HEX_HASH_LEN - 1 - i] = HEX[(hash >> (i * 4)) & 0xF];
    }
    return result;
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
        // mkdir the segment without a trailing slash, but keep `current` intact
        // (with its slash) so the next characters append after the separator.
        // Mutating current here used to drop the separator and concatenate
        // adjacent segments (e.g. /data + storage -> /datastorage).
        std::string segment = current;
        if (!segment.empty() && segment.back() == '/') {
            segment.pop_back();
        }
        if (segment.empty()) {
            continue;
        }
        if (mkdir(segment.c_str(), STORE_DIR_MODE) != 0 && errno != EEXIST) {
            TAG_LOGE(AAFwkTag::APPKIT, "mkdir %{public}s failed, errno %{public}d", segment.c_str(), errno);
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
    size_t added = 0;
    size_t refreshed = 0;
    size_t filteredPreload = 0;
    size_t filteredApp = 0;
    for (const auto& module : modules) {
        if (module.moduleName.empty()) {
            continue;
        }
        // scope: only system modules are collected; app modules (isAppModule=true,
        // e.g. the app's own .so under the hap) are dropped at the collection entry
        if (module.isAppModule) {
            filteredApp++;
            continue;
        }
        if (g_state.preloadKeys.count(module.moduleName) != 0) {
            filteredPreload++;
            continue;
        }
        auto [it, inserted] = g_state.sampled.emplace(module.moduleName, module);
        if (inserted) {
            added++;
        } else {
            // duplicate sighting: refresh the replay path only
            it->second.relativePath = module.relativePath;
            refreshed++;
        }
    }
    TAG_LOGD(AAFwkTag::APPKIT,
        "accumulate stage=%{public}d in=%{public}zu added=%{public}zu refreshed=%{public}zu "
        "filteredPreload=%{public}zu filteredApp=%{public}zu total=%{public}zu",
        firstStage, modules.size(), added, refreshed, filteredPreload, filteredApp, g_state.sampled.size());
}

bool NativeModuleCollector::SaveNow(const std::string& cacheDir, const std::string& processName,
    int64_t appVersionCode)
{
    if (!IsEnabled()) {
        return false;
    }
    std::vector<CollectedModule> round;
    std::set<std::string> consumerPreloads;
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
    TAG_LOGI(AAFwkTag::APPKIT,
        "save: sampled=%{public}zu consumerPreloads=%{public}zu async=%{public}d path=%{public}s",
        round.size(), consumerPreloads.size(), asyncSave ? 1 : 0, storePath.c_str());
    if (asyncSave) {
        std::thread([storePath, appVersionCode, round, consumerPreloads]() {
            (void)MergeAndSave(storePath, appVersionCode, round, consumerPreloads);
        }).detach();
        return true;
    }
    return MergeAndSave(storePath, appVersionCode, round, consumerPreloads);
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

CollectorParams NativeModuleCollector::GetParams()
{
    EnsureParamsLoaded();
    std::lock_guard<std::mutex> lock(g_paramCache.mutex);
    return g_paramCache.params;
}

void NativeModuleCollector::ResetParamCacheForTest()
{
    std::lock_guard<std::mutex> lock(g_paramCache.mutex);
    g_paramCache.loaded = false;
    g_paramCache.params = CollectorParams{}; // drop any override from a prior test
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
    if (!ValidateStoreVersion(root, errInfo)) {
        return false;
    }
    StoreMeta parsed;
    if (!ReadStoreMeta(root, parsed, errInfo)) {
        return false;
    }
    const auto modules = root.find("modules");
    std::map<std::string, ModuleStat> loaded;
    if (!ReadStoreModules(*modules, loaded, errInfo)) {
        return false;
    }
    meta = std::move(parsed);
    stats = std::move(loaded);
    return true;
}

void NativeModuleCollector::MergeRound(const std::vector<CollectedModule>& round,
    std::map<std::string, ModuleStat>& stats, uint32_t launchIndex, bool coldStart,
    const std::set<std::string>& consumerPreloads)
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
        // a module preloaded by the consumer this round is never rewarded as a hit
        // (lastSeenLaunch left untouched -> decayed below); refresh its replay path.
        // This breaks the self-feedback loop without needing loadCount accounting.
        if (consumerPreloads.count(module.moduleName) != 0) {
            stat.relativePath = module.relativePath;
            continue;
        }
        if (coldStart && stat.lastSeenLaunch == 0) {
            stat.score = 1.0f; // first round is fully trusted (spec R-7)
        } else {
            stat.score = params.alpha + (1.0f - params.alpha) * stat.score;
        }
        stat.relativePath = module.relativePath;
        stat.lastSeenLaunch = launchIndex;
    }
    for (auto& [name, stat] : stats) {
        if (stat.lastSeenLaunch != launchIndex) {
            stat.score = (1.0f - params.alpha) * stat.score; // decay misses
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
    // Minimal payload: only version + version-tracking meta + per-module fields
    // needed for preload (moduleName, relativePath) and prediction (score,
    // lastSeenLaunch). No predicted array (computed on demand), no identity or
    // debug fields, to keep the store file small.
    nlohmann::json root;
    root["version"] = STORE_FILE_VERSION;
    root["appVersionCode"] = meta.appVersionCode;
    root["launchIndex"] = meta.launchIndex;
    nlohmann::json modules = nlohmann::json::array();
    for (const auto& [name, stat] : stats) {
        nlohmann::json item;
        item["moduleName"] = name;
        item["relativePath"] = stat.relativePath;
        item["score"] = std::round(static_cast<double>(stat.score) * SCORE_PRECISION) / SCORE_PRECISION;
        item["lastSeenLaunch"] = stat.lastSeenLaunch;
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

bool NativeModuleCollector::MergeAndSave(const std::string& storePath, int64_t appVersionCode,
    const std::vector<CollectedModule>& round, const std::set<std::string>& consumerPreloads)
{
    std::map<std::string, ModuleStat> stats;
    StoreMeta meta;
    std::vector<std::string> predictedPrev;
    bool coldStart = PrepareMergeHistory(storePath, appVersionCode, stats, meta, predictedPrev);

    meta.appVersionCode = appVersionCode;
    meta.launchIndex++;

    MergeRound(round, stats, meta.launchIndex, coldStart, consumerPreloads);
    size_t beforeEvict = stats.size();
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
    TAG_LOGI(AAFwkTag::APPKIT,
        "merge: launchIndex=%{public}u coldStart=%{public}d round=%{public}zu stats=%{public}zu "
        "(preEvict=%{public}zu) predicted=%{public}zu written=%{public}d",
        meta.launchIndex, coldStart ? 1 : 0, round.size(), stats.size(), beforeEvict,
        ComputePredicted(stats).size(), written ? 1 : 0);
    ReportHitRateIfNeeded(written, coldStart, predictedPrev, round);
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

std::vector<PredictedModule> NativeModuleCollector::LoadPredictedSystemModules(const std::string& storePath)
{
    std::vector<PredictedModule> result;
    std::map<std::string, ModuleStat> stats;
    StoreMeta meta;
    std::string errInfo;
    if (!LoadStats(storePath, stats, meta, errInfo)) {
        TAG_LOGI(AAFwkTag::APPKIT, "no usable store file: %{public}s", errInfo.c_str());
        return result;
    }
    const CollectorParams& params = GetParams();
    for (const auto& [name, stat] : stats) {
        if (stat.score >= params.predictThreshold) {
            result.push_back({ name, stat.relativePath });
        }
    }
    TAG_LOGD(AAFwkTag::APPKIT, "predicted system modules: %{public}zu", result.size());
    return result;
}

void NativeModuleCollector::RecordConsumerPreload(const std::string& moduleName)
{
    if (moduleName.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_state.mutex);
    g_state.consumerPreloads.insert(moduleName);
}
} // namespace AppExecFwk
} // namespace OHOS
