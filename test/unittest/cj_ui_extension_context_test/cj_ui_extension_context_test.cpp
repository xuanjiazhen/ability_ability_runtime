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
#define protected public
#include "cj_ui_extension_context.h"
#include "cj_ui_extension_context.cpp"
#undef private
#undef protected
#include "cj_extension_context.h"
#include "extension_base.h"
#include "ffi_remote_data.h"
#include "ui_extension_context.h"
#include "want.h"

using namespace testing::ext;
using namespace OHOS::Rosen;

namespace OHOS {
namespace AbilityRuntime {
class CJUIExtensionContextImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
void myCallbackFunction(int64_t id, int32_t status, CJAbilityResult* result)
{}

void CJUIExtensionContextImplTest::SetUpTestCase(void)
{}

void CJUIExtensionContextImplTest::TearDownTestCase(void)
{}

void CJUIExtensionContextImplTest::SetUp()
{}

void CJUIExtensionContextImplTest::TearDown()
{}

/**
 * @tc.number: StartAbility_0100
 * @tc.name: StartAbility
 * @tc.desc: StartAbility.
 */
HWTEST_F(CJUIExtensionContextImplTest, StartAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0100 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    Want want;
    auto ret = extensionContext.StartAbility(want);
    auto error = static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
    EXPECT_NE(ret, error);
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0100 end");
}

/**
 * @tc.number: StartAbility_0200
 * @tc.name: StartAbility
 * @tc.desc: StartAbility.
 */
HWTEST_F(CJUIExtensionContextImplTest, StartAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0200 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    auto ret = extensionContext.StartAbility(want, startOptions);
    auto error = static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
    EXPECT_NE(ret, error);
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0200 end");
}

/**
 * @tc.number: StartAbility_0300
 * @tc.name: StartAbility
 * @tc.desc: StartAbility.
 */
HWTEST_F(CJUIExtensionContextImplTest, StartAbility_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0300 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    Want want;
    extensionContext.context_.reset();
    auto ret = extensionContext.StartAbility(want);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0300 end");
}

/**
 * @tc.number: StartAbility_0400
 * @tc.name: StartAbility
 * @tc.desc: StartAbility.
 */
HWTEST_F(CJUIExtensionContextImplTest, StartAbility_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0400 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    Want want;
    AAFwk::StartOptions startOptions;
    extensionContext.context_.reset();
    auto ret = extensionContext.StartAbility(want, startOptions);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    TAG_LOGI(AAFwkTag::TEST, "StartAbility_0400 end");
}

/**
 * @tc.number: StartAbilityForRes_0100
 * @tc.name: StartAbilityForRes
 * @tc.desc: Start a ability for result.
 */
HWTEST_F(CJUIExtensionContextImplTest, StartAbilityForRes_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForRes_0100 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    AAFwk::Want want;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    AAFwk::StartOptions startOptions;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    int32_t requestCode = 1;
    extensionContext.context_.reset();
    auto ret = extensionContext.StartAbilityForRes(want, startOptions, requestCode, cbInfo);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForRes_0100 end");
}

/**
 * @tc.number: StartAbilityForRes_0200
 * @tc.name: StartAbilityForRes
 * @tc.desc: Start a ability for result.
 */
HWTEST_F(CJUIExtensionContextImplTest, StartAbilityForRes_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForRes_0200 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    AAFwk::Want want;
    int32_t requestCode = 1;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    AAFwk::StartOptions startOptions;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    extensionContext.context_.reset();
    auto ret = extensionContext.StartAbilityForRes(want, requestCode, cbInfo);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForRes_0200 end");
}

/**
 * @tc.number: StartAbilityForRes_0300
 * @tc.name: StartAbilityForRes
 * @tc.desc: Start a ability for result.
 */
HWTEST_F(CJUIExtensionContextImplTest, StartAbilityForRes_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForRes_0300 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    AAFwk::Want want;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    AAFwk::StartOptions startOptions;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    int32_t requestCode = 1;
    auto ret = extensionContext.StartAbilityForRes(want, startOptions, requestCode, cbInfo);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForRes_0300 end");
}

/**
 * @tc.number: StartAbilityForRes_0400
 * @tc.name: StartAbilityForRes
 * @tc.desc: Start a ability for result.
 */
HWTEST_F(CJUIExtensionContextImplTest, StartAbilityForRes_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForRes_0400 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    AAFwk::Want want;
    int32_t requestCode = 1;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    AAFwk::StartOptions startOptions;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = extensionContext.StartAbilityForRes(want, requestCode, cbInfo);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityForRes_0400 end");
}

/**
 * @tc.number: ConnectServiceExtensionAbility_0100
 * @tc.name: ConnectServiceExtensionAbility
 * @tc.desc: ConnectServiceExtensionAbility
 */
HWTEST_F(CJUIExtensionContextImplTest, ConnectServiceExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServiceExtensionAbility_0100 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    AAFwk::Want want;
    int64_t connectOptId = 123456;
    int64_t connectionId = 123456789;
    auto ret = extensionContext.ConnectServiceExtensionAbility(want, connectOptId, connectionId);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ConnectServiceExtensionAbility_0100 end");
}

/**
 * @tc.number: ConnectServiceExtensionAbility_0200
 * @tc.name: ConnectServiceExtensionAbility
 * @tc.desc: ConnectServiceExtensionAbility
 */
HWTEST_F(CJUIExtensionContextImplTest, ConnectServiceExtensionAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectServiceExtensionAbility_0200 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    AAFwk::Want want;
    int64_t connectOptId = 123456;
    int64_t connectionId = 123456789;
    extensionContext.context_.reset();
    auto ret = extensionContext.ConnectServiceExtensionAbility(want, connectOptId, connectionId);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    TAG_LOGI(AAFwkTag::TEST, "ConnectServiceExtensionAbility_0200 end");
}

/**
 * @tc.number: DisconnectServiceExtensionAbility_0100
 * @tc.name: DisconnectServiceExtensionAbility
 * @tc.desc: DisconnectServiceExtensionAbility
 */
HWTEST_F(CJUIExtensionContextImplTest, DisconnectServiceExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DisconnectServiceExtensionAbility_0100 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    int64_t connectionId = 123456789;
    auto ret = extensionContext.DisconnectServiceExtensionAbility(connectionId);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "DisconnectServiceExtensionAbility_0100 end");
}

/**
 * @tc.number: TerminateSelf_0100
 * @tc.name: TerminateSelf
 * @tc.desc: TerminateSelf.
 */
HWTEST_F(CJUIExtensionContextImplTest, TerminateSelf_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0100 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    auto ret = extensionContext.TerminateSelf();
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0100 end");
}

/**
 * @tc.number: TerminateSelf_0200
 * @tc.name: TerminateSelf
 * @tc.desc: TerminateSelf.
 */
HWTEST_F(CJUIExtensionContextImplTest, TerminateSelf_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0200 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    extensionContext.context_.reset();
    auto ret = extensionContext.TerminateSelf();
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelf_0200 end");
}

/**
 * @tc.number: TerminateSelfWithResult_0100
 * @tc.name: TerminateSelfWithResult
 * @tc.desc: TerminateSelfWithResult.
 */
HWTEST_F(CJUIExtensionContextImplTest, TerminateSelfWithResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfWithResult_0100 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    AAFwk::Want want;
    int32_t resultCode = 123456789;
    auto ret = extensionContext.TerminateSelfWithResult(want, resultCode);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfWithResult_0100 end");
}

/**
 * @tc.number: TerminateSelfWithResult_0200
 * @tc.name: TerminateSelfWithResult
 * @tc.desc: TerminateSelfWithResult.
 */
HWTEST_F(CJUIExtensionContextImplTest, TerminateSelfWithResult_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfWithResult_0200 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    AAFwk::Want want;
    int32_t resultCode = 123456789;
    extensionContext.context_.reset();
    auto ret = extensionContext.TerminateSelfWithResult(want, resultCode);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    TAG_LOGI(AAFwkTag::TEST, "TerminateSelfWithResult_0200 end");
}

/**
 * @tc.number: ReportDrawnCompleted_0100
 * @tc.name: ReportDrawnCompleted
 * @tc.desc: ReportDrawnCompleted.
 */
HWTEST_F(CJUIExtensionContextImplTest, ReportDrawnCompleted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReportDrawnCompleted_0100 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    auto ret = extensionContext.ReportDrawnCompleted();
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ReportDrawnCompleted_0100 end");
}

/**
 * @tc.number: ReportDrawnCompleted_0200
 * @tc.name: ReportDrawnCompleted
 * @tc.desc: ReportDrawnCompleted.
 */
HWTEST_F(CJUIExtensionContextImplTest, ReportDrawnCompleted_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReportDrawnCompleted_0200 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    extensionContext.context_.reset();
    auto ret = extensionContext.ReportDrawnCompleted();
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    TAG_LOGI(AAFwkTag::TEST, "ReportDrawnCompleted_0200 end");
}

/**
 * @tc.number: OpenAtomicService_0100
 * @tc.name: OpenAtomicService
 * @tc.desc: OpenAtomicService.
 */
HWTEST_F(CJUIExtensionContextImplTest, OpenAtomicService_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenAtomicService_0100 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    std::string appid;
    AAFwk::Want want;
    int32_t requestCode = 1;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    AAFwk::StartOptions startOptions;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = extensionContext.OpenAtomicService(appid, startOptions, want, requestCode, cbInfo);
    EXPECT_EQ(ret, SUCCESS_CODE);
    TAG_LOGI(AAFwkTag::TEST, "OpenAtomicService_0100 end");
}

/**
 * @tc.number: OpenAtomicService_0200
 * @tc.name: OpenAtomicService
 * @tc.desc: OpenAtomicService.
 */
HWTEST_F(CJUIExtensionContextImplTest, OpenAtomicService_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenAtomicService_0200 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    std::string appid;
    AAFwk::Want want;
    int32_t requestCode = 1;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    AAFwk::StartOptions startOptions;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    extensionContext.context_.reset();
    auto ret = extensionContext.OpenAtomicService(appid, startOptions, want, requestCode, cbInfo);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    TAG_LOGI(AAFwkTag::TEST, "OpenAtomicService_0200 end");
}

/**
 * @tc.number: OpenLink_0100
 * @tc.name: OpenLink
 * @tc.desc: OpenLink.
 */
HWTEST_F(CJUIExtensionContextImplTest, OpenLink_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0100 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    std::string link = "link://www.test.com:888/test1";
    AAFwk::Want want;
    int32_t requestCode = 1;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    AAFwk::OpenLinkOptions Options;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    extensionContext.context_.reset();
    auto ret = extensionContext.OpenLink(link, Options, want, requestCode, cbInfo);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0100 end");
}

/**
 * @tc.number: OpenLink_0200
 * @tc.name: OpenLink
 * @tc.desc: OpenLink.
 */
HWTEST_F(CJUIExtensionContextImplTest, OpenLink_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0200 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    std::string link;
    AAFwk::Want want;
    int32_t requestCode = 1;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    AAFwk::OpenLinkOptions Options;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = extensionContext.OpenLink(link, Options, want, requestCode, cbInfo);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0200 end");
}

/**
 * @tc.number: OpenLink_0300
 * @tc.name: OpenLink
 * @tc.desc: OpenLink.
 */
HWTEST_F(CJUIExtensionContextImplTest, OpenLink_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0300 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    std::string link = "link://www.test.com:888/test1";
    AAFwk::Want want;
    int32_t requestCode = 1;
    int64_t lambdaId = 0;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    AAFwk::OpenLinkOptions Options;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = extensionContext.OpenLink(link, Options, want, requestCode, cbInfo);
    EXPECT_NE(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_OK));
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0300 end");
}

/**
 * @tc.number: OpenLink_0400
 * @tc.name: OpenLink
 * @tc.desc: OpenLink.
 */
HWTEST_F(CJUIExtensionContextImplTest, OpenLink_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0400 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    std::string link = "link://www.test.com:888/test1";
    AAFwk::Want want;
    int32_t requestCode = 1;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    AAFwk::OpenLinkOptions Options;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = extensionContext.OpenLink(link, Options, want, requestCode, cbInfo);
    EXPECT_NE(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_OK));
    TAG_LOGI(AAFwkTag::TEST, "OpenLink_0400 end");
}

/**
 * @tc.number: OpenLinkInner_0100
 * @tc.name: OpenLinkInner
 * @tc.desc: OpenLinkInner.
 */
HWTEST_F(CJUIExtensionContextImplTest, OpenLinkInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkInner_0100 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    std::string link = "link://www.test.com:888/test1";
    AAFwk::Want want;
    int32_t requestCode = 1;
    auto ret = extensionContext.OpenLinkInner(want, requestCode, startTime, link);
    EXPECT_NE(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_OK));
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkInner_0100 end");
}

/**
 * @tc.number: OpenLinkInner_0200
 * @tc.name: OpenLinkInner
 * @tc.desc: OpenLinkInner.
 */
HWTEST_F(CJUIExtensionContextImplTest, OpenLinkInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkInner_0200 start");
    std::shared_ptr<UIExtensionContext> context = std::make_shared<UIExtensionContext>();
    CJUIExtensionContextImpl extensionContext(context);
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    std::string link = "link://www.test.com:888/test1";
    AAFwk::Want want;
    int32_t requestCode = 1;
    extensionContext.context_.reset();
    auto ret = extensionContext.OpenLinkInner(want, requestCode, startTime, link);
    EXPECT_EQ(ret, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    TAG_LOGI(AAFwkTag::TEST, "OpenLinkInner_0200 end");
}

/**
 * @tc.number: FFICJUIExtAbilityGetContext_0100
 * @tc.name: FFICJUIExtAbilityGetContext
 * @tc.desc: FFICJUIExtAbilityGetContext.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtAbilityGetContext_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtAbilityGetContext_0100 start");
    ExtAbilityHandle handle = nullptr;
    int64_t existing_id = 123456789012345;
    int64_t* id = &existing_id;
    auto ret = FFICJUIExtAbilityGetContext(handle, id);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtAbilityGetContext_0100 end");
}

/**
 * @tc.number: FFICJUIExtAbilityGetContext_0200
 * @tc.name: FFICJUIExtAbilityGetContext
 * @tc.desc: FFICJUIExtAbilityGetContext.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtAbilityGetContext_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtAbilityGetContext_0200 start");
    int* intPtr = new int(0);
    ExtAbilityHandle handle = static_cast<ExtAbilityHandle>(intPtr);
    int64_t* id = nullptr;
    auto ret = FFICJUIExtAbilityGetContext(handle, id);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtAbilityGetContext_0200 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbility_0100
 * @tc.name: FFICJUIExtCtxStartAbility
 * @tc.desc: FFICJUIExtCtxStartAbility.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbility_0100 start");
    
    int* intPtr = new int(0);
    WantHandle want = static_cast<WantHandle>(intPtr);
    int64_t id = 0;
    auto ret = FFICJUIExtCtxStartAbility(id, want);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbility_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbility_0200
 * @tc.name: FFICJUIExtCtxStartAbility
 * @tc.desc: FFICJUIExtCtxStartAbility.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbility_0200 start");
    
    int* intPtr = nullptr;
    WantHandle want = static_cast<WantHandle>(intPtr);
    int64_t id = 1;
    auto ret = FFICJUIExtCtxStartAbility(id, want);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbility_0200 end");
}

/**
 * @tc.number: UnWrapStartOption_0100
 * @tc.name: UnWrapStartOption
 * @tc.desc: UnWrapStartOption.
 */
HWTEST_F(CJUIExtensionContextImplTest, UnWrapStartOption_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnWrapStartOption_0100 start");
    CJNewStartOptions newStartoptions;
    newStartoptions.windowMode = 1;
    newStartoptions.displayId = 2;
    newStartoptions.withAnimation = true;
    newStartoptions.windowLeft = 3;
    newStartoptions.windowTop = 4;
    newStartoptions.windowWidth = 5;
    newStartoptions.windowHeight = 6;
    AAFwk::StartOptions option;
    option.SetWindowMode(1);
    option.SetDisplayID(2);
    option.SetWithAnimation(true);
    option.SetWindowLeft(3);
    option.windowLeftUsed_ = true;
    option.SetWindowTop(4);
    option.windowTopUsed_ = true;
    option.SetWindowWidth(5);
    option.windowWidthUsed_ = true;
    option.SetWindowHeight(6);
    option.windowHeightUsed_ = true;
    AAFwk::StartOptions option2 = UnWrapStartOption(&newStartoptions);
    EXPECT_EQ(option2.windowMode_, option.windowMode_);
    EXPECT_EQ(option2.displayId_, option.displayId_);
    EXPECT_EQ(option2.withAnimation_, option.withAnimation_);
    EXPECT_EQ(option2.windowLeft_, option.windowLeft_);
    EXPECT_EQ(option2.windowTop_, option.windowTop_);
    EXPECT_EQ(option2.windowWidth_, option.windowWidth_);
    EXPECT_EQ(option2.windowHeight_, option.windowHeight_);
    TAG_LOGI(AAFwkTag::TEST, "UnWrapStartOption_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbilityWithOpt_0100
 * @tc.name: FFICJUIExtCtxStartAbilityWithOpt
 * @tc.desc: FFICJUIExtCtxStartAbilityWithOpt.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbilityWithOpt_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityWithOpt_0100 start");
    CJNewStartOptions newStartoptions;
    newStartoptions.windowMode = 1;
    newStartoptions.displayId = 2;
    newStartoptions.withAnimation = true;
    newStartoptions.windowLeft = 3;
    newStartoptions.windowTop = 4;
    newStartoptions.windowWidth = 5;
    newStartoptions.windowHeight = 6;
    int* intPtr = nullptr;
    WantHandle want = static_cast<WantHandle>(intPtr);
    int64_t id = 1;
    auto ret = FFICJUIExtCtxStartAbilityWithOpt(id, want, &newStartoptions);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityWithOpt_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbilityWithOpt_0200
 * @tc.name: FFICJUIExtCtxStartAbilityWithOpt
 * @tc.desc: FFICJUIExtCtxStartAbilityWithOpt.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbilityWithOpt_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityWithOpt_0200 start");
    int* intPtr = new int(0);
    WantHandle want = static_cast<WantHandle>(intPtr);
    int64_t id = 1;
    auto ret = FFICJUIExtCtxStartAbilityWithOpt(id, want, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityWithOpt_0200 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbilityWithOpt_0300
 * @tc.name: FFICJUIExtCtxStartAbilityWithOpt
 * @tc.desc: FFICJUIExtCtxStartAbilityWithOpt.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbilityWithOpt_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityWithOpt_0300 start");
    CJNewStartOptions newStartoptions;
    newStartoptions.windowMode = 1;
    newStartoptions.displayId = 2;
    newStartoptions.withAnimation = true;
    newStartoptions.windowLeft = 3;
    newStartoptions.windowTop = 4;
    newStartoptions.windowWidth = 5;
    newStartoptions.windowHeight = 6;
    int* intPtr = new int(0);
    WantHandle want = static_cast<WantHandle>(intPtr);
    int64_t id = 1;
    auto ret = FFICJUIExtCtxStartAbilityWithOpt(id, want, &newStartoptions);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityWithOpt_0300 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbilityForRes_0100
 * @tc.name: FFICJUIExtCtxStartAbilityForRes
 * @tc.desc: FFICJUIExtCtxStartAbilityForRes.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbilityForRes_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForRes_0100 start");
    int64_t id = 1;
    int32_t requestCode = 123;
    int* intPtr = nullptr;
    WantHandle want = static_cast<WantHandle>(intPtr);
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = FFICJUIExtCtxStartAbilityForRes(id, want, requestCode, &cbInfo);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForRes_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbilityForRes_0200
 * @tc.name: FFICJUIExtCtxStartAbilityForRes
 * @tc.desc: FFICJUIExtCtxStartAbilityForRes.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbilityForRes_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForRes_0200 start");
    int64_t id = 1;
    int32_t requestCode = 123;
    int* intPtr = new int(0);
    WantHandle want = static_cast<WantHandle>(intPtr);
    auto ret = FFICJUIExtCtxStartAbilityForRes(id, want, requestCode, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForRes_0200 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbilityForRes_0300
 * @tc.name: FFICJUIExtCtxStartAbilityForRes
 * @tc.desc: FFICJUIExtCtxStartAbilityForRes.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbilityForRes_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForRes_0300 start");
    int64_t id = 1;
    int32_t requestCode = 123;
    int* intPtr = new int(0);
    WantHandle want = static_cast<WantHandle>(intPtr);
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = FFICJUIExtCtxStartAbilityForRes(id, want, requestCode, &cbInfo);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForRes_0300 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbilityForResWithOpt_0100
 * @tc.name: FFICJUIExtCtxStartAbilityForResWithOpt
 * @tc.desc: FFICJUIExtCtxStartAbilityForResWithOpt.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbilityForResWithOpt_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForResWithOpt_0100 start");
    int64_t id = 1;
    int32_t requestCode = 123;
    int* intPtr = nullptr;
    WantHandle want = static_cast<WantHandle>(intPtr);
    CJNewStartOptions paramOpt;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = FFICJUIExtCtxStartAbilityForResWithOpt(id, want, &paramOpt, requestCode, &cbInfo);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForResWithOpt_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbilityForResWithOpt_0200
 * @tc.name: FFICJUIExtCtxStartAbilityForResWithOpt
 * @tc.desc: FFICJUIExtCtxStartAbilityForResWithOpt.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbilityForResWithOpt_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForResWithOpt_0200 start");
    int64_t id = 1;
    int32_t requestCode = 123;
    int* intPtr = new int(0);
    WantHandle want = static_cast<WantHandle>(intPtr);
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = FFICJUIExtCtxStartAbilityForResWithOpt(id, want, nullptr, requestCode, &cbInfo);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForResWithOpt_0200 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbilityForResWithOpt_0300
 * @tc.name: FFICJUIExtCtxStartAbilityForResWithOpt
 * @tc.desc: FFICJUIExtCtxStartAbilityForResWithOpt.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbilityForResWithOpt_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForResWithOpt_0300 start");
    int64_t id = 1;
    int32_t requestCode = 123;
    int* intPtr = new int(0);
    WantHandle want = static_cast<WantHandle>(intPtr);
    CJNewStartOptions paramOpt;
    auto ret = FFICJUIExtCtxStartAbilityForResWithOpt(id, want, &paramOpt, requestCode, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForResWithOpt_0300 end");
}

/**
 * @tc.number: FFICJUIExtCtxStartAbilityForResWithOpt_0400
 * @tc.name: FFICJUIExtCtxStartAbilityForResWithOpt
 * @tc.desc: FFICJUIExtCtxStartAbilityForResWithOpt.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxStartAbilityForResWithOpt_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForResWithOpt_0400 start");
    int64_t id = 1;
    int32_t requestCode = 123;
    int* intPtr = new int(0);
    WantHandle want = static_cast<WantHandle>(intPtr);
    CJNewStartOptions paramOpt;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = FFICJUIExtCtxStartAbilityForResWithOpt(id, want, &paramOpt, requestCode, &cbInfo);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxStartAbilityForResWithOpt_0400 end");
}

/**
 * @tc.number: FFICJUIExtCtxConnectServiceExtensionAbility_0100
 * @tc.name: FFICJUIExtCtxConnectServiceExtensionAbility
 * @tc.desc: FFICJUIExtCtxConnectServiceExtensionAbility.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxConnectServiceExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxConnectServiceExtensionAbility_0100 start");
    int64_t id = 1;
    int64_t connectOptId = 123;
    int64_t* connectionId = new int64_t(0);
    WantHandle want = static_cast<WantHandle>(nullptr);
    auto ret = FFICJUIExtCtxConnectServiceExtensionAbility(id, want, connectOptId, connectionId);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxConnectServiceExtensionAbility_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxConnectServiceExtensionAbility_0200
 * @tc.name: FFICJUIExtCtxConnectServiceExtensionAbility
 * @tc.desc: FFICJUIExtCtxConnectServiceExtensionAbility.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxConnectServiceExtensionAbility_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxConnectServiceExtensionAbility_0200 start");
    int64_t id = 1;
    int64_t connectOptId = 123;
    int64_t* connectionId = new int64_t(0);
    WantHandle want = static_cast<WantHandle>(connectionId);
    auto ret = FFICJUIExtCtxConnectServiceExtensionAbility(id, want, connectOptId, connectionId);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxConnectServiceExtensionAbility_0200 end");
}

/**
 * @tc.number: FFICJUIExtCtxConnectServiceExtensionAbility_0300
 * @tc.name: FFICJUIExtCtxConnectServiceExtensionAbility
 * @tc.desc: FFICJUIExtCtxConnectServiceExtensionAbility.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxConnectServiceExtensionAbility_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxConnectServiceExtensionAbility_0300 start");
    int64_t id = 1;
    int64_t connectOptId = 123;
    int64_t* connectionId = nullptr;
    WantHandle want = static_cast<WantHandle>(&connectionId);
    auto ret = FFICJUIExtCtxConnectServiceExtensionAbility(id, want, connectOptId, connectionId);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxConnectServiceExtensionAbility_0300 end");
}

/**
 * @tc.number: FFICJUIExtCtxDisconnectServiceExtensionAbility_0100
 * @tc.name: FFICJUIExtCtxDisconnectServiceExtensionAbility
 * @tc.desc: FFICJUIExtCtxDisconnectServiceExtensionAbility.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxDisconnectServiceExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxDisconnectServiceExtensionAbility_0100 start");
    int64_t id = 1;
    int64_t connectionId = 123;
    auto ret = FFICJUIExtCtxDisconnectServiceExtensionAbility(id, connectionId);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxDisconnectServiceExtensionAbility_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxTerminateSelf_0100
 * @tc.name: FFICJUIExtCtxTerminateSelf
 * @tc.desc: FFICJUIExtCtxTerminateSelf.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxTerminateSelf_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxTerminateSelf_0100 start");
    int64_t id = 1;
    auto ret = FFICJUIExtCtxTerminateSelf(id);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxTerminateSelf_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxTerminateSelfWithResult_0100
 * @tc.name: FFICJUIExtCtxTerminateSelfWithResult
 * @tc.desc: FFICJUIExtCtxTerminateSelfWithResult.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxTerminateSelfWithResult_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxTerminateSelfWithResult_0100 start");
    int64_t id = 1;
    int32_t resultCode = 1;
    auto ret = FFICJUIExtCtxTerminateSelfWithResult(id, nullptr, resultCode);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxTerminateSelfWithResult_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxTerminateSelfWithResult_0200
 * @tc.name: FFICJUIExtCtxTerminateSelfWithResult
 * @tc.desc: FFICJUIExtCtxTerminateSelfWithResult.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxTerminateSelfWithResult_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxTerminateSelfWithResult_0200 start");
    int64_t id = 1;
    int32_t resultCode = 1;
    int64_t* connectionId = new int64_t(0);
    WantHandle want = static_cast<WantHandle>(&connectionId);
    auto ret = FFICJUIExtCtxTerminateSelfWithResult(id, want, resultCode);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxTerminateSelfWithResult_0200 end");
}

/**
 * @tc.number: FFICJUIExtCtxReportDrawnCompleted_0100
 * @tc.name: FFICJUIExtCtxReportDrawnCompleted
 * @tc.desc: FFICJUIExtCtxReportDrawnCompleted.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxReportDrawnCompleted_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxReportDrawnCompleted_0100 start");
    int64_t id = 1;
    auto ret = FFICJUIExtCtxReportDrawnCompleted(id);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxReportDrawnCompleted_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxOpenAtomicService_0100
 * @tc.name: FFICJUIExtCtxOpenAtomicService
 * @tc.desc: FFICJUIExtCtxOpenAtomicService.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxOpenAtomicService_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenAtomicService_0100 start");
    int64_t id = 1;
    char* cAppId = nullptr;
    bool hasValue = true;
    int32_t flags = 9;
    char* parameters = nullptr;
    CJNewStartOptions startOptions;
    CJAtomicServiceOptions cAtomicServiceOptions = {
        hasValue,
        flags,
        parameters,
        startOptions
    };
    int32_t requestCode = 123;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = FFICJUIExtCtxOpenAtomicService(id, cAppId, &cAtomicServiceOptions, requestCode, &cbInfo);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenAtomicService_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxOpenAtomicService_0200
 * @tc.name: FFICJUIExtCtxOpenAtomicService
 * @tc.desc: FFICJUIExtCtxOpenAtomicService.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxOpenAtomicService_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenAtomicService_0200 start");
    int64_t id = 1;
    char appIdArray[] = "MyAppID123";
    char* cAppId = appIdArray;
    int32_t requestCode = 123;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = FFICJUIExtCtxOpenAtomicService(id, cAppId, nullptr, requestCode, &cbInfo);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenAtomicService_0200 end");
}

/**
 * @tc.number: FFICJUIExtCtxOpenAtomicService_0300
 * @tc.name: FFICJUIExtCtxOpenAtomicService
 * @tc.desc: FFICJUIExtCtxOpenAtomicService.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxOpenAtomicService_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenAtomicService_0300 start");
    int64_t id = 1;
    char appIdArray[] = "MyAppID123";
    char* cAppId = appIdArray;
    bool hasValue = true;
    int32_t flags = 9;
    char* parameters = nullptr;
    CJNewStartOptions cJstartOptions;
    CJAtomicServiceOptions cAtomicServiceOptions = {
        hasValue,
        flags,
        parameters,
        cJstartOptions
    };
    int32_t requestCode = 123;
    auto ret = FFICJUIExtCtxOpenAtomicService(id, cAppId, &cAtomicServiceOptions, requestCode, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenAtomicService_0300 end");
}

/**
 * @tc.number: FFICJUIExtCtxOpenAtomicService_0400
 * @tc.name: FFICJUIExtCtxOpenAtomicService
 * @tc.desc: FFICJUIExtCtxOpenAtomicService.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxOpenAtomicService_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenAtomicService_0400 start");
    int64_t id = 1;
    char appIdArray[] = "MyAppID123";
    char* cAppId = appIdArray;
    bool hasValue = true;
    int32_t flags = 9;
    char* parameters = nullptr;
    CJNewStartOptions cJstartOptions;
    CJAtomicServiceOptions cAtomicServiceOptions = {
        hasValue,
        flags,
        parameters,
        cJstartOptions
    };
    int32_t requestCode = 123;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = FFICJUIExtCtxOpenAtomicService(id, cAppId, &cAtomicServiceOptions, requestCode, &cbInfo);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenAtomicService_0400 end");
}

/**
 * @tc.number: UnwrapOpenLinkOptions_0100
 * @tc.name: UnwrapOpenLinkOptions
 * @tc.desc: UnwrapOpenLinkOptions.
 */
HWTEST_F(CJUIExtensionContextImplTest, UnwrapOpenLinkOptions_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnwrapOpenLinkOptions_0100 start");
    char* parameters = nullptr;
    bool hasValue = false;
    bool appLinkingOnly = true;
    Want want;
    CJOpenLinkOptions cOpenLinkOptions = {
        hasValue,
        appLinkingOnly,
        parameters
    };
    auto ret = UnwrapOpenLinkOptions(&cOpenLinkOptions, want);
    EXPECT_EQ(ret.appLinkingOnly_, false);
    TAG_LOGI(AAFwkTag::TEST, "UnwrapOpenLinkOptions_0100 end");
}

/**
 * @tc.number: UnwrapOpenLinkOptions_0200
 * @tc.name: UnwrapOpenLinkOptions
 * @tc.desc: UnwrapOpenLinkOptions.
 */
HWTEST_F(CJUIExtensionContextImplTest, UnwrapOpenLinkOptions_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnwrapOpenLinkOptions_0200 start");
    char* parameters = nullptr;
    bool hasValue = true;
    bool appLinkingOnly = false;
    Want want;
    CJOpenLinkOptions cOpenLinkOptions = {
        hasValue,
        appLinkingOnly,
        parameters
    };
    auto ret = UnwrapOpenLinkOptions(&cOpenLinkOptions, want);
    EXPECT_EQ(ret.appLinkingOnly_, false);
    TAG_LOGI(AAFwkTag::TEST, "UnwrapOpenLinkOptions_0200 end");
}

/**
 * @tc.number: UnwrapOpenLinkOptions_0300
 * @tc.name: UnwrapOpenLinkOptions
 * @tc.desc: UnwrapOpenLinkOptions.
 */
HWTEST_F(CJUIExtensionContextImplTest, UnwrapOpenLinkOptions_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnwrapOpenLinkOptions_0300 start");
    char parameter[] =  R"(
        "key1"="value1"
    )";
    char* parameters = parameter;
    bool hasValue = true;
    bool appLinkingOnly = true;
    Want want;
    CJOpenLinkOptions cOpenLinkOptions = {
        hasValue,
        appLinkingOnly,
        parameters
    };
    AAFwk::WantParams wantParams = AAFwk::WantParamWrapper::ParseWantParamsWithBrackets(parameters);
    auto ret = UnwrapOpenLinkOptions(&cOpenLinkOptions, want);
    EXPECT_EQ(ret.appLinkingOnly_, appLinkingOnly);
    TAG_LOGI(AAFwkTag::TEST, "UnwrapOpenLinkOptions_0300 end");
}

/**
 * @tc.number: FFICJUIExtCtxOpenLink_0100
 * @tc.name: FFICJUIExtCtxOpenLink
 * @tc.desc: FFICJUIExtCtxOpenLink.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxOpenLink_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenLink_0100 start");
    int64_t id = 1;
    char parametersArray[] = "parametersArray";
    char* cLink = nullptr;
    char* parameters = parametersArray;
    bool hasValue = false;
    bool appLinkingOnly = true;
    CJOpenLinkOptions cOpenLinkOptions = {
        hasValue,
        appLinkingOnly,
        parameters
    };
    int32_t requestCode = 123;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = FFICJUIExtCtxOpenLink(id, cLink, &cOpenLinkOptions, requestCode, &cbInfo);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenLink_0100 end");
}

/**
 * @tc.number: FFICJUIExtCtxOpenLink_0200
 * @tc.name: FFICJUIExtCtxOpenLink
 * @tc.desc: FFICJUIExtCtxOpenLink.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxOpenLink_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenLink_0200 start");
    int64_t id = 1;
    char cLinkArray[] = "cLink";
    char* cLink = cLinkArray;
    int32_t requestCode = 123;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = FFICJUIExtCtxOpenLink(id, cLink, nullptr, requestCode, &cbInfo);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenLink_0200 end");
}

/**
 * @tc.number: FFICJUIExtCtxOpenLink_0300
 * @tc.name: FFICJUIExtCtxOpenLink
 * @tc.desc: FFICJUIExtCtxOpenLink.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxOpenLink_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenLink_0300 start");
    int64_t id = 1;
    char cLinkArray[] = "cLink";
    char parametersArray[] = "parametersArray";
    char* cLink = cLinkArray;
    char* parameters = parametersArray;
    bool hasValue = false;
    bool appLinkingOnly = true;
    CJOpenLinkOptions cOpenLinkOptions = {
        hasValue,
        appLinkingOnly,
        parameters
    };
    int32_t requestCode = 123;
    auto ret = FFICJUIExtCtxOpenLink(id, cLink, &cOpenLinkOptions, requestCode, nullptr);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenLink_0300 end");
}

/**
 * @tc.number: FFICJUIExtCtxOpenLink_0400
 * @tc.name: FFICJUIExtCtxOpenLink
 * @tc.desc: FFICJUIExtCtxOpenLink.
 */
HWTEST_F(CJUIExtensionContextImplTest, FFICJUIExtCtxOpenLink_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenLink_0400 start");
    int64_t id = 1;
    char cLinkArray[] = "cLink";
    char parametersArray[] = "parametersArray";
    char* cLink = cLinkArray;
    char* parameters = parametersArray;
    bool hasValue = false;
    bool appLinkingOnly = true;
    CJOpenLinkOptions cOpenLinkOptions = {
        hasValue,
        appLinkingOnly,
        parameters
    };
    int32_t requestCode = 123;
    int64_t lambdaId = 123456789;
    CJAbilityResultCbFn callbackFn = myCallbackFunction;
    CJAbilityResultCbInfo cbInfo= {
        lambdaId,
        callbackFn
    };
    auto ret = FFICJUIExtCtxOpenLink(id, cLink, &cOpenLinkOptions, requestCode, &cbInfo);
    EXPECT_EQ(ret, ERR_INVALID_INSTANCE_CODE);
    TAG_LOGI(AAFwkTag::TEST, "FFICJUIExtCtxOpenLink_0400 end");
}
} // namespace AbilityRuntime
} // namespace OHOS
