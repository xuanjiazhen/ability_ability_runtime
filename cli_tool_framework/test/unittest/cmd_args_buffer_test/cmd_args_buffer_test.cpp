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

#include <cstring>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cmd_args_buffer.h"

using namespace testing::ext;
using namespace OHOS::CliTool;

namespace OHOS {
namespace CliTool {
class CmdArgsBufferTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CmdArgsBufferTest::SetUpTestCase(void)
{
    // Initialize test environment
}

void CmdArgsBufferTest::TearDownTestCase(void)
{
    // Cleanup test environment
}

void CmdArgsBufferTest::SetUp()
{
    // Reset state before each test
}

void CmdArgsBufferTest::TearDown()
{
    // Cleanup after each test
}

/**
 * @tc.name: CmdArgsBuffer_Construction_0100
 * @tc.desc: Test default construction of CmdArgsBuffer
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, Construction_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_Construction_0100 start";

    CmdArgsBuffer buffer;
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_NE(buffer.data(), nullptr);
    EXPECT_EQ(buffer[0], nullptr); // Should be null-terminated

    GTEST_LOG_(INFO) << "CmdArgsBuffer_Construction_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_AddArg_0100
 * @tc.desc: Test AddArg method with single argument
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, AddArg_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddArg_0100 start";

    CmdArgsBuffer buffer;
    buffer.AddArg("/usr/bin/command");

    EXPECT_EQ(buffer.size(), 1);
    EXPECT_NE(buffer[0], nullptr);
    EXPECT_STREQ(buffer[0], "/usr/bin/command");
    EXPECT_EQ(buffer[1], nullptr); // Null-terminated

    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddArg_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_AddArg_0200
 * @tc.desc: Test AddArg method with multiple arguments
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, AddArg_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddArg_0200 start";

    CmdArgsBuffer buffer;
    buffer.AddArg("/usr/bin/git");
    buffer.AddArg("status");
    buffer.AddArg("--verbose");

    EXPECT_EQ(buffer.size(), 3);
    EXPECT_STREQ(buffer[0], "/usr/bin/git");
    EXPECT_STREQ(buffer[1], "status");
    EXPECT_STREQ(buffer[2], "--verbose");
    EXPECT_EQ(buffer[3], nullptr); // Null-terminated

    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddArg_0200 end";
}

/**
 * @tc.name: CmdArgsBuffer_AddParam_0100
 * @tc.desc: Test AddParam method with key-value pair
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, AddParam_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddParam_0100 start";

    CmdArgsBuffer buffer;
    buffer.AddParam("output", "file.txt");

    EXPECT_EQ(buffer.size(), 2);
    EXPECT_STREQ(buffer[0], "--output");
    EXPECT_STREQ(buffer[1], "file.txt");
    EXPECT_EQ(buffer[2], nullptr);

    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddParam_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_AddParam_0200
 * @tc.desc: Test AddParam method with multiple key-value pairs
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, AddParam_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddParam_0200 start";

    CmdArgsBuffer buffer;
    buffer.AddParam("input", "input.txt");
    buffer.AddParam("output", "output.txt");
    buffer.AddParam("verbose", "true");

    EXPECT_EQ(buffer.size(), 6);
    EXPECT_STREQ(buffer[0], "--input");
    EXPECT_STREQ(buffer[1], "input.txt");
    EXPECT_STREQ(buffer[2], "--output");
    EXPECT_STREQ(buffer[3], "output.txt");
    EXPECT_STREQ(buffer[4], "--verbose");
    EXPECT_STREQ(buffer[5], "true");
    EXPECT_EQ(buffer[6], nullptr);

    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddParam_0200 end";
}

/**
 * @tc.name: CmdArgsBuffer_AddFlag_0100
 * @tc.desc: Test AddFlag method with single flag
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, AddFlag_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddFlag_0100 start";

    CmdArgsBuffer buffer;
    buffer.AddFlag("help");

    EXPECT_EQ(buffer.size(), 1);
    EXPECT_STREQ(buffer[0], "--help");
    EXPECT_EQ(buffer[1], nullptr);

    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddFlag_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_AddFlag_0200
 * @tc.desc: Test AddFlag method with multiple flags
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, AddFlag_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddFlag_0200 start";

    CmdArgsBuffer buffer;
    buffer.AddFlag("verbose");
    buffer.AddFlag("force");
    buffer.AddFlag("help");

    EXPECT_EQ(buffer.size(), 3);
    EXPECT_STREQ(buffer[0], "--verbose");
    EXPECT_STREQ(buffer[1], "--force");
    EXPECT_STREQ(buffer[2], "--help");
    EXPECT_EQ(buffer[3], nullptr);

    GTEST_LOG_(INFO) << "CmdArgsBuffer_AddFlag_0200 end";
}

/**
 * @tc.name: CmdArgsBuffer_MixedUsage_0100
 * @tc.desc: Test mixed usage of AddArg, AddParam, and AddFlag
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, MixedUsage_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_MixedUsage_0100 start";

    CmdArgsBuffer buffer;
    buffer.AddArg("/usr/bin/command");
    buffer.AddFlag("verbose");
    buffer.AddParam("output", "result.txt");
    buffer.AddArg("subcommand");
    buffer.AddFlag("force");

    EXPECT_EQ(buffer.size(), 7);
    EXPECT_STREQ(buffer[0], "/usr/bin/command");
    EXPECT_STREQ(buffer[1], "--verbose");
    EXPECT_STREQ(buffer[2], "--output");
    EXPECT_STREQ(buffer[3], "result.txt");
    EXPECT_STREQ(buffer[4], "subcommand");
    EXPECT_STREQ(buffer[5], "--force");
    EXPECT_EQ(buffer[6], nullptr);

    GTEST_LOG_(INFO) << "CmdArgsBuffer_MixedUsage_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_Clear_0100
 * @tc.desc: Test Clear method
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, Clear_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_Clear_0100 start";

    CmdArgsBuffer buffer;
    buffer.AddArg("test1");
    buffer.AddParam("key", "value");
    buffer.AddFlag("flag");

    EXPECT_EQ(buffer.size(), 4);

    buffer.clear();
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_EQ(buffer[0], nullptr);

    // Add again after clear
    buffer.AddArg("new");
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_STREQ(buffer[0], "new");

    GTEST_LOG_(INFO) << "CmdArgsBuffer_Clear_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_Reserve_0100
 * @tc.desc: Test Reserve method for capacity optimization
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, Reserve_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_Reserve_0100 start";

    CmdArgsBuffer buffer;
    buffer.reserve(100);

    // Add many arguments without reallocation
    for (int i = 0; i < 50; i++) {
        buffer.AddArg("arg" + std::to_string(i));
    }

    EXPECT_EQ(buffer.size(), 50);

    GTEST_LOG_(INFO) << "CmdArgsBuffer_Reserve_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_StringLifetime_0100
 * @tc.desc: Test that strings remain valid throughout buffer lifetime
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, StringLifetime_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_StringLifetime_0100 start";

    CmdArgsBuffer buffer;
    {
        std::string temp = "temporary_string";
        buffer.AddArg(temp);
        buffer.AddParam("key", temp);
    }
    // temp is out of scope, but buffer should have its own copy

    EXPECT_EQ(buffer.size(), 3);
    EXPECT_STREQ(buffer[0], "temporary_string");
    EXPECT_STREQ(buffer[1], "--key");
    EXPECT_STREQ(buffer[2], "temporary_string");

    GTEST_LOG_(INFO) << "CmdArgsBuffer_StringLifetime_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_EmptyStrings_0100
 * @tc.desc: Test handling of empty strings
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, EmptyStrings_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_EmptyStrings_0100 start";

    CmdArgsBuffer buffer;
    buffer.AddArg("");
    buffer.AddParam("key", "");
    buffer.AddFlag("");

    EXPECT_EQ(buffer.size(), 4);
    EXPECT_STREQ(buffer[0], "");
    EXPECT_STREQ(buffer[1], "--key");
    EXPECT_STREQ(buffer[2], "");
    EXPECT_STREQ(buffer[3], "--");

    GTEST_LOG_(INFO) << "CmdArgsBuffer_EmptyStrings_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_SpecialCharacters_0100
 * @tc.desc: Test handling of special characters in arguments
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, SpecialCharacters_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_SpecialCharacters_0100 start";

    CmdArgsBuffer buffer;
    buffer.AddArg("/path/to/file with spaces.txt");
    buffer.AddParam("config", "key=value&param=123");
    buffer.AddFlag("-");

    EXPECT_EQ(buffer.size(), 5);
    EXPECT_STREQ(buffer[0], "/path/to/file with spaces.txt");
    EXPECT_STREQ(buffer[1], "--config");
    EXPECT_STREQ(buffer[2], "key=value&param=123");
    EXPECT_STREQ(buffer[3], "--");

    GTEST_LOG_(INFO) << "CmdArgsBuffer_SpecialCharacters_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_LongArguments_0100
 * @tc.desc: Test handling of very long argument strings
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, LongArguments_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_LongArguments_0100 start";

    CmdArgsBuffer buffer;
    std::string longArg(1000, 'x'); // 1000 character string
    std::string longValue(5000, 'y'); // 5000 character string

    buffer.AddArg(longArg);
    buffer.AddParam("long", longValue);

    EXPECT_EQ(buffer.size(), 3);
    EXPECT_EQ(strlen(buffer[0]), 1000);
    EXPECT_EQ(strlen(buffer[2]), 5000);

    GTEST_LOG_(INFO) << "CmdArgsBuffer_LongArguments_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_NullTermination_0100
 * @tc.desc: Test that the array is always null-terminated
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, NullTermination_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_NullTermination_0100 start";

    CmdArgsBuffer buffer;
    buffer.AddArg("arg1");
    buffer.AddArg("arg2");
    buffer.AddArg("arg3");

    EXPECT_EQ(buffer.size(), 3);
    EXPECT_EQ(buffer[3], nullptr);

    buffer.clear();
    EXPECT_EQ(buffer.size(), 0);
    EXPECT_EQ(buffer[0], nullptr);

    buffer.AddArg("new");
    EXPECT_EQ(buffer.size(), 1);
    EXPECT_EQ(buffer[1], nullptr);

    GTEST_LOG_(INFO) << "CmdArgsBuffer_NullTermination_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_MoveSemantics_0100
 * @tc.desc: Test move constructor and move assignment
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, MoveSemantics_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_MoveSemantics_0100 start";

    CmdArgsBuffer buffer1;
    buffer1.AddArg("arg1");
    buffer1.AddParam("key", "value");

    // Test move constructor
    CmdArgsBuffer buffer2(std::move(buffer1));
    EXPECT_EQ(buffer2.size(), 3);
    EXPECT_STREQ(buffer2[0], "arg1");
    EXPECT_STREQ(buffer2[1], "--key");
    EXPECT_STREQ(buffer2[2], "value");

    // Test move assignment
    CmdArgsBuffer buffer3;
    buffer3 = std::move(buffer2);
    EXPECT_EQ(buffer3.size(), 3);
    EXPECT_STREQ(buffer3[0], "arg1");
    EXPECT_STREQ(buffer3[1], "--key");
    EXPECT_STREQ(buffer3[2], "value");

    GTEST_LOG_(INFO) << "CmdArgsBuffer_MoveSemantics_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_DataMethod_0100
 * @tc.desc: Test data() method returns correct pointer array
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, DataMethod_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_DataMethod_0100 start";

    CmdArgsBuffer buffer;
    buffer.AddArg("arg1");
    buffer.AddArg("arg2");

    char** ptr = buffer.data();
    EXPECT_NE(ptr, nullptr);
    EXPECT_STREQ(ptr[0], "arg1");
    EXPECT_STREQ(ptr[1], "arg2");
    EXPECT_EQ(ptr[2], nullptr);

    // Test const version
    const CmdArgsBuffer& constBuffer = buffer;
    const char* const* constPtr = constBuffer.data();
    EXPECT_NE(constPtr, nullptr);
    EXPECT_STREQ(constPtr[0], "arg1");
    EXPECT_STREQ(constPtr[1], "arg2");
    EXPECT_EQ(constPtr[2], nullptr);

    GTEST_LOG_(INFO) << "CmdArgsBuffer_DataMethod_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_IndexOperator_0100
 * @tc.desc: Test index operator for accessing individual arguments
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, IndexOperator_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_IndexOperator_0100 start";

    CmdArgsBuffer buffer;
    buffer.AddArg("first");
    buffer.AddParam("second", "value");
    buffer.AddFlag("third");

    EXPECT_STREQ(buffer[0], "first");
    EXPECT_STREQ(buffer[1], "--second");
    EXPECT_STREQ(buffer[2], "value");
    EXPECT_STREQ(buffer[3], "--third");

    // Test const version
    const CmdArgsBuffer& constBuffer = buffer;
    EXPECT_STREQ(constBuffer[0], "first");
    EXPECT_STREQ(constBuffer[1], "--second");

    GTEST_LOG_(INFO) << "CmdArgsBuffer_IndexOperator_0100 end";
}

/**
 * @tc.name: CmdArgsBuffer_RealWorldScenario_0100
 * @tc.desc: Test real-world command line construction scenario
 * @tc.type: FUNC
 */
HWTEST_F(CmdArgsBufferTest, RealWorldScenario_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CmdArgsBuffer_RealWorldScenario_0100 start";

    CmdArgsBuffer buffer;

    // Simulate building a command line like:
    // /usr/bin/git commit -m "Fix bug" --amend --author="John Doe"
    buffer.AddArg("/usr/bin/git");
    buffer.AddArg("commit");
    buffer.AddParam("m", "Fix bug");
    buffer.AddFlag("amend");
    buffer.AddParam("author", "John Doe");

    EXPECT_EQ(buffer.size(), 7);
    EXPECT_STREQ(buffer[0], "/usr/bin/git");
    EXPECT_STREQ(buffer[1], "commit");
    EXPECT_STREQ(buffer[2], "--m");
    EXPECT_STREQ(buffer[3], "Fix bug");
    EXPECT_STREQ(buffer[4], "--amend");
    EXPECT_STREQ(buffer[5], "--author");
    EXPECT_STREQ(buffer[6], "John Doe");
    EXPECT_EQ(buffer[7], nullptr);

    GTEST_LOG_(INFO) << "CmdArgsBuffer_RealWorldScenario_0100 end";
}

} // namespace OHOS
} // namespace CliTool
