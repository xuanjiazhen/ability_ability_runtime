
## 任务

补齐uripermmgr模块TDD用例

### 待覆盖TDD文件范围
file_uri_distribution_utils.cpp
tokenid_permission.cpp
media_permission_manager.cpp
batch_uri.cpp

### 已有TDD用例文件位置
test\unittest\uri_perm_mgr_test\
test\unittest\file_uri_distribution_utils_test
test\unittest\uri_permission_impl_test


### 目标输出路径
test\new_test\unit_test\uripermmgr_unit_test

### 要求
1. 使用new_test中 oh_mock_utils.h的测试框架来Mock依赖
2. 所有依赖的模块都必须mock, mock文件保存在 test\new_test\unit_test\uripermmgr_unit_test\mock路径下
3. TDD分支覆盖率必须达到80%

### 注意事项
新增加的文件，License时间为2026或XXX-2026.


### 以下内容忽略

问题1： 编译报错，找不到kvstore请参考test\unittest\uri_permission_impl_test\BUILD.gn依赖文件，再校验一下test\new_test\unit_test\uripermmgr_unit_test\BUILD.gn
问题2： 需要同步到黄区极速空间编译
问题3： 我进行了编译尝试，发现许多编译错误。希望你再深入分析4个需要补充用例的cpp文件，缺失的依赖需要通过mock来实现。给出你的计划
问题4： 最好告诉模型，对外部的依赖。简单的数据结构，依赖对应so即可:
```json
"ability_base:configuration",
"ability_base:want",
"ability_base:zuri",
"c_utils:utils",
"hilog:libhilog",
"hisysevent:libhisysevent",
"hitrace:hitrace_meter",
```
问题5：new_test中所有mock文件命名需要与原始头文件名称保持一致，测试用例文件不需要保存下来。
问题6：
    "bundle_framework:appexecfwk_base",
    "bundle_framework:appexecfwk_client",

问题7：编码规范，日志打印优化，缩减，空格；每行小于120个字符，函数小于50行

- **行宽**：不超过 120 个字符
- **缩进**：使用空格，每次缩进 4 个空格（**禁止使用 Tab**）
- **大括号风格**：K&R 风格
- **if/for/while 必须使用大括号**，即使只有一条语句
- **禁止 if/else/else if 写在同一行**
- 较长表达式换行时，**运算符放行末**
- **头文件排列顺序**：按照字典序进行排列
- **License年份**：必须是文件创建或修改的年份（如 `Copyright (c) 2023-2026`）


问题8：需要oh_mock.h中增加静态方法、单例的mock方法。默认参数的方法mock

问题10：mock 单例PermissionVerification::GetInstance()->VerifyPermissionByTokenId


问题9：在编写新的tdd之前，最好把所有相关的依赖给列出来。头文件中如果依赖宏定义，需要添加相应的文件

问题11：修改media_permission_manager_test用例，使得能够编译通过，按如下步骤

1. 分析 media_permission_manager.cpp 中
#include "iservice_registry.h"
#include "system_ability_definition.h"是否必要，若不必要则删除

2. 依赖头文件处理策略：
ability_manager_errors.h   ： media_permission_manager_copy
include "in_process_call_wrapper.h ： media_permission_manager_copy
file_uri_distribution_utils.h

3. InitMediaPermissionHelper 方法是void, 不需要用 OH_MOCK_METHOD来mock


12. TDD宏定义
#define private public
#define protected public

依赖的头文件

#undef private
#undef protected


13. 带有出参的方法
MediaPermissionHelper中的CheckPhotoUriPermission为带有出参的方法，其中results为出参数，flags不是出参。
函数定义为：int32_t CheckPhotoUriPermission(uint32_t callerTokenId, const std::vector<std::string>& uriVec, std::vector<bool>& results, const std::vector<uint32_t>& flags)
调用方法：
mediaPermissionHelper->CheckPhotoUriPermission(callerTokenId, uriVec, results, flags)
请用oh_mock_utils.h中的方法重新mock一下，注意：如果没有直接可以用的Mock宏请重新写一个。

注意opParam需要在对应参数位置写好参数名，否则编译报错找不到opParam，使用示例：
OH_MOCK_METHOD_WITH_OUTPUT_1(int32_t, results, MediaPermissionHelper, CheckPhotoUriPermission,
    uint32_t, const std::vector<std::string>&, std::vector<bool>& results, const std::vector<uint32_t>&);

注意：使用MOCK宏设置预期结果时，如OH_EXPECT_RET_AND_OUTPUT，1. 先定义临时变量如： auto expectResults = std::vector<std::vector<bool>>{{true, true}} 而不是直接使用 {{ true, true }}传入宏中。2. 出参需要带变量 3. 出参结果为空时也需要定义临时变量如： auto expectResults = std::vector<std::vector<bool>>(); 4. expectRet和expectResult的size要一致 5. expectRet也需要为变量

接口在mock过程中一个mock方法涉及多次调用：需要将预期结果放在列表中，而不是多次执行mock方法。

```cpp
HapTokenInfo hapInfo1;
hapInfo1.bundleName = "com.example.caller";

HapTokenInfo hapInfo2;
hapInfo2.bundleName = "com.example.target";

expectHapInfos = std::vector<std::vector<HapTokenInfo>>{{hapInfo1, hapInfo2}};
expectRets =  std::vector<int32_t>{static_cast<int>(AccessTokenKitRet::RET_SUCCESS), static_cast<int>(AccessTokenKitRet::RET_SUCCESS)};
OH_EXPECT_RET_AND_OUTPUT(expectRets, expectHapInfo2, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo& hapInfo);

```

14. tdd用例编号格式：方法名_测试场景_编号，对每个 方法名_测试场景_，编号都从001开始连续计数

15. 使用ohos_mock宏定义时，className不需要加命名空间前缀，否则可能导致Key不一致从而无法获取预期值。在test文件中，使用using引用涉及的命名空间。

16. 给oh_mock_utils.h增加测试用例并补全功能

17. 常见问题：
- 预期ret和预期output不是临时变量，而是{{xxx}}或{}或{true}; 
- 带出参的方法，mock方法或Mock预期结果时没有在出参类型后面加变量名；
- Mock方法和Mock预期结果参数类型不匹配
- 同一个方法在testcase中涉及多次调用：仅需要mock一次预期结果，将多次调用的预期结果放在预期结果列表中
- 需要确保Mock方法定义与MOCk预期结果时要完全一致。
- 默认参数的使用方法：OH_MOCK_FUNCTION_WITH_DEFAULT_PARAM_BODY


基于以上规则生层新的tdd用例:
upms_udmf_utils.cpp和file_permission_manager.cpp


====== tdd skill
---
name: new_test-tdd
description: 关于如何在test\new_test路径下生成tdd用例。
license: Apache-2.0
metadata:
  version: "1.0.0"
  author: agno-team
  tags: ["mock", "new_test", "tdd"]
---
### 任务

对一个新的cpp文件在new_test文件夹中生成tdd用例。

### 步骤
1. 找到待测试cpp文件的内部依赖和外部依赖。
2. 分析依赖是否需要mock, 已有的mock文件在 test\new_test\mock目录。
3. 开始编写测试用例
4. 检查测试用例是否符合预期

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
- **头文件排列顺序**：按照字典序进行排列
- **License年份**：必须是文件创建或修改的年份（如 `Copyright (c) 2023-2026`）
- **命名空间**：在test相关文件中使用using引入用到命名空间。
- **GN文件格式**：gn文件中的依赖文件按照字母序排列

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

4. 被mock的类如果是 DECLAR_SINGLETON 声明的单例，则继续使用DECLAR_SINGLETON来声明单例。判断GetInstance()是否为空的分支无需覆盖。

### 示例

参考已有的实现方法：
test\new_test\unit_test\uripermmgr_unit_test\batch_uri_test
test\new_test\unit_test\uripermmgr_unit_test\media_permission_manager_test
test\new_test\unit_test\uripermmgr_unit_test\tokenid_permission_test