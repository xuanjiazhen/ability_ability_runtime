/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#ifndef OHOS_ABILITY_RUNTIME_COMMAND_PERMISSION_H
#define OHOS_ABILITY_RUNTIME_COMMAND_PERMISSION_H

#include <string>
#include <vector>
#include <parcel.h>
#include "errors.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief 命令标识结构
 */
struct Command : public Parcelable {
    std::string toolName;    // 主命令名称
    std::string subCommand;  // 子命令名称，空字符串表示仅查询主命令

    Command() = default;
    ~Command() = default;

    bool Marshalling(Parcel &parcel) const override;
    static Command *Unmarshalling(Parcel &parcel);
};

/**
 * @brief 命令权限查询结果状态码
 */
enum QueryResultCode : int32_t {
    QUERY_SUCCESS = 0,              // 查询成功
    QUERY_COMMAND_NOT_EXIST = 1,    // 命令不存在
    QUERY_DB_ERROR = 2,             // 数据库查询失败
};

/**
 * @brief 命令权限查询结果
 */
struct CommandPermission : public Parcelable {
    Command cmd;                           // 查询的命令
    std::vector<std::string> permissions;  // 权限列表
    int32_t queryRet;                      // 查询结果状态码

    CommandPermission() : queryRet(QUERY_COMMAND_NOT_EXIST) {}
    ~CommandPermission() = default;

    bool Marshalling(Parcel &parcel) const override;
    static CommandPermission *Unmarshalling(Parcel &parcel);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_COMMAND_PERMISSION_H
