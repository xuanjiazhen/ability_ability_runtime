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
#include "ability_scheduler_proxy.h"

#include "ability_manager_errors.h"
#include "data_ability_observer_interface.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "error_msg_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ipc_capacity_wrap.h"
#include "ishared_result_set.h"
#include "session_info.h"
#include "values_bucket.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int64_t SCHEDULE_IPC_LOG_TIME = 10000;
}
bool AbilitySchedulerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(AbilitySchedulerProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return false;
    }
    return true;
}

bool AbilitySchedulerProxy::ScheduleAbilityTransaction(const Want &want, const LifeCycleStateInfo &stateInfo,
    sptr<SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "begin");
    auto start = std::chrono::system_clock::now();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!WriteInterfaceToken(data)) {
        return false;
    }
    auto msgKey = AbilityRuntime::ErrorMgsUtil::BuildErrorKey(reinterpret_cast<uintptr_t>(this),
        "ScheduleAbilityTransaction");
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write want failed");
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "write want failed");
        return false;
    }
    data.WriteParcelable(&stateInfo);
    if (sessionInfo) {
        SessionInfo tmpInfo = *sessionInfo;
        tmpInfo.want = Want();
        if (!data.WriteBool(true) || !data.WriteParcelable(&tmpInfo)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "write sessionInfo failed");
            AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "write sessionInfo failed");
            return false;
        }
    } else if (!data.WriteBool(false)) {
        return false;
    }
    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_ABILITY_TRANSACTION, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey,
            std::string("ScheduleAbilityTransaction ipc error " + std::to_string(err)));
        return false;
    }
    int64_t cost = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now() - start).count();
    if (cost > SCHEDULE_IPC_LOG_TIME) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ScheduleAbilityTransaction proxy cost %{public}" PRId64 "mirco seconds,"
            " data size: %{public}zu", cost, data.GetWritePosition());
    } else {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ScheduleAbilityTransaction proxy cost %{public}" PRId64 "mirco seconds,"
            " data size: %{public}zu", cost, data.GetWritePosition());
    }
    return true;
}

void AbilitySchedulerProxy::ScheduleShareData(const int32_t &uniqueId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return;
    }
    if (!data.WriteInt32(uniqueId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "uniqueId write failed");
        return;
    }
    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_SHARE_DATA, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
    return;
}

void AbilitySchedulerProxy::SendResult(int requestCode, int resultCode, const Want &resultWant)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteInt32(requestCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write requestCode failed");
        return;
    }
    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write resultCode failed");
        return;
    }
    if (!data.WriteParcelable(&resultWant)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable");
        return;
    }
    int32_t err = SendTransactCmd(IAbilityScheduler::SEND_RESULT, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::ScheduleConnectAbility(const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "fail to WriteParcelable");
        return;
    }
    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_ABILITY_CONNECT, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "fail, err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::ScheduleDisconnectAbility(const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "fail to WriteParcelable");
        return;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_ABILITY_DISCONNECT, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "fail, err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::ScheduleCommandAbility(const Want &want, bool restart, int startId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "WriteParcelable failed");
        return;
    }
    if (!data.WriteBool(restart)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "WriteBool failed");
        return;
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "WriteInt32,startId:%{public}d", startId);
    if (!data.WriteInt32(startId)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "fail to WriteInt32");
        return;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_ABILITY_COMMAND, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "fail, err: %{public}d", err);
    }
}

bool AbilitySchedulerProxy::SchedulePrepareTerminateAbility()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to write interface");
        return false;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_ABILITY_PREPARE_TERMINATE, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed, err: %{public}d", err);
        return false;
    }
    return true;
}

void AbilitySchedulerProxy::ScheduleCommandAbilityWindow(const Want &want, const sptr<SessionInfo> &sessionInfo,
    WindowCommand winCmd)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "writeParcelable failed");
        return;
    }
    if (!data.WriteParcelable(sessionInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "writeParcelable failed");
        return;
    }
    if (!data.WriteInt32(winCmd)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32");
        return;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_ABILITY_COMMAND_WINDOW, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::ScheduleSaveAbilityState()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_SAVE_ABILITY_STATE, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::ScheduleRestoreAbilityState(const PacMap &inState)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteParcelable(&inState)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "writeParcelable err");
        return;
    }
    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_RESTORE_ABILITY_STATE, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
}

std::vector<std::string> AbilitySchedulerProxy::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    std::vector<std::string> types;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return types;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return types;
    }

    if (!data.WriteString(mimeTypeFilter)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteString mimeTypeFilter");
        return types;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_GETFILETYPES, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }

    if (!reply.ReadStringVector(&types)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadStringVector types");
    }

    return types;
}

int AbilitySchedulerProxy::OpenFile(const Uri &uri, const std::string &mode)
{
    int fd = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return fd;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return fd;
    }

    if (!data.WriteString(mode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteString mode");
        return fd;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_OPENFILE, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return fd;
    }

    fd = reply.ReadFileDescriptor();
    if (fd == -1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadInt32 fd");
        return fd;
    }

    return fd;
}

int AbilitySchedulerProxy::OpenRawFile(const Uri &uri, const std::string &mode)
{
    int fd = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return fd;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return fd;
    }

    if (!data.WriteString(mode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteString mode");
        return fd;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_OPENRAWFILE, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return fd;
    }

    if (!reply.ReadInt32(fd)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadInt32 fd");
        return fd;
    }

    return fd;
}

int AbilitySchedulerProxy::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    int index = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return index;
    }

    if (!value.Marshalling(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable value");
        return index;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_INSERT, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return index;
    }

    if (!reply.ReadInt32(index)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadInt32 index");
        return index;
    }

    return index;
}

std::shared_ptr<AppExecFwk::PacMap> AbilitySchedulerProxy::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return nullptr;
    }

    if (!data.WriteString(method)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteString method");
        return nullptr;
    }

    if (!data.WriteString(arg)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteString arg");
        return nullptr;
    }

    if (!data.WriteParcelable(&pacMap)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable pacMap");
        return nullptr;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_CALL, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return nullptr;
    }
    std::shared_ptr<AppExecFwk::PacMap> result(reply.ReadParcelable<AppExecFwk::PacMap>());
    if (!result) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "readParcelable value null");
        return nullptr;
    }
    return result;
}

int AbilitySchedulerProxy::Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
    const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return index;
    }

    if (!value.Marshalling(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable value");
        return index;
    }

    if (!data.WriteParcelable(&predicates)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable predicates");
        return index;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_UPDATE, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return index;
    }

    if (!reply.ReadInt32(index)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadInt32 index");
        return index;
    }

    return index;
}

int AbilitySchedulerProxy::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return index;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return index;
    }

    if (!data.WriteParcelable(&predicates)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable predicates");
        return index;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_DELETE, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return index;
    }

    if (!reply.ReadInt32(index)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadInt32 index");
        return index;
    }

    return index;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> AbilitySchedulerProxy::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return nullptr;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return nullptr;
    }

    if (!data.WriteStringVector(columns)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteStringVector columns");
        return nullptr;
    }

    if (!data.WriteParcelable(&predicates)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable predicates");
        return nullptr;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_QUERY, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return nullptr;
    }
    return OHOS::NativeRdb::ISharedResultSet::ReadFromParcel(reply);
}

std::string AbilitySchedulerProxy::GetType(const Uri &uri)
{
    std::string type;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return type;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return type;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_GETTYPE, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return type;
    }

    type = reply.ReadString();
    if (type.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadString type");
        return type;
    }

    return type;
}

bool AbilitySchedulerProxy::Reload(const Uri &uri, const PacMap &extras)
{
    bool ret = false;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ret;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return ret;
    }

    if (!data.WriteParcelable(&extras)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable extras");
        return ret;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_RELOAD, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return ret;
    }

    ret = reply.ReadBool();
    if (!ret) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadBool ret");
        return ret;
    }

    return ret;
}

int AbilitySchedulerProxy::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    int ret = -1;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return ret;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return ret;
    }

    int count = (int)values.size();
    if (!data.WriteInt32(count)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteInt32 ret");
        return ret;
    }

    for (int i = 0; i < count; i++) {
        if (!values[i].Marshalling(data)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, index: %{public}d", i);
            return ret;
        }
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_BATCHINSERT, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return ret;
    }

    if (!reply.ReadInt32(ret)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadInt32 index");
        return ret;
    }

    return ret;
}

bool AbilitySchedulerProxy::ScheduleRegisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s WriteInterfaceToken(data) return false", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    if (!data.WriteRemoteObject(dataObserver->AsObject())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s failed to WriteParcelable dataObserver ", __func__);
        return false;
    }

    int32_t result = SendTransactCmd(IAbilityScheduler::SCHEDULE_REGISTEROBSERVER, data, reply, option);
    if (result == ERR_NONE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s SendRequest ok, retval: %{public}d", __func__, reply.ReadInt32());
        return true;
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
}

bool AbilitySchedulerProxy::ScheduleUnregisterObserver(const Uri &uri, const sptr<IDataAbilityObserver> &dataObserver)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s WriteInterfaceToken(data) return false", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    if (!data.WriteRemoteObject(dataObserver->AsObject())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s failed to WriteParcelable dataObserver ", __func__);
        return false;
    }

    int32_t result = SendTransactCmd(IAbilityScheduler::SCHEDULE_UNREGISTEROBSERVER, data, reply, option);
    if (result == ERR_NONE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s SendRequest ok, retval is %{public}d", __func__, reply.ReadInt32());
        return true;
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
}

bool AbilitySchedulerProxy::ScheduleNotifyChange(const Uri &uri)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s WriteInterfaceToken(data) return false", __func__);
        return false;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s failed to WriteParcelable uri ", __func__);
        return false;
    }

    int32_t result = SendTransactCmd(IAbilityScheduler::SCHEDULE_NOTIFYCHANGE, data, reply, option);
    if (result == ERR_NONE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "%{public}s SendRequest ok, retval: %{public}d", __func__, reply.ReadInt32());
        return true;
    } else {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s SendRequest error, result=%{public}d", __func__, result);
        return false;
    }
}

Uri AbilitySchedulerProxy::NormalizeUri(const Uri &uri)
{
    Uri urivalue("");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return urivalue;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return urivalue;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_NORMALIZEURI, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return Uri("");
    }

    std::unique_ptr<Uri> info(reply.ReadParcelable<Uri>());
    if (!info) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "readParcelable value null");
        return Uri("");
    }
    return *info;
}

Uri AbilitySchedulerProxy::DenormalizeUri(const Uri &uri)
{
    Uri urivalue("");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return urivalue;
    }

    if (!data.WriteParcelable(&uri)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to WriteParcelable uri");
        return urivalue;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_DENORMALIZEURI, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return Uri("");
    }

    std::unique_ptr<Uri> info(reply.ReadParcelable<Uri>());
    if (!info) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "readParcelable value null");
        return Uri("");
    }
    return *info;
}

std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> AbilitySchedulerProxy::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> results;
    results.clear();

    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "writer token failed");
        return results;
    }

    int count = (int)operations.size();
    if (!data.WriteInt32(count)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "writeInt32 ret failed");
        return results;
    }

    for (int i = 0; i < count; i++) {
        if (!data.WriteParcelable(operations[i].get())) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "failed, index: %{public}d", i);
            return results;
        }
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_EXECUTEBATCH, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sendRequest failed, err: %{public}d", err);
        return results;
    }

    int total = 0;
    if (!reply.ReadInt32(total)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail to ReadInt32 count %{public}d", total);
        return results;
    }

    for (int i = 0; i < total; i++) {
        std::shared_ptr<AppExecFwk::DataAbilityResult> dataAbilityResult(
            reply.ReadParcelable<AppExecFwk::DataAbilityResult>());
        if (dataAbilityResult == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "null dataAbilityResult, index: %{public}d", i);
            return results;
        }
        results.push_back(dataAbilityResult);
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "end %{public}d", total);
    return results;
}

void AbilitySchedulerProxy::ContinueAbility(const std::string& deviceId, uint32_t versionCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ContinueAbility fail to write token");
        return;
    }
    if (!data.WriteString(deviceId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ContinueAbility fail to write deviceId");
        return;
    }
    if (!data.WriteUint32(versionCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ContinueAbility fail to write versionCode");
        return;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::CONTINUE_ABILITY, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::NotifyContinuationResult(int32_t result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyContinuationResult fail to write token");
        return;
    }
    if (!data.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyContinuationResult fail to write result");
        return;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::NOTIFY_CONTINUATION_RESULT, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::DumpAbilityInfo(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "DumpAbilityRunner fail to write token");
        return;
    }

    if (!data.WriteStringVector(params)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "DumpAbilityRunner fail to write params");
        return;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::DUMP_ABILITY_RUNNER_INNER, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
}

void AbilitySchedulerProxy::CallRequest()
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "start");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!WriteInterfaceToken(data)) {
        return;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::REQUEST_CALL_REMOTE, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "end");
}

void AbilitySchedulerProxy::OnExecuteIntent(const Want &want)
{
    TAG_LOGI(AAFwkTag::INTENT, "on execute intent proxy");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&want);
    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_ONEXECUTE_INTENT, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "end");
}

void AbilitySchedulerProxy::ExecuteSkill(const Want &want)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "execute skill proxy");

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!WriteInterfaceToken(data)) {
        return;
    }
    data.WriteParcelable(&want);
    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_EXECUTE_SKILL, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "end");
}

int32_t AbilitySchedulerProxy::CreateModalUIExtension(const Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilitySchedulerProxy::CreateModalUIExtension start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    AAFwk::ExtendMaxIpcCapacityForInnerWant(data);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface fail");
        return INNER_ERR;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write wants fail");
        return INNER_ERR;
    }
    int32_t err = SendTransactCmd(IAbilityScheduler::CREATE_MODAL_UI_EXTENSION, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
        return err;
    }
    return reply.ReadInt32();
}

void AbilitySchedulerProxy::UpdateSessionToken(sptr<IRemoteObject> sessionToken)
{
    MessageParcel data;
    if (!WriteInterfaceToken(data)) {
        return;
    }
    if (!data.WriteRemoteObject(sessionToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write sessionToken failed");
        return;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t err = SendTransactCmd(IAbilityScheduler::UPDATE_SESSION_TOKEN, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sendRequest failed, err: %{public}d", err);
    }
}

int32_t AbilitySchedulerProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sendRequest failed, code: %{public}d, ret: %{public}d", code, ret);
        return ret;
    }
    return NO_ERROR;
}

void AbilitySchedulerProxy::ScheduleCollaborate(const Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return;
    }
    auto msgKey = AbilityRuntime::ErrorMgsUtil::BuildErrorKey(reinterpret_cast<uintptr_t>(this),
        "ScheduleCollaborate");
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write want failed");
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "write want failed");
        return;
    }
    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_COLLABORATE_DATA, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
    return;
}

void AbilitySchedulerProxy::ScheduleAbilityRequestFailure(const std::string &requestId,
    const AppExecFwk::ElementName &element, const std::string &message, int32_t resultCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return;
    }
    auto msgKey = AbilityRuntime::ErrorMgsUtil::BuildErrorKey(reinterpret_cast<uintptr_t>(this),
        "ScheduleAbilityRequestFailure");
    if (!data.WriteString(requestId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write requestId failed");
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "write requestId failed");
        return;
    }
    if (!data.WriteParcelable(&element)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write element failed");
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "write want failed");
        return;
    }
    if (!data.WriteString(message)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write message failed");
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "write message failed");
        return;
    }
    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write resultCode failed");
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "write resultCode failed");
        return;
    }
    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_ABILITY_REQUEST_FAILURE, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
    return;
}

void AbilitySchedulerProxy::ScheduleAbilityRequestSuccess(const std::string &requestId,
    const AppExecFwk::ElementName &element)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return;
    }
    auto msgKey = AbilityRuntime::ErrorMgsUtil::BuildErrorKey(reinterpret_cast<uintptr_t>(this),
        "ScheduleAbilityRequestSuccess");
    if (!data.WriteString(requestId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write requestId failed");
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "write requestId failed");
        return;
    }
    if (!data.WriteParcelable(&element)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write element failed");
        AbilityRuntime::ErrorMgsUtil::GetInstance().UpdateErrorMsg(msgKey, "write want failed");
        return;
    }
    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_ABILITY_REQUEST_SUCCESS, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
    return;
}

void AbilitySchedulerProxy::ScheduleAbilitiesRequestDone(const std::string &requestKey, int32_t resultCode)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!WriteInterfaceToken(data)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
        return;
    }

    if (!data.WriteString(requestKey)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write requestKey failed");
        return;
    }
    if (!data.WriteInt32(resultCode)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "write resultCode failed");
        return;
    }

    int32_t err = SendTransactCmd(IAbilityScheduler::SCHEDULE_ABILITIES_REQUEST_DONE, data, reply, option);
    if (err != NO_ERROR) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "fail, err: %{public}d", err);
    }
    return;
}
}  // namespace AAFwk
}  // namespace OHOS
