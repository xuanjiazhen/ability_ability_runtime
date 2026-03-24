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

#include "parameter_mapper.h"
#include "hilog_tag_wrapper.h"
#include <fstream>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstdlib>

namespace OHOS {
namespace CliMgr {

std::vector<std::string> ParameterMapper::MapToCommandLine(
    const ToolInfo& toolInfo,
    const Json::Value& abstractArgs)
{
    std::vector<std::string> argv;
    argv.push_back(toolInfo.executablePath);  // argv[0]

    // 检查是否有参数映射规则
    if (!toolInfo.argMapping ||
        !toolInfo.argMapping.isMember("type")) {
        // 使用默认规则映射
        ApplyTemplateMapping(toolInfo, abstractArgs, argv);
        return argv;
    }

    std::string mappingType = toolInfo.argMapping["type"].asString();

    if (mappingType == "flag" || mappingType == "default") {
        ApplyTemplateMapping(toolInfo, abstractArgs, argv);
    }
    else if (mappingType == "flattened") {
        ApplyFlattenedMapping(toolInfo.argMapping, abstractArgs, argv);
    }
    else if (mappingType == "json-string") {
        ApplyJsonStringMapping(toolInfo.argMapping["templates"], abstractArgs, argv);
    }
    else if (mappingType == "json-file") {
        ApplyJsonFileMapping(toolInfo.argMapping["templates"], abstractArgs, argv);
    }
    else if (mappingType == "positional") {
        ApplyPositionalMapping(toolInfo.argMapping, abstractArgs, argv);
    }
    else if (mappingType == "mixed") {
        ApplyMixedMapping(toolInfo.argMapping, abstractArgs, argv);
    }
    else {
        TAG_LOGW(TAG, "Unknown mapping type: %{public}s, using default",
                 mappingType.c_str());
        ApplyTemplateMapping(toolInfo, abstractArgs, argv);
    }

    return argv;
}

void ParameterMapper::ApplyTemplateMapping(
    const ToolInfo& toolInfo,
    const Json::Value& abstractArgs,
    std::vector<std::string>& argv)
{
    if (!toolInfo.argMapping ||
        !toolInfo.argMapping.isMember("templates")) {
        // 没有模板定义，使用默认规则
        for (const auto& key : abstractArgs.getMemberNames()) {
            Json::Value value = abstractArgs[key];

            if (value.isString()) {
                std::string arg = "--" + key + "=" + value.asString();
                argv.push_back(arg);
            }
            else if (value.isInt() || value.isUInt()) {
                std::string arg = "--" + key + "=" + std::to_string(value.asInt());
                argv.push_back(arg);
            }
            else if (value.isBool()) {
                if (value.asBool()) {
                    argv.push_back("--" + key);
                } else {
                    argv.push_back("--no-" + key);
                }
            }
        }
        return;
    }

    // 使用定义的模板
    const Json::Value& templates = toolInfo.argMapping["templates"];

    for (const auto& key : abstractArgs.getMemberNames()) {
        if (!templates.isMember(key)) {
            TAG_LOGW(TAG, "No template defined for key: %{public}s", key.c_str());
            continue;
        }

        Json::Value value = abstractArgs[key];
        const Json::Value& templateDef = templates[key];

        ProcessValue(key, value, templateDef, argv);
    }
}

void ParameterMapper::ApplyFlattenedMapping(
    const Json::Value& argMapping,
    const Json::Value& abstractArgs,
    std::vector<std::string>& argv,
    const std::string& prefix)
{
    // 获取分隔符
    std::string separator = ".";
    if (argMapping.isMember("separator")) {
        separator = argMapping["separator"].asString();
    }

    // 展平嵌套对象
    Json::Value flattened;
    FlattenObject(abstractArgs, "", flattened);

    // 获取模板定义
    const Json::Value& templates = argMapping["templates"];

    // 应用模板
    for (const auto& key : flattened.getMemberNames()) {
        if (!templates.isMember(key)) {
            TAG_LOGW(TAG, "No template defined for flattened key: %{public}s", key.c_str());
            continue;
        }

        Json::Value value = flattened[key];
        const Json::Value& templateDef = templates[key];

        ProcessValue(key, value, templateDef, argv);
    }
}

void ParameterMapper::ApplyJsonStringMapping(
    const Json::Value& templates,
    const Json::Value& abstractArgs,
    std::vector<std::string>& argv)
{
    for (const auto& key : abstractArgs.getMemberNames()) {
        if (!templates.isMember(key)) {
            continue;
        }

        const Json::Value& value = abstractArgs[key];
        const Json::Value& templateDef = templates[key];

        if (value.isObject() || value.isArray()) {
            // 将对象/数组序列化为 JSON 字符串
            std::string jsonStr = JsonToString(value);
            std::string arg = FormatTemplate(templateDef.asString(), jsonStr);
            argv.push_back(arg);
        }
        else {
            // 非对象类型，正常处理
            ProcessValue(key, value, templateDef, argv);
        }
    }
}

void ParameterMapper::ApplyJsonFileMapping(
    const Json::Value& templates,
    const Json::Value& abstractArgs,
    std::vector<std::string>& argv)
{
    for (const auto& key : abstractArgs.getMemberNames()) {
        if (!templates.isMember(key)) {
            continue;
        }

        const Json::Value& value = abstractArgs[key];
        const Json::Value& templateDef = templates[key];

        if (value.isObject() || value.isArray()) {
            // 创建临时文件
            std::string tempFile = CreateTempJsonFile(value);

            // 替换模板中的 {value}
            std::string arg = FormatTemplate(templateDef.asString(), tempFile);
            argv.push_back(arg);
        }
        else {
            // 非对象类型，正常处理
            ProcessValue(key, value, templateDef, argv);
        }
    }
}

void ParameterMapper::ApplyPositionalMapping(
    const Json::Value& argMapping,
    const Json::Value& abstractArgs,
    std::vector<std::string>& argv)
{
    if (!argMapping.isMember("order")) {
        TAG_LOGE(TAG, "Positional mapping requires 'order' field");
        return;
    }

    // 按照指定的顺序添加参数
    const Json::Value& order = argMapping["order"];
    for (const auto& key : order) {
        std::string keyStr = key.asString();
        if (!abstractArgs.isMember(keyStr)) {
            continue;
        }

        const Json::Value& value = abstractArgs[keyStr];

        if (value.isString()) {
            argv.push_back(value.asString());
        }
        else if (value.isInt() || value.isUInt()) {
            argv.push_back(std::to_string(value.asInt()));
        }
        else if (value.isBool()) {
            argv.push_back(value.asBool() ? "true" : "false");
        }
        else {
            // 对象/数组类型，转为 JSON 字符串
            argv.push_back(JsonToString(value));
        }
    }
}

void ParameterMapper::ApplyMixedMapping(
    const Json::Value& argMapping,
    const Json::Value& abstractArgs,
    std::vector<std::string>& argv)
{
    const Json::Value& mappings = argMapping["mappings"];

    for (const auto& key : abstractArgs.getMemberNames()) {
        if (!mappings.isMember(key)) {
            continue;
        }

        const Json::Value& value = abstractArgs[key];
        const Json::Value& mappingDef = mappings[key];

        // 检查是否有特殊处理指令
        if (mappingDef.isMember("type")) {
            std::string type = mappingDef["type"].asString();

            if (type == "array" && value.isArray()) {
                // 数组特殊处理
                std::string template_ = mappingDef["template"].asString();

                for (const auto& item : value) {
                    if (mappingDef.isMember("extract")) {
                        // 从对象中提取特定字段
                        std::string extract = mappingDef["extract"].asString();
                        if (item.isMember(extract)) {
                            std::string itemJson = JsonToString(item);
                            std::string arg = FormatTemplate(template_, itemJson);
                            argv.push_back(arg);
                        }
                    } else {
                        std::string itemStr = JsonToString(item);
                        std::string arg = FormatTemplate(template_, itemStr);
                        argv.push_back(arg);
                    }
                }
            }
            else if (type == "json") {
                // JSON 序列化
                std::string template_ = mappingDef["template"].asString();
                std::string jsonStr = JsonToString(value);
                std::string arg = FormatTemplateJson(template_, jsonStr);
                argv.push_back(arg);
            }
        }
        else {
            // 默认处理
            ProcessValue(key, value, mappingDef, argv);
        }
    }
}

void ParameterMapper::ProcessValue(
    const std::string& key,
    const Json::Value& value,
    const Json::Value& templateDef,
    std::vector<std::string>& argv)
{
    if (value.isString()) {
        std::string arg = FormatTemplate(templateDef.asString(), value.asString());
        argv.push_back(arg);
    }
    else if (value.isInt() || value.isUInt()) {
        std::string arg = FormatTemplate(
            templateDef.asString(),
            std::to_string(value.asInt())
        );
        argv.push_back(arg);
    }
    else if (value.isDouble()) {
        std::string arg = FormatTemplate(
            templateDef.asString(),
            std::to_string(value.asDouble())
        );
        argv.push_back(arg);
    }
    else if (value.isBool()) {
        // 布尔值特殊处理
        if (templateDef.isMember("if_true") &&
            templateDef.isMember("if_false")) {
            std::string arg = value.asBool() ?
                templateDef["if_true"].asString() :
                templateDef["if_false"].asString();
            if (!arg.empty()) {
                argv.push_back(arg);
            }
        }
        else {
            std::string arg = FormatTemplate(
                templateDef.asString(),
                value.asBool() ? "true" : "false"
            );
            argv.push_back(arg);
        }
    }
    else if (value.isArray()) {
        // 数组展开
        for (const auto& item : value) {
            if (item.isString()) {
                std::string arg = FormatTemplate(templateDef.asString(), item.asString());
                argv.push_back(arg);
            }
            else if (item.isNumeric()) {
                std::string itemStr = item.asString();
                std::string arg = FormatTemplate(templateDef.asString(), itemStr);
                argv.push_back(arg);
            }
            else {
                // 复杂类型，转为 JSON 字符串
                std::string itemJson = JsonToString(item);
                std::string arg = FormatTemplate(templateDef.asString(), itemJson);
                argv.push_back(arg);
            }
        }
    }
    else if (value.isObject()) {
        // 对象类型，序列化为 JSON
        std::string jsonStr = JsonToString(value);
        std::string arg = FormatTemplate(templateDef.asString(), jsonStr);
        argv.push_back(arg);
    }
}

std::string ParameterMapper::FormatTemplate(
    const std::string& tmpl,
    const std::string& value)
{
    std::string result = tmpl;
    size_t pos = result.find("{value}");
    if (pos != std::string::npos) {
        result.replace(pos, 7, value);
    }
    return result;
}

std::string ParameterMapper::FormatTemplateJson(
    const std::string& tmpl,
    const Json::Value& value)
{
    std::string result = tmpl;
    size_t pos = result.find("{json}");
    if (pos != std::string::npos) {
        std::string jsonStr = JsonToString(value);
        result.replace(pos, 6, jsonStr);
    }
    return result;
}

void ParameterMapper::FlattenObject(
    const Json::Value& obj,
    const std::string& prefix,
    Json::Value& flattened)
{
    for (const auto& key : obj.getMemberNames()) {
        std::string fullKey = prefix.empty() ? key : prefix + "." + key;
        const Json::Value& value = obj[key];

        if (value.isObject() && !value.isNull()) {
            // 递归展平嵌套对象
            FlattenObject(value, fullKey, flattened);
        }
        else {
            // 基本类型，直接添加
            flattened[fullKey] = value;
        }
    }
}

std::string ParameterMapper::CreateTempJsonFile(const Json::Value& json)
{
    // 创建临时目录
    std::string tempDir = "/tmp/cli-tools";
    std::string mkdirCmd = "mkdir -p " + tempDir;
    system(mkdirCmd.c_str());

    // 生成文件名
    std::string fileName = "arg_" + GenerateRandomId() + ".json";
    std::string filePath = tempDir + "/" + fileName;

    // 写入 JSON 文件
    std::ofstream file(filePath);
    if (!file.is_open()) {
        TAG_LOGE(TAG, "Failed to create temp file: %{public}s", filePath.c_str());
        return "";
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    std::string jsonStr = Json::writeString(builder, json);
    file << jsonStr;
    file.close();

    TAG_LOGD(TAG, "Created temp JSON file: %{public}s", filePath.c_str());

    return filePath;
}

std::string ParameterMapper::GenerateRandomId()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100000, 999999);

    std::stringstream ss;
    ss << std::hex << std::setw(8) << std::setfill('0') << dis(gen);
    return ss.str();
}

std::string ParameterMapper::EscapeShellArg(const std::string& arg)
{
    std::string escaped = arg;
    // 转义特殊字符
    size_t pos = 0;
    while ((pos = escaped.find("'", pos)) != std::string::npos) {
        escaped.replace(pos, 1, "'\\''");
        pos += 4;
    }
    return "'" + escaped + "'";
}

std::string ParameterMapper::JsonToString(const Json::Value& json)
{
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, json);
}

} // namespace CliMgr
} // namespace OHOS
