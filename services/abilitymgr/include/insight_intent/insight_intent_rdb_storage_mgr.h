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

#ifndef OHOS_INSIGHT_INTENT_RDB_STORAGE_MGR_H
#define OHOS_INSIGHT_INTENT_RDB_STORAGE_MGR_H

#include <singleton.h>
#include <string>
#include "insight_intent_rdb_data_mgr.h"

namespace OHOS {
namespace AAFwk {
class InsightRdbStorageMgr : public std::enable_shared_from_this<InsightRdbStorageMgr> {
    DECLARE_DELAYED_SINGLETON(InsightRdbStorageMgr)
public:

    ErrCode LoadInsightIntentInfos(std::vector<InstghtIntentGenericInfoVec> &genericInfos);
    ErrCode SaveStorageInsightIntentData(const InstghtIntentGenericInfoVec &genericInfo);
    ErrCode ModifyStorageInsightIntentData(const InstghtIntentGenericInfoVec &genericInfo);
    ErrCode DeleteStorageInsightIntentData(const std::string bundleName);
private:
    void SaveEntries(const std::unordered_map<std::string, std::string> &value,
        std::vector<InstghtIntentGenericInfoVec> &genericInfos);
    mutable std::mutex rdbStorePtrMutex_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif // OHOS_INSIGHT_INTENT_RDB_STORAGE_MGR_H