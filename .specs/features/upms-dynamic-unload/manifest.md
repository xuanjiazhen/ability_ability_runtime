# Manifest — UPMS 动态卸载

| 字段 | 值 |
|------|-----|
| id | upms-dynamic-unload |
| title | UPMS (SA 183) 安全延时自卸载 |
| type | feature |
| complexity | standard |
| profile | (none — 无匹配的子系统 profile) |
| target_release | 待确认（OQ-1，见 proposal §3.8） |
| language | zh-CN |
| created | 2026-06-25 |
| owner | wangzhen |

## Lineage / 相关链接
- **来源分析文档**：
  - [unload_upms.md](../../../../unload_upms.md) — 动态卸载可行性评估（结论：不建议，但给出最小实现）
  - [upms_mem_op.md](../../../../upms_mem_op.md) — 内存优化建议（依赖裁剪 / 数据层 / 代理冗余）
- **实现先例**：SRMS (SA 404) 延时自卸载 [service_router_mgr_service.cpp:86-111](../../../../service_router_framework/services/srms/src/service_router_mgr_service.cpp)
- **目标 SA**：Uri Permission Manager Service，SA 183，库 `libupms.z.so`，运行于 `foundation` 进程，`run-on-create: false`

## Stage 状态
- [x] **Stage 1 Define** — Approved 2026-06-25（需求方确认理解、范围/排除项、复杂度）
- [x] **Stage 2 Specify** — Approved 2026-06-26（design + spec + gates/specify 13/13）
- [~] **Stage 3 Implement** — 代码完成（task-01）；ADR-4 修订为镜像 SRMS（已接受）；验证（编译/UT/AC-3）**待 CI**；Stage 3 尚未关闭
- [ ] Stage 4 Release

## 基线关键约束（一句话）
仅当没有任何应用持有 URI 授权（`uriMap_`/`policyMap_` 全空）且空闲超时，UPMS 才自调 `UnloadSystemAbility` 卸载 `libupms`，配合 ondemand 配置实现下次访问自动重载；状态语义安全（账本为空时卸载），定性验收。
