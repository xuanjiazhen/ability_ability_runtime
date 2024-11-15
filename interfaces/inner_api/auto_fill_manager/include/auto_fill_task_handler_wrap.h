/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_FILL_TASK_HANDLER_WRAP_H
#define OHOS_ABILITY_RUNTIME_AUTO_FILL_TASK_HANDLER_WRAP_H

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <atomic>

namespace ffrt {
class mutex;
};

namespace OHOS {
namespace AAFwk {
class AutoFillTaskHandlerWrap;
class AutoFillInnerTaskHandle;
class AutoFillTaskHandle {
friend class AutoFillTaskHandlerWrap;
public:
    AutoFillTaskHandle() = default;
    AutoFillTaskHandle(std::shared_ptr<AutoFillTaskHandlerWrap> handler,
        std::shared_ptr<AutoFillInnerTaskHandle> InnerTaskHandle) :
        handler_(handler), innerTaskHandle_(InnerTaskHandle){}
    bool Cancel() const;
    int32_t GetTaskId() const
    {
        return taskId_;
    }
    bool PrintTaskLog() const
    {
        return printTaskLog_;
    }
private:
    std::weak_ptr<AutoFillTaskHandlerWrap> handler_;
    std::shared_ptr<AutoFillInnerTaskHandle> innerTaskHandle_;
    int32_t taskId_ = 0;
    bool printTaskLog_ = true;
};

class AutoFillTaskHandlerWrap : public std::enable_shared_from_this<AutoFillTaskHandlerWrap> {
friend class AutoFillTaskHandle;
public:

    static std::shared_ptr<AutoFillTaskHandlerWrap> GetFfrtHandler();

    AutoFillTaskHandlerWrap(AutoFillTaskHandlerWrap &) = delete;
    void operator=(AutoFillTaskHandlerWrap &) = delete;
    virtual ~AutoFillTaskHandlerWrap();
    /**
     * Submit task to be scheduled and executed
     * @return AutoFillTaskHandle, could be used later
     */
    AutoFillTaskHandle SubmitTask(const std::function<void()> &task,
        const std::string &name, int64_t &delayMillis);
    // This is only used for compatibility and could be be wrong if multi tasks with same name submitted.
    // TaskHandle::Cancel is preferred.
    bool CancelTask(const std::string &name);
protected:
    AutoFillTaskHandlerWrap();
    virtual std::shared_ptr<AutoFillInnerTaskHandle> SubmitTaskInner(std::function<void()> &&task,
        const std::string &taskName, const int64_t &delayMillis) = 0;
    virtual bool CancelTaskInner(const std::shared_ptr<AutoFillInnerTaskHandle> &taskHandle) = 0;
    virtual void WaitTaskInner(const std::shared_ptr<AutoFillInnerTaskHandle> &taskHandle) = 0;
    bool RemoveTask(const std::string &name, const AutoFillTaskHandle &taskHandle);
protected:
    static std::atomic_int32_t g_taskId;

    // this is used only for compatibility
    std::unordered_map<std::string, AutoFillTaskHandle> tasks_;
    std::unique_ptr<ffrt::mutex> tasksMutex_;
    bool printTaskLog_ = true;
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_TASK_HANDLER_WRAP_H