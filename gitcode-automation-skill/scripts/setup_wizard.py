#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
GitCode自动化配置向导

首次使用时引导用户配置必要的信息：
1. GitCode访问令牌
2. Git用户信息（用于DCO签名）
3. PR模板（可选）
"""

import json
import os
import sys
from pathlib import Path


def print_header(text):
    """打印标题"""
    print(f"\n{'='*70}")
    print(f"  {text}")
    print(f"{'='*70}\n")


def print_step(step, text):
    """打印步骤"""
    print(f"\n[步骤 {step}] {text}")
    print("-" * 70)


def get_config_dir():
    """获取配置目录"""
    return Path.home() / '.claude'


def setup_token():
    """配置GitCode访问令牌"""
    print_step(1, "配置GitCode访问令牌")

    print("GitCode访问令牌用于API调用（创建PR等操作）")
    print("\n获取令牌步骤：")
    print("  1. 访问: https://gitcode.com/-/settings/personal_access_tokens")
    print("  2. 点击「新建令牌」")
    print("  3. 勾选必要的权限（repo, write_repository）")
    print("  4. 复制生成的令牌")

    token_file = get_config_dir() / 'gitcode_token.json'

    # 检查是否已存在
    if token_file.exists():
        with open(token_file, 'r') as f:
            data = json.load(f)
            existing_token = data.get('token', '')
            if existing_token:
                print(f"\n[检测到] 已存在令牌: {existing_token[:10]}...{existing_token[-4:]}")
                choice = input("\n是否重新配置？(y/N): ").strip().lower()
                if choice != 'y':
                    print("[跳过] 使用现有令牌")
                    return True

    while True:
        token = input("\n请输入GitCode访问令牌: ").strip()

        if not token:
            print("[错误] 令牌不能为空，请重新输入")
            continue

        if len(token) < 20:
            print("[警告] 令牌长度似乎太短，请确认是否正确")
            confirm = input("是否继续？(y/N): ").strip().lower()
            if confirm != 'y':
                continue

        # 保存令牌
        token_data = {"token": token}
        with open(token_file, 'w', encoding='utf-8') as f:
            json.dump(token_data, f, indent=2, ensure_ascii=False)

        print(f"\n[成功] 令牌已保存到: {token_file}")
        return True


def setup_user_info():
    """配置Git用户信息（用于DCO签名）"""
    print_step(2, "配置Git用户信息（DCO签名）")

    print("DCO（Developer Certificate of Origin）签名格式：")
    print("  Signed-off-by: Your Name <your.email@example.com>")

    creds_file = get_config_dir() / 'gitcode.json'

    # 检查是否已存在
    if creds_file.exists():
        with open(creds_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
            existing_username = data.get('username', '')
            existing_email = data.get('email', '')
            if existing_username and existing_email:
                print(f"\n[检测到] 已存在配置:")
                print(f"  用户名: {existing_username}")
                print(f"  邮箱: {existing_email}")
                choice = input("\n是否重新配置？(y/N): ").strip().lower()
                if choice != 'y':
                    print("[跳过] 使用现有配置")
                    return True

    while True:
        username = input("\n请输入GitCode用户名: ").strip()

        if not username:
            print("[错误] 用户名不能为空，请重新输入")
            continue

        email = input("请输入邮箱地址: ").strip()

        if not email:
            print("[错误] 邮箱不能为空，请重新输入")
            continue

        if '@' not in email:
            print("[警告] 邮箱格式似乎不正确")
            confirm = input("是否继续？(y/N): ").strip().lower()
            if confirm != 'y':
                continue

        # 保存配置
        user_data = {
            "username": username,
            "email": email
        }
        with open(creds_file, 'w', encoding='utf-8') as f:
            json.dump(user_data, f, indent=2, ensure_ascii=False)

        print(f"\n[成功] 用户信息已保存到: {creds_file}")
        print(f"\nDCO签名将为:")
        print(f"  Signed-off-by: {username} <{email}>")
        return True


def setup_pr_templates():
    """配置PR模板（可选）"""
    print_step(3, "配置PR模板（可选）")

    template_file = get_config_dir() / 'pr_templates.json'

    if template_file.exists():
        with open(template_file, 'r', encoding='utf-8') as f:
            templates = json.load(f)
            if templates:
                print(f"\n[检测到] 已存在 {len(templates)} 个PR模板")
                for repo_key in templates.keys():
                    print(f"  - {repo_key}")
                choice = input("\n是否添加新的模板？(y/N): ").strip().lower()
                if choice != 'y':
                    print("[跳过] 使用现有模板")
                    return True

    print("\nPR模板用于创建Pull Request时自动填充描述内容")
    print("如果不配置，将使用默认模板")

    choice = input("\n是否现在配置PR模板？(y/N): ").strip().lower()

    if choice != 'y':
        print("[跳过] 将使用默认模板")

        # 创建一个空模板文件
        with open(template_file, 'w', encoding='utf-8') as f:
            json.dump({}, f, indent=2, ensure_ascii=False)

        return True

    # 引导用户添加模板
    templates = {}

    while True:
        print("\n" + "="*70)
        repo_key = input("请输入仓库路径（例如: openharmony/ability_ability_runtime）: ").strip()

        if not repo_key:
            print("[跳过] 结束模板配置")
            break

        print("\n请输入PR模板内容（支持占位符）：")
        print("占位符:")
        print("  {issue_no}    - Issue链接")
        print("  {description} - PR描述")
        print("  {tdd_result}  - TDD测试结果")
        print("  {xts_result}  - XTS测试结果")

        print("\n输入模板内容（以空行结束）:")
        template_lines = []
        while True:
            line = input()
            if line == '':
                break
            template_lines.append(line)

        template_content = '\n'.join(template_lines)

        if not template_content:
            print("[警告] 模板内容为空，将使用默认模板")
            template_content = get_default_template()

        templates[repo_key] = {
            "template_name": f"{repo_key.split('/')[-1]} PR Template",
            "template": template_content
        }

        print(f"\n[成功] 已添加模板: {repo_key}")

        another = input("\n是否继续添加模板？(y/N): ").strip().lower()
        if another != 'y':
            break

    # 保存模板
    with open(template_file, 'w', encoding='utf-8') as f:
        json.dump(templates, f, indent=2, ensure_ascii=False)

    print(f"\n[成功] 已保存 {len(templates)} 个PR模板到: {template_file}")
    return True


def get_default_template():
    """获取默认PR模板"""
    return """**IssueNo**:
{issue_no}

**Description**:
{description}

**稳定性自检：**
| 自检项                                                       | 自检结果  |
| ------------------------------------------------------------ | -------- |
| 涉及跨进程调用的相关操作需要抛至主线程或加锁防止并发              | 不涉及 |
| 成员变量进行赋值或创建需要排查并发                               | 不涉及 |
| 谨慎在lambda表达式中使用引用捕获                                | 不涉及 |
| 谨慎在未经拷贝的情况下使用外部传入的string、C字符串               | 不涉及 |
| map\\vector\\list\\set等stl模板类使用时需要排查并发                | 不涉及 |
| 谨慎考虑加锁范围                                               | 不涉及 |
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
| json对象在取值之前必须先判断类型，避免类型不匹配                   | 不涉及 |
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
| 禁止打印文件路径、口令等敏感信息，如有需要，使用private修饰          | 不涉及 |
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
{tdd_result}

**XTS Result**:
{xts_result}

### 是否已执行L0用例
- [ ] 已验证
- [ ] 不涉及
"""


def verify_configuration():
    """验证配置是否完整"""
    print_step(4, "验证配置")

    config_dir = get_config_dir()
    token_file = config_dir / 'gitcode_token.json'
    creds_file = config_dir / 'gitcode.json'
    template_file = config_dir / 'pr_templates.json'

    all_ok = True

    # 检查令牌
    if token_file.exists():
        with open(token_file, 'r') as f:
            data = json.load(f)
            token = data.get('token')
            if token:
                print(f"[OK] GitCode令牌已配置")
            else:
                print(f"[ERROR] 令牌文件存在但令牌为空")
                all_ok = False
    else:
        print(f"[ERROR] 令牌文件不存在: {token_file}")
        all_ok = False

    # 检查用户信息
    if creds_file.exists():
        with open(creds_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
            username = data.get('username')
            email = data.get('email')
            if username and email:
                print(f"[OK] Git用户信息已配置")
                print(f"     用户名: {username}")
                print(f"     邮箱: {email}")
            else:
                print(f"[ERROR] 用户信息不完整")
                all_ok = False
    else:
        print(f"[ERROR] 用户信息文件不存在: {creds_file}")
        all_ok = False

    # 检查模板
    if template_file.exists():
        with open(template_file, 'r', encoding='utf-8') as f:
            templates = json.load(f)
            if templates:
                print(f"[OK] PR模板已配置 ({len(templates)} 个)")
            else:
                print(f"[INFO] PR模板为空，将使用默认模板")
    else:
        print(f"[INFO] PR模板文件不存在，将使用默认模板")

    return all_ok


def main():
    """主函数"""
    print_header("GitCode自动化配置向导")

    print("欢迎使用GitCode自动化工具！")
    print("本向导将引导您完成必要的配置。")

    # 确保配置目录存在
    config_dir = get_config_dir()
    config_dir.mkdir(parents=True, exist_ok=True)

    try:
        # 步骤1: 配置令牌
        setup_token()

        # 步骤2: 配置用户信息
        setup_user_info()

        # 步骤3: 配置PR模板（可选）
        setup_pr_templates()

        # 步骤4: 验证配置
        print_header("配置完成")
        verify_configuration()

        print_header("准备就绪")
        print("配置已完成！您现在可以使用GitCode自动化工具了。")
        print("\n快速开始:")
        print("  1. 进入您的Git仓库")
        print("  2. 进行代码修改并提交")
        print("  3. 运行: python scripts/gitcode_automation.py push-pr --issue <issue_url>")
        print("\n示例:")
        print("  python scripts/gitcode_automation.py push-pr --issue \"14320\"")

        return 0

    except KeyboardInterrupt:
        print("\n\n[中断] 配置已取消")
        return 1
    except Exception as e:
        print(f"\n[错误] 配置过程中发生错误: {str(e)}")
        import traceback
        traceback.print_exc()
        return 1


if __name__ == '__main__':
    sys.exit(main())
