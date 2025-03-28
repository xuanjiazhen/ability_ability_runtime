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

#include "sts_form_extension.h"

#include <vector>
#include "ability_info.h"
#include "ani.h"
#include "ani_common/ani_common_want.h"
#include "form_provider_data.h"
#include "hilog_tag_wrapper.h"
#include "sts_form_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {

STSFormExtension *STSFormExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "call");
    return new STSFormExtension(static_cast<STSRuntime &>(*runtime));
}

const STSRuntime &STSFormExtension::GetSTSRuntime()
{
    return stsRuntime_;
}

STSFormExtension::STSFormExtension(STSRuntime &stsRuntime) : stsRuntime_(stsRuntime) {}

STSFormExtension::~STSFormExtension()
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "destructor");
}

void STSFormExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "Init call");
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null localAbilityRecord");
        return;
    }
    auto abilityInfo = record->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null abilityInfo");
        return;
    }
    FormExtension::Init(record, application, handler, token);

    std::string srcPath(abilityInfo->package);
    if (!abilityInfo->isModuleJson) {
        srcPath.append("/assets/js/");
        if (!abilityInfo->srcPath.empty()) {
            srcPath.append(abilityInfo->srcPath);
        }
        srcPath.append("/").append(abilityInfo->name).append(".abc");
    } else {
        if (abilityInfo->srcEntrance.empty()) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "empty srcEntrance");
            return;
        }
        srcPath.append("/");
        srcPath.append(abilityInfo->srcEntrance);
        srcPath.erase(srcPath.rfind("."));
        srcPath.append(".abc");
        TAG_LOGI(AAFwkTag::FORM_EXT, "stsAbility srcPath: %{public}s", srcPath.c_str());
    }

    std::string moduleName(abilityInfo->moduleName);
    moduleName.append("::").append(abilityInfo->name);
    BindContext(abilityInfo, record->GetWant(), moduleName, srcPath);
    TAG_LOGI(AAFwkTag::FORM_EXT, "Init End");
}

void STSFormExtension::UpdateFormExtensionObj(
    const std::shared_ptr<AbilityInfo> &abilityInfo, const std::string &moduleName, const std::string &srcPath)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "UpdateFormExtensionObj call");
    std::string key = moduleName + "::" + srcPath;
    std::unique_ptr<NativeReference> moduleObj = nullptr;
    stsAbilityObj_ = stsRuntime_.LoadModule(moduleName, srcPath, abilityInfo->hapPath,
        abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE, false, abilityInfo->srcEntrance);
    TAG_LOGI(AAFwkTag::FORM_EXT, "UpdateFormExtensionObj End");
}

void STSFormExtension::BindContext(const std::shared_ptr<AbilityInfo> &abilityInfo,
    const std::shared_ptr<AAFwk::Want> &want, const std::string &moduleName, const std::string &srcPath)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "BindContext call");
    auto env = stsRuntime_.GetAniEnv();
    UpdateFormExtensionObj(abilityInfo, moduleName, srcPath);
    if (stsAbilityObj_ == nullptr || want == nullptr || env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null stsAbilityObj_ or abilityContext_ or want or env");
        return;
    }

    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get context error");
        return;
    }
    ani_ref contextObj = CreateStsFormExtensionContext(env, context);
    ani_ref contextGlobalRef = nullptr;
    ani_field field = nullptr;
    ani_status status = ANI_ERROR;

    if ((status = env->GlobalReference_Create(contextObj, &contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
    }
    if ((status = env->Class_FindField(stsAbilityObj_->aniCls, "context", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
    }
    if ((status = env->Object_SetField_Ref(stsAbilityObj_->aniObj, field, contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status : %{public}d", status);
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "BindContext End");
}

std::string STSFormExtension::ANIUtils_ANIStringToStdString(ani_env *env, ani_string ani_str)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "ANIUtils_ANIStringToStdString Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env is nullptr ");
        return "";
    }
    ani_size strSize;
    if (ANI_OK != env->String_GetUTF8Size(ani_str, &strSize)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "String_GetUTF8Size Failed");
        return "";
    }

    std::vector<char> buffer(strSize + 1);
    char *utf8_buffer = buffer.data();

    ani_size bytes_written = 0;
    if (ANI_OK != env->String_GetUTF8(ani_str, utf8_buffer, strSize + 1, &bytes_written)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "String_GetUTF8 Failed");
        return "";
    }

    utf8_buffer[bytes_written] = '\0';
    std::string content = std::string(utf8_buffer);
    TAG_LOGI(AAFwkTag::FORM_EXT, "ANIUtils_ANIStringToStdString End");
    return content;
}

bool STSFormExtension::ConvertFromDataProxies(
    ani_env *env, ani_object arrayValue, std::vector<FormDataProxy> &formDataProxies)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "ConvertFromDataProxies Call");
    if (env == nullptr || arrayValue == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "env || arrayValue is nullptr ");
        return false;
    }
    ani_status status = ANI_OK;
    ani_double length;
    if (ANI_OK != env->Object_GetPropertyByName_Double(arrayValue, "length", &length)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetPropertyByName_Double length Failed");
        return false;
    }
    for (int i = 0; i < int(length); i++) {
        FormDataProxy formDataProxy("", "");
        ani_ref stringEntryRef;
        if (ANI_OK !=
            env->Object_CallMethodByName_Ref(arrayValue, "$_get", "I:Lstd/core/Object;", &stringEntryRef, (ani_int)i)) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Object_CallMethodByName_Ref _get Failed");
            return false;
        }
        ani_field key;
        if ((status = env->Class_FindField(stsAbilityObj_->aniCls, "key", &key)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindField status : %{public}d", status);
            return false;
        }
        ani_ref keyRef;
        if ((status = env->Object_GetField_Ref(static_cast<ani_object>(stringEntryRef), key, &keyRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetField_Ref status : %{public}d", status);
            return false;
        }
        formDataProxy.key = ANIUtils_ANIStringToStdString(env, static_cast<ani_string>(keyRef));
        ani_field subscriberId;
        if ((status = env->Class_FindField(stsAbilityObj_->aniCls, "subscriberId", &subscriberId)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindField status : %{public}d", status);
            return false;
        }
        ani_ref subscriberIdRef;
        if ((status = env->Object_GetField_Ref(
            static_cast<ani_object>(stringEntryRef), subscriberId, &subscriberIdRef)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetField_Ref status : %{public}d", status);
            return false;
        }
        formDataProxy.subscribeId = ANIUtils_ANIStringToStdString(env, static_cast<ani_string>(subscriberIdRef));
        formDataProxies.push_back(formDataProxy);
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "ConvertFromDataProxies End");
    return true;
}

OHOS::AppExecFwk::FormProviderInfo STSFormExtension::OnCreate(const OHOS::AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnCreate call");
    OHOS::AppExecFwk::FormProviderInfo formProviderInfo;
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return formProviderInfo;
    }
    ani_status status = ANI_OK;
    ani_object aniWant = OHOS::AppExecFwk::WrapWant(env, want);
    ani_method function;
    if ((status = env->Class_FindMethod(stsAbilityObj_->aniCls, "<onAddForm>",
        "L@ohos/app/ability/Want/Want;:L@ohos/app/form/formBindingData/formBindingData/FormBindingData;",
        &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status : %{public}d", status);
        return formProviderInfo;
    }
    ani_ref nativeResult;
    if ((status = env->Object_CallMethod_Ref(stsAbilityObj_->aniObj, function, &nativeResult, aniWant)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New status : %{public}d", status);
        return formProviderInfo;
    }
    ani_field data;
    if ((status = env->Class_FindField(stsAbilityObj_->aniCls, "data", &data)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindField status : %{public}d", status);
        return formProviderInfo;
    }
    ani_ref dataRef;
    if ((status = env->Object_GetField_Ref(static_cast<ani_object>(nativeResult), data, &dataRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetField_Ref status : %{public}d", status);
        return formProviderInfo;
    }
    std::string dataStr = ANIUtils_ANIStringToStdString(env, static_cast<ani_string>(dataRef));
    auto formData = AppExecFwk::FormProviderData(dataStr);
    formProviderInfo.SetFormData(formData);
    ani_field proxies;
    if ((status = env->Class_FindField(stsAbilityObj_->aniCls, "proxies", &proxies)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindField status : %{public}d", status);
        return formProviderInfo;
    }
    ani_ref proxiesRef;
    if ((status = env->Object_GetField_Ref(static_cast<ani_object>(nativeResult), proxies, &proxiesRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_GetField_Ref status : %{public}d", status);
        return formProviderInfo;
    }
    return SetFormData(formProviderInfo, proxiesRef);
}

OHOS::AppExecFwk::FormProviderInfo STSFormExtension::SetFormData(
    OHOS::AppExecFwk::FormProviderInfo &formProviderInfo, ani_ref proxiesRef)
{
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return formProviderInfo;
    }
    std::vector<FormDataProxy> formDataProxies;
    if (proxiesRef != nullptr && !ConvertFromDataProxies(env, static_cast<ani_object>(proxiesRef), formDataProxies)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Convert formDataProxies failed");
        return formProviderInfo;
    }
    formProviderInfo.SetFormDataProxies(formDataProxies);
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnCreate End");
    return formProviderInfo;
}

void STSFormExtension::OnDestroy(const int64_t formId)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnDestroy formId: %{public}" PRId64, formId);
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    ani_status status = ANI_OK;
    ani_string formIdStr;
    std::string str = std::to_string(formId);
    if ((status = env->String_NewUTF8(str.c_str(), str.size(), &formIdStr))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "String_NewUTF8 status : %{public}d", status);
        return;
    }
    ani_method function;
    if ((status = env->Class_FindMethod(stsAbilityObj_->aniCls, "<onRemoveForm>", "Lstd/core/String;:V", &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status : %{public}d", status);
        return;
    }
    if ((status = env->Object_CallMethod_Void(stsAbilityObj_->aniObj, function, formIdStr)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnDestroy End");
}

void STSFormExtension::OnEvent(const int64_t formId, const std::string &message)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnEvent formId: %{public}" PRId64, formId);
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    ani_status status = ANI_OK;
    ani_string formIdStr;
    std::string str = std::to_string(formId);
    if ((status = env->String_NewUTF8(str.c_str(), str.size(), &formIdStr))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "String_NewUTF8 status : %{public}d", status);
        return;
    }
    ani_string aniMessage;
    if ((status = env->String_NewUTF8(message.c_str(), message.size(), &aniMessage))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "String_NewUTF8 status : %{public}d", status);
        return;
    }
    ani_method function;
    if ((status = env->Class_FindMethod(
        stsAbilityObj_->aniCls, "<onFormEvent>", "Lstd/core/String;Lstd/core/String;:V", &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status : %{public}d", status);
        return;
    }

    status = env->Object_CallMethod_Void(stsAbilityObj_->aniObj, function, formIdStr, aniMessage);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnEvent End");
}

void STSFormExtension::OnUpdate(const int64_t formId, const AAFwk::WantParams &wantParams)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnUpdate formId: %{public}" PRId64, formId);
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    ani_status status = ANI_OK;
    ani_string formIdStr;
    std::string str = std::to_string(formId);
    if ((status = env->String_NewUTF8(str.c_str(), str.size(), &formIdStr))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "String_NewUTF8 status : %{public}d", status);
        return;
    }
    ani_ref aniWantParams = OHOS::AppExecFwk::WrapWantParams(env, wantParams);
    ani_method function;
    if ((status = env->Class_FindMethod(
        stsAbilityObj_->aniCls, "<onUpdateForm>", "Lstd/core/String;Lescompat/Record;:V", &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status : %{public}d", status);
        return;
    }
    if ((status = env->Object_CallMethod_Void(
        stsAbilityObj_->aniObj, function, formIdStr, static_cast<ani_object>(aniWantParams))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnUpdate End");
}

void STSFormExtension::OnCastToNormal(const int64_t formId)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnCastToNormal formId: %{public}" PRId64, formId);
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    ani_status status = ANI_OK;

    ani_string formIdStr;
    std::string str = std::to_string(formId);
    if ((status = env->String_NewUTF8(str.c_str(), str.size(), &formIdStr))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "String_NewUTF8 status : %{public}d", status);
        return;
    }
    ani_method function;
    if ((status = env->Class_FindMethod(
        stsAbilityObj_->aniCls, "<onCastToNormalForm>", "Lstd/core/String;:V", &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status : %{public}d", status);
        return;
    }
    if ((status = env->Object_CallMethod_Void(stsAbilityObj_->aniObj, function, formIdStr)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnCastToNormal End");
}

void STSFormExtension::OnVisibilityChange(const std::map<int64_t, int32_t> &formEventsMapp)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnVisibilityChange call");
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    ani_status status = ANI_OK;
    ani_method function;
    if ((status = env->Class_FindMethod(
        stsAbilityObj_->aniCls, "<onChangeFormVisibility>", "Lescompat/Record;:V", &function))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_FindMethod status : %{public}d", status);
        return;
    }
    ani_class recordCls;
    if ((status = env->FindClass("Lescompat/Record;", &recordCls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "FindClass failed status : %{public}d", status);
        return;
    }
    ani_method objectMethod;
    if ((status = env->Class_FindMethod(recordCls, "<ctor>", ":V", &objectMethod)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status : %{public}d", status);
        return;
    }
    ani_object recordObject;
    if ((status = env->Object_New(recordCls, objectMethod, &recordObject)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status : %{public}d", status);
        return;
    }
    ani_method recordSetMethod;
    if ((status = env->Class_FindMethod(
        recordCls, "$_set", "Lstd/core/Object;Lstd/core/Object;:V", &recordSetMethod)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status : %{public}d", status);
        return;
    }
    for (auto iter = formEventsMapp.begin(); iter != formEventsMapp.end(); ++iter) {
        std::string key = std::to_string(iter->first);
        ani_string ani_key;
        ani_int ani_value = iter->second;
        if ((status = env->String_NewUTF8(key.c_str(), key.length(), &ani_key)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 key failed status : %{public}d", status);
            return;
        }
        if ((status = env->Object_CallMethod_Void(recordObject, recordSetMethod, ani_key, ani_value)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void failed status : %{public}d", status);
            return;
        }
    }
    if ((status = env->Object_CallMethod_Void(stsAbilityObj_->aniObj, function, recordObject)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_New status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnVisibilityChange End");
}
} // namespace AbilityRuntime
} // namespace OHOS
