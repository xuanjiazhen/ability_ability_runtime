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

#include "cli_tool_registry.h"
#include "cli_manager_service_interface.h"
#include "cli_manager_proxy.h"
#include "cli_manager_error.h"
#include "system_ability_definition.h"
#include "isystem_ability_manager.h"
#include "hilog/log.h"
#include <mutex>
#include <memory>

namespace OHOS {
namespace CliMgr {

namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel TAG = {LOG_CORE, 0, "CliToolRegistry"};
}

// ============================================================================
// CliToolRegistry::CliToolRegistryImpl - IPC 实现类
// ============================================================================

class CliToolRegistry::CliToolRegistryImpl {
public:
    CliToolRegistryImpl() : initialized_(false), connected_(false)
    {}

    ~CliToolRegistryImpl()
    {
        Disconnect();
    }

    /**
     * @brief 连接到 CLI Manager Service
     */
    bool Connect()
    {
        if (connected_) {
            HILOG_WARN(TAG, "Already connected to CLI Manager Service");
            return true;
        }

        // 获取服务
        auto samgr = SystemAbilityManagerClient::GetInstance();
        if (!samgr) {
            HILOG_ERROR(TAG, "Failed to get SystemAbilityManager");
            return false;
        }

        // 获取 CLI Manager Service 的远程对象
        remoteObject_ = samgr->GetSystemAbility(CLI_MANAGER_SERVICE_ID);
        if (!remoteObject_) {
            HILOG_ERROR(TAG, "Failed to get CLI Manager Service");
            return false;
        }

        // 创建代理
        serviceProxy_ = new CLIManagerProxy(remoteObject_);
        if (!serviceProxy_) {
            HILOG_ERROR(TAG, "Failed to create service proxy");
            return false;
        }

        connected_ = true;
        HILOG_INFO(TAG, "Connected to CLI Manager Service");
        return true;
    }

    /**
     * @brief 断开连接
     */
    void Disconnect()
    {
        if (!connected_) {
            return;
        }

        serviceProxy_ = nullptr;
        remoteObject_ = nullptr;
        connected_ = false;

        HILOG_INFO(TAG, "Disconnected from CLI Manager Service");
    }

    /**
     * @brief 检查连接状态
     */
    bool IsConnected() const
    {
        return connected_ && serviceProxy_ != nullptr;
    }

    /**
     * @brief 确保已连接
     */
    bool EnsureConnected()
    {
        if (!IsConnected()) {
            HILOG_ERROR(TAG, "Not connected to CLI Manager Service");
            return false;
        }
        return true;
    }

    /**
     * @brief 注册工具
     */
    int32_t RegisterTool(const ToolInfo& tool, bool persist)
    {
        if (!EnsureConnected()) {
            return ErrCode::ERR_SERVICE_NOT_READY;
        }

        return serviceProxy_->RegisterTool(tool, persist);
    }

    /**
     * @brief 批量注册工具
     */
    int32_t RegisterTools(const std::vector<ToolInfo>& tools, bool persist)
    {
        if (!EnsureConnected()) {
            return ErrCode::ERR_SERVICE_NOT_READY;
        }

        return serviceProxy_->RegisterTools(tools, persist);
    }

    /**
     * @brief 更新工具
     */
    int32_t UpdateTool(const ToolInfo& tool, bool persist)
    {
        if (!EnsureConnected()) {
            return ErrCode::ERR_SERVICE_NOT_READY;
        }

        return serviceProxy_->UpdateTool(tool, persist);
    }

    /**
     * @brief 注销工具
     */
    int32_t UnregisterTool(const std::string& name, bool persist)
    {
        if (!EnsureConnected()) {
            return ErrCode::ERR_SERVICE_NOT_READY;
        }

        return serviceProxy_->UnregisterTool(name, persist);
    }

    /**
     * @brief 检查工具是否已注册
     */
    int32_t IsToolRegistered(const std::string& name, bool& exists)
    {
        if (!EnsureConnected()) {
            return ErrCode::ERR_SERVICE_NOT_READY;
        }

        ToolInfo tool;
        int32_t ret = serviceProxy_->QueryTool(name, tool);
        exists = (ret == ErrCode::OK);
        return ErrCode::OK;
    }

    /**
     * @brief 查询工具
     */
    int32_t QueryTool(const std::string& toolName, ToolInfo& tool)
    {
        if (!EnsureConnected()) {
            return ErrCode::ERR_SERVICE_NOT_READY;
        }

        return serviceProxy_->QueryTool(toolName, tool);
    }

    /**
     * @brief 查询所有工具
     */
    int32_t QueryAllTools(std::vector<ToolInfo>& tools)
    {
        if (!EnsureConnected()) {
            return ErrCode::ERR_SERVICE_NOT_READY;
        }

        return serviceProxy_->QueryTools(tools);
    }

    /**
     * @brief 重新加载注册表
     */
    int32_t ReloadRegistry()
    {
        if (!EnsureConnected()) {
            return ErrCode::ERR_SERVICE_NOT_READY;
        }

        return serviceProxy_->ReloadRegistry();
    }

private:
    bool initialized_;
    bool connected_;
    sptr<IRemoteObject> remoteObject_;
    sptr<CLIManagerProxy> serviceProxy_;
};

// ============================================================================
// CliToolRegistry - 静态成员
// ============================================================================

std::mutex CliToolRegistry::instanceMutex_;
CliToolRegistry* CliToolRegistry::instance_ = nullptr;

// ============================================================================
// CliToolRegistry - 实现
// ============================================================================

CliToolRegistry* CliToolRegistry::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutex_);
    if (instance_ == nullptr) {
        instance_ = new CliToolRegistry();
    }
    return instance_;
}

CliToolRegistry::CliToolRegistry()
    : impl_(std::make_unique<CliToolRegistryImpl>())
{
}

CliToolRegistry::~CliToolRegistry()
{
    Release();
}

bool CliToolRegistry::Initialize()
{
    if (impl_->initialized_) {
        HILOG_WARN(TAG, "Already initialized");
        return true;
    }

    if (!impl_->Connect()) {
        HILOG_ERROR(TAG, "Failed to connect to CLI Manager Service");
        return false;
    }

    impl_->initialized_ = true;
    HILOG_INFO(TAG, "CliToolRegistry initialized successfully");
    return true;
}

void CliToolRegistry::Release()
{
    if (!impl_->initialized_) {
        return;
    }

    impl_->Disconnect();
    impl_->initialized_ = false;

    HILOG_INFO(TAG, "CliToolRegistry released");
}

int32_t CliToolRegistry::RegisterTool(const ToolInfo& tool, bool persist)
{
    if (!impl_->initialized_) {
        HILOG_ERROR(TAG, "Not initialized");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    // 基本验证
    if (tool.name.empty()) {
        HILOG_ERROR(TAG, "Tool name is empty");
        return ErrCode::ERR_INVALID_ARGS;
    }

    if (tool.executablePath.empty()) {
        HILOG_ERROR(TAG, "Executable path is empty");
        return ErrCode::ERR_INVALID_ARGS;
    }

    HILOG_INFO(TAG, "Registering tool: %{public}s (persist=%{public}d)",
              tool.name.c_str(), persist);

    int32_t ret = impl_->RegisterTool(tool, persist);
    if (ret == ErrCode::OK) {
        HILOG_INFO(TAG, "Tool registered successfully: %{public}s", tool.name.c_str());
    } else {
        HILOG_ERROR(TAG, "Failed to register tool: %{public}s, error=%{public}d",
                   tool.name.c_str(), ret);
    }

    return ret;
}

int32_t CliToolRegistry::RegisterTools(const std::vector<ToolInfo>& tools, bool persist)
{
    if (!impl_->initialized_) {
        HILOG_ERROR(TAG, "Not initialized");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    if (tools.empty()) {
        HILOG_WARN(TAG, "Tools list is empty");
        return ErrCode::ERR_INVALID_ARGS;
    }

    HILOG_INFO(TAG, "Registering %{public}zu tools (persist=%{public}d)",
              tools.size(), persist);

    int32_t ret = impl_->RegisterTools(tools, persist);
    if (ret == ErrCode::OK) {
        HILOG_INFO(TAG, "Tools registered successfully: %{public}zu tools", tools.size());
    } else {
        HILOG_ERROR(TAG, "Failed to register tools, error=%{public}d", ret);
    }

    return ret;
}

int32_t CliToolRegistry::UpdateTool(const ToolInfo& tool, bool persist)
{
    if (!impl_->initialized_) {
        HILOG_ERROR(TAG, "Not initialized");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    if (tool.name.empty()) {
        HILOG_ERROR(TAG, "Tool name is empty");
        return ErrCode::ERR_INVALID_ARGS;
    }

    HILOG_INFO(TAG, "Updating tool: %{public}s (persist=%{public}d)",
              tool.name.c_str(), persist);

    int32_t ret = impl_->UpdateTool(tool, persist);
    if (ret == ErrCode::OK) {
        HILOG_INFO(TAG, "Tool updated successfully: %{public}s", tool.name.c_str());
    } else {
        HILOG_ERROR(TAG, "Failed to update tool: %{public}s, error=%{public}d",
                   tool.name.c_str(), ret);
    }

    return ret;
}

int32_t CliToolRegistry::UnregisterTool(const std::string& name, bool persist)
{
    if (!impl_->initialized_) {
        HILOG_ERROR(TAG, "Not initialized");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    if (name.empty()) {
        HILOG_ERROR(TAG, "Tool name is empty");
        return ErrCode::ERR_INVALID_ARGS;
    }

    HILOG_INFO(TAG, "Unregistering tool: %{public}s (persist=%{public}d)",
              name.c_str(), persist);

    int32_t ret = impl_->UnregisterTool(name, persist);
    if (ret == ErrCode::OK) {
        HILOG_INFO(TAG, "Tool unregistered successfully: %{public}s", name.c_str());
    } else if (ret == ErrCode::ERR_NOT_FOUND) {
        HILOG_WARN(TAG, "Tool not found: %{public}s", name.c_str());
    } else {
        HILOG_ERROR(TAG, "Failed to unregister tool: %{public}s, error=%{public}d",
                   name.c_str(), ret);
    }

    return ret;
}

int32_t CliToolRegistry::IsToolRegistered(const std::string& name, bool& exists)
{
    if (!impl_->initialized_) {
        HILOG_ERROR(TAG, "Not initialized");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    if (name.empty()) {
        HILOG_ERROR(TAG, "Tool name is empty");
        return ErrCode::ERR_INVALID_ARGS;
    }

    return impl_->IsToolRegistered(name, exists);
}

int32_t CliToolRegistry::QueryTool(const std::string& toolName, ToolInfo& tool)
{
    if (!impl_->initialized_) {
        HILOG_ERROR(TAG, "Not initialized");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    if (toolName.empty()) {
        HILOG_ERROR(TAG, "Tool name is empty");
        return ErrCode::ERR_INVALID_ARGS;
    }

    return impl_->QueryTool(toolName, tool);
}

int32_t CliToolRegistry::QueryAllTools(std::vector<ToolInfo>& tools)
{
    if (!impl_->initialized_) {
        HILOG_ERROR(TAG, "Not initialized");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return impl_->QueryAllTools(tools);
}

int32_t CliToolRegistry::ReloadRegistry()
{
    if (!impl_->initialized_) {
        HILOG_ERROR(TAG, "Not initialized");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    HILOG_INFO(TAG, "Reloading registry");

    return impl_->ReloadRegistry();
}

int32_t CliToolRegistry::SetRegistryChangeListener(
    std::shared_ptr<RegistryChangeListener> callback)
{
    // TODO: 实现监听器设置
    HILOG_WARN(TAG, "Registry change listener not implemented yet");
    return ErrCode::OK;
}

} // namespace CliMgr
} // namespace OHOS
