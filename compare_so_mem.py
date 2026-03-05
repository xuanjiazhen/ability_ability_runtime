# -*- coding: utf-8 -*-
"""对比最新版本与6.0版本SO内存大小变化"""

def parse_so_file(filepath):
    """解析so文件数据，返回SO名称和内存大小"""
    data = {}
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    for line in lines[1:]:  # 跳过标题行
        line = line.strip()
        if not line:
            continue

        parts = line.split('\t')
        so_name = parts[0] if len(parts) > 0 else ''

        mem_kb = 0
        if len(parts) > 1 and parts[1]:
            try:
                mem_kb = float(parts[1])
            except:
                mem_kb = 0

        data[so_name] = mem_kb

    return data


def analyze_comparison(file1_path, file2_path, file1_name, file2_name):
    """对比两个文件"""
    data1 = parse_so_file(file1_path)  # 最新版本
    data2 = parse_so_file(file2_path)  # 6.0版本

    all_so = set(data1.keys()) | set(data2.keys())

    # 找出变大的SO（最新版本 > 6.0版本）
    increased_sos = []
    for so_name in sorted(all_so):
        mem1 = data1.get(so_name, 0)
        mem2 = data2.get(so_name, 0)

        if mem1 > mem2:
            delta = mem1 - mem2
            pct = (delta / mem2 * 100) if mem2 > 0 else float('inf')
            increased_sos.append((so_name, mem2, mem1, delta, pct))

    # 按增加量从大到小排序
    increased_sos.sort(key=lambda x: x[3], reverse=True)

    # 计算总内存
    total_mem1 = sum(data1.values())
    total_mem2 = sum(data2.values())

    output = []
    output.append('=' * 120)
    output.append(f'SO内存大小对比分析 ({file1_name} vs {file2_name})')
    output.append('=' * 120)
    output.append('')
    output.append(f'{file1_name} SO总内存: {total_mem1:.6f} KB')
    output.append(f'{file2_name} SO总内存: {total_mem2:.6f} KB')
    output.append(f'SO总内存变化: {total_mem1 - total_mem2:+.6f} KB ({(total_mem1 - total_mem2)/total_mem2*100 if total_mem2 > 0 else float("inf"):+.2f}%)')
    output.append('')
    output.append('=' * 120)
    output.append(f'变大的SO (按内存增加量从大到小排序，{file2_name}->{file1_name})')
    output.append('=' * 120)
    output.append(f'{"SO名称":<50} {file2_name+"内存(KB)":<15} {file1_name+"内存(KB)":<15} {"增加(KB)":<12} {"增加%":<10}')
    output.append('-' * 105)

    for so_name, mem2, mem1, delta, pct in increased_sos:
        output.append(f'{so_name:<50} {mem2:<15.6f} {mem1:<15.6f} {delta:<12.6f} {pct:<10.2f}%')

    output.append('')
    output.append('=' * 120)
    output.append('总体统计')
    output.append('=' * 120)
    output.append(f'{file1_name} SO数量: {len(data1)}')
    output.append(f'{file2_name} SO数量: {len(data2)}')
    output.append(f'变大的SO数量: {len(increased_sos)}')
    output.append(f'{file1_name} SO总内存: {total_mem1:.6f} KB')
    output.append(f'{file2_name} SO总内存: {total_mem2:.6f} KB')
    output.append(f'SO总内存变化: {total_mem1 - total_mem2:+.6f} KB ({(total_mem1 - total_mem2)/total_mem2*100 if total_mem2 > 0 else float("inf"):+.2f}%)')

    return '\n'.join(output)


if __name__ == '__main__':
    file1 = r'D:\code\26-1-13ability_runtime\ability_ability_runtime\最新版本so数据.txt'
    file2 = r'D:\code\26-1-13ability_runtime\ability_ability_runtime\6.0_so.txt'
    file1_name = '最新版本'
    file2_name = '6.0版本'

    report = analyze_comparison(file1, file2, file1_name, file2_name)

    output_file = 'SO内存大小对比_最新vs6.0.txt'
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(report)

    print(f'分析报告已生成: {output_file}')
