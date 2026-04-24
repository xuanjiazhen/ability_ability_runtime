#!/bin/bash

# 参数映射测试用例批量运行脚本

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 测试计数器
TOTAL=0
PASSED=0
FAILED=0

# 运行单个测试
run_test() {
    local test_num=$1
    local config="configs/test_${test_num}_*.json"
    local input="inputs/test_${test_num}_input.json"
    local expected="expected/test_${test_num}_expected.json"

    echo -e "\n${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${YELLOW}测试用例 ${test_num}${NC}"
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

    TOTAL=$((TOTAL + 1))

    # 读取测试描述
    if [ -f "$expected" ]; then
        description=$(python3 -c "import json; print(json.load(open('$expected'))['test_case'])")
        echo -e "📝 ${description}"
    fi

    # 运行 validate_and_run
    if python3 validate_and_run.py \
        --config "$config" \
        --input "$input"; then
        echo -e "${GREEN}✅ 测试用例 ${test_num} 通过${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}❌ 测试用例 ${test_num} 失败${NC}"
        FAILED=$((FAILED + 1))
    fi
}

# 主测试流程
main() {
    echo -e "${GREEN}"
    echo "╔════════════════════════════════════════════════════════════╗"
    echo "║   参数映射系统 - 测试用例批量运行                      ║"
    echo "╚════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"

    # 检查 validate_and_run.py 是否存在
    if [ ! -f "validate_and_run.py" ]; then
        echo -e "${RED}错误: validate_and_run.py 不存在${NC}"
        exit 1
    fi

    # 运行所有测试用例
    for i in {001..008}; do
        # 检查文件是否存在
        config_file=$(ls configs/test_${i}_*.json 2>/dev/null | head -1)
        if [ -f "$config_file" ] && [ -f "inputs/test_${i}_input.json" ]; then
            run_test "$i"
        fi
    done

    # 打印总结
    echo -e "\n${GREEN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║                    测试总结                                 ║${NC}"
    echo -e "${GREEN}╠════════════════════════════════════════════════════════════╣${NC}"
    echo -e "${GREEN}║  总测试数: ${TOTAL}                                           ║${NC}"
    echo -e "${GREEN}║  通过:    ${PASSED}                                           ║${NC}"
    echo -e "${GREEN}║  失败:    ${FAILED}                                           ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${NC}"

    if [ $FAILED -eq 0 ]; then
        echo -e "\n${GREEN}🎉 所有测试通过！${NC}"
        exit 0
    else
        echo -e "\n${RED}⚠️  部分测试失败${NC}"
        exit 1
    fi
}

# 运行主函数
main "$@"
