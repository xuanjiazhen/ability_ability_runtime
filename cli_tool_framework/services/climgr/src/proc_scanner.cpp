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

#include "proc_scanner.h"

#include <cerrno>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "hilog_tag_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr char PROC_PATH[] = "/proc";
// /proc/<pid>/stat lines are ~300 bytes; the bound only guards against unbounded reads on a
// corrupted entry.
constexpr size_t MAX_STAT_FILE_BYTES = 2048;
// Expected sscanf_s conversion counts: 1 for the pid-only leading parse, 2 for the state+ppid
// parse after the comm paren — compared literally before, flagged as magic numbers.
constexpr int NUMBER_OF_PID_FIELDS = 1;
constexpr int NUMBER_OF_STATE_AND_PPID_FIELDS = 2;
// Single read() chunk for /proc/<pid>/stat; the total read bound stays MAX_STAT_FILE_BYTES.
constexpr size_t STAT_READ_CHUNK_BYTES = 512;
// proc(5) process state codes: 'Z' means the process exited but has not been reaped yet.
constexpr char PROC_STATE_ZOMBIE = 'Z';

bool IsAllDigits(const char *name)
{
    if (name == nullptr || *name == '\0') {
        return false;
    }
    for (const char *p = name; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return false;
        }
    }
    return true;
}
} // namespace

bool ProcScanner::ParseProcStat(const std::string &content, ProcStatInfo &out)
{
    // Layout: "pid (comm) state ppid pgrp session ...". comm may contain spaces and ')', so
    // field parsing must start after the LAST ')' — every later field is numeric, so the last
    // ')' in the line is always comm's closing paren.
    size_t closePos = content.rfind(')');
    if (closePos == std::string::npos || closePos + 1 >= content.size()) {
        TAG_LOGD(AAFwkTag::CLI_TOOL, "malformed /proc stat line (no comm paren), length=%{public}zu",
            content.size());
        return false;
    }
    int pid = 0;
    char state = '\0';
    int ppid = 0;
    if (sscanf_s(content.c_str(), "%d", &pid) != NUMBER_OF_PID_FIELDS) {
        TAG_LOGD(AAFwkTag::CLI_TOOL, "malformed /proc stat line (bad pid), length=%{public}zu", content.size());
        return false;
    }
    if (sscanf_s(content.c_str() + closePos + 1, " %c %d", &state, sizeof(state), &ppid) !=
        NUMBER_OF_STATE_AND_PPID_FIELDS) {
        TAG_LOGD(AAFwkTag::CLI_TOOL, "malformed /proc stat line (bad fields), pid=%{public}d", pid);
        return false;
    }
    if (pid <= 0 || ppid < 0) {
        TAG_LOGD(AAFwkTag::CLI_TOOL, "malformed /proc stat line (out-of-range), pid=%{public}d, ppid=%{public}d",
            pid, ppid);
        return false;
    }
    out.pid = static_cast<pid_t>(pid);
    out.state = state;
    out.ppid = static_cast<pid_t>(ppid);
    return true;
}

bool ProcScanner::ReadStatFile(pid_t pid, ProcStatInfo &out, int32_t &openErrno)
{
    openErrno = 0;
    if (pid <= 0) {
        return false;
    }
    std::string path = std::string(PROC_PATH) + "/" + std::to_string(pid) + "/stat";
    int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        // Deliberately silent: the /proc scan calls this for EVERY pid, and per-pid logs would
        // flood. Callers that need diagnostics (CheckLiveness) receive the errno via openErrno
        // and log it themselves.
        openErrno = errno;
        return false;
    }
    std::string content;
    char buf[STAT_READ_CHUNK_BYTES];
    while (content.size() < MAX_STAT_FILE_BYTES) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }
        if (n == 0) {
            break;
        }
        content.append(buf, static_cast<size_t>(n));
    }
    close(fd);
    return ParseProcStat(content, out);
}

bool ProcScanner::IsProcessAlive(pid_t pid)
{
    return CheckLiveness(pid) == ProcLiveness::ALIVE;
}

ProcLiveness ProcScanner::CheckLiveness(pid_t pid)
{
    if (pid <= 0) {
        return ProcLiveness::DEAD;
    }
    ProcStatInfo info;
    int32_t openErrno = 0;
    if (!ReadStatFile(pid, info, openErrno)) {
        if (openErrno == ENOENT || openErrno == ENOTDIR) {
            // "Gone" — but on a hidepid=2 system without the readproc gid a LIVE process is
            // reported exactly the same way: keep this at DEBUG so the field can contrast this
            // line against a shell-domain `ps` when the anchor looks alive from the outside.
            TAG_LOGD(AAFwkTag::CLI_TOOL,
                "no /proc entry for pid=%{public}d, errno=%{public}d (exited, or hidden by hidepid)",
                pid, openErrno);
            return ProcLiveness::DEAD;
        }
        // Unreadable /proc entry (e.g. EACCES): a config anomaly, not a death — surface it.
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "/proc/%{public}d/stat unreadable, errno=%{public}d "
            "(check aimgr readproc gid and SELinux proc_file rules), treat as UNKNOWN",
            pid, openErrno);
        return ProcLiveness::UNKNOWN;
    }
    if (info.state == PROC_STATE_ZOMBIE) {
        TAG_LOGD(AAFwkTag::CLI_TOOL, "pid=%{public}d is a zombie (not reaped yet), state=%{public}c",
            pid, info.state);
        return ProcLiveness::DEAD;
    }
    return ProcLiveness::ALIVE;
}

pid_t ProcScanner::FindCommandProcess(pid_t parentPid, int32_t *readStatCount)
{
    // Contract: the out-parameter (when provided) is ALWAYS written — 0 stands for the early
    // returns below, so a caller can tell "scanned and found nothing" from "never scanned".
    if (readStatCount != nullptr) {
        *readStatCount = 0;
    }
    if (parentPid <= 0) {
        return -1;
    }
    // claw_sandbox forks exactly one command child, so the first live child of parentPid IS
    // the command process: single pass over /proc, return on the first hit. A zombie child
    // is a racing exit, not the command — keep polling for the real one.
    DIR *dir = opendir(PROC_PATH);
    if (dir == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "opendir /proc failed: %{public}d", errno);
        return -1;
    }
    pid_t commandPid = -1;
    int32_t checkedPids = 0;
    int32_t readableStats = 0;
    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (!IsAllDigits(entry->d_name)) {
            continue;
        }
        ++checkedPids;
        ProcStatInfo info;
        // atoi is fine here: not in the scanf family, and entry->d_name is pre-validated by
        // IsAllDigits plus the readdir directory-name contract.
        pid_t pid = static_cast<pid_t>(atoi(entry->d_name));
        // The child may exit between readdir and the stat read; a missed entry only delays
        // discovery to the next poll round (or falls back to the anchor).
        int32_t openErrno = 0;
        if (!ReadStatFile(pid, info, openErrno)) {
            continue;
        }
        // Count every stat that was opened AND parsed (ppid-agnostic): readableStats == 0
        // on a valid parent means the whole scan was blinded (every open denied), which
        // FindCommandProcess alone can never report.
        ++readableStats;
        if (info.ppid != parentPid) {
            continue;
        }
        if (info.state == PROC_STATE_ZOMBIE) {
            TAG_LOGD(AAFwkTag::CLI_TOOL,
                "zombie child pid=%{public}d under anchor pid=%{public}d, keep polling", pid, parentPid);
            continue;
        }
        TAG_LOGD(AAFwkTag::CLI_TOOL, "command child found, anchorPid=%{public}d, childPid=%{public}d, "
            "state=%{public}c, checkedPids=%{public}d", parentPid, pid, info.state, checkedPids);
        commandPid = pid;
        break;
    }
    closedir(dir);
    if (readStatCount != nullptr) {
        *readStatCount = readableStats;
    }
    if (commandPid < 0) {
        // Per-poll DEBUG heartbeat: "no live child" alone never tells the field whether the
        // scan even ran — checkedPids==0 would mean /proc visibility is broken (e.g. hidepid);
        // readableStats==0 means every stat open was denied (SELinux/readproc regression).
        TAG_LOGD(AAFwkTag::CLI_TOOL,
            "no live child of anchor pid=%{public}d this round, checkedPids=%{public}d, "
            "readableStats=%{public}d", parentPid, checkedPids, readableStats);
    }
    return commandPid;
}
} // namespace CliTool
} // namespace OHOS
