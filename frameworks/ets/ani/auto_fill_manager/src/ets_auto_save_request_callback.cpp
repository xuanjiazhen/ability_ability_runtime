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

#include "ets_auto_save_request_callback.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AutoFillManagerEts {
namespace {
const std::string METHOD_ON_SAVE_REQUEST_SUCCESS = "onSuccess";
const std::string METHOD_ON_SAVE_REQUEST_FAILED = "onFailure";
} // namespace

EtsAutoSaveRequestCallback::EtsAutoSaveRequestCallback(ani_env *env, int32_t instanceId,
    AutoFillManagerFunc autoFillManagerFunc)
    : env_(env), instanceId_(instanceId), autoFillManagerFunc_(autoFillManagerFunc) {}

EtsAutoSaveRequestCallback::~EtsAutoSaveRequestCallback() {}

void EtsAutoSaveRequestCallback::OnSaveRequestSuccess()
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "onSuccess called");
    ETSCallFunction(METHOD_ON_SAVE_REQUEST_SUCCESS);
    if (autoFillManagerFunc_ != nullptr) {
        autoFillManagerFunc_(instanceId_);
    }
}

void EtsAutoSaveRequestCallback::OnSaveRequestFailed()
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "onFailure called");
    ETSCallFunction(METHOD_ON_SAVE_REQUEST_FAILED);
    if (autoFillManagerFunc_ != nullptr) {
        autoFillManagerFunc_(instanceId_);
    }
}

void EtsAutoSaveRequestCallback::Register(ani_object object)
{
    TAG_LOGD(AAFwkTag::AUTOFILLMGR, "called");
    if (env_ == nullptr || object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null env_ or object");
        return;
    }

    if (IsEtsCallbackEquals(callback_, object)) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "callback exist");
        return;
    }

    ani_ref objRef = nullptr;
    if (env_->GlobalReference_Create(object, &objRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "GlobalReference_Create failed");
        return;
    }

    callback_ = std::make_shared<AbilityRuntime::STSNativeReference>();
    if (callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null callback_");
        return;
    }
    callback_->aniObj = object;
    callback_->aniRef = objRef;
}

void EtsAutoSaveRequestCallback::ETSCallFunction(const std::string &methodName)
{
    if (env_ == nullptr || callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null env_ or callback_");
        return;
    }

    ani_status status = ANI_OK;
    ani_ref funRef;
    status = env_->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(callback_->aniRef), methodName.c_str(),
        &funRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Object_GetField_Ref failed status: %{public}d", status);
        return;
    }

    ani_ref resutlt;
    if ((status = env_->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), 0, nullptr, &resutlt)) !=
        ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "FunctionalObject_Call failed, status: %{public}d", status);
        return;
    }
}

bool EtsAutoSaveRequestCallback::IsEtsCallbackEquals(std::shared_ptr<AbilityRuntime::STSNativeReference> callback,
    ani_object object)
{
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Invalid etsCallback");
        return false;
    }

    if (env_ == nullptr || object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "null env_ or object");
        return false;
    }

    ani_boolean isEquals = false;
    if ((env_->Reference_StrictEquals(reinterpret_cast<ani_ref>(object), reinterpret_cast<ani_ref>(callback->aniObj),
        &isEquals)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILLMGR, "Object not match");
        return false;
    }
    
    return isEquals;
}
} // namespace AutoFillManagerEts
} // namespace OHOS