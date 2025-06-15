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
#include "bundle_mgr_helper.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class BundleMgrHelperFirstTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static std::shared_ptr<BundleMgrHelper> bundleMgrHelper;
};

std::shared_ptr<BundleMgrHelper> BundleMgrHelperFirstTest::bundleMgrHelper =
    DelayedSingleton<BundleMgrHelper>::GetInstance();

void BundleMgrHelperFirstTest::SetUpTestCase(void)
{}

void BundleMgrHelperFirstTest::TearDownTestCase(void)
{}

void BundleMgrHelperFirstTest::SetUp()
{}

void BundleMgrHelperFirstTest::TearDown()
{}

/**
 * @tc.name: BundleMgrHelperFirstTest_Connect_001
 * @tc.desc: Connect
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_Connect_001, TestSize.Level1)
{
    auto ret = bundleMgrHelper->Connect();
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: BundleMgrHelperFirstTest_Connect_002
 * @tc.desc: Connect
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_Connect_002, TestSize.Level1)
{
    auto ret = bundleMgrHelper->Connect(true);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name: BundleMgrHelperFirstTest_Connect_001
 * @tc.desc: Connect
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_Connect_003, TestSize.Level1)
{
    auto ret = bundleMgrHelper->Connect(false);
    EXPECT_NE(ret, nullptr); 
}

/**
 * @tc.name: BundleMgrHelperFirstTest_GetNameAndIndexForUid_001
 * @tc.desc: GetNameAndIndexForUid
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_GetNameAndIndexForUid_001, TestSize.Level1)
{
    std::string name;
    int32_t uid = 1;
    int32_t appIndex;
    auto ret = bundleMgrHelper->GetNameAndIndexForUid(uid, name, appIndex);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: BundleMgrHelperFirstTest_GetAppIdByBundleName_001
 * @tc.desc: GetAppIdByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_GetAppIdByBundleName_001, TestSize.Level1)
{
    std::string name = "name";
    int32_t uid = 1;
    auto ret = bundleMgrHelper->GetAppIdByBundleName(name, uid);
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: BundleMgrHelperFirstTest_GetAppIdByBundleName_002
 * @tc.desc: GetAppIdByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_GetAppIdByBundleName_002, TestSize.Level1)
{
    std::string name = "";
    int32_t uid = 1;
    auto ret = bundleMgrHelper->GetAppIdByBundleName(name, uid);
    EXPECT_TRUE(ret.empty());
}

/**
 * @tc.name: BundleMgrHelperFirstTest_SetBmsReady_001
 * @tc.desc: SetBmsReady
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_SetBmsReady_001, TestSize.Level1)
{
    bundleMgrHelper->SetBmsReady(true);
    EXPECT_EQ(bundleMgrHelper->bmsReady_, true);
}

/**
 * @tc.name: BundleMgrHelperFirstTest_SetBmsReady_002
 * @tc.desc: SetBmsReady
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_SetBmsReady_002, TestSize.Level1)
{
    bundleMgrHelper->SetBmsReady(false);
    EXPECT_NE(bundleMgrHelper->bmsReady_, true);
}

/**
 * @tc.name: BundleMgrHelperFirstTest_GetBundleInfoForSelfWithOutCache_001
 * @tc.desc: GetBundleInfoForSelfWithOutCache
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_GetBundleInfoForSelfWithOutCache_001, TestSize.Level1)
{
    int32_t flags = 0;
    BundleInfo bundleInfo;
    auto ret = bundleMgrHelper->GetBundleInfoForSelfWithOutCache(flags, bundleInfo);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: BundleMgrHelperFirstTest_GetApplicationInfoWithAppIndex_001
 * @tc.desc: GetApplicationInfoWithAppIndex
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_GetApplicationInfoWithAppIndex_001, TestSize.Level1)
{
    std::string appName;
    int32_t appIndex = 1;
    int userId = 1;
    ApplicationInfo appInfo;
    auto ret = bundleMgrHelper->GetApplicationInfoWithAppIndex(appName, appIndex, userId, appInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperFirstTest_QueryAppGalleryBundleName_001
 * @tc.desc: QueryAppGalleryBundleName
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_QueryAppGalleryBundleName_001, TestSize.Level1)
{
    std::string bundleName;
    auto ret = bundleMgrHelper->QueryAppGalleryBundleName(bundleName);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: BundleMgrHelperFirstTest_QueryAppGalleryBundleName_002
 * @tc.desc: QueryAppGalleryBundleName
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_QueryAppGalleryBundleName_002, TestSize.Level1)
{
    std::string bundleName = "cn.com.bundleName";
    auto ret = bundleMgrHelper->QueryAppGalleryBundleName(bundleName);
    EXPECT_NE(ret, true);
}

/**
 * @tc.name: BundleMgrHelperFirstTest_GetCloneAppIndexes_001
 * @tc.desc: GetCloneAppIndexes
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_GetCloneAppIndexes_001, TestSize.Level1)
{
    std::string bundleName;
    std::vector<int32_t> appIndexes;
    int32_t userId = 1;
    auto ret = bundleMgrHelper->GetCloneAppIndexes(bundleName, appIndexes, userId);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: BundleMgrHelperFirstTest_GetStringById_001
 * @tc.desc: GetStringById
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_GetStringById_001, TestSize.Level1)
{
    std::string bundleName;
    std::string moduleName;
    uint32_t resId = 1;
    int32_t userId = 1;
    auto ret = bundleMgrHelper->GetStringById(bundleName, moduleName, resId, userId);
    EXPECT_TRUE(ret.empty()); 
}

/**
 * @tc.name: BundleMgrHelperFirstTest_GetDataDir_001
 * @tc.desc: GetDataDir
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_GetDataDir_001, TestSize.Level1)
{
    std::string bundleName;
    int32_t appIndex = 1;
    auto ret = bundleMgrHelper->GetDataDir(bundleName, appIndex);
    EXPECT_FALSE(ret.empty()); 
}

/**
 * @tc.name: BundleMgrHelperFirstTest_GetLauncherAbilityInfoSync_001
 * @tc.desc: GetLauncherAbilityInfoSync
 * @tc.type: FUNC
 */
HWTEST_F(BundleMgrHelperFirstTest, BundleMgrHelperFirstTest_GetLauncherAbilityInfoSync_001, TestSize.Level1)
{
    std::string bundleName;
    int32_t userId = 1;
    std::vector<AbilityInfo> abilityInfo;
    auto ret = bundleMgrHelper->GetLauncherAbilityInfoSync(bundleName, userId, abilityInfo);
    EXPECT_NE(ret, ERR_OK);
}
}  // namespace AppExecFwk
}  // namespace OHOS