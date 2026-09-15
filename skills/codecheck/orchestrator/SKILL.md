# Skill: codecheck-orchestrator

# Codecheck Orchestrator — 双模审查编排器

## 定位

本 skill 是 [`skills/codecheck/`](..) 工作台的**唯一调度层**。两种审查模式并行编排、一份统一报告：

| 模式 | 名称 | 目标 | 产出定级权 |
|------|------|------|-----------|
| **Mode A** | 真实漏洞审查（假设-验证管线） | 端到端可达的攻击链/故障链：崩溃、挂死、提权、代码加载绕过、资源耗尽 | **P0/P1 只能诞生于此**，且必须附验证状态 |
| **Mode B** | 静态编码审查（scalable checklist） | 基础编码卫生：敏感信息打印、解析无守卫、入参无校验、资源不回滚等 | 封顶 P2；命中攻击面数据流时**升格**为 Mode A 假设 |

设计原则（两轮试点实证，见 [`verification-playbook.md`](../verification-playbook.md)）：
- **scanner 产出是假设，不是结论**。"注册了≠会被调"，分级依据是验证状态，不是代码内缺陷存在性。
- **Pattern not rules**：知识以"共性原因+推理方法"沉淀（playbook/checklist 条目均须挂真实 finding 来源），不做规则堆积。
- **验证闭环**：跨仓拉源 → 完整故障链推理 → harness/UT 复现 → 才给最终审查结论。

---

## Step 0: 模式路由

| 用户意图 | 路由 |
|---|---|
| "检视一下 / code review / 深度扫描"（未指定） | **A+B 并行**（B 廉价全量，A 定向高价值面） |
| "找真实漏洞 / 高危 / 攻击链 / 安全审计" | 仅 Mode A |
| "编码规范 / 卫生检查 / checklist review" | 仅 Mode B |
| 指定单一 scanner 维度 | 对应维度作为 Mode A 的 S2 假设生成透镜 |

范围界定三项不变：目标路径/Kit、重点、版本信息（`git rev-parse HEAD`、Change-Id、subject）。

---

## Mode A — 假设-验证管线（S0-S7）

### S0 影响定标
一句话方法论（不建静态事故库）：**websearch 对标系统（安卓 AOSP/CVE/Security Bulletin）中与目标模块同位的真实事故根因类**，倒排本模块最坏结果（不开机/bootloop/黑屏/跨应用提权/解析面 RCE/资源耗尽）。

### S1 面推导
面清单 = **机械穷举部分** + **推理推导部分**，产出"面清单"而非"文件清单"。

**机械基底——注册点穷举 + 三面问**（命令配方与暴露级分层见 [`../verification-playbook.md`](../verification-playbook.md) §1.2；**全仓头文件**穷举，本仓实测 ≈104 个 hook，有限集合）：
1. 穷举全仓 `*.h` 的回调注册点（hook）：`Set*/Register*/Install*/Add*/Subscribe*` × `Tracker/Callback/Func/Handler/Delegate/Hook/Listener/Observer`，排除 cj_*/ets_*/frameworks/ets|cj/test/mock（静态语言绑定按 AGENTS 约定默认排除；实测不排除会混入 40+ 噪音）。基线 ≈131 个，有限集合。
2. **暴露级按构建元数据判定，不按目录猜**：GN `public_configs` 头文件公开性 + `version_script`(.map) 符号导出 + BUILD.gn 依赖反查（仓内/跨仓）三步判据（命令见 playbook §1.2）；目录启发只用于排序。
3. 每个入选 hook 回答**三面问**：谁注册（本仓 file:line）／谁消费（跨仓面必查对端仓，走 S4 配方，拉不到标 `pending(仓名)`；仓内面本仓全量 grep）／什么模式开关决定它被调（唯一生产设置点）。
4. 任何一问答不出的 hook **显式记为 pending 面**列入报告"盲区欠账"——盲区从"不知道不知道"变成清单上可见的欠账，禁止静默跳过。

**推理推导**：从 S0 最坏结果倒排入口面：模式开关、IPC 入口（逐 code）、文件/包解析汇点、动态加载通道；目标模块命中 [`../static-checklist.md`](../static-checklist.md) **附录一历史复发热点清单**时强制纳入并提高强度。

**覆盖限度（如实声明）**：穷举覆盖结构性面；**时序面**（销毁×在途竞态）靠精读交叉分析（模板：FIND-002 证伪链），**业务语义面**靠热点清单+用户点名——后两类不受穷举保障，报告中不得以"面清单已穷举"暗示全覆盖。

### S2 假设生成（scanner 的新职责）
对每个面**精读 + 攻击者视角输入面枚举**，产出假设卡：
```
HYP-xxx: source(攻击者可控输入) → 传播链(file:line) → sink(危险汇点) → 预期影响
         部署面猜想: 谁启用/谁调用/什么产品形态
         建议验证方式: 分诊可证伪? 跨仓? harness?
```
一轮每模块 5-10 条高质量假设，**不追求数量**。三个 scanner（security/logic/input）作为假设生成的三个透镜并行，但产出统一为假设卡格式，**不自行定级 P0/P1**。

### S3 部署面分诊（廉价证伪，先于昂贵验证）
每条假设过三元组，缺一即降级或标记 conditional：
1. **部署面**：入口是公开 API / systemapi / 仅系统进程？模式开关的生产设置点在哪（必须找到唯一调用点级证据）？
2. **攻击者能力**：量产默认态下谁能把恶意数据送到入口？自攻自身（无权限增益）→ 证伪。
3. **可达性**：真实调用链 file:line；跨仓不可证 → 标 `Reasoned-pending`，不定 P1。

### S4 跨仓追踪
按 [`verification-playbook.md`](../verification-playbook.md) §1 执行：依赖解析（BUILD.gn/bundle.json）→ 本地兄弟目录 → 持久缓存 → gitcode 拉仓（仓名映射表见 playbook）→ **追到唯一设置点/调用点**。clone 与 S3 并行，超时预算 5min，失败标 pending 继续。

### S5 执行验证
按 playbook §2-§4 执行：harness 直编**真实被测源码**+通用桩包 → 对照组先行 → 恶意/边界样本构造 → 红灯不变式用例（修复后转绿=回归用例）→ 资源约束多形态（默认/ulimit）。无法构建时如实停在 Reasoned 并附可复跑 UT 设计。

### S6 分级交付（验证状态驱动）
| 验证状态 | 定义 | 定级上限 |
|---|---|---|
| **Reproduced** | harness/UT/POC 实证复现 | P0/P1 |
| **Reasoned** | 跨仓完整故障链推理（每环 file:line），未运行复现 | P1(conditional)/P2 |
| **Refuted** | 硬证据证伪（不可达/防护成立/自攻自身） | 留档不进报告正文 |
| **Unverified** | 仅仓内代码阅读 | 最高 P2-hypothesis |

P0/P1 交付必附**验证包**：证据链 file:line + harness 路径 + 运行日志 + 忠实度边界声明 + 攻击成本标注。

### S7 蒸馏回灌
每轮结束两件事：
1. ≤3 条新 checklist 项回灌 [`../static-checklist.md`](../static-checklist.md)（必须挂本轮 finding ID）+ 一句话方法论共性（写入 playbook，不建新规则文件）。
2. 命中附录一热点模块的新发现，回填该模块行的问题特征计数。

**防重犯机制**（不在 S7，在修复闭环）：确认的 P0/P1 修复任务合入时**必须附红绿回归 UT**（harness 转正为仓库 test/ 用例，修复前 FAIL、修复后 PASS）——这是"犯过一次不重犯"的唯一硬载体。附录二案例索引为**用户指定制**：仅用户明确点名的严重案例登记，日常扫描结果不自动进表。

---

## Mode B — 静态编码审查（checklist lane）

1. **执行**：按 [`static-checklist.md`](../static-checklist.md) 逐类跑 grep 种子 → **LLM 精读确认**（去误报，不收无证据项）。可并行子代理，按类别分片。
2. **定级**：默认 P3；敏感信息/未守卫解析等清单标注 P2 的从其标注；**封顶 P2，不走 refute**（证据可 grep 直核，抽检 10% 复核）。
3. **升格阀**：命中点位于攻击者可控数据流（want 参数/包内容/IPC 入参/文件路径）且汇点危险（路径拼接/分配大小/条件判断/加载执行）→ 生成 HYP 卡转入 Mode A S3。
4. **产出**：hygiene 清单（ID/类别/file:line/建议/来源 finding），单独计数，不参与门禁评分。

---

## 合并报告

仍按 [`codecheck_report_TEMPLATE.md`](../codecheck_report_TEMPLATE.md) 统一模板：
- YAML `dimensions_executed` 词表增加 `vuln-hypothesis-verification`（Mode A）与 `static-checklist`（Mode B）
- §3 必须立即处理：**仅 Mode A 已验证 P0/P1**，每条卡片必填 `verification_status`
- §4 建议跟进：Mode A Reasoned/Unverified 项 + Mode B P2 项（标注 `[B]`）
- §5 分维度速览：两模式各一行 + Mode B 按类别聚合计数
- §6 详情卡片：P0/P1 全量（含验证包路径）；hygiene 项不占卡片
- 评分（conventions §9 双车道制）：`score = max(0, 100 − (30×P0v + 12×P1v + 5×P2a))`，仅计 Mode A 验证项（P0v/P1v）与 Mode A P2；**Mode B 不计分**，以独立 hygiene 计数呈现
- 门禁：存在 Reproduced P0 → block；P1（Reproduced 或 Reasoned-conditional）→ conditional；其余按 score

## Refute 公约（保留，权限收紧）

[`refute-rules.md`](refute-rules.md) 三层质疑继续适用于 Mode A 假设与 Mode B 升格项；新增约束：**仓内代码阅读只能把 P1 降到 Unverified/P2-hypothesis，推翻 P1 必须有跨仓源码或执行级证据**；Reproduced 项不可被纯阅读推翻。

## 不动代码

两模式均只产出报告/假设/验证包，修复由用户确认后另起任务。
