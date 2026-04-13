# IsSkipDisposeRule TDD 测试用例

## 概述

为 `DisposedRuleInterceptor::IsSkipDisposeRule` 函数单独添加了TDD测试用例。

## 函数签名

```cpp
bool DisposedRuleInterceptor::IsSkipDisposeRule(const std::string &callerName, const AbilityInterceptorParam &param)
```

## 函数逻辑

```cpp
if (param.startOptions == nullptr || param.startOptions->processOptions == nullptr) {
    return false;
}
return callerName == "7007" &&
       param.startOptions->processOptions->startupVisibility == StartupVisibility::STARTUP_HIDE &&
       param.startOptions->processOptions->isPreloadStart;
```

## 测试用例列表

### 001: startOptions 为 nullptr
- **测试场景**: `param.startOptions == nullptr`
- **callerName**: "7007"
- **预期结果**: `false`
- **覆盖点**: 空指针检查

### 002: processOptions 为 nullptr
- **测试场景**: `startOpts.processOptions == nullptr`
- **callerName**: "7007"
- **预期结果**: `false`
- **覆盖点**: 空指针检查

### 003: callerName 不匹配
- **测试场景**:
  - `callerName = "1001"` (不是 "7007")
  - `startupVisibility = STARTUP_HIDE`
  - `isPreloadStart = true`
- **预期结果**: `false`
- **覆盖点**: 调用者名称验证

### 004: startupVisibility 不匹配
- **测试场景**:
  - `callerName = "7007"`
  - `startupVisibility = STARTUP_SHOW` (不是 HIDE)
  - `isPreloadStart = true`
- **预期结果**: `false`
- **覆盖点**: 可见性条件验证

### 005: isPreloadStart 为 false
- **测试场景**:
  - `callerName = "7007"`
  - `startupVisibility = STARTUP_HIDE`
  - `isPreloadStart = false` (不是 true)
- **预期结果**: `false`
- **覆盖点**: 预加载标志验证

### 006: 所有条件满足 ⭐
- **测试场景**:
  - `callerName = "7007"` ✅
  - `startupVisibility = STARTUP_HIDE` ✅
  - `isPreloadStart = true` ✅
- **预期结果**: `true`
- **覆盖点**: 正向测试路径

### 007: 默认 processOptions 值
- **测试场景**:
  - `callerName = "7007"`
  - `startupVisibility = UNSPECIFIED` (默认值)
  - `isPreloadStart = false` (默认值)
- **预期结果**: `false`
- **覆盖点**: 边界值测试

## 测试覆盖矩阵

| 测试用例 | startOptions | processOptions | callerName | visibility | isPreloadStart | 预期结果 |
|---------|--------------|----------------|------------|------------|----------------|----------|
| 001 | nullptr | - | "7007" | - | - | false |
| 002 | ✅ | nullptr | "7007" | - | - | false |
| 003 | ✅ | ✅ | "1001" | HIDE | true | false |
| 004 | ✅ | ✅ | "7007" | SHOW | true | false |
| 005 | ✅ | ✅ | "7007" | HIDE | false | false |
| 006 | ✅ | ✅ | "7007" | HIDE | true | **true** |
| 007 | ✅ | ✅ | "7007" | UNSPECIFIED | false | false |

## 文件修改

### 1. 测试文件
**文件**: `disposed_rule_interceptor_test.cpp`
- 新增头文件:
  - `#include "process_options.h"`
  - `#include "start_options.h"`
- 新增测试用例: 7个
- 总行数: 964 → 1153 (+189行)

### 2. 构建配置
**文件**: `BUILD.gn`
- 新增包含路径:
  - `${ability_runtime_path}/interfaces/inner_api/ability_manager/include`

## 构建和运行

```bash
# 构建测试
./build.sh --product-name <product> --build-target disposed_rule_interceptor_test

# 运行测试
run -t UT -ts disposed_rule_interceptor_test
```

## 测试统计

- **原测试用例数**: 34个
- **新增测试用例数**: 7个
- **总计**: 41个
- **代码覆盖率**: 100% (IsSkipDisposeRule 函数所有分支)

## 分支覆盖

✅ `startOptions == nullptr` - 用例001
✅ `processOptions == nullptr` - 用例002
✅ `callerName != "7007"` - 用例003
✅ `startupVisibility != STARTUP_HIDE` - 用例004
✅ `isPreloadStart == false` - 用例005, 007
✅ **所有条件满足** - 用例006

## 注意事项

1. **仅测试 IsSkipDisposeRule**: 不包含 DoProcess 集成测试
2. **不修改原有测试**: 保持34个原始测试用例不变
3. **独立可编译**: 测试可独立运行，不依赖其他修改
4. **完整覆盖**: 覆盖所有代码分支和边界条件
