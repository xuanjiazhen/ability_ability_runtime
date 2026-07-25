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

#include "app_pidfd_manager.h"

#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "c/executor_task.h"
#include "ffrt.h"
#include "ffrt_inner.h"

#include "app_mgr_service_inner.h"
#include "hilog_tag_wrapper.h"
#include "task_handler_wrap.h"

#ifndef PIDFD_NONBLOCK
#define PIDFD_NONBLOCK O_NONBLOCK
#endif

namespace {
const char *TASK_ON_CHILD_PROCESS_EXITED = "OnChildProcessExitedTask";
} // namespace

namespace OHOS {
namespace AppExecFwk {

AppPidFdManager &AppPidFdManager::GetInstance()
{
    static AppPidFdManager manager;
    return manager;
}

AppPidFdManager::AppPidFdManager() {}

AppPidFdManager::~AppPidFdManager()
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    for (auto &item : pidfdMap_) {
        if (item.second.pidfd >= 0) {
            ffrt_qos_t qos = 0;
            ffrt_epoll_ctl(qos, EPOLL_CTL_DEL, item.second.pidfd, 0, nullptr, nullptr);
            close(item.second.pidfd);
        }
        if (item.second.pidToken != nullptr) {
            free(item.second.pidToken);
            item.second.pidToken = nullptr;
        }
    }
    pidfdMap_.clear();
}

void AppPidFdManager::Init(std::weak_ptr<AppMgrServiceInner> appMgrServiceInner,
    std::weak_ptr<AAFwk::TaskHandlerWrap> taskHandler)
{
    appMgrServiceInner_ = std::move(appMgrServiceInner);
    taskHandler_ = std::move(taskHandler);
    TAG_LOGI(AAFwkTag::APPMGR, "AppPidFdManager init");
}

int AppPidFdManager::OpenPidFd(pid_t pid, unsigned int flags)
{
    return static_cast<int>(syscall(SYS_pidfd_open, pid, flags));
}

void AppPidFdManager::AddWatcher(pid_t pid)
{
    if (pid <= 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "invalid pid");
        return;
    }
    int pidfd = OpenPidFd(pid, PIDFD_NONBLOCK);
    if (pidfd < 0) {
        // The process has already exited (ESRCH) before we could open the pidfd.
        // Treat it as a death and dispatch the cleanup directly.
        TAG_LOGW(AAFwkTag::APPMGR, "pidfd_open failed, pid:%{public}d, errno:%{public}d", pid, errno);
        DispatchCleanup(pid);
        return;
    }

    int32_t *token = static_cast<int32_t *>(malloc(sizeof(int32_t)));
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "malloc pid token failed, pid:%{public}d", pid);
        close(pidfd);
        return;
    }
    *token = static_cast<int32_t>(pid);

    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto it = pidfdMap_.find(pid);
        if (it != pidfdMap_.end()) {
            // Should not happen; replace the stale entry defensively.
            ffrt_qos_t qos = 0;
            ffrt_epoll_ctl(qos, EPOLL_CTL_DEL, it->second.pidfd, 0, nullptr, nullptr);
            close(it->second.pidfd);
            free(it->second.pidToken);
            it->second.pidfd = pidfd;
            it->second.pidToken = token;
        } else {
            pidfdMap_.emplace(pid, PidFdEntry{pidfd, token});
        }
    }

    ffrt_qos_t qos = 0;
    int ret = ffrt_epoll_ctl(qos, EPOLL_CTL_ADD, pidfd, EPOLLIN, token, &AppPidFdManager::OnPidfdEvent);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "ffrt_epoll_ctl add failed, pid:%{public}d, ret:%{public}d", pid, ret);
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto it = pidfdMap_.find(pid);
        if (it != pidfdMap_.end() && it->second.pidfd == pidfd) {
            free(it->second.pidToken);
            pidfdMap_.erase(it);
        }
        close(pidfd);
        DispatchCleanup(pid);
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "watch process pid:%{public}d, pidfd:%{public}d", pid, pidfd);
}

void AppPidFdManager::OnPidfdEvent(void *data, uint32_t event)
{
    if (data == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null pid token");
        return;
    }
    pid_t pid = static_cast<pid_t>(*static_cast<int32_t *>(data));
    TAG_LOGI(AAFwkTag::APPMGR, "OnPidfdEvent pid:%{public}d, event:%{public}u", pid, event);
    GetInstance().OnPidfdFired(pid);
}

void AppPidFdManager::OnPidfdFired(pid_t pid)
{
    // fd/token lifecycle is handled here on the poller thread, so it never races
    // with a free from another thread (mirrors appspawn ClosePidfdWatcher).
    int pidfd = -1;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto it = pidfdMap_.find(pid);
        if (it == pidfdMap_.end()) {
            return;
        }
        pidfd = it->second.pidfd;
        if (it->second.pidToken != nullptr) {
            free(it->second.pidToken);
            it->second.pidToken = nullptr;
        }
        pidfdMap_.erase(it);
    }
    if (pidfd >= 0) {
        ffrt_qos_t qos = 0;
        ffrt_epoll_ctl(qos, EPOLL_CTL_DEL, pidfd, 0, nullptr, nullptr);
        close(pidfd);
    }
    DispatchCleanup(pid);
}

void AppPidFdManager::DispatchCleanup(pid_t pid)
{
    auto inner = appMgrServiceInner_.lock();
    auto handler = taskHandler_.lock();
    if (!inner || !handler) {
        TAG_LOGE(AAFwkTag::APPMGR, "null inner or handler, pid:%{public}d", pid);
        return;
    }
    auto task = [inner, pid]() { inner->OnChildProcessExited(pid); };
    handler->SubmitTask(task, AAFwk::TaskAttribute{
        .taskName_ = TASK_ON_CHILD_PROCESS_EXITED,
        .taskQos_ = AAFwk::TaskQoS::USER_INTERACTIVE
    });
}
} // namespace AppExecFwk
} // namespace OHOS
