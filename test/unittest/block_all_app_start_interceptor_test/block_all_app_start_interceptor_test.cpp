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
#include "interceptor/block_all_app_start_interceptor.h"
#undef private
#undef protected

#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
class BlockAllAppStartInterceptorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

public:
};

void BlockAllAppStartInterceptorTest::SetUpTestCase()
{}

void BlockAllAppStartInterceptorTest::TearDownTestCase()
{}

void BlockAllAppStartInterceptorTest::SetUp()
{}

void BlockAllAppStartInterceptorTest::TearDown()
{}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_Execute_001
 * @tc.desc: Execute with no shouldBlockFunc set (nullptr) returns ERR_INVALID_VALUE
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, Execute_001, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    auto ret = blockAllAppStartInterceptor.Execute();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_Execute_002
 * @tc.desc: Execute returns ERR_ALL_APP_START_BLOCKED when shouldBlockFunc returns true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, Execute_002, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    blockAllAppStartInterceptor.SetShouldBlockFunc([]() -> bool { return true; });
    auto ret = blockAllAppStartInterceptor.Execute();
    EXPECT_EQ(ret, ERR_ALL_APP_START_BLOCKED);
}

/**
 * @tc.name: BlockAllAppStartInterceptorTest_Execute_003
 * @tc.desc: Execute returns ERR_OK when shouldBlockFunc returns false
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BlockAllAppStartInterceptorTest, Execute_003, TestSize.Level1)
{
    BlockAllAppStartInterceptor blockAllAppStartInterceptor;
    blockAllAppStartInterceptor.SetShouldBlockFunc([]() -> bool { return false; });
    auto ret = blockAllAppStartInterceptor.Execute();
    EXPECT_EQ(ret, ERR_OK);
}
} // namespace AAFwk
} // namespace OHOS
