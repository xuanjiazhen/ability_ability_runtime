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

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

namespace {
// Recommended exit codes for signal-caused exits: 128 + signal number (130 = SIGINT, 143 = SIGTERM),
// the conventional shell encoding, so the caller can distinguish cancel from other failures.
constexpr int EXIT_CANCELLED_BASE = 128;
constexpr int EXIT_BAD_ARGS = 2;
constexpr long DEFAULT_DURATION_SECONDS = 5;
constexpr long MAX_DURATION_SECONDS = 3600;

// volatile sig_atomic_t is the only signal-safe state shared between the handler and the main loop.
volatile sig_atomic_t g_cancelSignal = 0;

// Async-signal-safe handler: only write() and volatile sig_atomic_t assignment.
// No stdio (printf/fprintf) and no HiLog inside a signal handler.
void OnCancelSignal(int sig)
{
    int savedErrno = errno;
    g_cancelSignal = sig;
    static const char msg[] = "\n[cancel] received signal, finishing up\n";
    // write is async-signal-safe; short writes are acceptable for this best-effort marker.
    ssize_t unused = write(STDERR_FILENO, msg, sizeof(msg) - 1);
    (void)unused;
    errno = savedErrno;
}

// Install SIGINT/SIGTERM handlers with sigaction so a cooperative cancelSession
// can interrupt a running "run" subcommand.
bool InstallCancelHandlers()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = OnCancelSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART; // restart interrupted syscalls; the flag is checked below
    if (sigaction(SIGINT, &action, nullptr) != 0) {
        return false;
    }
    return sigaction(SIGTERM, &action, nullptr) == 0;
}
} // namespace

// Sample usage:
//   ohos-timer run [--duration N]   # interruptible timer
//   ohos-timer version              # prints the version, not declared cancellable
int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s run [--duration N] | version\n", argv[0]);
        return EXIT_BAD_ARGS;
    }

    if (strcmp(argv[1], "version") == 0) {
        printf("ohos-timer 1.0.0\n");
        return 0;
    }

    if (strcmp(argv[1], "run") != 0) {
        fprintf(stderr, "unknown subcommand: %s\n", argv[1]);
        return EXIT_BAD_ARGS;
    }

    long duration = DEFAULT_DURATION_SECONDS;
    static const struct option options[] = {
        {"duration", required_argument, nullptr, 'd'},
        {nullptr, 0, nullptr, 0},
    };
    int opt = 0;
    while ((opt = getopt_long(argc, argv, "d:", options, nullptr)) != -1) {
        if (opt == 'd') {
            char *end = nullptr;
            duration = strtol(optarg, &end, 10);
            if (end == nullptr || *end != '\0' || duration < 1 || duration > MAX_DURATION_SECONDS) {
                fprintf(stderr, "invalid duration: %s\n", optarg);
                return EXIT_BAD_ARGS;
            }
        } else {
            fprintf(stderr, "usage: %s run [--duration N]\n", argv[0]);
            return EXIT_BAD_ARGS;
        }
    }

    if (!InstallCancelHandlers()) {
        fprintf(stderr, "failed to install cancel signal handlers\n");
        return EXIT_CANCELLED_BASE;
    }

    long elapsed = 0;
    while (elapsed < duration) {
        if (g_cancelSignal != 0) {
            break;
        }
        sleep(1);
        ++elapsed;
        printf("tick %ld/%ld\n", elapsed, duration);
        fflush(stdout);
    }

    if (g_cancelSignal != 0) {
        // Cleanup path: flush the partial result so the caller still receives what was done.
        printf("{\"elapsed\": %ld, \"cancelled\": true}\n", elapsed);
        fflush(stdout);
        return EXIT_CANCELLED_BASE + g_cancelSignal;
    }

    printf("{\"elapsed\": %ld, \"cancelled\": false}\n", elapsed);
    fflush(stdout);
    return 0;
}
