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

#ifndef OHOS_ABILITY_RUNTIME_NATIVE_MODULE_COLLECTOR_H
#define OHOS_ABILITY_RUNTIME_NATIVE_MODULE_COLLECTOR_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {

/*
 * Tunables for native-module collect & prediction. Scores are kept in milli
 * scale so the EWMA math is integer-deterministic.
 */
struct CollectConfig {
    uint32_t collectWindowMs = 5000;    // startup collect window (HandleLaunchApplication + 5s)
    uint32_t maxPredictModules = 32;    // upper bound of the predicted set
    uint32_t ewmaAlphaMilli = 500;      // EWMA alpha, valid range (0, 1000]
    uint32_t scoreThresholdMilli = 200; // predicted-set admission threshold (0, 1000)

    // Reset invalid fields to defaults; shared by config parsing and collector init.
    static CollectConfig Sanitize(const CollectConfig& config);
};

/*
 * Collects system native modules explicitly imported during app startup,
 * persists them into a sub directory of the app sandbox cache dir, and keeps an
 * EWMA-based predicted set that the startup framework preloads on the next
 * launch.
 *
 * Persistence layout: <cacheDir>/native_module_collect/{collected.json,predict.json,.lock}.
 * Concurrent app instances (multiple processes of the same app) are serialized
 * by a cross-process flock on ".lock": each instance re-reads the latest
 * committed state, applies its own hits and commits atomically, so concurrent
 * hits are merged without lost updates.
 *
 * Every failure path degrades to logging; app startup is never blocked.
 */
class NativeModuleCollector {
public:
    // Process-local static singleton (custom static pattern; GetInstance is defined in native_module_collector.cpp).
    static NativeModuleCollector& GetInstance();

    // Switch set by the startup framework (system parameter). When disabled, no
    // file IO and no prediction behavior happens at all.
    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    // cacheDir: app sandbox cache directory; files live under
    // <cacheDir>/native_module_collect/<moduleName>/ so collection and preload are
    // scoped per launched module. Thread-safe; effective only on the first call.
    void Init(const std::string& cacheDir, const std::string& moduleName, const CollectConfig& config);
    bool IsInitialized() const;

    // Reads the predicted set. Missing/corrupted/unsupported content is treated
    // as empty history. Returns predicted system module names.
    std::vector<std::string> LoadPredictedModules();

    // Snapshots loaded system native modules, persists this launch's explicitly
    // imported system modules, updates EWMA scores and atomically rewrites the
    // predict file. Merge runs under a cross-process lock; if the lock is
    // unavailable the round is skipped without affecting startup.
    void CollectAndPersist();

private:
    NativeModuleCollector();
    ~NativeModuleCollector();
    NativeModuleCollector(const NativeModuleCollector&) = delete;
    NativeModuleCollector& operator=(const NativeModuleCollector&) = delete;

    std::string CollectDir() const;
    std::string CollectedFilePath() const;
    std::string PredictFilePath() const;
    std::string LockFilePath() const;
    bool EnsureCollectDir() const;
    bool ReadPredictFile(const std::string& predictPath, std::vector<std::string>& predicted,
        std::map<std::string, uint32_t>& scores) const;
    bool WriteCollectedFile(const std::vector<std::string>& collected) const;
    bool WritePredictFile(const std::map<std::string, uint32_t>& scores,
        const std::vector<std::string>& predicted) const;
    bool AtomicWriteFile(const std::string& path, const std::string& content) const;
    bool AcquireLock(int& fd) const;
    void UpdateScoresAndPredict(std::map<std::string, uint32_t>& scores,
        const std::vector<std::string>& hits, std::vector<std::string>& predicted) const;
    // Override the config with system parameters (const.abilityruntime.nativepreload.*)
    // so the EWMA parameters can be tuned without a rebuild, e.g. for testing.
    void LoadConfigFromParams();

    bool enabled_ = false;
    bool initialized_ = false;
    // Only persist collected.json (the raw per-launch snapshot) when the debug
    // parameter is on; predict.json is always persisted.
    bool debugCollectEnabled_ = false;
    std::string cacheDir_;
    std::string moduleName_;
    CollectConfig config_;
    // Guards Init/SetEnabled against concurrent access; the score table is not
    // held in memory between rounds (re-read under the cross-process lock).
    mutable std::mutex mutex_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_NATIVE_MODULE_COLLECTOR_H
