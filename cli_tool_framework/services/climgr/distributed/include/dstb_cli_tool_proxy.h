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

#ifndef OHOS_CLI_TOOL_DSTB_CLI_TOOL_PROXY_H
#define OHOS_CLI_TOOL_DSTB_CLI_TOOL_PROXY_H

#include <string>
#include <vector>

#include "dstb_types.h"
#include "iremote_proxy.h"
#include "tool_summary.h"
#include "tool_info.h"
#include "exec_tool_param.h"
#include "cli_session_info.h"
#include "icli_tool_manager_scheduler.h"

namespace OHOS {
namespace CliTool {

// 分布式 CLI 工具管理代理
// 用于向远端设备发起调用
class DstbCliToolProxy {
public:
    DstbCliToolProxy();
    ~DstbCliToolProxy();

    // 获取可用设备列表
    int32_t GetAvailableDevices(std::vector<DstbDeviceInfo>& devices);

    // 远端工具查询
    int32_t GetRemoteToolSummaries(const std::string& targetDeviceId,
        std::vector<ToolSummary>& summaries);
    int32_t GetRemoteToolInfoByName(const std::string& targetDeviceId,
        const std::string& name, ToolInfo& tool);

    // 获取远端服务的代理对象
    sptr<IRemoteObject> GetRemoteServiceProxy(const std::string& targetDeviceId);

private:
    // 获取远端 climgr 的 SystemAbility 代理
    sptr<IRemoteObject> GetRemoteCliToolMgr(const std::string& targetDeviceId);

    std::mutex proxyMutex_;
    std::map<std::string, sptr<IRemoteObject>> remoteProxies_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_DSTB_CLI_TOOL_PROXY_H
