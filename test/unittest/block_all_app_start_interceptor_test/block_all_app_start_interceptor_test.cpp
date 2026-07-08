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

#include <gtest/gtest.h>
#define private public
#define protected public
#include "interceptor/block_all_app_start_interceptor.h"
#undef private
#undef protected

#include "hilog_tag_wrapper.h"
#include "ability_info.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class BlockAllAppStartInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
};

void BlockAllAppStartInterceptorTest::SetUpTestCase()
{}

void BlockAllAppStartInterceptorTest::TearDownTestCase()
{}

void BlockAllAppStartInterceptorTest::SetUp()
{}

void BlockAllAppStartInterceptorTest::TearDown()
{}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_001
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_001, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldBlockAllAppStartFunc = nullptr;
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    auto ret = blockAllAppStartInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_002
 * @tc.desc: DoProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_002, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return true;
    };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    auto ret = blockAllAppStartInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_003
 * @tc.desc: DoProcess with shouldBlockAllAppStartFunc returning false
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_003, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    Want want;
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return false;
    };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    auto ret = blockAllAppStartInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_004
 * @tc.desc: DoProcess with implicit start (empty abilityName) and block enabled
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_004, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    Want want;
    // Set empty abilityName for implicit start
    want.SetElementName("com.example.test", "");
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return true;
    };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    // Implicit start with block enabled should be blocked (abilityName empty = not running)
    auto ret = blockAllAppStartInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_005
 * @tc.desc: DoProcess with non-PAGE type ability (SERVICE) and block enabled
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_005, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    Want want;
    want.SetElementName("com.example.test", "ServiceAbility");
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return true;
    };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    // Create abilityInfo with SERVICE type and set manually
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::SERVICE;
    param.abilityInfo = abilityInfo;
    // SERVICE type with block enabled: returns ERR_ALL_APP_START_BLOCKED
    // Covers: DoProcess branch2 + IsUIAbilityAlreadyRunning branch5
    auto ret = blockAllAppStartInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_006
 * @tc.desc: DoProcess with non-PAGE type ability (DATA) and block disabled
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_006, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    Want want;
    want.SetElementName("com.example.test", "DataAbility");
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return false;
    };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    // Create abilityInfo with DATA type and set manually
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::DATA;
    param.abilityInfo = abilityInfo;
    // DATA type with block disabled: returns ERR_OK
    // Covers: DoProcess branch3 + IsUIAbilityAlreadyRunning branch5
    auto ret = blockAllAppStartInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_007
 * @tc.desc: DoProcess with PAGE type ability and block function returning true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_007, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    Want want;
    want.SetElementName("com.example.test", "MainAbility");
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return true;
    };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    // Create abilityInfo with PAGE type and set manually
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    param.abilityInfo = abilityInfo;
    // PAGE type with block=true should be blocked (unless already running, but abilityms is nullptr in test)
    auto ret = blockAllAppStartInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_DoProcess_008
 * @tc.desc: DoProcess with PAGE type ability and block function returning false
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, DoProcess_008, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    Want want;
    want.SetElementName("com.example.test", "MainAbility");
    int requestCode = 123;
    int32_t userId = 1001;
    bool isWithUI = true;
    sptr<IRemoteObject> callerToken = nullptr;
    std::function<bool(void)> shouldBlockAllAppStartFunc = []() -> bool {
        return false;
    };
    AbilityInterceptorParam param(want, requestCode, userId, isWithUI, callerToken, shouldBlockAllAppStartFunc);
    // Create abilityInfo with PAGE type and set manually
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->type = AppExecFwk::AbilityType::PAGE;
    param.abilityInfo = abilityInfo;
    // PAGE type with block=false should return ERR_OK
    auto ret = blockAllAppStartInterceptor.DoProcess(param);
    EXPECT_EQ(ret, ERR_OK);
}
} // namespace AAFwk
} // namespace OHOS
