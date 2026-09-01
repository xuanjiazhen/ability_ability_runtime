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
#include <string>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

// Declared here to avoid pulling the heavy js_runtime.h include chain.
bool IsPathValid(const std::string &path, bool requireAbsolute);

namespace {
std::string StringWithNull()
{
    return std::string("a\0b", 3);
}
} // namespace

class PathValidTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PathValidTest::SetUpTestCase(void) {}
void PathValidTest::TearDownTestCase(void) {}
void PathValidTest::SetUp(void) {}
void PathValidTest::TearDown(void) {}

// ---------------- relative paths ----------------

HWTEST_F(PathValidTest, Relative_Valid_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsPathValid("entry", false));
    EXPECT_TRUE(IsPathValid("my_module-1", false));
    EXPECT_TRUE(IsPathValid("ets/pages/Index.ets", false));
}

HWTEST_F(PathValidTest, Relative_Empty_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("", false));
}

HWTEST_F(PathValidTest, Relative_AbsolutePath_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("/etc/passwd", false));
    EXPECT_FALSE(IsPathValid("/data/app/foo.hap", false));
}

HWTEST_F(PathValidTest, Relative_Traversal_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("..", false));
    EXPECT_FALSE(IsPathValid("../", false));
    EXPECT_FALSE(IsPathValid("../secret", false));
    EXPECT_FALSE(IsPathValid("foo/..", false));
    EXPECT_FALSE(IsPathValid("a/b/..", false));
    EXPECT_FALSE(IsPathValid("foo/..bar", false));
    EXPECT_FALSE(IsPathValid("a/../../etc", false));
    EXPECT_FALSE(IsPathValid("../../../etc/passwd", false));
    EXPECT_FALSE(IsPathValid("module/../../etc", false));
}

HWTEST_F(PathValidTest, Relative_NullByte_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid(StringWithNull(), false));
}

HWTEST_F(PathValidTest, Relative_Backslash_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("a\\b", false));
}

HWTEST_F(PathValidTest, Relative_ControlChar_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("a\tb", false));
    EXPECT_FALSE(IsPathValid("a\nb", false));
}

// Relaxed on purpose (modeled after BundleUtil::IsFileNameValid): characters
// outside the old whitelist and ".." inside a file name are accepted.
HWTEST_F(PathValidTest, Relative_RelaxedInputs_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsPathValid("a..b", false));
    EXPECT_TRUE(IsPathValid("a b", false));
    EXPECT_TRUE(IsPathValid("a|b", false));
    EXPECT_TRUE(IsPathValid("a;b", false));
}

// ---------------- absolute paths ----------------

HWTEST_F(PathValidTest, Absolute_Valid_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsPathValid("/data/app/el1/bundle/public/com.example/foo.hap", true));
    EXPECT_TRUE(IsPathValid("/system/app/foo.hap", true));
}

HWTEST_F(PathValidTest, Absolute_Empty_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("", true));
}

HWTEST_F(PathValidTest, Absolute_Relative_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("data/app/foo.hap", true));
    EXPECT_FALSE(IsPathValid("foo.hap", true));
}

HWTEST_F(PathValidTest, Absolute_Traversal_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("/../secret", true));
    EXPECT_FALSE(IsPathValid("/../..", true));
    EXPECT_FALSE(IsPathValid("/data/app/../../etc/passwd", true));
    EXPECT_FALSE(IsPathValid("/data/../.././etc", true));
    EXPECT_FALSE(IsPathValid("/data/app/foo/..", true));
    EXPECT_FALSE(IsPathValid("/data/app/../etc", true));
}

HWTEST_F(PathValidTest, Absolute_NullByte_0100, TestSize.Level0)
{
    std::string bad = std::string("/data/\0evil", 10);
    EXPECT_FALSE(IsPathValid(bad, true));
}

HWTEST_F(PathValidTest, Absolute_ControlChar_0100, TestSize.Level0)
{
    EXPECT_FALSE(IsPathValid("/data/app/foo\t.hap", true));
}

// Relaxed on purpose: procfs/devfs targets are no longer rejected, the
// traversal and direction checks still hold for them.
HWTEST_F(PathValidTest, Absolute_RelaxedProcDev_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsPathValid("/proc/self/fd/3", true));
    EXPECT_TRUE(IsPathValid("/dev/fd/3", true));
}

// Relaxed on purpose: characters outside the old whitelist are accepted.
HWTEST_F(PathValidTest, Absolute_RelaxedChars_0100, TestSize.Level0)
{
    EXPECT_TRUE(IsPathValid("/data/app/foo|.hap", true));
    EXPECT_TRUE(IsPathValid("/data/app/foo hap.hap", true));
}

}  // namespace AbilityRuntime
}  // namespace OHOS
