#!/bin/bash
# 简化版测试运行脚本

echo "╔════════════════════════════════════════════════════════════════════╗"
echo "║       CLI参数映射系统 - 简化版测试 (四种模式独立测试)          ║"
echo "╚════════════════════════════════════════════════════════════════════╝"
echo ""

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "📋 简化版测试说明"
echo "════════════════════════════════════════════════════════════════════"
echo "✅ 当前测试基于统一 test_all.py"
echo "✅ 配置样例已切换为 executable / requirePermissions / input"
echo "✅ 验证工具基于当前 config 口径运行"
echo ""

echo "🧪 第一阶段：统一测试"
echo "════════════════════════════════════════════════════════════════════"
echo "运行 test_all.py ..."
if python3 test_all.py > /tmp/simplified_test.log 2>&1; then
    echo "✅ 统一测试通过"
    grep -E "Ran.*tests" /tmp/simplified_test.log
else
    echo "❌ 统一测试失败"
    cat /tmp/simplified_test.log
fi
echo ""

echo "📁 第二阶段：配置验证演示"
echo "════════════════════════════════════════════════════════════════════"
echo "运行 quick_validation.sh ..."
if bash ./quick_validation.sh > /tmp/file_test.log 2>&1; then
    echo "✅ 配置验证演示通过"
else
    echo "❌ 配置验证演示失败"
    cat /tmp/file_test.log
fi
echo ""

echo "🔍 第三阶段：文件检查"
echo "════════════════════════════════════════════════════════════════════"
echo "列出可用文件..."
find configs inputs outputs -maxdepth 1 -type f | sort | head -10
echo ""

echo "📊 第四阶段：测试统计"
echo "════════════════════════════════════════════════════════════════════"
echo "测试覆盖范围:"
echo "   Flag模式 / Positional / Flattened / Json-String"
echo "   Input 验证与映射行为都在 test_all.py 中覆盖"
echo ""

echo "📁 文件状态:"
echo "   配置文件: $(ls -1 configs/*.json 2>/dev/null | wc -l) 个"
echo "   输入文件: $(ls -1 inputs/*.json 2>/dev/null | wc -l) 个"
echo "   输出文件: $(ls -1 outputs/*.json 2>/dev/null | wc -l) 个"
echo ""

echo "🎯 快速测试命令:"
echo "   运行所有测试: python3 test_all.py"
echo "   演示验证工具: python3 validate_and_run.py --config configs/flag_network_scanner.json --input inputs/flag_network_scanner_input.json"
echo "   快速演示:     bash ./quick_validation.sh"
echo ""

echo "✅ 简化版测试流程完成！"
echo ""
