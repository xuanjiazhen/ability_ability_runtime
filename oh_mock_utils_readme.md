

### 编码规范
- **tdd用例编号格式**：方法名_测试场景_编号，对每个 方法名_测试场景_，编号都从001开始连续计数
- **行宽**：不超过 120 个字符。较长表达式换行时，运算符放行末。
- **缩进**：使用空格，每次缩进4个空格（**禁止使用 Tab**）
- **大括号风格**：K&R 风格; if/for/while 必须使用大括号，即使只有一条语句;禁止 if/else/else if 写在同一行
- **头文件排列顺序**：按照字典序进行排列
- **License年份**：必须是文件创建或修改的年份（如 `Copyright (c) 2023-2026`）


### 注意事项
1. **命名空间**: 使用ohos_mock宏定义时，className不需要加命名空间前缀，否则可能导致Key不一致从而无法获取预期值。在test文件中，使用using引用涉及的命名空间。
2. **TDD宏定义**: 
```cpp
#define private public
#define protected public

依赖的头文件

#undef private
#undef protected
```
3. **mock方法和mock预期返回值**：返回值、预期出参的参数类型要一致。

4. **mock预期返回值**： 预期ret和预期output需要传入临时变量，而不是{{xxx}}或{}或{true}; 

例如：OH_EXPECT_RET_AND_OUTPUT，
a. 定义临时变量如： auto expectResults = std::vector<std::vector<bool>>{{true, true}}；auto expectRets = {true, true};然后将expectRets和expectResults传入H_EXPECT_RET_AND_OUTPUT中。
b. 出参需要带变量名与mock方法一致
c. 出参结果为空时也需要定义临时变量如： auto expectResults = std::vector<std::vector<bool>>();
d. expectRet和expectResult的size要一致 

5. **带有出参的方法**：opParam需要在对应参数位置写好参数名，否则编译报错找不到opParam。使用示例：
OH_MOCK_METHOD_WITH_OUTPUT_1(int32_t, results, MediaPermissionHelper, CheckPhotoUriPermission,
    uint32_t, const std::vector<std::string>&, std::vector<bool>& results, const std::vector<uint32_t>&);

6. 接口在mock过程中一个mock方法涉及多次调用：需要将预期结果放在列表中，而不是多次执行mock方法。

7. 单例mock方法：a) 使用"singleton.h"定义，参考test\new_test\mock\bms\bundle_mgr_client.h b) 定义GetInstance()方法， test\new_test\mock\udmf\udmf_client.h


### 目标输出路径
test\new_test\unit_test

### 案例
test\new_test\unit_test\uripermmgr_unit_test