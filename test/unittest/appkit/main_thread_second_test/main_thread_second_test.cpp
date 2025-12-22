/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "main_thread.h"
#include "hilog_tag_wrapper.h"
#include "ohos_application.h"
#undef private

#include "runtime.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class MainThreadSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MainThreadSecondTest::SetUpTestCase()
{}

void MainThreadSecondTest::TearDownTestCase()
{}

void MainThreadSecondTest::SetUp()
{}

void MainThreadSecondTest::TearDown()
{}

/**
 * @tc.name: NeedUpdateRuntimeModuleChecker_ShouldReturnFalseWhenRecordIsNull
 * @tc.desc: NeedUpdateRuntimeModuleChecker.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MainThreadSecondTest, NeedUpdateRuntimeModuleChecker_ShouldReturnFalseWhenRecordIsNull, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<MainThread> mainThread = std::make_shared<MainThread>();
    EXPECT_FALSE(mainThread->NeedUpdateRuntimeModuleChecker(nullptr));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NeedUpdateRuntimeModuleChecker_ShouldReturnFalseWhenAbilityInfoIsNull
 * @tc.desc: NeedUpdateRuntimeModuleChecker.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MainThreadSecondTest, NeedUpdateRuntimeModuleChecker_ShouldReturnFalseWhenAbilityInfoIsNull, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<MainThread> mainThread = std::make_shared<MainThread>();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    auto record = std::make_shared<AbilityLocalRecord>(nullptr, nullptr, nullptr, 0);
    EXPECT_FALSE(mainThread->NeedUpdateRuntimeModuleChecker(record));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NeedUpdateRuntimeModuleChecker_ShouldReturnFalseWhenExtensionTypeIsService
 * @tc.desc: NeedUpdateRuntimeModuleChecker.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MainThreadSecondTest, NeedUpdateRuntimeModuleChecker_ShouldReturnFalseWhenExtensionTypeIsService,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<MainThread> mainThread = std::make_shared<MainThread>();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->extensionAbilityType = ExtensionAbilityType::SERVICE;
    auto record = std::make_shared<AbilityLocalRecord>(info, nullptr, nullptr, 0);
    EXPECT_FALSE(mainThread->NeedUpdateRuntimeModuleChecker(record));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NeedUpdateRuntimeModuleChecker_ShouldReturnFalseWhenExtensionTypeIsDataShare
 * @tc.desc: NeedUpdateRuntimeModuleChecker.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MainThreadSecondTest, NeedUpdateRuntimeModuleChecker_ShouldReturnFalseWhenExtensionTypeIsDataShare,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<MainThread> mainThread = std::make_shared<MainThread>();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->extensionAbilityType = ExtensionAbilityType::DATASHARE;
    auto record = std::make_shared<AbilityLocalRecord>(info, nullptr, nullptr, 0);
    EXPECT_FALSE(mainThread->NeedUpdateRuntimeModuleChecker(record));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NeedUpdateRuntimeModuleChecker_ShouldReturnFalseWhenAllowMultiProcess
 * @tc.desc: NeedUpdateRuntimeModuleChecker.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MainThreadSecondTest, NeedUpdateRuntimeModuleChecker_ShouldReturnFalseWhenAllowMultiProcess, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<MainThread> mainThread = std::make_shared<MainThread>();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->extensionAbilityType = ExtensionAbilityType::WORK_SCHEDULER;
    info->applicationInfo.allowMultiProcess = true;
    auto record = std::make_shared<AbilityLocalRecord>(info, nullptr, nullptr, 0);
    EXPECT_FALSE(mainThread->NeedUpdateRuntimeModuleChecker(record));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NeedUpdateRuntimeModuleChecker_ShouldReturnTrueWhenNotAllowMultiProcess
 * @tc.desc: NeedUpdateRuntimeModuleChecker.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MainThreadSecondTest, NeedUpdateRuntimeModuleChecker_ShouldReturnTrueWhenNotAllowMultiProcess,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<MainThread> mainThread = std::make_shared<MainThread>();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->extensionAbilityType = ExtensionAbilityType::WORK_SCHEDULER;
    info->applicationInfo.allowMultiProcess = false;
    auto record = std::make_shared<AbilityLocalRecord>(info, nullptr, nullptr, 0);
    EXPECT_TRUE(mainThread->NeedUpdateRuntimeModuleChecker(record));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NeedUpdateRuntimeModuleChecker_ShouldNotUpdateCheckerWhenNotNotNeedUpdate
 * @tc.desc: UpdateRuntimeModuleChecker.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MainThreadSecondTest, UpdateRuntimeModuleChecker_ShouldNotUpdateCheckerWhenNotNotNeedUpdate, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<MainThread> mainThread = std::make_shared<MainThread>();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->extensionAbilityType = ExtensionAbilityType::WORK_SCHEDULER;
    info->applicationInfo.allowMultiProcess = true;
    auto record = std::make_shared<AbilityLocalRecord>(info, nullptr, nullptr, 0);
    mainThread->UpdateRuntimeModuleChecker(nullptr, record);
    EXPECT_EQ(mainThread->extensionConfigMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: NeedUpdateRuntimeModuleChecker_ShouldReturnNullptrWhenExtensionConfigMgrIsNullptr
 * @tc.desc: UpdateRuntimeModuleChecker.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MainThreadSecondTest, UpdateRuntimeModuleChecker_ShouldReturnNullptrWhenExtensionConfigMgrIsNullptr,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<MainThread> mainThread = std::make_shared<MainThread>();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->extensionAbilityType = ExtensionAbilityType::WORK_SCHEDULER;
    info->applicationInfo.allowMultiProcess = false;
    auto record = std::make_shared<AbilityLocalRecord>(info, nullptr, nullptr, 0);
    mainThread->UpdateRuntimeModuleChecker(nullptr, record);
    EXPECT_EQ(mainThread->extensionConfigMgr_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UpdateRuntimeModuleChecker_ShouldReturnTypeWhenExtensionConfigMgrIsValid
 * @tc.desc: UpdateRuntimeModuleChecker.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MainThreadSecondTest, UpdateRuntimeModuleChecker_ShouldReturnTypeWhenExtensionConfigMgrIsValid,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<MainThread> mainThread = std::make_shared<MainThread>();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->extensionAbilityType = ExtensionAbilityType::WORK_SCHEDULER;
    info->applicationInfo.allowMultiProcess = false;
    auto record = std::make_shared<AbilityLocalRecord>(info, nullptr, nullptr, 0);
    mainThread->extensionConfigMgr_ = std::make_shared<AbilityRuntime::ExtensionConfigMgr>();
    mainThread->UpdateRuntimeModuleChecker(nullptr, record);
    EXPECT_EQ(mainThread->extensionConfigMgr_->extensionType_, static_cast<int32_t>(ExtensionAbilityType::WORK_SCHEDULER));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UpdateRuntimeModuleChecker_ShouldReturnTypeWhenRuntimeIsNull
 * @tc.desc: UpdateRuntimeModuleChecker.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MainThreadSecondTest, UpdateRuntimeModuleChecker_ShouldReturnTypeWhenRuntimeIsNull,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<MainThread> mainThread = std::make_shared<MainThread>();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->extensionAbilityType = ExtensionAbilityType::WORK_SCHEDULER;
    info->applicationInfo.allowMultiProcess = false;
    auto record = std::make_shared<AbilityLocalRecord>(info, nullptr, nullptr, 0);
    mainThread->extensionConfigMgr_ = std::make_shared<AbilityRuntime::ExtensionConfigMgr>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    mainThread->UpdateRuntimeModuleChecker(application, record);
    EXPECT_EQ(mainThread->extensionConfigMgr_->extensionType_, static_cast<int32_t>(ExtensionAbilityType::WORK_SCHEDULER));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}

/**
 * @tc.name: UpdateRuntimeModuleChecker_ShouldReturnTypeWhenRuntimeIsNotNull
 * @tc.desc: UpdateRuntimeModuleChecker.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MainThreadSecondTest, UpdateRuntimeModuleChecker_ShouldReturnTypeWhenRuntimeIsNotNull,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "%{public}s start.", __func__);
    std::shared_ptr<MainThread> mainThread = std::make_shared<MainThread>();
    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    info->extensionAbilityType = ExtensionAbilityType::WORK_SCHEDULER;
    info->applicationInfo.allowMultiProcess = false;
    auto record = std::make_shared<AbilityLocalRecord>(info, nullptr, nullptr, 0);
    mainThread->extensionConfigMgr_ = std::make_shared<AbilityRuntime::ExtensionConfigMgr>();
    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    AbilityRuntime::Runtime::Options options;
    auto runtime = AbilityRuntime::Runtime::Create(options);
    application->SetRuntime(std::move(runtime));
    mainThread->UpdateRuntimeModuleChecker(application, record);
    EXPECT_EQ(mainThread->extensionConfigMgr_->extensionType_, static_cast<int32_t>(ExtensionAbilityType::WORK_SCHEDULER));
    TAG_LOGI(AAFwkTag::TEST, "%{public}s end.", __func__);
}
} // namespace AppExecFwk
} // namespace OHOS
