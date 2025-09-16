#include <map>

#include "ohos.distributedmissionmanager.proj.hpp"
#include "ohos.distributedmissionmanager.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "ability_manager_client.h"
#include "ani_common_want.h"
#include "ani_error_utils.h"
#include "ani_observer_utils.h"
#include "ani_remotelistener_utils.h"
#include "dms_continueInfo.h"
#include "hilog_tag_wrapper.h"
#include "mission_continue_interface.h"

namespace {
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

constexpr size_t VALUE_BUFFER_SIZE = 128;
constexpr const char* ON_CONTINUTESTATE_CHANGE = "continueStateChange";

std::mutex registrationLock_;
std::map<std::string, sptr<ani_remotelistenerutils::AniRemoteMissionListener>> registrationMap_;

std::recursive_mutex onLock_;
std::map<std::string, std::vector<sptr<ani_observerutils::MissionObserver>>> registrationOfOnMap_;

void OnContinueStateChange(::taihe::callback_view<void(::ohos::distributedmissionmanager::ContinueCallbackInfo const& info)> f, uintptr_t opq)
{
    std::lock_guard<std::recursive_mutex> lock(onLock_);
    auto &cbVec = registrationOfOnMap_[ON_CONTINUTESTATE_CHANGE];

    ani_object callbackObj = reinterpret_cast<ani_object>(opq);
    ani_ref callbackRef = nullptr;
    sptr<ani_observerutils::MissionObserver> registrationOfOnItem;
    auto item = registrationOfOnMap_.find(ON_CONTINUTESTATE_CHANGE);
    if (item != registrationOfOnMap_.end()) {
        TAG_LOGI(AAFwkTag::MISSION, "registrationOfOn exits");
        registrationOfOnItem = registrationOfOnMap_[ON_CONTINUTESTATE_CHANGE];
    } else {
        TAG_LOGI(AAFwkTag::MISSION, "registrationOfOn not exits");
        registrationOfOnItem = new (std::nothrow) ani_observerutils::MissionObserver();
    }
    registrationOfOnItem->AddCallback(f, callbackObj);

    int result = DmsSaClient::GetInstance().AddListener(ON_CONTINUTESTATE_CHANGE, registrationOfOnItem);
    if (result == OHOS::AAFwk::NO_ERROR) {
        TAG_LOGI(AAFwkTag::MISSION, "add registrationOfOn success");
        registrationOfOnMap_[ON_CONTINUTESTATE_CHANGE] = registrationOfOnItem;
    } else {
        TAG_LOGI(AAFwkTag::MISSION, "add registrationOfOn failed");
    }
}

void OffContinueStateChange(::taihe::optional_view<uintptr_t> opq)
{
    std::lock_guard<std::mutex> autoLock(onLock_);
    sptr<ani_observerutils::MissionObserver> registrationOfOnItem;
    auto item = registrationOfOnMap_.find(ON_CONTINUTESTATE_CHANGE);
    if (item != registrationOfOnMap_.end()) {
        TAG_LOGI(AAFwkTag::MISSION, "registrationOfOff exits");
        registrationOfOnItem = registrationOfOnMap_[ON_CONTINUTESTATE_CHANGE];
    } else {
        TAG_LOGI(AAFwkTag::MISSION, "registrationOfOff not exits");
        return;
    }
    ani_env *env = taihe::get_env();
    ani_ref jsCallbackRef = nullptr;
    ani_object callbackObj = nullptr;
    if (opq.has_value()) {
        callbackObj = reinterpret_cast<ani_object>(opq.value());
    }
    ani_observerutils::GlobalRefGuard guard(env, callbackObj);
    if (callbackObj != nullptr && !guard) {
        ZLOGE("GlobalRefGuard is false!");
        return;
    }
    jsCallbackRef = guard.get();

    registrationOfOnItem->DeleteCallback(jsCallbackRef);

    if (!registrationOfOnItem->IsCalllbackListEmpty()) {
        TAG_LOGI(AAFwkTag::MISSION, "callback remained");
        return;
    }
    DmsSaClient::GetInstance().DelListener(onCB->type, onCB->onRegistration);
    if (onCB->result == NO_ERROR) {
        TAG_LOGI(AAFwkTag::MISSION, "remove registration");
        registrationOfOnMap_.erase("continueStateChange");
    }
}

void StartSyncRemoteMissionsSync(::MissionParameter::MissionParameter const& parameter)
{
    std::string deviceId(parameter.deviceId);
    if (deviceId.length() > VALUE_BUFFER_SIZE) {
        TAG_LOGE(AAFwkTag::MISSION, "deviceId length not correct");
        std::string errInfo = "Parameter error. The length of \"deviceId\" must be less than " +
            std::to_string(VALUE_BUFFER_SIZE);
        ani_errorutils::ThrowError(ErrorCode::PARAMETER_CHECK_FAILED, errInfo.c_str());
        return;
    }
    int result = AbilityManagerClient::GetInstance()->StartSyncRemoteMissions(deviceId,
        parameter.fixConflict, parameter.tag);
    if (result != 0) {
        TAG_LOGE(AAFwkTag::MISSION, "StartSyncRemoteMissions failed, %{public}d", result);
        int32_t errCode = ani_errorutils::ErrorCodeReturn(result);
        std::string errInfo = ani_errorutils::ErrorMessageReturn(errCode);
        ani_errorutils::ThrowError(ErrorCode::PARAMETER_CHECK_FAILED, errInfo.c_str());
    }
}

void StopSyncRemoteMissionsSync(::MissionDeviceInfo::MissionDeviceInfo const& parameter)
{
    std::string deviceId(parameter.deviceId);
    if (deviceId.length() > VALUE_BUFFER_SIZE) {
        TAG_LOGE(AAFwkTag::MISSION, "deviceId length not correct");
        std::string errInfo = "Parameter error. The length of \"deviceId\" must be less than " +
            std::to_string(VALUE_BUFFER_SIZE);
        ani_errorutils::ThrowError(ErrorCode::PARAMETER_CHECK_FAILED, errInfo.c_str());
        return;
    }
    int result = AbilityManagerClient::GetInstance()->StopSyncRemoteMissions(deviceId);
    if (result != 0) {
        TAG_LOGE(AAFwkTag::MISSION, "StopSyncRemoteMissions failed, %{public}d", result);
        int32_t errCode = ani_errorutils::ErrorCodeReturn(result);
        std::string errInfo = ani_errorutils::ErrorMessageReturn(errCode);
        ani_errorutils::ThrowError(ErrorCode::PARAMETER_CHECK_FAILED, errInfo.c_str());
    }
}

void RegisterMissionListenerSync(::MissionDeviceInfo::MissionDeviceInfo const& parameter,
    ::MissionCallbacks::MissionCallback const& options)
{
    int result = OHOS::AAFwk::NO_ERROR;
    std::string deviceId(parameter.deviceId);
    std::lock_guard<std::mutex> autoLock(registrationLock_);
    sptr<ani_remotelistenerutils::AniRemoteMissionListener> registration;
    auto item = registrationMap_.find(deviceId);
    if (item != registrationMap_.end()) {
        TAG_LOGI(AAFwkTag::MISSION, "registration exits");
        registration = registrationMap_[deviceId];
    } else {
        TAG_LOGI(AAFwkTag::MISSION, "registration not exits");
        registration = new (std::nothrow) ani_remotelistenerutils::AniRemoteMissionListener(options);
    }
    if (registration == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null missionRegistration");
        result = -1;
        int32_t errCode = ani_errorutils::ErrorCodeReturn(result);
        std::string errInfo = ani_errorutils::ErrorMessageReturn(errCode);
        ani_errorutils::ThrowError(ErrorCode::PARAMETER_CHECK_FAILED, errInfo.c_str());
        return;
    }

    result = AbilityManagerClient::GetInstance()->RegisterMissionListener(deviceId, registration);
    if (result == OHOS::AAFwk::NO_ERROR) {
        TAG_LOGI(AAFwkTag::MISSION, "add registration");
        registrationMap_[deviceId] = registration;
    }
    if (result != 0) {
        TAG_LOGE(AAFwkTag::MISSION, "StopSyncRemoteMissions failed, %{public}d", result);
        int32_t errCode = ani_errorutils::ErrorCodeReturn(result);
        std::string errInfo = ani_errorutils::ErrorMessageReturn(errCode);
        ani_errorutils::ThrowError(ErrorCode::PARAMETER_CHECK_FAILED, errInfo.c_str());
    }
}

void UnRegisterMissionListenerSync(::MissionDeviceInfo::MissionDeviceInfo const& parameter)
{
    int result = OHOS::AAFwk::NO_ERROR;
    std::string deviceId(parameter.deviceId);
    std::lock_guard<std::mutex> autoLock(registrationLock_);
    sptr<ani_remotelistenerutils::AniRemoteMissionListener> registration;
    auto item = registrationMap_.find(deviceId);
    if (item != registrationMap_.end()) {
        TAG_LOGI(AAFwkTag::MISSION, "registration exits");
        registration = registrationMap_[deviceId];
    } else {
        TAG_LOGI(AAFwkTag::MISSION, "registration not exits");
        result = INVALID_PARAMETERS_ERR;
        int32_t errCode = ani_errorutils::ErrorCodeReturn(result);
        std::string errInfo = ani_errorutils::ErrorMessageReturn(errCode);
        ani_errorutils::ThrowError(errCode, errInfo.c_str());
        return;
    }
    result = AbilityManagerClient::GetInstance()->UnRegisterMissionListener(deviceId, registration);
    if (result == OHOS::AAFwk::NO_ERROR) {
        TAG_LOGI(AAFwkTag::MISSION, "remove registration");
        registration->Release();
        registrationMap_.erase(deviceId);
    } else {
        int32_t errCode = ani_errorutils::ErrorCodeReturn(result);
        std::string errInfo = ani_errorutils::ErrorMessageReturn(errCode);
        ani_errorutils::ThrowError(errCode, errInfo.c_str());
    }
}

void ContinueMissionSync(::ContinueMissionInfo::ContinueMissionInfo const& parameter)
{
    ani_env *env = taihe::get_env();
    AAFwk::WantParams wantParams;
    OHOS::AAFwk::ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.dstDeviceId = std::string(parameter.dstDeviceId);
    continueMissionInfo.srcDeviceId = std::string(parameter.srcDeviceId);
    continueMissionInfo.bundleName = std::string(parameter.bundleName);
    continueMissionInfo.srcBundleName = std::string(parameter.srcBundleName.value());
    if (parameter.continueType.has_value()) {
        continueMissionInfo.continueType = std::string(parameter.continueType.value());
    }
    UnwrapWantParams(env, reinterpret_cast<ani_ref>(parameter.wantParam), continueMissionInfo.wantParams);

    sptr<ani_remotelistenerutils::AniMissionContinue> continuation(new (std::nothrow) ani_remotelistenerutils::AniMissionContinue());
    int result = AAFwk::AbilityManagerClient::GetInstance()->ContinueMission(
            continueMissionInfo, continuation);
    if (result != OHOS::AAFwk::NO_ERROR) {
        int32_t errCode = ani_errorutils::ErrorCodeReturn(result);
        std::string errInfo = ani_errorutils::ErrorMessageReturn(errCode);
        ani_errorutils::ThrowError(errCode, errInfo.c_str());
    }
}

void ContinueMissionWithOptionSync(::ContinueDeviceInfo::ContinueDeviceInfo const& parameter,
    ::ContinueCallback::ContinueCallback const& options)
{
    ani_env *env = taihe::get_env();
    AAFwk::WantParams wantParams;
    OHOS::AAFwk::ContinueMissionInfo continueMissionInfo;
    continueMissionInfo.dstDeviceId = std::string(parameter.dstDeviceId);
    continueMissionInfo.srcDeviceId = std::string(parameter.srcDeviceId);
    UnwrapWantParams(env, reinterpret_cast<ani_ref>(parameter.wantParam), continueMissionInfo.wantParams);

    sptr<ani_observerutils::AniMissionContinue> continuation(new (std::nothrow) ani_remotelistenerutils::AniMissionContinue(options));
    int result = AAFwk::AbilityManagerClient::GetInstance()->ContinueMission(
            continueMissionInfo.srcDeviceId, continueMissionInfo.dstDeviceId,
            parameter.missionId, continuation, continueMissionInfo.wantParams);
    if (result != OHOS::AAFwk::NO_ERROR) {
        int32_t errCode = ani_errorutils::ErrorCodeReturn(result);
        std::string errInfo = ani_errorutils::ErrorMessageReturn(errCode);
        ani_errorutils::ThrowError(errCode, errInfo.c_str());
    }
}

}  // namespace

// Since these macros are auto-generate, lint will cause false positive.
// NOLINTBEGIN
TH_EXPORT_CPP_API_OnContinueStateChange(OnContinueStateChange);
TH_EXPORT_CPP_API_OffContinueStateChange(OffContinueStateChange);
TH_EXPORT_CPP_API_StartSyncRemoteMissionsSync(StartSyncRemoteMissionsSync);
TH_EXPORT_CPP_API_StopSyncRemoteMissionsSync(StopSyncRemoteMissionsSync);
TH_EXPORT_CPP_API_RegisterMissionListenerSync(RegisterMissionListenerSync);
TH_EXPORT_CPP_API_UnRegisterMissionListenerSync(UnRegisterMissionListenerSync);
TH_EXPORT_CPP_API_ContinueMissionSync(ContinueMissionSync);
TH_EXPORT_CPP_API_ContinueMissionWithOptionSync(ContinueMissionWithOptionSync);
// NOLINTEND
