# 详细流程图集（AI 任务管理机制）

> 生成：2026-08-26 · 依据：design.md（十七轮评审后状态，F-1~F-46 全部修复）
> 约定：图中标注规则（R-x）/发现（F-x）/决策（D-x）出处；**A-8（拦截器混合方案）/A-9（滞留任务处置）为待用户对齐项，图中以【A-8】【A-9】标记**；Mermaid 渲染于 GitCode/GitLab markdown 预览。

## 图 0：流程图索引

| 图 | 流程 | 类型 | 关键决策点 |
|----|------|------|-----------|
| 图 1 | 任务生命周期主流程（端到端） | 时序图 | 创建→激活→纳管→管控→交互→终态→清理 |
| 图 2 | 启动链路拦截与条件两段式纳管 | 决策流程图 | 5 收敛点/身份解析/targetPid 分支/attach 补注册 |
| 图 3 | 通道鉴权与 B-2 校验（AppMS 侧） | 决策流程图 | 规则 0~4/集合校验/上限/幂等 |
| 图 4 | 档位计算与豁免施加（P3 生效校验环） | 决策流程图 | 档位合并/505→507→517/EDS P7/线程模型 |
| 图 5 | INPUT/AUTH 交互承载（AI bar 内化，D-21） | 时序图 | payload 事实记录/监听可观测/AI bar 进程内闭环/流转携带响应 |
| 图 6 | 超时处置与续期（F-5 两段式+双计时器） | 决策流程图 | 交互超时/限时豁免/硬上限/续期 N=3 |
| 图 7 | 任务结束双通道清理 | 决策流程图 | 进程级撤销→任务级通知/重试 |
| 图 8 | 异常场景处置矩阵 | 表+流程 | 进程死亡/创建者死亡/RSS 重启/AppMS 重启 |

---

## 图 1：任务生命周期主流程（端到端时序）

```mermaid
sequenceDiagram
    autonumber
    participant AI as AI 助手(创建者)
    participant LSN as 监听者(已注册 ITaskListener 的系统应用)
    participant AMS as AMS(SA 180)
    participant TMS as TaskManagerService(AppMS·SA 501)
    participant PS as ProcessScheduleManager
    participant RCP as ResourceControlProxy
    participant EXT as RSS(505/507/517)+EDS

    rect rgb(245,248,255)
    Note over AI,TMS: 阶段一 创建与激活(尚无纳管进程)
    AI->>TMS: CreateTask(type, priority, payload)
    Note right of TMS: 系统应用鉴权→参数校验(枚举/payload≤4KB)→<br/>R-2 上限(活跃=非终态≤16)→B-1 随机128bit taskId
    TMS-->>AI: ERR_OK + taskId(任务 UNKNOWN)
    Note over TMS: TASK_CREATE 事件(hisysevent)
    AI->>TMS: SetTaskState(SUBMITTED)
    Note right of TMS: 状态机校验(UNKNOWN→SUBMITTED合法)·<br/>SUBMITTED属激活态(F-1)
    TMS-->>LSN: OnTaskStateChanged(oneway·R-14)
    AI->>TMS: SetTaskState(WORKING)
    end

    rect rgb(245,255,247)
    Note over AI,EXT: 阶段二 启动纳管与管控施加
    AI->>AMS: StartAbility(want含ohos.ai.taskId)
    Note over AMS: TaskInterceptor解析(5收敛点·A-8)→<br/>身份解析→条件两段式(详见图2)
    AMS->>TMS: RegisterTaskIntent或RegisterTaskProcess<br/>(IN_PROCESS_CALL身份重置·F-29)
    Note right of TMS: 通道鉴权(F-8/F-29)→B-2集合校验→<br/>R-22上限→纳管(详见图3)
    TMS->>PS: AddProcessToTask(taskId, pid)
    Note right of PS: effectiveLevel(pid)=max(激活任务档位)·<br/>档位变化→ApplyLevel
    PS->>RCP: ApplyControl(EXEMPT)
    Note right of RCP: RCP内部工作线程执行同步调用(F-44·P6)
    RCP->>EXT: 505豁免→507校验→517解冻(如需)→<br/>EDS ApplyAllowResource(P7)
    Note over RCP: 内存登记+TASK_RSSCONTROL事件(详见图4)
    end

    rect rgb(255,251,242)
    Note over AI,LSN: 阶段三 执行·进度与交互(D-21:进度由AI bar自呈现·交互进程内闭环)
    AI->>TMS: SetTaskParams(payload更新·进度记录)
    Note over AI: 进度呈现=AI bar(助手自有·数据在己方·无外部通知)
    AI->>TMS: SetTaskState(INPUT_REQUIRED, payload)
    Note right of TMS: payload留存TaskRecord(事实记录·R-16)·<br/>交互超时计时启动(30min)
    TMS-->>LSN: OnTaskStateChanged(含payload·安全等模块可观测)
    Note over AI: AI bar呈现描述与可选项→用户选择<br/>(助手进程内闭环·无IPC·无模块中介)
    AI->>TMS: SetTaskState(WORKING, payload可选携带响应)
    end

    rect rgb(255,244,244)
    Note over AI,EXT: 阶段四 终态与双通道清理
    AI->>TMS: SetTaskState(COMPLETED)
    Note right of TMS: 终态不可回退(R-4)·R-18交互态清理
    TMS->>PS: OnTaskActiveChanged(终态)
    PS->>RCP: ①RestoreControl(逐pid·按剩余任务重算)
    PS->>RCP: ②NotifyTaskEnd(taskId,pids,reason)
    Note over RCP: ①=CLEAR_EXEMPT+CLEAR_QOS(二期)·②在①后串行<br/>P7:EDS按uid引用计数撤销·③登记清理(详见图7·F-47)
    TMS-->>LSN: OnTaskStateChanged(COMPLETED)
    AI->>TMS: DestroyTask
    TMS-->>AI: ERR_OK(记录清理·不可再查询)
    end
```

**分步说明**

| 步骤 | 参与方 | 动作 | 依据 | 失败路径 |
|------|--------|------|------|----------|
| 1 | AI→TMS | CreateTask | R-1/R-2/B-1 | 非系统应用→ERR_TASK_PERMISSION_DENIED；参数非法→ERR_INVALID_PARAM；达上限→ERR_TASK_LIMIT_EXCEEDED |
| 2 | AI→TMS | SetTaskState(SUBMITTED) | R-3/状态机表 | 非法流转→ERR_TASK_INVALID_STATE_TRANSITION |
| 3 | AI→AMS | StartAbility(want+taskId) | R-7/R-13 | 无 taskId→原路径零开销 |
| 4 | AMS→TMS | 意向/进程注册（IN_PROCESS_CALL） | F-29 规则 0 | 通道鉴权/B-2 失败→拒绝纳管，启动不受影响（R-8） |
| 5 | TMS→PS | AddProcessToTask | ADR-5 | R-22 超限→拒绝+告警 |
| 6 | PS→RCP→EXT | 施加豁免+生效校验+EDS | R-19/P3/P7 | 未生效→告警+TASK 事件（图 4） |
| 7 | AI→TMS | SetTaskParams(payload) | B-4/D-21 | payload 超限→ERR_INVALID_PARAM（进度呈现由 AI bar 自呈现，无外部通知） |
| 8 | AI 进程内 | AI bar 交互闭环→SetTaskState 流转 | R-17/D-21 | 流转非法→ERR_TASK_INVALID_STATE_TRANSITION（响应可选经流转 payload 携带） |
| 9 | AI→TMS | SetTaskState(COMPLETED) | R-4/R-9 | — |
| 10 | PS→RCP | 双通道清理 | R-20 | 失败重试≤3+告警（图 7） |

---

## 图 2：启动链路拦截与条件两段式纳管（决策流程）

```mermaid
flowchart TD
    S(["启动/连接请求到达AMS"]) --> C1{"Want含<br/>ohos.ai.taskId?"}
    C1 -- "否" --> Z(["短路返回(零开销R-13)"])
    C1 -- "是" --> C2{"收敛点判定(5处)"}
    C2 --> K1["StartAbilityInner :1360<br/>TaskInterceptor"]
    C2 --> K2["StartExtensionAbilityInner :4656<br/>TaskInterceptor"]
    C2 --> K3["StartUIExtensionAbility :4989<br/>TaskInterceptor"]
    C2 --> K4["ConnectAbilityCommon :6063<br/>TaskInterceptor(fromConnect)"]
    C2 --> K5["RequestModalUIExtensionInner :4508<br/>ad-hoc挂接【A-8】"]
    K1 & K2 & K3 & K4 & K5 --> ID["身份解析(业务发起方)"]
    ID --> ID1{"来源判定"}
    ID1 -- "specifyTokenId≠0" --> IDA["取specifyTokenId"]
    ID1 -- "asCallerSourceToken≠0" --> IDB["取asCallerSourceToken"]
    ID1 -- "UIExtension路径" --> IDC["取SessionInfo.callerToken_/<br/>callingTokenId_(F-32)"]
    ID1 -- "Modal路径(应用直调)" --> IDD["取GetCallingTokenID"]
    ID1 -- "其他" --> IDE["callerToken解析→兜底GetCallingTokenID"]
    IDE --> CLI{"CLI路径判定(D-22)"}
    CLI -- "cli_tool_framework<br/>Skill会话" --> CLIS["taskId随skillArgs透传<br/>(ohos.skill.executeParam.args.*)<br/>→AMS收敛点天然覆盖"]
    CLI -- "CLI工具(ExecTool/ExecCmd)<br/>fork+claw_sandbox·不经AMS" --> CLIA["aimgr挂点(§5.4.3):<br/>CreateChildProcess成功后<br/>RegisterTaskProcess<br/>(callerTokenId=ExecTool现场值)"]
    CLIS --> AUTH
    CLIA --> AUTH
    IDA & IDB & IDC & IDD --> TP{"targetPid=<br/>record.GetPid()>0?"}
    TP -- "是(既有record:运行中<br/>Service/单例/扩展复用)" --> RA["分支A直接纳管"]
    TP -- "否(冷启动或热启动新实例<br/>pid仅attach时设置F-11)" --> RB["分支B意向登记"]
    RA --> CA["IN_PROCESS_CALL<br/>RegisterTaskProcess(taskId,pid,身份)"]
    RB --> CB1["IN_PROCESS_CALL<br/>RegisterTaskIntent(taskId,身份)"]
    CB1 --> CB2["AbilityRecord.taskOriginatorTokenId_=身份<br/>(F-31)·启动30s超时计时"]
    CB2 --> CB3{"attach到达?<br/>(AttachAbilityThread优先/<br/>OnAbilityRequestDone兜底)"}
    CB3 -- "30s内到达" --> CB4["IN_PROCESS_CALL<br/>RegisterTaskProcess(taskId,pid,①登记值)"]
    CB3 -- "attach失败" --> CB5["UnregisterTaskIntent<br/>(F-7主动清理)"]
    CB3 -- "30s超时未attach" --> CB6["意向清理"]
    CB6 --> CB7["attach晚到→直接补发②<br/>(F-39·身份取存储值)"]
    CA & CB4 & CB5 & CB7 --> AUTH["进入AppMS校验链(图3)"]
```

**分步说明**

| 步骤 | 动作 | 依据 | 备注 |
|------|------|------|------|
| 1 | Want 解析 taskId | R-13 | 无 taskId 短路，不产生任何副作用 |
| 2 | 收敛点判定 | F-22/D-20 | 4 点走 TaskInterceptor（A-8 待评审），Modal 单点 ad-hoc |
| 3 | 身份解析 | F-32/C-2 | UIExtension 必须取 SessionInfo（GetCallingTokenID=sceneboard 必被 B-2 拒）；CLI 工具路径身份=ExecTool 现场调用方（aimgr 转发，D-22） |
| 4 | targetPid 判定 | F-11 | pid 仅 attach 时设置（ability_record.cpp:1096，全库唯一路径） |
| 5a | 分支 A 直接纳管 | ADR-3 | 既有 record 场景，一次动作 |
| 5b | 分支 B 意向登记 | ADR-3/F-31 | 身份存入 AbilityRecord（新增字段） |
| 6 | attach 补注册 | §5.4.2 | 身份取①登记值（非 attach 现场 GetCallingTokenID，§5.3.1⑧） |
| 7 | attach 失败清理 | F-7 | 避免意向残留 30s |
| 8 | attach 晚于超时 | F-39 | 直接补发②（pid 已就绪，重发①无意义） |
| 9 | CLI 工具进程（D-22） | §5.4.3 | fork+claw_sandbox 不经 AMS——aimgr 挂点注册（通道鉴权规则 1b）；死亡经 aimgr SIGCHLD reaper 清理（与 AppMS 死亡链互补）；Skill 会话路径经收敛点天然覆盖 |

---

## 图 3：通道鉴权与 B-2 校验（AppMS 侧）

```mermaid
flowchart TD
    E(["RegisterTaskIntent/Process<br/>到达AppMS(SA501描述符路由F-21)"]) --> A0{"IPC调用方身份判定<br/>(规则0:AMS调用已经<br/>IN_PROCESS_CALL身份重置)"}
    A0 -- "native且processName=foundation<br/>(CheckSpecificSystemAbility<br/>AccessPermission通过)" --> A1["规则1:信任入参callerTokenId<br/>(AMS转发的业务发起方)"]
    A0 -- "非foundation native(直调)" --> A2{"规则2:入参callerTokenId==<br/>IPC实际调用方tokenId?"}
    A2 -- "否(自报他人身份)" --> X1["拒绝:ERR_TASK_PERMISSION_DENIED<br/>(不透传应用侧C-5)"]
    A2 -- "是" --> A3{"规则3:pid归属校验<br/>(uid一致或pid已在任务内)"}
    A3 -- "不通过(注册他人进程)" --> X1
    A1 --> B2{"B-2:callerTokenId∈<br/>creator∪任务内进程tokenId?"}
    A3 -- "通过" --> B2
    B2 -- "否(伪造/仿冒)" --> X2["拒绝纳管·不上报错误<br/>(防探测R-8)·启动不受影响"]
    B2 -- "是" --> LIM{"R-22上限:<br/>单任务进程≤16且<br/>全局豁免≤64?"}
    LIM -- "超限" --> X3["拒绝+TASK_ABUSE<br/>_WARNING(limit)"]
    LIM -- "未超" --> IDEM{"pid已在任务内?"}
    IDEM -- "是" --> OK2(["幂等成功"])
    IDEM -- "否" --> OK(["纳管成功→AddProcessToTask<br/>→档位重算(图4)"])
```

**分步说明**

| 步骤 | 判定 | 依据 | 攻击面闭环 |
|------|------|------|-----------|
| 0 | 通道身份 | F-29 规则 0 | 不包装则 attach/代理启动链路断裂（身份透传机制） |
| 1 | foundation native 信任 | F-29 规则 1 | 裸 IsSACall 可被其他进程 native SA 绕过，故必须 processName 校验 |
| 2 | 入参=实际身份 | F-8 规则 2 | 杜绝系统应用直调自报 creatorTokenId |
| 3 | pid 归属 | F-8 规则 3 | 防注册他人进程骗取豁免 |
| 4 | B-2 集合 | R-8/B-2 | taskId 伪造（B-1 随机性）+ 非信任链身份 |
| 5 | 上限 | R-22 | 链式纳管膨胀/全局豁免滥用 |

---

## 图 4：档位计算与豁免施加（P3 生效校验环）

```mermaid
flowchart TD
    T(["触发:AddProcessToTask或<br/>OnTaskActiveChanged(状态/优先级变化)"]) --> M["effectiveLevel(pid)=<br/>max(其任务集合中激活任务档位)"]
    M --> D{"档位变化?"}
    D -- "否" --> END1(["结束(无动作)"])
    D -- "是" --> AP["ApplyLevel增量施加/撤销"]
    AP --> W["RCP内部工作线程<br/>(F-44:不占taskHandler不持锁P6)"]
    W --> E1["EXEMPT:505(payload含uid+pid<br/>·value语义假设待RSS确认F-35)"]
    E1 --> Q1["507查询isFrozen<br/>(超时预算≤500ms建议)"]
    Q1 -- "isFrozen=true<br/>(P3已冻结分支)" --> TH["THAW:517解冻"]
    TH --> Q2["507复查"]
    Q2 -- "仍未解冻" --> FAIL["判定失败"]
    Q2 -- "已解冻" --> REG
    Q1 -- "isFrozen=false" --> REG["P7:EDS ApplyAllowResource<br/>(NETWORK·FREEZE位待评估F-34)"]
    REG --> W2["内存登记{taskId,pid,action,<br/>level,timestamp,result}"]
    W2 --> EV["TASK_RSSCONTROL事件+hilog<br/>(完整ControlRequest·对账基线D-16)"]
    EV --> OK3(["完成"])
    FAIL --> AL["TASK_ABUSE_WARNING(verify_failed)<br/>+TASK事件通知创建者(供重试/降级决策)"]
```

**分步说明**

| 步骤 | 动作 | 依据 | 备注 |
|------|------|------|------|
| 1 | 档位合并 | ADR-5/R-10 | 多任务归属按 max |
| 2 | 增量动作 | R-11 | 同 pid 同类型豁免幂等（幂等键 taskId+pid+类型，计数在 process_schedule） |
| 3 | 线程模型 | F-44/P6 | 同步调用移出串行线程，结果回投 taskHandler |
| 4 | 施加顺序 | P3 | 固定 EXEMPT→507→THAW→507 复查 |
| 5 | EDS 联动 | P7/F-30 | 依赖 standby_innerkits（plan 已列）；native 免权限 |
| 6 | 登记+事件 | D-16/R-19 | 内存登记仅活跃任务（重放+撤销重试依据）；审计走日志 |
| 7 | 失败处置 | §6.1.1-2 | 施加失败告警+事件（撤销失败才重试≤3，见图 7） |

---

## 图 5：INPUT/AUTH 交互承载（AI bar 内化，D-21）

```mermaid
sequenceDiagram
    autonumber
    participant AI as AI助手(创建者)
    participant BAR as AI bar(助手自有UI·进程内)
    participant TMS as TaskManagerService
    participant LSN as 监听者(安全等系统模块)
    participant USR as 用户

    AI->>TMS: SetTaskState(INPUT_REQUIRED, payload)
    Note right of TMS: payload必填校验(AC-4.1·≤4KB)·<br/>状态机WORKING→INPUT_REQUIRED·<br/>payload留存TaskRecord(事实记录·R-16)·<br/>交互态属激活态(豁免维持F-5)·<br/>交互超时计时启动(30min)
    TMS-->>LSN: OnTaskStateChanged(taskId,INPUT_REQUIRED,payload)
    Note over LSN: 交互事实可观测(安全/审计)<br/>不参与交互
    AI->>BAR: 呈现描述与可选项(进程内·数据在己方)
    BAR->>USR: 展示
    USR->>BAR: 选择/输入
    BAR->>AI: 响应(进程内·无IPC·无模块中介)
    AI->>TMS: SetTaskState(WORKING或COMPLETED·payload可选携带响应)
    Note over TMS: 出交互态·交互超时计时终止·<br/>TASK_INPUT(require)已记录·submit事件随D-21移除
```

**分步说明**

| 步骤 | 动作 | 依据 | 失败路径 |
|------|------|------|----------|
| 1 | 进入交互态（payload 留存） | R-16/D-21 | payload 缺失→ERR_INVALID_PARAM |
| 2 | 回调携带 payload（监听方可观测） | R-14/D-21 | 监听者死亡自动移除（DeathRecipient） |
| 3 | AI bar 呈现（助手进程内） | D-21 | 无模块中介——不经任务管理模块 |
| 4 | 用户响应→助手直接获得 | D-21 | 助手超时未流转→图 6 交互超时处置 |
| 5 | 流转出交互态（响应可选经 payload 携带） | R-17/D-21 | 流转非法→ERR_TASK_INVALID_STATE_TRANSITION；任务被取消/销毁→R-18 清理不再回调 |

---

## 图 6：超时处置与续期（F-5 两段式+双计时器）

```mermaid
flowchart TD
    A(["计时触发(双计时器取先到·先触发者终止另一计时)"]) --> W{"哪个计时器先到?"}
    W -- "交互超时30min<br/>(交互态任务)" --> N1["OnTaskNotice(INTERACTIVE_TIMEOUT,{deadline})<br/>+TASK_ABUSE_WARNING(timeout_demote)"]
    N1 --> G{"宽限5min内<br/>创建者流转状态?"}
    G -- "是(回WORKING/转终态/续交互)" --> OK(["正常继续"])
    G -- "否" --> DM["task_mgr主动降档:<br/>撤销豁免·按剩余任务max重算<br/>EDS按uid引用计数重评估(F-47)"]
    DM --> A9["任务状态处置【A-9待对齐】:<br/>(a)置FAILED(reason=timeout)·推荐<br/>(b)维持交互态(滞留占R-2配额)"]
    W -- "档位1限时豁免30min到期<br/>(未续期)" --> DM2["撤销豁免+降档+EDS重评估(F-47)+<br/>TASK_ABUSE_WARNING(limit)"]
    W -- "硬上限60min到期前5min<br/>(预警提前量)" --> WARN["OnTaskNotice(EXEMPT_EXPIRING)"]
    WARN --> RN{"创建者SetTaskParams<br/>priority变化续期?<br/>且计数<N=3?"}
    RN -- "是" --> OK2(["重置计时·继续"])
    RN -- "否(未续期或达上限N=3)" --> DM2
    DM2 --> A9
```

**分步说明**

| 步骤 | 动作 | 依据 | 备注 |
|------|------|------|------|
| 1 | 双计时器取先到 | F-5/§6.1 | 交互态同时存在交互超时与豁免计时，先到者处置即终止另一计时 |
| 2 | 限期处置通知 | F-5 第 1 步 | 创建者注册监听则实时感知宽限窗口 |
| 3 | 宽限后降档 | F-5 第 2 步 | 后续助手回 WORKING 时经 P3 分支重新施加 |
| 4 | 滞留任务处置 | F-43/A-9 | **待用户对齐**：(a) 窄口径系统流转（触碰 D-6 边界）；(b) 接受滞留 |
| 5 | 续期 | R-21/D-13 | 仅 priority 变化触发（payload-only 不续期）；N=3 已确认 |
| 6 | 各参数 | §6.1 | 30/5/30/60/5min 与 N=3 均已确认（2026-08-26），persist.sys.appms.taskmanager.* 可调（F-37） |

---

## 图 7：任务结束双通道清理

```mermaid
flowchart TD
    S(["任务进入终态(COMPLETED/CANCELED/<br/>FAILED/REJECTED)或DestroyTask"]) --> PRE["前置清理(终态处理内同步·F-47)<br/>R-18:payload清除·停止回调<br/>取消全部计时器(交互超时/限时/硬上限/续期计数)"]
    PRE --> CH1["通道①进程级撤销(逐pid·taskHandler串行)"]
    CH1 --> RC["RestoreControl:<br/>CLEAR_EXEMPT(撤销冻结豁免·一期)<br/>+ CLEAR_QOS(恢复调度优先级·二期预留)"]
    RC --> RC2["该pid按剩余任务max重算<br/>(无任务→完全恢复原调度·<br/>RSS按原策略自然回冻·无需re-freeze)"]
    RC2 --> ED{"EDS撤销判定(F-47):<br/>该uid+bundleName还有<br/>其他激活态任务?"}
    ED -- "有(引用计数>0)" --> KEEP["跳过Unapply<br/>(防误杀同uid其他任务待机豁免)"]
    ED -- "无(归零)" --> ED2["P7:UnapplyAllowResource"]
    KEEP & ED2 --> CH2["通道②任务级通知(①之后·串行)"]
    CH2 --> NE["NotifyTaskEnd(taskId,pids,reason)<br/>RSS清理任务级策略(豁免组/优先级组·防残留)<br/>注:EDS无任务级策略·uid归零即彻底清理"]
    NE --> CH3["③内存登记清理(F-47):<br/>该任务全部活跃豁免登记条目移除(D-16)"]
    CH3 --> RT{"任一通道失败?"}
    RT -- "是" --> RT2["重试≤3→仍失败→告警<br/>(R-20·日志留痕供复核)"]
    RT -- "否" --> FIN["TASK_DESTROY事件<br/>DestroyTask→记录清理不可再查询"]
    RT2 --> FIN
```

**分步说明**

| 步骤 | 动作 | 依据 | 备注 |
|------|------|------|------|
| 0 | 前置清理：payload 清除/停止回调 + **取消全部计时器** | R-18/F-47 | 在状态机终态处理中同步完成；超时处理器另校验非终态（双保险） |
| 1 | 进程级撤销：CLEAR_EXEMPT + **CLEAR_QOS（二期）** | R-9/R-20 | 多任务归属按剩余最高档继续管控（AC-5.4）；撤销后 RSS 按原策略自然回冻/回降，无需显式 re-freeze |
| 2 | **EDS 撤销判定：uid 级引用计数** | P7/F-47 | EDS 按 uid+bundleName 授权（非 pid）——归零才 Unapply，防误杀同 uid 其他任务 |
| 3 | 任务级通知 NotifyTaskEnd | R-20/§6.4 | ②在①之后串行执行；EDS 无任务级策略（uid 归零即彻底清理） |
| 4 | 内存登记清理 | D-16/F-47 | 该任务全部活跃豁免登记条目移除（③最后执行） |
| 5 | 失败重试 | R-20 | ≤3 次后告警，日志留痕（对账基线=动作日志序列） |

---

## 图 8：异常场景处置矩阵

| # | 场景 | 检测机制 | 处置流程 | 依据 |
|---|------|----------|----------|------|
| 1 | 纳管进程死亡 | AppMS 既有 AppDeathRecipient 链 | RemoveProcessFromTask → 释放豁免 → 档位按剩余任务重算（延迟 ≤1s）；该任务全部进程死亡且同 uid 无其他激活态任务 → EDS 按 uid 引用计数撤销（F-47） | R-12 |
| 2 | 创建者进程死亡 | 复用 AppDeathRecipient（创建者 uid/tokenId 已知） | 【A-9 待对齐】(a) 该创建者全部非终态任务置 CANCELED + 双通道清理（推荐）；(b) 任务滞留（占配额，创建者重启可恢复） | F-43 |
| 3 | RSS（SA 1901）重启 | suspend_manager_base_client 内置死亡通知 | 按内存登记幂等重放全部活跃豁免（重放失败重试 ≤3+告警）；二期 push observer 漂移检测 | P4 |
| 4 | AppMS/foundation 重启 | — | foundation 重启清理全部应用进程，任务记录（内存态）与 RSS 侧豁免随进程消亡同亡，无孤儿豁免 | C-1 |
| 5 | master 无 freezer 执行插件 | — | 505/517/520 上报被丢弃、507 恒 false（假成功）——模块级验证以 Stub 动作日志断言为唯一基线；端到端验证前置=产品含执行插件 | P1/F-23 |
| 6 | 意向超时（30s 未 attach） | task_mgr 定时器 | 意向清理；attach 晚到直接补发② | F-39 |
| 7 | attach 失败（进程启动失败） | attach 失败回调 | 主动 UnregisterTaskIntent | F-7 |
| 8 | 监听者死亡 | DeathRecipient | 注册表自动移除，不再派发 | R-14 |

```mermaid
flowchart LR
    subgraph DEATH["进程死亡处置链"]
        D1(["死亡通知<br/>(AppDeathRecipient)"]) --> D2{"死者身份"}
        D2 -- "纳管进程" --> D3["RemoveProcessFromTask<br/>→档位重算(≤1s)"]
        D2 -- "创建者【A-9】" --> D4["(a)全部非终态任务→CANCELED<br/>+双通道清理"]
        D2 -- "RSS服务(经client死亡通知)" --> D5["按内存登记幂等重放<br/>活跃豁免(重试≤3)"]
    end
```

---

## 附：与既有三张 SVG 的关系

| 文件 | 定位 | 关系 |
|------|------|------|
| architecture_overview.svg | 对外总览（背景/方案/修改点） | 宏观视图，不体现步骤 |
| module_design.svg | 三模块类图式设计 | 静态结构 |
| task_flow.svg | 泳道时序简图 | 主流程简化版 |
| **本文档** | **分步详细流程（含决策分支/失败路径/规则追溯）** | 十七轮评审后全量细化，含图 2~图 8 七个 SVG 未覆盖的流程 |

*本文档为设计文档的流程视图补充；机制细节以 design.md 为准，冲突时以 design.md 为准并回改本文档。*
