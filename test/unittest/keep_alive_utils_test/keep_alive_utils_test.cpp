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

#include "keep_alive_utils.h"
#include <gtest/gtest.h>

#define private public
#define protected public
#include "ability_manager_service.h"
#undef private
#undef protected

#include "ability_info.h"
#include "bundle_info.h"
#include "hap_module_info.h"
#include "mock_parameters.h"

#include <vector>
#include <memory>

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AAFwk {

namespace {
    const std::string PROCESS_NAME = "com.ohos.example.process";
    const std::string MAIN_ABILITY = "com.ohos.example.mainability";
    const std::string URI = "https://www.example.com:8080/path/to/resource?key1=value1&key2=value2#section1";
    const std::string BUNDLE_NAME = "com.ohos.example.bundlename";
    const std::string NAME = "com.ohos.example.name";
}

class KeepAliveUtilsTest : public testing::Test {
public:
    KeepAliveUtilsTest();
    ~KeepAliveUtilsTest();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

KeepAliveUtilsTest::KeepAliveUtilsTest()
{
}

KeepAliveUtilsTest::~KeepAliveUtilsTest()
{
}

void KeepAliveUtilsTest::SetUpTestCase()
{
}

void KeepAliveUtilsTest::TearDownTestCase()
{
}

void KeepAliveUtilsTest::SetUp()
{
}

void KeepAliveUtilsTest::TearDown()
{
}

/**
 * @tc.name:  NotifyDisableKeepAliveProcesses_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(KeepAliveUtilsTest, NotifyDisableKeepAliveProcesses_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDisableKeepAliveProcesses_0100 start";

    AppExecFwk::BundleInfo bundleInfo;
    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    bundleInfos.push_back(bundleInfo);
    int32_t userId = 0;

    std::shared_ptr<AAFwk::KeepAliveUtils> keepAliveUtils =
        std::make_shared<AAFwk::KeepAliveUtils>();
    keepAliveUtils->NotifyDisableKeepAliveProcesses(bundleInfos, userId);
    EXPECT_NE(keepAliveUtils, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    GTEST_LOG_(INFO) << "NotifyDisableKeepAliveProcesses_0100 end";
}

/**
 * @tc.name:  NotifyDisableKeepAliveProcesses_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(KeepAliveUtilsTest, NotifyDisableKeepAliveProcesses_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDisableKeepAliveProcesses_0200 start";

    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.applicationInfo.process = PROCESS_NAME;

    AppExecFwk::HapModuleInfo hapModuleInfo;
    hapModuleInfo.mainAbility = MAIN_ABILITY;
    hapModuleInfo.bundleName = BUNDLE_NAME;
    hapModuleInfo.name = NAME;

    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.process = PROCESS_NAME;
    abilityInfo.name = MAIN_ABILITY;
    abilityInfo.type = AppExecFwk::AbilityType::DATA;
    abilityInfo.uri = URI;
    hapModuleInfo.abilityInfos.push_back(abilityInfo);
    bundleInfo.hapModuleInfos.push_back(hapModuleInfo);

    std::vector<AppExecFwk::BundleInfo> bundleInfos;
    bundleInfos.push_back(bundleInfo);
    int32_t userId = 0;

    std::shared_ptr<AAFwk::KeepAliveUtils> keepAliveUtils =
        std::make_shared<AAFwk::KeepAliveUtils>();
    keepAliveUtils->NotifyDisableKeepAliveProcesses(bundleInfos, userId);
    EXPECT_NE(keepAliveUtils, nullptr);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    GTEST_LOG_(INFO) << "NotifyDisableKeepAliveProcesses_0200 end";
}

/**
 * @tc.name:  IsKeepAliveBundle_0100
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(KeepAliveUtilsTest, IsKeepAliveBundle_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsKeepAliveBundle_0100 start";

    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.name = BUNDLE_NAME;
    int32_t userId = 0;
    AAFwk::KeepAliveType type = AAFwk::KeepAliveType::UNSPECIFIED;

    std::shared_ptr<AAFwk::KeepAliveUtils> keepAliveUtils =
        std::make_shared<AAFwk::KeepAliveUtils>();
    auto test = keepAliveUtils->IsKeepAliveBundle(bundleInfo, userId, type);
    EXPECT_EQ(test, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    GTEST_LOG_(INFO) << "IsKeepAliveBundle_0100 end";
}

/**
 * @tc.name:  IsKeepAliveBundle_0200
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(KeepAliveUtilsTest, IsKeepAliveBundle_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsKeepAliveBundle_0200 start";

    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.isKeepAlive = true;
    int32_t userId = 0;
    AAFwk::KeepAliveType type = AAFwk::KeepAliveType::UNSPECIFIED;
    std::shared_ptr<AAFwk::KeepAliveUtils> keepAliveUtils =
        std::make_shared<AAFwk::KeepAliveUtils>();
    auto test = keepAliveUtils->IsKeepAliveBundle(bundleInfo, userId, type);
    EXPECT_EQ(test, true);
    EXPECT_EQ(type, AAFwk::KeepAliveType::RESIDENT_PROCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    GTEST_LOG_(INFO) << "IsKeepAliveBundle_0200 end";
}

/**
 * @tc.name:  IsKeepAliveBundle_0300
 * @tc.desc: basic function test.
 * @tc.type: FUNC
 */
HWTEST_F(KeepAliveUtilsTest, IsKeepAliveBundle_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsKeepAliveBundle_0300 start";

    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.isKeepAlive = false;
    int32_t userId = 0;
    AAFwk::KeepAliveType type = AAFwk::KeepAliveType::UNSPECIFIED;
    std::shared_ptr<AAFwk::KeepAliveUtils> keepAliveUtils =
        std::make_shared<AAFwk::KeepAliveUtils>();
    auto test = keepAliveUtils->IsKeepAliveBundle(bundleInfo, userId, type);
    EXPECT_EQ(test, false);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    GTEST_LOG_(INFO) << "IsKeepAliveBundle_0300 end";
}

} // namespace AbilityRuntime
} // namespace OHOS