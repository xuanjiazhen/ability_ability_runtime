# .codespec

本目录为 OpenHarmony SDD（ohos-sdd）流程交付件归档目录，遵循插件共享资产 `workflow/spec-repository-structure.md` 的目录契约。

## 目录说明

| 目录 | 用途 |
|------|------|
| `changes/` | 活跃变更交付件（proposal / manifest / design / spec / execution-plan / evidence） |
| `registry.md` | 全局索引，记录所有变更的状态和关系 |

## 流程

交付件依赖骨干：`proposal → spec → design → execution-plan → code`，`evidence/{checks,reviews}` 旁证。
声称完成前必跑 `ohos-sdd validate . --level all` 守门。
