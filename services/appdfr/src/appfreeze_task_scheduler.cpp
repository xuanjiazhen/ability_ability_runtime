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
#include "appfreeze_task_scheduler.h"

#include <algorithm>
#include <chrono>
#include <utility>

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int64_t WINDOW_MS = 60000;
constexpr int64_t CAPTURE_TIMEOUT_MS = 30000;
constexpr size_t MAX_PENDING = 2;
constexpr size_t MAX_WAITERS = 16;
constexpr size_t STORM_THRESHOLD = 3;
constexpr size_t MAX_STORM_ENTRIES = 128;
std::atomic<uint64_t> requestSequence {0};
int64_t MonotonicMillis()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
}

bool AppfreezeReportClaim::TryBegin(bool half)
{
    const uint8_t bit = half ? 1 : 2;
    return (claims_.fetch_or(bit) & bit) == 0;
}

void AppfreezeReportClaim::End(bool half, int reportResult)
{
    // HiSysEvent positive results are warnings on an event that was sent.
    if (reportResult < 0) {
        const uint8_t bit = half ? 1 : 2;
        claims_.fetch_and(static_cast<uint8_t>(~bit));
    }
}

AppfreezeRequest::AppfreezeRequest(int32_t instance, bool isHalf, AppfreezeTaskMetadata info)
    : requestId(++requestSequence), instanceId(instance), half(isHalf), metadata(std::move(info))
{
    if (!metadata.reportClaim) {
        metadata.reportClaim = std::make_shared<AppfreezeReportClaim>();
    }
}

AppfreezeTaskScheduler::AppfreezeTaskScheduler(Executor executor, Clock clock)
    : executor_(std::move(executor)), clock_(clock ? std::move(clock) : MonotonicMillis),
      maintenance_([this] { Maintain(); })
{}

AppfreezeTaskScheduler::~AppfreezeTaskScheduler()
{
    Reset();
    maintenance_.join();
}

bool AppfreezeTaskScheduler::IsHalfEvent(const std::string& name)
{
    return name == "THREAD_BLOCK_3S" || name == "LIFECYCLE_HALF_TIMEOUT" ||
        name == "LIFECYCLE_HALF_TIMEOUT_WARNING" || name == "BUSSINESS_THREAD_BLOCK_3S";
}

bool AppfreezeTaskScheduler::IsUpgradeEvent(const std::string& name)
{
    return name == "THREAD_BLOCK_6S" || name == "APP_INPUT_BLOCK" || name == "LIFECYCLE_TIMEOUT";
}

AppfreezeTaskMetadata AppfreezeTaskScheduler::Observe(int32_t pid, int32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AppfreezeTaskMetadata info;
    if (stopping_) {
        return info;
    }
    const int64_t now = clock_(); // Sample under the same lock as lastSeen updates.
    ExpireLocked(now);
    auto previous = recentApps_.find(uid);
    if (previous != recentApps_.end()) {
        appExpiries_.erase({previous->second, uid});
    }
    recentApps_[uid] = now;
    appExpiries_.insert({now, uid});
    info.freezeAppCount1min = static_cast<int64_t>(recentApps_.size());
    info.generationId = std::to_string(pid) + ":" + std::to_string(uid);
    const auto wallTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    lastEpisode_ = std::max<int64_t>(wallTime, lastEpisode_ + 1);
    info.episodeId = std::to_string(lastEpisode_);
    info.fingerprint = info.generationId + ":" + info.episodeId;
    if (stormEntries_.count(info.generationId) == 0) {
        if (stormEntries_.size() == MAX_STORM_ENTRIES) {
            auto oldest = std::min_element(stormEntries_.begin(), stormEntries_.end(),
                [](const auto& left, const auto& right) { return left.second < right.second; });
            stormEntries_.erase(oldest);
        }
        stormEntries_[info.generationId] = now;
        if (!stormTriggered_ && stormEntries_.size() >= STORM_THRESHOLD) {
            stormTriggered_ = true;
            info.stormTrigger = true;
            info.stormGenerationId = std::to_string(now) + ":" + std::to_string(++stormSequence_);
        }
    }
    wake_.notify_all();
    return info;
}

void AppfreezeTaskScheduler::Complete(const std::shared_ptr<Work>& work, const Result& result)
{
    if (work && !work->request->completed_.exchange(true) && work->completion) {
        work->completion(work->request, result);
    }
}

AppfreezeSubmitResult AppfreezeTaskScheduler::Submit(const Request& request, Capture capture,
    Completion completion, Start start)
{
    if (!request || !capture || !executor_ || request->metadata.generationId.empty()) {
        return AppfreezeSubmitResult::REJECTED;
    }
    auto work = std::make_shared<Work>();
    work->request = request;
    work->capture = std::move(capture);
    work->completion = std::move(completion);
    work->start = std::move(start);
    std::shared_ptr<Work> discarded;
    AppfreezeSubmitResult result = AppfreezeSubmitResult::DISPATCHED;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (request->submitted_) {
            return AppfreezeSubmitResult::DUPLICATE;
        }
        request->submitted_ = true;
        auto it = generations_.find(request->metadata.generationId);
        if (stopping_ || request->IsCancelled() || (it != generations_.end() &&
            (!it->second.accepting || it->second.instanceId != request->instanceId))) {
            discarded = work;
            result = AppfreezeSubmitResult::REJECTED;
        } else {
            auto& state = generations_[request->metadata.generationId];
            state.instanceId = request->instanceId;
            ExpireLocked(clock_());
            if (!state.active) {
                state.active = work;
            } else if (request->half && state.active->request->half && state.active->running &&
                !state.active->closing && !state.active->request->IsCancelled()) {
                request->metadata.episodeId = state.active->request->metadata.episodeId;
                request->metadata.fingerprint = state.active->request->metadata.fingerprint;
                request->metadata.reportClaim = state.active->request->metadata.reportClaim;
                request->metadata.processStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                if (state.waiters.size() == MAX_WAITERS) {
                    discarded = work;
                    result = AppfreezeSubmitResult::REJECTED;
                } else {
                    state.waiters.push_back(work);
                    result = AppfreezeSubmitResult::COALESCED;
                }
            } else {
                const bool activeHalf = state.active->request->half;
                const bool hasReservation = std::any_of(state.pending.begin(), state.pending.end(),
                    [](const auto& pending) { return pending->reserved; });
                work->reserved = activeHalf && !request->half && !hasReservation;
                if (activeHalf && !request->half && !state.active->closing &&
                    !state.active->request->IsCancelled()) {
                    request->metadata.episodeId = state.active->request->metadata.episodeId;
                    request->metadata.fingerprint = state.active->request->metadata.fingerprint;
                    request->metadata.reportClaim = state.active->request->metadata.reportClaim;
                }
                // A half active always leaves the first upgrade's slot available.
                const size_t limit = activeHalf && request->half && !hasReservation ? 1 : MAX_PENDING;
                if (state.pending.size() < limit) {
                    if (work->reserved) {
                        state.pending.insert(state.pending.begin(), work);
                    } else {
                        state.pending.push_back(work);
                    }
                    result = AppfreezeSubmitResult::QUEUED;
                } else {
                    auto victim = state.pending.end();
                    for (auto candidate = state.pending.begin(); candidate != state.pending.end(); ++candidate) {
                        if (!(*candidate)->reserved && (*candidate)->request->half && !request->half) {
                            victim = candidate; // latest among the lowest severity
                        }
                    }
                    if (victim != state.pending.end()) {
                        discarded = *victim;
                        *victim = work;
                        result = AppfreezeSubmitResult::QUEUED;
                    } else {
                        discarded = work;
                        result = AppfreezeSubmitResult::REJECTED;
                    }
                }
            }
        }
    }
    if (discarded) {
        discarded->request->cancelled_ = true;
        Complete(discarded, nullptr);
    }
    if (result == AppfreezeSubmitResult::DISPATCHED && !Dispatch(work)) {
        return AppfreezeSubmitResult::REJECTED;
    }
    return result;
}

bool AppfreezeTaskScheduler::Dispatch(const std::shared_ptr<Work>& work)
{
    auto weak = weak_from_this();
    if (executor_([weak, work] {
        if (auto scheduler = weak.lock()) {
            scheduler->Run(work);
        }
    })) {
        return true;
    }
    Cancel(work->request);
    return false;
}

bool AppfreezeTaskScheduler::TryStart(const std::shared_ptr<Work>& work)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = generations_.find(work->request->metadata.generationId);
    if (it == generations_.end() || !it->second.accepting || it->second.active != work ||
        work->running || work->request->IsCancelled()) {
        return false;
    }
    work->request->metadata.processStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    work->running = true;
    work->deadline = clock_() + CAPTURE_TIMEOUT_MS;
    wake_.notify_all();
    return true;
}

void AppfreezeTaskScheduler::Run(const std::shared_ptr<Work>& work)
{
    if (!TryStart(work)) {
        return;
    }
    Result result;
    // RAII keeps the execution fence through callbacks and exception unwinding.
    auto finish = std::shared_ptr<void>(nullptr, [this, work](void*) { Finish(work); });
#ifdef __cpp_exceptions
    try {
#endif
        if (work->start && !work->request->IsCancelled()) {
            work->start(work->request);
        }
        if (!work->request->IsCancelled()) {
            result = work->capture(work->request);
        }
#ifdef __cpp_exceptions
    } catch (...) {
        result.reset();
    }
#endif
    std::vector<std::shared_ptr<Work>> waiters;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ExpireLocked(clock_());
        auto it = generations_.find(work->request->metadata.generationId);
        if (it != generations_.end() && it->second.active == work) {
            work->closing = true;
            waiters.swap(it->second.waiters);
        }
        if (work->request->IsCancelled()) {
            result.reset();
        }
    }
    Complete(work, result);
    for (const auto& waiter : waiters) {
        Complete(waiter, work->request->IsCancelled() ? nullptr : result);
    }
}

void AppfreezeTaskScheduler::Finish(const std::shared_ptr<Work>& work)
{
    std::shared_ptr<Work> next;
    std::vector<std::shared_ptr<Work>> waiters;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = generations_.find(work->request->metadata.generationId);
        if (it == generations_.end() || it->second.active != work) {
            return; // old closure cannot complete a replacement request
        }
        auto& state = it->second;
        waiters.swap(state.waiters);
        state.active.reset();
        if (!state.accepting) {
            generations_.erase(it);
        } else if (!state.pending.empty()) {
            next = state.pending.front();
            state.pending.erase(state.pending.begin());
            next->reserved = false;
            state.active = next;
        }
        wake_.notify_all();
    }
    Complete(work, nullptr);
    for (const auto& waiter : waiters) {
        Complete(waiter, nullptr);
    }
    if (next) {
        Dispatch(next);
    }
}

void AppfreezeTaskScheduler::Cancel(const Request& request)
{
    if (!request) {
        return;
    }
    std::shared_ptr<Work> discarded;
    bool finish = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        request->cancelled_ = true;
        auto it = generations_.find(request->metadata.generationId);
        if (it == generations_.end()) {
            return;
        }
        auto& state = it->second;
        if (state.active && state.active->request == request) {
            discarded = state.active;
            finish = !discarded->running;
        } else {
            auto pending = std::find_if(state.pending.begin(), state.pending.end(),
                [&request](const auto& work) { return work->request == request; });
            if (pending != state.pending.end()) {
                discarded = *pending;
                state.pending.erase(pending);
            } else {
                auto waiter = std::find_if(state.waiters.begin(), state.waiters.end(),
                    [&request](const auto& work) { return work->request == request; });
                if (waiter != state.waiters.end()) {
                    discarded = *waiter;
                    state.waiters.erase(waiter);
                }
            }
        }
    }
    Complete(discarded, nullptr);
    if (finish) {
        Finish(discarded);
    }
}

void AppfreezeTaskScheduler::RemoveGeneration(const std::string& generation, int32_t instanceId)
{
    std::vector<std::shared_ptr<Work>> discarded;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = generations_.find(generation);
        if (it == generations_.end() || it->second.instanceId != instanceId) {
            return;
        }
        auto& state = it->second;
        CloseGeneration(state, discarded);
        if (!state.active || !state.active->running) {
            generations_.erase(it);
        }
    }
    for (auto& work : discarded) {
        Complete(work, nullptr);
    }
}

void AppfreezeTaskScheduler::CloseGeneration(Generation& state, std::vector<std::shared_ptr<Work>>& discarded)
{
    state.accepting = false;
    for (const auto& work : state.pending) {
        work->request->cancelled_ = true;
        discarded.push_back(work);
    }
    for (const auto& work : state.waiters) {
        work->request->cancelled_ = true;
        discarded.push_back(work);
    }
    state.pending.clear();
    state.waiters.clear();
    if (state.active) {
        state.active->request->cancelled_ = true;
        discarded.push_back(state.active);
    }
}

void AppfreezeTaskScheduler::CloseAll()
{
    std::vector<std::shared_ptr<Work>> discarded;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = generations_.begin(); it != generations_.end();) {
            CloseGeneration(it->second, discarded);
            if (!it->second.active || !it->second.active->running) {
                it = generations_.erase(it);
            } else {
                ++it;
            }
        }
        recentApps_.clear();
        appExpiries_.clear();
        stormEntries_.clear();
        stormTriggered_ = false;
        wake_.notify_all();
    }
    for (const auto& work : discarded) {
        Complete(work, nullptr);
    }
}

void AppfreezeTaskScheduler::Reset()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopping_ = true;
    }
    CloseAll();
}

void AppfreezeTaskScheduler::ExpireLocked(int64_t now)
{
    while (!appExpiries_.empty() && now - appExpiries_.begin()->first >= WINDOW_MS) {
        recentApps_.erase(appExpiries_.begin()->second);
        appExpiries_.erase(appExpiries_.begin());
    }
    for (auto it = stormEntries_.begin(); it != stormEntries_.end();) {
        if (now - it->second >= WINDOW_MS) {
            it = stormEntries_.erase(it);
        } else {
            ++it;
        }
    }
    if (stormEntries_.size() < STORM_THRESHOLD) {
        stormTriggered_ = false;
    }
    for (auto& entry : generations_) {
        auto& active = entry.second.active;
        if (active && active->running && now >= active->deadline) {
            active->request->cancelled_ = true;
        }
    }
}

void AppfreezeTaskScheduler::Expire()
{
    std::lock_guard<std::mutex> lock(mutex_);
    ExpireLocked(clock_());
}

void AppfreezeTaskScheduler::Maintain()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (!stopping_) {
        const int64_t now = clock_();
        ExpireLocked(now);
        int64_t next = now + WINDOW_MS;
        if (!appExpiries_.empty()) {
            next = std::min(next, appExpiries_.begin()->first + WINDOW_MS);
        }
        for (const auto& entry : generations_) {
            const auto& active = entry.second.active;
            if (active && active->running && !active->request->IsCancelled()) {
                next = std::min(next, active->deadline);
            }
        }
        wake_.wait_for(lock, std::chrono::milliseconds(std::max<int64_t>(1, next - now)));
    }
}

size_t AppfreezeTaskScheduler::PendingSize(const std::string& generation) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = generations_.find(generation);
    return it == generations_.end() ? 0 : it->second.pending.size();
}

size_t AppfreezeTaskScheduler::GenerationSize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return generations_.size();
}

size_t AppfreezeTaskScheduler::RecentAppSize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return recentApps_.size();
}

AppfreezeAttachmentPool::AppfreezeAttachmentPool()
{
    for (auto& worker : workers_) {
        worker = std::thread([this] { Run(); });
    }
}

AppfreezeAttachmentPool::~AppfreezeAttachmentPool()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopping_ = true;
        for (const auto& job : active_) {
            job->closed = true;
            job->done.notify_all();
        }
        wake_.notify_all();
    }
    for (auto& worker : workers_) {
        worker.join();
    }
}

AppfreezeAttachmentPool::Result AppfreezeAttachmentPool::Collect(Collector collector, Cancelled cancelled,
    int64_t budgetMs)
{
    if (!collector || (cancelled && cancelled())) {
        return {};
    }
    auto job = std::make_shared<Job>();
    job->collector = std::move(collector);
    job->cancelled = std::move(cancelled);
    job->deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(budgetMs);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopping_ || active_.size() >= 2) {
            return {};
        }
        active_.push_back(job);
        pending_.push_back(job);
        wake_.notify_one();
    }
    std::unique_lock<std::mutex> lock(job->mutex);
    while (!job->finished && !job->closed && !(job->cancelled && job->cancelled())) {
        if (job->done.wait_until(lock, job->deadline) == std::cv_status::timeout) {
            break;
        }
    }
    job->closed = true; // completion gate: no late write to the returned result
    return job->finished ? std::move(job->result) : Result {};
}

void AppfreezeAttachmentPool::Run()
{
    for (;;) {
        std::shared_ptr<Job> job;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            wake_.wait(lock, [this] { return stopping_ || !pending_.empty(); });
            if (pending_.empty()) {
                return;
            }
            job = pending_.front();
            pending_.erase(pending_.begin());
        }
        const auto cancelled = [job] {
            return job->closed.load() || std::chrono::steady_clock::now() >= job->deadline ||
                (job->cancelled && job->cancelled());
        };
        Result result;
#ifdef __cpp_exceptions
        try {
#endif
            if (!cancelled()) {
                result = job->collector(cancelled);
            }
#ifdef __cpp_exceptions
        } catch (...) {
            job->closed = true;
        }
#endif
        {
            std::lock_guard<std::mutex> lock(job->mutex);
            if (!cancelled()) {
                job->result = std::move(result);
                job->finished = true;
            }
            job->closed = true;
            job->done.notify_all();
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            active_.erase(std::find(active_.begin(), active_.end(), job));
        }
    }
}

size_t AppfreezeAttachmentPool::ActiveSize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return active_.size();
}
} // namespace AppExecFwk
} // namespace OHOS
