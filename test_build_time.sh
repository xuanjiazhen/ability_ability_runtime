#!/bin/bash

# 检查参数
if [ $# -eq 0 ]; then
    echo "用法: $0 <执行次数> [构建参数...]"
    echo "示例: $0 3                    # 执行3次构建，使用默认参数"
    echo "示例: $0 5 --product-name rk3568 --skip-download  # 执行5次，指定构建参数"
    exit 1
fi

# 获取执行次数
count=$1
shift

# 构建参数，如果没有提供则使用默认参数
if [ $# -eq 0 ]; then
    build_args="--product-name rk3568 --skip-download"
else
    build_args="$@"
fi

echo "=========================================="
echo "开始性能测试: 共执行 $count 次构建"
echo "构建参数: $build_args"
echo "=========================================="

# 统计变量
total_time=0
declare -a durations

# 执行多次构建
for ((i=1; i<=count; i++)); do
    echo ""
    echo "=========================================="
    echo "第 $i 次构建开始: $(date)"
    echo "=========================================="

    start_time=$(date +%s)

    ./build.sh $build_args

    end_time=$(date +%s)
    duration=$((end_time - start_time))

    # 记录每次耗时
    durations[$i]=$duration
    total_time=$((total_time + duration))

    echo "第 $i 次构建结束: $(date)"
    echo "本次耗时: $((duration / 60)) 分 $((duration % 60)) 秒 ($duration 秒)"
done

# 计算统计数据
echo ""
echo "=========================================="
echo "性能测试完成"
echo "=========================================="
echo "总执行次数: $count"
echo "总耗时: $((total_time / 60)) 分 $((total_time % 60)) 秒 ($total_time 秒)"

# 计算平均时间
avg_time=$((total_time / count))
echo "平均耗时: $((avg_time / 60)) 分 $((avg_time % 60)) 秒 ($avg_time 秒)"

# 找出最快和最慢
min_time=${durations[1]}
max_time=${durations[1]}

for time in "${durations[@]}"; do
    if [ $time -lt $min_time ]; then
        min_time=$time
    fi
    if [ $time -gt $max_time ]; then
        max_time=$time
    fi
done

echo "最快耗时: $((min_time / 60)) 分 $((min_time % 60)) 秒 ($min_time 秒)"
echo "最慢耗时: $((max_time / 60)) 分 $((max_time % 60)) 秒 ($max_time 秒)"

# 输出详细列表
echo ""
echo "详细耗时列表:"
for ((i=1; i<=count; i++)); do
    time=${durations[$i]}
    echo "  第 $i 次: $((time / 60)) 分 $((time % 60)) 秒 ($time 秒)"
done