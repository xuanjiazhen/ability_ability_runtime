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

#include <dlfcn.h>
#include <map>

#include <gtest/gtest.h>

#include "child_process_api.h"

using namespace testing::ext;

namespace {
constexpr const char *CHILD_LIB_NAME = "libappkit_child.z.so";
constexpr const char *RESET_SYMBOL = "ResetFakeChildMainThreadStartState";
constexpr const char *COUNT_SYMBOL = "GetFakeChildMainThreadStartCallCount";
constexpr const char *FD_SYMBOL = "GetFakeChildMainThreadFdValue";

using ResetFunc = void (*)();
using GetCountFunc = int (*)();
using GetFdFunc = int (*)(const char *);

void *OpenFakeChildLib()
{
    return dlopen(CHILD_LIB_NAME, RTLD_LAZY | RTLD_LOCAL);
}
} // namespace

namespace OHOS {
namespace AppExecFwk {
class ChildProcessApiTest : public testing::Test {
public:
    void SetUp() override;

protected:
    void *handle_ = nullptr;
    ResetFunc resetFunc_ = nullptr;
    GetCountFunc getCountFunc_ = nullptr;
    GetFdFunc getFdFunc_ = nullptr;
};

void ChildProcessApiTest::SetUp()
{
    handle_ = OpenFakeChildLib();
    ASSERT_NE(handle_, nullptr);

    resetFunc_ = reinterpret_cast<ResetFunc>(dlsym(handle_, RESET_SYMBOL));
    getCountFunc_ = reinterpret_cast<GetCountFunc>(dlsym(handle_, COUNT_SYMBOL));
    getFdFunc_ = reinterpret_cast<GetFdFunc>(dlsym(handle_, FD_SYMBOL));
    ASSERT_NE(resetFunc_, nullptr);
    ASSERT_NE(getCountFunc_, nullptr);
    ASSERT_NE(getFdFunc_, nullptr);

    resetFunc_();
}

/**
 * @tc.number: StartChild_0100
 * @tc.desc: Verify StartChild loads child entry successfully and passes fds to ChildMainThreadStart.
 * @tc.type: FUNC
 */
HWTEST_F(ChildProcessApiTest, StartChild_0100, TestSize.Level1)
{
    std::map<std::string, int32_t> fds = {
        {"mainPipe", 10},
        {"statusPipe", 20},
    };

    ChildProcessApi::StartChild(fds);

    EXPECT_EQ(getCountFunc_(), 1);
    EXPECT_EQ(getFdFunc_("mainPipe"), 10);
    EXPECT_EQ(getFdFunc_("statusPipe"), 20);
    EXPECT_EQ(getFdFunc_("missing"), -1);
}
} // namespace AppExecFwk
} // namespace OHOS
