# Check: execution-plan

| 字段 | 内容 |
|------|------|
| 交付件 | execution-plan.md |
| 检查时间 | 2026-07-28 |
| 结论 | 通过 |

## 纪律核对（ohos-plan Verification Checklist）

- [x] spec 每 AC 在 plan 覆盖（AC-1.1~AC-4.2 共 16 条 → TASK-2~TASK-7，映射表双向一致：spec.md 验收追溯表已回填 TASK 编号）
- [x] 受影响文件清单完整（15 行，覆盖 design 调用链全部层：napi 库/运行时/appkit/app_startup/构建/测试）
- [x] 生成文件声明生成源（本特性无 bridge/IDL/CAPI/cpptoc 生成文件；JSON schema 由 collector 代码生成，已在 TASK-3/TASK-7 声明）
- [x] 无 threat-model.md（设计阶段判据核对后不产出，无需缓解措施落 Task）
- [x] profile = none，无 profile 专项检查
- [x] 10 张 Task 卡均自包含（Files/Spec Context/Design Context/Required Rules/Steps/Handoff Summary）
- [x] 禁止项 6 条全部通过（无 TBD/占位符/隐式依赖/模糊指令）

## 机器校验

```
$ ohos-sdd validate <change> --level B
[PASS] level=B·锚点标题· (5 checks, 0 broken, 0 warn)

$ ohos-sdd validate <change> --level C
[PASS] level=C·依赖边一致· (4 checks, 0 broken, 0 warn)

$ ohos-sdd validate <change> --level all
[FAIL] level=all (31 checks, 1 broken)
  - D review (): 无 evidence/reviews/spec-compliance.md 且 review.md 为空 -> ohos-validate
```

说明：Level B/C 全过；Level all 唯一 broken 为 D 级归档就绪项（审查证据），属 ohos-review/ohos-validate 阶段产出，计划阶段预期缺失。

## 阶段划分

- Phase-1 跨仓接口 + 骨架：TASK-SKELETON-1, TASK-1, TASK-2
- Phase-2 采集与更新核心：TASK-3, TASK-4, TASK-5
- Phase-3 预加载消费 + 防自反馈：TASK-6, TASK-7
- Phase-4 参数化与齐套：TASK-8, TASK-9

## 跨仓前置动作（不阻塞文档链，阻塞 Phase-1 编码）

- RSK-1：napi 仓 owner 对齐 GetLoadedModuleInfos 接口签名（design.md「API 签名」节）
- RSK-2：TASK-1 中确认 refCount cache 命中递增语义（不满足则 TASK-7 启用降级）
