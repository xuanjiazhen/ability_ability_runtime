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

#include <algorithm>
#include <cerrno>
#include <cstddef>
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
    if (op == "sendData" || op == "authorize") {
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
        int32_t ret = (op == "sendData") ? receiver->SendData(hostObj, data)
                                         : receiver->Authorize(hostObj, data);
        if (ret != 0) {
            const char *opName = (op == "sendData") ? "SendData" : "Authorize";
            const char *evt = (op == "sendData") ? "send-failed" : "authorize-failed";
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

// Self-delimiting JSON reassembly (no '\n' framing): a JSON value defines its own boundary via
// nlohmann's parser; compact/pretty-printed and concatenated values need no delimiter.
// BoundarySax: tracks whether one top-level value completed and the parse_error position (bytes).
class BoundarySax : public nlohmann::json_sax<nlohmann::json> {
public:
    int valuesCompleted = 0;       // # of top-level values fully parsed (0 or 1)
    std::size_t errorPos = 0;     // parse_error position (bytes consumed)
    std::string errorMsg;

    bool null() override { OnScalar(); return true; }
    bool boolean(bool) override { OnScalar(); return true; }
    bool number_integer(number_integer_t) override { OnScalar(); return true; }
    bool number_unsigned(number_unsigned_t) override { OnScalar(); return true; }
    bool number_float(number_float_t, const string_t&) override { OnScalar(); return true; }
    bool string(string_t&) override { OnScalar(); return true; }
    bool binary(binary_t&) override { OnScalar(); return true; }
    bool start_object(std::size_t) override { ++depth_; return true; }
    bool start_array(std::size_t) override { ++depth_; return true; }
    bool end_object() override { if (--depth_ == 0) valuesCompleted = 1; return true; }
    bool end_array() override { if (--depth_ == 0) valuesCompleted = 1; return true; }
    bool key(string_t&) override { return true; }
    bool parse_error(std::size_t position, const std::string&, const nlohmann::detail::exception& ex) override
    {
        errorPos = position;
        errorMsg = ex.what();
        return false;  // stop parsing
    }

private:
    void OnScalar()
    {
        // a top-level scalar completes a value
        if (depth_ == 0) {
            valuesCompleted = 1;
        }
    }
    int depth_ = 0;
};

// Length of the first complete JSON value in `buf` (caller saw valuesCompleted>=1). `hint` is
// the trailing-token parse_error position. nlohmann reports it as a 1-indexed column (byte
// offset of the failing token + 1) and, for an incomplete trailing literal, past EOF -- so it
// is NOT a direct value end. Find the largest clean prefix by re-parsing downward. Bounded to
// <=4KB iterations in practice: RunCommandLoop reads stdin in 4KB chunks and DrainCompleteJsonValues
// runs after every read, so a complete value is sliced out the moment its closing token arrives;
// the trailing content co-located in any one drain is <= one read (<=4KB), not the full buffer.
std::size_t FindFirstCompleteValueEnd(const std::string &buf, std::size_t hint)
{
    std::size_t upper = std::min(hint, buf.size());
    if (upper == 0) {
        upper = buf.size();  // defensive
    }
    for (std::size_t e = upper; e > 0; --e) {  // largest clean prefix first; e=0 (empty) is never a value
        nlohmann::json j = nlohmann::json::parse(buf.substr(0, e), nullptr, false);
        if (!j.is_discarded()) {
            return e;
        }
    }
    return buf.size();  // fallback
}

// Dispatch every complete JSON value in `buf`; keep the incomplete tail. Malformed mid-buffer
// drops the offending byte and resyncs (non-fatal); incomplete (EOF mid-value) waits for bytes.
void DrainCompleteJsonValues(std::string &buf, const sptr<CliAgentConnection> &connection,
    const sptr<AgentRuntime::IAgentReceiver> &receiver, const sptr<IRemoteObject> &hostObj)
{
    while (!buf.empty()) {
        BoundarySax sax;
        // strict (default): trailing bytes after a value trigger parse_error at the next token.
        bool ok = nlohmann::json::sax_parse(buf, &sax);
        if (sax.valuesCompleted >= 1) {
            // ok (no trailing bytes) -> the whole buf is one value; !ok (trailing bytes after
            // the value) -> find the first value's exact end. FindFirstCompleteValueEnd handles
            // the 1-indexed errorPos and the incomplete-trailing-past-EOF cases (see its comment).
            std::size_t end = ok ? buf.size() : FindFirstCompleteValueEnd(buf, sax.errorPos);
            std::string one = buf.substr(0, end);
            buf.erase(0, end);
            if (!connection->IsDisconnectInitiated()) {
                DispatchLine(one, connection, receiver, hostObj);
            }
            continue;
        }
        // No complete value in the buffer.
        if (sax.errorPos < buf.size()) {
            // malformed token mid-buffer (parser did not reach EOF) -> drop it and resync the
            // remaining tail (non-fatal). A single bad byte must not discard subsequent valid
            // values that arrived in the same chunk (e.g. {"a":1}\x01{"b":2} keeps {"b":2}).
            EmitAgentEvent(BuildErrorEvent("bad-json", "malformed JSON: " + sax.errorMsg, false));
            // nlohmann's parse_error position is a 1-indexed column == byte_offset_of_bad + 1, so
            // dropping `errorPos` bytes discards exactly through the offending byte (guard >=1).
            std::size_t drop = (sax.errorPos > 0) ? sax.errorPos : 1;
            buf.erase(0, drop);
            continue;  // re-parse the remaining tail in the same drain
        }
        // else: incomplete (parser reached EOF mid-value) -> keep buffer, wait for more bytes.
        break;
    }
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
        // POLLHUP (without POLLIN) is what some kernels return for a pipe at EOF (write end
        // closed, no pending data): poll wakes with revents=POLLHUP-only and read() then returns 0.
        // Checking only POLLIN would skip the read and busy-spin forever on EOF; handle both so the
        // EOF read (n==0) exits the loop normally.
        if (pfds[0].revents & (POLLIN | POLLHUP)) {
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
            lineBuf.append(buf, static_cast<size_t>(n));
            // Cap: a buffer this large without a complete value is a hostile/broken stream.
            // Non-fatal: one bad line must not kill a long-lived session. Normal >10240 messages
            // are already blocked at cliMgr sendMessage (35600033); this guards direct stdin/pipe
            // streams. Drop the oversized partial line + continue (bounds memory growth).
            if (lineBuf.size() > MAX_LINE_BYTES) {
                EmitAgentEvent(BuildErrorEvent("bad-json", "exceeds 200KB limit", false));
                lineBuf.clear();
                continue;
            }
            // Self-delimiting reassembly (no '\n' framing); value boundary is the JSON itself.
            DrainCompleteJsonValues(lineBuf, connection, receiver, hostObj);
        }
    }
}

}  // namespace AAFwk
}  // namespace OHOS
