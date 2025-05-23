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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_CLIENT_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_CLIENT_H

#include <gtest/gtest.h>
#include <mutex>
#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "ability_scheduler_interface.h"
#include "abs_shared_result_set.h"
#include "data_ability_predicates.h"
#include "fa_ability_context.h"
#include "iremote_object.h"
#include "iremote_stub.h"
#include "values_bucket.h"
#include "want.h"

#define OPENFILENUM (1246)
#define INSERTNUM (2345)
#define UPDATENUM (3456)
#define OPENRAWFILENUM (5678)
#define DELETENUM (6789)
#define BATCHINSERTNUM (789)

namespace OHOS {
namespace AppExecFwk {
using Configuration = AppExecFwk::Configuration;
class MockAbility : public AbilityContext {
public:
    MockAbility() = default;
    virtual ~MockAbility() = default;
};

class MockAbilityThread : public IRemoteStub<AAFwk::IAbilityScheduler> {
public:
    virtual ErrCode ScheduleAbilityTransaction(const Want& want, const LifeCycleStateInfo& targetState,
        const sptr<SessionInfo> &sessionInfo)
    {
        return ERR_OK;
    }
    virtual ErrCode SendResult(int32_t requestCode, int32_t resultCode, const Want& resultWant)
    {
        return ERR_OK;
    };
    virtual ErrCode ScheduleConnectAbility(const Want& want)
    {
        return ERR_OK;
    };
    virtual ErrCode ScheduleDisconnectAbility(const Want& want)
    {
        return ERR_OK;
    };
    virtual ErrCode ScheduleCommandAbility(const Want& want, bool restart, int32_t startId)
    {
        return ERR_OK;
    };
    virtual ErrCode ScheduleCommandAbilityWindow(const Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
        int32_t winCmd)
    {
        return ERR_OK;
    };
    virtual ErrCode ScheduleSaveAbilityState()
    {
        return ERR_OK;
    };
    virtual ErrCode ScheduleRestoreAbilityState(const PacMap& inState)
    {
        return ERR_OK;
    };
    virtual ErrCode GetFileTypes(const Uri& uri, const std::string& mimeTypeFilter,
        std::vector<std::string>& funcResult)
    {
        return ERR_OK;
    };
    virtual ErrCode OpenFile(const Uri& uri, const std::string& mode, int& funcResult)
    {
        funcResult = OPENFILENUM;
        return ERR_OK;
    };
    virtual ErrCode OpenRawFile(const Uri& uri, const std::string& mode, int32_t& funcResult)
    {
        funcResult = OPENRAWFILENUM;
        return ERR_OK;
    };
    virtual ErrCode Insert(const Uri& uri, const NativeRdb::AmValuesBucket& value, int32_t& funcResult)
    {
        funcResult = INSERTNUM;
        return ERR_OK;
    };
    virtual ErrCode Update(
        const Uri& uri, const NativeRdb::AmValuesBucket& value, const NativeRdb::DataAbilityPredicates& predicates,
        int32_t& funcResult)
    {
        funcResult = UPDATENUM;
        return ERR_OK;
    };
    virtual ErrCode Delete(const Uri& uri, const NativeRdb::DataAbilityPredicates& predicates, int32_t& funcResult)
    {
        funcResult = DELETENUM;
        return ERR_OK;
    };
    virtual ErrCode Query(
        const Uri& uri, const std::vector<std::string>& columns, const NativeRdb::DataAbilityPredicates& predicates,
        std::shared_ptr<AmAbsSharedResultSet>& funcResult)
    {
        funcResult = std::make_shared<AmAbsSharedResultSet>(
            std::make_shared<NativeRdb::AbsSharedResultSet>("resultset"));
        return ERR_OK;
    };
    ErrCode Call(
        const Uri& uri, const std::string& method, const std::string& arg, const AppExecFwk::PacMap& pacMap,
        std::shared_ptr<PacMap>& funcResult)
    {
        funcResult = nullptr;
        return ERR_OK;
    }
    virtual ErrCode GetType(const Uri& uri, std::string& funcResult)
    {
        funcResult = "type";
        return ERR_OK;
    };
    virtual ErrCode Reload(const Uri& uri, const PacMap& extras, bool& funcResult)
    {
        funcResult = true;
        return ERR_OK;
    };
    virtual ErrCode BatchInsert(const Uri& uri, const std::vector<NativeRdb::AmValuesBucket>& values,
        int32_t& funcResult)
    {
        funcResult = BATCHINSERTNUM;
        return ERR_OK;
    };
    virtual ErrCode ScheduleRegisterObserver(const Uri& uri, const sptr<AAFwk::IDataAbilityObserver>& dataObserver)
    {
        return ERR_OK;
    };
    virtual ErrCode ScheduleUnregisterObserver(const Uri& uri, const sptr<AAFwk::IDataAbilityObserver>& dataObserver)
    {
        return ERR_OK;
    };
    virtual ErrCode ScheduleNotifyChange(const Uri& uri)
    {
        return ERR_OK;
    };
    virtual ErrCode NormalizeUri(const Uri& uri, AmUri& funcResult)
    {
        Uri tmpUri("dataability:///test.aaa");
        funcResult = AmUri(tmpUri);
        return ERR_OK;
    };
    virtual ErrCode DenormalizeUri(const Uri& uri, AmUri& funcResult)
    {
        Uri tmpUri("dataability:///test.aaa");
        funcResult = AmUri(tmpUri);
        return ERR_OK;
    };
    virtual ErrCode ExecuteBatch(
        const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>>& operations,
        std::vector<std::shared_ptr<DataAbilityResult>>& funcResult)
    {
        return ERR_OK;
    };
    virtual ErrCode NotifyContinuationResult(int32_t result)
    {
        return ERR_OK;
    };
    virtual ErrCode ContinueAbility(const std::string& deviceId, uint32_t versionCode)
    {
        return ERR_OK;
    };
    virtual ErrCode DumpAbilityInfo(const std::vector<std::string>& params)
    {
        return ERR_OK;
    };
    virtual ErrCode CallRequest()
    {
        return ERR_OK;
    };
    virtual ErrCode OnExecuteIntent(const Want &want)
    {
        return ERR_OK;
    }

    virtual ErrCode ScheduleShareData(int32_t uniqueId)
    {
        return ERR_OK;
    }

    virtual ErrCode SchedulePrepareTerminateAbility()
    {
        return ERR_OK;
    }
    virtual ErrCode CreateModalUIExtension(const Want &want)
    {
        return ERR_OK;
    }
    virtual ErrCode UpdateSessionToken(const sptr<IRemoteObject>& sessionToken)
    {
        return ERR_OK;
    }
    virtual ErrCode ScheduleCollaborate(const Want &want)
    {
        return ERR_OK;
    }
    virtual ErrCode ScheduleAbilityRequestFailure(const std::string &requestId, const AppExecFwk::ElementName &element,
        const std::string &message)
    {
        return ERR_OK;
    }

    virtual ErrCode ScheduleAbilityRequestSuccess(const std::string &requestId, const AppExecFwk::ElementName &element)
    {
        return ERR_OK;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_ABILITY_MANAGER_CLIENT_H
