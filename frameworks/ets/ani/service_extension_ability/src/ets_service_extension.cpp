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

#include "ets_service_extension.h"

#include "ability_info.h"
#include "ability_manager_client.h"
#include "configuration_utils.h"
#include "ets_service_extension_object.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_service_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

EtsServiceExtension *EtsServiceExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    return new EtsServiceExtension(static_cast<ETSRuntime &>(*runtime));
}

EtsServiceExtension::EtsServiceExtension(ETSRuntime &etsRuntime) : etsRuntime_(etsRuntime)
{
    etsAbilityObj_ = std::make_shared<ETSServiceExtensionObject>();
}

EtsServiceExtension::~EtsServiceExtension()
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "EtsServiceExtension destory");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
}

void EtsServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "EtsServiceExtension init");
    if ((token == nullptr) || (application == nullptr) || (handler == nullptr) || (record == nullptr)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "init failed, some obj null");
        return;
    }
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsAbilityObj_");
        return;
    }
    if (!etsAbilityObj_->Init(reinterpret_cast<void *>(this), reinterpret_cast<void *>(&etsRuntime_))) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Init failed");
        return;
    }
    Extension::Init(record, application, handler, token);
    if (Extension::abilityInfo_ == nullptr || Extension::abilityInfo_->srcEntrance.empty()) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "EtsServiceExtension Init abilityInfo error");
        return;
    }
    std::string srcPath(Extension::abilityInfo_->moduleName + "/");
    srcPath.append(Extension::abilityInfo_->srcEntrance);
    auto pos = srcPath.rfind(".");
    if (pos != std::string::npos) {
        srcPath.erase(pos);
        srcPath.append(".abc");
    }
    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    if (!etsAbilityObj_->LoadModule(moduleName, srcPath, abilityInfo_->hapPath,
        abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE, false, abilityInfo_->srcEntrance)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "LoadModule failed");
        return;
    }
}

void EtsServiceExtension::OnStart(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStart");

    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsAbilityObj_");
        return;
    }
    etsAbilityObj_->OnStart(want);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

void EtsServiceExtension::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnStop");
    ServiceExtension::OnStop();

    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsAbilityObj_");
        return;
    }
    etsAbilityObj_->OnStop();
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

sptr<IRemoteObject> EtsServiceExtension::OnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnConnect");
    Extension::OnConnect(want);

    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsAbilityObj_");
        return nullptr;
    }
    auto remoteObj = etsAbilityObj_->OnConnect(want);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
    return remoteObj;
}

sptr<IRemoteObject> EtsServiceExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnConnect");
    Extension::OnConnect(want);

    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsAbilityObj_");
        return nullptr;
    }
    return etsAbilityObj_->OnConnectCallback(want, callbackInfo, isAsyncCallback);
}

void EtsServiceExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnDisconnect");

    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsAbilityObj_");
        return;
    }
    etsAbilityObj_->OnDisconnect(want);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

void EtsServiceExtension::OnDisconnect(
    const AAFwk::Want &want, AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnDisconnect");

    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsAbilityObj_");
        return;
    }
    etsAbilityObj_->OnDisconnectCallback(want, callbackInfo, isAsyncCallback);
}

void EtsServiceExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnCommand");

    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsAbilityObj_");
        return;
    }
    etsAbilityObj_->OnCommand(want, startId);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
    return;
}

void EtsServiceExtension::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration) {}

void EtsServiceExtension::ConfigurationUpdated() {}

void EtsServiceExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info) {}
} // namespace AbilityRuntime
} // namespace OHOS