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

#ifndef CLI_PARAMETER_MAPPER_H
#define CLI_PARAMETER_MAPPER_H

#include <json/json.h>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include "cli_tool_info.h"

namespace OHOS {
namespace CliMgr {

/**
 * @brief 参数映射器
 *
 * 将抽象参数（JSON 对象）转换为命令行参数数组
 * 支持多种映射策略：flag、positional、flattened、json-string、json-file、mixed
 */
class ParameterMapper {
public:
    /**
     * @brief 将抽象参数转换为命令行参数数组
     * @param toolInfo 工具信息（包含映射规则）
     * @param abstractArgs 抽象参数（来自 JS 调用方）
     * @return 命令行参数数组（用于 execve）
     */
    static std::vector<std::string> MapToCommandLine(
        const ToolInfo& toolInfo,
        const Json::Value& abstractArgs);

private:
    /**
     * @brief 应用模板映射（flag 模式）
     */
    static void ApplyTemplateMapping(
        const ToolInfo& toolInfo,
        const Json::Value& abstractArgs,
        std::vector<std::string>& argv);

    /**
     * @brief 应用展平嵌套映射（flattened 模式）
     */
    static void ApplyFlattenedMapping(
        const Json::Value& argMapping,
        const Json::Value& abstractArgs,
        std::vector<std::string>& argv,
        const std::string& prefix = "");

    /**
     * @brief 应用 JSON 字符串映射
     */
    static void ApplyJsonStringMapping(
        const Json::Value& templates,
        const Json::Value& abstractArgs,
        std::vector<std::string>& argv);

    /**
     * @brief 应用 JSON 文件映射
     */
    static void ApplyJsonFileMapping(
        const Json::Value& templates,
        const Json::Value& abstractArgs,
        std::vector<std::string>& argv);

    /**
     * @brief 应用位置参数映射（positional 模式）
     */
    static void ApplyPositionalMapping(
        const Json::Value& argMapping,
        const Json::Value& abstractArgs,
        std::vector<std::string>& argv);

    /**
     * @brief 应用混合映射（mixed 模式）
     */
    static void ApplyMixedMapping(
        const Json::Value& argMapping,
        const Json::Value& abstractArgs,
        std::vector<std::string>& argv);

    /**
     * @brief 格式化模板字符串
     * @param template 模板字符串（如 "--key={value}"）
     * @param value 值
     * @return 格式化后的字符串
     */
    static std::string FormatTemplate(
        const std::string& tmpl,
        const std::string& value);

    /**
     * @brief 格式化模板字符串（支持 JSON）
     */
    static std::string FormatTemplateJson(
        const std::string& tmpl,
        const Json::Value& value);

    /**
     * @brief 处理单个参数值
     */
    static void ProcessValue(
        const std::string& key,
        const Json::Value& value,
        const Json::Value& templateDef,
        std::vector<std::string>& argv);

    /**
     * @brief 展平嵌套对象
     * @param obj 嵌套对象
     * @param prefix 前缀（如 "app."）
     * @param flattened 输出的展平对象
     */
    static void FlattenObject(
        const Json::Value& obj,
        const std::string& prefix,
        Json::Value& flattened);

    /**
     * @brief 创建临时 JSON 文件
     * @param json JSON 对象
     * @return 临时文件路径
     */
    static std::string CreateTempJsonFile(const Json::Value& json);

    /**
     * @brief 生成随机 ID
     */
    static std::string GenerateRandomId();

    /**
     * @brief 转义 shell 特殊字符
     */
    static std::string EscapeShellArg(const std::string& arg);

    /**
     * @brief 序列化 JSON 为字符串
     */
    static std::string JsonToString(const Json::Value& json);
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_PARAMETER_MAPPER_H
