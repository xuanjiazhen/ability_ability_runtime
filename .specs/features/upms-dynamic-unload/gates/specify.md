# Gate — Stage 2 Specify（规格化阶段）

> item: `upms-dynamic-unload` | 阶段切换：Stage 2 → Stage 3
> 规则依据：SKILL.md「Stage 2 退出条件」。本文件记录规格化门禁逐项检查结果。
> 取证范围：本轮已读 SRMS 先例、UPMS SA 类、stub_impl 账本结构与清账路径、BUILD.gn、sa_profile。

> **[修订 2026-06-26]** ADR-2 由 EventHandler+EventRunner 改为 **ffrt**（`ffrt::submit_h`+`.delay().name()` 提交、`ffrt::skip(handle)` 取消、`external_deps` +`ffrt:libffrt`）。理由：EventHandler 会为 UPMS 创建独立常驻线程，不合适；ffrt 走共享线程池，且为本仓主流异步方式。取证：本仓 `ffrt_task_handler_wrap.cpp`、`mission_list_manager.cpp:4230`（`submit_h`+handle）、`report_data_partition_usage_manager.cpp:45`（`.delay(US)`）。S3（机制取证）据此更新；其余 S1–S13 仍成立。

## 一、规格化阶段门禁

| # | 检查项 | 结果 | 证据 / 理由 |
|---|--------|------|------------|
| S1 | design.md 关键决策（ADR）已记录且含取舍 | ✅ 通过 | design.md §3 ADR-1~6，每条含决策+理由+取舍 |
| S2 | 与先例/现状的差异已澄清（不能照搬） | ✅ 通过 | design.md §2 对照表：UPMS≠SRMS（SA/impl 分离、状态不可重建、触发语义不同）→ 驱动 ADR-1 |
| S3 | 触发/卸载机制有源码取证 | ✅ 通过 | 已读 SRMS `DelayUnloadTask`(cpp:86-111, `UNLOAD_DELAY_TIME=90000`)；UPMS SA 类(cpp:38-90)；stub_impl 四容器+锁(h:290-300)；清账路径(`ClearPermissionTokenByMap`/`RevokeAll*`/`RevokeFileUriPermission`) |
| S4 | spec.md AC 全部 WHEN/THEN 且可测 | ✅ 通过 | spec.md §3，3×P0 + 3×P1，每条含验证手段 |
| S5 | AC 与 proposal/design 可追溯 | ✅ 通过 | spec.md §7 可追溯性矩阵（AC ↔ proposal §3.5 ↔ design ADR ↔ 验证手段） |
| S6 | Stage 1→2 不涉及项 carry-through 完成 | ✅ 通过 | design.md §6 表，8 项全部确认（含 sa_profile 由"待定"→"不改"） |
| S7 | 兼容性已分析（行为/重载/配置/线程） | ✅ 通过 | spec.md §6 |
| S8 | API 影响已界定 | ✅ 通过 | spec.md §5，对外 API 无变更，仅内部私有方法/成员 |
| S9 | Profile 判定一致 | ✅ 通过 | manifest.profile = none，无子系统 profile 命中 |
| S10 | 关键参数定义（超时/任务名/SA ID） | ✅ 通过 | spec.md §8 |
| S11 | 安全风险（误卸载→泄漏）已识别并有缓解 | ✅ 通过 | design.md §7；ADR-1/3：账本全空才卸 + cancel 穷尽所有 grant 路径 + AC-4 覆盖 |
| S12 | 关键不确定项有验证门槛/Fallback | ✅ 通过 | ADR-6：重载为 Stage 3/4 硬门槛，fallback 加 ondemand 条件 |
| S13 | 复杂度裁剪一致（standard：design 关键决策 + spec 全量） | ✅ 通过 | design.md（ADR 级）+ spec.md（全量 AC/兼容/追溯），未过度展开 |

**结论：Stage 2 门禁全部通过（13/13）。**

## 二、带入 Stage 3 的移交事项

1. task.md 按 design.md §5 代码点位拆任务（stub_impl.h/.cpp 新增方法+成员、13 处尾部插桩、service.cpp `SelfClean` 增强）。
2. OQ-1 `target_release` 在 Stage 3 期间向 owner 确认。
3. **硬门槛**：AC-3 重载必须在 Stage 3/4 实测通过；失败则评估 ADR-6 fallback（可能轻微扩范围，需重新评估 Stage 2）。
4. 代码须遵循 OpenHarmony C++ 规范（hilog 用 `hilog_tag_wrapper.h` 的 `AAFwkLogTag::URIPERMMGR`，与既有代码一致）。

## 三、审批状态

- Stage 2 产物（design.md / spec.md / gates/specify.md）已完成，门禁 13/13 通过。
- **等待需求方批准进入 Stage 3**。未批准前不创建 execution-plan.md / task.md，不写任何代码（HARD-GATE）。
