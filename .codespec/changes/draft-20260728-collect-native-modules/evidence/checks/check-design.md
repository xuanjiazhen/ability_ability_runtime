# Check: design

| 字段 | 内容 |
|------|------|
| 交付件 | design.md |
| 检查时间 | 2026-07-28 |
| 结论 | 通过（自评，待 Owner 评审） |

## 纪律核对（ohos-design Verification Checklist）

- [x] 设计决策有备选对比 + 理由（ADR-1~6，各含 2 个备选及放弃理由）
- [x] design AC ⊆ spec AC（骨架与 TASK 拆分引用的 AC 全部来自 spec AC-1.1~AC-4.2，未发明新行为）
- [x] 分层合规：NAPI 库 → 运行时 → appkit → app_startup 单向调用，无跨层违规、无 IPC/SAF
- [x] 安全基础检查已展开（proposal 安全=是触发）；深度威胁分析判据核对后不产出（已记录原因）
- [x] profile = none，无 profile 专项检查
- [x] 设计审批 11 项 checklist 全部勾选

## 机器校验

```
$ ohos-sdd validate <change> --level C
[FAIL] level=C (3 checks, 1 broken)
  - C spec→plan (): execution-plan.md 缺失 -> ohos-plan   # 预期项：下一阶段产出
```

spec→design 边一致（design.md 存在且被识别）；唯一 broken 为下游 plan 未产出。

## 遗留风险（设计阶段识别，不阻塞）

- RSK-1 napi 仓接口合入排期（高，Owner：yangzk + napi owner）
- RSK-2 refCount 语义待 napi 仓确认（中，含降级方案）
- RSK-3 CCM 参数名与下发通道（中）
- RSK-4 强场景分化预测准确度（低，beta 标定）
