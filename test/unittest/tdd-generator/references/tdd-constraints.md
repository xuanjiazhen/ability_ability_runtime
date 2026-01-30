# TDD Unit Test Constraints and Coding Standards

This file contains strict constraints and coding standards that MUST be followed when generating OpenHarmony C++ unit tests.

## Table of Contents

1. [Naming Conventions](#naming-conventions)
2. [Test Structure](#test-structure)
3. [Assertions](#assertions)
4. [Mock Usage](#mock-usage)
5. [Code Style](#code-style)
6. [Forbidden Patterns](#forbidden-patterns)

---

## Naming Conventions

### Test File Names

```
{component_name}_test.cpp
```

For large test suites, split with suffixes:
```
{component_name}_first_test.cpp
{component_name}_second_test.cpp
{component_name}_third_test.cpp
```

Examples:
- `ability_connect_manager_test.cpp`
- `ability_manager_service_first_test.cpp`
- `ability_extension_test.cpp`

### Test Directory Names

```
{component_name}_test/
```

Examples:
- `ability_connect_manager_test/`
- `ability_manager_service_first_test/`
- `ability_keep_alive_test/`

### Test Case Names

```cpp
/**
 * @tc.name: MethodName_SerialNumber
 * @tc.desc: Description of what the test does
 * @tc.type: FUNC
 * @tc.require: issueNumber (optional)
 */
HWTEST_F(YourTest, MethodName_0100, TestSize.Level1)
```

**Format**: `{MethodName}_{4位序号}`

Examples:
- `RegisterAbilityDebugResponse_0100` - Normal call
- `OnAbilitysDebugStarted_0100` - Normal call
- `ConnectService_Null_0200` - Null parameter test

### Test Fixture Class Names

```cpp
class YourComponentTest : public testing::Test {
    // ...
};
```

**Format**: `{ComponentName}Test`

Examples:
- `AbilityConnectManagerTest`
- `AbilityDebugDealTest`
- `AbilityExtensionTest`

---

## Test Structure

### Standard Test Fixture

```cpp
class YourTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    // Helper methods
    AbilityRequest GenerateAbilityRequest(...);
    std::shared_ptr<Record> InitAbilityRecord();

protected:
    // Test data members
    AbilityRequest abilityRequest_;
    std::shared_ptr<YourClass> testObject_;

private:
    // Internal state
    std::shared_ptr<Dependency> dependency_;
};
```

### Setup/Teardown Implementation

```cpp
void YourTest::SetUpTestCase()
{
    // Called once before all tests
}

void YourTest::TearDownTestCase()
{
    // Called once after all tests
}

void YourTest::SetUp()
{
    // Called before each test
    testObject_ = std::make_shared<YourClass>();
}

void YourTest::TearDown()
{
    // Called after each test
    testObject_.reset();
}
```

### Test Case Structure

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

    // Arrange - Prepare test data
    auto param = CreateTestData();

    // Act - Execute method under test
    auto result = testObject_->MethodName(param);

    // Assert - Verify results
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->GetValue(), expectedValue);

    TAG_LOGI(AAFwkTag::TEST, "MethodName_0100 end");
}
```

---

## Assertions

### EXPECT_* vs ASSERT_*

| Macro | Behavior |
|-------|----------|
| `EXPECT_*` | Failure continues test |
| `ASSERT_*` | Failure aborts test immediately |

### Common Assertions

```cpp
// Equality
EXPECT_EQ(expected, actual);
EXPECT_NE(val1, val2);

// Boolean
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Pointers
EXPECT_NE(ptr, nullptr);
EXPECT_EQ(ptr, nullptr);

// Floating point
EXPECT_FLOAT_EQ(expected, actual);
EXPECT_NEAR(val1, val2, epsilon);

// Exceptions (if used)
EXPECT_THROW(statement, exception_type);
```

### Assertion Guidelines

```cpp
// CORRECT - Use EXPECT for non-critical checks
EXPECT_NE(result, nullptr);
EXPECT_EQ(result->status, SUCCESS);

// Use ASSERT for critical preconditions
ASSERT_NE(testObject_, nullptr);
ASSERT_TRUE(initialized_);

// Use specific assertions
EXPECT_EQ(count, 1);           // Not: EXPECT_TRUE(count == 1)
EXPECT_NE(ptr, nullptr);       // Not: EXPECT_TRUE(ptr != nullptr)
```

---

## Mock Usage

### GMock Basics

```cpp
#include <gmock/gmock.h>

using testing::Return;
using testing::_;
using testing::Invoke;
using testing::SetArgReferee;
```

### Mock Method Expectations

```cpp
// Simple return
EXPECT_CALL(*mockObject, methodName(_))
    .Times(1)
    .WillOnce(Return(expectedValue));

// Multiple calls
EXPECT_CALL(*mockObject, methodName(_))
    .Times(3)
    .WillRepeatedly(Return(value));

// Specific parameter
EXPECT_CALL(*mockObject, methodName(expectedParam))
    .WillOnce(Return(result));

// Any parameter
EXPECT_CALL(*mockObject, methodName(_))
    .WillOnce(Return(result));

// Side effect
EXPECT_CALL(*mockObject, methodName(_))
    .WillOnce(DoAll(
        SetArgReferee<0>(newValue),
        Return(true)
    ));
```

### Mock Class Definition

```cpp
class MockAbilityConnectCallback : public IAbilityConnection {
public:
    MOCK_METHOD(void, OnAbilityConnectDone,
        (const ElementName &element, const sptr<IRemoteObject> &remoteObject,
         int32_t resultCode), (override));
    MOCK_METHOD(void, OnAbilityDisconnectDone,
        (const ElementName &element, int32_t resultCode), (override));
};
```

### ON_CALL vs EXPECT_CALL

```cpp
// ON_CALL - Default behavior when not specifically expected
ON_CALL(*mockObject, methodName(_))
    .WillByDefault(Return(defaultValue));

// EXPECT_CALL - Must be called with specified behavior
EXPECT_CALL(*mockObject, methodName(_))
    .WillOnce(Return(expectedValue));
```

---

## Code Style

### Copyright Header

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
```

### Include Order

```cpp
// 1. GTest/GMock
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// 2. Private/protected access macros
#define private public
#define protected public

// 3. Class under test
#include "your_class.h"

// 4. Undefine access macros
#undef private
#undef protected

// 5. Other headers (alphabetical)
#include "ability_config.h"
#include "ability_manager_errors.h"
#include "mock_abilities.h"
```

### Using Declarations

```cpp
using namespace testing::ext;
using namespace OHOS::AAFwk;
using testing::_;
using testing::Invoke;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;
```

### Constants and Magic Numbers

```cpp
// CORRECT - Use named constants
static constexpr int TEST_WAIT_TIME = 1000000;
static constexpr int MAX_RETRY_COUNT = 1000;

// AVOID - Magic numbers
usleep(1000000);  // What is this?
```

---

## Forbidden Patterns

### Missing Access Macros

```cpp
// FORBIDDEN - Can't access private members
#include "your_class.h"

// REQUIRED - Enable access
#define private public
#define protected public
#include "your_class.h"
#undef private
#undef protected
```

### Incomplete Test Fixture

```cpp
// FORBIDDEN - Missing setup/teardown
class YourTest : public testing::Test {
    // No SetUp/TearDown
};

// REQUIRED - Complete fixture
class YourTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};
```

### Missing Test Documentation

```cpp
// FORBIDDEN - No documentation
HWTEST_F(YourTest, MethodName_0100, TestSize.Level1)
{
}

// REQUIRED - Documented test
/**
 * @tc.name: MethodName_0100
 * @tc.desc: Test normal call scenario for MethodName
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, MethodName_0100, TestSize.Level1)
{
}
```

### Test Case Naming Violations

```cpp
// FORBIDDEN - Incorrect naming
HWTEST_F(YourTest, test_method_name, TestSize.Level1)  // snake_case
HWTEST_F(YourTest, MethodNameTest, TestSize.Level1)    // No number
HWTEST_F(YourTest, MethodName_99, TestSize.Level1)     // Wrong number format

// REQUIRED - Correct naming
HWTEST_F(YourTest, MethodName_0100, TestSize.Level1)   // Correct
```

---

## Test Framework Requirements

### GoogleTest Imports

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <thread>
#include <chrono>
```

### Test Macros

```cpp
// Standard test case
HWTEST_F(YourTest, MethodName_0100, TestSize.Level1)
{
    // test code
}

// With type parameter
TYPED_TEST_SUITE(YourTest, Types);
TYPED_TEST(YourTest, MethodName_0100)
{
    // test code
}
```

### Test Sizes

- `TestSize.Level0` - Small tests (Smoke Test)
- `TestSize.Level1` - Basic functionality tests
- `TestSize.Level2` - Complete functionality tests
- `TestSize.Level3` - Edge cases and boundary tests
- `TestSize.Level4` - Stress and performance tests

---

## Common Helper Functions

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
```

### Generate Ability Request

```cpp
AbilityRequest GenerateAbilityRequest(const std::string& deviceName,
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

    return abilityRequest;
}
```

---

## BUILD.gn Configuration

### Basic Template

```gni
ohos_unittest("test_name") {
  module_out_path = "ability_runtime/ability_runtime/abilitymgr"
  sanitize = {
    cfi = true
    cfi_cross_dso = true
    debug = false
    blocklist = "../../cfi_blocklist.txt"
  }
  branch_protector_ret = "pac_ret"

  include_dirs = [
    "${ability_runtime_test_path}/mock/services_abilitymgr_test/...",
  ]

  sources = [
    "test_file.cpp",
  ]

  configs = [
    "${ability_runtime_services_path}/abilitymgr:abilityms_config",
  ]

  deps = [
    "${ability_runtime_services_path}/abilitymgr:abilityms",
  ]

  external_deps = [
    "googletest:gmock_main",
    "googletest:gtest_main",
    "hilog:libhilog",
    "ipc:ipc_core",
  ]
}

group("unittest") {
  testonly = true
  deps = [ ":test_name" ]
}
```

### Common Dependencies

```gni
external_deps = [
  "ability_base:want",
  "access_token:libaccesstoken_sdk",
  "c_utils:utils",
  "ffrt:libffrt",
  "googletest:gmock_main",
  "googletest:gtest_main",
  "hilog:libhilog",
  "hisysevent:libhisysevent",
  "ipc:ipc_core",
  "safwk:system_ability_fwk",
  "samgr:samgr_proxy",
]
```
