# PR Template Format

This document describes the format and structure of PR templates used in GitCode automation.

## Template Configuration File

Templates are stored in `~/.claude/pr_templates.json`:

```json
{
  "openharmony/ability_ability_runtime": {
    "template_name": "ability_runtime PR Template",
    "last_updated": "2026-01-24",
    "template": "**IssueNo**:\\n{issue_no}\\n\\n**Description**:\\n{description}\\n\\n**稳定性自检：**\\n| 自检项 | 自检结果 |\\n|------------|--------|\\n..."
  }
}
```

## Template Placeholders

Templates support these placeholders:

- `{issue_no}`: Issue URL (e.g., `https://gitcode.com/openharmony/ability_ability_runtime/issues/14320`)
- `{description}`: PR description text
- `{tdd_result}`: TDD test results (usually empty)
- `{xts_result}`: XTS test results (usually empty)

## Template Structure

### Basic Template

```markdown
**IssueNo**:
{issue_no}

**Description**:
{description}

**TDD Result**:
{tdd_result}

**XTS Result**:
{xts_result}

### 是否已执行L0用例
- [ ] 已验证
- [ ] 不涉及
```

### Complete Template with Checklists

The ability_runtime template includes stability and security checklists:

```markdown
**IssueNo**:
{issue_no}

**Description**:
{description}

**稳定性自检：**
| 自检项                                                       | 自检结果  |
| ------------------------------------------------------------ | -------- |
| 涉及跨进程调用的相关操作需要抛至主线程或加锁防止并发              |          |
| 成员变量进行赋值或创建需要排查并发                               |          |
| 谨慎在lambda表达式中使用引用捕获                                |          |
| 谨慎在未经拷贝的情况下使用外部传入的string、C字符串               |          |
| map\vector\list\set等stl模板类使用时需要排查并发                |          |
| 谨慎考虑加锁范围                                               |          |
| 在IPC通信中谨慎使用同步通信方式                                 |          |
| 禁止传递this指针至其他模块或线程（特别是eventhandler任务）        |          |
| 禁止将外部传入的裸指针在内部直接构造智能指针                      |          |
| 禁止多个独立创建的智能指针管理同一地址                           |          |
| 禁止在析构函数中抛异步任务                                      |          |
| 禁止js对象在非js线程（例如在IPC线程）创建、使用或销毁             |          |
| 禁止在对外接口中未经判空直接使用外部传入的指针                    |          |
| 禁止接口返回局部变量引用                                        |          |
| 禁止在信号函数中加锁                                            |          |
| 禁止在关键流程（SA启动、应用启动等主流程）执行耗时的操作           |          |
| 禁止将同一个cpp编译在不同的so中                                 |          |

**安全编码自检：**
| 自检项                                                          | 自检结果 |
| -------------------------------------------------------------- | -------- |
| 裸指针避免通过隐式转换构造为sptr                                 |          |
| json对象在取值之前必须先判断类型，避免类型不匹配                   |          |
| 序列化时必须对传入的数组大小进行校验，避免出现超大数组              |          |
| 避免使用未明确位宽的整型，选择使用int8_t、uint8_t等类型            |          |
| 外部传入的路径要做规范化校验，对路径中的.、..、../等特殊字符严格校验 |          |
| 指针变量、表示资源描述符的变量、bool变量必须赋初值                  |          |
| readParcelable获取的对象使用前需要判空                            |          |
| 分配和释放内存的函数需要成对出现                                   |          |
| 申请内存后异常退出前需要及时进行内存释放                            |          |
| 内存申请前必须对内存大小进行合法性校验                              |          |
| 内存分配后必须判断是否成功                                         |          |
| 禁止使用realloc、alloca函数                                       |          |
| 禁止打印文件路径、口令等敏感信息，如有需要，使用private修饰          |          |
| 禁止打印内存地址                                                  |          |
| 整数之间运算时必须严格检查，确保不会出现溢出、反转、除0               |          |
| 禁止对有符号整数进行位操作符运算                                    |          |
| 禁止对指针进行逻辑或位运算                                         |          |
| 循环次数如果收外部数据控制，需要检验其合法性                         |          |
| 禁止使用内存操作类危险函数，需要使用安全函数                         |          |
| 谨慎使用不可重入函数                                               |          |
| 必须检查安全函数的返回值，并进行正确处理                             |          |
| 禁止仅通过TokenType类型判断绕过权限校验                             |          |

**TDD Result**:
{tdd_result}

**XTS Result**:
{xts_result}

### 是否已执行L0用例
- [ ] 已验证
- [ ] 不涉及
```

## Checklist Values

When filling checklists based on code analysis, use these values:

- **"需要检查"** (Need Check): When the code change involves the relevant pattern
- **"不涉及"** (Not Applicable): When the code change doesn't involve the pattern
- **"√"**: Passed checks (rarely used, only when manually verified)

## Template Filling Logic

The automation script fills checklists based on code analysis:

### Stability Checklist (18 items)

Triggers:
- **IPC/Threading**: `has_ipc` or `has_threading` → "需要检查"
- **Pointers**: `has_pointer` → "需要检查"
- **C++ STL**: `has_cpp` → "需要检查"
- **JS operations**: `has_js` → "需要检查"
- **File operations**: `has_file_ops` → "需要检查"

### Security Checklist (21 items)

Triggers:
- **Pointers**: `has_pointer` → "需要检查"
- **JSON**: `has_json` → "需要检查"
- **Memory operations**: `has_memory` → "需要检查"
- **File operations**: `has_file_ops` → "需要检查"
- **Thread safety**: `has_threading` → "需要检查"
- **C++/Java**: `has_cpp` or `has_java` → "需要检查"

## Adding New Repository Templates

To add a template for a new repository:

1. Copy the existing template from the repository
2. Save to `~/.claude/pr_templates.json`
3. Use repository path as key (e.g., `openharmony/your_repo`)
4. Include `template_name` and `last_updated` fields

Example:
```json
{
  "openharmony/your_new_repo": {
    "template_name": "Your Repo Template",
    "last_updated": "2026-01-26",
    "template": "**IssueNo**:\\n{issue_no}\\n\\n..."
  }
}
```

## Default Template

If no template is found for a repository, use this default:

```markdown
**IssueNo**:
{issue_no}

**Description**:
{description}

**Changes:**
- Summary of changes

**Testing:**
- Test results

**Reviewers:**
- @reviewer1
- @reviewer2
```
