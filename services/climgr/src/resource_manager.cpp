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

#include "resource_manager.h"
#include "hilog_tag_wrapper.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <cstring>

namespace OHOS {
namespace CliMgr {

ResourceManager::ResourceManager()
{
}

bool ResourceManager::Initialize()
{
    // 设置默认全局限制
    SetGlobalLimit(ResourceType::MEMORY, DEFAULT_MEMORY_LIMIT, DEFAULT_MEMORY_LIMIT);
    SetGlobalLimit(ResourceType::CPU, DEFAULT_CPU_LIMIT * 0.9, DEFAULT_CPU_LIMIT);
    SetGlobalLimit(ResourceType::FILE_DESCRIPTOR, DEFAULT_FD_LIMIT * 0.8, DEFAULT_FD_LIMIT);

    TAG_LOGI(TAG, "ResourceManager initialized with default limits");
    return true;
}

void ResourceManager::SetGlobalLimit(ResourceType type,
                                    uint64_t softLimit,
                                    uint64_t hardLimit)
{
    std::lock_guard<std::mutex> lock(globalLimitsMutex_);

    ResourceLimit limit;
    limit.type = type;
    limit.softLimit = softLimit;
    limit.hardLimit = hardLimit;

    if (type == ResourceType::MEMORY) {
        limit.unit = "bytes";
    } else if (type == ResourceType::CPU) {
        limit.unit = "percent";
    } else if (type == ResourceType::FILE_DESCRIPTOR) {
        limit.unit = "count";
    }

    globalLimits_[type] = limit;

    TAG_LOGI(TAG, "Global limit set: type=%{public}d, soft=%{public}llu, hard=%{public}llu",
             static_cast<int>(type), softLimit, hardLimit);
}

void ResourceManager::SetAppLimit(const std::string& bundleName,
                                ResourceType type,
                                uint64_t limit)
{
    std::lock_guard<std::mutex> lock(appLimitsMutex_);
    appLimits_[bundleName][type] = limit;

    TAG_LOGI(TAG, "App limit set: app=%{public}s, type=%{public}d, limit=%{public}llu",
             bundleName.c_str(), static_cast<int>(type), limit);
}

void ResourceManager::RegisterTask(const std::string& taskId,
                                 pid_t pid,
                                 const std::string& bundleName)
{
    std::lock_guard<std::mutex> lock(taskResourcesMutex_);

    TaskResourceInfo info;
    info.taskId = taskId;
    info.pid = pid;
    info.bundleName = bundleName;
    info.memoryUsage = 0;
    info.cpuUsage = 0;
    info.fdCount = 0;
    info.peakMemory = 0;
    info.peakCpu = 0;
    info.peakFdCount = 0;

    taskResources_[taskId] = info;

    // 更新应用进程计数
    {
        std::lock_guard<std::mutex> appLock(appProcessMutex_);
        appProcessCounts_[bundleName]++;
    }

    TAG_LOGI(TAG, "Task registered: %{public}s, pid=%{public}d, app=%{public}s",
             taskId.c_str(), pid, bundleName.c_str());
}

void ResourceManager::UnregisterTask(const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(taskResourcesMutex_);

    auto it = taskResources_.find(taskId);
    if (it != taskResources_.end()) {
        std::string bundleName = it->second.bundleName;
        taskResources_.erase(it);

        // 更新应用进程计数
        {
            std::lock_guard<std::mutex> appLock(appProcessMutex_);
            auto appIt = appProcessCounts_.find(bundleName);
            if (appIt != appProcessCounts_.end()) {
                appIt->second--;
                if (appIt->second <= 0) {
                    appProcessCounts_.erase(appIt);
                }
            }
        }

        TAG_LOGI(TAG, "Task unregistered: %{public}s", taskId.c_str());
    }
}

bool ResourceManager::CheckResourceLimits(const std::string& taskId)
{
    UpdateResourceUsage(taskId);

    std::lock_guard<std::mutex> lock(taskResourcesMutex_);
    auto it = taskResources_.find(taskId);
    if (it == taskResources_.end()) {
        return false;
    }

    const auto& info = it->second;

    // 检查全局限制
    {
        std::lock_guard<std::mutex> limitLock(globalLimitsMutex_);

        auto memIt = globalLimits_.find(ResourceType::MEMORY);
        if (memIt != globalLimits_.end()) {
            if (info.memoryUsage > memIt->second.hardLimit) {
                TAG_LOGW(TAG, "Memory limit exceeded: task=%{public}s, usage=%{public}llu, limit=%{public}llu",
                         taskId.c_str(), info.memoryUsage, memIt->second.hardLimit);
                return false;
            }
        }

        auto cpuIt = globalLimits_.find(ResourceType::CPU);
        if (cpuIt != globalLimits_.end()) {
            if (info.cpuUsage > cpuIt->second.hardLimit) {
                TAG_LOGW(TAG, "CPU limit exceeded: task=%{public}s, usage=%{public}llu, limit=%{public}llu",
                         taskId.c_str(), info.cpuUsage, cpuIt->second.hardLimit);
                return false;
            }
        }

        auto fdIt = globalLimits_.find(ResourceType::FILE_DESCRIPTOR);
        if (fdIt != globalLimits_.end()) {
            if (info.fdCount > fdIt->second.hardLimit) {
                TAG_LOGW(TAG, "FD limit exceeded: task=%{public}s, usage=%{public}llu, limit=%{public}llu",
                         taskId.c_str(), info.fdCount, fdIt->second.hardLimit);
                return false;
            }
        }
    }

    return true;
}

bool ResourceManager::GetResourceUsage(const std::string& taskId, ResourceUsage& usage)
{
    std::lock_guard<std::mutex> lock(taskResourcesMutex_);
    auto it = taskResources_.find(taskId);
    if (it == taskResources_.end()) {
        return false;
    }

    const auto& info = it->second;

    // 返回内存使用情况
    usage.taskId = taskId;
    usage.type = ResourceType::MEMORY;
    usage.currentUsage = info.memoryUsage;
    usage.peakUsage = info.peakMemory;
    usage.exceededSoftLimit = false;
    usage.exceededHardLimit = false;

    // 检查软限制
    {
        std::lock_guard<std::mutex> limitLock(globalLimitsMutex_);
        auto memIt = globalLimits_.find(ResourceType::MEMORY);
        if (memIt != globalLimits_.end()) {
            usage.exceededSoftLimit = info.memoryUsage > memIt->second.softLimit;
            usage.exceededHardLimit = info.memoryUsage > memIt->second.hardLimit;
        }
    }

    return true;
}

void ResourceManager::UpdateResourceUsage(const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(taskResourcesMutex_);
    auto it = taskResources_.find(taskId);
    if (it == taskResources_.end()) {
        return;
    }

    auto& info = it->second;

    // 读取进程资源使用
    info.memoryUsage = ReadProcessMemory(info.pid);
    info.cpuUsage = ReadProcessCpu(info.pid);
    info.fdCount = ReadProcessFdCount(info.pid);

    // 更新峰值
    if (info.memoryUsage > info.peakMemory) {
        info.peakMemory = info.memoryUsage;
    }
    if (info.cpuUsage > info.peakCpu) {
        info.peakCpu = info.cpuUsage;
    }
    if (info.fdCount > info.peakFdCount) {
        info.peakFdCount = info.fdCount;
    }

    TAG_LOGD(TAG, "Resource usage updated: task=%{public}s, mem=%{public}llu, cpu=%{public}llu, fd=%{public}llu",
             taskId.c_str(), info.memoryUsage, info.cpuUsage, info.fdCount);
}

bool ResourceManager::CanCreateTask(const std::string& bundleName)
{
    // 检查全局进程数限制
    {
        std::lock_guard<std::mutex> lock(taskResourcesMutex_);
        if (taskResources_.size() >= GLOBAL_PROCESS_LIMIT) {
            TAG_LOGW(TAG, "Global process limit reached: %{public}d", GLOBAL_PROCESS_LIMIT);
            return false;
        }
    }

    // 检查应用进程数限制
    {
        std::lock_guard<std::mutex> lock(appProcessMutex_);
        auto it = appProcessCounts_.find(bundleName);
        if (it != appProcessCounts_.end() && it->second >= DEFAULT_APP_PROCESS_LIMIT) {
            TAG_LOGW(TAG, "App process limit reached: app=%{public}s, count=%{public}d",
                     bundleName.c_str(), it->second);
            return false;
        }
    }

    return true;
}

uint64_t ResourceManager::ReadProcessMemory(pid_t pid)
{
    // 读取 /proc/[pid]/status 文件
    std::string path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream file(path);

    if (!file.is_open()) {
        return 0;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("VmRSS:") == 0) {
            // VmRSS: xxx kB
            std::istringstream iss(line);
            std::string label;
            uint64_t value;
            std::string unit;
            iss >> label >> value >> unit;

            // 转换为字节
            return value * 1024;
        }
    }

    return 0;
}

uint64_t ResourceManager::ReadProcessCpu(pid_t pid)
{
    // 读取 /proc/[pid]/stat 文件
    std::string path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream file(path);

    if (!file.is_open()) {
        return 0;
    }

    // stat 文件格式很复杂，这里简化实现
    // 实际应该读取 utime 和 stime，然后计算 CPU 使用率
    // 这里返回一个模拟值
    return 0;
}

uint64_t ResourceManager::ReadProcessFdCount(pid_t pid)
{
    // 计算 /proc/[pid]/fd 目录下的文件数
    std::string path = "/proc/" + std::to_string(pid) + "/fd";
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        return 0;
    }

    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            count++;
        }
    }

    closedir(dir);
    return count;
}

} // namespace CliMgr
} // namespace OHOS
