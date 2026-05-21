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
#include <singleton.h>
#include <cstdlib>
#include <cstring>
#include <uv.h>
#include "ability_context.h"
#include "ability_context_impl.h"
#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "errors.h"
#include "hilog_wrapper.h"
#define private public
#define protected public
#include "js_ability_context.h"
#undef private
#undef protected
#include "js_runtime_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/impl/ark/ark_native_deferred.h"
#include "native_engine/native_engine.h"
#include "js_runtime_lite.h"
#include "napi_common_want.h"

#include "mock_parse_requestcode.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;

const int USLEEPTIME = 100000;

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
        if (nref_ != nullptr) {
            napi_delete_reference(env_, nref_);
            nref_ = nullptr;
        }
        if (reason != nullptr) {
            napi_create_reference(env_, reason, 1, &nref_);
        }
    }

public:
    static bool GetLastResolveStatus() { return resolved_; }
    static napi_ref GetLastResolveValue() { return nref_; }
    static void Clear()
    {
        if (nref_) {
            napi_delete_reference(env_, nref_);
            nref_ = nullptr;
        }
        resolved_ = false;
    }
    static bool resolved_;
    static napi_ref nref_;
};
bool MockDeferred::resolved_ = false;
napi_ref MockDeferred::nref_ = nullptr;

class MockArkNativeEngine : public ArkNativeEngine {
public:
    MockArkNativeEngine(EcmaVM* vm, void* jsEngine, bool isLimitedWorker = false)
        :ArkNativeEngine(vm, jsEngine, isLimitedWorker)
    {}

    napi_value CreatePromise(NativeDeferred** deferred) override
    {
        napi_value ret = ArkNativeEngine::CreatePromise(deferred);
        *deferred = new (std::nothrow) MockDeferred;
        return ret;
    }
};

class MockAbilityContextImpl : public AbilityContextImpl {
public:
    virtual ErrCode ConnectUIServiceExtensionAbility(const AAFwk::Want &want,
        const sptr<AbilityConnectCallback> &connectCallback) override
    {
        callback_ = connectCallback;
        GTEST_LOG_(INFO) << "ConnectAbility, ret " << connectRet_;
        return connectRet_;
    }

    void DisconnectAbility(const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback,
        int32_t accountId = -1) override
    {}

    ErrCode StartSelf() override
    {
        return startSelfRet_;
    }

    ErrCode StartSelfUIAbilityInCurrentProcess(const AAFwk::Want &want, const std::string &specifiedFlag,
        const AAFwk::StartOptions &startOptions, bool hasOptions) override
    {
        return startSelfCurrentRet_;
    }

    virtual ErrCode StartSelfUIAbilityInChildProcess(const AAFwk::Want &want,
        const std::string &specifiedFlag) override
    {
        GTEST_LOG_(INFO) << "StartSelfUIAbilityInChildProcess mock called, ret " << startSelfRet_;
        return startSelfRet_;
    }

    ErrCode RequestModalUIExtension(const AAFwk::Want& want) override
    {
        return modalRet_;
    }

    ErrCode RequestModalUIExtensionWithAccount(const AAFwk::Want& want, int32_t accountId) override
    {
        return modalWithAccountRet_;
    }

    ErrCode RequestDialogService(napi_env env, AAFwk::Want &want, RequestDialogResultTask &&task) override
    {
        return requestDialogRet_;
    }

    ErrCode SetOnNewWantSkipScenarios(int32_t scenarios) override
    {
        return skipScenariosRet_;
    }

    ErrCode TerminateSelf() override
    {
        return terminateSelfRet_;
    }

    ErrCode ReportDrawnCompleted() override
    {
        return reportDrawnRet_;
    }

    ErrCode RevokeDelegator() override
    {
        return revokeDelegatorRet_;
    }
public:
    static void DoneConnect(int status)
    {
        GTEST_LOG_(INFO) << "DoneConnect " << status;
        AppExecFwk::ElementName element;
        sptr<IRemoteObject> remoteObject;
        callback_->OnAbilityConnectDone(element, remoteObject, 0);
    }
    static void DoneDisconnect(int status)
    {
        GTEST_LOG_(INFO) << "DoneDisconnect " << status;
        AppExecFwk::ElementName element;
        callback_->OnAbilityDisconnectDone(element, 0);
    }
    void SetConnectResult(ErrCode code) { connectRet_ = code; }
    void SetStartSelfResult(ErrCode code) { startSelfRet_ = code; }
    void SetStartSelfCurrentResult(ErrCode code) { startSelfCurrentRet_ = code; }
    void SetModalResult(ErrCode code) { modalRet_ = code; }
    void SetModalWithAccountResult(ErrCode code) { modalWithAccountRet_ = code; }
    void SetRequestDialogResult(ErrCode code) { requestDialogRet_ = code; }
    void SetSkipScenariosResult(ErrCode code) { skipScenariosRet_ = code; }
    void SetTerminateSelfResult(ErrCode code) { terminateSelfRet_ = code; }
    void SetReportDrawnResult(ErrCode code) { reportDrawnRet_ = code; }
    void SetRevokeDelegatorResult(ErrCode code) { revokeDelegatorRet_ = code; }
protected:
    static sptr<AbilityConnectCallback> callback_;
    ErrCode connectRet_ = ERR_OK;
    ErrCode startSelfRet_ = ERR_OK;
    ErrCode startSelfCurrentRet_ = ERR_OK;
    ErrCode modalRet_ = ERR_OK;
    ErrCode modalWithAccountRet_ = ERR_OK;
    ErrCode requestDialogRet_ = ERR_OK;
    ErrCode skipScenariosRet_ = ERR_OK;
    ErrCode terminateSelfRet_ = ERR_OK;
    ErrCode reportDrawnRet_ = ERR_OK;
    ErrCode revokeDelegatorRet_ = ERR_OK;
};

sptr<AbilityConnectCallback> MockAbilityContextImpl::callback_;

std::string GetStringProperty(napi_value object, const char *name)
{
    napi_value property = nullptr;
    napi_get_named_property(env_, object, name, &property);
    char buffer[256] = {0};
    size_t copied = 0;
    napi_get_value_string_utf8(env_, property, buffer, sizeof(buffer), &copied);
    return std::string(buffer, copied);
}

int32_t GetInt32Property(napi_value object, const char *name)
{
    napi_value property = nullptr;
    napi_get_named_property(env_, object, name, &property);
    napi_valuetype type = napi_undefined;
    napi_typeof(env_, property, &type);
    if (type == napi_string) {
        auto value = GetStringProperty(object, name);
        return static_cast<int32_t>(std::strtol(value.c_str(), nullptr, 10));
    }
    int32_t value = 0;
    napi_get_value_int32(env_, property, &value);
    return value;
}

void CheckLastRejectedError(int32_t expectedCode, const std::string &expectedMessage)
{
    EXPECT_FALSE(MockDeferred::GetLastResolveStatus());
    napi_ref nref = MockDeferred::GetLastResolveValue();
    ASSERT_NE(nref, nullptr);
    napi_value error = nullptr;
    ASSERT_EQ(napi_get_reference_value(env_, nref, &error), napi_ok);
    ASSERT_NE(error, nullptr);
    EXPECT_EQ(GetInt32Property(error, "code"), expectedCode);
    if (!expectedMessage.empty()) {
        EXPECT_EQ(GetStringProperty(error, "message"), expectedMessage);
    }
}

class AbilityContextTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    void RunNowait(uv_loop_t* loop)
    {
        usleep(USLEEPTIME);
        uv_run(loop, UV_RUN_NOWAIT);
    }
    void Connect(napi_value* argv, int32_t argc);
    void Disconnect(napi_value* argv, int32_t argc);
public:
    std::shared_ptr<JsAbilityContext> jsAbilityContext_;
    std::shared_ptr<MockAbilityContextImpl> abilityContextImpl_;
};

void AbilityContextTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    panda::RuntimeOption option;
    option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    const int64_t poolSize = 0x1000000;
    option.SetGcPoolSize(poolSize);
    option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
    option.SetDebuggerLibraryPath("");
    vm_ = panda::JSNApi::CreateJSVM(option);
    if (vm_ == nullptr) {
        GTEST_LOG_(INFO) << "Create vm failed.";
        return;
    }
    env_ = reinterpret_cast<napi_env>(new (std::nothrow) MockArkNativeEngine(vm_, nullptr));
}

void AbilityContextTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    MockArkNativeEngine* engine = reinterpret_cast<MockArkNativeEngine*>(env_);
    delete engine;
    engine = nullptr;
    if (vm_ != nullptr) {
        JSNApi::DestroyJSVM(vm_);
        vm_ = nullptr;
    }
}

void AbilityContextTest::SetUp()
{
    GTEST_LOG_(INFO) << "AbilityContextTest::SetUp";
    abilityContextImpl_ = std::make_shared<MockAbilityContextImpl>();
    if (abilityContextImpl_ == nullptr) {
        GTEST_LOG_(INFO) << "abilityContextImpl is nullptr.";
        return;
    }
    jsAbilityContext_ = std::make_shared<JsAbilityContext>(abilityContextImpl_);
}

void AbilityContextTest::TearDown()
{
    GTEST_LOG_(INFO) << "AbilityContextTest::TearDown " << (void*)this;
    MockDeferred::Clear();
    abilityContextImpl_.reset();
    jsAbilityContext_.reset();
}

void AbilityContextTest::Connect(napi_value* argv, int32_t argc)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0100 start";
    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        JsAbilityContext::ConnectUIServiceExtension(env, info);
        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        return result;
    };

    HandleScope handleScope(env_);
    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsAbilityContext_.get(),
        [](napi_env env, void* data, void* hint) {}, nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value funcValue = nullptr;
    napi_create_function(env_, "testFunc", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);

    napi_value funcResultValue = nullptr;
    napi_call_function(env_, recv, funcValue, argc, argv, &funcResultValue);
}

void AbilityContextTest::Disconnect(napi_value* argv, int32_t argc)
{
    auto func = [](napi_env env, napi_callback_info info) -> napi_value {
        JsAbilityContext::DisconnectUIServiceExtension(env, info);
        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        return result;
    };
    HandleScope handleScope(env_);
    napi_value recv = nullptr;
    napi_create_object(env_, &recv);
    napi_status wrapret = napi_wrap(env_, recv, jsAbilityContext_.get(), [](napi_env env, void* data, void* hint) {},
        nullptr, nullptr);
    EXPECT_EQ(wrapret, napi_ok);

    napi_value funcResultValue = nullptr;
    napi_value funcValue = nullptr;
    napi_create_function(env_, "disconnectFunc", NAPI_AUTO_LENGTH, func, nullptr, &funcValue);
    napi_call_function(env_, recv, funcValue, argc, argv, &funcResultValue);
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0100 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    napi_value argv[] = { };
    Connect(argv, ARGC_ZERO);
    EXPECT_TRUE(tryCatch.HasCaught());
    tryCatch.ClearException();
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }

    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_0101, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0101 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    napi_value undef = nullptr;
    napi_get_undefined(env_, &undef);
    napi_value argv[] = { undef };
    Connect(argv, ARGC_ONE);
    EXPECT_TRUE(tryCatch.HasCaught());
    tryCatch.ClearException();
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }

    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0101 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_0102, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0102 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    AAFwk::Want want;
    napi_value jswant = AppExecFwk::CreateJsWant(env_, want);
    napi_value argv[] = { jswant };
    Connect(argv, ARGC_ONE);
    EXPECT_TRUE(tryCatch.HasCaught());
    tryCatch.ClearException();
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }

    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0102 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_0103, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0103 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    AAFwk::Want want;
    napi_value jswant = AppExecFwk::CreateJsWant(env_, want);
    napi_value callbackObject = nullptr;
    napi_create_object(env_, &callbackObject);
    napi_value argv[] = { jswant, callbackObject};
    Connect(argv, ARGC_TWO);
    EXPECT_FALSE(tryCatch.HasCaught());

    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    uv_loop_t* loop = engine->GetUVLoop();
    RunNowait(loop);
    MockAbilityContextImpl::DoneConnect(ERR_OK);
    RunNowait(loop);

    EXPECT_TRUE(MockDeferred::GetLastResolveStatus());
    napi_ref nref = MockDeferred::GetLastResolveValue();
    EXPECT_NE(nref, nullptr);
    napi_value proxy = nullptr;
    ASSERT_EQ(napi_get_reference_value(env_, nref, &proxy), napi_ok);
    ASSERT_NE(proxy, nullptr);
    napi_value argv2[] = { proxy };
    Disconnect(argv2, ARGC_ONE);
    RunNowait(loop);

    MockAbilityContextImpl::DoneDisconnect(0);
    RunNowait(loop);
    abilityContextImpl_->SetConnectResult(ERR_OK);
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0103 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_ConnectUIServiceExtension_InnerErrorMessage_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ConnectUIServiceExtension_InnerErrorMessage_0100 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    AAFwk::Want want;
    napi_value jswant = AppExecFwk::CreateJsWant(env_, want);
    napi_value callbackObject = nullptr;
    napi_create_object(env_, &callbackObject);
    napi_value argv[] = { jswant, callbackObject };
    abilityContextImpl_->SetConnectResult(INNER_ERR);

    Connect(argv, ARGC_TWO);
    EXPECT_FALSE(tryCatch.HasCaught());
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        "Internal error. The UI service extension could not be connected. Check the extension configuration and try "
        "again.");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ConnectUIServiceExtension_InnerErrorMessage_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_ConnectUIServiceExtension_MappedError_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ConnectUIServiceExtension_MappedError_0100 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    AAFwk::Want want;
    napi_value jswant = AppExecFwk::CreateJsWant(env_, want);
    napi_value callbackObject = nullptr;
    napi_create_object(env_, &callbackObject);
    napi_value argv[] = { jswant, callbackObject };
    abilityContextImpl_->SetConnectResult(ERR_INVALID_CALLER);

    Connect(argv, ARGC_TWO);
    EXPECT_FALSE(tryCatch.HasCaught());
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CALLER), "");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ConnectUIServiceExtension_MappedError_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_0104, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0104 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    napi_value argv[] = {};
    Disconnect(argv, ARGC_ZERO);
    EXPECT_TRUE(tryCatch.HasCaught());
    tryCatch.ClearException();
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0104 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_0105, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0105 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    napi_value proxy = nullptr;
    napi_create_object(env_, &proxy);
    napi_value argv[] = {proxy};
    Disconnect(argv, ARGC_ONE);
    EXPECT_TRUE(tryCatch.HasCaught());
    tryCatch.ClearException();
    ArkNativeEngine* engine = (ArkNativeEngine*)env_;
    if (!engine->lastException_.IsEmpty()) {
        engine->lastException_.Empty();
    }
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_0105 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_DisconnectAbility_InnerErrorMessage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_DisconnectAbility_InnerErrorMessage_0100 start";
    HandleScope handleScope(env_);
    TryCatch tryCatch(env_);
    NapiCallbackInfo info;
    info.argc = ARGC_ONE;
    napi_create_int64(env_, 12345, &info.argv[0]);

    jsAbilityContext_->OnDisconnectAbility(env_, info);
    EXPECT_FALSE(tryCatch.HasCaught());

    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        "Internal error. The service connection does not exist. Use a connection ID returned by "
        "connectServiceExtensionAbility.");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_DisconnectAbility_InnerErrorMessage_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_StartSelf_InnerErrorMessage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_StartSelf_InnerErrorMessage_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    abilityContextImpl_->SetStartSelfResult(INNER_ERR);

    jsAbilityContext_->OnStartSelf(env_, info);
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        "Internal error. The ability could not start itself. Verify the caller process is foreground and starting "
        "abilities is allowed.");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_StartSelf_InnerErrorMessage_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_StartSelf_MappedError_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_StartSelf_MappedError_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    abilityContextImpl_->SetStartSelfResult(ERR_INVALID_CALLER);

    jsAbilityContext_->OnStartSelf(env_, info);
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CALLER), "");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_StartSelf_MappedError_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_RequestModalUIExtension_InnerErrorMessage_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestModalUIExtension_InnerErrorMessage_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    info.argc = ARGC_ONE;
    AAFwk::Want want;
    info.argv[0] = AppExecFwk::CreateJsWant(env_, want);
    abilityContextImpl_->SetModalResult(INNER_ERR);

    jsAbilityContext_->OnRequestModalUIExtension(env_, info);
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        "Internal error. The modal UI extension could not be created. Ensure the caller is the focused foreground "
        "UIAbility and try again.");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestModalUIExtension_InnerErrorMessage_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_RequestModalUIExtension_ReleasedContext_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestModalUIExtension_ReleasedContext_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    info.argc = ARGC_ONE;
    AAFwk::Want want;
    info.argv[0] = AppExecFwk::CreateJsWant(env_, want);
    jsAbilityContext_->context_.reset();

    jsAbilityContext_->OnRequestModalUIExtension(env_, info);
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), "");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestModalUIExtension_ReleasedContext_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_RequestModalUIExtensionWithAccount_InnerErrorMessage_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestModalUIExtensionWithAccount_InnerErrorMessage_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    info.argc = ARGC_TWO;
    AAFwk::Want want;
    info.argv[0] = AppExecFwk::CreateJsWant(env_, want);
    napi_create_int32(env_, 100, &info.argv[1]);
    abilityContextImpl_->SetModalWithAccountResult(INNER_ERR);

    jsAbilityContext_->OnRequestModalUIExtensionWithAccount(env_, info);
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        "Internal error. The modal UI extension could not be created. Ensure the caller is the focused foreground "
        "UIAbility and try again.");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestModalUIExtensionWithAccount_InnerErrorMessage_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_RequestModalUIExtensionWithAccount_MappedError_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestModalUIExtensionWithAccount_MappedError_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    info.argc = ARGC_TWO;
    AAFwk::Want want;
    info.argv[0] = AppExecFwk::CreateJsWant(env_, want);
    napi_create_int32(env_, 100, &info.argv[1]);
    abilityContextImpl_->SetModalWithAccountResult(ERR_INVALID_CALLER);

    jsAbilityContext_->OnRequestModalUIExtensionWithAccount(env_, info);
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CALLER), "");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestModalUIExtensionWithAccount_MappedError_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_RequestModalUIExtensionWithAccount_ReleasedContext_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestModalUIExtensionWithAccount_ReleasedContext_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    info.argc = ARGC_TWO;
    AAFwk::Want want;
    info.argv[0] = AppExecFwk::CreateJsWant(env_, want);
    napi_create_int32(env_, 100, &info.argv[1]);
    jsAbilityContext_->context_.reset();

    jsAbilityContext_->OnRequestModalUIExtensionWithAccount(env_, info);
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), "");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestModalUIExtensionWithAccount_ReleasedContext_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_SetOnNewWantSkipScenarios_Mapping_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_SetOnNewWantSkipScenarios_Mapping_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    info.argc = ARGC_ONE;
    napi_create_int32(env_, 1, &info.argv[0]);
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);

    abilityContextImpl_->SetSkipScenariosResult(ERR_INVALID_CONTEXT);
    jsAbilityContext_->OnSetOnNewWantSkipScenarios(env_, info);
    RunNowait(engine->GetUVLoop());
    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), "");

    abilityContextImpl_->SetSkipScenariosResult(ERR_INVALID_CALLER);
    jsAbilityContext_->OnSetOnNewWantSkipScenarios(env_, info);
    RunNowait(engine->GetUVLoop());
    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CALLER), "");

    abilityContextImpl_->SetSkipScenariosResult(INNER_ERR);
    jsAbilityContext_->OnSetOnNewWantSkipScenarios(env_, info);
    RunNowait(engine->GetUVLoop());
    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        "Internal error. The skip scenario setting could not be applied. Verify the scenario value and call this API "
        "while the ability is active.");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_SetOnNewWantSkipScenarios_Mapping_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_RequestDialogService_InnerErrorMessage_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestDialogService_InnerErrorMessage_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    info.argc = ARGC_ONE;
    AAFwk::Want want;
    info.argv[0] = AppExecFwk::CreateJsWant(env_, want);
    abilityContextImpl_->SetRequestDialogResult(INNER_ERR);

    jsAbilityContext_->OnRequestDialogService(env_, info);

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        "Internal error. Failed to request the dialog service. Check the Want parameters and try again.");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestDialogService_InnerErrorMessage_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_RequestDialogService_MappedError_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestDialogService_MappedError_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    info.argc = ARGC_ONE;
    AAFwk::Want want;
    info.argv[0] = AppExecFwk::CreateJsWant(env_, want);
    abilityContextImpl_->SetRequestDialogResult(ERR_INVALID_CALLER);

    jsAbilityContext_->OnRequestDialogService(env_, info);

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CALLER), "");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RequestDialogService_MappedError_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_TerminateSelf_InnerErrorMessage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_TerminateSelf_InnerErrorMessage_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    abilityContextImpl_->SetTerminateSelfResult(INNER_ERR);

    jsAbilityContext_->OnTerminateSelf(env_, info);
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        "Internal error. Failed to terminate the ability. Ensure the ability is active and try again.");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_TerminateSelf_InnerErrorMessage_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_ReportDrawnCompleted_InnerErrorMessage_0100,
    TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ReportDrawnCompleted_InnerErrorMessage_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    napi_value callback = nullptr;
    napi_create_function(env_, "cb", NAPI_AUTO_LENGTH, [](napi_env, napi_callback_info) -> napi_value {
        return nullptr;
    }, nullptr, &callback);
    info.argc = ARGC_ONE;
    info.argv[0] = callback;
    abilityContextImpl_->SetReportDrawnResult(INNER_ERR);

    jsAbilityContext_->OnReportDrawnCompleted(env_, info);
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        "Internal error. Failed to report drawn completed. Ensure the ability window is ready.");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ReportDrawnCompleted_InnerErrorMessage_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_RevokeDelegator_InnerErrorMessage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RevokeDelegator_InnerErrorMessage_0100 start";
    HandleScope handleScope(env_);
    NapiCallbackInfo info;
    abilityContextImpl_->SetRevokeDelegatorResult(INNER_ERR);

    jsAbilityContext_->OnRevokeDelegator(env_, info);
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    RunNowait(engine->GetUVLoop());

    CheckLastRejectedError(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
        "Internal error. Failed to revoke the delegator. Call this API while the ability is active.");
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_RevokeDelegator_InnerErrorMessage_0100 end";
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_RequestCodeFromStringToInt64_0100, TestSize.Level1)
{
    // requestCode is too long
    std::string requestCodeStr1 = "1000000000000001";
    auto requestCode = RequestCodeFromStringToInt64(requestCodeStr1);
    EXPECT_EQ(requestCode, 0);
    
    // requestCode not match
    std::string requestCodeStr2 = "001000";
    requestCode = RequestCodeFromStringToInt64(requestCodeStr2);
    EXPECT_EQ(requestCode, 0);

    std::string requestCodeStr3 = "aaa100";
    requestCode = RequestCodeFromStringToInt64(requestCodeStr3);
    EXPECT_EQ(requestCode, 0);

    std::string requestCodeStr4 = "100aaa";
    requestCode = RequestCodeFromStringToInt64(requestCodeStr4);
    EXPECT_EQ(requestCode, 0);

    std::string requestCodeStr5 = "-1";
    requestCode = RequestCodeFromStringToInt64(requestCodeStr5);
    EXPECT_EQ(requestCode, 0);
    
    // requestCode is too large
    std::string requestCodeStr6 = "562949953421312";
    requestCode = RequestCodeFromStringToInt64(requestCodeStr6);
    EXPECT_EQ(requestCode, 0);

    // requestCode is valid
    std::string requestCodeStr7 = "562949953421311";
    requestCode = RequestCodeFromStringToInt64(requestCodeStr7);
    EXPECT_EQ(requestCode, 562949953421311);

    std::string requestCodeStr8 = "0";
    requestCode = RequestCodeFromStringToInt64(requestCodeStr8);
    EXPECT_EQ(requestCode, 0);

    std::string requestCodeStr9 = "1";
    requestCode = RequestCodeFromStringToInt64(requestCodeStr9);
    EXPECT_EQ(requestCode, 1);
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_StartUIServiceExtension_0100, TestSize.Level1)
{
    napi_env env{nullptr};
    napi_callback_info info{nullptr};
    jsAbilityContext_->StartUIServiceExtension(env, info);
    EXPECT_EQ(info, nullptr);
}

HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_OnStartUIServiceExtension_0100, TestSize.Level1)
{
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());

    NapiCallbackInfo info{1};
    AAFwk::Want want;
    info.argv[0] = OHOS::AppExecFwk::WrapWant(env, want);
    jsAbilityContext_->OnStartUIServiceExtension(env, info);

    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
}

const char* GetExpectedContextType(int32_t inputType)
{
    static const char* contextTypeMap[] = {
        "ApplicationContext",
        "AbilityStageContext",
        "UIAbilityContext",
        "FormExtensionContext",
        "AppServiceExtensionContext",
        "ServiceExtensionContext",
        "UIServiceExtensionContext",
        "AutoFillExtensionContext",
    };
    if (inputType < 0 || inputType >= static_cast<int32_t>(sizeof(contextTypeMap) / sizeof(contextTypeMap[0]))) {
        return nullptr;
    }
    return contextTypeMap[inputType];
}

napi_value ContextTypeFunc(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisArg;
    napi_get_cb_info(env, info, &argc, argv, &thisArg, nullptr);

    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_number) {
        napi_value result = nullptr;
        napi_get_boolean(env, false, &result);
        return result;
    }

    int32_t inputType = 0;
    napi_get_value_int32(env, argv[0], &inputType);
    const char* expectedType = GetExpectedContextType(inputType);
    if (expectedType == nullptr) {
        napi_value result = nullptr;
        napi_get_boolean(env, false, &result);
        return result;
    }

    napi_value contextImpl = nullptr;
    napi_get_named_property(env, thisArg, "__context_impl__", &contextImpl);
    napi_value actualTypeValue = nullptr;
    napi_get_named_property(env, contextImpl, "contextType", &actualTypeValue);
    char actualTypeBuffer[64] = {0};
    size_t copied = 0;
    napi_get_value_string_utf8(env, actualTypeValue, actualTypeBuffer, sizeof(actualTypeBuffer), &copied);

    bool match = (strcmp(actualTypeBuffer, expectedType) == 0);
    napi_value resultValue = nullptr;
    napi_get_boolean(env, match, &resultValue);
    return resultValue;
}

void CreateContextTypeTestEnv(napi_env env, const char* typeStr, napi_value &contextObj, napi_value &contextTypeArg)
{
    napi_value contextImpl = nullptr;
    napi_create_object(env, &contextImpl);
    napi_value contextTypeValue = nullptr;
    napi_create_string_utf8(env, typeStr, NAPI_AUTO_LENGTH, &contextTypeValue);
    napi_set_named_property(env, contextImpl, "contextType", contextTypeValue);
    napi_create_object(env, &contextObj);
    napi_set_named_property(env, contextObj, "__context_impl__", contextImpl);
    contextTypeArg = nullptr;
}

void CallContextTypeAndCheck(napi_env env, napi_value contextObj, napi_value contextTypeArg, bool expected)
{
    napi_value funcValue = nullptr;
    napi_create_function(env, "isContextOf", NAPI_AUTO_LENGTH, ContextTypeFunc, nullptr, &funcValue);
    napi_value result = nullptr;
    napi_call_function(env, contextObj, funcValue, 1, &contextTypeArg, &result);
    bool boolResult = false;
    napi_get_value_bool(env, result, &boolResult);
    if (expected) {
        EXPECT_TRUE(boolResult);
    } else {
        EXPECT_FALSE(boolResult);
    }
}

/**
 * @tc.number: AbilityRuntime_AbilityContext_ContextType_0100
 * @tc.name: contextType with valid UIAbilityContext type
 * @tc.desc: Test contextType method with valid context type int (2 for UIAbilityContext)
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_ContextType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ContextType_0100 start";
    HandleScope handleScope(env_);

    napi_value contextObj = nullptr;
    napi_value contextTypeArg = nullptr;
    CreateContextTypeTestEnv(env_, "UIAbilityContext", contextObj, contextTypeArg);
    napi_create_int32(env_, 2, &contextTypeArg);
    CallContextTypeAndCheck(env_, contextObj, contextTypeArg, true);

    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ContextType_0100 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityContext_ContextType_0200
 * @tc.name: contextType with invalid type
 * @tc.desc: Test contextType method with invalid context type int (999)
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_ContextType_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ContextType_0200 start";
    HandleScope handleScope(env_);

    napi_value contextObj = nullptr;
    napi_value contextTypeArg = nullptr;
    CreateContextTypeTestEnv(env_, "UIAbilityContext", contextObj, contextTypeArg);
    napi_create_int32(env_, 999, &contextTypeArg);
    CallContextTypeAndCheck(env_, contextObj, contextTypeArg, false);

    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ContextType_0200 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityContext_ContextType_0300
 * @tc.name: contextType with non-number input
 * @tc.desc: Test contextType method with string input instead of number
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_ContextType_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ContextType_0300 start";
    HandleScope handleScope(env_);

    napi_value contextObj = nullptr;
    napi_value contextTypeArg = nullptr;
    CreateContextTypeTestEnv(env_, "UIAbilityContext", contextObj, contextTypeArg);
    napi_create_string_utf8(env_, "invalid", NAPI_AUTO_LENGTH, &contextTypeArg);
    CallContextTypeAndCheck(env_, contextObj, contextTypeArg, false);

    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ContextType_0300 end";
}

/**
 * @tc.number: AbilityRuntime_AbilityContext_ContextType_0400
 * @tc.name: contextType with mismatched type
 * @tc.desc: Test contextType method when actual type doesn't match expected type
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_ContextType_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ContextType_0400 start";
    HandleScope handleScope(env_);

    napi_value contextObj = nullptr;
    napi_value contextTypeArg = nullptr;
    CreateContextTypeTestEnv(env_, "UIAbilityContext", contextObj, contextTypeArg);
    napi_create_int32(env_, 0, &contextTypeArg);
    CallContextTypeAndCheck(env_, contextObj, contextTypeArg, false);

    GTEST_LOG_(INFO) << "AbilityRuntime_AbilityContext_ContextType_0400 end";
}

/**
 * @tc.name: AbilityRuntime_AbilityContext_StartSelfUIAbilityInChildProcess_0100
 * @tc.desc: Test StartSelfUIAbilityInChildProcess with normal parameters
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_StartSelfUIAbilityInChildProcess_0100, TestSize.Level1)
{
    auto contextImpl = std::make_shared<MockAbilityContextImpl>();
    AAFwk::Want want;
    want.SetElementName("com.test.bundle", "TestAbility");
    std::string specifiedFlag = "testFlag";
    contextImpl->SetStartSelfResult(ERR_OK);
    auto errCode = contextImpl->StartSelfUIAbilityInChildProcess(want, specifiedFlag);
    EXPECT_EQ(errCode, ERR_OK);
}

/**
 * @tc.name: AbilityRuntime_AbilityContext_StartSelfUIAbilityInChildProcess_0200
 * @tc.desc: Test StartSelfUIAbilityInChildProcess with empty want
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_StartSelfUIAbilityInChildProcess_0200, TestSize.Level1)
{
    auto contextImpl = std::make_shared<MockAbilityContextImpl>();
    AAFwk::Want want;
    std::string specifiedFlag = "";
    contextImpl->SetStartSelfResult(ERR_INVALID_VALUE);
    auto errCode = contextImpl->StartSelfUIAbilityInChildProcess(want, specifiedFlag);
    EXPECT_EQ(errCode, ERR_INVALID_VALUE);
}

/**
 * @tc.name: AbilityRuntime_AbilityContext_StartSelfUIAbilityInChildProcess_0300
 * @tc.desc: Test StartSelfUIAbilityInChildProcess with specifiedFlag
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_StartSelfUIAbilityInChildProcess_0300, TestSize.Level1)
{
    auto contextImpl = std::make_shared<MockAbilityContextImpl>();
    AAFwk::Want want;
    want.SetElementName("com.test.bundle", "TestAbility");
    want.SetParam("testKey", std::string("testValue"));
    std::string specifiedFlag = "specifiedFlag123";
    contextImpl->SetStartSelfResult(ERR_OK);
    auto errCode = contextImpl->StartSelfUIAbilityInChildProcess(want, specifiedFlag);
    EXPECT_EQ(errCode, ERR_OK);
}

/**
 * @tc.name: AbilityRuntime_AbilityContext_OnStartSelfUIAbilityInChildProcess_0100
 * @tc.desc: Test OnStartSelfUIAbilityInChildProcess JS binding
 * @tc.type: FUNC
 */
HWTEST_F(AbilityContextTest, AbilityRuntime_AbilityContext_OnStartSelfUIAbilityInChildProcess_0100, TestSize.Level1)
{
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    ASSERT_EQ(err, napi_status::napi_ok);
    ASSERT_NE(jsEnv, nullptr);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());

    NapiCallbackInfo info{2};
    AAFwk::Want want;
    info.argv[0] = OHOS::AppExecFwk::WrapWant(env, want);
    napi_create_string_latin1(env, "testFlag", NAPI_AUTO_LENGTH, &info.argv[1]);
    jsAbilityContext_->OnStartSelfUIAbilityInChildProcess(env, info);

    JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
}
}  // namespace AAFwk
}  // namespace OHOS
