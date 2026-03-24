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

#include "cli_callback_proxy.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliMgr {

// 消息 ID 定义
enum {
    ON_OUTPUT = 0,
    ON_EXIT = 1,
    ON_ERROR = 2,
};

CLICallbackProxy::CLICallbackProxy(const sptr<IRemoteObject>& object)
    : IRemoteProxy<ICLICallback>(object)
{
    TAG_LOGI(TAG, "CLICallbackProxy created");
}

void CLICallbackProxy::OnOutput(const std::string& taskId, const ToolEvent& event)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    // 写入接口 Token
    if (!data.WriteInterfaceToken()) {
        TAG_LOGE(TAG, "Failed to write interface token");
        return;
    }

    // 写入参数
    if (!data.WriteString(taskId)) {
        TAG_LOGE(TAG, "Failed to write taskId");
        return;
    }

    if (!WriteToolEvent(data, event)) {
        TAG_LOGE(TAG, "Failed to write tool event");
        return;
    }

    // 发送请求（异步调用，不等待回复）
    option.SetFlags(MessageOption::TF_ASYNC);
    int32_t ret = Remote()->SendRequest(ON_OUTPUT, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE(TAG, "SendRequest failed: %{public}d", ret);
        return;
    }

    TAG_LOGD(TAG, "OnOutput sent: task=%{public}s, type=%{public}s",
             taskId.c_str(), event.type.c_str());
}

void CLICallbackProxy::OnExit(const std::string& taskId, int32_t exitCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    // 写入接口 Token
    if (!data.WriteInterfaceToken()) {
        TAG_LOGE(TAG, "Failed to write interface token");
        return;
    }

    // 写入参数
    if (!data.WriteString(taskId)) {
        TAG_LOGE(TAG, "Failed to write taskId");
        return;
    }

    if (!data.WriteInt32(exitCode)) {
        TAG_LOGE(TAG, "Failed to write exitCode");
        return;
    }

    // 发送请求（异步调用）
    option.SetFlags(MessageOption::TF_ASYNC);
    int32_t ret = Remote()->SendRequest(ON_EXIT, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE(TAG, "SendRequest failed: %{public}d", ret);
        return;
    }

    TAG_LOGI(TAG, "OnExit sent: task=%{public}s, code=%{public}d",
             taskId.c_str(), exitCode);
}

void CLICallbackProxy::OnError(const std::string& taskId,
                               int32_t errorCode,
                               const std::string& errorMsg)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    // 写入接口 Token
    if (!data.WriteInterfaceToken()) {
        TAG_LOGE(TAG, "Failed to write interface token");
        return;
    }

    // 写入参数
    if (!data.WriteString(taskId)) {
        TAG_LOGE(TAG, "Failed to write taskId");
        return;
    }

    if (!data.WriteInt32(errorCode)) {
        TAG_LOGE(TAG, "Failed to write errorCode");
        return;
    }

    if (!data.WriteString(errorMsg)) {
        TAG_LOGE(TAG, "Failed to write errorMsg");
        return;
    }

    // 发送请求（异步调用）
    option.SetFlags(MessageOption::TF_ASYNC);
    int32_t ret = Remote()->SendRequest(ON_ERROR, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE(TAG, "SendRequest failed: %{public}d", ret);
        return;
    }

    TAG_LOGE(TAG, "OnError sent: task=%{public}s, code=%{public}d, msg=%{public}s",
             taskId.c_str(), errorCode, errorMsg.c_str());
}

bool CLICallbackProxy::WriteToolEvent(MessageParcel& parcel, const ToolEvent& event)
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

} // namespace CliMgr
} // namespace OHOS
