/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "data_ability_helper.h"
#include "abs_shared_result_set.h"
#include "datashare_helper.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "rdb_data_ability_utils.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::RdbDataAbilityAdapter;
DataAbilityHelper::DataAbilityHelper(const std::shared_ptr<DataAbilityHelperImpl> &helperImpl)
{
    dataAbilityHelperImpl_ = helperImpl;
}

DataAbilityHelper::DataAbilityHelper(const std::shared_ptr<DataShare::DataShareHelper> &dataShareHelper)
{
    dataShareHelper_ = dataShareHelper;
}


std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(const std::shared_ptr<Context> context)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "Creator with context");
    DataAbilityHelper *ptrDataAbilityHelper = nullptr;
    std::shared_ptr<DataAbilityHelperImpl> dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context);
    if (dataAbilityHelperImpl) {
        ptrDataAbilityHelper = new DataAbilityHelper(dataAbilityHelperImpl);
    }
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}


std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(
    const std::shared_ptr<Context> context, const std::shared_ptr<Uri> &uri)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "Creator with context & uri");
    if (!context || !uri) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return nullptr;
    }
    auto sharedPtrDataAbilityHelper = DataAbilityHelper::Creator(context, uri, false);
    if (sharedPtrDataAbilityHelper) {
        return sharedPtrDataAbilityHelper;
    }

    TAG_LOGI(AAFwkTag::DATA_ABILITY, "Creator failed");
    Uri dataShareUri("");
    if (!DataAbilityHelper::TransferScheme(*uri, dataShareUri)) {
        return nullptr;
    }
    DataAbilityHelper *ptrDataAbilityHelper = nullptr;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(context->GetToken(), dataShareUri.ToString());
    if (dataShareHelper) {
        ptrDataAbilityHelper = new DataAbilityHelper(dataShareHelper);
    }
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "end");
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}


std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(
    const std::shared_ptr<OHOS::AbilityRuntime::Context> context, const std::shared_ptr<Uri> &uri)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "Creator with ability runtime context & uri");
    if (!context || !uri) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return nullptr;
    }
    auto sharedPtrDataAbilityHelper = DataAbilityHelper::Creator(context, uri, false);
    if (sharedPtrDataAbilityHelper) {
        return sharedPtrDataAbilityHelper;
    }

    TAG_LOGI(AAFwkTag::DATA_ABILITY, "Creator failed");
    Uri dataShareUri("");
    if (!DataAbilityHelper::TransferScheme(*uri, dataShareUri)) {
        return nullptr;
    }
    DataAbilityHelper *ptrDataAbilityHelper = nullptr;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(context->GetToken(), dataShareUri.ToString());
    if (dataShareHelper) {
        ptrDataAbilityHelper = new DataAbilityHelper(dataShareHelper);
    }
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}


std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(
    const std::shared_ptr<Context> context, const std::shared_ptr<Uri> &uri, const bool tryBind)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "Creator with context & uri & tryBind");
    DataAbilityHelper *ptrDataAbilityHelper = nullptr;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context, uri, tryBind);
    if (dataAbilityHelperImpl) {
        ptrDataAbilityHelper = new DataAbilityHelper(dataAbilityHelperImpl);
    }
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}


std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(
    const std::shared_ptr<OHOS::AbilityRuntime::Context> context, const std::shared_ptr<Uri> &uri, const bool tryBind)
{
    TAG_LOGI(
        AAFwkTag::DATA_ABILITY, "Creator with ability runtime context & uri & tryBind");
    DataAbilityHelper *ptrDataAbilityHelper = nullptr;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(context, uri, tryBind);
    if (dataAbilityHelperImpl) {
        ptrDataAbilityHelper = new DataAbilityHelper(dataAbilityHelperImpl);
    }
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}


std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(const sptr<IRemoteObject> token)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "Creator with token");
    DataAbilityHelper *ptrDataAbilityHelper = nullptr;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token);
    if (dataAbilityHelperImpl) {
        ptrDataAbilityHelper = new DataAbilityHelper(dataAbilityHelperImpl);
    }
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}


std::shared_ptr<DataAbilityHelper> DataAbilityHelper::Creator(
    const sptr<IRemoteObject> token, const std::shared_ptr<Uri> &uri)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "Creator with token & uri");
    if (!token || !uri) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param");
        return nullptr;
    }
    DataAbilityHelper *ptrDataAbilityHelper = nullptr;
    auto dataAbilityHelperImpl = DataAbilityHelperImpl::Creator(token, uri);
    if (dataAbilityHelperImpl) {
        ptrDataAbilityHelper = new DataAbilityHelper(dataAbilityHelperImpl);
    } else {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Creator failed");
        Uri dataShareUri("");
        if (!DataAbilityHelper::TransferScheme(*uri, dataShareUri)) {
            return nullptr;
        }
        auto dataShareHelper = DataShare::DataShareHelper::Creator(token, dataShareUri.ToString());
        if (dataShareHelper) {
            ptrDataAbilityHelper = new DataAbilityHelper(dataShareHelper);
        }
    }
    return std::shared_ptr<DataAbilityHelper>(ptrDataAbilityHelper);
}


bool DataAbilityHelper::Release()
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    bool ret = false;
    if (dataAbilityHelperImpl_) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "DataAbilityHelperImpl Release");
        ret = dataAbilityHelperImpl_->Release();
    }
    if (dataShareHelper_) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "DataShareHelper Release");
        ret = dataShareHelper_->Release();
        dataShareHelper_.reset();
    }
    return ret;
}


std::vector<std::string> DataAbilityHelper::GetFileTypes(Uri &uri, const std::string &mimeTypeFilter)
{
    HITRACE_METER_NAME(HITRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    std::vector<std::string> matchedMIMEs;
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability GetFileTypes");
        matchedMIMEs = dataAbilityHelperImpl->GetFileTypes(uri, mimeTypeFilter);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share GetFileTypes");
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            matchedMIMEs = dataShareHelper->GetFileTypes(dataShareUri, mimeTypeFilter);
        }
    }
    return matchedMIMEs;
}


int DataAbilityHelper::OpenFile(Uri &uri, const std::string &mode)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    int fd = -1;
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability OpenFile");
        fd = dataAbilityHelperImpl->OpenFile(uri, mode);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        if (callFromJs_) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "Share no this interface");
        } else {
            TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share OpenFile");
            Uri dataShareUri("");
            if (TransferScheme(uri, dataShareUri)) {
                fd = dataShareHelper->OpenFile(dataShareUri, mode);
            }
        }
    }
    return fd;
}


int DataAbilityHelper::OpenRawFile(Uri &uri, const std::string &mode)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    int fd = -1;
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability OpenRawFile");
        fd = dataAbilityHelperImpl->OpenRawFile(uri, mode);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share OpenFile");
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            fd = dataShareHelper->OpenRawFile(dataShareUri, mode);
        }
    }
    return fd;
}


int DataAbilityHelper::Insert(Uri &uri, const NativeRdb::ValuesBucket &value)
{
    HITRACE_METER_NAME(HITRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    int index = -1;
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability Insert");
        index = dataAbilityHelperImpl->Insert(uri, value);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share Insert");
        DataShare::DataShareValuesBucket dataShareValue = RdbDataAbilityUtils::ToDataShareValuesBucket(value);
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            index = dataShareHelper->Insert(dataShareUri, dataShareValue);
        }
    }
    return index;
}

std::shared_ptr<AppExecFwk::PacMap> DataAbilityHelper::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    std::shared_ptr<AppExecFwk::PacMap> result = nullptr;
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability Call");
        result = dataAbilityHelperImpl->Call(uri, method, arg, pacMap);
    }
    if (dataShareHelper_) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Share no Call");
    }
    return result;
}


int DataAbilityHelper::Update(
    Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    HITRACE_METER_NAME(HITRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    int index = -1;
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability Update");
        index = dataAbilityHelperImpl->Update(uri, value, predicates);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share Update");
        DataShare::DataShareValuesBucket dataShareValue = RdbDataAbilityUtils::ToDataShareValuesBucket(value);
        DataShare::DataSharePredicates dataSharePredicates = RdbDataAbilityUtils::ToDataSharePredicates(predicates);
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            index = dataShareHelper->Update(dataShareUri, dataSharePredicates, dataShareValue);
        }
    }
    return index;
}


int DataAbilityHelper::Delete(Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    HITRACE_METER_NAME(HITRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    int index = -1;
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability Delete");
        return dataAbilityHelperImpl->Delete(uri, predicates);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share Delete");
        DataShare::DataSharePredicates dataSharePredicates = RdbDataAbilityUtils::ToDataSharePredicates(predicates);
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            index = dataShareHelper->Delete(dataShareUri, dataSharePredicates);
        }
    }
    return index;
}


std::shared_ptr<NativeRdb::AbsSharedResultSet> DataAbilityHelper::Query(
    Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    HITRACE_METER_NAME(HITRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "called");
    std::shared_ptr<NativeRdb::AbsSharedResultSet> resultSet = nullptr;
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "Ability Query");
        return dataAbilityHelperImpl->Query(uri, columns, predicates);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "Share Query");
        DataShare::DataSharePredicates dataSharePredicates = RdbDataAbilityUtils::ToDataSharePredicates(predicates);
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            std::shared_ptr<DataShare::DataShareResultSet> dataShareResultSet
                = dataShareHelper->Query(dataShareUri, dataSharePredicates, columns);
            if (!dataShareResultSet) {
                TAG_LOGE(AAFwkTag::DATA_ABILITY, "null dataShareResultSet");
                return nullptr;
            }
            resultSet = RdbDataAbilityUtils::ToAbsSharedResultSet(dataShareResultSet);
            if (!resultSet) {
                TAG_LOGE(AAFwkTag::DATA_ABILITY, "Transfer to AbsSharedResultSet failed");
            }
        }
    }
    return resultSet;
}


std::string DataAbilityHelper::GetType(Uri &uri)
{
    HITRACE_METER_NAME(HITRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    std::string type;
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability GetType");
        type = dataAbilityHelperImpl->GetType(uri);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share GetType");
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            type = dataShareHelper->GetType(dataShareUri);
        }
    }
    return type;
}


bool DataAbilityHelper::Reload(Uri &uri, const PacMap &extras)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    bool ret = false;
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability Reload");
        ret = dataAbilityHelperImpl->Reload(uri, extras);
    }
    if (dataShareHelper_) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Share no Reload");
    }
    return ret;
}


int DataAbilityHelper::BatchInsert(Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    HITRACE_METER_NAME(HITRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    int ret = -1;
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability BatchInsert");
        ret = dataAbilityHelperImpl->BatchInsert(uri, values);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share BatchInsert");
        std::vector<DataShare::DataShareValuesBucket> dataShareValues;
        for (auto value : values) {
            DataShare::DataShareValuesBucket dataShareValue = RdbDataAbilityUtils::ToDataShareValuesBucket(value);
            dataShareValues.push_back(dataShareValue);
        }
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            ret = dataShareHelper->BatchInsert(dataShareUri, dataShareValues);
        }
    }
    return ret;
}


void DataAbilityHelper::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability RegisterObserver");
        dataAbilityHelperImpl->RegisterObserver(uri, dataObserver);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share RegisterObserver");
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            dataShareHelper->RegisterObserver(dataShareUri, dataObserver);
        }
    }
}


void DataAbilityHelper::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability UnregisterObserver");
        dataAbilityHelperImpl->UnregisterObserver(uri, dataObserver);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share UnregisterObserver");
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            dataShareHelper->UnregisterObserver(dataShareUri, dataObserver);
        }
    }
}


void DataAbilityHelper::NotifyChange(const Uri &uri)
{
    HITRACE_METER_NAME(HITRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability NotifyChange");
        dataAbilityHelperImpl->NotifyChange(uri);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share NotifyChange");
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            dataShareHelper->NotifyChange(dataShareUri);
        }
    }
}


Uri DataAbilityHelper::NormalizeUri(Uri &uri)
{
    HITRACE_METER_NAME(HITRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    Uri urivalue("");
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability NormalizeUri");
        urivalue = dataAbilityHelperImpl->NormalizeUri(uri);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share NormalizeUri");
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            urivalue = dataShareHelper->NormalizeUri(dataShareUri);
        }
    }
    return urivalue;
}


Uri DataAbilityHelper::DenormalizeUri(Uri &uri)
{
    HITRACE_METER_NAME(HITRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    Uri urivalue("");
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability DenormalizeUri");
        urivalue = dataAbilityHelperImpl->DenormalizeUri(uri);
    }
    auto dataShareHelper = GetDataShareHelper();
    if (dataShareHelper) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Share DenormalizeUri");
        Uri dataShareUri("");
        if (TransferScheme(uri, dataShareUri)) {
            urivalue = dataShareHelper->DenormalizeUri(dataShareUri);
        }
    }
    return urivalue;
}

std::vector<std::shared_ptr<DataAbilityResult>> DataAbilityHelper::ExecuteBatch(
    const Uri &uri, const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "called");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    auto dataAbilityHelperImpl = GetDataAbilityHelperImpl();
    if (dataAbilityHelperImpl) {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "Ability ExecuteBatch");
        results = dataAbilityHelperImpl->ExecuteBatch(uri, operations);
    }
    if (dataShareHelper_) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "Share no ExecuteBatch");
    }
    return results;
}

bool DataAbilityHelper::TransferScheme(const Uri &uri, Uri &dataShareUri)
{
    const std::string dataAbilityScheme = "dataability";
    const std::string dataShareScheme = "datashare";
    const std::string fileScheme = "file";
    const std::string dataSharePrefix = "datashare:///";
    const std::string filePrefix = "file://";

    Uri inputUri = uri;
    if (inputUri.GetScheme() == dataShareScheme) {
        dataShareUri = Uri(inputUri.ToString());
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "data share uri: %{public}s no need transfer",
            inputUri.ToString().c_str());
        return true;
    }

    if (inputUri.GetScheme() == dataAbilityScheme) {
        string uriStr = inputUri.ToString();
        uriStr.replace(0, dataAbilityScheme.length(), dataShareScheme);
        dataShareUri = Uri(uriStr);
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "ability uri: %{public}s transfer to share uri: %{public}s",
            inputUri.ToString().c_str(), dataShareUri.ToString().c_str());
        return true;
    }

    if (inputUri.GetScheme() == fileScheme) {
        string uriStr = inputUri.ToString();
        uriStr.replace(0, filePrefix.length(), dataSharePrefix);
        dataShareUri = Uri(uriStr);
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "file uri: %{public}s transfer to share uri: %{public}s",
            inputUri.ToString().c_str(), dataShareUri.ToString().c_str());
        return true;
    }

    TAG_LOGE(AAFwkTag::DATA_ABILITY, "invalid param, uri: %{private}s", inputUri.ToString().c_str());
    return false;
}

void DataAbilityHelper::SetCallFromJs()
{
    callFromJs_ = true;
}
}  // namespace AppExecFwk
}  // namespace OHOS

