#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
分析so依赖数据对比 - 正确版本
"""

def parse_so_file(filepath):
    """解析so文件数据"""
    data = {}
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    # 表头固定格式：
    # 列0: so_name
    # 列1: mem(KB)
    # 列2-15: 依赖次数 (AMS, DATASHARE, BMS, ANS, FMS, WMS, ERMS, CALL, DMS, CES, STATE, SLS, 应用域名校验, 通知)
    # 列16-29: 分摊内存 (AMS, DATASHARE, BMS, ANS, FMS, WMS, ERMS, CALL, DMS, CES, STATE, SLS, 应用域名校验, 通知)
    ams_count_idx = 2
    ams_mem_idx = 16

    # 读取数据行
    total_ams_mem = 0
    for line in lines[1:]:
        line = line.strip()
        if not line:
            continue

        parts = line.split('\t')
        so_name = parts[0] if len(parts) > 0 else ""

        # AMS依赖次数
        ams_count = 0
        if len(parts) > ams_count_idx and parts[ams_count_idx]:
            try:
                ams_count = int(parts[ams_count_idx])
            except (ValueError, TypeError):
                ams_count = 0

        # AMS分摊内存
        ams_mem = 0
        if len(parts) > ams_mem_idx and parts[ams_mem_idx]:
            try:
                ams_mem = float(parts[ams_mem_idx])
            except (ValueError, TypeError):
                ams_mem = 0

        # mem(KB)
        mem_kb = 0
        if len(parts) > 1 and parts[1]:
            try:
                mem_kb = float(parts[1])
            except (ValueError, TypeError):
                mem_kb = 0

        data[so_name] = {
            'mem_kb': mem_kb,
            'ams_count': ams_count,
            'ams_mem': ams_mem
        }

        total_ams_mem += ams_mem

    print(f"文件: {filepath}")
    print(f"AMS总内存: {total_ams_mem:.9f} KB")
    return data


def analyze_comparison(file1_path, file2_path):
    """对比两个文件"""
    data1 = parse_so_file(file1_path)
    data2 = parse_so_file(file2_path)

    # 获取所有的so名称
    all_so = set(data1.keys()) | set(data2.keys())

    print("\n" + "=" * 120)
    print("AMS SO依赖数据对比分析")
    print("=" * 120)
    print(f"{'SO名称':<55} {'版本30':<12} {'最新版本':<12} {'依赖次数变化':<12} {'内存变化(KB)':<15}")
    print("-" * 120)

    new_ams_sos = []  # 新增AMS依赖的so
    removed_ams_sos = []  # 移除AMS依赖的so
    ams_count_changes = []  # AMS依赖次数变化的so
    ams_mem_changes = []  # AMS分摊内存变化的so

    total_ams_mem_30 = 0
    total_ams_mem_new = 0

    for so_name in sorted(all_so):
        info1 = data1.get(so_name, {'mem_kb': 0, 'ams_count': 0, 'ams_mem': 0})
        info2 = data2.get(so_name, {'mem_kb': 0, 'ams_count': 0, 'ams_mem': 0})

        ams_count1 = info1['ams_count']
        ams_count2 = info2['ams_count']
        ams_mem1 = info1['ams_mem']
        ams_mem2 = info2['ams_mem']

        # 统计AMS总内存
        total_ams_mem_30 += ams_mem1
        total_ams_mem_new += ams_mem2

        # 检查新增AMS依赖
        if ams_count1 == 0 and ams_count2 > 0:
            new_ams_sos.append((so_name, ams_count2, ams_mem2))

        # 检查移除AMS依赖
        if ams_count1 > 0 and ams_count2 == 0:
            removed_ams_sos.append((so_name, ams_count1, ams_mem1))

        # 检查AMS依赖次数变化
        if ams_count1 != ams_count2:
            ams_count_changes.append((so_name, ams_count1, ams_count2, ams_mem1, ams_mem2))

        # 检查AMS分摊内存变化
        if ams_mem1 != ams_mem2:
            ams_mem_changes.append((so_name, ams_mem1, ams_mem2))

    # 输出详细变化 - 只输出有AMS相关变化的SO
    for so_name in sorted(all_so):
        info1 = data1.get(so_name, {'mem_kb': 0, 'ams_count': 0, 'ams_mem': 0})
        info2 = data2.get(so_name, {'mem_kb': 0, 'ams_count': 0, 'ams_mem': 0})

        ams_count1 = info1['ams_count']
        ams_count2 = info2['ams_count']
        ams_mem1 = info1['ams_mem']
        ams_mem2 = info2['ams_mem']

        # 只输出AMS有变化或AMS有依赖的SO
        if ams_count1 > 0 or ams_count2 > 0 or ams_mem1 > 0 or ams_mem2 > 0:
            count_change = f"{ams_count1}->{ams_count2}" if ams_count1 != ams_count2 else "不变"
            mem_change = f"{ams_mem1:.4f}->{ams_mem2:.4f}" if ams_mem1 != ams_mem2 else "不变"
            print(f"{so_name:<55} {ams_count1:<4} {ams_count2:<4} {count_change:<12} {mem_change:<15}")

    print()
    print("=" * 120)
    print("新增AMS依赖的SO:")
    print("=" * 120)
    if new_ams_sos:
        print(f"{'SO名称':<55} {'依赖次数':<10} {'分摊内存(KB)':<15}")
        print("-" * 85)
        for so_name, count, mem in sorted(new_ams_sos, key=lambda x: x[2], reverse=True):
            print(f"{so_name:<55} {count:<10} {mem:<15.4f}")
    else:
        print("无新增AMS依赖的SO")

    print()
    print("=" * 120)
    print("移除AMS依赖的SO:")
    print("=" * 120)
    if removed_ams_sos:
        print(f"{'SO名称':<55} {'依赖次数':<10} {'分摊内存(KB)':<15}")
        print("-" * 85)
        for so_name, count, mem in sorted(removed_ams_sos, key=lambda x: x[2], reverse=True):
            print(f"{so_name:<55} {count:<10} {mem:<15.4f}")
    else:
        print("无移除AMS依赖的SO")

    print()
    print("=" * 120)
    print("AMS依赖次数变化的SO (排除新增和移除):")
    print("=" * 120)
    # 过滤掉新增和移除的
    filtered_count_changes = [(n, c1, c2, m1, m2) for (n, c1, c2, m1, m2) in ams_count_changes
                             if not (c1 == 0 or c2 == 0)]
    if filtered_count_changes:
        print(f"{'SO名称':<55} {'版本30':<8} {'最新版本':<10} {'内存30(KB)':<12} {'内存最新(KB)':<12}")
        print("-" * 105)
        for so_name, c1, c2, m1, m2 in filtered_count_changes:
            print(f"{so_name:<55} {c1:<8} {c2:<10} {m1:<12.4f} {m2:<12.4f}")
    else:
        print("无AMS依赖次数变化的SO")

    print()
    print("=" * 120)
    print("AMS分摊内存增大的SO (按增大值排序):")
    print("=" * 120)
    mem_increases = [(so_name, m1, m2, m2-m1) for so_name, m1, m2 in ams_mem_changes if m2 > m1]
    if mem_increases:
        mem_increases.sort(key=lambda x: x[3], reverse=True)
        print(f"{'SO名称':<55} {'版本30(KB)':<12} {'最新版本(KB)':<14} {'增加(KB)':<12} {'增加%':<10}")
        print("-" * 115)
        for so_name, m1, m2, delta in mem_increases:
            pct = (delta / m1 * 100) if m1 > 0 else float('inf')
            print(f"{so_name:<55} {m1:<12.4f} {m2:<14.4f} {delta:<12.4f} {pct:<10.2f}%")
    else:
        print("无AMS分摊内存增大的SO")

    print()
    print("=" * 120)
    print("AMS分摊内存减小的SO (按减小值排序):")
    print("=" * 120)
    mem_decreases = [(so_name, m1, m2, m1-m2) for so_name, m1, m2 in ams_mem_changes if m2 < m1]
    if mem_decreases:
        mem_decreases.sort(key=lambda x: x[3], reverse=True)
        print(f"{'SO名称':<55} {'版本30(KB)':<12} {'最新版本(KB)':<14} {'减小(KB)':<12} {'减小%':<10}")
        print("-" * 115)
        for so_name, m1, m2, delta in mem_decreases:
            pct = (delta / m1 * 100) if m1 > 0 else 0
            print(f"{so_name:<55} {m1:<12.4f} {m2:<14.4f} {delta:<12.4f} {pct:<10.2f}%")
    else:
        print("无AMS分摊内存减小的SO")

    print()
    print("=" * 120)
    print("总体统计:")
    print("=" * 120)
    print(f"版本30 AMS总依赖内存: {total_ams_mem_30:.9f} KB")
    print(f"最新版本 AMS总依赖内存: {total_ams_mem_new:.9f} KB")
    print(f"AMS总内存变化: {total_ams_mem_new - total_ams_mem_30:.9f} KB ({(total_ams_mem_new - total_ams_mem_30)/total_ams_mem_30*100:.2f}%)")
    print(f"新增AMS依赖的SO数量: {len(new_ams_sos)}")
    print(f"移除AMS依赖的SO数量: {len(removed_ams_sos)}")
    print(f"AMS依赖次数变化的SO数量: {len(filtered_count_changes)}")
    print(f"AMS分摊内存变化的SO数量: {len(ams_mem_changes)}")
    print(f"AMS分摊内存增大的SO数量: {len(mem_increases)}")
    print(f"AMS分摊内存减小的SO数量: {len(mem_decreases)}")


if __name__ == "__main__":
    file1 = r"D:\code\26-1-13ability_runtime\ability_ability_runtime\30_so.txt"
    file2 = r"D:\code\26-1-13ability_runtime\ability_ability_runtime\最新版本so数据.txt"
    analyze_comparison(file1, file2)
