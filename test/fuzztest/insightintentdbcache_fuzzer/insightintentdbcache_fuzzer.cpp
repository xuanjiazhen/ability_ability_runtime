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

#include "insightintentdbcache_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "ability_fuzz_util.h"
#include "insight_intent_db_cache.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
constexpr size_t U32_AT_SIZE = 4;

void SaveAndDeleteInsightIntent(int32_t userId, const std::string &bundleName, const std::string &moduleName,
    uint32_t versionCode, const ExtractInsightIntentProfileInfoVec &profileInfos,
    const std::vector<InsightIntentInfo> &configIntentInfos)
{
    auto cache = DelayedSingleton<InsightIntentDbCache>::GetInstance();
    cache->InitInsightIntentCache(userId);
    cache->SaveInsightIntentTotalInfo(
        InsightIntentSaveParam{bundleName, moduleName, versionCode, userId}, profileInfos, configIntentInfos);
    cache->DeleteInsightIntentTotalInfo(IntentKey{bundleName, moduleName, "", userId});
    cache->DeleteInsightIntentByUserId(userId);
}

void QueryInsightIntent(int32_t userId, int32_t userId2, const std::string &bundleName,
    const std::string &moduleName, const std::string &intentName,
    ExtractInsightIntentGenericInfo &genericInfo, std::vector<ExtractInsightIntentGenericInfo> &genericInfos,
    ExtractInsightIntentInfo &intentInfo, std::vector<ExtractInsightIntentInfo> &intentInfos,
    std::vector<InsightIntentInfo> &configInfos, InsightIntentInfo &info2)
{
    auto cache = DelayedSingleton<InsightIntentDbCache>::GetInstance();
    cache->GetAllInsightIntentGenericInfo(userId, genericInfos);
    cache->GetInsightIntentGenericInfoByName(IntentKey{bundleName, "", "", userId}, genericInfos);
    cache->GetInsightIntentGenericInfo(IntentKey{bundleName, moduleName, intentName, userId}, genericInfo);
    cache->GetAllInsightIntentInfo(userId, intentInfos, configInfos);
    cache->GetInsightIntentInfoByName(IntentKey{bundleName, "", "", userId}, intentInfos);
    cache->InitInsightIntentCache(userId2);
    cache->GetInsightIntentInfoByName(IntentKey{bundleName, "", "", userId}, intentInfos);
    cache->GetAllInsightIntentInfo(userId, intentInfos, configInfos);
    cache->GetAllInsightIntentGenericInfo(userId, genericInfos);
    cache->GetAllConfigInsightIntentInfo(userId, configInfos);
    cache->GetConfigInsightIntentInfo(IntentKey{bundleName, moduleName, intentName, userId}, info2);
    cache->GetConfigInsightIntentInfoByName(IntentKey{bundleName, "", "", userId}, configInfos);
    cache->GetInsightIntentInfo(IntentKey{bundleName, moduleName, intentName, userId}, intentInfo);
}
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    int32_t userId;
    int32_t userId2;
    std::string bundleName;
    std::string moduleName;
    std::string intentName;
    ExtractInsightIntentProfileInfo info;
    ExtractInsightIntentProfileInfoVec profileInfos;
    ExtractInsightIntentGenericInfo genericInfo;
    std::vector<ExtractInsightIntentGenericInfo> genericInfos;
    ExtractInsightIntentInfo intentInfo;
    std::vector<ExtractInsightIntentInfo> intentInfos;
    std::vector<InsightIntentInfo> configIntentInfos;
    std::vector<InsightIntentInfo> configInfos;
    InsightIntentInfo info2;
    FuzzedDataProvider fdp(data, size);
    userId = fdp.ConsumeIntegral<int32_t>();
    bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    moduleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    intentName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    uint32_t versionCode = fdp.ConsumeIntegralInRange<int32_t>(0, U32_AT_SIZE);
    profileInfos.insightIntents.push_back(info);
    AbilityFuzzUtil::GetRandomExtractInsightIntentGenericInfo(fdp, genericInfo);
    AbilityFuzzUtil::GetRandomExtractInsightIntentInfo(fdp, intentInfo);
    userId2 = fdp.ConsumeIntegral<int32_t>();
    SaveAndDeleteInsightIntent(userId, bundleName, moduleName, versionCode, profileInfos, configIntentInfos);
    QueryInsightIntent(userId, userId2, bundleName, moduleName, intentName, genericInfo, genericInfos,
        intentInfo, intentInfos, configInfos, info2);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    // Run your code on data.
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}