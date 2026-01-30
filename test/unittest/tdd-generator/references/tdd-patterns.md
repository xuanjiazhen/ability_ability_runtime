# TDD Unit Test Patterns Reference

This file contains common test patterns and templates for OpenHarmony C++ unit test generation.

## Table of Contents

1. [File Structure](#file-structure)
2. [Test Case Numbering](#test-case-numbering)
3. [Basic Test Template](#basic-test-template)
4. [Test Scenarios](#test-scenarios)
5. [Common Patterns](#common-patterns)

---

## File Structure

### Standard Test File Header

```cpp
/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <memory>

#define private public
#define protected public
#include "your_class.h"
#undef private
#undef protected

#include "ability_config.h"
#include "ability_manager_errors.h"
#include "mock_abilities.h"
#include "hilog_tag_wrapper.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;
using testing::_;
using testing::Invoke;
using testing::Return;
```

### Test Fixture Template

```cpp
class YourTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    // Helper methods
    std::shared_ptr<YourClass> GetTestObject() const;
    AbilityRequest GenerateAbilityRequest(const std::string& deviceName,
        const std::string& abilityName, const std::string& appName,
        const std::string& bundleName, const std::string& moduleName);

    static constexpr int TEST_WAIT_TIME = 1000000;

protected:
    // Test data members
    AbilityRequest abilityRequest_;
    std::shared_ptr<YourClass> testObject_;
    std::shared_ptr<MockDependency> mockDependency_;

private:
    // Internal state
};
```

---

## Test Case Numbering

### Format

```
{MethodName}_{场景}_{序号}
```

### Scenario Numbers

| Number | Scenario | Description |
|--------|----------|-------------|
| 0100 | Normal | Normal/expected call scenario (REQUIRED) |
| 0200-0299 | Exception | Null, empty, invalid values |
| 0300-0399 | Boundary | Boundary conditions |
| 0400-0499 | Async | Async/task handler tests |
| 0500-0599 | Error | Error handling tests |

### Examples

```
RegisterAbilityDebugResponse_0100    // Normal call
OnAbilitysDebugStarted_0100          // Normal call
ConnectService_Null_0200             // Null parameter
DisconnectService_Empty_0201         // Empty parameter
UpdateRecord_Boundary_0300           // Boundary test
AsyncOperation_0100                  // Async operation
```

---

## Basic Test Template

### JSDoc-Style Comments (Required)

```cpp
/**
 * @tc.name: MethodName_0100
 * @tc.desc: Test normal call scenario for MethodName
 * @tc.type: FUNC
 * @tc.require: issueI5TR35
 */
HWTEST_F(YourTest, MethodName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MethodName_0100 start");

    // Arrange
    auto param = CreateTestData();

    // Act
    auto result = testObject_->MethodName(param);

    // Assert
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->GetValue(), expectedValue);

    TAG_LOGI(AAFwkTag::TEST, "MethodName_0100 end");
}
```

### Alternative Documentation Style

```cpp
/*
 * Feature: Your Component
 * Function: MethodName
 * SubFunction: NA
 * FunctionPoints: MethodName
 * EnvConditions: NA
 * CaseDescription: Verify MethodName works correctly
 */
HWTEST_F(YourTest, MethodName_0100, TestSize.Level1)
{
    // test code
}
```

---

## Test Scenarios

### 0100 - Normal Call

```cpp
/**
 * @tc.name: RegisterAbilityDebugResponse_0100
 * @tc.desc: Verify register ability debug response calls normally.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityDebugDealTest, RegisterAbilityDebugResponse_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterAbilityDebugResponse_0100 start";

    EXPECT_EQ(deal_->abilityDebugResponse_, nullptr);
    deal_->RegisterAbilityDebugResponse();
    EXPECT_NE(deal_->abilityDebugResponse_, nullptr);

    GTEST_LOG_(INFO) << "RegisterAbilityDebugResponse_0100 end";
}
```

### 0200 - Null Parameter

```cpp
/**
 * @tc.name: ConnectService_Null_0200
 * @tc.desc: Verify ConnectService handles null parameter correctly.
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, ConnectService_Null_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectService_Null_0200 start");

    auto result = testObject_->ConnectService(nullptr);
    EXPECT_EQ(result, ERR_NULL_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ConnectService_Null_0200 end");
}
```

### 0300 - Boundary Test

```cpp
/**
 * @tc.name: UpdateRecord_Boundary_0300
 * @tc.desc: Verify UpdateRecord handles boundary values correctly.
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, UpdateRecord_Boundary_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateRecord_Boundary_0300 start");

    testObject_->SetValue(INT_MAX);
    EXPECT_EQ(testObject_->GetValue(), INT_MAX);

    testObject_->SetValue(INT_MIN);
    EXPECT_EQ(testObject_->GetValue(), INT_MIN);

    TAG_LOGI(AAFwkTag::TEST, "UpdateRecord_Boundary_0300 end");
}
```

### 0400 - Async/Task Handler Test

```cpp
/**
 * @tc.name: AsyncOperation_0100
 * @tc.desc: Verify async operation completes correctly.
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, AsyncOperation_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AsyncOperation_0100 start");

    std::atomic<bool> taskCalled(false);
    auto task = [&taskCalled]() {
        taskCalled.store(true);
    };

    EXPECT_TRUE(taskHandler_->SubmitTask(task));
    WaitUntilTaskCalled(task, taskHandler_, taskCalled);
    EXPECT_TRUE(taskCalled.load());

    TAG_LOGI(AAFwkTag::TEST, "AsyncOperation_0100 end");
}
```

---

## Common Patterns

### Ability Request Generation

```cpp
AbilityRequest YourTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName,
    const std::string& bundleName, const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;

    ApplicationInfo appinfo;
    appinfo.name = appName;
    abilityInfo.applicationInfo = appinfo;

    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;
    abilityInfo.process = bundleName;

    return abilityRequest;
}
```

### Wait Until Task Called

```cpp
template<typename F>
static void WaitUntilTaskCalled(const F& f,
    const std::shared_ptr<TaskHandlerWrap>& handler,
    std::atomic<bool>& taskCalled)
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    if (handler->SubmitTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

static void WaitUntilTaskDone(const std::shared_ptr<TaskHandlerWrap>& handler)
{
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    WaitUntilTaskCalled(f, handler, taskCalled);
}
```

### Mock Token Creation

```cpp
sptr<Token> YourTest::CreateMockToken(std::shared_ptr<AbilityRecord> record)
{
    return new Token(record);
}

// Usage
auto record = std::make_shared<AbilityRecord>(want, abilityInfo, appInfo);
sptr<Token> token = CreateMockToken(record);
```

### Connection Callback Mock

```cpp
class MockAbilityConnectCallback : public IAbilityConnection {
public:
    MOCK_METHOD(void, OnAbilityConnectDone,
        (const ElementName &element, const sptr<IRemoteObject> &remoteObject,
         int32_t resultCode), (override));
    MOCK_METHOD(void, OnAbilityDisconnectDone,
        (const ElementName &element, int32_t resultCode), (override));
};

// Usage
auto callback = std::make_shared<MockAbilityConnectCallback>();
EXPECT_CALL(*callback, OnAbilityConnectDone(_, _, _))
    .Times(1);
```

### Service Connection Pattern

```cpp
/**
 * @tc.name: ConnectService_0100
 * @tc.desc: Verify service connection works correctly.
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, ConnectService_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ConnectService_0100 start");

    // Arrange
    auto want = GenerateAbilityRequest("", "ServiceAbility", "TestApp",
        "com.test.bundle", "entry");
    auto callback = std::make_shared<MockAbilityConnectCallback>();

    EXPECT_CALL(*callback, OnAbilityConnectDone(_, _, ERR_OK))
        .Times(1);

    // Act
    auto result = connectManager_->ConnectAbility(
        want.want, callback, nullptr, -1);

    // Assert
    EXPECT_EQ(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ConnectService_0100 end");
}
```

### Extension Lifecycle Test

```cpp
/**
 * @tc.name: StartExtension_0100
 * @tc.desc: Verify extension starts correctly.
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, StartExtension_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartExtension_0100 start");

    // Arrange
    AbilityRequest abilityRequest = GenerateAbilityRequest(
        "", "TestExtension", "TestApp", "com.test.bundle", "entry");

    // Act
    auto result = extensionManager_->StartAbility(abilityRequest);

    // Assert
    EXPECT_EQ(result, ERR_OK);

    // Cleanup
    extensionManager_->TerminateAbility(abilityRequest.abilityInfo.code);

    TAG_LOGI(AAFwkTag::TEST, "StartExtension_0100 end");
}
```

### Mock Bundle Manager

```cpp
class MockBundleManager : public IBundleManager {
public:
    MOCK_METHOD(bool, QueryAbilityInfo, (const AbilityInfo &), (override));
    MOCK_METHOD(bool, GetApplicationInfo,
        (const std::string &, int, ApplicationInfo &), (override));
};

// Setup in SetUp()
void YourTest::SetUp() {
    mockBundleManager_ = std::make_shared<MockBundleManager>();

    ON_CALL(*mockBundleManager_, QueryAbilityInfo(_))
        .WillByDefault(Return(true));
}
```

### State Change Test

```cpp
/**
 * @tc.name: OnAbilityStateChanged_0100
 * @tc.desc: Verify ability state change callback works.
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, OnAbilityStateChanged_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnAbilityStateChanged_0100 start");

    // Arrange
    auto record = InitAbilityRecord();
    EXPECT_EQ(record->GetAbilityState(), AbilityState::INITIAL);

    // Act
    record->SetAbilityState(AbilityState::ACTIVE);

    // Assert
    EXPECT_EQ(record->GetAbilityState(), AbilityState::ACTIVE);

    TAG_LOGI(AAFwkTag::TEST, "OnAbilityStateChanged_0100 end");
}
```

### Multi-Step Scenario Test

```cpp
/**
 * @tc.name: FullLifecycle_0100
 * @tc.desc: Verify full ability lifecycle.
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, FullLifecycle_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FullLifecycle_0100 start");

    // Arrange
    auto want = GenerateAbilityRequest("", "TestAbility", "TestApp",
        "com.test.bundle", "entry");

    // Act - Start
    auto startResult = manager_->StartAbility(want);
    EXPECT_EQ(startResult, ERR_OK);

    // Act - Connect
    auto callback = std::make_shared<MockAbilityConnectCallback>();
    EXPECT_CALL(*callback, OnAbilityConnectDone(_, _, ERR_OK));
    auto connectResult = manager_->ConnectAbility(
        want.want, callback, nullptr, -1);
    EXPECT_EQ(connectResult, ERR_OK);

    // Act - Disconnect
    auto disconnectResult = manager_->DisconnectAbility(
        callback);
    EXPECT_EQ(disconnectResult, ERR_OK);

    // Cleanup
    manager_->TerminateAbility(want.abilityInfo.code);

    TAG_LOGI(AAFwkTag::TEST, "FullLifecycle_0100 end");
}
```

### Permission Verification Mock

```cpp
class MockPermissionVerification {
public:
    MOCK_METHOD(bool, VerifyAccountSwitchPermission, (), (const));
    MOCK_METHOD(bool, VerifyMissionPermission, (), (const));
};

// Usage
ON_CALL(*mockPermission_, VerifyAccountSwitchPermission())
    .WillByDefault(Return(true));
```

### Event Handler Test

```cpp
/**
 * @tc.name: HandleEvent_0100
 * @tc.desc: Verify event handler processes event correctly.
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, HandleEvent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleEvent_0100 start");

    // Arrange
    auto event = EventWrap(10, 101, true);
    EXPECT_TRUE(event.GetEventId() == 10);
    EXPECT_TRUE(event.GetParam() == 101);
    EXPECT_TRUE(event.IsExtension());

    // Act
    eventHandler_->ProcessEvent(event);

    // Assert
    EXPECT_TRUE(eventHandler_->GetLastProcessedEventId() == 10);

    TAG_LOGI(AAFwkTag::TEST, "HandleEvent_0100 end");
}
```

### Memory Leak Check Pattern

```cpp
/**
 * @tc.name: MemoryManagement_0100
 * @tc.desc: Verify proper memory management.
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, MemoryManagement_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MemoryManagement_0100 start");

    // Arrange
    auto* rawPtr = new YourClass();
    auto sharedPtr = std::shared_ptr<YourClass>(rawPtr);
    auto weakPtr = std::weak_ptr<YourClass>(sharedPtr);

    // Act
    EXPECT_EQ(weakPtr.use_count(), 1);
    sharedPtr.reset();

    // Assert
    EXPECT_EQ(weakPtr.use_count(), 0);
    EXPECT_TRUE(weakPtr.expired());

    TAG_LOGI(AAFwkTag::TEST, "MemoryManagement_0100 end");
}
```

### Thread Safety Test

```cpp
/**
 * @tc.name: ThreadSafety_0100
 * @tc.desc: Verify thread-safe operations.
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, ThreadSafety_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ThreadSafety_0100 start");

    // Arrange
    const int threadCount = 10;
    std::vector<std::thread> threads;
    std::atomic<int> counter(0);

    // Act
    for (int i = 0; i < threadCount; i++) {
        threads.emplace_back([this, &counter]() {
            testObject_->Increment();
            counter++;
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Assert
    EXPECT_EQ(counter.load(), threadCount);
    EXPECT_EQ(testObject_->GetCount(), threadCount);

    TAG_LOGI(AAFwkTag::TEST, "ThreadSafety_0100 end");
}
```

---

## Namespace and Using Declarations

```cpp
namespace OHOS {
namespace AAFwk {
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace {
    const int32_t SLEEP_TIME = 10000;
    const std::string STRING_PROCESS_NAME = "process_name";
}

class YourTest : public testing::Test {
    // ...
};

}  // namespace AAFwk
}  // namespace OHOS
```

---

## Common Test Macros

```cpp
// Sleep utility
#define SLEEP(ms) std::this_thread::sleep_for(std::chrono::milliseconds(ms))

// Log tags
#define TAG_LOGI(tag, ...) HILOG_INFO(tag, __VA_ARGS__)
#define TAG_LOGE(tag, ...) HILOG_ERROR(tag, __VA_ARGS__)

// Constants
static constexpr int TEST_WAIT_TIME = 1000000;
static constexpr int MAX_RETRY_COUNT = 1000;
```
