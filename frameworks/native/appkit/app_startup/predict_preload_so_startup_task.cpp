/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "predict_preload_so_startup_task.h"

#include <vector>

#include "event_report.h"
#include "hilog_tag_wrapper.h"
#include "native_module_collector.h"
#include "native_module_manager.h"

namespace OHOS {
namespace AbilityRuntime {
const std::string PredictPreloadSoStartupTask::TASK_TYPE = "PredictPreloadSo";

PredictPreloadSoStartupTask::PredictPreloadSoStartupTask(const std::string& name, const std::string& storePath)
    : PreloadSoStartupTask(name, ""), storePath_(storePath)
{
    SetWaitOnMainThread(false);
    SetCallCreateOnMainThread(false);
}

PredictPreloadSoStartupTask::~PredictPreloadSoStartupTask() = default;

const std::string &PredictPreloadSoStartupTask::GetType() const
{
    return TASK_TYPE;
}

int32_t PredictPreloadSoStartupTask::RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback)
{
    if (!AppExecFwk::NativeModuleCollector::IsEnabled()) {
        OnCompletedCallback::OnCallback(std::move(callback), ERR_OK);
        return ERR_OK;
    }

    // missing/corrupted/version-mismatched file yields an empty list: skip silently
    std::vector<AppExecFwk::PredictedModule> modules =
        AppExecFwk::NativeModuleCollector::LoadPredictedSystemModules(storePath_);
    if (modules.empty()) {
        TAG_LOGD(AAFwkTag::STARTUP, "no predicted system module to preload");
        OnCompletedCallback::OnCallback(std::move(callback), ERR_OK);
        return ERR_OK;
    }
    TAG_LOGI(AAFwkTag::STARTUP, "predict preload begin, count=%{public}zu", modules.size());

    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "moduleManager is null");
        OnCompletedCallback::OnCallback(std::move(callback), ERR_OK);
        AAFwk::EventInfo eventInfo;
        eventInfo.errCode = ERR_NATIVE_MODULE_MANAGER_CONSTRUCTION;
        eventInfo.errReason = "moduleManager is null";
        AAFwk::EventReport::SendLaunchFrameworkEvent(
            AAFwk::EventName::STARTUP_TASK_ERROR, HISYSEVENT_FAULT, eventInfo);
        return ERR_OK;
    }

    for (const auto& pred : modules) {
        std::string errInfo;
        // replay the original import: same moduleName + relativePath, system module
        NativeModule* module = moduleManager->LoadNativeModule(
            pred.moduleName.c_str(), nullptr, false, errInfo, false, pred.relativePath.c_str());
        if (module == nullptr) {
            // a single failure is recorded and skipped, never blocking others or startup
            TAG_LOGW(AAFwkTag::STARTUP, "predict preload %{public}s failed: %{public}s",
                pred.moduleName.c_str(), errInfo.c_str());
            AAFwk::EventInfo eventInfo;
            eventInfo.errCode = ERR_LOAD_NATIVE_MODULE;
            eventInfo.errReason = errInfo;
            AAFwk::EventReport::SendLaunchFrameworkEvent(
                AAFwk::EventName::STARTUP_TASK_ERROR, HISYSEVENT_FAULT, eventInfo);
            continue;
        }
        TAG_LOGD(AAFwkTag::STARTUP, "predict preload %{public}s ok", pred.moduleName.c_str());
        // let the collector discount consumer-triggered loads from real app imports
        AppExecFwk::NativeModuleCollector::RecordConsumerPreload(pred.moduleName);
    }
    TAG_LOGI(AAFwkTag::STARTUP, "predict preload end, count=%{public}zu", modules.size());
    OnCompletedCallback::OnCallback(std::move(callback), ERR_OK);
    return ERR_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS
