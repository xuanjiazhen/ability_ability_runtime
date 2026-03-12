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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_RECORD_MOCK_HELPER_H
#define OHOS_ABILITY_RUNTIME_ABILITY_RECORD_MOCK_HELPER_H

#include "ability_record.h"

namespace OHOS {
namespace AAFwk {

/**
 * Helper macro for mock classes to provide a simple DeepCopy implementation
 * This macro should be used in mock class declarations
 */
#define ENABLE_DEFAULT_DEEP_COPY() \
    std::shared_ptr<AbilityRecord> DeepCopy() const override \
    { \
        return std::make_shared<AbilityRecord>(want_, abilityInfo_, abilityInfo_.applicationInfo, requestCode_); \
    }

}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_RECORD_MOCK_HELPER_H