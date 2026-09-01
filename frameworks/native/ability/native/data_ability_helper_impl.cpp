/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "data_ability_helper_impl.h"

#include "ability_manager_client.h"
#include "ability_scheduler_interface.h"
#include "ability_thread.h"
#include "abs_shared_result_set.h"
#include "hitrace_meter.h"
#include "data_ability_observer_interface.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "data_ability_result.h"
#include "hilog_tag_wrapper.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
std::string SchemeOhos = "dataability";
using IAbilityScheduler = OHOS::AAFwk::IAbilityScheduler;
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
DataAbilityHelperImpl::DataAbilityHelperImpl(const std::shared_ptr<Context> &context, const std::shared_ptr<Uri> &uri,
    const sptr<IAbilityScheduler> &dataAbilityProxy, bool tryBind)
{
    token_ = context->GetToken();
    context_ = std::weak_ptr<Context>(context);
    uri_ = uri;
    tryBind_ = tryBind;
    dataAbilityProxy_ = dataAbilityProxy;
}

DataAbilityHelperImpl::DataAbilityHelperImpl(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context,
    const std::shared_ptr<Uri> &uri, const sptr<IAbilityScheduler> &dataAbilityProxy, bool tryBind)
{
    token_ = context->GetToken();
    uri_ = uri;
    tryBind_ = tryBind;
    dataAbilityProxy_ = dataAbilityProxy;
}

DataAbilityHelperImpl::DataAbilityHelperImpl(const std::shared_ptr<Context> &context)
{
    token_ = context->GetToken();
    context_ = std::weak_ptr<Context>(context);
}

DataAbilityHelperImpl::DataAbilityHelperImpl(const sptr<IRemoteObject> &token, const std::shared_ptr<Uri> &uri,
    const sptr<AAFwk::IAbilityScheduler> &dataAbilityProxy)
{
    token_ = token;
    uri_ = uri;
    tryBind_ = false;
    dataAbilityProxy_ = dataAbilityProxy;
    isSystemCaller_ = true;
}

DataAbilityHelperImpl::DataAbilityHelperImpl(const sptr<IRemoteObject> &token)
{
    token_ = token;
    isSystemCaller_ = true;
}

void DataAbilityHelperImpl::AddDataAbilityDeathRecipient(const sptr<IRemoteObject> &token)
{
    if (token != nullptr && callerDeathRecipient_ != nullptr) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Remove death recipient");
        token->RemoveDeathRecipient(callerDeathRecipient_);
    }
    if (callerDeathRecipient_ == nullptr) {
        std::weak_ptr<DataAbilityHelperImpl> thisWeakPtr(shared_from_this());
        callerDeathRecipient_ =
            new (std::nothrow) DataAbilityDeathRecipient([thisWeakPtr](const wptr<IRemoteObject> &remote) {
                auto DataAbilityHelperImpl = thisWeakPtr.lock();
                if (DataAbilityHelperImpl) {
                    DataAbilityHelperImpl->OnSchedulerDied(remote);
                }
            });
    }
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "Add death recipient");
    if (token == nullptr || !token->AddDeathRecipient(callerDeathRecipient_)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "AddDeathRecipient failed");
    }
}

void DataAbilityHelperImpl::OnSchedulerDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    std::lock_guard<std::mutex> guard(lock_);
    auto object = remote.promote();
    object = nullptr;
    dataAbilityProxy_ = nullptr;
    uri_ = nullptr;
}

std::shared_ptr<DataAbilityHelperImpl> DataAbilityHelperImpl::Creator(const std::shared_ptr<Context> &context)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return nullptr;
    }

    auto ptrDataAbilityHelperImpl = new (std::nothrow) DataAbilityHelperImpl(context);
    if (ptrDataAbilityHelperImpl == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "New failed");
        return nullptr;
    }

    return std::shared_ptr<DataAbilityHelperImpl>(ptrDataAbilityHelperImpl);
}


std::shared_ptr<DataAbilityHelperImpl> DataAbilityHelperImpl::Creator(
    const std::shared_ptr<Context> &context, const std::shared_ptr<Uri> &uri, const bool tryBind)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return nullptr;
    }

    if (!CheckUri(uri)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid uri");
        return nullptr;
    }

    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), tryBind, context->GetToken());
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "AcquireDataAbility error");
        return nullptr;
    }

    auto ptrDataAbilityHelperImpl = new (std::nothrow) DataAbilityHelperImpl(context, uri, dataAbilityProxy, tryBind);
    if (ptrDataAbilityHelperImpl == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "New error");
        return nullptr;
    }

    return std::shared_ptr<DataAbilityHelperImpl>(ptrDataAbilityHelperImpl);
}


std::shared_ptr<DataAbilityHelperImpl> DataAbilityHelperImpl::Creator(
    const std::shared_ptr<OHOS::AbilityRuntime::Context> &context, const std::shared_ptr<Uri> &uri, const bool tryBind)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return nullptr;
    }

    if (!CheckUri(uri)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid uri");
        return nullptr;
    }

    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), tryBind, context->GetToken());
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "AcquireDataAbility failed");
        return nullptr;
    }

    auto ptrDataAbilityHelperImpl = new (std::nothrow) DataAbilityHelperImpl(context, uri, dataAbilityProxy, tryBind);
    if (ptrDataAbilityHelperImpl == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "New failed");
        return nullptr;
    }

    return std::shared_ptr<DataAbilityHelperImpl>(ptrDataAbilityHelperImpl);
}


std::shared_ptr<DataAbilityHelperImpl> DataAbilityHelperImpl::Creator(const sptr<IRemoteObject> &token)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return nullptr;
    }

    auto ptrDataAbilityHelperImpl = new (std::nothrow) DataAbilityHelperImpl(token);
    if (ptrDataAbilityHelperImpl == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "New failed");
        return nullptr;
    }

    return std::shared_ptr<DataAbilityHelperImpl>(ptrDataAbilityHelperImpl);
}


std::shared_ptr<DataAbilityHelperImpl> DataAbilityHelperImpl::Creator(
    const sptr<IRemoteObject> &token, const std::shared_ptr<Uri> &uri)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "param invalid");
        return nullptr;
    }

    if (!CheckUri(uri)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid uri");
        return nullptr;
    }

    sptr<IAbilityScheduler> dataAbilityProxy =
        AbilityManagerClient::GetInstance()->AcquireDataAbility(*uri.get(), false, token);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "AcquireDataAbility failed");
        return nullptr;
    }

    auto ptrDataAbilityHelperImpl = new (std::nothrow) DataAbilityHelperImpl(token, uri, dataAbilityProxy);
    if (ptrDataAbilityHelperImpl == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "New failed");
        return nullptr;
    }

    return std::shared_ptr<DataAbilityHelperImpl>(ptrDataAbilityHelperImpl);
}


bool DataAbilityHelperImpl::Release()
{
    if (uri_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Release failed");
        return false;
    }

    int err = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy_, token_);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "ReleaseDataAbility err:%{public}d", err);
        return false;
    }

    return true;
}


std::vector<std::string> DataAbilityHelperImpl::GetFileTypes(Uri &uri, const std::string &mimeTypeFilter)
{
    std::vector<std::string> matchedMIMEs;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return matchedMIMEs;
    }

    matchedMIMEs = dataAbilityProxy->GetFileTypes(uri, mimeTypeFilter);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "matchedMIMEs size: %{public}zu", matchedMIMEs.size());
    return matchedMIMEs;
}


int DataAbilityHelperImpl::OpenFile(Uri &uri, const std::string &mode)
{
    int fd = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return fd;
    }

    fd = dataAbilityProxy->OpenFile(uri, mode);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "fd: %{public}d", fd);
    return fd;
}


int DataAbilityHelperImpl::OpenRawFile(Uri &uri, const std::string &mode)
{
    int fd = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return fd;
    }

    fd = dataAbilityProxy->OpenRawFile(uri, mode);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "fd: %{public}d", fd);
    return fd;
}


int DataAbilityHelperImpl::Insert(Uri &uri, const NativeRdb::ValuesBucket &value)
{
    int index = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return index;
    }

    index = dataAbilityProxy->Insert(uri, value);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "index: %{public}d", index);
    return index;
}

std::shared_ptr<AppExecFwk::PacMap> DataAbilityHelperImpl::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    std::shared_ptr<AppExecFwk::PacMap> result = nullptr;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return result;
    }

    result = dataAbilityProxy->Call(uri, method, arg, pacMap);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "null result?: %{public}d", result == nullptr);
    return result;
}


int DataAbilityHelperImpl::Update(
    Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return index;
    }

    index = dataAbilityProxy->Update(uri, value, predicates);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "index: %{public}d", index);
    return index;
}


int DataAbilityHelperImpl::Delete(Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return index;
    }

    index = dataAbilityProxy->Delete(uri, predicates);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "index: %{public}d", index);
    return index;
}


std::shared_ptr<NativeRdb::AbsSharedResultSet> DataAbilityHelperImpl::Query(
    Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultset = nullptr;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return resultset;
    }

    resultset = dataAbilityProxy->Query(uri, columns, predicates);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "null resultset?: %{public}d.", resultset == nullptr);
    return resultset;
}


std::string DataAbilityHelperImpl::GetType(Uri &uri)
{
    std::string type;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return type;
    }

    type = dataAbilityProxy->GetType(uri);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "type: %{public}s", type.c_str());
    return type;
}


bool DataAbilityHelperImpl::Reload(Uri &uri, const PacMap &extras)
{
    bool ret = false;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return ret;
    }

    ret = dataAbilityProxy->Reload(uri, extras);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "ret: %{public}d", ret);
    return ret;
}


int DataAbilityHelperImpl::BatchInsert(Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    int ret = -1;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return ret;
    }

    ret = dataAbilityProxy->BatchInsert(uri, values);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "ret: %{public}d", ret);
    return ret;
}

bool DataAbilityHelperImpl::CheckUriParam(const Uri &uri)
{
    Uri checkUri(uri.ToString());
    if (!CheckOhosUri(checkUri)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Check failed, uri: %{public}s", uri.ToString().c_str());
        return false;
    }

    // do not directly use uri_ here, otherwise, it will probably crash.
    std::vector<std::string> segments;
    {
        std::lock_guard<std::mutex> guard(lock_);
        if (!uri_) {
            TAG_LOGI(AAFwkTag::DATA_ABILITY, "no need check");
            return true;
        }
        Uri checkUri(uri_->ToString());
        if (!CheckOhosUri(checkUri)) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "Check failed, uri_: %{public}s", uri_->ToString().c_str());
            return false;
        }

        uri_->GetPathSegments(segments);
    }

    std::vector<std::string> checkSegments;
    checkUri.GetPathSegments(checkSegments);
    if (checkSegments.empty() || segments.empty() || checkSegments[0] != segments[0]) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "uri different");
        return false;
    }

    return true;
}

bool DataAbilityHelperImpl::CheckOhosUri(const Uri &checkUri)
{
    Uri uri(checkUri);
    if (uri.GetScheme() != SchemeOhos) {
        TAG_LOGE(
            AAFwkTag::DATA_ABILITY, "not dataability uri: %{public}s", uri.ToString().c_str());
        return false;
    }

    std::vector<std::string> segments;
    uri.GetPathSegments(segments);
    if (segments.empty()) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "segments empty");
        return false;
    }

    if (uri.GetPath() == "") {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "path empty");
        return false;
    }

    return true;
}


void DataAbilityHelperImpl::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    if (!CheckUriAndDataObserver(uri, dataObserver)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return;
    }

    Uri tmpUri(uri.ToString());

    std::lock_guard<std::mutex> lock_l(oplock_);
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = nullptr;
    if (uri_ == nullptr) {
        auto dataability = registerMap_.find(dataObserver);
        if (dataability == registerMap_.end()) {
            dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
            registerMap_.emplace(dataObserver, dataAbilityProxy);
            uriMap_.emplace(dataObserver, tmpUri.GetPath());
        } else {
            auto path = uriMap_.find(dataObserver);
            if (path == uriMap_.end()) {
                return;
            }
            if (path->second != tmpUri.GetPath()) {
                TAG_LOGE(AAFwkTag::DATA_ABILITY, "Input uri's path diff from observer's");
                return;
            }
            dataAbilityProxy = dataability->second;
        }
    } else {
        dataAbilityProxy = dataAbilityProxy_;
    }

    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null dataAbilityProxy");
        registerMap_.erase(dataObserver);
        uriMap_.erase(dataObserver);
        return;
    }
    dataAbilityProxy->ScheduleRegisterObserver(uri, dataObserver);
}


void DataAbilityHelperImpl::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    if (!CheckUriAndDataObserver(uri, dataObserver)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return;
    }

    Uri tmpUri(uri.ToString());
    std::lock_guard<std::mutex> lock_l(oplock_);
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = nullptr;
    if (uri_ == nullptr) {
        auto dataability = registerMap_.find(dataObserver);
        if (dataability == registerMap_.end()) {
            return;
        }
        auto path = uriMap_.find(dataObserver);
        if (path == uriMap_.end()) {
            return;
        }
        if (path->second != tmpUri.GetPath()) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "Input uri's path diff from observer's");
            return;
        }
        dataAbilityProxy = dataability->second;
    } else {
        dataAbilityProxy = dataAbilityProxy_;
    }

    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null dataAbilityProxy");
        return;
    }

    dataAbilityProxy->ScheduleUnregisterObserver(uri, dataObserver);
    ReleaseDataAbility(dataAbilityProxy_);
    if (uri_ == nullptr) {
        dataAbilityProxy_ = nullptr;
    }
    registerMap_.erase(dataObserver);
    uriMap_.erase(dataObserver);
}


void DataAbilityHelperImpl::NotifyChange(const Uri &uri)
{
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return;
    }

    dataAbilityProxy->ScheduleNotifyChange(uri);
    ReleaseDataAbility(dataAbilityProxy);
}


Uri DataAbilityHelperImpl::NormalizeUri(Uri &uri)
{
    Uri urivalue("");
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null dataAbilityProxy");
        return urivalue;
    }

    urivalue = dataAbilityProxy->NormalizeUri(uri);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "uri: %{public}s", urivalue.ToString().c_str());
    return urivalue;
}


Uri DataAbilityHelperImpl::DenormalizeUri(Uri &uri)
{
    Uri urivalue("");
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return urivalue;
    }

    urivalue = dataAbilityProxy->DenormalizeUri(uri);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "uri: %{public}s", urivalue.ToString().c_str());
    return urivalue;
}

std::vector<std::shared_ptr<DataAbilityResult>> DataAbilityHelperImpl::ExecuteBatch(
    const Uri &uri, const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = GetDataAbilityProxy(uri, false);
    if (dataAbilityProxy == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "GetDataAbilityProxy failed");
        return results;
    }

    results = dataAbilityProxy->ExecuteBatch(operations);

    ReleaseDataAbility(dataAbilityProxy);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "results size: %{public}zu", results.size());
    return results;
}

sptr<AAFwk::IAbilityScheduler> DataAbilityHelperImpl::GetDataAbilityProxy(const Uri &uri, bool addDeathRecipient)
{
    if (!CheckUriParam(uri)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Check uri failed");
        return nullptr;
    }
    // if uri_ is nullptr, it indicates the operation(such as insert, delete and so on) is temporary,
    // so, we need acquire the dataability before the operation.
    sptr<AAFwk::IAbilityScheduler> dataAbilityProxy = dataAbilityProxy_;
    if (uri_ == nullptr) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "null uri_");
        dataAbilityProxy = AbilityManagerClient::GetInstance()->AcquireDataAbility(uri, tryBind_, token_);
        if (dataAbilityProxy == nullptr) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "AcquireDataAbility failed");
            return nullptr;
        }
        if (addDeathRecipient && isSystemCaller_) {
            AddDataAbilityDeathRecipient(dataAbilityProxy->AsObject());
        }
    }
    return dataAbilityProxy;
}

void DataAbilityHelperImpl::ReleaseDataAbility(sptr<AAFwk::IAbilityScheduler> dataAbilityProxy)
{
    // if uri_ is nullptr, it indicates the operation(such as insert, delete and so on) is temporary,
    // so, we need release the dataability after the operation.
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "start");
    if (!uri_ && dataAbilityProxy && token_) {
        int ret = AbilityManagerClient::GetInstance()->ReleaseDataAbility(dataAbilityProxy, token_);
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "ReleaseDataAbility ret: %{public}d", ret);
    }
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "end");
}

bool DataAbilityHelperImpl::CheckUri(const std::shared_ptr<Uri> &uri)
{
    if (uri == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return false;
    }

    if (uri->GetScheme() != SchemeOhos) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "uri not data ability, Scheme: %{private}s",
            uri->GetScheme().c_str());
        return false;
    }

    return true;
}

bool DataAbilityHelperImpl::CheckUriAndDataObserver(const Uri &uri,
    const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    if (!CheckUriParam(uri)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Check uri failed");
        return false;
    }

    if (dataObserver == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return false;
    }

    return true;
}

void DataAbilityDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    if (handler_) {
        handler_(remote);
    }
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "end");
}

DataAbilityDeathRecipient::DataAbilityDeathRecipient(RemoteDiedHandler handler) : handler_(handler)
{}

DataAbilityDeathRecipient::~DataAbilityDeathRecipient()
{}
}  // namespace AppExecFwk
}  // namespace OHOS

