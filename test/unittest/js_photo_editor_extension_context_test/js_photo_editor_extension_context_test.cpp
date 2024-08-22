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

#define private public
#define protected public
#include "js_photo_editor_extension_context.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_context.h"
#include "napi/native_api.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "pixel_map_napi.h"
#include "session_info.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace testing::ext;
class JsPhotoEditorExtensionContextTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void JsPhotoEditorExtensionContextTest::SetUpTestCase(void)
{}

void JsPhotoEditorExtensionContextTest::TearDownTestCase(void)
{}

void JsPhotoEditorExtensionContextTest::SetUp(void)
{}

void JsPhotoEditorExtensionContextTest::TearDown(void)
{}

/**
 * @tc.number: FinalizerTest_0100
 * @tc.name: Finalizer
 * @tc.desc: Js photo editor extension context Finalizer.
 */
HWTEST_F(JsPhotoEditorExtensionContextTest, FinalizerTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FinalizerTest_0100 start");
    std::shared_ptr<PhotoEditorExtensionContext> context;
    auto jsContext = std::make_shared<JsPhotoEditorExtensionContext>(context);
    napi_env env = nullptr;
    void *data = nullptr;
    void *hint = nullptr;
    jsContext->Finalizer(env, data, hint);
    TAG_LOGI(AAFwkTag::TEST, "FinalizerTest_0100 end");
}

/**
 * @tc.number: CreateJsPhotoEditorExtensionContextTest_0100
 * @tc.name: CreateJsPhotoEditorExtensionContext
 * @tc.desc: Js photo editor extension context CreateJsPhotoEditorExtensionContext.
 */
HWTEST_F(JsPhotoEditorExtensionContextTest, CreateJsPhotoEditorExtensionContextTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateJsPhotoEditorExtensionContextTest_0100 start");
    std::shared_ptr<PhotoEditorExtensionContext> context;
    auto jsContext = std::make_shared<JsPhotoEditorExtensionContext>(context);
    napi_env env = nullptr;
    jsContext->CreateJsPhotoEditorExtensionContext(env, context);
    TAG_LOGI(AAFwkTag::TEST, "CreateJsPhotoEditorExtensionContextTest_0100 end");
}

/**
 * @tc.number: SaveEditedContentWithUriTest_0100
 * @tc.name: SaveEditedContentWithUri
 * @tc.desc: Js photo editor extension context SaveEditedContentWithUri.
 */
HWTEST_F(JsPhotoEditorExtensionContextTest, SaveEditedContentWithUriTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SaveEditedContentWithUriTest_0100 start");
    std::shared_ptr<PhotoEditorExtensionContext> context;
    auto jsContext = std::make_shared<JsPhotoEditorExtensionContext>(context);
    napi_env env = nullptr;
    napi_callback_info info = nullptr;
    jsContext->SaveEditedContentWithUri(env, info);
    TAG_LOGI(AAFwkTag::TEST, "SaveEditedContentWithUriTest_0100 end");
}

/**
 * @tc.number: SaveEditedContentWithImageTest_0100
 * @tc.name: SaveEditedContentWithImage
 * @tc.desc: Js photo editor extension context SaveEditedContentWithImage.
 */
HWTEST_F(JsPhotoEditorExtensionContextTest, SaveEditedContentWithImageTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SaveEditedContentWithImageTest_0100 start");
    std::shared_ptr<PhotoEditorExtensionContext> context;
    auto jsContext = std::make_shared<JsPhotoEditorExtensionContext>(context);
    napi_env env = nullptr;
    napi_callback_info info = nullptr;
    jsContext->SaveEditedContentWithImage(env, info);
    TAG_LOGI(AAFwkTag::TEST, "SaveEditedContentWithImageTest_0100 end");
}

/**
 * @tc.number: UnwrapPackOptionTest_0100
 * @tc.name: UnwrapPackOption
 * @tc.desc: Js photo editor extension context UnwrapPackOption.
 */
HWTEST_F(JsPhotoEditorExtensionContextTest, UnwrapPackOptionTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UnwrapPackOptionTest_0100 start");
    std::shared_ptr<PhotoEditorExtensionContext> context;
    auto jsContext = std::make_shared<JsPhotoEditorExtensionContext>(context);
    napi_env env = nullptr;
    napi_value jsOption = nullptr;
    Media::PackOption packOption;
    std::string format = "format";
    EXPECT_EQ(jsContext->UnwrapPackOption(env, jsOption, packOption), false);
    TAG_LOGI(AAFwkTag::TEST, "UnwrapPackOptionTest_0100 end");
}
} // namespace AbilityRuntime
} // namespace OHOS
