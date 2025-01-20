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

#include "preload_so_startup_task.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class PreloadSoStartupTaskTest : public testing::Test {
public:
    PreloadSoStartupTaskTest() {}
    ~PreloadSoStartupTaskTest() {}

    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

private:
    std::unique_ptr<PreloadSoStartupTask> preloadSoStartupTask_ = nullptr;
};

void PreloadSoStartupTaskTest::SetUpTestCase() {}

void PreloadSoStartupTaskTest::TearDownTestCase() {}

void PreloadSoStartupTaskTest::SetUp()
{
    std::string name = "";
    std::string ohmUrl = "";
    preloadSoStartupTask_ = std::make_unique<PreloadSoStartupTask>(name, ohmUrl);
}

void PreloadSoStartupTaskTest::TearDown()
{
    preloadSoStartupTask_.reset();
}

/**
 * @tc.number: ParseNormalizedOhmUrlTest_0100
 * @tc.name: ParseNormalizedOhmUrl test
 * @tc.desc: Test whether the RunTaskInit method handles normalized OHM URLs correctly.
 */
HWTEST_F(PreloadSoStartupTaskTest, ParseNormalizedOhmUrlTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "ParseNormalizedOhmUrlTest_0100 start";

    std::unique_ptr<StartupTaskResultCallback> callback = nullptr;
    EXPECT_EQ(preloadSoStartupTask_->RunTaskInit(std::move(callback)), 0);

    preloadSoStartupTask_->ohmUrl_ = "@normalized:Y&strPosTes";
    EXPECT_EQ(preloadSoStartupTask_->RunTaskInit(std::move(callback)), 0);

    preloadSoStartupTask_->ohmUrl_ = "@normalized:Y&strPosTest&strPosTest&strPosTest&strPosTest";
    EXPECT_NE(preloadSoStartupTask_->RunTaskInit(std::move(callback)), 0);

    GTEST_LOG_(INFO) << "ParseNormalizedOhmUrlTest_0100 end";
}

/**
 * @tc.number: RunTaskInitTest_0100
 * @tc.name: RunTaskInit test
 * @tc.desc: Test whether the RunTaskInit method processes application OHM URLs correctly.
 */
HWTEST_F(PreloadSoStartupTaskTest, RunTaskInitTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "RunTaskInitTest_0100 start";

    std::unique_ptr<StartupTaskResultCallback> callback = nullptr;
    EXPECT_EQ(preloadSoStartupTask_->RunTaskInit(std::move(callback)), 0);

    preloadSoStartupTask_->ohmUrl_ = "@app:strPosTes";
    EXPECT_EQ(preloadSoStartupTask_->RunTaskInit(std::move(callback)), 0);

    preloadSoStartupTask_->ohmUrl_ = "@app:strPosTes:strPosTes:strPosTes";
    EXPECT_EQ(preloadSoStartupTask_->RunTaskInit(std::move(callback)), 0);

    GTEST_LOG_(INFO) << "RunTaskInitTest_0100 end";
}

/**
 * @tc.number: OtherFunctionTest_0100
 * @tc.name: RunTaskInit、GetType and RunTaskOnDependencyCompleted test
 * @tc.desc: Test whether the RunTaskInit and related methods work correctly for other function scenarios.
 */
HWTEST_F(PreloadSoStartupTaskTest, OtherFunctionTest_0100, Function | MediumTest | Level1)
{
    GTEST_LOG_(INFO) << "OtherFunctionTest_0100 start";

    std::unique_ptr<StartupTaskResultCallback> callback = nullptr;
    preloadSoStartupTask_->ohmUrl_ = "OtherFunctionTest";
    EXPECT_EQ(preloadSoStartupTask_->RunTaskInit(std::move(callback)), 0);

    EXPECT_EQ(preloadSoStartupTask_->GetType(), "PreloadSo");

    EXPECT_EQ(preloadSoStartupTask_->RunTaskOnDependencyCompleted("", nullptr), 0);

    GTEST_LOG_(INFO) << "OtherFunctionTest_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS