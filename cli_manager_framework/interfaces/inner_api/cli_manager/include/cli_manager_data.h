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

#ifndef OHOS_ABILITY_RUNTIME_CLI_MANAGER_DATA_H
#define OHOS_ABILITY_RUNTIME_CLI_MANAGER_DATA_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include "parcel.h"

namespace OHOS {
namespace AbilityRuntime {

/**
 * @brief Tool summary information (lightweight for listing)
 * @since 2025-03-28
 */
class ToolSummary : public Parcelable {
public:
    std::string name;
    std::string version;
    std::string description;

    bool Marshalling(Parcel &parcel) const;
    static ToolSummary *Unmarshalling(Parcel &parcel);
};

/**
 * @brief Tool information structure (full version)
 */
class ToolInfo : public Parcelable {
public:
    std::string name;
    std::string version;
    std::string description;
    std::string executablePath;
    std::vector<std::string> permissions;
    std::string inputSchema;       // JSON string
    std::string outputSchema;      // JSON string
    std::string argMapping;        // JSON string
    std::string eventSchemas;      // JSON string (map of event type to schema)
    int32_t timeoutMs;
    std::vector<std::string> eventTypes;
    int64_t registeredTime;
    bool enabled;
    bool hasSubcommands;
    std::string subcommands;       // JSON string (map of subcommand name to SubCommandInfo)

    bool Marshalling(Parcel &parcel) const;
    static ToolInfo *Unmarshalling(Parcel &parcel);
};

/**
 * @brief Tool execution options
 */
class ExecOptions : public Parcelable {
public:
    bool background;               // true: async, false: sync with yieldMs timeout
    int32_t yieldMs;               // foreground wait timeout (only when background=false)
    int32_t timeoutMs;             // total execution timeout (0 = use tool default)
    std::map<std::string, std::string> env;
    std::string workingDir;

    bool Marshalling(Parcel &parcel) const;
    static ExecOptions *Unmarshalling(Parcel &parcel);
};

/**
 * @brief Tool execution result
 */
class ExecResult : public Parcelable {
public:
    std::string taskId;
    int32_t exitCode;
    std::string stdOut;
    std::string stdErr;
    int32_t signal;
    bool timedOut;
    int64_t executionTime;

    bool Marshalling(Parcel &parcel) const;
    static ExecResult *Unmarshalling(Parcel &parcel);
};

/**
 * @brief Session information
 */
class SessionInfo : public Parcelable {
public:
    std::string sessionId;
    std::string toolName;
    std::string status;            // "running", "completed", "failed"
    int64_t startTime;
    int64_t endTime;
    std::shared_ptr<ExecResult> result;  // optional, only when status="completed"

    SessionInfo() = default;

    bool Marshalling(Parcel &parcel) const;
    static SessionInfo *Unmarshalling(Parcel &parcel);
};

/**
 * @brief Tool event (for async mode)
 */
class ToolEvent : public Parcelable {
public:
    std::string type;              // "stdout", "stderr", "exit", "error"
    std::string data;
    int32_t exitCode;
    int64_t timestamp;

    bool Marshalling(Parcel &parcel) const;
    static ToolEvent *Unmarshalling(Parcel &parcel);
};

/**
 * @brief Session status (for query)
 */
class SessionStatus : public Parcelable {
public:
    std::string sessionId;
    std::string toolName;
    std::string status;            // "running", "completed", "failed"
    int64_t startTime;
    int64_t endTime;
    std::shared_ptr<ExecResult> result;  // optional, only when status="completed"

    SessionStatus() = default;

    bool Marshalling(Parcel &parcel) const;
    static SessionStatus *Unmarshalling(Parcel &parcel);
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MANAGER_DATA_H
