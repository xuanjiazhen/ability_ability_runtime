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

#include "ability_util.h"
#define private public
#define protected public
#include "rdb/ability_resident_process_rdb.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AAFwk {
class AbilityResidentProcessRdbTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AbilityResidentProcessRdbTest::SetUpTestCase() {}

void AbilityResidentProcessRdbTest::TearDownTestCase() {}

void AbilityResidentProcessRdbTest::SetUp() {}

void AbilityResidentProcessRdbTest::TearDown() {}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb Init
 */
HWTEST_F(AbilityResidentProcessRdbTest, Init_001, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().Init(), Rdb_OK);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: Init
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb Init
 */

HWTEST_F(AbilityResidentProcessRdbTest, Init_002, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().Init(), Rdb_OK);
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().Init(), Rdb_OK);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: VerifyConfigurationPermissions
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb VerifyConfigurationPermissions
 */
HWTEST_F(AbilityResidentProcessRdbTest, VerifyConfigurationPermissions_001, TestSize.Level1) {
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().VerifyConfigurationPermissions(
        "test.com", "test.com"), Rdb_OK);
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().VerifyConfigurationPermissions(
        "", "test.com"), Rdb_Parameter_Err);
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().VerifyConfigurationPermissions(
        "", ""), Rdb_Parameter_Err);
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().VerifyConfigurationPermissions(
        "com.target.bundle", "com.caller.bundle"), Rdb_Search_Record_Err);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: GetResidentProcessEnable
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb GetResidentProcessEnable
 */
HWTEST_F(AbilityResidentProcessRdbTest, GetResidentProcessEnable_001, TestSize.Level1) {
    bool enable = false;
    EXPECT_NE(AmsResidentProcessRdb::GetInstance().GetResidentProcessEnable(
        "test.com", enable), Rdb_OK);
    EXPECT_EQ(enable, false);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: UpdateResidentProcessEnable
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb UpdateResidentProcessEnable
 */
HWTEST_F(AbilityResidentProcessRdbTest, UpdateResidentProcessEnable_001, TestSize.Level1) {
    std::string bundleName;
    bool enable = true;
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().UpdateResidentProcessEnable(bundleName,
        enable), Rdb_Parameter_Err);
    std::string validBundle = "com.ohos.resident.test";
    EXPECT_EQ(AmsResidentProcessRdb::GetInstance().UpdateResidentProcessEnable(validBundle,
        enable), Rdb_OK);
}

/*
 * Feature: AbilityResidentProcessRdb
 * Function: RemoveData
 * SubFunction: NA
 * FunctionPoints: AbilityResidentProcessRdb RemoveData
 */
HWTEST_F(AbilityResidentProcessRdbTest, RemoveData_001, TestSize.Level1) {
    std::string bundleName;
    EXPECT_NE(AmsResidentProcessRdb::GetInstance().RemoveData(bundleName), NativeRdb::E_ERROR);
    std::string validBundle = "com.ohos.resident.test";
    EXPECT_NE(AmsResidentProcessRdb::GetInstance().RemoveData(validBundle), NativeRdb::E_ERROR);
}
} // namespace AAFwk
} // namespace OHOS
