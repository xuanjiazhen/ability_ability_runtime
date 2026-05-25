/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_RESIDENT_ON_DEMAND_RDB_H
#define OHOS_ABILITY_RUNTIME_RESIDENT_ON_DEMAND_RDB_H

#include <map>
#include <memory>
#include <string>

#include "rdb_data_manager.h"

namespace OHOS {
namespace AbilityRuntime {

/**
 * @class ResidentOnDemandRdbCallBack
 * @brief RDB callback for resident on-demand database.
 */
class ResidentOnDemandRdbCallBack : public NativeRdb::RdbOpenCallback {
public:
    explicit ResidentOnDemandRdbCallBack(const AmsRdbConfig &rdbConfig);
    ~ResidentOnDemandRdbCallBack() override = default;

    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
    int32_t OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
    int32_t OnOpen(NativeRdb::RdbStore &rdbStore) override;
    int32_t onCorruption(std::string databaseFile) override;

private:
    AmsRdbConfig rdbConfig_;
};

/**
 * @class ResidentOnDemandRdb
 * @brief Manages the on-demand startup configuration for resident processes.
 *
 * This class provides functionality to store and retrieve on-demand startup
 * configurations for resident applications in a key-value database.
 */
class ResidentOnDemandRdb final {
public:
    ResidentOnDemandRdb() {}
    ~ResidentOnDemandRdb() {}

    static ResidentOnDemandRdb& GetInstance();

    int32_t Init();

    bool GetOnDemandStartup(const std::string &bundleName);

    int32_t SetOnDemandStartup(const std::string &bundleName, bool onDemandStartup);

    int32_t SetOnDemandStartupBatch(const std::map<std::string, bool> &configList);

private:
    std::unique_ptr<RdbDataManager> rdbMgr_ = nullptr;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_RESIDENT_ON_DEMAND_RDB_H