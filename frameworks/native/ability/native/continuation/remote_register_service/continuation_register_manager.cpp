/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "continuation_register_manager.h"

#include "continuation_device_callback_interface.h"
#include "continuation_register_manager_proxy.h"
#include "extra_params.h"
#include "hilog_tag_wrapper.h"
#include "request_callback.h"

namespace OHOS {
namespace AppExecFwk {
void ContinuationRegisterManager::Init(
    const std::shared_ptr<ContinuationRegisterManagerProxy> &continuationRegisterManagerProxy)
{
    continuationRegisterManagerProxy_ = continuationRegisterManagerProxy;
}


void ContinuationRegisterManager::Register(const std::string &bundleName, const ExtraParams &parameter,
    const std::shared_ptr<IContinuationDeviceCallback> &deviceCallback,
    const std::shared_ptr<RequestCallback> &requestCallback)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (continuationRegisterManagerProxy_ != nullptr) {
        continuationRegisterManagerProxy_->Register(bundleName, parameter, deviceCallback, requestCallback);
    } else {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null proxy");
    }
}


void ContinuationRegisterManager::Unregister(int token, const std::shared_ptr<RequestCallback> &requestCallback)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (continuationRegisterManagerProxy_ != nullptr) {
        continuationRegisterManagerProxy_->Unregister(token, requestCallback);
    } else {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null proxy");
    }
}


void ContinuationRegisterManager::UpdateConnectStatus(
    int token, const std::string &deviceId, int status, const std::shared_ptr<RequestCallback> &requestCallback)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (continuationRegisterManagerProxy_ != nullptr) {
        continuationRegisterManagerProxy_->UpdateConnectStatus(token, deviceId, status, requestCallback);
    } else {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null proxy");
    }
}


void ContinuationRegisterManager::ShowDeviceList(
    int token, const ExtraParams &parameter, const std::shared_ptr<RequestCallback> &requestCallback)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (continuationRegisterManagerProxy_ != nullptr) {
        continuationRegisterManagerProxy_->ShowDeviceList(token, parameter, requestCallback);
    } else {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null proxy");
    }
}


void ContinuationRegisterManager::Disconnect(void)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (continuationRegisterManagerProxy_ != nullptr) {
        continuationRegisterManagerProxy_->Disconnect();
    } else {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null proxy");
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
