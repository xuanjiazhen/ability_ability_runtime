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

#include "hilog_tag_wrapper.h"
#include "want.h"
#include "want_params_wrapper.h"
#include "insight_intent_execute_param.h"
#include "int_wrapper.h"
#include "string_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
using InsightIntentExecuteParam = AppExecFwk::InsightIntentExecuteParam;

class InsightIntentExecuteParamSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecuteParamSecondTest::SetUpTestCase(void)
{}

void InsightIntentExecuteParamSecondTest::TearDownTestCase(void)
{}

void InsightIntentExecuteParamSecondTest::SetUp()
{}

void InsightIntentExecuteParamSecondTest::TearDown()
{}

/**
 * @tc.name: UpdateInsightIntentCallerInfo_0100
 * @tc.desc: basic function test of get caller info.
 * @tc.type: FUNC
 * @tc.require: issueI91RLM
 */
HWTEST_F(InsightIntentExecuteParamSecondTest, UpdateInsightIntentCallerInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    WantParams wantParams;
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME,
        AAFwk::String::Box("test.bundleName"));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID,
        AAFwk::String::Box("1"));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_MODE,
        AAFwk::String::Box("execution"));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM,
        AAFwk::String::Box("executionParameter"));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_SRC_ENTRY,
        AAFwk::String::Box("testEntry"));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_URI,
        AAFwk::String::Box("testUri"));
    wantParams.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_FLAGS,
        AAFwk::String::Box("testFlags"));

    Want want;
    want.SetElementName("", TEST_BUNDLE_NANE, TEST_ABILITY_NANE, TEST_MODULE_NANE);
    want.SetParams(wantParams);
    WantParams insightIntentParam;
    InsightIntentExecuteParam info;
    auto ret = info.RemoveInsightIntent(want);
    EXPECT_EQ(ret, true);

    TAG_LOGI(AAFwkTag::TEST, "end.");
}
} // namespace AAFwk
} // namespace OHOS
