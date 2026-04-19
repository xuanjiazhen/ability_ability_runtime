#!/usr/bin/env python3
"""
Input 和 ArgumentMapping 验证工具

演示如何验证 input 并执行 argumentMapping 得到结果。
"""

import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from mapper.schema_validator import validate_input
from mapper import MapperFactory


def print_section(title):
    """打印分节标题"""
    print(f"\n{'='*70}")
    print(f"  {title}")
    print(f"{'='*70}")


def validate_and_map(config_file: str, input_file: str = None, input_args: dict = None):
    """
    验证 input 并执行 argumentMapping

    Args:
        config_file: 工具配置文件路径
        input_file: 输入参数文件路径（可选）
        input_args: 直接提供输入参数（可选）
    """
    print_section("1. 加载工具配置")

    config_path = Path(config_file)
    if not config_path.exists():
        print(f"❌ 配置文件不存在: {config_file}")
        return

    with open(config_path) as f:
        tool_config = json.load(f)

    print(f"✅ 配置文件: {config_path.name}")
    print(f"   工具路径: {tool_config.get('executable', 'N/A')}")

    # 加载输入参数
    print_section("2. 加载输入参数")

    if input_args is None:
        if input_file:
            with open(input_file) as f:
                input_args = json.load(f)
            print(f"✅ 输入文件: {Path(input_file).name}")
        else:
            print("❌ 未提供输入参数")
            return

    print(f"   输入参数:")
    print(f"   {json.dumps(input_args, indent=2, ensure_ascii=False)}")

    # 验证 input
    print_section("3. 验证 input")

    if 'input' not in tool_config:
        print("❌ 配置中缺少 input")
        return

    is_valid, errors = validate_input(input_args, tool_config['input'])

    if is_valid:
        print("✅ input 验证通过")
    else:
        print("❌ input 验证失败:")
        for error in errors:
            print(f"   ❌ {error}")
        return

    # 执行 argumentMapping
    print_section("4. 执行 argumentMapping")

    argument_mapping = tool_config.get('argumentMapping', {})
    if not argument_mapping:
        print("❌ 配置中缺少 argumentMapping")
        return

    print(f"   映射类型: {argument_mapping.get('type', 'unknown')}")
    print(f"   映射配置:")
    print(f"   {json.dumps(argument_mapping, indent=2, ensure_ascii=False)}")

    try:
        mapper = MapperFactory.create(argument_mapping)
        mapped_argv = mapper.map(input_args)

        print(f"\n✅ 映射成功，生成 {len(mapped_argv)} 个参数:")
        for i, arg in enumerate(mapped_argv, 1):
            print(f"   argv[{i}] = {arg}")

    except Exception as e:
        print(f"❌ 映射失败: {e}")
        return

    # 构建完整 argv
    print_section("5. 构建完整 argv")

    executable_path = tool_config.get('executable', '/system/bin/cli_tool/executable/tool')
    complete_argv = [executable_path] + mapped_argv

    print(f"✅ 完整 argv ({len(complete_argv)} 个参数):")
    for i, arg in enumerate(complete_argv):
        print(f"   argv[{i}] = {arg}")

    # 显示执行命令
    print_section("6. 执行命令")

    # 转义特殊字符用于 shell 显示
    def escape_for_shell(arg):
        if ' ' in arg or '\t' in arg or '"' in arg or "'" in arg:
            escaped = arg.replace('"', '\\"')
            return f'"{escaped}"'
        return arg

    command_str = ' '.join(escape_for_shell(arg) for arg in complete_argv)
    print(f"   $ {command_str}")

    print(f"\n✅ 验证完成！")
    print(f"\n💡 说明:")
    print(f"   - argv[0] 是可执行文件路径 (由 executable 指定)")
    print(f"   - argv[1...] 是由 argumentMapping 生成的参数")
    print(f"   - 所有参数都通过了 input 的类型和约束验证")


def demo_validation():
    """演示完整的验证流程"""
    print_section("Input 和 ArgumentMapping 验证演示")

    # 示例 1: 使用测试配置
    print("\n📋 示例 1: 使用测试配置 (Flag 模式)")

    flag_config = Path(__file__).parent / 'configs' / 'flag_network_scanner.json'
    flag_input = Path(__file__).parent / 'inputs' / 'flag_network_scanner_input.json'
    if flag_config.exists() and flag_input.exists():
        validate_and_map(str(flag_config), str(flag_input))

    # 示例 2: 使用测试配置
    print("\n\n📋 示例 2: 使用测试配置 (Positional 模式)")

    positional_config = Path(__file__).parent / 'configs' / 'positional_file_copy.json'
    positional_input = Path(__file__).parent / 'inputs' / 'positional_file_copy_input.json'

    if positional_config.exists() and positional_input.exists():
        validate_and_map(str(positional_config), str(positional_input))

    # 示例 3: Flattened 模式
    print("\n\n📋 示例 3: Flattened 模式")

    flattened_config = Path(__file__).parent / 'configs' / 'flattened_app_deploy.json'
    flattened_input = Path(__file__).parent / 'inputs' / 'flattened_app_deploy_input.json'
    if flattened_config.exists() and flattened_input.exists():
        validate_and_map(str(flattened_config), str(flattened_input))

    # 示例 4: Json-String 模式
    print("\n\n📋 示例 4: Json-String 模式")

    jsonstring_config = Path(__file__).parent / 'configs' / 'jsonstring_cloud_deploy.json'
    jsonstring_input = Path(__file__).parent / 'inputs' / 'jsonstring_cloud_deploy_input.json'
    if jsonstring_config.exists() and jsonstring_input.exists():
        validate_and_map(str(jsonstring_config), str(jsonstring_input))


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='验证 input 和 argumentMapping')
    parser.add_argument('--config', '-c', help='工具配置文件路径')
    parser.add_argument('--input', '-i', help='输入参数文件路径')
    parser.add_argument('--demo', '-d', action='store_true', help='运行演示示例')
    parser.add_argument('--json', '-j', help='直接提供 JSON 输入参数')

    args = parser.parse_args()

    if args.demo:
        demo_validation()
    elif args.config:
        input_args = None
        if args.json:
            input_args = json.loads(args.json)

        validate_and_map(args.config, args.input, input_args)
    else:
        # 默认运行演示
        demo_validation()
