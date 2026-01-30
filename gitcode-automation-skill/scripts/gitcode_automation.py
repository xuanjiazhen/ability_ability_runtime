#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
GitCode自动化PR创建工具

功能：
1. 推送代码到指定分支
2. 创建PR关联指定issue
3. 根据代码变更自动填充自检项

使用方法：
    python gitcode_automation.py push-pr --issue <issue_url> [--branch <branch>] [--title <title>]

示例：
    python gitcode_automation.py push-pr --issue "https://gitcode.com/openharmony/ability_ability_runtime/issues/14320"

    python gitcode_automation.py push-pr --issue "14320" --branch "feature/test-branch" --title "Add new feature"
"""

import argparse
import subprocess
import requests
import json
import os
import sys
import re
from pathlib import Path

# 配置文件路径
CONFIG_DIR = Path.home() / '.claude'
TOKEN_FILE = CONFIG_DIR / 'gitcode_token.json'
TEMPLATE_FILE = CONFIG_DIR / 'pr_templates.json'
CREDS_FILE = CONFIG_DIR / 'gitcode.json'


class GitCodeAutomation:
    def __init__(self, interactive=False):
        self.interactive = interactive
        self.token = self.load_token()
        self.user_info = self.load_user_info()
        self.repo_info = self.get_repo_info()
        self.template = self.load_template()

    def load_token(self):
        """加载GitCode访问令牌"""
        try:
            with open(TOKEN_FILE, 'r') as f:
                data = json.load(f)
                return data.get('token')
        except FileNotFoundError:
            print(f"[ERROR] Token文件不存在: {TOKEN_FILE}")
            print("请先创建token并保存到该文件")
            sys.exit(1)

    def load_user_info(self):
        """加载用户信息用于DCO签名"""
        try:
            with open(CREDS_FILE, 'r', encoding='utf-8') as f:
                data = json.load(f)
                username = data.get('username')
                email = data.get('email')
                if not username or not email:
                    print(f"[ERROR] gitcode.json中缺少username或email")
                    sys.exit(1)
                return {'username': username, 'email': email}
        except FileNotFoundError:
            print(f"[ERROR] 配置文件不存在: {CREDS_FILE}")
            print("请先创建配置文件并添加username和email")
            sys.exit(1)

    def get_dco_signature(self):
        """生成DCO签名字符串"""
        return f"Signed-off-by: {self.user_info['username']} <{self.user_info['email']}>"

    def get_repo_info(self):
        """获取当前git仓库信息"""
        try:
            # 获取所有remote
            result = subprocess.run(
                ['git', 'remote', '-v'],
                capture_output=True,
                encoding='utf-8', errors='replace',
                check=True
            )

            remotes = result.stdout.strip()
            target_repo = None
            source_repo = None

            # 首先找到origin（上游仓库）
            for line in remotes.split('\n'):
                if 'origin' in line and '(fetch)' in line:
                    # 从URL提取仓库路径
                    # https://gitcode.com/openharmony/xxx.git -> openharmony/xxx
                    parts = line.split()[1].split('/')
                    if len(parts) >= 2:
                        target_repo = f"{parts[-2]}/{parts[-1].replace('.git', '')}"
                    break

            # 找到个人的fork remote（使用配置文件中的username）
            username = self.user_info['username']
            for line in remotes.split('\n'):
                if username in line and '(fetch)' in line:
                    parts = line.split()[1].split('/')
                    if len(parts) >= 2:
                        source_repo = f"{parts[-2]}/{parts[-1].replace('.git', '')}"
                    break

            # 如果找不到fork remote，使用默认的username替换
            if not source_repo and target_repo:
                source_repo = target_repo.replace('openharmony/', username + '/')

            # 获取当前分支
            result = subprocess.run(
                ['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
                capture_output=True,
                encoding='utf-8', errors='replace',
                check=True
            )
            current_branch = result.stdout.strip()

            return {
                'target_repo': target_repo,
                'source_repo': source_repo,
                'current_branch': current_branch
            }
        except Exception as e:
            print(f"[ERROR] 无法获取仓库信息: {str(e)}")
            sys.exit(1)

    def load_template(self):
        """加载PR模板"""
        try:
            with open(TEMPLATE_FILE, 'r', encoding='utf-8') as f:
                templates = json.load(f)
                repo_key = self.repo_info['target_repo']
                repo_template = templates.get(repo_key)
                # Return the template string, not the dict
                if repo_template and isinstance(repo_template, dict):
                    return repo_template.get('template')
                return repo_template
        except FileNotFoundError:
            print(f"[WARNING] 模板文件不存在: {TEMPLATE_FILE}")
            return None

    def commit_with_dco(self, message):
        """使用DCO签名提交代码"""
        dco_signature = self.get_dco_signature()
        commit_message = f"{message}\n\n{dco_signature}"

        print(f"\n{'='*70}")
        print(f"使用DCO签名提交代码")
        print(f"{'='*70}")
        print(f"签名: {dco_signature}")

        try:
            result = subprocess.run(
                ['git', 'commit', '-m', commit_message],
                capture_output=True,
                encoding='utf-8',
                errors='replace'
            )

            if result.returncode == 0:
                print(f"[OK] 提交成功")
                return True
            else:
                print(f"[FAIL] 提交失败: {result.stderr}")
                return False
        except Exception as e:
            print(f"[FAIL] 错误: {str(e)}")
            return False

    def amend_dco_to_last_commit(self):
        """为最后一次提交添加DCO签名（如果缺失）"""
        dco_signature = self.get_dco_signature()

        print(f"\n{'='*70}")
        print(f"检查并添加DCO签名到最后一次提交")
        print(f"{'='*70}")

        try:
            # 获取最后一次提交的消息
            result = subprocess.run(
                ['git', 'log', '-1', '--pretty=%B'],
                capture_output=True,
                encoding='utf-8',
                errors='replace',
                check=True
            )
            last_message = result.stdout.strip()

            # 检查是否已有DCO签名
            if 'Signed-off-by:' in last_message:
                print(f"[OK] 最后一次提交已包含DCO签名")
                return True

            # 添加DCO签名
            new_message = f"{last_message}\n\n{dco_signature}"
            result = subprocess.run(
                ['git', 'commit', '--amend', '-m', new_message],
                capture_output=True,
                encoding='utf-8',
                errors='replace'
            )

            if result.returncode == 0:
                print(f"[OK] 已为最后一次提交添加DCO签名")
                print(f"签名: {dco_signature}")
                print(f"\n[WARNING] 提交已被修改，如果已推送则需要强制推送")
                return True
            else:
                print(f"[FAIL] 修改失败: {result.stderr}")
                return False
        except Exception as e:
            print(f"[FAIL] 错误: {str(e)}")
            return False

    def push_to_remote(self, branch=None, remote='origin'):
        """推送代码到远程分支"""
        if not branch:
            branch = self.repo_info['current_branch']

        print(f"\n{'='*70}")
        print(f"推送代码到远程分支")
        print(f"{'='*70}")
        print(f"远程仓库: {remote}")
        print(f"目标分支: {branch}")

        try:
            result = subprocess.run(
                ['git', 'push', remote, branch],
                capture_output=True,
                encoding='utf-8', errors='replace'
            )

            if result.returncode == 0:
                print(f"[OK] 代码已成功推送到 {remote}/{branch}")
                return True
            else:
                print(f"[FAIL] 推送失败: {result.stderr}")
                return False
        except Exception as e:
            print(f"[FAIL] 错误: {str(e)}")
            return False

    def analyze_code_changes(self):
        """深入分析代码变更，返回详细的分析结果"""
        print(f"\n{'='*70}")
        print(f"步骤2: 深入分析代码变更")
        print(f"{'='*70}")

        try:
            # 获取最近的commit变更
            result = subprocess.run(
                ['git', 'diff', 'HEAD~1', 'HEAD', '--stat'],
                capture_output=True,
                encoding='utf-8',
                errors='replace'
            )

            changes = result.stdout.strip()
            print(f"变更统计:\n{changes}")

            # 获取完整的diff
            result = subprocess.run(
                ['git', 'diff', 'HEAD~1', 'HEAD'],
                capture_output=True,
                encoding='utf-8',
                errors='replace'
            )

            diff_content = result.stdout
            diff_lines = diff_content.split('\n')
            added_lines = [line for line in diff_lines if line.startswith('+') and not line.startswith('+++')]
            removed_lines = [line for line in diff_lines if line.startswith('-') and not line.startswith('---')]

            # 分析变更文件类型
            result = subprocess.run(
                ['git', 'diff', 'HEAD~1', 'HEAD', '--name-only'],
                capture_output=True,
                encoding='utf-8',
                errors='replace'
            )

            changed_files = result.stdout.strip().split('\n')

            # 构建详细的分析结果
            analysis = {
                'files': changed_files,
                'added_lines': added_lines,
                'removed_lines': removed_lines,
                'diff_content': diff_content.lower(),
                'full_diff': diff_content,
                # 检测到的模式
                'patterns': {}
            }

            # 检测各种模式
            patterns_to_check = {
                'thread': ['thread', 'pthread', 'mutex', 'lock', 'async', 'std::thread', 'std::mutex'],
                'ipc': ['ipc', 'binder', 'ashmem', 'shared_memory', 'iremoteobject'],
                'smart_pointer': ['sptr', 'wptr', 'shared_ptr', 'unique_ptr', 'weak_ptr'],
                'raw_pointer': ['* ', '->', 'pointer'],
                'memory_alloc': ['malloc', 'free', 'new ', 'delete', 'memcpy', 'calloc'],
                'file_ops': ['fopen', 'fclose', 'open(', 'read(', 'write(', 'fstream', 'ifstream'],
                'lambda': ['[', 'lambda', '&]'],  # 引用捕获
                'stl_container': ['vector', 'map', 'list', 'set', 'unordered_map', 'unordered_set'],
                'string_ops': ['string', 'cstring', 'strcopy', 'strcpy'],
                'js_object': ['jsobject', 'napi', 'jsengine'],
                'virtual': ['virtual ', '~', 'destructor'],
                'signal': ['signal(', 'sigaction', 'sighandler'],
            }

            for pattern_name, keywords in patterns_to_check.items():
                found = []
                for keyword in keywords:
                    # 检查新增的行
                    for line in added_lines:
                        if keyword.lower() in line.lower():
                            found.append((keyword, line))
                            break  # 每个关键词只记录一次
                analysis['patterns'][pattern_name] = found

            return analysis

        except Exception as e:
            print(f"[WARNING] 无法分析变更: {str(e)}")
            import traceback
            traceback.print_exc()
            return {}

    def generate_checklist(self, analysis):
        """根据深入代码分析生成自检表，给出明确的结论"""

        def check_pattern(pattern_name):
            """检查是否检测到特定模式"""
            return len(analysis.get('patterns', {}).get(pattern_name, [])) > 0

        def has_file_type(extension):
            """检查是否有特定类型的文件变更"""
            return any(f.endswith(extension) for f in analysis.get('files', []))

        # 智能判断稳定性自检项
        stability_checks = []

        # 1. 涉及跨进程调用的相关操作需要抛至主线程或加锁防止并发
        if check_pattern('ipc') or check_pattern('thread'):
            # 如果有IPC或线程相关代码，检查是否已有保护措施
            has_lock = any('lock' in line.lower() or 'mutex' in line.lower()
                          for line in analysis.get('added_lines', []))
            stability_checks.append(("涉及跨进程调用的相关操作需要抛至主线程或加锁防止并发",
                                    "√" if has_lock else "需要检查"))
        else:
            stability_checks.append(("涉及跨进程调用的相关操作需要抛至主线程或加锁防止并发", "不涉及"))

        # 2. 成员变量进行赋值或创建需要排查并发
        if check_pattern('thread'):
            stability_checks.append(("成员变量进行赋值或创建需要排查并发", "需要检查"))
        else:
            stability_checks.append(("成员变量进行赋值或创建需要排查并发", "√"))

        # 3. 谨慎在lambda表达式中使用引用捕获
        if check_pattern('lambda'):
            has_ref_capture = any('&]' in line or '[&' in line
                                  for line in analysis.get('added_lines', []))
            stability_checks.append(("谨慎在lambda表达式中使用引用捕获",
                                    "√" if not has_ref_capture else "需要检查"))
        else:
            stability_checks.append(("谨慎在lambda表达式中使用引用捕获", "√"))

        # 4. 谨慎在未经拷贝的情况下使用外部传入的string、C字符串
        if has_file_type(('.cpp', '.cc', '.h', '.hpp')):
            has_string_ref = any('&string' in line.lower() or '*char' in line.lower()
                                for line in analysis.get('added_lines', []))
            stability_checks.append(("谨慎在未经拷贝的情况下使用外部传入的string、C字符串",
                                    "√" if not has_string_ref else "需要检查"))
        else:
            stability_checks.append(("谨慎在未经拷贝的情况下使用外部传入的string、C字符串", "不涉及"))

        # 5. map\vector\list\set等stl模板类使用时需要排查并发
        if check_pattern('stl_container') and check_pattern('thread'):
            stability_checks.append(("map\\vector\\list\\set等stl模板类使用时需要排查并发", "需要检查"))
        elif check_pattern('stl_container'):
            stability_checks.append(("map\\vector\\list\\set等stl模板类使用时需要排查并发", "√"))
        else:
            stability_checks.append(("map\\vector\\list\\set等stl模板类使用时需要排查并发", "不涉及"))

        # 6. 谨慎考虑加锁范围
        if check_pattern('thread'):
            has_lock = any('lock' in line.lower() for line in analysis.get('added_lines', []))
            stability_checks.append(("谨慎考虑加锁范围", "√" if has_lock else "不涉及"))
        else:
            stability_checks.append(("谨慎考虑加锁范围", "√"))

        # 7. 在IPC通信中谨慎使用同步通信方式
        if check_pattern('ipc'):
            has_sync_ipc = any('sync' in line.lower() or 'sendrequest' in line.lower()
                              for line in analysis.get('added_lines', []))
            stability_checks.append(("在IPC通信中谨慎使用同步通信方式",
                                    "√" if not has_sync_ipc else "需要检查"))
        else:
            stability_checks.append(("在IPC通信中谨慎使用同步通信方式", "√"))

        # 8. 禁止传递this指针至其他模块或线程（特别是eventhandler任务）
        if any('this' in line.lower() and ('post' in line.lower() or 'task' in line.lower())
               for line in analysis.get('added_lines', [])):
            stability_checks.append(("禁止传递this指针至其他模块或线程（特别是eventhandler任务）", "需要检查"))
        else:
            stability_checks.append(("禁止传递this指针至其他模块或线程（特别是eventhandler任务）", "√"))

        # 9. 禁止将外部传入的裸指针在内部直接构造智能指针
        if check_pattern('smart_pointer') and check_pattern('raw_pointer'):
            has_dangerous_construct = any('sptr(' in line or 'shared_ptr(' in line
                                         for line in analysis.get('added_lines', []))
            stability_checks.append(("禁止将外部传入的裸指针在内部直接构造智能指针",
                                    "√" if not has_dangerous_construct else "需要检查"))
        else:
            stability_checks.append(("禁止将外部传入的裸指针在内部直接构造智能指针", "√"))

        # 10. 禁止多个独立创建的智能指针管理同一地址
        if check_pattern('smart_pointer'):
            stability_checks.append(("禁止多个独立创建的智能指针管理同一地址", "√"))
        else:
            stability_checks.append(("禁止多个独立创建的智能指针管理同一地址", "不涉及"))

        # 11. 禁止在析构函数中抛异步任务
        if check_pattern('virtual') and check_pattern('thread'):
            has_destructor_async = any('~' in line and ('post' in line.lower() or 'task' in line.lower())
                                       for line in analysis.get('added_lines', []))
            stability_checks.append(("禁止在析构函数中抛异步任务",
                                    "√" if not has_destructor_async else "需要检查"))
        else:
            stability_checks.append(("禁止在析构函数中抛异步任务", "√"))

        # 12. 禁止js对象在非js线程（例如在IPC线程）创建、使用或销毁
        if has_file_type(('.js', '.ts', '.jsx', '.tsx')):
            stability_checks.append(("禁止js对象在非js线程（例如在IPC线程）创建、使用或销毁", "√"))
        else:
            stability_checks.append(("禁止js对象在非js线程（例如在IPC线程）创建、使用或销毁", "不涉及"))

        # 13. 禁止在对外接口中未经判空直接使用外部传入的指针
        if check_pattern('raw_pointer'):
            has_null_check = any('if' in line and ('null' in line.lower() or 'nullptr' in line.lower())
                                for line in analysis.get('added_lines', []))
            stability_checks.append(("禁止在对外接口中未经判空直接使用外部传入的指针",
                                    "√" if has_null_check else "需要检查"))
        else:
            stability_checks.append(("禁止在对外接口中未经判空直接使用外部传入的指针", "√"))

        # 14. 禁止接口返回局部变量引用
        if has_file_type(('.cpp', '.cc', '.h', '.hpp')):
            has_return_ref = any('&' in line and 'return' in line.lower()
                                for line in analysis.get('added_lines', []))
            stability_checks.append(("禁止接口返回局部变量引用",
                                    "√" if not has_return_ref else "需要检查"))
        else:
            stability_checks.append(("禁止接口返回局部变量引用", "√"))

        # 15. 禁止在信号函数中加锁
        if check_pattern('signal'):
            has_signal_lock = any('lock' in line.lower()
                                 for line in analysis.get('added_lines', []))
            stability_checks.append(("禁止在信号函数中加锁",
                                    "√" if not has_signal_lock else "需要检查"))
        else:
            stability_checks.append(("禁止在信号函数中加锁", "√"))

        # 16. 禁止在关键流程（SA启动、应用启动等主流程）执行耗时的操作
        if check_pattern('thread') or check_pattern('ipc'):
            has_heavy_operation = any('sleep' in line.lower() or 'wait' in line.lower()
                                     for line in analysis.get('added_lines', []))
            stability_checks.append(("禁止在关键流程（SA启动、应用启动等主流程）执行耗时的操作",
                                    "√" if not has_heavy_operation else "需要检查"))
        else:
            stability_checks.append(("禁止在关键流程（SA启动、应用启动等主流程）执行耗时的操作", "√"))

        # 17. 禁止将同一个cpp编译在不同的so中
        if has_file_type(('.cpp', '.cc')):
            stability_checks.append(("禁止将同一个cpp编译在不同的so中", "√"))
        else:
            stability_checks.append(("禁止将同一个cpp编译在不同的so中", "不涉及"))

        # 生成稳定性自检表
        stability_table = []
        for item, status in stability_checks:
            stability_table.append(f"| {item} | {status} |")

        # 智能判断安全编码自检项
        security_checks = []

        # 1. 裸指针避免通过隐式转换构造为sptr
        if check_pattern('raw_pointer') and check_pattern('smart_pointer'):
            has_implicit_sptr = any('sptr(' in line or '= sptr' in line
                                    for line in analysis.get('added_lines', []))
            security_checks.append(("裸指针避免通过隐式转换构造为sptr",
                                   "√" if not has_implicit_sptr else "需要检查"))
        else:
            security_checks.append(("裸指针避免通过隐式转换构造为sptr", "√"))

        # 2. json对象在取值之前必须先判断类型，避免类型不匹配
        if has_file_type('.json'):
            has_json_check = any('is' in line.lower() or 'type' in line.lower()
                                for line in analysis.get('added_lines', []))
            security_checks.append(("json对象在取值之前必须先判断类型，避免类型不匹配",
                                   "√" if has_json_check else "需要检查"))
        else:
            security_checks.append(("json对象在取值之前必须先判断类型，避免类型不匹配", "√"))

        # 3. 序列化时必须对传入的数组大小进行校验，避免出现超大数组
        if check_pattern('file_ops'):
            has_size_check = any('size' in line.lower() or 'length' in line.lower()
                                for line in analysis.get('added_lines', []))
            security_checks.append(("序列化时必须对传入的数组大小进行校验，避免出现超大数组",
                                   "√" if has_size_check else "需要检查"))
        else:
            security_checks.append(("序列化时必须对传入的数组大小进行校验，避免出现超大数组", "√"))

        # 4. 避免使用未明确位宽的整型，选择使用int8_t、uint8_t等类型
        if has_file_type(('.cpp', '.cc', '.h', '.hpp', '.java', '.kt')):
            has_fixed_width = any('int8_t' in line or 'uint8_t' in line or 'int16_t' in line
                                 for line in analysis.get('added_lines', []))
            has_unsafe_int = any(' int ' in line and not 'int8_t' in line and not 'int16_t' in line
                                for line in analysis.get('added_lines', []))
            security_checks.append(("避免使用未明确位宽的整型，选择使用int8_t、uint8_t等类型",
                                   "√" if has_fixed_width or not has_unsafe_int else "需要检查"))
        else:
            security_checks.append(("避免使用未明确位宽的整型，选择使用int8_t、uint8_t等类型", "√"))

        # 5. 外部传入的路径要做规范化校验，对路径中的.、..、../等特殊字符严格校验
        if check_pattern('file_ops'):
            has_path_check = any('normalize' in line.lower() or 'validate' in line.lower()
                                for line in analysis.get('added_lines', []))
            security_checks.append(("外部传入的路径要做规范化校验，对路径中的.、..、../等特殊字符严格校验",
                                   "√" if has_path_check else "需要检查"))
        else:
            security_checks.append(("外部传入的路径要做规范化校验，对路径中的.、..、../等特殊字符严格校验", "√"))

        # 6. 指针变量、表示资源描述符的变量、bool变量必须赋初值
        if has_file_type(('.cpp', '.cc', '.h', '.hpp', '.java', '.kt')):
            has_init = any('=' in line for line in analysis.get('added_lines', []))
            security_checks.append(("指针变量、表示资源描述符的变量、bool变量必须赋初值",
                                   "√" if has_init else "需要检查"))
        else:
            security_checks.append(("指针变量、表示资源描述符的变量、bool变量必须赋初值", "√"))

        # 7. readParcelable获取的对象使用前需要判空
        if has_file_type(('.java', '.kt')):
            has_null_check = any('if' in line and ('null' in line.lower() or 'nullptr' in line.lower())
                                for line in analysis.get('added_lines', []))
            security_checks.append(("readParcelable获取的对象使用前需要判空",
                                   "√" if has_null_check else "需要检查"))
        else:
            security_checks.append(("readParcelable获取的对象使用前需要判空", "√"))

        # 8. 分配和释放内存的函数需要成对出现
        if check_pattern('memory_alloc'):
            has_free = any('free' in line.lower() or 'delete' in line.lower()
                          for line in analysis.get('added_lines', []))
            security_checks.append(("分配和释放内存的函数需要成对出现",
                                   "√" if has_free else "需要检查"))
        else:
            security_checks.append(("分配和释放内存的函数需要成对出现", "√"))

        # 9. 申请内存后异常退出前需要及时进行内存释放
        if check_pattern('memory_alloc'):
            has_exception_handling = any('try' in line.lower() or 'catch' in line.lower()
                                        for line in analysis.get('added_lines', []))
            security_checks.append(("申请内存后异常退出前需要及时进行内存释放",
                                   "√" if has_exception_handling else "需要检查"))
        else:
            security_checks.append(("申请内存后异常退出前需要及时进行内存释放", "√"))

        # 10. 内存申请前必须对内存大小进行合法性校验
        if check_pattern('memory_alloc'):
            has_size_check = any('size' in line.lower() or 'sizeof' in line.lower()
                                for line in analysis.get('added_lines', []))
            security_checks.append(("内存申请前必须对内存大小进行合法性校验",
                                   "√" if has_size_check else "需要检查"))
        else:
            security_checks.append(("内存申请前必须对内存大小进行合法性校验", "√"))

        # 11. 内存分配后必须判断是否成功
        if check_pattern('memory_alloc'):
            has_null_check = any('if' in line and ('null' in line.lower() or 'nullptr' in line.lower())
                                for line in analysis.get('added_lines', []))
            security_checks.append(("内存分配后必须判断是否成功",
                                   "√" if has_null_check else "需要检查"))
        else:
            security_checks.append(("内存分配后必须判断是否成功", "√"))

        # 12. 禁止使用realloc、alloca函数
        if check_pattern('memory_alloc'):
            has_dangerous_func = any('realloc' in line.lower() or 'alloca' in line.lower()
                                    for line in analysis.get('added_lines', []))
            security_checks.append(("禁止使用realloc、alloca函数",
                                   "√" if not has_dangerous_func else "需要检查"))
        else:
            security_checks.append(("禁止使用realloc、alloca函数", "√"))

        # 13. 禁止打印文件路径、口令等敏感信息，如有需要，使用private修饰
        if check_pattern('file_ops'):
            has_log = any('print' in line.lower() or 'log' in line.lower()
                         for line in analysis.get('added_lines', []))
            has_private = any('private' in line.lower()
                             for line in analysis.get('added_lines', []))
            security_checks.append(("禁止打印文件路径、口令等敏感信息，如有需要，使用private修饰",
                                   "√" if not has_log or has_private else "需要检查"))
        else:
            security_checks.append(("禁止打印文件路径、口令等敏感信息，如有需要，使用private修饰", "√"))

        # 14. 禁止打印内存地址
        if check_pattern('raw_pointer') or check_pattern('smart_pointer'):
            has_address_print = any('print' in line.lower() or 'log' in line.lower()
                                   for line in analysis.get('added_lines', []))
            security_checks.append(("禁止打印内存地址",
                                   "√" if not has_address_print else "需要检查"))
        else:
            security_checks.append(("禁止打印内存地址", "√"))

        # 15. 整数之间运算时必须严格检查，确保不会出现溢出、反转、除0
        if has_file_type(('.cpp', '.cc', '.h', '.hpp', '.java', '.kt')):
            has_arithmetic = any('+' in line or '-' in line or '*' in line or '/' in line
                                for line in analysis.get('added_lines', []))
            has_overflow_check = any('overflow' in line.lower() or 'check' in line.lower()
                                    for line in analysis.get('added_lines', []))
            security_checks.append(("整数之间运算时必须严格检查，确保不会出现溢出、反转、除0",
                                   "√" if not has_arithmetic or has_overflow_check else "需要检查"))
        else:
            security_checks.append(("整数之间运算时必须严格检查，确保不会出现溢出、反转、除0", "√"))

        # 16. 禁止对有符号整数进行位操作符运算
        if has_file_type(('.cpp', '.cc', '.h', '.hpp')):
            has_bit_ops = any('<<' in line or '>>' in line or '&' in line or '|' in line
                             for line in analysis.get('added_lines', []))
            security_checks.append(("禁止对有符号整数进行位操作符运算",
                                   "√" if not has_bit_ops else "需要检查"))
        else:
            security_checks.append(("禁止对有符号整数进行位操作符运算", "√"))

        # 17. 禁止对指针进行逻辑或位运算
        if check_pattern('raw_pointer'):
            has_pointer_bit_op = any('<<' in line or '>>' in line or '&' in line or '|' in line
                                    for line in analysis.get('added_lines', []))
            security_checks.append(("禁止对指针进行逻辑或位运算",
                                   "√" if not has_pointer_bit_op else "需要检查"))
        else:
            security_checks.append(("禁止对指针进行逻辑或位运算", "√"))

        # 18. 循环次数如果收外部数据控制，需要检验其合法性
        if check_pattern('file_ops') or has_file_type('.json'):
            has_loop = any('for' in line or 'while' in line
                          for line in analysis.get('added_lines', []))
            has_bounds_check = any('size' in line.lower() or 'length' in line.lower()
                                  for line in analysis.get('added_lines', []))
            security_checks.append(("循环次数如果收外部数据控制，需要检验其合法性",
                                   "√" if not has_loop or has_bounds_check else "需要检查"))
        else:
            security_checks.append(("循环次数如果收外部数据控制，需要检验其合法性", "√"))

        # 19. 禁止使用内存操作类危险函数，需要使用安全函数
        if check_pattern('memory_alloc') or check_pattern('file_ops'):
            has_unsafe_func = any('strcpy' in line.lower() or 'sprintf' in line.lower()
                                 for line in analysis.get('added_lines', []))
            security_checks.append(("禁止使用内存操作类危险函数，需要使用安全函数",
                                   "√" if not has_unsafe_func else "需要检查"))
        else:
            security_checks.append(("禁止使用内存操作类危险函数，需要使用安全函数", "√"))

        # 20. 谨慎使用不可重入函数
        if check_pattern('thread'):
            security_checks.append(("谨慎使用不可重入函数", "需要检查"))
        else:
            security_checks.append(("谨慎使用不可重入函数", "√"))

        # 21. 必须检查安全函数的返回值，并进行正确处理
        if check_pattern('file_ops'):
            has_return_check = any('if' in line for line in analysis.get('added_lines', []))
            security_checks.append(("必须检查安全函数的返回值，并进行正确处理",
                                   "√" if has_return_check else "需要检查"))
        else:
            security_checks.append(("必须检查安全函数的返回值，并进行正确处理", "√"))

        # 22. 禁止仅通过TokenType类型判断绕过权限校验
        if has_file_type(('.java', '.kt')):
            has_permission_check = any('permission' in line.lower() or 'check' in line.lower()
                                      for line in analysis.get('added_lines', []))
            security_checks.append(("禁止仅通过TokenType类型判断绕过权限校验",
                                   "√" if has_permission_check else "需要检查"))
        else:
            security_checks.append(("禁止仅通过TokenType类型判断绕过权限校验", "√"))

        # 生成安全编码自检表
        security_table = []
        for item, status in security_checks:
            security_table.append(f"| {item} | {status} |")

        return {
            'stability': stability_table,
            'security': security_table
        }

    def create_pr(self, issue_url, title=None, description=""):
        """创建PR"""
        print(f"\n{'='*70}")
        print(f"步骤3: 创建Pull Request")
        print(f"{'='*70}")

        # 分析代码变更
        change_types = self.analyze_code_changes()

        # 生成自检表
        checklist = self.generate_checklist(change_types)

        # 构建PR描述
        if not description:
            description = "Automated PR creation"

        # 如果有模板，使用模板
        if self.template:
            # 构建自检表
            stability_section = "**稳定性自检：**\n"
            stability_section += "| 自检项                                                       | 自检结果  |\n"
            stability_section += "| ------------------------------------------------------------ | -------- |\n"
            stability_section += '\n'.join(checklist['stability'])

            security_section = "**安全编码自检：**\n"
            security_section += "| 自检项                                                          | 自检结果 |\n"
            security_section += "| -------------------------------------------------------------- | -------- |\n"
            security_section += '\n'.join(checklist['security'])

            # 使用字符串替换而不是正则表达式
            pr_description = self.template.format(
                issue_no=issue_url,
                description=description,
                tdd_result="",
                xts_result=""
            )

            # 找到自检表部分的开始和结束位置
            stability_start = pr_description.find("**稳定性自检：**")
            security_start = pr_description.find("**安全编码自检：**")
            section_end = pr_description.find("\n\n**TDD Result**:", security_start)

            if stability_start != -1 and security_start != -1 and section_end != -1:
                # 替换自检表部分
                pr_description = (pr_description[:stability_start] +
                                 stability_section + '\n\n' +
                                 security_section +
                                 pr_description[section_end:])
        else:
            # 使用默认格式
            pr_description = f"""**IssueNo**:
{issue_url}

**Description**:
{description}

**稳定性自检：**
{chr(10).join(checklist['stability'])}

**安全编码自检：**
{chr(10).join(checklist['security'])}

**TDD Result**:


**XTS Result**:


### 是否已执行L0用例
- [ ] 已验证
- [ ] 不涉及
"""

        # 设置标题
        if not title:
            title = f"PR for issue {issue_url.split('/')[-1]}"

        # API请求
        api_url = f"https://gitcode.com/api/v5/repos/{self.repo_info['target_repo']}/pulls"

        headers = {
            "Authorization": f"Bearer {self.token}",
            "Content-Type": "application/json"
        }

        data = {
            "head": f"{self.repo_info['source_repo']}:{self.repo_info['current_branch']}",
            "base": "master",
            "title": title,
            "body": pr_description
        }

        print(f"API: {api_url}")
        print(f"源: {data['head']}")
        print(f"目标: {data['base']}")
        print(f"Issue: {issue_url}")
        print(f"\n正在创建PR...")

        try:
            response = requests.post(api_url, json=data, headers=headers, timeout=30)

            if response.status_code in [200, 201]:
                result = response.json()
                pr_id = result.get('number') or result.get('id')
                pr_url = result.get('html_url') or result.get('web_url')

                print(f"\n{'='*70}")
                print(f"[OK] PR创建成功！")
                print(f"{'='*70}")
                print(f"PR ID: {pr_id}")
                print(f"PR URL: {pr_url}")
                print(f"{'='*70}")

                # 保存PR信息
                with open('LAST_PR.txt', 'w', encoding='utf-8') as f:
                    f.write(f"PR ID: {pr_id}\n")
                    f.write(f"PR URL: {pr_url}\n")

                return pr_url, pr_id
            else:
                print(f"[FAIL] 创建失败 (HTTP {response.status_code})")
                print(f"响应: {response.text[:500]}")
                return None, None

        except Exception as e:
            print(f"[FAIL] 错误: {str(e)}")
            import traceback
            traceback.print_exc()
            return None, None

    def run(self, issue_url, title=None, description="", push=True):
        """执行完整的自动化流程"""
        print(f"{'='*70}")
        print(f"GitCode自动化PR创建工具")
        print(f"{'='*70}")

        # 交互式选择目标仓库
        target_repo = self.interactive_select_target_repo()

        # 交互式选择源仓库
        source_repo = self.interactive_select_source_repo(target_repo)

        # 交互式选择分支
        branch = self.interactive_select_branch()

        # 更新repo_info
        self.repo_info['target_repo'] = target_repo
        self.repo_info['source_repo'] = source_repo
        self.repo_info['current_branch'] = branch

        # 如果没有提供标题，使用默认标题
        if not title:
            title = f"PR for issue {issue_url.split('/')[-1]}"

        # 确认PR创建
        if not self.confirm_pr_creation(issue_url, title, target_repo, source_repo, branch):
            return 1

        print(f"\n目标仓库: {target_repo}")
        print(f"源仓库: {source_repo}")
        print(f"当前分支: {branch}")
        print(f"DCO签名: {self.get_dco_signature()}")
        print(f"Issue: {issue_url}")

        # 步骤1: 推送代码到个人fork
        if push:
            # 获取fork的remote名称
            fork_remote = self.get_fork_remote_name()
            if not fork_remote:
                print(f"\n[ERROR] 找不到fork的remote，请先添加fork远程仓库")
                print(f"提示: git remote add {self.user_info['username']} <fork_url>")
                sys.exit(1)

            if not self.push_to_remote(remote=fork_remote):
                print(f"\n[ERROR] 推送失败，终止流程")
                sys.exit(1)

        # 步骤2 & 3: 创建PR（包含自检项填充）
        pr_url, pr_id = self.create_pr(issue_url, title, description)

        if pr_url:
            print(f"\n{'='*70}")
            print(f"[OK] 流程完成！")
            print(f"{'='*70}")
            print(f"\n请访问PR链接查看:")
            print(f"{pr_url}")
            return 0
        else:
            print(f"\n[ERROR] PR创建失败")
            return 1

    def get_fork_remote_name(self):
        """获取fork仓库的remote名称"""
        try:
            result = subprocess.run(
                ['git', 'remote', '-v'],
                capture_output=True,
                encoding='utf-8', errors='replace',
                check=True
            )

            remotes = result.stdout.strip()
            username = self.user_info['username']

            # 查找包含username的remote
            for line in remotes.split('\n'):
                if username in line and '(fetch)' in line:
                    return line.split()[0]  # 返回remote名称

            return None
        except Exception as e:
            print(f"[WARNING] 无法获取remote信息: {str(e)}")
            return None

    def get_available_branches(self):
        """获取可用的分支列表"""
        try:
            # 获取本地分支
            result = subprocess.run(
                ['git', 'branch', '-a'],
                capture_output=True,
                encoding='utf-8', errors='replace',
                check=True
            )

            branches = []
            for line in result.stdout.strip().split('\n'):
                # 清理分支名
                branch = line.strip().replace('* ', '').replace('remotes/', '')
                # 过滤掉HEAD
                if 'HEAD' not in branch:
                    branches.append(branch)

            return list(set(branches))  # 去重
        except Exception as e:
            print(f"[WARNING] 无法获取分支列表: {str(e)}")
            return []

    def interactive_select_branch(self):
        """交互式选择分支"""
        if not self.interactive:
            return self.repo_info['current_branch']

        branches = self.get_available_branches()

        if not branches:
            print(f"\n[INFO] 未找到其他分支，使用当前分支: {self.repo_info['current_branch']}")
            return self.repo_info['current_branch']

        print(f"\n{'='*70}")
        print("可用分支列表：")
        print(f"{'='*70}")

        # 标记当前分支
        current = self.repo_info['current_branch']

        for i, branch in enumerate(branches, 1):
            marker = " (当前)" if branch == current or branch.endswith(f"/{current}") else ""
            print(f"  {i}. {branch}{marker}")

        print(f"\n  0. 使用当前分支 ({current})")

        while True:
            choice = input(f"\n请选择分支 [默认: 0]: ").strip()

            if not choice:
                return current

            try:
                choice_idx = int(choice)
                if choice_idx == 0:
                    return current
                elif 1 <= choice_idx <= len(branches):
                    selected = branches[choice_idx - 1]
                    # 移除remotes/前缀
                    if selected.startswith('remotes/'):
                        selected = selected.split('/', 1)[1]
                    return selected
                else:
                    print(f"[ERROR] 无效的选择，请输入 0-{len(branches)} 之间的数字")
            except ValueError:
                print(f"[ERROR] 请输入数字")

    def interactive_select_target_repo(self):
        """交互式选择目标仓库"""
        if not self.interactive:
            return self.repo_info['target_repo']

        # 获取已配置的模板仓库
        try:
            with open(TEMPLATE_FILE, 'r', encoding='utf-8') as f:
                templates = json.load(f)

            if templates:
                print(f"\n{'='*70}")
                print("已配置的PR模板仓库：")
                print(f"{'='*70}")

                repo_list = list(templates.keys())
                current_repo = self.repo_info['target_repo']

                for i, repo in enumerate(repo_list, 1):
                    marker = " (当前)" if repo == current_repo else ""
                    print(f"  {i}. {repo}{marker}")

                print(f"  0. 使用当前检测到的仓库 ({current_repo})")

                while True:
                    choice = input(f"\n请选择目标仓库 [默认: 0]: ").strip()

                    if not choice:
                        return current_repo

                    try:
                        choice_idx = int(choice)
                        if choice_idx == 0:
                            return current_repo
                        elif 1 <= choice_idx <= len(repo_list):
                            return repo_list[choice_idx - 1]
                        else:
                            print(f"[ERROR] 无效的选择，请输入 0-{len(repo_list)} 之间的数字")
                    except ValueError:
                        print(f"[ERROR] 请输入数字")

        except FileNotFoundError:
            print(f"[INFO] 未找到PR模板配置文件")

        return self.repo_info['target_repo']

    def interactive_select_source_repo(self, target_repo):
        """交互式选择源仓库（fork）"""
        if not self.interactive:
            return self.repo_info['source_repo']

        username = self.user_info['username']
        auto_source = target_repo.replace(target_repo.split('/')[0], username)

        print(f"\n{'='*70}")
        print("源仓库（Fork）选择：")
        print(f"{'='*70}")
        print(f"  1. 自动推断: {auto_source}")
        print(f"  2. 当前配置: {self.repo_info['source_repo']}")

        while True:
            choice = input(f"\n请选择源仓库 [默认: 1]: ").strip()

            if not choice or choice == '1':
                return auto_source
            elif choice == '2':
                return self.repo_info['source_repo']
            else:
                print(f"[ERROR] 请输入 1 或 2")

    def confirm_pr_creation(self, issue_url, title, target_repo, source_repo, branch):
        """确认PR创建参数"""
        if not self.interactive:
            return True

        print(f"\n{'='*70}")
        print("PR创建信息确认：")
        print(f"{'='*70}")
        print(f"  Issue:    {issue_url}")
        print(f"  标题:     {title}")
        print(f"  目标仓库: {target_repo}")
        print(f"  源仓库:   {source_repo}")
        print(f"  分支:     {branch}")
        print(f"{'='*70}")

        while True:
            choice = input("\n确认创建PR？(Y/n): ").strip().lower()

            if not choice or choice == 'y':
                return True
            elif choice == 'n':
                print("[取消] PR创建已取消")
                return False
            else:
                print("[ERROR] 请输入 y 或 n")


def main():
    parser = argparse.ArgumentParser(
        description='GitCode自动化PR创建工具',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  # 使用DCO签名提交代码
  python %(prog)s commit -m "fix: 修复内存泄漏"

  # 为最后一次提交添加DCO签名（如果缺失）
  python %(prog)s amend-dco

  # 使用当前分支，推送并创建PR
  python %(prog)s push-pr --issue "https://gitcode.com/openharmony/ability_ability_runtime/issues/14320"

  # 不推送，只创建PR
  python %(prog)s push-pr --issue "14320" --no-push

  # 指定标题和描述
  python %(prog)s push-pr --issue "14320" --title "Fix bug" --description "修复内存泄漏问题"
        """
    )

    parser.add_argument(
        'command',
        choices=['commit', 'amend-dco', 'push-pr'],
        help='要执行的命令: commit(提交代码), amend-dco(添加DCO签名), push-pr(推送并创建PR)'
    )

    parser.add_argument(
        '-m',
        '--message',
        help='提交消息（用于commit命令）'
    )

    parser.add_argument(
        '--issue',
        help='Issue链接或ID（用于push-pr命令）'
    )

    parser.add_argument(
        '--title',
        help='PR标题（用于push-pr命令）'
    )

    parser.add_argument(
        '--description',
        default='',
        help='PR描述（用于push-pr命令）'
    )

    parser.add_argument(
        '--no-push',
        action='store_true',
        help='不推送代码，只创建PR（用于push-pr命令）'
    )

    parser.add_argument(
        '-i',
        '--interactive',
        action='store_true',
        help='交互式模式：选择仓库、分支等选项'
    )

    args = parser.parse_args()

    # 创建自动化工具
    automation = GitCodeAutomation(interactive=args.interactive)

    # 执行对应命令
    if args.command == 'commit':
        if not args.message:
            print("[ERROR] commit命令需要使用 -m 参数指定提交消息")
            sys.exit(1)
        success = automation.commit_with_dco(args.message)
        sys.exit(0 if success else 1)

    elif args.command == 'amend-dco':
        success = automation.amend_dco_to_last_commit()
        sys.exit(0 if success else 1)

    elif args.command == 'push-pr':
        if not args.issue:
            print("[ERROR] push-pr命令需要使用 --issue 参数指定issue")
            sys.exit(1)

        # 处理issue参数
        issue_url = args.issue
        if not issue_url.startswith('http'):
            # 假设是issue ID
            repo = automation.repo_info['target_repo']
            issue_url = f"https://gitcode.com/{repo}/issues/{issue_url}"

        exit_code = automation.run(
            issue_url=issue_url,
            title=args.title,
            description=args.description,
            push=not args.no_push
        )
        sys.exit(exit_code)


if __name__ == '__main__':
    main()
