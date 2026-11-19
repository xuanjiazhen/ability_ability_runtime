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

#include "tool_pid_resolver.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <charconv>
#include <cstring>
#include <string>

#include "cli_tool_manager_service.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int64_t MS_TO_US = 1000; // ffrt task_attr::delay takes microseconds
constexpr size_t STAT_LINE_MAX = 512; // comm(15) + fixed fields; state/ppid sit right after ')'
constexpr size_t COMM_MAX = 16; // TASK_COMM_LEN including NUL
// Probe window above the host pid (enumeration-free discovery): the tool child is forked
// shortly after the host, so its pid lands within a small monotonic offset (measured
// on-device: +3..+9). 64 leaves ~7x headroom; pid wrap-around would simply miss.
constexpr int32_t PROBE_WINDOW = 64;

// /proc/<pid>/stat: "<pid> (<comm>) <state> <ppid> ...". comm may contain '(' or spaces, so
// the fields only become parseable after the LAST ')'. sscanf/strtol are forbidden by the
// secure-coding rules (unsafe memory functions), so state and ppid are located by direct
// character scanning and parsed with std::from_chars: no format string, no magic numbers,
// and overflow is reported as an error code instead of being undefined behavior.
bool ParseProcStat(const char *line, pid_t expectedPid, char &state, pid_t &ppid)
{
    const char *lineEnd = line + strlen(line);
    int pidValue = 0;
    auto [pidEnd, pidEc] = std::from_chars(line, lineEnd, pidValue);
    if (pidEc != std::errc() || pidValue != static_cast<int>(expectedPid)) {
        return false;
    }
    const char *closeParen = strrchr(line, ')');
    if (closeParen == nullptr || closeParen < pidEnd) {
        return false;
    }
    const char *cursor = closeParen + 1;
    while (*cursor == ' ') {
        ++cursor;
    }
    if (*cursor == '\0') {
        return false; // truncated line: the state field is missing
    }
    state = *cursor;
    ++cursor;
    while (*cursor == ' ') {
        ++cursor;
    }
    int ppidValue = 0;
    auto [ppidEnd, ppidEc] = std::from_chars(cursor, lineEnd, ppidValue);
    if (ppidEc != std::errc() || ppidEnd == cursor) {
        return false; // missing or malformed ppid
    }
    ppid = static_cast<pid_t>(ppidValue);
    return true;
}

// comm is audit-only; a process may set arbitrary bytes via prctl(PR_SET_NAME), so
// non-printable characters are masked to keep the log line well-formed.
void ExtractSanitizedComm(const char *line, std::string &comm)
{
    const char *openParen = strchr(line, '(');
    const char *closeParen = strrchr(line, ')');
    if (openParen == nullptr || closeParen == nullptr || closeParen <= openParen) {
        return;
    }
    size_t length = static_cast<size_t>(closeParen - openParen - 1);
    if (length > COMM_MAX - 1) {
        length = COMM_MAX - 1;
    }
    comm.assign(openParen + 1, length);
    for (char &ch : comm) {
        if (ch < 0x20 || static_cast<unsigned char>(ch) > 0x7e) {
            ch = '?';
        }
    }
}
} // namespace

ToolPidResolver &ToolPidResolver::GetInstance()
{
    static ToolPidResolver instance;
    return instance;
}

void ToolPidResolver::ResetForTest()
{
    // Fence first: chains already scheduled captured the old epoch and abort on the
    // mismatch, so a delayed poll can never run against a reset/test boundary.
    epoch_.fetch_add(1);
    scan_ = nullptr;
    scheduler_ = nullptr;
}

void ToolPidResolver::Resolve(const std::shared_ptr<SessionRecord> &record)
{
    if (record == nullptr || record->taskId.empty()) {
        return; // unattributed sessions pay zero cost
    }
    if (record->processId.load() <= 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "ToolPidResolver skipped: invalid host pid, sessionId=%{public}s, taskId=%{public}s",
            record->sessionId.c_str(), record->taskId.c_str());
        return;
    }
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(RESOLVE_BUDGET_MS);
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "ToolPidResolver chain started, sessionId=%{public}s, hostPid=%{public}d, taskId=%{public}s",
        record->sessionId.c_str(), record->processId.load(), record->taskId.c_str());
    // burst = -1 so the first scheduled poll runs as burst 0 at the burst interval.
    ScheduleNext(record, -1, deadline, BURST_INTERVAL_MS);
}

void ToolPidResolver::Poll(const std::shared_ptr<SessionRecord> &record, int32_t burst,
    const std::chrono::steady_clock::time_point &deadline)
{
    // The reaper invalidates processId at any moment; this atomic read is lock-free by contract.
    pid_t anchor = record->processId.load();
    if (anchor <= 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "ToolPidResolver give up: host reaped before tool observed, no tool attribution, "
            "hostPid=%{public}d, sessionId=%{public}s, taskId=%{public}s",
            anchor, record->sessionId.c_str(), record->taskId.c_str());
        return;
    }
    if (!IsRecordInTable(record)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "ToolPidResolver give up: session ended before tool observed, no tool attribution, "
            "hostPid=%{public}d, sessionId=%{public}s, taskId=%{public}s",
            anchor, record->sessionId.c_str(), record->taskId.c_str());
        return;
    }
    if (std::chrono::steady_clock::now() >= deadline) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "ToolPidResolver give up: tool pid never observed within budget, no tool attribution, "
            "hostPid=%{public}d, sessionId=%{public}s, taskId=%{public}s",
            anchor, record->sessionId.c_str(), record->taskId.c_str());
        return;
    }
    std::string comm;
    pid_t toolPid = ScanForChild(anchor, comm);
    if (toolPid > 0) {
        TAG_LOGI(AAFwkTag::CLI_TOOL,
            "ToolPidResolver tool pid resolved, sessionId=%{public}s, hostPid=%{public}d, "
            "toolPid=%{public}d, comm=%{public}s, poll=%{public}d",
            record->sessionId.c_str(), anchor, toolPid, comm.c_str(), burst);
        Attach(record, toolPid);
        return;
    }
    int64_t delayMs = (burst + 1 < BURST_POLL_COUNT) ? BURST_INTERVAL_MS : BACKOFF_INTERVAL_MS;
    ScheduleNext(record, burst, deadline, delayMs);
}

pid_t ToolPidResolver::ScanForChild(pid_t anchor, std::string &comm)
{
    if (scan_) {
        return scan_(anchor, comm);
    }
    // Enumeration-free probe (on-device triage, 2026-11-19): readdir("/proc") is
    // visibility-clipped on this device — climgr enumerates only its own uid + root
    // entries (measured entries=2), so the tool child is structurally invisible to a
    // full-table sweep. Opening a KNOWN pid path walks a different permission path, and
    // the tool pid always lands in a small monotonic window above the host pid
    // (measured offsets: +3..+9), so probe that window instead of enumerating.
    pid_t toolPid = -1;
    int32_t probes = 0;
    int32_t permFail = 0;
    for (int32_t offset = 1; offset <= PROBE_WINDOW; ++offset) {
        int32_t candidate = anchor + offset;
        ++probes;
        // snprintf is forbidden by the secure-coding rules; string concatenation cannot
        // truncate, which also removes the old fixed-buffer length proof.
        std::string statPath = "/proc/" + std::to_string(candidate) + "/stat";
        int fd = open(statPath.c_str(), O_RDONLY | O_CLOEXEC);
        if (fd < 0) {
            if (errno == EACCES || errno == EPERM) {
                // Decisive triage signal: the permission wall clips by-name access too
                // (hidepid-class semantics) — probing cannot work either.
                ++permFail;
            }
            continue; // ENOENT (no process at this pid) or EMFILE: keep probing
        }
        char line[STAT_LINE_MAX];
        ssize_t nread = read(fd, line, sizeof(line) - 1);
        close(fd);
        if (nread <= 0) {
            continue;
        }
        line[nread] = '\0';
        pid_t pid = static_cast<pid_t>(candidate);
        char state = '\0';
        pid_t ppid = -1;
        if (!ParseProcStat(line, pid, state, ppid) || ppid != anchor) {
            continue;
        }
        if (state == 'Z' || state == 'X') {
            continue; // already dead: never a valid attribution target, keep probing
        }
        ExtractSanitizedComm(line, comm);
        toolPid = pid;
        break; // one tool child per sandbox session: the first live child claims the slot
    }
    // One line per probe sweep (on-device triage): probes shows the window was walked,
    // permFail separates "pid not present" (ENOENT) from "permission wall" (EACCES/EPERM).
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "ScanForChild probe sweep, hostPid=%{public}d, probes=%{public}d, permFail=%{public}d, "
        "result=%{public}d, comm=%{public}s",
        anchor, probes, permFail, toolPid, comm.c_str());
    return toolPid;
}

void ToolPidResolver::Attach(const std::shared_ptr<SessionRecord> &record, pid_t toolPid)
{
    auto service = CliToolManagerService::GetInstance();
    if (service == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "ToolPidResolver attach skipped: service unavailable, sessionId=%{public}s",
            record->sessionId.c_str());
        return;
    }
    service->AttachTaskAttribution(record, toolPid);
}

bool ToolPidResolver::IsRecordInTable(const std::shared_ptr<SessionRecord> &record)
{
    auto service = CliToolManagerService::GetInstance();
    if (service == nullptr) {
        return false;
    }
    std::lock_guard<ffrt::mutex> guard(service->sessionsMutex_);
    auto it = service->sessionRecords_.find(record->sessionId);
    return it != service->sessionRecords_.end() && it->second == record;
}

void ToolPidResolver::ScheduleNext(const std::shared_ptr<SessionRecord> &record, int32_t burst,
    const std::chrono::steady_clock::time_point &deadline, int64_t delayMs)
{
    uint64_t epoch = epoch_.load();
    // Snapshot for the give-up line: after teardown the record is gone, so the lambda
    // cannot read the anchor from it anymore.
    pid_t lastAnchor = record->processId.load();
    // Weak capture keeps the session table the record's sole owner: once teardown has
    // destroyed the record, the chain ends here instead of holding it for the rest of the
    // budget (only the control block survives until this tick). The id copies keep the
    // give-up line as observable as the in-Poll variant after the record is gone.
    auto next = [weakRecord = std::weak_ptr<SessionRecord>(record), sessionId = record->sessionId,
                    taskId = record->taskId, lastAnchor, burst, deadline, this, epoch]() {
        if (epoch_.load() != epoch) {
            return; // fenced by ResetForTest: drop the stale chain silently
        }
        std::shared_ptr<SessionRecord> liveRecord = weakRecord.lock();
        if (liveRecord == nullptr) {
            TAG_LOGW(AAFwkTag::CLI_TOOL,
                "ToolPidResolver give up: session ended before tool observed, no tool attribution, "
                "lastHostPid=%{public}d, sessionId=%{public}s, taskId=%{public}s",
                lastAnchor, sessionId.c_str(), taskId.c_str());
            return;
        }
        Poll(liveRecord, burst + 1, deadline);
    };
    if (scheduler_) {
        scheduler_(delayMs, next);
        return;
    }
    // Polling deviation from the condvar rule is documented here: the tool is not climgr's
    // direct child (SIGCHLD never fires for it) and no kernel fork event is observable on
    // this device, so bounded /proc polling is the only in-repo discovery primitive.
    ffrt::submit(std::move(next), ffrt::task_attr().delay(static_cast<uint64_t>(delayMs) * MS_TO_US));
}
} // namespace CliTool
} // namespace OHOS
