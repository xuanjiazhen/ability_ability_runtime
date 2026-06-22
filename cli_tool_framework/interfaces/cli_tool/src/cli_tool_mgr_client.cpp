/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "cli_tool_mgr_client.h"

#include "cli_error_code.h"
#include "cli_event_reply_manager.h"
#include "cli_mgr_load_callback.h"
#include "cli_session_subscription_manager.h"
#include "cli_tool_mgr_scheduler_recipient.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t ERR_OK = 0;
} // namespace

// 获取当前进程内访问 CLI tool manager 服务的单例客户端。
CliToolMGRClient& CliToolMGRClient::GetInstance()
{
    static CliToolMGRClient instance;
    return instance;
}
// 这个函数做的不是“直接执行工具并拿结果”，而是：创建回调通道，注册 eventId 对应的本地 callback，把请求发给服务端，然后等待服务端通过 schedulerStub_ 异步回调回来。
ErrCode CliToolMGRClient::ExecTool(const ExecToolParam &param, const ExecToolReplyCallback &callback)
{
    // 1. 确保客户端已经创建 schedulerStub_。
    // schedulerStub_ 是客户端提供给服务端的“反向回调通道”。
    ErrCode ret = EnsureSchedulerStubCreated();

    // 2. 如果 schedulerStub_ 创建失败，说明后续服务端无法回调客户端，直接返回错误。
    if (ret != ERR_OK) {
        return ret;
    }

    // 3. 获取 climgr 服务端的代理对象。
    // proxy 可以理解为“客户端手里的服务端遥控器”。
    auto proxy = GetCliToolMgrProxy();

    // 4. 如果 proxy 为空，说明没有连上 cli tool manager 服务。
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }

    // 5. 向 CliEventReplyManager 注册一个一次性回调。
    // 它会生成一个 eventId，并保存映射关系：
    //
    // eventId -> 下面这个 lambda 回调
    //
    // 后面服务端执行完，会带着同一个 eventId 回调回来，
    // 客户端就能用 eventId 找到这次请求对应的 callback。
    std::string eventId = CliEventReplyManager::GetInstance().AddEventReplyCallback(param.toolName,
        [cb = std::move(callback)](const CliEventReplyResult &result) {
            // 6. cb 就是上层传进来的 ExecToolReplyCallback。
            // 它最终会继续通知 JS NAPI 层，让 Promise resolve/reject。
            if (cb) {
                // 7. 默认构造一个 sessionInfo。
                CliSessionInfo sessionInfo;

                // 8. 如果服务端回调结果里带了 sessionInfo，就取出来。
                // execTool 成功时通常会带 session 信息，比如 sessionId、状态等。
                if (result.sessionInfo.has_value()) {
                    sessionInfo = result.sessionInfo.value();
                }

                // 9. 执行真正的上层回调。
                // result.code 是服务端返回的执行结果码。
                // sessionInfo 是服务端创建出来的会话信息。
                cb(result.code, sessionInfo);
            }
        });

    // 10. 通过 Binder IPC 调用服务端 ExecTool。
    // 传过去三个核心东西：
    // param：本次要执行的工具参数
    // eventId：本次请求的唯一编号
    // schedulerStub_：客户端回调通道
    //
    // 注意：这里返回只表示“请求发给服务端并且服务端入口处理返回了”，
    // 不代表工具已经完整执行结束。
    ret = proxy->ExecTool(param, eventId, schedulerStub_);

    // 11. 如果 IPC 调用或服务端入口处理失败，
    // 说明这次 eventId 不会再有正常回调了，所以删除本地保存的 callback。
    if (ret != ERR_OK) {
        CliEventReplyManager::GetInstance().RemoveEventReplyCallback(eventId);

    // 12. 如果请求成功发出，就激活这个 eventId 对应的 callback。
    // 这里是为了处理竞态：
    // 有可能服务端回调回来得很快，甚至早于 ActivateEventReplyCallback。
    // 所以 manager 里有“先缓存结果，再激活后执行”的机制。
    } else {
        CliEventReplyManager::GetInstance().ActivateEventReplyCallback(eventId);
    }

    // 13. 返回本次发送请求的结果。
    // 注意它不是工具执行最终结果，最终结果会走 callback。
    return ret;
}

// 向 climgr 发送 execCmd 请求，注册结果回调，并准备会话事件订阅。
ErrCode CliToolMGRClient::ExecCmd(const ExecCmdParam &param,
    const ExecToolReplyCallback &callback, const std::shared_ptr<SessionEventCallback> &sessionEventCallback)
{
    ErrCode ret = EnsureSchedulerStubCreated();
    if (ret != ERR_OK) {
        return ret;
    }

    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }

    std::string eventId = CliEventReplyManager::GetInstance().AddEventReplyCallback("shell",
        [cb = std::move(callback)](const CliEventReplyResult &result) {
            if (cb) {
                CliSessionInfo sessionInfo;
                if (result.sessionInfo.has_value()) {
                    sessionInfo = result.sessionInfo.value();
                }
                cb(result.code, sessionInfo);
            }
        });
    
    std::string subscriptionId = CliSessionSubscriptionManager::GetInstance().AddProvisionalSubscription("shell",
        [sessionEventCallback](const std::string &sessionId,
            const std::string &subscriptionId, const CliToolEvent &event) {
            if (sessionEventCallback) {
                sessionEventCallback->OnToolEvent(sessionId, subscriptionId, event);
            }
        });
    ret = proxy->ExecCmd(param, eventId, schedulerStub_, subscriptionId);
    if (ret != ERR_OK) {
        CliEventReplyManager::GetInstance().RemoveEventReplyCallback(eventId);
        CliSessionSubscriptionManager::GetInstance().RemoveSubscription(subscriptionId);
    } else {
        CliEventReplyManager::GetInstance().ActivateEventReplyCallback(eventId);
        CliSessionSubscriptionManager::GetInstance().ActivateSubscription(subscriptionId);
    }
    return ret;
}

// 创建本进程的 scheduler stub，供 climgr 回调执行结果和会话事件。
ErrCode CliToolMGRClient::EnsureSchedulerStubCreated()
{
    std::lock_guard<std::mutex> lock(schedulerMutex_);
    if (schedulerStub_ != nullptr) {
        return ERR_OK;
    }
    schedulerStub_ = sptr<CliToolManagerSchedulerRecipient>::MakeSptr();
    if (schedulerStub_ == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "EnsureSchedulerStubCreated failed");
        return ERR_NO_INIT;
    }
    return ERR_OK;
}

// 从远端 manager 服务查询轻量级 CLI 工具摘要。
ErrCode CliToolMGRClient::GetAllToolSummaries(std::vector<ToolSummary> &summaries)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->GetAllToolSummaries(summaries);
}

// 按工具名称从远端 manager 服务查询完整 CLI 工具定义。
ErrCode CliToolMGRClient::GetToolInfoByName(const std::string &name, ToolInfo &tool)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->GetToolInfoByName(name, tool);
}

// 查询全部 CLI 工具定义，并将远端返回的 raw data 转换成 ToolInfo 列表。
ErrCode CliToolMGRClient::GetAllToolInfos(std::vector<ToolInfo> &tools)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    ToolsRawData rawData;
    auto ret = proxy->GetAllToolInfos(rawData);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolInfos failed: %{public}d", ret);
        return ret;
    }
    return ToolsRawData::ToToolInfoVec(rawData, tools);
}

// 将工具注册请求转发给远端 manager 服务。
ErrCode CliToolMGRClient::RegisterTool(const ToolInfo &tool)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->RegisterTool(tool);
}

// 将 function 注册请求转发给远端 manager 服务。
ErrCode CliToolMGRClient::RegisterFunction(const FunctionInfo &function)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->RegisterFunction(function);
}

// 从远端 manager 服务查询单个已注册 function 定义。
ErrCode CliToolMGRClient::GetFunctionInfo(const std::string &funcNamespace, const std::string &functionName,
    FunctionInfo &function)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->GetFunctionInfo(funcNamespace, functionName, function);
}

// 将 function 反注册请求转发给远端 manager 服务。
ErrCode CliToolMGRClient::UnregisterFunction(const std::string &funcNamespace, const std::string &functionName)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->UnregisterFunction(funcNamespace, functionName);
}

// 将按命名空间反注册 intent function 的请求转发给远端 manager 服务。
ErrCode CliToolMGRClient::UnregisterIntentFunctionsByNamespace(const std::string &funcNamespace)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->UnregisterIntentFunctionsByNamespace(funcNamespace);
}

// 从远端 manager 服务查询全部已注册 function 定义。
ErrCode CliToolMGRClient::GetAllFunctions(std::vector<FunctionInfo> &functions)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->GetAllFunctions(functions);
}

// 将批量子命令权限查询请求转发给远端 manager 服务。
int32_t CliToolMGRClient::BatchQueryPermissionBySubCommand(
    const std::vector<Command> &cmds,
    std::vector<CommandPermission> &cmdPermissions)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "proxy is null");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->BatchQueryPermissionBySubCommand(cmds, cmdPermissions);
}

// 处理 climgr 按需加载成功回调，并唤醒等待加载结果的客户端线程。
void CliToolMGRClient::OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    SetCliToolMgr(remoteObject);
    std::unique_lock<std::mutex> lock(loadSaMutex_);
    loadSaFinished_ = true;
    loadSaCondation_.notify_one();
}

// 处理 climgr 按需加载失败回调，并唤醒等待加载结果的客户端线程。
void CliToolMGRClient::OnLoadSystemAbilityFail()
{
    SetCliToolMgr(nullptr);
    std::unique_lock<std::mutex> lock(loadSaMutex_);
    loadSaFinished_ = true;
    loadSaCondation_.notify_one();
}

// 返回缓存的 climgr proxy；没有缓存时加载 SystemAbility 并注册死亡回调。
sptr<ICliToolManager> CliToolMGRClient::GetCliToolMgrProxy()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CLI_TOOL, "called");
    auto cliToolMgr = GetCliToolMgr();
    if (cliToolMgr != nullptr) {
        TAG_LOGD(AAFwkTag::CLI_TOOL, "Cli tool manager has been started");
        return cliToolMgr;
    }

    if (!LoadCliToolMgrService()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Load cli tool manager service failed");
        return nullptr;
    }

    cliToolMgr = GetCliToolMgr();
    if (cliToolMgr == nullptr || cliToolMgr->AsObject() == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get cli tool manager");
        return nullptr;
    }

    const auto &onClearProxyCallback = [](const wptr<IRemoteObject> &remote) {
        auto &instance = GetInstance();
        auto cliToolMgr = instance.GetCliToolMgr();
        if (cliToolMgr != nullptr && cliToolMgr->AsObject() == remote) {
            instance.ClearProxy();
        }
    };

    sptr<CliMgrDeathRecipient> recipient(new (std::nothrow) CliMgrDeathRecipient(onClearProxyCallback));
    cliToolMgr->AsObject()->AddDeathRecipient(recipient);

    return cliToolMgr;
}

// 按需加载 CLI tool manager SystemAbility，并等待加载回调返回。
bool CliToolMGRClient::LoadCliToolMgrService()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    const int32_t LOAD_SA_TIMEOUT_MS = 4 * 1000;
    {
        std::unique_lock<std::mutex> lock(loadSaMutex_);
        loadSaFinished_ = false;
    }

    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "GetSystemAbilityManager");
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get SystemAbilityManager");
        return false;
    }

    sptr<CliMgrLoadCallback> loadCallback = sptr<CliMgrLoadCallback>::MakeSptr();
    if (loadCallback == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Create load callback failed");
        return false;
    }

    auto ret = systemAbilityMgr->LoadSystemAbility(CLI_TOOL_MGR_SERVICE_ID, loadCallback);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Load system ability %{public}d failed with %{public}d", CLI_TOOL_MGR_SERVICE_ID,
            ret);
        return false;
    }

    {
        std::unique_lock<std::mutex> lock(loadSaMutex_);
        auto waitStatus = loadSaCondation_.wait_for(lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
            [this]() {
                return loadSaFinished_;
            });
        if (!waitStatus) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Wait for load sa timeout");
            return false;
        }
    }

    return true;
}

// 将远端 climgr 对象保存为类型化 manager proxy。
void CliToolMGRClient::SetCliToolMgr(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    cliToolMgr_ = iface_cast<ICliToolManager>(remoteObject);
}

// 返回当前缓存的 CLI tool manager proxy。
sptr<ICliToolManager> CliToolMGRClient::GetCliToolMgr()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    return cliToolMgr_;
}

// 清理缓存的 proxy，并丢弃待处理的事件回复和会话订阅。
void CliToolMGRClient::ClearProxy()
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "called");
    std::lock_guard<std::mutex> lock(proxyMutex_);
    cliToolMgr_ = nullptr;
    CliEventReplyManager::GetInstance().ClearAllEvent();
    CliSessionSubscriptionManager::GetInstance().ClearAllSubscriptions();
}

// 在 climgr 进程死亡时触发已注册的 proxy 清理回调。
void CliToolMGRClient::CliMgrDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (callback_ != nullptr) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "cli tool manager service died");
        callback_(remote);
    }
}

// 订阅已有会话事件，并在远端订阅成功后激活本地回调。
ErrCode CliToolMGRClient::SubscribeSession(const std::string &sessionId,
                                           const std::shared_ptr<SessionEventCallback> &callback,
                                           std::string &subscriptionId)
{
    ErrCode ret = EnsureSchedulerStubCreated();
    if (ret != ERR_OK) {
        return ret;
    }

    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }

    subscriptionId = CliSessionSubscriptionManager::GetInstance().AddProvisionalSubscription(sessionId,
        [callback](const std::string &sessionId,
            const std::string &subscriptionId, const CliToolEvent &event) {
            if (callback) {
                callback->OnToolEvent(sessionId, subscriptionId, event);
            }
        });
    ret = proxy->SubscribeSession(sessionId, subscriptionId, schedulerStub_);
    if (ret != ERR_OK) {
        CliSessionSubscriptionManager::GetInstance().RemoveSubscription(subscriptionId);
        subscriptionId = "";
    } else {
        CliSessionSubscriptionManager::GetInstance().ActivateSubscription(subscriptionId);
    }
    return ret;
}

// 移除本地会话订阅，并将取消订阅请求转发给 climgr。
ErrCode CliToolMGRClient::UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    CliSessionSubscriptionManager::GetInstance().RemoveSubscription(subscriptionId);

    return proxy->UnsubscribeSession(sessionId, subscriptionId);
}

// 请求 climgr 取消并清理一个运行中的会话。
ErrCode CliToolMGRClient::ClearSession(const std::string &sessionId)
{
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->ClearSession(sessionId);
}

// 从 climgr 查询当前会话信息。
ErrCode CliToolMGRClient::QuerySession(const std::string &sessionId, CliSessionInfo &session)
{
    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }
    return proxy->QuerySession(sessionId, session);
}

// 向运行中的会话发送 stdin 文本，并注册一次性发送结果回调。
ErrCode CliToolMGRClient::SendMessage(const std::string &sessionId, const std::string &inputText,
    const EventReplyCallback &callback)
{
    ErrCode ret = EnsureSchedulerStubCreated();
    if (ret != ERR_OK) {
        return ret;
    }

    auto proxy = GetCliToolMgrProxy();
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "connect failed");
        return GET_CLI_TOOL_MGR_SERVICE_FAILED;
    }

    std::string eventId = CliEventReplyManager::GetInstance().AddEventReplyCallback(sessionId,
        [cb = std::move(callback)](const CliEventReplyResult &result) {
            if (cb) {
                cb(result.code);
            }
        });
    ret = proxy->SendMessage(sessionId, inputText, eventId, schedulerStub_);
    if (ret != ERR_OK) {
        CliEventReplyManager::GetInstance().RemoveEventReplyCallback(eventId);
    } else {
        CliEventReplyManager::GetInstance().ActivateEventReplyCallback(eventId);
    }
    return ret;
}
} // namespace CliTool
} // namespace OHOS
