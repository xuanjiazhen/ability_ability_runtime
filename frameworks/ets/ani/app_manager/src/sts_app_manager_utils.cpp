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

#include "sts_app_manager_utils.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"

namespace OHOS {
namespace AppManagerSts {
    using namespace OHOS;
    using namespace AppExecFwk;
    bool createDouble(ani_env *env, ani_double value, ani_object &outObj)
    {
        ani_class cls;
        ANI_FAILED_AND_RETURN(env->FindClass("Lstd/core/Double;", &cls));
        ani_method ctor;
        ANI_FAILED_AND_RETURN(env->Class_FindMethod(cls, "<ctor>", "D:V", &ctor));
        ANI_FAILED_AND_RETURN(env->Object_New(cls, ctor, &outObj, value));
        return true;
    }

    ProcessState ConvertToProcessState(
        const AppExecFwk::AppProcessState &appProcessState, const bool &isFocused) {
        ProcessState processState;
        switch (appProcessState)
        {
        case AppExecFwk::AppProcessState::APP_STATE_CREATE:
        case AppExecFwk::AppProcessState::APP_STATE_READY:
            processState = ProcessState::STATE_CREATE;
            break;
        case AppExecFwk::AppProcessState::APP_STATE_FOREGROUND:
            processState = isFocused ? ProcessState::STATE_ACTIVE : ProcessState::STATE_FOREGROUND;
            break;
        case AppExecFwk::AppProcessState::APP_STATE_BACKGROUND:
            processState = ProcessState::STATE_BACKGROUND;
            break;
        case AppExecFwk::AppProcessState::APP_STATE_TERMINATED:
        case AppExecFwk::AppProcessState::APP_STATE_END:
            processState = ProcessState::STATE_DESTROY;
            break;
        default:
            processState = ProcessState::STATE_DESTROY;
            break;
        }
        return processState;
    }

    bool CreateClassObjByClassName(ani_env *env, const char *className, ani_class &cls, ani_object &outAniObj)
    {
        ANI_FAILED_AND_RETURN(env->FindClass(className, &cls));
        ani_method ctor;
        ANI_FAILED_AND_RETURN(env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor));
        outAniObj = {};
        ANI_FAILED_AND_RETURN(env->Object_New(cls, ctor, &outAniObj));
        return true;
    }

    bool ConvertToAniProcessInfo(ani_env *env, const RunningProcessInfo &processInfo, ani_object &outAniObj)
    {
        ani_class cls;
        if (!CreateClassObjByClassName(env, processInfoClassName, cls, outAniObj)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "CreateClassObjByClassName Faild");
            return false;
        }
        if (!SetFieldInt(env, cls, outAniObj, "pid", processInfo.pid_)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessInfo  set pid Faild");
            return false;
        }
        if (!SetFieldInt(env, cls, outAniObj, "uid", processInfo.uid_)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessInfo  set uid Faild");
            return false;
        }
        if (!SetFieldString(env, cls, outAniObj, "processName", processInfo.processName_)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessInfo  set processName Faild");
            return false;
        }
        ani_enum_item processInfoState {};
        OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToSts(env, processStateEnumName,
                ConvertToProcessState(processInfo.state_, processInfo.isFocused), processInfoState);
        if(!SetFieldRef(env, cls, outAniObj, "state", processInfoState)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessInfo  set state Faild");
            return false;
        }
        if (!SetFieldArrayString(env, cls, outAniObj, "bundleNames", processInfo.bundleNames)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessInfo  set bundleNames Faild");
            return false;
        }
        ani_enum_item bundleType {};
        OHOS::AAFwk::AniEnumConvertUtil::EnumConvert_NativeToSts(env, bundleTypeEnumName,
            static_cast<OHOS::AppExecFwk::BundleType>(processInfo.bundleType), bundleType);
        if(!SetFieldRef(env, cls, outAniObj, "bundleType", bundleType)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessInfo  set bundleType Faild");
            return false;
        }
        ani_object obj;
        if (createDouble(env, processInfo.appCloneIndex, obj)) {
            if (!SetFieldRef(env, cls, outAniObj, "appCloneIndex", static_cast<ani_ref>(obj))) {
                TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessInfo  set appCloneIndex Faild");
            }
        }
        return true;
    }

    bool ConvertToAniProcessData(ani_env *env, const ProcessData &processData, ani_object &outAniObj)
    {
        ani_class cls;
        if (!CreateClassObjByClassName(env, processDataClassName, cls, outAniObj)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "CreateClassObjByClassName Faild");
            return false;
        }
        if (!SetFieldString(env, cls, outAniObj, "bundleName", processData.bundleName)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessData  set bundleName Faild");
            return false;
        }
        if (!SetFieldDouble(env, cls, outAniObj, "pid", processData.pid)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessData  set pid Faild");
            return false;
        }
        if (!SetFieldDouble(env, cls, outAniObj, "uid", processData.uid)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessData  set uid Faild");
            return false;
        }
        if (!SetFieldDouble(env, cls, outAniObj, "state",
                static_cast<double>(ConvertToProcessState(processData.state, processData.isFocused)))) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessData  set state Faild");
            return false;
        }
        if (!SetFieldBoolean(env, cls, outAniObj, "isContinuousTask", processData.isContinuousTask)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessData  set isContinuousTask Faild");
            return false;
        }
        if (!SetFieldBoolean(env, cls, outAniObj, "isKeepAlive", processData.isKeepAlive)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessData  set isKeepAlive Faild");
            return false;
        }
        return true;
    }

    bool ConvertToAniAbilityStateData(ani_env *env, const AbilityStateData &abilityStateData, ani_object &outAniObj)
    {
        ani_class cls;
        if (!CreateClassObjByClassName(env, abilityStateDataClassName, cls, outAniObj)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "CreateClassObjByClassName Faild");
            return false;
        }
        if (!SetFieldString(env, cls, outAniObj, "moduleName", abilityStateData.moduleName)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAbilityStateData  set bundleName Faild");
            return false;
        }
        if (!SetFieldString(env, cls, outAniObj, "bundleName", abilityStateData.bundleName)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAbilityStateData  set bundleName Faild");
            return false;
        }
        if (!SetFieldString(env, cls, outAniObj, "abilityName", abilityStateData.abilityName)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAbilityStateData  set abilityName Faild");
            return false;
        }
        if (!SetFieldDouble(env, cls, outAniObj, "pid", abilityStateData.pid)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAbilityStateData  set pid Faild");
            return false;
        }
        if (!SetFieldDouble(env, cls, outAniObj, "uid", abilityStateData.uid)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAbilityStateData  set uid Faild");
            return false;
        }
        if (!SetFieldDouble(env, cls, outAniObj, "state", abilityStateData.abilityState)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAbilityStateData  set state Faild");
            return false;
        }
        if (!SetFieldDouble(env, cls, outAniObj, "abilityType", abilityStateData.abilityType)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAbilityStateData  set abilityType Faild");
            return false;
        }
        if (!SetFieldBoolean(env, cls, outAniObj, "isAtomicService", abilityStateData.isAtomicService)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAbilityStateData  set isAtomicService Faild");
            return false;
        }
        ani_object obj;
        if (createDouble(env, abilityStateData.appCloneIndex, obj)) {
            if (!SetFieldRef(env, cls, outAniObj, "appCloneIndex", static_cast<ani_ref>(obj))) {
                TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAbilityStateData  set appCloneIndex Faild");
            }
        }
        return true;
    }

    bool ConvertToAniAppStateData(ani_env *env, const AppStateData &appStateData, ani_object &outAniObj)
    {
        ani_class cls;
        if (!CreateClassObjByClassName(env, appStateDataClassName, cls, outAniObj)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "CreateClassObjByClassName Faild");
            return false;
        }
        if (!SetFieldString(env, cls, outAniObj, "bundleName", appStateData.bundleName)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAppStateData  set bundleName Faild");
            return false;
        }
        if (!SetFieldDouble(env, cls, outAniObj, "uid", appStateData.uid)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAppStateData  set abilityType Faild");
            return false;
        }
        if (!SetFieldDouble(env, cls, outAniObj, "state", appStateData.state)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAppStateData  set abilityType Faild");
            return false;
        }
        if (!SetFieldBoolean(env, cls, outAniObj, "isSplitScreenMode", appStateData.isSplitScreenMode)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAppStateData  set isSplitScreenMode Faild");
            return false;
        }
        if (!SetFieldBoolean(env, cls, outAniObj, "isFloatingWindowMode", appStateData.isFloatingWindowMode)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAppStateData  set isFloatingWindowMode Faild");
            return false;
        }
        return true;
    }

    ani_object newArrayClass(ani_env *env, int length)
    {
        ani_class arrayCls = nullptr;
        if (ANI_OK != env->FindClass("Lescompat/Array;", &arrayCls)){
            TAG_LOGD(AAFwkTag::DELEGATOR, "FindClass Lescompat/Array; Failed");
            return nullptr;
        }
        ani_method arrayCtor;
        if(ANI_OK != env->Class_FindMethod(arrayCls, "<ctor>", "I:V", &arrayCtor)){
            TAG_LOGD(AAFwkTag::DELEGATOR, "Class_FindMethod <ctor> Failed");
            return nullptr;
        }
        ani_object arrayObj = nullptr;
        if(ANI_OK != env->Object_New(arrayCls, arrayCtor, &arrayObj, length)){
            TAG_LOGD(AAFwkTag::DELEGATOR, "Object_New Array Faild");
            return arrayObj;
        }
        return arrayObj;
    }

    bool ConvertToStdVectorString(ani_env *env, const ani_object &ani_obj, std::vector<std::string> &std_Vstring)
    {
        ani_double length;
        ANI_FAILED_AND_RETURN(env->Object_GetPropertyByName_Double(ani_obj, "length", &length));
        for(int i = 0; i < int(length); i++) {
            ani_ref stringEntryRef;
            ANI_FAILED_AND_RETURN(env->Object_CallMethodByName_Ref(ani_obj,
                "$_get", "I:Lstd/core/Object;", &stringEntryRef, (ani_int)i));
            std::string std_string;
            if (!GetStdString(env, static_cast<ani_string>(stringEntryRef), std_string)) {
                TAG_LOGD(AAFwkTag::DELEGATOR, "GetStdString faild");
                return false;
            }
            std_Vstring.emplace_back(std_string);
        }
        return true;
    }

    bool ConvertToAniArrayProcessInfo(ani_env *env, const std::vector<AppExecFwk::RunningProcessInfo> &infos, ani_object &outAniObj)
    {
        outAniObj = newArrayClass(env, infos.size());
        if (outAniObj == nullptr) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "outAniObj is null, newArrayClass Faild");
            return false;
        }
        int index = 0;
        for (auto &it : infos) {
            ani_object infoObj;
            if (!ConvertToAniProcessInfo(env, it, infoObj)) {
                TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniProcessInfo Faild");
                return false;
            }
            ANI_FAILED_AND_RETURN(env->Object_CallMethodByName_Void(outAniObj,
                "$_set", "ILstd/core/Object;:V", index, infoObj));
            index++;
        }
        return true;
    }

    bool ConvertToAniAppStateDataArray(ani_env *env, const std::vector<AppExecFwk::AppStateData> &infos, ani_object &outAniObj)
    {
        outAniObj = newArrayClass(env, infos.size());
        if (outAniObj == nullptr) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "outAniObj is null, newArrayClass Faild");
            return false;
        }
        int index = 0;
        for (auto &it : infos) {
            ani_object infoObj;
            if (!ConvertToAniAppStateData(env, it, infoObj)) {
                TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniAppStateData Faild");
                return false;
            }
            ANI_FAILED_AND_RETURN(env->Object_CallMethodByName_Void(outAniObj,
                "$_set", "ILstd/core/Object;:V", index, infoObj));
            index++;
        }
        return true;
    }
    
    bool ConvertToAniAbilityFirstFrameStateData(ani_env *env, const AbilityFirstFrameStateData &firstFrameStateData, ani_object &outAniObj)
    {
        ani_status status = ANI_ERROR;
        ani_class cls;
        status = env->FindClass(firstFrameStateDataClassName, &cls);
        if (status != ANI_OK) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "FindClass faild");
            return false;
        }
        ani_method ctor;
        status = env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor);
        if (status != ANI_OK) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "Class_FindMethod faild. status: %{public}d", status);
            return false;
        }
        if (ANI_OK != env->Object_New(cls, ctor, &outAniObj)){
            TAG_LOGD(AAFwkTag::DELEGATOR, "Create Object Failed. className: %{public}s", firstFrameStateDataClassName);
            return false;
        }
        if (!SetFieldString(env, cls, outAniObj, "bundleName", firstFrameStateData.bundleName)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "SetFieldString bundleName faild");
            return false;
        }
        if (!SetFieldString(env, cls, outAniObj, "moduleName", firstFrameStateData.moduleName)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "SetFieldString moduleName faild");
            return false;
        }
        if (!SetFieldString(env, cls, outAniObj, "abilityName", firstFrameStateData.abilityName)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "SetFieldString abilityName faild");
            return false;
        }
        if (!SetFieldDouble(env, cls, outAniObj, "appIndex", static_cast<double>(firstFrameStateData.appIndex))) {
            std::cerr << "" << std::endl;
            TAG_LOGD(AAFwkTag::DELEGATOR, "SetFieldString appIndex faild");
            return false;
        }
        if (!SetFieldBoolean(env, cls, outAniObj, "isColdStart", firstFrameStateData.coldStart)) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "SetFieldString isColdStart faild");
            return false;
        }
        return true;
    }

	bool ConvertToAniRunningMultiAppInfoArray(ani_env *env,
		const std::vector<std::shared_ptr<AppExecFwk::RunningMultiAppInfo>> &infos, 
		ani_object &outAniObj)
	{
		outAniObj = newArrayClass(env, infos.size());
		if (outAniObj == nullptr) {
			TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniRunningMultiAppInfoArray: Failed to create array object");
			return false;
		}

		for (size_t i = 0; i < infos.size(); ++i) {
			if (infos[i] == nullptr) {
				continue;
			}

			ani_object infoObj;
			if (!ConvertToAniRunningMultiAppInfo(env, *(infos[i]), infoObj)) {
				TAG_LOGD(AAFwkTag::DELEGATOR, "ConvertToAniRunningMultiAppInfoArray: Failed to convert element at index %zu", i);
				return false;
			}

			ANI_FAILED_AND_RETURN(env->Object_CallMethodByName_Void(outAniObj,
				"$_set", "ILstd/core/Object;:V", static_cast<ani_int>(i), infoObj));
		}

		return true;
	}

    /**
	 * @brief 将整型vector转换为ANI数组对象
	 * @param env ANI环境指针
	 * @param list 要转换的整型vector
	 * @param arrayObj 返回的ANI数组对象， 失败为nullptr
	 * @return 成功返回true，失败返回 false
	 */
	bool ConvertIntVectorToAniArray(ani_env* env, const std::vector<int32_t>& list, ani_object &arrayObj)
	{
		if (env == nullptr) {
			TAG_LOGD(AAFwkTag::ABILITYMGR, "ConvertIntVectorToAniArray: env is null");
			return false;
		}

		arrayObj = newArrayClass(env, list.size());
		if (arrayObj == nullptr) {
			TAG_LOGD(AAFwkTag::ABILITYMGR, "ConvertIntVectorToAniArray: Failed to create array object");
			return false;
		}

		for (size_t i = 0; i < list.size(); ++i) {
			ani_object elementObj;
			if (!createDouble(env, static_cast<ani_double>(list[i]), elementObj)) {
				TAG_LOGD(AAFwkTag::ABILITYMGR, "ConvertIntVectorToAniArray: Failed to create element object at index %zu", i);
				continue;
			}

			if (ANI_OK != env->Object_CallMethodByName_Void(
				arrayObj, "$_set", "ILstd/core/Object;:V",
				static_cast<ani_int>(i), elementObj)) {
				TAG_LOGD(AAFwkTag::ABILITYMGR, "ConvertIntVectorToAniArray: Failed to set element at index %zu", i);
			}
		}

		return true;
	}

    AutoSTSReference::AutoSTSReference(ani_env *env, const ani_object &obj)
    :env_(env)
    {
        TAG_LOGD(AAFwkTag::APPMGR, "call");
        if (env_ == nullptr) return;
        if (obj == nullptr) return;
        ani_ref ref;
        if (env_->GlobalReference_Create(obj, &ref) != ANI_OK) return;
        refernece_ = std::make_shared<OHOS::AbilityRuntime::STSNativeReference>();
        refernece_->aniCls = nullptr;
        refernece_->aniRef = ref;
        refernece_->aniObj = obj;
    }
    AutoSTSReference::~AutoSTSReference()
    {
        TAG_LOGD(AAFwkTag::APPMGR, "call");
        if (!isInit()) return;
        if (refernece_->aniRef == nullptr) return;
        if (env_->GlobalReference_Delete(refernece_->aniRef) != ANI_OK) {
            TAG_LOGD(AAFwkTag::DELEGATOR, "GlobalReference_Delete faild");
        }
        refernece_ = nullptr;
    }
    bool AutoSTSReference::isInit()
    {
        return (env_ != nullptr && refernece_ != nullptr && refernece_->aniRef != nullptr);
    }
    bool AutoSTSReference::compare(const ani_object &obj)
    {
        if (!isInit()) return false;
        if (obj == nullptr) return false;
        ani_ref ref;
        if (env_->GlobalReference_Create(obj, &ref) != ANI_OK) return false;
        ani_boolean result = ANI_FALSE;
        env_->Reference_StrictEquals(ref, refernece_->aniRef, &result);
        env_->GlobalReference_Delete(ref);
        return AniBooleanToBool(result);
    }

} // namespace AppManagerSts
} // namespace OHOS