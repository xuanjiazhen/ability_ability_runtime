/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "abilitypermissionutilsecond_fuzzer.h"

#define private public
#define protected public
#include "ability_permission_util.h"
#undef protected
#undef private

#include <fuzzer/FuzzedDataProvider.h>
#include <string>

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr size_t STRING_MAX_LENGTH = 128;
}

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    AbilityPermissionUtil &util = AbilityPermissionUtil::GetInstance();
    FuzzedDataProvider fdp(data, size);

    uint32_t tokenId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t hapTokenId = 0;
    util.GetClosestHapTokenId(tokenId, hapTokenId);

    std::string bundleName = fdp.ConsumeRandomLengthString(STRING_MAX_LENGTH);
    util.CheckStartUIAbilityByUserLockStatus(bundleName);

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
