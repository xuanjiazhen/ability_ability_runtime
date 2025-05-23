/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "ability_scheduler.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
AbilityScheduler::AbilityScheduler()
{}

AbilityScheduler::~AbilityScheduler()
{}

ErrCode AbilityScheduler::ScheduleAbilityTransaction(const Want& want, const LifeCycleStateInfo& targetState,
    const sptr<SessionInfo> &sessionInfo)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityScheduler ScheduleAbilityTransaction %d", targetState.state);
    (void)want;
    return ERR_OK;
}

ErrCode AbilityScheduler::ScheduleShareData(int32_t uniqueId)
{
    return ERR_OK;
}

ErrCode AbilityScheduler::SendResult(int requestCode, int resultCode, const Want& resultWant)
{
    TAG_LOGI(AAFwkTag::TEST, "AbilityScheduler SendResult %d resultCode %d", requestCode, resultCode);
    result_ = AbilityResult(requestCode, resultCode, resultWant);
    return ERR_OK;
}

const AbilityResult& AbilityScheduler::GetResult() const
{
    return result_;
}

ErrCode AbilityScheduler::ScheduleConnectAbility(const Want& want)
{
    (void)want;
    return ERR_OK;
}

ErrCode AbilityScheduler::ScheduleDisconnectAbility(const Want& want)
{
    return ERR_OK;
}

ErrCode AbilityScheduler::SchedulePrepareTerminateAbility()
{
    return ERR_OK;
}

ErrCode AbilityScheduler::ScheduleCommandAbility(const Want& want, bool restart, int32_t startId)
{
    return ERR_OK;
}

ErrCode AbilityScheduler::ScheduleCommandAbilityWindow(const Want &want, const sptr<SessionInfo> &sessionInfo,
    int32_t winCmd)
{
    return ERR_OK;
}

ErrCode AbilityScheduler::ScheduleSaveAbilityState()
{
    return ERR_OK;
}

ErrCode AbilityScheduler::ScheduleRestoreAbilityState(const PacMap& inState)
{
    return ERR_OK;
}

ErrCode AbilityScheduler::GetFileTypes(const Uri& uri, const std::string& mimeTypeFilter,
    std::vector<std::string>& funcResult)
{
    return ERR_OK;
}

ErrCode AbilityScheduler::OpenFile(const Uri& uri, const std::string& mode, int& funcResult)
{
    funcResult = -1;
    return ERR_OK;
}

ErrCode AbilityScheduler::Insert(const Uri& uri, const NativeRdb::AmValuesBucket& value, int32_t& funcResult)
{
    funcResult = -1;
    return ERR_OK;
}

ErrCode AbilityScheduler::Update(const Uri& uri, const NativeRdb::AmValuesBucket& value,
    const NativeRdb::DataAbilityPredicates& predicates, int32_t& funcResult)
{
    funcResult = -1;
    return ERR_OK;
}

ErrCode AbilityScheduler::Delete(const Uri& uri, const NativeRdb::DataAbilityPredicates& predicates,
    int32_t& funcResult)
{
    funcResult = -1;
    return ERR_OK;
}

ErrCode AbilityScheduler::Query(const Uri& uri, const std::vector<std::string>& columns,
    const NativeRdb::DataAbilityPredicates& predicates, std::shared_ptr<AmAbsSharedResultSet>& funcResult)
{
    funcResult = nullptr;
    return ERR_OK;
}

ErrCode AbilityScheduler::Call(const Uri& uri, const std::string& method, const std::string& arg,
    const AppExecFwk::PacMap& pacMap, std::shared_ptr<PacMap>& funcResult)
{
    funcResult = nullptr;
    return ERR_OK;
}

ErrCode AbilityScheduler::GetType(const Uri& uri, std::string& funcResult)
{
    return ERR_OK;
}

ErrCode AbilityScheduler::OpenRawFile(const Uri& uri, const std::string& mode, int32_t& funcResult)
{
    funcResult = -1;
    return ERR_OK;
}

ErrCode AbilityScheduler::Reload(const Uri& uri, const PacMap& extras, bool& funcResult)
{
    funcResult = false;
    return ERR_OK;
}

ErrCode AbilityScheduler::BatchInsert(const Uri& uri, const std::vector<NativeRdb::AmValuesBucket>& values,
    int32_t& funcResult)
{
    funcResult = -1;
    return ERR_OK;
}

ErrCode AbilityScheduler::NormalizeUri(const Uri& uri, AmUri& funcResult)
{
    Uri urivalue("");
    funcResult = AmUri(urivalue);
    return ERR_OK;
}

ErrCode AbilityScheduler::DenormalizeUri(const Uri& uri, AmUri& funcResult)
{
    Uri urivalue("");
    funcResult = AmUri(urivalue);
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
