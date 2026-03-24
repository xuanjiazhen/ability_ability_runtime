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

#include "cli_callback_stub.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliMgr {

CLICallbackStub::CLICallbackStub()
{
    TAG_LOGI(TAG, "CLICallbackStub created");
}

void CLICallbackStub::OnOutput(const std::string& taskId, const ToolEvent& event)
{
    TAG_LOGD(TAG, "OnOutput called: task=%{public}s, type=%{public}s",
             taskId.c_str(), event.type.c_str());

    // 调用应用层回调
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (eventCallback_) {
        eventCallback_(taskId, event);
    }
}

void CLICallbackStub::OnExit(const std::string& taskId, int32_t exitCode)
{
    TAG_LOGD(TAG, "OnExit called: task=%{public}s, code=%{public}d",
             taskId.c_str(), exitCode);

    // 调用应用层回调
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (exitCallback_) {
        exitCallback_(taskId, exitCode);
    }
}

void CLICallbackStub::OnError(const std::string& taskId,
                            int32_t errorCode,
                            const std::string& errorMsg)
{
    TAG_LOGD(TAG, "OnError called: task=%{public}s, code=%{public}d, msg=%{public}s",
             taskId.c_str(), errorCode, errorMsg.c_str());

    // 调用应用层回调
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (errorCallback_) {
        errorCallback_(taskId, errorCode, errorMsg);
    }
}

int32_t CLICallbackStub::OnRemoteRequest(uint32_t code,
                                         MessageParcel& data,
                                         MessageParcel& reply,
                                         MessageOption& option)
{
    TAG_LOGD(TAG, "OnRemoteRequest: code=%{public}u", code);

    // 验证接口 Token
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TAG_LOGE(TAG, "Invalid interface token");
        return ERR_INVALID_STATE;
    }

    int32_t ret = ERR_OK;

    switch (code) {
        case 0:  // OnOutput
            ret = HandleOnOutput(data, reply);
            break;
        case 1:  // OnExit
            ret = HandleOnExit(data, reply);
            break;
        case 2:  // OnError
            ret = HandleOnError(data, reply);
            break;
        default:
            TAG_LOGW(TAG, "Unknown code: %{public}u", code);
            ret = ERR_UNKNOWN_TRANSACTION;
            break;
    }

    return ret;
}

int32_t CLICallbackStub::HandleOnOutput(MessageParcel& data, MessageParcel& reply)
{
    // 读取参数
    std::string taskId;
    ToolEvent event;

    if (!data.ReadString(taskId)) {
        TAG_LOGE(TAG, "Failed to read taskId");
        return ERR_INVALID_DATA;
    }

    if (!ReadToolEvent(data, event)) {
        TAG_LOGE(TAG, "Failed to read tool event");
        return ERR_INVALID_DATA;
    }

    TAG_LOGD(TAG, "HandleOnOutput: task=%{public}s, type=%{public}s, dataLen=%{public}zu",
             taskId.c_str(), event.type.c_str(), event.data.size());

    // 调用虚函数（由子类实现）
    OnOutput(taskId, event);

    // 写入返回值
    if (!reply.WriteInt32(ERR_OK)) {
        return ERR_INVALID_DATA;
    }

    return ERR_OK;
}

int32_t CLICallbackStub::HandleOnExit(MessageParcel& data, MessageParcel& reply)
{
    // 读取参数
    std::string taskId;
    int32_t exitCode;

    if (!data.ReadString(taskId)) {
        TAG_LOGE(TAG, "Failed to read taskId");
        return ERR_INVALID_DATA;
    }

    if (!data.ReadInt32(exitCode)) {
        TAG_LOGE(TAG, "Failed to read exitCode");
        return ERR_INVALID_DATA;
    }

    TAG_LOGD(TAG, "HandleOnExit: task=%{public}s, code=%{public}d",
             taskId.c_str(), exitCode);

    // 调用虚函数（由子类实现）
    OnExit(taskId, exitCode);

    // 写入返回值
    if (!reply.WriteInt32(ERR_OK)) {
        return ERR_INVALID_DATA;
    }

    return ERR_OK;
}

int32_t CLICallbackStub::HandleOnError(MessageParcel& data, MessageParcel& reply)
{
    // 读取参数
    std::string taskId;
    int32_t errorCode;
    std::string errorMsg;

    if (!data.ReadString(taskId)) {
        TAG_LOGE(TAG, "Failed to read taskId");
        return ERR_INVALID_DATA;
    }

    if (!data.ReadInt32(errorCode)) {
        TAG_LOGE(TAG, "Failed to read errorCode");
        return ERR_INVALID_DATA;
    }

    if (!data.ReadString(errorMsg)) {
        TAG_LOGE(TAG, "Failed to read errorMsg");
        return ERR_INVALID_DATA;
    }

    TAG_LOGD(TAG, "HandleOnError: task=%{public}s, code=%{public}d, msg=%{public}s",
             taskId.c_str(), errorCode, errorMsg.c_str());

    // 调用虚函数（由子类实现）
    OnError(taskId, errorCode, errorMsg);

    // 写入返回值
    if (!reply.WriteInt32(ERR_OK)) {
        return ERR_INVALID_DATA;
    }

    return ERR_OK;
}

bool CLICallbackStub::ReadToolEvent(MessageParcel& data, ToolEvent& event)
{
    if (!data.ReadString(event.type)) {
        return false;
    }
    if (!data.ReadString(event.data)) {
        return false;
    }
    if (!data.ReadInt64(event.timestamp)) {
        return false;
    }
    if (event.type == "exit") {
        if (!data.ReadInt32(event.exitCode)) {
            return false;
        }
    }

    return true;
}

bool CLICallbackStub::WriteToolEvent(MessageParcel& parcel, const ToolEvent& event)
{
    if (!parcel.WriteString(event.type)) {
        return false;
    }
    if (!parcel.WriteString(event.data)) {
        return false;
    }
    if (!parcel.WriteInt64(event.timestamp)) {
        return false;
    }
    if (event.type == "exit") {
        if (!parcel.WriteInt32(event.exitCode)) {
            return false;
        }
    }

    return true;
}

void CLICallbackStub::SetEventCallback(EventCallback callback)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    eventCallback_ = std::move(callback);
}

void CLICallbackStub::SetExitCallback(ExitCallback callback)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    exitCallback_ = std::move(callback);
}

void CLICallbackStub::SetErrorCallback(ErrorCallback callback)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    errorCallback_ = std::move(callback);
}

} // namespace CliMgr
} // namespace OHOS
