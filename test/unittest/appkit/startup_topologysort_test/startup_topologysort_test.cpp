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
#include <vector>
#include <map>

#define private public
#define protected public
#include "startup_topologysort.h"
#include "startup_task.h"
#undef private
#undef protected

#include "startup_sort_result.h"
#include "startup_utils.h"

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {
class MockStartupTask : public StartupTask {
public:
    explicit MockStartupTask(const std::string &name) : StartupTask(name)
    {
        state_ = State::CREATED;
    }
    ~MockStartupTask() = default;

    const std::string &GetType() const override
    {
        static const std::string type = "mock";
        return type;
    }

    int32_t RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback) override
    {
        return ERR_OK;
    }

    int32_t RunTaskOnDependencyCompleted(const std::string &name,
        const std::shared_ptr<StartupTaskResult> &result) override
    {
        return ERR_OK;
    }
};

class StartupTopologySortTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}

    static std::shared_ptr<MockStartupTask> MakeTask(const std::string &name,
        const std::vector<std::string> &deps = {})
    {
        auto task = std::make_shared<MockStartupTask>(name);
        task->SetDependencies(deps);
        return task;
    }
};

/**
 * @tc.number: StartupTopologySortTest_Sort_0100
 * @tc.name: Sort empty map
 * @tc.desc: Verify Sort with empty startup map returns ERR_OK and empty result.
 */
HWTEST_F(StartupTopologySortTest, StartupTopologySortTest_Sort_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0100 start";
    std::map<std::string, std::shared_ptr<StartupTask>> startupMap;
    std::shared_ptr<StartupSortResult> result;
    int32_t ret = StartupTopologySort::Sort(startupMap, result);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_NE(result, nullptr);
    EXPECT_TRUE(result->zeroDequeResult_.empty());
    EXPECT_TRUE(result->startupChildrenMap_.empty());
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0100 end";
}

/**
 * @tc.number: StartupTopologySortTest_Sort_0200
 * @tc.name: Sort single node no dependency
 * @tc.desc: Verify Sort with one task and no dependencies puts it in zeroDeque.
 */
HWTEST_F(StartupTopologySortTest, StartupTopologySortTest_Sort_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0200 start";
    std::map<std::string, std::shared_ptr<StartupTask>> startupMap;
    startupMap["A"] = MakeTask("A");
    std::shared_ptr<StartupSortResult> result;
    int32_t ret = StartupTopologySort::Sort(startupMap, result);
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->zeroDequeResult_.size(), 1u);
    EXPECT_EQ(result->zeroDequeResult_[0], "A");
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0200 end";
}

/**
 * @tc.number: StartupTopologySortTest_Sort_0300
 * @tc.name: Sort linear chain A->B->C
 * @tc.desc: A depends on nothing, B depends on A, C depends on B.
 */
HWTEST_F(StartupTopologySortTest, StartupTopologySortTest_Sort_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0300 start";
    std::map<std::string, std::shared_ptr<StartupTask>> startupMap;
    startupMap["A"] = MakeTask("A");
    startupMap["B"] = MakeTask("B", {"A"});
    startupMap["C"] = MakeTask("C", {"B"});
    std::shared_ptr<StartupSortResult> result;
    int32_t ret = StartupTopologySort::Sort(startupMap, result);
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->zeroDequeResult_.size(), 1u);
    EXPECT_EQ(result->zeroDequeResult_[0], "A");
    auto &children = result->startupChildrenMap_;
    ASSERT_TRUE(children.find("A") != children.end());
    EXPECT_EQ(children["A"].size(), 1u);
    EXPECT_EQ(children["A"][0], "B");
    ASSERT_TRUE(children.find("B") != children.end());
    EXPECT_EQ(children["B"].size(), 1u);
    EXPECT_EQ(children["B"][0], "C");
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0300 end";
}

/**
 * @tc.number: StartupTopologySortTest_Sort_0400
 * @tc.name: Sort diamond dependency A->{B,C}, {B,C}->D
 * @tc.desc: A is root, B and C depend on A, D depends on both B and C.
 */
HWTEST_F(StartupTopologySortTest, StartupTopologySortTest_Sort_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0400 start";
    std::map<std::string, std::shared_ptr<StartupTask>> startupMap;
    startupMap["A"] = MakeTask("A");
    startupMap["B"] = MakeTask("B", {"A"});
    startupMap["C"] = MakeTask("C", {"A"});
    startupMap["D"] = MakeTask("D", {"B", "C"});
    std::shared_ptr<StartupSortResult> result;
    int32_t ret = StartupTopologySort::Sort(startupMap, result);
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->zeroDequeResult_.size(), 1u);
    EXPECT_EQ(result->zeroDequeResult_[0], "A");
    EXPECT_EQ(result->startupChildrenMap_["A"].size(), 2u);
    EXPECT_EQ(result->startupChildrenMap_["B"].size(), 1u);
    EXPECT_EQ(result->startupChildrenMap_["C"].size(), 1u);
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0400 end";
}

/**
 * @tc.number: StartupTopologySortTest_Sort_0500
 * @tc.name: Sort circular dependency A->B->A
 * @tc.desc: Verify Sort detects circular dependency and returns error.
 */
HWTEST_F(StartupTopologySortTest, StartupTopologySortTest_Sort_0500, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0500 start";
    std::map<std::string, std::shared_ptr<StartupTask>> startupMap;
    startupMap["A"] = MakeTask("A", {"B"});
    startupMap["B"] = MakeTask("B", {"A"});
    std::shared_ptr<StartupSortResult> result;
    int32_t ret = StartupTopologySort::Sort(startupMap, result);
    EXPECT_EQ(ret, ERR_STARTUP_CIRCULAR_DEPENDENCY);
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0500 end";
}

/**
 * @tc.number: StartupTopologySortTest_Sort_0600
 * @tc.name: Sort self-loop A->A
 * @tc.desc: Verify Sort detects self circular dependency.
 */
HWTEST_F(StartupTopologySortTest, StartupTopologySortTest_Sort_0600, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0600 start";
    std::map<std::string, std::shared_ptr<StartupTask>> startupMap;
    startupMap["A"] = MakeTask("A", {"A"});
    std::shared_ptr<StartupSortResult> result;
    int32_t ret = StartupTopologySort::Sort(startupMap, result);
    EXPECT_EQ(ret, ERR_STARTUP_CIRCULAR_DEPENDENCY);
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0600 end";
}

/**
 * @tc.number: StartupTopologySortTest_Sort_0700
 * @tc.name: Sort dependency not found
 * @tc.desc: B depends on X which is not in the map.
 */
HWTEST_F(StartupTopologySortTest, StartupTopologySortTest_Sort_0700, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0700 start";
    std::map<std::string, std::shared_ptr<StartupTask>> startupMap;
    startupMap["B"] = MakeTask("B", {"X"});
    std::shared_ptr<StartupSortResult> result;
    int32_t ret = StartupTopologySort::Sort(startupMap, result);
    EXPECT_EQ(ret, ERR_STARTUP_DEPENDENCY_NOT_FOUND);
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0700 end";
}

/**
 * @tc.number: StartupTopologySortTest_Sort_0800
 * @tc.name: Sort multiple independent roots
 * @tc.desc: A and B have no dependencies, C depends on A.
 */
HWTEST_F(StartupTopologySortTest, StartupTopologySortTest_Sort_0800, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0800 start";
    std::map<std::string, std::shared_ptr<StartupTask>> startupMap;
    startupMap["A"] = MakeTask("A");
    startupMap["B"] = MakeTask("B");
    startupMap["C"] = MakeTask("C", {"A"});
    std::shared_ptr<StartupSortResult> result;
    int32_t ret = StartupTopologySort::Sort(startupMap, result);
    EXPECT_EQ(ret, ERR_OK);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->zeroDequeResult_.size(), 2u);
    EXPECT_EQ(result->startupChildrenMap_["A"].size(), 1u);
    EXPECT_EQ(result->startupChildrenMap_["A"][0], "C");
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0800 end";
}

/**
 * @tc.number: StartupTopologySortTest_Sort_0900
 * @tc.name: Sort null task in map
 * @tc.desc: Verify Sort returns error when a task in map is null.
 */
HWTEST_F(StartupTopologySortTest, StartupTopologySortTest_Sort_0900, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0900 start";
    std::map<std::string, std::shared_ptr<StartupTask>> startupMap;
    startupMap["A"] = nullptr;
    std::shared_ptr<StartupSortResult> result;
    int32_t ret = StartupTopologySort::Sort(startupMap, result);
    EXPECT_EQ(ret, ERR_STARTUP_INTERNAL_ERROR);
    GTEST_LOG_(INFO) << "StartupTopologySortTest_Sort_0900 end";
}
} // namespace AbilityRuntime
} // namespace OHOS
