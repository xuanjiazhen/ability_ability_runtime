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

#include "sts_ability_delegator_utils.h"

#include <map>
#include "hilog_tag_wrapper.h"
#include "sts_ability_delegator.h"
#include <ani_signature_builder.h>

namespace OHOS {
namespace AbilityDelegatorSts {
ani_object CreateStsAbilityDelegator(ani_env *aniEnv)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "CreateJsAbilityDelegator");
    ani_class abilityDelegator = nullptr;
    ani_status status = ANI_ERROR;
    auto classTypeAbilityDelegatorInner = arkts::ani_signature::Builder::BuildClass({"application", "AbilityDelegator", "AbilityDelegatorInner"});
    status = aniEnv->FindClass(classTypeAbilityDelegatorInner.Descriptor().c_str(), &abilityDelegator);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "find AbilityDelegator failed status : %{public}d", status);
        return {};
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "find AbilityDelegator success");

    arkts::ani_signature::SignatureBuilder PrintSyncSigBuilder;
    PrintSyncSigBuilder.AddClass({"std", "core", "String"});
    auto PrintSyncSignature = PrintSyncSigBuilder.BuildSignatureDescriptor();
    std::array delegatorFunctions = {
                ani_native_function{ "printSync", PrintSyncSignature.c_str(), reinterpret_cast<void*>(PrintSync) },
    };
    status = aniEnv->Class_BindNativeMethods(abilityDelegator, delegatorFunctions.data(), delegatorFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_BindNativeMethods failed status : %{public}d", status);
        return {};
    }

    ani_method method = nullptr;
    auto methodCtor = arkts::ani_signature::Builder::BuildConstructorName();
    arkts::ani_signature::SignatureBuilder methodCtorSigBuilder;
    auto methodCtorSignature = methodCtorSigBuilder.BuildSignatureDescriptor();
    status = aniEnv->Class_FindMethod(abilityDelegator, methodCtor.c_str(), methodCtorSignature.c_str(), &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod ctor failed status : %{public}d", status);
        return {};
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "Class_FindMethod ctor success");

    ani_object object = nullptr;
    if (aniEnv->Object_New(abilityDelegator, method, &object) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_New failed status : %{public}d", status);
        return {};
    }

    TAG_LOGI(AAFwkTag::DELEGATOR, "CreateStsAbilityDelegator success");
    return object;
}

void SetBundleName(ani_env *aniEnv, ani_class arguments, ani_object argumentObject, const std::string &bundleName)
{
    ani_status status = ANI_ERROR;
    ani_string aniStr;
    // Get a ani_string from std::string
    status = aniEnv->String_NewUTF8(bundleName.c_str(), bundleName.length(), &aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 failed status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "String_NewUTF8 success");

    // find the setter method
    ani_method nameSetter;
    auto methodSetBundlename = arkts::ani_signature::Builder::BuildSetterName("bundleName");
    status = aniEnv->Class_FindMethod(arguments, methodSetBundlename.c_str(), nullptr, &nameSetter);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "Class_FindMethod success");

    // call set bundleName(n:string)
    status = aniEnv->Object_CallMethod_Void(argumentObject, nameSetter, aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void failed status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "Object_CallMethod_Void success");
}

void SetParameters(ani_env *aniEnv, ani_class arguments, ani_object argumentObject,
    const std::map<std::string, std::string> &paras)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "SetParameters begin");
    ani_status status = ANI_ERROR;
    // get getter and setter methond of Record
    ani_class recordCls;
    auto classTypeRecord = arkts::ani_signature::Builder::BuildClass({"escompat", "Record"});
    status = aniEnv->FindClass(classTypeRecord.Descriptor().c_str(), &recordCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "FindClass failed status : %{public}d", status);
        return;
    }

    ani_method recordGetMethod;
    arkts::ani_signature::SignatureBuilder $_getSigBuilder;
    $_getSigBuilder.AddClass({"std", "core", "Object"});
    $_getSigBuilder.SetReturnClass("std.core.Object");
    auto $_getSignature = $_getSigBuilder.BuildSignatureDescriptor();
    status = aniEnv->Class_FindMethod(recordCls, "$_get", $_getSignature.c_str(), &recordGetMethod);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status : %{public}d", status);
        return;
    }

    ani_method recordSetMethod;
    arkts::ani_signature::SignatureBuilder $_setSigBuilder;
    $_setSigBuilder.AddClass({"std", "core", "Object"});
    $_setSigBuilder.AddClass({"std", "core", "Object"});
    auto $_setSignature = $_setSigBuilder.BuildSignatureDescriptor();
    status = aniEnv->Class_FindMethod(recordCls, "$_set", $_setSignature.c_str(), &recordSetMethod);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status : %{public}d", status);
        return;
    }

    // get parameters ref of object
    ani_ref parameterRef;
    auto methodGetParameters = arkts::ani_signature::Builder::BuildGetterName("parameters");
    arkts::ani_signature::SignatureBuilder methodGetParametersSigBuilder;
    methodGetParametersSigBuilder.SetReturnClass("escompat.Record");
    auto methodGetParametersSignature = methodGetParametersSigBuilder.BuildSignatureDescriptor();
    status = aniEnv->Object_CallMethodByName_Ref(argumentObject, methodGetParameters.c_str(), methodGetParametersSignature.c_str(), &parameterRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethodByName_Ref failed status : %{public}d", status);
        return;
    }
    ani_object parameterObject = static_cast<ani_object>(parameterRef);

    for (auto iter = paras.begin(); iter != paras.end(); ++iter) {
        std::string key = iter->first;
        std::string value = iter->second;
        ani_string ani_key;
        ani_string ani_value;

        status = aniEnv->String_NewUTF8(key.c_str(), key.length(), &ani_key);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 key failed status : %{public}d", status);
            return;
        }

        status = aniEnv->String_NewUTF8(value.c_str(), value.length(), &ani_value);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 value failed status : %{public}d", status);
            return;
        }

        // 调用set方法给Record类型的property赋值
        status = aniEnv->Object_CallMethod_Void(parameterObject, recordSetMethod, ani_key, ani_value);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void failed status : %{public}d", status);
            return;
        }
    }

    TAG_LOGI(AAFwkTag::DELEGATOR, "SetParameters end");
}

void SetTestCaseNames(ani_env *aniEnv, ani_class arguments, ani_object argumentObject, const std::string &testcaseNames)
{
    ani_status status = ANI_ERROR;
    ani_string aniStr;
    status = aniEnv->String_NewUTF8(testcaseNames.c_str(), testcaseNames.length(), &aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 failed status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "String_NewUTF8 success");

    // find the setter method
    ani_method nameSetter;
    auto methodSetTestcasenames = arkts::ani_signature::Builder::BuildSetterName("testCaseNames");
    status = aniEnv->Class_FindMethod(arguments, methodSetTestcasenames.c_str(), nullptr, &nameSetter);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "Class_FindMethod success");

    // call set testcaseNames(n:string)
    status = aniEnv->Object_CallMethod_Void(argumentObject, nameSetter, aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void failed status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "Object_CallMethod_Void success");
}

void SetTestRunnerClassName(ani_env *aniEnv, ani_class arguments, ani_object argumentObject,
    const std::string &className)
{
    ani_status status = ANI_ERROR;
    ani_string aniStr;
    status = aniEnv->String_NewUTF8(className.c_str(), className.length(), &aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "String_NewUTF8 failed status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "String_NewUTF8 success");

    // find the setter method
    ani_method nameSetter;
    auto methodSetTestrunnerclassname = arkts::ani_signature::Builder::BuildSetterName("testRunnerClassName");
    status = aniEnv->Class_FindMethod(arguments, methodSetTestrunnerclassname.c_str(), nullptr, &nameSetter);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod failed status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "Class_FindMethod success");

    // call set testRunnerClassName(n:string)
    status = aniEnv->Object_CallMethod_Void(argumentObject, nameSetter, aniStr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_CallMethod_Void failed status : %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "Object_CallMethod_Void success");
}

ani_object CreateStsAbilityDelegatorArguments(
    ani_env *aniEnv, const std::shared_ptr<AppExecFwk::AbilityDelegatorArgs> abilityDelegatorArgs)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "CreateJsAbilityDelegatorArguments");
    ani_class arguments = nullptr;
    ani_status status = ANI_ERROR;
    auto classTypeAbilityDelegatorArgsInner = arkts::ani_signature::Builder::BuildClass({"application", "AbilityDelegatorArgs", "AbilityDelegatorArgsInner"});
    status = aniEnv->FindClass(classTypeAbilityDelegatorArgsInner.Descriptor().c_str(), &arguments);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "find AbilityDelegatorArgs failed status : %{public}d", status);
        return {};
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "find AbilityDelegatorArgs success");

    ani_method method = nullptr;
    auto methodCtor = arkts::ani_signature::Builder::BuildConstructorName();
    arkts::ani_signature::SignatureBuilder methodCtorSigBuilder;
    auto methodCtorSignature = methodCtorSigBuilder.BuildSignatureDescriptor();
    status = aniEnv->Class_FindMethod(arguments, methodCtor.c_str(), methodCtorSignature.c_str(), &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Class_FindMethod ctor failed status : %{public}d", status);
        return {};
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "Class_FindMethod ctor success");

    ani_object argumentObject = nullptr;
    status = aniEnv->Object_New(arguments, method, &argumentObject);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Object_New failed status : %{public}d", status);
        return {};
    }
    TAG_LOGI(AAFwkTag::DELEGATOR, "Object_New success");

    std::string bundleName = abilityDelegatorArgs->GetTestBundleName();
    SetBundleName(aniEnv, arguments, argumentObject, bundleName);

    std::string testcaseName = abilityDelegatorArgs->GetTestCaseName();
    SetTestCaseNames(aniEnv, arguments, argumentObject, testcaseName);

    std::string className = abilityDelegatorArgs->GetTestRunnerClassName();
    SetTestRunnerClassName(aniEnv, arguments, argumentObject, className);

    auto parameters = abilityDelegatorArgs->GetTestParam();
    SetParameters(aniEnv, arguments, argumentObject, parameters);

    return argumentObject;
}
} // namespace AbilityDelegatorSts
} // namespace OHOS
