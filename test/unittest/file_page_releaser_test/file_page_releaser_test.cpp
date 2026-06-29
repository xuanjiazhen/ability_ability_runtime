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
#include <vector>

#include "file_page_releaser.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class FilePageReleaserTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

// Validation must reject empty input before any bundle lookup.
HWTEST_F(FilePageReleaserTest, ReleaseByFileNames_Empty_0100, TestSize.Level1)
{
    EXPECT_EQ(FilePageReleaser::ReleaseByFileNames({}), FilePageReleaser::ERR_INVALID_VALUE);
}

// Path separators / traversal must be rejected (atomic, before release).
HWTEST_F(FilePageReleaserTest, ReleaseByFileNames_Traversal_0100, TestSize.Level1)
{
    EXPECT_EQ(FilePageReleaser::ReleaseByFileNames({"../etc/passwd"}),
        FilePageReleaser::ERR_INVALID_VALUE);
    EXPECT_EQ(FilePageReleaser::ReleaseByFileNames({"/system/lib/libfoo.so"}),
        FilePageReleaser::ERR_INVALID_VALUE);
    EXPECT_EQ(FilePageReleaser::ReleaseByFileNames({"sub/libfoo.so"}),
        FilePageReleaser::ERR_INVALID_VALUE);
}

// Unsupported suffix must be rejected.
HWTEST_F(FilePageReleaserTest, ReleaseByFileNames_Suffix_0100, TestSize.Level1)
{
    EXPECT_EQ(FilePageReleaser::ReleaseByFileNames({"readme.txt"}),
        FilePageReleaser::ERR_INVALID_VALUE);
    EXPECT_EQ(FilePageReleaser::ReleaseByFileNames({"image.png"}),
        FilePageReleaser::ERR_INVALID_VALUE);
}

// Atomic: one illegal item among legal ones -> whole call rejected.
HWTEST_F(FilePageReleaserTest, ReleaseByFileNames_Atomic_0100, TestSize.Level1)
{
    EXPECT_EQ(FilePageReleaser::ReleaseByFileNames({"libfoo.so", "bad.txt"}),
        FilePageReleaser::ERR_INVALID_VALUE);
}

// Empty module name rejected.
HWTEST_F(FilePageReleaserTest, ReleaseRawFileByModule_Empty_0100, TestSize.Level1)
{
    EXPECT_EQ(FilePageReleaser::ReleaseRawFileByModule(""), FilePageReleaser::ERR_INVALID_VALUE);
}

// Module names with separators rejected.
HWTEST_F(FilePageReleaserTest, ReleaseRawFileByModule_Separator_0100, TestSize.Level1)
{
    EXPECT_EQ(FilePageReleaser::ReleaseRawFileByModule("a/b"), FilePageReleaser::ERR_INVALID_VALUE);
    EXPECT_EQ(FilePageReleaser::ReleaseRawFileByModule(".."), FilePageReleaser::ERR_INVALID_VALUE);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
