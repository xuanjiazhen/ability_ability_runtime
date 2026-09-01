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

#include "session_protocol.h"

#include <cerrno>
#include <cstdlib>
#include <poll.h>
#include <string>
#include <unistd.h>

#include <nlohmann/json.hpp>

#include "agent_manager_client.h"  // AgentRuntime::AgentManagerClient (DisconnectAgentExtensionAbility)
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

namespace {
// Single JSON-line byte cap. Matches the OHOS IPC single-message convention (dataobs
// MAX_DATA_SIZE = 200*1024); a larger line would already fail the cliMgr SendMessage IPC, but the
// downstream must enforce its own limit too for clean failure instead of unbounded lineBuf growth.
constexpr size_t MAX_LINE_BYTES = 200 * 1024;

// No-throw JSON string field extraction (returns "" if absent or non-string). Robust regardless of
// whether nlohmann was built with JSON_NOEXCEPTION.
std::string GetJsonString(const nlohmann::json &j, const char *key)
{
    if (j.contains(key) && j[key].is_string()) {
        return j[key].get<std::string>();
    }
    return "";
}

// Dispatch one JSON-line command. All paths are non-fatal — a rejected disconnect or a bad line
// does NOT end the session (the connection is still alive; the caller may retry). The only fatal
// exits live in RunCommandLoop itself (poll/read failure).
void DispatchLine(const std::string &line, const sptr<CliAgentConnection> &connection,
    const sptr<AgentRuntime::IAgentReceiver> &receiver, const sptr<IRemoteObject> &hostObj)
{
    // allow_exceptions=false: parse failure returns a discarded json instead of throwing.
    nlohmann::json cmd = nlohmann::json::parse(line, nullptr, false);
    if (cmd.is_discarded() || !cmd.is_object()) {
        EmitAgentEvent(BuildErrorEvent("bad-json", "not a JSON object", false));
        return;  // skip malformed line, keep the loop alive
    }
    std::string op = GetJsonString(cmd, "op");
    if (op == "sendmessage" || op == "authorize") {
        if (receiver == nullptr) {
            EmitAgentEvent(BuildErrorEvent("no-receiver", "agent receiver proxy is null", false));
            return;
        }
        // JS/ETS-aligned (js_agent_receiver_proxy OnSendData): data must be present and a string;
        // an empty string is passed through. Missing/non-string -> error event (not a fatal exit).
        if (!cmd.contains("data") || !cmd["data"].is_string()) {
            EmitAgentEvent(BuildErrorEvent("bad-data", "data field missing or not a string", false));
            return;
        }
        std::string data = cmd["data"].get<std::string>();
        int32_t ret = (op == "sendmessage") ? receiver->SendData(hostObj, data)
                                            : receiver->Authorize(hostObj, data);
        if (ret != 0) {
            const char *opName = (op == "sendmessage") ? "SendData" : "Authorize";
            const char *evt = (op == "sendmessage") ? "send-failed" : "authorize-failed";
            EmitAgentEvent(BuildErrorEvent(evt, std::string(opName) + " ret=" + std::to_string(ret), false));
        }
        return;
    }
    if (op == "disconnect") {
        // TOCTOU guard (TASK-5 防重入): if a peer disconnect (OnAbilityDisconnectDone on a binder
        // thread) already fired and set the Disconnecting state, a late stdin disconnect is a
        // no-op — disconnect-done was already emitted and the loop is exiting on wakeup.
        if (connection->IsDisconnectInitiated()) {
            return;
        }
        std::string reason = GetJsonString(cmd, "reason");
        connection->SetDisconnectReason(reason.empty() ? "user" : reason);
        const char *idEnv = getenv("ohos_cli_callerIdentity");
        std::string callerIdentity = (idEnv != nullptr) ? idEnv : "";
        int32_t ret = AgentRuntime::AgentManagerClient::GetInstance().DisconnectAgentExtensionAbilityForCli(
            connection, callerIdentity);
        if (ret != 0) {
            // Non-fatal (Q1): a rejected disconnect does not end the session — the connection is
            // still alive and the caller may retry. Do NOT mark Disconnecting (flag stays clear so
            // a retry can re-issue the RPC). If a peer disconnect raced in during the RPC it already
            // emitted disconnect-done; suppress a spurious disconnect-failed after it.
            if (!connection->IsDisconnectInitiated()) {
                EmitAgentEvent(BuildErrorEvent("disconnect-failed",
                    "DisconnectAgentExtensionAbilityForCli ret=" + std::to_string(ret), false));
            }
            return;
        }
        // RPC accepted: mark Disconnecting (idempotent CAS — OnAbilityDisconnectDone may have raced
        // ahead during the RPC and set it; either way we now wait for the callback to emit
        // disconnect-done + SignalDisconnect -> the loop exits normally). A second stdin disconnect
        // is suppressed by the IsDisconnectInitiated() check above + the RunCommandLoop skip-dispatch.
        (void)connection->TryInitiateDisconnect();
        return;
    }
    EmitAgentEvent(BuildErrorEvent("unknown-op", "unknown op: " + op, false));
}
}  // namespace

bool RunCommandLoop(const sptr<CliAgentConnection> &connection,
    const sptr<AgentRuntime::IAgentReceiver> &receiver, const sptr<IRemoteObject> &hostObj)
{
    if (connection == nullptr) {
        return false;  // nothing to loop on; not a fatal infra error
    }
    int wakeupFd = connection->GetWakeupFd();
    std::string lineBuf;
    while (true) {
        struct pollfd pfds[2];
        pfds[0].fd = STDIN_FILENO;
        pfds[0].events = POLLIN;
        pfds[0].revents = 0;
        pfds[1].fd = wakeupFd;
        pfds[1].events = POLLIN;
        pfds[1].revents = 0;
        nfds_t nfds = (wakeupFd >= 0) ? 2 : 1;
        int ret = poll(pfds, nfds, -1);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            EmitAgentEvent(BuildErrorEvent("poll-error", "poll failed", true));
            return true;  // fatal infra error -> caller exits non-zero (BR-12)
        }
        // Peer disconnect -> normal exit (disconnect-done already emitted by OnAbilityDisconnectDone).
        if (wakeupFd >= 0 && (pfds[1].revents & (POLLIN | POLLHUP))) {
            return false;
        }
        if (pfds[0].revents & POLLIN) {
            char buf[4096];
            ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
            if (n < 0) {
                if (errno == EINTR) {
                    continue;
                }
                EmitAgentEvent(BuildErrorEvent("stdin-read-error", "read stdin failed", true));
                return true;  // fatal infra error
            }
            if (n == 0) {
                return false;  // EOF: stdin closed (e.g. cliMgr ClearSession) -> normal exit
            }
            // Cap a single JSON line at the IPC single-message convention; a larger line would already
            // fail the cliMgr SendMessage IPC. Enforce here too for clean failure (no OOM/loop).
            if (lineBuf.size() + static_cast<size_t>(n) > MAX_LINE_BYTES) {
                EmitAgentEvent(BuildErrorEvent("bad-json", "line exceeds 200KB limit", true));
                return true;  // fatal: hostile/broken stream, do not recover
            }
            lineBuf.append(buf, static_cast<size_t>(n));
            // Cross-block reassembly: split on '\n' frame delimiter (cliMgr SendMessage writes raw
            // text without adding '\n'; the caller must frame each command with a trailing '\n').
            size_t pos;
            while ((pos = lineBuf.find('\n')) != std::string::npos) {
                std::string line = lineBuf.substr(0, pos);
                lineBuf.erase(0, pos + 1);
                if (!line.empty() && !connection->IsDisconnectInitiated()) {
                    // Disconnecting/CleaningUp state: drain stdin but do not dispatch further
                    // commands on a tearing-down connection.
                    DispatchLine(line, connection, receiver, hostObj);
                }
            }
        }
    }
}

}  // namespace AAFwk
}  // namespace OHOS
