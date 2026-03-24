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

#ifndef CLI_MANAGER_ERROR_H
#define CLI_MANAGER_ERROR_H

namespace OHOS {
namespace CliMgr {

/**
 * @brief CLI Manager 错误码定义
 */
namespace ErrCode {
    constexpr int32_t OK = 0;                          // 成功
    constexpr int32_t ERR_NOT_FOUND = 1;               // 工具不存在
    constexpr int32_t ERR_PERMISSION_DENIED = 2;       // 权限不足
    constexpr int32_t ERR_INVALID_ARGS = 3;            // 参数无效
    constexpr int32_t ERR_TIMEOUT = 4;                 // 执行超时
    constexpr int32_t ERR_PROCESS_CRASH = 5;           // 进程崩溃
    constexpr int32_t ERR_SERVICE_NOT_READY = 6;       // 服务未就绪
    constexpr int32_t ERR_TOO_MANY_TASKS = 7;          // 任务数过多
    constexpr int32_t ERR_INVALID_SCHEMA = 8;          // 描述文件格式错误
    constexpr int32_t ERR_ALREADY_EXISTS = 9;          // 工具已存在
    constexpr int32_t ERR_TASK_NOT_FOUND = 10;         // 任务不存在
    constexpr int32_t ERR_TASK_CANCELLED = 11;         // 任务已取消
    constexpr int32_t ERR_NO_MEMORY = 12;              // 内存不足
    constexpr int32_t ERR_IO_ERROR = 13;               // I/O 错误
    constexpr int32_t ERR_IPC_ERROR = 14;              // IPC 错误
    constexpr int32_t ERR_UNKNOWN = 255;               // 未知错误
}

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_MANAGER_ERROR_H
