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

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr char PROC_PATH[] = "/proc";
// /proc/<pid>/stat lines are ~300 bytes; the bound only guards against unbounded reads on a
// corrupted entry.
constexpr size_t MAX_STAT_FILE_BYTES = 2048;

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
        return false;
    }
    int pid = 0;
    char state = '\0';
    int ppid = 0;
    int pgrp = 0;
    if (sscanf(content.c_str(), "%d", &pid) != 1) {
        return false;
    }
    if (sscanf(content.c_str() + closePos + 1, " %c %d %d", &state, &ppid, &pgrp) != 3) {
        return false;
    }
    if (pid <= 0 || ppid < 0 || pgrp < 0) {
        return false;
    }
    out.pid = static_cast<pid_t>(pid);
    out.state = state;
    out.ppid = static_cast<pid_t>(ppid);
    out.pgrp = static_cast<pid_t>(pgrp);
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
        // Expected when the pid exits between the directory listing and this open.
        openErrno = errno;
        return false;
    }
    std::string content;
    char buf[512];
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
            return ProcLiveness::DEAD;
        }
        // Unreadable /proc entry (e.g. EACCES) or a parse failure on a racing exit: the pid
        // may well be alive — never conclude death from a failed observation.
        return ProcLiveness::UNKNOWN;
    }
    return info.state == 'Z' ? ProcLiveness::DEAD : ProcLiveness::ALIVE;
}

std::vector<pid_t> ProcScanner::FindChildProcesses(pid_t parentPid)
{
    std::vector<pid_t> children;
    if (parentPid <= 0) {
        return children;
    }
    DIR *dir = opendir(PROC_PATH);
    if (dir == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "opendir /proc failed: %{public}d", errno);
        return children;
    }
    struct dirent *entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (!IsAllDigits(entry->d_name)) {
            continue;
        }
        ProcStatInfo info;
        pid_t pid = static_cast<pid_t>(atoi(entry->d_name));
        // The child may exit between readdir and the stat read; a missed entry only delays
        // discovery to the next poll round (or falls back to the anchor).
        int32_t openErrno = 0;
        if (!ReadStatFile(pid, info, openErrno)) {
            continue;
        }
        if (info.ppid == parentPid) {
            children.push_back(pid);
        }
    }
    closedir(dir);
    std::sort(children.begin(), children.end());
    return children;
}

pid_t ProcScanner::FindCommandProcess(pid_t parentPid)
{
    if (parentPid <= 0) {
        return -1;
    }
    std::vector<pid_t> children = FindChildProcesses(parentPid);
    pid_t leader = -1;
    pid_t firstLive = -1;
    int32_t liveCount = 0;
    for (pid_t pid : children) {
        ProcStatInfo info;
        int32_t openErrno = 0;
        if (!ReadStatFile(pid, info, openErrno) || info.state == 'Z') {
            continue; // a zombie command is dead; keep polling for the real one
        }
        ++liveCount;
        if (firstLive < 0) {
            firstLive = pid;
        }
        if (info.pgrp == pid && leader < 0) {
            leader = pid;
        }
    }
    if (leader > 0) {
        return leader;
    }
    if (liveCount == 1) {
        // Single live child even before setpgid(0, 0): it is the command (claw_sandbox forks
        // exactly one command child); accepting it early widens the attribution window.
        return firstLive;
    }
    if (liveCount > 1) {
        // claw_sandbox forks exactly one command child, so this is an unexpected topology;
        // the lowest pid is a deterministic choice, logged for diagnosis.
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "multiple live children, parentPid=%{public}d, count=%{public}d, picking lowest",
            parentPid, liveCount);
        return firstLive;
    }
    return -1;
}
} // namespace CliTool
} // namespace OHOS
