/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "ability_thread.h"

#include "extension_ability_thread.h"
#include "fa_ability_thread.h"
#include "ui_ability_thread.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AppExecFwk {
void AbilityThread::AbilityThreadMain(const std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord, const std::shared_ptr<EventRunner> &mainRunner,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "begin");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null abilityRecord");
        return;
    }
    std::shared_ptr<AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null abilityInfo");
        return;
    }

    sptr<AbilityThread> thread = nullptr;
    if (abilityInfo->type == AbilityType::PAGE && abilityInfo->isStageBasedModel) {
        thread = sptr<AbilityRuntime::UIAbilityThread>::MakeSptr();
    } else if (abilityInfo->type == AbilityType::EXTENSION) {
        thread = sptr<AbilityRuntime::ExtensionAbilityThread>::MakeSptr();
    } else {
        thread = sptr<AbilityRuntime::FAAbilityThread>::MakeSptr();
    }
    if (thread == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null thread");
        return;
    }
    thread->Attach(application, abilityRecord, mainRunner, stageContext);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

void AbilityThread::AbilityThreadMain(const std::shared_ptr<OHOSApplication> &application,
    const std::shared_ptr<AbilityLocalRecord> &abilityRecord,
    const std::shared_ptr<AbilityRuntime::Context> &stageContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITY, "begin");
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null abilityRecord");
        return;
    }

    std::shared_ptr<AbilityInfo> abilityInfo = abilityRecord->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null abilityInfo");
        return;
    }

    sptr<AbilityThread> thread = nullptr;
    if (abilityInfo->type == AbilityType::PAGE && abilityInfo->isStageBasedModel) {
        thread = sptr<AbilityRuntime::UIAbilityThread>::MakeSptr();
    } else if (abilityInfo->type == AbilityType::EXTENSION) {
        thread = sptr<AbilityRuntime::ExtensionAbilityThread>::MakeSptr();
    } else {
        thread = sptr<AbilityRuntime::FAAbilityThread>::MakeSptr();
    }
    if (thread == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null thread");
        return;
    }
    thread->Attach(application, abilityRecord, stageContext);
    TAG_LOGD(AAFwkTag::ABILITY, "end");
}

ErrCode AbilityThread::ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState,
    const sptr<SessionInfo>& sessionInfo)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleShareData(int32_t requestCode)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleConnectAbility(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleDisconnectAbility(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleCommandAbilityWindow(
    const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, int32_t winCmd)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::SchedulePrepareTerminateAbility()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleSaveAbilityState()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleRestoreAbilityState(const PacMap &state)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::SendResult(int requestCode, int resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter,
    std::vector<std::string>& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::OpenFile(const Uri &uri, const std::string &mode, int& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::OpenRawFile(const Uri &uri, const std::string &mode, int32_t& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::Insert(const Uri &uri, const NativeRdb::AmValuesBucket &value, int32_t& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::Call(const Uri &uri, const std::string &method, const std::string &arg,
    const AppExecFwk::PacMap &pacMap, std::shared_ptr<PacMap>& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::Update(const Uri &uri, const NativeRdb::AmValuesBucket &value,
    const NativeRdb::DataAbilityPredicates &predicates, int32_t& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates, int32_t& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::Query(const Uri &uri, const std::vector<std::string> &columns,
    const NativeRdb::DataAbilityPredicates &predicates, std::shared_ptr<AmAbsSharedResultSet>& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::GetType(const Uri &uri, std::string& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::Reload(const Uri &uri, const PacMap &extras, bool& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::BatchInsert(const Uri &uri, const std::vector<NativeRdb::AmValuesBucket> &values,
    int32_t& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ContinueAbility(const std::string &deviceId, uint32_t versionCode)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::NotifyContinuationResult(int32_t result)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::NormalizeUri(const Uri &uri, AmUri& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::DenormalizeUri(const Uri &uri, AmUri& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleRegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleUnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleNotifyChange(const Uri &uri)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::DumpAbilityInfo(const std::vector<std::string> &params)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::CallRequest()
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::OnExecuteIntent(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations,
    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>>& funcResult)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::CreateModalUIExtension(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::UpdateSessionToken(const sptr<IRemoteObject>& sessionToken)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleCollaborate(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
    const std::string &message)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}

ErrCode AbilityThread::ScheduleAbilityRequestSuccess(const std::string &requestId,
    const AppExecFwk::ElementName &element)
{
    TAG_LOGD(AAFwkTag::ABILITY, "called");
    return ERR_OK;
}
} // namespace AppExecFwk
} // namespace OHOS
