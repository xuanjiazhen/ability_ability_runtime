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

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include "cli_execution_context.h"

namespace OHOS {
namespace CliMgr {

/**
 * @brief 资源类型
 */
enum class ResourceType : uint8_t {
    MEMORY = 0,      // 内存
    CPU = 1,         // CPU
    FILE_DESCRIPTOR = 2,  // 文件描述符
    PROCESS_COUNT = 3,    // 进程数量
};

/**
 * @brief 资源限制配置
 */
struct ResourceLimit {
    ResourceType type;
    uint64_t softLimit;      // 软限制（警告阈值）
    uint64_t hardLimit;      // 硬限制（强制终止）
    const char* unit;

    ResourceLimit()
        : type(ResourceType::MEMORY),
          softLimit(0),
          hardLimit(0),
          unit("bytes")
    {}
};

/**
 * @brief 资源使用情况
 */
struct ResourceUsage {
    std::string taskId;
    ResourceType type;
    uint64_t currentUsage;
    uint64_t peakUsage;
    bool exceededSoftLimit;
    bool exceededHardLimit;
};

/**
 * @brief 资源管理器
 *
 * 管理所有 CLI 工具进程的资源使用
 */
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager() = default;

    /**
     * @brief 初始化
     * @return 是否成功
     */
    bool Initialize();

    /**
     * @brief 设置全局资源限制
     * @param type 资源类型
     * @param softLimit 软限制
     * @param hardLimit 硬限制
     */
    void SetGlobalLimit(ResourceType type,
                       uint64_t softLimit,
                       uint64_t hardLimit);

    /**
     * @brief 设置单个应用的资源限制
     * @param bundleName 应用包名
     * @param type 资源类型
     * @param limit 限制值
     */
    void SetAppLimit(const std::string& bundleName,
                    ResourceType type,
                    uint64_t limit);

    /**
     * @brief 注册任务资源跟踪
     * @param taskId 任务 ID
     * @param pid 进程 ID
     * @param bundleName 应用包名
     */
    void RegisterTask(const std::string& taskId,
                     pid_t pid,
                     const std::string& bundleName);

    /**
     * @brief 注销任务
     * @param taskId 任务 ID
     */
    void UnregisterTask(const std::string& taskId);

    /**
     * @brief 检查资源是否超出限制
     * @param taskId 任务 ID
     * @return 是否超出限制
     */
    bool CheckResourceLimits(const std::string& taskId);

    /**
     * @brief 获取资源使用情况
     * @param taskId 任务 ID
     * @param usage 输出参数，资源使用情况
     * @return 是否成功
     */
    bool GetResourceUsage(const std::string& taskId, ResourceUsage& usage);

    /**
     * @brief 更新资源使用情况
     * @param taskId 任务 ID
     */
    void UpdateResourceUsage(const std::string& taskId);

    /**
     * @brief 检查是否可以创建新任务
     * @param bundleName 应用包名
     * @return 是否允许
     */
    bool CanCreateTask(const std::string& bundleName);

private:
    /**
     * @brief 读取进程内存使用
     * @param pid 进程 ID
     * @return 内存使用量（字节）
     */
    uint64_t ReadProcessMemory(pid_t pid);

    /**
     * @brief 读取进程 CPU 使用率
     * @param pid 进程 ID
     * @return CPU 使用率（百分比）
     */
    uint64_t ReadProcessCpu(pid_t pid);

    /**
     * @brief 读取进程文件描述符数量
     * @param pid 进程 ID
     * @return 文件描述符数量
     */
    uint64_t ReadProcessFdCount(pid_t pid);

    /**
     * @brief 检查应用进程数量限制
     * @param bundleName 应用包名
     * @param count 当前进程数
     * @return 是否超出限制
     */
    bool CheckAppProcessLimit(const std::string& bundleName, int count);

private:
    // 全局资源限制
    std::unordered_map<ResourceType, ResourceLimit> globalLimits_;
    std::mutex globalLimitsMutex_;

    // 应用级别限制
    std::unordered_map<std::string, std::unordered_map<ResourceType, uint64_t>> appLimits_;
    std::mutex appLimitsMutex_;

    // 任务资源跟踪
    struct TaskResourceInfo {
        std::string taskId;
        pid_t pid;
        std::string bundleName;
        uint64_t memoryUsage;
        uint64_t cpuUsage;
        uint64_t fdCount;
        uint64_t peakMemory;
        uint64_t peakCpu;
        uint64_t peakFdCount;
    };

    std::unordered_map<std::string, TaskResourceInfo> taskResources_;
    std::mutex taskResourcesMutex_;

    // 应用进程计数
    std::unordered_map<std::string, int> appProcessCounts_;
    std::mutex appProcessMutex_;

    // 默认限制
    static constexpr uint64_t DEFAULT_MEMORY_LIMIT = 512 * 1024 * 1024;  // 512MB
    static constexpr uint64_t DEFAULT_CPU_LIMIT = 80;                   // 80%
    static constexpr uint64_t DEFAULT_FD_LIMIT = 1024;                  // 1024
    static constexpr int DEFAULT_APP_PROCESS_LIMIT = 10;                // 10
    static constexpr int GLOBAL_PROCESS_LIMIT = 100;                    // 100

    static constexpr const char* TAG = "ResourceManager";
};

} // namespace CliMgr
} // namespace OHOS

#endif // RESOURCE_MANAGER_H
