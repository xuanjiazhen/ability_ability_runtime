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

#ifndef OHOS_CLI_TOOL_DSTB_DEVICE_MANAGER_H
#define OHOS_CLI_TOOL_DSTB_DEVICE_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "dstb_types.h"
#include "single_instance.h"
#include "iremote_object.h"

namespace OHOS {
namespace CliTool {

// 设备状态变化回调
class DstbDeviceListener {
public:
    virtual ~DstbDeviceListener() = default;
    virtual void OnDeviceOnline(const DstbDeviceInfo& deviceInfo) = 0;
    virtual void OnDeviceOffline(const DstbDeviceInfo& deviceInfo) = 0;
    virtual void OnDeviceInfoChanged(const DstbDeviceInfo& deviceInfo) = 0;
};

// 设备管理适配器
// 封装 DeviceManager 的设备发现功能
class DstbDeviceManager {
    DECLARE_SINGLE_INSTANCE_BASE(DstbDeviceManager);

public:
    // 初始化设备管理器
    void Init();

    // 反初始化
    void UnInit();

    // 获取在线设备列表
    int32_t GetAvailableDevices(std::vector<DstbDeviceInfo>& devices);

    // 获取指定设备信息
    std::optional<DstbDeviceInfo> GetDeviceInfo(const std::string& deviceId);

    // 检查设备是否在线
    bool IsDeviceOnline(const std::string& deviceId);

    // 获取本地设备ID
    std::string GetLocalDeviceId();

    // 注册设备状态监听器
    int32_t RegisterDeviceListener(const std::shared_ptr<DstbDeviceListener>& listener);

    // 注销设备状态监听器
    int32_t UnregisterDeviceListener(const std::shared_ptr<DstbDeviceListener>& listener);

private:
    DstbDeviceManager();
    ~DstbDeviceManager();

    // 设备上线处理
    void HandleDeviceOnline(const std::string& deviceId, const std::string& deviceName, int32_t deviceType);

    // 设备离线处理
    void HandleDeviceOffline(const std::string& deviceId);

    // 设备信息变更处理
    void HandleDeviceInfoChanged(const std::string& deviceId);

    // 从 DeviceManager 获取设备列表
    int32_t FetchDeviceListFromDM(std::vector<DstbDeviceInfo>& devices);

    std::mutex deviceMutex_;
    std::map<std::string, DstbDeviceInfo> onlineDevices_;
    std::string localDeviceId_;

    std::mutex listenerMutex_;
    std::vector<std::shared_ptr<DstbDeviceListener>> listeners_;

    bool initialized_ = false;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_DSTB_DEVICE_MANAGER_H
