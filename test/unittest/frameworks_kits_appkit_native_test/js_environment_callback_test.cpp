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

#include "configuration.h"
#include "environment_callback.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "native_engine/native_engine.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;

namespace {
constexpr int32_t TEST_MEMORY_LEVEL = 1;
}

class JsEnvironmentCallbackTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override;
    void TearDown() override;

protected:
    std::unique_ptr<JsRuntime> jsRuntime_ = nullptr;
    napi_env env_ = nullptr;
    std::shared_ptr<JsEnvironmentCallback> jsEnvCallback_ = nullptr;
};

void JsEnvironmentCallbackTest::SetUp()
{
    JsRuntime::Options options;
    jsRuntime_ = JsRuntime::Create(options);
    ASSERT_NE(jsRuntime_, nullptr);
    env_ = jsRuntime_->GetNapiEnv();
    ASSERT_NE(env_, nullptr);
    jsEnvCallback_ = std::make_shared<JsEnvironmentCallback>(env_);
    ASSERT_NE(jsEnvCallback_, nullptr);
}

void JsEnvironmentCallbackTest::TearDown()
{
    jsEnvCallback_ = nullptr;
    jsRuntime_.reset();
}

napi_value TestCallback(napi_env env, napi_callback_info info)
{
    return nullptr;
}

/**
 * @tc.name: JsEnvironmentCallback_OnConfigurationUpdated_001
 * @tc.desc: Test OnConfigurationUpdated with valid config
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_OnConfigurationUpdated_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_001 start");
    AppExecFwk::Configuration config;
    jsEnvCallback_->OnConfigurationUpdated(config);
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_001 end");
}

/**
 * @tc.name: JsEnvironmentCallback_OnConfigurationUpdated_002
 * @tc.desc: Test OnConfigurationUpdated multiple times
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_OnConfigurationUpdated_002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_002 start");
    AppExecFwk::Configuration config;
    for (int i = 0; i < 3; i++) {
        jsEnvCallback_->OnConfigurationUpdated(config);
    }
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnConfigurationUpdated_002 end");
}

/**
 * @tc.name: JsEnvironmentCallback_OnMemoryLevel_001
 * @tc.desc: Test OnMemoryLevel with valid level
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_OnMemoryLevel_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "Test OnMemoryLevel with valid level");
    jsEnvCallback_->OnMemoryLevel(TEST_MEMORY_LEVEL);
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnMemoryLevel_001 end");
}

/**
 * @tc.name: JsEnvironmentCallback_OnMemoryLevel_002
 * @tc.desc: Test OnMemoryLevel multiple times
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_OnMemoryLevel_002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnMemoryLevel_002 start");
    for (int i = 0; i < 3; i++) {
        jsEnvCallback_->OnMemoryLevel(TEST_MEMORY_LEVEL + i);
    }
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_OnMemoryLevel_002 end");
}

/**
 * @tc.name: JsEnvironmentCallback_Register_001
 * @tc.desc: Test Register with valid callback
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_Register_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_Register_001 start");
    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);
    
    int32_t callbackId = jsEnvCallback_->Register(callback, false);
    EXPECT_GE(callbackId, 0);
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_Register_001 end");
}

/**
 * @tc.name: JsEnvironmentCallback_Register_002
 * @tc.desc: Test Register with sync callback
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_Register_002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_Register_002 start");
    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);
    
    int32_t callbackId = jsEnvCallback_->Register(callback, true);
    EXPECT_GE(callbackId, 0);
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_Register_002 end");
}

/**
 * @tc.name: JsEnvironmentCallback_Register_003
 * @tc.desc: Test Register multiple callbacks
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_Register_003, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_Register_003 start");
    const int32_t callbackCount = 5;
    std::vector<int32_t> callbackIds;
    
    for (int32_t i = 0; i < callbackCount; i++) {
        napi_value callback = nullptr;
        napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
        ASSERT_NE(callback, nullptr);
        
        int32_t callbackId = jsEnvCallback_->Register(callback, false);
        EXPECT_GE(callbackId, 0);
        callbackIds.push_back(callbackId);
    }
    
    EXPECT_FALSE(jsEnvCallback_->IsEmpty());
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_Register_003 end");
}

/**
 * @tc.name: JsEnvironmentCallback_UnRegister_001
 * @tc.desc: Test UnRegister with valid callbackId
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_UnRegister_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_UnRegister_001 start");
    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);
    
    int32_t callbackId = jsEnvCallback_->Register(callback, false);
    EXPECT_GE(callbackId, 0);
    
    bool result = jsEnvCallback_->UnRegister(callbackId, false);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_UnRegister_001 end");
}

/**
 * @tc.name: JsEnvironmentCallback_UnRegister_002
 * @tc.desc: Test UnRegister sync callback
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_UnRegister_002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_UnRegister_002 start");
    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);
    
    int32_t callbackId = jsEnvCallback_->Register(callback, true);
    EXPECT_GE(callbackId, 0);
    
    bool result = jsEnvCallback_->UnRegister(callbackId, true);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_UnRegister_002 end");
}

/**
 * @tc.name: JsEnvironmentCallback_UnRegister_003
 * @tc.desc: Test UnRegister with invalid callbackId
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_UnRegister_003, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_UnRegister_003 start");
    bool result = jsEnvCallback_->UnRegister(-1, false);
    EXPECT_FALSE(result);
    result = jsEnvCallback_->UnRegister(9999, true);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_UnRegister_003 end");
}

/**
 * @tc.name: JsEnvironmentCallback_IsEmpty_001
 * @tc.desc: Test IsEmpty when no callbacks registered
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_IsEmpty_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_IsEmpty_001 start");
    bool isEmpty = jsEnvCallback_->IsEmpty();
    EXPECT_TRUE(isEmpty);
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_IsEmpty_001 end");
}

/**
 * @tc.name: JsEnvironmentCallback_IsEmpty_002
 * @tc.desc: Test IsEmpty when callbacks registered
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_IsEmpty_002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_IsEmpty_002 start");
    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);
    
    jsEnvCallback_->Register(callback, false);
    bool isEmpty = jsEnvCallback_->IsEmpty();
    EXPECT_FALSE(isEmpty);
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_IsEmpty_002 end");
}

/**
 * @tc.name: JsEnvironmentCallback_IsEmpty_003
 * @tc.desc: Test IsEmpty after unregister all callbacks
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_IsEmpty_003, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_IsEmpty_003 start");
    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);
    
    int32_t callbackId = jsEnvCallback_->Register(callback, false);
    EXPECT_GE(callbackId, 0);
    
    jsEnvCallback_->UnRegister(callbackId, false);
    bool isEmpty = jsEnvCallback_->IsEmpty();
    EXPECT_TRUE(isEmpty);
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_IsEmpty_003 end");
}

/**
 * @tc.name: JsEnvironmentCallback_Integration_001
 * @tc.desc: Test integration of Register, OnConfigurationUpdated, and UnRegister
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_Integration_001, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_Integration_001 start");
    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);
    
    int32_t callbackId = jsEnvCallback_->Register(callback, false);
    EXPECT_GE(callbackId, 0);
    
    AppExecFwk::Configuration config;
    jsEnvCallback_->OnConfigurationUpdated(config);
    
    bool result = jsEnvCallback_->UnRegister(callbackId, false);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_Integration_001 end");
}

/**
 * @tc.name: JsEnvironmentCallback_Integration_002
 * @tc.desc: Test integration with both sync and async callbacks
 * @tc.type: FUNC
 */
HWTEST_F(JsEnvironmentCallbackTest, JsEnvironmentCallback_Integration_002, Function | MediumTest | Level1)
{
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_Integration_002 start");
    napi_value callback = nullptr;
    napi_create_function(env_, "testCallback", NAPI_AUTO_LENGTH, TestCallback, nullptr, &callback);
    ASSERT_NE(callback, nullptr);
    
    int32_t asyncCallbackId = jsEnvCallback_->Register(callback, false);
    EXPECT_GE(asyncCallbackId, 0);
    
    int32_t syncCallbackId = jsEnvCallback_->Register(callback, true);
    EXPECT_GE(syncCallbackId, 0);
    
    EXPECT_FALSE(jsEnvCallback_->IsEmpty());
    
    jsEnvCallback_->UnRegister(asyncCallbackId, false);
    jsEnvCallback_->UnRegister(syncCallbackId, true);
    
    EXPECT_TRUE(jsEnvCallback_->IsEmpty());
    TAG_LOGI(AAFwkTag::APPKIT, "JsEnvironmentCallback_Integration_002 end");
}
