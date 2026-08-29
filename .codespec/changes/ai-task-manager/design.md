# 架构设计：AI 语音助手任务管理机制

## 设计元数据

| 字段 | 内容 |
|------|------|
| Design ID | DESIGN-AI-TASK-MGR-001 |
| 关联需求 | `.codespec/changes/ai-task-manager/proposal.md`（基线通过） |
| 关联 Epic | 无 |
| 目标 Feature | FEAT-AI-TASK-MGR-001 |
| 复杂度 | 标准 |
| 目标版本 | 7.1 |
| Owner | yangzk |
| 状态 | Draft（待设计评审） |

## 需求基线

> 基线详见 proposal.md。以下仅列设计阶段需强调的要点。

| 项 | 补充说明 |
|----|----------|
| 模块形态 | 并入 AppMS（SA 501，foundation 进程）：任务管理模块 + 通用"App 进程调度管理"能力（A-2 收口） |
| 防仿冒 | B-1+B-2：不可猜测随机 taskId + caller 身份绑定校验（本期确认） |
| RSS 集成 | 复用现有接口（505/517/506/507 等），不新增 RSS 插件/resType；管控维度设计阶段细化 |
| RSS 对接方式 | **RSS 具体设计由 RSS 团队主导，暂未明确**；本模块封装 **ResourceControlProxy 资源管控代理层**（按管控动作语义抽象接口并暴露完整任务/进程上下文，一期骨架实现：内存登记+告警，真实豁免/解冻在 RSS 对接后生效），后续明确后替换实现，上层零改动 |
| 枚举对齐 | 任务类型/优先级/状态枚举初稿（§4.1）需与 **AI 语音助手团队对齐**（开放项，评审后确认） |
| 锁屏豁免边界 | 搁置，不阻塞设计；真机验证阶段与 RSS owner 对齐 |
| sessionQos | 设计阶段探索（本设计 6.3 节结论） |
| 任务类型/优先级枚举 | 设计阶段细化（本设计 4.1 节初稿，随设计评审确认） |

## 上下文和现状

### 涉及仓和模块

| 仓库 | 补充架构说明 |
|------|-------------|
| ability_ability_runtime | AppMS（SA 501）与 AMS（SA 180）同 foundation 进程；`services/appmgr/` 为 🔴 高风险目录（AGENTS.md），新模块须独立目录 + 最小挂点；`services/abilitymgr/` 同样 🔴，拦截点改动须最小化；appmgr BUILD.gn:159 已依赖 `resource_schedule_service:ressched_client` |
| resource_schedule_service | `ResSchedClient::ReportData(resType, value, mapPayload)`（res_sched_client.h:69）与 `ReportSyncEvent`（:79）；505/517/506/507/518 为冻结豁免/解冻/查询通道；221/134/23/8 为 CPU 优先级通道（白名单 res_sched_service.cpp:151,175）；520 为低内存杀进程前同步查询；qos_manager 仓为内核 sched_qos 用户态封装（`/proc/thread-self/sched_qos_ctrl`，qos_interface.cpp:48），无 session 维度接口 |

### 调用链层级分析

| 层 | 模块 | 职责 | 修改类型 |
|----|------|------|----------|
| 应用层（AI 助手；安全管控/审计/系统状态监控等监听方，系统应用） | 调用方进程 | 经 TaskManagerClient 调用任务接口（D-21：监听消费方重定位） | 新增（外部调用方，本仓提供客户端） |
| inner API 客户端 | `interfaces/inner_api/app_manager/task_manager/` | TaskManagerClient / ITaskManager stub·proxy / TaskListener | 新增 |
| 服务层 | `services/appmgr/src/task_mgr/` | 任务管理：任务模型、状态机、监听、交互传达、DFX | 新增 |
| 服务层 | `services/appmgr/src/process_schedule/` | 通用进程调度管理：进程组、管控档位、管控动作施加/撤销、多任务归属合并 | 新增 |
| 服务层（RSS 适配层） | `services/appmgr/src/resource_control_proxy/` | **RSS Proxy 代理层**：按管控动作语义抽象 RSS 能力接口；一期骨架实现（内存登记+告警日志，可配探测对接开关）；RSS 明确后替换真实实现 | 新增 |
| 服务层 | `services/abilitymgr` | 启动/连接链路 Want 拦截：解析 taskId → 经 app_manager inner_api 客户端上报 AppMS | 修改（最小拦截点） |
| 平台能力 | resource_schedule_service（ressched_client） | 冻结豁免/解冻/状态查询（一期）；进程级 QoS（二期 221） | 复用 |
| 平台能力 | qos_manager（QosController/RTG） | 线程级 QoS 提权（二期，需线程 tid） | 复用（二期） |

**检查项：**
- [x] 调用链每一层都已覆盖（应用 → inner API 客户端 → AppMS 服务 → RSS/qos 平台能力；AMS 拦截点并入）
- [x] 每层职责边界清晰（任务语义 vs 进程调度 vs 平台管控；无跨层违规调用——AMS 经 inner_api 客户端调 AppMS，符合 AGENTS.md）
- [x] 每层修改类型明确

### 适用架构规则

| Rule ID | 适用原因 | 设计结论 | 验证方式 |
|---------|----------|----------|----------|
| OH-ARCH-LAYERING | services 与 interfaces/inner_api 解耦 | AppMS 内部实现不直接暴露；对外仅 ITaskManager IPC；客户端独立组件 | 架构评审/依赖检查 |
| OH-ARCH-SUBSYSTEM | AMS→AppMS 跨子模块 | AMS 拦截点经 `interfaces/inner_api/app_manager/` 客户端调用（AGENTS.md 明示，禁止直接调 AppMS 内部方法） | 代码评审 |
| OH-ARCH-IPC-SAF | 新增 SA 级 IPC 接口 | 独立 ITaskManager IDL（stub/proxy 生成），不扩 IAppMgr 既有接口、不改既有序列化 | 集成测试 |
| OH-ARCH-API-LEVEL | 新增 InnerApi | InnerApi 级别，系统应用身份校验（不新增权限项） | API 评审 |
| OH-ARCH-COMPONENT-BUILD | 新增源文件/IDL | appmgr BUILD.gn 增源文件；新增 inner_api 组件；hisysevent.yaml 增事件 | 构建验证 |
| OH-ARCH-ERROR-LOG | 新增错误码/日志 | 任务域错误码段 + hilog_tag_wrapper 日志 + hisysevent 事件 | 单测/hilog |

## 不涉及项承接

| 维度 | 设计结论 |
|------|----------|
| 性能 | 拦截点零开销（无 taskId 直接短路）；任务操作（AI 助手/监听方系统模块）为 ITaskManager 跨进程 IPC（含 Binder 往返），AMS 拦截点→AppMS 为 foundation 进程内调用（经 IN_PROCESS_CALL 身份重置，F-29）；状态机事件处理器串行，任务操作端到端 <5ms（spec NFR） |
| 安全与权限 | 仅基础鉴权：接口入口校验系统应用身份；纳管走 B-2 caller 校验；安全管控细则（隐私/授权策略）不在范围 |
| 兼容性 | 全新增 inner API 与模块；无 taskId 启动行为不变（R-13）；新增配置项不改变既有默认行为 |
| API/SDK | 新增 ITaskManager InnerApi（IDL），无 Public/System API |
| IPC/跨进程 | ITaskManager stub/proxy；AMS→AppMS 经既有 app_manager 客户端体系 |
| 构建与部件 | appmgr 部件扩展（libappms.z.so 内新子模块），无新部件 |

## 关键设计决策

| 决策 ID | 问题 | 推荐方案 | 探索过的替代方案 | 取舍理由 | 影响 |
|---------|------|----------|-----------------|------|------|
| ADR-1 | 模块形态 | 并入 AppMS（SA 501）：task_mgr + process_schedule 两子模块 | agentmgr 内嵌（A-1：A2A/意图/Cli 进程不经过，收口不全，放弃）；独立 SA（A-3：进程/IPC 形态重，进程信息仍需 AppMS，放弃） | 进程权威信息在 AppMS；RSS 通道现成；收口单一接口面（proposal 方案探索 A-2） | 影响范围表、任务 #5 |
| ADR-2 | taskId 传递与防仿冒 | Want 参数统一拦截（key 约定 `ohos.ai.taskId`）+ B-1 随机 taskId + B-2 caller 身份绑定校验 | token 化（B-3：短时 token 换 taskId，防重放增强但增加调用方负担，作可选增强）；显式注册（B-4：与统一拦截冲突，放弃） | 调用方无感知；caller 校验防仿冒最彻底；链式启动语义与 ADK supervisor 可信链一致 | 接口 RegisterTaskProcess 带 callerTokenId；AC-2.2 |
| ADR-3 | 进程纳管时机 | **条件两段式**：拦截点① 解析 Want + 判定目标 pid——进程已存在（record->GetPid()>0）→ 直接 RegisterTaskProcess；进程未创建 → RegisterTaskIntent 意向登记，attach（pid 就绪）时补 RegisterTaskProcess | 仅启动请求时纳管（进程未创建无 pid，失败率高）；仅 attach 后纳管（已存在进程无 attach，无法覆盖，且 taskId 需穿透 attach 链） | 覆盖进程未创建/已存在两态（评审第六轮修正）；attach 点复用 AMS 既有 pid 就绪回调，appmgr 热路径零侵入 | AC-2.1 |
| ADR-4 | RSS 集成方式 | **封装 RSS Proxy 代理层**（代理模式）：按管控动作语义定义接口（豁免/解冻/查询/提权预留），一期骨架实现（内存登记+告警，可配探测开关），RSS 团队设计明确后替换真实实现（优先复用现有 resType 505/517，如 RSS 提供新接口则直接对接） | 直接复用现有 resType 对接（payload 语义未明确，无法落地）；新增 RSS resType/插件（RSS 团队主导，触发内存基线评审） | RSS 设计由 RSS 团队主导（暂未明确）；代理层隔离使 RSS 对接变化不波及任务/调度上层；白名单已确认（res_sched_service.cpp:175）可供对接期参考 | AC-5.x |
| ADR-5 | 管控档位模型 | 档位化 + 合并：任务优先级→管控档位（0~3）；进程档位 = max(纳管任务档位)；档位变化增量施加/撤销 RSS 动作 | 每任务独立施加（同进程重复调用 RSS，幂等管理复杂）；全局统一（无法表达优先级差） | 档位合并天然满足需求 11（多任务按最高）；增量动作避免重复调用 | AC-2.3, AC-5.4 |
| ADR-6 | 状态机驱动 | AI 助手驱动 + 模块校验（合法流转表；终态不可回退；REJECTED 仅创建者可设） | 助手全权设置（状态易乱，放弃）；模块自动推导（语义易偏差，放弃） | 与需求 2/5 一致；流转合法性由系统把关 | AC-1.2, AC-1.3 |
| ADR-7 | INPUT/AUTH 交互承载 | **AI bar 内化交互（D-21，2026-08-26 需求变更）**：INPUT_REQUIRED/AUTH_REQUIRED 状态 + payload（描述+可选项）由创建者设置并留存任务记录（供 AI bar 呈现与状态可观测）；人机交互（用户选择/输入）在助手进程内经 AI bar 完成，**不经任务管理模块传递**；交互响应可选取流转 payload 携带（无专用机制）。状态监听通道保留，消费者=安全等系统模块（状态感知/审计） | ~~状态监听回调携带 payload + SubmitTaskInput 回传通道（D-11，2026-08-24 决策）~~——被 D-21 取代（进度展示与交互均由 AI bar 承担，外部传递通道无消费方）；独立消息通道（更重，放弃） | 与 ADK HITL 语义对应（挂起-恢复由助手自闭环）；任务模块职责收敛为状态事实记录；监听能力保留供安全等模块 | AC-4.x |
| ADR-8 | 线程模型 | AppMS 内独立事件处理器（taskHandler 串行线程）处理全部任务操作与回调派发；**IPC 入口线程模型（F-44 补全，第十七轮）：stub 方法 post-and-wait taskHandler（CreateTask 等需同步返回值）；RCP 同步调用（507/517）在 RCP 内部工作线程执行、结果回投 taskHandler——不得在 taskHandler 上直接发起同步调用（串行线程即全局锁，一次 507 超时将阻塞全部任务操作与等待中的 binder 线程）** | 多线程并发处理（需锁/一致性管理，收益低）；RCP 同步调用直接在 taskHandler 执行（实现简单但阻塞放大：超时预算 × 并发任务数） | 与 appmgr/agentmgr 既有 EventHandlerWrap 风格一致；串行免锁、回调顺序有保证；P6"不持锁"要求在串行模型下唯一自洽解 = 同步调用移出串行线程 | 非功能（并发 ≥16） |
| ADR-9 | QoS 提权（sessionQos 探索结论） | 一期冻结豁免主线；管控档位预留 QoS 扩展位；二期进程级走 RSS 221（PROCESS_QOS_CHANGE），线程级走 QosController/RTG | 一期即上线程级 QoS（需线程 tid：目标进程协作或 /proc 遍历，复杂度高且收益不明，后置） | "sessionQos" 无既有接口（全树勘察）；QosManager 为内核 sched_qos 用户态封装，线程级；任务级"会话语义"由本模块管控档位承载 | 资源调度子模块扩展点 |

## 任务模型（设计细化，随评审确认）

### 4.1 枚举初稿

**TaskState（需求给定，规范化为枚举）：**

```
enum class TaskState {
    UNKNOWN,        // 创建后初始态
    SUBMITTED,      // 已提交（助手提交执行）
    WORKING,        // 执行中
    INPUT_REQUIRED, // 等待用户输入（携带 payload）
    AUTH_REQUIRED,  // 等待用户授权（携带 payload）
    COMPLETED,      // 成功完成（终态）
    CANCELED,       // 已取消（终态）
    FAILED,         // 执行失败（终态）
    REJECTED,       // 被创建者拒绝（终态，仅创建者可设）
};
```

**TaskPriority（4 档 → 映射管控档位）：**

```
enum class TaskPriority {
    LOW,        // 档位 0：不豁免
    MEDIUM,     // 档位 1：限时豁免（时长可配，默认 30min）
    HIGH,       // 档位 2：激活态持续豁免（受时长硬上限约束，可续期）
    CRITICAL,   // 档位 3：豁免 + QoS 提权（二期实现提权部分）
};
```

**TaskType（初稿，覆盖 AI 助手典型场景，可扩展）：**

```
enum class TaskType {
    GENERAL,            // 通用
    VOICE_PLAYBACK,     // 语音播报
    TIMER_TASK,         // 定时任务（锁屏场景核心）
    DATA_PROCESSING,    // 数据处理/长时任务（切后台场景核心）
    SYNC_TRANSFER,      // 同步/传输
    INTERACTION,        // 交互类（INPUT/AUTH 为主）
    DOWNLOAD,           // 下载
    CUSTOM,             // 自定义（保留扩展位）
};
```

### 4.2 状态机流转表

```
创建(UNKNOWN) ──┬──> SUBMITTED ──> WORKING ──┬──> INPUT_REQUIRED ──┐
                │                            ├──> AUTH_REQUIRED ──┤
                └──> REJECTED（终态）         │                    ├─（AI bar 交互完成·创建者流转）──> WORKING
                                             │                    └─（回传即完成）──> COMPLETED
                                             ├──> COMPLETED（终态）
                                             ├──> FAILED（终态）
                                             └──> CANCELED（终态，CancelTask/任意非终态）
```

| 当前状态 | 合法目标状态 | 触发者 | 约束 |
|----------|--------------|--------|------|
| UNKNOWN | SUBMITTED / CANCELED | 创建者 | CANCELED 经 CancelTask（或创建者显式 SetTaskState）触发，任意非终态可取消（A-1 对齐） |
| UNKNOWN | REJECTED | 创建者 | REJECTED 仅创建者可设（AC-1.3） |
| SUBMITTED | WORKING / REJECTED / CANCELED | 创建者 | CANCELED 同上 |
| WORKING | INPUT_REQUIRED / AUTH_REQUIRED / COMPLETED / FAILED / CANCELED | 创建者 | 进入交互状态必须携带 payload（AC-4.1） |
| INPUT_REQUIRED | WORKING / COMPLETED / FAILED / CANCELED | 创建者 | 交互在 AI bar 内闭环（D-21），流转由创建者驱动 |
| AUTH_REQUIRED | WORKING / COMPLETED / FAILED / CANCELED | 创建者 | 同上 |
| 任意终态 | 无（不可回退） | — | R-4 |

> 校验实现：`TaskStateMachine::CanTransition(from, to)` + 创建者身份校验（R-6）。
>
> **副作用钩子（D-3 补全）**：SetTaskState / CancelTask / DestroyTask / SetTaskParams 校验通过并更新 TaskRecord 后，task_mgr 在同一事件处理中同步调用 `OnTaskActiveChanged(taskId)` 触发 process_schedule 档位重算（§5.6；数据流步骤 3/8）——状态变化与管控变化严格联动，无异步窗口。

### 4.3 任务记录（TaskRecord）

> 基础字段 ≤1KB；payload 为变长字段 ≤4KB（可配，另计，F-12 修正；input 字段随 D-21 移除）。豁免幂等引用计数不在 TaskRecord（per-pid 归属 process_schedule 档位状态，F-15 修正）。

```
struct TaskRecord {
    std::string taskId;            // 不可猜测随机 ID（16 字节熵，UUID v4 风格）
    TaskType type;
    TaskPriority priority;
    TaskState state;
    std::string payload;           // 状态 payload（交互描述/可选项/进度）
    int32_t creatorTokenId;        // B-2 校验：创建者 tokenId
    int32_t creatorUid;            // 创建者 uid（多用户隔离维度）
    int32_t userId;                // 创建者 userId（跨用户不可见）
    std::set<int32_t> processPids; // 纳管进程集合
    uint64_t createTime;           // 创建时间戳
    uint64_t updateTime;           // 最近状态变更时间戳
};
// 豁免计时与续期计数（N=3 可配）同为 TaskRecord 内存态字段（§6.1/R-21，第十轮）
```

## 模块设计

### 5.1 模块结构（services/appmgr 内新增，独立目录）

```
services/appmgr/
├── include/task_mgr/                     # 任务管理模块（任务语义层）
│   ├── task_manager_service.h            # 任务管理入口（AppMS 内嵌，事件处理器）
│   ├── task_record.h                     # 任务记录（4.3）
│   ├── task_state_machine.h              # 状态机与流转校验（4.2）
│   ├── task_listener_manager.h           # 监听注册/注销/回调派发
│   ├── task_intent_manager.h             # 意向登记（两段式①）：taskId→{callerTokenId, 登记时间}；30s 超时/attach 失败清理（F-7，B-7 补全）
│   └── task_dfx.h                        # hisysevent 上报/dump
├── src/task_mgr/…（对应实现）
├── include/process_schedule/             # 通用进程调度管理（进程调度能力层，与任务语义解耦）
│   ├── process_schedule_manager.h        # 进程组注册/注销、档位计算、管控动作施加/撤销
│   ├── process_schedule_policy.h         # 策略：优先级→档位映射、上限/时长配置
│   └── process_schedule_types.h          # 档位枚举、管控动作记录
├── src/process_schedule/…（对应实现）
├── include/resource_control_proxy/                    # RSS Proxy 代理层（RSS 适配，代理模式）
│   ├── resource_control_proxy.h                       # 管控动作语义接口（豁免/解冻/查询/提权预留）
│   ├── resource_control_proxy_stub.h                  # 一期骨架实现：内存登记（活跃豁免）+ 告警（可配探测开关）
│   └── resource_control_proxy_impl.h                  # RSS 对接实现（RSS 团队明确后落地，默认编译期二选一）
└── src/resource_control_proxy/…（对应实现）

interfaces/inner_api/app_manager/task_manager/
├── ITaskManager.idl                      # 接口定义（IDL 生成 stub/proxy）
├── include/task_manager_client.h         # C++ 客户端（AI 助手/监听方系统模块/AMS 使用）
├── include/task_listener.h               # ITaskListener 回调接口（IRemoteBroker）
└── BUILD.gn
```

**进程调度管理（通用能力）接口（内部，AppMS 域内）：**

```
class ProcessScheduleManager {          // 单例，taskHandler 串行访问
    int32_t AddProcessToTask(const std::string& taskId, int32_t pid, int32_t priority);
    int32_t RemoveProcessFromTask(const std::string& taskId, int32_t pid);
    void    OnProcessDied(int32_t pid);                        // 复用 appmgr 死亡通知链
    void    OnTaskActiveChanged(const std::string& taskId);    // 任务状态/优先级变化触发重算
    Level   GetEffectiveLevel(int32_t pid);                    // 档位合并（max）
private:
    void    ApplyLevel(int32_t pid, Level level);              // 增量施加/撤销管控动作（经 ResourceControlProxy）
    std::unordered_map<int32_t, std::set<std::string>> pidToTasks_;
    std::unordered_map<std::string, std::set<int32_t>> taskToPids_;
    std::unordered_map<int32_t, Level> effectiveLevel_;
};
```

> 该层不感知"任务"业务语义（状态机/交互），只管理"进程组 + 档位 + 管控动作"，供任务层及其他未来场景（系统级长时任务等）复用（ADR-1 抽象化）。管控动作的执行经 **ResourceControlProxy** 下发（§6.4），不直接依赖 RSS 接口细节。

### 5.2 接口定义（ITaskManager.idl，InnerApi）

接口方法（签名为 IDL 形式，以定稿为准）：

```
interface ITaskManager {
    int32 CreateTask(in TaskCreateParams params, out string taskId);
    int32 DestroyTask(in string taskId);
    int32 CancelTask(in string taskId);
    int32 SetTaskState(in string taskId, in int32 state, in string payload);
    int32 SetTaskParams(in string taskId, in TaskParams params);
    int32 GetTaskInfo(in string taskId, out TaskInfo info);
    int32 GetTaskInfos(out List<TaskInfo> infos);
    int32 RegisterTaskListener(in IRemoteObject listener);
    int32 UnregisterTaskListener(in IRemoteObject listener);
    // SubmitTaskInput 已移除（D-21：交互经 AI bar 内化，无外部传递通道）
    // 内部（AMS 拦截点→AppMS，经 app_manager 客户端体系）
    int32 RegisterTaskIntent(in string taskId, in int32 callerTokenId);  // 两段式①
    int32 RegisterTaskProcess(in string taskId, in int32 pid, in int32 callerTokenId); // 两段式②
    int32 UnregisterTaskProcess(in string taskId, in int32 pid);
}
```

数据结构（IDL struct / parcelable）：

```
struct TaskCreateParams { int32 type; int32 priority; string payload; };  // payload ≤4KB
struct TaskParams { int32 priority; string payload; };  // 可调参数：priority/payload（type 创建后不可调，受限规则见 §5.6，B-3/B-4 补全）
struct TaskInfo {
    string taskId; int32 type; int32 priority; int32 state;
    string payload; List<int32> processPids;
    int64 createTime; int64 updateTime;
};
```

**回调接口：**

```
interface ITaskListener {   // 监听方注册（安全管控/审计/系统状态监控等系统模块，D-21）
    [oneway] void OnTaskStateChanged(in string taskId, in int32 state, in string payload, in int64 timestamp);
    [oneway] void OnTaskNotice(in string taskId, in int32 noticeType, in string payload, in int64 timestamp);
    // 通知类事件（B-5 补全）：交互超时限期处置（F-5）、限时豁免到期预警（C-2）等非状态变更通知；
    // 两回调均 oneway（F-38 修正：R-14"回调不阻塞状态机"要求，慢监听者不得阻塞 taskHandler 派发线程）
};
```

#### 5.2.1 创建者进程（AI 助手自身）纳管边界（评审答复 2026-08-24）

**问题**：发起任务的 AI 语音助手进程自身是否应自动加入任务纳管集合（共享管控档位/豁免）？

**结论：默认不自动纳管，但保留显式自注册通道。**

不自动纳管的理由：

1. **控制面/被控面分离**：助手是任务的控制面（CreateTask/SetTaskState/DestroyTask 均由其发起）。若纳入被控集合会引入自指——豁免的施加与撤销依赖被豁免者自身的调用行为；
2. **进程级纳管的爆炸半径**（F-3 原则）：进程加入任务后其**全部组件活动**共享管控档位。助手进程承载的并非单个任务（还包含其他并发任务、语音会话、卡片等无关功能），纳管助手 = 无差别豁免整个进程；
3. **防滥用模型被削弱**（R-22）：60min 豁免上限按任务计。若助手自动进入其创建的每个任务，只要保持 ≥1 个活跃任务即获得事实上的永久豁免，上限机制失效；
4. **超时兜底覆盖资源面，不覆盖状态面（F-43 修正，第十七轮）**：若助手冻结/死亡导致状态机无法推进，30min 交互超时/60min 豁免上限使**豁免被撤销**（F-5 两段式/R-21，资源不死锁）；但**任务状态不自动流转**（ADR-6：状态机仅创建者驱动；F-5/R-21 的超时处置仅降档+告警，不改状态）——任务滞留激活态（如 INPUT_REQUIRED），**占用创建者活跃任务配额（R-2）**。原表述"任务自动流转到超时终态"与 ADR-6/F-5/R-21 矛盾，已修正。滞留任务处置两方案（**A-9，待用户对齐**）：(a) ADR-6 增补窄口径系统流转——超时降档时任务置 FAILED(reason=timeout)、创建者进程死亡时置 CANCELED（AppMS 经既有 AppDeathRecipient 链可知创建者死亡，app_death_recipient.h）；(b) 维持"仅创建者驱动"，接受滞留（创建者重启后可自行 Cancel/Destroy 恢复配额；创建者永久死亡则配额滞留至 foundation 重启）。推荐 (a)——否则 16 个滞留任务即配额死锁且无人可解。

显式自注册通道（B-2 校验规则的显式化推论；通道鉴权规则②，F-8 补全）：

- B-2 允许 `callerTokenId = creatorTokenId` 通过校验，因此**助手可按需显式调用 `RegisterTaskProcess(taskId, ownPid, creatorTokenId)` 将自身纳入任务**（占用该任务单任务纳管进程数上限名额，默认 16 可配，R-22）；直调时通道鉴权规则②③适用：入参 callerTokenId 须等于助手自身 IPC tokenId（不可代报他人身份）、ownPid 归属 uid 须与助手一致；
- 是否使用由助手侧自评估决定，系统不做无条件纳管；
- 纳管后助手进程在该任务生命周期内享受与任务进程同等的档位保障，任务结束时随进程组一并撤销。

二期可选增强（若真机验证发现多轮长时任务中助手退后台被冻结导致交互链路停摆）：

- 引入 **supervisor 弱保障档位**：process_schedule 层单独维护"创建者 pid → 其活跃任务最高档（设上限，如 ≤2）"映射，仅在助手有活跃任务期间生效；不进入任何任务的 `processPids`、不改变按任务计的上限模型、任务结束即失效。

#### 5.2.2 ITaskManager 在 SA 501 上的承载机制（F-21 补全，第十四轮勘察）

**问题**：AppMgrService（SA 501）的 IPC 入口 `AppMgrStub::OnRemoteRequest` 校验接口描述符（app_mgr_stub.cpp:57-62）——非 IAppMgr 描述符的请求直接返回 ERR_INVALID_STATE。ITaskManager 代理发出的请求（描述符为 ITaskManager）**无法经既有桩到达 TaskManagerService**，承载机制必须显式设计。

**方案（描述符路由）**：

1. **服务端**：AppMgrService 覆写 `OnRemoteRequest`——读取接口令牌，命中 `ITaskManager::GetDescriptor()` 时委托 TaskManagerService 桩的内部分发入口（跳过重复令牌校验；或缓存令牌后 RewindRead 再走标准桩入口），否则回落 AppMgrStub 既有分发（IAppMgr 行为零改动，符合"不改既有 IPC 序列化"约束）；
2. **客户端**：TaskManagerClient 经 `SystemAbilityManagerClient::GetSystemAbility(APP_MGR_SERVICE_ID)` 取 501 远端对象后 `iface_cast<ITaskManager>`（与既有 AMS→AppMS 取代理方式一致，app_service_manager.cpp:29-33 先例）；**AMS 侧全部调用经 IN_PROCESS_CALL 包装**（身份重置，F-29，§5.3 通道鉴权规则 0）；
3. **否决备选**：同进程新 SA ID（183/184/185 同 foundation 进程先例）——需 SA ID 审批 + sa_profile 新增，与 D-1"不新增 SA"决策冲突，不采纳；扩 IAppMgr 接口——违反既定约束，不采纳。

> 该机制为 TASK-0 骨架的一部分（AppMgrService 挂点 + 路由代码），TASK-0 完成判据相应包含"ITaskManager 代理经 501 可达 TaskManagerService（最小 IPC 用例）"。

### 5.3 防仿冒设计落地（B-1+B-2）

1. **taskId 生成**（B-1）：`CreateTask` 时以随机数源（`RAND_bytes` 或 `std::random_device`+mt19937）生成 16 字节，格式化为 UUID v4 风格字符串（`xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx`），≥128bit 熵，不可猜测（R-1）。
2. **caller 身份校验**（B-2）：
   - 任务创建时绑定 `creatorTokenId/creatorUid/userId`（TaskRecord）；
   - 拦截点两段式上报携带**发起方 tokenId**（AMS 拦截点取 `IPCSkeleton::GetCallingTokenID()`）；
   - AppMS 校验（权威）：`callerTokenId ∈ {creatorTokenId} ∪ {该任务纳管进程的 tokenId 集合}` 才接受纳管（R-8）；
   - 校验失败：拒绝纳管、**不阻断启动**、不上报错误给调用方（防探测）；
   - 链式启动：任务内进程 A 启动 B → callerTokenId = A 的 tokenId ∈ 纳管集合 → 合法（与 ADK supervisor→sub-agent 可信链一致）。
   - **通道鉴权（F-8 补全，第十三轮；F-29 修正，第十五轮）**：callerTokenId 为 IPC 入参而非通道派生值，ITaskManager 为可直调的 IPC 接口——若不校验通道身份，任意系统应用可自报 callerTokenId=creatorTokenId 绕过 B-2（§5.3.1 ⑧"恶意应用无法直接触达内部通道"的断言必须由显式校验支撑）。规则：
     0. **调用方式前提（F-29，第十五轮实证）**：AMS 拦截点/attach 点对 TaskManagerClient 的全部调用必须经 **IN_PROCESS_CALL（ResetCallingIdentity）包装**——同进程 SA 代理调用为进程内直接分发，被调方看到的 calling identity **透传原始 IPC 调用方**而非 native（机制实证：`services/common/include/in_process_call_wrapper.h`；AMS→AppMS/agentmgr 既有调用全部如此包装，先例 app_scheduler.cpp:82、ability_manager_service.cpp:6092）。**不包装的后果**：规则①永不命中（actual=原始调用方 HAP token），规则②对 attach 路径（param=①登记的业务发起方 ≠ actual=目标进程 token）与代理启动路径（param=specifyTokenId ≠ actual=发起 app token）误拒——两条核心纳管链路断裂；
     1. IPC 调用方为 **foundation 进程 native 身份** → 信任入参 callerTokenId（AMS 转发的业务发起方）。判定原语 = `PermissionVerification::CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS_NAME)`（IsSACall + native token processName=="foundation"；agentmgr 鉴权 AMS 同进程调用的同一先例：agent_manager_service.cpp:993、permission_verification.cpp:115-131）。**不可用裸 IsSACall**——任何进程的 native SA（如其他系统进程）直调均可过裸 native 判定，伪造入参即绕过；信任边界 = foundation 进程内互信（与 agentmgr 先例一致）；
     2. 其他调用方（含创建者自注册，§5.2.1）→ **入参 callerTokenId 必须等于 IPC 实际调用方 tokenId**（杜绝自报他人身份），再过 B-2 集合校验；
     3. **pid 归属校验**（直调路径强化，②⑦既有 pid 归属校验的直调版）：直调注册的 pid 归属 uid 须与调用方一致，或该 pid 已在目标任务内（防创建者/任务内进程注册**他人进程**骗取豁免）；
     4. 不满足 → 返回 ERR_TASK_PERMISSION_DENIED（内部通道错误码，不透传应用侧，C-5）。
3. **B-3 可选增强**（预留）：若后续需要防重放/防泄露增强，增加短时 token 换 taskId 通道，接口与内存登记已预留扩展点。

#### 5.3.1 两段式纳管安全分析（评审答复 2026-08-21 第五轮）

对"拦截点两段式（意向 + attach 纳管）是否有仿冒风险"的逐面分析：

| 仿冒面 | 分析 | 结论 |
|--------|------|------|
| ① 恶意应用直接调 RegisterTaskIntent/RegisterTaskProcess 伪造 taskId | **两阶段均执行 B-2 caller 校验**：callerTokenId 必须 ∈ {creatorTokenId} ∪ {该任务纳管进程 tokenId 集合}；恶意应用非创建者/非任务内进程 → 拒绝。校验失败不阻断启动、不上报错误（R-8 防探测，调用方无法区分"任务不存在"与"无权限"） | 无新增仿冒面 |
| ② attach 阶段 pid 篡改/误绑 | pid 由 AMS attach 回调（**AttachAbilityThread 优先**；OnAbilityRequestDone 在每次状态迁移均触发，仅作兜底）提供，**非调用方自报**；强化：AppMS 侧校验 pid 存活且 pid 归属 uid/tokenId 与启动目标一致（防 AMS 层异常/中间人） | 无仿冒面；补 pid 归属校验 |
| ③ 任务内进程链式纳管（任务内进程启动新进程并携带 taskId） | 属 B-2 接受的信任语义（创建者可信链，对齐 ADK supervisor→sub-agent）；滥用面受**单任务纳管进程数上限**（默认 16 可配）与**全局活跃豁免数量上限**（默认 64）及豁免时长上限约束（均归 R-22），且 TASK_RSSCONTROL/TASK_ABUSE_WARNING 全程审计 | 接受，上限+审计约束 |
| ④ AMS 拦截点覆盖不全（绕过拦截直接启动） | 所有启动链路收敛于 AMS 统一入口（StartAbilityInner/ConnectAbility 收敛点），拦截点置于收敛处；execution-plan TASK-3 以四链路（Start/Connect/意图/Cli）集成用例验证覆盖 | 收敛点覆盖保证 |
| ⑤ taskId 猜测 | B-1：128bit 随机熵，不可猜测 | 无风险 |
| ⑥ 代理启动（StartAbilityAsCaller/WithSpecifyTokenId）绕过校验 | 拦截点校验身份取**业务发起方**（asCallerSourceToken/specifyTokenId 优先，非 AMS 内部调用者）——代理启动时被代理身份才是 B-2 校验对象；系统服务内部转发（无业务发起方）视为无身份 → 不纳管 | 无仿冒面（补充：校验身份规则明确） |
| ⑦ 已存在进程直接纳管（条件两段式）的身份一致性 | 直接注册路径同样携带业务发起方 tokenId 过 B-2 校验（与意向路径同一校验函数），AppMS 校验 pid 存活且归属与目标一致 | 无仿冒面 |
| ⑧ attach 补注册通道的身份伪造（恶意进程冒充 attach 上报他人 taskId/pid） | **代码勘察发现（P2，2026-08-24）**：attach 挂点 `AttachAbilityThread`（ability_manager_service.cpp:7957）由**目标进程自身经 IPC 调起**——attach 现场 `IPCSkeleton::GetCallingTokenID()` 恒为目标进程 tokenId，若直接取用则恒被 B-2 拒绝，且构成"目标进程自报身份"的仿冒面。**修正**：②的 callerTokenId 必须取**①意向登记时随 want 解析并存入 AbilityRecord 的业务发起方 tokenId**（非 attach 现场取值）；信任锚 = AMS→AppMS 为 foundation 进程内调用（native token）——**该信任锚由 AppMS 侧通道鉴权显式校验**（F-8，§5.3 通道鉴权规则①：native 调用方才信任入参身份；非 native 直调一律"入参身份=IPC 实际身份"，恶意应用自报他人身份必被拒） | 无仿冒面（需落实：attach 身份来源 = ①登记值 + 通道鉴权） |

**结论**：两段式（修正为条件两段式）本身不引入新增仿冒面——仿冒防护完全由 B-2 caller 校验承担，且在意向/直接注册路径都执行；设计强化点 = ②pid 归属校验 + ③进程集合上限 + ⑥代理启动身份规则 + ⑧attach 身份来源（取①登记的业务发起方 tokenId，非 attach 现场 GetCallingTokenID）+ **通道鉴权**（F-8：ITaskManager 可被系统应用直调，AppMS 必须校验 IPC 通道身份——native 信任入参、非 native 强制入参身份=IPC 实际身份+pid 归属校验）（均已在 5.3/5.4/5.6/5.7/R-22 覆盖，落地时落实）。

### 5.4 AMS 拦截点设计（启动链路场景全覆盖 + 条件两段式）

#### 5.4.1 启动链路场景全覆盖（评审答复 2026-08-21 第六轮）

| # | 场景 | 收敛点（全部经核实的 file:line） | 拦截设计 | 覆盖 |
|---|------|-------------------------------|----------|------|
| 1 | StartAbility（前台/后台 want 启动） | `StartAbilityInner`（ability_manager_service.cpp:1360 定义；调用点 :807/:876/:928/:1013/:1130/:1195/:2372/:14183/:14777/:15241/:16359/:18349/:19367 共 13 处，另 implicit_start_processor.cpp:820、preload_manager_service.cpp:111） | 拦截点①：Want 参数解析于 StartAbilityInner 入口；分支判定（目标 record pid 可得性）于获得目标 AbilityRecord 后（F-11 精确化，详见 §5.4.2） | ✅ |
| 2 | 免安装启动 StartAbilityByFreeInstall | :876 收敛 StartAbilityInner | 同上（天然覆盖） | ✅ |
| 3 | 指定身份启动 StartAbilityWithSpecifyTokenId | :895（→StartAbilityWithSpecifyTokenIdInner）/:1013 收敛 StartAbilityInner（specifyTokenId 传入） | 同上；**校验身份 = specifyTokenId 而非 GetCallingTokenID**（业务发起方） | ✅ |
| 4 | 以调用方身份启动 StartAbilityAsCaller / ImplicitStartAbilityAsCaller | :1145/:1156（→StartAbilityAsCallerDetails）/:1195 收敛（asCallerSourceToken 传入） | 同上；**校验身份 = asCallerSourceToken**（业务发起方） | ✅ |
| 5 | ConnectAbility / ConnectAbilityWithExtensionType | `ConnectAbilityCommon`（ability_manager_service.cpp:6063；ConnectAbilityWithExtensionType 为客户端方法 ability_manager_client.cpp:686，服务端收敛于 ConnectAbilityCommon；含 agentmgr 转发路径 VerifyAgentConnectRequest 先例 ~:6094） | 拦截点①在 ConnectAbilityCommon 解析；②在 ServiceRecord attach 处补注册 | ✅ |
| 6 | StartServiceExtensionAbility / StartUIExtensionAbility / RequestModalUIExtension | **三独立路径，均不经 StartAbilityInner（F-22 修正，第十四轮勘察）**：① StartExtensionAbility/ImplicitStartExtensionAbility（:4338/:4357）→ `StartExtensionAbilityInner`（:4656，StartAbilityInner 亦在 :1404/:2437 内部转调它）；② `StartUIExtensionAbility`（:4989，Want 在 SessionInfo 内，经 UIExtensionAbilityManager）；③ `RequestModalUIExtension`（:4475）→ `RequestModalUIExtensionInner`（:4508） | 拦截点①分别置于 StartExtensionAbilityInner / StartUIExtensionAbility / RequestModalUIExtensionInner 三处（与 StartAbilityInner 共用同一解析/上报辅助函数）。**身份解析来源（F-32 补全，第十五轮勘察）**：①有 callerToken 参数（正常解析）；② StartUIExtensionAbility **无 callerToken 参数**且 IPC 调用方为 sceneboard——须取 `SessionInfo.callerToken_/callingTokenId_`（ws_common.h:464/:488）为业务发起方，直接取 GetCallingTokenID 会得 sceneboard 身份被 B-2 必拒；③由应用侧框架直调 AMS（js_service_extension_context.cpp:187），GetCallingTokenID=应用自身 ✓ | ✅（修正后） |
| 7 | 意图框架 InsightIntent | `StartAbilityWithInsightIntent`（:15241）收敛 StartAbilityInner（意图执行器最终调用 AMS StartAbility/ConnectAbility 既有接口） | 天然覆盖（:15241 为意图链路实证收敛点） | ✅ |
| 8 | Cli（aa 命令） | `tools/aa` 经 AbilityManagerClient::StartAbility（ability_command.cpp:486/:491）→ 收敛 StartAbilityInner | 天然覆盖；shell 身份带 taskId 时按 B-2 校验（非创建者 → 拒） | ✅ |
| 9 | A2A（Agent 扩展） | A2A 服务端为 AgentExtensionAbility，经 agentmgr→AMS ConnectAbilityWithExtensionType（服务端收敛 ConnectAbilityCommon，场景 5）；taskId 由 supervisor 参数透传 | 覆盖（场景 5 链路） | ✅ |
| 10 | 后台拉起（WorkScheduler/BackgroundTask 拉起进程） | 非 AI 助手启动链，无 taskId 透传 | **不纳管**（设计原则：纳管只发生在带合法 taskId 的启动链） | 原则 |
| 11 | 卡片/表单启动、系统恢复启动 | 无 taskId | 不纳管（R-13 零影响）；注：RestartApp（:16359）亦收敛 StartAbilityInner，恢复启动无 taskId 不纳管 | 原则 |
| 12 | 任务内进程派生子进程（ChildProcess） | 子进程经 appmgr ChildProcessRecord 创建（非 AMS 链路） | **扩展场景**：appmgr 侧挂点——父进程在任务内则子进程自动纳入任务（复用同一挂点链）；一期最小实现或开放项（记录待确认） | 扩展 |

> 原则（F-22 修正）：拦截点①置于**全部启动/连接收敛处**——`StartAbilityInner`（:1360）+ `StartExtensionAbilityInner`（:4656）+ `StartUIExtensionAbility`（:4989）+ `RequestModalUIExtensionInner`（:4508）+ `ConnectAbilityCommon`（:6063）**共 5 处**，共用同一 Want 解析/身份判定/上报辅助函数；"任何新启动变体只要收敛于此即自动覆盖"仅对上述 5 收敛点成立，新增独立入口（如未来新的扩展类型入口）需评估补点。拦截仅做参数解析与身份判定，失败不阻断启动（R-8/R-13）。

#### 5.4.1a 拦截器框架方案分析（F-36 补全，第十六轮勘察）

**勘察事实**：AMS 已有规范拦截器框架（`AbilityInterceptorExecuter` + `IAbilityInterceptor`，CLAUDE.md 列"Adding a New Interceptor"为标准扩展程序），且在 5 收敛点中的 **4 处已运行**：StartAbilityInner（:1490，另有 afterCheckExecuter_ :1705 后置段）、StartExtensionAbilityInner（:4712）、StartUIExtensionAbility（:5100）、ConnectAbilityCommon（:6135，fromConnect=true）；仅 RequestModalUIExtension（:4508）无拦截器。本设计原方案（ad-hoc 辅助函数插入 5 处）**未分析该框架选项**——此为本轮修正的方案分析缺口。

**两方案对比**：

| 维度 | 方案 A：TaskInterceptor（走拦截器框架） | 方案 B：ad-hoc 辅助函数（原方案） |
|------|----------------------------------------|-----------------------------------|
| 侵入性 | 4/5 收敛点零改动（AddInterceptor 注册即可） | 5 处插入调用代码 |
| 架构一致性 | AMS 规范扩展机制（CLAUDE.md 标准程序） | 自建挂点，新变体需逐点评估 |
| 身份获取 | **摩擦**：全部拦截器调用点 callerToken=nullptr（:1489/:4712/:5100/:6135 实证）——需扩展 AbilityInterceptorParam（触碰全部拦截器共享结构）或拦截器内另行取身份（函数参数/SessionInfo，F-32） | 直接取函数参数（callerToken/specifyTokenId/asCallerSourceToken） |
| 时序 | 拦截器在 record 解析**前**执行——"既有 record 直接纳管"分支需拦截器内自查 record 管理器 | 可置于 record 解析后 |
| 失败语义 | 拦截器返回非 ERR_OK **阻断启动**——任务拦截器必须恒返 ERR_OK（观察者模式，与 R-8/R-13 一致，可控） | 天然不阻断 |
| 覆盖 | RequestModalUIExtension 无拦截器，仍需 ad-hoc 挂点（或推动框架补点） | 全覆盖 |

**结论（推荐混合方案，待评审确认——列入未对齐清单 A-8）**：**方案 A 为主 + B 补角**——新建 `TaskInterceptor`（`IAbilityInterceptor` 实现，恒返 ERR_OK）经 `AddInterceptor("Task", …)` 注册，承载 4 个已覆盖收敛点的 Want 解析 + 意向登记（冷启动路径①）；身份捕获在拦截器内按来源取（StartAbility 路径取 StartAbilityWrapParam 不可得——**需扩展 AbilityInterceptorParam 增加 callerTokenId 字段**（低风险：纯增字段，既有拦截器不受影响）或复用 Want 内 AMS 保留参数机制）；"既有 record 直接纳管"分支由拦截器自查 record 管理器判定（**或挂 afterCheckExecuter_ 后置段——abilityInfo 已解析处 :1705/:1922/:2234，F-46 补充**）；RequestModalUIExtension 单点 ad-hoc 挂同一辅助函数。若评审否决扩展 AbilityInterceptorParam，则回退方案 B（原 5 点 ad-hoc）。

#### 5.4.2 条件两段式拦截逻辑（修正：覆盖"已存在进程"场景）

**问题**（评审指出）：原两段式的②挂于"进程 attach"（AttachAbilityThread/OnAbilityRequestDone）——该回调**仅在进程新建时发生**；若启动目标进程已存在（如复用的 ServiceRecord、已运行的扩展进程、预加载进程），②不会触发 → 已存在进程无法被纳管。

**修正：条件两段式**——拦截点①在获得目标 AbilityRecord 后判断 pid 可得性：
```
taskId = want.GetStringParam("ohos.ai.taskId")              // 无 → 短路（零开销 R-13）
callerTokenId = 业务发起方身份（specifyTokenId / asCallerSourceToken 优先，
                 否则 callerToken 解析，兜底 IPCSkeleton::GetCallingTokenID()）
targetPid = abilityRecord ? abilityRecord->GetPid() : 0      // 目标 record 已关联进程时可得
// 以下全部 TaskManagerClient 调用经 IN_PROCESS_CALL 包装（身份重置，F-29）
if (targetPid > 0) {
    TaskManagerClient::RegisterTaskProcess(taskId, targetPid, callerTokenId)   // 进程已存在 → 直接纳管
} else {
    TaskManagerClient::RegisterTaskIntent(taskId, callerTokenId)               // 进程未创建 → 意向登记
    // attach 时（AttachAbilityThread 优先，OnAbilityRequestDone 仅兜底；pid 就绪）：
    //   callerTokenId 必须取①意向登记时存入 AbilityRecord 的业务发起方 tokenId——
    //   不能取 attach 现场值（GetCallingTokenID() = 目标进程自身 tokenId，B-2 必拒，见 §5.3.1 ⑧）
    TaskManagerClient::RegisterTaskProcess(taskId, pid, callerTokenId /* =①登记值 */)  // 补注册
}
```
- 意向超时清理：30s 未补注册则移除（attach 晚于超时由 attach 处**直接补发②**——pid 已就绪，身份取 AbilityRecord 存储值（F-31），重发①无意义，F-39 修正）；**attach 失败清理**（F-7 修正）：进程启动失败（attach 失败回调）时主动 UnregisterTaskIntent，避免意向残留 30s；
- 进程已存在直接纳管时无意向阶段（简化登记：一次动作）；
- **分支①适用边界（F-11 勘察补强，第十三轮）**：`AbilityRecord` 的 pid **仅在 attach 时设置**（`SetScheduler` 内 `SetPid(IPCSkeleton::GetCallingPid())`，ability_record.cpp:1096——目标进程自报；全库无其他 SetPid 路径），因此"直接纳管"分支仅对**既有 record 且已 attach**（运行中 Service/单例/扩展复用等场景）成立；**进程已存在但目标 ability record 为新建**（热启动新实例）时 record 尚无 pid → 走意向路径，attach（该实例 SetScheduler）时补注册——两路径全覆盖，无遗漏；
- `AbilityRecord::GetPid()` 可得性已核实（ability_connect_manager.cpp:327 既有使用）；连接场景对已运行服务同理（ServiceRecord 持有 pid，:791）。
- **attach 身份来源（P2 修正，代码勘察 2026-08-24）**：`AttachAbilityThread`（ability_manager_service.cpp:7957）由**目标进程自身经 IPC 调起**，现场 `GetCallingTokenID()` 恒为目标进程 tokenId——直接取用不仅恒被 B-2 拒绝，还构成自报身份仿冒面（§5.3.1 ⑧）。因此②的 callerTokenId 必须取①意向登记时解析并存入 AbilityRecord 的业务发起方 tokenId；信任锚 = AMS→AppMS 为 foundation 进程内调用（经 IN_PROCESS_CALL 身份重置后 AppMS 侧见 native token，F-29），恶意应用无法直接触达该内部通道。该来源同时正确覆盖"创建者直启目标进程"场景（发起方 = 创建者，B-2 通过，§5.2.1）。
- **AbilityRecord 存储缺口（F-31 补全，第十五轮勘察）**：`AbilityRecord` 现无"业务发起方 tokenId"字段（ability_record.h 仅有 want_ 与 CallerRecord 列表）——①意向登记时须**新增字段**（如 `taskOriginatorTokenId_`，随 want 解析写入）或 AMS 侧维护 taskId→originator 侧表；该修改属 TASK-3 文件范围（ability_record.h/cpp，execution-plan 已同步）。

> 调用经 `interfaces/inner_api/app_manager/` 客户端（既有 AMS→AppMS 通道体系），符合 AGENTS.md；所有调用失败仅日志告警，不阻断启动（R-7/R-8/R-13）。

### 5.5 监听与交互机制（D-21 重写，2026-08-26 需求变更）

- **监听（保留，消费者重定位）**：`TaskListenerManager` 维护注册表（IRemoteObject 序列化后为 key）；状态变更在 taskHandler 线程派发（回调顺序与状态变更顺序一致，R-14）；监听者死亡自动移除（DeathRecipient）。**监听者 = 任意已注册 ITaskListener 的系统应用**——典型消费方为**安全管控、审计、系统状态监控等系统模块**（状态感知：任务何时进入交互态/终态、归属进程变化）；执行进度展示**不再依赖本通道**（由 AI 助手自有 AI bar 呈现，助手即创建者、数据在己方）。
- **交互承载（ADR-7/D-21：AI bar 内化）**：
  - 进入 INPUT_REQUIRED/AUTH_REQUIRED：创建者 `SetTaskState` 携带 payload `{"type":"input"|"auth","text":"<描述>","options":["<可选项>"]}`（≤4KB，R-16）——payload 留存 TaskRecord（**事实记录**：交互请求的内容快照，供 AI bar 呈现、GetTaskInfo 查询与审计）；监听者经状态回调感知交互态（含 payload，安全模块可见交互发生与内容）；
  - **人机交互在助手进程内闭环**：AI bar（助手自有 UI）呈现描述与可选项 → 用户选择/输入 → 助手直接获得响应（进程内/自有通道）→ 助手 `SetTaskState` 流转出交互态（WORKING/COMPLETED 等）；**交互响应可选取该次流转的 payload 携带**（可选，无专用机制、无专用存储字段）；
  - ~~SubmitTaskInput 回传通道 / INPUT_ARRIVED 通知 / input 字段 / 白名单传递（D-11，2026-08-24）~~——**随 D-21 移除**（外部传递通道无消费方：进度展示与交互均由 AI bar 承担）；
  - 交互态任务被取消/销毁：payload 清除、不再回调（R-18）。
- **通知类事件（B-5 补全；D-21 后余 3 类）**：交互超时限期处置（F-5）、限时豁免到期预警（C-2）等非状态变更通知经 `OnTaskNotice(taskId, noticeType, payload, timestamp)` 回调派发（与状态回调同一监听注册、同一派发线程、同受 userId 过滤）；noticeType 枚举：INTERACTIVE_TIMEOUT / EXEMPT_EXPIRING / EXEMPT_EXPIRED（~~INPUT_ARRIVED~~/~~PAYLOAD_UPDATED~~ 随 D-21 移除——前者无外部回传方，后者面向外部进度展示无消费方）。
- **payload 内容约定（C-4，评审决策 2026-08-24：权限管控）**：payload 为任务业务描述；敏感内容可见性经**权限管控**约束——RegisterTaskListener 校验系统应用身份，仅授权系统应用可接收任务数据（含回调 payload 与 GetTaskInfo 查询）；模块不做内容过滤。

### 5.6 并发与多任务归属（需求 10/11）

- 双向索引（`taskToPids_`/`pidToTasks_`，5.1）；单任务记录 ≤1KB；
- 档位合并：`effectiveLevel(pid) = max(其任务集合中激活任务的档位)`（ADR-5，R-10）；
- 档位变化 → `ApplyLevel` 增量施加/撤销 RSS 动作（同 pid 同类型豁免幂等——幂等引用计数按 pid 维护于 process_schedule 档位状态，幂等键 taskId+pid+豁免类型，R-11）；
- 任务结束/降级 → 重算剩余任务档位并撤销（R-9/R-20）；
- 并发上限：创建者活跃任务 ≤ 16（R-2）；全局活跃豁免进程数上限（默认 64）与**单任务纳管进程数上限（默认 16）**（R-22，A-5 补全：单任务上限约束链式纳管膨胀；**两项上限经可配参数支持调整**，评审决策 2026-08-24）；豁免时长上限见 R-21。
- **进程级纳管原则**（F-3 修正，评审第七轮）：纳管以**进程**为单位——进程加入任务后，其内全部组件活动（新建 ability/连接等）共享该任务的管控档位，无需组件级单独纳管；taskId 仅用于"进程加入任务"的触发点，同进程内后续组件启动无需携带 taskId（也不改变归属）。同进程组件在任务内的活动天然受管控（进程被豁免 → 其内组件活动不被冻结）。
- **优先级动态调整联动**（F-6 修正）：SetTaskParams 提升优先级且任务处于激活态时，触发 OnTaskActiveChanged → process_schedule 档位重算，管控立即按新档位生效（与状态变更同一联动路径）。
- **SetTaskParams 受限规则**（B-4 补全）：仅创建者可调；任务处于非终态（终态返回 ERR_TASK_INVALID_STATE）；可调字段 = priority / payload（type 创建后不可调）；priority 变化经上述联动路径重算档位（升档立即施加、降档立即撤销）；payload 变化仅更新 TaskRecord（~~PAYLOAD_UPDATED 通知~~随 D-21 移除——进度展示由 AI bar 自呈现，助手即数据源，无需回调通知）；激活态任务且 **priority 发生变化**时触发豁免时长**续期**（payload-only 变更不续期，防滥用续期；计时/续期计数为 TaskRecord 内存态，§6.1/R-21，评审决策 2026-08-24）。

### 5.7 权限与身份（基础鉴权）

| 入口 | 校验 |
|------|------|
| CreateTask/DestroyTask/CancelTask/SetTaskState/SetTaskParams | 系统应用身份（tokenId 属系统应用/SA 白名单）；除 CreateTask 外再校验创建者（R-6） |
| GetTaskInfo(s)/RegisterTaskListener | 系统应用身份（R-15；监听典型消费方=安全管控/审计/系统状态监控等系统模块，D-21） |
| RegisterTaskIntent/RegisterTaskProcess | B-2 caller 校验（5.3）+ **通道鉴权**（F-8：native 调用方信任入参身份；非 native 调用方入参 callerTokenId 须等于 IPC 实际身份 + pid 归属校验） |
| 多用户 | 任务归属 userId；**GetTaskInfos 按调用方 userId 过滤、跨用户 taskId 的 GetTaskInfo 返回 ERR_TASK_NOT_FOUND、OnTaskStateChanged/OnTaskNotice 仅派发给同 userId 监听者**（C-3 落地，R-14/R-15） |

> 系统应用白名单复用 AppMS 既有校验能力（`PermissionVerification` 或 token 校验工具），不新增权限项。

## 资源调度子模块设计（RSS 管控细化）

### 6.1 管控档位与 RSS 动作映射

| 档位 | 语义 | 一期管控动作 | 二期扩展动作 |
|------|------|---------------|--------------|
| 0 | 不豁免（UNKNOWN/终态任务） | 无 | — |
| 1 | 限时豁免（MEDIUM 激活态） | 豁免（EXEMPT，限时，默认 30min 可配） | — |
| 2 | 持续豁免（HIGH 激活态；受时长硬上限约束，可经 SetTaskParams 限次续期——A-6 修正） | 豁免（EXEMPT）+ 必要时解冻（THAW） | — |
| 3 | 豁免 + 提权（CRITICAL） | 豁免 + 解冻 | 进程级 QoS 提权（RSS 221）；线程级 QoS（QosController/RTG，需线程 tid） |

**激活态集合**（判定"档位≥1"生效的任务状态）：{SUBMITTED, WORKING, INPUT_REQUIRED, AUTH_REQUIRED}。说明：① SUBMITTED 纳入激活态（F-1 修正）——任务提交后若被冻结（如锁屏定时任务就绪），将无法推进到 WORKING，必须在就绪态即保障可调度；② 交互态（INPUT/AUTH）属激活态（F-5 已含）；③ UNKNOWN（已创建未提交）非激活。

**交互态时长管理**（F-5）：交互态任务维持豁免，但配置交互超时（可配，默认 30min）——超时由 task_mgr 触发策略动作，避免用户长时间不响应导致豁免无限占用。

**交互态超时策略动作（F-5 细化，评审答复 2026-08-24 第八轮）**：超时后执行两段式处置——
1. **限期处置通知**：task_mgr 向任务创建者上报 TASK 事件（含超时信息），给予短宽限期（可配，默认 5min）供助手重设状态（回 WORKING、转 FAILED/CANCELED 或续期交互）；
2. **宽限后仍未流转 → task_mgr 主动降档**：撤销该任务对纳管进程的豁免（档位按剩余归属任务 max 重算），上报 TASK_ABUSE_WARNING 告警；后续用户若响应、助手 SetTaskState 回 WORKING 时，经 §6.1.1-4 的 P3 分支重新施加（EXEMPT→507→THAW→复查）。
3. 兜底关系：本策略与 R-22 豁免时长硬上限（默认 60min）叠加生效——超时降档通常先于硬上限触发；硬上限仍是防滥用的最终兜底。
4. 明确排除"超时后保持豁免仅上报告警"路径（该路径与"避免豁免无限占用"目标矛盾，已废弃）。
5. **通知载体（B-5 补全）**：第 1 步"限期处置通知"经 `OnTaskNotice(taskId, INTERACTIVE_TIMEOUT, {deadline})` 回调派发给已注册监听者（创建者注册监听即可实时感知宽限窗口；未注册时可经 GetTaskInfo 轮询，或依赖第 2 步降档兜底），同时上报 hisysevent TASK_ABUSE_WARNING(reason=timeout_demote)。

**限时豁免到期与续期（C-2/A-6 补全，评审答复 2026-08-24 第九轮）**：

- 档位 1（MEDIUM）限时豁免到期前经 `OnTaskNotice(EXEMPT_EXPIRING)` 预警（提前量可配，默认 5min）；到期未续期 → 撤销豁免、档位按剩余归属任务重算、TASK_ABUSE_WARNING(reason=limit)；
- **续期机制（全档位）**：豁免时长硬上限（默认 60min，R-21/R-22）到期前，创建者可经 SetTaskParams 续期（**仅 priority 发生变化时触发续期**，payload-only 变更不续期——防滥用续期；计时/续期计数为 TaskRecord 内存态）；档位 2 语义由"全时"修正为"持续（激活期间、可续期）"——长时 HIGH 任务经周期性续期维持豁免，**续期次数上限 N 次（可配，默认 3 次，评审决策 2026-08-24）**，达到上限后到期即撤销豁免并降档、TASK_ABUSE_WARNING(reason=limit)；每次续期动作经 TASK_RSSCONTROL 记录审计；
- **双计时器叠加**：交互态任务同时存在交互超时（30min+5min 宽限）与豁免时长计时（档位 1 为 30min 限时）时，取先到者触发，先触发者的处置（降档/撤销豁免）即终止另一计时，避免双重降档。**上述超时相关参数（交互 30min/宽限 5min/档位 1 限时 30min/硬上限 60min/到期预警提前量 5min/续期次数 N=3）默认值已确认**（评审决策 2026-08-24 提出、2026-08-26 第十三轮用户确认），落地时全部经可配参数支持调整。
- **参数落地机制（F-14 明确，第十三轮；F-37 修正前缀，第十六轮）**：可调参数（上限/时长/续期次数/开关）经本仓 **sysparam 参数体系**落地——`.para` 定义 + `OHOS::system::Get*Parameter` 读取（先例：app_running_manager.cpp:1145）；**命名必须用 `persist.sys.appms.taskmanager.*` 前缀而非 const.**（本仓惯例实证：`const.*` 启动后不可变（appfwk.para:43-49），**运行时可调参数用 `persist.*`**（先例 persist.sys.abilityms.support.ecologicalrulemgrservice 等，appfwk.para:21-25，运行时经 param set 调整、Get*Parameter 实时读取）——"CCM 可调"若含运行时调参（产品调优不重启），const.* 前缀不达目标）；appmgr 无既有 .para 文件，随 TASK-4 新增参数定义文件并接入 BUILD.gn（需求侧"CCM 可调"要求即由该机制承载）。

> **开放项（二期评估）**：交互态降档折中——HIGH/CRITICAL 任务进入交互态后自动降为限时豁免档（交互等待本质为"低活动、需快速唤醒"，与 MEDIUM 语义接近），可压缩用户无响应期间的资源占用；一期不引入（收益不抵"状态×档位"二维映射复杂度）。

内存登记（评审决策 2026-08-24：**不持久化**，仅内存保存**正在运行中任务**的调度/豁免信息）：`{taskId, pid, action, level, timestamp, result}`——单一职责为 **RSS 重启重放（§6.1.1-5）与撤销重试**的运行时依据；AppMS 重启随进程消亡（C-1 结论一致）；撤销/计时/限次等运行时状态以 TaskRecord 为准；**审计、对接验收、问题定位、复核一律经日志查看**（TASK_RSSCONTROL hisysevent + hilog，含完整 ControlRequest 上下文），不依赖登记查询。

### 6.1.1 生效校验环（F-4 修正，评审第七轮）

管控动作施加后**校验是否真实生效**（防 RSS 内部策略拒绝后系统不知、任务仍失败）：
1. 施加 EXEMPT/THAW 后，经 RCP 查询冻结状态（`GetSuspendState(pid)`，对接后实现为 SYNC_RES_TYPE_GET_SUSPEND_STATE_BY_PID 507）；
2. 未生效（进程仍被冻结/策略拒绝）→ 上报 TASK_ABUSE_WARNING 告警 + 向任务创建者侧上报 TASK 事件（供 AI 助手决策重试/降级/取消）；
3. 校验结果记入内存登记（result 字段）并打印日志，供对接验收与问题定位（评审决策 2026-08-24：经日志查看）。
4. **已冻结进程纳管分支（P3，代码勘察 2026-08-24）**：施加管控时目标进程可能已被冻结（典型：SUBMITTED 定时任务就绪前进程已被 RSS 冻结）——ApplyControl 施加顺序固定为：EXEMPT(505) → 507 查询 → 若 `isFrozen=true` → THAW(517) → 507 复查；复查仍未解冻才判定失败（告警 + TASK 事件）。注意：507 在无消费插件的环境恒返回 `isFrozen=false` 默认值（suspend_manager_base_service.cpp:66-72），该分支仅在有 freezer 执行器的产品环境下有实际意义；
5. **RSS 重启状态重放（P4）**：RSS（SA 1901）异常重启会清空其内部豁免状态，而 AppMS 内存登记仍显示"已豁免"（状态漂移）。RCP-Impl 复用 RSS 客户端内置死亡通知（`suspend_manager_base_client` 懒加载代理 + 死亡监听），RSS 重启后**按内存登记幂等重放（replay）全部活跃豁免**，重放失败按 R-20 重试 ≤3 + 告警；二期以 push observer 做运行时漂移检测（§6.4 P5）；
6. **同步调用纪律（P6；F-44 补强，第十七轮）**：507/517/520 均经 `ReportSyncEvent` **同步**上报——RCP 不得在持有 task_mgr/process_schedule 锁时发起同步调用（死锁/长阻塞风险），**且不得在 taskHandler 串行线程上直接发起**（串行线程即全局锁：一次 507 超时阻塞全部任务操作 + post-and-wait 的 binder 线程；F-44——同步调用在 RCP 内部工作线程执行、结果回投 taskHandler，ADR-8）；统一设置超时保护（建议 ≤500ms 预算，落地随 TASK-4 定稿）；状态查询优先走本地 observer 缓存（§6.4 P5）。
7. **待机豁免联动（P7，评审决策 2026-08-24；F-30/F-34 补全，第十五轮实证）**：任务进入激活态（豁免生效）时，ResourceControlProxy 经 `StandbyServiceClient::ApplyAllowResource` 向 device_standby 申请待机资源豁免（网络/任务执行），覆盖夜间深度待机场景（需求场景 1，D-2 关闭）；任务终态/豁免撤销/进程移除时 `UnapplyAllowResource`；与 505/517 同一生命周期内存登记管理，避免待机豁免残留。**落地要点（实证）**：① ResourceRequest 构造字段 = {allowType(位掩码), uid, name, duration, reason, reasonCode=REASON_NATIVE_API}（resource_request.h）；② **AllowType 含 FREEZE 位**（allow_type.h:25-34：NETWORK/RUNNING_LOCK/TIMER/WORK_SCHEDULER/AUTO_SYNC/PUSH/FREEZE）——待机**冻结**豁免可经 EDS 申请，与 O-2（锁屏冻结边界）直接相关，P7 是否一并申请 FREEZE 位列入 device_standby 对齐评估；③ 权限无阻碍：native 调用方免权限（standby_service_impl.cpp:626-636 CheckCallerPermission 仅 HAP 需 DEVICE_STANDBY_EXEMPTION）；④ **新增依赖**：appmgr BUILD.gn 须新增 `device_standby:standby_innerkits`（现有依赖无 device_standby，execution-plan 已同步）。

> **AppMS 重启反向场景（C-1 分析，结论：无需处理）**：P4 只覆盖"RSS 重启、AppMS 登记漂移"；反向的"AppMS 重启、RSS 侧豁免残留"不存在——AppMS 位于 foundation 进程，异常重启即 foundation 进程重启，系统将清理全部应用进程，纳管进程与 RSS 侧豁免随进程消亡同亡，无孤儿豁免（评审结论 2026-08-24）。内存登记同为进程内存态，同亡一致。

> 一期 RCP-Stub：施加动作内存登记并默认"成功"，生效校验在 RCP-Impl 对接后启用（接口已预留）。

### 6.1.2 低优先级保障档（F-2 修正，评审第七轮）

LOW（档位 0）任务切后台/锁屏同样会被冻结。为避免"已纳管低优先级任务的进程反而受 RSS 干扰"，将档位 0 细化：
- **档位 0（不豁免）**：UNKNOWN/终态任务——无管控动作，进程回归原调度；
- **低优先级基础保障**（需求 11"按更高优先级处理"不改变）：MEDIUM/HIGH/CRITICAL 按档位施加豁免；进程归属仍按 **max 档位**合并，低档任务与高档任务同进程时自然按高档管控。

> 即：档位=0 的任务进程不加管控（遵循原设计），多任务归属按 max；不存在"低优先级任务的进程被额外干扰"问题（低档任务进程本就回归原调度，与原系统行为一致）。F-2 结论：**需求 11 的 max 合并语义正确，无遗漏**；低档任务进程与原系统行为一致（未纳管时同样会被冻结），纳管不改变其保障水平，仅当任务档位≥1 时提升保障。

### 6.2 RSS 管控维度细化（需求第 5 点反馈）

| 维度 | 现状接口（勘察结论） | 一期 | 二期 | 说明 |
|------|---------------------|------|------|------|
| 冻结/解冻 | 505 DYNAMICALLY_SET_SUSPEND_EXEMPT（白名单 res_sched_service.cpp:175）；517 THAW_ONE_APP（同步）；506/507/518 状态查询 | ✅ 主线 | — | payload 语义与 RSS owner 对齐（对接必答项）；**豁免粒度已决策为 pid（进程级，评审决策 2026-08-24，D-1 关闭）**——505 payload 拟携带 uid+pid（字段模式对齐 AMS 既有 RSS payload 先例 res_sched_util.cpp:228-234——该先例实为 517 payload，F-35 精确化；**505 的 value 施加/撤销语义（拟 1/0）在 res_type.h 无文档，纯为假设待 RSS 确认**，对齐问题 #2），R-22 全局豁免计数按进程（pid）计；**开源 master 上 505/517/520 无任何消费插件**——505 上报过权限/白名单检查后即被丢弃、507 恒返回 `isFrozen=false`（suspend_manager_base_service.cpp:66-72），端到端冻结管控仅在含 freezer 执行插件的产品上生效（P1，TASK-6 门禁前置条件；F-23 勘察修正：`const.taskmanager.low_memory_frozen_enable` 参数在开源 RSS master 不存在，产品侧启用开关以实际配置为准——RSS 对齐问题 #8） |
| CPU 调度（进程级） | 221 PROCESS_QOS_CHANGE（白名单 res_sched_service.cpp:151）；134 SET_BACKGROUND_PROCESS_PRIORITY；8 CGROUP_ADJUSTER | 预留（档位 3 扩展位） | ✅ | 进程级无需线程信息；与 RSS owner 对齐 payload 后启用 |
| CPU 调度（线程级） | QosController::SetThreadQosForOtherThread/Reset（qos_manager，内核 sched_qos 封装）；RTG（AddThreadToProcRtg） | — | ✅ | 需线程 tid（目标进程协作或 /proc 遍历），复杂度高 |
| 内存/低内存查杀 | 520 SYNC_RES_TYPE_SHOULD_FORCE_KILL_PROCESS（lmkd 杀前同步查询）；214 LOAD_KILL_REASON_CONFIG | 评估预留 | ✅ | 高优先级任务进程可响应"不应杀"；需与 RSS owner 确认 520 消费方语义 |
| 功耗 | 92 POWER_MODE_CHANGED / 155 BATTERY_STATUS / 170 APP_HIGH_POWER_CONSUMPTION / 215 BATTERY_LOW | 不主动豁免（观察维度） | 待评估 | 任务豁免不豁免功耗策略；高功耗场景告警观察 |
| 温控 | 66 THERMAL_STATE / 93 THERMAL_SCENARIO / 519 GET_THERMAL_DATA | 不主动豁免（系统级热管控） | 待评估 | 不做任务级豁免 |
| 设备待机（EDS） | device_standby 独立通道：ApplyAllowResource/UnapplyAllowResource/GetAllowList（IStandbyService.idl:22-25），与 505 冻结豁免**互不相通** | ✅ 一期支持（P7，评审决策 2026-08-24） | ✅ 已决策 | 任务激活期间经 StandbyServiceClient::ApplyAllowResource 申请待机资源豁免（网络/任务执行），任务终态/豁免撤销时 UnapplyAllowResource，与 505/517 同一生命周期内存登记（D-2 关闭）；依赖 device_standby 接口联调验收 |

**结论**：一期 = 冻结/解冻（档位 0~2 完整落地，档位 3 豁免部分落地）；二期 = CPU 调度（221/QosController）与低内存查杀（520）作为扩展点接入；功耗/温控为观察维度。

### 6.3 sessionQos 探索结论（Q-10 后续）

- 全树勘察（qos_manager / resource_schedule_service / ability_runtime）无 "sessionQos" 接口；QosManager 为内核 `sched_qos`（/proc/thread-self/sched_qos_ctrl）的用户态封装，提供**线程级** QoS 设置/恢复与 RTG 线程组，无"会话"维度。
- "会话级 QoS"语义由本模块的**任务管控档位**承载（任务 ≈ 会话；档位 ≈ 会话级管控强度），RSS 221 为进程级 QoS 通道。
- 结论：不引入 QosManager 作为一期依赖；二期经 `RES_TYPE_PROCESS_QOS_CHANGE`（进程级）与 QosController/RTG（线程级，需线程信息）扩展（ADR-9）。

### 6.4 资源管控代理层（ResourceControlProxy）设计（RSS 对接方式）

**背景**：RSS 的具体设计由 RSS 团队主导，目前暂未明确。任务管理模块**封装资源管控代理层（代理模式）**，按"管控动作语义"抽象资源管控能力，一期以骨架实现承载内存登记（活跃豁免，供 RSS 重启重放）与告警日志（不持久化，评审决策 2026-08-24），RSS 明确后替换为真实对接实现，上层（task_mgr/process_schedule）零改动。

**命名**：早期版本曾称 RSS Proxy / RssControlProxy，2026-08-21 第五轮评审定为 **ResourceControlProxy**——本层是任务管理模块的**资源管控出口**，代理对接各资源管控方（RSS 为主，未来 qos/温控/低内存查杀等管控方），不绑定 RSS 命名，为后续扩展留空间。

**接口定义（管控动作语义 + 完整上下文暴露，与 RSS resType 细节解耦）：**

```
enum class ControlAction {
    EXEMPT,          // 施加冻结豁免（一期）
    CLEAR_EXEMPT,    // 撤销冻结豁免（一期）
    THAW,            // 解冻（一期）
    SET_QOS,         // 设置进程级 QoS（二期预留）
    CLEAR_QOS,       // 恢复进程级 QoS（二期预留）
    // 预留扩展位
};

struct ControlRequest {          // 完整上下文：供 RSS 对接实现与未来管控方（qos/温控等）定制策略
    std::string taskId;          // 任务标识
    int32_t pid;                 // 目标进程 pid
    int32_t uid;                 // 进程 uid
    int32_t tokenId;             // 进程 tokenId
    std::string bundleName;      // 进程 bundleName
    int32_t taskType;            // 任务类型（枚举）
    int32_t priority;            // 任务优先级
    int32_t taskState;           // 任务状态
    int32_t level;               // 管控档位（0~3）
    std::string reason;          // 触发原因（任务激活/优先级变化/降级/任务结束/进程死亡）
    uint64_t timestamp;          // 事件时间戳
};

class ResourceControlProxy {     // 单例；taskHandler 串行访问
    virtual int32_t ApplyControl(const ControlRequest& request) = 0;    // 施加管控（按 action 与 level）
    virtual int32_t RestoreControl(const ControlRequest& request) = 0;  // 撤销管控（单进程）
    virtual int32_t NotifyTaskEnd(const TaskEndInfo& info) = 0;         // 任务结束通知（RSS 侧清理任务级策略）
    virtual int32_t QueryControlState(int32_t pid, bool& controlled) = 0; // 查询（预留）
    virtual int32_t ResetAll(int32_t pid) = 0;                          // 恢复该进程全部管控
};

struct TaskEndInfo {             // 任务结束清理信息（评审第六轮：task 结束后通知 RSS 清理资源管控策略）
    std::string taskId;          // 结束的任务
    std::vector<int32_t> pids;   // 该任务全部纳管进程（结束前快照）
    int32_t endReason;           // 结束原因（COMPLETED/CANCELED/FAILED/REJECTED/DESTROY）
    uint64_t timestamp;
};
```

> **任务结束清理双通道**（评审第六轮）：任务进入终态/DestroyTask 时——
> ① **进程级撤销**：对每个纳管进程 RestoreControl（撤销豁免/恢复 QoS），进程按剩余任务最高档重算（R-9/R-10）；
> ② **任务级清理通知**：NotifyTaskEnd(taskId, pids, reason) 一次性通知 RSS 侧清理按 taskId 建立的任务级策略（如豁免组、优先级组），防止 RSS 侧残留策略；RSS 对接后实现为 RSS 清理接口，一期 Stub 记录日志 + 告警。
> 两者在 taskHandler 串行执行，②在①之后；执行失败重试 ≤3 并告警（日志留痕供复核，R-20 扩展）。

> **暴露更多信息的意图**（评审反馈）：ControlRequest 携带任务与进程完整上下文，① RSS 对接实现可基于 bundleName/type/priority/state 等字段做定制化策略（RSS 侧策略未定时信息不缺失）；② 未来对接其他管控方（qos/温控/低内存查杀）时请求结构直接复用；③ 日志记录完整上下文，对接验收与问题定位信息完备。

**一期实现 `ResourceControlProxyStub`（骨架，代理模式中的"空代理"）：**
- 全部接口：内存登记（活跃豁免，供重放）+ TASK_RSSCONTROL hisysevent 上报 + hilog 日志（完整 ControlRequest 序列）；默认返回成功，**不产生真实 RSS 副作用**；
- 可配探测开关 `resource_control_probe_enable`（默认 false）：开启后对既有候选 resType（505 豁免/517 解冻，白名单已确认 res_sched_service.cpp:175）做真实调用，供联调期探测。**探测边界（P1，代码勘察 2026-08-24）**：开源 master 上 505/517/520 无任何消费插件——505 上报过权限/白名单检查后被静默丢弃，507 查询恒返回 `isFrozen=false` 默认值（suspend_manager_base_service.cpp:66-72）。因此 master 上开启探测只能验证"上报链路与权限配置正确"，**不能证明豁免生效**（会假成功）；端到端生效验证必须以产品含 freezer 执行插件（启用开关以产品配置为准，F-23）为前置条件（TASK-6 门禁）；
- 策略配置（档位→动作、上限/时长）仍在 process_schedule_policy，与代理实现无关；
- **冻结状态获取通道（P5，代码勘察 2026-08-24；第十四轮实证补强）**：RSS 已具备 push 通道 `RegisterSuspendObserver`（`ISuspendStateObserverBase.idl`：OnFrozen/OnActive/OnDoze oneway 回调，携带 pidList+uid；现网尚无消费者；mmi 有使用先例 suspend_state_manager.cpp:60-78——注册 observer 维护 frozen pid 本地缓存，与 P5 方案同构），且 `suspend_manager_base_client` 已随 `ressched_client` innerkit 交付（无独立 BUILD.gn，由 ressched_client 的 public_config 暴露——appmgr BUILD.gn 已依赖，无新增依赖，第十四轮实证）。RCP-Impl 落地时优先注册 observer 维护进程冻结状态**本地缓存**（O(1) 查询替代同步 507）并做登记-RSS 漂移检测；507/517 同步调用不持锁 + 超时保护（P6，见 §6.1.1-6）。
- **RSS 侧权限前置（F-24 补全，第十四轮勘察实证；F-33 扩大，第十五轮）**：① 505/517 经 `ResSchedClient::ReportData/ReportSyncEvent` 上报要求调用方为 **TOKEN_NATIVE** 且持有 **`ohos.permission.REPORT_RESOURCE_SCHEDULE_EVENT`**（res_sched_service.cpp:43/:273-300 IsHasPermission；505/517 在 SA_RES 白名单 :145/:175）——foundation 进程 native token 已具备该权限（AMS 既有 ResSchedUtil 上报先例佐证，无需新增配置）；② 507 查询**与 RegisterSuspendObserver 注册**（P5 本地缓存通道）均要求 **`ohos.permission.GET_SUSPEND_STATE`**（suspend_manager_base_service.cpp:27/:39/:103-111——observer 注册同走 CheckSuspendPermission，F-33）——foundation native token 是否已配置该权限**待确认**（列入 RSS 对齐问题，未配置则需产品侧权限配置或改走 ReportSyncEvent 直报通道）；③ 520 上报需权限 5523（res_sched_service.cpp:214 权限映射表，二期对接时确认）。

**对接策略（RSS 团队明确后）：**
1. RSS 提供接口（无论复用 505/517 还是新接口）→ 实现 `ResourceControlProxyImpl`，替换编译单元（条件编译），上层接口不变；
2. 对账：以**动作日志**（TASK_RSSCONTROL/hilog）作为对接验收基线（日志动作序列 == RSS 实际生效序列；评审决策 2026-08-24：经日志查看）；
3. 真机验证（TASK-6）在对接完成后执行端到端豁免生效验证；对接前以"动作日志断言"作为模块级验证。

## DFX 设计

### 7.1 hisysevent 事件（hisysevent.yaml 新增，AAFWK 域；不修改既有事件）

> 格式约束（F-40 补全）：每事件需 `__BASE`（type/level/tag/desc）+ 类型化字段（hisysevent.yaml 既有格式实证）；BEHAVIOR 事件 level 建议 MINOR、TASK_ABUSE_WARNING（FAULT）建议 CRITICAL + tag=STABILITY，随 TASK-5 评审定稿。

| 事件名 | 类型 | 关键字段 |
|--------|------|----------|
| TASK_CREATE | BEHAVIOR | taskId, type, priority, callerUid, result |
| TASK_STATE_CHANGE | BEHAVIOR | taskId, oldState, newState, result |
| TASK_DESTROY | BEHAVIOR | taskId, state, reason(cancel/destroy), result |
| TASK_INPUT | BEHAVIOR | taskId, action(require/clear), result（submit 随 D-21 移除——无外部回传） |
| TASK_RSSCONTROL | BEHAVIOR | taskId, pid, action(exempt/thaw/restore), level, result |
| TASK_ABUSE_WARNING | FAULT | taskId, callerUid, reason(limit/permission_forgery/timeout_demote/verify_failed/replay_failed) |

### 7.2 dump（hidumper -s 501 -a "<选项>" / aa）

输出任务全景：taskId、type、priority、state、纳管进程（pid 列表）、生效档位、payload 摘要（截断）、创建/更新时间；RSS 活跃豁免内存登记摘要。复用 appmgr 既有 SA dump 入口 `AppMgrService::Dump(fd, args)`（app_mgr_service.cpp:928，SystemAbility::Dump 覆写；F-26 修正：实际调用语法为 `hidumper -s 501 -a "<选项>"`，非 `hidumper -a 501`），新增任务选项分支；shell/hidumper 校验复用既有 `HasDumpPermission`（R-23）。

### 7.3 日志

hilog_tag_wrapper 新增 `AAFwkLogTag::TASK`（或复用 AppMgr tag），TAG_LOGI/TAG_LOGE 全链路（创建/流转/纳管/RSS 动作）。

## 错误码设计

新增任务域错误码段（`interfaces/inner_api/error_utils/` 任务域段，编号设计定稿，不占既有段）：

> **编号约束（F-40 补全，第十六轮）**：16000001~16000205 为公共 JS API 错误码契约段（ability_runtime_error_util.h 现有 57 个 EXTERNAL 错误码占用），任务域错误码（inner API）**必须避开该段**——建议新段（如 16000300+ 或独立段，随 TASK-0 定稿并与 API 评审确认）；inner API 错误码不进 JS 侧映射，无 @since 兼容负担。

| 错误码 | 值（占位） | 语义 |
|--------|-----------|------|
| ERR_TASK_NOT_FOUND | TASK 段 + 1 | 任务不存在 |
| ERR_TASK_INVALID_STATE | + 2 | 当前状态不允许该操作（如终态 CancelTask、终态 SetTaskParams） |
| ERR_TASK_INVALID_STATE_TRANSITION | + 3 | 非法状态流转 |
| ERR_TASK_PERMISSION_DENIED | + 4 | 非创建者操作/仿冒拒绝/非系统应用 |
| ERR_TASK_LIMIT_EXCEEDED | + 5 | 任务数/豁免上限 |
| ERR_INVALID_PARAM | 复用既有 | 参数非法（枚举越界/payload 超限） |

## 设计骨架

### 骨架范围

| 骨架项 | 目标 | 不包含 | 验证方式 |
|--------|------|--------|----------|
| IDL 骨架 | ITaskManager.idl + 数据结构 + stub/proxy 编译通过 | 完整业务逻辑 | 编译/IDL 生成 |
| 模块骨架 | task_mgr/process_schedule 目录与类骨架、BUILD.gn 接入 | 复杂策略 | 构建通过 |
| 客户端骨架 | TaskManagerClient/TaskListener | 全场景 | 最小用例编译 |
| 测试骨架 | 单测工程（task_mgr 单测 target） | 全场景 | 最小用例通过 |

### 骨架 Spec 拆分

| Task ID | 目标 | 受影响文件 | AC |
|---------|------|------------|-----|
| TASK-0（原 TASK-SKELETON-1，编号随 execution-plan 对齐，F-13） | IDL + 客户端 + 模块目录/BUILD.gn | ITaskManager.idl、task_manager_client、appmgr BUILD.gn | WHEN 构建 task_mgr 目标 THEN 编译通过且 stub/proxy/客户端生成 |

## 后续 Task 拆分

| Task ID | 目标 | 受影响文件 | 依赖 |
|---------|------|------------|------|
| TASK-0 | 骨架接线：IDL/客户端/模块目录/BUILD/gni/错误码 | ITaskManager.idl、task_manager_client、appmgr BUILD.gn | 无 |
| TASK-1 | 任务模型与状态机（TaskRecord/StateMachine/CreateTask/SetTaskState/生命周期） | task_mgr 核心 | design+spec Approved |
| TASK-2 | 查询/监听（ListenerManager/payload 事实记录；交互经 AI bar 内化，D-21） | task_mgr | TASK-1 |
| TASK-3 | AMS 拦截点（条件两段式纳管 + caller 校验 + 通道鉴权） | abilitymgr 拦截点 | TASK-1 |
| TASK-4 | 管控与 ResourceControlProxy（档位合并/RSS 动作/内存登记/上限/续期） | process_schedule、resource_control_proxy | TASK-1, TASK-3 |
| TASK-5 | DFX（hisysevent/dump/日志）+ 错误码接入 | hisysevent.yaml、task_dfx | TASK-1~4 |
| TASK-6 | 验证收口（全量 AC + 真机锁屏/后台场景） | test/ | TASK-1~5 |

> 编号与粒度以 execution-plan.md 为准（F-13 修正，第十三轮：本表此前与 execution-plan TASK-2/TASK-4 编号错位，已对齐）。

## API 签名、Kit 与权限

### 新增 API

| API 签名 | 类型 | Kit | d.ts 位置 | 权限要求 | SysCap |
|----------|------|-----|-----------|----------|--------|
| `int32_t ITaskManager::CreateTask(const TaskCreateParams&, std::string&)` | InnerApi | —（inner 层） | `interfaces/inner_api/app_manager/task_manager/`（IDL 生成） | 系统应用身份 | - |
| `int32_t ITaskManager::DestroyTask(const std::string&)` | InnerApi | — | 同上 | 系统应用+创建者 | - |
| `int32_t ITaskManager::CancelTask(const std::string&)` | InnerApi | — | 同上 | 系统应用+创建者 | - |
| `int32_t ITaskManager::SetTaskState(const std::string&, TaskState, const std::string&)` | InnerApi | — | 同上 | 系统应用+创建者 | - |
| `int32_t ITaskManager::SetTaskParams(const std::string&, const TaskParams&)` | InnerApi | — | 同上 | 系统应用+创建者 | - |
| `int32_t ITaskManager::GetTaskInfo(const std::string&, TaskInfo&)` | InnerApi | — | 同上 | 系统应用 | - |
| `int32_t ITaskManager::GetTaskInfos(std::vector<TaskInfo>&)` | InnerApi | — | 同上 | 系统应用 | - |
| `int32_t ITaskManager::RegisterTaskListener(const sptr<IRemoteObject>&)` | InnerApi | — | 同上 | 系统应用 | - |
| `int32_t ITaskManager::UnregisterTaskListener(const sptr<IRemoteObject>&)` | InnerApi | — | 同上 | 系统应用 | - |
| `int32_t ITaskManager::RegisterTaskIntent(const std::string&, int32_t)`（内部） | InnerApi | — | 同上 | AMS 调用 | - |
| `int32_t ITaskManager::RegisterTaskProcess(const std::string&, int32_t, int32_t)`（内部） | InnerApi | — | 同上 | AMS 调用 | - |
| `int32_t ITaskManager::UnregisterTaskProcess(const std::string&, int32_t)`（内部） | InnerApi | — | 同上 | AMS 调用 | - |

### 变更/废弃 API

无（全部新增）。

## 构建系统影响

### BUILD.gn 变更

```
文件路径: services/appmgr/BUILD.gn
变更说明: 新增源文件 task_mgr/（task_manager_service、task_record、task_state_machine、
          task_listener_manager、task_dfx）与 process_schedule/（process_schedule_manager、
          process_schedule_policy、process_schedule_types）；依赖 resource_schedule_service:
          ressched_client 已具备（:159）

文件路径: interfaces/inner_api/app_manager/task_manager/BUILD.gn（新增）
变更说明: ohos_idl 生成 ITaskManager stub/proxy；ohos_shared_library 或 ohos_library 产出
          task_manager_client 组件

文件路径: services/abilitymgr/BUILD.gn
变更说明: 新增对 task_manager_client 的依赖（拦截点调用）
```

### bundle.json 变更

无新增部件（并入 appms 部件）。

### 其他配置

```
hisysevent.yaml: 新增 7.1 节 6 个事件（AAFWK 域）
ability_runtime.gni: 新增特性开关 ability_runtime_ai_task_manager_enable（默认 true，
                    产品可裁剪；本期随资源调度开关一并编译）
sa_profile/501.json: 不变（复用 appms）
```

## 可选设计扩展

### 架构图

```
[AI 助手（系统应用）]──ITaskManager IPC──▶[AppMS (SA 501, foundation)]
[监听方系统模块（安全/审计等，系统应用）]──ITaskManager IPC──▶[AppMS]  │
        [AMS (SA 180, foundation)]──拦截点(app_manager 客户端)──▶[AppMS]
                                            AppMS 内部：
                        ┌───────────────┐    ┌──────────────────────┐
                        │ task_mgr      │──▶│ process_schedule     │
                        │ 任务模型/状态机 │    │ 进程组/档位/管控动作  │
                        │ 监听/交互/DFX  │    └──────────┬───────────┘
                        └───────────────┘               ▼
                                        ┌──────────────────────┐
                                        │ ResourceControlProxy（代理层）     │
                                        │ 一期：ResourceControlProxyStub     │
                                        │ （内存登记+告警日志+探测开关） │
                                        └──────────┬───────────┘
                                        （RSS 对接后替换为 ResourceControlProxyImpl）
                                        ┌──────────────▼──────────────┐
                                        │ resource_schedule_service    │
                                        │ 505/517/506/507（一期候选）   │
                                        │ 221/520（二期扩展）           │
                                        └─────────────────────────────┘
```

### 数据流/控制流

**任务主流程（创建→执行→结束）：**

| 步骤 | 调用方 | 被调用方 | 数据/接口 | 说明 |
|------|--------|----------|-----------|------|
| 1 | AI 助手 | AppMS | CreateTask(type, priority) | 校验系统应用；生成随机 taskId；创建 TaskRecord（UNKNOWN）；上报 TASK_CREATE |
| 2 | AI 助手 | AppMS | SetTaskState(SUBMITTED→WORKING) | 状态机校验；监听回调；激活档位判定 |
| 3 | AppMS | process_schedule | OnTaskActiveChanged | 档位合并重算；ApplyLevel 施加豁免（505） |
| 4 | AI 助手 | AMS | StartAbility(want[ohos.ai.taskId]) | 拦截点解析 → RegisterTaskIntent/RegisterTaskProcess |
| 5 | AMS | AppMS | RegisterTaskProcess(taskId, pid, callerTokenId) | B-2 校验；纳管；档位重算（进程加入后豁免生效） |
| 6 | AppMS | RSS | ReportData(505, exempt) | 内存登记（活跃豁免，供重放）；TASK_RSSCONTROL 日志 |
| 7 | AI 助手 | AppMS | SetTaskState(COMPLETED) | 终态；监听回调 |
| 8 | AppMS | process_schedule | OnTaskActiveChanged | 档位重算；撤销豁免（剩余任务最高档）；TASK_RSSCONTROL |

**INPUT/AUTH 交互流（D-21 重写：AI bar 内化，对齐 ADK HITL 挂起-恢复由助手自闭环）：**

| 步骤 | 调用方 | 被调用方 | 数据/接口 | 说明 |
|------|--------|----------|-----------|------|
| 1 | AI 助手 | AppMS | SetTaskState(INPUT_REQUIRED, payload{text, options}) | 状态机校验 payload 必填；payload 留存 TaskRecord（事实记录）；监听回调携带 payload（安全等模块感知交互态）；TASK_INPUT(require) |
| 2 | AI 助手（AI bar） | 用户 | 助手进程内呈现 | 描述与可选项经 AI bar 展示——**不经任务管理模块**（助手即 payload 源） |
| 3 | 用户 → AI bar → 助手 | （助手进程内） | 用户选择/输入 | 助手直接获得响应，无 IPC、无模块中介 |
| 4 | AI 助手 | AppMS | SetTaskState(WORKING/COMPLETED, payload 可选携带响应) | 流转出交互态；监听回调；交互超时计时终止 |

**异常/降级流：** 豁免撤销失败重试 ≤3 → 告警（日志留痕，R-20）；进程死亡 → 死亡通知链 → RemoveProcessFromTask + 档位重算（R-12）；AppMS 异常重启 → foundation 进程重启、全部应用进程被清理，任务记录与 RSS 侧豁免随进程消亡同亡（无孤儿豁免，C-1 结论；spec 非功能可靠性）。

### 风险与开放项

| 风险/开放项 | 影响 | 缓解/关闭方式 |
|-------------|------|---------------|
| RSS 设计未明确，且**开源 master 上 505/517/520 无消费执行器**（P1：505 上报过权限检查后即被丢弃、507 恒返回未冻结）——真实豁免/解冻在 master 上无法端到端生效，探测会"假成功" | 高 | **ResourceControlProxy 代理层隔离**（ADR-4/§6.4）：一期骨架实现（内存登记+告警日志+探测开关），模块行为可单测验证，**Stub 动作日志断言为 master 上唯一可复现的验证基线**；端到端真机验证前置条件 = 产品含 freezer 执行插件（启用开关以产品配置为准，F-23 勘察修正）（TASK-6 门禁） |
| RSS（SA 1901）异常重启导致豁免状态丢失、AppMS 内存登记与 RSS 实际状态漂移（P4） | 中 | RCP-Impl 复用 RSS 客户端内置死亡通知 → RSS 重启后按内存登记幂等重放全部活跃豁免（§6.1.1-5），重放失败重试 ≤3 + 告警；二期 push observer 运行时漂移检测 |
| RSS 505 payload 语义与豁免覆盖范围未对齐（含锁屏冻结是否覆盖） | 高 | **豁免粒度已决策 pid（2026-08-24，D-1 关闭）**；payload 语义仍为对接必答项；以动作日志（TASK_RSSCONTROL）为验收基线；RSS 明确前不阻断模块开发 |
| 520 SHOULD_FORCE_KILL_PROCESS 消费方语义未确认 | 中 | 二期启用前确认；一期仅评估预留 |
| 设备待机（EDS）限制通道与 505 豁免互不相通：夜间待机期间 AI 定时任务可能被待机网络/任务限制阻断（需求场景 1 典型时序，D-2） | 中 | **已决策一期支持（2026-08-24）**：新增 P7 待机豁免联动（ApplyAllowResource/UnapplyAllowResource，与 505 同生命周期内存登记）；依赖 device_standby 接口联调验收 |
| 两段式纳管的意向超时窗口（30s）内进程未 attach | 低 | 意向超时清理；attach 晚于超时由 attach 处直接补发②（身份取 AbilityRecord 存储值，F-39 修正） |
| 系统应用白名单口径 | 低 | 复用 AppMS 既有 token 校验；白名单范围随评审定 |
| 多用户隔离细节（任务跨用户可见性） | 低 | 一期按 userId 隔离；细化随 execution-plan 评审 |
| AI 助手进程自身的冻结策略与自注册需求（§5.2.1） | 中 | 与 AI 助手团队对齐目标设备上助手的冻结策略（前台/continuous task/白名单现状）；真机验证阶段与 RSS owner 确认"助手冻结→任务空转至超时"窗口是否真实发生；显式自注册通道一期即可用，supervisor 弱保障档位列二期评估 |
| **创建者失联/死亡时任务滞留激活态（F-43，第十七轮）** | 中 | 超时仅撤销豁免不改状态（ADR-6）→ 滞留任务占用 R-2 配额（活跃=非终态）；16 个滞留即配额死锁且无人可解（创建者死亡时）。处置方案 A-9 待对齐：(a) 窄口径系统流转（超时→FAILED/创建者死亡→CANCELED，复用 AppDeathRecipient 链）——推荐；(b) 接受滞留（创建者重启可恢复） |

### 设计→规格追溯表（Define→Specify carry-through）

| 设计决策/模块 | 对应 spec 规则 | 对应 AC |
|---------------|----------------|---------|
| 状态机流转表（4.2） | R-3, R-4 | AC-1.2, AC-1.3 |
| 随机 taskId + B-2 caller 校验（5.3） | R-1, R-8 | AC-1.1, AC-2.2 |
| 两段式纳管（ADR-3, 5.4） | R-7, R-13 | AC-2.1, AC-2.5 |
| 档位合并模型（ADR-5, 6.1） | R-10, R-11, R-19~R-21 | AC-2.3, AC-5.1~5.4 |
| 监听/交互 payload 机制（5.5） | R-14, R-16~R-18 | AC-3.x, AC-4.x |
| 上限与防滥用（R-2, R-22 对应实现） | R-2, R-22 | AC-1.5, AC-5.5 |
| 进程死亡清理（5.6） | R-12 | AC-2.4 |
| 降级恢复与重试（6.1 内存登记与日志） | R-9, R-20 | AC-5.2, AC-5.4 |
| 错误码设计 | R-4~R-6 等错误码引用 | AC-1.x~AC-6.x |
| DFX（7.1~7.3） | R-23, R-24 | AC-6.1, AC-6.2 |
