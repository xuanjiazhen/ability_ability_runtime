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

#include "watchdog.h"

#include <charconv>
#include <parameter.h>
#include <unistd.h>

#include "app_mgr_client.h"
#include "app_recovery.h"
#include "appfreeze_inner.h"
#include "hisysevent.h"
#include "hilog_tag_wrapper.h"
#include "xcollie/watchdog.h"
#include "parameters.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr uint32_t CHECK_MAIN_THREAD_IS_ALIVE = 1;
constexpr int RESET_RATIO = 2;
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
constexpr int32_t CHARACTER_WIDTH = 2;
#endif

constexpr int32_t BACKGROUND_REPORT_COUNT_MAX = 5;
constexpr int32_t WATCHDOG_REPORT_COUNT_MAX = 5;
#ifdef SUPPORT_ASAN
constexpr uint32_t CHECK_INTERVAL_TIME = 45000;
#else
constexpr uint32_t CHECK_INTERVAL_TIME = 3000;
#endif
static uint32_t checkIntervalTime_ = 3000;
constexpr float FLOAT_EPSILON = 0.01f;
constexpr int32_t TIME_CONVERT_RATIO = 1000;
constexpr int32_t MAX_RATIO_SIZE = 4;
#ifdef APP_NO_RESPONSE_DIALOG_WEARABLE
constexpr uint32_t WEARABLE_CHECK_INTERVAL_TIME = 5000;
#endif

constexpr uint32_t BACKGROUND_INTERVAL_TIME = 20000;
}
std::shared_ptr<EventHandler> Watchdog::appMainHandler_ = nullptr;

#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
OHOS::HiviewDFX::HiTraceId* Watchdog::hitraceId_ = nullptr;
#endif

static constexpr const char* const CHECK_BACKGROUND_THREAD[] = {
    "com.ohos.sceneboard"
};

Watchdog::Watchdog()
{
    checkIntervalTime_ = CHECK_INTERVAL_TIME * getRatioValue();
}

Watchdog::~Watchdog()
{
    if (!stopWatchdog_) {
        TAG_LOGD(AAFwkTag::APPDFR, "Stop watchdog");
        OHOS::HiviewDFX::Watchdog::GetInstance().StopWatchdog();
    }
}

float Watchdog::getRatioValue()
{
    float defaultRatio = 1.0f;
    std::string ratioStr = OHOS::system::GetParameter("const.sys.dfx.appfreeze.timeout_unit_time_ratio", "1000");
    if (ratioStr.empty() || !IsNumeric(ratioStr)) {
        return defaultRatio;
    }

    if (ratioStr.size() > MAX_RATIO_SIZE) {
        return defaultRatio;
    }

    uint64_t ratioVal = 0;
    auto [ptr, ec] = std::from_chars(ratioStr.data(), ratioStr.data() + ratioStr.size(), ratioVal);
    if (ec != std::errc()) {
        return defaultRatio;
    }

    float result = (ratioVal * 1.0) / TIME_CONVERT_RATIO;
    return (result < FLOAT_EPSILON) ? defaultRatio : result;
}

bool Watchdog::IsNumeric(const std::string &str)
{
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

void Watchdog::Init(const std::shared_ptr<EventHandler> mainHandler)
{
    std::shared_ptr<EventHandler> handlerSnap;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        Watchdog::appMainHandler_ = mainHandler;
        lastWatchTime_ = 0;
        handlerSnap = appMainHandler_;
    }
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
    hitraceId_ = OHOS::HiviewDFX::HiTraceChain::GetIdAddress();
#endif
    if (handlerSnap != nullptr) {
        TAG_LOGD(AAFwkTag::APPDFR, "Watchdog init send event");
        handlerSnap->SendEvent(CHECK_MAIN_THREAD_IS_ALIVE, 0, EventQueue::Priority::VIP);
    }
    auto watchdogTask = [this] { this->Timer(); };
#ifdef APP_NO_RESPONSE_DIALOG_WEARABLE
    OHOS::HiviewDFX::Watchdog::GetInstance().RunPeriodicalTask("AppkitWatchdog", watchdogTask,
        WEARABLE_CHECK_INTERVAL_TIME, INI_TIMER_FIRST_SECOND);
#else
    OHOS::HiviewDFX::Watchdog::GetInstance().RunOneShotTask("AppkitOneShotWatchdogTask", watchdogTask,
        INI_ONE_SHOE_TASK_FIRST_SECOND);
    OHOS::HiviewDFX::Watchdog::GetInstance().RunPeriodicalTask("AppkitWatchdog", watchdogTask,
        checkIntervalTime_, INI_TIMER_FIRST_SECOND);
#endif
    SetMainThreadSample();
    SetReportLifeCycleAsAppfreeze();
}

void Watchdog::SetReportLifeCycleAsAppfreeze()
{
    char* env = getenv("DFX_APPFREEZE_LOG_OPTIONS");
    if (env == nullptr) {
        return;
    }
    AppExecFwk::AppfreezeInner::GetInstance()->SetReportLifeCycleAsAppfreeze(
        strstr(env, "report_lifecycle_as_appfreeze:enable") != nullptr);
}

void Watchdog::SetMainThreadSample()
{
    char* env = getenv("DFX_APPFREEZE_LOG_OPTIONS");
    if (env == nullptr) {
        return;
    }
    AppExecFwk::AppfreezeInner::GetInstance()->SetMainThreadSample(
        strstr(env, "mainthread_sampling:enable") != nullptr);
}

void Watchdog::Stop()
{
    TAG_LOGD(AAFwkTag::APPDFR, "called");
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (stopWatchdog_.load()) {
            TAG_LOGD(AAFwkTag::APPDFR, "stoped");
            return;
        }
        stopWatchdog_.store(true);
        if (appMainHandler_) {
            appMainHandler_.reset();
            appMainHandler_ = nullptr;
        }
    }
    OHOS::HiviewDFX::Watchdog::GetInstance().StopWatchdog();
}

void Watchdog::SetAppMainThreadState(const bool appMainThreadState)
{
    appMainThreadIsAlive_.store(appMainThreadState);
}

#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
void Watchdog::SetHiTraceChainId()
{
    OHOS::HiviewDFX::HiTraceId hitraceId = *hitraceId_;
    if (hitraceId.IsValid() == 0) {
        TAG_LOGW(AAFwkTag::APPDFR, "get hitrace id is invalid.");
        return;
    }
    OHOS::HiviewDFX::HiTraceChain::SetId(hitraceId);
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(CHARACTER_WIDTH) << hitraceId.GetChainId();
    TAG_LOGI(AAFwkTag::APPDFR, "Main thread blocked, %{public}s", ss.str().c_str());
}
#endif

#ifdef APP_NO_RESPONSE_DIALOG
bool isDeviceType2in1()
{
    const int bufferLen = 128;
    char paramOutBuf[bufferLen] = {0};
    const char *devicetype2in1 = "2in1";
    int ret = GetParameter("const.product.devicetype", "", paramOutBuf, bufferLen);
    return ret > 0 && strncmp(paramOutBuf, devicetype2in1, strlen(devicetype2in1)) == 0;
}
#endif

#ifdef APP_NO_RESPONSE_DIALOG
void Watchdog::ChangeTimeOut(const std::string& bundleName)
{
    constexpr char SCENEBOARD_SERVICE_ABILITY[] = "com.ohos.sceneboard";
    constexpr int TIMEOUT = 5000;
    if (bundleName == SCENEBOARD_SERVICE_ABILITY) {
        OHOS::HiviewDFX::Watchdog::GetInstance().RemovePeriodicalTask("AppkitWatchdog");
        auto watchdogTask = [this] { this->Timer(); };
        OHOS::HiviewDFX::Watchdog::GetInstance().RunPeriodicalTask("AppkitWatchdog", watchdogTask, TIMEOUT,
            INI_TIMER_FIRST_SECOND);
    }
}
#endif

void Watchdog::SetBundleInfo(const std::string& bundleName, const std::string& bundleVersion, bool isSystemApp)
{
    OHOS::HiviewDFX::Watchdog::GetInstance().SetSystemApp(isSystemApp);
    OHOS::HiviewDFX::Watchdog::GetInstance().SetBundleInfo(bundleName, bundleVersion);
#ifdef APP_NO_RESPONSE_DIALOG
    if (isDeviceType2in1()) {
        ChangeTimeOut(bundleName);
    }
#endif
    {
        std::unique_lock<std::mutex> lock(mutex_);
        bundleName_ = bundleName;
    }
}

void Watchdog::SetBackgroundStatus(const bool isInBackground)
{
    isInBackground_.store(isInBackground);
    OHOS::HiviewDFX::Watchdog::GetInstance().SetForeground(!isInBackground);
    AppExecFwk::AppfreezeInner::GetInstance()->SetAppInForeground(!isInBackground);
}

void Watchdog::AllowReportEvent()
{
    needReport_.store(true);
    isSixSecondEvent_.store(false);
    backgroundReportCount_.store(0);
    watchdogReportCount_.store(0);
}

bool Watchdog::IsReportEvent()
{
    if (appMainThreadIsAlive_.exchange(false)) {
        return false;
    }
    TAG_LOGD(AAFwkTag::APPDFR, "AppMainThread not alive");
    return true;
}

bool Watchdog::IsStopWatchdog()
{
    return stopWatchdog_.load();
}

void Watchdog::SetBgWorkingThreadStatus(const bool isBgWorkingThread)
{
    isBgWorkingThread_.store(isBgWorkingThread);
}

void Watchdog::Timer()
{
    if (stopWatchdog_.load()) {
        TAG_LOGD(AAFwkTag::APPDFR, "stoped");
        return;
    }

    std::shared_ptr<EventHandler> handlerSnap;
    bool shouldReport = false;
    bool needReport = needReport_.load();

    if (!needReport) {
        int count = watchdogReportCount_.fetch_add(1) + 1;
        TAG_LOGE(AAFwkTag::APPDFR, "wait count: %{public}d", count);
        if (count >= WATCHDOG_REPORT_COUNT_MAX) {
#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
            SetHiTraceChainId();
#endif
#ifndef APP_NO_RESPONSE_DIALOG
            AppExecFwk::AppfreezeInner::GetInstance()->AppfreezeHandleOverReportCount(true);
#endif
            watchdogReportCount_.store(0);
        } else if (count >= (WATCHDOG_REPORT_COUNT_MAX - 1)) {
#ifndef APP_NO_RESPONSE_DIALOG
            AppExecFwk::AppfreezeInner::GetInstance()->AppfreezeHandleOverReportCount(false);
#endif
        }
    } else {
        shouldReport = !appMainThreadIsAlive_.exchange(false);
        {
            std::unique_lock<std::mutex> lock(mutex_);
            handlerSnap = appMainHandler_;
        }
    }

    if (needReport && shouldReport) {
        const int bufferLen = 128;
        char paramOutBuf[bufferLen] = {0};
        const char *hook_mode = "startup:";
        int ret = GetParameter("libc.hook_mode", "", paramOutBuf, bufferLen);
        if (ret <= 0 || strncmp(paramOutBuf, hook_mode, strlen(hook_mode)) != 0) {
            ReportEvent();
        }
    }
    if (handlerSnap != nullptr) {
        handlerSnap->SendEvent(CHECK_MAIN_THREAD_IS_ALIVE, 0, EventQueue::Priority::VIP);
    }

    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (stopWatchdog_.load()) {
            return;
        }
#ifdef APP_NO_RESPONSE_DIALOG_WEARABLE
        if ((now - lastWatchTime_) < 0 || (now - lastWatchTime_) >= (WEARABLE_CHECK_INTERVAL_TIME / RESET_RATIO)) {
#else
        if ((now - lastWatchTime_) < 0 || (now - lastWatchTime_) >= (checkIntervalTime_ / RESET_RATIO)) {
#endif
            lastWatchTime_ = now;
        }
    }
}

bool Watchdog::CheckReportEvent(int64_t now)
{
    std::string bundleName;
    int64_t lastBackGroundWatchTime = 0;
    int64_t lastWatchTime = 0;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        bundleName = bundleName_;
        lastBackGroundWatchTime = lastBackGroundWatchTime_;
        lastWatchTime = lastWatchTime_;
    }
    if (isInBackground_.load() && backgroundReportCount_.load() < BACKGROUND_REPORT_COUNT_MAX) {
        bool enableCheck = std::find(std::begin(CHECK_BACKGROUND_THREAD), std::end(CHECK_BACKGROUND_THREAD),
            bundleName) != std::end(CHECK_BACKGROUND_THREAD);
        if ((now - lastBackGroundWatchTime) < 0 || (now - lastBackGroundWatchTime) >= (BACKGROUND_INTERVAL_TIME)) {
            TAG_LOGW(AAFwkTag::APPDFR, "In Background, thread may be blocked in, not report time"
                "currTime: %{public}" PRIu64 ", lastTime: %{public}" PRIu64 ", enableCheck: %{public}d",
                static_cast<uint64_t>(now), static_cast<uint64_t>(lastWatchTime), enableCheck);
            std::unique_lock<std::mutex> lock(mutex_);
            lastBackGroundWatchTime_ = now;
        }
        if (enableCheck) {
            backgroundReportCount_++;
            return false;
        }
        if (halfBckGroundCount_.load() > 0) {
            return false;
        }
        halfBckGroundCount_++;
    }
    return true;
}

void Watchdog::ReportEvent()
{
    if (isBgWorkingThread_.load()) {
        TAG_LOGD(AAFwkTag::APPDFR, "Thread is working in the background, do not report this time");
        return;
    }
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    int64_t lastWatchTime = 0;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        lastWatchTime = lastWatchTime_;
    }
#ifdef APP_NO_RESPONSE_DIALOG_WEARABLE
    if ((now - lastWatchTime) > (RESET_RATIO * WEARABLE_CHECK_INTERVAL_TIME) ||
        (now - lastWatchTime) < (WEARABLE_CHECK_INTERVAL_TIME / RESET_RATIO)) {
#else
    if ((now - lastWatchTime) > (RESET_RATIO * checkIntervalTime_) ||
        (now - lastWatchTime) < (checkIntervalTime_ / RESET_RATIO)) {
#endif
        TAG_LOGI(AAFwkTag::APPDFR,
            "Thread may be blocked, not report time. currTime: %{public}llu, lastTime: %{public}llu",
            static_cast<unsigned long long>(now), static_cast<unsigned long long>(lastWatchTime));
        return;
    }
    if (!CheckReportEvent(now)) {
        return;
    }

    backgroundReportCount_++;

    if (!needReport_.load()) {
        return;
    }

    bool isSixSecondEvent = isSixSecondEvent_.load();
    bool isInBackground = isInBackground_.load();
#ifndef APP_NO_RESPONSE_DIALOG
    if (isSixSecondEvent) {
        needReport_.store(false);
    }
#endif

#ifdef ABILITY_RUNTIME_HITRACE_ENABLE
    if (isSixSecondEvent) {
        SetHiTraceChainId();
    }
#endif
    AppExecFwk::AppfreezeInner::GetInstance()->ThreadBlock(isSixSecondEvent, 0, now, isInBackground);
}
}  // namespace AppExecFwk
}  // namespace OHOS
