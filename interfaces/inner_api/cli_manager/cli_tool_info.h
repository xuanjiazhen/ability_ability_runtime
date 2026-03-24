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

#ifndef CLI_TOOL_INFO_H
#define CLI_TOOL_INFO_H

#include <string>
#include <vector>
#include <cstdint>
#include <json/json.h>

namespace OHOS {
namespace CliMgr {

/**
 * @brief 工具类型
 */
enum class ToolType : uint8_t {
    SHORT_LIVED = 0,  // 短时运行，快速退出
    LONG_LIVED = 1,   // 长时运行，持续输出
};

/**
 * @brief 工具元数据
 */
struct ToolInfo {
    std::string name;                    // 工具名称
    std::string version;                 // 工具版本
    std::string description;             // 工具描述
    std::string executablePath;          // 可执行文件路径
    ToolType type;                       // 工具类型
    std::vector<std::string> permissions; // 所需权限列表
    Json::Value inputSchema;             // 输入参数 Schema
    Json::Value outputSchema;            // 输出结果 Schema
    uint32_t timeoutMs;                  // 超时时间 (毫秒)
    std::vector<std::string> eventTypes; // 支持的事件类型
    uint64_t registeredTime;             // 注册时间戳
    bool enabled;                        // 是否启用

    ToolInfo()
        : type(ToolType::SHORT_LIVED),
          timeoutMs(30000),
          registeredTime(0),
          enabled(true)
    {}
};

/**
 * @brief 调用者信息
 */
struct CallerInfo {
    pid_t pid;                    // 调用者进程 ID
    uid_t uid;                    // 调用者用户 ID
    std::string bundleName;       // 应用包名
    std::string abilityName;      // Ability 名称
    std::string token;            // 安全令牌

    CallerInfo() : pid(-1), uid(-1) {}
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_TOOL_INFO_H
