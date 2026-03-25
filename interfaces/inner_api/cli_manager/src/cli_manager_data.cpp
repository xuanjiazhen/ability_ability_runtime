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
    if (!parcel.WriteStringVector(permissions)) {
        return false;
    }
    if (!parcel.WriteInt32(timeout)) {
        return false;
    }
    if (!parcel.WriteStringVector(supportedEvents)) {
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
    if (!parcel.ReadStringVector(&info->permissions)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadInt32(info->timeout)) {
        delete info;
        return nullptr;
    }
    if (!parcel.ReadStringVector(&info->supportedEvents)) {
        delete info;
        return nullptr;
    }
    return info;
}

bool ExecOptions::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(mode)) {
        return false;
    }
    if (!parcel.WriteInt32(timeout)) {
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
    if (!parcel.WriteString(cwd)) {
        return false;
    }
    return true;
}

ExecOptions *ExecOptions::Unmarshalling(Parcel &parcel)
{
    auto *options = new (std::nothrow) ExecOptions();
    if (options && !parcel.ReadString(options->mode)) {
        delete options;
        return nullptr;
    }
    if (!parcel.ReadInt32(options->timeout)) {
        delete options;
        return nullptr;
    }
    uint32_t envSize = 0;
    if (!parcel.ReadUint32(envSize)) {
        delete options;
        return nullptr;
    }
    for (uint32_t i = 0; i < envSize; i++) {
        std::string key, value;
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
    if (!parcel.ReadString(options->cwd)) {
        delete options;
        return nullptr;
    }
    return options;
}

bool ExecResult::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(taskId)) {
        return false;
    }
    if (!parcel.WriteInt32(exitCode)) {
        return false;
    }
    if (!parcel.WriteString(stdout)) {
        return false;
    }
    if (!parcel.WriteString(stderr)) {
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
    if (!parcel.ReadString(result->stdout)) {
        delete result;
        return nullptr;
    }
    if (!parcel.ReadString(result->stderr)) {
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

} // namespace AbilityRuntime
} // namespace OHOS
