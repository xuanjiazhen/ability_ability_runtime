# Proposal — UPMS 动态卸载

> id: `upms-dynamic-unload` | type: feature | complexity: standard | profile: none
> 状态：**Stage 1 基线已通过（需求方确认 2026-06-25）**
> 本文件三合一：原始需求 → 澄清记录 → 需求基线（一份文档不断生长）。

---

## 一、原始需求 (Raw Request)

为 UPMS（Uri Permission Manager Service，SA 183，库 `libupms.z.so`，运行于 `foundation` 进程）增加**空闲期动态卸载**能力，把该 SA 在无授权业务期间的内存还给 `foundation` 进程，以满足精简产品 / 进程级 RSS 预算这一**硬性内存目标**。

仓库内已有两份分析文档作为输入：
- [unload_upms.md](../../../../unload_upms.md)：评估"能否在空闲时把 UPMS 卸载掉"，**结论不建议**，核心障碍是临时授权的状态语义 + 误卸载风险，另给出最小实现路径。
- [upms_mem_op.md](../../../../upms_mem_op.md)：建议优先做依赖裁剪。本需求与之并行，聚焦卸载机制本身。

需求方在明确"不建议"的前提下，仍选择推进卸载——因为内存目标是硬性的，且接受为满足安全约束而付出复杂度代价。

---

## 二、澄清记录 (Clarification Record)

逐轮问答（每次一问，多选优先）。每轮记录：问题 / 选项 / 决策 / 理由。

### 轮 1：需求范围（哪一项变更）
- **问**：基于两份 UPMS 文档，要正式定义并实现哪一项？
- **选项**：依赖裁剪 / 运行时数据层优化 / **动态卸载** / 全部打包
- **决策**：**动态卸载**。

### 轮 2：驱动力（为什么做）
- **问**：unload_upms.md 明确"不建议"，仍要做的驱动力是什么？
- **选项**：**硬性内存目标** / 产品架构指令 / 同时解决状态问题 / 先上机制后补状态
- **决策**：**硬性内存目标**（有进程级 RSS 预算 / 精简产品约束，依赖裁剪不够，必须拿卸载这块内存；复杂度与状态风险可接受）。

### 轮 3：触发策略（什么时候卸载）—— 命门
- **问**：UPMS 何时被卸载？（决定状态安全性与内存目标可达性）
- **选项**：**无活跃授权才卸** / 持久化+调用空闲 / 调用空闲+接受风险 / 按授权类型区分
- **决策**：**无活跃授权才卸** —— 仅当 `uriMap_`/`policyMap_` 全空（无任何应用持有授权）且空闲超时才卸载。
- **理由**：账本为空时卸载，状态语义安全，无需持久化；规避了 unload_upms.md 指出的误卸载/撤销断裂风险。
- **代价（已知晓并接受）**：若系统常有持久授权，UPMS 几乎不卸载，内存回收是**尽力而为**，不保证额度。

### 轮 4：验收口径（怎么算达标）
- **问**：选了安全的"无活跃授权才卸"后，内存目标怎么验收？
- **选项**：**定性验收（能力成立）** / 定量验收（有数值） / 先出能力再定数值
- **决策**：**定性验收** —— "无活跃授权窗口期能成功卸载 `libupms`"这个能力成立即可，用 `hidumper`/`aa`/samgr 查 SA 183 unloaded 状态验证；不绑定具体 MB。

### 理解确认（基线冻结前）
已向需求方输出完整理解草稿（核心/范围含/排除项/涉及仓/复杂度/P0-P1 AC），需求方于 2026-06-25 回复"确认"，基线冻结。

---

## 三、需求基线 (Baseline)

### 3.1 核心目标
给 UPMS 加"安全延时自卸载"：当没有任何应用持有 URI 授权（授权账本全空）且空闲超时，自调 `UnloadSystemAbility(183)` 卸载 `libupms`；配合 ondemand 配置，使下次 `GrantUriPermission` 时 SAMGR 自动重载。**在状态语义安全的前提下，把空闲期内存还给 `foundation` 进程。**

### 3.2 范围 (In Scope)
1. 新增延时卸载任务（仿 SRMS `DelayUnloadTask` 模式，[service_router_mgr_service.cpp:86-111](../../../../service_router_framework/services/srms/src/service_router_mgr_service.cpp)）。
2. 卸载触发点：在授权账本清空路径（`ClearPermissionTokenByMap` / `RevokeAllUriPermissions` 等，确认 `uriMap_`/`policyMap_` 为空后）启动延时计时；任意新授权 / 访问到来时取消计时并重挂。
3. `services/sa_profile/183.json` 增 ondemand 配置，让 SAMGR 在下次访问时自动重载（重载从空状态重建，无需持久化）。
4. 校核/补强 `OnStop` → `SelfClean`（[uri_permission_manager_service.cpp:82-90](../../../../services/uripermmgr/src/uri_permission_manager_service.cpp) 现状只置 flag），确认卸载时析构/清理到位、计时器停止、订阅注销。

### 3.3 排除项 (Out of Scope)
- ❌ 不做授权账本持久化（"无活跃授权才卸"时账本已空，无落盘必要）。
- ❌ 不改下游服务（storageManager 共享文件撤销、SandboxManager 策略）——卸载发生在账本清空后，无待撤销项，无残留泄漏。
- ❌ 不绑定具体 MB 目标（定性验收）。
- ❌ 不改授权 / 撤销主链路逻辑（仅在其尾部加"空 → 计时 → 卸载"旁路）。
- ❌ 不做依赖裁剪（属 [upms_mem_op.md](../../../../upms_mem_op.md) 范畴，独立推进）。

### 3.4 涉及仓 / 子系统
**单仓** `ability_runtime`：
- `services/uripermmgr/`（触发/计时逻辑、`OnStop` 清理）
- `services/sa_profile/183.json`（ondemand 配置）

### 3.5 验收标准 (AC，WHEN/THEN，可测)
**P0**
- **AC-1 卸载能力**：WHEN `uriMap_`/`policyMap_` 全空且空闲超时 THEN UPMS 自调 `UnloadSystemAbility`，SA 183 状态变为 unloaded。
  - 验证：`hidumper -s 183` / `aa dump` / samgr 查询确认 SA 183 不在 loaded 列表。
- **AC-2 触发与取消**：WHEN 最后一份授权被撤销（账本变空）THEN 启动延时卸载计时；WHEN 计时未到且新授权/访问到来 THEN 取消计时并重挂。
  - 验证：单测覆盖 "空→计时" 与 "非空→不计时 / 新授权→取消"。
- **AC-3 ondemand 重载**：WHEN SA 183 已 unloaded 且应用再请求 `GrantUriPermission` THEN SAMGR 按 ondemand 配置重新加载 UPMS 并正常授权（状态从空重建）。
  - 验证：卸载后触发一次跨应用 URI 授权，确认 SA 183 回到 loaded 且授权成功。

**P1**
- **AC-4 不误卸载**：WHEN 有应用仍持有授权（账本非空）THEN 即使长时间无调用也不卸载。
  - 验证：授予权限后等待超过卸载超时，SA 183 仍 loaded。
- **AC-5 回归**：WHEN 卸载机制启用 THEN uripermmgr 全套 UT 通过，授权/撤销主链路不破坏。
  - 验证：`run -t UT -tp ability_runtime`（或 uripermmgr 相关套）通过。

### 3.6 约束与风险
- **可达性风险（已知并接受）**：安全触发可能导致 UPMS 长期不卸载，内存回收是尽力而为；定性验收覆盖此风险。
- **dlclose 实际收益有限**：依赖库被 `foundation` 内其他 SA 共享、`dlclose` 不保证释放、allocator 持有 vmem（见 [unload_upms.md](../../../../unload_upms.md) §5）——本需求交付的是"能力"，RSS 实测降幅属验证观测项，非硬性门槛。
- **ondemand 重载正确性**：重载后需从空状态正确初始化，确保首条授权路径无误。
- **计时器生命周期**：`OnStop` 必须停止计时器，避免卸载后回调悬空。

### 3.7 不涉及项确认表
| 项 | 是否涉及 | 说明 |
|----|---------|------|
| 授权账本持久化 | N/A | 账本空时卸载，无需落盘 |
| 下游撤销服务改造 | N/A | 卸载发生在撤销完成后 |
| 具体 MB 量化目标 | N/A | 定性验收 |
| 依赖裁剪 (BUILD.gn) | N/A | 属 upms_mem_op.md 范畴 |
| 授权/撤销主链路改写 | N/A | 仅尾部加旁路 |
| 新增对外 API | N/A | 无 |
| 跨仓 / SIG 协调 | N/A | 单仓 |

### 3.8 待解决问题 (Open Questions，进入 Stage 2 前澄清)
| # | 问题 | owner | 处理时机 |
|---|------|-------|---------|
| OQ-1 | `target_release` 归属哪个版本？ | owner | Stage 2 起，可暂置 |
| OQ-2 | 空闲超时阈值取多少（SRMS 用常量 `UNLOAD_DELAY_TIME`，UPMS 需定值）？ | design | Stage 2 design.md |
| OQ-3 | 哪些调用应"取消计时/重挂"——仅授权写入，还是含 Verify/Check 读路径？ | design | Stage 2 design.md |
| OQ-4 | ondemand 重载触发事件定义为哪个 transId/能力名（`GrantUriPermission` 对应）？ | design | Stage 2 design.md |
| OQ-5 | `SelfClean` 是否需要从"只置 flag"升级为真实析构清理？ | design | Stage 2 design.md（需读 OnStop 现状） |
