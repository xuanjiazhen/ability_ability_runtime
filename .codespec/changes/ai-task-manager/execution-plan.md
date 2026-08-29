# 执行计划：AI 语音助手任务管理机制

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-AI-TASK-MGR-001 |
| 关联 Feature/Bug | FEAT-AI-TASK-MGR-001 |
| 关联文档 | proposal.md（基线通过）/ design.md（Draft，随本计划评审）/ spec.md（Draft） |
| 复杂度 | 标准 |
| 状态 | Draft |
| Owner | yangzk |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved（基线通过，2026-08-21） |
| Design | `design.md` | Draft（随本计划一并评审） |
| Spec | `spec.md` | Draft（随本计划一并评审） |

## 受影响文件全量清单

> 对照 design.md 调用链层级分析（应用层客户端/服务层/平台能力），逐层列出具体文件。

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| ability_ability_runtime | inner API 客户端 | `interfaces/inner_api/app_manager/task_manager/ITaskManager.idl` | 新增 | IDL：接口方法 + TaskCreateParams/TaskInfo 数据结构 |
| ability_ability_runtime | inner API 客户端 | `interfaces/inner_api/app_manager/task_manager/include/task_manager_client.h` + `src/task_manager_client.cpp` | 新增 | C++ 客户端（AI 助手/监听方系统模块/AMS 调用入口，stub/proxy 封装，D-21） |
| ability_ability_runtime | inner API 客户端 | `interfaces/inner_api/app_manager/task_manager/include/task_listener.h` | 新增 | ITaskListener 回调接口（IRemoteBroker） |
| ability_ability_runtime | inner API 客户端 | `interfaces/inner_api/app_manager/task_manager/BUILD.gn` | 新增 | IDL 生成 + 客户端组件构建 |
| ability_ability_runtime | 服务层 | `services/appmgr/include/task_mgr/task_manager_service.h` + `src/task_mgr/task_manager_service.cpp` | 新增 | 任务管理入口：stub 实现、taskHandler、IPC 分发、接口鉴权 |
| ability_ability_runtime | 服务层 | `services/appmgr/include/task_mgr/task_record.h` | 新增 | 任务记录结构（4.3 节）与增删改查 |
| ability_ability_runtime | 服务层 | `services/appmgr/include/task_mgr/task_state_machine.h` + `src/task_mgr/task_state_machine.cpp` | 新增 | 状态机流转校验（4.2 节流转表） |
| ability_ability_runtime | 服务层 | `services/appmgr/include/task_mgr/task_listener_manager.h` + `src/task_mgr/task_listener_manager.cpp` | 新增 | 监听注册/注销/回调派发（DeathRecipient 清理） |
| ability_ability_runtime | 服务层 | `services/appmgr/include/task_mgr/task_dfx.h` + `src/task_mgr/task_dfx.cpp` | 新增 | hisysevent 上报、dump 输出 |
| ability_ability_runtime | 服务层 | `services/appmgr/include/process_schedule/process_schedule_manager.h` + `src/process_schedule/process_schedule_manager.cpp` | 新增 | 通用进程调度：双向索引、档位合并、管控动作施加/撤销（经 ResourceControlProxy）、内存登记（活跃任务豁免记录，不持久化） |
| ability_ability_runtime | 服务层 | `services/appmgr/include/process_schedule/process_schedule_policy.h` + `src/process_schedule/process_schedule_policy.cpp` | 新增 | 策略：优先级→档位映射、上限/时长配置加载 |
| ability_ability_runtime | 服务层 | `services/appmgr/include/process_schedule/process_schedule_types.h` | 新增 | 档位枚举、管控动作记录结构 |
| ability_ability_runtime | 服务层（RSS 适配层） | `services/appmgr/include/resource_control_proxy/resource_control_proxy.h` | 新增 | RSS Proxy 语义接口（豁免/解冻/查询/提权预留/ResetAll） |
| ability_ability_runtime | 服务层（RSS 适配层） | `services/appmgr/include/resource_control_proxy/resource_control_proxy_stub.h` + `src/resource_control_proxy/resource_control_proxy_stub.cpp` | 新增 | 一期骨架实现：内存登记 + 日志审计 + 告警 + 探测开关（代理模式） |
| ability_ability_runtime | 服务层（RSS 适配层） | `services/appmgr/include/resource_control_proxy/resource_control_proxy_impl.h` + `src/resource_control_proxy/resource_control_proxy_impl.cpp` | 新增（预留） | RSS 对接实现（RSS 团队明确后落地，条件编译二选一） |
| ability_ability_runtime | 服务层（修改） | `services/appmgr/BUILD.gn` | 修改 | 新增 task_mgr/process_schedule 源文件（ressched_client 依赖已有） |
| ability_ability_runtime | 服务层（修改） | `services/appmgr/src/app_mgr_service.cpp`（或 appmgr 主类 OnStart） | 修改 | 初始化 task_mgr 模块（最小挂点：创建 taskHandler/事件处理器） |
| ability_ability_runtime | 服务层（修改） | `services/appmgr/` 进程死亡通知挂点（复用 AppStateObserverManager/AppDeathRecipient 链） | 修改 | 进程死亡 → ProcessScheduleManager::OnProcessDied（最小挂点） |
| ability_ability_runtime | 服务层 | `services/abilitymgr/src/ability_manager_service.cpp` | 修改 | 拦截点①：StartAbilityInner 解析 `ohos.ai.taskId` → RegisterTaskIntent |
| ability_ability_runtime | 服务层 | `services/abilitymgr/src/ability_connect_manager.cpp` | 修改 | 拦截点①：ConnectAbility 收敛点解析 + 拦截点②（attach 处 pid 就绪 → RegisterTaskProcess） |
| ability_ability_runtime | 服务层 | `services/abilitymgr/BUILD.gn` | 修改 | 新增 task_manager_client 依赖 |
| ability_ability_runtime | 服务层 | `services/abilitymgr/include/`（拦截工具头文件，如 ability_start_utils.h 或新建拦截点辅助） | 修改/新增 | taskId 解析/上报辅助函数 |
| ability_ability_runtime | DFX/构建 | `hisysevent.yaml` | 修改 | 新增 TASK_CREATE/TASK_STATE_CHANGE/TASK_DESTROY/TASK_INPUT/TASK_RSSCONTROL/TASK_ABUSE_WARNING 6 事件 |
| ability_ability_runtime | 构建 | `ability_runtime.gni` | 修改 | 新增特性开关 `ability_runtime_ai_task_manager_enable`（默认 true） |
| ability_ability_runtime | 构建/配置 | `services/appmgr/etc/taskmanager.para`（新增） | 新增 | 可调参数定义（上限/时长/续期次数/探测开关，命名 `persist.sys.appms.taskmanager.*`（F-37 修正：const.* 启动后不可变，运行时可调须 persist.*，先例 persist.sys.abilityms.*）；sysparam 体系，F-14 补全；随 BUILD.gn 安装） |
| ability_ability_runtime | 服务层（修改） | `services/abilitymgr/include/ability_record.h` + `src/ability_record.cpp` | 修改 | **新增业务发起方 taskId/tokenId 存储（F-31：意向登记写入、attach 读取）** |
| ability_ability_runtime | 依赖 | `services/appmgr/BUILD.gn` 新增外部依赖 `device_standby:standby_innerkits` | 修改 | **P7 EDS 联动前置（F-30：appmgr 现无 device_standby 依赖；native 调用免权限）** |
| ability_ability_runtime | 错误码 | `interfaces/inner_api/error_utils/include/ability_runtime_error_util.h`（或任务域新头文件） | 修改 | 新增任务域错误码段（5 个 + 复用 ERR_INVALID_PARAM） |
| ability_ability_runtime | 测试 | `test/unittest/task_mgr_test/`（新增，F-27：扁平命名与仓惯例/套件名一致） | 新增 | 任务管理单测（状态机/生命周期/权限/并发/档位合并/防仿冒） |
| ability_ability_runtime | 测试 | `test/unittest/task_intercept_test/`（新增拦截相关用例，F-27） | 新增 | 拦截点纳管集成用例（两段式） |

**检查项：**
- [x] design.md 调用链每一层都有对应文件列出（客户端/服务层 task_mgr/process_schedule/AMS 拦截/DFX/测试）
- [x] 每个文件修改类型和职责说明明确
- [x] 平台能力层（RSS/qos）无本仓文件（复用接口，RSS owner 评审）

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式 | 覆盖？ |
|----|------|------|----------|--------|
| AC-1（AC-1.1~AC-1.6） | spec.md | TASK-1, TASK-2 | 单测 | 是 |
| AC-2（AC-2.1~AC-2.5） | spec.md | TASK-3 | 单测+集成 | 是 |
| AC-3（AC-3.1~AC-3.4） | spec.md | TASK-2 | 单测 | 是 |
| AC-4（AC-4.1~AC-4.3） | spec.md | TASK-2 | 单测+集成 | 是（AC-4.4 随 D-21 移除） |
| AC-5（AC-5.1~AC-5.5） | spec.md | TASK-4 | 单测+集成+真机 | 是（真机验证在 TASK-6 门禁） |
| AC-6（AC-6.1~AC-6.2） | spec.md | TASK-5 | 人工+集成 | 是 |

## 首批实现边界

**首批必须实现：** 任务模型与状态机（TASK-1）、监听与交互（TASK-2）、拦截纳管+防仿冒（TASK-3）、冻结豁免/降级（TASK-4）、DFX（TASK-5）、单测与集成（各 Task 自带）
**可后置：** QoS 提权（RSS 221/QosController）、低内存查杀 520、线程级 RTG、真机锁屏场景全量验证（TASK-6 门禁）
**不建议延后：** 冻结豁免/降级（需求核心痛点，不得后置）、B-2 防仿冒校验（安全底线）

## 阶段计划（如适用）

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 | 骨架接线 | TASK-0 | IDL/模块/BUILD 编译通过 | `./build.sh --build-target libappms` |
| Phase-2 | 核心能力 | TASK-1~TASK-3 | 任务生命周期+纳管+防仿冒单测通过 | `run -t UT -ts task_mgr_test` |
| Phase-3 | 管控与 DFX | TASK-4~TASK-5 | 豁免/降级集成用例+事件检索通过 | `run -t UT -ts task_mgr_test` + 事件检索 |
| Phase-4 | 验证收口 | TASK-6 | 全部 AC 用例通过；真机锁屏/后台场景验证报告 | `run -t UT -tp ability_runtime` + 真机 |

## Task 粒度原则

- 每个 Task 对应一个可独立验收的最小能力闭环（按"能力闭环"而非代码行数划分）
- 状态机、监听交互、拦截纳管、RSS 管控、DFX 各自独立闭环 → 拆分 TASK-1~5
- 骨架接线独立（IDL/BUILD）→ TASK-0
- 真机验证与 AC 门禁独立 → TASK-6

## 禁止项

- [x] 无 TBD/TODO/占位符
- [x] 无"根据需要实现"等模糊指令
- [x] 无跨 Task 隐式依赖（前置依赖列显式声明）
- [x] 无要求 Agent 自行寻找未列出上下文文件
- [x] 每个 AC 有验证方式
- [x] 无"参考 Task-N 实现"类引用（每个 Task 自包含，Task 详情含直接 AC 与规则）

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 规模预估 |
|---------|------|----------|---------|----------|----------|----------|----------|
| TASK-0 | 骨架接线：IDL/客户端/模块目录/BUILD | `interfaces/inner_api/app_manager/task_manager/*`、`services/appmgr/BUILD.gn`、gni、error_utils | —（骨架） | 无 | 编译通过，stub/proxy/客户端生成 | `./build.sh --build-target libappms` | 生产 ~1,000 行（IDL 150 + stub 分发 300 + 客户端 350 + listener 60 + BUILD/gni/error 130）；无单测 |
| TASK-1 | 任务模型与状态机：CreateTask/SetTaskState/DestroyTask/CancelTask/SetTaskParams + 生命周期 | task_record、task_state_machine、task_manager_service（核心路径）、单测 | AC-1.1~AC-1.6 | TASK-0 | 单测覆盖合法/非法流转、权限、上限 | `run -t UT -ts task_mgr_test` | 生产 ~1,300（record 300 + state_machine 200 + service 800）；单测 ~800 |
| TASK-2 | 查询/监听：GetTaskInfo(s)/RegisterTaskListener/payload 事实记录与回调（**交互经 AI bar 内化，D-21：~~SubmitTaskInput/INPUT_ARRIVED/白名单传递~~移除**；监听消费方=安全等系统模块） | task_listener_manager、task_manager_service、单测 | AC-3.1~AC-3.4, AC-4.1~AC-4.3 | TASK-1 | 单测覆盖回调顺序/负载/交互态 payload 留存与回调/注销停止 | `run -t UT -ts task_mgr_test` | 生产 ~700（listener_mgr 400 + service 增量 300）；单测 ~550 |
| TASK-3 | 拦截纳管+防仿冒：**条件两段式**（进程已存在直接纳管/未创建意向+attach）、B-1 随机 taskId、B-2 caller 校验（含代理启动身份规则） | ability_manager_service.cpp、ability_connect_manager.cpp、task_manager_service（RegisterTaskIntent/Process）、集成用例 | AC-2.1~AC-2.5 | TASK-1 | 集成用例覆盖四链路（Start/Connect/意图/Cli）+ 代理启动变体（AsCaller/WithSpecifyTokenId）+ **已存在进程场景** + 仿冒拒绝；无 taskId 零影响回归 | `run -t UT -ts ability_manager_service_first_test` | 生产 ~950（AMS 拦截点 350 + 意向表/B-2 校验 500 + 死亡挂点 50 + 工具 50）；集成用例 ~900 |
| TASK-4 | 管控与 ResourceControlProxy：档位合并、经代理施加/撤销豁免（一期骨架）、**NotifyTaskEnd 任务级清理通知**、降级恢复、上限防滥用（sysparam 参数化，F-14）、续期与到期处置（仅 priority 变化触发、N=3 默认可配）、进程死亡清理 | process_schedule_*、resource_control_proxy_*、task_mgr 联动、单测+集成 | AC-5.1~AC-5.5 | TASK-1, TASK-3 | 单测覆盖档位合并/降级/幂等/动作日志断言（含 NotifyTaskEnd 双通道时序、续期次数上限）；集成覆盖动作序列 | `run -t UT -ts task_mgr_test` | 生产 ~1,700（schedule_mgr 700 + policy 200 + types 80 + RCP 接口 100 + stub 300 + impl 占位 150 + 联动 200）；单测 ~1,000 |
| TASK-5 | DFX：hisysevent 6 事件、dump、日志、错误码接入 | hisysevent.yaml、task_dfx、dump 入口 | AC-6.1~AC-6.2 | TASK-1~TASK-4 | 事件上报可检索；dump 输出任务全景 | 事件检索 + `hidumper -a 501` | 生产 ~630（task_dfx 400 + yaml 80 + 接入 150）；验证用例 ~300 |
| TASK-6 | 验证收口：全量 AC 用例、真机锁屏/后台场景验证（锁屏豁免边界与 RSS owner 对齐；**前置（P1 代码勘察 2026-08-24）：产品含 freezer 执行插件（启用开关以产品配置为准，F-23 勘察修正：该参数在开源 master 不存在）——开源 master 无 505/517/520 消费者，端到端验证会假成功**） | test/ 全量 + 真机 | 全部 AC | TASK-1~TASK-5；RSS 对接（外部依赖）；真机端到端另需产品 freezer 环境 | 全量单测/集成通过；真机验证报告（锁屏定时任务、后台长时任务，须记录产品 freezer/开关配置）；master 上以 Stub 动作日志断言为基线 | `run -t UT -tp ability_runtime` | 用例补强 ~700（真机脚本/集成补强） |

> 规模预估：C++ 行数（含头文件），±20% 误差；IDL 生成物（stub/proxy 本体）由构建生成不计入；不含 RSS 真实对接（ResourceControlProxyImpl 落地，RSS 团队明确后另估 ~300~500 行）与二期扩展（QoS 提权 221/低内存查杀 520/线程级 RTG/NAPI）。合计：生产 ~6,400 行 + 单测 ~4,150 行 ≈ 1.05 万行（D-21 后 TASK-2 缩减 ~200 生产 + ~150 单测）；单人开发约 4~6 周（骨架 1~2 天 → 核心 5~7 天 → 拦截 3~4 天 → 管控 4~5 天 → DFX 2 天 → 联调/真机 3~5 天，单测贯穿）。

## Task 详情

### TASK-0: 骨架接线

| 字段 | 内容 |
|------|------|
| 任务目标 | IDL 定义 + stub/proxy 生成 + TaskManagerClient/TaskListener + **SA 501 描述符路由承载（F-21：AppMgrService 覆写 OnRemoteRequest，ITaskManager 描述符请求委托 TaskManagerService 桩，IAppMgr 行为零改动；design §5.2.2）** + appmgr BUILD.gn/gni/error_utils 接入，编译通过 |
| 规模预估 | 生产 ~1,000 行（IDL 150 + stub 分发 300 + 客户端 350 + listener 60 + BUILD/gni/error 130）；无单测 |
| AC 映射 | 骨架（对应 AC-1.1 可调用的接口存在） |
| 前置依赖 | design.md、spec.md 评审通过 |
| 非目标 | 无业务逻辑 |
| 完成判据 | `./build.sh --build-target libappms` 编译通过；stub/proxy/客户端头文件生成；**ITaskManager 代理经 SA 501 可达 TaskManagerService（最小 IPC 用例，F-21）** |
| 停止条件 | IDL 编译错误无法定位；接口结构与 design 不一致需先回 design 评审 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 新增 | `interfaces/inner_api/app_manager/task_manager/ITaskManager.idl` | 12 个方法（~~SubmitTaskInput~~随 D-21 移除）+ 3 个 struct（TaskCreateParams/TaskParams/TaskInfo）+ ITaskListener 回调接口（task_listener.h，IRemoteBroker 手写，design 5.2 节；F-19 修正） |
| 新增 | `interfaces/inner_api/app_manager/task_manager/include/task_manager_client.h` + `src/task_manager_client.cpp` | 客户端（含 AMS 内部调用入口；GetSystemAbility(501)+iface_cast<ITaskManager>，design §5.2.2） |
| 新增 | `interfaces/inner_api/app_manager/task_manager/include/task_listener.h` | ITaskListener 回调 |
| 新增 | `interfaces/inner_api/app_manager/task_manager/BUILD.gn` | IDL 生成 + 客户端组件 |
| 修改 | `services/appmgr/src/app_mgr_service.cpp` + `include/app_mgr_service.h` | **OnRemoteRequest 描述符路由（F-21：ITaskManager 描述符 → TaskManagerService 桩分发；IAppMgr 零改动）** |
| 修改 | `services/appmgr/BUILD.gn` | 接入新组件依赖 |
| 修改 | `ability_runtime.gni` | 新增 `ability_runtime_ai_task_manager_enable`（默认 true） |
| 修改 | `interfaces/inner_api/error_utils/include/ability_runtime_error_util.h` | 任务域错误码段 |

### TASK-1: 任务模型与状态机

| 字段 | 内容 |
|------|------|
| 任务目标 | 任务创建/销毁/取消/状态设置/参数设置的最小能力闭环：TaskRecord 存储、状态机流转校验（design 4.2 表）、创建者身份绑定、随机 taskId（B-1）、任务数上限（R-2）、终态不可回退（R-4）、REJECTED 仅创建者（R-6） |
| 规模预估 | 生产 ~1,300（record 300 + state_machine 200 + service 800）；单测 ~800 |
| AC 映射 | AC-1.1, AC-1.2, AC-1.3, AC-1.4, AC-1.5, AC-1.6 |
| 前置依赖 | TASK-0 |
| 非目标 | 进程纳管（TASK-3）、RSS 动作（TASK-4）、监听回调（TASK-2）——本 Task 状态变更仅更新 TaskRecord |
| 完成判据 | task_mgr_test 覆盖：合法/非法流转矩阵、终态回退、REJECTED 权限、任务上限、随机 taskId 唯一性 |
| 停止条件 | 状态机语义与需求 2 冲突（如流转表判定不合理）→ 回 design 评审 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 新增 | `services/appmgr/include/task_mgr/task_record.h` + `src/task_mgr/task_record.cpp` | TaskRecord 存储与查询 |
| 新增 | `services/appmgr/include/task_mgr/task_state_machine.h` + `src/task_mgr/task_state_machine.cpp` | CanTransition + 创建者校验 |
| 新增 | `services/appmgr/include/task_mgr/task_manager_service.h` + `src/task_mgr/task_manager_service.cpp` | stub 实现：CreateTask/DestroyTask/CancelTask/SetTaskState/SetTaskParams；系统应用鉴权；taskHandler 串行 |
| 新增 | `test/unittest/task_mgr_test/task_manager_service_test.cpp` 等（F-27） | 单测 |

### TASK-2: 查询/监听/交互

| 字段 | 内容 |
|------|------|
| 任务目标 | GetTaskInfo(s)/RegisterTaskListener/UnregisterTaskListener 闭环：监听注册表与回调派发（顺序一致 R-14）、交互态 payload 事实记录与回调（R-16/R-18，D-21：交互经 AI bar 闭环，无回传接口）、监听者死亡清理、通知类事件派发（INTERACTIVE_TIMEOUT/EXEMPT_EXPIRING/EXEMPT_EXPIRED）；payload 权限管控（监听方=安全等系统模块） |
| 规模预估 | 生产 ~900（listener_mgr 400 + service 增量 500）；单测 ~700 |
| AC 映射 | AC-3.1, AC-3.2, AC-3.3, AC-3.4, AC-4.1, AC-4.2, AC-4.3（AC-4.4 随 D-21 移除） |
| 前置依赖 | TASK-1 |
| 非目标 | RSS 管控联动（TASK-4）——本 Task 交互状态流转仅更新状态与 payload |
| 完成判据 | 单测覆盖回调顺序/注销停止/查询快照/交互态 payload 留存与回调/交互清理（R-18）/权限拒绝 |
| 停止条件 | 回调跨进程序列化异常无法定位 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 新增 | `services/appmgr/include/task_mgr/task_listener_manager.h` + `src/task_mgr/task_listener_manager.cpp` | 注册表/派发/DeathRecipient |
| 修改 | `services/appmgr/src/task_mgr/task_manager_service.cpp` | GetTaskInfo(s)/Register/Unregister 实现（~~SubmitTaskInput~~随 D-21 移除） |
| 新增 | `test/unittest/task_mgr_test/task_listener_test.cpp`（F-27） | 单测 |

### TASK-3: 拦截纳管与防仿冒

| 字段 | 内容 |
|------|------|
| 任务目标 | AMS 启动/连接链路拦截：解析 `ohos.ai.taskId` → 两段式（RegisterTaskIntent 意向 + attach 后 RegisterTaskProcess）；**拦截实现采用混合方案（F-36，design §5.4.1a，待评审确认 A-8）：新建 TaskInterceptor（IAbilityInterceptor，恒返 ERR_OK）经 AddInterceptor 注册覆盖 4 收敛点（StartAbilityInner:1490 / StartExtensionAbilityInner:4712 / StartUIExtensionAbility:5100 / ConnectAbilityCommon:6135）+ RequestModalUIExtension 单点 ad-hoc；需扩展 AbilityInterceptorParam 增加 callerTokenId 字段（纯增字段）**；B-2 caller 身份校验（creatorTokenId ∪ 纳管进程 tokenId）+ 通道鉴权（F-8：**AMS 侧调用统一 IN_PROCESS_CALL 包装 + AppMS 侧 CheckSpecificSystemAbilityAccessPermission(FOUNDATION_PROCESS_NAME) 判定，F-29**）；**UIExtension 路径身份取 SessionInfo.callerToken_/callingTokenId_（F-32）**；**AbilityRecord 新增业务发起方 tokenId 存储（F-31）**；意向超时清理（attach 晚于超时直接补发②，F-39）；无 taskId 零开销（R-13）；仿冒拒绝不阻断启动（R-8） |
| 规模预估 | 生产 ~1,050（AMS 拦截点 400 + 意向表/B-2/通道鉴权 500 + 死亡挂点 50 + 工具 50 + AbilityRecord 字段 50，F-29/F-31 增量）；集成用例 ~1,000 |
| AC 映射 | AC-2.1, AC-2.2, AC-2.3, AC-2.4, AC-2.5 |
| 前置依赖 | TASK-1 |
| 非目标 | RSS 动作施加（TASK-4）——本 Task 纳管仅更新进程集合 |
| 完成判据 | 集成用例覆盖 StartAbility/ConnectAbility 纳管成功、仿冒拒绝、链式启动合法、进程死亡移除（挂点）；**通道鉴权用例（F-8/F-29）：非 foundation native 直调自报身份拒绝、pid 归属不符拒绝、attach/代理启动路径纳管成功（身份重置生效）**；无 taskId 回归用例通过 |
| 停止条件 | 拦截点位置与 AMS 架构不符（如启动早期无法取 callerTokenId）→ 回 design 评审 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 新增 | `services/abilitymgr/include/interceptor/task_interceptor.h` + `src/interceptor/task_interceptor.cpp` | **TaskInterceptor（F-36：IAbilityInterceptor 实现，恒返 ERR_OK；Want 解析+意向登记+既有 record 自查）** |
| 修改 | `services/abilitymgr/include/interceptor/ability_interceptor_interface.h` | **AbilityInterceptorParam 增加 callerTokenId 字段（F-36：纯增字段，既有拦截器不受影响）** |
| 修改 | `services/abilitymgr/src/ability_manager_service.cpp` | TaskInterceptor 注册（AddInterceptor）+ RequestModalUIExtensionInner 单点 ad-hoc 挂接（F-36）；**TaskManagerClient 调用统一 IN_PROCESS_CALL 包装（F-29）** |
| 修改 | `services/abilitymgr/src/ability_connect_manager.cpp` | attach 拦截点②（pid 就绪 → RegisterTaskProcess） |
| 修改 | `services/abilitymgr/include/ability_record.h` + `src/ability_record.cpp` | **新增业务发起方 taskId/tokenId 存储（F-31，意向登记时随 want 解析写入，attach 时读取）** |
| 修改 | `services/appmgr/src/task_mgr/task_manager_service.cpp` | RegisterTaskIntent/RegisterTaskProcess/UnregisterTaskProcess + B-2 校验 + 通道鉴权（F-8/F-29：CheckSpecificSystemAbilityAccessPermission 原语） |
| 修改 | `services/appmgr/` 进程死亡通知挂点 | OnProcessDied 接入 |
| 新增 | `test/unittest/task_intercept_test/`（F-27：扁平命名，与套件名一致） | 集成用例（含 UIExtension Want 参数 taskId 存活验证，F-41） |

### TASK-4: RSS 管控（资源调度子模块）

| 字段 | 内容 |
|------|------|
| 任务目标 | process_schedule 闭环：进程组双向索引、档位合并（max）、ApplyLevel 增量施加/撤销（经 ResourceControlProxy）、任务结束降级（剩余任务最高档）、上限防滥用（R-22）、幂等（豁免引用计数按 pid 维护于 process_schedule，F-15 修正）；**ResourceControlProxy 代理层**：语义接口（ControlAction/ControlRequest）+ 一期骨架实现（内存登记 + TASK_RSSCONTROL 日志/告警 + 探测开关 resource_control_proxy_probe_enable 默认关）+ 对接实现占位（条件编译）；**续期机制**（仅 priority 变化触发、次数上限 N=3 默认可配）与各项上限参数化（sysparam 落地：`.para` 定义 + GetParameter 读取，design §6.1 F-14；评审决策 2026-08-24，默认值 2026-08-26 第十三轮确认） |
| 规模预估 | 生产 ~1,700（schedule_mgr 700 + policy 200 + types 80 + RCP 接口 100 + stub 300 + impl 占位 150 + 联动 200）；单测 ~1,000 |
| AC 映射 | AC-5.1, AC-5.2, AC-5.3, AC-5.4, AC-5.5 |
| 前置依赖 | TASK-1, TASK-3 |
| 非目标 | RSS 真实对接（RSS 团队明确后由 ResourceControlProxyImpl 承接，本 Task 交付骨架）；二期扩展（221/QosController/520）——仅预留接口与档位扩展位 |
| 完成判据 | 单测覆盖档位合并矩阵/降级/幂等/上限/续期次数上限；集成覆盖动作序列（动作日志断言：施加→撤销次序、重试 ≤3）；ResourceControlProxyStub 骨架编译/单测通过 |
| 停止条件 | RSS 设计明确且与语义接口不兼容（接口需调整）→ 回 design 评审更新 ResourceControlProxy 接口 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 新增 | `services/appmgr/include/process_schedule/process_schedule_manager.h` + `src/process_schedule/process_schedule_manager.cpp` | 索引/档位/管控动作（经 ResourceControlProxy）/内存登记 |
| 新增 | `services/appmgr/include/process_schedule/process_schedule_policy.h` + `src/process_schedule/process_schedule_policy.cpp` | 优先级→档位映射、上限/时长配置 |
| 新增 | `services/appmgr/include/process_schedule/process_schedule_types.h` | 档位/动作记录 |
| 新增 | `services/appmgr/etc/taskmanager.para` | 可调参数定义（sysparam，F-14/F-37：上限/时长/续期次数/探测开关，命名 `persist.sys.appms.taskmanager.*`；随 BUILD.gn 安装） |
| 修改 | `services/appmgr/BUILD.gn`（依赖段） | **新增 `device_standby:standby_innerkits` 外部依赖（F-30：P7 EDS 联动前置，appmgr 现无该依赖）** |
| 新增 | `services/appmgr/include/resource_control_proxy/resource_control_proxy.h` | ResourceControlProxy 语义接口 |
| 新增 | `services/appmgr/include/resource_control_proxy/resource_control_proxy_stub.h` + `src/resource_control_proxy/resource_control_proxy_stub.cpp` | 一期骨架（内存登记+日志+告警+探测开关） |
| 新增 | `services/appmgr/include/resource_control_proxy/resource_control_proxy_impl.h` + `src/resource_control_proxy/resource_control_proxy_impl.cpp` | 对接实现占位（条件编译，RSS 明确后落地） |
| 修改 | `services/appmgr/src/task_mgr/task_manager_service.cpp` | 状态/生命周期事件联动 process_schedule |
| 新增 | `test/unittest/task_mgr_test/process_schedule_test.cpp`（F-27） | 单测（动作日志断言 + 桩 ResourceControlProxy） |

### TASK-5: DFX 与错误码

| 字段 | 内容 |
|------|------|
| 任务目标 | hisysevent 6 事件定义与上报接入、任务 dump（hidumper -a 501）、hilog 日志（hilog_tag_wrapper）、错误码段落地 |
| 规模预估 | 生产 ~630（task_dfx 400 + yaml 80 + 接入 150）；验证用例 ~300 |
| AC 映射 | AC-6.1, AC-6.2 |
| 前置依赖 | TASK-1~TASK-4 |
| 非目标 | AI bar 侧呈现（AI 助手自有，D-21）；监听方业务消费（安全等模块自行实现） |
| 完成判据 | 事件上报可检索（hievent 工具）；dump 输出任务全景；既有事件未被破坏（AGENTS.md 约束） |
| 停止条件 | hisysevent 事件定义评审未过 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 修改 | `hisysevent.yaml` | 新增 6 事件（AAFWK 域，design 7.1 节） |
| 新增 | `services/appmgr/include/task_mgr/task_dfx.h` + `src/task_mgr/task_dfx.cpp` | 上报/dump 实现 |
| 修改 | `services/appmgr/src/task_mgr/task_manager_service.cpp` | 各操作点接入事件上报与日志 |
| 修改 | `interfaces/inner_api/error_utils/include/ability_runtime_error_util.h` | 错误码落地（design 8 节） |

### TASK-6: 验证收口

| 字段 | 内容 |
|------|------|
| 任务目标 | 全量 AC 用例执行；真机场景验证：锁屏定时任务执行成功、后台长时任务不被冻结、任务结束后进程恢复原调度；**依赖 RSS 对接完成（ResourceControlProxyImpl 落地 + RSS 团队评审），对接前以 ResourceControlProxyStub 动作日志断言作为模块级验证基线**；**真机端到端前置条件（P1 代码勘察 2026-08-24）**：目标产品须含 freezer 执行插件（启用开关以产品配置为准，F-23）——开源 master 上 505/517/520 无消费插件、507 恒返回未冻结，端到端豁免验证会"假成功"，master 环境仅能以动作日志断言为基线 |
| 规模预估 | 用例补强 ~700（真机脚本/集成补强） |
| AC 映射 | 全部 AC（AC-1.1~AC-6.2） |
| 前置依赖 | TASK-1~TASK-5；RSS 对接（ResourceControlProxyImpl，外部依赖）；真机端到端另需产品镜像含 freezer 执行插件（启用开关以产品配置为准，F-23） |
| 非目标 | 二期能力（QoS 提权/520/线程级） |
| 完成判据 | `run -t UT -tp ability_runtime` 通过；真机验证报告含锁屏/后台场景结果（对接完成后）或模块级动作日志断言结果（对接前，标注开放项）；**真机报告须记录所用产品形态的 freezer 执行插件与启用开关配置（防 master 环境假成功）**；RSS 对齐结论记录 |
| 停止条件 | RSS 对接未就绪且真机场景不可达 → 输出模块级验证结论与对接待办清单 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| 新增 | `test/` 全量用例补充与整理 | 覆盖矩阵 |
| 新增 | 验证报告（`docs/` 或评审附件） | 真机场景结果、RSS 对齐结论 |
