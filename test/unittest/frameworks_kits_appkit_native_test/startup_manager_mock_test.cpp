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
#include <fstream>
#include <gtest/gtest.h>
#define private public
#define protected public
#include "startup_manager.h"
#include "js_startup_task.h"
#include "js_insight_intent_executor.h"
#include "mock_config_policy_utils.h"
#undef private
#undef protected

using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
class StartupManagerMockTest : public testing::Test {
public:
    StartupManagerMockTest() {}
    ~StartupManagerMockTest() {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StartupManagerMockTest::SetUpTestCase(void) {}
void StartupManagerMockTest::TearDownTestCase(void) {}
void StartupManagerMockTest::SetUp(void) {}
void StartupManagerMockTest::TearDown(void) {}

/**
 * @tc.name: GetStartupConfigString_0200
 * @tc.type: FUNC
 * @tc.Function: GetStartupConfigString
 */
HWTEST_F(StartupManagerMockTest, GetStartupConfigString_0100, Function | MediumTest | Level1)
{
    std::string name = "test_name";
    std::string config = "test_config";
    std::string startupConfig = "$profile:test";
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    EXPECT_TRUE(startupManager != nullptr);
    ModuleStartupConfigInfo info(name, startupConfig, "", AppExecFwk::ModuleType::UNKNOWN, false);
    int32_t ret = startupManager->GetStartupConfigString(info, config);
    EXPECT_EQ(ret, ERR_STARTUP_CONFIG_PARSE_ERROR);
}

/**
 * @tc.name: PreloadSoStartupTask_0100
 * @tc.type: FUNC
 * @tc.Function: RunTaskInit
 */
HWTEST_F(StartupManagerMockTest, PreloadSoStartupTask_0100, Function | MediumTest | Level1)
{
    std::string name = "test_name";
    std::string ohmUrl = "@normalized:Y&&<bundleName>&<IMPORT_PATH>&<VERSION>";
    std::shared_ptr<PreloadSoStartupTask> startupTask = std::make_shared<PreloadSoStartupTask>(name, ohmUrl);
    auto ret = startupTask->RunTaskInit(nullptr);
    EXPECT_EQ(ret, ERR_STARTUP_INTERNAL_ERROR);
}

/**
 * @tc.name: JsStartupTask_0100
 * @tc.type: FUNC
 * @tc.Function: RunTaskInit
 */
HWTEST_F(StartupManagerMockTest, JsStartupTask_0100, Function | MediumTest | Level1)
{
    std::string name = "test_name";
    JsRuntime jsRuntime;
    std::unique_ptr<NativeReference> startupJsRef = nullptr;
    std::shared_ptr<NativeReference> contextJsRef = nullptr;
    std::shared_ptr<JsStartupTask> startupTask = std::make_shared<JsStartupTask>(
        name, jsRuntime, startupJsRef, contextJsRef);
    startupTask->callCreateOnMainThread_ = false;
    auto ret = startupTask->RunTaskInit(nullptr);
    EXPECT_EQ(ret, ERR_STARTUP_INTERNAL_ERROR);
}

/**
 * @tc.name: js_insight_intent_executor_0100
 * @tc.type: FUNC
 * @tc.Function: RunTaskInit
 */
HWTEST_F(StartupManagerMockTest, js_insight_intent_executor_0100, Function | MediumTest | Level1)
{
    InsightIntentExecutorAsyncCallback* callback = InsightIntentExecutorAsyncCallback::Create();
    JsInsightIntentExecutor::ReplyFailed(callback);
    EXPECT_TRUE(callback != nullptr);
    callback = nullptr;
}

/**
 * @tc.name: InitPreloadSystemSoAllowlist_0100
 * @tc.type: FUNC
 * @tc.Function: InitPreloadSystemSoAllowlist
 */
HWTEST_F(StartupManagerMockTest, InitPreloadSystemSoAllowlist_0100, Function | MediumTest | Level1)
{
    std::unordered_set<std::string> preloadSystemSoAllowlist;
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    startupManager->preloadSystemSoAllowlist_.clear();

    OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().SetOneCfgFilePathIsNull();
    nlohmann::json parseResult;
    if (!startupManager->ReadPreloadSystemSoAllowlistFile(parseResult)) {
        preloadSystemSoAllowlist.clear();
        GTEST_LOG_(INFO) << "LoadConfiguration failed, using default preloadSystemSoAllowlist";
    }

    if (!startupManager->ParsePreloadSystemSoAllowlist(parseResult, preloadSystemSoAllowlist)) {
        preloadSystemSoAllowlist.clear();
        GTEST_LOG_(INFO) << "ParsePreloadSystemSoAllowlist failed, using default preloadSystemSoAllowlist";
    }

    startupManager->InitPreloadSystemSoAllowlist();
    EXPECT_EQ(preloadSystemSoAllowlist.size(), startupManager->preloadSystemSoAllowlist_.size());
    startupManager->preloadSystemSoAllowlist_.clear();
}

/**
 * @tc.name: InitPreloadSystemSoAllowlist_0200
 * @tc.type: FUNC
 * @tc.Function: InitPreloadSystemSoAllowlist
 */
HWTEST_F(StartupManagerMockTest, InitPreloadSystemSoAllowlist_0200, Function | MediumTest | Level1)
{
    std::string preloadSystemSoAllowlistFileName = "";
    std::unordered_set<std::string> preloadSystemSoAllowlist;
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    startupManager->preloadSystemSoAllowlist_.clear();

    OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().SetOneCfgFilePath(preloadSystemSoAllowlistFileName);
    nlohmann::json parseResult;
    if (!startupManager->ReadPreloadSystemSoAllowlistFile(parseResult)) {
        preloadSystemSoAllowlist.clear();
        GTEST_LOG_(INFO) << "LoadConfiguration failed, using default preloadSystemSoAllowlist";
    }

    if (!startupManager->ParsePreloadSystemSoAllowlist(parseResult, preloadSystemSoAllowlist)) {
        preloadSystemSoAllowlist.clear();
        GTEST_LOG_(INFO) << "ParsePreloadSystemSoAllowlist failed, using default preloadSystemSoAllowlist";
    }

    startupManager->InitPreloadSystemSoAllowlist();
    EXPECT_EQ(preloadSystemSoAllowlist.size(), startupManager->preloadSystemSoAllowlist_.size());
    startupManager->preloadSystemSoAllowlist_.clear();
}

/**
 * @tc.name: InitPreloadSystemSoAllowlist_0300
 * @tc.type: FUNC
 * @tc.Function: InitPreloadSystemSoAllowlist
 */
HWTEST_F(StartupManagerMockTest, InitPreloadSystemSoAllowlist_0300, Function | MediumTest | Level1)
{
    std::string fileName = "ability_runtime_test_";
    std::unordered_set<std::string> preloadSystemSoAllowlist;
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    startupManager->preloadSystemSoAllowlist_.clear();

    fileName.append(fileName).append(fileName).append(fileName).append(fileName);
    OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().SetOneCfgFilePath(fileName);
    nlohmann::json parseResult;
    if (!startupManager->ReadPreloadSystemSoAllowlistFile(parseResult)) {
        preloadSystemSoAllowlist.clear();
        GTEST_LOG_(INFO) << "LoadConfiguration failed, using default preloadSystemSoAllowlist";
    }

    if (!startupManager->ParsePreloadSystemSoAllowlist(parseResult, preloadSystemSoAllowlist)) {
        preloadSystemSoAllowlist.clear();
        GTEST_LOG_(INFO) << "ParsePreloadSystemSoAllowlist failed, using default preloadSystemSoAllowlist";
    }

    startupManager->InitPreloadSystemSoAllowlist();
    EXPECT_EQ(preloadSystemSoAllowlist.size(), startupManager->preloadSystemSoAllowlist_.size());
    startupManager->preloadSystemSoAllowlist_.clear();
}

/**
 * @tc.name: InitPreloadSystemSoAllowlist_0400
 * @tc.type: FUNC
 * @tc.Function: InitPreloadSystemSoAllowlist
 */
HWTEST_F(StartupManagerMockTest, InitPreloadSystemSoAllowlist_0400, Function | MediumTest | Level1)
{
    std::string preloadSystemSoAllowlistFileName = "ability_runtime_app_startup.json";
    std::unordered_set<std::string> preloadSystemSoAllowlist;
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    startupManager->preloadSystemSoAllowlist_.clear();

    OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().SetOneCfgFilePath(preloadSystemSoAllowlistFileName);
    OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().SetRealPathIsNull(true);
    nlohmann::json parseResult;
    if (!startupManager->ReadPreloadSystemSoAllowlistFile(parseResult)) {
        preloadSystemSoAllowlist.clear();
        GTEST_LOG_(INFO) << "LoadConfiguration failed, using default preloadSystemSoAllowlist";
    }

    if (!startupManager->ParsePreloadSystemSoAllowlist(parseResult, preloadSystemSoAllowlist)) {
        preloadSystemSoAllowlist.clear();
        GTEST_LOG_(INFO) << "ParsePreloadSystemSoAllowlist failed, using default preloadSystemSoAllowlist";
    }

    startupManager->InitPreloadSystemSoAllowlist();
    EXPECT_EQ(preloadSystemSoAllowlist.size(), startupManager->preloadSystemSoAllowlist_.size());
    startupManager->preloadSystemSoAllowlist_.clear();
}

/**
 * @tc.name: InitPreloadSystemSoAllowlist_0500
 * @tc.type: FUNC
 * @tc.Function: InitPreloadSystemSoAllowlist
 */
HWTEST_F(StartupManagerMockTest, InitPreloadSystemSoAllowlist_0500, Function | MediumTest | Level1)
{
    std::string preloadSystemSoAllowlistFileName = "ability_runtime_app_startup.json";
    std::unordered_set<std::string> preloadSystemSoAllowlist;
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    startupManager->preloadSystemSoAllowlist_.clear();

    // root path test
    OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().SetOneCfgFilePath(
        preloadSystemSoAllowlistFileName, false, true);
    OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().SetRealPathIsNull(false);
    nlohmann::json parseResult;
    if (!startupManager->ReadPreloadSystemSoAllowlistFile(parseResult)) {
        preloadSystemSoAllowlist.clear();
        GTEST_LOG_(INFO) << "LoadConfiguration failed, using default preloadSystemSoAllowlist";
    }

    if (!startupManager->ParsePreloadSystemSoAllowlist(parseResult, preloadSystemSoAllowlist)) {
        preloadSystemSoAllowlist.clear();
        GTEST_LOG_(INFO) << "ParsePreloadSystemSoAllowlist failed, using default preloadSystemSoAllowlist";
    }

    startupManager->InitPreloadSystemSoAllowlist();
    EXPECT_EQ(preloadSystemSoAllowlist.size(), startupManager->preloadSystemSoAllowlist_.size());
    startupManager->preloadSystemSoAllowlist_.clear();
}

/**
 * @tc.name: InitPreloadSystemSoAllowlist_0600
 * @tc.type: FUNC
 * @tc.Function: InitPreloadSystemSoAllowlist
 */
HWTEST_F(StartupManagerMockTest, InitPreloadSystemSoAllowlist_0600, Function | MediumTest | Level1)
{
    std::string preloadSystemSoAllowlistFileName = "ability_runtime_app_startup.json";
    std::unordered_set<std::string> allowlist;
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    startupManager->preloadSystemSoAllowlist_.clear();
    std::string jsonData = R"({"systemPreloadSoAllowList":["@ohos:testUrl"]})";

    // work path test
    OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().SetOneCfgFilePath(
        preloadSystemSoAllowlistFileName, true);
    OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().SetRealPathIsNull(false);

    // Write json file
    auto jsonPath = OHOS::AbilityRuntime::MockConfigPolicyUtils::GetInstance().GetOneCfgFilePath();
    GTEST_LOG_(INFO) << "InitPreloadSystemSoAllowlist_0600 jsonPath is " << jsonPath;
    std::ofstream outFile(jsonPath, std::ios::out | std::ios::trunc);
    if (outFile.is_open()) {
        outFile << jsonData;
        EXPECT_TRUE(outFile.good());
        outFile.close();
        GTEST_LOG_(INFO) << "InitPreloadSystemSoAllowlist_0600 outFile.is_open() success";
    } else {
        GTEST_LOG_(INFO) << "InitPreloadSystemSoAllowlist_0600 outFile.is_open() false";
    }

    nlohmann::json parseResult;
    if (!startupManager->ReadPreloadSystemSoAllowlistFile(parseResult)) {
        allowlist.clear();
        GTEST_LOG_(INFO) << "LoadConfiguration failed, using default preloadSystemSoAllowlist";
    }

    if (!startupManager->ParsePreloadSystemSoAllowlist(parseResult, allowlist)) {
        allowlist.clear();
        GTEST_LOG_(INFO) << "ParsePreloadSystemSoAllowlist failed, using default preloadSystemSoAllowlist";
    }

    startupManager->InitPreloadSystemSoAllowlist();
    EXPECT_EQ(allowlist.size(), startupManager->preloadSystemSoAllowlist_.size());
    EXPECT_TRUE(std::all_of(
        startupManager->preloadSystemSoAllowlist_.begin(),
        startupManager->preloadSystemSoAllowlist_.end(),
        [&allowlist](const std::string &elem) {
            return allowlist.find(elem) != allowlist.end();
        }));
    startupManager->preloadSystemSoAllowlist_.clear();
}
}
}