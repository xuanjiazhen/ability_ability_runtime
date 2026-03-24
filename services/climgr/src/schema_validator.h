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

#ifndef CLI_SCHEMA_VALIDATOR_H
#define CLI_SCHEMA_VALIDATOR_H

#include <json/json.h>
#include <vector>
#include <string>

namespace OHOS {
namespace CliMgr {

/**
 * @brief 验证结果
 */
struct ValidationResult {
    bool valid = true;                              // 是否通过验证
    std::vector<std::string> errors;                // 错误信息列表
    std::vector<std::string> warnings;              // 警告信息列表

    void AddError(const std::string& error) {
        valid = false;
        errors.push_back(error);
    }

    void AddWarning(const std::string& warning) {
        warnings.push_back(warning);
    }

    void Merge(const ValidationResult& other) {
        if (!other.valid) {
            valid = false;
        }
        errors.insert(errors.end(), other.errors.begin(), other.errors.end());
        warnings.insert(warnings.end(), other.warnings.begin(), other.warnings.end());
    }
};

/**
 * @brief JSON Schema 验证器
 *
 * 验证工具参数是否符合 JSON Schema 规范
 * 支持：基本类型、复合类型、特殊类型、嵌套对象、数组、约束验证
 */
class SchemaValidator {
public:
    /**
     * @brief 验证参数是否符合 Schema
     * @param args 待验证的参数
     * @param schema JSON Schema
     * @return 验证结果
     */
    static ValidationResult Validate(
        const Json::Value& args,
        const Json::Value& schema);

private:
    /**
     * @brief 验证单个字段
     */
    static void ValidateField(
        const std::string& fieldName,
        const Json::Value& value,
        const Json::Value& schema,
        ValidationResult& result);

    /**
     * @brief 类型验证
     */
    static bool ValidateType(
        const std::string& fieldName,
        const Json::Value& value,
        const Json::Value& schema,
        ValidationResult& result);

    /**
     * @brief 检查值是否匹配类型
     */
    static bool CheckType(const Json::Value& value, const std::string& expectedType);

    /**
     * @brief 验证字符串约束
     */
    static void ValidateStringConstraints(
        const std::string& fieldName,
        const Json::Value& value,
        const Json::Value& schema,
        ValidationResult& result);

    /**
     * @brief 验证数值约束
     */
    static void ValidateNumberConstraints(
        const std::string& fieldName,
        const Json::Value& value,
        const Json::Value& schema,
        ValidationResult& result);

    /**
     * @brief 验证数组约束
     */
    static void ValidateArrayConstraints(
        const std::string& fieldName,
        const Json::Value& value,
        const Json::Value& schema,
        ValidationResult& result);

    /**
     * @brief 验证对象约束
     */
    static void ValidateObjectConstraints(
        const std::string& fieldName,
        const Json::Value& value,
        const Json::Value& schema,
        ValidationResult& result);

    /**
     * @brief 验证字符串格式
     */
    static bool ValidateFormat(const std::string& value, const std::string& format);

    /**
     * @brief 验证正则表达式
     */
    static bool ValidatePattern(const std::string& value, const std::string& pattern);

    /**
     * @brief 验证枚举值
     */
    static bool ValidateEnum(const Json::Value& value, const Json::Value& enumSchema);

    /**
     * @brief 验证常量值
     */
    static bool ValidateConst(const Json::Value& value, const Json::Value& constValue);
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_SCHEMA_VALIDATOR_H
