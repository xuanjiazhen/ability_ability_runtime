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

#ifndef CLI_MANAGER_PROXY_H
#define CLI_MANAGER_PROXY_H

#include "cli_manager_service_interface.h"
#include "iremote_proxy.h"
#include "iremote_object.h"

namespace OHOS {
namespace CliMgr {

/**
 * @brief CLI Manager Service Proxy (客户端)
 *
 * 客户端使用的代理类，通过 IPC 与服务端通信
 */
class CLIManagerProxy : public IRemoteProxy<ICLIManagerService> {
public:
    explicit CLIManagerProxy(const sptr<IRemoteObject>& object);
    ~CLIManagerProxy() override = default;

    // ICLIManagerService 接口实现
    int32_t QueryTools(std::vector<ToolInfo>& tools) override;
    int32_t QueryToolByName(const std::string& name, ToolInfo& tool) override;
    int32_t ExecTool(
        const std::string& name,
        const Json::Value& arguments,
        const ExecOptions& options,
        ExecutionResult& result
    ) override;
    int32_t ExecToolAsync(
        const std::string& name,
        const Json::Value& arguments,
        const sptr<ICLICallback>& callback,
        const ExecOptions& options,
        std::string& taskId
    ) override;
    int32_t CancelTask(const std::string& taskId) override;
    int32_t QueryTaskStatus(const std::string& taskId, TaskStatus& status) override;

private:
    // 读取 ToolInfo 从 MessageParcel
    bool ReadToolInfo(MessageParcel& reply, ToolInfo& tool);

    // 写入 ToolInfo 到 MessageParcel
    bool WriteToolInfo(MessageParcel& parcel, const ToolInfo& tool);

    // 读取 ExecutionResult 从 MessageParcel
    bool ReadExecutionResult(MessageParcel& reply, ExecutionResult& result);

    // 写入 Json::Value 到 MessageParcel
    bool WriteJsonValue(MessageParcel& parcel, const Json::Value& value);

    // 读取 Json::Value 从 MessageParcel
    bool ReadJsonValue(MessageParcel& reply, Json::Value& value);

    static inline BrokerDelegator<CLIManagerProxy> delegator_;
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_MANAGER_PROXY_H
