# 会话交接文档：AI Task Manager 分析工作（SESSION-HANDOVER）

> 用途：供后续 AI/工程师在**其他环境**接续本特性方案分析工作。与项目级 `HANDOVER.md`（面向特性全貌）互补，本文聚焦"工作状态 + 未决事项 + 继续分析的抓手"。
> 生成时间：2026-08-24 · 第十二轮评审完成后 · **文档阶段，未写任何生产代码**。

---

## 1. 环境与仓库信息

| 项 | 值 |
|---|---|
| 仓库 | OpenHarmony `foundation/ability/ability_runtime` |
| 工作目录 | `/srv/workspace/openharmony_master_default_20260819175948_huawei_65b5005ee/code/foundation/ability/ability_runtime` |
| 分支 | `feature/ai-task-manager`（基于 master `aaf020f296`；远端 `yzk/feature/ai-task-manager`，无 upstream 跟踪） |
| 交付目录 | `.codespec/changes/ai-task-manager/` |
| 提交规范 | `git commit -s`（Signed-off-by: yangzk <yangzhongkai@huawei.com>）；**push 由用户手动执行** |
| SDD 工具 | ohos-sdd 插件：`export PATH=/home/huawei/.claude/plugins/cache/ohos-sdd-local/ohos-sdd/0.3.3/bin:$PATH` |
| 校验命令 | 在 `.codespec/changes/ai-task-manager` 内执行 `ohos-sdd validate . --level c`（当前 4 checks 0 broken PASS；`--level all` 因缺归档证据必 fail，属流程末端项，勿误报） |

**Git 状态（截至本文生成）**：本目录 9 个提交，最新 `6325ac84f0`（第十二轮）。**4 个提交待用户手动 push**：
`365cff1acf`（第九轮）→ `9a5864e8b5`（第十轮）→ `7747518002`（第十一轮）→ `6325ac84f0`（第十二轮）。

---

## 2. 特性一句话与当前阶段

为 AI 语音助手提供**任务级资源管控机制**：助手创建任务（9 态状态机 + 4 档优先级）→ 启动链路 Want 参数 `ohos.ai.taskId` 拦截纳管目标进程 → 按任务档位对进程做冻结豁免/解冻（RSS）→ 任务结束降级恢复。解决两大痛点：**定时任务锁屏执行失败**、**长时任务切后台被冻结**。

**当前阶段**：SDD 文档已完成十二轮评审迭代，全交付件（proposal/spec/design/execution-plan/HANDOVER/3 张 SVG）内容一致、validate PASS。代码实现冻结——用户明确"本会话不做开发实现，仅方案分析"，TASK-0 启动须等用户显式指令。

---

## 3. 交付件清单（全部在 `.codespec/changes/ai-task-manager/`）

| 文件 | 内容与状态 |
|---|---|
| `proposal.md` | 需求基线（2026-08-21 通过）+ Q-1~Q-12 澄清 + **讨论记录补录至第十一轮**（含各轮 commit 追溯） |
| `spec.md` | US-1~6 + AC-1.1~AC-6.2 + 规则 R-1~R-24 + 6 节接口行为规格 |
| `design.md` | 架构设计全貌：任务模型/状态机/防仿冒 B-1+B-2/AMS 拦截 12 场景/RSS 管控档位/ResourceControlProxy 代理层 + ADR-1~9 + 风险表 |
| `execution-plan.md` | TASK-0~6 分解 + 文件全量清单 + AC 追溯 + 代码量估算（生产 ~6,600 + 单测 ~4,300 行） |
| `HANDOVER.md` | 项目级交接（决策表 D-1~D-16、代码勘察 file:line 证据、仓库约束、接手指引） |
| `review-analysis.md` | 第七轮深度审查报告（F-1~F-7 修正记录，历史文档未再更新——八轮后缺陷走 design/spec 直接修复） |
| `architecture_overview.svg` / `module_design.svg` / `task_flow.svg` | 三张对外图，第十二轮已与文档对齐（内存登记/CCM/N=3/上限数字） |
| `SESSION-HANDOVER.md` | 本文 |

---

## 4. 核心决策快照（十一轮固化，勿轻易推翻）

完整版见 `HANDOVER.md` §3（D-1~D-16）。接手最少需知的几条：

- **模块形态**：并入 AppMS（SA 501），三层：`task_mgr`（任务语义）/ `process_schedule`（通用进程调度）/ `resource_control_proxy`（资源管控代理）
- **纳管**：Want 参数 `ohos.ai.taskId` 统一拦截 + 条件两段式（进程已存在直接纳管/未创建先登记意向、attach 补纳管）；**进程级纳管**（进程入任务后全部组件共享档位）
- **防仿冒**：B-1 随机 128bit taskId + B-2 callerTokenId ∈ {creator} ∪ {任务内进程}
- **传递通道**（第十轮 D-11）：UI↔助手不假设既有通道，**由本模块提供**——SubmitTaskInput 白名单 = 创建者 + 已注册监听的系统 UI；input 到达经 `OnTaskNotice(INPUT_ARRIVED)` 通知创建者
- **豁免记录**（第十一轮 D-16，本轮主题）："动作账本"已降级为**内存登记**：不持久化、仅活跃任务 `{taskId, pid, action, level, timestamp, result}`，单一职责 = RSS 重启重放（P4）+ 撤销重试，随 AppMS 进程消亡；计时/续期计数记 TaskRecord；**审计/验收/定位/复核一律走 TASK_RSSCONTROL + hilog 日志，对账基线 = 动作日志序列**（不是登记查询）
- **关键参数**（均 CCM 可调、默认值评估中）：创建者任务 ≤16（R-2）、单任务进程 ≤16 + 全局豁免进程 ≤64（R-22，**粒度按 pid**）、交互超时 30min+宽限 5min、档位 1 限时 30min、硬上限 60min、续期 N=3（**仅 priority 变化触发**）
- **RSS 对接**：一期 RCP-Stub（内存登记+日志+探测开关，master 无 freezer 执行器会"假成功"）→ 对接后 RCP-Impl 替换（条件编译，上层零改动）；EDS 一期经 P7 ApplyAllowResource（design §6.4）
- **审查红线**：AGENTS.md——不扩 IAppMgr 接口、AMS 经 inner_api 调 AppMS、RSS 仓不改代码、hisysevent 已发布事件不可改

---

## 5. 未决事项（继续分析的主要抓手）

### 5.1 等外部对齐（非阻塞文档收口）
| # | 事项 | 依赖方 |
|---|---|---|
| O-1 | 任务类型/优先级/状态枚举最终对齐 | AI 语音助手团队 |
| O-2 | 锁屏豁免边界——EDS P7 已设计、**联合调试验收**待 device_standby | RSS / device_standby |
| O-3 | RSS 真实对接（RCP-Impl 落地） | RSS 团队 |
| O-4 | 子进程自动纳入（场景 12）是否进一期 | 需求方 |
| O-5 | 520 低内存查杀豁免消费方语义 | RSS 团队 |
| **O-6** | **RSS 505 payload 语义与豁免覆盖范围**——唯一"必须回答"的外部项（RCP-Impl 对接前置） | RSS 团队 |

### 5.2 等用户确认（AI 自主选择、已 flag 未确认）
- **N=3 默认续期次数**（第十轮自主选择）
- **INPUT_ARRIVED 枚举命名**（第十轮自主选择）
- 交互 30min/宽限 5min/限时 30min/硬上限 60min 各默认值（已声明"评估调整中"，落地前 CCM 可调）

### 5.3 等用户显式指令
- **TASK-0 开发启动**（用户明确：当前仅方案分析，不写代码）
- 待 push 的 4 个提交

### 5.4 上一会话的开放性分析（未落文档，可作为二期规划输入）
第十二轮后应用户要求做过"机制还能解决什么痛点 + 扩展场景"分析，结论摘要：
- **A 层（零设计增量）**：系统级长时任务通用化（process_schedule 供复用，design:235）；A2A 多 Agent 编排资源保障（场景 9 链路已覆盖）；TaskType 驱动差异化管控（ControlRequest 已暴露上下文）
- **B 层（一次设计增量）**：任务资源画像/Cost Limits 动态配额；交互态智能降档（design:443 已列二期）；多 AI 应用资源仲裁（档位合并/上限已是仲裁原语）；OnTaskNotice 扩 RESOURCE_PRESSURE 类预警
- **C 层（远期跟踪）**：以任务为中心的调度观；跨设备任务迁移；分布式远端进程纳管
- **边界三不碰**：安全管控、定时调度本身、任务内容语义理解

---

## 6. 接手操作指引

1. **先读**：本文件 → `HANDOVER.md`（项目全貌）→ `proposal.md`（基线与决策链）→ `design.md` → `spec.md` → `execution-plan.md`
2. **改文档后必做**：`grep -n '账本' *.md *.svg` 确认无新增残留（历史决策记录行除外）→ `ohos-sdd validate . --level c` → `git commit -s`（不 push）
3. **若继续分析**：优先啃 §5 未决事项；扩展场景分析如需落文档，建议新增 `docs/extension-analysis.md` 而非改动已定稿六件套
4. **若开始开发**：须用户显式确认后从 TASK-0 起（IDL + 骨架），每 Task 的 Files/判据/验证命令见 execution-plan.md
5. **术语陷阱**：文中"内存登记"≠旧"账本"（后者已废弃）；"动作日志断言"≠"账本断言"；N 指 SetTaskParams 续期次数上限；master 环境 505/517/520 无消费者，端到端验证会假成功（P1 勘察结论）

---

*本文由第十二轮评审收口会话生成；如与 `HANDOVER.md` 冲突，以更新时间较新者为准并回改另一份。*