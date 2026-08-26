# AI 语音助手团队对齐表：枚举与使用模式（O-1）

> 用途：与 AI 语音助手团队对齐任务模型枚举与调用模式 · 生成：2026-08-26 第十三轮
> 背景：任务管理机制（AppMS/SA 501，ITaskManager inner API）枚举初稿见 design.md §4.1；以下每项给出我方定义、语义、待确认问题。**对齐结论将回写 design.md §4.1 并冻结**（评审后确认，proposal 基线开放项 1/6）。

## 1. TaskState（9 态状态机）

| 枚举 | 语义 | 谁可设置 | 到达路径 |
|------|------|---------|---------|
| UNKNOWN | 创建后初始态（已表达诉求、未提交执行） | —（CreateTask 默认） | CreateTask |
| SUBMITTED | 已提交（就绪，等待/即将执行） | 创建者 | UNKNOWN→SUBMITTED |
| WORKING | 执行中 | 创建者 | SUBMITTED→WORKING |
| INPUT_REQUIRED | 等待用户输入（**必带 payload**：描述+可选项） | 创建者 | WORKING→INPUT_REQUIRED |
| AUTH_REQUIRED | 等待用户授权（**必带 payload**） | 创建者 | WORKING→AUTH_REQUIRED |
| COMPLETED | 成功完成（终态） | 创建者 | WORKING/交互态→COMPLETED |
| CANCELED | 已取消（终态） | 创建者（CancelTask 或 SetTaskState） | 任意非终态→CANCELED |
| FAILED | 执行失败（终态） | 创建者 | WORKING/交互态→FAILED |
| REJECTED | 被创建者拒绝（终态，**仅创建者可设**） | 创建者 | UNKNOWN/SUBMITTED→REJECTED |

**关键规则**：终态不可回退；流转合法性由模块校验（非法返回 ERR_TASK_INVALID_STATE_TRANSITION）；激活态集合 = {SUBMITTED, WORKING, INPUT_REQUIRED, AUTH_REQUIRED}（决定资源保障档位）。

**待确认问题：**

| # | 问题 | 说明 |
|---|------|------|
| Q-1 | UNKNOWN→WORKING 直达是否需要？ | 现设计强制 UNKNOWN→SUBMITTED→WORKING 两跳（两 IPC 调用）；若助手普遍"创建即执行"，可评估允许 UNKNOWN→WORKING（状态机表一处改动，需评审） |
| Q-2 | 进度上报模式确认 | 任务执行进度经 SetTaskParams 更新 payload → 系统 UI 经 OnTaskNotice(**PAYLOAD_UPDATED**) 实时收到（第十三轮 F-9 补全）；确认进度 payload 的 JSON 约定（如 {"progress": 0~100, "text": "..."}），我方对内容不做解析 |
| Q-3 | INPUT/AUTH payload JSON 约定 | 现约定 {"type":"input"|"auth","text":"<描述>","options":["<可选项>"]}（≤4KB 可配）；用户选择经 SubmitTaskInput 回传原始 input 字符串——确认是否需要结构化回传约定（如 {"selectedIndex": n} 或自由文本） |
| Q-4 | REJECTED 的实际使用场景 | 现语义"创建者拒绝执行该任务"（如用户明确拒绝授权后助手放弃）；确认与 CANCELED 的分工是否符合助手侧模型 |

## 2. TaskPriority（4 档 → 资源管控档位）

| 枚举 | 管控语义 | 适用建议 |
|------|---------|---------|
| LOW (0) | 不豁免（与原调度一致） | 进度可丢失的次要任务 |
| MEDIUM (1) | 限时豁免（默认 30min，可配） | 常规后台任务 |
| HIGH (2) | 激活期持续豁免（可续期：仅 priority 变化触发，≤N=3 次；硬上限 60min） | 锁屏定时任务、长时处理任务（**核心场景档位**） |
| CRITICAL (3) | 豁免+解冻（QoS 提权二期） | 极少数关键任务（如用户正在等待的语音交互） |

**待确认问题：**

| # | 问题 | 说明 |
|---|------|------|
| Q-5 | 助手侧典型任务的档位映射表 | 请按典型场景（闹钟联动/定时播报/批量处理/下载/播报中）给出建议档位，用于校准默认策略与上限 |
| Q-6 | 动态调档使用预期 | SetTaskParams 可调 priority（升档立即生效/降档立即撤销 + 触发续期）；确认助手是否会周期性升降档（影响豁免时长统计与续期次数消耗模型） |

## 3. TaskType（8 值初稿）

| 枚举 | 典型场景 |
|------|---------|
| GENERAL | 通用 |
| VOICE_PLAYBACK | 语音播报 |
| TIMER_TASK | 定时任务（锁屏场景核心） |
| DATA_PROCESSING | 数据处理/长时任务（切后台场景核心） |
| SYNC_TRANSFER | 同步/传输 |
| INTERACTION | 交互类（INPUT/AUTH 为主） |
| DOWNLOAD | 下载 |
| CUSTOM | 自定义（保留扩展位） |

**待确认问题：**

| # | 问题 | 说明 |
|---|------|------|
| Q-7 | 枚举值增删 | 是否需要补充（如 REMINDER/ALARM/CALL）或合并；TaskType 随 ControlRequest 透传 RSS，可驱动差异化管控策略（见 docs/extension-analysis.md A-3） |
| Q-8 | CUSTOM 的使用约定 | 若使用 CUSTOM，payload 中是否约定子类型字段（我方不解析，仅供 UI/RSS 展示与策略参考） |

## 4. 调用模式对齐（非枚举，一并确认）

| # | 事项 | 我方设计 | 需助手侧确认 |
|---|------|---------|-------------|
| Q-9 | **创建者进程自注册**（design §5.2.1） | 默认不自动纳管助手进程（控制面/被控面分离、防豁免滥用）；助手可显式 RegisterTaskProcess(taskId, ownPid, 自身 tokenId) 自注册（占单任务 16 进程名额）；二期备选 supervisor 弱保障档 | 是否需要？若助手退后台被冻结导致交互停摆，请提供场景描述（决定二期是否启动 supervisor 档） |
| Q-10 | **监听注册模式** | 助手注册 ITaskListener 后可收：状态回调 OnTaskStateChanged + 通知 OnTaskNotice（INPUT_ARRIVED / PAYLOAD_UPDATED / INTERACTIVE_TIMEOUT / EXEMPT_EXPIRING / EXEMPT_EXPIRED） | 确认助手会注册监听（INPUT_ARRIVED 实时收 input 的前提）；未注册时仅能 GetTaskInfo 轮询 |
| Q-11 | taskId 透传责任 | 助手在构造 Want 时显式携带 `ohos.ai.taskId`（string 参数）；链式调用（任务内进程再启动）由发起进程自行转发该参数 | 确认助手侧 Want 构造点可控；taskId 经 hisysevent（TASK_CREATE 等）留痕，按系统事件权限管控 |
| Q-12 | 交互超时行为 | INPUT/AUTH 态默认 30min 超时 → OnTaskNotice(INTERACTIVE_TIMEOUT, {deadline}) 限期处置（宽限 5min）→ 仍未流转则模块主动降档撤销豁免 + 告警 | 确认助手能响应 INTERACTIVE_TIMEOUT（重设状态/取消/续交互），否则任务将被降档 |

---
*对齐结论处理：Q-1~Q-8 结论回写 design.md §4.1（枚举冻结）；Q-9~Q-12 结论回写 design.md §5.2.1/§5.5。关联：review-analysis.md 开放项 O-1。*
