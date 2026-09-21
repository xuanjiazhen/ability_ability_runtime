# Specs 目录说明

本目录由 OHOS SDD 工作流（`.opencode/skills/ohos-sdd/`）自动维护，存放规范驱动开发各阶段产物。

## 目录结构

```
.specs/
├── README.md                  # 本文件
├── registry.md                # 全局产物索引（首次 intake 后生成）
├── features/<FEAT-id>/       # 特性产物
│   ├── manifest.md
│   ├── proposal.md
│   ├── design.md
│   ├── spec.md
│   ├── execution-plan.md
│   ├── tasks/
│   ├── gates/
│   └── review.md
└── bugfixes/<BUG-id>/         # 缺陷产物
```

## 使用入口

通过 `.opencode/commands/ohos-*.md` 斜杠命令驱动各阶段：

- `/ohos-intake`   — Stage 1：需求入口
- `/ohos-clarify`  — Stage 1：逐轮澄清
- `/ohos-baseline` — Stage 1：固化基线
- `/ohos-design`   — Stage 2：架构设计
- `/ohos-spec`     — Stage 2：特性规格
- `/ohos-plan`     — Stage 3：执行计划
- `/ohos-review`   — Stage 3/4：阶段审查

工作流规则见 `.opencode/skills/ohos-sdd/SKILL.md`，流程定义见 `shared/ohos-sdd/workflow.md`。

当前目录尚未初始化任何产物，运行 `/ohos-intake` 后开始生成。
