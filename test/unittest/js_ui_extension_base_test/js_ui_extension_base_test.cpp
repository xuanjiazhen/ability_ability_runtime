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
#include "js_environment.h"
#include "js_ui_extension_base.h"
#undef private
#undef protected

#include "ability_handler.h"
#include "ability_local_record.h"
#include "ark_native_engine.h"
#include "context.h"
#include "context_impl.h"
#include "element_name.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_lite.h"
#include "js_ui_extension_context.h"
#include "mock_ability_token.h"
#include "mock_iremote_object.h"
#include "mock_runtime.h"
#include "napi_common_util.h"
#include "ohos_application.h"
#include "operation.h"
#include "window.h"
#include "window_option.h"
#include "want_params.h"

#include <memory>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
    const std::string TEST_MODULENAME = "com.example.moduleName";
    const std::string TEST_SRCENTRANCE = "com.example.srcentrance.js";
    const std::string TEST_HAPPATH = "com.example.hapPath";
    const std::string TEST_NAME = "com.example.name";
    const std::string TEST_BUNDLENAME = "com.example.bundlename";
    const std::string TEST_DEVICEID = "com.example.deviceId";
    const std::string TEST_ABILITYNAME = "com.example.abilityname";
}

napi_env g_env = nullptr;
panda::ecmascript::EcmaVM* g_vm = nullptr;

class MockDeferred : public NativeDeferred {
public:
    void Resolve(napi_value data) override
    {
        resolved_ = true;
        if (nref_ != nullptr) {
            napi_delete_reference(g_env, nref_);
            nref_ = nullptr;
        }
        if (data != nullptr) {
            napi_create_reference(g_env, data, 1, &nref_);
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
    operator napi_value() override
    {
        return reinterpret_cast<napi_value>(this);
    }

    MOCK_METHOD0(SetDeleteSelf, void());
    MOCK_METHOD0(GetRefCount, uint32_t());
    MOCK_METHOD0(GetFinalRun, bool());

    napi_value GetNapiValue() override
    {
        napi_env env = nullptr;
        napi_value object = AppExecFwk::CreateJSObject(env);
        return object;
    }
};

class JsUiExtensionBaseTest : public testing::Test {
public:
    JsUiExtensionBaseTest();
    ~JsUiExtensionBaseTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<AbilityRuntime::JsUIExtensionBase> jsUiExtensionBase = nullptr;
};

JsUiExtensionBaseTest::JsUiExtensionBaseTest()
{
}
JsUiExtensionBaseTest::~JsUiExtensionBaseTest()
{
}
void JsUiExtensionBaseTest::SetUpTestCase()
{
    panda::RuntimeOption pandaOption;
    g_vm = panda::JSNApi::CreateJSVM(pandaOption);
    if (g_vm == nullptr) {
        TAG_LOGE(AAFwkTag::TEST, "Create vm failed.");
        return;
    }

    g_env = reinterpret_cast<napi_env>(new ArkNativeEngine(g_vm, nullptr));
}

void JsUiExtensionBaseTest::TearDownTestCase()
{
    if (g_env != nullptr) {
        delete reinterpret_cast<NativeEngine*>(g_env);
        g_env = nullptr;
    }

    if (g_vm != nullptr) {
        panda::JSNApi::DestroyJSVM(g_vm);
        g_vm = nullptr;
    }
}

void JsUiExtensionBaseTest::SetUp()
{
    auto runtime = std::make_unique<AbilityRuntime::JsRuntime>();
    jsUiExtensionBase = std::make_shared<AbilityRuntime::JsUIExtensionBase>(std::move(runtime));
}

void JsUiExtensionBaseTest::TearDown()
{
    jsUiExtensionBase.reset();
}

/**
 * @tc.name: Init_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, Init_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "Init_0100 start.");

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityinfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_TRUE(abilityinfo != nullptr);
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    ASSERT_TRUE(want != nullptr);
    int32_t abilityRecordId = 1;
    sptr<IRemoteObject> token1 = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_TRUE(token1 != nullptr);
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> abilityrecord =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityinfo, token1, want, abilityRecordId);
    ASSERT_TRUE(abilityrecord != nullptr);
    std::shared_ptr<AppExecFwk::OHOSApplication> application =
        std::make_shared<AppExecFwk::OHOSApplication>();
    ASSERT_TRUE(application != nullptr);
    std::shared_ptr<AppExecFwk::AbilityHandler> handler = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    ASSERT_TRUE(handler != nullptr);
    sptr<IRemoteObject> token2 = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_TRUE(token2 != nullptr);

    jsUiExtensionBase->abilityInfo_ = abilityinfo;
    jsUiExtensionBase->abilityInfo_->moduleName = TEST_MODULENAME;
    jsUiExtensionBase->abilityInfo_->name = TEST_NAME;
    jsUiExtensionBase->abilityInfo_->srcEntrance = TEST_SRCENTRANCE;
    jsUiExtensionBase->abilityInfo_->hapPath = TEST_HAPPATH;

    std::shared_ptr<NativeReferenceMock> nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();

    jsUiExtensionBase->jsRuntime_.jsEnv_->vm_ = g_vm;
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);
    jsUiExtensionBase->jsRuntime_.modules_.emplace("com.example.moduleName/com.example.srcentrance.abc",
        nativeRefenceMock.get());

    jsUiExtensionBase->Init(abilityrecord, application, handler, token2);
    EXPECT_EQ(jsUiExtensionBase->token_, abilityrecord->token_);
    EXPECT_EQ(jsUiExtensionBase->jsRuntime_.moduleName_, jsUiExtensionBase->abilityInfo_->moduleName);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "Init_0100 end.");
}

/**
 * @tc.name: BindContext_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, BindContext_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "BindContext_0100 start.");

    jsUiExtensionBase->jsObj_ = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->BindContext();
    EXPECT_EQ(jsUiExtensionBase->shellContextRef_, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "BindContext_0100 end.");
}

/**
 * @tc.name: OnStart_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnStart_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnStart_0100 start.");

    AAFwk::Want want;
    want.parameters_.params_.emplace("ohos.insightIntent.executeParam.name", nullptr);
    AAFwk::LaunchParam launchParam;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();

    jsUiExtensionBase->jsObj_ = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->config_ = std::make_shared<AppExecFwk::Configuration>();
    jsUiExtensionBase->shellContextRef_ = std::make_shared<NativeReferenceMock>();

    want.operation_.abilityName_ = TEST_NAME;
    want.operation_.bundleName_ = TEST_BUNDLENAME;
    want.operation_.moduleName_ = TEST_MODULENAME;
    want.operation_.deviceId_ = TEST_DEVICEID;

    jsUiExtensionBase->OnStart(want, launchParam, sessionInfo);
    EXPECT_EQ(launchParam.launchReason, 9);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnStart_0100 end.");
}

/**
 * @tc.name: OnStart_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
#ifdef SUPPORT_GRAPHICS
HWTEST_F(JsUiExtensionBaseTest, OnStart_0200, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnStart_0200 start.");

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->displayId = 1;
    sessionInfo->density = 2.0;
    sessionInfo->orientation = 1;

    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->config_ = std::make_shared<AppExecFwk::Configuration>();
    std::shared_ptr<Global::Resource::ResourceManager>
        resourceManager(Global::Resource::CreateResourceManager());
    jsUiExtensionBase->context_->resourceManager_ = resourceManager;
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);
    jsUiExtensionBase->shellContextRef_ = std::make_shared<NativeReferenceMock>();

    AAFwk::Want want;
    AAFwk::LaunchParam launchParam;
    want.parameters_.params_.emplace("ohos.insightIntent.executeParam.name", nullptr);
    want.operation_.abilityName_ = TEST_NAME;
    want.operation_.bundleName_ = TEST_BUNDLENAME;
    want.operation_.moduleName_ = TEST_MODULENAME;
    want.operation_.deviceId_ = TEST_DEVICEID;

    jsUiExtensionBase->OnStart(want, launchParam, sessionInfo);
    EXPECT_EQ(launchParam.launchReason, 9);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnStart_0200 start.");
}
#endif

/**
 * @tc.name: OnStop_2_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnStop_2_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnStop_2_0100 start.");

    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo = nullptr;
    bool isAsyncCallback = true;
    jsUiExtensionBase->jsObj_ = nullptr;
    jsUiExtensionBase->OnStop(callbackInfo, isAsyncCallback);
    EXPECT_EQ(isAsyncCallback, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnStop_2_0100 end.");
}

/**
 * @tc.name: OnStop_2_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnStop_2_0200, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnStop_2_0200 start.");

    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo =
        AppExecFwk::AbilityTransactionCallbackInfo<>::Create();
    bool isAsyncCallback = true;

    jsUiExtensionBase->jsObj_ = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);
    jsUiExtensionBase->OnStop(callbackInfo, isAsyncCallback);

    EXPECT_EQ(isAsyncCallback, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnStop_2_0200 end.");
}

/**
 * @tc.name: OnStopCallBack_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnStopCallBack_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnStopCallBack_0100 start.");

    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();
    jsUiExtensionBase->context_->applicationContext_ = std::make_shared<ApplicationContext>();
    jsUiExtensionBase->jsObj_ = std::make_shared<NativeReferenceMock>();

    std::shared_ptr<AbilityRuntime::ContextImpl> conImpl =
        std::make_shared<AbilityRuntime::ContextImpl>();
    sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    conImpl->SetToken(token);

    jsUiExtensionBase->jsObj_ = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);

    napi_env env = jsUiExtensionBase->jsRuntime_.GetNapiEnv();
    std::shared_ptr<AbilityRuntime::JsAbilityLifecycleCallback> jsablcb =
        std::make_shared<AbilityRuntime::JsAbilityLifecycleCallback>(env);
    jsUiExtensionBase->context_->applicationContext_->callbacks_.push_back(jsablcb);

    jsUiExtensionBase->OnStopCallBack();
    EXPECT_EQ(jsUiExtensionBase->context_->applicationContext_->callbacks_.size(), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnStopCallBack_0100 end.");
}


/**
 * @tc.name: CheckPromise_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CheckPromise_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "CheckPromise_0100 start.");

    napi_value result = jsUiExtensionBase->CallObjectMethod("onCreate", nullptr, 0, true);
    bool test = jsUiExtensionBase->CheckPromise(result);
    EXPECT_EQ(test, false);

    TAG_LOGE(AAFwkTag::TEST, "CheckPromise_0100 end.");
}

/**
 * @tc.name: CheckPromise_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CheckPromise_0200, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "CheckPromise_0200 start.");

    std::unique_ptr<MockDeferred> deferred = std::make_unique<MockDeferred>();
    NativeDeferred* deferredPtr = deferred.get();
    std::shared_ptr<MockArkNativeEngine> mockArkNativeEngine =
        std::make_shared<MockArkNativeEngine>(g_vm, nullptr);
    ASSERT_NE(mockArkNativeEngine, nullptr);
    napi_value promise = mockArkNativeEngine->CreatePromise(&deferredPtr);
    EXPECT_NE(promise, nullptr);
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);

    bool test = jsUiExtensionBase->CheckPromise(promise);
    EXPECT_EQ(test, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "CheckPromise_0200 end.");
}

/**
 * @tc.name: CallPromise_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CallPromise_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "CallPromise_0100 start.");

    std::unique_ptr<MockDeferred> deferred = std::make_unique<MockDeferred>();
    NativeDeferred* deferredPtr = deferred.get();
    std::shared_ptr<MockArkNativeEngine> mockArkNativeEngine =
        std::make_shared<MockArkNativeEngine>(g_vm, nullptr);
    ASSERT_NE(mockArkNativeEngine, nullptr);
    napi_value promise = mockArkNativeEngine->CreatePromise(&deferredPtr);
    EXPECT_NE(promise, nullptr);
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo =
        AppExecFwk::AbilityTransactionCallbackInfo<>::Create();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);

    auto test = jsUiExtensionBase->CallPromise(promise, callbackInfo);
    EXPECT_EQ(test, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "CallPromise_0100 end.");
}

/**
 * @tc.name: OnCommandWindow_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindow_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindow_0100 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand wincmd = AAFwk::WIN_CMD_FOREGROUND;
    sptr<Rosen::WindowOption> windowoption = sptr<Rosen::WindowOption>::MakeSptr();
    EXPECT_TRUE(windowoption != nullptr);
    jsUiExtensionBase->context_ = std::make_shared<AbilityRuntime::UIExtensionContext>();
    jsUiExtensionBase->context_->applicationInfo_ = std::make_shared<AppExecFwk::ApplicationInfo>();

    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    EXPECT_NE(uiwindow, nullptr);
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    EXPECT_NE(jsUiExtensionBase->uiWindowMap_.size(), 0);
    jsUiExtensionBase->context_ = std::make_shared<AbilityRuntime::UIExtensionContext>();
    jsUiExtensionBase->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();

    jsUiExtensionBase->OnCommandWindow(want, sessionInfo, AAFwk::WIN_CMD_FOREGROUND);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindow_0100 end.");
}

/**
 * @tc.name: OnCommandWindow_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindow_0200, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindow_0200 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand wincmd = AAFwk::WIN_CMD_BACKGROUND;

    sptr<Rosen::WindowOption> option = sptr<Rosen::WindowOption>::MakeSptr();
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();

    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jsUiExtensionBase->OnCommandWindow(want, sessionInfo, wincmd);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindow_0200 end.");
}

/**
 * @tc.name: OnCommandWindow_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindow_0300, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindow_0300 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand wincmd = AAFwk::WIN_CMD_DESTROY;

    jsUiExtensionBase->abilityResultListeners_ = std::make_shared<AbilityRuntime::AbilityResultListeners>();
    std::shared_ptr<AbilityRuntime::UISessionAbilityResultListener> listener =
        std::make_shared<AbilityRuntime::UISessionAbilityResultListener>();
    std::shared_ptr<NativeReferenceMock> ptr = std::make_shared<NativeReferenceMock>();

    sptr<Rosen::WindowOption> option = sptr<Rosen::WindowOption>::MakeSptr();
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();

    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->contentSessions_.emplace(sessionInfo->uiExtensionComponentId, ptr);
    jsUiExtensionBase->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jsUiExtensionBase->abilityResultListeners_->listeners_.emplace(sessionInfo->uiExtensionComponentId, listener);

    jsUiExtensionBase->OnCommandWindow(want, sessionInfo, wincmd);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->uiWindowMap_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->contentSessions_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->abilityResultListeners_->listeners_.size(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindow_0300 end.");
}


/**
 * @tc.name: ForegroundWindowWithInsightIntent_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, ForegroundWindowWithInsightIntent_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "ForegroundWindowWithInsightIntent_0100 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    bool needForeground = false;

    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->abilityInfo_ = std::make_shared<AppExecFwk::AbilityInfo>();
    jsUiExtensionBase->context_->abilityInfo_->hapPath = TEST_HAPPATH;
    jsUiExtensionBase->context_->abilityInfo_->compileMode = AppExecFwk::CompileMode::ES_MODULE;
    jsUiExtensionBase->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();

    std::shared_ptr<NativeReferenceMock> ptr = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->contentSessions_.emplace(sessionInfo->uiExtensionComponentId, ptr);

    bool test = jsUiExtensionBase->ForegroundWindowWithInsightIntent(want, sessionInfo, needForeground);
    EXPECT_EQ(test, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "ForegroundWindowWithInsightIntent_0100 end.");
}

/**
 * @tc.name: ForegroundWindowWithInsightIntent_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, ForegroundWindowWithInsightIntent_0200, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "ForegroundWindowWithInsightIntent_0200 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    bool needForeground = false;

    bool test = jsUiExtensionBase->ForegroundWindowWithInsightIntent(want, sessionInfo, needForeground);
    EXPECT_EQ(test, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "ForegroundWindowWithInsightIntent_0200 end.");
}

/**
 * @tc.name: OnCommandWindowDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindowDone_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindowDone_0100 start.");

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand winCmd = WIN_CMD_FOREGROUND;
    jsUiExtensionBase->context_ = std::make_shared<AbilityRuntime::UIExtensionContext>();

    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);

    jsUiExtensionBase->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jsUiExtensionBase->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();
    jsUiExtensionBase->OnCommandWindowDone(sessionInfo, winCmd);
    EXPECT_NE(jsUiExtensionBase, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindowDone_0100 end.");
}

/**
 * @tc.name: OnCommandWindowDone_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindowDone_0200, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindowDone_0200 start.");

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand winCmd = WIN_CMD_FOREGROUND;

    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();

    jsUiExtensionBase->OnCommandWindowDone(sessionInfo, winCmd);
    EXPECT_NE(jsUiExtensionBase, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindowDone_0200 end.");
}

/**
 * @tc.name: OnCommandWindowDone_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommandWindowDone_0300, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindowDone_0300 start.");

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand winCmd = WIN_CMD_FOREGROUND;

    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();

    jsUiExtensionBase->OnCommandWindowDone(sessionInfo, winCmd);
    EXPECT_NE(jsUiExtensionBase, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnCommandWindowDone_0300 end.");
}

/**
 * @tc.name: OnCommand_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommand_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnCommand_0100 start.");

    AAFwk::Want want;
    bool restart = false;
    int32_t startId = 1;
    want.operation_.abilityName_ = TEST_ABILITYNAME;
    want.operation_.moduleName_ = TEST_MODULENAME;
    want.operation_.deviceId_ = TEST_DEVICEID;
    want.operation_.bundleName_ = TEST_BUNDLENAME;

    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);
    jsUiExtensionBase->OnCommand(want, restart, startId);
    EXPECT_NE(jsUiExtensionBase, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnCommand_0100 end.");
}

/**
 * @tc.name: OnForeground_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnForeground_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnForeground_0100 start.");

    AAFwk::Want want;
    want.parameters_.params_.emplace("ohos.insightIntent.executeParam.name", nullptr);
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);

    jsUiExtensionBase->abilityInfo_ = std::make_shared<AbilityInfo>();
    jsUiExtensionBase->abilityInfo_->hapPath = TEST_HAPPATH;
    jsUiExtensionBase->abilityInfo_->compileMode = AppExecFwk::CompileMode::ES_MODULE;
    jsUiExtensionBase->context_ = std::make_shared<AbilityRuntime::UIExtensionContext>();
    jsUiExtensionBase->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();
    std::shared_ptr<NativeReferenceMock> ptr = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->contentSessions_.emplace(sessionInfo->uiExtensionComponentId, ptr);
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);

    jsUiExtensionBase->OnForeground(want, sessionInfo);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnForeground_0100 end.");
}

/**
 * @tc.name: CallJsOnSessionCreate_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CallJsOnSessionCreate_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "CallJsOnSessionCreate_0100 start.");

    jsUiExtensionBase->jsObj_ = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    uint32_t uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    AAFwk::Want want;
    want.operation_.SetModuleName(TEST_MODULENAME);
    want.operation_.SetBundleName(TEST_BUNDLENAME);
    want.operation_.SetAbilityName(TEST_ABILITYNAME);
    want.operation_.SetDeviceId(TEST_DEVICEID);
    jsUiExtensionBase->abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();

    bool test = jsUiExtensionBase->CallJsOnSessionCreate(want, sessionInfo,
        uiwindow, uiExtensionComponentId);
    EXPECT_EQ(jsUiExtensionBase->contentSessions_.size(), 1);
    EXPECT_EQ(test, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "CallJsOnSessionCreate_0100 end.");
}

/**
 * @tc.name:  CallJsOnSessionCreate_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CallJsOnSessionCreate_0200, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "CallJsOnSessionCreate_0200 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    uint32_t uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    bool test =  jsUiExtensionBase->CallJsOnSessionCreate(want, sessionInfo,
        uiwindow, uiExtensionComponentId);
    EXPECT_EQ(test, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "CallJsOnSessionCreate_0200 end.");
}

/**
 * @tc.name:  CallJsOnSessionCreate_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CallJsOnSessionCreate_0300, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "CallJsOnSessionCreate_0300 start.");

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    uint32_t uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    AAFwk::Want want;

    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);

    bool test = jsUiExtensionBase->CallJsOnSessionCreate(want, sessionInfo,
        uiwindow, uiExtensionComponentId);
    EXPECT_EQ(test, true);

    TAG_LOGE(AAFwkTag::TEST, "CallJsOnSessionCreate_0300 end.");
}

/**
 * @tc.name:  CreateWindowOption_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CreateWindowOption_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "CreateWindowOption_0100 start.");

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->hostWindowId = 1;
    sessionInfo->realHostWindowId = 1;
    jsUiExtensionBase->context_ = std::make_shared<AbilityRuntime::UIExtensionContext>();
    jsUiExtensionBase->context_->applicationInfo_ = std::make_shared<AppExecFwk::ApplicationInfo>();
    jsUiExtensionBase->context_->applicationInfo_->bundleName = TEST_BUNDLENAME;
    jsUiExtensionBase->context_->abilityInfo_ = std::make_shared<AppExecFwk::AbilityInfo>();
    jsUiExtensionBase->context_->abilityInfo_->name = TEST_NAME;

    auto test = jsUiExtensionBase->CreateWindowOption(sessionInfo);
    EXPECT_NE(test, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "CreateWindowOption_0100 end.");
}

/**
 * @tc.name:  HandleSessionCreate_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, HandleSessionCreate_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0100 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    EXPECT_EQ(jsUiExtensionBase->HandleSessionCreate(want, sessionInfo), false);

    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0100 end.");
}

/**
 * @tc.name:  HandleSessionCreate_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, HandleSessionCreate_0200, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0200 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 0;
    EXPECT_EQ(jsUiExtensionBase->HandleSessionCreate(want, sessionInfo), false);

    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0200 end.");
}

/**
 * @tc.name:  HandleSessionCreate_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, HandleSessionCreate_0300, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0300 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);

    bool test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0300 end.");
}

/**
 * @tc.name:  HandleSessionCreate_0400
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, HandleSessionCreate_0400, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0400 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;

    jsUiExtensionBase->uiWindowMap_.clear();
    jsUiExtensionBase->context_ = nullptr;
    bool test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0400 end.");
}

/**
 * @tc.name:  HandleSessionCreate_0500
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, HandleSessionCreate_0500, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0500 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;

    jsUiExtensionBase->uiWindowMap_.clear();
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->abilityInfo_ = nullptr;
    bool test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0500 end.");
}

/**
 * @tc.name:  HandleSessionCreate_0600
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, HandleSessionCreate_0600, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0600 start.");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    jsUiExtensionBase->uiWindowMap_.clear();
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->abilityInfo_ = std::make_shared<AppExecFwk::AbilityInfo>();

    bool test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "HandleSessionCreate_0600 end.");
}

/**
 * @tc.name:  ForegroundWindow_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, ForegroundWindow_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "ForegroundWindow_0100 start.");

    AAFwk::Want want;
    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);

    jsUiExtensionBase->ForegroundWindow(want, sessionInfo);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "ForegroundWindow_0100 end.");
}

/**
 * @tc.name:  BackgroundWindow_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, BackgroundWindow_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "BackgroundWindow_0100 start.");

    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();

    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);

    jsUiExtensionBase->BackgroundWindow(sessionInfo);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "BackgroundWindow_0100 end.");
}

/**
 * @tc.name:  DestroyWindow_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, DestroyWindow_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "DestroyWindow_0100 start.");

    auto sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();

    jsUiExtensionBase->abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
    std::shared_ptr<AbilityRuntime::UISessionAbilityResultListener> abilityResultListener =
        std::make_shared<AbilityRuntime::UISessionAbilityResultListener>();
    auto nativereferece = std::make_shared<NativeReferenceMock>();

    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->contentSessions_.emplace(sessionInfo->uiExtensionComponentId, nativereferece);
    jsUiExtensionBase->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jsUiExtensionBase->abilityResultListeners_->listeners_.emplace(
        sessionInfo->uiExtensionComponentId, abilityResultListener);

    jsUiExtensionBase->DestroyWindow(sessionInfo);
    EXPECT_EQ(jsUiExtensionBase->uiWindowMap_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->contentSessions_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->abilityResultListeners_->listeners_.size(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "DestroyWindow_0100 end.");
}

/**
 * @tc.name:  CallObjectMethod_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CallObjectMethod_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "CallObjectMethod_0100 start.");

    jsUiExtensionBase->jsObj_ = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);
    auto vv = std::make_shared<NativeReferenceMock>();
    napi_value argv[] = { vv->GetNapiValue() };
    auto test = jsUiExtensionBase->CallObjectMethod("onDestroy", argv, 1, true);
    EXPECT_EQ(test, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "CallObjectMethod_0100 end.");
}

/**
 * @tc.name:  OnConfigurationUpdated_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnConfigurationUpdated_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnConfigurationUpdated_0100 start.");

    AppExecFwk::Configuration configuration;
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->config_ = std::make_shared<AppExecFwk::Configuration>();

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    jsUiExtensionBase->context_->resourceManager_ = resourceManager;
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = reinterpret_cast<NativeEngine*>(g_env);
    jsUiExtensionBase->shellContextRef_ = std::make_shared<NativeReferenceMock>();

    jsUiExtensionBase->OnConfigurationUpdated(configuration);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnConfigurationUpdated_0100 end.");
}

/**
 * @tc.name:  Dump_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, Dump_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "Dump_0100 start.");

    std::vector<std::string> param;
    param.emplace_back("-h");
    std::vector<std::string> info;
    jsUiExtensionBase->jsObj_ = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->Dump(param, info);
    EXPECT_NE(info.size(), 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "Dump_0100 end.");
}

/**
 * @tc.name:  OnAbilityResult_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnAbilityResult_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnAbilityResult_0100 start.");

    int32_t requestcode = 1;
    int32_t resultcode = 1;
    AAFwk::Want resultdata;
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    auto runtimetask = [](int, const AAFwk::Want&, bool) {};
    jsUiExtensionBase->context_->resultCallbacks_.emplace(requestcode, runtimetask);
    jsUiExtensionBase->abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
    jsUiExtensionBase->abilityResultListeners_->listeners_.emplace(requestcode, nullptr);

    jsUiExtensionBase->OnAbilityResult(requestcode, resultcode, resultdata);
    EXPECT_EQ(jsUiExtensionBase->context_->resultCallbacks_.size(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnAbilityResult_0100 end.");
}

/**
 * @tc.name:  SetAbilityInfo_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, SetAbilityInfo_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "SetAbilityInfo_0100 start.");

    jsUiExtensionBase->abilityInfo_ = nullptr;
    auto abilityinfo = std::make_shared<AppExecFwk::AbilityInfo>();
    jsUiExtensionBase->SetAbilityInfo(abilityinfo);
    EXPECT_NE(jsUiExtensionBase->abilityInfo_, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "SetAbilityInfo_0100 end.");
}

/**
 * @tc.name:  SetContext_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, SetContext_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "SetContext_0100 start.");

    jsUiExtensionBase->context_ = nullptr;
    auto context = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->SetContext(context);
    EXPECT_NE(jsUiExtensionBase->context_, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "SetContext_0100 end.");
}

/**
 * @tc.name:  OnInsightIntentExecuteDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnInsightIntentExecuteDone_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnInsightIntentExecuteDone_0100 start.");

    auto runtime = std::make_unique<AbilityRuntime::JsRuntime>();
    std::shared_ptr<AbilityRuntime::JsUIExtensionBase> jsUiExtensionBase =
        std::make_shared<AbilityRuntime::JsUIExtensionBase>(std::move(runtime));
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    AppExecFwk::InsightIntentExecuteResult result;

    sptr<Rosen::Window> uiwindow =  sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(1, uiwindow);
    jsUiExtensionBase->OnInsightIntentExecuteDone(sessionInfo, result);
    EXPECT_FALSE(jsUiExtensionBase->foregroundWindows_.empty());

    TAG_LOGE(AAFwkTag::TEST, "OnInsightIntentExecuteDone_0100 end.");
}

/**
 * @tc.name:  PostInsightIntentExecuted_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, PostInsightIntentExecuted_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "PostInsightIntentExecuted start.");

    auto runtime = std::make_unique<AbilityRuntime::JsRuntime>();
    std::shared_ptr<AbilityRuntime::JsUIExtensionBase> jsUiExtensionBase =
        std::make_shared<AbilityRuntime::JsUIExtensionBase>(std::move(runtime));
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;

    AppExecFwk::InsightIntentExecuteResult result;
    bool needForeground = true;
    jsUiExtensionBase->token_ = sptr<MockIRemoteObject>::MakeSptr();
    jsUiExtensionBase->PostInsightIntentExecuted(sessionInfo, result, needForeground);
    EXPECT_TRUE(jsUiExtensionBase->foregroundWindows_.empty());

    TAG_LOGE(AAFwkTag::TEST, "PostInsightIntentExecuted start.");
}

/**
 * @tc.name:  OnDisplayInfoChange_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
#ifdef SUPPORT_GRAPHICS
HWTEST_F(JsUiExtensionBaseTest, OnDisplayInfoChange_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "OnDisplayInfoChange_0100 start.");

    jsUiExtensionBase->context_ = std::make_shared<AbilityRuntime::UIExtensionContext>();
    jsUiExtensionBase->context_->config_ = std::make_shared<AppExecFwk::Configuration>();

    std::shared_ptr<Global::Resource::ResourceManager> resourceManager(Global::Resource::CreateResourceManager());
    jsUiExtensionBase->context_->resourceManager_ = resourceManager;
    sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    Rosen::DisplayId displayId = 1;
    float density { 2.0 };
    Rosen::DisplayOrientation orientation = Rosen::DisplayOrientation::LANDSCAPE;
    std::shared_ptr<AppExecFwk::AbilityHandler> handler =
        std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    jsUiExtensionBase->handler_ = handler;
    jsUiExtensionBase->OnDisplayInfoChange(token, displayId, density, orientation);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "OnDisplayInfoChange_0100 end.");
}
#endif

/**
 * @tc.name:  RegisterDisplayInfoChangedListener_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
#ifdef SUPPORT_GRAPHICS
HWTEST_F(JsUiExtensionBaseTest, RegisterDisplayInfoChangedListener_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "RegisterDisplayInfoChangedListener_0100 start.");

    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();
    jsUiExtensionBase->RegisterDisplayInfoChangedListener();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "RegisterDisplayInfoChangedListener_0100 end.");
}
#endif

/**
 * @tc.name:  UnregisterDisplayInfoChangedListener_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
#ifdef SUPPORT_GRAPHICS
HWTEST_F(JsUiExtensionBaseTest, UnregisterDisplayInfoChangedListener_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "RUnregisterDisplayInfoChangedListener_0100 start.");

    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();

    jsUiExtensionBase->UnregisterDisplayInfoChangedListener();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    TAG_LOGE(AAFwkTag::TEST, "UnregisterDisplayInfoChangedListener_0100 end.");
}
#endif

} // namespace AbilityRuntime
} // namespace OHOS