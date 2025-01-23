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
#include <gmock/gmock.h>

#define private public
#define protected public
#include "js_environment.h"
#include "js_ui_extension_base.h"
#undef private
#undef protected

#include "ability_handler.h"
#include "ability_local_record.h"
#include "ability_manager_client.h"
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
#include "napi_common_want.h"
#include "ohos_application.h"
#include "operation.h"
#include "window.h"
#include "window_option.h"
#include "want_params.h"

#include <memory>

using namespace testing;
using namespace testing::ext;
using EventRunner = OHOS::AppExecFwk::EventRunner;

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
    panda::ecmascript::EcmaVM* create_vm()
    {
        panda::RuntimeOption options;
        panda::ecmascript::EcmaVM* vm = panda::JSNApi::CreateJSVM(options);
        return vm;
    }
    NativeEngine* create_engine()
    {
        panda::ecmascript::EcmaVM* vm = NativeReferenceMock().create_vm();
        NativeEngine *engine = new MockArkNativeEngine(vm, nullptr);
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

    std::shared_ptr<JsUIExtensionBase> jsUiExtensionBase;
};

void JsUiExtensionBaseTest::SetUpTestCase()
{
}
void JsUiExtensionBaseTest::TearDownTestCase()
{
}
void JsUiExtensionBaseTest::SetUp()
{
    std::unique_ptr<JsRuntime> jsRuntime =  std::make_unique<JsRuntime>();
    jsUiExtensionBase = std::make_shared<JsUIExtensionBase>(std::move(jsRuntime));
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
    TAG_LOGI(AAFwkTag::TEST, "Init_0100 start");
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityinfo =
        std::make_shared<AppExecFwk::AbilityInfo>();
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    int32_t abilityRecordId = 1;
    sptr<IRemoteObject> token1 = sptr<MockIRemoteObject>::MakeSptr();
    std::shared_ptr<AppExecFwk::AbilityLocalRecord> abilityrecord =
        std::make_shared<AppExecFwk::AbilityLocalRecord>(abilityinfo, token1, want, abilityRecordId);
    std::shared_ptr<AppExecFwk::OHOSApplication> application =
        std::make_shared<AppExecFwk::OHOSApplication>();
    std::shared_ptr<AppExecFwk::AbilityHandler> handler =
        std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    sptr<IRemoteObject> token2 = sptr<MockIRemoteObject>::MakeSptr();
    jsUiExtensionBase->abilityInfo_ = nullptr;
    auto result = jsUiExtensionBase->Init(abilityrecord, application, handler, token2);
    EXPECT_EQ(result, nullptr);
    jsUiExtensionBase->abilityInfo_ = abilityinfo;
    jsUiExtensionBase->abilityInfo_->srcEntrance = "";
    result = jsUiExtensionBase->Init(abilityrecord, application, handler, token2);
    EXPECT_EQ(result, nullptr);
    jsUiExtensionBase->abilityInfo_->srcEntrance = TEST_SRCENTRANCE;
    jsUiExtensionBase->Init(nullptr, application, handler, token2);
    EXPECT_EQ(jsUiExtensionBase->token_, nullptr);
    jsUiExtensionBase->abilityInfo_->moduleName = TEST_MODULENAME;
    jsUiExtensionBase->abilityInfo_->name = TEST_NAME;
    jsUiExtensionBase->abilityInfo_->hapPath = TEST_HAPPATH;
    std::shared_ptr<NativeReferenceMock> nativeRefenceMock =
        std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    panda::RuntimeOption options;
    jsUiExtensionBase->jsRuntime_.jsEnv_->vm_ = panda::JSNApi::CreateJSVM(options);
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    jsUiExtensionBase->jsRuntime_.modules_.emplace(
        "com.example.moduleName/com.example.srcentrance.abc", nativeRefenceMock.get());
    jsUiExtensionBase->Init(abilityrecord, application, handler, token2);
    EXPECT_EQ(jsUiExtensionBase->token_, abilityrecord->token_);
    EXPECT_EQ(jsUiExtensionBase->jsRuntime_.moduleName_,
        jsUiExtensionBase->abilityInfo_->moduleName);
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
    jsUiExtensionBase->jsObj_ = nullptr;
    jsUiExtensionBase->BindContext();
    EXPECT_EQ(jsUiExtensionBase->shellContextRef_, nullptr);
    jsUiExtensionBase->jsObj_ = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->BindContext();
    EXPECT_EQ(jsUiExtensionBase->shellContextRef_, nullptr);
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    jsUiExtensionBase->BindContext();
    EXPECT_EQ(jsUiExtensionBase->shellContextRef_, nullptr);
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->BindContext();
    EXPECT_NE(jsUiExtensionBase->shellContextRef_, nullptr);
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
    AAFwk::Want want;
    want.parameters_.params_.emplace("ohos.insightIntent.executeParam.name", nullptr);
    AAFwk::LaunchParam launchParam;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    AAFwk::WindowCommand winCmd = WIN_CMD_FOREGROUND;
    jsUiExtensionBase->context_ = nullptr;
    jsUiExtensionBase->OnStopCallBack();
    jsUiExtensionBase->OnCommandWindowDone(sessionInfo, winCmd);
    jsUiExtensionBase->OnStart(want, launchParam, sessionInfo);
    EXPECT_EQ(launchParam.launchReason, 9);
    TAG_LOGI(AAFwkTag::TEST, "OnStart_0100 end");
}

/**
 * @tc.name: OnStart_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnStart_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStart_0200 start");
    AAFwk::Want want;
    want.parameters_.params_.emplace("ohos.insightIntent.executeParam.name", nullptr);
    AAFwk::LaunchParam launchParam;
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    AAFwk::WindowCommand winCmd = WIN_CMD_FOREGROUND;
    jsUiExtensionBase->uiWindowMap_.clear();
    jsUiExtensionBase->OnCommandWindowDone(sessionInfo, winCmd);
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(1, uiwindow);
    jsUiExtensionBase->foregroundWindows_.clear();
    jsUiExtensionBase->OnCommandWindowDone(sessionInfo, winCmd);
    jsUiExtensionBase->foregroundWindows_.insert(1);
    jsUiExtensionBase->OnCommandWindowDone(sessionInfo, winCmd);
    jsUiExtensionBase->OnStart(want, launchParam, sessionInfo);
    EXPECT_EQ(launchParam.launchReason, 9);
    TAG_LOGI(AAFwkTag::TEST, "OnStart_0200 end");
}

/**
 * @tc.name: OnStart_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnStart_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStart_0300 start");
    AAFwk::Want want;
    want.parameters_.params_.emplace("ohos.insightIntent.executeParam.name", nullptr);
    AAFwk::LaunchParam launchParam;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->OnStart(want, launchParam, sessionInfo);
    EXPECT_EQ(launchParam.launchReason, 9);
    TAG_LOGI(AAFwkTag::TEST, "OnStart_0300 end");
}

/**
 * @tc.name: OnStop_2_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnStop_2_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnStop_2_0100 start");
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo = nullptr;
    bool isAsyncCallback = true;
    jsUiExtensionBase->jsObj_ = nullptr;
    jsUiExtensionBase->OnStop();
    jsUiExtensionBase->OnStop(callbackInfo, isAsyncCallback);
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
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo =
        AppExecFwk::AbilityTransactionCallbackInfo<>::Create();
    bool isAsyncCallback = true;
    jsUiExtensionBase->jsObj_ = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    jsUiExtensionBase->OnStop(callbackInfo, isAsyncCallback);
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();
    Context::applicationContext_ = std::make_shared<ApplicationContext>();
    jsUiExtensionBase->OnStopCallBack();
    EXPECT_EQ(isAsyncCallback, false);
    TAG_LOGI(AAFwkTag::TEST, "OnStop_2_0200 end");
}

/**
 * @tc.name: CheckPromise_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CheckPromise_0100, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::TEST, "CheckPromise_0100 start.");
    napi_value result =  nullptr;
    bool test = jsUiExtensionBase->CheckPromise(result);
    EXPECT_EQ(test, false);
    std::unique_ptr<MockDeferred> deferred = std::make_unique<MockDeferred>();
    NativeDeferred* deferredPtr = deferred.get();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    panda::ecmascript::EcmaVM* vm = nativeRefenceMock->create_vm();
    std::shared_ptr<MockArkNativeEngine> mockArkNativeEngine =
        std::make_shared<MockArkNativeEngine>(vm, nullptr);
    EXPECT_NE(mockArkNativeEngine, nullptr);
    result = mockArkNativeEngine->CreatePromise(&deferredPtr);
    EXPECT_NE(result, nullptr);
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = nullptr;
    EXPECT_EQ(jsUiExtensionBase->CheckPromise(result), false);
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    test = jsUiExtensionBase->CheckPromise(result);
    EXPECT_EQ(test, true);
    TAG_LOGE(AAFwkTag::TEST, "CheckPromise_0100 end.");
}

/**
 * @tc.name: CallPromise_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CallPromise_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CallPromise_0100 start");
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo =
        AppExecFwk::AbilityTransactionCallbackInfo<>::Create();
    napi_value promise = nullptr;
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    bool test = jsUiExtensionBase->CallPromise(promise, callbackInfo);
    EXPECT_EQ(test, false);
    std::unique_ptr<MockDeferred> deferred = std::make_unique<MockDeferred>();
    NativeDeferred* deferredPtr = deferred.get();
    panda::ecmascript::EcmaVM* vm = nativeRefenceMock->create_vm();
    std::shared_ptr<MockArkNativeEngine> mockArkNativeEngine =
        std::make_shared<MockArkNativeEngine>(vm, nullptr);
    EXPECT_NE(mockArkNativeEngine, nullptr);
    promise = mockArkNativeEngine->CreatePromise(&deferredPtr);
    EXPECT_NE(promise, nullptr);
    test = jsUiExtensionBase->CallPromise(promise, callbackInfo);
    EXPECT_EQ(test, true);
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
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand wincmd = AAFwk::WIN_CMD_FOREGROUND;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->OnCommandWindow(want, sessionInfo, wincmd);
    EXPECT_FALSE(jsUiExtensionBase->foregroundWindows_.empty());
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
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand wincmd = AAFwk::WIN_CMD_BACKGROUND;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jsUiExtensionBase->OnCommandWindow(want, sessionInfo, wincmd);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 0);
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
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    AAFwk::WindowCommand wincmd = AAFwk::WIN_CMD_DESTROY;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    auto ptr = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->contentSessions_.emplace(sessionInfo->uiExtensionComponentId, ptr);
    jsUiExtensionBase->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jsUiExtensionBase->abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
    auto listener = std::make_shared<UISessionAbilityResultListener>();
    jsUiExtensionBase->abilityResultListeners_->listeners_.emplace(
        sessionInfo->uiExtensionComponentId, listener);
    jsUiExtensionBase->OnCommandWindow(want, sessionInfo, wincmd);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->uiWindowMap_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->contentSessions_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->abilityResultListeners_->listeners_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "OnCommandWindow_0300 end");
}

/**
 * @tc.name: ForegroundWindowInitInsightIntentExecutorInfo_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, ForegroundWindowInitInsightIntentExecutorInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ForegroundWindowInitInsightIntentExecutorInfo_0100 start");
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    InsightIntentExecutorInfo executorInfo;
    jsUiExtensionBase->ForegroundWindowInitInsightIntentExecutorInfo(want,
        sessionInfo, executorInfo);
    EXPECT_EQ(executorInfo.token, nullptr);
    sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    jsUiExtensionBase->context_ = nullptr;
    jsUiExtensionBase->OnStopCallBack();
    jsUiExtensionBase->ForegroundWindowInitInsightIntentExecutorInfo(want,
        sessionInfo, executorInfo);
    EXPECT_EQ(executorInfo.token, nullptr);
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->ForegroundWindowInitInsightIntentExecutorInfo(want,
        sessionInfo, executorInfo);
    EXPECT_EQ(executorInfo.token, nullptr);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->hapPath = TEST_HAPPATH;
    jsUiExtensionBase->context_->SetAbilityInfo(abilityInfo);
    jsUiExtensionBase->context_->token_ = sptr<MockIRemoteObject>::MakeSptr();
    jsUiExtensionBase->ForegroundWindowInitInsightIntentExecutorInfo(want,
        sessionInfo, executorInfo);
    EXPECT_EQ(executorInfo.hapPath, TEST_HAPPATH);
    EXPECT_EQ(executorInfo.token, jsUiExtensionBase->context_->token_);
    TAG_LOGI(AAFwkTag::TEST, "ForegroundWindowInitInsightIntentExecutorInfo_0100 end");
}

/**
 * @tc.name: ForegroundWindowWithInsightIntent_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, ForegroundWindowWithInsightIntent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ForegroundWindowWithInsightIntent_0100 start");
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    bool test = jsUiExtensionBase->ForegroundWindowWithInsightIntent(want, sessionInfo, false);
    EXPECT_EQ(test, false);
    sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    test = jsUiExtensionBase->ForegroundWindowWithInsightIntent(want, sessionInfo, false);
    EXPECT_EQ(test, true);
    TAG_LOGI(AAFwkTag::TEST, "ForegroundWindowWithInsightIntent_0100 end");
}

/**
 * @tc.name: ExecuteInsightIntentDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, ExecuteInsightIntentDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInsightIntentDone_0100 start");
    uint64_t intentId = 1;
    InsightIntentExecuteResult result;
    jsUiExtensionBase->token_ = nullptr;
    jsUiExtensionBase->ExecuteInsightIntentDone(intentId, result);
    EXPECT_NE(AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(
        jsUiExtensionBase->token_, intentId, result), ERR_OK);
    jsUiExtensionBase->token_ = sptr<AppExecFwk::MockAbilityToken>::MakeSptr();
    jsUiExtensionBase->ExecuteInsightIntentDone(intentId, result);
    EXPECT_EQ(AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(
        jsUiExtensionBase->token_, intentId, result), ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ExecuteInsightIntentDone_0100 end");
}

/**
 * @tc.name: PostInsightIntentExecuted_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, PostInsightIntentExecuted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PostInsightIntentExecuted_0100 start");
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    AppExecFwk::InsightIntentExecuteResult result;
    bool needForeground = true;
    jsUiExtensionBase->token_ = nullptr;
    jsUiExtensionBase->PostInsightIntentExecuted(sessionInfo, result, needForeground);
    EXPECT_TRUE(jsUiExtensionBase->foregroundWindows_.empty());
    needForeground = false;
    sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    result.result = std::shared_ptr<AAFwk::WantParams>();
    result.code = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->PostInsightIntentExecuted(sessionInfo, result, needForeground);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "PostInsightIntentExecuted_0100 end");
}

/**
 * @tc.name: OnInsightIntentExecuteDone_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnInsightIntentExecuteDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnInsightIntentExecuteDone_0100 start");
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    AppExecFwk::InsightIntentExecuteResult result;
    jsUiExtensionBase->OnInsightIntentExecuteDone(sessionInfo, result);
    EXPECT_TRUE(jsUiExtensionBase->foregroundWindows_.empty());
    sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sessionInfo->sessionToken = sptr<MockIRemoteObject>::MakeSptr();
    result.result = nullptr;
    jsUiExtensionBase->OnInsightIntentExecuteDone(sessionInfo, result);
    EXPECT_TRUE(jsUiExtensionBase->foregroundWindows_.empty());
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->OnInsightIntentExecuteDone(sessionInfo, result);
    result.result = std::make_shared<WantParams>();
    jsUiExtensionBase->OnInsightIntentExecuteDone(sessionInfo, result);
    EXPECT_FALSE(jsUiExtensionBase->foregroundWindows_.empty());
    TAG_LOGI(AAFwkTag::TEST, "OnInsightIntentExecuteDone_0100 end");
}

/**
 * @tc.name: OnCommand_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommand_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommand_0100 start");
    AAFwk::Want want;
    bool restart = false;
    int32_t startId = 1;
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = nullptr;
    jsUiExtensionBase->OnCommand(want, restart, startId);
    EXPECT_EQ(jsUiExtensionBase->jsRuntime_.GetNapiEnv(), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnCommand_0100 end");
}

/**
 * @tc.name: OnCommand_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnCommand_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnCommand_0200 start");
    AAFwk::Want want;
    bool restart = false;
    int32_t startId = 1;
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    jsUiExtensionBase->OnCommand(want, restart, startId);
    EXPECT_NE(jsUiExtensionBase->jsRuntime_.GetNapiEnv(), nullptr);
    EXPECT_NE(AppExecFwk::WrapWant(jsUiExtensionBase->jsRuntime_.GetNapiEnv(), want), nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnCommand_0200 end");
}

/**
 * @tc.name: OnForeground_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnForeground_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnForeground_0100 start");
    AAFwk::Want want;
    want.parameters_.params_.emplace("ohos.insightIntent.executeParam.name", nullptr);
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->OnBackground();
    jsUiExtensionBase->OnForeground(want, sessionInfo);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 1);
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
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    uint32_t uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    bool test =  jsUiExtensionBase->CallJsOnSessionCreate(want, sessionInfo,
        uiwindow, uiExtensionComponentId);
    EXPECT_EQ(test, false);
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    test = jsUiExtensionBase->CallJsOnSessionCreate(want, sessionInfo,
        uiwindow, uiExtensionComponentId);
    EXPECT_EQ(jsUiExtensionBase->contentSessions_.size(), 1);
    EXPECT_EQ(test, true);
    TAG_LOGI(AAFwkTag::TEST, "CallJsOnSessionCreate_0100 end");
}

/**
 * @tc.name:  CreateWindowOption_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CreateWindowOption_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateWindowOption_0100 start");
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->hostWindowId = 1;
    sessionInfo->realHostWindowId = 1;
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->context_->applicationInfo_ = std::make_shared<ApplicationInfo>();
    jsUiExtensionBase->context_->applicationInfo_->bundleName = TEST_BUNDLENAME;
    jsUiExtensionBase->context_->abilityInfo_ = std::make_shared<AbilityInfo>();
    jsUiExtensionBase->context_->abilityInfo_->name = TEST_NAME;
    auto test = jsUiExtensionBase->CreateWindowOption(sessionInfo);
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
    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    bool test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
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
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    bool test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, false);
    sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 0;
    test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, false);
    sessionInfo->uiExtensionComponentId = 1;
    jsUiExtensionBase->context_ = nullptr;
    test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, false);
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, false);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    jsUiExtensionBase->context_->SetAbilityInfo(abilityInfo);
    test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, false);
    jsUiExtensionBase->context_->applicationInfo_ = std::make_shared<ApplicationInfo>();
    jsUiExtensionBase->context_->applicationInfo_->bundleName = TEST_BUNDLENAME;
    jsUiExtensionBase->context_->abilityInfo_ = std::make_shared<AbilityInfo>();
    jsUiExtensionBase->context_->abilityInfo_->name = TEST_NAME;
    test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, false);
    sessionInfo->displayId = 1;
    sessionInfo->sessionToken = sptr<MockIRemoteObject>::MakeSptr();
    test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_EQ(test, false);
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    test = jsUiExtensionBase->HandleSessionCreate(want, sessionInfo);
    EXPECT_NE(jsUiExtensionBase->context_->window_, nullptr);
    EXPECT_EQ(test, true);
    TAG_LOGI(AAFwkTag::TEST, "HandleSessionCreate_0200 end");
}

/**
 * @tc.name:  ForegroundWindow_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, ForegroundWindow_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ForegroundWindow_0100 start");

    AAFwk::Want want;
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    jsUiExtensionBase->ForegroundWindow(want, sessionInfo);
    EXPECT_TRUE(jsUiExtensionBase->foregroundWindows_.empty());
    sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->ForegroundWindow(want, sessionInfo);
    EXPECT_FALSE(jsUiExtensionBase->foregroundWindows_.empty());
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
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    jsUiExtensionBase->BackgroundWindow(sessionInfo);
    sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    jsUiExtensionBase->BackgroundWindow(sessionInfo);
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, nullptr);
    jsUiExtensionBase->BackgroundWindow(sessionInfo);
    jsUiExtensionBase->uiWindowMap_.clear();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    jsUiExtensionBase->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jsUiExtensionBase->BackgroundWindow(sessionInfo);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 0);
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
    sptr<AAFwk::SessionInfo> sessionInfo = nullptr;
    jsUiExtensionBase->DestroyWindow(sessionInfo);
    sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    jsUiExtensionBase->DestroyWindow(sessionInfo);
    sessionInfo->uiExtensionComponentId = 1;
    sptr<Rosen::Window> uiwindow = sptr<Rosen::Window>::MakeSptr();
    jsUiExtensionBase->uiWindowMap_.emplace(sessionInfo->uiExtensionComponentId, uiwindow);
    auto nativereferece = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->contentSessions_.emplace(sessionInfo->uiExtensionComponentId, nativereferece);
    jsUiExtensionBase->foregroundWindows_.insert(sessionInfo->uiExtensionComponentId);
    jsUiExtensionBase->abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
    auto abilityResultListener = std::make_shared<UISessionAbilityResultListener>();
    jsUiExtensionBase->abilityResultListeners_->listeners_.emplace(sessionInfo->uiExtensionComponentId,
        abilityResultListener);
    jsUiExtensionBase->DestroyWindow(sessionInfo);
    EXPECT_EQ(jsUiExtensionBase->uiWindowMap_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->contentSessions_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->foregroundWindows_.size(), 0);
    EXPECT_EQ(jsUiExtensionBase->abilityResultListeners_->listeners_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "DestroyWindow_0100 end");
}

/**
 * @tc.name:  CallObjectMethod_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, CallObjectMethod_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CallObjectMethod_0100 start");

    jsUiExtensionBase->jsObj_ = nullptr;
    auto test = jsUiExtensionBase->CallObjectMethod("", nullptr, 0, true);
    EXPECT_EQ(test, nullptr);
    jsUiExtensionBase->jsObj_ = std::make_shared<NativeReferenceMock>();
    auto nativeRefenceMock = std::make_shared<NativeReferenceMock>();
    jsUiExtensionBase->jsRuntime_.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
    jsUiExtensionBase->jsRuntime_.jsEnv_->engine_ = nativeRefenceMock->create_engine();
    auto vv = jsUiExtensionBase->jsObj_;
    napi_value argv[] = { vv->GetNapiValue() };
    test = jsUiExtensionBase->CallObjectMethod("onTest", argv, 1, true);
    EXPECT_EQ(test, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "CallObjectMethod_0100 end");
}

/**
 * @tc.name:  Dump_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, Dump_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Dump_0100 start");
    std::vector<std::string> params;
    params.emplace_back("dumptest");
    std::vector<std::string> info;
    jsUiExtensionBase->jsObj_ = nullptr;
    jsUiExtensionBase->Dump(params, info);
    EXPECT_TRUE(info.empty());
    TAG_LOGI(AAFwkTag::TEST, "Dump_0100 end");
}

/**
 * @tc.name:  OnAbilityResult_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, OnAbilityResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityResult_0100 start");
    int32_t requestcode = 1;
    int32_t resultcode = 1;
    AAFwk::Want resultdata;
    jsUiExtensionBase->context_ = nullptr;
    jsUiExtensionBase->OnAbilityResult(requestcode, resultcode, resultdata);
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    auto runtimetask = [](int, const AAFwk::Want&, bool) {};
    jsUiExtensionBase->context_->resultCallbacks_.emplace(requestcode, runtimetask);
    jsUiExtensionBase->OnAbilityResult(requestcode, resultcode, resultdata);
    jsUiExtensionBase->abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
    jsUiExtensionBase->abilityResultListeners_->listeners_.emplace(requestcode, nullptr);
    jsUiExtensionBase->OnAbilityResult(requestcode, resultcode, resultdata);
    EXPECT_EQ(jsUiExtensionBase->context_->resultCallbacks_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityResult_0100 end");
}

/**
 * @tc.name:  SetAbilityInfo_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, SetAbilityInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityInfo_0100 start");
    jsUiExtensionBase->abilityInfo_ = nullptr;
    auto abilityinfo = std::make_shared<AppExecFwk::AbilityInfo>();
    jsUiExtensionBase->SetAbilityInfo(abilityinfo);
    jsUiExtensionBase->context_ = nullptr;
    jsUiExtensionBase->ConfigurationUpdated();
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->ConfigurationUpdated();
    std::shared_ptr<AppExecFwk::Configuration> configurationPtr =
        std::make_shared<AppExecFwk::Configuration>();
    jsUiExtensionBase->context_->SetConfiguration(configurationPtr);
    jsUiExtensionBase->ConfigurationUpdated();
    EXPECT_NE(jsUiExtensionBase->abilityInfo_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "SetAbilityInfo_0100 end");
}

/**
 * @tc.name:  SetContext_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, SetContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetContext_0100 start");

    jsUiExtensionBase->context_ = nullptr;
    sptr<MockIRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    Rosen::DisplayId displayId = 1;
    float density { 2.0 };
    Rosen::DisplayOrientation orientation = Rosen::DisplayOrientation::LANDSCAPE;
    jsUiExtensionBase->OnDisplayInfoChange(token, displayId, density, orientation);
    AppExecFwk::Configuration configuration;
    jsUiExtensionBase->OnConfigurationUpdated(configuration);
    auto context = std::make_shared<UIExtensionContext>();
    jsUiExtensionBase->SetContext(context);
    jsUiExtensionBase->OnDisplayInfoChange(token, displayId, density, orientation);
    auto configurationPtr = std::make_shared<AppExecFwk::Configuration>();
    jsUiExtensionBase->context_->SetConfiguration(configurationPtr);
    jsUiExtensionBase->OnDisplayInfoChange(token, displayId, density, orientation);
    std::shared_ptr<Global::Resource::ResourceManager>
        resourceManager(Global::Resource::CreateResourceManager());
    jsUiExtensionBase->context_->SetResourceManager(resourceManager);
    jsUiExtensionBase->handler_ = std::make_shared<AppExecFwk::AbilityHandler>(nullptr);
    jsUiExtensionBase->OnDisplayInfoChange(token, displayId, density, orientation);
    EXPECT_NE(jsUiExtensionBase->context_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "SetContext_0100 end");
}

/**
 * @tc.name:  RegisterDisplayInfoChangedListener_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, RegisterDisplayInfoChangedListener_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RegisterDisplayInfoChangedListener_0100 start");
    jsUiExtensionBase->context_ = nullptr;
    jsUiExtensionBase->RegisterDisplayInfoChangedListener();
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    auto token = sptr<MockIRemoteObject>::MakeSptr();
    jsUiExtensionBase->context_->SetToken(token);
    jsUiExtensionBase->RegisterDisplayInfoChangedListener();
    jsUiExtensionBase->jsUIExtensionBaseDisplayListener_ =
        sptr<JsUIExtensionBase::JsUIExtensionBaseDisplayListener>::MakeSptr(jsUiExtensionBase);
    auto test = Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(
        jsUiExtensionBase->context_->GetToken(), jsUiExtensionBase->jsUIExtensionBaseDisplayListener_);
    EXPECT_EQ(test, Rosen::WMError::WM_OK);
    TAG_LOGI(AAFwkTag::TEST, "RegisterDisplayInfoChangedListener_0100 end");
}

/**
 * @tc.name:  UnregisterDisplayInfoChangedListener_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(JsUiExtensionBaseTest, UnregisterDisplayInfoChangedListener_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnregisterDisplayInfoChangedListener_0100 start");

    jsUiExtensionBase->context_ = nullptr;
    jsUiExtensionBase->UnregisterDisplayInfoChangedListener();
    jsUiExtensionBase->context_ = std::make_shared<UIExtensionContext>();
    auto token = sptr<MockIRemoteObject>::MakeSptr();
    jsUiExtensionBase->context_->SetToken(token);
    jsUiExtensionBase->jsUIExtensionBaseDisplayListener_ =
        sptr<JsUIExtensionBase::JsUIExtensionBaseDisplayListener>::MakeSptr(jsUiExtensionBase);
    Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(
        jsUiExtensionBase->context_->GetToken(), jsUiExtensionBase->jsUIExtensionBaseDisplayListener_);
    jsUiExtensionBase->UnregisterDisplayInfoChangedListener();
    EXPECT_EQ(Rosen::WindowManager::GetInstance().UnregisterDisplayInfoChangedListener(
        jsUiExtensionBase->context_->GetToken(), jsUiExtensionBase->jsUIExtensionBaseDisplayListener_),
        Rosen::WMError::WM_OK);

    TAG_LOGI(AAFwkTag::TEST, "RegisterDisplayInfoChangedListener_0100 end");
}

} // namespace AbilityRuntime
} // namespace OHOS