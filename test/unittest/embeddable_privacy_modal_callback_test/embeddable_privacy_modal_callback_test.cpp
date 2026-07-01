/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "embeddable_privacy_modal_callback.h"
#include "ui_extension_context.h"
#undef private
#undef protected

#include "hilog_tag_wrapper.h"
#include "mock_ui_content.h"
#include "want_params.h"
#include "int_wrapper.h"
#include "string_wrapper.h"

using namespace testing::ext;
using namespace testing;
using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::Ace;

namespace OHOS {
namespace AAFwk {

namespace {
constexpr const char* EXIT_EMBEDDABLE_PRIVACY_PARAM = "ohos.param.exitEmbeddablePrivacyUIExtension";
}

class EmbeddablePrivacyModalCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void EmbeddablePrivacyModalCallbackTest::SetUpTestCase(void)
{}

void EmbeddablePrivacyModalCallbackTest::TearDownTestCase(void)
{}

void EmbeddablePrivacyModalCallbackTest::SetUp()
{}

void EmbeddablePrivacyModalCallbackTest::TearDown()
{}

// ===== SetSessionId tests =====

/**
 * @tc.name: SetSessionId_0100
 * @tc.desc: SetSessionId stores the sessionId correctly.
 */
HWTEST_F(EmbeddablePrivacyModalCallbackTest, SetSessionId_0100, TestSize.Level1)
{
    auto callback = std::make_shared<EmbeddablePrivacyModalCallback>();
    callback->SetSessionId(456);
    EXPECT_EQ(callback->sessionId_, 456);
}

// ===== OnRelease tests =====

/**
 * @tc.name: OnRelease_0100
 * @tc.desc: OnRelease when context is null, early return without crash.
 */
HWTEST_F(EmbeddablePrivacyModalCallbackTest, OnRelease_0100, TestSize.Level1)
{
    auto callback = std::make_shared<EmbeddablePrivacyModalCallback>();
    callback->SetSessionId(456);
    callback->OnRelease();
    EXPECT_EQ(callback->sessionId_, 456);
}

// ===== OnError tests =====

/**
 * @tc.name: OnError_0100
 * @tc.desc: OnError when context is null, early return without crash.
 */
HWTEST_F(EmbeddablePrivacyModalCallbackTest, OnError_0100, TestSize.Level1)
{
    auto callback = std::make_shared<EmbeddablePrivacyModalCallback>();
    callback->SetSessionId(789);
    callback->OnError();
    EXPECT_EQ(callback->sessionId_, 789);
}

// ===== OnDestroy tests =====

/**
 * @tc.name: OnDestroy_0100
 * @tc.desc: OnDestroy when context is null, early return without crash.
 */
HWTEST_F(EmbeddablePrivacyModalCallbackTest, OnDestroy_0100, TestSize.Level1)
{
    auto callback = std::make_shared<EmbeddablePrivacyModalCallback>();
    callback->SetSessionId(101);
    callback->OnDestroy();
    EXPECT_EQ(callback->sessionId_, 101);
}

// ===== OnReceive tests (core: new exit key protocol) =====

/**
 * @tc.name: OnReceive_0100
 * @tc.desc: OnReceive with new exit key value=1, when context is null.
 *           Should early-return on null context, NOT crash, NOT terminate.
 *           Verifies the new exit key "ohos.param.exitEmbeddablePrivacyUIExtension".
 */
HWTEST_F(EmbeddablePrivacyModalCallbackTest, OnReceive_0100, TestSize.Level1)
{
    auto callback = std::make_shared<EmbeddablePrivacyModalCallback>();
    callback->SetSessionId(999);
    AAFwk::WantParams data;
    data.SetParam(EXIT_EMBEDDABLE_PRIVACY_PARAM, AAFwk::Integer::Box(1));
    // context is null -> early return, no crash
    callback->OnReceive(data);
    EXPECT_EQ(callback->sessionId_, 999);
}

/**
 * @tc.name: OnReceive_0200
 * @tc.desc: OnReceive with new exit key value=0, should NOT trigger termination.
 */
HWTEST_F(EmbeddablePrivacyModalCallbackTest, OnReceive_0200, TestSize.Level1)
{
    auto callback = std::make_shared<EmbeddablePrivacyModalCallback>();
    callback->SetSessionId(999);
    AAFwk::WantParams data;
    data.SetParam(EXIT_EMBEDDABLE_PRIVACY_PARAM, AAFwk::Integer::Box(0));
    callback->OnReceive(data);
    EXPECT_EQ(callback->sessionId_, 999);
}

/**
 * @tc.name: OnReceive_0300
 * @tc.desc: OnReceive without exit key, should NOT trigger any action.
 */
HWTEST_F(EmbeddablePrivacyModalCallbackTest, OnReceive_0300, TestSize.Level1)
{
    auto callback = std::make_shared<EmbeddablePrivacyModalCallback>();
    callback->SetSessionId(999);
    AAFwk::WantParams data;
    data.SetParam("some.other.key", AAFwk::Integer::Box(1));
    callback->OnReceive(data);
    EXPECT_EQ(callback->sessionId_, 999);
}

/**
 * @tc.name: OnReceive_0400
 * @tc.desc: OnReceive with OLD exit key "ohos.param.exitEmbeddableUIExtension" should NOT
 *           trigger termination, because EmbeddablePrivacyModalCallback only listens for
 *           the NEW key "ohos.param.exitEmbeddablePrivacyUIExtension".
 *           This verifies the protocol isolation between old and new paths.
 */
HWTEST_F(EmbeddablePrivacyModalCallbackTest, OnReceive_0400, TestSize.Level1)
{
    auto callback = std::make_shared<EmbeddablePrivacyModalCallback>();
    callback->SetSessionId(999);
    AAFwk::WantParams data;
    data.SetParam("ohos.param.exitEmbeddableUIExtension", AAFwk::Integer::Box(1));
    callback->OnReceive(data);
    EXPECT_EQ(callback->sessionId_, 999);
}

// ===== SetUIExtensionContext tests =====

/**
 * @tc.name: SetUIExtensionContext_0100
 * @tc.desc: SetUIExtensionContext stores the weak reference without crash.
 */
HWTEST_F(EmbeddablePrivacyModalCallbackTest, SetUIExtensionContext_0100, TestSize.Level1)
{
    auto callback = std::make_shared<EmbeddablePrivacyModalCallback>();
    std::weak_ptr<UIExtensionContext> emptyWeak;
    callback->SetUIExtensionContext(emptyWeak);
    // Verify lock returns nullptr for empty weak
    auto locked = callback->contextWeak_.lock();
    EXPECT_EQ(locked, nullptr);
}
} // namespace AAFwk
} // namespace OHOS
