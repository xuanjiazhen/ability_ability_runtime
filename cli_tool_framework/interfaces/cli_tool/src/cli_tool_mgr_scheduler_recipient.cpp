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

#include "cli_tool_mgr_scheduler_recipient.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t ERROR_CODE = -1;
} // namespace

int32_t CliToolManagerSchedulerRecipient::SchedulerSessionEvent(const std::string &sessionId,
                                                                const std::string &subscriptionId,
                                                                const CliToolEvent &event)
{
    if (!handler_) {
        return ERROR_CODE;
    }
    return handler_(sessionId, subscriptionId, event);
}

int32_t CliToolManagerSchedulerRecipient::SchedulerInputReplyEvent(const std::string &eventId, int32_t resultCode)
{
    return SchedulerReplyEvent(eventId, CliEventReplyResult {resultCode, std::nullopt});
}

int32_t CliToolManagerSchedulerRecipient::SchedulerExecToolReplyEvent(const std::string &eventId,
                                                                      int32_t resultCode,
                                                                      const CliSessionInfo &session)
{
    return SchedulerReplyEvent(eventId, CliEventReplyResult {resultCode, session});
}

int32_t CliToolManagerSchedulerRecipient::SchedulerReplyEvent(const std::string &eventId,
                                                              const CliEventReplyResult &result)
{
    if (!reply_) {
        return ERROR_CODE;
    }
    return reply_(eventId, result);
}

} // namespace CliTool
} // namespace OHOS