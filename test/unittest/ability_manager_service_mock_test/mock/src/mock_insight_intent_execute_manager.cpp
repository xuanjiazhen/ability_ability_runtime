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

#include "insight_intent_execute_manager.h"

namespace OHOS {
namespace AAFwk {
int32_t InsightIntentExecuteManager::CheckAndUpdateParam(uint64_t key, const sptr<IRemoteObject> &callerToken,
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param, std::string callerBundleName)
{
    return ERR_OK;
}

int32_t InsightIntentExecuteManager::CheckAndUpdateWant(
    Want &want, AppExecFwk::ExecuteMode executeMode, std::string callerBundleName)
{
    return ERR_OK;
}
 
int32_t InsightIntentExecuteManager::RemoveExecuteIntent(uint64_t intentId)
{
    return ERR_OK;
}
 
int32_t InsightIntentExecuteManager::ExecuteIntentDone(uint64_t intentId, int32_t resultCode,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    return ERR_OK;
}
 
int32_t InsightIntentExecuteManager::RemoteDied(uint64_t intentId)
{
    return ERR_OK;
}
 
int32_t InsightIntentExecuteManager::GetBundleName(uint64_t intentId, std::string &bundleName) const
{
    return ERR_OK;
}
 
int32_t InsightIntentExecuteManager::GetCallerBundleName(uint64_t intentId, std::string &callerBundleName) const
{
    return ERR_OK;
}
 
int32_t InsightIntentExecuteManager::GenerateWant(
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param, Want &want)
{
    return ERR_OK;
}
 
std::map<int32_t, int64_t> InsightIntentExecuteManager::GetAllIntentExemptionInfo() const
{
    std::map<int32_t, int64_t> map;
    return map;
}

int32_t InsightIntentExecuteManager::AddWantUirsAndFlagsFromParam(
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &param, Want &want)
{
    return ERR_OK;
}
 
void InsightIntentExecuteManager::SetIntentExemptionInfo(int32_t uid) {}
 
bool InsightIntentExecuteManager::CheckIntentIsExemption(int32_t uid)
{
    return true;
}
}
}