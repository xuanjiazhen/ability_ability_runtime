# Unit Tests for AbilityStageContext ElementName Feature

## Overview
This document describes the unit tests for the new ElementName functionality added to AbilityStageContext.

## Test Files

### 1. ability_stage_context_test.cpp
**Location:** `test/unittest/ability_stage_context_test/ability_stage_context_test.cpp`

**Test Cases:**
- `AbilityStageContext_SetElementName_001`: Test SetElementName and GetElementName with valid ElementName
- `AbilityStageContext_SetElementName_002`: Test with default (empty) ElementName
- `AbilityStageContext_SetElementName_003`: Test multiple updates to ElementName
- `AbilityStageContext_SetElementName_004`: Test that GetElementName returns const reference

**Coverage:**
- Basic setter/getter functionality
- Empty ElementName handling
- Multiple updates
- Return type verification

### 2. js_ability_stage_context_test.cpp (NEW)
**Location:** `test/unittest/js_ability_stage_context_test/js_ability_stage_context_test.cpp`

**Test Cases:**
- `JsAbilityStageContext_GetCurrentElementName_001`: Test GetCurrentElementName returns correct JS object when ElementName is set
- `JsAbilityStageContext_GetCurrentElementName_002`: Test GetCurrentElementName when ElementName is not set (empty)
- `JsAbilityStageContext_GetCurrentElementName_003`: Test GetCurrentElementName when context is null
- `JsAbilityStageContext_CreateJsAbilityStageContext_001`: Test that CreateJsAbilityStageContext properly binds GetCurrentElementName function
- `JsAbilityStageContext_ElementName_Values_001`: Test GetCurrentElementName with all ElementName fields populated

**Coverage:**
- NAPI function implementation
- JS object binding verification
- Null and empty value handling
- Complete ElementName field testing

### 3. ohos_application_test.cpp
**Location:** `test/unittest/appkit/ohos_application_test/ohos_application_test.cpp`

**Test Cases:**
- `AppExecFwk_OHOSApplicationTest_AddAbilityStage_ElementName_0100`: Verify that AddAbilityStage properly sets ElementName to AbilityStageContext
- `AppExecFwk_OHOSApplicationTest_AddAbilityStage_ElementName_0200`: Verify that AddAbilityStage handles null Want gracefully
- `AppExecFwk_OHOSApplicationTest_AddAbilityStage_ElementName_0300`: Verify that AddAbilityStage handles empty ElementName in Want

**Coverage:**
- ElementName setting in AddAbilityStage flow
- Null Want safety (prevents crash)
- Empty ElementName handling

## Build Configuration

### New Test Module: js_ability_stage_context_test
**BUILD.gn Location:** `test/unittest/js_ability_stage_context_test/BUILD.gn`

**Dependencies:**
- `ability_runtime_appkit_native`
- `ability_stage_context`
- `ohos_application`
- `ability_context_native`
- `ability_runtime_context_kit`
- `ability_stage_context_kit`
- `mock_ability_stage_context`
- `mock_ability_context`

### Mock Files
**Location:** `test/unittest/common/mock_ability_stage_context.h`

## Running the Tests

### Build all tests:
```bash
./build.sh --product-name <product> --build-target ability_runtime_test
```

### Build specific test:
```bash
# AbilityStageContext tests
./build.sh --product-name <product> --build-target ability_stage_context_test

# NAPI tests
./build.sh --product-name <product> --build-target js_ability_stage_context_test

# OHOSApplication tests
./build.sh --product-name <product> --build-target ohos_application_test
```

### Run tests:
```bash
# Run all unit tests
run -t UT -tp ability_runtime

# Run specific test
run -t UT -ts ability_stage_context_test
run -t UT -ts js_ability_stage_context_test
run -t UT -ts ohos_application_test
```

## Test Scenarios Covered

### ✅ Normal Flow
1. ElementName is properly set via `SetElementName()`
2. ElementName is properly retrieved via `GetElementName()`
3. NAPI layer correctly converts ElementName to JS object
4. AddAbilityStage sets ElementName before calling onCreate

### ✅ Edge Cases
1. Empty ElementName (default constructed)
2. Null Want pointer (crash prevention)
3. Null context handling
4. Multiple updates to ElementName

### ✅ Integration
1. C++ core layer → NAPI binding layer
2. OHOSApplication → AbilityStageContext → ElementName
3. ElementName → WrapElementName → JS object

## Known Limitations

1. **NAPI Test Environment**: The JS context tests require a proper NAPI environment setup. Some test cases may be limited in their ability to fully simulate JS runtime behavior.

2. **Mock Dependencies**: The tests rely on mock objects (MockAbilityStage, MockRuntime, etc.) to isolate the functionality under test.

## Future Test Enhancements

Potential areas for additional testing:
1. Integration tests with actual ArkTS runtime
2. Performance tests for ElementName operations
3. Concurrent access tests
4. Stress tests with rapid ElementName updates

## Maintenance Notes

- When modifying ElementName functionality, ensure corresponding test updates
- Check test coverage after code changes
- Maintain consistency with existing test patterns in the codebase
