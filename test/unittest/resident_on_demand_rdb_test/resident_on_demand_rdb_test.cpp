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
#include "rdb/resident_on_demand_rdb.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AAFwk {
class ResidentOnDemandRdbTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void ResidentOnDemandRdbTest::SetUpTestCase() {}

void ResidentOnDemandRdbTest::TearDownTestCase() {}

void ResidentOnDemandRdbTest::SetUp() {}

void ResidentOnDemandRdbTest::TearDown() {}

/*
 * Feature: ResidentOnDemandRdb
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: ResidentOnDemandRdb Init
 */
HWTEST_F(ResidentOnDemandRdbTest, Init_001, TestSize.Level1)
{
    EXPECT_EQ(ResidentOnDemandRdb::GetInstance().Init(), 0);
}

/*
 * Feature: ResidentOnDemandRdb
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: ResidentOnDemandRdb Init multiple times
 */
HWTEST_F(ResidentOnDemandRdbTest, Init_002, TestSize.Level1)
{
    EXPECT_EQ(ResidentOnDemandRdb::GetInstance().Init(), 0);
    EXPECT_EQ(ResidentOnDemandRdb::GetInstance().Init(), 0);
}

/*
 * Feature: ResidentOnDemandRdb
 * Function: SetOnDemandStartup
 * SubFunction: NA
 * FunctionPoints: ResidentOnDemandRdb SetOnDemandStartup with empty bundle name
 */
HWTEST_F(ResidentOnDemandRdbTest, SetOnDemandStartup_001, TestSize.Level1)
{
    std::string emptyBundleName = "";
    bool onDemandStartup = true;
    int32_t result = ResidentOnDemandRdb::GetInstance().SetOnDemandStartup(emptyBundleName, onDemandStartup);
    EXPECT_NE(result, 0);
}

/*
 * Feature: ResidentOnDemandRdb
 * Function: SetOnDemandStartup
 * SubFunction: NA
 * FunctionPoints: ResidentOnDemandRdb SetOnDemandStartup with valid bundle name
 */
HWTEST_F(ResidentOnDemandRdbTest, SetOnDemandStartup_002, TestSize.Level1)
{
    ResidentOnDemandRdb::GetInstance().Init();
    std::string bundleName = "com.test.ondemand";
    bool onDemandStartup = true;
    int32_t result = ResidentOnDemandRdb::GetInstance().SetOnDemandStartup(bundleName, onDemandStartup);
    EXPECT_EQ(result, 0);

    // Verify the value was set correctly
    bool getValue = false;
    getValue = ResidentOnDemandRdb::GetInstance().GetOnDemandStartup(bundleName);
    EXPECT_EQ(getValue, true);
}

/*
 * Feature: ResidentOnDemandRdb
 * Function: GetOnDemandStartup
 * SubFunction: NA
 * FunctionPoints: ResidentOnDemandRdb GetOnDemandStartup with empty bundle name
 */
HWTEST_F(ResidentOnDemandRdbTest, GetOnDemandStartup_001, TestSize.Level1)
{
    std::string emptyBundleName = "";
    bool result = ResidentOnDemandRdb::GetInstance().GetOnDemandStartup(emptyBundleName);
    EXPECT_EQ(result, false);
}

/*
 * Feature: ResidentOnDemandRdb
 * Function: GetOnDemandStartup
 * SubFunction: NA
 * FunctionPoints: ResidentOnDemandRdb GetOnDemandStartup with non-existent bundle name
 */
HWTEST_F(ResidentOnDemandRdbTest, GetOnDemandStartup_002, TestSize.Level1)
{
    ResidentOnDemandRdb::GetInstance().Init();
    std::string bundleName = "com.test.nonexistent";
    bool result = ResidentOnDemandRdb::GetInstance().GetOnDemandStartup(bundleName);
    EXPECT_EQ(result, false);
}

/*
 * Feature: ResidentOnDemandRdb
 * Function: SetOnDemandStartupBatch
 * SubFunction: NA
 * FunctionPoints: ResidentOnDemandRdb SetOnDemandStartupBatch with empty map
 */
HWTEST_F(ResidentOnDemandRdbTest, SetOnDemandStartupBatch_001, TestSize.Level1)
{
    std::map<std::string, bool> emptyConfigList;
    int32_t result = ResidentOnDemandRdb::GetInstance().SetOnDemandStartupBatch(emptyConfigList);
    EXPECT_NE(result, 0);
}

/*
 * Feature: ResidentOnDemandRdb
 * Function: SetOnDemandStartupBatch
 * SubFunction: NA
 * FunctionPoints: ResidentOnDemandRdb SetOnDemandStartupBatch with valid config list
 */
HWTEST_F(ResidentOnDemandRdbTest, SetOnDemandStartupBatch_002, TestSize.Level1)
{
    ResidentOnDemandRdb::GetInstance().Init();
    std::map<std::string, bool> configList;
    configList["com.test.bundle1"] = true;
    configList["com.test.bundle2"] = false;
    configList["com.test.bundle3"] = true;

    int32_t result = ResidentOnDemandRdb::GetInstance().SetOnDemandStartupBatch(configList);
    EXPECT_EQ(result, 0);

    // Verify the values were set correctly
    EXPECT_EQ(ResidentOnDemandRdb::GetInstance().GetOnDemandStartup("com.test.bundle1"), true);
    EXPECT_EQ(ResidentOnDemandRdb::GetInstance().GetOnDemandStartup("com.test.bundle2"), false);
    EXPECT_EQ(ResidentOnDemandRdb::GetInstance().GetOnDemandStartup("com.test.bundle3"), true);
}

/*
 * Feature: ResidentOnDemandRdb
 * Function: SetOnDemandStartup
 * SubFunction: NA
 * FunctionPoints: ResidentOnDemandRdb SetOnDemandStartup disable
 */
HWTEST_F(ResidentOnDemandRdbTest, SetOnDemandStartup_003, TestSize.Level1)
{
    ResidentOnDemandRdb::GetInstance().Init();
    std::string bundleName = "com.test.disable";

    // First enable
    int32_t result = ResidentOnDemandRdb::GetInstance().SetOnDemandStartup(bundleName, true);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(ResidentOnDemandRdb::GetInstance().GetOnDemandStartup(bundleName), true);

    // Then disable
    result = ResidentOnDemandRdb::GetInstance().SetOnDemandStartup(bundleName, false);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(ResidentOnDemandRdb::GetInstance().GetOnDemandStartup(bundleName), false);
}
} // namespace AAFwk
} // namespace OHOS
