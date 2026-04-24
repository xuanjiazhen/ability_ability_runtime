#!/bin/bash
# 批量运行 testcase 目录下的所有测试用例

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 计数器
total=0
passed=0
failed=0

echo "======================================================================"
echo "  参数映射测试用例批量测试 (001-020)"
echo "======================================================================"
echo ""

# 循环测试所有用例
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
    # 补零 - 使用 $((10#$i)) 避免八进制问题
    num=$(printf "%03d" "$((10#$i))")

    config_file="testcase/config/test_${num}_*.json"
    input_file="testcase/input/test_${num}_input.json"

    # 检查文件是否存在
    if ! ls $config_file 1> /dev/null 2>&1 || [ ! -f "$input_file" ]; then
        echo -e "${YELLOW}⚠️  跳过测试 $num - 文件不存在${NC}"
        continue
    fi

    total=$((total + 1))

    # 获取实际的 config 文件名（带具体描述）
    config_file=$(ls testcase/config/test_${num}_*.json | head -1)

    echo -n "测试 $num: "

    # 运行测试并捕获输出
    output=$(python3 validate_and_run.py \
        --config "$config_file" \
        --input "$input_file" \
        2>&1)

    # 检查是否成功
    if echo "$output" | grep -q "✅ 验证完成！"; then
        echo -e "${GREEN}✅ PASSED${NC}"
        passed=$((passed + 1))
    else
        echo -e "${RED}❌ FAILED${NC}"
        failed=$((failed + 1))
        echo "错误输出:"
        echo "$output" | tail -20
    fi
done

echo ""
echo "======================================================================"
echo "  测试结果汇总"
echo "======================================================================"
echo -e "总计: $total | ${GREEN}通过: $passed${NC} | ${RED}失败: $failed${NC}"

if [ $failed -eq 0 ]; then
    echo -e "${GREEN}✅ 所有测试通过！${NC}"
    exit 0
else
    echo -e "${RED}❌ 有 $failed 个测试失败${NC}"
    exit 1
fi
