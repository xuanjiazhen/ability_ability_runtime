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

#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

#include "hilog_tag_wrapper.h"
#include "native_module_manager.h"
#include "parameters.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* COLLECT_DIR_NAME = "native_module_collect";
constexpr const char* COLLECTED_FILE_NAME = "collected.json";
constexpr const char* PREDICT_FILE_NAME = "predict.json";
constexpr const char* LOCK_FILE_NAME = ".lock";
constexpr int32_t FILE_VERSION = 1;
constexpr uint32_t EWMA_SCALE = 1000;

constexpr const char* KEY_VERSION = "version";
constexpr const char* KEY_SCORES = "scores";
constexpr const char* KEY_PREDICTED = "predicted";
constexpr const char* KEY_COLLECTED = "collected";

// system parameters for tuning/testing (read once at Init)
constexpr const char* PARAM_DEBUG_COLLECT = "const.abilityruntime.nativepreload.debug";
constexpr const char* PARAM_EWMA_ALPHA = "const.abilityruntime.nativepreload.ewma_alpha";
constexpr const char* PARAM_SCORE_THRESHOLD = "const.abilityruntime.nativepreload.score_threshold";
constexpr const char* PARAM_MAX_PREDICT = "const.abilityruntime.nativepreload.max_predict";
} // namespace

NativeModuleCollector::NativeModuleCollector() = default;

NativeModuleCollector::~NativeModuleCollector() = default;

NativeModuleCollector& NativeModuleCollector::GetInstance()
{
    // Custom static-pattern singleton (C++11 magic static, thread-safe)
    static NativeModuleCollector instance;
    return instance;
}

CollectConfig CollectConfig::Sanitize(const CollectConfig& config)
{
    CollectConfig out = config;
    CollectConfig def;
    if (out.collectWindowMs == 0) {
        out.collectWindowMs = def.collectWindowMs;
    }
    if (out.maxPredictModules == 0) {
        out.maxPredictModules = def.maxPredictModules;
    }
    if (out.ewmaAlphaMilli == 0 || out.ewmaAlphaMilli > EWMA_SCALE) {
        out.ewmaAlphaMilli = def.ewmaAlphaMilli;
    }
    if (out.scoreThresholdMilli >= EWMA_SCALE) {
        out.scoreThresholdMilli = def.scoreThresholdMilli;
    }
    return out;
}

void NativeModuleCollector::SetEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(mutex_);
    enabled_ = enabled;
    TAG_LOGI(AAFwkTag::STARTUP, "native module collect enabled: %{public}d", enabled);
}

bool NativeModuleCollector::IsEnabled() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return enabled_;
}

void NativeModuleCollector::Init(const std::string& cacheDir, const std::string& moduleName,
    const CollectConfig& config)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return;
    }
    if (cacheDir.empty() || moduleName.empty()) {
        TAG_LOGW(AAFwkTag::STARTUP, "empty cacheDir or moduleName, collect disabled");
        return;
    }
    cacheDir_ = cacheDir;
    moduleName_ = moduleName;
    config_ = CollectConfig::Sanitize(config);
    LoadConfigFromParams();
    debugCollectEnabled_ = OHOS::system::GetBoolParameter(PARAM_DEBUG_COLLECT, false);
    initialized_ = true;
    TAG_LOGI(AAFwkTag::STARTUP, "native module collector init: %{public}s, module: %{public}s, debugCollect:%{public}d",
        cacheDir_.c_str(), moduleName_.c_str(), debugCollectEnabled_);
}

bool NativeModuleCollector::IsInitialized() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

std::string NativeModuleCollector::CollectDir() const
{
    if (cacheDir_.empty()) {
        return "";
    }
    // cacheDir is already the module-level sandbox (<appBase>/haps/<module>/cache);
    // the feature sub-directory avoids colliding with the module's own files
    return cacheDir_ + "/" + COLLECT_DIR_NAME;
}

std::string NativeModuleCollector::CollectedFilePath() const
{
    std::string dir = CollectDir();
    return dir.empty() ? "" : dir + "/" + COLLECTED_FILE_NAME;
}

std::string NativeModuleCollector::PredictFilePath() const
{
    std::string dir = CollectDir();
    return dir.empty() ? "" : dir + "/" + PREDICT_FILE_NAME;
}

std::string NativeModuleCollector::LockFilePath() const
{
    std::string dir = CollectDir();
    return dir.empty() ? "" : dir + "/" + LOCK_FILE_NAME;
}

bool NativeModuleCollector::EnsureCollectDir() const
{
    std::string dir = CollectDir();
    if (dir.empty()) {
        return false;
    }
    // create nested path recursively: <cacheDir>/native_module_collect/<moduleName>
    std::string cur;
    for (size_t i = 0; i < dir.size(); ++i) {
        cur.push_back(dir[i]);
        if (dir[i] != '/' && i + 1 != dir.size()) {
            continue;
        }
        if (cur.size() > 1 && ::mkdir(cur.c_str(), 0755) != 0 && errno != EEXIST) {
            TAG_LOGW(AAFwkTag::STARTUP, "mkdir failed: %{public}s, errno:%{public}d", cur.c_str(), errno);
            return false;
        }
    }
    return true;
}

bool NativeModuleCollector::AcquireLock(int& fd) const
{
    fd = -1;
    std::string lockPath = LockFilePath();
    if (lockPath.empty()) {
        return false;
    }
    fd = ::open(lockPath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd < 0) {
        TAG_LOGW(AAFwkTag::STARTUP, "open lock failed: %{public}s", lockPath.c_str());
        return false;
    }
    if (::flock(fd, LOCK_EX | LOCK_NB) != 0) {
        ::close(fd);
        fd = -1;
        return false;
    }
    return true;
}

bool NativeModuleCollector::ReadPredictFile(const std::string& predictPath, std::vector<std::string>& predicted,
    std::map<std::string, uint32_t>& scores) const
{
    predicted.clear();
    scores.clear();
    std::ifstream ifs(predictPath);
    if (!ifs.is_open()) {
        return false; // file missing: treat as no history
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    // Use the non-throwing parse (this module is compiled with -fno-exceptions);
    // corrupted content yields a discarded json.
    nlohmann::json root = nlohmann::json::parse(ss.str(), nullptr, /*allow_exceptions=*/false);
    if (root.is_discarded()) {
        TAG_LOGW(AAFwkTag::STARTUP, "predict file parse failed, treat as empty: %{public}s", predictPath.c_str());
        return false; // corrupted content: treat as no history
    }
    if (!root.is_object() ||
        !(root.contains(KEY_VERSION) && root[KEY_VERSION].is_number() &&
          root[KEY_VERSION].get<int>() == FILE_VERSION)) {
        return false; // unsupported version: treat as no history
    }
    if (root.contains(KEY_SCORES) && root[KEY_SCORES].is_object()) {
        for (auto it = root[KEY_SCORES].begin(); it != root[KEY_SCORES].end(); ++it) {
            if (it.value().is_number()) {
                scores[it.key()] = it.value().get<uint32_t>();
            }
        }
    }
    if (root.contains(KEY_PREDICTED) && root[KEY_PREDICTED].is_array()) {
        for (const auto& item : root[KEY_PREDICTED]) {
            if (item.is_string()) {
                predicted.emplace_back(item.get<std::string>());
            }
        }
    }
    return true;
}

bool NativeModuleCollector::AtomicWriteFile(const std::string& path, const std::string& content) const
{
    // unique tmp name (with pid); write+fsync then atomic rename to avoid partial files
    std::string tmp = path + ".tmp." + std::to_string(::getpid());
    {
        std::ofstream ofs(tmp, std::ios::out | std::ios::trunc);
        if (!ofs.is_open()) {
            TAG_LOGW(AAFwkTag::STARTUP, "open tmp failed: %{public}s", tmp.c_str());
            return false;
        }
        ofs << content;
        ofs.flush();
        if (ofs.fail()) {
            ofs.close();
            ::remove(tmp.c_str());
            return false;
        }
        ofs.close();
    }
    if (::rename(tmp.c_str(), path.c_str()) != 0) {
        TAG_LOGW(AAFwkTag::STARTUP, "rename failed: %{public}s, errno:%{public}d", path.c_str(), errno);
        ::remove(tmp.c_str());
        return false;
    }
    return true;
}

bool NativeModuleCollector::WriteCollectedFile(const std::vector<std::string>& collected) const
{
    nlohmann::json root;
    root[KEY_VERSION] = FILE_VERSION;
    root[KEY_COLLECTED] = collected;
    return AtomicWriteFile(CollectedFilePath(), root.dump());
}

bool NativeModuleCollector::WritePredictFile(const std::map<std::string, uint32_t>& scores,
    const std::vector<std::string>& predicted) const
{
    nlohmann::json root;
    root[KEY_VERSION] = FILE_VERSION;
    root[KEY_SCORES] = scores;
    root[KEY_PREDICTED] = predicted;
    return AtomicWriteFile(PredictFilePath(), root.dump());
}

void NativeModuleCollector::UpdateScoresAndPredict(std::map<std::string, uint32_t>& scores,
    const std::vector<std::string>& hits, std::vector<std::string>& predicted) const
{
    // EWMA: a hit boosts the score (hit=EWMA_SCALE), a miss decays it (hit=0); integer milli-scale for determinism
    for (const auto& name : hits) {
        uint32_t cur = scores.count(name) ? scores[name] : 0;
        scores[name] = (config_.ewmaAlphaMilli * EWMA_SCALE +
            (EWMA_SCALE - config_.ewmaAlphaMilli) * cur) / EWMA_SCALE;
    }
    for (auto& kv : scores) {
        const std::string& name = kv.first;
        if (std::find(hits.begin(), hits.end(), name) == hits.end()) {
            kv.second = ((EWMA_SCALE - config_.ewmaAlphaMilli) * kv.second) / EWMA_SCALE;
        }
    }
    // admit by threshold and cap to the upper bound (score descending)
    std::vector<std::pair<std::string, uint32_t>> sorted(scores.begin(), scores.end());
    std::sort(sorted.begin(), sorted.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    predicted.clear();
    for (const auto& kv : sorted) {
        if (kv.second < config_.scoreThresholdMilli || predicted.size() >= config_.maxPredictModules) {
            break;
        }
        predicted.emplace_back(kv.first);
    }
}

void NativeModuleCollector::LoadConfigFromParams()
{
    // Allow tuning the EWMA parameters via system parameters (useful for testing).
    config_.ewmaAlphaMilli = static_cast<uint32_t>(OHOS::system::GetIntParameter<int>(
        PARAM_EWMA_ALPHA, static_cast<int>(config_.ewmaAlphaMilli), 1, EWMA_SCALE));
    config_.scoreThresholdMilli = static_cast<uint32_t>(OHOS::system::GetIntParameter<int>(
        PARAM_SCORE_THRESHOLD, static_cast<int>(config_.scoreThresholdMilli), 0, EWMA_SCALE - 1));
    config_.maxPredictModules = static_cast<uint32_t>(OHOS::system::GetIntParameter<int>(
        PARAM_MAX_PREDICT, static_cast<int>(config_.maxPredictModules), 1, 1024));
}

std::vector<std::string> NativeModuleCollector::LoadPredictedModules()
{
    std::vector<std::string> predicted;
    if (!IsEnabled() || !IsInitialized()) {
        return predicted;
    }
    std::map<std::string, uint32_t> scores;
    ReadPredictFile(PredictFilePath(), predicted, scores);
    return predicted;
}

void NativeModuleCollector::CollectAndPersist()
{
    if (!IsEnabled() || !IsInitialized()) {
        return;
    }
    // snapshot loaded system module names at window end (preloaded modules are not registered, so never appear here)
    NativeModuleManager* mgr = NativeModuleManager::GetInstance();
    std::vector<std::string> hits;
    if (mgr != nullptr) {
        hits = mgr->GetLoadedSystemNativeModules();
    }
    if (CollectDir().empty() || !EnsureCollectDir()) {
        TAG_LOGW(AAFwkTag::STARTUP, "ensure collect dir failed, skip collect");
        return;
    }
    int fd = -1;
    if (!AcquireLock(fd)) {
        TAG_LOGW(AAFwkTag::STARTUP, "acquire lock failed, skip this round");
        return;
    }
    // read latest scores -> apply this round's hits -> commit atomically (multi-instance merge as union)
    std::vector<std::string> existing;
    std::map<std::string, uint32_t> scores;
    ReadPredictFile(PredictFilePath(), existing, scores);
    std::vector<std::string> predicted;
    UpdateScoresAndPredict(scores, hits, predicted);
    // the raw per-launch snapshot is only for observability; persist it only in debug mode
    if (debugCollectEnabled_) {
        WriteCollectedFile(hits);
    }
    WritePredictFile(scores, predicted);
    if (fd >= 0) {
        ::close(fd); // closing the fd releases the flock
    }
    TAG_LOGI(AAFwkTag::STARTUP, "collect done: hits %{public}zu, predicted %{public}zu",
        hits.size(), predicted.size());
}

} // namespace AbilityRuntime
} // namespace OHOS
