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

#include "ani.h"
#include "mock_ui_content.h"
#define private public
#define protected public
#include "sts_ui_extension_callback.h"
#undef protected
#undef private

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AbilityRuntime;
class StsUIExtensionCallbackTest : public testing::Test {
public:
    StsUIExtensionCallbackTest() : stsUIExtensionCallback_() {}
    ~StsUIExtensionCallbackTest() {}
    std::shared_ptr<StsUIExtensionCallback> stsUIExtensionCallback_;
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StsUIExtensionCallbackTest::SetUpTestCase(void) {}

void StsUIExtensionCallbackTest::TearDownTestCase(void) {}

void StsUIExtensionCallbackTest::SetUp(void) {}

void StsUIExtensionCallbackTest::TearDown(void) {}

/**
 * @tc.name: SetSessionId_0100
 * @tc.desc: test SetSessionId of StsUIExtensionCallback.
 * @tc.type: FUNC
 */
HWTEST_F(StsUIExtensionCallbackTest, SetSessionId_001, TestSize.Level1)
{
    stsUIExtensionCallback_ = std::make_shared<StsUIExtensionCallback>();
    ASSERT_NE(stsUIExtensionCallback_, nullptr);
    stsUIExtensionCallback_->OnError(0);
    stsUIExtensionCallback_->OnRelease(0);
    AAFwk::Want want;
    stsUIExtensionCallback_->OnResult(0, want);
    int32_t sessionId = 1;
    stsUIExtensionCallback_->SetSessionId(sessionId);
    ASSERT_EQ(stsUIExtensionCallback_->aniSessionId_, sessionId);
}

/**
 * @tc.name: SetUIContent_0100
 * @tc.desc: test SetUIContent of StsUIExtensionCallback.
 * @tc.type: FUNC
 */
HWTEST_F(StsUIExtensionCallbackTest, SetUIContent_001, TestSize.Level1)
{
    stsUIExtensionCallback_ = std::make_shared<StsUIExtensionCallback>();
    ASSERT_NE(stsUIExtensionCallback_, nullptr);
    Ace::UIContent *uiContent = nullptr;
    stsUIExtensionCallback_->SetUIContent(uiContent);
    stsUIExtensionCallback_->CloseModalUIExtension();
    ASSERT_EQ(stsUIExtensionCallback_->aniUIContent_, nullptr);
}

/**
 * @tc.name: SetUIContent_0200
 * @tc.desc: test SetUIContent of StsUIExtensionCallback.
 * @tc.type: FUNC
 */
HWTEST_F(StsUIExtensionCallbackTest, SetUIContent_002, TestSize.Level1)
{
    stsUIExtensionCallback_ = std::make_shared<StsUIExtensionCallback>();
    ASSERT_NE(stsUIExtensionCallback_, nullptr);
    Ace::MockUIContent *uiContent = new Ace::MockUIContent();
    stsUIExtensionCallback_->SetUIContent(uiContent);
    stsUIExtensionCallback_->CloseModalUIExtension();
    ASSERT_NE(stsUIExtensionCallback_->aniUIContent_, nullptr);
}

/**
 * @tc.name: SetStsCallbackObject_001
 * @tc.desc: test SetStsCallbackObject of StsUIExtensionCallback.
 * @tc.type: FUNC
 */
HWTEST_F(StsUIExtensionCallbackTest, SetStsCallbackObject_001, TestSize.Level1)
{
    stsUIExtensionCallback_ = std::make_shared<StsUIExtensionCallback>();
    ASSERT_NE(stsUIExtensionCallback_, nullptr);
    ani_vm *aniVM = nullptr;
    ani_object aniObject = nullptr;
    stsUIExtensionCallback_->SetStsCallbackObject(aniVM, aniObject);
    ASSERT_EQ(stsUIExtensionCallback_->aniVM_, nullptr);
    ASSERT_NE(stsUIExtensionCallback_->startAbilityAniCallback_, nullptr);
}

/**
 * @tc.name: GetAniEnv_001
 * @tc.desc: test GetAniEnv of StsUIExtensionCallback.
 * @tc.type: FUNC
 */
HWTEST_F(StsUIExtensionCallbackTest, GetAniEnv_001, TestSize.Level1)
{
    stsUIExtensionCallback_ = std::make_shared<StsUIExtensionCallback>();
    ASSERT_NE(stsUIExtensionCallback_, nullptr);
    auto aniEnv = stsUIExtensionCallback_->GetAniEnv();
    ASSERT_EQ(aniEnv, nullptr);
}
} // namespace AppExecFwk
} // namespace OHOS
