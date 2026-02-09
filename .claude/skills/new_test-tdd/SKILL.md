---
name: new_test-tdd
description: 智能生成 test/new_test 路径下的 TDD 测试用例，包含自动 Mock 分析、完整代码生成和智能确认机制
license: Apache-2.0
metadata:
  version: "3.1.0"
  author: agno-team
  tags: ["mock", "new_test", "tdd", "test-generation"]
---

## 任务

为 ability_runtime 组件的 C++ 代码智能生成 TDD 测试用例，包括：
- 自动分析被测文件的依赖
- 智能选择合适的 Mock 宏
- 生成完整的测试代码（含 Mock 设置、Act、Assert）
- 生成配套的 BUILD.gn 文件
- 提供智能确认机制和决策辅助

## 前置条件

使用本技能前需要满足以下条件：
- 被测文件位于 `services/` 或 `interfaces/` 目录下
- 已了解 oh_mock_utils.h 的基本用法
- 测试目标在 `test/new_test/` 目录下有对应的测试文件夹

## 信息收集

在生成测试用例前，需要收集以下信息：

### 必需信息
- **被测文件路径**：完整路径（如 `services/uripermmgr/src/media_permission_manager.cpp`）
- **测试用例名称**：简洁的测试名称（如 `media_permission_manager_test`）
- **所属组件**：所属的服务组件（如 `uripermmgr`）

### 可选信息
- **需要 Mock 的外部依赖**：列出需要 Mock 的依赖（可选，系统会自动分析）
- **特殊测试需求**：如需要测试特定分支或场景
- **测试场景覆盖**：希望生成哪些测试场景（可选，系统会根据函数复杂度推荐）

## 步骤

### 步骤 1：收集并分析被测代码

读取被测文件，执行以下分析：
- 识别被测类的名称和命名空间
- 分析所有外部依赖（include 的头文件）
- 识别每个依赖的函数签名（返回类型、参数类型、是否静态）
- 评估函数复杂度（分支数量、依赖数量）

**输出**：
- 依赖列表（包含函数签名）
- 函数复杂度评估
- 需要创建的 Mock 定义列表

### 步骤 2：检查现有 Mock 定义

对于每个依赖，检查是否已存在 Mock 定义：
- 搜索 `test/new_test/mock/` 目录
- 检查 Mock 类是否包含所需的 Mock 方法

**智能确认**：如果 Mock 不存在，提供以下选项：
- [A] 自动生成 Mock 框架（推荐）
- [B] 跳过该依赖的 Mock
- [C] 手动指定 Mock 路径

### 步骤 3：智能选择 Mock 宏

根据函数签名自动推荐最合适的 Mock 宏：

#### Mock 宏选择决策树

```
需要 Mock 函数
├─ 成员函数？
│  ├─ 有返回值？
│  │  ├─ 返回 sptr<T> → OH_MOCK_METHOD_RET_SPTR
│  │  ├─ 带输出参数？
│  │  │  ├─ vector 输出 → OH_MOCK_METHOD_WITH_OUTPUT_VECTOR
│  │  │  └─ 标量输出 → OH_MOCK_METHOD_WITH_OUTPUT_1
│  │  └─ 基本返回 → OH_MOCK_METHOD
│  └─ 无返回值 → OH_MOCK_VOID_METHOD
├─ 静态/虚函数？
│  ├─ 有返回值？
│  │  ├─ 带输出参数？
│  │  │  ├─ vector 输出 → OH_MOCK_METHOD_WITH_DECORATOR_AND_OUTPUT_VECTOR
│  │  │  └─ 标量输出 → OH_MOCK_METHOD_WITH_DECORATOR_AND_OUTPUT_1
│  │  └─ 基本返回 → OH_MOCK_METHOD_WITH_DECORATOR
│  └─ 无返回值 → OH_MOCK_VOID_METHOD_WITH_DECORATOR
└─ 全局函数？
   ├─ 有返回值 → OH_MOCK_GLOBAL_METHOD
   └─ 无返回值 → OH_MOCK_GLOBAL_VOID_METHOD
```

**智能确认**：当存在多种可行方案时，展示推荐方案并请求确认。

### 步骤 4：设计测试场景并生成完整用例

根据函数复杂度自动推荐测试场景：

#### 简单函数（少于 3 个分支）
- 成功场景（正常输入）
- 失败场景（错误输入）
- 边界场景（空值/极值）

#### 中等复杂函数（3-5 个分支）
- 主流程成功
- 各个分支失败
- 特殊输入处理
- 边界条件

#### 复杂函数（5 个以上分支或多个依赖）
- 基本功能测试
- 依赖失败场景
- 多次调用序列
- 并发场景
- 资源释放验证

**智能确认**：对于复杂函数，展示场景建议并请求确认。

**生成完整代码**：每个测试用例包含：
- **Arrange 阶段**：准备测试数据、设置 Mock 期望值
- **Act 阶段**：调用被测函数
- **Assert 阶段**：使用 EXPECT_* 验证结果

**测试用例示例**：

```cpp
/**
 * @tc.name: CheckUriPermission_Success_001
 * @tc.desc: Verify CheckUriPermission returns correct results when helper exists.
 *           Branch: helper != nullptr, CheckPhotoUriPermission succeeds
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_Success_001, TestSize.Level1)
{
    // ===== Arrange：准备数据和 Mock =====
    std::vector<std::string> uriVec = {"file://media/test1.jpg", "file://media/test2.jpg"};
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    // Mock helper 单例
    auto mockHelper = new MediaPermissionHelper();
    OH_EXPECT_RET({mockHelper}, MediaPermissionHelper, GetMediaPermissionHelper);

    // Mock CheckPhotoUriPermission（带输出参数）
    std::vector<std::vector<bool>> expectResults = {{true, true}};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int32_t>(ERR_OK)}, expectResults,
        MediaPermissionHelper, CheckPhotoUriPermission,
        uint32_t, const std::vector<std::string>&, std::vector<bool>& results,
        const std::vector<uint32_t>&);

    // ===== Act：调用被测代码 =====
    auto& instance = MediaPermissionManager::GetInstance();
    auto results = instance.CheckUriPermission(uriVec, callerTokenId, flag);

    // ===== Assert：验证结果 =====
    EXPECT_EQ(results.size(), 2U);
    EXPECT_EQ(results[0], true);
    EXPECT_EQ(results[1], true);
}
```

### 步骤 5：生成测试文件和 BUILD.gn

生成以下文件：
- 测试 cpp 文件（包含所有测试用例）
- BUILD.gn 文件（包含 copy、ohos_unittest、group）
- Mock 头文件（如果需要新建）

**文件预览确认**：展示即将生成的文件列表和基本信息，请求确认。

### 步骤 6：验证和指导

- 提供编译命令
- 提供运行命令
- 提示常见问题和解决方案

## 文件模板

### 测试 cpp 文件模板

```cpp
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

#define private public
#define protected public

#include <gtest/gtest.h>
#include "${TARGET_CLASS}.h"
${MOCK_INCLUDES}
#include "oh_mock_utils.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
${NAMESPACE_USINGS}

namespace OHOS {
namespace ${NAMESPACE} {

class ${TEST_CLASS_NAME} : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void ${TEST_CLASS_NAME}::SetUpTestCase() {}

void ${TEST_CLASS_NAME}::TearDownTestCase() {}

void ${TEST_CLASS_NAME}::SetUp() {}

void ${TEST_CLASS_NAME}::TearDown() {}

${TEST_CASES}

}  // namespace ${NAMESPACE}
}  // namespace OHOS
```

### Mock 头文件模板

```cpp
#ifndef OHOS_ABILITY_RUNTIME_${CLASS_NAME}_MOCK_H
#define OHOS_ABILITY_RUNTIME_${CLASS_NAME}_MOCK_H

#include "oh_mock_utils.h"
${ORIGINAL_INCLUDES}

namespace OHOS {
namespace ${NAMESPACE} {

class ${CLASS_NAME} {
public:
${MOCK_METHODS}
};

}  // namespace ${NAMESPACE}
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_${CLASS_NAME}_MOCK_H
```

### BUILD.gn 模板

```gn
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# ...

import("//build/test.gni")
import("//foundation/ability/ability_runtime/ability_runtime.gni")

module_output_path = "ability_runtime/ability_runtime/${COMPONENT}"

copy("${TARGET}_copy") {
  sources = [
${SOURCE_FILES}
  ]
  outputs = [ "${target_gen_dir}/services/${COMPONENT}/src/{{source_file_part}}" ]
}

ohos_unittest("${TEST_NAME}") {
  module_out_path = module_output_path

  include_dirs = [
    "${target_gen_dir}/services/${COMPONENT}/src",
    "../../../mock_utils",
${MOCK_INCLUDE_DIRS}
  ]

  sources = [
${TEST_SOURCES}
  ]

  deps = [ ":${TARGET}_copy" ]

  external_deps = [
${EXTERNAL_DEPS}
  ]
}

group("unittest") {
  testonly = true
  deps = [ ":${TEST_NAME}" ]
}
```

## 快速参考

### Mock 宏速查表

| 宏名称 | 适用场景 | 返回值 | 输出参数 | 示例函数 |
|--------|---------|--------|---------|----------|
| `OH_MOCK_METHOD` | 成员函数 | ✅ 基本类型 | ❌ | `bool GetValue(int id)` |
| `OH_MOCK_VOID_METHOD` | 成员函数 | ❌ void | ❌ | `void SetValue(int id)` |
| `OH_MOCK_METHOD_RET_SPTR` | 成员函数 | ✅ sptr<T> | ❌ | `sptr<Mgr> GetInstance()` |
| `OH_MOCK_METHOD_WITH_OUTPUT_1` | 成员函数 | ✅ 基本类型 | ✅ 1个标量 | `bool GetInfo(int id, Info& info)` |
| `OH_MOCK_METHOD_WITH_OUTPUT_VECTOR` | 成员函数 | ✅ 基本类型 | ✅ vector | `int GetResults(vector<int>& results)` |
| `OH_MOCK_METHOD_WITH_DECORATOR` | static/virtual | ✅ 基本类型 | ❌ | `static bool Check(int id)` |
| `OH_MOCK_VOID_METHOD_WITH_DECORATOR` | static/virtual | ❌ void | ❌ | `static void Init()` |
| `OH_MOCK_METHOD_WITH_DECORATOR_AND_OUTPUT_1` | static/virtual | ✅ 基本类型 | ✅ 1个标量 | `static int Get(int id, Info& info)` |
| `OH_MOCK_METHOD_WITH_DECORATOR_AND_OUTPUT_VECTOR` | static/virtual | ✅ 基本类型 | ✅ vector | `static int Get(vector<int>& results)` |
| `OH_MOCK_GLOBAL_METHOD` | 全局函数 | ✅ 基本类型 | ❌ | `int GlobalFunc()` |
| `OH_MOCK_GLOBAL_VOID_METHOD` | 全局函数 | ❌ void | ❌ | `void GlobalVoidFunc()` |

**期望设置宏**：

| 宏名称 | 用途 | 示例 |
|--------|------|------|
| `OH_EXPECT_RET` | 设置返回值（成员/静态） | `OH_EXPECT_RET({true}, Class, Method, types...)` |
| `OH_EXPECT_RET_AND_OUTPUT` | 设置返回值+输出参数 | `OH_EXPECT_RET_AND_OUTPUT({ret}, {out}, Class, Method, types...)` |
| `OH_GLOBAL_EXPECT_RET` | 设置返回值（全局） | `OH_GLOBAL_EXPECT_RET({val}, Func, types...)` |

### 常见错误和解决方案

#### 错误 1：编译错误 - 找不到 opParam
```
错误：'opParam' was not declared in this scope
原因：Mock 定义时未指定输出参数名
解决：在 OH_MOCK_METHOD_WITH_OUTPUT_* 中指定输出参数名
```

#### 错误 2：any_bad_cast 运行时崩溃
```
错误：std::any_bad_cast 异常
原因：期望值类型不明确（如 ERR_OK、INNER_ERR）
解决：使用显式类型转换
  ❌ OH_EXPECT_RET({ERR_OK}, ...)
  ✅ OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, ...)
```

#### 错误 3：返回值不符合预期
```
错误：Mock 函数返回了默认值（0/false）
原因：Key 不匹配（命名空间、参数类型）
解决：
  1. 使用 using namespace 引入命名空间
  2. className 不带命名空间前缀
  3. 参数类型字符串必须完全一致
```

#### 错误 4：多次调用返回错误值
```
错误：第2次及以后的调用返回了错误值
原因：期望值队列设置不正确
解决：
  ❌ OH_EXPECT_RET({true, false}, Class, Method, ...)  // 错误
  ✅ auto v = vector<bool>{true, false}; OH_EXPECT_RET(v, ...)  // 正确
```

#### 错误 5：OH_EXPECT_RET_AND_OUTPUT 编译错误
```
错误：no matching function for call to 'CreateResultWrap'
原因：直接传了嵌套初始化列表 {{...}}
解决：
  ❌ OH_EXPECT_RET_AND_OUTPUT({ERR_OK}, {{true, false}}, ...)
  ✅ vector<vector<bool>> v = {{true, false}}; OH_EXPECT_RET_AND_OUTPUT({ERR_OK}, v, ...)
```

### 命名规范和代码风格

#### 测试用例命名
```
格式：{方法名}_{场景描述}_{编号}
编号：从 001 开始连续计数

示例：
✅ CheckUriPermission_Success_001
✅ CheckUriPermission_EmptyInput_002
✅ GetMediaPermissionHelper_Failure_003
❌ TestCase1  // 缺少场景描述
❌ CheckUriPermission_001  // 缺少场景描述
```

#### 变量命名
```cpp
// 测试数据
std::vector<std::string> uriVec;
uint32_t callerTokenId = 1001;
uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

// Mock 期望值
auto mockHelper = new MediaPermissionHelper();
std::vector<std::vector<bool>> expectResults = {{true, true}};

// 测试结果
auto& instance = MediaPermissionManager::GetInstance();
auto results = instance.CheckUriPermission(...);
```

#### 代码风格
- **行宽**：不超过 120 字符
- **缩进**：4 个空格（禁止 Tab）
- **大括号**：K&R 风格
- **if/for/while**：必须使用大括号
- **License**：Apache 2.0（年份为创建或修改年份）

### 关键注意事项

#### 1. 临时变量要求
- **多次返回**：必须使用临时变量
- **输出参数**：必须使用临时变量
- **单次返回**：可直接传初始化列表

```cpp
// ✅ 多次返回使用临时变量
auto expectRets = std::vector<bool>{true, false, true};
OH_EXPECT_RET(expectRets, Class, Method, ...);

// ✅ 单次返回可简化
OH_EXPECT_RET({true}, Class, Method, ...);
```

#### 2. 类型转换
- **错误码**：必须显式转换 `static_cast<int32_t>(ERR_OK)`
- **枚举类型**：建议显式转换
- **基本类型**：不需要转换

#### 3. 命名空间处理
```cpp
// ✅ 正确：使用 using 声明
using namespace OHOS::YourNamespace;
OH_EXPECT_RET({true}, YourClass, Method, ...);

// ❌ 错误：className 带命名空间前缀
OH_EXPECT_RET({true}, OHOS::YourNamespace::YourClass, Method, ...);
```

#### 4. TDD 宏定义位置
```cpp
// ✅ 正确顺序
#define private public
#define protected public

#include "your_class.h"
#include "oh_mock_utils.h"

#undef private
#undef protected
```

#### 5. 单例 Mock 处理
- 继承自 `DelayedSingleton<T>` 的单例：使用 `OH_MOCK_METHOD_WITH_DECORATOR`
- `DECLAR_SINGLETON` 声明的单例：继续使用 `DECLAR_SINGLETON`
- `GetInstance()` 为空的判断分支无需覆盖

## 编译和运行

```bash
# 构建特定 TDD 测试
./build.sh --product-name <product> --build-target <test_name>

# 运行测试
run -t UT -ts <test_name>

# 构建所有 TDD 测试
./build.sh --product-name <product> --build-target ability_runtime_test

# 运行所有 TDD 测试
run -t UT -tp ability_runtime
```

## 依赖配置

### 外部依赖（直接添加）
```gn
external_deps = [
  "ability_base:configuration",
  "ability_base:want",
  "ability_base:zuri",
  "c_utils:utils",
  "hilog:libhilog",
  "hisysevent:libhisysevent",
  "hitrace:hitrace_meter",
  "ffrt:libffrt",
  "googletest:gmock_main",
  "googletest:gtest_main",
]
```

### 内部依赖（使用 copy）
```gn
copy("target_copy") {
  sources = [
    "${ability_runtime_path}/services/uripermmgr/src/target.cpp",
    "${ability_runtime_path}/services/uripermmgr/include/target.h",
  ]
  outputs = [ "${target_gen_dir}/services/uripermmgr/src/{{source_file_part}}" ]
}
```

**注意**：ability_manager_errors.h 引用路径：
```gn
"${ability_runtime_path}/interfaces/inner_api/ability_manager/include/ability_manager_errors.h"
```

## 真实案例参考

### 基础 Mock 用法
- `test/new_test/unit_test/uripermmgr_unit_test/tokenid_permission_test/`
  - 简单的 bool 返回值
  - 静态成员函数 Mock

### 输出参数 Mock
- `test/new_test/unit_test/uripermmgr_unit_test/media_permission_manager_test/`
  - vector 输出参数
  - 单例 Mock
  - 成功和失败场景

### 多个 Mock 协同
- `test/new_test/unit_test/uripermmgr_unit_test/file_uri_distribution_utils_test/`
  - 多个依赖的协同 Mock
  - 静态函数 + 输出参数
  - 多次调用序列

### 高级场景
- `test/new_test/unit_test/uripermmgr_unit_test/upms_udmf_utils_test/`
  - 复杂的依赖链
  - 多次调用和状态管理
  - 错误处理场景

## 参考文档

- **详细指南**：`test/new_test/mock_utils/oh_mock_utils_user_guide.md`
- **快速指南**：`test/new_test/mock_utils/oh_mock_utils_quick_guide.md`
- **工具定义**：`test/new_test/mock_utils/oh_mock_utils.h`
