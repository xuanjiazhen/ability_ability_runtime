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

#ifndef PERMISSION_CHECKER_H
#define PERMISSION_CHECKER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include "cli_tool_info.h"

namespace OHOS {
namespace CliMgr {

/**
 * @brief 权限校验结果
 */
struct PermissionCheckResult {
    bool granted;           // 是否授权
    std::string reason;     // 拒绝原因
};

/**
 * @brief 权限校验器
 *
 * 负责校验调用者是否具备执行工具所需的权限
 */
class PermissionChecker {
public:
    PermissionChecker();
    ~PermissionChecker() = default;

    /**
     * @brief 初始化
     * @return 是否成功
     */
    bool Initialize();

    /**
     * @brief 校验调用者权限
     * @param caller 调用者信息
     * @param requiredPermissions 所需权限列表
     * @return 校验结果
     */
    PermissionCheckResult CheckPermission(const CallerInfo& caller,
                                          const std::vector<std::string>& requiredPermissions);

    /**
     * @brief 校验调用者是否可执行指定工具
     * @param caller 调用者信息
     * @param tool 工具信息
     * @return 校验结果
     */
    PermissionCheckResult CheckToolPermission(const CallerInfo& caller,
                                              const ToolInfo& tool);

    /**
     * @brief 校验系统应用签名
     * @param caller 调用者信息
     * @return 是否为有效的系统应用
     */
    bool ValidateSystemApp(const CallerInfo& caller);

private:
    /**
     * @brief 校验单个权限
     * @param caller 调用者信息
     * @param permission 权限名称
     * @return 是否授权
     */
    bool CheckSinglePermission(const CallerInfo& caller,
                               const std::string& permission);

    /**
     * @brief 记录审计日志
     * @param caller 调用者信息
     * @param toolName 工具名称
     * @param granted 是否授权
     * @param reason 原因
     */
    void LogAudit(const CallerInfo& caller,
                  const std::string& toolName,
                  bool granted,
                  const std::string& reason);

private:
    // 权限缓存
    std::unordered_map<std::string, bool> permissionCache_;
    std::mutex cacheMutex_;

    static constexpr const char* TAG = "PermissionChecker";
};

} // namespace CliMgr
} // namespace OHOS

#endif // PERMISSION_CHECKER_H
