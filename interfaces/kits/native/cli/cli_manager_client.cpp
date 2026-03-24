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

#include "cli_manager_client.h"
#include "cli_manager_proxy.h"
#include "cli_callback_stub.h"
#include "system_ability_definition.h"
#include "isystem_ability_manager.h"
#include "hilog_tag_wrapper.h"
#include <chrono>

namespace OHOS {
namespace CliMgr {

// 单例实现
CLIManagerClient::CLIManagerClientImpl* CLIManagerClient::instance_ = nullptr;

/**
 * @brief CLI Manager Client 实现类
 */
class CLIManagerClient::CLIManagerClientImpl {
public:
    CLIManagerClientImpl() = default;
    ~CLIManagerClientImpl() = default;

    sptr<ICLIManagerService> GetServiceProxy()
    {
        // 使用单例模式缓存代理
        if (serviceProxy_ != nullptr) {
            return serviceProxy_;
        }

        // 连接服务
        if (ConnectService() != ErrCode::OK) {
            return nullptr;
        }

        return serviceProxy_;
    }

    int32_t ConnectService()
    {
        // 获取 SystemAbility Manager
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            TAG_LOGE("CLIManagerClient", "Failed to get SystemAbilityManager");
            return ErrCode::ERR_SERVICE_NOT_READY;
        }

        // 获取 CLI Manager Service (SA ID: 9001)
        auto object = samgr->GetSystemAbility(SA_CLIMGR_ID);
        if (object == nullptr) {
            TAG_LOGE("CLIManagerClient", "Failed to get CLI Manager Service");
            return ErrCode::ERR_SERVICE_NOT_READY;
        }

        // 创建代理
        serviceProxy_ = iface_cast<ICLIManagerService>(object);
        if (serviceProxy_ == nullptr) {
            TAG_LOGE("CLIManagerClient", "Failed to cast service proxy");
            return ErrCode::ERR_IPC_ERROR;
        }

        TAG_LOGI("CLIManagerClient", "Connected to CLI Manager Service");
        return ErrCode::OK;
    }

private:
    sptr<ICLIManagerService> serviceProxy_;
    std::mutex proxyMutex_;
};

int32_t CLIManagerClient::QueryTools(std::vector<ToolInfo>& tools)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return proxy->QueryTools(tools);
}

int32_t CLIManagerClient::QueryToolByName(const std::string& name, ToolInfo& tool)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return proxy->QueryToolByName(name, tool);
}

int32_t CLIManagerClient::QueryToolsByPermission(const std::string& permission,
                                                 std::vector<ToolInfo>& tools)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    // 服务端没有直接提供按权限查询的接口
    // 这里先查询所有工具，然后过滤
    std::vector<ToolInfo> allTools;
    int32_t ret = proxy->QueryTools(allTools);
    if (ret != ErrCode::OK) {
        return ret;
    }

    tools.clear();
    for (const auto& tool : allTools) {
        for (const auto& perm : tool.permissions) {
            if (perm == permission) {
                tools.push_back(tool);
                break;
            }
        }
    }

    return ErrCode::OK;
}

int32_t CLIManagerClient::ExecTool(
    const std::string& name,
    const Json::Value& arguments,
    ExecutionResult& result,
    const ExecOptions& options)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return proxy->ExecTool(name, arguments, options, result);
}

int32_t CLIManagerClient::ExecToolAsync(
    const std::string& name,
    const Json::Value& arguments,
    EventCallback callback,
    std::string& taskId,
    const ExecOptions& options)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    // 创建回调 Stub
    sptr<CLICallbackStub> stub = new (std::nothrow) CLICallbackStub();
    if (stub == nullptr) {
        TAG_LOGE("CLIManagerClient", "Failed to create callback stub");
        return ErrCode::ERR_NO_MEMORY;
    }

    // 设置回调函数
    stub->SetEventCallback(callback);
    stub->SetExitCallback(callback);
    stub->SetErrorCallback([callback](const std::string& taskId, int32_t errorCode, const std::string& errorMsg) {
        // 构造错误事件
        ToolEvent event;
        event.type = "error";
        event.data = errorMsg;
        event.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        callback(taskId, event);
    });

    return proxy->ExecToolAsync(name, arguments, stub, options, taskId);
}

int32_t CLIManagerClient::CancelTask(const std::string& taskId)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return proxy->CancelTask(taskId);
}

int32_t CLIManagerClient::QueryTaskStatus(const std::string& taskId, TaskStatus& status)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return proxy->QueryTaskStatus(taskId, status);
}

sptr<ICLIManagerService> CLIManagerClient::GetServiceProxy()
{
    if (instance_ == nullptr) {
        instance_ = new CLIManagerClientImpl();
    }

    return instance_->GetServiceProxy();
}

int32_t CLIManagerClient::ConnectService()
{
    if (instance_ == nullptr) {
        instance_ = new CLIManagerClientImpl();
    }

    return instance_->ConnectService();
}

} // namespace CliMgr
} // namespace OHOS
