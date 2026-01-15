/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "abilitymgrecologicalruleinterceptorfirst_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>

#define private public
#include "ecological_rule_interceptor.h"
#undef private

#include "ability_ecological_rule_mgr_service_param.h"
#include "ability_record.h"
#include "securec.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;
using namespace OHOS::EcologicalRuleMgrService;

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
constexpr int32_t DEFAULT_USER_ID = 10;
constexpr int32_t MAX_BUNDLE_TYPE = 3;
}  // namespace

// 从模糊测试输入数据中提取uint32_t类型数据
uint32_t GetU32Data(const char* ptr)
{
    // 防止指针为空导致越界访问
    if (ptr == nullptr) {
        return 0;
    }
    return (ptr[INPUT_ZERO] << OFFSET_ZERO) | (ptr[INPUT_ONE] << OFFSET_ONE) |
        (ptr[INPUT_TWO] << OFFSET_TWO) | ptr[INPUT_THREE];
}

// 构造有效的AbilityToken，避免空指针
sptr<Token> GetFuzzAbilityToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    
    // 初始化有效的AppInfo和AbilityInfo，避免空属性
    abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.appInfo.bundleType = BundleType::APP;
    abilityRequest.appInfo.appProvisionType = "test_provision_type";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.bundleName = "com.example.fuzzTest";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    abilityRequest.abilityInfo.applicationInfo = abilityRequest.appInfo;
    
    std::shared_ptr<AbilityRecord> abilityRecord =
        AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (abilityRecord != nullptr) {
        token = abilityRecord->GetToken();
    }
    return token;
}

// 构造有效的AbilityInfo实例，避免空指针传递
std::shared_ptr<AppExecFwk::AbilityInfo> GetFuzzAbilityInfo()
{
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    if (abilityInfo == nullptr) {
        return nullptr;
    }
    
    // 初始化核心属性，避免访问空属性
    abilityInfo->name = "FuzzTestAbility";
    abilityInfo->bundleName = "com.example.fuzzTest";
    abilityInfo->type = AbilityType::PAGE;
    abilityInfo->applicationInfo.bundleName = "com.example.fuzzTest";
    abilityInfo->applicationInfo.bundleType = BundleType::APP;
    abilityInfo->applicationInfo.appDistributionType = "test_dist_type";
    abilityInfo->applicationInfo.appProvisionType = "test_provision_type";
    abilityInfo->extensionAbilityType = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    
    return abilityInfo;
}

// 模糊测试核心逻辑，覆盖所有成员函数，避免空指针
bool DoSomethingInterestingWithMyAPI(const char* data, size_t size)
{
    // 防止输入数据为空
    if (data == nullptr || size < U32_AT_SIZE) {
        return false;
    }
    
    // 构造被测试类实例，确保非空
    std::shared_ptr<EcologicalRuleInterceptor> executer =
        std::make_shared<EcologicalRuleInterceptor>();
    if (executer == nullptr) {
        return false;
    }
    
    // 初始化Want对象，设置有效属性，避免空属性访问
    Want want;
    std::string bundleName = "com.example.fuzzTest";
    std::string abilityName = "com.example.fuzzTest.MainAbility";
    std::string callerBundleName = "com.example.fuzzCaller";
    int32_t uid = static_cast<int32_t>(GetU32Data(data));
    int32_t pid = static_cast<int32_t>(GetU32Data(data + U32_AT_SIZE));
    ElementName element("", bundleName, abilityName);
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME, callerBundleName);
    want.SetParam(Want::PARAM_RESV_CALLER_UID, uid);
    want.SetParam(Want::PARAM_RESV_CALLER_PID, pid);
    
    // 提取模糊测试数据，初始化参数
    int requestCode = static_cast<int>(GetU32Data(data)) % 1000;
    int32_t userId = static_cast<int32_t>(GetU32Data(data + U32_AT_SIZE)) % DEFAULT_USER_ID + 1;
    bool isWithUI = (*data) % ENABLE;
    sptr<IRemoteObject> token = GetFuzzAbilityToken();
    auto shouldBlockFunc = []() { return false; };
    
    // 构造有效的AbilityInterceptorParam，避免空成员
    AbilityInterceptorParam param =
        AbilityInterceptorParam(want, requestCode, userId, isWithUI, token, shouldBlockFunc);
    param.isTargetPlugin = false;
    param.isStartAsCaller = false;
    param.isWithUI = isWithUI;
    
    // 初始化核心辅助对象，避免空指针
    auto abilityInfo = GetFuzzAbilityInfo();
    ErmsCallerInfo callerInfo;  // 直接实例化，避免空指针
    AtomicServiceStartupRule rule;  // 直接实例化，避免空指针
    sptr<Want> replaceWant = sptr<Want>(new Want());  // 构造有效Want指针
    int32_t bundleType = static_cast<int32_t>(GetU32Data(data + 2 * U32_AT_SIZE)) % MAX_BUNDLE_TYPE;
    
    // 覆盖所有公有权限（含private转为public）的成员函数，避免空指针调用
    executer->DoProcess(param);
    executer->DoProcess(want, userId);
    if (abilityInfo != nullptr) {
        executer->GetEcologicalTargetInfo(want, abilityInfo, callerInfo);
    }
    executer->GetEcologicalCallerInfo(want, callerInfo, userId, token);
    if (abilityInfo != nullptr) {
        executer->InitErmsCallerInfo(want, abilityInfo, callerInfo, userId, token);
    }
    executer->GetAppTypeByBundleType(bundleType);
    executer->QueryAtomicServiceStartupRule(want, token, userId, rule, replaceWant);
    
    return true;
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* 校验输入数据有效性，避免空指针和过小数据 */
    if (data == nullptr) {
        return 0;
    }
    if (size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    /* 分配内存并拷贝输入数据，避免直接操作原始数据 */
    char* ch = static_cast<char*>(malloc(size + 1));
    if (ch == nullptr) {
        return 0;
    }

    /* 初始化内存并安全拷贝，使用securec函数避免内存越界 */
    (void)memset_s(ch, size + 1, 0x00, size + 1);
    if (memcpy_s(ch, size + 1, data, size) != EOK) {
        free(ch);
        ch = nullptr;
        return 0;
    }

    /* 执行模糊测试核心逻辑 */
    OHOS::DoSomethingInterestingWithMyAPI(ch, size);

    /* 释放内存，避免内存泄漏 */
    free(ch);
    ch = nullptr;
    return 0;
}