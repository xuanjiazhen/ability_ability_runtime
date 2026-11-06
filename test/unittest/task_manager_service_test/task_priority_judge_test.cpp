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

// Workaround to access private members for testing
// NOLINTNEXTLINE(g.nam.03-cpp): Redefining C++ keyword 'private' for unit testing
#define private public
#include "task_mgr/task_priority_judge.h"
#undef private

#include "task_mgr/common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr pid_t TEST_FG_PID = 4100;

class StubLoadProvider : public ISystemLoadProvider {
public:
    explicit StubLoadProvider(int32_t level, bool registerOk = true)
        : level_(level), registerOk_(registerOk)
    {
    }

    bool RegisterListener(const std::function<void(int32_t)> &callback) override
    {
        if (registerOk_ && callback != nullptr) {
            callback(level_);
        }
        registered_ = registerOk_;
        return registerOk_;
    }

    void UnregisterListener() override
    {
        registered_ = false;
    }

    int32_t GetCurrentLevel() const override
    {
        return level_;
    }

    int32_t level_;
    bool registerOk_;
    bool registered_ = false;
};
}  // namespace

class TaskPriorityJudgeTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }
    void SetUp() override
    {
        auto &judge = TaskPriorityJudge::GetInstance();
        judge.fgAbilityCounts_.clear();
        judge.loadLevel_ = -1;
        judge.loadListenerRegistered_ = false;
        judge.SetLoadProvider(nullptr);
        judge.loadProvider_ = nullptr;
    }
    void TearDown() override
    {
        auto &judge = TaskPriorityJudge::GetInstance();
        judge.fgAbilityCounts_.clear();
        judge.loadLevel_ = -1;
        judge.loadListenerRegistered_ = false;
        judge.SetLoadProvider(nullptr);
        judge.loadProvider_ = nullptr;
    }
};

/**
 * @tc.number: TaskPriorityJudge_Matrix_0100
 * @tc.name: Mapping matrix: foreground -> HIGH regardless of load.
 * @tc.desc: Covers the matrix row "foreground = HIGH" for low/high/unavailable loads.
 */
HWTEST_F(TaskPriorityJudgeTest, TaskPriorityJudge_Matrix_0100, TestSize.Level1)
{
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(true, 0), TASK_PRIORITY_HIGH);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(true, 2), TASK_PRIORITY_HIGH);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(true, 4), TASK_PRIORITY_HIGH);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(true, 5), TASK_PRIORITY_HIGH);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(true, 7), TASK_PRIORITY_HIGH);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(true, -1), TASK_PRIORITY_HIGH);
}

/**
 * @tc.number: TaskPriorityJudge_Matrix_0200
 * @tc.name: Mapping matrix: background & fusion level 0~4 -> MEDIUM, 5~7 -> LOW.
 * @tc.desc: Covers the background branch including the boundary levels 4/5.
 */
HWTEST_F(TaskPriorityJudgeTest, TaskPriorityJudge_Matrix_0200, TestSize.Level1)
{
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(false, 0), TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(false, 1), TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(false, 2), TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(false, 3), TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(false, 4), TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(false, 5), TASK_PRIORITY_LOW);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(false, 6), TASK_PRIORITY_LOW);
    EXPECT_EQ(TaskPriorityJudge::DetermineLevel(false, 7), TASK_PRIORITY_LOW);
}

/**
 * @tc.number: TaskPriorityJudge_Matrix_0300
 * @tc.name: JudgeLevel uses the cached foreground counts and load level.
 * @tc.desc: Drives the cache through the event entry points and asserts the judged level.
 */
HWTEST_F(TaskPriorityJudgeTest, TaskPriorityJudge_Matrix_0300, TestSize.Level1)
{
    auto &judge = TaskPriorityJudge::GetInstance();
    // no foreground ability, load 2 -> MEDIUM
    judge.OnSystemLoadLevelChanged(2);
    EXPECT_EQ(judge.JudgeLevel(TEST_FG_PID), TASK_PRIORITY_MEDIUM);
    // one foreground ability -> HIGH
    judge.OnForegroundAbilityChanged(TEST_FG_PID, true);
    EXPECT_TRUE(judge.IsForeground(TEST_FG_PID));
    EXPECT_EQ(judge.JudgeLevel(TEST_FG_PID), TASK_PRIORITY_HIGH);
    // load jumps to 6 while foreground -> still HIGH (foreground ignores load)
    judge.OnSystemLoadLevelChanged(6);
    EXPECT_EQ(judge.JudgeLevel(TEST_FG_PID), TASK_PRIORITY_HIGH);
    // ability leaves foreground with load 6 -> LOW
    judge.OnForegroundAbilityChanged(TEST_FG_PID, false);
    EXPECT_FALSE(judge.IsForeground(TEST_FG_PID));
    EXPECT_EQ(judge.JudgeLevel(TEST_FG_PID), TASK_PRIORITY_LOW);
    // load drops back to 4 -> MEDIUM (boundary)
    judge.OnSystemLoadLevelChanged(4);
    EXPECT_EQ(judge.JudgeLevel(TEST_FG_PID), TASK_PRIORITY_MEDIUM);
}

/**
 * @tc.number: TaskPriorityJudge_Fallback_0100
 * @tc.name: Fallback: load unavailable keeps background tasks at MEDIUM; foreground stays HIGH.
 * @tc.desc: Simulates a missing/never-registered load listener and out-of-range values.
 */
HWTEST_F(TaskPriorityJudgeTest, TaskPriorityJudge_Fallback_0100, TestSize.Level1)
{
    auto &judge = TaskPriorityJudge::GetInstance();
    // load never updated (-1): background -> MEDIUM
    EXPECT_EQ(judge.CurrentLoadLevel(), -1);
    EXPECT_EQ(judge.JudgeLevel(TEST_FG_PID), TASK_PRIORITY_MEDIUM);
    // out-of-range values are treated as unavailable
    judge.OnSystemLoadLevelChanged(8);
    EXPECT_EQ(judge.CurrentLoadLevel(), -1);
    EXPECT_EQ(judge.JudgeLevel(TEST_FG_PID), TASK_PRIORITY_MEDIUM);
    judge.OnSystemLoadLevelChanged(-5);
    EXPECT_EQ(judge.CurrentLoadLevel(), -1);
    // foreground stays HIGH even with unavailable load
    judge.OnForegroundAbilityChanged(TEST_FG_PID, true);
    EXPECT_EQ(judge.JudgeLevel(TEST_FG_PID), TASK_PRIORITY_HIGH);
    // provider registration failure keeps the listener unregistered and the load unavailable
    judge.SetLoadProvider(std::make_shared<StubLoadProvider>(-1, false));
    EXPECT_FALSE(judge.RegisterSystemLoadListener());
    EXPECT_EQ(judge.CurrentLoadLevel(), -1);
    EXPECT_EQ(judge.JudgeLevel(TEST_FG_PID), TASK_PRIORITY_HIGH);
}

/**
 * @tc.number: TaskPriorityJudge_ForegroundCount_0100
 * @tc.name: Foreground ability count: multiple abilities of one uid keep the foreground state.
 * @tc.desc: Two abilities foreground, one leaves -> still foreground; both leave -> background.
 */
HWTEST_F(TaskPriorityJudgeTest, TaskPriorityJudge_ForegroundCount_0100, TestSize.Level1)
{
    auto &judge = TaskPriorityJudge::GetInstance();
    judge.OnForegroundAbilityChanged(TEST_FG_PID, true);
    judge.OnForegroundAbilityChanged(TEST_FG_PID, true);
    EXPECT_TRUE(judge.IsForeground(TEST_FG_PID));
    judge.OnForegroundAbilityChanged(TEST_FG_PID, false);
    EXPECT_TRUE(judge.IsForeground(TEST_FG_PID));
    judge.OnForegroundAbilityChanged(TEST_FG_PID, false);
    EXPECT_FALSE(judge.IsForeground(TEST_FG_PID));
    // defensive: an extra foreground=false must not produce a negative count
    judge.OnForegroundAbilityChanged(TEST_FG_PID, false);
    EXPECT_FALSE(judge.IsForeground(TEST_FG_PID));
}

/**
 * @tc.number: TaskPriorityJudge_LoadListener_0100
 * @tc.name: Load listener registration seeds the cache with the current level.
 * @tc.desc: A stub provider reports level 5; the cache must hold it right after registration.
 */
HWTEST_F(TaskPriorityJudgeTest, TaskPriorityJudge_LoadListener_0100, TestSize.Level1)
{
    auto &judge = TaskPriorityJudge::GetInstance();
    judge.SetLoadProvider(std::make_shared<StubLoadProvider>(5));
    EXPECT_TRUE(judge.RegisterSystemLoadListener());
    EXPECT_EQ(judge.CurrentLoadLevel(), 5);
    EXPECT_EQ(judge.JudgeLevel(TEST_FG_PID), TASK_PRIORITY_LOW);
    judge.UnregisterSystemLoadListener();
}

/**
 * @tc.number: TaskPriorityJudge_ForegroundChanged_0100
 * @tc.name: OnForegroundAbilityChanged rejects non-positive uids (early-return branch).
 * @tc.desc: uid 0 and negative uids must not create any foreground count entry.
 */
HWTEST_F(TaskPriorityJudgeTest, TaskPriorityJudge_ForegroundChanged_0100, TestSize.Level1)
{
    auto &judge = TaskPriorityJudge::GetInstance();
    judge.OnForegroundAbilityChanged(0, true);
    judge.OnForegroundAbilityChanged(-1, true);
    {
        std::lock_guard<std::mutex> lock(judge.judgeMutex_);
        EXPECT_TRUE(judge.fgAbilityCounts_.empty());
    }
    EXPECT_FALSE(judge.IsForeground(0));
}

/**
 * @tc.number: TaskPriorityJudge_LoadListener_0200
 * @tc.name: RegisterSystemLoadListener is idempotent (already-registered branch).
 * @tc.desc: The second registration returns true without touching the provider again.
 */
HWTEST_F(TaskPriorityJudgeTest, TaskPriorityJudge_LoadListener_0200, TestSize.Level1)
{
    auto &judge = TaskPriorityJudge::GetInstance();
    auto provider = std::make_shared<StubLoadProvider>(2);
    judge.SetLoadProvider(provider);
    EXPECT_TRUE(judge.RegisterSystemLoadListener());
    EXPECT_TRUE(provider->registered_);
    int32_t firstCount = provider->level_;  // 2, seeded once at registration
    EXPECT_EQ(judge.CurrentLoadLevel(), firstCount);
    EXPECT_TRUE(judge.RegisterSystemLoadListener());  // second call: no provider re-registration
    EXPECT_EQ(judge.CurrentLoadLevel(), firstCount);
    judge.UnregisterSystemLoadListener();
}

/**
 * @tc.number: TaskPriorityJudge_LoadListener_0300
 * @tc.name: UnregisterSystemLoadListener covers both the not-registered and registered branches.
 * @tc.desc: Unregistering with nothing registered is a safe no-op; after a real registration the
 *           provider flag flips back to false.
 */
HWTEST_F(TaskPriorityJudgeTest, TaskPriorityJudge_LoadListener_0300, TestSize.Level1)
{
    auto &judge = TaskPriorityJudge::GetInstance();
    // not-registered branch: must not crash and must stay unregistered
    judge.UnregisterSystemLoadListener();
    {
        std::lock_guard<std::mutex> lock(judge.judgeMutex_);
        EXPECT_FALSE(judge.loadListenerRegistered_);
    }
    // registered branch
    auto provider = std::make_shared<StubLoadProvider>(3);
    judge.SetLoadProvider(provider);
    EXPECT_TRUE(judge.RegisterSystemLoadListener());
    EXPECT_TRUE(provider->registered_);
    judge.UnregisterSystemLoadListener();
    EXPECT_FALSE(provider->registered_);
}

/**
 * @tc.number: TaskPriorityJudge_SetLoadProvider_0100
 * @tc.name: SetLoadProvider resets the registration state (switch-provider branch).
 * @tc.desc: After switching providers the judge re-registers against the new provider.
 */
HWTEST_F(TaskPriorityJudgeTest, TaskPriorityJudge_SetLoadProvider_0100, TestSize.Level1)
{
    auto &judge = TaskPriorityJudge::GetInstance();
    auto first = std::make_shared<StubLoadProvider>(1);
    auto second = std::make_shared<StubLoadProvider>(6);
    judge.SetLoadProvider(first);
    EXPECT_TRUE(judge.RegisterSystemLoadListener());
    {
        std::lock_guard<std::mutex> lock(judge.judgeMutex_);
        EXPECT_TRUE(judge.loadListenerRegistered_);
    }
    judge.SetLoadProvider(second);
    {
        std::lock_guard<std::mutex> lock(judge.judgeMutex_);
        EXPECT_FALSE(judge.loadListenerRegistered_);
    }
    EXPECT_TRUE(judge.RegisterSystemLoadListener());
    EXPECT_EQ(judge.CurrentLoadLevel(), 6);
    EXPECT_TRUE(second->registered_);
    EXPECT_FALSE(first->registered_);
}

/**
 * @tc.number: TaskPriorityJudge_DumpJudgeState_0100
 * @tc.name: DumpJudgeState renders the empty and populated branches.
 * @tc.desc: Empty state prints the "empty" placeholder and "not registered"; populated state
 *           prints the uid/count pair and the registered listener state.
 */
HWTEST_F(TaskPriorityJudgeTest, TaskPriorityJudge_DumpJudgeState_0100, TestSize.Level1)
{
    auto &judge = TaskPriorityJudge::GetInstance();
    std::string emptyDump = judge.DumpJudgeState();
    EXPECT_NE(emptyDump.find("foreground ability counts [empty]"), std::string::npos);
    EXPECT_NE(emptyDump.find("cached systemload level [-1]"), std::string::npos);
    EXPECT_NE(emptyDump.find("load listener [not registered]"), std::string::npos);

    judge.OnForegroundAbilityChanged(TEST_FG_PID, true);
    auto provider = std::make_shared<StubLoadProvider>(4);
    judge.SetLoadProvider(provider);
    EXPECT_TRUE(judge.RegisterSystemLoadListener());
    std::string populatedDump = judge.DumpJudgeState();
    EXPECT_NE(populatedDump.find("pid:" + std::to_string(TEST_FG_PID) + " count:1"), std::string::npos);
    EXPECT_NE(populatedDump.find("cached systemload level [4]"), std::string::npos);
    EXPECT_NE(populatedDump.find("load listener [registered]"), std::string::npos);
    judge.UnregisterSystemLoadListener();
}
}  // namespace AppExecFwk
}  // namespace OHOS
