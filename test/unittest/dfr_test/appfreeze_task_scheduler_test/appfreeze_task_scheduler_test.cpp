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

#include <gtest/gtest.h>
#include <deque>
#include <future>
#include <algorithm>

using namespace OHOS::AppExecFwk;
using Scheduler = AppfreezeTaskScheduler;
using SubmitResult = AppfreezeSubmitResult;
namespace {
struct Harness {
    std::atomic<int64_t> now {0};
    std::deque<Scheduler::Task> tasks;
    int captures = 0;
    int starts = 0;
    int completions = 0;
    int discards = 0;
    std::vector<std::string> stacks;
    std::vector<int64_t> snapshots;
    bool reject = false;
    std::shared_ptr<Scheduler> scheduler = std::make_shared<Scheduler>([this](auto task) {
        if (reject) {
            return false;
        }
        tasks.push_back(std::move(task));
        return true;
    }, [this] { return now.load(); });

    Scheduler::Request Make(const std::string& event = "THREAD_BLOCK_3S", int pid = 10, int uid = 20,
        int instance = 1)
    {
        auto info = scheduler->Observe(pid, uid);
        const bool half = Scheduler::IsHalfEvent(event);
        info.freezeSeverity = half ? "WARNING" : "FATAL";
        return std::make_shared<AppfreezeRequest>(instance, half, std::move(info));
    }
    SubmitResult Submit(const Scheduler::Request& request, Scheduler::Capture capture = {})
    {
        if (!capture) {
            capture = [this](const auto&) {
                auto result = std::make_shared<AppfreezeCaptureResult>();
                result->stack = "captured-stack-" + std::to_string(++captures);
                return result;
            };
        }
        return scheduler->Submit(request, std::move(capture), [this](const auto& req, const auto& result) {
            if (!result) {
                ++discards;
                return;
            }
            ++completions;
            stacks.push_back(result->stack);
            snapshots.push_back(req->metadata.freezeAppCount1min);
        }, [this](const auto&) { ++starts; });
    }
    void Run()
    {
        ASSERT_FALSE(tasks.empty());
        auto task = std::move(tasks.front());
        tasks.pop_front();
        task();
    }
};

TEST(AppfreezeTaskSchedulerTest, HalfReuseFourCombinationsShareActualResultAndOwnSnapshots)
{
    for (const auto& first : {"THREAD_BLOCK_3S", "LIFECYCLE_HALF_TIMEOUT"}) {
        for (const auto& second : {"THREAD_BLOCK_3S", "LIFECYCLE_HALF_TIMEOUT"}) {
            Harness h;
            auto request = h.Make(first);
            h.Submit(request, [&](const auto&) {
                ++h.captures;
                h.scheduler->Observe(50, 60);
                auto waiter = h.Make(second);
                EXPECT_EQ(h.Submit(waiter), SubmitResult::COALESCED);
                EXPECT_EQ(h.Submit(waiter), SubmitResult::DUPLICATE);
                EXPECT_EQ(h.scheduler->PendingSize("10:20"), 0u);
                auto result = std::make_shared<AppfreezeCaptureResult>();
                result->stack = "real-result-owned-by-first-request";
                return result;
            });
            h.Run();
            EXPECT_EQ(h.captures, 1);
            EXPECT_EQ(h.starts, 1);
            EXPECT_EQ(h.completions, 2);
            EXPECT_EQ(h.stacks[0], h.stacks[1]);
            EXPECT_EQ(h.snapshots, (std::vector<int64_t>{1, 2}));
            EXPECT_TRUE(h.tasks.empty());
        }
    }
}

TEST(AppfreezeTaskSchedulerTest, HalfAfterCompletionFourCombinationsAlwaysCaptureAgain)
{
    for (const auto& first : {"THREAD_BLOCK_3S", "LIFECYCLE_HALF_TIMEOUT"}) {
        for (const auto& second : {"THREAD_BLOCK_3S", "LIFECYCLE_HALF_TIMEOUT"}) {
            Harness h;
            auto old = h.Make(first);
            h.Submit(old);
            h.Run();
            EXPECT_EQ(h.scheduler->GenerationSize(), 1u); // live IDLE entry
            auto fresh = h.Make(second);
            EXPECT_NE(old->requestId, fresh->requestId);
            EXPECT_NE(old->metadata.episodeId, fresh->metadata.episodeId);
            EXPECT_EQ(h.Submit(fresh), SubmitResult::DISPATCHED);
            h.Run();
            EXPECT_EQ(h.captures, 2);
            EXPECT_NE(h.stacks[0], h.stacks[1]);
        }
    }
}

TEST(AppfreezeTaskSchedulerTest, AllUpgradeTypesQueueBehindBothHalfTypes)
{
    for (const auto& half : {"THREAD_BLOCK_3S", "LIFECYCLE_HALF_TIMEOUT"}) {
        for (const auto& fatal : {"THREAD_BLOCK_6S", "APP_INPUT_BLOCK", "LIFECYCLE_TIMEOUT"}) {
            Harness h;
            auto request = h.Make(half);
            h.Submit(request, [&](const auto&) {
                ++h.captures;
                auto upgrade = h.Make(fatal);
                EXPECT_EQ(h.Submit(upgrade), SubmitResult::QUEUED);
                EXPECT_EQ(h.Submit(upgrade), SubmitResult::DUPLICATE);
                EXPECT_EQ(upgrade->metadata.episodeId, request->metadata.episodeId);
                EXPECT_EQ(h.starts, 1);
                EXPECT_TRUE(h.tasks.empty());
                return std::make_shared<AppfreezeCaptureResult>();
            });
            h.Run();
            EXPECT_EQ(h.scheduler->PendingSize("10:20"), 0u);
            ASSERT_EQ(h.tasks.size(), 1u);
            h.Run();
            EXPECT_EQ(h.captures, 2);
            EXPECT_EQ(h.starts, 2);
        }
    }
}

TEST(AppfreezeTaskSchedulerTest, FirstUpgradeReservedAndOtherAdmittedGroupReplacesHalf)
{
    Harness h;
    auto half = h.Make();
    h.Submit(half); // still DISPATCHED, so another half occupies the ordinary slot
    auto pendingHalf = h.Make("LIFECYCLE_HALF_TIMEOUT");
    EXPECT_EQ(h.Submit(pendingHalf), SubmitResult::QUEUED);
    auto first = h.Make("THREAD_BLOCK_6S");
    auto second = h.Make("APP_INPUT_BLOCK"); // represents the independently admitted input group
    EXPECT_EQ(h.Submit(first), SubmitResult::QUEUED);
    EXPECT_EQ(h.Submit(second), SubmitResult::QUEUED);
    EXPECT_EQ(h.scheduler->PendingSize("10:20"), 2u);
    EXPECT_EQ(h.discards, 1);
    EXPECT_TRUE(pendingHalf->IsCancelled());
    EXPECT_FALSE(first->IsCancelled());
    EXPECT_EQ(h.Submit(first), SubmitResult::DUPLICATE);
    h.Run(); h.Run(); h.Run();
    EXPECT_EQ(h.captures, 3);
    EXPECT_EQ(h.completions, 3);
}

TEST(AppfreezeTaskSchedulerTest, ExecutorWaitingAndRejectedTasksNeverStartDisposition)
{
    Harness h;
    auto request = h.Make("LIFECYCLE_TIMEOUT");
    EXPECT_EQ(h.Submit(request), SubmitResult::DISPATCHED);
    EXPECT_EQ(h.starts, 0);
    auto stale = h.tasks.front();
    h.scheduler->Cancel(request);
    stale();
    EXPECT_EQ(h.starts, 0);
    EXPECT_EQ(h.captures, 0);
    h.reject = true;
    EXPECT_EQ(h.Submit(h.Make("THREAD_BLOCK_6S")), SubmitResult::REJECTED);
    EXPECT_EQ(h.starts, 0);
    EXPECT_EQ(h.scheduler->PendingSize("10:20"), 0u);
}

TEST(AppfreezeTaskSchedulerTest, DuplicateExecutorClosureCannotClaimTwice)
{
    Harness h;
    auto request = h.Make();
    h.Submit(request);
    auto replay = h.tasks.front();
    h.Run();
    replay();
    EXPECT_EQ(h.captures, 1);
    EXPECT_EQ(h.starts, 1);
    EXPECT_EQ(h.completions, 1);
}

TEST(AppfreezeTaskSchedulerTest, DeadlineRetainsExecutionFenceUntilCaptureReturns)
{
    Harness h;
    auto first = h.Make();
    h.Submit(first, [&](const auto& req) {
        ++h.captures;
        h.now = 30000;
        h.scheduler->Expire();
        EXPECT_TRUE(req->IsCancelled());
        EXPECT_EQ(h.Submit(h.Make("LIFECYCLE_HALF_TIMEOUT")), SubmitResult::QUEUED);
        EXPECT_EQ(h.Submit(h.Make("LIFECYCLE_TIMEOUT")), SubmitResult::QUEUED);
        EXPECT_TRUE(h.tasks.empty());
        EXPECT_EQ(h.starts, 1);
        return std::make_shared<AppfreezeCaptureResult>();
    });
    h.Run();
    EXPECT_EQ(h.completions, 0);
    h.Run(); h.Run();
    EXPECT_EQ(h.captures, 3);
    EXPECT_EQ(h.completions, 2);
}

TEST(AppfreezeTaskSchedulerTest, ProcessExitRetainsRunningFenceAndCancelsPending)
{
    Harness h;
    auto first = h.Make();
    h.Submit(first, [&](const auto&) {
        ++h.captures;
        EXPECT_EQ(h.Submit(h.Make("APP_INPUT_BLOCK")), SubmitResult::QUEUED);
        h.scheduler->RemoveGeneration("10:20", 1);
        EXPECT_EQ(h.scheduler->GenerationSize(), 1u);
        EXPECT_EQ(h.scheduler->PendingSize("10:20"), 0u);
        EXPECT_EQ(h.Submit(h.Make("THREAD_BLOCK_3S", 10, 20, 2)), SubmitResult::REJECTED);
        return std::make_shared<AppfreezeCaptureResult>();
    });
    h.Run();
    EXPECT_EQ(h.scheduler->GenerationSize(), 0u);
    EXPECT_EQ(h.completions, 0);
    EXPECT_EQ(h.Submit(h.Make("THREAD_BLOCK_3S", 10, 20, 2)), SubmitResult::DISPATCHED);
    h.Run();
    EXPECT_EQ(h.completions, 1);
}

TEST(AppfreezeTaskSchedulerTest, OldClosureAndOldRemovalCannotAffectRecreatedInstance)
{
    Harness h;
    auto old = h.Make();
    h.Submit(old);
    auto stale = h.tasks.front();
    h.tasks.pop_front();
    h.scheduler->RemoveGeneration("10:20", 1);
    auto fresh = h.Make("THREAD_BLOCK_3S", 10, 20, 2);
    h.Submit(fresh);
    h.scheduler->RemoveGeneration("10:20", 1);
    stale();
    EXPECT_EQ(h.captures, 0);
    EXPECT_EQ(h.starts, 0);
    h.Run();
    EXPECT_EQ(h.captures, 1);
    EXPECT_EQ(h.completions, 1);
}

TEST(AppfreezeTaskSchedulerTest, ReuseWaitersAreBoundedWithoutCreatingExtraCaptures)
{
    Harness h;
    h.Submit(h.Make(), [&](const auto&) {
        ++h.captures;
        for (int i = 0; i < 16; ++i) {
            EXPECT_EQ(h.Submit(h.Make("LIFECYCLE_HALF_TIMEOUT")), SubmitResult::COALESCED);
        }
        EXPECT_EQ(h.Submit(h.Make("LIFECYCLE_HALF_TIMEOUT")), SubmitResult::REJECTED);
        EXPECT_EQ(h.scheduler->PendingSize("10:20"), 0u);
        return std::make_shared<AppfreezeCaptureResult>();
    });
    h.Run();
    EXPECT_EQ(h.captures, 1);
    EXPECT_EQ(h.completions, 17);
}

TEST(AppfreezeTaskSchedulerTest, DistinctKeysExecuteInParallelAndSameKeyWaitsOutsideExecutor)
{
    std::mutex mutex;
    std::condition_variable wake;
    std::deque<Scheduler::Task> queued;
    bool stopping = false;
    auto scheduler = std::make_shared<Scheduler>([&](auto task) {
        std::lock_guard<std::mutex> lock(mutex);
        queued.push_back(std::move(task));
        wake.notify_all();
        return true;
    });
    auto worker = [&] {
        for (;;) {
            Scheduler::Task task;
            {
                std::unique_lock<std::mutex> lock(mutex);
                wake.wait(lock, [&] { return stopping || !queued.empty(); });
                if (queued.empty()) { return; }
                task = std::move(queued.front());
                queued.pop_front();
            }
            task();
        }
    };
    std::thread workers[2] = {std::thread(worker), std::thread(worker)};
    std::promise<void> release;
    auto gate = release.get_future().share();
    std::atomic<int> running {0};
    std::atomic<int> completed {0};
    auto capture = [&](const auto&) {
        ++running;
        wake.notify_all();
        gate.wait();
        --running;
        return std::make_shared<AppfreezeCaptureResult>();
    };
    auto first = std::make_shared<AppfreezeRequest>(1, true, scheduler->Observe(10, 20));
    auto other = std::make_shared<AppfreezeRequest>(2, true, scheduler->Observe(11, 21));
    scheduler->Submit(first, capture, [&](const auto&, const auto&) { ++completed; });
    scheduler->Submit(other, capture, [&](const auto&, const auto&) { ++completed; });
    {
        std::unique_lock<std::mutex> lock(mutex);
        EXPECT_TRUE(wake.wait_for(lock, std::chrono::seconds(2), [&] { return running == 2; }));
    }
    auto upgrade = std::make_shared<AppfreezeRequest>(1, false, scheduler->Observe(10, 20));
    EXPECT_EQ(scheduler->Submit(upgrade, capture, [&](const auto&, const auto&) { ++completed; }),
        SubmitResult::QUEUED);
    {
        std::lock_guard<std::mutex> lock(mutex);
        EXPECT_TRUE(queued.empty());
    }
    release.set_value();
    {
        std::lock_guard<std::mutex> lock(mutex);
        stopping = true;
        wake.notify_all();
    }
    for (auto& thread : workers) { thread.join(); }
    EXPECT_EQ(completed, 3);
}

TEST(AppfreezeTaskSchedulerTest, RecentUidWindowUsesOpenLowerBoundary)
{
    for (int64_t boundary : {59999, 60000, 60001}) {
        Harness h;
        EXPECT_EQ(h.scheduler->Observe(10, 20).freezeAppCount1min, 1);
        h.now = boundary;
        h.scheduler->Expire();
        EXPECT_EQ(h.scheduler->RecentAppSize(), boundary < 60000 ? 1u : 0u);
    }
}

TEST(AppfreezeTaskSchedulerTest, RecentUidRefreshUsesOneNodeAndExitKeepsObservation)
{
    Harness h;
    auto first = h.Make();
    h.Submit(first); h.Run();
    h.scheduler->RemoveGeneration("10:20", 1);
    h.now = 59000;
    EXPECT_EQ(h.scheduler->Observe(11, 20).freezeAppCount1min, 1); // same app, another process
    h.now = 60000;
    h.scheduler->Expire();
    EXPECT_EQ(h.scheduler->RecentAppSize(), 1u);
    EXPECT_EQ(h.scheduler->Observe(12, 21).freezeAppCount1min, 2);
    h.now = 119000;
    h.scheduler->Expire();
    EXPECT_EQ(h.scheduler->RecentAppSize(), 1u);
    h.now = 120000;
    h.scheduler->Expire();
    EXPECT_EQ(h.scheduler->RecentAppSize(), 0u);
    EXPECT_EQ(first->metadata.freezeAppCount1min, 1);
}

TEST(AppfreezeTaskSchedulerTest, SchedulerReplayDoesNotObserveUidAgain)
{
    Harness h;
    auto first = h.Make();
    h.Submit(first); h.Run();
    h.now = 59000;
    EXPECT_EQ(h.Submit(first), SubmitResult::DUPLICATE);
    h.now = 61000;
    EXPECT_EQ(h.scheduler->Observe(12, 21).freezeAppCount1min, 1);
    EXPECT_EQ(first->metadata.freezeAppCount1min, 1);
}

TEST(AppfreezeTaskSchedulerTest, StormUsesDistinctProcessesAndFirstAppearanceSeparatelyFromUidRefresh)
{
    Harness h;
    EXPECT_FALSE(h.scheduler->Observe(10, 20).stormTrigger);
    EXPECT_FALSE(h.scheduler->Observe(11, 20).stormTrigger);
    auto third = h.scheduler->Observe(12, 20);
    EXPECT_TRUE(third.stormTrigger);
    EXPECT_FALSE(third.stormGenerationId.empty());
    EXPECT_EQ(third.freezeAppCount1min, 1);
    EXPECT_FALSE(h.scheduler->Observe(13, 21).stormTrigger);
    h.now = 59000;
    EXPECT_FALSE(h.scheduler->Observe(10, 20).stormTrigger);
    h.now = 60000;
    EXPECT_FALSE(h.scheduler->Observe(20, 30).stormTrigger);
    EXPECT_FALSE(h.scheduler->Observe(21, 31).stormTrigger);
    auto next = h.scheduler->Observe(22, 32);
    EXPECT_TRUE(next.stormTrigger);
    EXPECT_NE(next.stormGenerationId, third.stormGenerationId);
    EXPECT_EQ(next.freezeAppCount1min, 4); // refreshed app A plus three new apps
}

TEST(AppfreezeTaskSchedulerTest, RecentCountIsIndependentOfScheduler)
{
    Harness h;
    for (int uid = 0; uid < 200; ++uid) {
        EXPECT_EQ(h.scheduler->Observe(uid, uid).freezeAppCount1min, uid + 1);
    }
    EXPECT_EQ(h.scheduler->RecentAppSize(), 200u);
    h.scheduler->Reset();
    EXPECT_EQ(h.scheduler->RecentAppSize(), 0u);
}

TEST(AppfreezeTaskSchedulerTest, EpisodeReportClaimDeduplicatesWarningsAndKeepsFirstFatal)
{
    Harness h;
    auto first = h.Make();
    std::weak_ptr<AppfreezeReportClaim> lifetime = first->metadata.reportClaim;
    h.Submit(first, [&](const auto& request) {
        auto waiter = h.Make("LIFECYCLE_HALF_TIMEOUT");
        EXPECT_EQ(h.Submit(waiter), SubmitResult::COALESCED);
        auto fatal = h.Make("LIFECYCLE_TIMEOUT");
        EXPECT_EQ(h.Submit(fatal), SubmitResult::QUEUED);
        EXPECT_EQ(waiter->metadata.reportClaim, request->metadata.reportClaim);
        EXPECT_EQ(fatal->metadata.reportClaim, request->metadata.reportClaim);
        EXPECT_TRUE(request->metadata.reportClaim->TryBegin(true));
        request->metadata.reportClaim->End(true, 0);
        EXPECT_FALSE(waiter->metadata.reportClaim->TryBegin(true));
        EXPECT_TRUE(fatal->metadata.reportClaim->TryBegin(false));
        fatal->metadata.reportClaim->End(false, 0);
        EXPECT_FALSE(fatal->metadata.reportClaim->TryBegin(false));
        return std::make_shared<AppfreezeCaptureResult>();
    });
    h.Run(); h.Run();
    first.reset();
    EXPECT_TRUE(lifetime.expired()); // IDLE holds no episode report state
    auto fresh = h.Make();
    EXPECT_TRUE(fresh->metadata.reportClaim->TryBegin(true));
}

TEST(AppfreezeTaskSchedulerTest, FailedReportReleasesOnlyItsSeverityClaim)
{
    AppfreezeReportClaim claim;
    EXPECT_TRUE(claim.TryBegin(true));
    claim.End(true, 0);
    EXPECT_TRUE(claim.TryBegin(false));
    claim.End(false, -1);
    EXPECT_FALSE(claim.TryBegin(true));
    EXPECT_TRUE(claim.TryBegin(false)); // next admitted upgrade may recover the failed send
    claim.End(false, 0);
    EXPECT_FALSE(claim.TryBegin(false));
}

TEST(AppfreezeTaskSchedulerTest, PositiveHiSysEventWarningKeepsTheSentSeverityClaim)
{
    AppfreezeReportClaim claim;
    EXPECT_TRUE(claim.TryBegin(true));
    claim.End(true, 2); // HiSysEvent accepted the event, with a truncated parameter.
    EXPECT_FALSE(claim.TryBegin(true));
    EXPECT_TRUE(claim.TryBegin(false));
    claim.End(false, -2);
    EXPECT_TRUE(claim.TryBegin(false));
    claim.End(false, 2);
    EXPECT_FALSE(claim.TryBegin(false));
    EXPECT_FALSE(claim.TryBegin(true));
}

TEST(AppfreezeTaskSchedulerTest, CloseAllResetsObservationsAndRetainsOnlyExecutingFences)
{
    Harness h;
    auto first = h.Make();
    h.Submit(first, [&](const auto&) {
        EXPECT_EQ(h.Submit(h.Make("LIFECYCLE_TIMEOUT")), SubmitResult::QUEUED);
        auto dispatched = h.Make("THREAD_BLOCK_3S", 11, 21, 2);
        EXPECT_EQ(h.Submit(dispatched), SubmitResult::DISPATCHED);
        h.scheduler->Observe(12, 22);
        h.scheduler->CloseAll();
        EXPECT_TRUE(first->IsCancelled());
        EXPECT_TRUE(dispatched->IsCancelled());
        EXPECT_EQ(h.scheduler->RecentAppSize(), 0u);
        EXPECT_EQ(h.scheduler->GenerationSize(), 1u);
        auto replacement = h.Make("THREAD_BLOCK_3S", 10, 20, 3);
        EXPECT_EQ(replacement->metadata.freezeAppCount1min, 1);
        EXPECT_FALSE(replacement->metadata.stormTrigger);
        EXPECT_EQ(h.Submit(replacement), SubmitResult::REJECTED);
        return std::make_shared<AppfreezeCaptureResult>();
    });
    h.Run();
    h.Run(); // old DISPATCHED closure cannot claim a replacement instance
    EXPECT_EQ(h.completions, 0);
    EXPECT_EQ(h.scheduler->GenerationSize(), 0u);
    auto fresh = h.Make("THREAD_BLOCK_3S", 10, 20, 3);
    EXPECT_EQ(h.Submit(fresh), SubmitResult::DISPATCHED);
    h.Run();
    EXPECT_EQ(h.captures, 1);
    EXPECT_EQ(h.completions, 1);
}

TEST(AppfreezeTaskSchedulerTest, StormObservationDoesNotPerformSlowCapture)
{
    Harness h;
    std::vector<int64_t> elapsed;
    for (int i = 0; i < 1000; ++i) {
        auto start = std::chrono::steady_clock::now();
        h.scheduler->Observe(i, i);
        elapsed.push_back(std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start).count());
    }
    std::sort(elapsed.begin(), elapsed.end());
    EXPECT_LT(elapsed[989], 3000);
    EXPECT_LT(elapsed.back(), 10000);
    EXPECT_EQ(h.captures, 0);
}

TEST(AppfreezeAttachmentPoolTest, TimelyResultPublished)
{
    AppfreezeAttachmentPool pool;
    auto result = pool.Collect([](const auto&) {
        return AppfreezeAttachmentPool::Result {"binder", "memory"};
    }, [] { return false; });
    EXPECT_EQ(result.binderInfo, "binder");
    EXPECT_EQ(result.memoryContent, "memory");
}

TEST(AppfreezeAttachmentPoolTest, TimeoutAndCancellationKeepSlotsUntilExecutionActuallyExits)
{
    AppfreezeAttachmentPool pool;
    std::promise<void> release;
    auto gate = release.get_future().share();
    std::atomic<int> entered {0};
    auto collector = [&](const auto&) {
        ++entered;
        gate.wait(); // controlled existing non-cancellable collector
        return AppfreezeAttachmentPool::Result {"late-binder", "late-memory"};
    };
    auto first = pool.Collect(collector, [] { return false; }, 20);
    auto second = pool.Collect(collector, [] { return false; }, 20);
    EXPECT_EQ(entered, 2);
    EXPECT_EQ(pool.ActiveSize(), 2u);
    EXPECT_TRUE(first.binderInfo.empty());
    EXPECT_TRUE(second.memoryContent.empty());
    EXPECT_TRUE(pool.Collect(collector, [] { return false; }, 20).binderInfo.empty());
    EXPECT_EQ(entered, 2);
    release.set_value();
    EXPECT_TRUE(first.binderInfo.empty()); // caller's finalized result cannot be changed
}

TEST(AppfreezeAttachmentPoolTest, PreCancelledTaskNeverEntersCollector)
{
    AppfreezeAttachmentPool pool;
    int invoked = 0;
    auto result = pool.Collect([&](const auto&) {
        ++invoked;
        return AppfreezeAttachmentPool::Result {};
    }, [] { return true; });
    EXPECT_EQ(invoked, 0);
    EXPECT_EQ(pool.ActiveSize(), 0u);
    EXPECT_TRUE(result.binderInfo.empty());
}
TEST(AppfreezeAttachmentPoolTest, DefaultBudgetClosesAtFiveHundredMilliseconds)
{
    AppfreezeAttachmentPool pool;
    std::promise<void> release;
    auto gate = release.get_future().share();
    const auto start = std::chrono::steady_clock::now();
    auto result = pool.Collect([&](const auto&) {
        gate.wait();
        return AppfreezeAttachmentPool::Result {"late", "late"};
    }, [] { return false; });
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count();
    EXPECT_GE(elapsed, 490);
    EXPECT_LT(elapsed, 1000);
    EXPECT_TRUE(result.binderInfo.empty());
    EXPECT_EQ(pool.ActiveSize(), 1u);
    release.set_value();
}

} // namespace
