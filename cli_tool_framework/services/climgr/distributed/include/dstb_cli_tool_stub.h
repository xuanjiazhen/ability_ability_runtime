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

#ifndef OHOS_CLI_TOOL_DSTB_CLI_TOOL_STUB_H
#define OHOS_CLI_TOOL_DSTB_CLI_TOOL_STUB_H

#include <map>
#include <mutex>

#include "dstb_types.h"
#include "iremote_stub.h"
#include "tool_summary.h"
#include "tool_info.h"
#include "exec_tool_param.h"
#include "cli_session_info.h"
#include "icli_tool_manager_scheduler.h"

namespace OHOS {
namespace CliTool {

// 分布式 CLI 工具管理桩
// 用于接收并处理远端设备的调用请求
class DstbCliToolStub : public IRemoteStub<IRemoteBroker> {
public:
    DstbCliToolStub();
    ~DstbCliToolStub() override;

    // 处理远程请求
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data,
        MessageParcel& reply, MessageOption& option) override;

    // 设置本地服务实例（用于实际处理请求）
    void SetLocalService(const sptr<IRemoteObject>& localService);

private:
    // 处理远端查询请求
    int32_t HandleGetAllToolSummariesFromRemote(MessageParcel& data, MessageParcel& reply);
    int32_t HandleGetToolInfoByNameFromRemote(MessageParcel& data, MessageParcel& reply);

    // 权限校验
    bool CheckCallerPermission(const DstbCallerInfo& callerInfo);

    // 读取调用者信息
    bool ReadCallerInfo(MessageParcel& data, DstbCallerInfo& callerInfo);

    sptr<IRemoteObject> localService_;
    std::mutex serviceMutex_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_DSTB_CLI_TOOL_STUB_H
