/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INNER_COMMON_H
#define OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INNER_COMMON_H

#include "data_ability_observer_interface.h"

namespace OHOS {
namespace AAFwk {

struct ObserverNode {
    sptr<IDataAbilityObserver> observer_ = nullptr;
    int32_t userID_ = -1;
    int32_t appIndex_ = -1;
    std::string key_ = "";

    ObserverNode(sptr<IDataAbilityObserver> observer, int32_t userID, int32_t appIndex, std::string key)
    {
        observer_ = observer;
        userID_ = userID;
        appIndex_ = appIndex;
        key_ = key;
    }
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DATAOBS_MGR_INNER_COMMON_H
