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
| 应用层（AI 助手/系统 UI，系统应用） | 调用方进程 | 经 TaskManagerClient 调用任务接口 | 新增（外部调用方，本仓提供客户端） |
| inner API 客户端 | `interfaces/inner_api/app_manager/task_manager/` | TaskManagerClient / ITaskManager stub·proxy / TaskListener | 新增 |
| 服务层 | `services/appmgr/src/task_mgr/` | 任务管理：任务模型、状态机、监听、交互传达、DFX | 新增 |
| 服务层 | `services/appmgr/src/process_schedule/` | 通用进程调度管理：进程组、管控档位、RSS 动作施加/撤销、多任务归属合并 | 新增 |
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
| 性能 | 拦截点零开销（无 taskId 直接短路）；任务操作同进程 IN_PROCESS；状态机事件处理器串行，单任务操作 <5ms（spec NFR） |
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
| ADR-3 | 进程纳管时机 | 两段式：① 拦截点解析 Want → RegisterTaskIntent(taskId, callerTokenId)（AppMS 校验 caller 并登记意向，超时 30s 清理）；② 进程 attach（pid 就绪）→ RegisterTaskProcess(taskId, pid, callerTokenId) 完成纳管 | 仅启动请求时纳管（进程未创建无 pid，需按 pid 匹配进程，失败率高）；仅 attach 后纳管（taskId 需穿透 attach 链，侵入 appmgr 热路径） | 覆盖进程未创建/已存在两态；attach 点复用 AMS 既有 pid 就绪回调（OnAbilityRequestDone/AttachAbilityThread），appmgr 热路径零侵入 | AC-2.1 |
| ADR-4 | RSS 集成方式 | 复用现有 resType（505 豁免 / 517 解冻 / 506·507·518 查询），payload 语义与 RSS owner 对齐 | 新增 RSS resType/插件（触发 RSS 内存基线评审，跨仓周期长）；二期再评估专用接口 | 落地最快；白名单已确认（res_sched_service.cpp:175） | AC-5.x |
| ADR-5 | 管控档位模型 | 档位化 + 合并：任务优先级→管控档位（0~3）；进程档位 = max(纳管任务档位)；档位变化增量施加/撤销 RSS 动作 | 每任务独立施加（同进程重复调用 RSS，幂等管理复杂）；全局统一（无法表达优先级差） | 档位合并天然满足需求 11（多任务按最高）；增量动作避免重复调用 | AC-2.3, AC-5.4 |
| ADR-6 | 状态机驱动 | AI 助手驱动 + 模块校验（合法流转表；终态不可回退；REJECTED 仅创建者可设） | 助手全权设置（状态易乱，放弃）；模块自动推导（语义易偏差，放弃） | 与需求 2/5 一致；流转合法性由系统把关 | AC-1.2, AC-1.3 |
| ADR-7 | INPUT/AUTH 交互通道 | 状态监听回调携带 payload（描述+可选项 JSON），SubmitTaskInput 回传，input 存任务记录 | 独立消息通道（机制更重，与状态监听解耦，一期不需要） | 与 ADK HITL（request_credential 挂起-恢复）语义一致；系统 UI 已监听状态流 | AC-4.x |
| ADR-8 | 线程模型 | AppMS 内独立事件处理器（taskHandler 串行线程）处理全部任务操作与回调派发 | 多线程并发处理（需锁/一致性管理，收益低） | 与 appmgr/agentmgr 既有 EventHandlerWrap 风格一致；串行免锁、回调顺序有保证 | 非功能（并发 ≥16） |
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
    HIGH,       // 档位 2：工作态全时豁免
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
                └──> REJECTED（终态）         │                    ├─（SubmitTaskInput 回传）──> WORKING
                                             │                    └─（回传即完成）──> COMPLETED
                                             ├──> COMPLETED（终态）
                                             ├──> FAILED（终态）
                                             └──> CANCELED（终态，CancelTask/任意非终态）
```

| 当前状态 | 合法目标状态 | 触发者 | 约束 |
|----------|--------------|--------|------|
| UNKNOWN | SUBMITTED | 创建者 | — |
| UNKNOWN | REJECTED | 创建者 | REJECTED 仅创建者可设（AC-1.3） |
| SUBMITTED | WORKING / REJECTED | 创建者 | — |
| WORKING | INPUT_REQUIRED / AUTH_REQUIRED / COMPLETED / FAILED / CANCELED | 创建者 | 进入交互状态必须携带 payload（AC-4.1） |
| INPUT_REQUIRED | WORKING / COMPLETED / FAILED / CANCELED | 创建者 | 出交互状态前可选校验 input 已提交 |
| AUTH_REQUIRED | WORKING / COMPLETED / FAILED / CANCELED | 创建者 | 同上 |
| 任意终态 | 无（不可回退） | — | R-4 |

> 校验实现：`TaskStateMachine::CanTransition(from, to)` + 创建者身份校验（R-6）。

### 4.3 任务记录（TaskRecord）

```
struct TaskRecord {
    std::string taskId;            // 不可猜测随机 ID（16 字节熵，UUID v4 风格）
    TaskType type;
    TaskPriority priority;
    TaskState state;
    std::string payload;           // 状态 payload（交互描述/可选项/进度）
    std::string input;             // SubmitTaskInput 回传内容（交互态）
    int32_t creatorTokenId;        // B-2 校验：创建者 tokenId
    int32_t creatorUid;            // 创建者 uid（多用户隔离维度）
    int32_t userId;                // 创建者 userId（跨用户不可见）
    std::set<int32_t> processPids; // 纳管进程集合
    uint64_t createTime;           // 创建时间戳
    uint64_t updateTime;           // 最近状态变更时间戳
    int32_t exemptRefCount;        // 豁免引用计数（同进程幂等）
};
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
│   └── task_dfx.h                        # hisysevent 上报/dump
├── src/task_mgr/…（对应实现）
├── include/process_schedule/             # 通用进程调度管理（进程调度能力层，与任务语义解耦）
│   ├── process_schedule_manager.h        # 进程组注册/注销、档位计算、RSS 动作施加/撤销
│   ├── process_schedule_policy.h         # 策略：优先级→档位映射、上限/时长配置
│   └── process_schedule_types.h          # 档位枚举、RSS 动作记录
└── src/process_schedule/…（对应实现）

interfaces/inner_api/app_manager/task_manager/
├── ITaskManager.idl                      # 接口定义（IDL 生成 stub/proxy）
├── include/task_manager_client.h         # C++ 客户端（AI 助手/系统 UI/AMS 使用）
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
    void    ApplyLevel(int32_t pid, Level level);              // 增量施加/撤销 RSS 动作
    std::unordered_map<int32_t, std::set<std::string>> pidToTasks_;
    std::unordered_map<std::string, std::set<int32_t>> taskToPids_;
    std::unordered_map<int32_t, Level> effectiveLevel_;
};
```

> 该层不感知"任务"业务语义（状态机/交互），只管理"进程组 + 档位 + RSS 动作"，供任务层及其他未来场景（系统级长时任务等）复用（ADR-1 抽象化）。

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
    int32 SubmitTaskInput(in string taskId, in string input);
    // 内部（AMS 拦截点→AppMS，经 app_manager 客户端体系）
    int32 RegisterTaskIntent(in string taskId, in int32 callerTokenId);  // 两段式①
    int32 RegisterTaskProcess(in string taskId, in int32 pid, in int32 callerTokenId); // 两段式②
    int32 UnregisterTaskProcess(in string taskId, in int32 pid);
}
```

数据结构（IDL struct / parcelable）：

```
struct TaskCreateParams { int32 type; int32 priority; string payload; };  // payload ≤4KB
struct TaskInfo {
    string taskId; int32 type; int32 priority; int32 state;
    string payload; string input; List<int32> processPids;
    int64 createTime; int64 updateTime;
};
```

**回调接口：**

```
interface ITaskListener {   // 系统 UI 注册
    void OnTaskStateChanged(in string taskId, in int32 state, in string payload, in int64 timestamp);
};
```

### 5.3 防仿冒设计落地（B-1+B-2）

1. **taskId 生成**（B-1）：`CreateTask` 时以随机数源（`RAND_bytes` 或 `std::random_device`+mt19937）生成 16 字节，格式化为 UUID v4 风格字符串（`xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx`），≥128bit 熵，不可猜测（R-1）。
2. **caller 身份校验**（B-2）：
   - 任务创建时绑定 `creatorTokenId/creatorUid/userId`（TaskRecord）；
   - 拦截点两段式上报携带**发起方 tokenId**（AMS 拦截点取 `IPCSkeleton::GetCallingTokenID()`）；
   - AppMS 校验（权威）：`callerTokenId ∈ {creatorTokenId} ∪ {该任务纳管进程的 tokenId 集合}` 才接受纳管（R-8）；
   - 校验失败：拒绝纳管、**不阻断启动**、不上报错误给调用方（防探测）；
   - 链式启动：任务内进程 A 启动 B → callerTokenId = A 的 tokenId ∈ 纳管集合 → 合法（与 ADK supervisor→sub-agent 可信链一致）。
3. **B-3 可选增强**（预留）：若后续需要防重放/防泄露增强，增加短时 token 换 taskId 通道，接口与账本已预留扩展点。

### 5.4 AMS 拦截点设计（最小改动）

| 链路 | 拦截位置 | 说明 |
|------|----------|------|
| StartAbility | `AbilityManagerService::StartAbilityInner`（ability_manager_service.cpp，启动参数解析处） | 解析 `want.GetStringParam("ohos.ai.taskId")`；无参数直接短路（零开销 R-13） |
| ConnectAbility / ServiceExtension / UIExtension | `AbilityConnectManager` 连接处理收敛点 | 同上 |
| 意图框架（InsightIntent） | 意图执行最终收敛于 StartAbility/ConnectAbility 既有链路 | 无需独立拦截点 |
| Cli | `tools/aa` 启动最终收敛于 AMS StartAbility | 同上 |
| A2A | A2A 服务端为 Agent 扩展，启动/连接收敛于 AMS 链路；taskId 由 supervisor 透传 | 同上 |

拦截逻辑（不侵入 appmgr 热路径）：
```
taskId = want.GetStringParam("ohos.ai.taskId")          // 无 → 短路返回
callerTokenId = IPCSkeleton::GetCallingTokenID()
TaskManagerClient::RegisterTaskIntent(taskId, callerTokenId)   // ① 意向（AppMS 校验+登记，30s 超时清理）
// 进程 attach 时（AMS OnAbilityRequestDone / AttachAbilityThread 处 pid 就绪）：
TaskManagerClient::RegisterTaskProcess(taskId, pid, callerTokenId)  // ② 完成纳管
```
> 调用经 `interfaces/inner_api/app_manager/` 客户端（既有 AMS→AppMS 通道体系），符合 AGENTS.md；所有调用失败仅日志告警，不阻断启动（R-7/R-8/R-13）。

### 5.5 监听与交互机制

- **监听**：`TaskListenerManager` 维护注册表（IRemoteObject 序列化后为 key）；状态变更在 taskHandler 线程派发（回调顺序与状态变更顺序一致，R-14）；监听者死亡自动移除（DeathRecipient）。
- **交互传达**（ADR-7，对齐 ADK HITL）：
  - 进入 INPUT_REQUIRED/AUTH_REQUIRED：创建者 `SetTaskState` 携带 payload `{"type":"input"|"auth","text":"<描述>","options":["<可选项>"]}`（≤4KB，R-16）；监听者（系统 UI）收到回调展示；
  - 用户选择/输入后系统 UI 调 `SubmitTaskInput(taskId, input)`（仅创建者合法——注意：此处创建者为 AI 助手，系统 UI 回传的用户选择经 AI 助手或授权机制提交，交互归属规则见 5.7）；
  - input 写入 TaskRecord；创建者经 GetTaskInfo/监听获得；任务可流转出交互态（R-17）；
  - 交互态任务被取消/销毁：payload/input 清除、不再回调（R-18）。

### 5.6 并发与多任务归属（需求 10/11）

- 双向索引（`taskToPids_`/`pidToTasks_`，5.1）；单任务记录 ≤1KB；
- 档位合并：`effectiveLevel(pid) = max(其任务集合中激活任务的档位)`（ADR-5，R-10）；
- 档位变化 → `ApplyLevel` 增量施加/撤销 RSS 动作（同 pid 同类型豁免幂等，exemptRefCount，R-11）；
- 任务结束/降级 → 重算剩余任务档位并撤销（R-9/R-20）；
- 并发上限：创建者活跃任务 ≤ 16（R-2）；豁免进程数/时长上限可配（R-22）。

### 5.7 权限与身份（基础鉴权）

| 入口 | 校验 |
|------|------|
| CreateTask/DestroyTask/CancelTask/SetTaskState/SetTaskParams/SubmitTaskInput | 系统应用身份（tokenId 属系统应用/SA 白名单）；DestroyTask 等操作再校验创建者（R-6） |
| GetTaskInfo(s)/RegisterTaskListener | 系统应用身份（R-15） |
| RegisterTaskIntent/RegisterTaskProcess | B-2 caller 校验（5.3） |
| 多用户 | 任务归属 userId，跨用户不可见/不可操作（spec 全局特性影响） |

> 系统应用白名单复用 AppMS 既有校验能力（`PermissionVerification` 或 token 校验工具），不新增权限项。

## 资源调度子模块设计（RSS 管控细化）

### 6.1 管控档位与 RSS 动作映射

| 档位 | 语义 | 一期 RSS 动作 | 二期扩展动作 |
|------|------|---------------|--------------|
| 0 | 不豁免 | 无 | — |
| 1 | 限时豁免（MEDIUM 工作态） | 豁免（505，限时，默认 30min 可配） | — |
| 2 | 全时豁免（HIGH 工作态） | 豁免（505）+ 必要时解冻（517） | — |
| 3 | 豁免 + 提权（CRITICAL） | 豁免 + 解冻 | 进程级 QoS 提权（RSS 221）；线程级 QoS（QosController/RTG，需线程 tid） |

动作账本：`{taskId, pid, action, level, timestamp, result}`，供撤销（R-20 重试 ≤3）、dump 与审计使用。

### 6.2 RSS 管控维度细化（需求第 5 点反馈）

| 维度 | 现状接口（勘察结论） | 一期 | 二期 | 说明 |
|------|---------------------|------|------|------|
| 冻结/解冻 | 505 DYNAMICALLY_SET_SUSPEND_EXEMPT（白名单 res_sched_service.cpp:175）；517 THAW_ONE_APP（同步）；506/507/518 状态查询 | ✅ 主线 | — | payload 语义与 RSS owner 对齐（开放项） |
| CPU 调度（进程级） | 221 PROCESS_QOS_CHANGE（白名单 res_sched_service.cpp:151）；134 SET_BACKGROUND_PROCESS_PRIORITY；8 CGROUP_ADJUSTER | 预留（档位 3 扩展位） | ✅ | 进程级无需线程信息；与 RSS owner 对齐 payload 后启用 |
| CPU 调度（线程级） | QosController::SetThreadQosForOtherThread/Reset（qos_manager，内核 sched_qos 封装）；RTG（AddThreadToProcRtg） | — | ✅ | 需线程 tid（目标进程协作或 /proc 遍历），复杂度高 |
| 内存/低内存查杀 | 520 SYNC_RES_TYPE_SHOULD_FORCE_KILL_PROCESS（lmkd 杀前同步查询）；214 LOAD_KILL_REASON_CONFIG | 评估预留 | ✅ | 高优先级任务进程可响应"不应杀"；需与 RSS owner 确认 520 消费方语义 |
| 功耗 | 92 POWER_MODE_CHANGED / 155 BATTERY_STATUS / 170 APP_HIGH_POWER_CONSUMPTION / 215 BATTERY_LOW | 不主动豁免（观察维度） | 待评估 | 任务豁免不豁免功耗策略；高功耗场景告警观察 |
| 温控 | 66 THERMAL_STATE / 93 THERMAL_SCENARIO / 519 GET_THERMAL_DATA | 不主动豁免（系统级热管控） | 待评估 | 不做任务级豁免 |

**结论**：一期 = 冻结/解冻（档位 0~2 完整落地，档位 3 豁免部分落地）；二期 = CPU 调度（221/QosController）与低内存查杀（520）作为扩展点接入；功耗/温控为观察维度。

### 6.3 sessionQos 探索结论（Q-10 后续）

- 全树勘察（qos_manager / resource_schedule_service / ability_runtime）无 "sessionQos" 接口；QosManager 为内核 `sched_qos`（/proc/thread-self/sched_qos_ctrl）的用户态封装，提供**线程级** QoS 设置/恢复与 RTG 线程组，无"会话"维度。
- "会话级 QoS"语义由本模块的**任务管控档位**承载（任务 ≈ 会话；档位 ≈ 会话级管控强度），RSS 221 为进程级 QoS 通道。
- 结论：不引入 QosManager 作为一期依赖；二期经 `RES_TYPE_PROCESS_QOS_CHANGE`（进程级）与 QosController/RTG（线程级，需线程信息）扩展（ADR-9）。

## DFX 设计

### 7.1 hisysevent 事件（hisysevent.yaml 新增，AAFWK 域；不修改既有事件）

| 事件名 | 类型 | 关键字段 |
|--------|------|----------|
| TASK_CREATE | BEHAVIOR | taskId, type, priority, callerUid, result |
| TASK_STATE_CHANGE | BEHAVIOR | taskId, oldState, newState, result |
| TASK_DESTROY | BEHAVIOR | taskId, state, reason(cancel/destroy), result |
| TASK_INPUT | BEHAVIOR | taskId, action(require/submit/clear), result |
| TASK_RSSCONTROL | BEHAVIOR | taskId, pid, action(exempt/thaw/restore), level, result |
| TASK_ABUSE_WARNING | FAULT | taskId, callerUid, reason(limit/permission_forgery) |

### 7.2 dump（hidumper -a 501 / aa）

输出任务全景：taskId、type、priority、state、纳管进程（pid 列表）、生效档位、payload 摘要（截断）、创建/更新时间；RSS 动作账本摘要。复用 appmgr DumpState 入口（shell/hidumper 校验，R-23）。

### 7.3 日志

hilog_tag_wrapper 新增 `AAFwkLogTag::TASK`（或复用 AppMgr tag），TAG_LOGI/TAG_LOGE 全链路（创建/流转/纳管/RSS 动作）。

## 错误码设计

新增任务域错误码段（`interfaces/inner_api/error_utils/` 任务域段，编号设计定稿，不占既有段）：

| 错误码 | 值（占位） | 语义 |
|--------|-----------|------|
| ERR_TASK_NOT_FOUND | TASK 段 + 1 | 任务不存在 |
| ERR_TASK_INVALID_STATE | + 2 | 当前状态不允许该操作（如终态 CancelTask、非交互态 SubmitTaskInput） |
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
| TASK-SKELETON-1 | IDL + 客户端 + 模块目录/BUILD.gn | ITaskManager.idl、task_manager_client、appmgr BUILD.gn | WHEN 构建 task_mgr 目标 THEN 编译通过且 stub/proxy/客户端生成 |

## 后续 Task 拆分

| Task ID | 目标 | 受影响文件 | 依赖 |
|---------|------|------------|------|
| TASK-1 | 任务模型与状态机（TaskRecord/StateMachine/CreateTask/SetTaskState/生命周期） | task_mgr 核心 | design+spec Approved |
| TASK-2 | 进程调度管理（档位合并/RSS 动作/账本） | process_schedule | TASK-1 |
| TASK-3 | AMS 拦截点（两段式纳管 + caller 校验） | abilitymgr 拦截点 | TASK-1 |
| TASK-4 | 监听与交互传达（ListenerManager/SubmitTaskInput/payload） | task_mgr | TASK-1 |
| TASK-5 | DFX（hisysevent/dump/日志）+ 错误码接入 | hisysevent.yaml、task_dfx | TASK-1~4 |
| TASK-6 | 单测/集成用例 + 真机场景验证（锁屏/后台） | test/ | TASK-1~5 |

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
| `int32_t ITaskManager::SubmitTaskInput(const std::string&, const std::string&)` | InnerApi | — | 同上 | 系统应用 | - |
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
[系统 UI（系统应用）]──ITaskManager IPC──▶[AppMS]         │
        [AMS (SA 180, foundation)]──拦截点(app_manager 客户端)──▶[AppMS]
                                            AppMS 内部：
                        ┌───────────────┐    ┌──────────────────────┐
                        │ task_mgr      │──▶│ process_schedule     │
                        │ 任务模型/状态机 │    │ 进程组/档位/RSS 动作  │
                        │ 监听/交互/DFX  │    └──────────┬───────────┘
                        └───────────────┘               │ ResSchedClient
                                        ┌──────────────▼──────────────┐
                                        │ resource_schedule_service    │
                                        │ 505/517/506/507（一期）       │
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
| 6 | AppMS | RSS | ReportData(505, exempt) | 豁免账本；TASK_RSSCONTROL |
| 7 | AI 助手 | AppMS | SetTaskState(COMPLETED) | 终态；监听回调 |
| 8 | AppMS | process_schedule | OnTaskActiveChanged | 档位重算；撤销豁免（剩余任务最高档）；TASK_RSSCONTROL |

**INPUT/AUTH 交互流（对齐 ADK HITL）：**

| 步骤 | 调用方 | 被调用方 | 数据/接口 | 说明 |
|------|--------|----------|-----------|------|
| 1 | AI 助手 | AppMS | SetTaskState(INPUT_REQUIRED, payload{text, options}) | 状态机校验 payload 必填；监听回调携带 payload；TASK_INPUT(require) |
| 2 | AppMS | 系统 UI | OnTaskStateChanged(INPUT_REQUIRED, payload) | UI 展示描述与可选项 |
| 3 | 系统 UI（用户选择） | AppMS | SubmitTaskInput(taskId, input) | 校验创建者；写入 TaskRecord.input；TASK_INPUT(submit) |
| 4 | AI 助手 | AppMS | GetTaskInfo / 监听 | 获得 input；SetTaskState(WORKING/COMPLETED) 继续 |

**异常/降级流：** 豁免撤销失败重试 ≤3 → 告警保留账本（R-20）；进程死亡 → 死亡通知链 → RemoveProcessFromTask + 档位重算（R-12）；AppMS 异常重启 → 任务记录丢失（可接受，进程管控自动失效，spec 非功能可靠性）。

### 风险与开放项

| 风险/开放项 | 影响 | 缓解/关闭方式 |
|-------------|------|---------------|
| RSS 505 payload 语义与豁免覆盖范围未对齐（含锁屏冻结是否覆盖） | 高 | 设计评审后与 RSS owner 对齐；一期降级为"不豁免不阻断" |
| 520 SHOULD_FORCE_KILL_PROCESS 消费方语义未确认 | 中 | 二期启用前确认；一期仅评估预留 |
| 两段式纳管的意向超时窗口（30s）内进程未 attach | 低 | 意向超时清理；attach 晚于超时则重新走①（拦截点在 attach 处重发） |
| 系统应用白名单口径 | 低 | 复用 AppMS 既有 token 校验；白名单范围随评审定 |
| 多用户隔离细节（任务跨用户可见性） | 低 | 一期按 userId 隔离；细化随 execution-plan 评审 |

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
| 降级恢复与重试（6.1 账本） | R-9, R-20 | AC-5.2, AC-5.4 |
| 错误码设计 | R-4~R-6 等错误码引用 | AC-1.x~AC-6.x |
| DFX（7.1~7.3） | R-23, R-24 | AC-6.1, AC-6.2 |
