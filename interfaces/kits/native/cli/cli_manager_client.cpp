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
#include <map>
#include <mutex>

namespace OHOS {
namespace CliMgr {

// 单例实现
CLIManagerClient::CLIManagerClientImpl* CLIManagerClient::instance_ = nullptr;

// 会话回调管理器
class SessionCallbackManager {
public:
    static SessionCallbackManager& GetInstance() {
        static SessionCallbackManager instance;
        return instance;
    }

    void RegisterCallback(const std::string& sessionId, CLIManagerClient::EventCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_[sessionId] = callback;
    }

    void UnregisterCallback(const std::string& sessionId) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_.erase(sessionId);
    }

    void TriggerCallback(const std::string& sessionId, const ToolEvent& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = callbacks_.find(sessionId);
        if (it != callbacks_.end() && it->second) {
            it->second(sessionId, event);
        }
    }

private:
    std::map<std::string, CLIManagerClient::EventCallback> callbacks_;
    std::mutex mutex_;
};

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

int32_t CLIManagerClient::QueryToolSummaries(std::vector<ToolSummary>& summaries)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        TAG_LOGE("CLIManagerClient", "Failed to get service proxy");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return proxy->QueryToolSummaries(summaries);
}

int32_t CLIManagerClient::GetToolInfoByName(const std::string& name, ToolInfo& tool)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        TAG_LOGE("CLIManagerClient", "Failed to get service proxy");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return proxy->GetToolInfoByName(name, tool);
}

int32_t CLIManagerClient::ExecTool(
    const std::string& name,
    const Json::Value& arguments,
    SessionInfo& session,
    const ExecOptions& options)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return proxy->ExecTool(name, arguments, options, session);
}

int32_t CLIManagerClient::SubscribeSession(
    const std::string& sessionId,
    EventCallback callback)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    // 注册回调到管理器
    SessionCallbackManager::GetInstance().RegisterCallback(sessionId, callback);

    // 创建 IPC 回调 Stub
    sptr<CLICallbackStub> stub = new (std::nothrow) CLICallbackStub();
    if (stub == nullptr) {
        TAG_LOGE("CLIManagerClient", "Failed to create callback stub");
        return ErrCode::ERR_NO_MEMORY;
    }

    // 设置回调函数，将 IPC 回调转发到管理器
    stub->SetEventCallback([sessionId](const std::string& taskId, const ToolEvent& event) {
        SessionCallbackManager::GetInstance().TriggerCallback(sessionId, event);
    });

    stub->SetExitCallback([sessionId](const std::string& taskId, int32_t exitCode) {
        ToolEvent event;
        event.type = "exit";
        event.data = "";
        event.exitCode = exitCode;
        event.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        SessionCallbackManager::GetInstance().TriggerCallback(sessionId, event);
    });

    stub->SetErrorCallback([sessionId](const std::string& taskId, int32_t errorCode, const std::string& errorMsg) {
        ToolEvent event;
        event.type = "error";
        event.data = errorMsg;
        event.timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        SessionCallbackManager::GetInstance().TriggerCallback(sessionId, event);
    });

    return proxy->SubscribeSession(sessionId, stub);
}

int32_t CLIManagerClient::CancelSession(const std::string& sessionId)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    // 取消订阅时也要移除回调
    SessionCallbackManager::GetInstance().UnregisterCallback(sessionId);

    return proxy->CancelSession(sessionId);
}

int32_t CLIManagerClient::QuerySessionStatus(const std::string& sessionId, SessionInfo& status)
{
    auto proxy = GetServiceProxy();
    if (proxy == nullptr) {
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return proxy->QuerySessionStatus(sessionId, status);
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
