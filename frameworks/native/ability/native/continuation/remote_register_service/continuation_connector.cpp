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

#include "continuation_connector.h"

#include "continuation_device_callback_proxy.h"
#include "hilog_tag_wrapper.h"
#include "remote_register_service_proxy.h"

namespace OHOS {
namespace AppExecFwk {
sptr<ContinuationConnector> ContinuationConnector::instance_ = nullptr;
std::mutex ContinuationConnector::mutex_;
const std::string ContinuationConnector::CONNECTOR_DEVICE_ID("");
const std::string ContinuationConnector::CONNECTOR_BUNDLE_NAME("com.ohos.controlcenter");
const std::string ContinuationConnector::CONNECTOR_ABILITY_NAME(
    "com.ohos.controlcenter.fatransfer.service.FeatureAbilityRegisterService");

ContinuationConnector::ContinuationConnector(const std::weak_ptr<Context> &context) : context_(context)
{}


sptr<ContinuationConnector> ContinuationConnector::GetInstance(const std::weak_ptr<Context> &context)
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = sptr<ContinuationConnector>(new (std::nothrow) ContinuationConnector(context));
        }
    }
    return instance_;
}


void ContinuationConnector::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null remote");
        return;
    }
    remoteRegisterService_ = iface_cast<IRemoteRegisterService>(remoteObject);
    if (remoteRegisterService_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null remoteRegisterService_");
        return;
    }
    isConnected_.store(true);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto &iter : continuationRequestList_) {
            iter->Execute();
        }
        continuationRequestList_.clear();
    }
}


void ContinuationConnector::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    remoteRegisterService_ = nullptr;
    isConnected_.store(false);
}


void ContinuationConnector::BindRemoteRegisterAbility(const std::shared_ptr<AppExecFwk::ContinuationRequest> &request)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr tmpcontext = context_.lock();
    if (tmpcontext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null tmpcontext");
        return;
    }
    if (request == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null request");
        return;
    }
    if (IsAbilityConnected()) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "remote register bounded");
        request->Execute();
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        continuationRequestList_.push_back(request);
    }
    BindRemoteRegisterAbility();
}


void ContinuationConnector::UnbindRemoteRegisterAbility()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr tmpcontext = context_.lock();
    if (tmpcontext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null tmpcontext");
        return;
    }

    tmpcontext->DisconnectAbility(this);
    isConnected_.store(false);
    remoteRegisterService_ = nullptr;
}


bool ContinuationConnector::IsAbilityConnected()
{
    return isConnected_.load();
}


bool ContinuationConnector::Unregister(int token)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (remoteRegisterService_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null remoteRegisterService_");
        return false;
    }

    return remoteRegisterService_->Unregister(token);
}


bool ContinuationConnector::UpdateConnectStatus(int token, const std::string &deviceId, int status)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (remoteRegisterService_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null remoteRegisterService_");
        return false;
    }

    return remoteRegisterService_->UpdateConnectStatus(token, deviceId, status);
}


bool ContinuationConnector::ShowDeviceList(int token, const AppExecFwk::ExtraParams &parameter)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    if (remoteRegisterService_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null remoteRegisterService_");
        return false;
    }
    return remoteRegisterService_->ShowDeviceList(token, parameter);
}


int ContinuationConnector::Register(std::weak_ptr<Context> &context, std::string bundleName,
    const AppExecFwk::ExtraParams &parameter, std::shared_ptr<IContinuationDeviceCallback> &callback)
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr pcontext = context.lock();
    if (pcontext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null pcontext");
        return -1;
    }
    if (remoteRegisterService_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null remoteRegisterService_");
        return -1;
    }
    sptr<IRemoteObject> token = pcontext->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null token");
        return -1;
    }

    sptr<ContinuationDeviceCallbackProxy> callBackSptr(new (std::nothrow) ContinuationDeviceCallbackProxy(callback));

    return remoteRegisterService_->Register(bundleName, token, parameter, callBackSptr);
}


void ContinuationConnector::BindRemoteRegisterAbility()
{
    TAG_LOGI(AAFwkTag::CONTINUATION, "begin");
    std::shared_ptr tmpcontext = context_.lock();
    if (tmpcontext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION, "null tmpcontext");
        return;
    }
    Want want;
    want.SetElementName(CONNECTOR_DEVICE_ID, CONNECTOR_BUNDLE_NAME, CONNECTOR_ABILITY_NAME);
    want.AddFlags(Want::FLAG_NOT_OHOS_COMPONENT);
    tmpcontext->ConnectAbility(want, this);
}
}  // namespace AppExecFwk
}  // namespace OHOS
