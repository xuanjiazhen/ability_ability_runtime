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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_SCHEDULER_RECIPIENT_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_SCHEDULER_RECIPIENT_H

#include "cli_event_reply_manager.h"
#include "cli_tool_manager_scheduler_stub.h"

namespace OHOS {
namespace CliTool {

class CliToolManagerSchedulerRecipient : public CliToolManagerSchedulerStub {
public:
    using SessionEventHandler =
        std::function<int32_t(const std::string &, const std::string &, const CliToolEvent &)>;
    using ReplyEventHandler = std::function<int32_t(const std::string &, const CliEventReplyResult &)>;

    explicit CliToolManagerSchedulerRecipient(const SessionEventHandler &handler,
        const ReplyEventHandler &reply)
        : handler_(handler), reply_(reply)
    {}

    ~CliToolManagerSchedulerRecipient() override = default;

    int32_t SchedulerSessionEvent(const std::string &sessionId,
                                 const std::string &subscriptionId,
                                 const CliToolEvent &event) override;

    int32_t SchedulerInputReplyEvent(const std::string &eventId, int32_t resultCode) override;

    int32_t SchedulerExecToolReplyEvent(const std::string &eventId,
                                       int32_t resultCode,
                                       const CliSessionInfo &session) override;

private:
    int32_t ScheduleReplyEvent(const std::string &eventId, const CliEventReplyResult &result);

private:
    SessionEventHandler handler_;

    ReplyEventHandler reply_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_SCHEDULER_RECIPIENT_H