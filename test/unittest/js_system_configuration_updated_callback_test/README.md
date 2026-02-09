# JsSystemConfigurationUpdatedCallback TDD Unit Tests

## Overview

This directory contains unit tests for the `JsSystemConfigurationUpdatedCallback` class located at:
`frameworks/native/appkit/ability_runtime/context/js_system_configuration_updated_callback.cpp`

## Test Files

### 1. js_system_configuration_updated_callback_test.cpp
Main unit test file covering:
- Constructor and destructor tests
- Register/UnRegister callback tests
- IsEmpty and IsEqual method tests
- HasJsMethodExist tests
- All Notify* method tests (ColorMode, FontSizeScale, FontWeightScale, Language, FontId, MCC, MNC, Locale, HasPointerDevice)
- NotifySystemConfigurationUpdated tests
- CallJsMethodInnerCommon tests
- FreeNativeReference tests

### 2. js_system_configuration_callback_helper_test.cpp
Helper function utility tests covering:
- `ConvertToDouble` function tests (14 test cases)
- `IsValidValue` function tests (5 test cases)

### 3. Mock Files
- `mock_configuration.h`: Mock implementation of Configuration class
- `mock_native_reference.h`: Mock implementation of NativeReference class

## Test Coverage

### Class Methods Tested

| Method | Test Cases | Coverage |
|--------|-----------|----------|
| Constructor | 1 | Basic instantiation |
| Destructor | 1 | Cleanup verification |
| Register | 2 | Null env, null callback |
| UnRegister | 2 | Clear all, empty list |
| IsEmpty | 1 | Empty state check |
| IsEqual | 3 | Null env, null ref, null value |
| HasJsMethodExist | 2 | Null env, null callback |
| NotifyColorModeUpdated | 1 | Valid callback |
| NotifyFontSizeScaleUpdated | 2 | Valid value, empty string |
| NotifyFontWeightScaleUpdated | 1 | Valid value |
| NotifyLanguageUpdated | 1 | Valid language |
| NotifyFontIdUpdated | 1 | Valid font ID |
| NotifyMCCUpdated | 1 | Valid MCC |
| NotifyMNCUpdated | 1 | Valid MNC |
| NotifyLocaleUpdated | 1 | Valid locale |
| NotifyHasPointerDeviceUpdated | 2 | True, false values |
| NotifySystemConfigurationUpdated | 1 | Empty callback list |
| CallJsMethodInnerCommon | 1 | Null callback |
| FreeNativeReference | 1 | Null reference |

### Helper Functions Tested

| Function | Test Cases | Description |
|----------|-----------|-------------|
| ConvertToDouble | 14 | Valid numbers, edge cases, error conditions |
| IsValidValue | 5 | Null checks, error conditions, valid cases |

## Test Naming Convention

Tests follow the HarmonyOS/OpenHarmony naming convention:
```
{ClassName}_{MethodName}_{SequenceID}
```

Example:
- `JsSystemConfigurationUpdatedCallback_Constructor_0100`
- `JsSystemConfigurationUpdatedCallback_Register_0100`

## Building Tests

To build these tests, ensure you have:
1. GTest framework installed
2. Access to the project dependencies
3. Proper build configuration for the target platform

Build command (typical):
```bash
./build.sh --product-name <product> --build-target js_system_configuration_updated_callback_test
```

## Running Tests

Execute all tests:
```bash
./js_system_configuration_updated_callback_test
```

Execute specific test:
```bash
./js_system_configuration_updated_callback_test --gtest_filter=JsSystemConfigurationUpdatedCallbackTest.*
```

Execute helper tests:
```bash
./js_system_configuration_callback_helper_test
```

## Test Levels

- **Level1**: Basic functionality tests (smoke tests)
- **Level2**: Comprehensive tests
- **Level3**: Edge cases and stress tests

Current tests use **Level1** for basic functionality verification.

## Dependencies

- `gtest`: Google Test framework
- `configuration.h`: Configuration class definition
- `native_reference.h`: NativeReference class definition
- `js_system_configuration_updated_callback.h`: Target class header

## Future Enhancements

Potential areas for additional test coverage:
1. Integration tests with actual N-API environment
2. Multi-threaded callback registration tests
3. Performance tests for large callback lists
4. Memory leak detection tests
5. Async callback execution tests

## Notes

- Tests use `#define private public` to access private members for testing
- Mock objects are used to simulate N-API environment without actual JavaScript runtime
- Tests are designed to be independent and can run in any order
- No actual JavaScript execution occurs in these tests
