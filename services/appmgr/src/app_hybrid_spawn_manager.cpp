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

#include "app_hybrid_spawn_manager.h"

#include <nlohmann/json.hpp>
#include <sys/epoll.h>

#include "app_exit_reason_data_manager.h"
#include "appspawn.h"
#include "c/executor_task.h"
#include "ffrt.h"
#include "ffrt_inner.h"
#include "syspara/parameter.h"

namespace OHOS {
namespace AppExecFwk {

namespace {
//listen fd use
constexpr int32_t PIPE_MSG_READ_BUFFER = 1024;
constexpr const char* HYBRIDSPAWN_EXIT = "startup.service.ctl.hybridspawn";
constexpr const char* HYBRIDSPAWN_EXIT_SIGNAL = "5";
}

AppHybridSpawnManager::~AppHybridSpawnManager() {}

AppHybridSpawnManager::AppHybridSpawnManager() {}

AppHybridSpawnManager &AppHybridSpawnManager::GetInstance()
{
    static AppHybridSpawnManager manager;
    return manager;
}

static void HybridSpawnExitCallback(const char *key, const char *value, void *context)
{
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "value nullptr");
        return;
    }
    TAG_LOGI(AAFwkTag::APPMGR, "value is: %{public}s", value);
    // set flag
    if (strcmp(value, HYBRIDSPAWN_EXIT_SIGNAL) == 0) {
        int ret = HybridSpawnListenCloseSet();
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::APPMGR, "HybridSpawnListenCloseSet failed");
        }
    }
}

static void ProcessSignalData(void *token, uint32_t event)
{
    int rFd = *(reinterpret_cast<int*>(token));
    // read data from appspawn
    char buffer[PIPE_MSG_READ_BUFFER] = {0};
    std::string readResult = "";
    int count = read(rFd, buffer, PIPE_MSG_READ_BUFFER - 1);
    if (count == -1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read pipe failed");
    } else if (count == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write end closed");
        close(rFd);
    } else {
        int32_t pid = -1;
        int32_t signal = -1;
        int32_t uid = 0;
        std::string bundleName = "";
        std::string bufferStr = buffer;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "buffer read: %{public}s", bufferStr.c_str());
        nlohmann::json jsonObject = nlohmann::json::parse(bufferStr, nullptr, false);
        if (jsonObject.is_discarded()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "parse json string failed");
            return;
        }
        if (!jsonObject.contains("pid") || !jsonObject.contains("signal") || !jsonObject.contains("uid")
            || !jsonObject.contains("bundleName")) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "info lost!");
            return;
        }
        pid = jsonObject["pid"];
        signal = jsonObject["signal"];
        uid = jsonObject["uid"];
        bundleName = jsonObject["bundleName"];
        if (signal == 0) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "ignore signal 0, pid: %{public}d", pid);
            return;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "To update reason detail info because of SIGNAL");
        if(DelayedSingleton<AbilityRuntime::AppExitReasonDataManager>::GetInstance()->RecordSignalReason(
            pid, uid, signal, bundleName) != 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "RecordSignalReason failed");
        }
    }
}

void AppHybridSpawnManager::InitHybridSpawnMsgPipe()
{
    int pipeFd[2];
    if (pipe(pipeFd) != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create pipe failed");
        return;
    }
    hrFd_ = pipeFd[0];
    hwFd_ = pipeFd[1];
	*ptrHRFd_ = hrFd_;
	TAG_LOGI(AAFwkTag::ABILITYMGR, "hrFd is: %{public}d, hwFd is: %{public}d", hrFd_, hwFd_);

    // send fd
    int ret = HybridSpawnListenFdSet(hwFd_);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "send fd to hybridspawn failed, ret: %{public}d", ret);
        close(hrFd_);
        close(hwFd_);
        return;
    }
    ret = WatchParameter(HYBRIDSPAWN_EXIT, HybridSpawnExitCallback, nullptr);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "watch parameter ret: %{public}d", ret);
		close(hrFd_);
        close(hwFd_);
        return;
    }
    ffrt_qos_t taskQos = 0;
    ret = ffrt_epoll_ctl(taskQos, EPOLL_CTL_ADD, hrFd_, EPOLLIN, static_cast<void*>(ptrHRFd_.get()), ProcessSignalData);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ffrt_epoll_ctl failed, ret: %{public}d", ret);
        close(hrFd_);
		close(hwFd_);
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Listen signal msg ...");
}
} // end AppExecFwk
} // end OHOS