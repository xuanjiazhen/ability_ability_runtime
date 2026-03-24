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

#ifndef CLI_CALLBACK_STUB_H
#define CLI_CALLBACK_STUB_H

#include "cli_callback.h"
#include "cli_execution_context.h"
#include "iremote_stub.h"
#include "message_parcel.h"
#include <functional>

namespace OHOS {
namespace CliMgr {

/**
 * @brief CLI Callback Stub (客户端)
 *
 * 客户端实现，接收服务端的回调调用并转发给应用层
 */
class CLICallbackStub : public IRemoteStub<ICLICallback> {
public:
    using EventCallback = std::function<void(const std::string&, const ToolEvent&)>;
    using ExitCallback = std::function<void(const std::string&, int32_t)>;
    using ErrorCallback = std::function<void(const std::string&, int32_t, const std::string&)>;

    explicit CLICallbackStub();
    ~CLICallbackStub() override = default;

    // ICLICallback 接口实现
    void OnOutput(const std::string& taskId, const ToolEvent& event) override;
    void OnExit(const std::string& taskId, int32_t exitCode) override;
    void OnError(const std::string& taskId,
                int32_t errorCode,
                const std::string& errorMsg) override;

    // IRemoteStub 接口
    int32_t OnRemoteRequest(uint32_t code,
                           MessageParcel& data,
                           MessageParcel& reply,
                           MessageOption& option) override;

    // 设置回调函数
    void SetEventCallback(EventCallback callback);
    void SetExitCallback(ExitCallback callback);
    void SetErrorCallback(ErrorCallback callback);

private:
    // 处理 OnOutput 请求
    int32_t HandleOnOutput(MessageParcel& data, MessageParcel& reply);

    // 处理 OnExit 请求
    int32_t HandleOnExit(MessageParcel& data, MessageParcel& reply);

    // 处理 OnError 请求
    int32_t HandleOnError(MessageParcel& data, MessageParcel& reply);

    // 读取 ToolEvent 从 MessageParcel
    bool ReadToolEvent(MessageParcel& data, ToolEvent& event);

    // 写入 ToolEvent 到 MessageParcel
    bool WriteToolEvent(MessageParcel& parcel, const ToolEvent& event);

    // 回调函数
    EventCallback eventCallback_;
    ExitCallback exitCallback_;
    ErrorCallback errorCallback_;
    std::mutex callbackMutex_;

    static constexpr const char* TAG = "CLICallbackStub";
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_CALLBACK_STUB_H
