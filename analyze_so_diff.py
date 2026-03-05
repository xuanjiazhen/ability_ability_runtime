#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
from collections import defaultdict

def parse_so_file(file_path):
    """解析SO文件，返回SO信息字典"""
    so_info = {}

    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    # 跳过第一行（表头）
    for line in lines[1:]:
        # 使用制表符分割
        parts = line.strip().split('\t')
        if len(parts) < 3:
            continue

        so_name = parts[0]
        try:
            mem_kb = float(parts[1])
            ams_count = int(parts[2])

            # AMS分摊内存在索引16（第17列）
            if len(parts) > 16:
                ams_mem = float(parts[16])
            else:
                ams_mem = 0.0

            so_info[so_name] = {
                'mem_kb': mem_kb,
                'ams_count': ams_count,
                'ams_mem': ams_mem
            }
        except ValueError:
            # 跳过无法解析的行
            continue

    return so_info

def analyze_so_diff(file1_path, file2_path, name1="30版本", name2="最新版本"):
    """分析两个SO文件的差异"""
    # 解析文件
    so1 = parse_so_file(file1_path)
    so2 = parse_so_file(file2_path)

    # 获取所有SO名称
    all_so_names = set(so1.keys()) | set(so2.keys())

    # 统计
    added_sos = []
    removed_sos = []
    changed_sos = []

    total_mem_1 = 0
    total_mem_2 = 0
    total_ams_mem_1 = 0
    total_ams_mem_2 = 0

    # 计算30版本的总值
    for so_name, info in so1.items():
        total_mem_1 += info['mem_kb']
        total_ams_mem_1 += info['ams_mem']

    # 计算最新版本的总值
    for so_name, info in so2.items():
        total_mem_2 += info['mem_kb']
        total_ams_mem_2 += info['ams_mem']

    # 比较每个SO
    for so_name in all_so_names:
        if so_name in so1 and so_name not in so2:
            removed_sos.append(so_name)
        elif so_name not in so1 and so_name in so2:
            added_sos.append(so_name)
        elif so_name in so1 and so_name in so2:
            info1 = so1[so_name]
            info2 = so2[so_name]

            # 检查是否有变化
            if (info1['mem_kb'] != info2['mem_kb'] or
                info1['ams_count'] != info2['ams_count'] or
                abs(info1['ams_mem'] - info2['ams_mem']) > 0.001):
                changed_sos.append((so_name, info1, info2))

    return {
        'added_sos': added_sos,
        'removed_sos': removed_sos,
        'changed_sos': changed_sos,
        'total_mem_1': total_mem_1,
        'total_mem_2': total_mem_2,
        'total_ams_mem_1': total_ams_mem_1,
        'total_ams_mem_2': total_ams_mem_2,
        'so1': so1,
        'so2': so2
    }

def print_analysis_result(result):
    """打印分析结果"""
    print("=" * 80)
    print("SO依赖变化分析结果")
    print("=" * 80)

    # 1. 新增的SO
    print("\n1. 新增的SO文件：")
    if result['added_sos']:
        for so in result['added_sos']:
            info = result['so2'][so]
            print(f"   - {so}: 内存={info['mem_kb']}KB, AMS依赖={info['ams_count']}次, AMS分摊={info['ams_mem']}KB")
    else:
        print("   无新增SO文件")

    # 2. 删除的SO
    print("\n2. 删除的SO文件：")
    if result['removed_sos']:
        for so in result['removed_sos']:
            info = result['so1'][so]
            print(f"   - {so}: 内存={info['mem_kb']}KB, AMS依赖={info['ams_count']}次, AMS分摊={info['ams_mem']}KB")
    else:
        print("   无删除SO文件")

    # 3. 有变化的SO
    print("\n3. AMS依赖有变化的SO文件：")
    if result['changed_sos']:
        for so, info1, info2 in result['changed_sos']:
            print(f"   - {so}:")
            print(f"     内存: {info1['mem_kb']}KB → {info2['mem_kb']}KB (变化: {info2['mem_kb']-info1['mem_kb']:+.1f}KB)")
            print(f"     AMS依赖: {info1['ams_count']}次 → {info2['ams_count']}次 (变化: {info2['ams_count']-info1['ams_count']:+}次)")
            print(f"     AMS分摊: {info1['ams_mem']}KB → {info2['ams_mem']}KB (变化: {info2['ams_mem']-info1['ams_mem']:+.1f}KB)")
    else:
        print("   无AMS依赖变化")

    # 4. 总内存变化
    print("\n4. 内存总量变化：")
    mem_change = result['total_mem_2'] - result['total_mem_1']
    ams_mem_change = result['total_ams_mem_2'] - result['total_ams_mem_1']
    print(f"   总内存: {result['total_mem_1']:.1f}KB → {result['total_mem_2']:.1f}KB (变化: {mem_change:+.1f}KB)")
    print(f"   AMS分摊内存: {result['total_ams_mem_1']:.1f}KB → {result['total_ams_mem_2']:.1f}KB (变化: {ams_mem_change:+.1f}KB)")
    print(f"   AMS分摊内存占比: {result['total_ams_mem_1']/result['total_mem_1']*100:.1f}% → {result['total_ams_mem_2']/result['total_mem_2']*100:.1f}%")

    # 5. AMS依赖次数统计
    print("\n5. AMS依赖次数统计：")
    ams_count_1 = sum(info['ams_count'] for info in result['so1'].values())
    ams_count_2 = sum(info['ams_count'] for info in result['so2'].values())
    print(f"   30版本总AMS依赖次数: {ams_count_1}次")
    print(f"   最新版本总AMS依赖次数: {ams_count_2}次")
    print(f"   变化: {ams_count_2 - ams_count_1:+}次")

    # 6. 按AMS内存增长排序的变化
    print("\n6. AMS分摊内存增长TOP10：")
    # 只显示AMS内存增长的SO
    growth_sos = [(so, info1, info2) for so, info1, info2 in result['changed_sos'] if info2['ams_mem'] > info1['ams_mem']]
    growth_sos.sort(key=lambda x: x[2]['ams_mem'] - x[1]['ams_mem'], reverse=True)
    for i, (so, info1, info2) in enumerate(growth_sos[:10], 1):
        change = info2['ams_mem'] - info1['ams_mem']
        print(f"   {i}. {so}: {info1['ams_mem']:.2f}KB → {info2['ams_mem']:.2f}KB (增长: {change:+.2f}KB)")
    if not growth_sos:
        print("   无AMS内存增长")

def main():
    # 分析文件
    file1 = "30_so.txt"
    file2 = "最新版本so数据.txt"

    print(f"分析 {file1} 和 {file2} 的差异...")
    result = analyze_so_diff(file1, file2)

    # 打印结果
    print_analysis_result(result)

    # 保存详细结果到文件
    with open("so_analysis_diff_detail.txt", "w", encoding="utf-8") as f:
        f.write("SO依赖变化详细分析\n")
        f.write("=" * 80 + "\n\n")

        # 新增的SO
        f.write("新增的SO:\n")
        for so in result['added_sos']:
            info = result['so2'][so]
            f.write(f"{so}: 内存={info['mem_kb']}KB, AMS依赖={info['ams_count']}次, AMS分摊={info['ams_mem']}KB\n")

        # 删除的SO
        f.write("\n删除的SO:\n")
        for so in result['removed_sos']:
            info = result['so1'][so]
            f.write(f"{so}: 内存={info['mem_kb']}KB, AMS依赖={info['ams_count']}次, AMS分摊={info['ams_mem']}KB\n")

        # 有变化的SO
        f.write("\n有变化的SO:\n")
        for so, info1, info2 in result['changed_sos']:
            f.write(f"{so}:\n")
            f.write(f"  内存: {info1['mem_kb']}KB → {info2['mem_kb']}KB (变化: {info2['mem_kb']-info1['mem_kb']:+.1f}KB)\n")
            f.write(f"  AMS依赖: {info1['ams_count']}次 → {info2['ams_count']}次 (变化: {info2['ams_count']-info1['ams_count']:+}次)\n")
            f.write(f"  AMS分摊: {info1['ams_mem']}KB → {info2['ams_mem']}KB (变化: {info2['ams_mem']-info1['ams_mem']:+.1f}KB)\n")

    print("\n详细分析结果已保存到 so_analysis_diff_detail.txt")

if __name__ == "__main__":
    main()