# Check: spec

| 字段 | 内容 |
|------|------|
| 交付件 | spec.md |
| 检查时间 | 2026-07-28 |
| 结论 | 通过 |

## 纪律核对（ohos-spec Verification Checklist）

- [x] AC 用 WHEN/THEN，可测可度量（AC-1.1~AC-4.2 共 16 条，数值阈值明确：α=0.4、0.5、16 轮、0.05、1024 条、0600）
- [x] 验收追溯 ≥1 AC（16 条 AC 全部关联规则 R-1~R-14，覆盖 proposal 四项成功指标）
- [x] 无实现细节（框架类名/内部链路仅出现在 context-references 查询意向中；正文只写行为契约）
- [x] 兼容性声明完整（API 行为不变、新增文件 version 化、缺省关闭）；API 变更清单与 proposal 锚点一致（GetLoadedModuleInfos + NativeModuleInfo，InnerApi）
- [x] profile = none，无 profile 专项检查
- [x] 自审清单逐项通过（无占位符/无模糊表述/AC↔规则交叉一致）

## 机器校验

```
$ ohos-sdd validate <change> --level B
[PASS] level=B·锚点标题· (4 checks, 0 broken, 0 warn)

$ ohos-sdd validate <change> --level C
[FAIL] level=C (3 checks, 1 broken)
  - C spec→plan (): execution-plan.md 缺失 -> ohos-plan   # 预期项：下一阶段 ohos-plan 产出
```

spec→design 依赖边一致（gaps=0）；唯一 broken 为下游 plan 未产出，属本阶段预期状态。
