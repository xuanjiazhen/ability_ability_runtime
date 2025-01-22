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
#include "cj_ui_extension_content_session.h"
#undef private
#include "foundation/ability/ability_runtime/frameworks/native/ability/native/ui_extension_base/cj_ui_extension_content_session.cpp"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t SUCCESS_CODE = 0;
constexpr int32_t ERR_INVALID_INSTANCE_CODE = -1;
constexpr int64_t SESSION_ID_1 = 1;
constexpr int64_t INVALID_SESSION_ID = -1;
}

class CJUIExtensionContentSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<CJUIExtensionContentSession> cjUIExtensionContentSession = nullptr;
};

void CJUIExtensionContentSessionTest::SetUpTestCase()
{}

void CJUIExtensionContentSessionTest::TearDownTestCase()
{}

void CJUIExtensionContentSessionTest::SetUp()
{
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    sptr<Rosen::Window> uiWindow = sptr<Rosen::Window>::MakeSptr();
    std::shared_ptr<AbilityRuntime::Context> context = std::make_shared<AbilityRuntime::ContextImpl>();
    std::weak_ptr<AbilityRuntime::Context> contextWeak = context;
    cjUIExtensionContentSession = CJUIExtensionContentSession::Create(sessionInfo, uiWindow, context);
}

void CJUIExtensionContentSessionTest::TearDown()
{
    cjUIExtensionContentSession = nullptr;
}

/**
 * @tc.name: LoadContentTest_0100
 * @tc.desc: CJUIExtensionContentSessionTest test for LoadContent.
 * @tc.type: FUNC
 */
HWTEST_F(CJUIExtensionContentSessionTest, LoadContentTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadContentTest_0100 start";
    std::string path = "testPath";
    int32_t ret = cjUIExtensionContentSession->LoadContent(path);
    EXPECT_EQ(ret, SUCCESS_CODE);
    EXPECT_TRUE(cjUIExtensionContentSession->isFirstTriggerBindModal_);

    cjUIExtensionContentSession->sessionInfo_->isAsyncModalBinding = true;
    ret = cjUIExtensionContentSession->LoadContent(path);
    EXPECT_EQ(ret, SUCCESS_CODE);
    EXPECT_FALSE(cjUIExtensionContentSession->isFirstTriggerBindModal_);

    ret = cjUIExtensionContentSession->LoadContent(path);
    EXPECT_EQ(ret, SUCCESS_CODE);
    GTEST_LOG_(INFO) << "LoadContentTest_0100 end";
}

/**
 * @tc.name: TerminateSelfWithResultTest_0100
 * @tc.desc: CJUIExtensionContentSessionTest test for TerminateSelfWithResult.
 * @tc.type: FUNC
 */
HWTEST_F(CJUIExtensionContentSessionTest, TerminateSelfWithResultTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TerminateSelfWithResultTest_0100 start";
    AAFwk::Want* want = nullptr;
    int32_t resultCode = 0;
    int32_t ret = cjUIExtensionContentSession->TerminateSelfWithResult(want, resultCode);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    cjUIExtensionContentSession->uiWindow_ = nullptr;
    ret = cjUIExtensionContentSession->TerminateSelfWithResult(want, resultCode);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    GTEST_LOG_(INFO) << "TerminateSelfWithResultTest_0100 end";
}

/**
 * @tc.name: SetWindowPrivacyModeTest_0100
 * @tc.desc: CJUIExtensionContentSessionTest test for SetWindowPrivacyMode.
 * @tc.type: FUNC
 */
HWTEST_F(CJUIExtensionContentSessionTest, SetWindowPrivacyModeTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetWindowPrivacyModeTest_0100 start";
    bool isPrivacyMode = true;
    int32_t ret = cjUIExtensionContentSession->SetWindowPrivacyMode(isPrivacyMode);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED));
    GTEST_LOG_(INFO) << "SetWindowPrivacyModeTest_0100 end";
}

/**
 * @tc.name: StartAbilityByTypeTest_0100
 * @tc.desc: CJUIExtensionContentSessionTest test for StartAbilityByType.
 * @tc.type: FUNC
 */
HWTEST_F(CJUIExtensionContentSessionTest, StartAbilityByTypeTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAbilityByTypeTest_0100 start";
    std::string type;
    AAFwk::WantParams wantParam;
    std::shared_ptr<CjUIExtensionCallback> cjUIExtensionCallback{nullptr};
    int32_t ret = cjUIExtensionContentSession->StartAbilityByType(type, wantParam, cjUIExtensionCallback);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));

    cjUIExtensionContentSession->uiWindow_ = nullptr;
    ret = cjUIExtensionContentSession->StartAbilityByType(type, wantParam, cjUIExtensionCallback);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
    GTEST_LOG_(INFO) << "StartAbilityByTypeTest_0100 end";
}

/**
 * @tc.name: FFICJExtSessionLoadContentTest_0100
 * @tc.desc: CJUIExtensionContentSessionTest test for FFICJExtSessionLoadContent.
 * @tc.type: FUNC
 */
HWTEST_F(CJUIExtensionContentSessionTest, FFICJExtSessionLoadContentTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFICJExtSessionLoadContentTest_0100 start";
    const char* path = nullptr;
    int32_t ret = FFICJExtSessionLoadContent(SESSION_ID_1, path);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);

    const char* testPath = "testPath";
    ret = FFICJExtSessionLoadContent(SESSION_ID_1, testPath);
    EXPECT_EQ(ret, SUCCESS_CODE);

    ret = FFICJExtSessionLoadContent(INVALID_SESSION_ID, testPath);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    GTEST_LOG_(INFO) << "FFICJExtSessionLoadContentTest_0100 end";
}

/**
 * @tc.name: FFICJExtSessionTerminateSelfTest_0100
 * @tc.desc: CJUIExtensionContentSessionTest test for FFICJExtSessionTerminateSelf.
 * @tc.type: FUNC
 */
HWTEST_F(CJUIExtensionContentSessionTest, FFICJExtSessionTerminateSelfTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFICJExtSessionTerminateSelfTest_0100 start";
    int32_t ret = FFICJExtSessionTerminateSelf(SESSION_ID_1);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    ret = FFICJExtSessionTerminateSelf(INVALID_SESSION_ID);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    GTEST_LOG_(INFO) << "FFICJExtSessionTerminateSelfTest_0100 end";
}

/**
 * @tc.name: FFICJExtSessionTerminateSelfWithResultTest_0100
 * @tc.desc: CJUIExtensionContentSessionTest test for FFICJExtSessionTerminateSelfWithResult.
 * @tc.type: FUNC
 */
HWTEST_F(CJUIExtensionContentSessionTest, FFICJExtSessionTerminateSelfWithResultTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFICJExtSessionTerminateSelfWithResultTest_0100 start";
    WantHandle wantHandle = nullptr;
    int32_t resultCode = 0;
    int32_t ret = FFICJExtSessionTerminateSelfWithResult(SESSION_ID_1, wantHandle, resultCode);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);

    Want want;
    wantHandle = const_cast<AAFwk::Want *>(&want);
    ret = FFICJExtSessionTerminateSelfWithResult(SESSION_ID_1, wantHandle, resultCode);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    ret = FFICJExtSessionTerminateSelfWithResult(INVALID_SESSION_ID, wantHandle, resultCode);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    GTEST_LOG_(INFO) << "FFICJExtSessionTerminateSelfWithResultTest_0100 end";
}

/**
 * @tc.name: FFICJExtSessionSetWindowPrivacyModeTest_0100
 * @tc.desc: CJUIExtensionContentSessionTest test for FFICJExtSessionSetWindowPrivacyMode.
 * @tc.type: FUNC
 */
HWTEST_F(CJUIExtensionContentSessionTest, FFICJExtSessionSetWindowPrivacyModeTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFICJExtSessionSetWindowPrivacyModeTest_0100 start";
    bool isPrivacyMode = true;
    int32_t ret = FFICJExtSessionSetWindowPrivacyMode(SESSION_ID_1, isPrivacyMode);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED));

    ret = FFICJExtSessionSetWindowPrivacyMode(INVALID_SESSION_ID, isPrivacyMode);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    GTEST_LOG_(INFO) << "FFICJExtSessionSetWindowPrivacyModeTest_0100 end";
}

/**
 * @tc.name: FFICJExtSessionStartAbilityByTypeTest_0100
 * @tc.desc: CJUIExtensionContentSessionTest test for FFICJExtSessionStartAbilityByType.
 * @tc.type: FUNC
 */
HWTEST_F(CJUIExtensionContentSessionTest, FFICJExtSessionStartAbilityByTypeTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FFICJExtSessionStartAbilityByTypeTest_0100 start";
    char* cType = nullptr;
    char* cWantParams = nullptr;
    void (*onError)(int32_t, char*, char*) = nullptr;
    void (*onResult)(CJAbilityResult) = nullptr;
    int32_t ret = FFICJExtSessionStartAbilityByType(SESSION_ID_1, cType, cWantParams, onError, onResult);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);

    cType = new char[9];
    strcpy_s(cType, 9, "testType");
    ret = FFICJExtSessionStartAbilityByType(SESSION_ID_1, cType, cWantParams, onError, onResult);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);

    cWantParams = new char[15];
    strcpy_s(cWantParams, 15, "testWantParams");
    ret = FFICJExtSessionStartAbilityByType(SESSION_ID_1, cType, cWantParams, onError, onResult);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);

    onError = [] (int32_t code, char* type, char* wantParams) {};
    ret = FFICJExtSessionStartAbilityByType(SESSION_ID_1, cType, cWantParams, onError, onResult);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);

    onResult = [] (CJAbilityResult cjAbilityResult) {};
    ret = FFICJExtSessionStartAbilityByType(SESSION_ID_1, cType, cWantParams, onError, onResult);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));

    ret = FFICJExtSessionStartAbilityByType(INVALID_SESSION_ID, cType, cWantParams, onError, onResult);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);

    delete[] cType;
    delete[] cWantParams;
    GTEST_LOG_(INFO) << "FFICJExtSessionStartAbilityByTypeTest_0100 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
