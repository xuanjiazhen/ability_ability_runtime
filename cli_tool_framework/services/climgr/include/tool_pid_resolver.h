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

#ifndef OHOS_ABILITY_RUNTIME_TOOL_PID_RESOLVER_H
#define OHOS_ABILITY_RUNTIME_TOOL_PID_RESOLVER_H

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "session_record.h"

namespace OHOS {
namespace CliTool {

// Short-term bridge that discovers the real CLI tool child process forked inside the
// sandbox host and switches the task-attribution anchor from the (near-idle) host to the
// tool. The tool is born outside climgr's view and no kernel fork event is observable on
// the current device (proc connector: EPROTONOSUPPORT; /proc children file:
// CONFIG_PROC_CHILDREN unset), so discovery is a bounded, self-rescheduling poll of
// /proc matching ppid == host pid (tgid-level: children forked by any host thread match).
// The chain is serial — the next poll is scheduled only after the previous scan returns —
// so a slow scan widens the effective interval instead of piling tasks up, and the whole
// chain is bounded by RESOLVE_BUDGET_MS. Scheduled continuations capture only a weak
// reference to the record: the session table stays its sole owner, so teardown destroys
// the record at once and the chain self-terminates on the next tick. Retires wholesale
// when the pgid group attribution lands.
class ToolPidResolver {
public:
    static ToolPidResolver &GetInstance();

    ToolPidResolver(const ToolPidResolver &) = delete;
    ToolPidResolver &operator=(const ToolPidResolver &) = delete;
    ToolPidResolver(ToolPidResolver &&) = delete;
    ToolPidResolver &operator=(ToolPidResolver &&) = delete;

    // Starts the discovery chain after the sandbox host fork succeeded. No-op for
    // sessions without a taskId: unattributed sessions pay zero cost.
    void Resolve(const std::shared_ptr<SessionRecord> &record);

    // Test seam only: replaces the /proc scan. scan(anchor, comm) must return a live
    // child pid of anchor (filling comm for audit logging), or -1 when none exists.
    void SetScanForTest(std::function<pid_t(pid_t, std::string &)> scan)
    {
        scan_ = std::move(scan);
    }

    // Test seam only: replaces the delayed scheduling of the next poll. scheduler(delayMs,
    // next) must eventually run next(); tests inject recorders/drivers instead of ffrt.
    void SetSchedulerForTest(std::function<void(int64_t, std::function<void()>)> scheduler)
    {
        scheduler_ = std::move(scheduler);
    }

    // Test seam only: restores the production scan and scheduler.
    void ResetForTest();

    // Test driver: runs one poll iteration synchronously (production reaches the same
    // logic via the delayed scheduler). burst is the current poll index (0-based).
    void PollForTest(const std::shared_ptr<SessionRecord> &record, int32_t burst,
        const std::chrono::steady_clock::time_point &deadline)
    {
        Poll(record, burst, deadline);
    }

    // Pacing: the first BURST_POLL_COUNT polls run at BURST_INTERVAL_MS (covering the
    // sandbox setup + spawn window), later polls at BACKOFF_INTERVAL_MS until the budget.
    // DEBUG pacing (on-device triage, 2026-11-19, high-frequency profile): widened until
    // the "tool pid never observed" miss is located — revert to (10, 10, 200, 2000) once
    // the resolved.log poll-index data settles the real spawn latency. Note the chain is
    // serial: a slow sweep widens the effective interval to max(interval, sweep time).
    static constexpr int32_t BURST_POLL_COUNT = 30;
    static constexpr int64_t BURST_INTERVAL_MS = 5;
    static constexpr int64_t BACKOFF_INTERVAL_MS = 50;
    static constexpr int64_t RESOLVE_BUDGET_MS = 5000;

private:
    ToolPidResolver() = default;
    ~ToolPidResolver() = default;

    void Poll(const std::shared_ptr<SessionRecord> &record, int32_t burst,
        const std::chrono::steady_clock::time_point &deadline);
    // One /proc sweep for a live child of anchor (tgid-level ppid match, cross-thread
    // forks included). Fills comm for audit logging only. Returns -1 when not found.
    pid_t ScanForChild(pid_t anchor, std::string &comm);
    void Attach(const std::shared_ptr<SessionRecord> &record, pid_t toolPid);
    // Early-stop probe: true while the record is still the live table entry, so the
    // chain stops within one interval of a session teardown.
    bool IsRecordInTable(const std::shared_ptr<SessionRecord> &record);
    void ScheduleNext(const std::shared_ptr<SessionRecord> &record, int32_t burst,
        const std::chrono::steady_clock::time_point &deadline, int64_t delayMs);

    // Empty means production: real /proc scan and ffrt delayed submit.
    std::function<pid_t(pid_t, std::string &)> scan_;
    std::function<void(int64_t, std::function<void()>)> scheduler_;
    // Bumped by ResetForTest. Every scheduled poll captures the current value and
    // aborts on mismatch, so an outstanding chain can never outlive the test
    // boundary that reset it (a delayed ffrt poll firing after suite teardown
    // would run against reset/destroying state — UB).
    std::atomic<uint64_t> epoch_ { 0 };
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_TOOL_PID_RESOLVER_H
