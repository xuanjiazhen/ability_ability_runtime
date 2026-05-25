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

#ifndef OHOS_CLI_TOOL_DSTB_IPC_MANAGER_H
#define OHOS_CLI_TOOL_DSTB_IPC_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "dstb_types.h"
#include "iremote_object.h"
#include "single_instance.h"
#include "tool_info.h"
#include "tool_summary.h"

namespace OHOS {
namespace CliTool {

// 分布式 IPC 管理器
// 负责跨设备通信和远端服务代理管理
class DstbIpcManager {
    DECLARE_SINGLE_INSTANCE_BASE(DstbIpcManager);

public:
    // 初始化
    void Init();

    // 反初始化
    void UnInit();

    // 获取本地设备ID
    std::string GetLocalDeviceId();

    // 获取可用设备列表
    int32_t GetAvailableDevices(std::vector<DstbDeviceInfo>& devices);

    // 检查设备是否在线
    bool IsDeviceOnline(const std::string& deviceId);

    // 获取远端 climgr 服务代理
    sptr<IRemoteObject> GetRemoteCliToolMgrProxy(const std::string& targetDeviceId);

    // 清理远端代理缓存
    void ClearRemoteProxy(const std::string& deviceId);

    // 远端工具查询
    int32_t GetRemoteToolSummaries(const std::string& targetDeviceId,
        std::vector<ToolSummary>& summaries);

    // 远端工具信息查询
    int32_t GetRemoteToolInfoByName(const std::string& targetDeviceId,
        const std::string& name, ToolInfo& tool);

private:
    DstbIpcManager();
    ~DstbIpcManager();

    std::mutex initMutex_;
    bool initialized_ = false;
    std::string localDeviceId_;

    std::mutex proxyMutex_;
    std::map<std::string, sptr<IRemoteObject>> remoteProxies_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_DSTB_IPC_MANAGER_H
