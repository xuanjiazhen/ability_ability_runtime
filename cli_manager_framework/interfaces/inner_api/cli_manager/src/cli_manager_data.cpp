/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cli_manager_data.h"
#include "parcel.h"

namespace OHOS {
namespace AbilityRuntime {

// ToolSummary
bool ToolSummary::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(name)) {
        return false;
    }
    if (!parcel.WriteString(version)) {
        return false;
    }
    if (!parcel.WriteString(description)) {
        return false;
    }
    return true;
}

ToolSummary *ToolSummary::Unmarshalling(Parcel &parcel)
{
    auto *summary = new (std::nothrow) ToolSummary();
    if (summary && !parcel.ReadString(summary->name)) {
        delete summary;
        return nullptr;
    }
    if (!parcel.ReadString(summary->version)) {
        delete summary;
        return nullptr;
    }
    if (!parcel.ReadString(summary->description)) {
        delete summary;
        return nullptr;
    }
    return summary;
}

// ToolInfo
bool ToolInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(name)) {
        return false;
    }
    if (!parcel.WriteString(version)) {
        return false;
    }
    if (!parcel.WriteString(description)) {
        return false;
    }
    if (!parcel.WriteString(executablePath)) {
        return false;
    }
    if (!parcel.WriteStringVector(permissions)) {
        return false;
    }
    if (!parcel.WriteString(inputSchema)) {
        return false;
    }
    if (!parcel.WriteString(outputSchema)) {
        return false;
    }
    if (!parcel.WriteString(argMapping)) {
        return false;
    }
    if (!parcel.WriteString(eventSchemas)) {
        return false;
    }
    if (!parcel.WriteInt32(timeoutMs)) {
        return false;
    }
    if (!parcel.WriteStringVector(eventTypes)) {
        return false;
    }
    if (!parcel.WriteInt64(registeredTime)) {
        return false;
    }
    if (!parcel.WriteBool(enabled)) {
        return false;
    }
    if (!parcel.WriteBool(hasSubcommands)) {
        return false;
    }
    if (!parcel.WriteString(subcommands)) {
        return false;
    }
    return true;
}

ToolInfo *ToolInfo::Unmarshalling(Parcel &parcel)
{
    auto *info = new (std::nothrow) ToolInfo();
    if (info && !parcel.ReadString(info->name)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->version)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->description)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->executablePath)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadStringVector(&info->permissions)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->inputSchema)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->outputSchema)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->argMapping)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->eventSchemas)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadInt32(info->timeoutMs)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadStringVector(&info->eventTypes)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadInt64(info->registeredTime)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadBool(info->enabled)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadBool(info->hasSubcommands)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->subcommands)) {
        delete info;
        return nullptr;
    }
    return info;
}

// ExecOptions
bool ExecOptions::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteBool(background)) {
        return false;
    }
    if (!parcel.WriteInt32(yieldMs)) {
        return false;
    }
    if (!parcel.WriteInt32(timeoutMs)) {
        return false;
    }
    if (!parcel.WriteUint32(static_cast<uint32_t>(env.size()))) {
        return false;
    }
    for (const auto &[key, value] : env) {
        if (!parcel.WriteString(key)) {
            return false;
        }
        if (!parcel.WriteString(value)) {
            return false;
        }
    }
    if (!parcel.WriteString(workingDir)) {
        return false;
    }
    return true;
}

ExecOptions *ExecOptions::Unmarshalling(Parcel &parcel)
{
    auto *options = new (std::nothrow) ExecOptions();
    if (options && !parcel.ReadBool(options->background)) {
        delete options;
        return nullptr;
    }
    if (!parcel.ReadInt32(options->yieldMs)) {
        delete options;
        return nullptr;
    }
    if (!parcel.ReadInt32(options->timeoutMs)) {
        delete options;
        return nullptr;
    }
    uint32_t envSize = 0;
    if (!parcel.ReadUint32(envSize)) {
        delete options;
        return nullptr;
    }
    for (uint32_t i = 0; i < envSize; i++) {
        std::string key;
        std::string value;
        if (!parcel.ReadString(key)) {
            delete options;
            return nullptr;
        }
        if (!parcel.ReadString(value)) {
            delete options;
            return nullptr;
        }
        options->env[key] = value;
    }
    if (!parcel.ReadString(options->workingDir)) {
        delete options;
        return nullptr;
    }
    return options;
}

// ExecResult
bool ExecResult::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(taskId)) {
        return false;
    }
    if (!parcel.WriteInt32(exitCode)) {
        return false;
    }
    if (!parcel.WriteString(stdOut)) {
        return false;
    }
    if (!parcel.WriteString(stdErr)) {
        return false;
    }
    if (!parcel.WriteInt32(signal)) {
        return false;
    }
    if (!parcel.WriteBool(timedOut)) {
        return false;
    }
    if (!parcel.WriteInt64(executionTime)) {
        return false;
    }
    return true;
}

ExecResult *ExecResult::Unmarshalling(Parcel &parcel)
{
    auto *result = new (std::nothrow) ExecResult();
    if (result && !parcel.ReadString(result->taskId)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadInt32(result->exitCode)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadString(result->stdOut)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadString(result->stdErr)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadInt32(result->signal)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadBool(result->timedOut)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadInt64(result->executionTime)) {
        delete result;
        return nullptr;
    }
    return result;
}

// SessionInfo
bool SessionInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(sessionId)) {
        return false;
    }
    if (!parcel.WriteString(toolName)) {
        return false;
    }
    if (!parcel.WriteString(status)) {
        return false;
    }
    if (!parcel.WriteInt64(startTime)) {
        return false;
    }
    if (!parcel.WriteInt64(endTime)) {
        return false;
    }
    // Write result presence flag
    bool hasResult = (result != nullptr);
    if (!parcel.WriteBool(hasResult)) {
        return false;
    }
    if (hasResult && !result->Marshalling(parcel)) {
        return false;
    }
    return true;
}

SessionInfo *SessionInfo::Unmarshalling(Parcel &parcel)
{
    auto *info = new (std::nothrow) SessionInfo();
    if (info && !parcel.ReadString(info->sessionId)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->toolName)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadString(info->status)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadInt64(info->startTime)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadInt64(info->endTime)) {
        delete info;
        return nullptr;
    }
    bool hasResult = false;
    if (!parcel.ReadBool(hasResult)) {
        delete info;
        return nullptr;
    }
    if (hasResult) {
        ExecResult *resultPtr = ExecResult::Unmarshalling(parcel);
        if (resultPtr == nullptr) {
            delete info;
            return nullptr;
        }
        info->result = std::shared_ptr<ExecResult>(resultPtr);
    }
    return info;
}

// ToolEvent
bool ToolEvent::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(type)) {
        return false;
    }
    if (!parcel.WriteString(data)) {
        return false;
    }
    if (!parcel.WriteInt32(exitCode)) {
        return false;
    }
    if (!parcel.WriteInt64(timestamp)) {
        return false;
    }
    return true;
}

ToolEvent *ToolEvent::Unmarshalling(Parcel &parcel)
{
    auto *event = new (std::nothrow) ToolEvent();
    if (event && !parcel.ReadString(event->type)) {
        delete event;
        return nullptr;
    }
    if (!parcel.ReadString(event->data)) {
        delete event;
        return nullptr;
    }
    if (!parcel.ReadInt32(event->exitCode)) {
        delete event;
        return nullptr;
    }
    if (!parcel.ReadInt64(event->timestamp)) {
        delete event;
        return nullptr;
    }
    return event;
}

// SessionStatus
bool SessionStatus::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(sessionId)) {
        return false;
    }
    if (!parcel.WriteString(toolName)) {
        return false;
    }
    if (!parcel.WriteString(status)) {
        return false;
    }
    if (!parcel.WriteInt64(startTime)) {
        return false;
    }
    if (!parcel.WriteInt64(endTime)) {
        return false;
    }
    // Write result presence flag
    bool hasResult = (result != nullptr);
    if (!parcel.WriteBool(hasResult)) {
        return false;
    }
    if (hasResult && !result->Marshalling(parcel)) {
        return false;
    }
    return true;
}

SessionStatus *SessionStatus::Unmarshalling(Parcel &parcel)
{
    auto *status = new (std::nothrow) SessionStatus();
    if (status && !parcel.ReadString(status->sessionId)) {
        delete status;
        return nullptr;
    }
    if (!parcel.ReadString(status->toolName)) {
        delete status;
        return nullptr;
    }
    if (!parcel.ReadString(status->status)) {
        delete status;
        return nullptr;
    }
    if (!parcel.ReadInt64(status->startTime)) {
        delete status;
        return nullptr;
    }
    if (!parcel.ReadInt64(status->endTime)) {
        delete status;
        return nullptr;
    }
    bool hasResult = false;
    if (!parcel.ReadBool(hasResult)) {
        delete status;
        return nullptr;
    }
    if (hasResult) {
        ExecResult *resultPtr = ExecResult::Unmarshalling(parcel);
        if (resultPtr == nullptr) {
            delete status;
            return nullptr;
        }
        status->result = std::shared_ptr<ExecResult>(resultPtr);
    }
    return status;
}

} // namespace AbilityRuntime
} // namespace OHOS
