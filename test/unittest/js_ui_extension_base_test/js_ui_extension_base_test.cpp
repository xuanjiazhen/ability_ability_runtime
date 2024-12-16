/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>

#define private public
#define protected public
#include "js_ui_extension_base.h"
#include "js_environment.h"
#undef private
#undef protected
#include "ark_native_engine.h"
#include "mock_iremote_object.h"
#include "context.h"
#include "context_impl.h"
#include "napi_common_util.h"
#include "ability_handler.h"
#include "js_runtime.h"
#include "js_runtime_lite.h"
#include "ability_local_record.h"
#include "mock_ability_token.h"
#include "ohos_application.h"
#include "window.h"
#include "window_option.h"
#include "want_params.h"
#include "mock_runtime.h"
#include "element_name.h"
#include "operation.h"

#include <memory>

using namespace testing;
using namespace testing::ext;
using EventRunner = OHOS::AppExecFwk::EventRunner;

namespace OHOS {
namespace AbilityRuntime {
namespace {
    const std::string TEST_MODULENAME = "com.example.moduleName";
    const std::string TEST_SRCENTRANCE = "/test/src.js";
    const std::string TEST_HAPPATH = "com.example.hapPath";
    const std::string TEST_NAME = "my_test";
    const std::string TEST_BUNDLENAME = "com.example.bundlename";
    const std::string TEST_DEVICEID = "deviceId";
    const std::string TEST_ABILITYNAME = "com.example.abilityname";
}

napi_env env_ = nullptr;
panda::ecmascript::EcmaVM* vm_ = nullptr;

class MockDeferred : public NativeDeferred {
public:
    void Resolve(napi_value data) override
    {
        resolved_ = true;
        if (nref_ != nullptr) {
            napi_delete_reference(env_, nref_);
            nref_ = nullptr;
        }
        if (data != nullptr) {
            napi_create_reference(env_, data, 1, &nref_);
        }
    }

    void Reject(napi_value reason) override
    {
        resolved_ = false;
    }

public:
    static bool GetLastResolveStatus() { return resolved_; }
    static napi_ref GetLastResolveValue() { return nref_; }
    static void Clear()
    {
        if (nref_) {
            delete (reinterpret_cast<NativeReference*>(nref_));
            nref_ = nullptr;
        }
    }
    static bool resolved_;
    static napi_ref nref_;
};

bool MockDeferred::resolved_ = false;
napi_ref MockDeferred::nref_ = nullptr;

class MockArkNativeEngine : public ArkNativeEngine {
public:
    MockArkNativeEngine(EcmaVM* vm, void* jsEngine, bool isLimitedWorker = false)
        :ArkNativeEngine(vm, jsEngine, isLimitedWorker) {}

    napi_value CreatePromise(NativeDeferred** deferred) override
    {
        napi_value ret = ArkNativeEngine::CreatePromise(deferred);
        *deferred = new (std::nothrow) MockDeferred;
        return ret;
    }
};

class NativeReferenceMock : public NativeReference {
public:
    NativeReferenceMock() = default;
    virtual ~NativeReferenceMock() = default;
    MOCK_METHOD0(Ref, uint32_t());
    MOCK_METHOD0(Unref, uint32_t());
    MOCK_METHOD0(Get, napi_value());
    MOCK_METHOD0(GetData, void*());
    virtual operator napi_value() override
    {
        return reinterpret_cast<napi_value>(this);
    }
    MOCK_METHOD0(SetDeleteSelf, void());
    MOCK_METHOD0(GetRefCount, uint32_t());
    MOCK_METHOD0(GetFinalRun, bool());

    NativeEngine* create_engine()
    {
        void* jsEngine = new int(1);
        panda::RuntimeOption option;
        option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
        const int64_t poolSize = 0x1000000;
        option.SetGcPoolSize(poolSize);
        option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
        option.SetDebuggerLibraryPath("");
        panda::ecmascript::EcmaVM* vm = panda::JSNApi::CreateJSVM(option);
        NativeEngine *engine = new MockArkNativeEngine(vm, jsEngine);
        return engine;
    }

    napi_env create_env()
    {
        NativeEngine* engine = NativeReferenceMock().create_engine();
        return reinterpret_cast<napi_env>(engine);
    }
    napi_value GetNapiValue() override
    {
        napi_env env = create_env();
        napi_value object = AppExecFwk::CreateJSObject(env);
        return object;
    }
};

class JsUiExtensionBaseTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void JsUiExtensionBaseTest::SetUpTestCase()
{}
void JsUiExtensionBaseTest::TearDownTestCase()
{}
void JsUiExtensionBaseTest::SetUp()
{}
void JsUiExtensionBaseTest::TearDown()
{}

/**
 * @tc.name: Init_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, Init_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Init_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    auto abilityinfo = std::make_shared<AppExecFwk::AbilityInfo>();
    auto want = std::make_shared<AAFwk::Want>();
    int32_t abilityRecordId = 1;
    sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    auto abilityrecord = std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityinfo, token, want, abilityRecordId);
    auto application = std::make_shared<OHOS::AppExecFwk::OHOSApplication>();
    std::string str = "Newthread";
    auto runner = AppExecFwk::EventRunner::Create(str);
    auto handler = std::make_shared<AppExecFwk::AbilityHandler>(runner);
    sptr<IRemoteObject> tokenn = sptr<MockIRemoteObject>::MakeSptr();
    jueb_ptr->abilityInfo_ = std::make_shared<AppExecFwk::AbilityInfo>();
    jueb_ptr->abilityInfo_->srcEntrance = TEST_SRCENTRANCE;
    jueb_ptr->abilityInfo_->moduleName = TEST_MODULENAME;
    jueb_ptr->abilityInfo_->name = TEST_NAME;
    jueb_ptr->abilityInfo_->hapPath = TEST_HAPPATH;
    jueb_ptr->abilityInfo_->compileMode = CompileMode::ES_MODULE;
    auto test = jueb_ptr->Init(abilityrecord, application, handler, tokenn);
    EXPECT_EQ(jueb_ptr->token_, abilityrecord->token_);
    EXPECT_EQ(test, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "Init_0100 end");
}

/**
 * @tc.name: BindContext_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, BindContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BindContext_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    jueb_ptr->jsObj_ = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    jueb_ptr->BindContext();
    EXPECT_NE(jueb_ptr->shellContextRef_, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "BindContext_0100 end");
}

/**
 * @tc.name: OnStart_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnStart_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStart_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    want.parameters_.params_.emplace("ohos.insightIntent.executeParam.name", nullptr);
    AAFwk::LaunchParam launchParam;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    jueb_ptr->jsObj_ = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    jueb_ptr->context_->config_ = std::make_shared<AppExecFwk::Configuration>();
    jueb_ptr->shellContextRef_ = std::make_shared<NativeReferenceMock>();
    want.operation_.abilityName_ = TEST_NAME;
    want.operation_.bundleName_ = TEST_BUNDLENAME;
    want.operation_.moduleName_ = TEST_MODULENAME;
    want.operation_.deviceId_ = TEST_DEVICEID;
    jueb_ptr->OnStart(want, launchParam, sessionInfo);
    EXPECT_EQ(launchParam.launchReason, AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT);

    TAG_LOGI(AAFwkTag::TEST, "OnStart_0100 end");
}

/**
 * @tc.name: OnStop_2_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnStop_2_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStop_2_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo = nullptr;
    bool isAsyncCallback = true;
    jueb_ptr->jsObj_ = nullptr;
    jueb_ptr->OnStop(callbackInfo, isAsyncCallback);
    EXPECT_EQ(isAsyncCallback, false);

    TAG_LOGI(AAFwkTag::TEST, "OnStop_2_0100 end");
}

/**
 * @tc.name: OnStop_2_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnStop_2_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStop_2_0200 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<>::Create();
    bool isAsyncCallback = true;
    jueb_ptr->jsObj_ = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    jueb_ptr->OnStop(callbackInfo, isAsyncCallback);
    EXPECT_EQ(isAsyncCallback, false);

    TAG_LOGI(AAFwkTag::TEST, "OnStop_2_0200 end");
}

/**
 * @tc.name: OnStopCallBack_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnStopCallBack_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStopCallBack_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    jueb_ptr->context_->token_ = sptr<MockIRemoteObject>(new MockIRemoteObject);
    jueb_ptr->context_->applicationContext_ = std::make_shared<ApplicationContext>();
    jueb_ptr->jsObj_ = std::make_shared<NativeReferenceMock>();
    auto conImpl = std::make_shared<AbilityRuntime::ContextImpl>();
    sptr<IRemoteObject> token = new AppExecFwk::MockAbilityToken();
    conImpl->SetToken(token);
    jueb_ptr->jsObj_ = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();

    napi_env env = jueb_ptr->jsRuntime_.GetNapiEnv();
    auto jsablcb = std::make_shared<JsAbilityLifecycleCallback>(env);
    jueb_ptr->context_->applicationContext_->callbacks_.push_back(jsablcb);
    jueb_ptr->OnStopCallBack();
    EXPECT_EQ(jueb_ptr->context_->applicationContext_->callbacks_.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "OnStopCallBack_0100 end");
}

/**
 * @tc.name: CheckPromise_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CheckPromise_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPromise_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    napi_value result = jueb_ptr->CallObjectMethod("onDestroy", nullptr, 0, true);
    jueb_ptr->jsObj_ = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    bool test = jueb_ptr->CheckPromise(result);
    EXPECT_EQ(test, false);

    TAG_LOGI(AAFwkTag::TEST, "CheckPromise_0100 end");
}

/**
 * @tc.name: CheckPromise_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CheckPromise_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPromise_0200 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    napi_value result =  jueb_ptr->CallObjectMethod("onDestroy", nullptr, 0, true);
    bool promise = jueb_ptr->CheckPromise(result);
    EXPECT_EQ(promise, false);

    TAG_LOGI(AAFwkTag::TEST, "CheckPromise_0200 end");
}

/**
 * @tc.name: CheckPromise_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CheckPromise_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckPromise_0300 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    napi_value result =  nullptr;
    bool promise = jueb_ptr->CheckPromise(result);
    EXPECT_EQ(promise, false);

    TAG_LOGI(AAFwkTag::TEST, "CheckPromise_0300 end");
}

/**
 * @tc.name: CallPromise_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CallPromise_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CallPromise_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    napi_value result =  jueb_ptr->CallObjectMethod("TEST", nullptr, 0, true);
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<>::Create();
    jueb_ptr->jsObj_ = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    auto test = jueb_ptr->CallPromise(result, callbackInfo);
    EXPECT_EQ(test, false);

    TAG_LOGI(AAFwkTag::TEST, "CallPromise_0100 end");
}

/**
 * @tc.name: OnCommandWindow_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindow_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindow_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand wincmd = AAFwk::WIN_CMD_FOREGROUND;
    auto windowoption = sptr<Rosen::WindowOption>::MakeSptr();
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    jueb_ptr->context_->applicationInfo_ = std::make_shared<AppExecFwk::ApplicationInfo>();
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    EXPECT_NE(uiwindow, nullptr);
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    EXPECT_NE(jueb_ptr->uiWindowMap_.size(), 0);
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    jueb_ptr->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();
    jueb_ptr->OnCommandWindow(want, sessionInfo, AAFwk::WIN_CMD_FOREGROUND);
    EXPECT_EQ(jueb_ptr->foregroundWindows_.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindow_0100 end");
}

/**
 * @tc.name: OnCommandWindow_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindow_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindow_0200 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand wincmd = AAFwk::WIN_CMD_BACKGROUND;
    auto option = sptr<Rosen::WindowOption>::MakeSptr();
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jueb_ptr->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jueb_ptr->OnCommandWindow(want, sessionInfo, wincmd);
    EXPECT_EQ(jueb_ptr->foregroundWindows_.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindow_0200 end");
}

/**
 * @tc.name: OnCommandWindow_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindow_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindow_0300 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand wincmd = AAFwk::WIN_CMD_DESTROY;
    auto option = sptr<Rosen::WindowOption>::MakeSptr();
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    auto ptr = std::make_shared<NativeReferenceMock>();
    jueb_ptr->contentSessions_.emplace(sessionInfo->uiExtensionComponentId, ptr);
    jueb_ptr->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jueb_ptr->abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
    auto listener = std::make_shared<UISessionAbilityResultListener>();
    jueb_ptr->abilityResultListeners_->listeners_.emplace(sessionInfo->uiExtensionComponentId, listener);
    jueb_ptr->OnCommandWindow(want, sessionInfo, wincmd);
    EXPECT_EQ(jueb_ptr->foregroundWindows_.size(), 0);
    EXPECT_EQ(jueb_ptr->uiWindowMap_.size(), 0);
    EXPECT_EQ(jueb_ptr->contentSessions_.size(), 0);
    EXPECT_EQ(jueb_ptr->abilityResultListeners_->listeners_.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindow_0300 end");
}


/**
 * @tc.name: ForegroundWindowWithInsightIntent_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, ForegroundWindowWithInsightIntent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ForegroundWindowWithInsightIntent_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    bool needForeground = false;
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    jueb_ptr->context_->abilityInfo_ = std::make_shared<AppExecFwk::AbilityInfo>();
    jueb_ptr->context_->abilityInfo_->hapPath = TEST_HAPPATH;
    jueb_ptr->context_->abilityInfo_->compileMode = AppExecFwk::CompileMode::ES_MODULE;
    jueb_ptr->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();
    auto ptr = std::make_shared<NativeReferenceMock>();
    jueb_ptr->contentSessions_.emplace(sessionInfo->uiExtensionComponentId, ptr);
    bool test = jueb_ptr->ForegroundWindowWithInsightIntent(want, sessionInfo, needForeground);
    EXPECT_EQ(test, true);

    TAG_LOGI(AAFwkTag::TEST, "ForegroundWindowWithInsightIntent_0100 end");
}

/**
 * @tc.name: ForegroundWindowWithInsightIntent_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, ForegroundWindowWithInsightIntent_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ForegroundWindowWithInsightIntent_0200 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    bool needForeground = false;
    bool test = jueb_ptr->ForegroundWindowWithInsightIntent(want, sessionInfo, needForeground);
    EXPECT_EQ(test, false);

    TAG_LOGI(AAFwkTag::TEST, "ForegroundWindowWithInsightIntent_0200 end");
}

/**
 * @tc.name: PostInsightIntentExecuted_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, PostInsightIntentExecuted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PostInsightIntentExecuted_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    AppExecFwk::InsightIntentExecuteResult result;
    result.result = std::shared_ptr<AAFwk::WantParams>();
    result.code = 1;
    bool needForeground = false;
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jueb_ptr->PostInsightIntentExecuted(sessionInfo, result, needForeground);
    EXPECT_EQ(jueb_ptr->foregroundWindows_.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "PostInsightIntentExecuted_0100 end");
}

/**
 * @tc.name: OnCommandWindowDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindowDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindowDone_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand winCmd = WIN_CMD_FOREGROUND;
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jueb_ptr->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jueb_ptr->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();
    jueb_ptr->OnCommandWindowDone(sessionInfo, winCmd);
    EXPECT_NE(jueb_ptr, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindowDone_0100 end");
}

/**
 * @tc.name: OnCommandWindowDone_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindowDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindowDone_0200 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand winCmd = WIN_CMD_FOREGROUND;
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    jueb_ptr->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();
    jueb_ptr->OnCommandWindowDone(sessionInfo, winCmd);
    EXPECT_NE(jueb_ptr, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindowDone_0200 end");
}

/**
 * @tc.name: OnCommandWindowDone_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindowDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindowDone_0300 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand winCmd = WIN_CMD_FOREGROUND;
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jueb_ptr->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();
    jueb_ptr->OnCommandWindowDone(sessionInfo, winCmd);
    EXPECT_NE(jueb_ptr, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindowDone_0300 end");
}

/**
 * @tc.name: OnInsightIntentExecuteDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnInsightIntentExecuteDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnInsightIntentExecuteDone_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    sessionInfo->sessionToken = sptr<MockIRemoteObject>::MakeSptr();
    AppExecFwk::InsightIntentExecuteResult result;
    result.result = std::make_shared<WantParams>();
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jueb_ptr->OnInsightIntentExecuteDone(sessionInfo, result);
    EXPECT_EQ(jueb_ptr->foregroundWindows_.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "OnInsightIntentExecuteDone_0100 end");
}

/**
 * @tc.name: OnInsightIntentExecuteDone_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnInsightIntentExecuteDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnInsightIntentExecuteDone_0200 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    AppExecFwk::InsightIntentExecuteResult result;
    jueb_ptr->OnInsightIntentExecuteDone(sessionInfo, result);
    EXPECT_EQ(jueb_ptr->foregroundWindows_.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "OnInsightIntentExecuteDone_0200 end");
}

/**
 * @tc.name: OnCommand_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommand_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommand_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    bool restart = false;
    int32_t startId = 1;
    jueb_ptr->jsObj_ = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    want.operation_.abilityName_ = TEST_ABILITYNAME;
    want.operation_.moduleName_ = TEST_MODULENAME;
    want.operation_.deviceId_ = TEST_DEVICEID;
    want.operation_.bundleName_ = TEST_BUNDLENAME;
    jueb_ptr->OnCommand(want, restart, startId);

    TAG_LOGI(AAFwkTag::TEST, "OnCommand_0100 end");
}

/**
 * @tc.name: OnForeground_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnForeground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnForeground_0100 start");

    std::unique_ptr<JsRuntime> rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    want.parameters_.params_.emplace(INSIGHT_INTENT_EXECUTE_PARAM_NAME, nullptr);
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jueb_ptr->abilityInfo_ = std::make_shared<AbilityInfo>();
    jueb_ptr->abilityInfo_->hapPath = TEST_HAPPATH;
    jueb_ptr->abilityInfo_->compileMode = AppExecFwk::CompileMode::ES_MODULE;
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    jueb_ptr->context_->token_ = new MockIRemoteObject();
    auto ptr = std::make_shared<NativeReferenceMock>();
    jueb_ptr->contentSessions_.emplace(sessionInfo->uiExtensionComponentId, ptr);
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jueb_ptr->OnForeground(want, sessionInfo);
    EXPECT_EQ(jueb_ptr->foregroundWindows_.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "OnForeground_0100 end");
}

/**
 * @tc.name: CallJsOnSessionCreate_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CallJsOnSessionCreate_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CallJsOnSessionCreate_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    jueb_ptr->jsObj_ = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    uint32_t uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    want.operation_.SetModuleName(TEST_MODULENAME);
    want.operation_.SetBundleName(TEST_BUNDLENAME);
    want.operation_.SetAbilityName(TEST_ABILITYNAME);
    want.operation_.SetDeviceId(TEST_DEVICEID);
    jueb_ptr->abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    bool test = jueb_ptr->CallJsOnSessionCreate(want, sessionInfo, uiwindow, uiExtensionComponentId);
    EXPECT_EQ(jueb_ptr->contentSessions_.size(), 1);
    EXPECT_EQ(test, true);

    TAG_LOGI(AAFwkTag::TEST, "CallJsOnSessionCreate_0100 end");
}

/**
 * @tc.name:  CallJsOnSessionCreate_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CallJsOnSessionCreate_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CallJsOnSessionCreate_0200 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    uint32_t uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    bool test =  jueb_ptr->CallJsOnSessionCreate(want, sessionInfo, uiwindow, uiExtensionComponentId);
    EXPECT_EQ(test, false);

    TAG_LOGI(AAFwkTag::TEST, "CallJsOnSessionCreate_0200 end");
}

/**
 * @tc.name:  CreateWindowOption_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CreateWindowOption_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateWindowOption_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->hostWindowId = 1;
    sessionInfo->realHostWindowId = 1;
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    jueb_ptr->context_->applicationInfo_ = std::make_shared<ApplicationInfo>();
    jueb_ptr->context_->applicationInfo_->bundleName = TEST_BUNDLENAME;
    jueb_ptr->context_->abilityInfo_ = std::make_shared<AbilityInfo>();
    jueb_ptr->context_->abilityInfo_->name = TEST_NAME;
    auto test = jueb_ptr->CreateWindowOption(sessionInfo);
    EXPECT_NE(test, nullptr);

    TAG_LOGI(AAFwkTag::TEST, "CreateWindowOption_0100 end");
}

/**
 * @tc.name:  HandleSessionCreate_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, HandleSessionCreate_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleSessionCreate_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    bool test = jueb_ptr->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, true);

    TAG_LOGI(AAFwkTag::TEST, "HandleSessionCreate_0100 end");
}

/**
 * @tc.name:  HandleSessionCreate_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, HandleSessionCreate_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleSessionCreate_0200 start");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    bool test = jueb_ptr->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, false);

    TAG_LOGI(AAFwkTag::TEST, "HandleSessionCreate_0200 end");
}

/**
 * @tc.name:  HandleSessionCreate_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, HandleSessionCreate_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleSessionCreate_0300 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = new AAFwk::SessionInfo();
    sessionInfo->uiExtensionComponentId = 1;
    sessionInfo->sessionToken = new MockIRemoteObject();
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    jueb_ptr->abilityInfo_ = std::make_shared<AppExecFwk::AbilityInfo>();
    jueb_ptr->context_ = std::make_shared<UIExtensionContext>();
    jueb_ptr->context_->applicationInfo_ = std::make_shared<ApplicationInfo>();
    jueb_ptr->context_->applicationInfo_->bundleName = TEST_BUNDLENAME;
    jueb_ptr->context_->abilityInfo_ = std::make_shared<AbilityInfo>();
    jueb_ptr->context_->abilityInfo_->name = TEST_NAME;
    jueb_ptr->jsObj_ = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jueb_ptr->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    want.operation_.moduleName_ = TEST_MODULENAME;
    want.operation_.deviceId_ = TEST_DEVICEID;
    want.operation_.bundleName_ = TEST_BUNDLENAME;
    want.operation_.abilityName_ = TEST_ABILITYNAME;
    bool test = jueb_ptr->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, true);

    TAG_LOGI(AAFwkTag::TEST, "HandleSessionCreate_0300 end");
}

/**
 * @tc.name:  ForegroundWindow_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, ForegroundWindow_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ForegroundWindow_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    AAFwk::Want want;
    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jueb_ptr->ForegroundWindow(want, sessionInfo);
    EXPECT_EQ(jueb_ptr->foregroundWindows_.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "ForegroundWindow_0100 end");
}

/**
 * @tc.name:  BackgroundWindow_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, BackgroundWindow_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BackgroundWindow_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jueb_ptr->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jueb_ptr->BackgroundWindow(sessionInfo);
    EXPECT_EQ(jueb_ptr->foregroundWindows_.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "BackgroundWindow_0100 end");
}

/**
 * @tc.name:  DestroyWindow_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, DestroyWindow_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DestroyWindow_0100 start");

    auto rt_ptr =  std::make_unique<JsRuntime>();
    auto jueb_ptr = std::make_shared<JsUIExtensionBase>(std::move(rt_ptr));
    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = new Rosen::Window();
    jueb_ptr->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    auto nativereferece = std::make_shared<NativeReferenceMock>();
    jueb_ptr->contentSessions_.emplace(sessionInfo->uiExtensionComponentId, nativereferece);
    jueb_ptr->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jueb_ptr->abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
    auto abilityResultListener = std::make_shared<UISessionAbilityResultListener>();
    jueb_ptr->abilityResultListeners_->listeners_.emplace(sessionInfo->uiExtensionComponentId, abilityResultListener);
    jueb_ptr->DestroyWindow(sessionInfo);
    EXPECT_EQ(jueb_ptr->uiWindowMap_.size(), 0);
    EXPECT_EQ(jueb_ptr->contentSessions_.size(), 0);
    EXPECT_EQ(jueb_ptr->foregroundWindows_.size(), 0);
    EXPECT_EQ(jueb_ptr->abilityResultListeners_->listeners_.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "DestroyWindow_0100 end");
}

} // namespace AbilityRuntime
} // namespace OHOS