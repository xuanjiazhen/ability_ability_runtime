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

#ifndef CLI_MANAGER_SERVICE_H
#define CLI_MANAGER_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "system_ability.h"
#include "iremote_stub.h"
#include "cli_manager_constants.h"
#include "cli_manager_service_interface.h"
#include "cli_tool_info.h"
#include "cli_execution_context.h"

namespace OHOS {
namespace CliMgr {

// 前向声明
class RegistryManager;
class ProcessManager;
class EventDispatcher;
class PermissionChecker;
class IOMonitor;
class ProcessMonitor;
class TimeoutManager;
class ResourceManager;

/**
 * @brief CLI Manager Service
 *
 * SystemAbility 实现，SA ID: 9001
 */
class CLIManagerService : public SystemAbility,
                          public IRemoteStub<ICLIManagerService> {
public:
    DECLARE_SYSTEM_ABILITY(CLIManagerService);

    explicit CLIManagerService(int32_t systemAbilityId,
                              bool runOnCreate = true);
    ~CLIManagerService() override;

    // SystemAbility 生命周期
    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId,
                           const std::string& deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId,
                              const std::string& deviceId) override;

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

    // IPC 服务端实现
    int32_t OnRemoteRequest(uint32_t code,
                           MessageParcel& data,
                           MessageParcel& reply,
                           MessageOption& option) override;

    // 获取子管理器
    std::shared_ptr<RegistryManager> GetRegistryManager() const;
    std::shared_ptr<ProcessManager> GetProcessManager() const;
    std::shared_ptr<EventDispatcher> GetEventDispatcher() const;
    std::shared_ptr<PermissionChecker> GetPermissionChecker() const;
    std::shared_ptr<ProcessMonitor> GetProcessMonitor() const;
    std::shared_ptr<TimeoutManager> GetTimeoutManager() const;
    std::shared_ptr<ResourceManager> GetResourceManager() const;

private:
    // 初始化组件
    bool InitComponents();

    // 生成任务 ID
    std::string GenerateTaskId();

    // 验证工具参数
    int32_t ValidateToolArguments(const ToolInfo& tool,
                                  const Json::Value& arguments);

    // 任务管理
    void AddTask(const std::string& taskId,
                std::shared_ptr<ExecutionContext> context);
    void RemoveTask(const std::string& taskId);
    std::shared_ptr<ExecutionContext> GetTask(const std::string& taskId);

private:
    // RPC 处理方法
    int32_t QueryToolsInternal(MessageParcel& data, MessageParcel& reply);
    int32_t QueryToolByNameInternal(MessageParcel& data, MessageParcel& reply);
    int32_t ExecToolInternal(MessageParcel& data, MessageParcel& reply);
    int32_t ExecToolAsyncInternal(MessageParcel& data, MessageParcel& reply);
    int32_t CancelTaskInternal(MessageParcel& data, MessageParcel& reply);
    int32_t QueryTaskStatusInternal(MessageParcel& data, MessageParcel& reply);

    // 辅助方法
    bool WriteToolInfo(MessageParcel& parcel, const ToolInfo& tool);

private:
    // 子管理器
    std::shared_ptr<RegistryManager> registryManager_;
    std::shared_ptr<ProcessManager> processManager_;
    std::shared_ptr<EventDispatcher> eventDispatcher_;
    std::shared_ptr<PermissionChecker> permissionChecker_;
    std::shared_ptr<IOMonitor> ioMonitor_;
    std::shared_ptr<ProcessMonitor> processMonitor_;
    std::shared_ptr<TimeoutManager> timeoutManager_;
    std::shared_ptr<ResourceManager> resourceManager_;

    // 任务管理
    std::unordered_map<std::string, std::shared_ptr<ExecutionContext>> tasks_;
    std::mutex tasksMutex_;

    // 任务 ID 计数器
    std::atomic<uint64_t> taskIdCounter_{0};

    // 服务状态
    std::atomic<bool> running_{false};

    // HiLog 标签
    static constexpr const char* TAG = "CLIManagerService";
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_MANAGER_SERVICE_H
