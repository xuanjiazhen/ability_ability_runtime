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

#include "ability_manager_service.h"
#include "hilog_tag_wrapper.h"

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
 * @tc.desc: DoProcess
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
 * @tc.name: Execute_RecoveryWhitelist_001
 * @tc.desc: An allowlisted bundle can start in strict recovery mode.
 * @tc.type: FUNC
 */
HWTEST_F(BlockAllAppStartInterceptorTest, Execute_RecoveryWhitelist_001, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    interceptor.SetIsAbilityStartedFunc([](AbilityRequest &, int32_t) { return false; });
    interceptor.SetAllowStartFunc([](AbilityRequest &request) {
        return request.abilityInfo.bundleName == "com.example.recovery";
    });
    interceptor.SetStrictBlockFunc([]() { return true; });
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.recovery";
    EXPECT_EQ(interceptor.Execute(request, 100), ERR_OK);
}

/**
 * @tc.name: Execute_RecoveryWhitelist_002
 * @tc.desc: A started non-allowlisted page remains blocked in strict recovery mode.
 * @tc.type: FUNC
 */
HWTEST_F(BlockAllAppStartInterceptorTest, Execute_RecoveryWhitelist_002, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    interceptor.SetIsAbilityStartedFunc([](AbilityRequest &, int32_t) { return true; });
    interceptor.SetAllowStartFunc([](AbilityRequest &) { return false; });
    interceptor.SetStrictBlockFunc([]() { return true; });
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.normal";
    request.abilityInfo.type = AbilityType::PAGE;
    EXPECT_EQ(interceptor.Execute(request, 100), ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: Execute_RecoveryBackupExtension_001
 * @tc.desc: 恢复模式允许受控 BackupExtensionAbility 使用专用例外。
 * @tc.type: FUNC
 */
HWTEST_F(BlockAllAppStartInterceptorTest, Execute_RecoveryBackupExtension_001, TestSize.Level1)
{
    BlockAllAppStartInterceptor interceptor;
    interceptor.SetShouldBlockFunc([]() { return true; });
    interceptor.SetIsAbilityStartedFunc([](AbilityRequest &, int32_t) { return false; });
    interceptor.SetAllowStartFunc([](AbilityRequest &request) {
        return request.abilityInfo.extensionAbilityType == ExtensionAbilityType::BACKUP;
    });
    interceptor.SetStrictBlockFunc([]() { return true; });
    AbilityRequest request;
    request.abilityInfo.bundleName = "com.example.selected";
    request.abilityInfo.extensionAbilityType = ExtensionAbilityType::BACKUP;
    request.abilityInfo.type = AbilityType::EXTENSION;
    EXPECT_EQ(interceptor.Execute(request, 100), ERR_OK);
}
} // namespace AAFwk
} // namespace OHOS
