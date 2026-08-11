# 代码检视报告 — insight_intent TDD 测试补充（Round 1 / 提交 1f89c15）

> 统一报告由 codecheck 工作台生成，**用于门禁管控**。章节顺序、字段名、报告元数据块、评分与门禁规则均为固定格式。权威评分与门禁规则见文末附录 A。

---

## 报告元数据

```yaml
codecheck_report:
  schema_version: "1.0"
  scope: "insight_intent TDD 测试补充"
  round: 1
  commit_id: "1f89c15367a0da708e7304449b13b561b5953ceb"
  change_id: "I26dc83ce2710e5b5e2a1286c11b50862e2c3d3ee"
  commit_subject: "意图TDD补充"
  date: "2026-08-10"
  dimensions_required: ["security-scanner", "logic-scanner"]
  dimensions_executed: ["security-scanner", "logic-scanner"]
  waived_dimensions: []
  findings_total: 7
  findings_by_severity: {P0: 0, P1: 0, P2: 1, P3: 6}
  score: 83
  risk_level: "medium"
  gate_decision: "conditional"
  gate_blockers: []
  must_fix: []
  followups: ["LOG-1", "LOG-2", "LOG-3", "LOG-4", "LOG-5", "LOG-6", "LOG-7"]
```

---

## 1. 基本信息

| 项目 | 值 |
|------|-----|
| 检视范围 | insight_intent 意图框架 TDD 测试补充（6 文件，+877 行，纯测试） |
| commit-id | `1f89c15367a0da708e7304449b13b561b5953ceb` |
| Change-Id | `I26dc83ce2710e5b5e2a1286c11b50862e2c3d3ee` |
| commit message | 意图TDD补充 |
| 检视日期 | 2026-08-10 |
| 检视轮次 | Round 1 |
| 检视维度 | security-scanner + logic-scanner |

### 提交内容核对

| 校验项 | 结果 |
|--------|------|
| 提交范围 | 6 文件（+877 行）：4 个测试 .cpp + 1 个 mock .cpp + 1 个 BUILD.gn，与检视内容一致 |
| 主要变更内容 | 为 InsightIntent 的 IPC 回调 Stub、系统事件接收器、RDB 数据/存储管理器补充 TDD 单测 |
| 提交完整性 | ✅ Change-Id / Signed-off-by 齐全；4 个测试目录的 BUILD.gn 均已将新 .cpp 注册进 sources（其中 3 个 BUILD.gn 为既有文件、1 个为本提交新增） |

---

## 2. 总体评价

### 2.1 上库质量评估结论

| 指标 | 结论 |
|------|------|
| **整体评分** | **83/100**（计算过程见附录 A，扣分明细见 2.3） |
| **风险等级** | 🟡 中风险 |
| **上库决策** | ⚠️ **有条件上库** |

**决策依据**：
- 依据 1：必检维度 security-scanner + logic-scanner 均已执行，无缺失 → 非 insufficient。
- 依据 2：无 P0、无 P1 → 不命中决策矩阵第 1/2 行。
- 依据 3：评分 83，≥70 且 <90 → 命中决策矩阵第 4 行 → conditional（中风险）。

**阻塞项（Gate Blocker）**：无。

**上库条件**：无强制 must_fix（无 P1）。建议合入前处理 LOG-1（P2 覆盖缺口），P3 项登记跟进。

### 2.2 各维度通过率

| 维度 | 通过率 | 等级 | 评价 |
|------|--------|------|------|
| Security & Bug（security-scanner） | 0/0 | 🟢 | 测试代码未引入崩溃/UAF/权限/IPC 鉴权类缺陷；IPC Stub 测试反而覆盖了生产侧 Stub 的鉴权与反序列化校验路径 |
| Logic（logic-scanner） | 6/7 | 🟡 | 1 项 P2 覆盖缺口（回调转发未断言）+ 6 项 P3 测试卫生（状态泄漏/魔法常量镜像/弱断言/共享单例耦合） |

### 2.3 评分扣分明细

| 严重等级 | 权重 | 数量 | 扣分 |
|---------|------|------|------|
| P0 致命 | −30 | 0 | 0 |
| P1 严重 | −12 | 0 | 0 |
| P2 一般 | −5 | 1 | −5 |
| P3 提示 | −2 | 6 | −12 |
| **合计** | | **7** | **−17** → 评分 **83** |

---

## 3. 问题统计

| 维度 | 总数 | P0 致命 | P1 严重 | P2 一般 | P3 提示 |
|------|------|---------|---------|---------|---------|
| logic-scanner | 7 | 0 | 0 | 1 | 6 |
| security-scanner | 0 | 0 | 0 | 0 | 0 |
| **总计** | **7** | **0** | **0** | **1** | **6** |

> security-scanner Core Scan 11 项均已给出结论：①空指针（测试侧 `rdbStore_.get()` 解引用依赖前序加载，合并入 LOG-5）/②越界/③UAF/④整数溢出/⑤死锁竞态/⑥资源泄漏/⑦反序列化/⑧敏感信息/⑨系统框架合规/⑩类安全/⑪IPC 鉴权——对测试代码均不适用或无独立发现（IPC Stub 测试反而正向验证了生产侧 Stub 的鉴权与 Parcel 校验）。

---

## 4. 高优先级发现（P0/P1，跨维度去重后）

**无。**

---

## 5. 分维度明细

### 5.1 Logic（经 logic-scanner）

| ID | 位置 | 类型 | 概述 | 等级 |
|----|------|------|------|------|
| LOG-1 | `test/unittest/insight_intent/insight_intent_execute_callback_stub_test/insight_intent_execute_callback_stub_test.cpp:45-46` | 覆盖缺口 | `OnExecuteDone` 回调 mock 为空实现，成功路径测试仅断言 stub 返回 ERR_OK，未验证回调实际收到正确的 (key, resultCode, executeResult) | P2 |
| LOG-2 | `test/unittest/insight_intent/insight_intent_sys_event_receiver_test/insight_intent_sys_event_receiver_test.cpp:435-436,454` | 状态泄漏 | AppUtils 单例 `isBopdOrRescueMode_.isLoaded=true` 未在用例结尾复位（仅复位 `.value`），单例状态残留至后续用例 | P3 |
| LOG-3 | `test/unittest/insight_intent/insight_intent_execute_callback_stub_test/insight_intent_execute_callback_stub_test.cpp:34` | 魔法常量镜像 | `FOUNDATION_UID=5523` 镜像 `services/abilitymgr/src/insight_intent/insight_intent_execute_callback_stub.cpp:26` 的真实常量，注释自认镜像；应共享常量而非复制 | P3 |
| LOG-4 | `test/unittest/insight_intent_rdb_data_manager_test/insight_intent_rdb_data_manager_test.cpp:15,20` | include 卫生 | `#define private public` 在标准头（gtest/vector）之前定义且重复定义两次，污染标准库私有成员 | P3 |
| LOG-5 | `test/unittest/insight_intent_rdb_data_manager_test/insight_intent_rdb_data_manager_test.cpp:46-64,116` | 测试隔离 | 共享 `DelayedSingleton` + 空 SetUp/TearDown，用例间状态耦合：0300 `IsIntentRdbLoaded(true)` 依赖 0100 先触发 DB 加载；0200 `*(rdbStore_.get())` 解引用依赖前序加载完成 | P3 |
| LOG-6 | `test/unittest/insight_intent_rdb_storage_mgr_test/insight_intent_rdb_storage_mgr_test.cpp:515-521` | 弱断言 | test 026 `BackupRdb()` 后仅 `SUCCEED()`，无可观测断言（仅 no-crash） | P3 |
| LOG-7 | `test/unittest/insight_intent/insight_intent_sys_event_receiver_test/insight_intent_sys_event_receiver_test.cpp:84-98` 及 0002/0008-0010 | 弱断言/主机受限 | `OnReceiveEvent_0001` 的 `EXPECT_NE(lastUserId_,0)` 被前序累积状态满足而非隔离验证当前事件；`SaveInsightIntentInfos` 对"有效参数"仍 `EXPECT_FALSE`（BundleMgrHelper IPC 主机不可用，作者已注释说明） | P3 |

### 5.2 Security & Bug（经 security-scanner）

无独立发现。详见第 3 节备注与 refute_log（测试侧 `rdbStore_` 空指针风险已合并入 LOG-5）。

---

## 6. 待跟进（P2/P3 + Suspicious）

| # | ID | 发现 | 等级 | 需要行动 | 负责人/排期 |
|---|----|------|------|---------|------------|
| 1 | LOG-1 | 回调转发值未断言（覆盖缺口） | P2 | 将 `OnExecuteDone` mock 改为捕获 (key, resultCode, executeResult) 并断言，验证 stub 真实转发 | 留空 |
| 2 | LOG-2 | AppUtils 单例 isLoaded 未复位 | P3 | 用例结尾复位 `isLoaded=false`，或在 TearDown 统一复位 | 留空 |
| 3 | LOG-3 | FOUNDATION_UID 魔法常量镜像 | P3 | 改为 #include 共享生产侧常量或在生产头导出常量 | 留空 |
| 4 | LOG-4 | `#define private public` 重复且前置 | P3 | 移除第 15 行冗余定义，仅保留第 20 行并于 23 行 #undef | 留空 |
| 5 | LOG-5 | 共享单例 + 空 SetUp/TearDown 耦合 | P3 | SetUp 中确保 DB 已加载或每用例独立实例；0200 加 `ASSERT_NE(rdbStore_, nullptr)` | 留空 |
| 6 | LOG-6 | test 026 仅 SUCCEED() | P3 | 补充对 BackupRdb 副作用的可观测断言或明确标注为 no-crash 探针 | 留空 |
| 7 | LOG-7 | 弱断言 + 主机环境受限 | P3 | 0001 改为每事件前重置 receiver 或断言具体 userId；主机受限项在 @tc.desc 显式标注 no-crash-only | 留空 |

---

## 7. 附录

### 7.1 变更文件清单

| 文件 | 状态 |
|------|------|
| `test/unittest/insight_intent/insight_intent_execute_callback_stub_test/insight_intent_execute_callback_stub_test.cpp` | ➕ 新增（183 行） |
| `test/unittest/insight_intent/insight_intent_sys_event_receiver_test/insight_intent_sys_event_receiver_test.cpp` | ➕ 新增（617 行） |
| `test/unittest/insight_intent_rdb_data_manager_test/insight_intent_rdb_data_manager_test.cpp` | ➕ 新增（372 行） |
| `test/unittest/insight_intent_rdb_storage_mgr_test/insight_intent_rdb_storage_mgr_test.cpp` | ➕ 新增（751 行） |
| `test/unittest/insight_intent_rdb_storage_mgr_test/mock_insight_intent_rdb_data_mgr.cpp` | ➕ 新增（152 行） |
| `test/unittest/insight_intent_rdb_storage_mgr_test/BUILD.gn` | ➕ 新增（76 行） |

### 7.2 检视轨迹

| 轮次 | 范围 | 评分 | 风险等级 | 上库决策 | 结论 |
|------|------|------|---------|---------|------|
| Round 1 | insight_intent TDD 测试补充 | 83 | medium | conditional | 无 P0/P1；1 项 P2 覆盖缺口建议合入前处理，6 项 P3 测试卫生登记跟进 |

### 7.3 各 skill 原始产出

| skill | 产出文件 | 发现数 |
|-------|---------|--------|
| logic-scanner | 并入本报告第 5.1 节（无独立 md/csv） | 7 |
| security-scanner | 并入本报告第 5.2 节（无独立 md/Excel） | 0 |
| refute_log | `refute_log_insight_intent_tdd_20260810.md` | 见 refute 日志 |

---

## 附录 A：评分与门禁规则（权威定义，勿改）

### A.1 严重等级统一归一化

| 统一等级 | 含义 | 各 skill 别名 | 门禁含义 |
|---------|------|--------------|---------|
| **P0 致命** | 崩溃/UAF/OOM/死锁/权限绕过/数据损坏/敏感数据泄漏等必现或易触发 | `critical` / `致命` / Confirmed P0 | **阻断上库** |
| **P1 严重** | 影响正确性/安全边界，低概率触发或需组合条件 | `high` / `严重` / Confirmed P1、Likely P0 | 需修复或人工裁决 |
| **P2 一般** | 逻辑缺陷/资源小泄漏/健壮性问题 | `medium` / `一般` / Likely P1、P2 | 建议修复，登记跟进 |
| **P3 提示** | 风格/潜在风险/观察项，当前不可达 | `low` / `提示` / Suspicious | 不阻塞，登记跟进 |

归一化映射以**问题实际影响**为准，不以 skill 内部措辞为准；同一问题被多 skill 标注不同等级时，取最高等级。

### A.2 评分公式与决策矩阵（确定性，可复现）

```
评分 = max(0, 100 − (30×P0 + 12×P1 + 5×P2 + 2×P3))
```

决策矩阵（自上而下判定，命中即止）：

| 序 | 条件 | risk_level | gate_decision |
|----|------|-----------|---------------|
| 0 | 必检维度（dimensions_required）未执行且未豁免 | `unknown` | `insufficient` |
| 1 | 存在 ≥1 项 P0 | `high` | `block` |
| 2 | 存在 ≥1 项 P1（无 P0） | `medium` | `conditional` |
| 3 | 评分 ≥ 90 | `low` | `approve` |
| 4 | 评分 ≥ 70 | `medium` | `conditional` |
| 5 | 评分 < 70 | `high` | `block` |

**决策语义**：
- `approve`：可以上库，P2/P3 项登记进 followups 跟踪。
- `conditional`：可上库但附条件——必须处理所有 `must_fix`（P1 项）或在门禁复核人书面裁决后放行；P2/P3 登记跟进。
- `block`：禁止上库，必须修复 `gate_blockers`（P0 项）后进入下一轮重检。
- `insufficient`：无法评估——必检维度缺失，补齐扫描后重出报告；`waived_dimensions` 中记录的用户豁免项除外。

### A.3 必检维度

| 目标特征 | 必检维度（缺失即 insufficient） |
|---------|-------------------------------|
| 通用路径/提交 | `security-scanner` + `logic-scanner` |
| `services/` 下、IPC/DB/文件/配置密集区 | `security-scanner` + `logic-scanner` + `input-scanner` |
| `interfaces/kits/`、NAPI/ANI/C 绑定、指定 Kit | `api-scanner` + 实现侧 `security-scanner` + `logic-scanner` |

### A.4 格式一致性要求（门禁校验项）

1. 章节编号与顺序固定（1 基本信息 → 2 总体评价 → 3 问题统计 → 4 高优先级发现 → 5 分维度明细 → 6 待跟进 → 7 附录 → 附录 A）。
2. YAML 报告元数据块字段名、取值域必须合法；`score`/`risk_level`/`gate_decision` 三者与 2.1 表格一致。
3. 每条发现可追溯到 `file:line` + 触发路径；无证据项不得计入 P0/P1。
4. 严重等级只允许 P0–P3，跨 skill 统一归一化后汇总。
5. 多轮重检保留 7.2 检视轨迹，展示分数与决策演进，作为门禁复核依据。
