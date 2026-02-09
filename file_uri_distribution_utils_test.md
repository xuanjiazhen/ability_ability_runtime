
### Mock方法
mock方法在oh_mock_utils.h文件中：
- **单例类**：#include "singleton.h" ,class T:  public DelayedSingleton<T> 
- **静态方法**： OH_MOCK_METHOD_WITH_DECORATOR -> OH_EXPECT_RET
- **带有返回值的普通成员方法**：OH_MOCK_METHOD -> OH_EXPECT_RET
- **不带返回值的普通成员方法**：OH_MOCK_VOID_METHOD -> 不用Mock预期结果
- **只带有出参的普通成员方法**：OH_MOCK_VOID_METHOD_WITH_OUTPUT_1 -> OH_EXPECT_OUTPUT(暂未实现，需要时再实现)
- **带有1个出参的成员方法**：OH_MOCK_METHOD_WITH_OUTPUT_1或OH_MOCK_METHOD_WITH_OUTPUT_VECTOR -> OH_EXPECT_RET_AND_OUTPUT
- **全局方法**： OH_MOCK_GLOBAL_METHOD -> OH_GLOBAL_EXPECT_RET
- **带有默认值的方法**：OH_MOCK_FUNCTION_WITH_DEFAULT_PARAM_BODY -> OH_EXPECT_RET


### 编码规范
- **tdd用例编号格式**：方法名_测试场景_编号，对每个 方法名_测试场景_，编号都从001开始连续计数
- **行宽**：不超过 120 个字符。较长表达式换行时，运算符放行末。
- **缩进**：使用空格，每次缩进4个空格（**禁止使用 Tab**）
- **大括号风格**：K&R 风格
- **if/for/while 必须使用大括号**，即使只有一条语句
- **禁止 if/else/else if 写在同一行**
- **文件排列顺序**：头文件和gn文件依赖都按照字典序进行排列
- **License年份**：必须是文件创建或修改的年份（如 `Copyright (c) 2023-2026`）
- **命名空间**：在test相关文件中使用using引入用到命名空间。

### 依赖
- 对外部依赖的简单类，直接添加依赖，如
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
  "hilog:libhilog",
]
```
- 对内部依赖的简单类，copy头文件，如：
```GN
copy("media_permission_manager_copy") {
  sources = [
    "${ability_runtime_path}/interfaces\inner_api\ability_manager\include\ability_manager_errors.h",
    "${ability_runtime_path}/services/uripermmgr/src/media_permission_manager.cpp",
    "${ability_runtime_path}/services/uripermmgr/include/media_permission_manager.h",
    "${ability_runtime_path}/services/common/include/in_process_call_wrapper.h",
    "${ability_runtime_path}/services/common/include/hilog_tag_wrapper.h",
  ]
  outputs = [ "${target_gen_dir}/services/uripermmgr/src/{{source_file_part}}" ]
}
```

### 注意
1. 新创建的tdd测试cpp文件，增加一下宏定义
```
#define private public
#define protected public
依赖的头文件
#undef private
#undef protected
```
2. tdd用例编写完之后需要再检查2遍

3. 分支覆盖率大于80%，输出分支覆盖分析。


### 示例

参考已有的实现方法：
test\new_test\unit_test\uripermmgr_unit_test\batch_uri_test
test\new_test\unit_test\uripermmgr_unit_test\media_permission_manager_test
test\new_test\unit_test\uripermmgr_unit_test\tokenid_permission_test

