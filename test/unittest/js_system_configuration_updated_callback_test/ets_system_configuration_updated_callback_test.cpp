/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
.
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

/**
 * @file ets_system_configuration_updated_callback_test.cpp
 * @brief Unit tests for EtsSystemConfigurationUpdatedCallback class
 * @description This file contains tests for ETS system configuration updated callback functionality.
 */

#include <gtest/gtest.h>
#include <mutex>
#include <set>
#include <cerrno>
#include <cstdlib>
#include <string>

#define private public
#define protected public
#include "ets_system_configuration_updated_callback.h"
#undef private
#undef protected

#include "mock_native_reference.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class EtsSystemConfigurationUpdatedCallbackTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();

    ani_vm *etsVm_ = nullptr;
    std::shared_ptr<EtsSystemConfigurationUpdatedCallback> callback_ = nullptr;
};

void EtsSystemConfigurationUpdatedCallbackTest::SetUpTestCase()
{}

void EtsSystemConfigurationUpdatedCallbackTest::TearDownTestCase()
{}

void EtsSystemConfigurationUpdatedCallbackTest::SetUp()
{
    etsVm_ = reinterpret_cast<ani_vm *>(0x1);
    callback_ = std::make_shared<EtsSystemConfigurationUpdatedCallback>(etsVm_);
}

void EtsSystemConfigurationUpdatedCallbackTest::TearDown()
{
    callback_.reset();
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_Constructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_Constructor_0100 start";
    ani_vm *vm = reinterpret_cast<ani_vm *>(0x1);
    auto callback = std::make_shared<EtsSystemConfigurationUpdatedCallback>(vm);
    EXPECT_NE(callback, nullptr);
    EXPECT_EQ(callback->etsVm_, vm);
    EXPECT_TRUE(callback->callbacksRef_.empty());
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_Constructor_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_Destructor_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_Destructor_0100 start";
    auto testCallback = std::make_shared<EtsSystemConfigurationUpdatedCallback>(etsVm_);
    testCallback.reset();
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_Destructor_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_IsEmpty_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_IsEmpty_0100 start";
    EXPECT_TRUE(callback_->IsEmpty());
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_IsEmpty_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_Register_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_Register_0100 start";
    callback_->Register(nullptr);
    EXPECT_TRUE(callback_->callbacksRef_.empty());
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_Register_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_UnRegister_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_UnRegister_0100 start";
    ani_ref mockRef = reinterpret_cast<ani_ref>(0x2);
    callback_->callbacksRef_.insert(mockRef);
    EXPECT_FALSE(callback_->IsEmpty());

    bool result = callback_->UnRegister(nullptr);
    EXPECT_TRUE(result);
    EXPECT_TRUE(callback_->IsEmpty());
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_UnRegister_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_UnRegister_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_UnRegister_0200 start";
    bool result = callback_->UnRegister(nullptr);
    EXPECT_TRUE(result);
    EXPECT_TRUE(callback_->IsEmpty());
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_UnRegister_0200 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_IsEquel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_IsEquel_0100 start";
    ani_object aniCallback = reinterpret_cast<ani_object>(0x2);
    ani_ref refCallback = reinterpret_cast<ani_ref>(0x3);

    bool result = callback_->IsEquel(nullptr, aniCallback, refCallback);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_IsEquel_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_CheckAndGetAniMethod_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_CheckAndGetAniMethod_0100 start";
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = nullptr;

    bool result = callback_->CheckAndGetAniMethod(nullptr, callback, "onUpdate", method);
    EXPECT_FALSE(result);
    EXPECT_EQ(method, nullptr);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_CheckAndGetAniMethod_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_CheckAndGetAniMethod_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_CheckAndGetAniMethod_0200 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = nullptr;

    bool result = callback_->CheckAndGetAniMethod(env, callback, nullptr, method);
    EXPECT_FALSE(result);
    EXPECT_EQ(method, nullptr);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_CheckAndGetAniMethod_0200 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyColorModeUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyColorModeUpdated_0100 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = reinterpret_cast<ani_ref>(0x3);
    std::string colorMode = "DARK_MODE";

    callback_->NotifyColorModeUpdated(env, callback, method, colorMode);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyColorModeUpdated_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyColorModeUpdated_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyColorModeUpdated_0200 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = nullptr;
    std::string colorMode = "LIGHT_MODE";

    callback_->NotifyColorModeUpdated(env, callback, method, colorMode);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyColorModeUpdated_0200 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0100 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = reinterpret_cast<ani_ref>(0x3);
    std::string fontSizeScale = "1.5";

    callback_->NotifyFontSizeScaleUpdated(env, callback, method, fontSizeScale);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0200 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = reinterpret_cast<ani_ref>(0x3);
    std::string fontSizeScale = "";

    callback_->NotifyFontSizeScaleUpdated(env, callback, method, fontSizeScale);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyFontSizeScaleUpdated_0200 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyFontWeightScaleUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyFontWeightScaleUpdated_0100 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = reinterpret_cast<ani_ref>(0x3);
    std::string fontWeightScale = "1.2";

    callback_->NotifyFontWeightScaleUpdated(env, callback, method, fontWeightScale);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyFontWeightScaleUpdated_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyLanguageUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyLanguageUpdated_0100 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = reinterpret_cast<ani_ref>(0x3);
    std::string language = "zh-CN";

    callback_->NotifyLanguageUpdated(env, callback, method, language);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyLanguageUpdated_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyFontIdUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyFontIdUpdated_0100 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = reinterpret_cast<ani_ref>(0x3);
    std::string fontId = "default";

    callback_->NotifyFontIdUpdated(env, callback, method, fontId);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyFontIdUpdated_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyMCCUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyMCCUpdated_0100 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = reinterpret_cast<ani_ref>(0x3);
    std::string mcc = "460";

    callback_->NotifyMCCUpdated(env, callback, method, mcc);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyMCCUpdated_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyMNCUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyMNCUpdated_0100 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = reinterpret_cast<ani_ref>(0x3);
    std::string mnc = "01";

    callback_->NotifyMNCUpdated(env, callback, method, mnc);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyMNCUpdated_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyLocaleUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyLocaleUpdated_0100 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = reinterpret_cast<ani_ref>(0x3);
    std::string locale = "zh-Hans-CN";

    callback_->NotifyLocaleUpdated(env, callback, method, locale);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyLocaleUpdated_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0100 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = reinterpret_cast<ani_ref>(0x3);
    std::string hasPointerDevice = "true";

    callback_->NotifyHasPointerDeviceUpdated(env, callback, method, hasPointerDevice);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0200 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = reinterpret_cast<ani_ref>(0x3);
    std::string hasPointerDevice = "false";

    callback_->NotifyHasPointerDeviceUpdated(env, callback, method, hasPointerDevice);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifyHasPointerDeviceUpdated_0200 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0100 start";
    AppExecFwk::Configuration config;

    callback_->NotifySystemConfigurationUpdated(config);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0100 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0200 start";
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "DARK_MODE");

    callback_->NotifySystemConfigurationUpdated(config);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0200 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0300 start";
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE, "1.5");

    callback_->NotifySystemConfigurationUpdated(config);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0300 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0400 start";
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh-CN");

    callback_->NotifySystemConfigurationUpdated(config);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0400 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0500 start";
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "DARK_MODE");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_SIZE_SCALE, "1.5");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh-CN");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_MCC, "460");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_MNC, "01");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LOCALE, "zh-Hans-CN");
    config.AddItem(AAFwk::GlobalConfigurationKey::INPUT_POINTER_DEVICE, "true");
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_FONT_ID, "default");

    callback_->NotifySystemConfigurationUpdated(config);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_NotifySystemConfigurationUpdated_0500 end";
}

HWTEST_F(EtsSystemConfigurationUpdatedCallbackTest, EtsSystemConfigurationUpdatedCallback_CallAniMethod_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_CallAniMethod_0100 start";
    ani_env *env = reinterpret_cast<ani_env *>(0x1);
    ani_ref callback = reinterpret_cast<ani_ref>(0x2);
    ani_ref method = nullptr;
    std::string value = "test";

    callback_->CallAniMethod<std::string, ani_string>(env, callback, method, value);
    GTEST_LOG_(INFO) << "EtsSystemConfigurationUpdatedCallback_CallAniMethod_0100 end";
}

class EtsConvertToDoubleTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();

    static bool IsValidValue(const char *end, const std::string &str)
    {
        if (!end) {
            return false;
        }

        if (end == str.c_str() || errno == ERANGE || *end != '\0') {
            return false;
        }
        return true;
    }

    static bool ConvertToDouble(const std::string &str, double &outValue)
    {
        if (str.empty()) {
            return false;
        }
        char *end = nullptr;
        errno = 0;
        double value = std::strtod(str.c_str(), &end);
        if (!IsValidValue(end, str)) {
            return false;
        }
        outValue = value;
        return true;
    }
};

void EtsConvertToDoubleTest::SetUpTestCase()
{}

void EtsConvertToDoubleTest::TearDownTestCase()
{}

void EtsConvertToDoubleTest::SetUp()
{
    errno = 0;
}

void EtsConvertToDoubleTest::TearDown()
{}

HWTEST_F(EtsConvertToDoubleTest, EtsConvertToDouble_Valid_Integer_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsConvertToDouble_Valid_Integer_0100 start";
    std::string str = "123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.0);
    GTEST_LOG_(INFO) << "EtsConvertToDouble_Valid_Integer_0100 end";
}

HWTEST_F(EtsConvertToDoubleTest, EtsConvertToDouble_Valid_Decimal_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsConvertToDouble_Valid_Decimal_0100 start";
    std::string str = "123.456";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_TRUE(ret);
    EXPECT_DOUBLE_EQ(result, 123.456);
    GTEST_LOG_(INFO) << "EtsConvertToDouble_Valid_Decimal_0100 end";
}

HWTEST_F(EtsConvertToDoubleTest, EtsConvertToDouble_Error_Empty_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsConvertToDouble_Error_Empty_0100 start";
    std::string str = "";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "EtsConvertToDouble_Error_Empty_0100 end";
}

HWTEST_F(EtsConvertToDoubleTest, EtsConvertToDouble_Error_Invalid_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EtsConvertToDouble_Error_Invalid_0100 start";
    std::string str = "abc123";
    double result = 0.0;
    bool ret = ConvertToDouble(str, result);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "EtsConvertToDoubleDouble_Error_Invalid_0100 end";
}

}  // namespace AbilityRuntime
}  // namespace OHOS
