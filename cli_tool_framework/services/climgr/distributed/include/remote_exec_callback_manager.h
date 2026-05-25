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

#ifndef OHOS_ABILITY_RUNTIME_REMOTE_EXEC_CALLBACK_MANAGER_H
#define OHOS_ABILITY_RUNTIME_REMOTE_EXEC_CALLBACK_MANAGER_H

#include <mutex>
#include <unordered_map>
#include <string>

#include "cli_session_info.h"
#include "icli_tool_manager.h"
#include "iremote_object.h"
#include "single_instance.h"

namespace OHOS {
namespace CliTool {

// 远程执行回调信息
struct RemoteExecCallbackInfo {
    std::string sessionId;
    std::string eventId;
    sptr<ICliToolManagerScheduler> scheduler;
    int64_t createTime = 0;
};

// 远程执行回调管理器
// 用于保存发起端的 scheduler，等待远端执行完成后回调
class RemoteExecCallbackManager {
    DECLARE_SINGLE_INSTANCE(RemoteExecCallbackManager);

public:
    // 添加回调记录
    void AddCallback(const std::string &sessionId, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler);

    // 获取回调记录
    std::shared_ptr<RemoteExecCallbackInfo> GetCallback(const std::string &sessionId);

    // 移除回调记录
    void RemoveCallback(const std::string &sessionId);

    // 处理远端执行结果通知
    void HandleRemoteExecResult(const std::string &sessionId, int32_t resultCode,
        const CliSessionInfo &session);

private:
    std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<RemoteExecCallbackInfo>> callbacks_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_REMOTE_EXEC_CALLBACK_MANAGER_H