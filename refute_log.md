# Refute 审查记录 — Round 2（复核重分级）

> 范围：commit `aa9e535042`｜依据用户反馈"很多没问题"，对 P1/P2 项做二次证据复核
> 原则：只基于代码证据调整，不以评分倒退为目的；仍保留确认不可推翻项。

## Round 2 调整

| 原级 | 调整后 | 发现 | 证据依据 |
|---|---|---|---|
| P1-1 | **→ P2** | GetProcessKillReason 失败语义/空 REASON | 空 REASON 仅在 `open(/dev/sysload)<0`（`:1437-1439`）出现（设备缺失/裁剪 ROM）；ioctl 失败分支 `:1461` 已回填 `INVALID_KILL_REASON`（非空）。重试条件属 DFX 上报健壮性缺陷，未触及正确性/安全边界 → 不符 P1 定义 |
| P1-2 | **保持 P1** | fourth_test 编译断裂 | `app_mgr_service_inner.h` 声明已删（diff 确认），测试仅靠 `-Dprivate=public`（BUILD.gn:51）暴露 private，但成员已不存在 → `no member named 'SetKilledEventInfo'`，硬编译错误，证据不可推翻 |
| P2-2 | **保持 P2** | KillInfo 内核 ABI 跨仓契约 | 新特征依赖内核新增 SET cmd（magic 0xE5AC03）；版本漂移会静默失效整个归因 → Suspicious 保留 P2 |
| P2-1 | **→ P3** | ProcessKillInfo 成员未初始化 | 生产唯一调用方 `app_mgr_service_inner.cpp:6717` 用 `= {}` 已初始化；仅 fuzzer/未来调用方理论 UB（防御性） |
| P2-3 | **→ P3** | SET_KILL_INFO 保护不足 | 产品路径 `pid` 恒 >0；killId==0 可达性存疑（默认 -1）；设备 ACL 属内核/系统职责 → 设计确认项 |
| P2-4 | **→ P3** | callingProcessName 信任优先级 | `:1400` 注释仅声明 foreground 以内核为准；进程名用实时解析属意图性设计（pid 复用窗口极小）→ 观察项 |
| P2-5 | **→ P3** | FOREGROUND 权威来源变更 | 带明确注释的意图性行为变更，需产品确认而非缺陷 → 观察项 |

## 复核后分级分布

P0=0，P1=1（仅编译断裂），P2=2（重试语义/DFX 质量、内核 ABI），P3=14。

## Round 1 概要（保留记录）

| 指标 | 数值 |
|---|---|
| 审查范围 | 17 条（SEC 9 / LOG 10 / INP 11，去重合并后） |
| 维持 | 9 组 |
| 降级 | 2 条 |
| 合并 | 5 组 |

*Round 1 详细逐条审查见上版 refute_log；本文件为 Round 2 复核增量。*