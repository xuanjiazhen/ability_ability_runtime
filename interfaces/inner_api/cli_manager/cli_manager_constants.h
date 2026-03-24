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

#ifndef CLI_MANAGER_CONSTANTS_H
#define CLI_MANAGER_CONSTANTS_H

#include <cstdint>

namespace OHOS {
namespace CliMgr {

// SystemAbility ID
constexpr int32_t SA_CLIMGR_ID = 9001;

// 服务描述符
constexpr const char* CLI_MANAGER_SERVICE = "OHOS.CliMgr.ICLIManagerService";
constexpr const char* CLI_CALLBACK = "OHOS.CliMgr.ICLICallback";

// 系统能力名称
constexpr const char* CLIMGR_SERVICE_NAME = "CLIManagerService";

// 默认值
constexpr uint32_t DEFAULT_TIMEOUT_MS = 30000;         // 30 秒
constexpr uint32_t DEFAULT_TIMEOUT_SHORT_MS = 5000;   // 5 秒
constexpr uint32_t DEFAULT_TIMEOUT_LONG_MS = 300000;   // 5 分钟

constexpr uint32_t MAX_CONCURRENT_TASKS = 100;        // 最大并发任务数
constexpr uint32_t MAX_APP_TASKS = 10;                // 单应用最大任务数

// 资源限制
constexpr uint64_t DEFAULT_MEMORY_LIMIT = 512 * 1024 * 1024;  // 512MB
constexpr uint64_t DEFAULT_CPU_LIMIT = 80;                 // 80%
constexpr uint64_t DEFAULT_FD_LIMIT = 1024;                // 1024 个文件描述符

// 监控间隔
constexpr uint32_t TIMEOUT_CHECK_INTERVAL_MS = 100;    // 超时检查间隔
constexpr uint32_t PROCESS_MONITOR_INTERVAL_MS = 50;   // 进程监控检查间隔
constexpr uint32_t RESOURCE_UPDATE_INTERVAL_MS = 1000;  // 资源更新间隔（1秒）

// 管道路径
constexpr const char* PIPE_BUFFER_SIZE = "64KB";
constexpr int PIPE_READ_FD = 0;
constexpr int PIPE_WRITE_FD = 1;

// 信号
constexpr int SIGTERM_TIMEOUT_SEC = 3;  // SIGTERM 到 SIGKILL 的等待时间（秒）

// 日志标签
constexpr const char* TAG = "CliMgr";

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_MANAGER_CONSTANTS_H
