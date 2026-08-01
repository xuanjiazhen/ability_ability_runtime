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

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace OHOS {
namespace AppExecFwk {

/**
 * @brief One sampled module of the current launch round.
 *
 * Decoupled from the napi NativeModuleInfo so the merge engine stays
 * independently testable; the checkpoint wiring converts between them.
 */
struct CollectedModule {
    std::string moduleName;   /* name used in import/require */
    std::string fileName;     /* file path the module was loaded from */
    bool isAppModule = false;
    uint32_t loadCount = 0;   /* reserved for consumer-preload disambiguation */
    uint8_t firstStage = 0;   /* 0 = app_ready, 1 = late */
};

/**
 * @brief Persisted per-module statistics, updated across launches via EWMA.
 */
struct ModuleStat {
    std::string fileName;
    bool isAppModule = false;
    float score = 0.0f;          /* EWMA hit score in [0, 1] */
    uint32_t hits = 0;           /* rounds with a confirmed hit */
    uint32_t lastSeenLaunch = 0; /* launch index of the last hit */
    uint8_t firstStage = 0;      /* stage of the first ever hit */
    uint32_t loadCount = 0;      /* loadCount observed at the last hit */
};

/**
 * @brief Tunable parameters. Defaults match the baselined spec; the system
 * parameter channel may override them (wired separately).
 */
struct CollectorParams {
    float alpha = 0.4f;            /* EWMA coefficient */
    float predictThreshold = 0.5f; /* score >= threshold enters predicted */
    float evictScore = 0.05f;      /* score < evictScore evicts the entry */
    uint32_t evictRounds = 16;     /* unseen for this many rounds evicts */
    size_t maxEntries = 1024;      /* hard cap of persisted entries */
};

/**
 * @brief Persisted file-level metadata (JSON v2).
 */
struct StoreMeta {
    std::string bundleName;
    std::string processName;
    int64_t appVersionCode = 0;
    uint32_t launchIndex = 0;   /* rounds persisted so far */
    std::string scenario = "unknown";
};
/**
 * @brief Collects native modules imported by the application during startup and
 * persists them into the application sandbox cache directory.
 *
 * The collector is gated by a system parameter (disabled by default). When the
 * switch is absent or disabled, every entry point is a no-op with zero file I/O.
 *
 * Store file layout:
 *   <cacheDir>/.ohos.ability_runtime/native_modules_<processHash8>.v2.json
 * The dot-prefixed hidden directory marks the file as system-managed so it never
 * collides with developer files. processHash8 isolates files between processes
 * of the same application. The file permission is 0600.
 */
class NativeModuleCollector {
public:
    /**
     * @brief Whether collection is enabled.
     *
     * Placeholder: always returns false until the system parameter is wired
     * (tracked separately). The result is cached per process once wired.
     */
    static bool IsEnabled();

    /**
     * @brief Build the store file path under the given base directory.
     *
     * @param baseDir application cache directory, e.g. /data/storage/el2/base/cache
     * @param processName current process name, used for the hash suffix
     * @return full store file path, e.g.
     *         <baseDir>/.ohos.ability_runtime/native_modules_a1b2c3d4.v2.json
     */
    static std::string GetStorePath(const std::string& baseDir, const std::string& processName);

    /**
     * @brief Build the store directory under the given base directory.
     */
    static std::string GetStoreDir(const std::string& baseDir);

    /**
     * @brief 8-char lowercase hex FNV-1a (32-bit) hash of the process name.
     *
     * Used only for file naming isolation between processes, not for security.
     */
    static std::string ProcessNameHash(const std::string& processName);

    /**
     * @brief Recursively create the directory if it does not exist.
     * @return true if the directory exists after the call
     */
    static bool EnsureDir(const std::string& dir);

    /**
     * @brief Write content to a file with permission 0600.
     *
     * Plain (non-atomic) write is sufficient here; the atomic tmp+rename
     * strategy lands together with the merge engine.
     *
     * @return true on success
     */
    static bool WriteFileWithPermission(const std::string& path, const std::string& content);

    /**
     * @brief Accumulate sampled modules for the current launch round.
     *
     * Modules whose key is in the preload key set are dropped (AC-1.5).
     * Duplicate keys refresh fileName/loadCount but keep the first-sighting stage.
     *
     * @param modules sampled modules (firstStage field is overwritten per entry)
     * @param firstStage stage of this sample: 0 = app_ready, 1 = late
     */
    static void AccumulateModules(const std::vector<CollectedModule>& modules, uint8_t firstStage);

    /**
     * @brief Merge accumulated samples into history and write the store file.
     *
     * At most once per launch; gated by IsEnabled(); the actual write runs on a
     * detached thread unless SetAsyncSaveForTest(false) was called.
     *
     * @return true if the save was dispatched (or performed synchronously)
     */
    static bool SaveNow(const std::string& cacheDir, const std::string& bundleName,
        const std::string& processName, int64_t appVersionCode, const std::string& scenario);

    /**
     * @brief Preload (pre-fork) module keys bridged from the runtime layer by
     * MainThread after runtime creation. Sampling filters these out.
     */
    static void SetPreloadModuleKeys(std::set<std::string> keys);
    static std::set<std::string> GetPreloadModuleKeys();

    // ----- test hooks -----
    static void SetEnabledForTest(bool enabled);
    static void SetAsyncSaveForTest(bool async);
    static void ResetStateForTest();
    static size_t GetSampledCountForTest();
    static bool GetSampledModuleForTest(const std::string& moduleName, CollectedModule& out);

    /**
     * @brief Current tunable parameters (defaults until the param channel lands).
     */
    static const CollectorParams& GetParams();

    /**
     * @brief Load persisted stats from the store file.
     *
     * Strict validation: JSON structure, file version (must be 2), entry count
     * and field length caps. Any violation discards the whole file content.
     *
     * @param path store file path
     * @param stats output stats, cleared first; untouched semantics on failure
     *        is NOT guaranteed, treat failure as "rebuild from scratch"
     * @param meta output file-level metadata
     * @param errInfo failure reason for logging
     * @return true only when a fully valid history was loaded
     */
    static bool LoadStats(const std::string& path, std::map<std::string, ModuleStat>& stats,
        StoreMeta& meta, std::string& errInfo);

    /**
     * @brief Merge one launch round into the stats via EWMA.
     *
     * Hit:  score <- alpha + (1 - alpha) * score
     * Miss: score <- (1 - alpha) * score
     * On a cold start (no prior history) every hit starts at score 1 so the
     * first round is fully trusted (spec R-7).
     *
     * A module in consumerPreloads (preloaded by the startup consumer this
     * round) only counts as a hit when its loadCount grew by more than the
     * consumer preload count, i.e. there is evidence of a real app import
     * (spec R-9); otherwise its score/hits stay untouched and only the
     * observed loadCount is refreshed.
     *
     * @param round modules confirmed imported in this launch
     * @param stats stats map to update in place
     * @param launchIndex index of the current launch round
     * @param coldStart whether this is a rebuild-from-scratch round
     * @param consumerPreloads moduleName -> preload count by the consumer this round
     */
    static void MergeRound(const std::vector<CollectedModule>& round,
        std::map<std::string, ModuleStat>& stats, uint32_t launchIndex, bool coldStart,
        const std::map<std::string, uint32_t>& consumerPreloads = {});

    /**
     * @brief Evict decayed/stale entries and enforce the entry cap.
     *
     * Removes entries with score < evictScore or unseen for evictRounds
     * launches; then evicts lowest-score entries down to maxEntries.
     */
    static void EvictStats(std::map<std::string, ModuleStat>& stats, uint32_t launchIndex);

    /**
     * @brief Compute the predicted set: modules with score >= threshold.
     */
    static std::vector<std::string> ComputePredicted(const std::map<std::string, ModuleStat>& stats);

    /**
     * @brief Serialize meta + stats into the store JSON (v2).
     */
    static std::string SerializeStats(const StoreMeta& meta, const std::map<std::string, ModuleStat>& stats);

    /**
     * @brief Atomically replace the store file (write tmp + rename), mode 0600.
     * @return true on success; on failure the formal file is untouched or absent
     */
    static bool WriteAtomic(const std::string& path, const std::string& content);

    /**
     * @brief Full round pipeline: load history (with cold-start fallbacks for
     * missing/corrupted/version-mismatched/app-upgraded files), merge, evict,
     * serialize and atomically write back.
     *
     * @return true if the file was written
     */
    static bool MergeAndSave(const std::string& storePath, const std::string& bundleName,
        const std::string& processName, int64_t appVersionCode, const std::string& scenario,
        const std::vector<CollectedModule>& round,
        const std::map<std::string, uint32_t>& consumerPreloads = {});

    /**
     * @brief Read the store file and return predicted system modules
     * (score >= threshold and isAppModule == false) for the startup consumer.
     *
     * @return module name list; empty when the file is missing/invalid or
     *         nothing qualifies
     */
    static std::vector<std::string> LoadPredictedSystemModules(const std::string& storePath);

    /**
     * @brief Record one consumer preload of a module in this launch round.
     * Used by the startup consumer so the merge engine can discount
     * consumer-triggered loads from real app imports (spec R-9).
     */
    static void RecordConsumerPreload(const std::string& moduleName);
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_NATIVE_MODULE_COLLECTOR_H
