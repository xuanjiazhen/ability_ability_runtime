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

#ifndef OHOS_ABILITY_RUNTIME_AA_START_COMMAND_H
#define OHOS_ABILITY_RUNTIME_AA_START_COMMAND_H

#include <string>
#include <vector>
#include <map>

#include "ability_manager_interface.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {

// 参数类型定义
using ParametersInteger = std::map<std::string, int>;
using ParametersString = std::map<std::string, std::string>;
using ParametersBool = std::map<std::string, bool>;

// 解析上下文结构体
struct ParseContext {
    std::string deviceId;
    std::string bundleName;
    std::string abilityName;
    std::string moduleName;
    std::string windowMode;
    ParametersInteger parametersInteger;
    ParametersString parametersString;
    ParametersBool parametersBool;
    std::string uri;
    std::string action;
    std::vector<std::string> entities;
    std::string typeVal;
    int windowLeft = 0;
    bool hasWindowLeft = false;
    int windowTop = 0;
    bool hasWindowTop = false;
    int windowHeight = 0;
    bool hasWindowHeight = false;
    int windowWidth = 0;
    bool hasWindowWidth = false;
};

class AAStartCommand {
public:
    AAStartCommand();
    ~AAStartCommand() = default;

    // 长选项定义
    enum {
        OPTION_PARAMETER_INTEGER = 1000,
        OPTION_PARAMETER_STRING,
        OPTION_PARAMETER_BOOL,
        OPTION_PARAMETER_NULL_STRING,
        OPTION_WINDOW_LEFT,
        OPTION_WINDOW_TOP,
        OPTION_WINDOW_HEIGHT,
        OPTION_WINDOW_WIDTH
    };

    // 执行启动命令
    ErrCode Execute(const std::vector<std::string>& args);

private:
    // 从命令行参数构建 Want 对象
    ErrCode MakeWantFromCmd(const std::vector<std::string>& args, Want& want, int32_t& userId);

    // 构建 argv 数组
    char **BuildArgv(const std::vector<std::string>& args, int& argc);

    // 处理基本选项
    bool HandleBasicOption(int option, char* optarg, ParseContext& context, int32_t& userId);

    // 处理参数选项（需要额外值）
    bool HandleParameterOption(int option, int argc, char **argv, ParseContext& context);

    // 处理窗口选项
    void HandleWindowOption(int option, char* optarg, ParseContext& context);

    // 解析选项到上下文
    ErrCode ParseOptions(int argc, char **argv, ParseContext& context, int32_t& userId);

    // 构建Want对象
    void BuildWantObject(const ParseContext& context, Want& want);

    // 设置窗口属性
    void SetWindowProperties(const ParseContext& context, Want& want);

    // 启动 Ability
    ErrCode StartAbility(const Want& want, int32_t userId);

    // 带等待启动 Ability
    ErrCode StartAbilityWithWait(const Want& want, int32_t userId);

    // 参数设置辅助函数
    void SetParams(const ParametersInteger& pi, Want& want);
    void SetParams(const ParametersString& ps, Want& want);
    void SetParams(const ParametersBool& pb, Want& want);

    // 输出标准化事件
    void EmitResult(const std::string& status, const std::string& message);
    void EmitStdout(const std::string& message);
    void ShowHelp();

    // 错误码转消息
    std::string GetMessageFromCode(ErrCode code);

    // 标志位
    bool waitFlag_ = false;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_AA_START_COMMAND_H
