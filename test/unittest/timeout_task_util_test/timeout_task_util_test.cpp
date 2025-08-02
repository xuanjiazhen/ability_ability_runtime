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

#include <chrono>
#include <functional>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>

#include "hilog_tag_wrapper.h"
#include "timeout_task_util.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int32_t TIME_DELAY = 100;
constexpr int32_t TIME_SLEEP = 200;
}
std::shared_ptr<MockLogger> MockLogger::instance;

class TimeoutTaskUtilTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void TimeoutTaskUtilTest::SetUpTestCase(void)
{}
void TimeoutTaskUtilTest::TearDownTestCase(void)
{}
void TimeoutTaskUtilTest::SetUp()
{
    MockLogger::instance = std::make_shared<MockLogger>();
}
void TimeoutTaskUtilTest::TearDown()
{
    MockLogger::instance.reset();
}

/**
 * @tc.name: PostSyncTaskWithTimeout_0010
 * @tc.desc: Invalid param.
 * @tc.type: FUNC PostSyncTaskWithTimeout
 */
HWTEST_F(TimeoutTaskUtilTest, PostSyncTaskWithTimeout_0010, TestSize.Level1)
{
    EXPECT_CALL(*MockLogger::instance, PrintLog).Times(1);
    auto result = TimeoutTaskUtil::PostSyncTaskWithTimeout(std::function<void(void)>(), 0, "");
    EXPECT_FALSE(result);

    EXPECT_CALL(*MockLogger::instance, PrintLog).Times(1);
    result = TimeoutTaskUtil::PostSyncTaskWithTimeout(std::function<void(void)>(), 1, "");
    EXPECT_FALSE(result);

    EXPECT_CALL(*MockLogger::instance, PrintLog).Times(1);
    result = TimeoutTaskUtil::PostSyncTaskWithTimeout([]() {}, 0, "");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: PostSyncTaskWithTimeout_0020
 * @tc.desc: Task timeout or ok.
 * @tc.type: FUNC PostSyncTaskWithTimeout
 */
HWTEST_F(TimeoutTaskUtilTest, PostSyncTaskWithTimeout_0020, TestSize.Level1)
{
    EXPECT_CALL(*MockLogger::instance, PrintLog).Times(1);
    auto result = TimeoutTaskUtil::PostSyncTaskWithTimeout([]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(TIME_SLEEP));
        }, TIME_DELAY, "");
    EXPECT_FALSE(result);

    EXPECT_CALL(*MockLogger::instance, PrintLog).Times(0);
    result = TimeoutTaskUtil::PostSyncTaskWithTimeout([]() {}, TIME_DELAY, "");
    EXPECT_TRUE(result);
}
}  // namespace AAFwk
}  // namespace OHOS