# 会话交接文档：AI Task Manager 分析工作（SESSION-HANDOVER）

> 用途：供后续 AI/工程师在**其他环境**接续本特性方案分析工作。与项目级 `HANDOVER.md`（面向特性全貌）互补，本文聚焦"工作状态 + 未决事项 + 继续分析的抓手"。
> 生成时间：2026-08-24 · 最近更新：2026-08-26（第十四轮代码实证复核完成后）· **文档阶段，未写任何生产代码**。

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

**Git 状态（截至本文更新）**：本目录 12 个提交，最新为第十四轮提交（F-21~F-28 修复）。**第九~十二轮 4 个提交用户已 push**（`365cff1acf`→`6325ac84f0`）；`9f567b4de6`（SESSION-HANDOVER）、第十三轮与第十四轮提交待用户手动 push。

---

## 2. 特性一句话与当前阶段

为 AI 语音助手提供**任务级资源管控机制**：助手创建任务（9 态状态机 + 4 档优先级）→ 启动链路 Want 参数 `ohos.ai.taskId` 拦截纳管目标进程 → 按任务档位对进程做冻结豁免/解冻（RSS）→ 任务结束降级恢复。解决两大痛点：**定时任务锁屏执行失败**、**长时任务切后台被冻结**。

**当前阶段**：SDD 文档已完成十四轮评审迭代（第十三轮自审 + 第十四轮代码实证复核），全交付件（proposal/spec/design/execution-plan/HANDOVER/3 张 SVG/docs 3 份）内容一致、validate PASS。代码实现冻结——用户明确"暂不进入编码"，TASK-0 启动须等用户显式指令。

---

## 3. 交付件清单（全部在 `.codespec/changes/ai-task-manager/`）

| 文件 | 内容与状态 |
|---|---|
| `proposal.md` | 需求基线（2026-08-21 通过）+ Q-1~Q-12 澄清 + **讨论记录补录至第十三轮**（含各轮 commit 追溯） |
| `spec.md` | US-1~6 + AC-1.1~AC-6.2 + 规则 R-1~R-24 + 6 节接口行为规格 |
| `design.md` | 架构设计全貌：任务模型/状态机/防仿冒 B-1+B-2+通道鉴权/AMS 拦截 12 场景/RSS 管控档位/ResourceControlProxy 代理层 + ADR-1~9 + 风险表 |
| `execution-plan.md` | TASK-0~6 分解 + 文件全量清单（含 taskmanager.para）+ AC 追溯 + 代码量估算（生产 ~6,600 + 单测 ~4,300 行） |
| `HANDOVER.md` | 项目级交接（决策表 D-1~D-18、代码勘察 file:line 证据、仓库约束、接手指引） |
| `review-analysis.md` | 第七轮深度审查（F-1~F-7）+ 第十三轮自审（F-8~F-20）+ **第十四轮代码实证复核（F-21~F-28：SA 承载机制/扩展启动覆盖/参数与权限实证）** |
| `docs/extension-analysis.md` | 扩展场景分析（A/B/C 三层 + 三不碰边界 + 优先级建议，二期规划输入） |
| `docs/rss-alignment.md` | RSS 团队对齐问题清单（O-2/O-3/O-5/O-6；P0=505/517/507 语义 8 问含 master 假成功问题） |
| `docs/ai-assistant-alignment.md` | AI 助手团队对齐表（O-1 枚举 + Q-1~Q-12 调用模式） |
| `architecture_overview.svg` / `module_design.svg` / `task_flow.svg` | 三张对外图，第十三轮已与文档对齐（内存登记/sysparam/幂等计数归属/默认值已确认） |
| `SESSION-HANDOVER.md` | 本文 |

---

## 4. 核心决策快照（十四轮固化，勿轻易推翻）

完整版见 `HANDOVER.md` §3（D-1~D-20）。接手最少需知的几条：

- **模块形态**：并入 AppMS（SA 501），三层：`task_mgr`（任务语义）/ `process_schedule`（通用进程调度）/ `resource_control_proxy`（资源管控代理）；**ITaskManager 经 AppMgrService::OnRemoteRequest 描述符路由挂 SA 501**（D-19/F-21，第十四轮：AppMgrStub 既有描述符校验会拒非 IAppMgr 请求，须显式路由，design §5.2.2）
- **纳管**：Want 参数 `ohos.ai.taskId` 统一拦截 + 条件两段式（既有 record 直接纳管/其余意向登记+attach 补注册）；**拦截点①共 5 收敛点**（D-20/F-22，第十四轮：StartAbilityInner/StartExtensionAbilityInner/StartUIExtensionAbility/RequestModalUIExtensionInner/ConnectAbilityCommon——扩展启动均不经 StartAbilityInner）；**进程级纳管**（进程入任务后全部组件共享档位）
- **防仿冒**：B-1 随机 128bit taskId + B-2 callerTokenId ∈ {creator} ∪ {任务内进程} + **通道鉴权**（D-17/F-8，第十三轮：native 信任入参身份；非 native 强制入参=IPC 实际身份 + pid 归属校验——RegisterTaskProcess/Intent 可被系统应用直调的仿冒面闭环）
- **传递通道**（第十轮 D-11）：UI↔助手不假设既有通道，**由本模块提供**——SubmitTaskInput 白名单 = 创建者 + 已注册监听的系统 UI；input 到达经 `OnTaskNotice(INPUT_ARRIVED)` 通知创建者；**payload 更新经 `OnTaskNotice(PAYLOAD_UPDATED)` 通知监听者**（D-18/F-9，第十三轮，进度实时展示）
- **豁免记录**（第十一轮 D-16）：**内存登记**：不持久化、仅活跃任务 `{taskId, pid, action, level, timestamp, result}`，单一职责 = RSS 重启重放（P4）+ 撤销重试，随 AppMS 进程消亡；计时/续期计数记 TaskRecord；**审计/验收/定位/复核一律走 TASK_RSSCONTROL + hilog 日志，对账基线 = 动作日志序列**（不是登记查询）
- **关键参数**（均 sysparam 可调（F-14：`.para`+GetParameter，taskmanager.para 随 TASK-4 新增）、**默认值已确认 2026-08-26 第十三轮**）：创建者任务 ≤16（R-2）、单任务进程 ≤16 + 全局豁免进程 ≤64（R-22，粒度按 pid）、交互超时 30min+宽限 5min+预警提前量 5min、档位 1 限时 30min、硬上限 60min、续期 N=3（**仅 priority 变化触发**）
- **RSS 对接**：一期 RCP-Stub（内存登记+日志+探测开关，master 无 freezer 执行插件会"假成功"——`low_memory_frozen_enable` 参数在开源 master 不存在，F-23；产品启用开关以实际配置为准）→ 对接后 RCP-Impl 替换（条件编译，上层零改动）；EDS 一期经 P7 ApplyAllowResource（design §6.4）；**RSS 权限前置（F-24）**：505/517 需 REPORT_RESOURCE_SCHEDULE_EVENT（foundation 已具备）、507 专用入口需 GET_SUSPEND_STATE（foundation 配置待确认，RSS 对齐 #5）
- **审查红线**：AGENTS.md——不扩 IAppMgr 接口、AMS 经 inner_api 调 AppMS、RSS 仓不改代码、hisysevent 已发布事件不可改

---

## 5. 未决事项（继续分析的主要抓手）

### 5.1 等外部对齐（非阻塞文档收口；**对齐材料已备好，可直接发出**）
| # | 事项 | 依赖方 | 材料 |
|---|---|---|---|
| O-1 | 任务类型/优先级/状态枚举最终对齐 | AI 语音助手团队 | docs/ai-assistant-alignment.md |
| O-2 | 锁屏豁免边界——EDS P7 已设计、**联合调试验收**待 device_standby | RSS / device_standby | docs/rss-alignment.md P2 |
| O-3 | RSS 真实对接（RCP-Impl 落地） | RSS 团队 | docs/rss-alignment.md P1 |
| O-4 | 子进程自动纳入（场景 12）是否进一期 | 需求方 | — |
| O-5 | 520 低内存查杀豁免消费方语义 | RSS 团队 | docs/rss-alignment.md #12 |
| **O-6** | **RSS 505 payload 语义与豁免覆盖范围**——唯一"必须回答"的外部项（RCP-Impl 对接前置） | RSS 团队 | docs/rss-alignment.md P0（8 问） |

### 5.2 待定项（已全部确认，2026-08-26 第十三轮用户拍板）
- ~~N=3 默认续期次数~~ → **确认保留**
- ~~INPUT_ARRIVED 枚举命名~~ → **确认保留**
- ~~交互 30min/宽限 5min/限时 30min/硬上限 60min/预警提前量 5min 默认值~~ → **全部确认保留**（design §6.1/spec R-21 已标注"已确认"）

### 5.3 等用户显式指令
- **TASK-0 开发启动**（用户明确：暂不进入编码；启动时须含 SA 501 描述符路由，F-21）
- 待 push 的提交（`9f567b4de6` SESSION-HANDOVER + 第十三/十四轮提交）

### 5.4 扩展场景分析（已落文档，2026-08-26 第十三轮）
第十二轮后的开放性分析已沉淀为 `docs/extension-analysis.md`：A 层零增量（系统级长时任务通用化/A2A 编排资源保障/TaskType 差异化管控）、B 层一次增量（动态配额/交互态智能降档/多应用仲裁/资源压力预警）、C 层远期（任务为中心调度观/跨设备迁移/分布式纳管）+ 边界三不碰 + 优先级建议（B-2 → A-1 → B-4 → B-1/B-3）。

---

## 6. 接手操作指引

1. **先读**：本文件 → `HANDOVER.md`（项目全貌）→ `proposal.md`（基线与决策链）→ `design.md` → `spec.md` → `execution-plan.md` → `review-analysis.md`（两轮审查记录）
2. **改文档后必做**：`grep -n '账本\|记账' *.md *.svg` 确认无新增残留（历史决策记录行除外）→ `ohos-sdd validate . --level c` → `git commit -s`（不 push）
3. **若继续分析**：优先发出 docs/ 两份对齐材料并跟进结论（O-1/O-6 为关键路径）；结论回写 design §4.1/§5 后冻结；扩展场景见 docs/extension-analysis.md
4. **若开始开发**：须用户显式确认后从 TASK-0 起（IDL + 骨架 + **SA 501 描述符路由**，F-21），每 Task 的 Files/判据/验证命令见 execution-plan.md；TASK-3 需覆盖通道鉴权用例（F-8）与 5 收敛点拦截用例（F-22，含扩展启动三路径）
5. **术语陷阱**：文中"内存登记"≠旧"账本"（后者已废弃）；"动作日志断言"≠"账本断言"；N 指 SetTaskParams 续期次数上限；master 环境 505/517/520 无消费者，端到端验证会假成功（P1 勘察结论）；"CCM"为需求侧术语，本仓落地= sysparam（.para + GetParameter，F-14）

---

*本文由第十二轮评审收口会话生成，2026-08-26 第十三轮自审与第十四轮代码实证复核会话更新；如与 `HANDOVER.md` 冲突，以更新时间较新者为准并回改另一份。*