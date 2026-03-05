# -*- coding: utf-8 -*-
"""生成最新版本 vs 6.0 SO依赖分析报告"""

def parse_so_file(filepath):
    """解析so文件数据"""
    data = {}
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    ams_count_idx = 2
    ams_mem_idx = 16

    for line in lines[1:]:
        line = line.strip()
        if not line:
            continue

        parts = line.split('\t')
        so_name = parts[0] if len(parts) > 0 else ''

        ams_count = 0
        if len(parts) > ams_count_idx and parts[ams_count_idx]:
            try:
                ams_count = int(parts[ams_count_idx])
            except:
                ams_count = 0

        ams_mem = 0
        if len(parts) > ams_mem_idx and parts[ams_mem_idx]:
            try:
                ams_mem = float(parts[ams_mem_idx])
            except:
                ams_mem = 0

        mem_kb = 0
        if len(parts) > 1 and parts[1]:
            try:
                mem_kb = float(parts[1])
            except:
                mem_kb = 0

        data[so_name] = {'mem_kb': mem_kb, 'ams_count': ams_count, 'ams_mem': ams_mem}

    return data


def analyze_comparison(file1_path, file2_path, file1_name, file2_name):
    """对比两个文件"""
    data1 = parse_so_file(file1_path)
    data2 = parse_so_file(file2_path)

    all_so = set(data1.keys()) | set(data2.keys())

    new_ams_sos = []
    removed_ams_sos = []
    ams_count_changes = []
    ams_mem_changes = []

    total_ams_mem_1 = 0
    total_ams_mem_2 = 0

    for so_name in sorted(all_so):
        info1 = data1.get(so_name, {'mem_kb': 0, 'ams_count': 0, 'ams_mem': 0})
        info2 = data2.get(so_name, {'mem_kb': 0, 'ams_count': 0, 'ams_mem': 0})

        ams_count1 = info1['ams_count']
        ams_count2 = info2['ams_count']
        ams_mem1 = info1['ams_mem']
        ams_mem2 = info2['ams_mem']

        total_ams_mem_1 += ams_mem1
        total_ams_mem_2 += ams_mem2

        if ams_count1 == 0 and ams_count2 > 0:
            new_ams_sos.append((so_name, ams_count2, ams_mem2))

        if ams_count1 > 0 and ams_count2 == 0:
            removed_ams_sos.append((so_name, ams_count1, ams_mem1))

        if ams_count1 != ams_count2:
            ams_count_changes.append((so_name, ams_count1, ams_count2, ams_mem1, ams_mem2))

        if ams_mem1 != ams_mem2:
            ams_mem_changes.append((so_name, ams_mem1, ams_mem2))

    output = []
    output.append('=' * 120)
    output.append(f'AMS SO依赖数据对比分析 ({file1_name} vs {file2_name})')
    output.append('=' * 120)
    output.append('')
    output.append(f'{file1_name} AMS总依赖内存: {total_ams_mem_1:.9f} KB')
    output.append(f'{file2_name} AMS总依赖内存: {total_ams_mem_2:.9f} KB')
    mem_change = total_ams_mem_1 - total_ams_mem_2  # 最新版本 - 6.0版本
    mem_change_pct = (mem_change / total_ams_mem_2 * 100) if total_ams_mem_2 > 0 else float("inf")
    output.append(f'AMS总内存变化: {mem_change:+.9f} KB ({mem_change_pct:+.2f}%)')
    output.append('')
    output.append('=' * 120)
    output.append('详细对比列表')
    output.append('=' * 120)
    output.append(f'{"SO名称":<50} {file1_name}依赖 {file2_name}依赖 {"依赖变化":<10} {file1_name+"内存(KB)":<12} {file2_name+"内存(KB)":<12}')
    output.append('-' * 120)

    for so_name in sorted(all_so):
        info1 = data1.get(so_name, {'mem_kb': 0, 'ams_count': 0, 'ams_mem': 0})
        info2 = data2.get(so_name, {'mem_kb': 0, 'ams_count': 0, 'ams_mem': 0})

        ams_count1 = info1['ams_count']
        ams_count2 = info2['ams_count']
        ams_mem1 = info1['ams_mem']
        ams_mem2 = info2['ams_mem']

        if ams_count1 > 0 or ams_count2 > 0 or ams_mem1 > 0 or ams_mem2 > 0:
            count_change = f'{ams_count1}->{ams_count2}' if ams_count1 != ams_count2 else '不变'
            output.append(f'{so_name:<50} {ams_count1:<4} {ams_count2:<4} {count_change:<10} {ams_mem1:<12.4f} {ams_mem2:<12.4f}')

    output.append('')
    output.append('=' * 120)
    output.append(f'新增AMS依赖的SO ({file1_name}中无，{file2_name}中有)')
    output.append('=' * 120)
    if new_ams_sos:
        output.append(f'{"SO名称":<50} {"依赖次数":<8} {"分摊内存(KB)":<12}')
        output.append('-' * 75)
        for so_name, count, mem in sorted(new_ams_sos, key=lambda x: x[2], reverse=True):
            output.append(f'{so_name:<50} {count:<4} {mem:<12.4f}')
    else:
        output.append('无新增AMS依赖的SO')

    output.append('')
    output.append('=' * 120)
    output.append(f'移除AMS依赖的SO ({file1_name}中有，{file2_name}中无)')
    output.append('=' * 120)
    if removed_ams_sos:
        output.append(f'{"SO名称":<50} {"依赖次数":<8} {"分摊内存(KB)":<12}')
        output.append('-' * 75)
        for so_name, count, mem in sorted(removed_ams_sos, key=lambda x: x[2], reverse=True):
            output.append(f'{so_name:<50} {count:<4} {mem:<12.4f}')
    else:
        output.append('无移除AMS依赖的SO')

    output.append('')
    output.append('=' * 120)
    output.append(f'AMS依赖次数变化的SO (排除新增和移除，{file1_name}->{file2_name})，最新版本内存变大')
    output.append('=' * 120)
    # 过滤最新版本内存 > 6.0版本内存的SO，并按内存增加量排序
    filtered_count_changes = [(n, c1, c2, m1, m2, m1-m2) for (n, c1, c2, m1, m2) in ams_count_changes if not (c1 == 0 or c2 == 0) and m1 > m2]
    if filtered_count_changes:
        filtered_count_changes.sort(key=lambda x: x[5], reverse=True)
        output.append(f'{"SO名称":<50} {file1_name}依赖 {file2_name}依赖 {file2_name+"内存(KB)":<12} {file1_name+"内存(KB)":<12} {"增加(KB)":<10}')
        output.append('-' * 105)
        for so_name, c1, c2, m1, m2, delta in filtered_count_changes:
            pct = (delta / m2 * 100) if m2 > 0 else float('inf')
            output.append(f'{so_name:<50} {c1:<4} {c2:<4} {m2:<12.4f} {m1:<12.4f} {delta:<10.4f}')
    else:
        output.append('无AMS依赖次数变化的SO')

    output.append('')
    output.append('=' * 120)
    output.append(f'AMS分摊内存增大的SO (按增大值排序，{file2_name}->{file1_name})')
    output.append('=' * 120)
    mem_increases = [(so_name, m1, m2, m1-m2) for so_name, m1, m2 in ams_mem_changes if m1 > m2]
    if mem_increases:
        mem_increases.sort(key=lambda x: x[3], reverse=True)
        output.append(f'{"SO名称":<50} {file2_name+"内存(KB)":<12} {file1_name+"内存(KB)":<12} {"增加(KB)":<10} {"增加%":<8}')
        output.append('-' * 100)
        for so_name, m1, m2, delta in mem_increases:
            pct = (delta / m2 * 100) if m2 > 0 else float('inf')
            output.append(f'{so_name:<50} {m2:<12.4f} {m1:<12.4f} {delta:<10.4f} {pct:<8.2f}%')
    else:
        output.append('无AMS分摊内存增大的SO')

    output.append('')
    output.append('=' * 120)
    output.append(f'AMS分摊内存减小的SO (按减小值排序，{file2_name}->{file1_name})')
    output.append('=' * 120)
    mem_decreases = [(so_name, m1, m2, m2-m1) for so_name, m1, m2 in ams_mem_changes if m1 < m2]
    if mem_decreases:
        mem_decreases.sort(key=lambda x: x[3], reverse=True)
        output.append(f'{"SO名称":<50} {file2_name+"内存(KB)":<12} {file1_name+"内存(KB)":<12} {"减小(KB)":<10} {"减小%":<8}')
        output.append('-' * 100)
        for so_name, m1, m2, delta in mem_decreases:
            pct = (delta / m2 * 100) if m2 > 0 else 0
            output.append(f'{so_name:<50} {m2:<12.4f} {m1:<12.4f} {delta:<10.4f} {pct:<8.2f}%')
    else:
        output.append('无AMS分摊内存减小的SO')

    output.append('')
    output.append('=' * 120)
    output.append('总体统计')
    output.append('=' * 120)
    output.append(f'{file1_name} AMS总依赖内存: {total_ams_mem_1:.9f} KB')
    output.append(f'{file2_name} AMS总依赖内存: {total_ams_mem_2:.9f} KB')
    output.append(f'AMS总内存变化 ({file2_name}->{file1_name}): {mem_change:+.9f} KB ({mem_change_pct:+.2f}%)')
    output.append(f'新增AMS依赖的SO数量: {len(new_ams_sos)}')
    output.append(f'移除AMS依赖的SO数量: {len(removed_ams_sos)}')
    output.append(f'AMS依赖次数变化的SO数量: {len(filtered_count_changes)}')
    output.append(f'AMS分摊内存变化的SO数量: {len(ams_mem_changes)}')
    output.append(f'AMS分摊内存增大的SO数量: {len(mem_increases)}')
    output.append(f'AMS分摊内存减小的SO数量: {len(mem_decreases)}')

    return '\n'.join(output)


if __name__ == '__main__':
    file1 = r'D:\code\26-1-13ability_runtime\ability_ability_runtime\最新版本so数据.txt'
    file2 = r'D:\code\26-1-13ability_runtime\ability_ability_runtime\6.0_so.txt'
    file1_name = '最新版本'
    file2_name = '6.0版本'

    report = analyze_comparison(file1, file2, file1_name, file2_name)

    output_file = 'AMS_SO依赖分析报告_最新vs6.0.txt'
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(report)

    print(f'分析报告已生成: {output_file}')
