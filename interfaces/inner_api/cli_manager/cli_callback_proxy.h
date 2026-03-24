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

#ifndef CLI_CALLBACK_PROXY_H
#define CLI_CALLBACK_PROXY_H

#include "cli_callback.h"
#include "iremote_proxy.h"
#include "message_parcel.h"

namespace OHOS {
namespace CliMgr {

/**
 * @brief CLI Callback Proxy (服务端)
 *
 * 服务端用于调用客户端回调对象的代理类
 */
class CLICallbackProxy : public IRemoteProxy<ICLICallback> {
public:
    explicit CLICallbackProxy(const sptr<IRemoteObject>& object);
    ~CLICallbackProxy() override = default;

    /**
     * @brief 发送输出事件到客户端
     * @param taskId 任务 ID
     * @param event 事件数据
     */
    void OnOutput(const std::string& taskId, const ToolEvent& event) override;

    /**
     * @brief 发送退出事件到客户端
     * @param taskId 任务 ID
     * @param exitCode 退出码
     */
    void OnExit(const std::string& taskId, int32_t exitCode) override;

    /**
     * @brief 发送错误事件到客户端
     * @param taskId 任务 ID
     * @param errorCode 错误码
     * @param errorMsg 错误消息
     */
    void OnError(const std::string& taskId,
                int32_t errorCode,
                const std::string& errorMsg) override;

private:
    // 写入 ToolEvent 到 MessageParcel
    bool WriteToolEvent(MessageParcel& parcel, const ToolEvent& event);

    static constexpr const char* TAG = "CLICallbackProxy";
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_CALLBACK_PROXY_H
