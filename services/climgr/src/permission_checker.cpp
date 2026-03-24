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

#include "permission_checker.h"
#include "hilog_tag_wrapper.h"
#include <chrono>
#include <sstream>
#include <iomanip>

namespace OHOS {
namespace CliMgr {

PermissionChecker::PermissionChecker()
{
}

bool PermissionChecker::Initialize()
{
    TAG_LOGI(TAG, "PermissionChecker initialized");
    return true;
}

PermissionCheckResult PermissionChecker::CheckPermission(
    const CallerInfo& caller,
    const std::vector<std::string>& requiredPermissions)
{
    PermissionCheckResult result;
    result.granted = true;
    result.reason = "All permissions granted";

    // 如果没有所需权限，直接通过
    if (requiredPermissions.empty()) {
        return result;
    }

    // 逐个检查权限
    for (const auto& permission : requiredPermissions) {
        if (!CheckSinglePermission(caller, permission)) {
            result.granted = false;
            result.reason = "Missing permission: " + permission;
            TAG_LOGW(TAG, "Permission denied: %{public}s for caller %{public}s",
                     permission.c_str(), caller.bundleName.c_str());
            return result;
        }
    }

    return result;
}

PermissionCheckResult PermissionChecker::CheckToolPermission(
    const CallerInfo& caller,
    const ToolInfo& tool)
{
    // 首先验证是否为系统应用
    if (!ValidateSystemApp(caller)) {
        PermissionCheckResult result;
        result.granted = false;
        result.reason = "Caller is not a valid system app";
        TAG_LOGW(TAG, "Access denied: not a system app: %{public}s",
                 caller.bundleName.c_str());
        return result;
    }

    // 检查工具所需权限
    return CheckPermission(caller, tool.permissions);
}

bool PermissionChecker::ValidateSystemApp(const CallerInfo& caller)
{
    // TODO: 实际实现需要：
    // 1. 验证 Token 的有效性
    // 2. 检查应用的签名
    // 3. 验证应用是否为系统应用

    // 简化实现：检查 bundleName 是否以 "com.huawei." 或 "com.ohos." 开头
    if (caller.bundleName.empty()) {
        TAG_LOGW(TAG, "Empty bundle name");
        return false;
    }

    // 系统应用前缀检查
    if (caller.bundleName.find("com.huawei.") == 0 ||
        caller.bundleName.find("com.ohos.") == 0 ||
        caller.bundleName.find("ohos.") == 0) {
        return true;
    }

    TAG_LOGW(TAG, "Not a system app bundle name: %{public}s",
             caller.bundleName.c_str());
    return false;
}

bool PermissionChecker::CheckSinglePermission(
    const CallerInfo& caller,
    const std::string& permission)
{
    // TODO: 实际实现需要：
    // 1. 查询权限管理服务
    // 2. 检查调用者是否被授予该权限
    // 3. 考虑权限的动态授予状态

    // 简化实现：使用缓存记录权限决策
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        std::string cacheKey = caller.bundleName + ":" + permission;
        auto it = permissionCache_.find(cacheKey);
        if (it != permissionCache_.end()) {
            return it->second;
        }
    }

    // 模拟权限检查
    bool granted = true;

    // 特殊权限需要特别检查
    if (permission == "ohos.permission.INTERNET") {
        // 网络权限通常默认授予
        granted = true;
    } else if (permission == "ohos.permission.GET_NETWORK_INFO") {
        // 网络信息权限需要检查
        granted = true;
    } else if (permission.find("ohos.permission.") == 0) {
        // 其他系统权限默认授予（简化实现）
        granted = true;
    } else {
        TAG_LOGW(TAG, "Unknown permission: %{public}s", permission.c_str());
        granted = false;
    }

    // 缓存结果
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        std::string cacheKey = caller.bundleName + ":" + permission;
        permissionCache_[cacheKey] = granted;
    }

    return granted;
}

void PermissionChecker::LogAudit(const CallerInfo& caller,
                                const std::string& toolName,
                                bool granted,
                                const std::string& reason)
{
    // 构建审计日志
    std::stringstream ss;
    ss << "CLI_Permission_Audit: "
       << "tool=" << toolName << ", "
       << "caller=" << caller.bundleName << ", "
       << "pid=" << caller.pid << ", "
       << "uid=" << caller.uid << ", "
       << "granted=" << (granted ? "true" : "false") << ", "
       << "reason=" << reason;

    TAG_LOGI(TAG, "%{public}s", ss.str().c_str());

    // TODO: 实际实现应该：
    // 1. 写入审计日志文件
    // 2. 发送到安全审计服务
    // 3. 记录时间戳和调用堆栈
}

} // namespace CliMgr
} // namespace OHOS
