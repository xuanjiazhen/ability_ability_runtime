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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "child_process_api.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {

class ChildProcessApiTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ChildProcessApiTest::SetUpTestCase()
{}

void ChildProcessApiTest::TearDownTestCase()
{}

void ChildProcessApiTest::SetUp()
{}

void ChildProcessApiTest::TearDown()
{}

/**
 * @tc.number: StartChild_0100
 * @tc.desc: Test StartChild with empty fds map, dlopen will fail in test environment
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessApiTest, StartChild_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0100 start.");
    std::map<std::string, int32_t> fds;
    ChildProcessApi::StartChild(fds);
    // dlopen fails in test env, function should return safely without crash
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0100 end.");
}

/**
 * @tc.number: StartChild_0200
 * @tc.desc: Test StartChild with non-empty fds map, dlopen will fail in test environment
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessApiTest, StartChild_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0200 start.");
    std::map<std::string, int32_t> fds;
    fds["test_fd"] = 1;
    ChildProcessApi::StartChild(fds);
    // dlopen fails in test env, function should return safely without crash
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0200 end.");
}

/**
 * @tc.number: StartChild_0300
 * @tc.desc: Test StartChild called multiple times with same fds, verify stability
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessApiTest, StartChild_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0300 start.");
    std::map<std::string, int32_t> fds;
    fds["fd1"] = 1;
    fds["fd2"] = 2;
    ChildProcessApi::StartChild(fds);
    ChildProcessApi::StartChild(fds);
    // dlopen fails and static handle remains nullptr, no crash on repeated calls
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0300 end.");
}

/**
 * @tc.number: StartChild_0400
 * @tc.desc: Test StartChild with invalid fd value (-1)
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessApiTest, StartChild_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0400 start.");
    std::map<std::string, int32_t> fds;
    fds["invalid_fd"] = -1;
    ChildProcessApi::StartChild(fds);
    // Invalid fd value, but dlopen fails first, function should return safely
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0400 end.");
}

/**
 * @tc.number: StartChild_0500
 * @tc.desc: Test StartChild with large fds map
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessApiTest, StartChild_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0500 start.");
    std::map<std::string, int32_t> fds;
    for (int i = 0; i < 100; i++) {
        fds["fd_" + std::to_string(i)] = i;
    }
    ChildProcessApi::StartChild(fds);
    // dlopen fails in test env, function should return safely without crash
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0500 end.");
}

/**
 * @tc.number: StartChild_0600
 * @tc.desc: Test StartChild with empty string key in fds map
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessApiTest, StartChild_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0600 start.");
    std::map<std::string, int32_t> fds;
    fds[""] = 0;
    ChildProcessApi::StartChild(fds);
    // Empty string key, but dlopen fails first, function should return safely
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0600 end.");
}

/**
 * @tc.number: StartChild_0700
 * @tc.desc: Test StartChild called with different fds maps sequentially
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessApiTest, StartChild_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0700 start.");
    std::map<std::string, int32_t> fds1;
    fds1["fd_a"] = 10;
    ChildProcessApi::StartChild(fds1);

    std::map<std::string, int32_t> fds2;
    fds2["fd_b"] = 20;
    fds2["fd_c"] = 30;
    ChildProcessApi::StartChild(fds2);
    // Static handle remains nullptr after first dlopen failure, no crash
    TAG_LOGI(AAFwkTag::TEST, "StartChild_0700 end.");
}
} // namespace AppExecFwk
} // namespace OHOS
