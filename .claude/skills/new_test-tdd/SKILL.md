---
name: new_test-tdd
description: 关于如何在test/new_test路径下生成tdd用例
license: Apache-2.0
metadata:
  version: "2.0.0"
  author: agno-team
  tags: ["mock", "new_test", "tdd"]
---
### 任务

对一个新的cpp文件在new_test文件夹中生成tdd用例。

### 步骤

1. **分析依赖**：找出待测试文件的内部依赖和外部依赖
2. **检查现有Mock**：查看 `test/new_test/mock` 目录是否已有需要的mock定义
3. **编写测试用例**：按Mock方法选择合适的宏
4. **验证覆盖率**：确保分支覆盖率 > 80%

### Mock方法速查表

| 函数类型 | Mock定义宏 | 期望设置宏 | 注意事项 |
|---------|-----------|-----------|---------|
| **静态成员函数** | `OH_MOCK_METHOD_WITH_DECORATOR` | `OH_EXPECT_RET` | 需要带 `static` 参数 |
| **普通成员函数(有返回)** | `OH_MOCK_METHOD` | `OH_EXPECT_RET` | 返回基本类型 |
| **普通成员函数(无返回)** | `OH_MOCK_VOID_METHOD` | - | void函数 |
| **返回sptr<T>** | `OH_MOCK_METHOD_RET_SPTR` | `OH_EXPECT_RET` | 智能指针 |
| **带1个输出参数** | `OH_MOCK_METHOD_WITH_OUTPUT_1` | `OH_EXPECT_RET_AND_OUTPUT` | 标量输出 |
| **带vector输出参数** | `OH_MOCK_METHOD_WITH_OUTPUT_VECTOR` | `OH_EXPECT_RET_AND_OUTPUT` | vector输出 |
| **静态+输出参数** | `OH_MOCK_METHOD_WITH_DECORATOR_AND_OUTPUT_1` | `OH_EXPECT_RET_AND_OUTPUT` | 静态+输出 |
| **全局函数** | `OH_MOCK_GLOBAL_METHOD` | `OH_GLOBAL_EXPECT_RET` | 不属于任何类 |
| **带默认参数** | `OH_MOCK_FUNCTION_WITH_DEFAULT_PARAM_BODY` | `OH_EXPECT_RET` | 仅代表函数体，需手动声明函数签名 |

### 关键注意事项

#### 1. 多次返回必须使用临时变量

```cpp
// ✅ 正确：多次返回使用临时变量
auto expectRets = std::vector<bool>{true, false, true};
OH_EXPECT_RET(expectRets, Class, Method, ...);

// ❌ 错误：直接传初始化列表
OH_EXPECT_RET({true, false, true}, Class, Method, ...);
```

#### 2. 输出参数必须使用临时变量

```cpp
// ✅ 正确：使用临时变量
std::vector<std::vector<bool>> expectResults = {{true, false}};
OH_EXPECT_RET_AND_OUTPUT({ERR_OK}, expectResults, Class, Method, ...);

// ❌ 错误：直接传初始化列表
OH_EXPECT_RET_AND_OUTPUT({ERR_OK}, {{true, false}}, Class, Method, ...);
```

#### 3. 命名空间处理

```cpp
// ✅ 正确：使用using声明
using namespace OHOS::YourNamespace;
OH_EXPECT_RET({true}, YourClass, Method, ...);

// ❌ 错误：className带命名空间前缀
OH_EXPECT_RET({true}, OHOS::YourNamespace::YourClass, Method, ...);
```

#### 4. TDD宏定义位置

```cpp
// ✅ 正确顺序
#define private public
#define protected public

#include "your_class.h"
#include "oh_mock_utils.h"

#undef private
#undef protected
```

#### 5. 单例Mock处理

- 继承自 `DelayedSingleton<T>` 的单例：使用 `OH_MOCK_METHOD_WITH_DECORATOR`
- `DECLAR_SINGLETON` 声明的单例：继续使用 `DECLAR_SINGLETON`
- `GetInstance()` 为空的判断分支无需覆盖

### 编码规范

- **测试用例命名**：`方法名_测试场景_编号`（编号从001开始连续计数）
- **行宽**：不超过120字符
- **缩进**：4个空格（禁止Tab）
- **大括号**：K&R风格
- **if/for/while**：必须使用大括号
- **License年份**：必须是创建或修改年份

### 编译和运行

```bash
# 构建特定TDD测试
./build.sh --product-name <product> --build-target <test_name>

# 运行测试
run -t UT -ts <test_name>

# 构建所有TDD测试
./build.sh --product-name <product> --build-target ability_runtime_test

# 运行所有TDD测试
run -t UT -tp ability_runtime
```

### 依赖

**外部依赖**（直接添加）：
```json
external_deps = [
  "ability_base:configuration",
  "ability_base:want",
  "ability_base:zuri",
  "c_utils:utils",
  "hilog:libhilog",
  "hisysevent:libhisysevent",
  "hitrace:hitrace_meter",
  "c_utils:utils",
  "ffrt:libffrt",
  "googletest:gmock_main",
  "googletest:gtest_main",
]
```

**内部依赖**（使用copy）：
```gn
copy("media_permission_manager_copy") {
  sources = [
    "${ability_runtime_path}/services/uripermmgr/src/media_permission_manager.cpp",
    "${ability_runtime_path}/services/uripermmgr/include/media_permission_manager.h",
  ]
  outputs = [ "${target_gen_dir}/services/uripermmgr/src/{{source_file_part}}" ]
}

注意：ability_manager_errors.h引用路径：
"${ability_runtime_path}/interfaces/inner_api/ability_manager/include/ability_manager_errors.h",
```

### 参考示例

- `test/new_test/unit_test/uripermmgr_unit_test/batch_uri_test` - 基础Mock用法
- `test/new_test/unit_test/uripermmgr_unit_test/media_permission_manager_test` - 输出参数Mock
- `test/new_test/unit_test/uripermmgr_unit_test/tokenid_permission_test` - 简单bool返回
- `test/new_test/unit_test/uripermmgr_unit_test/file_uri_distribution_utils_test` - 多个Mock协同

### 完整文档

详细使用指南：`test/new_test/mock_utils/oh_mock_utils_user_guide.md`
