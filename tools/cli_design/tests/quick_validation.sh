#!/bin/bash
# 快速验证 input 和 argumentMapping

cd "$(dirname "$0")"/..

echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║       Input 和 ArgumentMapping 快速验证演示                    ║"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""

# 示例 1: Flag 模式（正确输入）
echo "📋 示例 1: Flag 模式 - 验证成功"
echo "─────────────────────────────────────────────────────────────────────"
python3 tests/validate_and_run.py \
  --config tests/configs/flag_network_scanner.json \
  --json '{"target":"192.168.1.1","ports":[80,443],"timeout":30,"verbose":true}' \
  2>&1 | grep -A 3 "执行命令"

echo ""
echo "📋 示例 2: Flag 模式 - 验证失败（IP 格式错误）"
echo "─────────────────────────────────────────────────────────────────────"
python3 tests/validate_and_run.py \
  --config tests/configs/flag_network_scanner.json \
  --json '{"target":"999.999.999.999","ports":[80]}' \
  2>&1 | grep -A 2 "验证失败"

echo ""
echo "📋 示例 3: Flattened 模式"
echo "─────────────────────────────────────────────────────────────────────"
python3 tests/validate_and_run.py \
  --config tests/configs/flattened_app_deploy.json \
  --json '{"app":{"name":"myapp","env":{"production":true}},"server":{"host":"10.0.0.1","port":8080}}' \
  2>&1 | grep -A 3 "执行命令"

echo ""
echo "📋 示例 4: Json-String 模式"
echo "─────────────────────────────────────────────────────────────────────"
python3 tests/validate_and_run.py \
  --config tests/configs/jsonstring_cloud_deploy.json \
  --json '{"config":{"application":{"name":"webapp","replicas":2}},"metadata":{"author":"devops","environment":"prod"}}' \
  2>&1 | grep -A 3 "执行命令"

echo ""
echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║                        验证完成                                   ║"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""
echo "💡 提示："
echo "   - 查看完整输出: python3 tests/validate_and_run.py --demo"
echo "   - 验证自定义配置: python3 tests/validate_and_run.py --config <file> --json '{...}'"
echo "   - 查看当前规范说明: cat README.md"
