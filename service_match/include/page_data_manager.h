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

#ifndef FOUNDATION_ABILITY_RUNTIME_SERVICE_MATCH_PAGE_DATA_MANAGER_H
#define FOUNDATION_ABILITY_RUNTIME_SERVICE_MATCH_PAGE_DATA_MANAGER_H

#include "datashare_helper.h"

#include "ability_context.h"
#include "errors.h"
#include "nocopyable.h"
#include "page_data_manager.h"

namespace OHOS {
namespace AppExecFwk {
class PageDataManager final : public NoCopyable {
public:
    PageDataManager() = default;
    ~PageDataManager() = default;

    static PageDataManager &GetInstance();

    ErrCode Initialize(std::weak_ptr<OHOS::AbilityRuntime::AbilityContext> &context);

    bool IsInitialized() const;

    ErrCode GetStringValue(const std::string &key, std::string &value);
    
    private:

    void SetToken(const sptr<IRemoteObject>& token);
    
    void CreateDataShareHelperAndUri(const std::string& key,
        std::string& uri, std::shared_ptr<DataShare::DataShareHelper>& helper) const;

    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper() const;

    static bool ReleaseDataShareHelper(const std::shared_ptr<DataShare::DataShareHelper>& helper);

    std::string AssembleUri(const std::string& key) const;

    sptr<IRemoteObject> remoteObject_ = nullptr;

    bool isInitialized_ = false;

    std::mutex initializeMutex_;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // FOUNDATION_ABILITY_RUNTIME_SERVICE_MATCH_PAGE_DATA_MANAGER_H