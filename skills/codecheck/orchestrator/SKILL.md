---
name: codecheck-orchestrator
description: "旧 codecheck orchestrator 路径兼容入口；收到代码检视或深度扫描请求时，转交 Codecheck V2 自适应检视 Skill。"
---

# 旧入口兼容：转交 Codecheck V2

> 此路径只用于旧调用者兼容。不要调用已移除的 `scanners/`、`patterns/`，也不要执行旧的路径分类调度。

读取并完整执行 [`../SKILL.md`](../SKILL.md)；按需引用该文件下的 `references/`、`conventions.md` 与 `codecheck_report_TEMPLATE.md`。按 V2 的范围、证据和交付契约执行；Challenge 根据代码风险自适应深入，不强制固定反例数量。
