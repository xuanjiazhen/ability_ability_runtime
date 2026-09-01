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

#include <gtest/gtest.h>

#include <fcntl.h>
#include <sstream>
#include <string>
#include <unistd.h>

#include "agent_receiver_stub.h"
#include "cli_agent_connection.h"
#include "ipc_object_stub.h"
#include "session_protocol.h"
#include "want.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;

namespace {
// A host->agent IAgentReceiver stub whose SendData/Authorize return configurable rets and record
// calls, so DispatchLine's sendmessage/authorize paths can be driven without a live agent.
class MockAgentReceiver : public AgentRuntime::AgentReceiverStub {
public:
    int32_t sendDataCount = 0;
    int32_t authorizeCount = 0;
    int32_t sendDataRet = 0;
    int32_t authorizeRet = 0;
    std::string lastData;

    int32_t SendData(const sptr<IRemoteObject> &connectorProxy, const std::string &data) override
    {
        (void)connectorProxy;
        sendDataCount++;
        lastData = data;
        return sendDataRet;
    }
    int32_t Authorize(const sptr<IRemoteObject> &connectorProxy, const std::string &data) override
    {
        (void)connectorProxy;
        authorizeCount++;
        lastData = data;
        return authorizeRet;
    }
    int32_t AgentInvoked(const std::string &agentId) override
    {
        (void)agentId;
        return 0;
    }
};

// Captures std::cout (EmitAgentEvent writes there) for assertion. RAII-restores on destruction.
class StdoutCapture {
public:
    StdoutCapture() : saved_(std::cout.rdbuf(buf_.rdbuf())) {}
    ~StdoutCapture() { std::cout.rdbuf(saved_); }
    std::string Take() { std::string s = buf_.str(); buf_.str(""); buf_.clear(); return s; }
private:
    std::stringstream buf_;  // declared first so it initializes before saved_
    std::streambuf *saved_ = nullptr;
};

// Redirects fd 0 (stdin) from a pipe + captures std::cout, so RunCommandLoop can be driven. The
// pipe capacity is raised to 1 MiB so the 200 KiB single-line cap test can be written in one shot.
class StdioRedirect {
public:
    StdioRedirect() : savedStdin_(dup(STDIN_FILENO)), savedCout_(std::cout.rdbuf(buf_.rdbuf()))
    {
        int p[2];
        if (pipe(p) != 0) {
            return;
        }
        readFd_ = p[0];
        writeFd_ = p[1];
        (void)fcntl(writeFd_, F_SETPIPE_SZ, 1024 * 1024);
        dup2(readFd_, STDIN_FILENO);
    }
    ~StdioRedirect()
    {
        if (savedStdin_ >= 0) {
            dup2(savedStdin_, STDIN_FILENO);
            close(savedStdin_);
        }
        if (readFd_ >= 0) {
            close(readFd_);
        }
        if (writeFd_ >= 0) {
            close(writeFd_);
        }
        std::cout.rdbuf(savedCout_);
    }
    void Write(const std::string &s) { (void)write(writeFd_, s.data(), s.size()); }
    void CloseStdin()
    {
        if (writeFd_ >= 0) {
            close(writeFd_);
            writeFd_ = -1;
        }
    }
    std::string Take() { std::string s = buf_.str(); buf_.str(""); buf_.clear(); return s; }

private:
    std::stringstream buf_;  // declared first so it initializes before savedCout_
    int savedStdin_ = -1;
    std::streambuf *savedCout_ = nullptr;
    int readFd_ = -1;
    int writeFd_ = -1;
};
}  // namespace

// ---- pure-logic helpers ----

HWTEST(OhosAgentCliTest, EscapeJsonString_EscapesSpecialChars, TestSize.Level1)
{
    EXPECT_EQ(EscapeJsonString(""), "");
    EXPECT_EQ(EscapeJsonString("plain"), "plain");
    EXPECT_EQ(EscapeJsonString("a\"b"), "a\\\"b");
    EXPECT_EQ(EscapeJsonString("a\\b"), "a\\\\b");
    EXPECT_EQ(EscapeJsonString("a\nb"), "a\\nb");
    EXPECT_EQ(EscapeJsonString("a\rb"), "a\\rb");
    EXPECT_EQ(EscapeJsonString("a\tb"), "a\\tb");
    EXPECT_EQ(EscapeJsonString(std::string("a\bb")), "a\\bb");
    // Control char below 0x20 -> \uXXXX.
    std::string ctrl(1, '\x01');
    EXPECT_EQ(EscapeJsonString(ctrl), "\\u0001");
}

HWTEST(OhosAgentCliTest, BuildEvents_Shape, TestSize.Level1)
{
    EXPECT_EQ(BuildConnectDoneEvent(true, "aid", 0, ""), "{\"type\":\"connect-done\",\"status\":\"ok\","
        "\"agentId\":\"aid\",\"errCode\":\"0\",\"errMsg\":\"\"}");
    EXPECT_EQ(BuildConnectDoneEvent(false, "aid", -2, "receiver proxy null"),
        "{\"type\":\"connect-done\",\"status\":\"fail\",\"agentId\":\"aid\",\"errCode\":\"-2\","
        "\"errMsg\":\"receiver proxy null\"}");
    EXPECT_EQ(BuildDisconnectDoneEvent("peer"), "{\"type\":\"disconnect-done\",\"reason\":\"peer\"}");
    EXPECT_EQ(BuildOnDataEvent("hello"), "{\"type\":\"OnData\",\"data\":\"hello\"}");
    EXPECT_EQ(BuildOnAuthorizeEvent("tok"), "{\"type\":\"OnAuthorize\",\"data\":\"tok\"}");
    EXPECT_EQ(BuildErrorEvent("bad-json", "oops", false),
        "{\"type\":\"error\",\"errCode\":\"bad-json\",\"errMsg\":\"oops\",\"fatal\":false}");
    EXPECT_EQ(BuildErrorEvent("poll-error", "x", true),
        "{\"type\":\"error\",\"errCode\":\"poll-error\",\"errMsg\":\"x\",\"fatal\":true}");
}

// ---- host-side CliAgentConnectorStub (agent->host pushes) ----

HWTEST(OhosAgentCliTest, ConnectorStub_SendData_EmitsOnData, TestSize.Level1)
{
    StdoutCapture cap;
    auto stub = sptr<CliAgentConnectorStub>::MakeSptr();
    EXPECT_EQ(stub->SendData("payload"), 0);
    auto out = cap.Take();
    EXPECT_NE(out.find("\"type\":\"OnData\""), std::string::npos);
    EXPECT_NE(out.find("\"data\":\"payload\""), std::string::npos);
}

HWTEST(OhosAgentCliTest, ConnectorStub_Authorize_EmitsOnAuthorize, TestSize.Level1)
{
    StdoutCapture cap;
    auto stub = sptr<CliAgentConnectorStub>::MakeSptr();
    EXPECT_EQ(stub->Authorize("decision"), 0);
    auto out = cap.Take();
    EXPECT_NE(out.find("\"type\":\"OnAuthorize\""), std::string::npos);
    EXPECT_NE(out.find("\"data\":\"decision\""), std::string::npos);
}

// ---- OnAbilityConnectDone + CR-1 proxy-null guard ----

HWTEST(OhosAgentCliTest, ConnectDone_Success_OkProxy, TestSize.Level1)
{
    StdoutCapture cap;
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    conn->SetAgentId("aid");
    auto receiver = sptr<MockAgentReceiver>::MakeSptr();
    AppExecFwk::ElementName element;
    conn->OnAbilityConnectDone(element, receiver->AsObject(), 0);
    auto out = cap.Take();
    EXPECT_NE(out.find("\"status\":\"ok\""), std::string::npos);
    sptr<AgentRuntime::IAgentReceiver> proxy;
    sptr<IRemoteObject> hostObj;
    EXPECT_EQ(conn->WaitForConnect(100, proxy, hostObj), 0);  // ERR_OK
    EXPECT_NE(proxy, nullptr);
}

// CR-1: resultCode==0 but the remoteObject is not an IAgentReceiver (null remoteObject or iface_cast
// mismatch) -> connect-done(fail, CONNECT_RESULT_PROXY_NULL=-2), not connect-done(ok)+idle spin.
HWTEST(OhosAgentCliTest, ConnectDone_ProxyNull_FailNeg2_Cr1, TestSize.Level1)
{
    StdoutCapture cap;
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    conn->SetAgentId("aid");
    AppExecFwk::ElementName element;
    // A bare stub with a non-matching descriptor: iface_cast<IAgentReceiver> returns null.
    auto bogus = sptr<IPCObjectStub>::MakeSptr(u"not.agentruntime.IAgentReceiver");
    conn->OnAbilityConnectDone(element, bogus, 0);  // IPCObjectStub IS the IRemoteObject
    auto out = cap.Take();
    EXPECT_NE(out.find("\"status\":\"fail\""), std::string::npos);
    EXPECT_NE(out.find("\"errCode\":\"-2\""), std::string::npos);
    EXPECT_NE(out.find("receiver proxy null"), std::string::npos);
    sptr<AgentRuntime::IAgentReceiver> proxy;
    sptr<IRemoteObject> hostObj;
    EXPECT_EQ(conn->WaitForConnect(100, proxy, hostObj), -2);  // CONNECT_RESULT_PROXY_NULL
    EXPECT_EQ(proxy, nullptr);
}

HWTEST(OhosAgentCliTest, ConnectDone_NonZeroResult_Fail, TestSize.Level1)
{
    StdoutCapture cap;
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    conn->SetAgentId("aid");
    AppExecFwk::ElementName element;
    conn->OnAbilityConnectDone(element, nullptr, 99);
    auto out = cap.Take();
    EXPECT_NE(out.find("\"status\":\"fail\""), std::string::npos);
    EXPECT_NE(out.find("\"errCode\":\"99\""), std::string::npos);
}

// WaitForConnect times out (returns -1) when OnAbilityConnectDone never fires, so the connect-phase
// wait has a bounded failure mode (no infinite block) — covers the WaitForConnect timeout branch.
HWTEST(OhosAgentCliTest, WaitForConnect_Timeout_ReturnsNeg1, TestSize.Level1)
{
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    conn->SetAgentId("aid");
    sptr<AgentRuntime::IAgentReceiver> proxy;
    sptr<IRemoteObject> hostObj;
    EXPECT_EQ(conn->WaitForConnect(50, proxy, hostObj), -1);  // timeout
    EXPECT_EQ(proxy, nullptr);
    EXPECT_EQ(hostObj, nullptr);
}

// ---- OnAbilityDisconnectDone + wakeup + reason ----

HWTEST(OhosAgentCliTest, DisconnectDone_PeerReason_WakesLoop, TestSize.Level1)
{
    StdoutCapture cap;
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    ASSERT_TRUE(conn->HasWakeup());
    AppExecFwk::ElementName element;
    conn->OnAbilityDisconnectDone(element, 0);
    auto out = cap.Take();
    EXPECT_NE(out.find("\"type\":\"disconnect-done\""), std::string::npos);
    EXPECT_NE(out.find("\"reason\":\"peer\""), std::string::npos);
    // peer disconnect marks Disconnecting + signals the wakeup pipe.
    EXPECT_TRUE(conn->IsDisconnectInitiated());
    EXPECT_TRUE(conn->TryInitiateDisconnect() == false);  // already initiated
    // wakeup byte readable now.
    char c = 0;
    EXPECT_EQ(read(conn->GetWakeupFd(), &c, 1), 1);
}

HWTEST(OhosAgentCliTest, DisconnectDone_UserReason_AfterSet, TestSize.Level1)
{
    StdoutCapture cap;
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    conn->SetDisconnectReason("user");
    AppExecFwk::ElementName element;
    conn->OnAbilityDisconnectDone(element, 0);
    auto out = cap.Take();
    EXPECT_NE(out.find("\"reason\":\"user\""), std::string::npos);
}

HWTEST(OhosAgentCliTest, DisconnectDone_NonZeroResult_ErrorReason, TestSize.Level1)
{
    StdoutCapture cap;
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    AppExecFwk::ElementName element;
    conn->OnAbilityDisconnectDone(element, -1);
    auto out = cap.Take();
    EXPECT_NE(out.find("\"reason\":\"error\""), std::string::npos);
}

// ---- 防重入 state machine (TASK-5) ----

HWTEST(OhosAgentCliTest, TryInitiateDisconnect_Cas, TestSize.Level1)
{
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    EXPECT_FALSE(conn->IsDisconnectInitiated());
    EXPECT_TRUE(conn->TryInitiateDisconnect());   // first initiator wins
    EXPECT_TRUE(conn->IsDisconnectInitiated());
    EXPECT_FALSE(conn->TryInitiateDisconnect());  // second is suppressed (no double RPC)
}

// ---- RunCommandLoop: dispatch + reassembly + caps + 防重入 skip-dispatch ----

HWTEST(OhosAgentCliTest, RunCommandLoop_SendMessage_Success, TestSize.Level1)
{
    StdioRedirect io;
    auto receiver = sptr<MockAgentReceiver>::MakeSptr();
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    io.Write("{\"op\":\"sendmessage\",\"data\":\"hi\"}\n");
    io.CloseStdin();
    EXPECT_FALSE(RunCommandLoop(conn, receiver, receiver->AsObject()));
    EXPECT_EQ(receiver->sendDataCount, 1);
    EXPECT_EQ(receiver->lastData, "hi");
    EXPECT_TRUE(io.Take().empty());  // no error event on success
}

HWTEST(OhosAgentCliTest, RunCommandLoop_SendMessage_Fail_SendFailed, TestSize.Level1)
{
    StdioRedirect io;
    auto receiver = sptr<MockAgentReceiver>::MakeSptr();
    receiver->sendDataRet = -1;
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    io.Write("{\"op\":\"sendmessage\",\"data\":\"x\"}\n");
    io.CloseStdin();
    EXPECT_FALSE(RunCommandLoop(conn, receiver, receiver->AsObject()));
    auto out = io.Take();
    EXPECT_NE(out.find("\"errCode\":\"send-failed\""), std::string::npos);
}

HWTEST(OhosAgentCliTest, RunCommandLoop_Authorize_Success, TestSize.Level1)
{
    StdioRedirect io;
    auto receiver = sptr<MockAgentReceiver>::MakeSptr();
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    io.Write("{\"op\":\"authorize\",\"data\":\"perm\"}\n");
    io.CloseStdin();
    EXPECT_FALSE(RunCommandLoop(conn, receiver, receiver->AsObject()));
    EXPECT_EQ(receiver->authorizeCount, 1);
    EXPECT_EQ(receiver->lastData, "perm");
}

HWTEST(OhosAgentCliTest, RunCommandLoop_BadJson_NonFatal, TestSize.Level1)
{
    StdioRedirect io;
    auto receiver = sptr<MockAgentReceiver>::MakeSptr();
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    io.Write("not json\n");
    io.CloseStdin();
    EXPECT_FALSE(RunCommandLoop(conn, receiver, receiver->AsObject()));  // non-fatal, keeps looping
    auto out = io.Take();
    EXPECT_NE(out.find("\"errCode\":\"bad-json\""), std::string::npos);
    EXPECT_NE(out.find("\"fatal\":false"), std::string::npos);
}

HWTEST(OhosAgentCliTest, RunCommandLoop_BadData_MissingAndNonString_NonFatal, TestSize.Level1)
{
    StdioRedirect io;
    auto receiver = sptr<MockAgentReceiver>::MakeSptr();
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    io.Write("{\"op\":\"sendmessage\"}\n");          // data missing
    io.Write("{\"op\":\"sendmessage\",\"data\":123}\n");  // data non-string
    io.CloseStdin();
    EXPECT_FALSE(RunCommandLoop(conn, receiver, receiver->AsObject()));
    auto out = io.Take();
    EXPECT_NE(out.find("\"errCode\":\"bad-data\""), std::string::npos);
    EXPECT_EQ(receiver->sendDataCount, 0);  // never dispatched
}

HWTEST(OhosAgentCliTest, RunCommandLoop_NoReceiver_NoReceiverEvent, TestSize.Level1)
{
    StdioRedirect io;
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    io.Write("{\"op\":\"sendmessage\",\"data\":\"x\"}\n");
    io.CloseStdin();
    EXPECT_FALSE(RunCommandLoop(conn, nullptr, nullptr));
    auto out = io.Take();
    EXPECT_NE(out.find("\"errCode\":\"no-receiver\""), std::string::npos);
}

HWTEST(OhosAgentCliTest, RunCommandLoop_UnknownOp, TestSize.Level1)
{
    StdioRedirect io;
    auto receiver = sptr<MockAgentReceiver>::MakeSptr();
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    io.Write("{\"op\":\"bogus\"}\n");
    io.CloseStdin();
    EXPECT_FALSE(RunCommandLoop(conn, receiver, receiver->AsObject()));
    auto out = io.Take();
    EXPECT_NE(out.find("\"errCode\":\"unknown-op\""), std::string::npos);
}

HWTEST(OhosAgentCliTest, RunCommandLine_ReassemblesAcrossBlocks, TestSize.Level1)
{
    StdioRedirect io;
    auto receiver = sptr<MockAgentReceiver>::MakeSptr();
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    // One JSON command split across two writes (no newline until the second).
    io.Write("{\"op\":\"sendmessage\",\"da");
    io.Write("ta\":\"split\"}\n");
    io.CloseStdin();
    EXPECT_FALSE(RunCommandLoop(conn, receiver, receiver->AsObject()));
    EXPECT_EQ(receiver->sendDataCount, 1);
    EXPECT_EQ(receiver->lastData, "split");
}

// 200 KiB single-line cap (BR-10): a larger line is fatal (return true) and does not unbounded-grow.
HWTEST(OhosAgentCliTest, RunCommandLoop_LineExceeds200KB_Fatal, TestSize.Level1)
{
    StdioRedirect io;
    auto receiver = sptr<MockAgentReceiver>::MakeSptr();
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    // 200 KiB + 1 byte, no newline -> exceeds MAX_LINE_BYTES on append.
    std::string big(200 * 1024 + 1, 'a');
    io.Write(big);
    io.CloseStdin();
    EXPECT_TRUE(RunCommandLoop(conn, receiver, receiver->AsObject()));  // fatal -> true
    auto out = io.Take();
    EXPECT_NE(out.find("exceeds 200KB"), std::string::npos);
    EXPECT_NE(out.find("\"fatal\":true"), std::string::npos);
    EXPECT_EQ(receiver->sendDataCount, 0);  // never dispatched
}

HWTEST(OhosAgentCliTest, RunCommandLoop_EOF_NormalExit, TestSize.Level1)
{
    StdioRedirect io;
    auto receiver = sptr<MockAgentReceiver>::MakeSptr();
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    io.CloseStdin();  // immediate EOF
    EXPECT_FALSE(RunCommandLoop(conn, receiver, receiver->AsObject()));  // normal exit
}

// 防重入 skip-dispatch: once disconnect is initiated, subsequent stdin commands are drained but not
// dispatched (the connection is tearing down; no sendmessage on a disconnecting connection).
HWTEST(OhosAgentCliTest, RunCommandLoop_AfterDisconnectInitiated_SkipsDispatch, TestSize.Level1)
{
    StdioRedirect io;
    auto receiver = sptr<MockAgentReceiver>::MakeSptr();
    auto conn = sptr<CliAgentConnection>::MakeSptr();
    ASSERT_TRUE(conn->TryInitiateDisconnect());  // simulate a disconnect RPC already in flight
    io.Write("{\"op\":\"sendmessage\",\"data\":\"late\"}\n");
    io.CloseStdin();
    EXPECT_FALSE(RunCommandLoop(conn, receiver, receiver->AsObject()));
    EXPECT_EQ(receiver->sendDataCount, 0);  // suppressed: Disconnecting state drains but skips
}
