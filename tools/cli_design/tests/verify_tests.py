#!/usr/bin/env python3
"""
测试用例验证脚本

验证 validate_and_run 的输出是否与预期一致
"""

import json
import sys
import subprocess
from pathlib import Path


def run_test(test_num):
    """运行单个测试用例"""
    config_file = list(Path("configs").glob(f"test_{test_num}_*.json"))[0]
    input_file = Path(f"inputs/test_{test_num}_input.json")
    expected_file = Path(f"expected/test_{test_num}_expected.json")

    # 读取预期输出
    with open(expected_file) as f:
        expected = json.load(f)

    # 运行 validate_and_run
    result = subprocess.run(
        ["python3", "validate_and_run.py",
         "--config", str(config_file),
         "--input", str(input_file)],
        capture_output=True,
        text=True
    )

    # 解析实际输出
    if result.returncode != 0:
        return False, f"执行失败: {result.stderr}"

    # 从输出中提取 argv
    import re
    argv_match = re.search(r'生成的 argv:(.*?)(?=\n\n|\n\[|$)', result.stdout, re.DOTALL)
    if not argv_match:
        return False, "无法从输出中提取 argv"

    # 比较 argv
    actual_argv = eval(f"[{argv_match.group(1).strip()}]")
    expected_argv = expected['expected_argv']

    if actual_argv == expected_argv:
        return True, "通过"
    else:
        return False, f"argv 不匹配\n预期: {expected_argv}\n实际: {actual_argv}"


def main():
    """主函数"""
    print("=" * 70)
    print("参数映射测试用例验证")
    print("=" * 70)

    passed = 0
    failed = 0
    results = []

    # 运行所有测试
    for i in range(1, 9):  # 001-008
        test_num = f"{i:03d}"
        try:
            success, message = run_test(test_num)
            results.append((test_num, success, message))

            if success:
                print(f"✅ 测试 {test_num}: {message}")
                passed += 1
            else:
                print(f"❌ 测试 {test_num}: {message}")
                failed += 1
        except Exception as e:
            print(f"❌ 测试 {test_num}: 异常 - {str(e)}")
            failed += 1

    # 打印总结
    print("\n" + "=" * 70)
    print("测试总结")
    print("=" * 70)
    print(f"总计: {passed + failed}")
    print(f"通过: {passed}")
    print(f"失败: {failed}")
    print("=" * 70)

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
