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

#include "dstb_ipc_manager.h"

#include "device_manager.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "single_instance.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace CliTool {

namespace {
const std::string DSTB_PKG_NAME = "climgr_distributed";

// DeviceManager 初始化回调
class DstbDmInitCallback : public DistributedHardware::DmInitCallback {
public:
    void OnRemoteDied() override
    {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "DeviceManager remote died");
    }
};
} // namespace

IMPLEMENT_SINGLE_INSTANCE(DstbIpcManager);

DstbIpcManager::DstbIpcManager() {}

DstbIpcManager::~DstbIpcManager()
{
    UnInit();
}

void DstbIpcManager::Init()
{
    std::lock_guard<std::mutex> lock(initMutex_);
    if (initialized_) {
        return;
    }

    // 初始化 DeviceManager
    auto dmInitCallback = std::make_shared<DstbDmInitCallback>();
    auto ret = DistributedHardware::DeviceManager::GetInstance().
        InitDeviceManager(DSTB_PKG_NAME, dmInitCallback);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "InitDeviceManager failed: %{public}d", ret);
        // 继续尝试获取本地设备信息
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "DeviceManager initialized successfully");
    }

    // 获取本地设备信息
    DistributedHardware::DmDeviceInfo localInfo;
    ret = DistributedHardware::DeviceManager::GetInstance().GetLocalDeviceInfo(DSTB_PKG_NAME, localInfo);
    if (ret == 0) {
        localDeviceId_ = localInfo.networkId;
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Local device ID: %{public}s", localDeviceId_.substr(0, 8).c_str());
    } else {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to get local device info: %{public}d", ret);
    }

    initialized_ = true;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "DstbIpcManager initialized");
}

void DstbIpcManager::UnInit()
{
    std::lock_guard<std::mutex> lock(initMutex_);
    if (!initialized_) {
        return;
    }

    std::lock_guard<std::mutex> proxyLock(proxyMutex_);
    remoteProxies_.clear();
    localDeviceId_.clear();

    // 反初始化 DeviceManager
    auto ret = DistributedHardware::DeviceManager::GetInstance().UnInitDeviceManager(DSTB_PKG_NAME);
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "UnInitDeviceManager failed: %{public}d", ret);
    }

    initialized_ = false;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "DstbIpcManager uninitialized");
}

std::string DstbIpcManager::GetLocalDeviceId()
{
    std::lock_guard<std::mutex> lock(initMutex_);
    return localDeviceId_;
}

int32_t DstbIpcManager::GetAvailableDevices(std::vector<DstbDeviceInfo>& devices)
{
    std::vector<DistributedHardware::DmDeviceInfo> dmDeviceList;
    auto ret = DistributedHardware::DeviceManager::GetInstance().
        GetTrustedDeviceList(DSTB_PKG_NAME, "", dmDeviceList);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetTrustedDeviceList failed: %{public}d", ret);
        return DstbErrorCode::ERR_DEVICE_NOT_FOUND;
    }

    devices.clear();
    for (const auto& dmDevice : dmDeviceList) {
        DstbDeviceInfo info;
        info.deviceId = dmDevice.networkId;
        info.deviceName = dmDevice.deviceName;
        info.deviceType = dmDevice.deviceTypeId;
        info.deviceState = static_cast<int32_t>(DstbDeviceState::ONLINE);
        devices.push_back(info);
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Found %{public}zu online devices", devices.size());
    return DstbErrorCode::ERR_OK;
}

bool DstbIpcManager::IsDeviceOnline(const std::string& deviceId)
{
    std::vector<DstbDeviceInfo> devices;
    auto ret = GetAvailableDevices(devices);
    if (ret != DstbErrorCode::ERR_OK) {
        return false;
    }

    for (const auto& device : devices) {
        if (device.deviceId == deviceId) {
            return true;
        }
    }
    return false;
}

sptr<IRemoteObject> DstbIpcManager::GetRemoteCliToolMgrProxy(const std::string& targetDeviceId)
{
    // 检查缓存
    {
        std::lock_guard<std::mutex> lock(proxyMutex_);
        auto it = remoteProxies_.find(targetDeviceId);
        if (it != remoteProxies_.end() && it->second != nullptr) {
            return it->second;
        }
    }

    // 检查设备是否在线
    if (!IsDeviceOnline(targetDeviceId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Device %{public}s is offline", targetDeviceId.substr(0, 8).c_str());
        return nullptr;
    }

    // 通过 SAMgr 获取远端 SA 代理
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Get SAMGR failed");
        return nullptr;
    }

    // 使用 DBinder 获取远端 SA
    sptr<IRemoteObject> remote = samgr->CheckSystemAbility(CLI_TOOL_MGR_SERVICE_ID, targetDeviceId);
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Get remote SA %{public}d failed for device %{public}s",
            CLI_TOOL_MGR_SERVICE_ID, targetDeviceId.substr(0, 8).c_str());
        return nullptr;
    }

    // 缓存代理
    {
        std::lock_guard<std::mutex> lock(proxyMutex_);
        remoteProxies_[targetDeviceId] = remote;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Got remote proxy for device %{public}s", targetDeviceId.substr(0, 8).c_str());
    return remote;
}

void DstbIpcManager::ClearRemoteProxy(const std::string& deviceId)
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    remoteProxies_.erase(deviceId);
}

int32_t DstbIpcManager::GetRemoteToolSummaries(const std::string& targetDeviceId,
    std::vector<ToolSummary>& summaries)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetRemoteToolSummaries for device %{public}s",
        targetDeviceId.substr(0, 8).c_str());

    auto remoteProxy = GetRemoteCliToolMgrProxy(targetDeviceId);
    if (remoteProxy == nullptr) {
        return DstbErrorCode::ERR_GET_REMOTE_PROXY_FAILED;
    }

    // 构造调用者信息
    DstbCallerInfo callerInfo;
    callerInfo.sourceDeviceId = GetLocalDeviceId();
    callerInfo.uid = getuid();
    callerInfo.pid = getpid();

    // 发起远程调用
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    // 写入接口令牌
    if (!data.WriteInterfaceToken(u"OHOS.CliTool.ICliToolManager")) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write interface token failed");
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }

    // 写入调用者信息
    if (!callerInfo.Marshalling(data)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write caller info failed");
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }

    // 发送请求 - 分布式接口码
    constexpr uint32_t GET_ALL_TOOL_SUMMARIES_FROM_REMOTE_CODE = 100;
    auto ret = remoteProxy->SendRequest(GET_ALL_TOOL_SUMMARIES_FROM_REMOTE_CODE, data, reply, option);

    if (ret != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendRequest failed: %{public}d", ret);
        ClearRemoteProxy(targetDeviceId);
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }

    // 读取返回值
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read result failed");
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }

    if (result != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Remote call failed: %{public}d", result);
        return result;
    }

    // 读取工具摘要列表
    int32_t size = 0;
    if (!reply.ReadInt32(size)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read size failed");
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }

    summaries.clear();
    for (int32_t i = 0; i < size; ++i) {
        ToolSummary summary;
        if (!reply.ReadString(summary.name) ||
            !reply.ReadString(summary.description) ||
            !reply.ReadString(summary.version)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Read tool summary failed at index %{public}d", i);
            return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
        }
        summaries.push_back(summary);
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetRemoteToolSummaries success, count: %{public}zu", summaries.size());
    return DstbErrorCode::ERR_OK;
}

int32_t DstbIpcManager::GetRemoteToolInfoByName(const std::string& targetDeviceId,
    const std::string& name, ToolInfo& tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetRemoteToolInfoByName for device %{public}s, tool: %{public}s",
        targetDeviceId.substr(0, 8).c_str(), name.c_str());

    auto remoteProxy = GetRemoteCliToolMgrProxy(targetDeviceId);
    if (remoteProxy == nullptr) {
        return DstbErrorCode::ERR_GET_REMOTE_PROXY_FAILED;
    }

    // 构造调用者信息
    DstbCallerInfo callerInfo;
    callerInfo.sourceDeviceId = GetLocalDeviceId();
    callerInfo.uid = getuid();
    callerInfo.pid = getpid();

    // 发起远程调用
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    // 写入接口令牌
    if (!data.WriteInterfaceToken(u"OHOS.CliTool.ICliToolManager")) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write interface token failed");
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }

    // 写入调用者信息和工具名
    if (!callerInfo.Marshalling(data) || !data.WriteString(name)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Write params failed");
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }

    // 发送请求 - 分布式接口码
    constexpr uint32_t GET_TOOL_INFO_BY_NAME_FROM_REMOTE_CODE = 101;
    auto ret = remoteProxy->SendRequest(GET_TOOL_INFO_BY_NAME_FROM_REMOTE_CODE, data, reply, option);

    if (ret != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendRequest failed: %{public}d", ret);
        ClearRemoteProxy(targetDeviceId);
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }

    // 读取返回值
    int32_t result = 0;
    if (!reply.ReadInt32(result)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Read result failed");
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }

    if (result != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Remote call failed: %{public}d", result);
        return result;
    }

    // 读取工具信息
    auto* toolPtr = ToolInfo::Unmarshalling(reply);
    if (toolPtr == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Unmarshalling tool info failed");
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }
    tool = *toolPtr;
    delete toolPtr;

    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetRemoteToolInfoByName success");
    return DstbErrorCode::ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
