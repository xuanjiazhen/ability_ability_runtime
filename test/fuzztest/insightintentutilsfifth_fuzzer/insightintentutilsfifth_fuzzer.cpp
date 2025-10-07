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

#include "insightintentutilsfifth_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

#include "ability_fuzz_util.h"
#include "insight_intent_utils.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t CODE_FOUR = 4;
constexpr size_t STRING_MAX_LENGTH = 128;
}
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    InsightIntentInfo info;
    AppExecFwk::ExecuteMode executeMode;
    std::string abilityName;
    abilityName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    executeMode = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    CheckAbilityName(info, abilityName, executeMode);

    executeMode = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    CheckAbilityName(info, abilityName, executeMode);

    executeMode = AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY;
    CheckAbilityName(info, abilityName, executeMode);

    executeMode = AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY;
    CheckAbilityName(info, abilityName, executeMode);    
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