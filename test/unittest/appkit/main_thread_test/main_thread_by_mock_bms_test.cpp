/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <gtest/gtest.h>

#define private public
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "main_thread.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager.h"
#include "mock_bundle_manager_service.h"
#include "mock_overlay_manager.h"
#include "mock_system_ability_manager.h"
#include "ohos_application.h"
#include "process_info.h"
#include "sys_mgr_client.h"
#include "system_ability_definition.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<BundleMgrService> mockBundleMgr = new (std::nothrow) BundleMgrService();

class MainThreadByMockBmsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<MainThread> mainThread_ = nullptr;
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
    sptr<OHOS::MockBundleManagerService> mockBundleMgrService_ = nullptr;
};

void MainThreadByMockBmsTest::SetUpTestCase() {}

void MainThreadByMockBmsTest::TearDownTestCase() {}

void MainThreadByMockBmsTest::SetUp()
{
    std::shared_ptr<EventRunner> runner = EventRunner::GetMainEventRunner();
    ASSERT_NE(runner, nullptr);
    mainThread_ = sptr<MainThread>(new (std::nothrow) MainThread());
    ASSERT_NE(mainThread_, nullptr);
    mainThread_->Init(runner);
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
}

void MainThreadByMockBmsTest::TearDown()
{
    mainThread_->applicationForDump_.reset();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
    mockBundleMgrService_ = nullptr;
}

void MainThreadByMockBmsTest::MockBundleInstallerSetup(sptr<BundleMgrService> mockBms,
    sptr<AppExecFwk::MockSystemAbilityManager> mockSaMgr,
    sptr<ISystemAbilityManager> iSaMgr)
{
    auto mockGetBundleInstaller = [mockBms]() { return mockBundleInstaller; };
    auto mockGetSystemAbility = [mockBms, iSaMgr](int32_t systemAbilityId) {
        GTEST_LOG_(INFO) << "MockBundleInstaller systemAbilityId: " << systemAbilityId << " )";
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID || systemAbilityId == APP_MGR_SERVICE_ID) {
            return mockBms->AsObject();
        } else {
            return iSaMgr->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockSaMgr, GetSystemAbility(testing::_))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
}

void MainThreadByMockBmsTest::MockBundleManagerForLaunchSetup(sptr<OHOS::MockBundleManagerService> mockBmsService,
    sptr<AppExecFwk::MockSystemAbilityManager> mockSaMgr,
    sptr<ISystemAbilityManager> iSaMgr,
    const BundleInfo &bundleInfo)
{
    EXPECT_CALL(*mockBmsService, GetBundleInfoForSelf(testing::_, testing::_))
        .WillOnce(testing::Invoke([bundleInfo](int32_t, BundleInfo &outInfo) {
            outInfo = bundleInfo;
            return ERR_OK;
        }));
    EXPECT_CALL(*mockBmsService, GetBaseSharedBundleInfos(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(ERR_OK));

    auto mockGetSystemAbility = [mockBmsService, iSaMgr](int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID || systemAbilityId == APP_MGR_SERVICE_ID) {
            return mockBmsService->AsObject();
        }
        return iSaMgr->GetSystemAbility(systemAbilityId);
    };
    EXPECT_CALL(*mockSaMgr, GetSystemAbility(testing::_))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
}

/**
 * @tc.name: SetNativeLibPath_0100
 * @tc.desc: set native lib path.
 * @tc.type: FUNC
 * @tc.require: issueI64MUJ
 */
HWTEST_F(MainThreadByMockBmsTest, SetNativeLibPath_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    Configuration config;
    AppLaunchData launchData;
    ProcessInfo processInfo("test_quickfix", 9999);
    ApplicationInfo appInfo;
    appInfo.name = "MainAbility";
    appInfo.bundleName = "com.ohos.quickfix";
    launchData.SetApplicationInfo(appInfo);
    launchData.SetProcessInfo(processInfo);
    MockBundleInstallerSetup(mockBundleMgr, mockSystemAbility_, iSystemAbilityMgr_);
    // SetNativeLibPath is implemented in anonymous space, called by HandleLaunchApplication
    mainThread_->HandleLaunchApplication(launchData, config);
    ASSERT_NE(mainThread_->application_, nullptr);
    EXPECT_NE(mainThread_->application_->abilityRuntimeContext_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: AppInnerHspPathList_0100
 * @tc.desc: Verify appInnerHspPathList is filled for SHARED static modules.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadByMockBmsTest, AppInnerHspPathList_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    Configuration config;
    AppLaunchData launchData;
    ProcessInfo processInfo("test_static_hsp", 9999);
    ApplicationInfo appInfo;
    appInfo.name = "MainAbility";
    appInfo.bundleName = "com.ohos.staticmodule";
    launchData.SetApplicationInfo(appInfo);
    launchData.SetProcessInfo(processInfo);

    BundleInfo bundleInfo;
    bundleInfo.name = appInfo.bundleName;
    bundleInfo.applicationInfo = appInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "test_moduleName";
    hapModuleInfo.hapPath = "/data/test_moduleName.hap";
    hapModuleInfo.moduleArkTSMode = "static";
    hapModuleInfo.moduleType = ModuleType::SHARED;
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);

    mockBundleMgrService_ = new (std::nothrow) OHOS::MockBundleManagerService();
    ASSERT_NE(mockBundleMgrService_, nullptr);
    MockBundleManagerForLaunchSetup(mockBundleMgrService, mockSystemAbility_, iSystemAbilityMgr_, bundleInfo);
    mainThread_->HandleLaunchApplication(launchData, config);
    ASSERT_NE(mainThread_->application_, nullptr);
    const auto &innerHsp = mainThread_->runtimeUpdateParam_.option.appInnerHspPathList;
    EXPECT_EQ(innerHsp.size(), 1);
    EXPECT_EQ(innerHsp[0], "/data/test_moduleName.hap");
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: StaticHapModule_0100
 * @tc.desc: Verify staticHapModuleNameList is filled when hapModuleInfos is available.
 * @tc.type: FUNC
 */
HWTEST_F(MainThreadByMockBmsTest, StaticHapModule_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    Configuration config;
    AppLaunchData launchData;
    ProcessInfo processInfo("test_static_module", 9999);
    ApplicationInfo appInfo;
    appInfo.name = "MainAbility";
    appInfo.bundleName = "com.ohos.staticmodule";
    launchData.SetApplicationInfo(appInfo);
    launchData.SetProcessInfo(processInfo);

    BundleInfo bundleInfo;
    bundleInfo.name = appInfo.bundleName;
    bundleInfo.applicationInfo = appInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "test_moduleName";
    hapModuleInfo.hapPath = "/data/test_moduleName.hap";
    hapModuleInfo.moduleArkTSMode = "static";
    hapModuleInfo.moduleType = ModuleType::ENTRY;
    bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);

    mockBundleMgrService_ = new (std::nothrow) OHOS::MockBundleManagerService();
    ASSERT_NE(mockBundleMgrService_, nullptr);
    MockBundleManagerForLaunchSetup(mockBundleMgrService, mockSystemAbility_, iSystemAbilityMgr_, bundleInfo);
    mainThread_->HandleLaunchApplication(launchData, config);
    ASSERT_NE(mainThread_->application_, nullptr);
    const auto &modules = mainThread_->runtimeUpdateParam_.option.staticHapModuleNameList;
    EXPECT_EQ(modules.size(), 1);
    EXPECT_EQ(modules[0], "test_moduleName");
    const auto &innerHsp = mainThread_->runtimeUpdateParam_.option.appInnerHspPathList;
    EXPECT_TRUE(innerHsp.empty());
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: GetOverlayModuleInfos_0100
 * @tc.desc: Get overlay paths form mock bms.
 * @tc.require: issueI6SAQC
 */
HWTEST_F(MainThreadByMockBmsTest, GetOverlayModuleInfos_0100, TestSize.Level1)
{
    EXPECT_TRUE(mainThread_ != nullptr);
    std::string bundleName = "com.ohos.demo";
    std::string moduleName = "entry";
    std::vector<OverlayModuleInfo> overlayModuleInfos;
    OverlayModuleInfo overlayModuleInfo;
    overlayModuleInfo.bundleName = "com.ohos.demo";
    overlayModuleInfo.moduleName = "entry";
    overlayModuleInfo.hapPath = "test";
    overlayModuleInfo.priority = 99;
    overlayModuleInfo.state = OverlayState::OVERLAY_ENABLE;
    overlayModuleInfos.emplace_back(overlayModuleInfo);
    overlayModuleInfos[0].state = OverlayState::OVERLAY_DISABLED;
    MockBundleInstaller();
    int result = mainThread_->GetOverlayModuleInfos(bundleName, moduleName, overlayModuleInfos);
    EXPECT_TRUE(result == 0);
    EXPECT_TRUE(overlayModuleInfos.size() == 1);
}

/**
 * @tc.name: DoUpdatePluginInfoInstalled_0100
 * @tc.desc: test DoUpdatePluginInfoInstalled with null applicationInfo_.
 * @tc.require:
 */
HWTEST_F(MainThreadByMockBmsTest, DoUpdatePluginInfoInstalled_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_TRUE(mainThread_ != nullptr);

    std::vector<AppExecFwk::PluginBundleInfo> pluginBundleInfos;
    std::vector<std::string> pluginModuleNames;

    // Set applicationInfo_ to null
    mainThread_->applicationInfo_ = nullptr;
    mainThread_->application_ = std::make_shared<OHOSApplication>();

    // Call DoUpdatePluginInfoInstalled (should return early)
    mainThread_->DoUpdatePluginInfoInstalled(pluginBundleInfos, pluginModuleNames);
    EXPECT_TRUE(mainThread_->applicationInfo_ == nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: DoUpdatePluginInfoInstalled_0200
 * @tc.desc: test DoUpdatePluginInfoInstalled with null application_.
 * @tc.require:
 */
HWTEST_F(MainThreadByMockBmsTest, DoUpdatePluginInfoInstalled_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    EXPECT_TRUE(mainThread_ != nullptr);

    std::vector<AppExecFwk::PluginBundleInfo> pluginBundleInfos;
    std::vector<std::string> pluginModuleNames;

    mainThread_->applicationInfo_ = std::make_shared<ApplicationInfo>();
    mainThread_->application_ = nullptr;

    mainThread_->DoUpdatePluginInfoInstalled(pluginBundleInfos, pluginModuleNames);
    EXPECT_TRUE(mainThread_->application_ == nullptr);

    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS