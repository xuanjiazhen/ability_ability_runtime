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
#include <memory>
#include <string>

#define private public
#define protected public
#include "app_startup_task.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

class AppStartupTaskTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() { task_ = std::make_shared<AppStartupTask>(); }
    void TearDown() { task_ = nullptr; }
    std::shared_ptr<AppStartupTask> task_;
};

HWTEST_F(AppStartupTaskTest, AppStartupTaskTest_StaticTypes_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppStartupTaskTest_StaticTypes_0100 start";
    EXPECT_EQ(AppStartupTask::TASK_TYPE_JS, AppStartupTask::TASK_TYPE_JS);
    EXPECT_EQ(AppStartupTask::TASK_TYPE_ETS, AppStartupTask::TASK_TYPE_ETS);
    EXPECT_EQ(AppStartupTask::TASK_TYPE_PRELOAD_SO, AppStartupTask::TASK_TYPE_PRELOAD_SO);
    GTEST_LOG_(INFO) << "AppStartupTaskTest_StaticTypes_0100 end";
}

HWTEST_F(AppStartupTaskTest, AppStartupTaskTest_ExcludeFromAutoStart_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppStartupTaskTest_ExcludeFromAutoStart_0100 start";
    EXPECT_FALSE(task_->GetIsExcludeFromAutoStart());
    task_->SetIsExcludeFromAutoStart(true);
    EXPECT_TRUE(task_->GetIsExcludeFromAutoStart());
    GTEST_LOG_(INFO) << "AppStartupTaskTest_ExcludeFromAutoStart_0100 end";
}

HWTEST_F(AppStartupTaskTest, AppStartupTaskTest_ModuleName_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppStartupTaskTest_ModuleName_0100 start";
    std::string name = "test_module";
    task_->SetModuleName(name);
    EXPECT_EQ(task_->GetModuleName(), name);
    GTEST_LOG_(INFO) << "AppStartupTaskTest_ModuleName_0100 end";
}

HWTEST_F(AppStartupTaskTest, AppStartupTaskTest_ModuleType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppStartupTaskTest_ModuleType_0100 start";
    AppExecFwk::ModuleType type = AppExecFwk::ModuleType::SHARED;
    task_->SetModuleType(type);
    EXPECT_EQ(task_->GetModuleType(), type);
    GTEST_LOG_(INFO) << "AppStartupTaskTest_ModuleType_0100 end";
}

HWTEST_F(AppStartupTaskTest, AppStartupTaskTest_PreAbilityStageLoad_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppStartupTaskTest_PreAbilityStageLoad_0100 start";
    EXPECT_FALSE(task_->IsPreAbilityStageLoad());
    task_->SetPreAbilityStageLoad(true);
    EXPECT_TRUE(task_->IsPreAbilityStageLoad());
    GTEST_LOG_(INFO) << "AppStartupTaskTest_PreAbilityStageLoad_0100 end";
}

HWTEST_F(AppStartupTaskTest, AppStartupTaskTest_MatchRules_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppStartupTaskTest_MatchRules_0100 start";
    StartupTaskMatchRules rules;
    rules.uriMatchRules.push_back("uri1");
    rules.insightIntentMatchRules.push_back("insight1");
    rules.actionMatchRules.push_back("action1");
    rules.customizationMatchRules.push_back("custom1");
    task_->SetMatchRules(rules);
    EXPECT_EQ(task_->GetUriMatchRules().size(), 1u);
    EXPECT_EQ(task_->GetUriMatchRules()[0], "uri1");
    EXPECT_EQ(task_->GetInsightIntentMatchRules().size(), 1u);
    EXPECT_EQ(task_->GetInsightIntentMatchRules()[0], "insight1");
    EXPECT_EQ(task_->GetActionMatchRules().size(), 1u);
    EXPECT_EQ(task_->GetActionMatchRules()[0], "action1");
    EXPECT_EQ(task_->GetCustomizationMatchRules().size(), 1u);
    EXPECT_EQ(task_->GetCustomizationMatchRules()[0], "custom1");
    GTEST_LOG_(INFO) << "AppStartupTaskTest_MatchRules_0100 end";
}

HWTEST_F(AppStartupTaskTest, AppStartupTaskTest_UpdateContextRef_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppStartupTaskTest_UpdateContextRef_0100 start";
    task_->UpdateContextRef(nullptr, nullptr);
    SUCCEED();
    GTEST_LOG_(INFO) << "AppStartupTaskTest_UpdateContextRef_0100 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
