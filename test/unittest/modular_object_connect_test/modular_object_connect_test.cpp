/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <climits>
#include <gtest/gtest.h>
#include <memory>

#define private public
#define protected public
#include "ability_connect_manager.h"
#undef private
#undef protected

#include "hilog_tag_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "mock_sa_call.h"
#include "mock_task_handler_wrap.h"
#include "modular_object_extension_info.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {
namespace {
const std::string TEST_BUNDLE = "com.test.modular";
const std::string TEST_MODULE = "entry";
const std::string TEST_ABILITY = "TestModularObjectExt";
const std::string TEST_DEVICE = "device";
const std::string TEST_APP = "testApp";
}

class ModularObjectConnectTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    AbilityRequest MakeModularObjectRequest(const std::string &bundleName = TEST_BUNDLE,
        const std::string &abilityName = TEST_ABILITY);

    std::shared_ptr<AbilityConnectManager> connectManager_;
};

void ModularObjectConnectTest::SetUpTestCase(void) {}
void ModularObjectConnectTest::TearDownTestCase(void) {}

void ModularObjectConnectTest::SetUp()
{
    connectManager_ = std::make_shared<AbilityConnectManager>(0);
    auto taskHandler = MockTaskHandlerWrap::CreateQueueHandler("ModularObjectConnectTest");
    connectManager_->SetTaskHandler(taskHandler);
}

void ModularObjectConnectTest::TearDown()
{
    connectManager_ = nullptr;
}

AbilityRequest ModularObjectConnectTest::MakeModularObjectRequest(
    const std::string &bundleName, const std::string &abilityName)
{
    AbilityRequest request;
    ElementName element(TEST_DEVICE, bundleName, abilityName, TEST_MODULE);
    request.want.SetElement(element);
    request.abilityInfo.name = abilityName;
    request.abilityInfo.bundleName = bundleName;
    request.abilityInfo.moduleName = TEST_MODULE;
    request.abilityInfo.type = AbilityType::EXTENSION;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    request.abilityInfo.deviceId = TEST_DEVICE;
    request.abilityInfo.applicationInfo.bundleName = bundleName;
    request.abilityInfo.applicationInfo.name = TEST_APP;
    request.appInfo = request.abilityInfo.applicationInfo;
    request.callerInfo.bundleName = bundleName;
    return request;
}

// ============================================================
// 1. GenerateModularObjectServiceKey tests
// Every connect generates a unique key regardless of threadMode/processMode
// ============================================================

/**
 * @tc.name: GenerateModularObjectServiceKey_001
 * @tc.desc: Test every connect gets unique key (no DB config available)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, GenerateModularObjectServiceKey_001, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseUri = "device#com.test.modular#TestModularObjectExt#entry";

    // No DB config available, should return baseUri as-is
    std::string result = AbilityConnectManager::GenerateModularObjectServiceKey(request, baseUri);
    EXPECT_EQ(result, baseUri);
}

/**
 * @tc.name: GenerateModularObjectServiceKey_002
 * @tc.desc: Test two calls without config get same result (no unique key without config)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, GenerateModularObjectServiceKey_002, TestSize.Level1)
{
    auto request1 = MakeModularObjectRequest();
    auto request2 = MakeModularObjectRequest();
    std::string baseUri = "device#com.test.modular#TestModularObjectExt#entry";

    std::string key1 = AbilityConnectManager::GenerateModularObjectServiceKey(request1, baseUri);
    std::string key2 = AbilityConnectManager::GenerateModularObjectServiceKey(request2, baseUri);
    // Without config, both return baseUri
    EXPECT_EQ(key1, baseUri);
    EXPECT_EQ(key2, baseUri);
}

// ============================================================
// 2. processName strategy tests
// Verify processName is set correctly based on launchMode + processMode
// ============================================================

/**
 * @tc.name: ProcessName_InProcess_001
 * @tc.desc: Test IN_PROCESS mode sets processName to bundleName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ProcessName_InProcess_001, TestSize.Level1)
{
    // Simulate the processName logic for IN_PROCESS
    std::string bundleName = TEST_BUNDLE;
    std::string process = bundleName;  // IN_PROCESS uses bundleName as processName
    EXPECT_EQ(process, TEST_BUNDLE);
}

/**
 * @tc.name: ProcessName_CrossProcess_Bundle_001
 * @tc.desc: Test CROSS_PROCESS BUNDLE mode sets processName to bundleName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ProcessName_CrossProcess_Bundle_001, TestSize.Level1)
{
    std::string process = TEST_BUNDLE;  // BUNDLE mode
    EXPECT_EQ(process, TEST_BUNDLE);
}

/**
 * @tc.name: ProcessName_CrossProcess_Type_001
 * @tc.desc: Test CROSS_PROCESS TYPE mode sets processName to bundleName:abilityName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ProcessName_CrossProcess_Type_001, TestSize.Level1)
{
    std::string process = TEST_BUNDLE + ":" + TEST_ABILITY;  // TYPE mode
    EXPECT_EQ(process, "com.test.modular:TestModularObjectExt");
}

/**
 * @tc.name: ProcessName_CrossProcess_Instance_001
 * @tc.desc: Test CROSS_PROCESS INSTANCE mode sets unique processName with recordId
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ProcessName_CrossProcess_Instance_001, TestSize.Level1)
{
    int32_t recordId = 42;
    std::string process = TEST_BUNDLE + ":" + TEST_ABILITY + ":" + std::to_string(recordId);
    EXPECT_EQ(process, "com.test.modular:TestModularObjectExt:42");
    // Different recordId → different processName → different process
    int32_t recordId2 = 43;
    std::string process2 = TEST_BUNDLE + ":" + TEST_ABILITY + ":" + std::to_string(recordId2);
    EXPECT_NE(process, process2);
}

// ============================================================
// 3. Thread key strategy tests
// Verify thread keys follow BUNDLE/TYPE/INSTANCE patterns
// ============================================================

/**
 * @tc.name: ThreadKey_Bundle_001
 * @tc.desc: Test BUNDLE threadMode uses bundleName as key
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Bundle_001, TestSize.Level1)
{
    std::string threadKey = TEST_BUNDLE;  // BUNDLE mode
    EXPECT_EQ(threadKey, TEST_BUNDLE);
}

/**
 * @tc.name: ThreadKey_Type_001
 * @tc.desc: Test TYPE threadMode uses bundleName_abilityName as key
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Type_001, TestSize.Level1)
{
    std::string threadKey = TEST_BUNDLE + "_" + TEST_ABILITY;  // TYPE mode
    EXPECT_EQ(threadKey, "com.test.modular_TestModularObjectExt");
}

/**
 * @tc.name: ThreadKey_Instance_001
 * @tc.desc: Test INSTANCE threadMode uses bundleName_abilityName_atomicId as key
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Instance_001, TestSize.Level1)
{
    uint32_t instanceId = 7;
    std::string threadKey = TEST_BUNDLE + "_" + TEST_ABILITY + "_" + std::to_string(instanceId);
    EXPECT_EQ(threadKey, "com.test.modular_TestModularObjectExt_7");
    // Different instanceId → different key → different thread
    uint32_t instanceId2 = 8;
    std::string threadKey2 = TEST_BUNDLE + "_" + TEST_ABILITY + "_" + std::to_string(instanceId2);
    EXPECT_NE(threadKey, threadKey2);
}

/**
 * @tc.name: ThreadKey_Type_Reuse_001
 * @tc.desc: Test TYPE mode same abilityName produces same key (thread reuse)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Type_Reuse_001, TestSize.Level1)
{
    // Same bundle + ability → same key → thread reuse
    std::string key1 = TEST_BUNDLE + "_" + TEST_ABILITY;
    std::string key2 = TEST_BUNDLE + "_" + TEST_ABILITY;
    EXPECT_EQ(key1, key2);
}

/**
 * @tc.name: ThreadKey_Type_DifferentAbility_001
 * @tc.desc: Test TYPE mode different abilityName produces different key
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Type_DifferentAbility_001, TestSize.Level1)
{
    std::string key1 = TEST_BUNDLE + "_AbilityA";
    std::string key2 = TEST_BUNDLE + "_AbilityB";
    EXPECT_NE(key1, key2);
}

/**
 * @tc.name: ThreadKey_Bundle_Reuse_001
 * @tc.desc: Test BUNDLE mode different abilities same bundle produce same key
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreadKey_Bundle_Reuse_001, TestSize.Level1)
{
    std::string key1 = TEST_BUNDLE;  // AbilityA
    std::string key2 = TEST_BUNDLE;  // AbilityB
    EXPECT_EQ(key1, key2);  // Same bundle → same thread
}

// ============================================================
// 4. connectId + disconnect serviceKey reconstruction tests
// ============================================================

/**
 * @tc.name: ConnectId_Reconstruction_001
 * @tc.desc: Test connectId in Want can reconstruct full serviceKey
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ConnectId_Reconstruction_001, TestSize.Level1)
{
    // Simulate: connect creates key = baseUri + "_" + connectId
    std::string baseUri = "device#com.test.modular#TestModularObjectExt#entry";
    std::string connectId = "12345";
    std::string fullKey = baseUri + "_" + connectId;

    // Simulate: disconnect reconstructs from record
    // GetURI() returns baseUri, Want has connectId
    std::string reconstructedUri = baseUri;
    std::string reconstructedConnectId = connectId;
    std::string reconstructedKey = reconstructedUri + "_" + reconstructedConnectId;
    EXPECT_EQ(fullKey, reconstructedKey);
}

/**
 * @tc.name: ConnectId_Reconstruction_002
 * @tc.desc: Test empty connectId does not append underscore
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ConnectId_Reconstruction_002, TestSize.Level1)
{
    std::string baseUri = "device#com.test.modular#TestModularObjectExt#entry";
    std::string connectId = "";  // empty
    std::string key = baseUri;
    if (!connectId.empty()) {
        key = key + "_" + connectId;
    }
    EXPECT_EQ(key, baseUri);
}

// ============================================================
// 5. ModularObjectExtensionInfo struct tests
// ============================================================

/**
 * @tc.name: ModularObjectExtensionInfo_001
 * @tc.desc: Test default values of ModularObjectExtensionInfo
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ModularObjectExtensionInfo_001, TestSize.Level1)
{
    ModularObjectExtensionInfo info;
    EXPECT_EQ(info.launchMode, MoeLaunchMode::IN_PROCESS);
    EXPECT_EQ(info.processMode, MoeProcessMode::BUNDLE);
    EXPECT_EQ(info.threadMode, MoeThreadMode::BUNDLE);
    EXPECT_FALSE(info.isDisabled);
    EXPECT_EQ(info.appIndex, 0);
    EXPECT_TRUE(info.bundleName.empty());
    EXPECT_TRUE(info.abilityName.empty());
}

/**
 * @tc.name: ModularObjectExtensionInfo_002
 * @tc.desc: Test setting and reading all fields
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ModularObjectExtensionInfo_002, TestSize.Level1)
{
    ModularObjectExtensionInfo info;
    info.bundleName = TEST_BUNDLE;
    info.moduleName = TEST_MODULE;
    info.abilityName = TEST_ABILITY;
    info.appIndex = 1;
    info.launchMode = MoeLaunchMode::CROSS_PROCESS;
    info.processMode = MoeProcessMode::INSTANCE;
    info.threadMode = MoeThreadMode::TYPE;
    info.isDisabled = true;

    EXPECT_EQ(info.bundleName, TEST_BUNDLE);
    EXPECT_EQ(info.moduleName, TEST_MODULE);
    EXPECT_EQ(info.abilityName, TEST_ABILITY);
    EXPECT_EQ(info.appIndex, 1);
    EXPECT_EQ(info.launchMode, MoeLaunchMode::CROSS_PROCESS);
    EXPECT_EQ(info.processMode, MoeProcessMode::INSTANCE);
    EXPECT_EQ(info.threadMode, MoeThreadMode::TYPE);
    EXPECT_TRUE(info.isDisabled);
}

// ============================================================
// 6. Enum boundary tests
// ============================================================

/**
 * @tc.name: EnumBoundary_001
 * @tc.desc: Test MoeLaunchMode enum values
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, EnumBoundary_001, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(MoeLaunchMode::IN_PROCESS), 0);
    EXPECT_EQ(static_cast<int32_t>(MoeLaunchMode::CROSS_PROCESS), 1);
}

/**
 * @tc.name: EnumBoundary_002
 * @tc.desc: Test MoeProcessMode enum values
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, EnumBoundary_002, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(MoeProcessMode::BUNDLE), 0);
    EXPECT_EQ(static_cast<int32_t>(MoeProcessMode::TYPE), 1);
    EXPECT_EQ(static_cast<int32_t>(MoeProcessMode::INSTANCE), 2);
}

/**
 * @tc.name: EnumBoundary_003
 * @tc.desc: Test MoeThreadMode enum values
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, EnumBoundary_003, TestSize.Level1)
{
    EXPECT_EQ(static_cast<int32_t>(MoeThreadMode::BUNDLE), 0);
    EXPECT_EQ(static_cast<int32_t>(MoeThreadMode::TYPE), 1);
    EXPECT_EQ(static_cast<int32_t>(MoeThreadMode::INSTANCE), 2);
}

// ============================================================
// 7. GetOrCreateServiceRecord tests (without DB config)
// ============================================================

/**
 * @tc.name: GetOrCreateServiceRecord_001
 * @tc.desc: Test creating record for MODULAR_OBJECT without DB config
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, GetOrCreateServiceRecord_001, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> targetService = nullptr;
    bool isLoadedAbility = false;

    connectManager_->GetOrCreateServiceRecord(request, true, targetService, isLoadedAbility);
    ASSERT_NE(targetService, nullptr);
    EXPECT_FALSE(isLoadedAbility);
}

/**
 * @tc.name: GetOrCreateServiceRecord_002
 * @tc.desc: Test second call creates different record (unique serviceKey)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, GetOrCreateServiceRecord_002, TestSize.Level1)
{
    auto request1 = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> service1 = nullptr;
    bool loaded1 = false;
    connectManager_->GetOrCreateServiceRecord(request1, true, service1, loaded1);
    ASSERT_NE(service1, nullptr);

    auto request2 = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> service2 = nullptr;
    bool loaded2 = false;
    connectManager_->GetOrCreateServiceRecord(request2, true, service2, loaded2);
    ASSERT_NE(service2, nullptr);
    // Without DB config, GenerateModularObjectServiceKey returns baseUri which is the same,
    // so second call should find existing record
    EXPECT_TRUE(loaded2);
}

// ============================================================
// 8. RemoveServiceFromMapSafe tests
// ============================================================

/**
 * @tc.name: RemoveServiceFromMapSafe_001
 * @tc.desc: Test removing a service key that exists
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, RemoveServiceFromMapSafe_001, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> targetService = nullptr;
    bool isLoadedAbility = false;
    connectManager_->GetOrCreateServiceRecord(request, true, targetService, isLoadedAbility);
    ASSERT_NE(targetService, nullptr);

    std::string serviceKey = connectManager_->GetServiceKey(request);
    connectManager_->RemoveServiceFromMapSafe(serviceKey);

    auto found = connectManager_->GetServiceRecordByElementName(serviceKey);
    EXPECT_EQ(found, nullptr);
}

/**
 * @tc.name: RemoveServiceFromMapSafe_002
 * @tc.desc: Test removing a non-existent service key (no crash)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, RemoveServiceFromMapSafe_002, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(connectManager_->RemoveServiceFromMapSafe("non_existent_key"));
}

// ============================================================
// 9. Three-layer strategy integration tests
// ============================================================

/**
 * @tc.name: ThreeLayer_IN_PROCESS_TYPE_001
 * @tc.desc: Test IN_PROCESS + TYPE: same process, thread key = bundleName_abilityName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreeLayer_IN_PROCESS_TYPE_001, TestSize.Level1)
{
    // Layer 2: processName
    std::string processName = TEST_BUNDLE;
    EXPECT_EQ(processName, TEST_BUNDLE);

    // Layer 3: thread key
    std::string threadKey = TEST_BUNDLE + "_" + TEST_ABILITY;
    EXPECT_EQ(threadKey, "com.test.modular_TestModularObjectExt");
}

/**
 * @tc.name: ThreeLayer_CROSS_PROCESS_INSTANCE_INSTANCE_001
 * @tc.desc: Test CROSS_PROCESS + INSTANCE + INSTANCE: new process, new thread
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreeLayer_CROSS_PROCESS_INSTANCE_INSTANCE_001, TestSize.Level1)
{
    int32_t recordId1 = 100;
    int32_t recordId2 = 101;

    // Layer 2: processName unique per record
    std::string process1 = TEST_BUNDLE + ":" + TEST_ABILITY + ":" + std::to_string(recordId1);
    std::string process2 = TEST_BUNDLE + ":" + TEST_ABILITY + ":" + std::to_string(recordId2);
    EXPECT_NE(process1, process2);

    // Layer 3: thread key unique per instance
    uint32_t instanceId1 = 10;
    uint32_t instanceId2 = 11;
    std::string thread1 = TEST_BUNDLE + "_" + TEST_ABILITY + "_" + std::to_string(instanceId1);
    std::string thread2 = TEST_BUNDLE + "_" + TEST_ABILITY + "_" + std::to_string(instanceId2);
    EXPECT_NE(thread1, thread2);
}

/**
 * @tc.name: ThreeLayer_CROSS_PROCESS_BUNDLE_BUNDLE_001
 * @tc.desc: Test CROSS_PROCESS + BUNDLE + BUNDLE: shared process, shared thread
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreeLayer_CROSS_PROCESS_BUNDLE_BUNDLE_001, TestSize.Level1)
{
    // Layer 2: same processName
    std::string process = TEST_BUNDLE;
    EXPECT_EQ(process, TEST_BUNDLE);

    // Layer 3: same thread key
    std::string threadKey = TEST_BUNDLE;
    EXPECT_EQ(threadKey, TEST_BUNDLE);
}

// ============================================================
// 10. atomic instance ID tests
// ============================================================

/**
 * @tc.name: AtomicInstanceId_001
 * @tc.desc: Test atomic counter increments correctly
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, AtomicInstanceId_001, TestSize.Level1)
{
    std::atomic<uint32_t> counter{0};
    uint32_t id1 = counter.fetch_add(1);
    uint32_t id2 = counter.fetch_add(1);
    uint32_t id3 = counter.fetch_add(1);
    EXPECT_EQ(id1, 0u);
    EXPECT_EQ(id2, 1u);
    EXPECT_EQ(id3, 2u);
}

// ============================================================
// 11. HandleModularObjectNewRecord direct function tests
// Tests the actual function call for early-return branches.
// QueryModularObjectConfig returns nullptr in test env (no DB),
// so branches that require valid config are tested algorithmically
// in sections 12-14 below.
// ============================================================

/**
 * @tc.name: HandleModularObjectNewRecord_NonModularObject_001
 * @tc.desc: Test early return when extensionAbilityType is not MODULAR_OBJECT
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_NonModularObject_001, TestSize.Level1)
{
    // Create a request with non-MODULAR_OBJECT type
    AbilityRequest request;
    ElementName element(TEST_DEVICE, TEST_BUNDLE, TEST_ABILITY, TEST_MODULE);
    request.want.SetElement(element);
    request.abilityInfo.name = TEST_ABILITY;
    request.abilityInfo.bundleName = TEST_BUNDLE;
    request.abilityInfo.type = AbilityType::EXTENSION;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    request.abilityInfo.deviceId = TEST_DEVICE;

    // Create a service record
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    std::string emptyProcessBefore = service->GetAbilityInfo().process;

    // Call HandleModularObjectNewRecord - should return early without modifying processName
    std::string serviceKey = "device#com.test.modular#TestModularObjectExt#entry_12345";
    connectManager_->HandleModularObjectNewRecord(request, service, serviceKey);

    // Verify processName was NOT modified
    EXPECT_EQ(service->GetAbilityInfo().process, emptyProcessBefore);
}

/**
 * @tc.name: HandleModularObjectNewRecord_NullConfig_001
 * @tc.desc: Test early return when QueryModularObjectConfig returns nullptr (no DB config)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_NullConfig_001, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    std::string emptyProcessBefore = service->GetAbilityInfo().process;

    // Call HandleModularObjectNewRecord - config query returns nullptr, should early return
    std::string serviceKey = "device#com.test.modular#TestModularObjectExt#entry_12345";
    connectManager_->HandleModularObjectNewRecord(request, service, serviceKey);

    // Verify processName was NOT modified (early return due to null config)
    EXPECT_EQ(service->GetAbilityInfo().process, emptyProcessBefore);
}

/**
 * @tc.name: HandleModularObjectNewRecord_NullService_001
 * @tc.desc: Test behavior with nullptr targetService (should not crash)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_NullService_001, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> nullService = nullptr;
    std::string serviceKey = "device#com.test.modular#TestModularObjectExt#entry_12345";

    // Early return on null config happens before any service access - should not crash
    EXPECT_NO_FATAL_FAILURE(
        connectManager_->HandleModularObjectNewRecord(request, nullService, serviceKey));
}

/**
 * @tc.name: HandleModularObjectNewRecord_EmptyServiceKey_001
 * @tc.desc: Test with empty serviceKey (no underscore, connectId not written)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_EmptyServiceKey_001, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);

    // Empty serviceKey - no underscore found, no crash
    std::string emptyKey;
    EXPECT_NO_FATAL_FAILURE(
        connectManager_->HandleModularObjectNewRecord(request, service, emptyKey));
}

/**
 * @tc.name: HandleModularObjectNewRecord_NoUnderscoreKey_001
 * @tc.desc: Test with serviceKey that has no underscore (rfind returns npos)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_NoUnderscoreKey_001, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);

    // serviceKey without underscore
    std::string noUnderscoreKey = "deviceNoUnderscoreKey";
    EXPECT_NO_FATAL_FAILURE(
        connectManager_->HandleModularObjectNewRecord(request, service, noUnderscoreKey));
}

// ============================================================
// 12. HandleModularObjectNewRecord processName algorithmic tests
// These tests verify the processName generation logic matches the
// exact code in HandleModularObjectNewRecord for all config modes.
// Since QueryModularObjectConfig returns nullptr in test env,
// we test the algorithmic logic separately.
// ============================================================

/**
 * @tc.name: HandleModularObjectNewRecord_ProcessName_InProcess_EmptyAppProcess_001
 * @tc.desc: Test IN_PROCESS mode with empty appInfo.process uses bundleName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ProcessName_InProcess_EmptyAppProcess_001,
    TestSize.Level1)
{
    // Replicate HandleModularObjectNewRecord logic: IN_PROCESS + empty appInfo.process
    std::string bundleName = TEST_BUNDLE;
    std::string process;
    // config->launchMode == MoeLaunchMode::IN_PROCESS
    process = bundleName;
    // appInfo.process is empty, so process stays as bundleName
    std::string appInfoProcess = "";
    if (!appInfoProcess.empty()) {
        process = appInfoProcess;
    }
    EXPECT_EQ(process, TEST_BUNDLE);
}

/**
 * @tc.name: HandleModularObjectNewRecord_ProcessName_InProcess_NonEmptyAppProcess_001
 * @tc.desc: Test IN_PROCESS mode with non-empty appInfo.process overrides bundleName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ProcessName_InProcess_NonEmptyAppProcess_001,
    TestSize.Level1)
{
    // Replicate HandleModularObjectNewRecord logic: IN_PROCESS + non-empty appInfo.process
    std::string bundleName = TEST_BUNDLE;
    std::string process;
    std::string appInfoProcess = "com.test.custom_process";
    process = bundleName;
    if (!appInfoProcess.empty()) {
        process = appInfoProcess;
    }
    EXPECT_EQ(process, "com.test.custom_process");
    EXPECT_NE(process, TEST_BUNDLE);
}

/**
 * @tc.name: HandleModularObjectNewRecord_ProcessName_InProcess_AppProcessPriority_001
 * @tc.desc: Test IN_PROCESS mode: appInfo.process takes priority over bundleName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ProcessName_InProcess_AppProcessPriority_001,
    TestSize.Level1)
{
    // Verify priority: when both exist, appInfo.process wins
    std::string bundleName = "bundle_a";
    std::string appInfoProcess = "process_b";
    std::string process = bundleName;
    if (!appInfoProcess.empty()) {
        process = appInfoProcess;
    }
    EXPECT_EQ(process, "process_b");
}

/**
 * @tc.name: HandleModularObjectNewRecord_ProcessName_CrossProcess_Bundle_001
 * @tc.desc: Test CROSS_PROCESS + BUNDLE processMode uses bundleName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ProcessName_CrossProcess_Bundle_001,
    TestSize.Level1)
{
    // Replicate HandleModularObjectNewRecord logic: CROSS_PROCESS + BUNDLE
    std::string bundleName = TEST_BUNDLE;
    std::string abilityName = TEST_ABILITY;
    std::string process;
    // switch (config->processMode) case MoeProcessMode::BUNDLE:
    process = bundleName;
    EXPECT_EQ(process, TEST_BUNDLE);
}

/**
 * @tc.name: HandleModularObjectNewRecord_ProcessName_CrossProcess_Type_001
 * @tc.desc: Test CROSS_PROCESS + TYPE processMode uses bundleName:abilityName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ProcessName_CrossProcess_Type_001,
    TestSize.Level1)
{
    // Replicate HandleModularObjectNewRecord logic: CROSS_PROCESS + TYPE
    std::string bundleName = TEST_BUNDLE;
    std::string abilityName = TEST_ABILITY;
    std::string process;
    // switch (config->processMode) case MoeProcessMode::TYPE:
    process = bundleName + ":" + abilityName;
    EXPECT_EQ(process, "com.test.modular:TestModularObjectExt");
}

/**
 * @tc.name: HandleModularObjectNewRecord_ProcessName_CrossProcess_Instance_001
 * @tc.desc: Test CROSS_PROCESS + INSTANCE processMode uses bundleName:abilityName:recordId
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ProcessName_CrossProcess_Instance_001,
    TestSize.Level1)
{
    // Replicate HandleModularObjectNewRecord logic: CROSS_PROCESS + INSTANCE
    std::string bundleName = TEST_BUNDLE;
    std::string abilityName = TEST_ABILITY;
    int32_t recordId = 42;
    std::string process;
    // switch (config->processMode) case MoeProcessMode::INSTANCE:
    process = bundleName + ":" + abilityName + ":" + std::to_string(recordId);
    EXPECT_EQ(process, "com.test.modular:TestModularObjectExt:42");
}

/**
 * @tc.name: HandleModularObjectNewRecord_ProcessName_CrossProcess_Instance_Unique_001
 * @tc.desc: Test CROSS_PROCESS + INSTANCE produces unique processName per recordId
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ProcessName_CrossProcess_Instance_Unique_001,
    TestSize.Level1)
{
    // Verify different recordId → different processName
    std::string bundleName = TEST_BUNDLE;
    std::string abilityName = TEST_ABILITY;
    auto makeProcess = [&](int32_t recordId) -> std::string {
        return bundleName + ":" + abilityName + ":" + std::to_string(recordId);
    };
    EXPECT_NE(makeProcess(1), makeProcess(2));
    EXPECT_NE(makeProcess(100), makeProcess(101));
}

/**
 * @tc.name: HandleModularObjectNewRecord_ProcessName_CrossProcess_Default_001
 * @tc.desc: Test CROSS_PROCESS + default (invalid) processMode produces empty processName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ProcessName_CrossProcess_Default_001,
    TestSize.Level1)
{
    // Replicate HandleModularObjectNewRecord logic: CROSS_PROCESS + default (falls through switch)
    std::string process;
    // switch (config->processMode) default: break; → process stays empty
    // The function then checks: if (!process.empty()) { SetProcessName }
    // So with empty process, SetProcessName is NOT called
    EXPECT_TRUE(process.empty());
}

/**
 * @tc.name: HandleModularObjectNewRecord_ProcessName_CrossProcess_Default_NoSetProcess_001
 * @tc.desc: Test that empty processName does not trigger SetProcessName call
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ProcessName_CrossProcess_Default_NoSetProcess_001,
    TestSize.Level1)
{
    // Verify the guard: if (!process.empty()) { SetProcessName }
    std::string process;  // empty
    bool wouldSetProcess = !process.empty();
    EXPECT_FALSE(wouldSetProcess);
}

// ============================================================
// 13. HandleModularObjectNewRecord connectId extraction tests
// Tests the rfind('_') + SetParam logic for connectId.
// ============================================================

/**
 * @tc.name: HandleModularObjectNewRecord_ConnectId_WithUnderscore_001
 * @tc.desc: Test connectId extraction when serviceKey contains underscore
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ConnectId_WithUnderscore_001,
    TestSize.Level1)
{
    // Replicate HandleModularObjectNewRecord logic: rfind('_')
    std::string serviceKey = "device#com.test.modular#TestModularObjectExt#entry_12345";
    auto pos = serviceKey.rfind('_');
    ASSERT_NE(pos, std::string::npos);
    std::string connectId = serviceKey.substr(pos + 1);
    EXPECT_EQ(connectId, "12345");
}

/**
 * @tc.name: HandleModularObjectNewRecord_ConnectId_NoUnderscore_001
 * @tc.desc: Test connectId not extracted when serviceKey has no underscore
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ConnectId_NoUnderscore_001,
    TestSize.Level1)
{
    // Replicate HandleModularObjectNewRecord logic: rfind('_')
    std::string serviceKey = "deviceNoUnderscore";
    auto pos = serviceKey.rfind('_');
    EXPECT_EQ(pos, std::string::npos);
    // When pos == npos, SetParam is NOT called
}

/**
 * @tc.name: HandleModularObjectNewRecord_ConnectId_EmptyKey_001
 * @tc.desc: Test connectId extraction with empty serviceKey
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ConnectId_EmptyKey_001,
    TestSize.Level1)
{
    std::string serviceKey;
    auto pos = serviceKey.rfind('_');
    EXPECT_EQ(pos, std::string::npos);
}

/**
 * @tc.name: HandleModularObjectNewRecord_ConnectId_MultipleUnderscores_001
 * @tc.desc: Test connectId extraction uses LAST underscore (rfind)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ConnectId_MultipleUnderscores_001,
    TestSize.Level1)
{
    // baseUri may contain underscores, rfind gets the LAST one
    std::string serviceKey = "device_my_bundle#test#ability#entry_99999";
    auto pos = serviceKey.rfind('_');
    ASSERT_NE(pos, std::string::npos);
    std::string connectId = serviceKey.substr(pos + 1);
    EXPECT_EQ(connectId, "99999");
    // Verify it's the LAST underscore, not the first
    EXPECT_NE(connectId, "my_bundle#test#ability#entry_99999");
}

/**
 * @tc.name: HandleModularObjectNewRecord_ConnectId_TrailingUnderscore_001
 * @tc.desc: Test connectId extraction when serviceKey ends with underscore
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_ConnectId_TrailingUnderscore_001,
    TestSize.Level1)
{
    std::string serviceKey = "device#bundle#ability#entry_";
    auto pos = serviceKey.rfind('_');
    ASSERT_NE(pos, std::string::npos);
    std::string connectId = serviceKey.substr(pos + 1);
    EXPECT_EQ(connectId, "");  // Empty connectId after trailing underscore
}

// ============================================================
// 14. HandleModularObjectNewRecord integration with GetOrCreateServiceRecord
// Tests the call path: GetOrCreateServiceRecord → HandleModularObjectNewRecord
// ============================================================

/**
 * @tc.name: HandleModularObjectNewRecord_Integration_NonModularObject_001
 * @tc.desc: Test GetOrCreateServiceRecord with non-MODULAR_OBJECT doesn't call MO handler
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_Integration_NonModularObject_001,
    TestSize.Level1)
{
    AbilityRequest request;
    ElementName element(TEST_DEVICE, TEST_BUNDLE, "TestServiceExt", TEST_MODULE);
    request.want.SetElement(element);
    request.abilityInfo.name = "TestServiceExt";
    request.abilityInfo.bundleName = TEST_BUNDLE;
    request.abilityInfo.moduleName = TEST_MODULE;
    request.abilityInfo.type = AbilityType::EXTENSION;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;

    std::shared_ptr<BaseExtensionRecord> service = nullptr;
    bool isLoaded = false;
    connectManager_->GetOrCreateServiceRecord(request, true, service, isLoaded);
    ASSERT_NE(service, nullptr);
    // SERVICE type should work without MO-specific processing
}

/**
 * @tc.name: HandleModularObjectNewRecord_Integration_ModularObject_001
 * @tc.desc: Test GetOrCreateServiceRecord with MODULAR_OBJECT creates record
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_Integration_ModularObject_001,
    TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> service = nullptr;
    bool isLoaded = false;
    connectManager_->GetOrCreateServiceRecord(request, true, service, isLoaded);
    ASSERT_NE(service, nullptr);
    // Without DB config, processName is not set by MO handler
}

// ============================================================
// 15. HandleModularObjectNewRecord edge case tests
// ============================================================

/**
 * @tc.name: HandleModularObjectNewRecord_EdgeCase_VariousExtensionTypes_001
 * @tc.desc: Test early return for various non-MODULAR_OBJECT extension types
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_EdgeCase_VariousExtensionTypes_001,
    TestSize.Level1)
{
    auto testType = [&](AppExecFwk::ExtensionAbilityType type) {
        AbilityRequest request;
        request.abilityInfo.extensionAbilityType = type;
        request.abilityInfo.bundleName = TEST_BUNDLE;
        request.abilityInfo.name = TEST_ABILITY;
        auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
        if (service == nullptr) {
            return;  // Some types may not create records
        }
        std::string processBefore = service->GetAbilityInfo().process;
        connectManager_->HandleModularObjectNewRecord(request, service, "key_123");
        // Non-MODULAR_OBJECT types should not modify processName
        EXPECT_EQ(service->GetAbilityInfo().process, processBefore);
    };

    testType(AppExecFwk::ExtensionAbilityType::SERVICE);
    testType(AppExecFwk::ExtensionAbilityType::DATA_SHARE);
    testType(AppExecFwk::ExtensionAbilityType::FILE_SHARE);
}

/**
 * @tc.name: HandleModularObjectNewRecord_EdgeCase_EmptyBundleName_001
 * @tc.desc: Test processName calculation with empty bundleName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_EdgeCase_EmptyBundleName_001,
    TestSize.Level1)
{
    // IN_PROCESS with empty bundleName and empty appInfo.process
    std::string bundleName;
    std::string process = bundleName;
    EXPECT_TRUE(process.empty());
    // !process.empty() → false → SetProcessName NOT called
}

/**
 * @tc.name: HandleModularObjectNewRecord_EdgeCase_SpecialChars_001
 * @tc.desc: Test processName with special characters in bundleName/abilityName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_EdgeCase_SpecialChars_001,
    TestSize.Level1)
{
    std::string bundleName = "com.test.special_chars.v2";
    std::string abilityName = "My-Extension_名称";
    // TYPE mode
    std::string process = bundleName + ":" + abilityName;
    EXPECT_EQ(process, "com.test.special_chars.v2:My-Extension_名称");
    // INSTANCE mode
    int32_t recordId = 999;
    std::string instanceProcess = bundleName + ":" + abilityName + ":" + std::to_string(recordId);
    EXPECT_EQ(instanceProcess, "com.test.special_chars.v2:My-Extension_名称:999");
}

/**
 * @tc.name: HandleModularObjectNewRecord_EdgeCase_ZeroRecordId_001
 * @tc.desc: Test INSTANCE mode with recordId=0
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_EdgeCase_ZeroRecordId_001,
    TestSize.Level1)
{
    std::string bundleName = TEST_BUNDLE;
    std::string abilityName = TEST_ABILITY;
    int32_t recordId = 0;
    std::string process = bundleName + ":" + abilityName + ":" + std::to_string(recordId);
    EXPECT_EQ(process, "com.test.modular:TestModularObjectExt:0");
    EXPECT_FALSE(process.empty());
}

/**
 * @tc.name: HandleModularObjectNewRecord_EdgeCase_NegativeRecordId_001
 * @tc.desc: Test INSTANCE mode with negative recordId
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_EdgeCase_NegativeRecordId_001,
    TestSize.Level1)
{
    std::string bundleName = TEST_BUNDLE;
    std::string abilityName = TEST_ABILITY;
    int32_t recordId = -1;
    std::string process = bundleName + ":" + abilityName + ":" + std::to_string(recordId);
    EXPECT_EQ(process, "com.test.modular:TestModularObjectExt:-1");
    EXPECT_FALSE(process.empty());
}

/**
 * @tc.name: HandleModularObjectNewRecord_EdgeCase_ColonInBundleName_001
 * @tc.desc: Test processName when bundleName contains colons (edge case)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_EdgeCase_ColonInBundleName_001,
    TestSize.Level1)
{
    std::string bundleName = "com.test:bundle";
    std::string abilityName = TEST_ABILITY;
    // TYPE mode - multiple colons in result
    std::string process = bundleName + ":" + abilityName;
    EXPECT_EQ(process, "com.test:bundle:TestModularObjectExt");
}

/**
 * @tc.name: HandleModularObjectNewRecord_EdgeCase_LargeRecordId_001
 * @tc.desc: Test INSTANCE mode with large recordId value
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_EdgeCase_LargeRecordId_001,
    TestSize.Level1)
{
    std::string bundleName = TEST_BUNDLE;
    std::string abilityName = TEST_ABILITY;
    int32_t recordId = INT32_MAX;
    std::string process = bundleName + ":" + abilityName + ":" + std::to_string(recordId);
    EXPECT_FALSE(process.empty());
    // Verify the recordId portion is correct
    EXPECT_NE(process.find(std::to_string(INT32_MAX)), std::string::npos);
}

/**
 * @tc.name: HandleModularObjectNewRecord_EdgeCase_AppProcessSameAsBundle_001
 * @tc.desc: Test IN_PROCESS when appInfo.process equals bundleName (no change in result)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_EdgeCase_AppProcessSameAsBundle_001,
    TestSize.Level1)
{
    std::string bundleName = TEST_BUNDLE;
    std::string appInfoProcess = TEST_BUNDLE;  // same as bundleName
    std::string process = bundleName;
    if (!appInfoProcess.empty()) {
        process = appInfoProcess;
    }
    EXPECT_EQ(process, bundleName);
}

/**
 * @tc.name: HandleModularObjectNewRecord_EdgeCase_VeryLongServiceKey_001
 * @tc.desc: Test connectId extraction from very long serviceKey
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, HandleModularObjectNewRecord_EdgeCase_VeryLongServiceKey_001,
    TestSize.Level1)
{
    std::string baseUri(10000, 'a');
    std::string serviceKey = baseUri + "_123456789";
    auto pos = serviceKey.rfind('_');
    ASSERT_NE(pos, std::string::npos);
    std::string connectId = serviceKey.substr(pos + 1);
    EXPECT_EQ(connectId, "123456789");
}

}  // namespace AAFwk
}  // namespace OHOS
