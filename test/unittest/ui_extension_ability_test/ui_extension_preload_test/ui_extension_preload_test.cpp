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

#include <gtest/gtest.h>

#include "ability_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "session_info.h"
#include "want.h"
#include "ipc_skeleton.h"

#include <thread> 
#define private public
#include "preload_ui_extension_host_client.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UIExtensionPreloadTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UIExtensionPreloadTest::SetUpTestCase(void)
{}

void UIExtensionPreloadTest::TearDownTestCase(void)
{}

void UIExtensionPreloadTest::SetUp()
{}

void UIExtensionPreloadTest::TearDown()
{}

/**
 * @tc.name: PermissionCheck_0100
 * @tc.desc: permission check test.
 * @tc.type: FUNC
 * @tc.require: I9NW1A
 */
HWTEST_F(UIExtensionPreloadTest, PermissionCheck_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    Want providerWant;
    AppExecFwk::ElementName providerElement("0", "com.ohos.uiextensionprovider", "UIExtensionProvider", "entry");
    providerWant.SetElement(providerElement);
    std::string hostBundleName = "com.ohos.uiextensionuser";
    auto ret = AbilityManagerClient::GetInstance()->PreloadUIExtensionAbility(providerWant, hostBundleName,
        DEFAULT_INVAL_VALUE);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbility_0100
 * @tc.desc: Test ClearPreloadedUIExtensionAbility with valid and invalid conditions.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, ClearPreloadedUIExtensionAbility_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    auto client = AbilityManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);

    int32_t extensionAbilityId = 123;
    int32_t userId = 0;

    ErrCode ret = client->ClearPreloadedUIExtensionAbility(extensionAbilityId, userId);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/**
 * @tc.name: ClearPreloadedUIExtensionAbilities_0100
 * @tc.desc: Test ClearPreloadedUIExtensionAbilities with valid hostBundleName.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, ClearPreloadedUIExtensionAbilities_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    auto client = AbilityManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);

    std::string hostBundleName = "com.ohos.testbundle";
    int32_t userId = 0;

    ErrCode ret = client->ClearPreloadedUIExtensionAbilities(hostBundleName, userId);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/**
 * @tc.name: RegisterPreloadUIExtensionHostClient_0100
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient with valid and null callerToken.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, RegisterPreloadUIExtensionHostClient_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    auto client = AbilityManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);

    sptr<IRemoteObject> callerToken = nullptr;
    ErrCode ret = client->RegisterPreloadUIExtensionHostClient(callerToken);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/**
 * @tc.name: UnRegisterPreloadUIExtensionHostClient_0100
 * @tc.desc: Test UnRegisterPreloadUIExtensionHostClient normal call.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, UnRegisterPreloadUIExtensionHostClient_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "start.");
    auto client = AbilityManagerClient::GetInstance();
    ASSERT_NE(client, nullptr);

    ErrCode ret = client->UnRegisterPreloadUIExtensionHostClient();
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "finish.");
}

/**
 * @tc.name: PreloadUIExtensionHostClient_0100
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient normal call.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_0100, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);
    instance->RegisterPreloadUIExtensionHostClient();
    EXPECT_EQ(instance->registeredPids_.size(), 0);
    std::string bundleName = "com.ohos.testbundle";
    int32_t key = instance->AddLoadedCallback(nullptr, bundleName);
    EXPECT_EQ(instance->loadedCallbackMap_.size(), 1);

    std::string bundleName2 = "com.ohos.testbundle2";
    int32_t key2 = instance->AddLoadedCallback(nullptr, bundleName2);
    EXPECT_EQ(instance->loadedCallbackMap_.size(), 2);

    instance->registeredPids_.emplace_back(1);
    EXPECT_EQ(instance->registeredPids_.size(), 1);
    instance->RegisterPreloadUIExtensionHostClient();
    EXPECT_EQ(instance->registeredPids_.size(), 1);

    instance->RemoveLoadedCallback(key, bundleName);
    instance->RemoveLoadedCallback(key2, bundleName2);
    EXPECT_EQ(instance->loadedCallbackMap_.size(), 0);
    instance->registeredPids_.clear();
}

/**
 * @tc.name: PreloadUIExtensionHostClient_0200
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient normal call.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_0200, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);
    instance->RegisterPreloadUIExtensionHostClient();
    EXPECT_EQ(instance->registeredPids_.size(), 0);
    std::string bundleName = "com.ohos.testbundle";
    int32_t key = instance->AddDestroyCallback(nullptr, bundleName);
    EXPECT_EQ(instance->destroyCallbackMap_.size(), 1);

    std::string bundleName2 = "com.ohos.testbundle2";
    int32_t key2 = instance->AddDestroyCallback(nullptr, bundleName2);
    EXPECT_EQ(instance->destroyCallbackMap_.size(), 2);

    instance->registeredPids_.emplace_back(1);
    EXPECT_EQ(instance->registeredPids_.size(), 1);
    instance->RegisterPreloadUIExtensionHostClient();
    EXPECT_EQ(instance->registeredPids_.size(), 1);

    instance->RemoveDestroyCallback(key, bundleName);
    instance->RemoveDestroyCallback(key2, bundleName2);
    EXPECT_EQ(instance->destroyCallbackMap_.size(), 0);
    instance->registeredPids_.clear();
}

/**
 * @tc.name: PreloadUIExtensionHostClient_0300
 * @tc.desc: Test RegisterPreloadUIExtensionHostClient normal call.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_0300, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);
    instance->RegisterPreloadUIExtensionHostClient();
    EXPECT_EQ(instance->registeredPids_.size(), 0);
    std::string bundleName = "com.ohos.testbundle";
    instance->resultCallbacks_.emplace(1, nullptr);
    EXPECT_EQ(instance->resultCallbacks_.size(), 1);
    instance->RegisterPreloadUIExtensionHostClient();

    instance->resultCallbacks_.clear();
    EXPECT_EQ(instance->resultCallbacks_.size(), 0);
}

/**
 * @tc.name: PreloadUIExtensionHostClient_0400
 * @tc.desc: Test RemoveLoadedCallback normal call.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_0400, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);
    std::string bundleName = "com.ohos.testbundle";
    int32_t ret = instance->RemoveLoadedCallback(-1, bundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    int32_t key = instance->AddLoadedCallback(nullptr, bundleName);
    int32_t key2 = instance->AddLoadedCallback(nullptr, bundleName);
    ret = instance->RemoveLoadedCallback(-1, bundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    ret = instance->RemoveLoadedCallback(key, bundleName);
    EXPECT_EQ(instance->loadedCallbackMap_.size(), 1);
    EXPECT_EQ(ret, ERR_OK);

    instance->loadedCallbackMap_.clear();
}

/**
 * @tc.name: PreloadUIExtensionHostClient_0500
 * @tc.desc: Test RemoveLoadedCallback normal call.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_0500, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);
    std::string bundleName = "com.ohos.testbundle";
    int32_t key = instance->AddLoadedCallback(nullptr, bundleName);

    auto ret = instance->RemoveLoadedCallback(key, bundleName);
    EXPECT_EQ(instance->loadedCallbackMap_.size(), 0);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PreloadUIExtensionHostClient_0600
 * @tc.desc: Test RemoveDestroyCallback normal call.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_0600, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);
    std::string bundleName = "com.ohos.testbundle";
    int32_t ret = instance->RemoveDestroyCallback(-1, bundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    int32_t key = instance->AddDestroyCallback(nullptr, bundleName);
    int32_t key2 = instance->AddDestroyCallback(nullptr, bundleName);
    ret = instance->RemoveDestroyCallback(-1, bundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    ret = instance->RemoveDestroyCallback(key, bundleName);
    EXPECT_EQ(instance->destroyCallbackMap_.size(), 1);
    EXPECT_EQ(ret, ERR_OK);

    instance->destroyCallbackMap_.clear();
}

/**
 * @tc.name: PreloadUIExtensionHostClient_0700
 * @tc.desc: Test RemoveDestroyCallback normal call.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_0700, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);
    std::string bundleName = "com.ohos.testbundle";
    int32_t key = instance->AddDestroyCallback(nullptr, bundleName);

    auto ret = instance->RemoveDestroyCallback(key, bundleName);
    EXPECT_EQ(instance->destroyCallbackMap_.size(), 0);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: PreloadUIExtensionHostClient_0800
 * @tc.desc: Test RemoveAllLoadedCallback normal call.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_0800, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);
    std::string bundleName = "com.ohos.testbundle";
    instance->RemoveAllLoadedCallback(bundleName);
    EXPECT_EQ(instance->loadedCallbackMap_.size(), 0);

    int32_t key = instance->AddLoadedCallback(nullptr, bundleName);
    EXPECT_EQ(instance->loadedCallbackMap_.size(), 1);
    instance->RemoveAllLoadedCallback(bundleName);
    EXPECT_EQ(instance->loadedCallbackMap_.size(), 0);

    key = instance->AddLoadedCallback(nullptr, bundleName);
    std::string bundleName2 = "com.ohos.testbundle2";
    int32_t key2 = instance->AddLoadedCallback(nullptr, bundleName2);
    EXPECT_EQ(instance->loadedCallbackMap_.size(), 2);
    instance->RemoveAllLoadedCallback(bundleName);
    EXPECT_EQ(instance->loadedCallbackMap_.size(), 1);

    instance->loadedCallbackMap_.clear();
}

/**
 * @tc.name: PreloadUIExtensionHostClient_0900
 * @tc.desc: Test RemoveAllDestroyCallback normal call.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_0900, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);
    std::string bundleName = "com.ohos.testbundle";
    instance->RemoveAllDestroyCallback(bundleName);
    EXPECT_EQ(instance->loadedCallbackMap_.size(), 0);

    int32_t key = instance->AddDestroyCallback(nullptr, bundleName);
    EXPECT_EQ(instance->destroyCallbackMap_.size(), 1);
    instance->RemoveAllDestroyCallback(bundleName);
    EXPECT_EQ(instance->destroyCallbackMap_.size(), 0);

    key = instance->AddDestroyCallback(nullptr, bundleName);
    std::string bundleName2 = "com.ohos.testbundle2";
    int32_t key2 = instance->AddDestroyCallback(nullptr, bundleName2);
    EXPECT_EQ(instance->destroyCallbackMap_.size(), 2);
    instance->RemoveAllDestroyCallback(bundleName);
    EXPECT_EQ(instance->destroyCallbackMap_.size(), 1);

    instance->destroyCallbackMap_.clear();
}

HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionAbility_0100, TestSize.Level1)
{
    auto client = PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(client, nullptr);

    Want want;
    want.SetElementName("com.example.test", "TestAbility");
    std::string hostBundleName = "com.example.host";

    std::atomic<bool> taskExecuted = false;
    PreloadTask task = [&taskExecuted](int32_t pid, int32_t id) {
        (void)pid; (void)id;
        taskExecuted = true;
    };

    client->PreloadUIExtensionAbility(want, hostBundleName, std::move(task));
    int waitCount = 0;
    while (!taskExecuted.load() && waitCount++ < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(taskExecuted.load());
}

HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionAbility_0200, TestSize.Level1)
{
    auto client = PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(client, nullptr);

    Want want;
    want.SetElementName("com.example.test", "TestAbility");
    std::string hostBundleName = "com.example.host";

    std::atomic<int> completedCount = 0;

    PreloadTask task1 = [&completedCount](int32_t, int32_t) { completedCount++; };
    client->PreloadUIExtensionAbility(want, hostBundleName, std::move(task1));
    PreloadTask task2 = [&completedCount](int32_t, int32_t) { completedCount++; };
    client->PreloadUIExtensionAbility(want, hostBundleName, std::move(task2));
    PreloadTask task3 = [&completedCount](int32_t, int32_t) { completedCount++; };
    client->PreloadUIExtensionAbility(want, hostBundleName, std::move(task3));

    int waitCount = 0;
    while (completedCount.load() < 3 && waitCount++ < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(completedCount.load(), 3);
}

HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionAbility_0300, TestSize.Level1)
{
    auto client = PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(client, nullptr);
    Want invalidWant;
    std::string hostBundleName = "";

    std::atomic<bool> taskExecuted = false;
    PreloadTask task = [&taskExecuted](int32_t pid, int32_t id) {
        (void)pid; (void)id;
        taskExecuted = true;
    };
    client->PreloadUIExtensionAbility(invalidWant, hostBundleName, std::move(task));

    int waitCount = 0;
    while (!taskExecuted.load() && waitCount++ < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(taskExecuted.load());
}

HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionAbility_0400, TestSize.Level1)
{
    auto client = PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(client, nullptr);

    Want want;
    want.SetElementName("com.example.test", "TestAbility");
    std::string hostBundleName = "com.example.host";

    std::atomic<int> executedCount = 0;
    const int totalTasks = 5;

    for (int i = 0; i < totalTasks; i++) {
        PreloadTask task = [&executedCount](int32_t, int32_t) {
            executedCount++;
        };
        client->PreloadUIExtensionAbility(want, hostBundleName, std::move(task));
    }

    int waitCount = 0;
    while (executedCount.load() < totalTasks && waitCount++ < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(executedCount.load(), totalTasks);
}

HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionAbility_0500, TestSize.Level1)
{
    auto client = PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(client, nullptr);

    Want want;
    want.SetElementName("com.example.test", "TestAbility");
    std::string hostBundleName = "com.example.host";

    std::atomic<bool> taskExecuted = false;
    PreloadTask task = [&taskExecuted](int32_t pid, int32_t id) {
        (void)pid; (void)id;
        taskExecuted = true;
    };

    auto startTime = std::chrono::steady_clock::now();
    client->PreloadUIExtensionAbility(want, hostBundleName, std::move(task));

    int waitCount = 0;
    while (!taskExecuted && waitCount++ < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();

    EXPECT_TRUE(taskExecuted.load());
}

HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionAbility_0600, TestSize.Level1)
{
    auto client = PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(client, nullptr);
    Want want;
    want.SetElementName("com.example.test", "TestAbility");
    std::string hostBundleName = "com.example.host";

    bool taskRun = false;
    PreloadTask task = [&taskRun](int32_t, int32_t) {
        taskRun = true;
    };
    client->PreloadUIExtensionAbility(want, hostBundleName, std::move(task));

    int waitCount = 0;
    while (!taskRun && waitCount++ < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(taskRun);
}

/**
 * @tc.name: PreloadUIExtensionHostClient_OnLoadedDone_0100
 * @tc.desc: Test OnLoadedDone with various scenarios.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_OnLoadedDone_0100, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);
    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        instance->loadedCallbackMap_.clear();
    }

    std::string testBundleName = "com.ohos.testbundle";
    int32_t testExtensionAbilityId = 999;
    int32_t callingPid = IPCSkeleton::GetCallingPid();

    instance->OnLoadedDone(callingPid, testBundleName, testExtensionAbilityId);
    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_EQ(instance->loadedCallbackMap_.size(), 0);
    }
    instance->AddLoadedCallback(nullptr, testBundleName);
    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_GT(instance->loadedCallbackMap_.size(), 0);
    }
    instance->OnLoadedDone(99999, testBundleName, testExtensionAbilityId);  

    instance->OnLoadedDone(callingPid, "com.ohos.different", testExtensionAbilityId);

    instance->AddLoadedCallback(nullptr, testBundleName);
    instance->AddLoadedCallback(nullptr, testBundleName);
    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(callingPid, testBundleName);
        EXPECT_EQ(instance->loadedCallbackMap_[key].size(), 3);
    }
    instance->OnLoadedDone(callingPid, testBundleName, testExtensionAbilityId);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(callingPid, testBundleName);
        instance->loadedCallbackMap_[key][999] = nullptr;
    }
    instance->OnLoadedDone(callingPid, testBundleName, testExtensionAbilityId);

    instance->OnLoadedDone(callingPid, testBundleName, 0);
    instance->OnLoadedDone(callingPid, testBundleName, -1);
    instance->OnLoadedDone(callingPid, testBundleName, 999999);

    std::string emptyBundle = "";
    instance->AddLoadedCallback(nullptr, emptyBundle);
    instance->OnLoadedDone(callingPid, emptyBundle, testExtensionAbilityId);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(12345, "com.test.empty");
        instance->loadedCallbackMap_[key] = {};
    }
    instance->OnLoadedDone(12345, "com.test.empty", testExtensionAbilityId);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_GT(instance->loadedCallbackMap_.size(), 0);
    }
    instance->loadedCallbackMap_.clear();
}

/**
 * @tc.name: PreloadUIExtensionHostClient_OnDestroyDone_0100
 * @tc.desc: Test OnDestroyDone with various scenarios.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_OnDestroyDone_0100, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        instance->destroyCallbackMap_.clear();
    }

    std::string testBundleName = "com.ohos.testbundle";
    int32_t testExtensionAbilityId = 999;
    int32_t testHostPid = 12345;

    instance->OnDestroyDone(testHostPid, testBundleName, testExtensionAbilityId);
    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_EQ(instance->destroyCallbackMap_.size(), 0);
    }

    instance->AddDestroyCallback(nullptr, testBundleName);
    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_GT(instance->destroyCallbackMap_.size(), 0);
    }
    instance->OnDestroyDone(99999, testBundleName, testExtensionAbilityId); 

    instance->OnDestroyDone(testHostPid, "com.ohos.different", testExtensionAbilityId);

    instance->AddDestroyCallback(nullptr, testBundleName);
    instance->AddDestroyCallback(nullptr, testBundleName);
    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(testHostPid, testBundleName);
        EXPECT_GE(instance->destroyCallbackMap_[key].size(), 1);
    }
    instance->OnDestroyDone(testHostPid, testBundleName, testExtensionAbilityId);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(testHostPid, testBundleName);
        instance->destroyCallbackMap_[key][999] = nullptr;
    }
    instance->OnDestroyDone(testHostPid, testBundleName, testExtensionAbilityId);

    instance->OnDestroyDone(testHostPid, testBundleName, 0);
    instance->OnDestroyDone(testHostPid, testBundleName, -1);
    instance->OnDestroyDone(testHostPid, testBundleName, 999999);
    std::string emptyBundle = "";
    int32_t testHostPid2 = 54321;
    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(testHostPid2, emptyBundle);
        instance->destroyCallbackMap_[key][1] = nullptr;
    }
    instance->OnDestroyDone(testHostPid2, emptyBundle, testExtensionAbilityId);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(11111, "com.test.empty");
        instance->destroyCallbackMap_[key] = {};
    }
    instance->OnDestroyDone(11111, "com.test.empty", testExtensionAbilityId);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(testHostPid, testBundleName);
        instance->destroyCallbackMap_[key][100] = nullptr;
        instance->destroyCallbackMap_[key][200] = nullptr;
    }
    instance->OnDestroyDone(testHostPid, testBundleName, testExtensionAbilityId);
    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_GT(instance->destroyCallbackMap_.size(), 0);
    }
    instance->destroyCallbackMap_.clear();
}

HWTEST_F(UIExtensionPreloadTest, PreloadUIExtensionHostClient_OnPreloadSuccess_0100, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);
    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_.clear();
    }

    int32_t requestCode = 100;
    int32_t extensionAbilityId = 200;
    int32_t innerErrCode = 0;
    instance->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);
    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        EXPECT_TRUE(instance->resultCallbacks_.empty());
    }

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_[requestCode] = nullptr;
    }
    instance->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);
    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        EXPECT_TRUE(instance->resultCallbacks_.empty());
    }
    requestCode = 101;
    std::atomic<bool> taskExecuted{false};
    auto testTask = [&taskExecuted](int32_t id, int32_t err) {
        (void)id;
        (void)err;
        taskExecuted = true;
    };

    auto runner = AppExecFwk::EventRunner::Create("TestRunner");
    ASSERT_NE(runner, nullptr);
    auto handler = std::make_shared<AppExecFwk::EventHandler>(runner);

    auto callData = std::make_shared<AbilityRuntime::PreloadByCallData>(nullptr);
    callData->handler_ = handler;

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_[requestCode] = callData;
    }
    instance->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);
    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        EXPECT_TRUE(instance->resultCallbacks_.empty());
    }

    requestCode = 102;
    callData = std::make_shared<AbilityRuntime::PreloadByCallData>(testTask);
    callData->handler_ = nullptr;

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_[requestCode] = callData;
    }
    instance->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);
    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        EXPECT_TRUE(instance->resultCallbacks_.empty());
    }
    requestCode = 103;
    taskExecuted = false;
    callData = std::make_shared<AbilityRuntime::PreloadByCallData>(testTask);
    callData->handler_ = handler;

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_[requestCode] = callData;
    }

    instance->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);
    int waitCount = 0;
    while (!taskExecuted && waitCount < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        waitCount++;
    }

    EXPECT_TRUE(taskExecuted.load());
    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        EXPECT_TRUE(instance->resultCallbacks_.empty());
    }
    requestCode = 104;
    int32_t anotherRequestCode = 105;
    
    auto callData1 = std::make_shared<AbilityRuntime::PreloadByCallData>(testTask);
    callData1->handler_ = handler;
    auto callData2 = std::make_shared<AbilityRuntime::PreloadByCallData>(testTask);
    callData2->handler_ = handler;

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_[requestCode] = callData1;
        instance->resultCallbacks_[anotherRequestCode] = callData2;
    }

    instance->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        EXPECT_EQ(instance->resultCallbacks_.size(), 1);
        EXPECT_TRUE(instance->resultCallbacks_.find(anotherRequestCode) != instance->resultCallbacks_.end());
    }
    std::atomic<int32_t> receivedId{-1};
    std::atomic<int32_t> receivedErr{-1};
    auto verifyTask = [&receivedId, &receivedErr](int32_t id, int32_t err) {
        receivedId = id;
        receivedErr = err;
    };

    requestCode = 106;
    callData = std::make_shared<AbilityRuntime::PreloadByCallData>(verifyTask);
    callData->handler_ = handler;

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_[requestCode] = callData;
    }

    instance->OnPreloadSuccess(requestCode, 999, -100);

    waitCount = 0;
    while (receivedId == -1 && waitCount < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        waitCount++;
    }

    EXPECT_EQ(receivedId.load(), 999);
    EXPECT_EQ(receivedErr.load(), -100);
    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_.clear();
    }
}
} // namespace AAFwk
} // namespace OHOS
