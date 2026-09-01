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

#include "data_ability_impl.h"

#include "abs_shared_result_set.h"
#include "accesstoken_kit.h"
#include "data_ability_operation.h"
#include "data_ability_predicates.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "values_bucket.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string READ = "r";
const std::string WRITE = "w";
}
using AbilityManagerClient = OHOS::AAFwk::AbilityManagerClient;
using OHOS::Security::AccessToken::AccessTokenKit;
void DataAbilityImpl::HandleAbilityTransaction(const Want &want, const AAFwk::LifeCycleStateInfo &targetState,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY,
        "sourceState:%{public}d; targetState:%{public}d; isNewWant:%{public}d", lifecycleState_,
        targetState.state, targetState.isNewWant);
    if ((lifecycleState_ == targetState.state) && !targetState.isNewWant) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "lifeCycleState equals");
        return;
    }

    switch (targetState.state) {
        case AAFwk::ABILITY_STATE_ACTIVE: {
            if (lifecycleState_ == AAFwk::ABILITY_STATE_INITIAL) {
                SetUriString(targetState.caller.deviceId + "/" + targetState.caller.bundleName + "/" +
                             targetState.caller.abilityName);
                Start(want);
            } else {
                return;
            }
            break;
        }
        default: {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "state error");
            return;
        }
    }

    AbilityManagerClient::GetInstance()->AbilityTransitionDone(token_, targetState.state, GetRestoreData());
}


std::vector<std::string> DataAbilityImpl::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    std::vector<std::string> types;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return types;
    }

    types = ability_->GetFileTypes(uri, mimeTypeFilter);
    return types;
}


int DataAbilityImpl::OpenFile(const Uri &uri, const std::string &mode)
{
    int fd = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return fd;
    }

    if (!CheckOpenFilePermission(mode)) {
        return fd;
    }

    fd = ability_->OpenFile(uri, mode);
    return fd;
}


int DataAbilityImpl::OpenRawFile(const Uri &uri, const std::string &mode)
{
    int fd = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return fd;
    }

    if (!CheckOpenFilePermission(mode)) {
        return fd;
    }

    fd = ability_->OpenRawFile(uri, mode);
    return fd;
}


int DataAbilityImpl::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    int index = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return index;
    }

    if (!CheckReadAndWritePermission(WRITE)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
        return index;
    }

    index = ability_->Insert(uri, value);
    return index;
}


int DataAbilityImpl::Update(
    const Uri &uri, const NativeRdb::ValuesBucket &value, const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return index;
    }

    if (!CheckReadAndWritePermission(WRITE)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
        return index;
    }

    index = ability_->Update(uri, value, predicates);
    return index;
}


int DataAbilityImpl::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    int index = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return index;
    }

    if (!CheckReadAndWritePermission(WRITE)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
        return index;
    }

    index = ability_->Delete(uri, predicates);
    return index;
}


std::shared_ptr<NativeRdb::AbsSharedResultSet> DataAbilityImpl::Query(
    const Uri &uri, std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return nullptr;
    }

    if (!CheckReadAndWritePermission(READ)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no read permission");
        return nullptr;
    }

    return ability_->Query(uri, columns, predicates);
}

std::shared_ptr<AppExecFwk::PacMap> DataAbilityImpl::Call(
    const Uri &uri, const std::string &method, const std::string &arg, const AppExecFwk::PacMap &pacMap)
{
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return nullptr;
    }

    return ability_->Call(uri, method, arg, pacMap);
}


std::string DataAbilityImpl::GetType(const Uri &uri)
{
    std::string type;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return type;
    }
    type = ability_->GetType(uri);
    return type;
}


bool DataAbilityImpl::Reload(const Uri &uri, const PacMap &extras)
{
    bool ret = false;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return ret;
    }
    ret = ability_->Reload(uri, extras);
    return ret;
}


int DataAbilityImpl::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    int ret = -1;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return ret;
    }

    if (!CheckReadAndWritePermission(WRITE)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
        return ret;
    }

    ret = ability_->BatchInsert(uri, values);
    return ret;
}

Uri DataAbilityImpl::NormalizeUri(const Uri &uri)
{
    Uri urivalue("");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return urivalue;
    }

    if (!CheckReadAndWritePermission(READ)) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "no read permission");
        return urivalue;
    }

    urivalue = ability_->NormalizeUri(uri);
    return urivalue;
}


Uri DataAbilityImpl::DenormalizeUri(const Uri &uri)
{
    Uri urivalue("");
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        return urivalue;
    }

    if (!CheckReadAndWritePermission(READ)) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "no read permission");
        return urivalue;
    }

    urivalue = ability_->DenormalizeUri(uri);
    return urivalue;
}

std::vector<std::shared_ptr<DataAbilityResult>> DataAbilityImpl::ExecuteBatch(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations)
{
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "start");
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    if (ability_ == nullptr) {
        TAG_LOGE(AAFwkTag::DATA_ABILITY, "null ability_");
        results.clear();
        return results;
    }

    if (!CheckExecuteBatchPermission(operations)) {
        results.clear();
        return results;
    }

    results = ability_->ExecuteBatch(operations);
    TAG_LOGI(AAFwkTag::DATA_ABILITY, "end, results size:%{public}zu", results.size());
    return results;
}

bool DataAbilityImpl::CheckExecuteBatchPermission(
    const std::vector<std::shared_ptr<DataAbilityOperation>> &operations) const
{
    bool needCheckReadPermission = false;
    bool needCheckWritePermission = false;

    size_t size = operations.size();
    for (size_t i = 0; i < size; i++) {
        std::shared_ptr<DataAbilityOperation> operation = operations[i];
        if (operation == nullptr) {
            TAG_LOGE(AAFwkTag::DATA_ABILITY, "null operation, index: %{public}d", static_cast<int32_t>(i));
            return false;
        }
        if (operation->IsInsertOperation() || operation->IsUpdateOperation() || operation->IsDeleteOperation()) {
            needCheckWritePermission = true;
        } else if (operation->IsAssertOperation()) {
            needCheckReadPermission = true;
        }
        if (needCheckReadPermission && needCheckWritePermission) {
            break;
        }
    }

    if (needCheckReadPermission) {
        if (!CheckReadAndWritePermission(READ)) {
            TAG_LOGW(AAFwkTag::DATA_ABILITY, "no read permission");
            return false;
        }
    }

    if (needCheckWritePermission) {
        if (!CheckReadAndWritePermission(WRITE)) {
            TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
            return false;
        }
    }

    return true;
}

bool DataAbilityImpl::CheckOpenFilePermission(const std::string &mode) const
{
    if (mode.find(READ) != string::npos) {
        if (!CheckReadAndWritePermission(READ)) {
            TAG_LOGW(AAFwkTag::DATA_ABILITY, "no read permission");
            return false;
        }
    }
    if (mode.find(WRITE) != string::npos) {
        if (!CheckReadAndWritePermission(WRITE)) {
            TAG_LOGW(AAFwkTag::DATA_ABILITY, "no write permission");
            return false;
        }
    }

    return true;
}

bool DataAbilityImpl::CheckReadAndWritePermission(const std::string &permissionType) const
{
    std::string permission = GetPermissionInfo(permissionType);
    if (permission.empty()) {
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "empty permission");
        return true;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    int checkReadPermission = AccessTokenKit::VerifyAccessToken(tokenId, permission);
    if (checkReadPermission != ERR_OK) {
        std::shared_ptr<AbilityInfo> abilityInfo = ability_->GetAbilityInfo();
        TAG_LOGD(AAFwkTag::DATA_ABILITY, "no permission, bundleName: %{public}s", abilityInfo->bundleName.c_str());
        return false;
    }

    return true;
}

std::string DataAbilityImpl::GetPermissionInfo(const std::string &permissionType) const
{
    TAG_LOGD(AAFwkTag::DATA_ABILITY, "permissionType:%{public}s", permissionType.c_str());
    std::shared_ptr<AbilityInfo> abilityInfo = ability_->GetAbilityInfo();
    if (abilityInfo == nullptr) {
        TAG_LOGW(AAFwkTag::DATA_ABILITY, "null abilityInfo");
        return "";
    }
    if (permissionType == READ) {
        return abilityInfo->readPermission;
    } else if (permissionType == WRITE) {
        return abilityInfo->writePermission;
    } else {
        TAG_LOGI(AAFwkTag::DATA_ABILITY, "invalid permissionType:%{public}s", permissionType.c_str());
        return "";
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
