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

#ifndef COMMAND_SANITIZER_H
#define COMMAND_SANITIZER_H

#include <string>
#include <vector>
#include <json/json.h>

namespace OHOS {
namespace CliMgr {

/**
 * @brief 命令清理器
 *
 * 防止命令注入攻击，验证和清理用户输入
 */
class CommandSanitizer {
public:
    /**
     * @brief 验证并清理命令参数
     * @param rawInput 原始输入 (JSON 格式)
     * @param schema 预期 Schema
     * @param sanitizedOutput 输出参数，清理后的参数
     * @return 是否有效
     */
    static bool SanitizeArgs(const Json::Value& rawInput,
                           const Json::Value& schema,
                           Json::Value& sanitizedOutput);

    /**
     * @brief 验证单个字符串值
     * @param value 字符串值
     * @param dangerousChars 危险字符列表
     * @return 是否安全
     */
    static bool ValidateString(const std::string& value,
                              const std::string& dangerousChars = ";|&$`<>\\");

    /**
     * @brief 转义字符串中的特殊字符
     * @param input 输入字符串
     * @return 转义后的字符串
     */
    static std::string EscapeString(const std::string& input);

    /**
     * @brief 验证路径是否安全
     * @param path 文件路径
     * @return 是否安全
     */
    static bool ValidatePath(const std::string& path);

    /**
     * @brief 验证 IP 地址格式
     * @param ip IP 地址字符串
     * @return 是否有效
     */
    static bool ValidateIP(const std::string& ip);

    /**
     * @brief 验证端口号
     * @param port 端口号
     * @return 是否有效
     */
    static bool ValidatePort(int port);

    /**
     * @brief 检查 JSON Schema 是否有效
     * @param schema JSON Schema
     * @return 是否有效
     */
    static bool ValidateSchema(const Json::Value& schema);

private:
    /**
     * @brief 检查字符串中是否包含危险字符
     * @param str 字符串
     * @param chars 要检查的字符
     * @return 是否包含
     */
    static bool ContainsDangerousChars(const std::string& str,
                                      const std::string& chars);

    /**
     * @brief 验证 JSON 类型
     * @param value JSON 值
     * @param expectedType 期望的类型
     * @return 是否匹配
     */
    static bool ValidateType(const Json::Value& value,
                            const std::string& expectedType);

    /**
     * @brief 验证 JSON 值是否在范围内
     * @param value JSON 值
     * @param schema 包含 minimum/maximum 的 Schema
     * @return 是否在范围内
     */
    static bool ValidateRange(const Json::Value& value,
                             const Json::Value& schema);

    // 危险字符列表
    static constexpr const char* DANGEROUS_CHARS = ";|&$`<>\\*?[]{}$()";
    static constexpr const char* DANGEROUS_COMMANDS = "rm;mv;cp;dd;kill;chmod;chown";
};

} // namespace CliMgr
} // namespace OHOS

#endif // COMMAND_SANITIZER_H
