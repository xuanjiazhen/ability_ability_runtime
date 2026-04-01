/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include <gtest/hwext/gtest-multithread.h>

#include "ability_handler.h"
#include "ability_local_record.h"
#include "auto_fill_extension_context.h"
#include "configuration_convertor.h"
#define private public
#define protected public
#include "configuration_utils.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#define private public
#define protected public
#include "js_auto_fill_extension.h"
#include "js_auto_fill_extension_context.h"
#undef private
#undef protected
#include "js_auto_fill_extension_util.h"
#include "js_runtime.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "runtime.h"
#include "string_wrapper.h"
#include "want.h"
#ifdef SUPPORT_GRAPHICS
#include "locale_config.h"
#include "window_scene.h"
#endif

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char JS_AUTO_FILL_EXTENSION_TASK_RUNNER[] = "JsAutoFillExtension";
constexpr char DEFAULT_LANGUAGE[] = "zh_CN";
} // namespace

class JsAutoFillExtensionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<JsAutoFillExtension> jsAutoFillExtension_ = nullptr;
    std::shared_ptr<ApplicationContext> applicationContext_ = nullptr;
    std::unique_ptr<Runtime> jsRuntime_ = nullptr;

private:
    void CreateJsAutoFillExtension();
};

void JsAutoFillExtensionTest::SetUpTestCase()
{}

void JsAutoFillExtensionTest::TearDownTestCase()
{}

void JsAutoFillExtensionTest::SetUp()
{
    CreateJsAutoFillExtension();
}

void JsAutoFillExtensionTest::TearDown()
{}

void JsAutoFillExtensionTest::CreateJsAutoFillExtension()
{
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    sptr<IRemoteObject> token(new (std::nothrow) MockAbilityToken());
    auto record = std::make_shared<AbilityLocalRecord>(abilityInfo, token, nullptr, 0);

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    std::shared_ptr<AbilityRuntime::ContextImpl> contextImpl = std::make_shared<AbilityRuntime::ContextImpl>();

    Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, DEFAULT_LANGUAGE);
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, ConfigurationInner::COLOR_MODE_LIGHT);
    contextImpl->SetConfiguration(std::make_shared<Configuration>(config));

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
#ifdef SUPPORT_GRAPHICS
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag("zh", status);
    TAG_LOGI(AAFwkTag::TEST, "language: %{public}s, script: %{public}s, region: %{public}s", locale.getLanguage(),
             locale.getScript(), locale.getCountry());
    resConfig->SetLocaleInfo(locale);
#endif
    Global::Resource::RState updateRet = resourceManager->UpdateResConfig(*resConfig);
    if (updateRet != Global::Resource::RState::SUCCESS) {
        TAG_LOGE(AAFwkTag::TEST, "Init locale failed.");
    }
    contextImpl->SetResourceManager(resourceManager);

    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext =
        AbilityRuntime::ApplicationContext::GetInstance();
    applicationContext->AttachContextImpl(contextImpl);
    application->SetApplicationContext(applicationContext);
    applicationContext_ = applicationContext;

    std::shared_ptr<AbilityHandler> handler = std::make_shared<AbilityHandler>(nullptr);

    auto eventRunner = AppExecFwk::EventRunner::Create(JS_AUTO_FILL_EXTENSION_TASK_RUNNER);
    Runtime::Options options;
    options.preload = true;
    options.eventRunner = eventRunner;
    jsRuntime_ = JsRuntime::Create(options);
    ASSERT_NE(jsRuntime_, nullptr);

    JsAutoFillExtension *extension = JsAutoFillExtension::Create(jsRuntime_);
    ASSERT_NE(extension, nullptr);
    jsAutoFillExtension_.reset(extension);

    jsAutoFillExtension_->Init(record, application, handler, token);
}

/**
 * @tc.name: Configuration_0100
 * @tc.desc: Js auto fill extension init.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, Init_0100, TestSize.Level1)
{
    ASSERT_NE(jsAutoFillExtension_, nullptr);
    ASSERT_NE(applicationContext_, nullptr);

    auto context = jsAutoFillExtension_->GetContext();
    ASSERT_NE(context, nullptr);
    auto configuration = context->GetConfiguration();
    EXPECT_NE(configuration, nullptr);
    auto appConfig = applicationContext_->GetConfiguration();
    EXPECT_NE(appConfig, nullptr);
}

/**
 * @tc.name: OnStart_0100
 * @tc.desc: Js auto fill extension OnStart.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, OnStart_0100, TestSize.Level1)
{
    Want want;
    ASSERT_NE(jsAutoFillExtension_, nullptr);
    sptr<AAFwk::SessionInfo> sessionInfo = new (std::nothrow) AAFwk::SessionInfo();
    ASSERT_NE(sessionInfo, nullptr);
    jsAutoFillExtension_->OnStart(want, sessionInfo);

    auto context = jsAutoFillExtension_->GetContext();
    ASSERT_NE(context, nullptr);
    auto configuration = context->GetConfiguration();
    EXPECT_NE(configuration, nullptr);
    auto resourceManager = context->GetResourceManager();
    EXPECT_NE(resourceManager, nullptr);
    auto appConfig = applicationContext_->GetConfiguration();
    EXPECT_NE(appConfig, nullptr);
}

/**
 * @tc.name: OnReloadInModal_0100
 * @tc.desc: Js auto fill extension OnReloadInModal.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, OnReloadInModal_0100, TestSize.Level1)
{
    CustomData customData;
    ASSERT_NE(jsAutoFillExtension_, nullptr);
    jsAutoFillExtension_->isPopup_ = true;
    auto ret = jsAutoFillExtension_->OnReloadInModal(nullptr, customData);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);

    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    ret = jsAutoFillExtension_->OnReloadInModal(sessionInfo, customData);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);
}

/**
 * @tc.name: OnIsContextType_0100
 * @tc.desc: Test IsContextType with matching context type.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, OnIsContextType_0100, TestSize.Level1)
{
    auto context = jsAutoFillExtension_->GetContext();
    ASSERT_NE(context, nullptr);

    auto autoFillContext = std::static_pointer_cast<AutoFillExtensionContext>(context);
    ASSERT_NE(autoFillContext, nullptr);

    JsAutoFillExtensionContext jsContext(autoFillContext);

    napi_env env = reinterpret_cast<napi_env>(jsRuntime_.get());
    NapiCallbackInfo info;
    napi_value argv[1];
    napi_status status = napi_create_string_utf8(env, "AutoFillExtensionContext",
        NAPI_AUTO_LENGTH, &argv[0]);
    EXPECT_EQ(status, napi_ok);
    info.argc = 1;
    info.argv = argv;

    auto result = jsContext.OnIsContextType(env, info);
    EXPECT_NE(result, nullptr);

    bool boolResult = false;
    status = napi_get_value_bool(env, result, &boolResult);
    EXPECT_EQ(status, napi_ok);
    EXPECT_TRUE(boolResult);
}

/**
 * @tc.name: OnIsContextType_0200
 * @tc.desc: Test IsContextType with non-matching context type.
 * @tc.type: FUNC
 */
HWTEST_F(JsAutoFillExtensionTest, OnIsContextType_0200, TestSize.Level1)
{
    auto context = jsAutoFillExtension_->GetContext();
    ASSERT_NE(context, nullptr);

    auto autoFillContext = std::static_pointer_cast<AutoFillExtensionContext>(context);
    ASSERT_NE(autoFillContext, nullptr);

    JsAutoFillExtensionContext jsContext(autoFillContext);

    napi_env env = reinterpret_cast<napi_env>(jsRuntime_.get());
    NapiCallbackInfo info;
    napi_value argv[1];
    napi_status status = napi_create_string_utf8(env, "OtherContext",
        NAPI_AUTO_LENGTH, &argv[0]);
    EXPECT_EQ(status, napi_ok);
    info.argc = 1;
    info.argv = argv;

    auto result = jsContext.OnIsContextType(env, info);
    EXPECT_NE(result, nullptr);

    bool boolResult = false;
    status = napi_get_value_bool(env, result, &boolResult);
    EXPECT_EQ(status, napi_ok);
    EXPECT_FALSE(boolResult);
}
} // namespace AbilityRuntime
} // namespace OHOS
