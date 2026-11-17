# 执行计划

> 将 Approved Spec 拆成可执行、可验证、可审查的闭环 Task。AC→Task 的唯一事实源为本文“AC 到 Task 追溯”；实现后回填实际范围、验证证据和偏差。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-20260914692272 |
| 关联 Feature/Bug | FEAT-AI-TASK-MGR-001（RR 30096095） |
| 关联文档 | proposal.md / spec.md / design.md |
| 复杂度 | 标准 |
| 状态 | Approved（yangzk 授权连续完成实现与检视，2026-11-16） |
| Owner | yangzk |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved |
| Design | `design.md` | Approved |
| Spec | `spec.md` | Approved |

## 执行原则

- **Spec 权威：** 实现与 AC、兼容性声明冲突时，停止并修订 spec/design/plan，不在代码里自行改语义。
- **测试/证据先行：** 先运行现有测试并补会失败的新用例，证明当前缺失能力，再写最小实现；不得用已经满足的行为冒充 Red。
- **单一闭环：** uid 缓存、受管集合、主类型、单串行队列、尾随窗口和空代理共享同一最小验证闭环，使用一张 Task Card，避免并行改动互相破坏。
- **文件边界：** 只能修改 Files 表列出的文件；新增依赖或文件必须先修订 Plan。
- **状态所有权唯一：** STATE-1～STATE-4 的 owner、key/index、创建、读取、更新和清理必须与 design 一致。
- **证据回填：** 完成当次会话回填追溯状态、实际文件、Review Evidence 和 Actual Result，不跨会话补证。
- **反伪完成：** 只写结构、只跑既有测试或只填 PASS 文本都不能替代 AC 闭环。
- **生成文件：** 无 IDL/bridge/CAPI 生成物；所有代码均手写并直接编译，不修改生成文件。

## 受影响文件全量清单

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| ability_ability_runtime | 任务生命周期服务 | `services/appmgr/include/task_mgr/task_manager_service.h` | 修改 | uid resolver、恢复回写和原因传递声明 |
| ability_ability_runtime | 任务生命周期服务 | `services/appmgr/src/task_mgr/task_manager_service.cpp` | 修改 | uid 查询缓存、重试、死亡优先和完整事件提交 |
| ability_ability_runtime | 归因数据模型 | `services/appmgr/include/task_mgr/attribution_record.h` | 修改 | 新增 uid 缓存及 VIRTUAL_SCREEN_ABILITY 枚举 |
| ability_ability_runtime | 归因数据模型 | `services/appmgr/src/task_mgr/attribution_record.cpp` | 修改 | 新枚举字符串映射 |
| ability_ability_runtime | 进程归属调度 | `services/appmgr/include/task_mgr/process_schedule.h` | 修改 | 受管上下文、候选事件、主类型、单队列入口声明 |
| ability_ability_runtime | 进程归属调度 | `services/appmgr/src/task_mgr/process_schedule.cpp` | 修改 | 受管过滤、工具引用、max/主类型、快照与事件提交 |
| ability_ability_runtime | 调度出口与防抖 | `services/appmgr/include/task_mgr/resource_control_proxy.h` | 新增 | Apply/Restore/EmitEvent、EventProvider、pending/lastNotified 声明 |
| ability_ability_runtime | 调度出口与防抖 | `services/appmgr/src/task_mgr/resource_control_proxy.cpp` | 新增 | 固定 1000ms 尾随窗口、epoch、空实现日志 |
| ability_ability_runtime | 构建 | `services/appmgr/BUILD.gn` | 修改 | 接入 resource_control_proxy.cpp，不新增外部依赖 |
| ability_ability_runtime | 单元测试 | `test/unittest/process_schedule_test/process_schedule_test.cpp` | 修改 | 受管字段、动作、主类型、uid、死亡和兼容矩阵 |
| ability_ability_runtime | 单元测试 | `test/unittest/process_schedule_test/resource_control_proxy_test.cpp` | 新增 | 尾随窗口、陈旧回调、失败恢复、EmitEvent 无递归 |
| ability_ability_runtime | 单元测试构建 | `test/unittest/process_schedule_test/BUILD.gn` | 修改 | 接入代理实现和新增测试源文件 |
| ability_ability_runtime | 单元测试 | `test/unittest/task_manager_service_test/task_manager_service_test.cpp` | 修改 | uid 缓存/重试、创建者自纳管死亡及既有回归适配 |
| ability_ability_runtime | 单元测试构建 | `test/unittest/task_manager_service_test/BUILD.gn` | 修改（如现有 target 需要） | 补齐新增代理实现与重试测试依赖 |
| ability_ability_runtime | 单元测试构建 | `test/unittest/{ams_service_app_spawn_client_test,ams_service_event_drive_test,cache_process_manager_second_test,app_mgr_service_inner_seventh_test,app_mgr_service_event_handler_test,ams_service_startup_test}/BUILD.gn` | 修改 | 直接聚合 process_schedule.cpp 的既有目标同步接入 resource_control_proxy.cpp |
| ability_ability_runtime | 单元测试构建 | `test/unittest/{ams_app_workflow_test,app_mgr_service_inner_eighth_test,ams_recent_app_list_test,app_mgr_service_inner_ninth_test,app_mgr_service_inner_tenth_test,ams_service_load_ability_process_test}/BUILD.gn`、`test/moduletest/common/ams/BUILD.gn` | 修改 | 直接聚合 process_schedule.cpp 的其余既有目标同步接入 resource_control_proxy.cpp |
| ability_ability_runtime | SDD 交付件 | `.codespec/changes/issue-20260914692272-task-rss-tiered-control/proposal.md` | 修改 | 范围与审批记录 |
| ability_ability_runtime | SDD 交付件 | `.codespec/changes/issue-20260914692272-task-rss-tiered-control/spec.md` | 修改 | AC、规则和验证映射 |
| ability_ability_runtime | SDD 交付件 | `.codespec/changes/issue-20260914692272-task-rss-tiered-control/design.md` | 修改 | 架构与状态设计 |
| ability_ability_runtime | SDD 交付件 | `.codespec/changes/issue-20260914692272-task-rss-tiered-control/execution-plan.md` | 修改 | 本计划与实施证据 |

**检查项：**
- [x] design 调用链每层均有具体文件。
- [x] 每个文件修改类型和职责明确，无越界仓或模块。
- [x] 无生成器产物需要手工修改。

## AC 到 Task 追溯

| AC | 来源 | Task | 计划代码范围 | 实际 Code Ref | Commit | Review Evidence | 验证状态 |
|----|------|------|--------------|-----------------|--------|-----------------|----------|
| AC-5.1 | spec.md US-1 / R-1 | TASK-1 | process_schedule.*、attribution_record.*、task_manager_service.* | 已实现（当前工作区） | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/`（已生成） | Blocked（运行验证未闭合） |
| AC-5.2 | spec.md US-1 / R-1 | TASK-1 | process_schedule.*、attribution_record.* | 已实现（当前工作区） | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/`（已生成） | Blocked（运行验证未闭合） |
| AC-5.3 | spec.md US-1 / R-2 | TASK-1 | process_schedule.*、resource_control_proxy.* | 已实现（当前工作区） | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/`（已生成） | Blocked（运行验证未闭合） |
| AC-5.4 | spec.md US-2 / R-3 | TASK-1 | process_schedule.*、resource_control_proxy.*、task_manager_service.* | 已实现（当前工作区） | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/`（已生成） | Blocked（运行验证未闭合） |
| AC-5.5 | spec.md US-2 / R-4 | TASK-1 | resource_control_proxy.*、process_schedule_test/* | 已实现（当前工作区） | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/`（已生成） | Blocked（运行验证未闭合） |
| AC-5.6 | spec.md US-2 / R-5 | TASK-1 | resource_control_proxy.*、process_schedule_test/* | 已实现（当前工作区） | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/`（已生成） | Blocked（运行验证未闭合） |
| AC-5.7 | spec.md US-2 / R-6 | TASK-1 | process_schedule.*、resource_control_proxy.*、task_manager_service.* | 已实现（当前工作区） | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/`（已生成） | Blocked（运行验证未闭合） |
| AC-5.8 | spec.md US-1 / R-7 | TASK-1 | resource_control_proxy.*、resource_control_proxy_test.cpp | 已实现（当前工作区） | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/`（已生成） | Blocked（运行验证未闭合） |
| AC-5.9 | spec.md US-3 / R-8 | TASK-1 | process_schedule.*、task_manager_service_test.cpp、resource_control_proxy.* | 已实现（当前工作区） | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/`（已生成） | Blocked（运行验证未闭合） |
| AC-5.10 | spec.md US-1 / R-9 | TASK-1 | attribution_record.*、task_manager_service.*、process_schedule_test/* | 已实现（当前工作区） | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/`（已生成） | Blocked（运行验证未闭合） |

## 首批实现边界

**首批必须实现：** 全部受管行为、uid 失败闭环、单队列顺序、主类型和空代理；不得留下真实 RSS 调用。
**可后置：** 新类型实际纳管通道、真实 RSS 策略及性能功耗端到端验收，均由后续需求承担。
**不建议延后：** 受管过滤、工具引用、主类型、死亡优先、epoch 防陈旧和测试 Red，否则主链不闭合。

## 阶段计划（如适用）

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 | 数据模型与 Red | TASK-1 | 新增用例能证明当前缺失字段、过滤、动作和窗口，既有用例基线可运行 | `process_schedule_test` 聚焦 Red |
| Phase-2 | 单队列完整实现 | TASK-1 | AC-5.1～AC-5.10 用例转 Green，无真实 RSS 调用 | `ability_runtime_test` 全量编译及聚焦用例 |
| Phase-3 | 回归与代码检视 | TASK-1 | 相关回归无新增失败，spec/plan 追溯和代码检视证据齐全 | 最终构建、静态检查和 review 记录 |

## Task 粒度原则

- uid、受管集合、单队列、主类型、防抖和空代理共享同一最小验证闭环，合并为一张 Task Card。
- 本 Task 不拆分出需要相互等待的并行子任务；Task 内按数据模型→队列→代理→任务链集成顺序执行。
- 预计实现和测试小于 3000 行，允许单 Task；若实际范围超界或新增真实 RSS 依赖，先停止并修订 Plan。

## 禁止项

执行计划和 Task 不得出现以下内容：

- [x] 禁止项均已明确，无模糊指令。
- [x] 没有“根据需要实现”“酌情处理”等模糊指令。
- [x] 没有跨 Task 隐式依赖。
- [x] 没有要求 Agent 自行寻找未列出的上下文文件。
- [x] 没有无验证方式的 AC。
- [x] 没有“与 Task-N 类似”“参考 Task-N 实现”等引用。

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 状态 |
|---------|------|----------|---------|----------|----------|----------|------|
| TASK-1 | 完成 uid 缓存、受管集合/主类型、单串行队列、尾随合并与空代理、死亡优先和全部测试 | Files 表列出的生产与测试文件 | AC-5.1～AC-5.10 | proposal/spec/design 均 Approved | AC 用例转 Green；目标构建通过；代码范围无偏差；检视证据齐全 | `./build.sh --product-name rk3568 --ccache --build-target ability_runtime_test --no-prebuilt-sdk --gn-args use_thin_lto=false`（工作目录 `/code`） | Blocked（运行验证未闭合） |

## Task 详情

### TASK-1: 受管调度决策、单队列防抖与空代理闭环

| 字段 | 内容 |
|------|------|
| 任务目标 | 在既有 appmgr 内实现受管工具过滤、uid 缓存与重试、完整事件载荷、单串行队列、固定 1000ms 尾随合并、死亡优先和空实现日志，AC-5.1～AC-5.10 全部可验证 |
| AC 映射 | AC-5.1、AC-5.2、AC-5.3、AC-5.4、AC-5.5、AC-5.6、AC-5.7、AC-5.8、AC-5.9、AC-5.10 |
| 前置依赖 | Approved proposal.md、spec.md、design.md；已合入依赖需求 20260914692273 |
| 非目标 | 真实 RSS 调用、性能功耗、资源配额、动态策略、独立 NotifyTaskEnd、新类型实际纳管通道、Public/System API |
| 完成判据 | 全部 AC 测试通过，目标构建无编译错误，无计划外文件，spec 符合性/代码质量/验证审查证据完整 |
| 停止条件 | 需要新增仓/模块/真实 RSS 接口、改变前端配额或公开 API、无法证明 Red/Green、发生未解释的行为偏差 |
| Design 状态/不变量 | STATE-1～STATE-4；INV-1～INV-8 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-1 / INV-1、INV-8 | 任务归因及 uid 缓存 | TaskManagerService | `tasks_[taskId].attributions` / attributionId | 登记时锁外解析 uid 后写入，失败初值 -1 | TaskManagerService 读取/重试更新；ProcessSchedule 只读值副本 | 注销、任务结束、死亡；重试成功覆盖 -1 | uid 缓存完整才可输出事件；持续失败只告警不输出，迟到回写不得覆盖已删记录 |
| STATE-2 / INV-1、INV-2、INV-3 | 受管上下文与 effectiveLevel/主类型 | ProcessSchedule 串行队列 | `pidEntries_[pid]`、`taskToPids_[taskId]` | 首个 CLI 或 VIRTUAL_SCREEN_ABILITY 归因创建 | 单队列读写；dump/provider 只读快照 | 受管工具归零、最后受管任务移除、死亡 | effectiveLevel=max(受管任务)，同档主类型 VIRTUAL_SCREEN_ABILITY > CLI；ABILITY/A2A/INSIGHT 不影响 |
| STATE-3 / INV-4、INV-5、INV-6、INV-7、INV-8 | 已通知档位/主类型与 pending | ResourceControlProxy 串行队列 | `lastNotifiedLevel[pid]`、`lastNotifiedPrimaryTool[pid]`、`pending[pid].epoch` | 可合并事件创建 pending；EmitEvent 后更新已通知状态 | 单队列更新；到期 provider 同队列取最新快照 | 立即事件、死亡、无变化、到期、timer 失败 | 方向只看前后状态；每 pid 至多一个有效 pending；END 只结束当前生命周期 |
| STATE-4 / INV-5、INV-7、INV-8 | 串行事件、epoch 与 timer 资源 | ResourceControlProxy + TaskHandlerWrap/FFRT | 全局单调 epoch、队列任务、不可变事件值 | 候选/窗口创建时分配并捕获值 | 队列消费者只读；陈旧回调只读 epoch 后退出 | 日志完成、到期自然释放、进程退出 | 旧 epoch 不得覆盖新状态；对象进程级存活，Reset 先 drain 且不回退 epoch |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `ScheduleAttributionInfo`、`TaskScheduleContext`、`ProcessScheduleEvent`、`EventProvider` | TASK-1 内部：TaskManagerService → ProcessSchedule → ResourceControlProxy | 供本 Task 的归属、候选与到期处理 | 不可变值；仅 CLI 与 VIRTUAL_SCREEN_ABILITY；taskLevel/max/主类型一致；epoch 单调 |
| Produces | `ApplyControl(event, provider)`、`RestoreControl(event, provider)`、`EmitEvent(event)` | TASK-1 内部：ProcessSchedule → ResourceControlProxy | 供立即、尾随窗口及日志出口 | Apply/Restore 不直接输出，EmitEvent 不递归；真实 RSS 调用为 0 |
| Consumes | `AttributionRecord.pid/tool/uid`、`TaskRecord.creatorPid/creatorUid/priority`、任务结束/死亡/前后台/负载原因 | TASK-1 内部：既有 TaskRecord 和依赖需求 | 来自现有任务事实，不重新定义 | 既有字段和枚举值不变，仅追加 uid 与 VIRTUAL_SCREEN_ABILITY |
| Consumes | `TaskHandlerWrap::CreateQueueHandler`、`SubmitTask`、FFRT timer | TASK-1 内部：单串行队列与延时任务 | 既有基础设施 | 只允许非阻塞提交保序；到期同队列执行 provider，禁止跨线程直接改状态 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `services/appmgr/src/task_mgr/process_schedule.cpp` | 现有 max、重复引用、注销、死亡和占位日志实现 |
| `services/appmgr/src/task_mgr/task_manager_service.cpp` | 注册/注销/结束/重判/死亡顺序与锁域 |
| `services/appmgr/include/task_mgr/task_record.h`、`attribution_record.h` | 创建者和归因字段、工具枚举 |
| `services/appdfr/src/appfreeze_util.cpp:133-157` | `/proc/<pid>/status` uid 解析先例，仅只读复用模式 |
| `services/common/src/task_handler_wrap.cpp`、`queue_task_handler_wrap.cpp` | 串行队列、延时提交与生命周期 |
| `services/abilitymgr/src/ability_manager_service.cpp:581-584` | 确认 AMS 已占用 TaskFinishedCallback，禁止接管 |
| `test/unittest/process_schedule_test/process_schedule_test.cpp`、`task_manager_service_test.cpp` | 既有 fixture、回归和死亡后重新归因场景 |
| `.codespec/changes/issue-20260914692272-task-rss-tiered-control/proposal.md`、`spec.md`、`design.md` | 上游行为、AC、ADR、STATE 和 INV 的唯一事实源 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | `services/appmgr/include/task_mgr/task_manager_service.h` | uid resolver 与恢复回写声明 |
| Modify | `services/appmgr/src/task_mgr/task_manager_service.cpp` | uid 查询缓存、重试、死亡优先、原因和保序提交 |
| Modify | `services/appmgr/include/task_mgr/attribution_record.h` | uid 字段与 VIRTUAL_SCREEN_ABILITY |
| Modify | `services/appmgr/src/task_mgr/attribution_record.cpp` | 新枚举字符串映射 |
| Modify | `services/appmgr/include/task_mgr/process_schedule.h` | 受管上下文、事件模型和单队列声明 |
| Modify | `services/appmgr/src/task_mgr/process_schedule.cpp` | 受管过滤、max/主类型、快照、候选事件和 provider |
| Create | `services/appmgr/include/task_mgr/resource_control_proxy.h` | Apply/Restore/EmitEvent、EventProvider、pending/lastNotified |
| Create | `services/appmgr/src/task_mgr/resource_control_proxy.cpp` | 固定 1000ms 尾随窗口、epoch、空实现日志 |
| Modify | `services/appmgr/BUILD.gn` | 接入代理实现 |
| Modify | `test/unittest/process_schedule_test/process_schedule_test.cpp` | AC 矩阵与既有回归适配 |
| Create | `test/unittest/process_schedule_test/resource_control_proxy_test.cpp` | 防抖、陈旧回调、失败与无递归测试 |
| Modify | `test/unittest/process_schedule_test/BUILD.gn` | 接入代理实现和新增测试 |
| Modify | `test/unittest/task_manager_service_test/task_manager_service_test.cpp` | uid 重试与自纳管死亡测试及回归适配 |
| Modify（如需要） | `test/unittest/task_manager_service_test/BUILD.gn` | 补齐新增代理实现和重试测试依赖 |
| Modify | `test/unittest/{ams_service_app_spawn_client_test,ams_service_event_drive_test,cache_process_manager_second_test,app_mgr_service_inner_seventh_test,app_mgr_service_event_handler_test,ams_service_startup_test,ams_app_workflow_test,app_mgr_service_inner_eighth_test,ams_recent_app_list_test,app_mgr_service_inner_ninth_test,app_mgr_service_inner_tenth_test,ams_service_load_ability_process_test}/BUILD.gn`、`test/moduletest/common/ams/BUILD.gn` | 为全部直接聚合 process_schedule.cpp 的 13 个既有目标补入 resource_control_proxy.cpp |
| Modify | `.codespec/changes/issue-20260914692272-task-rss-tiered-control/proposal.md`、`spec.md`、`design.md`、`execution-plan.md` | 范围、审批和实施证据 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| `services/abilitymgr/**` | AMS 任务完成回调和其他模块不属于本需求，禁止接管或扩大影响 |
| `services/appdfr/**` | 只读参考 uid 解析，禁止新增跨模块依赖或修改 |
| `interfaces/kits/**`、`frameworks/**`、SDK d.ts | 无 Public/System API 变化 |
| `interfaces/inner_api/app_manager/**` 既有 IPC 序列化 | 本实现只在 appmgr 内部，不修改既有 IAppMgr 消息 |
| `20260914692272/`、`20260914692273/` | 前置分析与依赖目录只读，禁止修改 |
| 真实 RSS 实现仓及 `resource_schedule_service/**` | 本期空实现，禁止真实对接 |
| 构建生成物、凭据、本地私有配置、无关未跟踪文件 | 禁止纳入实现或提交 |

**Spec References**

- AC / Rules: `AC-5.1`、`AC-5.2`、`AC-5.3`、`AC-5.4`、`AC-5.5`、`AC-5.6`、`AC-5.7`、`AC-5.8`、`AC-5.9`、`AC-5.10`；`R-1`、`R-2`、`R-3`、`R-4`、`R-5`、`R-6`、`R-7`、`R-8`、`R-9`
- Anchors: `spec.md#用户故事`、`spec.md#验收追溯`、`spec.md#规则定义`、`spec.md#验证映射`

**Design References**

- IDs / Anchors: `ADR-1`、`ADR-2`、`ADR-3`、`ADR-4`、`ADR-5`、`ADR-6`、`ADR-7`、`ADR-8`、`ADR-9`；`STATE-1`、`STATE-2`、`STATE-3`、`STATE-4`；`INV-1`、`INV-2`、`INV-3`、`INV-4`、`INV-5`、`INV-6`、`INV-7`、`INV-8`
- Anchors: `design.md#关键设计决策`、`design.md#状态归属与不变量`、`design.md#算法与状态机`、`design.md#线程与并发模型`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | 必须保持 TaskManagerService → ProcessSchedule → ResourceControlProxy 单向值传递，禁止反向业务依赖 |
| OH-ARCH-SUBSYSTEM | 不得新增 appdfr/RSS 实现依赖或修改未授权仓模块 |
| OH-ARCH-IPC-SAF | 不得新增 Binder 事务码、Parcel 格式或真实 RSS 调用 |
| OH-ARCH-API-LEVEL | 不得修改 Public/System API；内部枚举只追加不重排 |
| OH-ARCH-COMPONENT-BUILD | 只修改既有 BUILD.gn，不新增部件或 bundle.json 依赖 |
| OH-ARCH-ERROR-LOG | 不得用错误码代替事件日志；不得输出残缺事件或伪造已通知/真实生效 |
| P5-3.5 / P5-4.1 | 禁止锁内执行代理/provider；只允许 tasksMutex_ 内非阻塞 SubmitTask 保序，队列内才执行业务 |

**Steps**

- [ ] 运行现有 `process_schedule_test` 与 `task_manager_service_test` 基线，记录当前通过项和不可用于 Red 的兼容项。（ARM 无 qemu、hdc `E002110`，未执行）
- [x] 按 VM-1～VM-10 写失败测试：受管字段/过滤/主类型、动作、单队列顺序、固定窗口、陈旧回调、死亡优先、uid 失败恢复和空代理。
- [ ] 运行聚焦测试，确认缺失能力产生具体 Red；AC-5.9 仅保留兼容性 N/A 说明，不制造假 Red。（实现前 Red 未执行，未以此作为完成证据）
- [x] 实现 AttributionRecord uid 与 VIRTUAL_SCREEN_ABILITY、TaskManagerService 查询/重试/原因/死亡优先。
- [x] 实现 ProcessSchedule 受管集合、max/主类型、单队列候选事件与 provider。
- [x] 实现 ResourceControlProxy 尾随窗口、epoch、独立 EmitEvent 和空实现日志。
- [x] 更新 BUILD.gn，运行目标编译并修复所有错误；再运行聚焦及既有相关回归。（全量构建通过；运行回归因环境阻塞）
- [x] 回填 AC 追溯、实际文件、Actual Result、计划偏差和 Review Evidence，执行 spec 符合性、代码质量和验证审查。（静态审查完成，GC 因运行/Commit 证据缺失而 Blocked）

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `services/appdfr/src/appfreeze_util.cpp:133-157 / GetUidByPid` | `/proc/<pid>/status` 的 `Uid:` 行解析与 -1 失败语义 | 不调用 appdfr，在 appmgr 内实现并注入 resolver，查询移到 tasksMutex_ 外 |
| `task_manager_service.cpp:137-146,750-773 / TimeoutContext` | 定时上下文捕获与陈旧校验 | 改为全局单调 epoch + 单队列逻辑取消，禁止旧生命周期覆盖新状态 |
| `services/common/src/task_handler_wrap.cpp:70-85,147-170 / TaskHandlerWrap` | 复用串行队列和延时任务 | 不新建线程；额外用 epoch 解决取消后重建和 PID 重用 |
| `task_manager_service.cpp:233-293,776-783 / FinishTaskInner / NotifyTaskFinished` | 沿用直接归因清理与 reason | 不接管 AMS 现有 callback，调度事件仍走 FinishTaskInner 直接链 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | AC-5.1～AC-5.10 每项均有对应正向/异常/边界用例及 fresh 输出；AC-5.9 的兼容性 N/A 有既有用例支持 |
| Scope respected | `git diff --name-only` 只包含 Files 表和本计划允许的文档，无生产越界、无真实 RSS、无生成物 |
| State lifecycle complete | STATE-1～4 创建、读取、更新、清理、恢复和 Reset 均有测试；epoch 不回退、旧生命周期不复活 |
| Interface contract matched | Produces/Consumes 与实际签名、字段、枚举和 EventProvider 一致；无独立 NotifyTaskEnd、无递归 Apply/Restore |
| Relevant verification | 直接运行目标构建和本 Task 聚焦/相关回归，不仅运行无关测试或只写 PASS |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| TDD Red | `process_schedule_test` 聚焦新增用例 | 当前缺失能力产生具体字段/过滤/动作/窗口/uid 失败断言失败；AC-5.9 为 N/A 说明 | BLOCKED（未执行实现前 Red，不以此为完成证据） |
| 构建 | `/code` 下 `./build.sh --product-name rk3568 --ccache --build-target ability_runtime_test --no-prebuilt-sdk --gn-args use_thin_lto=false` | 全量目标及新增代理/测试编译、链接成功，无新增编译错误 | PASS：`rk3568 build success`，4079 steps，退出码 0；`ability_runtime_build3.log` |
| 聚焦与回归 | `process_schedule_test`、`task_manager_service_test` 相关用例 | AC-5.1～AC-5.10 全 Green，既有相关回归无新增失败 | BLOCKED（最终构建 PASS；ARM 无 qemu，hdc `E002110`，未执行运行断言） |
| 静态检查 | `git diff --check`、SDD `validate . --level C` 与代码检视工具 | 无 whitespace 错误；spec→design/plan 依赖边通过；发现项有处置记录 | PASS：diff-check 通过；Level C `9 checks, 0 broken, 0 warn`；codecheck `PASS`；稳定性专项无未解决 Finding |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-1：受管调度决策、单队列防抖与空代理闭环 |
| 允许修改 | `services/appmgr/include/task_mgr/`、`services/appmgr/src/task_mgr/`、`services/appmgr/BUILD.gn`、`test/unittest/process_schedule_test/`、`test/unittest/task_manager_service_test/`、本 change 文档及本计划列出的直接聚合目标 BUILD.gn |
| 允许新建 | `services/appmgr/include/task_mgr/resource_control_proxy.h`、`services/appmgr/src/task_mgr/resource_control_proxy.cpp`、`test/unittest/process_schedule_test/resource_control_proxy_test.cpp`、`evidence/reviews/` |
| 只读参考 | `services/appdfr/src/appfreeze_util.cpp`、`services/common/*task_handler_wrap*`、`services/abilitymgr/src/ability_manager_service.cpp`、两个原需求目录、Approved 上游文档 |
| Spec 引用 | AC-5.1～AC-5.10 / R-1～R-9 / `spec.md#规则定义`、`spec.md#验证映射` |
| Design 引用 | ADR-1～ADR-9 / STATE-1～STATE-4 / INV-1～INV-8 / `design.md#状态归属与不变量` |
| 执行步骤 | 本 Task Card 的 Steps，先 Red 后实现再 Green 和审查 |
| 验证命令 | `/code` 下 `./build.sh --product-name rk3568 --ccache --build-target ability_runtime_test --no-prebuilt-sdk --gn-args use_thin_lto=false` / 期望: PASS |
| 完成规则 | 不得修改允许范围外文件；需要扩大范围时停止并修订 Plan；没有 fresh verification evidence 不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-5.1～AC-5.10→TASK-1→实际代码→测试/证据；受管过滤、主类型、uid 异常、死亡优先和计划偏差 |
| Code Quality | 实际代码范围、单队列顺序、epoch/生命周期、锁边界、资源所有权、Base/Head SHA、静态与构建结果 |
| Verification | TDD Red/Green、目标构建、聚焦与既有回归、Actual Result、未执行真机项目的明确说明 |

## 代码范围映射

> Plan 审批时填写计划范围；实现后如实回填实际范围、操作、Commit 和偏差。实际代码范围产生后必须生成三份 Review Evidence，不能用 N/A 规避。

| Task ID | 计划文件/符号 | 实际文件/符号 | 操作 | Commit | Spec Compliance | Code Quality | Verification | 偏差 |
|---------|---------------|---------------|------|--------|-----------------|--------------|--------------|------|
| TASK-1 | `services/appmgr/include/task_mgr/attribution_record.h`、`services/appmgr/src/task_mgr/attribution_record.cpp` | 已实现（当前工作区） | Modify | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/spec-compliance.md#ac-5-1-ac-5-2` | `evidence/reviews/code-quality.md#scope` | `evidence/reviews/verification.md#build` | 无 |
| TASK-1 | `services/appmgr/include/task_mgr/task_manager_service.h`、`services/appmgr/src/task_mgr/task_manager_service.cpp` | 已实现（当前工作区） | Modify | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/spec-compliance.md#ac-5-4-ac-5-6-ac-5-7-ac-5-10` | `evidence/reviews/code-quality.md#scope` | `evidence/reviews/verification.md#build` | 无 |
| TASK-1 | `services/appmgr/include/task_mgr/process_schedule.h`、`services/appmgr/src/task_mgr/process_schedule.cpp` | 已实现（当前工作区） | Modify | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/spec-compliance.md#ac-5-1-ac-5-7` | `evidence/reviews/code-quality.md#scope` | `evidence/reviews/verification.md#build` | 无 |
| TASK-1 | `services/appmgr/include/task_mgr/resource_control_proxy.h`、`services/appmgr/src/task_mgr/resource_control_proxy.cpp` | 已实现（当前工作区） | Create | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/spec-compliance.md#ac-5-4-ac-5-8` | `evidence/reviews/code-quality.md#scope` | `evidence/reviews/verification.md#build` | 无 |
| TASK-1 | `services/appmgr/BUILD.gn`、`test/unittest/process_schedule_test/BUILD.gn`、`test/unittest/task_manager_service_test/BUILD.gn` | 已实现（当前工作区） | Modify | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/spec-compliance.md#build` | `evidence/reviews/code-quality.md#build` | `evidence/reviews/verification.md#build` | 无 |
| TASK-1 | `test/unittest/{ams_service_app_spawn_client_test,ams_service_event_drive_test,cache_process_manager_second_test,app_mgr_service_inner_seventh_test,app_mgr_service_event_handler_test,ams_service_startup_test,ams_app_workflow_test,app_mgr_service_inner_eighth_test,ams_recent_app_list_test,app_mgr_service_inner_ninth_test,app_mgr_service_inner_tenth_test,ams_service_load_ability_process_test}/BUILD.gn`、`test/moduletest/common/ams/BUILD.gn` | 已实现（当前工作区） | Modify | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/spec-compliance.md#build` | `evidence/reviews/code-quality.md#build` | `evidence/reviews/verification.md#build` | 全量构建发现：既有目标直接聚合 process_schedule.cpp，必须同步补入代理实现 |
| TASK-1 | `test/unittest/process_schedule_test/process_schedule_test.cpp`、`resource_control_proxy_test.cpp`、`test/unittest/task_manager_service_test/task_manager_service_test.cpp` | 已实现（当前工作区） | Modify/Create | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/spec-compliance.md#verification` | `evidence/reviews/code-quality.md#tests` | `evidence/reviews/verification.md#tests` | 无 |
| TASK-1 | `.codespec/changes/issue-20260914692272-task-rss-tiered-control/{proposal,spec,design,execution-plan}.md`、`evidence/reviews/` | 已实现（当前工作区） | Modify/Create | 2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99 | `evidence/reviews/spec-compliance.md#summary` | `evidence/reviews/code-quality.md#summary` | `evidence/reviews/verification.md#summary` | 无 |

## 实现偏差和开放问题

| Task ID | 类型 | 问题/偏差 | 处理状态 | 回退能力/Owner |
|---------|------|-----------|----------|----------------|
| TASK-1 | scope/spec/design/dependency | 无真实 RSS、纳管通道、动态策略或计划外生产文件；13 个直接聚合目标已先纳入 design/plan 再修改 | Resolved（范围守界完成） | ohos-plan / ohos-spec / ohos-design / yangzk |
| TASK-1 | verification | 实现前 Red 未执行；ARM 无 qemu，hdc `E002110`，无法形成 fresh 设备 Green | Open（环境阻塞） | ohos-review / yangzk |
| TASK-1 | traceability | 实现提交已创建，AC→Commit 已关联到 `2ab0f2b02ebe417d5e5bc990d5db97fcdd65fe99` | Resolved（本地提交完成） | yangzk |
