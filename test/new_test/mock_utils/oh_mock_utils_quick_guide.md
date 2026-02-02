# oh_mock_utils.h 快速指南

**版本**: 1.0
**组件**: ability_runtime
**文件位置**: `test/new_test/mock_utils/oh_mock_utils.h`

---

## 目录

1. [5分钟快速入门](#1-5分钟快速入门)
2. [常用宏速查](#2-常用宏速查)
3. [关键注意事项](#3-关键注意事项)
4. [典型示例](#4-典型示例)
5. [完整参考](#5-完整参考)

---

## 1. 5分钟快速入门

### 1.1 基本结构

```cpp
#define private public
#define protected public

#include <gtest/gtest.h>
#include "your_class.h"        // 被测类
#include "oh_mock_utils.h"     // Mock 工具

#undef private
#undef protected
```

**⚠️ 关键**: `#define private public` 必须在所有 `#include` 之前！

### 1.2 编写第一个测试

```cpp
#include "permission_verification.h"  // 假设需要 Mock 这个类

// 步骤1: 创建 Mock 类（通常在独立的 mock 头文件中）
// test/new_test/mock/permission_verification/permission_verification.h
class PermissionVerification {
public:
    OH_MOCK_METHOD(bool, PermissionVerification, VerifyPermissionByTokenId,
        const int&, const std::string&);
};

// 步骤2: 在测试中设置期望值
HWTEST_F(MyTest, TestCase_001, TestSize.Level1)
{
    // 设置期望返回 true
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId,
        const int&, const std::string&);

    // 调用被测代码
    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyProxyAuthorizationUriPermission();

    // 验证结果
    EXPECT_EQ(result, true);
}
```

### 1.3 核心工作流程

```
1. 定义 Mock 函数（一次性，在 mock 头文件中）
   └─> 使用 OH_MOCK_METHOD_* 系列宏

2. 设置期望值（每次测试前）
   └─> 使用 OH_EXPECT_RET / OH_EXPECT_RET_AND_OUTPUT

3. 调用被测代码
   └─> 正常调用你的生产代码

4. 验证结果
   └─> 使用 EXPECT_* 宏验证
```

---

## 2. 常用宏速查

### 2.1 Mock 定义宏（在 mock 头文件中使用）

| 宏名称 | 适用场景 | 示例 |
|--------|---------|------|
| `OH_MOCK_METHOD` | 普通成员函数，有返回值 | `bool GetValue(int id)` |
| `OH_MOCK_VOID_METHOD` | 普通成员函数，无返回值 | `void SetValue(int id)` |
| `OH_MOCK_METHOD_WITH_DECORATOR` | **静态**成员函数，有返回值 | `static bool Check(int id)` |
| `OH_MOCK_METHOD_WITH_OUTPUT_1` | 带单个输出参数 | `bool GetInfo(int id, Info& info)` |
| `OH_MOCK_METHOD_WITH_OUTPUT_VECTOR` | 带 vector 输出参数 | `int GetResults(vector<int>& results)` |
| `OH_MOCK_METHOD_RET_SPTR` | 返回 sptr<T> 智能指针 | `sptr<MyObj> GetInstance()` |

**完整宏列表**: 参见第 5 节完整参考

### 2.2 期望设置宏（在测试函数中使用）

| 宏名称 | 用途 | 示例 |
|--------|------|------|
| `OH_EXPECT_RET` | 设置返回值（基本类型、string、sptr） | 单次/多次返回 |
| `OH_EXPECT_RET_AND_OUTPUT` | 设置返回值 + 输出参数 | 带输出参数的函数 |

**语法格式**:
```cpp
// OH_EXPECT_RET
OH_EXPECT_RET(返回值列表, 类名, 方法名, 参数类型列表);

// OH_EXPECT_RET_AND_OUTPUT
OH_EXPECT_RET_AND_OUTPUT(返回值列表, 输出值列表, 类名, 方法名, 参数类型列表);
```

---

## 3. 关键注意事项

### ⚠️ 3.1 临时变量要求（最重要！）

**规则**: 多次返回和输出参数**必须**使用临时变量，不能直接传初始化列表。

```cpp
// ✅ 正确：多次返回使用临时变量
auto expectRets = std::vector<bool>{true, false, true};
OH_EXPECT_RET(expectRets, PermissionVerification, VerifyPermissionByTokenId,
    const int&, const std::string&);

// ❌ 错误：多次返回直接传字面量（编译错误）
OH_EXPECT_RET({true, false, true}, PermissionVerification, VerifyPermissionByTokenId,
    const int&, const std::string&);

// ✅ 正确：单次返回可以简化
OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId,
    const int&, const std::string&);
```

**输出参数也必须使用临时变量**:
```cpp
// ✅ 正确：输出参数使用临时变量
std::vector<std::vector<bool>> expectResults = {{true, false}};
OH_EXPECT_RET_AND_OUTPUT({ERR_OK}, expectResults, MediaPermissionHelper,
    CheckPhotoUriPermission, uint32_t, const std::vector<std::string>&,
    std::vector<bool>& results, const std::vector<uint32_t>&);

// ❌ 错误：输出参数直接传嵌套初始化列表
OH_EXPECT_RET_AND_OUTPUT({ERR_OK}, {{true, false}}, ...);  // 编译错误！
```

### ⚠️ 3.2 命名空间处理

**规则**: 使用 `using namespace` 引入命名空间，**className 不带命名空间前缀**。

```cpp
// ✅ 正确：使用 using 声明，className 不带前缀
using namespace OHOS::UDMF;
OH_EXPECT_RET({true}, UdmfClient, GetBundleNameByUdKey, const std::string&);

// ❌ 错误：className 带命名空间前缀（key 不匹配）
OH_EXPECT_RET({true}, UDMF::UdmfClient, GetBundleNameByUdKey, const std::string&);
```

### ⚠️ 3.3 参数类型字符串匹配

**规则**: 参数类型字符串必须与 mock 定义中的类型完全一致（包括命名空间前缀）。

```cpp
// Mock 定义（注意参数类型带 UDMF:: 前缀）
OH_MOCK_METHOD(int32_t, UdmfClient, AddPrivilege,
    const UDMF::QueryOption&, const UDMF::Privilege&);

// 测试中：参数类型必须一致
OH_EXPECT_RET(expectRets, UdmfClient, AddPrivilege,
    const UDMF::QueryOption&, const UDMF::Privilege&);  // ✅ 正确
```

### ⚠️ 3.4 静态方法 vs 成员方法

**规则**: 区分静态方法和实例方法，使用不同的宏。

| 方法类型 | 宏 | 示例 |
|---------|-----|------|
| 实例方法 | `OH_MOCK_METHOD` | `obj.Method()` |
| 静态方法 | `OH_MOCK_METHOD_WITH_DECORATOR` | `Class::Method()` |
| 全局函数 | `OH_MOCK_GLOBAL_METHOD` | `Function()` |

```cpp
// 静态方法 Mock（注意 static 参数）
OH_MOCK_METHOD_WITH_DECORATOR(static, bool, PermissionVerification, VerifyPermissionByTokenId,
    const int&, const std::string&);

// 实例方法 Mock（无需 static 参数）
OH_MOCK_METHOD(int32_t, UdmfClient, AddPrivilege,
    const UDMF::QueryOption&, const UDMF::Privilege&);
```

### ⚠️ 3.5 测试用例命名规范

**格式**: `方法名_场景描述_编号`（编号从 001 开始连续计数）

```cpp
// ✅ 正确命名
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_NotCreatedByCaller_001, TestSize.Level1)
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_AddPrivilegeTargetFailed_002, TestSize.Level1)
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_Success_003, TestSize.Level1)

// ❌ 错误命名
HWTEST_F(UdmfUtilsTest, Test1, TestSize.Level1)  // 缺少场景描述
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_001, TestSize.Level1)  // 缺少场景描述
```

---

## 4. 典型示例

### 4.1 示例1: 简单 Bool 返回值

**场景**: Mock `PermissionVerification::VerifyPermissionByTokenId` 返回 bool

```cpp
// ========== Mock 定义（permission_verification.h） ==========
class PermissionVerification {
public:
    OH_MOCK_METHOD_WITH_DECORATOR(static, bool, PermissionVerification, VerifyPermissionByTokenId,
        const int&, const std::string&);
};

// ========== 测试代码 ==========
HWTEST_F(TokenIdPermissionTest, VerifyProxyAuthorizationUriPermission_Granted_001, TestSize.Level1)
{
    // Arrange: 设置期望返回 true
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId,
        const int&, const std::string&);

    // Act: 调用被测代码
    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyProxyAuthorizationUriPermission();

    // Assert: 验证结果
    EXPECT_EQ(result, true);
}
```

### 4.2 示例2: 带输出参数的函数

**场景**: Mock `MediaPermissionHelper::CheckPhotoUriPermission`，带 vector<bool> 输出参数

```cpp
// ========== Mock 定义（media_permission_helper.h） ==========
class MediaPermissionHelper {
public:
    OH_MOCK_METHOD_WITH_DECORATOR(static, int32_t, MediaPermissionHelper, CheckPhotoUriPermission,
        uint32_t, const std::vector<std::string>&, std::vector<bool>&, const std::vector<uint32_t>&);
};

// ========== 测试代码 ==========
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_AllGranted_001, TestSize.Level1)
{
    // Arrange: 设置返回值和输出参数
    auto expectRets = std::vector<int32_t>{ERR_OK};
    std::vector<std::vector<bool>> expectResults = {{true, true, true}};  // ⚠️ 使用临时变量
    OH_EXPECT_RET_AND_OUTPUT(expectRets, expectResults, MediaPermissionHelper,
        CheckPhotoUriPermission, uint32_t, const std::vector<std::string>&,
        std::vector<bool>&, const std::vector<uint32_t>&);

    // Act: 调用被测代码
    std::vector<bool> results;
    int32_t ret = mediaPermissionMgr->CheckUriPermission(uris, results);

    // Assert: 验证返回值和输出参数
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(results.size(), 3u);
    EXPECT_TRUE(results[0]);
}
```

### 4.3 示例3: 多次调用序列

**场景**: 同一个函数被调用多次，每次返回不同值

```cpp
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_AddPrivilegeSequence_004, TestSize.Level1)
{
    using namespace UDMF;
    // Arrange: AddPrivilege 被调用两次，第一次成功，第二次失败
    auto expectAddPrivilegeRets = std::vector<int32_t>{ERR_OK, INNER_ERR};  // ⚠️ 临时变量
    OH_EXPECT_RET(expectAddPrivilegeRets, UdmfClient, AddPrivilege,
        const UDMF::QueryOption&, const UDMF::Privilege&);

    // Act: 调用被测代码
    std::vector<std::string> uris;
    int32_t ret = UDMFUtils::ProcessUdmfKey(TEST_KEY, TEST_CALLER_TOKEN_ID, TEST_TARGET_TOKEN_ID, uris);

    // Assert: 验证结果
    EXPECT_EQ(ret, ERR_UPMS_ADD_PRIVILEGED_FAILED);  // 第二次调用失败
}
```

### 4.4 示例4: Vector 输出参数

**场景**: Mock `UdmfClient::GetBatchData`，带 `vector<UnifiedData>&` 输出参数

```cpp
// ========== Mock 定义（udmf_client.h） ==========
class UdmfClient {
public:
    int32_t GetBatchData(const QueryOption &query, std::vector<UnifiedData> &unifiedDataset)
    {
        OH_MOCK_METHOD_WITH_OUTPUT_VECTOR(int32_t, unifiedDataset, UdmfClient, GetBatchData,
            const UDMF::QueryOption&, std::vector<UDMF::UnifiedData>&);
    }
};

// ========== 测试代码 ==========
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_Success_005, TestSize.Level1)
{
    using namespace UDMF;
    // Arrange: 准备测试数据
    auto record = std::make_shared<UnifiedRecord>(TEST_URI);
    std::vector<std::shared_ptr<UnifiedRecord>> records = {record};
    auto unifiedData = UnifiedData(records);
    std::vector<std::vector<UDMF::UnifiedData>> expectUnifiedDataList = {{unifiedData}};  // ⚠️ 临时变量

    OH_EXPECT_RET_AND_OUTPUT({ERR_OK}, expectUnifiedDataList, UdmfClient, GetBatchData,
        const UDMF::QueryOption&, std::vector<UDMF::UnifiedData>&);

    // Act: 调用被测代码
    std::vector<std::string> uris;
    int32_t ret = UDMFUtils::ProcessUdmfKey(TEST_KEY, TEST_CALLER_TOKEN_ID, TEST_TARGET_TOKEN_ID, uris);

    // Assert: 验证结果
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(uris.size(), 1u);
    EXPECT_EQ(uris[0], TEST_URI);
}
```

---

## 5. 完整参考

### 5.1 所有 Mock 定义宏

| 宏名称 | 用途 | 参数说明 |
|--------|------|---------|
| `OH_MOCK_METHOD` | 普通成员函数（有返回值） | `(返回类型, 类名, 方法名, 参数类型...)` |
| `OH_MOCK_VOID_METHOD` | 普通成员函数（无返回值） | `(类名, 方法名, 参数类型...)` |
| `OH_MOCK_METHOD_RET_SPTR` | 返回 sptr<T> | `(返回类型, 类名, 方法名, 参数类型...)` |
| `OH_MOCK_METHOD_WITH_OUTPUT_1` | 带单个输出参数 | `(返回类型, 输出参数名, 类名, 方法名, 参数类型...)` |
| `OH_MOCK_METHOD_WITH_OUTPUT_VECTOR` | 带 vector 输出参数 | `(返回类型, 输出参数名, 类名, 方法名, 参数类型...)` |
| `OH_MOCK_METHOD_WITH_DECORATOR` | static/virtual 函数（有返回值） | `(修饰符, 返回类型, 类名, 方法名, 参数类型...)` |
| `OH_MOCK_VOID_METHOD_WITH_DECORATOR` | static/virtual void 函数 | `(修饰符, 类名, 方法名, 参数类型...)` |
| `OH_MOCK_METHOD_WITH_DECORATOR_AND_OUTPUT_1` | static + 输出参数 | `(修饰符, 返回类型, 输出参数名, 类名, 方法名, 参数类型...)` |
| `OH_MOCK_FUNCTION_WITH_DEFAULT_PARAM_BODY` | 带默认参数的函数 | `(返回类型, 类名, 方法名, 参数类型字符串)` |
| `OH_MOCK_GLOBAL_METHOD` | 全局函数（有返回值） | `(返回类型, 函数名, 参数类型...)` |
| `OH_MOCK_GLOBAL_VOID_METHOD` | 全局 void 函数 | `(函数名, 参数类型...)` |
| `OH_MOCK_GLOBAL_TEMPLATE_METHOD_RET_SPTR` | 全局模板函数返回 sptr | `(返回类型, 函数名, 参数类型...)` |

### 5.2 宏选择决策树

```
需要 Mock 函数
├─ 是成员函数？
│  ├─ 有返回值？
│  │  ├─ 返回 sptr<T> → OH_MOCK_METHOD_RET_SPTR
│  │  ├─ 带输出参数？
│  │  │  ├─ vector 输出 → OH_MOCK_METHOD_WITH_OUTPUT_VECTOR
│  │  │  └─ 单个输出 → OH_MOCK_METHOD_WITH_OUTPUT_1
│  │  └─ 基本返回 → OH_MOCK_METHOD
│  └─ 无返回值 → OH_MOCK_VOID_METHOD
├─ 是静态成员函数？
│  ├─ 有返回值 → OH_MOCK_METHOD_WITH_DECORATOR
│  ├─ 无返回值 → OH_MOCK_VOID_METHOD_WITH_DECORATOR
│  └─ 带输出参数 → OH_MOCK_METHOD_WITH_DECORATOR_AND_OUTPUT_1
└─ 是全局函数？
   ├─ 有返回值 → OH_MOCK_GLOBAL_METHOD
   └─ 无返回值 → OH_MOCK_GLOBAL_VOID_METHOD
```

### 5.3 测试文件结构模板

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
#include "your_class.h"
#include "mock_class.h"
#include "oh_mock_utils.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace YourNamespace {

constexpr uint32_t TEST_TOKEN_ID = 1001;

class YourTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void YourTest::SetUpTestCase() {}
void YourTest::TearDownTestCase() {}
void YourTest::SetUp() {}
void YourTest::TearDown() {}

/**
 * @tc.name: Method_Scenario_001
 * @tc.desc: Description of what this test covers
 *           Branch: Which branch/condition is being tested
 * @tc.type: FUNC
 */
HWTEST_F(YourTest, Method_Scenario_001, TestSize.Level1)
{
    // Arrange: 准备数据和 Mock
    using namespace YourNamespace;  // 如果需要
    OH_EXPECT_RET({true}, YourClass, Method, const int&);

    // Act: 执行被测代码
    bool result = object.Method();

    // Assert: 验证结果
    EXPECT_EQ(result, true);
}

}  // namespace YourNamespace
}  // namespace OHOS
```

### 5.4 BUILD.gn 模板

```gn
# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import("//build/test.gni")
import("//foundation/ability/ability_runtime/ability_runtime.gni")

module_output_path = "ability_runtime/ability_runtime/your_component"

copy("your_test_copy") {
  sources = [
    "${ability_runtime_path}/services/your_component/src/your_class.cpp",
    "${ability_runtime_path}/services/your_component/include/your_class.h",
  ]
  outputs = [ "${target_gen_dir}/services/your_component/src/{{source_file_part}}" ]
}

ohos_unittest("your_test_unit_test") {
  module_out_path = module_output_path

  include_dirs = [
    "${target_gen_dir}/services/your_component/src",
    "../../../mock_utils",
    "../../../mock/your_mock",
  ]

  sources = [
    "${target_gen_dir}/services/your_component/src/your_class.cpp",
    "your_test.cpp",
  ]

  deps = [ ":your_test_copy" ]

  external_deps = [
    "c_utils:utils",
    "ffrt:libffrt",
    "googletest:gmock_main",
    "googletest:gtest_main",
    "hilog:libhilog",
  ]
}

group("unittest") {
  testonly = true
  deps = [ ":your_test_unit_test" ]
}
```

---

## 附录

### A. 参考文档

- **完整版使用指南**: `oh_mock_utils_user_guide.md`
- **编码规范**: `oh_mock_utils_readme.md`
- **示例测试**:
  - `test/new_test/unit_test/uripermmgr_unit_test/tokenid_permission_test/`
  - `test/new_test/unit_test/uripermmgr_unit_test/media_permission_manager_test/`
  - `test/new_test/unit_test/uripermmgr_unit_test/file_uri_distribution_utils_test/`

### B. 常见错误速查

| 错误现象 | 可能原因 | 解决方案 |
|---------|---------|---------|
| 编译错误：找不到 opParam | Mock 定义时未指定输出参数名 | 在 OH_MOCK_METHOD_WITH_OUTPUT_* 中指定输出参数名 |
| 返回值不符合预期 | Key 不匹配（命名空间、参数类型） | 检查 using 声明和参数类型字符串 |
| 多次调用返回错误值 | 期望值队列设置不正确 | 确保所有期望值在一个临时变量列表中 |
| 输出参数未被正确设置 | 直接传嵌套初始化列表 | 使用临时变量存储输出参数 |

### C. 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.0 | 2026-02-02 | 初始版本，精简版快速指南 |

---

**文档结束**

如有问题或建议，请联系 TDD 团队或参考完整版使用指南。
