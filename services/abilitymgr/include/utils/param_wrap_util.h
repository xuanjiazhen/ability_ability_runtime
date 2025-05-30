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

#ifndef OHOS_ABILITY_RUNTIME_OPTION_START_PARAM_H
#define OHOS_ABILITY_RUNTIME_OPTION_START_PARAM_H

#include <cinttypes>

namespace OHOS {
namespace AAFwk {
/**
 * @class OptionStartParamWrap
 * provides optional start params.
 */
class OptionStartParamWrap {
public:
    bool isStartAsCaller = false;
    bool isForegroundToRestartApp = false;
    bool isImplicit = false;
    bool isUIAbilityOnly = false;
    bool isFromRecovery = false;
    uint32_t specifyTokenId = 0;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_OPTION_START_PARAM_H
