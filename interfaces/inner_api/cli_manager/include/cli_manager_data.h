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

#include <string>
#include <vector>
#include <map>
#include "parcel.h"

namespace OHOS {
namespace AbilityRuntime {

/**
 * @brief Tool information structure
 */
class ToolInfo : public Parcelable {
public:
    std::string name;
    std::string version;
    std::string description;
    std::vector<std::string> permissions;
    int32_t timeout;
    std::vector<std::string> supportedEvents;

    bool Marshalling(Parcel &parcel) const;
    static ToolInfo *Unmarshalling(Parcel &parcel);
};

/**
 * @brief Tool execution options
 */
class ExecOptions : public Parcelable {
public:
    std::string mode; // "sync" or "async"
    int32_t timeout;
    std::map<std::string, std::string> env;
    std::string cwd;

    bool Marshalling(Parcel &parcel) const;
    static ExecOptions *Unmarshalling(Parcel &parcel);
};

/**
 * @brief Tool execution result (sync mode)
 */
class ExecResult : public Parcelable {
public:
    std::string taskId;
    int32_t exitCode;
    std::string stdout;
    std::string stderr;
    int32_t signal;
    bool timedOut;
    int64_t executionTime;

    bool Marshalling(Parcel &parcel) const;
    static ExecResult *Unmarshalling(Parcel &parcel);
};

/**
 * @brief Tool event (async mode)
 */
class ToolEvent : public Parcelable {
public:
    std::string type; // "stdout", "stderr", "exit", "error"
    std::string data;
    int32_t exitCode;
    int64_t timestamp;

    bool Marshalling(Parcel &parcel) const;
    static ToolEvent *Unmarshalling(Parcel &parcel);
};

/**
 * @brief Task status
 */
class TaskStatus : public Parcelable {
public:
    std::string taskId;
    std::string toolName;
    std::string status; // "running", "completed", "failed", "timeout", "cancelled"
    int64_t startTime;
    int64_t endTime;
    int32_t exitCode;

    bool Marshalling(Parcel &parcel) const;
    static TaskStatus *Unmarshalling(Parcel &parcel);
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MANAGER_DATA_H
