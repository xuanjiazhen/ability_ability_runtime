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

#include "screenunlockinterceptorfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "bundle_mgr_helper.h"
#define private public
#include "screen_unlock_interceptor.h"
#undef private

#include "ability_record.h"
#include "securec.h"
#include "want_params.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace {
constexpr int INPUT_ZERO = 0;
constexpr int INPUT_ONE = 1;
constexpr int INPUT_TWO = 2;
constexpr int INPUT_THREE = 3;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t OFFSET_ZERO = 24;
constexpr size_t OFFSET_ONE = 16;
constexpr size_t OFFSET_TWO = 8;
constexpr uint8_t ENABLE = 2;
constexpr size_t MAX_STR_LEN = 256;
} // namespace

uint32_t GetU32Data(const char* ptr)
{
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) |
           (ptr[INPUT_TWO] << OFFSET_TWO) | ptr[INPUT_THREE];
}

// 构造模糊的Want对象，覆盖bundleName/abilityName等关键字段
Want BuildFuzzWant(const char* data, size_t size)
{
    Want want;
    std::string bundleName(data, size > MAX_STR_LEN ? MAX_STR_LEN : size);
    std::string abilityName(data + INPUT_THREE, size > INPUT_THREE + MAX_STR_LEN ? 
        MAX_STR_LEN : size - INPUT_THREE);
    
    ElementName element(bundleName, abilityName, "");
    want.SetElement(element);
    want.SetStringParam("test_key", std::string(data, size % MAX_STR_LEN));
    return want;
}

// 构造模糊的AbilityInfo，覆盖type/extensionType等分支判断字段
AbilityInfo BuildFuzzAbilityInfo(const char* data, size_t size)
{
    AbilityInfo abilityInfo;
    abilityInfo.type = static_cast<AbilityType>(GetU32Data(data) % 
        static_cast<uint32_t>(AbilityType::UNKNOWN));
    abilityInfo.extensionAbilityType = static_cast<ExtensionAbilityType>(
        GetU32Data(data + INPUT_THREE) % static_cast<uint32_t>(ExtensionAbilityType::UNSPECIFIED));
    abilityInfo.extensionTypeName = std::string(data + INPUT_TWO, size % MAX_STR_LEN);
    abilityInfo.applicationInfo.bundleName = std::string(data, size % MAX_STR_LEN);
    abilityInfo.applicationInfo.name = std::string(data + INPUT_ONE, size % MAX_STR_LEN);
    abilityInfo.applicationInfo.isSystemApp = (GetU32Data(data) % 2) == 1;
    abilityInfo.applicationInfo.allowAppRunWhenDeviceFirstLocked = (GetU32Data(data + INPUT_TWO) % 2) == 1;
    return abilityInfo;
}

sptr<Token> GetFuzzAbilityToken(const char* data, size_t size)
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = std::string(data, size % MAX_STR_LEN);
    abilityRequest.abilityInfo.name = std::string(data + INPUT_THREE, size % MAX_STR_LEN);
    abilityRequest.abilityInfo.type = static_cast<AbilityType>(GetU32Data(data) % 
        static_cast<uint32_t>(AbilityType::UNKNOWN));
    
    std::shared_ptr<AbilityRecord> abilityRecord =
        AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord) {
        token = abilityRecord->GetToken();
    }
    return token;
}

bool DoSomethingInterestingWithMyAPI(const char *data, size_t size)
{
    std::shared_ptr<ScreenUnlockInterceptor> screenUnlockInterceptor =
        std::make_shared<ScreenUnlockInterceptor>();
    int intParam = static_cast<int>(GetU32Data(data));
    int32_t int32Param = static_cast<int32_t>(GetU32Data(data + INPUT_ONE));
    Want want = BuildFuzzWant(data, size);
    bool boolParam = *data % ENABLE;
    sptr<IRemoteObject> token = GetFuzzAbilityToken(data, size);
    auto shouldBlockFunc = []() { return false; };
    AbilityInterceptorParam param(want, intParam, int32Param, boolParam, token, shouldBlockFunc);
    AppExecFwk::AbilityInfo targetAbilityInfo = BuildFuzzAbilityInfo(data, size);
    
    // 覆盖核心函数，触发更多分支
    screenUnlockInterceptor->DoProcess(param);
    screenUnlockInterceptor->QueryTargetAbilityInfo(param, targetAbilityInfo);
    screenUnlockInterceptor->RecordExtensionEventWhenScreenUnlock(param, targetAbilityInfo);
    
    // 额外构造空参数场景，覆盖空值判断分支
    AbilityInterceptorParam emptyParam(Want(), 0, 0, false, nullptr, shouldBlockFunc);
    AppExecFwk::AbilityInfo emptyAbilityInfo;
    screenUnlockInterceptor->DoProcess(emptyParam);
    screenUnlockInterceptor->RecordExtensionEventWhenScreenUnlock(emptyParam, emptyAbilityInfo);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        std::cout << "invalid data" << std::endl;
        return 0;
    }

    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    char *ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        std::cout << "malloc failed." << std::endl;
        return 0;
    }

    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        std::cout << "copy failed." << std::endl;
        free(ch);
        ch = nullptr;
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(ch, size);
    free(ch);
    ch = nullptr;
    return 0;
}