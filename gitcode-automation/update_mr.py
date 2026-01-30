#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Update MR description for GitCode"""

import json
import requests

# Configuration
token = "3Btsdq8oJAVrWwYdTudzUcu_"
repo = "openharmony/ability_ability_runtime"
mr_id = "18156"

# MR body
body = """**IssueNo**:
关联issue

**Description**:
本次提交修复了extension_config_mgr中的多个代码问题：

1. **命名歧义修复**：将变量名从`blackList`改为`blocklistConfig`，消除命名歧义
2. **代码深度优化**：重构LoadExtensionBlockList方法，将嵌套深度从5降低到3，移除不必要的双层循环
3. **日志级别恢复**：将文件打开失败的日志级别从LOGW恢复为LOGE，更准确反映错误严重性
4. **逻辑简化**：使用contains()直接检查JSON key，移除found标志变量

**稳定性自检：**
| 自检项                                                       | 自检结果  |
| ------------------------------------------------------------ | -------- |
| 涉及跨进程调用的相关操作需要抛至主线程或加锁防止并发              | 不涉及 |
| 成员变量进行赋值或创建需要排查并发                               | 不涉及 |
| 谨慎在lambda表达式中使用引用捕获                                | 不涉及 |
| 谨慎在未经拷贝的情况下使用外部传入的string、C字符串               | 不涉及 |
| map\\vector\\list\\set等stl模板类使用时需要排查并发                | 不涉及 |
| 谨慎考虑加锁范围                                               | √（保持原有加锁逻辑） |
| 在IPC通信中谨慎使用同步通信方式                                 | 不涉及 |
| 禁止传递this指针至其他模块或线程（特别是eventhandler任务）        | 不涉及 |
| 禁止将外部传入的裸指针在内部直接构造智能指针                      | 不涉及 |
| 禁止多个独立创建的智能指针管理同一地址                           | 不涉及 |
| 禁止在析构函数中抛异步任务                                      | 不涉及 |
| 禁止js对象在非js线程（例如在IPC线程）创建、使用或销毁             | 不涉及 |
| 禁止在对外接口中未经判空直接使用外部传入的指针                    | 不涉及 |
| 禁止接口返回局部变量引用                                        | 不涉及 |
| 禁止在信号函数中加锁                                            | 不涉及 |
| 禁止在关键流程（SA启动、应用启动等主流程）执行耗时的操作           | 不涉及 |
| 禁止将同一个cpp编译在不同的so中                                 | 不涉及 |

**安全编码自检：**
| 自检项                                                          | 自检结果 |
| -------------------------------------------------------------- | -------- |
| 裸指针避免通过隐式转换构造为sptr                                 | 不涉及 |
| json对象在取值之前必须先判断类型，避免类型不匹配                   | √（使用contains()和is_array()检查） |
| 序列化时必须对传入的数组大小进行校验，避免出现超大数组              | 不涉及 |
| 避免使用未明确位宽的整型，选择使用int8_t、uint8_t等类型            | 不涉及 |
| 外部传入的路径要做规范化校验，对路径中的.、..、../等特殊字符严格校验 | 不涉及 |
| 指针变量、表示资源描述符的变量、bool变量必须赋初值                  | 不涉及 |
| readParcelable获取的对象使用前需要判空                            | 不涉及 |
| 分配和释放内存的函数需要成对出现                                   | 不涉及 |
| 申请内存后异常退出前需要及时进行内存释放                            | 不涉及 |
| 内存申请前必须对内存大小进行合法性校验                              | 不涉及 |
| 内存分配后必须判断是否成功                                         | 不涉及 |
| 禁止使用realloc、alloca函数                                       | 不涉及 |
| 禁止打印文件路径、口令等敏感信息，如有需要，使用private修饰          | √（使用%{public}s修饰符） |
| 禁止打印内存地址                                                  | 不涉及 |
| 整数之间运算时必须严格检查，确保不会出现溢出、反转、除0               | 不涉及 |
| 禁止对有符号整数进行位操作符运算                                    | 不涉及 |
| 禁止对指针进行逻辑或位运算                                         | 不涉及 |
| 循环次数如果收外部数据控制，需要检验其合法性                         | 不涉及 |
| 禁止使用内存操作类危险函数，需要使用安全函数                         | 不涉及 |
| 谨慎使用不可重入函数                                               | 不涉及 |
| 必须检查安全函数的返回值，并进行正确处理                             | 不涉及 |
| 禁止仅通过TokenType类型判断绕过权限校验                             | 不涉及 |

**TDD Result**:
已有单元测试覆盖，测试文件：test/unittest/extension_config_mgr_test/extension_config_mgr_test.cpp

**XTS Result**:
不涉及

### 是否已执行L0用例
- [x] 已验证（已有单元测试覆盖LoadExtensionBlockList方法）"""

# Update MR
url = f"https://gitcode.com/api/v5/repos/{repo}/merge_requests/{mr_id}"
headers = {
    "Authorization": f"Bearer {token}",
    "Content-Type": "application/json"
}

data = {
    "description": body
}

response = requests.put(url, headers=headers, json=data)
print(f"Status Code: {response.status_code}")
print(f"Response: {response.text[:500]}")

if response.status_code == 200:
    print(f"\nMR {mr_id} updated successfully!")
    print(f"URL: https://gitcode.com/openharmony/ability_ability_runtime/merge_requests/{mr_id}")
