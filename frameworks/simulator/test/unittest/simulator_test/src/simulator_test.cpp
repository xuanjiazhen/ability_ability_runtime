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
#include <gtest/hwext/gtest-multithread.h>
#include <string>

#include "dfx_jsnapi.h"
#include "simulator.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class SimulatorTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SimulatorTest::SetUpTestCase() {}
void SimulatorTest::TearDownTestCase() {}
void SimulatorTest::SetUp() {}
void SimulatorTest::TearDown() {}

// Same eager source map format as sourceMapTest.h sourceMapData
static const std::string EAGER_SOURCE_MAP_DATA =
    "{\n"
    "  \"entry/src/main/ets/pages/Index.ts\": {\n"
    "    \"version\": 3,\n"
    "    \"file\": \"Index.ets\",\n"
    "    \"sources\": [\n"
    "      \"entry/src/main/ets/pages/Index.ets\"\n"
    "    ],\n"
    "    \"names\": [],\n"
    "    \"mappings\": \"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\"\n"
    "    \"entry-package-info\": \"entry|1.0.0\"\n"
    "  }\n"
    "}";

/**
 * @tc.name: TranslateErrorStack_NoSourceMap_001
 * @tc.desc: Verify Simulator::TranslateErrorStack skips InitStatus and keeps raw stack for unknown URL
 * @tc.type: FUNC
 */
HWTEST_F(SimulatorTest, TranslateErrorStack_NoSourceMap_001, TestSize.Level0)
{
    // useInitStatus=false: even when InitStatus=IN_EXECUTED, status parsing is skipped
    panda::DFXJSNApi::SourceMapSetInitStatus(panda::ecmascript::InitStatus::IN_EXECUTED);
    std::string stackStr = "at foo (unknown.js:1:1)";
    std::string result = Simulator::TranslateErrorStack(stackStr);
    EXPECT_EQ(result.find("SourceMap is not initialized yet"), std::string::npos);
    EXPECT_NE(result.find("unknown.js"), std::string::npos);

    panda::DFXJSNApi::SourceMapSetInitStatus(panda::ecmascript::InitStatus::NOT_EXECUTED);
}

/**
 * @tc.name: TranslateErrorStack_WithSourceMap_001
 * @tc.desc: Verify Simulator::TranslateErrorStack performs real mapping with eager data, regardless of InitStatus
 * @tc.type: FUNC
 */
HWTEST_F(SimulatorTest, TranslateErrorStack_WithSourceMap_001, TestSize.Level0)
{
    // InitStatus=NO_SOURCEMAP: status-based parsing would refuse to translate, but
    // useInitStatus=false bypasses the status check and maps via eager source map data
    panda::DFXJSNApi::SourceMapSetInitStatus(panda::ecmascript::InitStatus::NO_SOURCEMAP);
    panda::DFXJSNApi::SourceMapSplitSourceMap(EAGER_SOURCE_MAP_DATA);

    std::string stackStr = "at func (entry/src/main/ets/pages/Index.ts:10:5)";
    std::string result = Simulator::TranslateErrorStack(stackStr);
    EXPECT_EQ(result.find("Cannot get SourceMap info"), std::string::npos);
    EXPECT_NE(result.find("entry/src/main/ets/pages/Index.ets"), std::string::npos);

    panda::DFXJSNApi::SourceMapSetInitStatus(panda::ecmascript::InitStatus::NOT_EXECUTED);
}

/**
 * @tc.name: TranslateErrorStack_EmptyStack_001
 * @tc.desc: Verify Simulator::TranslateErrorStack handles empty stack boundary case
 * @tc.type: FUNC
 */
HWTEST_F(SimulatorTest, TranslateErrorStack_EmptyStack_001, TestSize.Level0)
{
    std::string result = Simulator::TranslateErrorStack("");
    EXPECT_FALSE(result.empty());
}
} // namespace AbilityRuntime
} // namespace OHOS
