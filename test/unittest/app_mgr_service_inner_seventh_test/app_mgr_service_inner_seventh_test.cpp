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
#include "app_mgr_service_inner.h"
#undef private
#include "user_record_manager.h"
#include "mock_my_status.h"
#include "ability_manager_errors.h"
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using OHOS::AppExecFwk::ExtensionAbilityType;
namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInnerSeventhTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceInnerSeventhTest::SetUpTestCase() {}

void AppMgrServiceInnerSeventhTest::TearDownTestCase() {}

void AppMgrServiceInnerSeventhTest::SetUp() {}

void AppMgrServiceInnerSeventhTest::TearDown() {}

/**
 * @tc.name: GetBundleAndHapInfo_001
 * @tc.desc: test GetBundleAndHapInfo_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBundleAndHapInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleAndHapInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    AbilityInfo abilityInfo;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    int32_t appIndex = 0;
    bool ret = appMgrServiceInner->GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "GetBundleAndHapInfo_001 end");
}

/**
 * @tc.name: GetBundleAndHapInfo_002
 * @tc.desc: test GetBundleAndHapInfo_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, GetBundleAndHapInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleAndHapInfo_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getSandboxBundleInfo_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().getSandboxHapModuleInfo_ = ERR_OK;
    AbilityInfo abilityInfo;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    int32_t appIndex = 1001;
    bool ret = appMgrServiceInner->GetBundleAndHapInfo(abilityInfo, appInfo, bundleInfo, hapModuleInfo, appIndex);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "GetBundleAndHapInfo_002 end");
}

/**
 * @tc.name: UpdateApplicationInfoInstalled_001
 * @tc.desc: test UpdateApplicationInfoInstalled_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateApplicationInfoInstalled_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    AAFwk::MyStatus::GetInstance().getCallingUid_ = 0;
    auto bundleMgrHelper = appMgrServiceInner->remoteClientManager_->GetBundleManagerHelper();

    std::string bundleName = "";
    int uid = 0;
    std::string moduleName = "";
    bool isPlugin = true;
    int32_t ret = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, isPlugin);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_001 end");
}

/**
 * @tc.name: UpdateApplicationInfoInstalled_002
 * @tc.desc: test UpdateApplicationInfoInstalled_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateApplicationInfoInstalled_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = 0;
    AAFwk::MyStatus::GetInstance().processUpdate_ = ERR_INVALID_STATE;

    std::string bundleName = "";
    int uid = 0;
    std::string moduleName = "";
    bool isPlugin = true;
    int32_t ret = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, isPlugin);
    EXPECT_EQ(ret, ERR_INVALID_STATE);
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_002 end");
}

/**
 * @tc.name: UpdateApplicationInfoInstalled_003
 * @tc.desc: test UpdateApplicationInfoInstalled_003
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateApplicationInfoInstalled_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getCallingUid_ = 0;
    AAFwk::MyStatus::GetInstance().processUpdate_ = ERR_OK;

    std::string bundleName = "";
    int uid = 0;
    std::string moduleName = "";
    bool isPlugin = true;
    int32_t ret = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName, isPlugin);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_003 end");
}

/**
 * @tc.name: KillApplication_001
 * @tc.desc: test KillApplication_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, KillApplication_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;

    std::string bundleName = "";
    bool clearPageStack = false;
    int32_t appIndex = 0;
    int32_t ret = appMgrServiceInner->KillApplication(bundleName, clearPageStack, appIndex);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "KillApplication_001 end");
}

/**
 * @tc.name: ForceKillApplication_001
 * @tc.desc: test ForceKillApplication_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, ForceKillApplication_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ForceKillApplication_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().isSACall_ = false;
    AAFwk::MyStatus::GetInstance().isShellCall_ = false;
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getNameForUid_ = "com.ohos.sceneboard";
    appMgrServiceInner->appRunningManager_ = nullptr;

    std::string bundleName = "";
    int userId = 0;
    int appIndex = 0;
    int32_t ret = appMgrServiceInner->ForceKillApplication(bundleName, userId, appIndex);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "ForceKillApplication_001 end");
}

/**
 * @tc.name: KillProcessesByAccessTokenId_001
 * @tc.desc: test ForceKillApplicationInner_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, KillProcessesByAccessTokenId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByAccessTokenId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;

    int32_t accessTokenId = 0;
    int32_t ret = appMgrServiceInner->KillProcessesByAccessTokenId(accessTokenId);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByAccessTokenId_001 end");
}

/**
 * @tc.name: KillProcessesByAccessTokenId_002
 * @tc.desc: test ForceKillApplicationInner_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, KillProcessesByAccessTokenId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByAccessTokenId_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = true;
    appMgrServiceInner->appRunningManager_ = nullptr;
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;

    int32_t accessTokenId = 0;
    int32_t ret = appMgrServiceInner->KillProcessesByAccessTokenId(accessTokenId);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByAccessTokenId_002 end");
}

/**
 * @tc.name: UpdateProcessMemoryState_001
 * @tc.desc: test UpdateProcessMemoryState_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateProcessMemoryState_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateProcessMemoryState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string temp = "";
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, temp);

    ProcessMemoryState state;
    std::vector<ProcessMemoryState> procMemState = {state};
    int32_t ret = appMgrServiceInner->UpdateProcessMemoryState(procMemState);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UpdateProcessMemoryState_001 end");
}

/**
 * @tc.name: UpdateProcessMemoryState_002
 * @tc.desc: test UpdateProcessMemoryState_002
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, UpdateProcessMemoryState_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateProcessMemoryState_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = nullptr;

    ProcessMemoryState state;
    std::vector<ProcessMemoryState> procMemState = {state};
    int32_t ret = appMgrServiceInner->UpdateProcessMemoryState(procMemState);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "UpdateProcessMemoryState_002 end");
}

/**
 * @tc.name: KillApplicationByUserIdLocked_001
 * @tc.desc: test KillApplicationByUserIdLocked_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, KillApplicationByUserIdLocked_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByUserIdLocked_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    std::string bundleName = "";
    int32_t appCloneIndex = 0;
    int userId = 0;
    KillProcessConfig config;
    int32_t ret = appMgrServiceInner->KillApplicationByUserIdLocked(bundleName, appCloneIndex, userId, config);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillApplicationByUserIdLocked_001 end");
}

/**
 * @tc.name: ClearUpApplicationDataBySelf_001
 * @tc.desc: test ClearUpApplicationDataBySelf_001
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSeventhTest, ClearUpApplicationDataBySelf_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataBySelf_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string temp = "";
    AAFwk::MyStatus::GetInstance().getAppRunningRecordByPid_ = std::make_shared<AppRunningRecord>(nullptr, 0, temp);

    int32_t callerUid = 0;
    pid_t callerPid = 0;
    int32_t userId = -1;
    int32_t ret = appMgrServiceInner->ClearUpApplicationDataBySelf(callerUid, callerPid, userId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataBySelf_001 end");
}

/**
* @tc.name: ClearUpApplicationDataByUserId_001
* @tc.desc: test ClearUpApplicationDataByUserId_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, ClearUpApplicationDataByUserId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    const std::string bundleName = "";
    int32_t callerUid = 1;
    pid_t callerPid = 1;
    int32_t appCloneIndex = 1;
    int32_t userId = 0;
    bool isBySelf = false;
    std::string reason = "";
    int32_t ret = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex,
        userId, isBySelf, reason);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_001 end");
}

/**
* @tc.name: ClearUpApplicationDataByUserId_002
* @tc.desc: test ClearUpApplicationDataByUserId_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, ClearUpApplicationDataByUserId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().clearUserGranted_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().cleanBundleDataFiles_ = false;

    const std::string bundleName = "";
    int32_t callerUid = 1;
    pid_t callerPid = 1;
    int32_t appCloneIndex = 1;
    int32_t userId = 0;
    bool isBySelf = false;
    std::string reason = "";
    int32_t ret = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex,
        userId, isBySelf, reason);
    EXPECT_EQ(ret, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_002 end");
}

/**
* @tc.name: ClearUpApplicationDataByUserId_003
* @tc.desc: test ClearUpApplicationDataByUserId_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, ClearUpApplicationDataByUserId_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().clearUserGranted_ = ERR_OK;
    AAFwk::MyStatus::GetInstance().cleanBundleDataFiles_ = true;
    appMgrServiceInner->appRunningManager_ = nullptr;

    const std::string bundleName = "";
    int32_t callerUid = 1;
    pid_t callerPid = 1;
    int32_t appCloneIndex = 1;
    int32_t userId = 0;
    bool isBySelf = true;
    std::string reason = "";
    int32_t ret = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex,
        userId, isBySelf, reason);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ClearUpApplicationDataByUserId_003 end");
}

/**
* @tc.name: GetRunningProcessesByBundleType_001
* @tc.desc: test GetRunningProcessesByBundleType_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetRunningProcessesByBundleType_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = true;
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    std::string temp = "";
    auto appRecord = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    appRecord->SetUid(0);
    auto appRecord2 = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    appRecord2->SetUid(200000);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(0, appRecord));
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, appRecord2));
    appMgrServiceInner->currentUserId_ = 1;

    BundleType bundleType = BundleType::APP;
    std::vector<RunningProcessInfo> info;
    int32_t ret = appMgrServiceInner->GetRunningProcessesByBundleType(bundleType, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_001 end");
}

/**
* @tc.name: GetRunningProcessesByBundleType_002
* @tc.desc: test GetRunningProcessesByBundleType_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetRunningProcessesByBundleType_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = true;
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    std::string temp = "";
    auto appRecord2 = std::make_shared<AppRunningRecord>(nullptr, 0, temp);
    appRecord2->SetUid(200000);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.clear();
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, appRecord2));
    appMgrServiceInner->currentUserId_ = 1;

    BundleType bundleType = BundleType::APP;
    std::vector<RunningProcessInfo> info;
    int32_t ret = appMgrServiceInner->GetRunningProcessesByBundleType(bundleType, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_002 end");
}

/**
* @tc.name: GetRunningProcessesByBundleType_003
* @tc.desc: test GetRunningProcessesByBundleType_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetRunningProcessesByBundleType_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = true;
    AAFwk::MyStatus::GetInstance().judgeCallerIsAllowed_ = true;
    std::string temp = "";
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>();
    info->bundleType = BundleType::APP;
    auto appRecord2 = std::make_shared<AppRunningRecord>(info, 0, temp);
    appRecord2->SetUid(200000);
    AAFwk::MyStatus::GetInstance().getAppRunningRecordMap_.insert(std::pair<const int32_t,
        const std::shared_ptr<AppExecFwk::AppRunningRecord>>(1, appRecord2));
    appMgrServiceInner->currentUserId_ = 1;

    BundleType bundleType = BundleType::APP;
    std::vector<RunningProcessInfo> info2;
    int32_t ret = appMgrServiceInner->GetRunningProcessesByBundleType(bundleType, info2);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetRunningProcessesByBundleType_003 end");
}

/**
* @tc.name: GetAllRunningInstanceKeysByBundleName_001
* @tc.desc: test GetAllRunningInstanceKeysByBundleName_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetAllRunningInstanceKeysByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;

    std::string bundleName = "";
    std::vector<std::string> instanceKeys;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys, userId);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_001 end");
}

/**
* @tc.name: GetAllRunningInstanceKeysByBundleNameInner_001
* @tc.desc: test GetAllRunningInstanceKeysByBundleNameInner_001
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetAllRunningInstanceKeysByBundleNameInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;

    std::string bundleName = "111";
    std::vector<std::string> instanceKeys;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_001 end");
}

/**
* @tc.name: GetAllRunningInstanceKeysByBundleNameInner_002
* @tc.desc: test GetAllRunningInstanceKeysByBundleNameInner_002
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetAllRunningInstanceKeysByBundleNameInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_002 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    appMgrServiceInner->appRunningManager_ = nullptr;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().getApplicationInfo_ = true;
    AAFwk::MyStatus::GetInstance().applicationInfo_ = {};
    AAFwk::MyStatus::GetInstance().applicationInfo_.multiAppMode.multiAppModeType =
        AppExecFwk::MultiAppModeType::MULTI_INSTANCE;

    std::string bundleName = "111";
    std::vector<std::string> instanceKeys;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_002 end");
}

/**
* @tc.name: GetAllRunningInstanceKeysByBundleNameInner_003
* @tc.desc: test GetAllRunningInstanceKeysByBundleNameInner_003
* @tc.type: FUNC
*/
HWTEST_F(AppMgrServiceInnerSeventhTest, GetAllRunningInstanceKeysByBundleNameInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_003 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AAFwk::MyStatus::GetInstance().verifyCallingPermission_ = false;
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
    AAFwk::MyStatus::GetInstance().getApplicationInfo_ = true;
    AAFwk::MyStatus::GetInstance().applicationInfo_ = {};
    AAFwk::MyStatus::GetInstance().applicationInfo_.multiAppMode.multiAppModeType =
        AppExecFwk::MultiAppModeType::UNSPECIFIED;
    std::string bundleName = "111";
    std::vector<std::string> instanceKeys;
    int32_t userId = 0;
    int32_t ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleNameInner(bundleName, instanceKeys, userId);
    EXPECT_EQ(ret, ERR_MULTI_INSTANCE_NOT_SUPPORTED);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleNameInner_003 end");
}
} // namespace AppExecFwk
} // namespace OHOS