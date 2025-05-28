/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_SCHEDULER_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_SCHEDULER_H

#include "ability_scheduler_stub.h"
#include "ability_record.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityScheduler
 * AbilityScheduler is used to schedule ability kit lifecycle.
 */
class AbilityScheduler : public AbilitySchedulerStub, virtual RefBase {
public:
    AbilityScheduler();
    virtual ~AbilityScheduler();

    ErrCode ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &targetState,
        const sptr<SessionInfo>& sessionInfo) override;

    ErrCode ScheduleShareData(int32_t uniqueId) override;

    ErrCode SendResult(int requestCode, int resultCode, const Want& resultWant) override;

    const AbilityResult& GetResult() const;

    ErrCode ScheduleConnectAbility(const Want& want) override;

    ErrCode ScheduleDisconnectAbility(const Want& want) override;

    ErrCode SchedulePrepareTerminateAbility() override;

    ErrCode ScheduleCommandAbility(const Want& want, bool restart, int32_t startId) override;

    ErrCode ScheduleCommandAbilityWindow(const Want &want, const sptr<SessionInfo> &sessionInfo,
        int32_t winCmd) override;

    ErrCode ScheduleSaveAbilityState() override;

    ErrCode ScheduleRestoreAbilityState(const PacMap& inState) override;

    ErrCode GetFileTypes(const Uri& uri, const std::string& mimeTypeFilter,
        std::vector<std::string>& funcResult) override;

    ErrCode OpenFile(const Uri& uri, const std::string& mode, int& funcResult) override;

    ErrCode Insert(const Uri& uri, const NativeRdb::AmValuesBucket& value, int32_t& funcResult) override;

    ErrCode Update(const Uri& uri, const NativeRdb::AmValuesBucket& value,
        const NativeRdb::DataAbilityPredicates& predicates, int32_t& funcResult) override;

    ErrCode Delete(const Uri& uri, const NativeRdb::DataAbilityPredicates& predicates, int32_t& funcResult) override;

    ErrCode Query(const Uri& uri, const std::vector<std::string>& columns,
        const NativeRdb::DataAbilityPredicates& predicates, std::shared_ptr<AmAbsSharedResultSet>& funcResult) override;

    ErrCode Call(const Uri& uri, const std::string& method, const std::string& arg, const AppExecFwk::PacMap& pacMap,
        std::shared_ptr<PacMap>& funcResult) override;

    ErrCode GetType(const Uri& uri, std::string& funcResult) override;

    ErrCode OpenRawFile(const Uri& uri, const std::string& mode, int32_t& funcResult) override;

    ErrCode Reload(const Uri& uri, const PacMap& extras, bool& funcResult) override;

    ErrCode BatchInsert(const Uri& uri, const std::vector<NativeRdb::AmValuesBucket>& values,
        int32_t& funcResult) override;

    ErrCode NormalizeUri(const Uri& uri, AmUri& funcResult) override;
    ErrCode DenormalizeUri(const Uri& uri, AmUri& funcResult) override;
    virtual ErrCode ScheduleRegisterObserver(const Uri& uri, const sptr<IDataAbilityObserver>& dataObserver) override
    {
        return ERR_OK;
    };
    virtual ErrCode ScheduleUnregisterObserver(const Uri& uri, const sptr<IDataAbilityObserver>& dataObserver) override
    {
        return ERR_OK;
    };
    virtual ErrCode ScheduleNotifyChange(const Uri& uri) override
    {
        return ERR_OK;
    };
    virtual ErrCode ExecuteBatch(const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>>& operations,
        std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>>& funcResult) override
    {
        return ERR_OK;
    };
    virtual ErrCode NotifyContinuationResult(int32_t result) override
    {
        return ERR_OK;
    };
    virtual ErrCode ContinueAbility(const std::string& deviceId, uint32_t versionCode) override
    {
        return ERR_OK;
    };
    virtual ErrCode DumpAbilityInfo(const std::vector<std::string>& params) override
    {
        return ERR_OK;
    };
    virtual ErrCode CallRequest() override
    {
        return ERR_OK;;
    };
    virtual ErrCode OnExecuteIntent(const Want &want) override
    {
        return ERR_OK;
    };
    virtual ErrCode CreateModalUIExtension(const Want &want) override
    {
        return ERR_OK;
    };
    virtual ErrCode UpdateSessionToken(const sptr<IRemoteObject>& sessionToken) override
    {
        return ERR_OK;
    }

    virtual ErrCode ScheduleCollaborate(const Want &want) override
    {
        return ERR_OK;
    }

    virtual ErrCode ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message) override
    {
        return ERR_OK;
    }

    virtual ErrCode ScheduleAbilityRequestSuccess(const std::string &requestId,
        const AppExecFwk::ElementName &element) override
    {
        return ERR_OK;
    }
private:
    AbilityResult result_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_ABILITY_SCHEDULER_H
