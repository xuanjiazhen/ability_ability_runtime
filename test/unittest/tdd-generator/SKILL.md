---
name: tdd-generator
description: Generate OpenHarmony C++ unit test cases using GoogleTest/GMock framework for Ability Runtime components. Use when generating or modifying unit test files (.cpp) for OpenHarmony Ability Runtime, including: (1) Creating new test files from class definitions, (2) Adding test cases for new methods, (3) Generating tests for connection/extension lifecycle, (4) Creating mock implementations, (5) Writing async/task handler tests. This skill enforces strict test patterns (private/protected access), proper setup/teardown, and GN build configuration.
---

# TDD Unit Test Generator

Generate OpenHarmony C++ unit tests following strict coding standards and test patterns.

## Quick Reference

When generating TDD unit tests, consult these references:

- **[Test Patterns & Templates](references/tdd-patterns.md)** - File structure, test scenarios, code templates
- **[Constraints & Standards](references/tdd-constraints.md)** - Naming conventions, GMock patterns, forbidden patterns

## Test File Generation Process

### 1. Parse Class Definition

From C++ header files (.h), extract:
- Class name and namespace
- Public/private/protected methods
- Member variables
- Dependencies and collaborators
- Return types and parameters

### 2. Generate Test Cases

For each class method, generate test cases following these scenarios:

| Scenario | Number | Description |
|----------|--------|-------------|
| Normal | `0100` | Required - Normal/expected call |
| Null/Empty | `0200` | Exception - null/empty parameter |
| Boundary | `0300` | Boundary conditions |
| Async | `0400` | Async/task handler tests |
| Error | `0500` | Error handling tests |

### 3. Test Case Number Format

```
{MethodName}_{场景}_{序号}
```

Examples:
- `RegisterAbilityDebugResponse_0100` - Normal call
- `OnAbilitysDebugStarted_Null_0200` - Null parameter
- `ConnectService_0100` - Service connection test

## Critical Constraints

### Private/Protected Access

```cpp
// REQUIRED - Access private members for testing
#define private public
#define protected public
#include "your_class.h"
#undef private
#undef protected
```

### Test Fixture Structure

```cpp
class YourTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    // Helper methods
    // Test member variables
private:
    // Internal state
};
```

### GMock Usage

```cpp
// CORRECT - Use GMock for dependencies
using testing::Return;
using testing::_;
EXPECT_CALL(*mockObject, methodName(_))
    .Times(1)
    .WillOnce(Return(expectedValue));

// FORBIDDEN - Direct implementation without mock
```

## Common Test Template

```cpp
/**
 * @tc.name: MethodName_0100
 * @tc.desc: Test normal call scenario for MethodName
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, MethodName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MethodName_0100 start");

    // Arrange
    auto result = objectUnderTest->MethodName(param);

    // Assert
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(result->GetValue(), expectedValue);

    TAG_LOGI(AAFwkTag::TEST, "MethodName_0100 end");
}
```

## When to Use References

### Read tdd-patterns.md for:
- Complete file structure templates
- Test fixture implementations
- Common test helper functions (WaitUntilTaskCalled, GenerateAbilityRequest)
- Specific test scenario patterns (0100, 0200, 0300)
- Async/task handler patterns
- Mock object setup

### Read tdd-constraints.md for:
- Naming conventions for test files and cases
- GMock best practices
- Code style requirements
- Test framework requirements (GoogleTest/GMock)
- Common assertion patterns
- BUILD.gn configuration

## Input Types

### From C++ Header Files
- Class definitions with methods
- Member variable declarations
- Constructor/destructor signatures
- Friend classes and inheritance

### Test Requirements
- Component name (e.g., AbilityConnectManager)
- Test scenarios to cover
- Mock dependencies needed

## Output

Generated `.cpp` test file with:
- Proper copyright header
- GoogleTest/GMock imports
- Complete test fixture class
- Numbered test cases for each scenario
- Private/protected member access
- Proper setup/teardown
- Helper functions for common operations

Generated `BUILD.gn` with:
- ohos_unittest template
- Source file references
- Dependency declarations
- External dependencies (gtest, gmock)
