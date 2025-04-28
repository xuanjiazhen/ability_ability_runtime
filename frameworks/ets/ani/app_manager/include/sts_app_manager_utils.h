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

#ifndef OHOS_APPMANAGERSTS_APPMGR_STS_COMMON_UTILS_H
#define OHOS_APPMANAGERSTS_APPMGR_STS_COMMON_UTILS_H

#include <memory>
#include <string>
#include "ability_state_data.h"
#include "app_state_data.h"
#include "process_data.h"
#include "sts_runtime.h"
#include "running_process_info.h"
#include "process_data.h"
#include "hilog_tag_wrapper.h"
#include "ability_first_frame_state_data.h"
#include "running_multi_info.h"

namespace OHOS {
namespace AppManagerSts {
using OHOS::AppExecFwk::AppStateData;
using OHOS::AppExecFwk::AbilityStateData;
using OHOS::AppExecFwk::ProcessData;
using OHOS::AppExecFwk::RunningProcessInfo;
using OHOS::AppExecFwk::AbilityFirstFrameStateData;
    static const char *processInfoClassName = "Lapplication/ProcessInformation/ProcessInformationInner;";
    static const char *processDataClassName = "Lapplication/ProcessData/ProcessData;";
    static const char *abilityStateDataClassName = "Lapplication/AbilityStateData/AbilityStateData;";
    static const char *appStateDataClassName = "Lapplication/AppStateData/AppStateData;";
    static const char *processStateEnumName = "L@ohos/app/ability/appManager/appManager/ProcessState;";
    static const char *firstFrameStateDataClassName = "Lapplication/AbilityFirstFrameStateData/AbilityFirstFrameStateDataInner;";
    static const char *runningMultiAppInfoClassName = "L@ohos/app/ability/appManager/RunningMultiAppInfoInner;";
    static const char *runningAppCloneClassName = "L@ohos/app/ability/appManager/RunningAppCloneInner;";
    static const char *runningMultiInstanceInfoClassName = "L@ohos/app/ability/appManager/RunningMultiInstanceInfoInner;";
    static const char *bundleTypeEnumName = "L@ohos/bundle/bundleManager/bundleManager/BundleType;";

    static const char *onApplicationStateObserverSignature = 
        "Lapplication/ApplicationStateObserver/ApplicationStateObserver;:D";
    static const char *onApplicationStateObserverByArraySignature = 
        "Lapplication/ApplicationStateObserver/ApplicationStateObserver;Lescompat/Array;:D";
    static const char *onAppForegroundStateObserverSignature = 
        "Lapplication/AppForegroundStateObserver/AppForegroundStateObserver;:V";
    static const char *offAppForegroundStateObserverSignature = 
        "Lapplication/AppForegroundStateObserver/AppForegroundStateObserver;:V";
    static const char *onAbilityFirstFrameStateObserverSignature = 
        "Lapplication/AbilityFirstFrameStateObserver/AbilityFirstFrameStateObserver;Lstd/core/String;:V";
    static const char *offAbilityFirstFrameStateObserverSignature = 
        "Lapplication/AbilityFirstFrameStateObserver/AbilityFirstFrameStateObserver;:V";
    static const char *offAllByType = "Lstd/core/String;:V";
    static const char *getProcessInfoByBundleTypeSignature =
        "L@ohos/bundle/bundleManager/bundleManager/BundleType;:Lescompat/Array;";

    enum ProcessState
    {
        STATE_CREATE,
        STATE_FOREGROUND,
        STATE_ACTIVE,
        STATE_BACKGROUND,
        STATE_DESTROY
    };
    enum KeepAliveAppType
    {
        ALL = 0,
        THIRD_PARTY = 1,
        SYSTEM_APPTYPE = 2
    };
    enum KeepAliveSetter
    {
        SYSTEM_SETTER = 0,
        USER = 1
    };

    // Converts AppProcessState to ProcessState.
    ProcessState ConvertToProcessState(const AppExecFwk::AppProcessState &appProcessState, const bool &isFocused);
    // Creates a new double value and returns it as an ani_object.
    bool createDouble(ani_env *env, ani_double value, ani_object &outObj);
    // Creates a class object by class name.
    bool CreateClassObjByClassName(ani_env *env, const char *className, ani_class &cls, ani_object &outAniObj);
    // Converts RunningProcessInfo to an ani_object.
    bool ConvertToAniProcessInfo(ani_env *env, const RunningProcessInfo &processInfo, ani_object &outAniObj);
    // Converts ProcessData to an ani_object.
    bool ConvertToAniProcessData(ani_env *env, const ProcessData &processData, ani_object &outAniObj);
    // Converts AbilityStateData to an ani_object.
    bool ConvertToAniAbilityStateData(ani_env *env, const AbilityStateData &abilityStateData, ani_object &outAniObj);
    // Converts AppStateData to an ani_object.
    bool ConvertToAniAppStateData(ani_env *env, const AppStateData &appStateData, ani_object &outAniObj);
    // Converts ani_boolean to bool.
    inline bool AniBooleanToBool(ani_boolean value) {
        return value == ANI_TRUE;
    }
    // Converts bool to ani_boolean.
    inline ani_boolean BoolToAniBoolean(bool value) {
        return value ? ANI_TRUE : ANI_FALSE;
    }
    // Creates a new array class.
    ani_object newArrayClass(ani_env *env, const int length);
    // Converts ani_object to std::vector<std::string>.
    bool ConvertToStdVectorString(ani_env *env, const ani_object &ani_obj, std::vector<std::string> &std_Vstring);
    // Converts std::vector<AppExecFwk::RunningProcessInfo> to an ani_object.
    bool ConvertToAniArrayProcessInfo(ani_env *env, 
        const std::vector<AppExecFwk::RunningProcessInfo> &infos, ani_object &outAniObj);
    bool ConvertToAniAppStateDataArray(ani_env *env, const std::vector<AppExecFwk::AppStateData> &infos,
        ani_object &outAniObj);
        // Converts AbilityFirstFrameStateData to an ani_object.
    bool ConvertToAniAbilityFirstFrameStateData(ani_env *env,
        const AbilityFirstFrameStateData &firstFrameStateData, ani_object &outAniObj);

	bool ConvertToAniRunningMultiAppInfo(ani_env* env, const AppExecFwk::RunningMultiAppInfo &processInfo, ani_object &outAniObj);

	bool ConvertIntVectorToAniArray(ani_env* env, const std::vector<int32_t>& list, ani_object &arrayObj);

    class AutoSTSReference {
    public:
        AutoSTSReference(ani_env *env, const ani_object &obj);
        ~AutoSTSReference();
        bool isInit();
        bool compare(const ani_object &obj);
        std::shared_ptr<OHOS::AbilityRuntime::STSNativeReference> getReference() {
            return isInit() ? refernece_ : nullptr;
        }
    private:
        ani_env *env_ = nullptr;
        std::shared_ptr<OHOS::AbilityRuntime::STSNativeReference> refernece_ = nullptr;
    };

#define ANI_FAILED_AND_RETURN(status)                                                    \
do                                                                                       \
{                                                                                        \
    ani_status const local_status = (status);                                            \
    if (ani_status::ANI_OK != local_status)                                              \
    {                                                                                    \
        TAG_LOGD(AAFwkTag::APPMGR, "check status error: %{public}d", (int)local_status); \
        return false;                                                                    \
    }                                                                                    \
} while (0)

} // namespace AppManagerSts
} // namespace OHOS
#endif // OHOS_APPMANAGERSTS_APPMGR_STS_COMMON_UTILS_H