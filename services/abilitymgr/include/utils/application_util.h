/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_UTIL_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_UTIL_H

#include "appspawn.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "hilog_tag_wrapper.h"
#include "parameters.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace ApplicationUtil {
using Want = OHOS::AAFwk::Want;
constexpr int32_t BOOT_COMPLETED_SIZE = 6;
constexpr const char* BOOTEVENT_BOOT_COMPLETED = "bootevent.boot.completed";
constexpr const char* ENABLE_APP_GALLERY_SELECTOR_UTIL = "abilitymanagerservice.support.appgallery.selector";
constexpr int32_t PIPE_MSG_READ_BUFFER = 1024;

[[maybe_unused]] static void AppFwkBootEventCallback(const char *key, const char *value, void *context)
{
    if (strcmp(key, "bootevent.boot.completed") == 0 && strcmp(value, "true") == 0) {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED);
        EventFwk::CommonEventData commonData {want};
        TAG_LOGI(AAFwkTag::ABILITYMGR, "init call, AppFwkBootEventCallback");
        EventFwk::CommonEventManager::PublishCommonEvent(commonData);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "init call, AppFwkBootEventCallback push BootEvent");
        return;
    }
    TAG_LOGE(AAFwkTag::ABILITYMGR, "init call, AppFwkBootEventCallback key: %{public}s, value: %{public}s", key, value);
}

static void ProcessSignalData(void *token, uint32_t event)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ProcessSignalData called");
    int rFd = *(reinterpret_cast<int*>(token));
    // read data from appspawn
    char buffer[PIPE_MSG_READ_BUFFER] = {0};

    int count = read(rFd, buffer, PIPE_MSG_READ_BUFFER);
    if (count == -1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read pipe failed");
        return;
    } else if (count == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write end closed");
        close(rFd);
        return;
    } else {
        int32_t pid = -1;
        int32_t signal = -1;
        std::string bufferStr = buffer;
        TAG_LOGI(AAFwkTag::ABILITYMGR, "buffer read: %{public}s", bufferStr.c_str());

        // {"pid":xxx, "signal":xxx}
        nlohmann::json jsonObject = nlohmann::json::parse(bufferStr, nullptr, false);
        if (jsonObject.is_discarded()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "parse json string failed");
            return;
        }
        if (!jsonObject.contains("pid") || !jsonObject.contains("signal")) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "pid or signal not exist!");
            return;
        }
        pid = jsonObject["pid"];
        signal = jsonObject["signal"];
        TAG_LOGI(AAFwkTag::ABILITYMGR, "pid: %{public}d, signal: %{public}d", pid, signal);

        // // record reason
        // std::shared_ptr<AppExitReasonHelper> appExitReasonHelper = std::make_shared<AppExitReasonHelper>(subManagersHelper_);
        // CHECK_POINTER_AND_RETURN(appExitReasonHelper, ERR_NULL_OBJECT);
        // std::string exitMsg = "signal is " + std::to_string(signal);
        // AAFwk::ExitReason exitReason = { REASON_SIGNAL, exitMsg };
        // auto ret = appExitReasonHelper->RecordAppExitReason(exitReason, pid);
        // if (ret != 0) {
        //     TAG_LOGE(AAFwkTag::ABILITYMGR, "RecordAppExitReason failed, ret: %{public}s", ret);
        //     return;
        // }
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ProcessSignalData success");
}

static void StartAppSpawnPipeLink()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartAppSpawnPipeLink called");
    int pipeFd[2];
    if (pipe(pipeFd) != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "create pipe failed");
        return;
    }
    auto ret = 0;
    // send fd
    // int SpawnListen(int fd, const char *serviceName, const char *processName);
    ret = SpawnListen(pipeFd[1], APPSPAWN_SERVICE_NAME, "foundation");
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "send fd to appspawn failed");
        close(pipeFd[0]);
        close(pipeFd[1]);
        return;
    }
    // create pipe listen thread
    int rFd = pipeFd[0];
    ffrt_qos_t taskQos = ffrt_this_task_get_qos();
    ret = ffrt_epoll_ctl(taskQos, EPOLL_CTL_ADD, pipeFd[0], EPOLLIN, (void*)&rFd, ProcessSignalData);
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ffrt_epoll_ctl failed, ret: %{public}d", ret);
        close(rFd);
        return;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "StartAppSpawnPipeLink success");
}

static void AppSpawnStartCallback(const char *key, const char *value, void *context)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AppSpawnStartCallback called");
    if (strcmp(key, "startup.service.ctl.appspawn.pid") != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not appspawn start event!");
        return;
    }
    StartAppSpawnPipeLink();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "AppSpawnStartCallback success");
}

inline bool IsBootCompleted()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    std::string ret = OHOS::system::GetParameter(BOOTEVENT_BOOT_COMPLETED, "false");
    if (ret == "true") {
        return true;
    }
    return false;
}

inline bool IsEnableAppGallerySelector()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    std::string ret = OHOS::system::GetParameter(ENABLE_APP_GALLERY_SELECTOR_UTIL, "true");
    if (ret == "true") {
        return true;
    }
    return false;
}
}  // namespace ApplicationlUtil
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_UTIL_H
