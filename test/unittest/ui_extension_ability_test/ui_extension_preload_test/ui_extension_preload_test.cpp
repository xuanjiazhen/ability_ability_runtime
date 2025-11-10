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
using PreloadUIExtensionHostClient = OHOS::AbilityRuntime::PreloadUIExtensionHostClient;
using PreloadTask = OHOS::AbilityRuntime::PreloadTask;

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

/**
 * @tc.name: OnLoadedDone_0100
 * @tc.desc: Test OnLoadedDone when loadedCallbackMap_ is empty.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnLoadedDone_0100, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        instance->loadedCallbackMap_.clear();
        EXPECT_EQ(instance->loadedCallbackMap_.size(), 0);
    }

    int32_t callingPid = IPCSkeleton::GetCallingPid();
    std::string testBundleName = "com.ohos.testbundle";
    int32_t testExtensionAbilityId = 999;

    instance->OnLoadedDone(callingPid, testBundleName, testExtensionAbilityId);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_EQ(instance->loadedCallbackMap_.size(), 0);
    }
}

/**
 * @tc.name: OnLoadedDone_0200
 * @tc.desc: Test AddLoadedCallback adds entries correctly.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnLoadedDone_0200, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->loadedCallbackMap_.clear();

    std::string testBundleName = "com.ohos.testbundle";
    instance->AddLoadedCallback(nullptr, testBundleName);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_GT(instance->loadedCallbackMap_.size(), 0);
        bool found = false;
        for (const auto& pair : instance->loadedCallbackMap_) {
            if (pair.first.second == testBundleName) {
                found = true;
                EXPECT_GT(pair.second.size(), 0);
            }
        }
        EXPECT_TRUE(found);
    }
}

/**
 * @tc.name: OnLoadedDone_0300
 * @tc.desc: Test OnLoadedDone with different callingPid does not clear unrelated callbacks.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnLoadedDone_0300, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->loadedCallbackMap_.clear();

    std::string testBundleName = "com.ohos.testbundle";
    int32_t testExtensionAbilityId = 999;
    int32_t callingPid = IPCSkeleton::GetCallingPid();

    instance->AddLoadedCallback(nullptr, testBundleName);

    instance->OnLoadedDone(99999, testBundleName, testExtensionAbilityId);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(callingPid, testBundleName);
        EXPECT_GT(instance->loadedCallbackMap_[key].size(), 0);
    }
}

/**
 * @tc.name: OnLoadedDone_0400
 * @tc.desc: Test OnLoadedDone with different bundleName does not clear unrelated callbacks.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnLoadedDone_0400, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->loadedCallbackMap_.clear();

    int32_t callingPid = IPCSkeleton::GetCallingPid();
    int32_t testExtensionAbilityId = 999;

    instance->AddLoadedCallback(nullptr, "com.ohos.testbundle");

    instance->OnLoadedDone(callingPid, "com.ohos.different", testExtensionAbilityId);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(callingPid, "com.ohos.testbundle");
        EXPECT_GT(instance->loadedCallbackMap_[key].size(), 0);
        auto keyDifferent = std::make_pair(callingPid, "com.ohos.different");
        EXPECT_EQ(instance->loadedCallbackMap_.count(keyDifferent), 0);
    }
}

/**
 * @tc.name: OnLoadedDone_0500
 * @tc.desc: Test AddLoadedCallback multiple times increases callback count.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnLoadedDone_0500, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->loadedCallbackMap_.clear();

    int32_t callingPid = IPCSkeleton::GetCallingPid();
    std::string testBundleName = "com.ohos.testbundle";

    instance->AddLoadedCallback(nullptr, testBundleName);
    instance->AddLoadedCallback(nullptr, testBundleName);
    instance->AddLoadedCallback(nullptr, testBundleName);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(callingPid, testBundleName);
        EXPECT_EQ(instance->loadedCallbackMap_[key].size(), 3);
    }
}

/**
 * @tc.name: OnLoadedDone_0600
 * @tc.desc: Test OnLoadedDone with special extensionAbilityId values 0, -1, large number.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnLoadedDone_0600, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->loadedCallbackMap_.clear();

    int32_t callingPid = IPCSkeleton::GetCallingPid();
    std::string testBundleName = "com.ohos.testbundle";

    instance->AddLoadedCallback(nullptr, testBundleName);

    int32_t extensionIds[] = {0, -1, 999999};
    for (auto id : extensionIds) {
        instance->OnLoadedDone(callingPid, testBundleName, id);
    }

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(callingPid, testBundleName);
        EXPECT_GT(instance->loadedCallbackMap_[key].size(), 0);
    }
}

/**
 * @tc.name: OnLoadedDone_0700
 * @tc.desc: Test OnLoadedDone with empty bundle name and callbacks.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnLoadedDone_0700, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->loadedCallbackMap_.clear();

    int32_t callingPid = IPCSkeleton::GetCallingPid();
    std::string emptyBundle = "";

    instance->AddLoadedCallback(nullptr, emptyBundle);
    instance->OnLoadedDone(callingPid, emptyBundle, 999);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(callingPid, emptyBundle);
        EXPECT_GE(instance->loadedCallbackMap_[key].size(), 0);
    }
}

/**
 * @tc.name: OnLoadedDone_0800
 * @tc.desc: Test OnLoadedDone with empty callback vector for a key.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnLoadedDone_0800, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->loadedCallbackMap_.clear();

    auto key = std::make_pair(12345, std::string("com.test.empty"));
    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        instance->loadedCallbackMap_[key] = {};
    }

    instance->OnLoadedDone(12345, "com.test.empty", 999);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_TRUE(instance->loadedCallbackMap_[key].empty());
    }
}

/**
 * @tc.name: OnLoadedDone_0900
 * @tc.desc: Test OnLoadedDone with multiple callback IDs for a single key.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnLoadedDone_0900, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->loadedCallbackMap_.clear();

    int32_t callingPid = IPCSkeleton::GetCallingPid();
    std::string testBundleName = "com.ohos.testbundle";
    auto key = std::make_pair(callingPid, testBundleName);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        instance->loadedCallbackMap_[key][100] = nullptr;
        instance->loadedCallbackMap_[key][200] = nullptr;
    }

    int32_t testExtensionAbilityId = 999;
    instance->OnLoadedDone(callingPid, testBundleName, testExtensionAbilityId);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_GT(instance->loadedCallbackMap_.size(), 0);
    }
}


/**
 * @tc.name: OnDestroyDone_0100
 * @tc.desc: Test OnDestroyDone when destroyCallbackMap_ is empty.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnDestroyDone_0100, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        instance->destroyCallbackMap_.clear();
        EXPECT_EQ(instance->destroyCallbackMap_.size(), 0);
    }
    instance->OnDestroyDone(12345, "com.ohos.testbundle", 999);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_EQ(instance->destroyCallbackMap_.size(), 0);
    }
}

/**
 * @tc.name: OnDestroyDone_0200
 * @tc.desc: Test AddDestroyCallback adds callback entries correctly.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnDestroyDone_0200, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->destroyCallbackMap_.clear();

    std::string bundleName = "com.ohos.testbundle";
    instance->AddDestroyCallback(nullptr, bundleName);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_GT(instance->destroyCallbackMap_.size(), 0);
        bool found = false;
        for (const auto& pair : instance->destroyCallbackMap_) {
            if (pair.first.second == bundleName) {
                found = true;
                EXPECT_GT(pair.second.size(), 0);
            }
        }
        EXPECT_TRUE(found);
    }
}

/**
 * @tc.name: OnDestroyDone_0300
 * @tc.desc: Test OnDestroyDone when bundle name is empty string.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnDestroyDone_0300, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->destroyCallbackMap_.clear();

    int32_t hostPid2 = 54321;
    std::string emptyBundle = "";

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(hostPid2, emptyBundle);
        instance->destroyCallbackMap_[key][1] = nullptr;
    }

    instance->OnDestroyDone(hostPid2, emptyBundle, 999);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(hostPid2, emptyBundle);
        EXPECT_GE(instance->destroyCallbackMap_[key].size(), 0);
    }
}

/**
 * @tc.name: OnDestroyDone_0400
 * @tc.desc: Test OnDestroyDone when destroyCallbackMap_ contains empty callback vector.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnDestroyDone_0400, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->destroyCallbackMap_.clear();

    int32_t hostPid = 11111;
    std::string bundleName = "com.test.empty";

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(hostPid, bundleName);
        instance->destroyCallbackMap_[key] = {};
    }

    instance->OnDestroyDone(hostPid, bundleName, 999);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(hostPid, bundleName);
        EXPECT_TRUE(instance->destroyCallbackMap_[key].empty());
    }
}

/**
 * @tc.name: OnDestroyDone_0500
 * @tc.desc: Test OnDestroyDone with multiple callback entries for a given key.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnDestroyDone_0500, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    instance->destroyCallbackMap_.clear();

    std::string bundleName = "com.ohos.testbundle";
    int32_t hostPid = 12345;

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        auto key = std::make_pair(hostPid, bundleName);
        instance->destroyCallbackMap_[key][100] = nullptr;
        instance->destroyCallbackMap_[key][200] = nullptr;
    }
    int32_t extensionAbilityId = 999;
    instance->OnDestroyDone(hostPid, bundleName, extensionAbilityId);

    {
        std::lock_guard<std::mutex> lock(instance->preloadUIExtensionLoadedCallbackMutex_);
        EXPECT_GT(instance->destroyCallbackMap_.size(), 0);
    }
}

/**
 * @tc.name: OnPreloadSuccess_0100
 * @tc.desc: Test OnPreloadSuccess when resultCallbacks_ is empty.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnPreloadSuccess_0100, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_.clear();
        EXPECT_TRUE(instance->resultCallbacks_.empty());
    }

    int32_t requestCode = 100;
    int32_t extensionAbilityId = 200;
    int32_t innerErrCode = 0;

    instance->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        EXPECT_TRUE(instance->resultCallbacks_.empty());
    }
}

/**
 * @tc.name: OnPreloadSuccess_0200
 * @tc.desc: Test OnPreloadSuccess when callback is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnPreloadSuccess_0200, TestSize.Level1)
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

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_[requestCode] = nullptr;
    }

    instance->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        EXPECT_TRUE(instance->resultCallbacks_.empty());
    }
}

/**
 * @tc.name: OnPreloadSuccess_0300
 * @tc.desc: Test OnPreloadSuccess when callback task is nullptr but handler exists.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnPreloadSuccess_0300, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_.clear();
    }

    int32_t requestCode = 101;
    int32_t extensionAbilityId = 200;
    int32_t innerErrCode = 0;

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
}

/**
 * @tc.name: OnPreloadSuccess_0400
 * @tc.desc: Test OnPreloadSuccess when handler is nullptr but callback task exists.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnPreloadSuccess_0400, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_.clear();
    }

    int32_t requestCode = 102;
    int32_t extensionAbilityId = 200;
    int32_t innerErrCode = 0;

    std::atomic<bool> taskExecuted{false};
    auto testTask = [&taskExecuted](int32_t id, int32_t err) {
        (void)id;
        (void)err;
        taskExecuted = true;
    };

    auto callData = std::make_shared<AbilityRuntime::PreloadByCallData>(testTask);
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
    EXPECT_FALSE(taskExecuted.load());
}

/**
 * @tc.name: OnPreloadSuccess_0500
 * @tc.desc: Test OnPreloadSuccess executes callback task successfully.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnPreloadSuccess_0500, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_.clear();
    }

    int32_t requestCode = 103;
    int32_t extensionAbilityId = 200;
    int32_t innerErrCode = 0;

    std::atomic<bool> taskExecuted{false};
    auto testTask = [&taskExecuted](int32_t id, int32_t err) {
        (void)id;
        (void)err;
        taskExecuted = true;
    };

    auto runner = AppExecFwk::EventRunner::Create("TestRunner");
    ASSERT_NE(runner, nullptr);
    auto handler = std::make_shared<AppExecFwk::EventHandler>(runner);

    auto callData = std::make_shared<AbilityRuntime::PreloadByCallData>(testTask);
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
}

/**
 * @tc.name: OnPreloadSuccess_0600
 * @tc.desc: Test OnPreloadSuccess removes only the matching callback, leaves others intact.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnPreloadSuccess_0600, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_.clear();
    }

    int32_t requestCode = 104;
    int32_t anotherRequestCode = 105;
    int32_t extensionAbilityId = 200;
    int32_t innerErrCode = 0;

    std::atomic<bool> taskExecuted{false};
    auto testTask = [&taskExecuted](int32_t id, int32_t err) {
        (void)id;
        (void)err;
        taskExecuted = true;
    };

    auto runner = AppExecFwk::EventRunner::Create("TestRunner");
    ASSERT_NE(runner, nullptr);
    auto handler = std::make_shared<AppExecFwk::EventHandler>(runner);

    auto callData1 = std::make_shared<AbilityRuntime::PreloadByCallData>(testTask);
    callData1->handler_ = handler;
    auto callData2 = std::make_shared<AbilityRuntime::PreloadByCallData>(testTask);
    callData2->handler_ = handler;

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_[requestCode] = callData1;
        instance->resultCallbacks_[anotherRequestCode] = callData2;
        EXPECT_EQ(instance->resultCallbacks_.size(), 2);
    }

    instance->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        EXPECT_EQ(instance->resultCallbacks_.size(), 1);
        EXPECT_TRUE(instance->resultCallbacks_.find(anotherRequestCode) != instance->resultCallbacks_.end());
        EXPECT_TRUE(instance->resultCallbacks_.find(requestCode) == instance->resultCallbacks_.end());
    }
}

/**
 * @tc.name: OnPreloadSuccess_0700
 * @tc.desc: Test OnPreloadSuccess passes correct parameters to callback.
 * @tc.type: FUNC
 */
HWTEST_F(UIExtensionPreloadTest, OnPreloadSuccess_0700, TestSize.Level1)
{
    auto instance = OHOS::AbilityRuntime::PreloadUIExtensionHostClient::GetInstance();
    ASSERT_NE(instance, nullptr);

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_.clear();
    }

    int32_t requestCode = 106;
    int32_t extensionAbilityId = 999;
    int32_t innerErrCode = -100;

    std::atomic<int32_t> receivedId{-1};
    std::atomic<int32_t> receivedErr{-1};

    auto verifyTask = [&receivedId, &receivedErr](int32_t id, int32_t err) {
        receivedId = id;
        receivedErr = err;
    };

    auto runner = AppExecFwk::EventRunner::Create("TestRunner");
    ASSERT_NE(runner, nullptr);
    auto handler = std::make_shared<AppExecFwk::EventHandler>(runner);

    auto callData = std::make_shared<AbilityRuntime::PreloadByCallData>(verifyTask);
    callData->handler_ = handler;

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        instance->resultCallbacks_[requestCode] = callData;
    }

    instance->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);

    int waitCount = 0;
    while (receivedId == -1 && waitCount < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        waitCount++;
    }

    EXPECT_EQ(receivedId.load(), extensionAbilityId);
    EXPECT_EQ(receivedErr.load(), innerErrCode);

    {
        std::lock_guard<std::mutex> lock(instance->requestCodeMutex_);
        EXPECT_TRUE(instance->resultCallbacks_.empty());
    }
}
} // namespace AAFwk
} // namespace OHOS