#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
分析SO数据对比脚本
对比30_so.txt和最新版本so数据.txt，找出AMS相关SO的依赖增长
"""

import sys

def parse_so_file(filepath):
    """解析SO文件，返回字典 {so_name: (mem, ams_value)}"""
    result = {}
    with open(filepath, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('so_name'):
                continue

            parts = line.split('\t')
            if len(parts) < 2:
                continue

            so_name = parts[0].strip()
            mem = float(parts[1]) if parts[1] else 0

            # 找到AMS列（第一个AMS列）
            ams_value = None
            for i, p in enumerate(parts):
                if p == 'AMS' and ams_value is None:
                    ams_value = int(p) if p.isdigit() else 0
                    break

            result[so_name] = {
                'mem': mem,
                'ams_value': ams_value
            }
    return result

def analyze():
    """分析两个文件的差异"""
    old_file = r"D:\code\26-1-13ability_runtime\ability_ability_runtime\30_so.txt"
    new_file = r"D:\code\26-1-13ability_runtime\ability_ability_runtime\最新版本so数据.txt"

    # 解析两个文件
    old_data = parse_so_file(old_file)
    new_data = parse_so_file(new_file)

    # 分析AMS新增的SO
    ams_new_sos = []
    ams_increased_sos = []

    print("=" * 80)
    print("AMS新增SO依赖分析")
    print("=" * 80)
    print()

    # 找出在旧文件中不存在但在新文件中存在的SO
    old_so_names = set(old_data.keys())
    new_so_names = set(new_data.keys())

    # 新增的SO（只在30_so.txt中不存在，但在最新版本中存在）
    newly_added = new_so_names - old_so_names

    if newly_added:
        print(f"【新增的SO（{len(newly_added)}个，这些SO不在30_so.txt中）】")
        print("-" * 80)
        for so in sorted(newly_added):
            mem = new_data[so]['mem']
            ams = new_data[so]['ams_value'] if new_data[so]['ams_value'] is not None else '无'
            print(f"  {so:50s}  mem={mem:.2f} KB  AMS={ams}")
        print()

    # 共有的SO，分析AMS依赖次数变化
    common_sos = old_so_names & new_so_names

    # AMS依赖次数增加的SO（AMS值增加1次）
    print(f"【AMS依赖次数增加的SO（{len(common_sos)}个共有SO中筛选）】")
    print("-" * 80)

    for so in sorted(common_sos):
        old_ams = old_data[so]['ams_value'] if old_data[so]['ams_value'] is not None else 0
        new_ams = new_data[so]['ams_value'] if new_data[so]['ams_value'] is not None else 0
        old_mem = old_data[so]['mem']
        new_mem = new_data[so]['mem']

        # 判断是否AMS新增了1次
        if new_ams == old_ams + 1:
            ams_new_sos.append(so)
            mem_increase = new_mem - old_mem
            ams_increased_sos.append(so)
            print(f"  {so:50s}  AMS: {old_ams} -> {new_ams} (+1)  mem: {old_mem:.2f} -> {new_mem:.2f} KB ({mem_increase:+.2f} KB)")
        elif new_ams != old_ams:
            print(f"  {so:50s}  AMS: {old_ams} -> {new_ams} ({new_ams - old_ams:+d})  mem: {old_mem:.2f} -> {new_mem:.2f} KB ({new_mem - old_mem:+.2f} KB)")

    print()
    print("=" * 80)

    # 统计AMS新增的SO的内存增长
    if ams_increased_sos:
        total_mem_increase = sum(new_data[so]['mem'] - old_data[so]['mem'] for so in ams_increased_sos if so in old_data)
        print(f"【AMS新增依赖的SO内存统计】")
        print("-" * 80)
        print(f"  AMS新增1次依赖的SO数量: {len(ams_increased_sos)}")
        print(f"  这些SO的内存总增长: {total_mem_increase:.2f} KB")
        print(f"  平均每SO增长: {total_mem_increase / len(ams_increased_sos):.2f} KB")
        print()

        # 按内存增长量排序
        print(f"  按内存增长量排序:")
        so_increases = [(so, new_data[so]['mem'] - old_data[so]['mem']) for so in ams_increased_sos if so in old_data]
        so_increases.sort(key=lambda x: x[1], reverse=True)
        for i, (so, inc) in enumerate(so_increases[:10], 1):
            print(f"    {i+1}. {so:50s}  +{inc:.2f} KB")
        if len(so_increases) > 10:
            print(f"    ... 共{len(so_increases)}个")
        print()

    # 所有SO的内存增长统计
    all_increased = []
    all_decreased = []

    for so in sorted(common_sos):
        if so in old_data:
            old_mem = old_data[so]['mem']
            new_mem = new_data[so]['mem']
            diff = new_mem - old_mem
            if diff > 0:
                all_increased.append((so, diff))
            elif diff < 0:
                all_decreased.append((so, diff))

    print(f"【所有SO内存变化统计】")
    print("-" * 80)
    total_increase = sum(v for _, v in all_increased)
    total_decrease = sum(abs(v) for _, v in all_decreased)
    print(f"  内存增长的SO数量: {len(all_increased)}")
    print(f"  内存减少的SO数量: {len(all_decreased)}")
    print(f"  总内存增长: {total_increase:.2f} KB")
    print(f"  总内存减少: {total_decrease:.2f} KB")
    print(f"  净内存变化: {total_increase - total_decrease:.2f} KB")
    print()

    if all_increased:
        print(f"  内存增长最多的SO (Top 10):")
        all_increased.sort(key=lambda x: x[1], reverse=True)
        for i, (so, inc) in enumerate(all_increased[:10], 1):
            print(f"    {i+1}. {so:50s}  +{inc:.2f} KB")

    if all_decreased:
        print()
        print(f"  内存减少最多的SO (Top 10):")
        all_decreased.sort(key=lambda x: x[1])
        for i, (so, dec) in enumerate(all_decreased[:10], 1):
            print(f"    {i+1}. {so:50s}  {dec:.2f} KB")

if __name__ == "__main__":
    analyze()
