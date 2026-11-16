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

#include "task_mgr/resource_control_proxy.h"

#include "hilog_tag_wrapper.h"
#include "task_mgr/common.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t PROXY_OK = 0;
constexpr int32_t PROXY_ERR = -1;
}  // namespace

ResourceControlProxy &ResourceControlProxy::GetInstance()
{
    static ResourceControlProxy instance;
    return instance;
}

std::string ResourceControlProxy::MakeKey(const std::string &taskId, pid_t managedPid)
{
    return taskId + "#" + std::to_string(managedPid);
}

int32_t ResourceControlProxy::ApplyControl(const ControlRequest &req)
{
    if (req.managedPid <= 0) {
        return PROXY_ERR;
    }
    {
        std::lock_guard<std::mutex> lock(registryMutex_);
        controlRegistry_[MakeKey(req.taskId, req.managedPid)] = req;
    }
    // Phase-1 Stub: registry + key-path logging, no real RSS delivery.
    TAG_LOGI(AAFwkTag::APPMGR,
        "rss schedule control apply(stub) taskId: %{public}s pid: %{public}d type: %{public}s level: %{public}s "
        "policy[freeze:%{public}d group:%{public}d qos:%{public}d speed:%{public}d]",
        req.taskId.c_str(), req.managedPid, AttributionToolToString(req.managedType),
        TaskPriorityToString(req.level), req.policy.freeze, req.policy.schedGroup,
        req.policy.qos, req.policy.speed);
    if (probeEnabled_) {
        TAG_LOGD(AAFwkTag::APPMGR, "rss schedule control probe apply pid: %{public}d", req.managedPid);
    }
    return PROXY_OK;
}

int32_t ResourceControlProxy::RevokeControl(const std::string &taskId, pid_t managedPid)
{
    revokeCallsForTest_.fetch_add(1);
    if (managedPid <= 0) {
        return PROXY_ERR;
    }
    if (failRevokeForTest_) {
        return PROXY_ERR;  // fault injection: force the revoke delivery to fail for retry tests
    }
    {
        std::lock_guard<std::mutex> lock(registryMutex_);
        controlRegistry_.erase(MakeKey(taskId, managedPid));
    }
    TAG_LOGI(AAFwkTag::APPMGR, "rss schedule control revoke(stub) taskId: %{public}s pid: %{public}d",
        taskId.c_str(), managedPid);
    return PROXY_OK;
}

int32_t ResourceControlProxy::QueryControlState(const std::string &taskId, pid_t managedPid, int32_t &level)
{
    if (failQueryForTest_) {
        return PROXY_ERR;  // fault injection: simulate an ineffective control for verify tests
    }
    std::lock_guard<std::mutex> lock(registryMutex_);
    auto it = controlRegistry_.find(MakeKey(taskId, managedPid));
    if (it == controlRegistry_.end()) {
        return PROXY_ERR;
    }
    level = it->second.level;
    return PROXY_OK;
}

void ResourceControlProxy::NotifyTaskEnd(const TaskEndInfo &info)
{
    // Task-level cleanup with a bounded retry (<=3) before alerting on persistent failure.
    constexpr int32_t MAX_RETRY = 3;
    int32_t attempt = 0;
    bool delivered = false;
    while (attempt <= MAX_RETRY) {
        if (!failNotifyForTest_) {
            delivered = true;
            break;
        }
        attempt++;
        TAG_LOGW(AAFwkTag::APPMGR, "rss schedule control task end retry, taskId: %{public}s attempt: %{public}d",
            info.taskId.c_str(), attempt);
    }
    if (!delivered) {
        TAG_LOGW(AAFwkTag::APPMGR,
            "rss schedule control task end failed after %{public}d retries, task alert, taskId: %{public}s",
            MAX_RETRY, info.taskId.c_str());
    }
    // Phase-1 Stub: task-level cleanup notification with key-path logging.
    TAG_LOGI(AAFwkTag::APPMGR, "rss schedule control task end(stub) taskId: %{public}s pids: %{public}zu",
        info.taskId.c_str(), info.pids.size());
    std::lock_guard<std::mutex> lock(registryMutex_);
    for (pid_t pid : info.pids) {
        controlRegistry_.erase(MakeKey(info.taskId, pid));
    }
}

}  // namespace AppExecFwk
}  // namespace OHOS
