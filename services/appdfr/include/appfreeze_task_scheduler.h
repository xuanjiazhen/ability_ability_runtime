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
#ifndef OHOS_ABILITY_RUNTIME_APPFREEZE_TASK_SCHEDULER_H
#define OHOS_ABILITY_RUNTIME_APPFREEZE_TASK_SCHEDULER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

namespace OHOS {
namespace AppExecFwk {
// Owned only by requests in an unfinished episode. A failed send releases its
// severity bit so a later admitted report can still publish that severity.
class AppfreezeReportClaim {
public:
    __attribute__((visibility("hidden"))) bool TryBegin(bool half);
    __attribute__((visibility("hidden"))) void End(bool half, int reportResult);
private:
    std::atomic<uint8_t> claims_ {0};
};

struct AppfreezeTaskMetadata {
    std::string generationId;
    std::string episodeId;
    std::string fingerprint;
    std::string freezeSeverity;
    std::shared_ptr<AppfreezeReportClaim> reportClaim;
    bool stormTrigger = false;
    std::string stormGenerationId;
    int64_t processStartTime = 0;
    int64_t freezeAppCount1min = -1; // -1 is absent, not a measured zero.
};

struct AppfreezeCaptureResult {
    std::string stack;
    std::string binderInfo;
    std::string memoryContent;
    int dumpResult = -1;
    int64_t dumpLatency = -1;
    int64_t processStart = 0;
};

class AppfreezeRequest {
public:
    AppfreezeRequest(int32_t instanceId, bool half, AppfreezeTaskMetadata metadata);
    const uint64_t requestId;
    const int32_t instanceId;
    const bool half;
    AppfreezeTaskMetadata metadata;
    bool IsCancelled() const { return cancelled_.load(); }

private:
    friend class AppfreezeTaskScheduler;
    std::atomic<bool> cancelled_ {false};
    std::atomic<bool> completed_ {false};
    bool submitted_ = false;
};

enum class AppfreezeSubmitResult { DISPATCHED, QUEUED, COALESCED, DUPLICATE, REJECTED };

// Waiters never enter the executor. A cancelled RUNNING request keeps its key
// until its worker exits; IDLE entries retain no capture or completed cache.
class AppfreezeTaskScheduler : public std::enable_shared_from_this<AppfreezeTaskScheduler> {
public:
    using Request = std::shared_ptr<AppfreezeRequest>;
    using Result = std::shared_ptr<const AppfreezeCaptureResult>;
    using Task = std::function<void()>;
    using Executor = std::function<bool(Task)>;
    using Clock = std::function<int64_t()>;
    using Capture = std::function<Result(const Request&)>;
    using Completion = std::function<void(const Request&, const Result&)>;
    using Start = std::function<void(const Request&)>;

    explicit AppfreezeTaskScheduler(Executor executor, Clock clock = {});
    ~AppfreezeTaskScheduler();
    AppfreezeTaskMetadata Observe(int32_t pid, int32_t uid);
    AppfreezeSubmitResult Submit(const Request& request, Capture capture, Completion completion, Start start = {});
    __attribute__((visibility("hidden"))) void Cancel(const Request& request);
    void RemoveGeneration(const std::string& generation, int32_t instanceId);
    void CloseAll();
    void Reset();
    void Expire();
    size_t PendingSize(const std::string& generation) const;
    size_t GenerationSize() const;
    size_t RecentAppSize() const;
    static bool IsHalfEvent(const std::string& name);
    __attribute__((visibility("hidden"))) static bool IsUpgradeEvent(const std::string& name);

private:
    struct Work {
        Request request;
        Capture capture;
        Completion completion;
        Start start;
        bool running = false;
        bool closing = false;
        bool reserved = false;
        int64_t deadline = 0;
    };
    struct Generation {
        int32_t instanceId = 0;
        bool accepting = true;
        std::shared_ptr<Work> active;
        std::vector<std::shared_ptr<Work>> pending;
        std::vector<std::shared_ptr<Work>> waiters;
    };
    __attribute__((visibility("hidden"))) bool Dispatch(const std::shared_ptr<Work>& work);
    __attribute__((visibility("hidden"))) bool TryStart(const std::shared_ptr<Work>& work);
    __attribute__((visibility("hidden"))) void Run(const std::shared_ptr<Work>& work);
    __attribute__((visibility("hidden"))) void Finish(const std::shared_ptr<Work>& work);
    __attribute__((visibility("hidden"))) static void Complete(const std::shared_ptr<Work>& work, const Result& result);
    __attribute__((visibility("hidden"))) static void CloseGeneration(Generation& state,
        std::vector<std::shared_ptr<Work>>& discarded);
    __attribute__((visibility("hidden"))) void ExpireLocked(int64_t now);
    __attribute__((visibility("hidden"))) void Maintain();
    Executor executor_;
    Clock clock_;
    mutable std::mutex mutex_;
    std::condition_variable wake_;
    std::map<std::string, Generation> generations_;
    // One expiry node per UID, updated in place on every observed event.
    std::map<int32_t, int64_t> recentApps_;
    std::set<std::pair<int64_t, int32_t>> appExpiries_;
    std::map<std::string, int64_t> stormEntries_;
    bool stormTriggered_ = false;
    uint64_t stormSequence_ = 0;
    int64_t lastEpisode_ = 0;
    bool stopping_ = false;
    std::thread maintenance_;
};

// Two managed execution slots. Timeout closes publication, but never releases
// an executing slot early. The destructor joins both collectors.
class AppfreezeAttachmentPool {
public:
    struct Result {
        std::string binderInfo;
        std::string memoryContent;
    };
    using Cancelled = std::function<bool()>;
    using Collector = std::function<Result(const Cancelled&)>;
    __attribute__((visibility("hidden"))) AppfreezeAttachmentPool();
    __attribute__((visibility("hidden"))) ~AppfreezeAttachmentPool();
    __attribute__((visibility("hidden"))) Result Collect(Collector collector, Cancelled cancelled,
        int64_t budgetMs = 500);
    __attribute__((visibility("hidden"))) size_t ActiveSize() const;

private:
    struct Job {
        Collector collector;
        Cancelled cancelled;
        std::mutex mutex;
        std::condition_variable done;
        std::atomic<bool> closed {false};
        bool finished = false;
        Result result;
        std::chrono::steady_clock::time_point deadline;
    };
    __attribute__((visibility("hidden"))) void Run();
    mutable std::mutex mutex_;
    std::condition_variable wake_;
    std::vector<std::shared_ptr<Job>> pending_;
    std::vector<std::shared_ptr<Job>> active_;
    bool stopping_ = false;
    std::thread workers_[2];
};
} // namespace AppExecFwk
} // namespace OHOS
#endif
