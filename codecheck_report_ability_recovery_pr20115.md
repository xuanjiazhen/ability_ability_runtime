# 代码检视报告 — ability_recovery ScheduleRestoreAbilityState 打点（PR !20115）

---

## 报告元数据

```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "ability_recovery (ScheduleRestoreAbilityState)"
  round: 1
  commit_id: "206a08767b870c397a6f8675f76778ed6d059885"
  change_id: "N/A (GitCode PR !20115)"
  commit_subject: "refactor: 改用 AAFwk::HisyseventReport 封装替代手写 HiSysEventParam"
  date: "2026-08-10"
  dimensions_required: ["security-scanner", "logic-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner"]
  waived_dimensions: []
  findings_total: 1
  findings_by_severity: {P0: 0, P1: 0, P2: 0, P3: 1}
  score: 98
  risk_level: "low"
  gate_decision: "approve"
  gate_blockers: []
  must_fix: []
  followups: ["F-01"]
```

---

## 1. 基本信息

| 项目 | 值 |
|------|-----|
| 检视范围 | `frameworks/native/ability/native/recovery/ability_recovery.cpp` 新增打点 + `BUILD.gn` 依赖 |
| commit-id | `206a08767b` |
| Change-Id | N/A（GitCode PR !20115） |
| commit message | refactor: 改用 AAFwk::HisyseventReport 封装替代手写 HiSysEventParam |
| 检视日期 | 2026-08-10 |
| 检视轮次 | Round 1 |
| 检视维度 | security-scanner + logic-scanner |

### 提交内容核对

| 校验项 | 结果 |
|--------|------|
| 提交范围 | 2 文件（+47 行）：`ability_recovery.cpp`(+46) + `BUILD.gn`(+1)，与检视内容一致 |
| 主要变更内容 | 新增 `ReportRestoreAbilityStateResult` 辅助函数，复用 `AAFwk::HisyseventReport` 向 `PREVENT_START_ABILITY`(preserve:true) 事件填 8 参数；在 `ScheduleRestoreAbilityState` 恢复成功出口加一次打点 |
| 提交完整性 | ✅ Signed-off-by 存在；DCO 通过；无遗漏文件 |

---

## 2. 总体评价

### 2.1 上库质量评估结论

| 指标 | 结论 |
|------|------|
| **整体评分** | **98/100**（扣分明细见 2.3） |
| **风险等级** | 🟢 低风险 |
| **上库决策** | ✅ **可以上库** |

**决策依据**：
- 无 P0/P1/P2 项，仅 1 项 P3 观察项（字段借用语义偏离，属设计取舍非缺陷）。
- 必检维度 security-scanner + logic-scanner 均已执行。
- 评分 = 100 − 2×1 = 98 ≥ 90 → approve。

**阻塞项（Gate Blocker）**：
- 无

**上库条件**：
- 无

### 2.2 各维度通过率

| 维度 | 通过率 | 等级 | 评价 |
|------|--------|------|------|
| Security & Bug | 11/11 | 🟢 | 空指针/越界/UAF/竞态/泄漏/IPC 鉴权全项通过；Report() 返回值已检查；HisyseventReport 内部 nothrow+length 守门安全 |
| Logic | 7/7 | 🟢 | 控制流/数据流/状态机/边界/错误处理/并发/业务规则全项通过；打点失败不影响恢复结果（正确解耦） |

### 2.3 评分扣分明细

| 严重等级 | 权重 | 数量 | 扣分 |
|---------|------|------|------|
| P0 致命 | −30 | 0 | 0 |
| P1 严重 | −12 | 0 | 0 |
| P2 一般 | −5 | 0 | 0 |
| P3 提示 | −2 | 1 | −2 |
| **合计** | | **1** | **−2** → 评分 **98** |

---

## 3. 问题统计

| 维度 | 总数 | P0 致命 | P1 严重 | P2 一般 | P3 提示 |
|------|------|---------|---------|---------|---------|
| Security & Bug | 0 | 0 | 0 | 0 | 0 |
| Logic | 1 | 0 | 0 | 0 | 1 |
| **总计** | **1** | **0** | **0** | **0** | **1** |

---

## 4. 高优先级发现（P0/P1，跨维度去重后）

**无。**

---

## 5. 分维度明细

### 5.1 Security & Bug（经 security-scanner）

Core Scan 11 项逐条结论：

| # | 检查项 | 结论 | 依据 |
|---|--------|------|------|
| ① | 空指针/无效迭代器 | ✅ 不适用 | `abilityInfo` 经 `weak_ptr::lock()` 获取后传入 helper，helper 内部 `if (abilityInfo == nullptr) return` 兜底（ability_recovery.cpp:67-70）；`HisyseventReport` 构造 `new(std::nothrow)` 失败时 `params_=nullptr, length_=0`，InsertParam 守门 `if (length_ <= pos_) return`，Report 守门 `if (params_==nullptr) return -1`（hisysevent_report.cpp:365-368） |
| ② | 越界访问 | ✅ 不适用 | `HisyseventReport(8)` 恰好 8 个 InsertParam 调用，`pos_` 0→7，`length_=8`，最后一次 `8<=7` 为 false 不触发早退 |
| ③ | UAF/生命周期 | ✅ 不适用 | `abilityInfo` 为 `shared_ptr` 局部变量，函数同步执行期间生命周期有效；`report` 为 `make_shared` 自动释放 |
| ④ | 整数溢出/截断 | ✅ 不适用 | `static_cast<int32_t>(uid)` — OHOS UID 取值域 < 2^31，无截断；versionCode 同理 |
| ⑤ | 死锁/竞态 | ✅ 不适用 | 函数在 app 主线程生命周期回调中同步执行，未持锁；`OH_HiSysEvent_Write` 为本进程→hiview 单向 IPC，无锁序问题 |
| ⑥ | 资源泄漏 | ✅ 不适用 | `report` 为 `shared_ptr` RAII；HisyseventReport 析构 `delete[] params_`；string 参数经 `paramBuffers_`（`unique_ptr` vector）自动释放 |
| ⑦ | 反序列化 | ✅ 不适用 | 变更代码不涉及反序列化（mmap 读取在未修改的 `ReadSerializeDataFromFile` 中） |
| ⑧ | 敏感信息泄漏 | ✅ 不适用 | 上报字段为 uid/versionCode/versionName/bundleName/abilityName — 均为应用元数据，非红线关键字（udud/networkid/SN/challenge/token/password/cmdLine）；事件 `PREVENT_START_ABILITY` 字段隐私分级由 hisysevent.yaml 定义，复用不改变分级；TAG_LOGW 仅打印 `ret`(int) + `result`("RESTORE_SUCCESS" 固定常量) |
| ⑨ | 系统框架合规 | ✅ 不适用 | 代码运行在 app 进程，使用 app 自身身份上报，无越权 |
| ⑩ | 类安全/虚析构 | ✅ 不适用 | HisyseventReport 通过 `shared_ptr` 正确使用，无继承/移动语义问题 |
| ⑪ | IPC 鉴权 | ✅ 不适用 | 变更代码无 `OnRemoteRequest`/Stub；`OH_HiSysEvent_Write` 为 outgoing IPC（app→hiview），非 incoming stub |

**G 类同类横扫**：无命中。G14（忽略返回值）不适用 — `Report()` 返回值已在 ability_recovery.cpp:82 检查。

### 5.2 Logic（经 logic-scanner）

三层分析模型逐项结论：

| 维度 | 结论 | 依据 |
|------|------|------|
| 控制流（LOG-001~003） | ✅ 通过 | 无死代码；3 个失败出口（444/449/454 行）仅 TAG_LOGE 不打点属设计意图（PR 约束"仅打恢复成功点"）；成功出口（464 行）打点后 return true |
| 数据流（LOG-004~005） | ✅ 通过 | `abilityInfo` 由 `weak_ptr::lock()` 获取，无未初始化使用；字段借用映射在注释（48-56 行）中文档化 |
| 状态机（LOG-006~008） | ✅ 通过 | 函数返回值（true/false）未改变，恢复状态机语义不变 |
| 边界条件（LOG-012） | ✅ 通过 | 8 参数恰好填满；abilityInfo null 已兜底 |
| 错误处理（LOG-009, ERR-001~006） | ✅ 通过 | `Report()` 返回值已检查（ERR-001 通过）；打点失败（ret≠0）仅 TAG_LOGW 不影响恢复 return true（正确解耦：观测性打点不阻断业务路径） |
| 并发控制（CONC-001~004） | ✅ 通过 | 单线程主线程回调，无共享变量竞争 |
| 业务规则（LOG-010~011） | ⚠️ 观察项 | 字段借用语义偏离 → 见 F-01 |

---

## 6. 待跟进（P2/P3 + Suspicious）

| # | ID | 发现 | 等级 | 需要行动 | 负责人/排期 |
|---|----|------|------|---------|------------|
| 1 | F-01 | PREVENT_START_ABILITY 字段借用语义偏离：CALLER_PROCESS_NAME 实存 versionName、CALLEE_BUNDLE_NAME 实存固定标记 "APP_RECOVERY"、EXTENSION_ABILITY_TYPE 实存 versionCode | P3 | 接受设计取舍（PR body 已说明选 B 方案复用 preserve:true 事件不上云）；后续若审批通过新增 `APP_RECOVERY_RESTORE`(preserve:true) 事件，可迁移到语义干净的字段 | 留空 |

---

## 7. 附录

### 7.1 变更文件清单

| 文件 | 状态 |
|------|------|
| `frameworks/native/ability/native/recovery/ability_recovery.cpp` | ✏️ 修改（+46 行：includes +2、匿名命名空间常量+helper +37、ScheduleRestoreAbilityState 成功出口 +1、abilityInfo lock +1、调用 +1、注释 +3） |
| `frameworks/native/ability/native/BUILD.gn` | ✏️ 修改（+1 行：`uiabilitykit_native` 新增 `hisysevent_report` 显式依赖） |

### 7.2 检视轨迹

| 轮次 | 范围 | 评分 | 风险等级 | 上库决策 | 结论 |
|------|------|------|---------|---------|------|
| Round 1 | ability_recovery ScheduleRestoreAbilityState 打点 | 98 | 🟢 低 | ✅ approve | 无 P0/P1/P2，1 项 P3 观察项 |

### 7.3 各 skill 原始产出

| skill | 产出文件 | 发现数 |
|-------|---------|--------|
| security-scanner | 嵌入本报告 §5.1 | 0 |
| logic-scanner | 嵌入本报告 §5.2 | 1（P3） |

### 7.4 Refute 审查记录

本次无 P0/P1 项需 refute。P3 项不进入 refute（直接进 §6 待跟进）。

| 指标 | 数值 |
|------|------|
| 审查范围 | security: 0 条, logic: 0 条 P0/P1 |
| P0/P1 全审 | 0 条 |
| 维持 | 0 |
| 降级 | 0 |
| 推翻 | 0 |
| 合并 | 0 |
| **进入最终报告** | **1 条** (P0: 0, P1: 0, P2: 0, P3: 1) |

---

## 附录 A：评分与门禁规则（权威定义）

- 评分 = max(0, 100 − (30×0 + 12×0 + 5×0 + 2×1)) = **98**
- 决策矩阵：无 P0 → 非阻塞；无 P1 → 非条件；评分 98 ≥ 90 → **approve**
