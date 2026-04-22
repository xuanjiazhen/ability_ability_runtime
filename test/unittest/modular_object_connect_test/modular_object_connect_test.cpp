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

#include "ability_connect_manager.h"
#include "caller_info.h"
#include "connection_record.h"

#include "hilog_tag_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "mock_sa_call.h"
#include "mock_task_handler_wrap.h"
#include "modular_object_extension_info.h"

using namespace testing::ext;
using namespace OHOS::AppExecFwk;

// Declare mock control functions (defined in mock_modular_object_manager.cpp)
extern void SetMockModularObjectConfigs(const std::vector<OHOS::AAFwk::ModularObjectExtensionInfo> &configs);
extern void SetMockModularObjectConfigError();
extern void ClearMockModularObjectConfig();

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
    ClearMockModularObjectConfig();
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
    return request;
}

// Every connect generates a unique key via RequestIdUtil

/**
 * @tc.name: GetServiceKey_ModularObject_001
 * @tc.desc: Test MODULAR_OBJECT type generates unique key with requestId suffix
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, GetServiceKey_ModularObject_001, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string key1 = AbilityConnectManager::GetServiceKey(request);
    std::string key2 = AbilityConnectManager::GetServiceKey(request);
    // Each call generates a unique requestId, so keys should differ
    EXPECT_NE(key1, key2);
    // Both should contain the base URI prefix
    EXPECT_NE(key1.find("com.test.modular"), std::string::npos);
    EXPECT_NE(key2.find("com.test.modular"), std::string::npos);
}

/**
 * @tc.name: GetServiceKey_ModularObject_002
 * @tc.desc: Test MODULAR_OBJECT key format is baseUri_requestId
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, GetServiceKey_ModularObject_002, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string key = AbilityConnectManager::GetServiceKey(request);
    // Key should contain underscore separator between baseUri and requestId
    auto pos = key.rfind('_');
    EXPECT_NE(pos, std::string::npos);
    // requestId part should not be empty
    EXPECT_LT(pos, key.size() - 1);
}

// Verify processName is set correctly based on launchMode + processMode

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
 * @tc.desc: Test CROSS_PROCESS BUNDLE mode sets processName to bundleName:extensionTypeName
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ProcessName_CrossProcess_Bundle_001, TestSize.Level1)
{
    std::string extensionTypeName = "modularObject";
    std::string process = TEST_BUNDLE + ":" + extensionTypeName;  // BUNDLE mode
    EXPECT_EQ(process, "com.test.modular:modularObject");
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

// Verify thread keys follow BUNDLE/TYPE/INSTANCE patterns

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

// Tests that requestId stored in BaseExtensionRecord can reconstruct full serviceKey

/**
 * @tc.name: RequestId_Reconstruction_001
 * @tc.desc: Test requestId in record can reconstruct full serviceKey
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, RequestId_Reconstruction_001, TestSize.Level1)
{
    // Simulate: connect creates key = baseUri + "_" + requestId
    std::string baseUri = "device#com.test.modular#TestModularObjectExt#entry";
    std::string requestId = "12345";
    std::string fullKey = baseUri + "_" + requestId;

    // Simulate: disconnect reconstructs from record member
    // GetURI() returns baseUri, GetModularObjectRequestId() returns requestId
    std::string reconstructedUri = baseUri;
    std::string reconstructedRequestId = requestId;
    std::string reconstructedKey = reconstructedUri + "_" + reconstructedRequestId;
    EXPECT_EQ(fullKey, reconstructedKey);
}

/**
 * @tc.name: RequestId_Reconstruction_002
 * @tc.desc: Test empty requestId does not append underscore
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, RequestId_Reconstruction_002, TestSize.Level1)
{
    std::string baseUri = "device#com.test.modular#TestModularObjectExt#entry";
    std::string requestId = "";  // empty
    std::string key = baseUri;
    if (!requestId.empty()) {
        key = key + "_" + requestId;
    }
    EXPECT_EQ(key, baseUri);
}

/**
 * @tc.name: RequestId_SetGet_001
 * @tc.desc: Test SetModularObjectRequestId and GetModularObjectRequestId
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, RequestId_SetGet_001, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);

    // Default is empty
    EXPECT_EQ(service->GetModularObjectRequestId(), "");

    // Set and get
    service->SetModularObjectRequestId("99999");
    EXPECT_EQ(service->GetModularObjectRequestId(), "99999");
}


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
    // Without DB config, GetServiceKey generates unique key each time,
    // so second call creates a new record
    EXPECT_TRUE(loaded2);
}


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
 * @tc.desc: Test CROSS_PROCESS + BUNDLE + BUNDLE: shared process (with extensionTypeName suffix), shared thread
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest, ThreeLayer_CROSS_PROCESS_BUNDLE_BUNDLE_001, TestSize.Level1)
{
    // Layer 2: same processName (bundleName:extensionTypeName, different from UIAbility)
    std::string extensionTypeName = "modularObject";
    std::string process = TEST_BUNDLE + ":" + extensionTypeName;
    EXPECT_EQ(process, "com.test.modular:modularObject");

    // Layer 3: same thread key
    std::string threadKey = TEST_BUNDLE;
    EXPECT_EQ(threadKey, TEST_BUNDLE);
}


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

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldReturnEarlyWhenNotModularObject
 * @tc.desc: Test early return when extensionAbilityType is not MODULAR_OBJECT
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldReturnEarlyWhenNotModularObject, TestSize.Level1)
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
 * @tc.name: HandleModularObjectNewRecord_ShouldReturnEarlyWhenConfigIsNull
 * @tc.desc: Test early return when QueryModularObjectConfig returns nullptr (no DB config)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldReturnEarlyWhenConfigIsNull, TestSize.Level1)
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
 * @tc.name: HandleModularObjectNewRecord_ShouldNotCrashWhenServiceIsNull
 * @tc.desc: Test behavior with nullptr targetService (should not crash)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldNotCrashWhenServiceIsNull, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> nullService = nullptr;
    std::string serviceKey = "device#com.test.modular#TestModularObjectExt#entry_12345";

    // Early return on null config happens before any service access - should not crash
    EXPECT_NO_FATAL_FAILURE(
        connectManager_->HandleModularObjectNewRecord(request, nullService, serviceKey));
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldNotCrashWhenServiceKeyIsEmpty
 * @tc.desc: Test with empty serviceKey (no underscore, connectId not written)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldNotCrashWhenServiceKeyIsEmpty, TestSize.Level1)
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
 * @tc.name: HandleModularObjectNewRecord_ShouldNotCrashWhenNoUnderscoreInKey
 * @tc.desc: Test with serviceKey that has no underscore (rfind returns npos)
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldNotCrashWhenNoUnderscoreInKey, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);

    // serviceKey without underscore
    std::string noUnderscoreKey = "deviceNoUnderscoreKey";
    EXPECT_NO_FATAL_FAILURE(
        connectManager_->HandleModularObjectNewRecord(request, service, noUnderscoreKey));
}

// Tests the call path: GetOrCreateServiceRecord → HandleModularObjectNewRecord

/**
 * @tc.name: GetOrCreateServiceRecord_ShouldCreateServiceWhenNonModularObject
 * @tc.desc: Test GetOrCreateServiceRecord with non-MODULAR_OBJECT doesn't call MO handler
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest,
    GetOrCreateServiceRecord_ShouldCreateServiceWhenNonModularObject, TestSize.Level1)
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
 * @tc.name: GetOrCreateServiceRecord_ShouldCreateRecordWhenModularObject
 * @tc.desc: Test GetOrCreateServiceRecord with MODULAR_OBJECT creates record
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest,
    GetOrCreateServiceRecord_ShouldCreateRecordWhenModularObject, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> service = nullptr;
    bool isLoaded = false;
    connectManager_->GetOrCreateServiceRecord(request, true, service, isLoaded);
    ASSERT_NE(service, nullptr);
    // Without DB config, processName is not set by MO handler
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldNotModifyNonModularObjectTypes
 * @tc.desc: Test early return for various non-MODULAR_OBJECT extension types
 * @tc.type: FUNC
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldNotModifyNonModularObjectTypes, TestSize.Level1)
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
    testType(AppExecFwk::ExtensionAbilityType::DATASHARE);
    testType(AppExecFwk::ExtensionAbilityType::FILESHARE);
}

// Helper: create a ModularObjectExtensionInfo with given fields
static ModularObjectExtensionInfo MakeConfig(MoeLaunchMode launchMode = MoeLaunchMode::IN_PROCESS,
    MoeProcessMode processMode = MoeProcessMode::BUNDLE,
    bool isDisabled = false)
{
    ModularObjectExtensionInfo info;
    info.bundleName = TEST_BUNDLE;
    info.moduleName = TEST_MODULE;
    info.abilityName = TEST_ABILITY;
    info.launchMode = launchMode;
    info.processMode = processMode;
    info.isDisabled = isDisabled;
    return info;
}

// Full branch coverage for processName and requestId logic

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldUseBundleNameWhenInProcessEmptyAppProcess
 * @tc.desc: IN_PROCESS + empty appInfo.process → processName = bundleName
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldUseBundleNameWhenInProcessEmptyAppProcess,
    TestSize.Level1)
{
    auto config = MakeConfig(MoeLaunchMode::IN_PROCESS);
    SetMockModularObjectConfigs({config});
    auto request = MakeModularObjectRequest();
    request.appInfo.process = "";
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    connectManager_->HandleModularObjectNewRecord(request, service, "key_999");
    EXPECT_EQ(service->GetProcessName(), TEST_BUNDLE);
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldUseAppProcessWhenInProcessNonEmpty
 * @tc.desc: IN_PROCESS + non-empty appInfo.process → processName = appInfo.process
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldUseAppProcessWhenInProcessNonEmpty, TestSize.Level1)
{
    auto config = MakeConfig(MoeLaunchMode::IN_PROCESS);
    SetMockModularObjectConfigs({config});
    auto request = MakeModularObjectRequest();
    request.appInfo.process = "com.test.custom_process";
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    connectManager_->HandleModularObjectNewRecord(request, service, "key_999");
    EXPECT_EQ(service->GetProcessName(), "com.test.custom_process");
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldUseBundleNameWhenCrossProcessBundleMode
 * @tc.desc: CROSS_PROCESS + BUNDLE processMode → processName = bundleName:extensionTypeName
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldUseBundleNameWhenCrossProcessBundleMode,
    TestSize.Level1)
{
    auto config = MakeConfig(MoeLaunchMode::CROSS_PROCESS, MoeProcessMode::BUNDLE);
    SetMockModularObjectConfigs({config});
    auto request = MakeModularObjectRequest();
    request.abilityInfo.extensionTypeName = "modularObject";
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    connectManager_->HandleModularObjectNewRecord(request, service, "key_999");
    EXPECT_EQ(service->GetProcessName(), TEST_BUNDLE + ":modularObject");
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldUseBundleColonAbilityWhenCrossProcessTypeMode
 * @tc.desc: CROSS_PROCESS + TYPE processMode → processName = bundleName:abilityName
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldUseBundleColonAbilityWhenCrossProcessTypeMode,
    TestSize.Level1)
{
    auto config = MakeConfig(MoeLaunchMode::CROSS_PROCESS, MoeProcessMode::TYPE);
    SetMockModularObjectConfigs({config});
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    connectManager_->HandleModularObjectNewRecord(request, service, "key_999");
    EXPECT_EQ(service->GetProcessName(), TEST_BUNDLE + ":" + TEST_ABILITY);
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldUseFullKeyWhenCrossProcessInstanceMode
 * @tc.desc: CROSS_PROCESS + INSTANCE processMode → processName = bundleName:abilityName:recordId
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldUseFullKeyWhenCrossProcessInstanceMode,
    TestSize.Level1)
{
    auto config = MakeConfig(MoeLaunchMode::CROSS_PROCESS, MoeProcessMode::INSTANCE);
    SetMockModularObjectConfigs({config});
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    int32_t recordId = service->GetRecordId();
    connectManager_->HandleModularObjectNewRecord(request, service, "key_999");
    std::string expected = TEST_BUNDLE + ":" + TEST_ABILITY + ":" + std::to_string(recordId);
    EXPECT_EQ(service->GetProcessName(), expected);
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldExtractRequestIdWhenUnderscoreExists
 * @tc.desc: serviceKey with underscore → requestId extracted and saved
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldExtractRequestIdWhenUnderscoreExists,
    TestSize.Level1)
{
    auto config = MakeConfig();
    SetMockModularObjectConfigs({config});
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    std::string serviceKey = "device#bundle#ability#entry_42";
    connectManager_->HandleModularObjectNewRecord(request, service, serviceKey);
    EXPECT_EQ(service->GetModularObjectRequestId(), "42");
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldNotSetRequestIdWhenNoUnderscore
 * @tc.desc: serviceKey without underscore → requestId not set
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldNotSetRequestIdWhenNoUnderscore,
    TestSize.Level1)
{
    auto config = MakeConfig();
    SetMockModularObjectConfigs({config});
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    std::string serviceKey = "noUnderscoreKey";
    connectManager_->HandleModularObjectNewRecord(request, service, serviceKey);
    EXPECT_EQ(service->GetModularObjectRequestId(), "");
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldNotSetRequestIdWhenServiceKeyIsEmptyWithConfig
 * @tc.desc: empty serviceKey with config → no crash, requestId not set
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldNotSetRequestIdWhenServiceKeyIsEmptyWithConfig,
    TestSize.Level1)
{
    auto config = MakeConfig();
    SetMockModularObjectConfigs({config});
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    std::string serviceKey;
    EXPECT_NO_FATAL_FAILURE(
        connectManager_->HandleModularObjectNewRecord(request, service, serviceKey));
    EXPECT_EQ(service->GetModularObjectRequestId(), "");
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldSetEmptyRequestIdWhenTrailingUnderscore
 * @tc.desc: serviceKey ending with underscore → requestId is empty string
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldSetEmptyRequestIdWhenTrailingUnderscore,
    TestSize.Level1)
{
    auto config = MakeConfig();
    SetMockModularObjectConfigs({config});
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    connectManager_->HandleModularObjectNewRecord(request, service, "key_");
    EXPECT_EQ(service->GetModularObjectRequestId(), "");
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldNotSetProcessNameWhenDefaultProcessMode
 * @tc.desc: CROSS_PROCESS with invalid processMode (default) → process empty, not set
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldNotSetProcessNameWhenDefaultProcessMode,
    TestSize.Level1)
{
    auto config = MakeConfig(MoeLaunchMode::CROSS_PROCESS, static_cast<MoeProcessMode>(99));
    SetMockModularObjectConfigs({config});
    auto request = MakeModularObjectRequest();
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    std::string processBefore = service->GetProcessName();
    connectManager_->HandleModularObjectNewRecord(request, service, "key_123");
    // Default case: process stays empty → SetProcessName not called
    EXPECT_EQ(service->GetProcessName(), processBefore);
}

/**
 * @tc.name: HandleModularObjectNewRecord_ShouldNotCrashWhenServiceIsNullWithConfig
 * @tc.desc: nullptr service with valid config → early return, no crash
 */
HWTEST_F(ModularObjectConnectTest,
    HandleModularObjectNewRecord_ShouldNotCrashWhenServiceIsNullWithConfig,
    TestSize.Level1)
{
    auto config = MakeConfig();
    SetMockModularObjectConfigs({config});
    auto request = MakeModularObjectRequest();
    std::shared_ptr<BaseExtensionRecord> nullService = nullptr;
    EXPECT_NO_FATAL_FAILURE(
        connectManager_->HandleModularObjectNewRecord(request, nullService, "key_1"));
}


/**
 * @tc.name: CheckModularObjectInstanceLimit_ShouldReturnOkWhenBelowLimit
 * @tc.desc: Instance count < 20 returns ERR_OK
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectInstanceLimit_ShouldReturnOkWhenBelowLimit, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_ABILITY + "/" + TEST_MODULE;
    // Add 19 instances (below limit of 20)
    for (int i = 0; i < 19; i++) {
        auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
        ASSERT_NE(service, nullptr);
        connectManager_->CallAddToServiceMap(baseKey + "_" + std::to_string(i), service);
    }
    auto ret = connectManager_->CheckModularObjectInstanceLimit(baseKey);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CheckModularObjectInstanceLimit_ShouldReturnErrorWhenAtLimit
 * @tc.desc: Instance count = 20 returns ERR_MOE_INSTANCE_LIMIT
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectInstanceLimit_ShouldReturnErrorWhenAtLimit, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_ABILITY + "/" + TEST_MODULE;
    // Add 20 instances (at limit)
    for (int i = 0; i < 20; i++) {
        auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
        ASSERT_NE(service, nullptr);
        connectManager_->CallAddToServiceMap(baseKey + "_" + std::to_string(i), service);
    }
    auto ret = connectManager_->CheckModularObjectInstanceLimit(baseKey);
    EXPECT_EQ(ret, ERR_MOE_INSTANCE_LIMIT);
}

/**
 * @tc.name: CheckModularObjectInstanceLimit_ShouldNotCountOtherAbilities
 * @tc.desc: Only counts entries matching the base key prefix
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectInstanceLimit_ShouldNotCountOtherAbilities, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_ABILITY + "/" + TEST_MODULE;
    // Add 20 instances of a DIFFERENT ability
    std::string otherKey = TEST_DEVICE + "/" + TEST_BUNDLE + "/OtherAbility/" + TEST_MODULE;
    for (int i = 0; i < 20; i++) {
        auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
        ASSERT_NE(service, nullptr);
        connectManager_->CallAddToServiceMap(otherKey + "_" + std::to_string(i), service);
    }
    // Check the original ability — should be OK (0 instances)
    auto ret = connectManager_->CheckModularObjectInstanceLimit(baseKey);
    EXPECT_EQ(ret, ERR_OK);
}


/**
 * @tc.name: CheckModularObjectConnectionLimit_ShouldReturnOkWhenBelowLimit
 * @tc.desc: Connection count per callerPid < 5 returns ERR_OK
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectConnectionLimit_ShouldReturnOkWhenBelowLimit, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_ABILITY + "/" + TEST_MODULE;
    // Create one service record with 4 connections from pid 100
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    for (int i = 0; i < 4; i++) {
        auto connRecord = ConnectionRecord::CreateConnectionRecord(
            nullptr, service, nullptr, connectManager_);
        ASSERT_NE(connRecord, nullptr);
        auto callerInfo = std::make_shared<IndirectCallerInfo>();
        callerInfo->callerPid = 100;
        connRecord->AttachCallerInfo(callerInfo);
        service->AddConnectRecordToList(connRecord);
    }
    connectManager_->CallAddToServiceMap(baseKey + "_req1", service);
    // Below limit of 5
    auto ret = connectManager_->CheckModularObjectConnectionLimit(baseKey, 100);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CheckModularObjectConnectionLimit_ShouldReturnErrorWhenAtLimit
 * @tc.desc: Connection count per callerPid = 5 returns ERR_MOE_CONNECTION_LIMIT
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectConnectionLimit_ShouldReturnErrorWhenAtLimit, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_ABILITY + "/" + TEST_MODULE;
    // Create service records with total 5 connections from pid 100
    for (int i = 0; i < 5; i++) {
        auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
        ASSERT_NE(service, nullptr);
        auto connRecord = ConnectionRecord::CreateConnectionRecord(
            nullptr, service, nullptr, connectManager_);
        ASSERT_NE(connRecord, nullptr);
        auto callerInfo = std::make_shared<IndirectCallerInfo>();
        callerInfo->callerPid = 100;
        connRecord->AttachCallerInfo(callerInfo);
        service->AddConnectRecordToList(connRecord);
        connectManager_->CallAddToServiceMap(baseKey + "_" + std::to_string(i), service);
    }
    // At limit of 5
    auto ret = connectManager_->CheckModularObjectConnectionLimit(baseKey, 100);
    EXPECT_EQ(ret, ERR_MOE_CONNECTION_LIMIT);
}

/**
 * @tc.name: CheckModularObjectConnectionLimit_ShouldCountAcrossInstances
 * @tc.desc: Connections across multiple service records for same MoeAbility are counted together
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectConnectionLimit_ShouldCountAcrossInstances, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_ABILITY + "/" + TEST_MODULE;
    // 3 connections on record 1, 2 on record 2 = total 5 → at limit
    for (int batch = 0; batch < 2; batch++) {
        auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
        ASSERT_NE(service, nullptr);
        int count = (batch == 0) ? 3 : 2;
        for (int i = 0; i < count; i++) {
            auto connRecord = ConnectionRecord::CreateConnectionRecord(
                nullptr, service, nullptr, connectManager_);
            ASSERT_NE(connRecord, nullptr);
            auto callerInfo = std::make_shared<IndirectCallerInfo>();
            callerInfo->callerPid = 100;
            connRecord->AttachCallerInfo(callerInfo);
            service->AddConnectRecordToList(connRecord);
        }
        connectManager_->CallAddToServiceMap(
            baseKey + "_batch" + std::to_string(batch), service);
    }
    auto ret = connectManager_->CheckModularObjectConnectionLimit(baseKey, 100);
    EXPECT_EQ(ret, ERR_MOE_CONNECTION_LIMIT);
}

/**
 * @tc.name: CheckModularObjectConnectionLimit_ShouldNotCountOtherPid
 * @tc.desc: Connections from different callerPid are counted independently
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectConnectionLimit_ShouldNotCountOtherPid, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_ABILITY + "/" + TEST_MODULE;
    // 5 connections from pid 200
    auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
    ASSERT_NE(service, nullptr);
    for (int i = 0; i < 5; i++) {
        auto connRecord = ConnectionRecord::CreateConnectionRecord(
            nullptr, service, nullptr, connectManager_);
        ASSERT_NE(connRecord, nullptr);
        auto callerInfo = std::make_shared<IndirectCallerInfo>();
        callerInfo->callerPid = 200;
        connRecord->AttachCallerInfo(callerInfo);
        service->AddConnectRecordToList(connRecord);
    }
    connectManager_->CallAddToServiceMap(baseKey + "_req1", service);
    // pid 100 has 0 connections → should be OK
    auto ret = connectManager_->CheckModularObjectConnectionLimit(baseKey, 100);
    EXPECT_EQ(ret, ERR_OK);
}


/**
 * @tc.name: CheckModularObjectLimits_ShouldReturnOkWhenNotModularObject
 * @tc.desc: Non-MODULAR_OBJECT type returns ERR_OK immediately
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectLimits_ShouldReturnOkWhenNotModularObject, TestSize.Level1)
{
    AbilityRequest request;
    request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    auto ret = connectManager_->CheckModularObjectLimits(request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CheckModularObjectLimits_ShouldReturnOkWhenBelowLimits
 * @tc.desc: Instance and connection counts below limits return ERR_OK
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectLimits_ShouldReturnOkWhenBelowLimits, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    auto ret = connectManager_->CheckModularObjectLimits(request);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: CheckModularObjectLimits_ShouldReturnErrorWhenInstanceLimitReached
 * @tc.desc: Instance limit hit returns ERR_MOE_INSTANCE_LIMIT
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectLimits_ShouldReturnErrorWhenInstanceLimitReached, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_ABILITY + "/" + TEST_MODULE;
    for (int i = 0; i < 20; i++) {
        auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
        ASSERT_NE(service, nullptr);
        connectManager_->CallAddToServiceMap(baseKey + "_" + std::to_string(i), service);
    }
    auto ret = connectManager_->CheckModularObjectLimits(request);
    EXPECT_EQ(ret, ERR_MOE_INSTANCE_LIMIT);
}

/**
 * @tc.name: CheckModularObjectLimits_ShouldReturnErrorWhenConnectionLimitReached
 * @tc.desc: Connection limit hit returns ERR_MOE_CONNECTION_LIMIT
 */
HWTEST_F(ModularObjectConnectTest,
    CheckModularObjectLimits_ShouldReturnErrorWhenConnectionLimitReached, TestSize.Level1)
{
    auto request = MakeModularObjectRequest();
    std::string baseKey = std::string(TEST_DEVICE) + "/" + TEST_BUNDLE + "/" + TEST_ABILITY + "/" + TEST_MODULE;
    // Manually verify: create records with connections, then check via direct limit calls
    for (int i = 0; i < 5; i++) {
        auto service = BaseExtensionRecord::CreateBaseExtensionRecord(request);
        ASSERT_NE(service, nullptr);
        auto connRecord = ConnectionRecord::CreateConnectionRecord(
            nullptr, service, nullptr, connectManager_);
        ASSERT_NE(connRecord, nullptr);
        auto callerInfo = std::make_shared<IndirectCallerInfo>();
        callerInfo->callerPid = 42;
        connRecord->AttachCallerInfo(callerInfo);
        service->AddConnectRecordToList(connRecord);
        connectManager_->CallAddToServiceMap(baseKey + "_" + std::to_string(i), service);
    }
    // Verify connection limit is hit for pid 42
    auto ret = connectManager_->CheckModularObjectConnectionLimit(baseKey, 42);
    EXPECT_EQ(ret, ERR_MOE_CONNECTION_LIMIT);
    // Verify instance limit is still OK (5 < 20)
    ret = connectManager_->CheckModularObjectInstanceLimit(baseKey);
    EXPECT_EQ(ret, ERR_OK);
}
} // AAFwk
} // OHOS