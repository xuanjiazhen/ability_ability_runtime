# Check: proposal（基线冻结证据）

| 字段 | 内容 |
|------|------|
| 交付件 | proposal.md |
| 检查时间 | 2026-07-28 |
| 检查人 | yangzk（需求方确认）+ AI（结构校验） |
| 结论 | 通过（基线 v1.0 冻结） |

## 人工确认项

- [x] 澄清逐项关闭（Q-1 ~ Q-6，两轮澄清 + 消费方具体化，需求方逐条回答）
- [x] 总结确认："启动框架预加载任务读取 predicted 系统模块并预加载；双检查点采集 fork 后应用显式 import 的模块，EWMA 跨启动评分合并更新文件；CCM 参数门控缺省关闭"——需求方回答"确认"（2026-07-28）
- [x] target_release = OpenHarmony-7.1-Release（status: committed）
- [x] 不涉及项已显式 N/A（IPC/跨进程、国际化/无障碍）
- [x] 方案探索 A-1/A-2/A-3 含取舍理由
- [x] manifest.profile = none、lineage = new 已确认

## 机器校验

```
$ ohos-sdd status .codespec/changes/draft-20260728-collect-native-modules
[OK]   proposal  proposal.md
[OK]   manifest  manifest.md
present=2 missing=6 (required-missing=1) gaps=0

$ ohos-sdd validate .codespec/changes/draft-20260728-collect-native-modules --level A
[FAIL] level=A (16 checks, 1 broken)
  - A spec (spec.md): required 交付件缺失 -> ohos-spec   # 预期项：下一阶段 ohos-spec 产出
```

说明：Level A 唯一 broken 项为 spec.md 缺失，属基线冻结时点的预期状态（spec 为下游交付件，由 ohos-spec 能力产出）。proposal 本身校验通过。
