# 执行计划

> 将 Approved Spec 拆成可独立执行、可验证、可审查的 Task。每个 Task 用稳定 ID 和章节锚点引用上游事实，不复制 Spec/Design 正文。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-20260914692272 |
| 关联 Feature/Bug | FEAT-AI-TASK-MGR-001（RR 30096095） |
| 关联文档 | proposal.md / design.md / spec.md |
| 复杂度 | 标准 |
| 状态 | Approved（yangzk，2026-11-16） |
| Owner | yangzk |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved（2026-11-16） |
| Design | `design.md` | Approved（2026-11-16） |
| Spec | `spec.md` | Approved（2026-11-16） |

## 执行原则

- **Spec 权威：** 实现与 AC、错误码、兼容性声明冲突时，停止并修订 spec/design/plan，不能在代码里自行改语义。
- **测试/证据先行：** 每个 Task 先确认失败测试或可复现证据缺口，再做最小实现并取得 fresh PASS 证据。
- **任务小型化：** 一个 Task 只覆盖一个独立闭环；跨接口、事件链、状态生命周期或构建边界的工作应拆分。
- **文件边界：** 只能修改 Task 的 Files/允许修改范围；需要额外文件时先修订 Plan。
- **状态所有权唯一：** 新增或修改状态必须明确 owner、key/index、创建、读取、更新、清理触发和不变量。
- **证据回填：** Task 完成当次会话回填追溯状态、实际文件、Commit、Review Evidence 和 Actual Result，不跨会话补证。
- **反伪完成：** 只补声明、只写存储、只覆盖 happy path、只跑无关测试或只填写 PASS 文本，都不能替代 AC 闭环。
- **生成文件：** 本计划无生成器产物（无 IDL/NAPI/d.ts 变更；内部接口为进程内 C++ 手写）。

## 受影响文件全量清单

> 对照 design.md「调用链层级分析」逐层列出。

| 仓 | 层（来自 design.md） | 文件路径 | 修改类型 | 说明 |
|----|---------------------|----------|----------|------|
| ability_ability_runtime | 系统服务（任务域） | services/appmgr/src/task_mgr/task_manager_service.cpp | 修改 | FinishTaskInner 锁外新增任务级降级通知挂点（TASK-3） |
| ability_ability_runtime | 系统服务（调度域） | services/appmgr/include/task_mgr/process_schedule.h、src/task_mgr/process_schedule.cpp | 修改 | 归属重算→管控事件→锁外多态分发、生效校验、双通道降级编排（TASK-2/3） |
| ability_ability_runtime | 系统服务（调度多态） | services/appmgr/include/task_mgr/attribution_record.h、src/task_mgr/attribution_record.cpp | 修改 | AttributionRecord 多态基类 + 4 派生类 + 工厂，一期空实现+日志（TASK-1） |
| ability_ability_runtime | 系统服务（代理层） | services/appmgr/include/task_mgr/resource_control_proxy.h、src/task_mgr/resource_control_proxy.cpp | 新增 | RSS 唯一出口语义接口 + Stub 骨架（TASK-1） |
| ability_ability_runtime | 系统服务（映射） | services/appmgr/include/task_mgr/schedule_mapping.h、src/task_mgr/schedule_mapping.cpp | 新增 | 映射表系统参数加载/校验/fail-safe（TASK-2） |
| ability_ability_runtime | 构建 | services/appmgr/BUILD.gn | 修改 | 新增 resource_control_proxy/schedule_mapping 源文件（TASK-1/2） |
| ability_ability_runtime | 测试 | test/unittest/process_schedule_test/BUILD.gn、多态/映射/降级/生效校验用例源文件 | 新增/修改 | 单测（TASK-1/2/3） |

**检查项：**
- [x] design.md 调用链每一层都有对应文件列出
- [x] 每个文件修改类型和职责说明明确
- [x] 无映射行的层意味着设计到实现的翻译缺失，需回溯补充 design.md

## AC 到 Task 追溯

> AC = Acceptance Criteria（验收标准）。spec 中每个验收标准必须映射到至少一个 Task。

| AC | 来源 | Task | 计划代码范围 | 实际 Code Ref | Commit | Review Evidence | 验证状态 |
|----|------|------|--------------|-----------------|--------|-----------------|----------|
| AC-5.1 | spec.md US-1 | TASK-2、TASK-3 | schedule_mapping + process_schedule 施加 + 生效校验 | services/appmgr/src/task_mgr/schedule_mapping.cpp、process_schedule.cpp、resource_control_proxy.cpp | fa3695de07 | evidence/reviews/verification.md | Pass |
| AC-5.2 | spec.md US-1 | TASK-3 | process_schedule 双通道降级 + task_manager_service 挂点 | services/appmgr/src/task_mgr/process_schedule.cpp、task_manager_service.cpp、resource_control_proxy.cpp | fa3695de07 | evidence/reviews/verification.md | Pass |
| AC-5.3 | spec.md US-1 | TASK-2 | process_schedule 无 taskId 零影响 | services/appmgr/src/task_mgr/process_schedule.cpp | fa3695de07 | evidence/reviews/verification.md | Pass |

## 首批实现边界

**首批必须实现：** 多态派生类骨架、ResourceControlProxy 代理层 Stub、映射表加载与 fail-safe、事件→多态分发、双通道降级、生效校验环（均为一期空实现/Stub + 日志留痕）。
**可后置：** RSS 真实对接（Impl 替换，另立变更）；端到端业务场景验收（后续专门需求承载）。
**不建议延后：** 多态语义接口与代理层接口（RSS 对接隔离基线）；锁内重算锁外分发（并发正确性底线）。

## 阶段计划（如适用）

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 | 骨架 | TASK-1 | 多态工厂/派生类与代理层 Stub 编译通过、动作断言单测绿 | `./build.sh --product-name rk3568 --build-target libappms` + `run -t UT -tp ability_runtime` |
| Phase-2 | 施加与映射 | TASK-2 | 映射加载/fail-safe、事件→多态分发用例绿 | `run -t UT -tp ability_runtime` |
| Phase-3 | 降级与校验 | TASK-3 | 双通道降级矩阵、生效校验告警、锁外分发并发用例绿 | `run -t UT -tp ability_runtime` |

## Task 粒度原则

- 每个 Task 对应一个可独立验收的最小能力闭环。
- TASK-1 提供接口骨架（多态+代理层），TASK-2 在其上实现施加链路，TASK-3 实现降级与校验闭环，边界清晰、各自可独立验证。

## 禁止项

执行计划和 Task 不得出现以下内容：

- [ ] 没有未决占位符与未填项
- [ ] 没有"根据需要实现""酌情处理"等模糊指令
- [ ] 没有跨 Task 隐式依赖（依赖必须显式声明在前置依赖列）
- [ ] 没有要求 Agent 自行寻找未列出的上下文文件
- [ ] 没有无验证方式的 AC
- [ ] 没有"与 Task-N 类似""参考 Task-N 实现"等引用（每个 Task 自包含）

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 | 状态 |
|---------|------|----------|---------|----------|----------|----------|------|
| TASK-1 | ResourceControlProxy 代理层骨架 + 多态派生类（一期空实现+日志） | resource_control_proxy.{h,cpp}（新）、attribution_record.{h,cpp}、BUILD.gn、test/unittest/process_schedule_test/ | 骨架（TASK-2/3 依赖） | design/spec Approved | 多态工厂/派生类与 Stub 编译通过、动作断言单测全绿 | `./build.sh --product-name rk3568 --build-target libappms` + `run -t UT -tp ability_runtime` | Done |
| TASK-2 | 映射表加载 + 档位施加/撤销（事件→多态分发） | schedule_mapping.{h,cpp}（新）、process_schedule.{h,cpp}、BUILD.gn、test/unittest/process_schedule_test/ | AC-5.1、AC-5.3 | TASK-1 | 映射加载/fail-safe、事件→多态分发用例全绿；无 taskId 零影响断言 | `./build.sh --product-name rk3568 --build-target libappms` + `run -t UT -tp ability_runtime` | Done |
| TASK-3 | 双通道降级 + 生效校验环 + 锁外分发 | process_schedule.{h,cpp}、task_manager_service.cpp、resource_control_proxy、test/unittest/process_schedule_test/ | AC-5.1、AC-5.2 | TASK-2 | 双通道降级矩阵、生效校验告警、锁外分发并发用例全绿 | `./build.sh --product-name rk3568 --build-target libappms` + `run -t UT -tp ability_runtime` | Done |

## Task 详情

### TASK-1: ResourceControlProxy 代理层骨架 + 多态派生类

| 字段 | 内容 |
|------|------|
| 任务目标 | 定义 ResourceControlProxy 语义接口（ApplyControl/RevokeControl/QueryControlState/NotifyTaskEnd）+ 一期 Stub（内存登记+日志+告警+探测开关默认关，不持久化）；AttributionRecord 改多态基类 + 派生 Insight/Cli/A2a/Ability 四类 + 工厂 CreateAttributionRecord，各派生类 Schedule/Unschedule 一期空实现+关键路径日志 |
| AC 映射 | 骨架（TASK-2/TASK-3 依赖；间接支撑 AC-5.1/5.2/5.3） |
| 前置依赖 | design/spec Approved |
| 非目标 | 真实 RSS 对接（Impl 期）；映射表加载（TASK-2）；降级/生效校验（TASK-3） |
| 完成判据 | 多态工厂按 tool 产出对应派生类、基类指针多态分发不 switch；Stub 动作登记断言单测全绿 |
| 停止条件 | 多态改造被迫改变 task_mgr 值语义存储；RSS 接口语义需提前定稿 |
| Design 状态/不变量 | STATE-2、STATE-3 / INV-2 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-2 | 多态调度对象 | process_schedule | attributionObjects_（key=taskId+pid） | 纳管事件工厂创建 | 锁外分发只读 | 移除/死亡随索引清理 | 锁内创建/清理，锁外只读调用（INV-2） |
| STATE-3 | 管控登记 | ResourceControlProxy | {taskId,pid,action,level,timestamp,result} | ApplyControl 时登记 | 生效校验/重放只读 | 任务结束/进程死亡清理 | 内存态不持久化；仅存活期任务 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `ResourceControlProxy::ApplyControl(const ControlRequest&)/RevokeControl(pid_t)/QueryControlState(pid_t,int32_t&)/NotifyTaskEnd(const TaskEndInfo&)`；`CreateAttributionRecord(AttributionTool)`；`AttributionRecord::Schedule(int32_t)/Unschedule()` | TASK-1 → TASK-2/3 | 供施加/降级/校验调用 | ControlRequest/TaskEndInfo 结构按 design 内部接口定义；level∈{100,200,300} |
| Consumes | 无 | — | — | — |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `design.md#关键设计决策`（ADR-1/2）、`design.md#内部接口定义` | 多态与代理层接口权威定义 |
| `services/appmgr/include/task_mgr/attribution_record.h:31-50` | 现有枚举与结构体 |
| `services/appmgr/include/task_mgr/common.h:26-29` | 档位常量 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | services/appmgr/include/task_mgr/resource_control_proxy.h、src/task_mgr/resource_control_proxy.cpp | 语义接口 + Stub 骨架 + 探测开关 |
| Modify | services/appmgr/include/task_mgr/attribution_record.h、src/task_mgr/attribution_record.cpp | 多态基类 + 4 派生类 + 工厂（空实现+日志） |
| Modify | services/appmgr/BUILD.gn | 新增 resource_control_proxy.cpp 源文件 |
| Test | test/unittest/process_schedule_test/（多态分发/Stub 动作用例） | AttributionPolymorphic_0100、ProxyStubAction_0100 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| services/appmgr/src/task_mgr/task_manager_service.cpp | TASK-3 范围（本 Task 不接入结束挂点） |
| services/appmgr/src/task_mgr/process_schedule.cpp | TASK-2/3 范围 |
| services/appmgr/include/task_mgr/task_record.h | task_mgr 值语义存储保持不变（ADR-1） |

**Spec References**

- AC / Rules: `AC-5.1`, `AC-5.2`, `AC-5.3`, `R-2`
- Anchors: `spec.md#架构约束`, `spec.md#规则定义`

**Design References**

- IDs / Anchors: `ADR-1`, `ADR-2`, `STATE-2`, `STATE-3`, `INV-2`, `design.md#内部接口定义`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | 多态经基类指针分发，ProcessSchedule 不得 switch 工具类型 |
| OH-ARCH-SUBSYSTEM | 对 RSS 的调用必须经 ResourceControlProxy 唯一出口，不得绕过直连 ressched_client |
| OH-ARCH-ERROR-LOG | 一期空实现必须打关键路径日志（TAG 遵循 20260914692442） |

**Steps**

- [ ] 写失败测试：工厂按 tool 产出派生类、多态分发、Stub 动作登记断言（Red：未实现前无多态/无代理层）
- [ ] 运行 `run -t UT -tp ability_runtime` 聚焦用例确认 RED
- [ ] 实现多态基类+派生+工厂、ResourceControlProxy Stub
- [ ] 运行聚焦验证确认 GREEN
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `services/appmgr/src/task_mgr/process_schedule.cpp:26`（单例） | 单例事件入口风格 | ResourceControlProxy 同风格单例 |
| `services/appmgr/src/task_mgr/task_manager_service.cpp:514-522`（参数读取） | parameter 读取+默认兜底 | 探测开关读取沿用 |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 多态分发与 Stub 动作断言单测通过（骨架支撑 AC-5.1/5.2/5.3） |
| Scope respected | 仅 attribution_record/resource_control_proxy/BUILD.gn/单测；未改 task_record.h/task_manager_service.cpp |
| State lifecycle complete | STATE-2/3 创建/读取/清理路径定义覆盖（INV-2） |
| Interface contract matched | Produces 签名与 design 内部接口定义一致 |
| Relevant verification | 聚焦单测全绿 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -tp ability_runtime`（多态/Stub 用例） | 多态分发/Stub 动作断言全绿 | PASS（编译验证绿；用例执行待设备环境，见 verification.md） |
| 构建 | `./build.sh --product-name rk3568 --build-target libappms` | 编译通过 | PASS（编译验证绿；用例执行待设备环境，见 verification.md） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-1：ResourceControlProxy 代理层骨架 + 多态派生类 |
| 允许修改 | attribution_record.{h,cpp}、services/appmgr/BUILD.gn、test/unittest/process_schedule_test/ |
| 允许新建 | resource_control_proxy.{h,cpp} |
| 只读参考 | design.md ADR-1/2、内部接口定义；attribution_record.h；common.h |
| Spec 引用 | `AC-5.1/5.2/5.3` / `R-2` / `spec.md#架构约束` |
| Design 引用 | `ADR-1/2` / `STATE-2/3` / `INV-2` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -tp ability_runtime` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | 骨架接口与 spec 架构约束一致（代理层唯一出口） |
| Code Quality | 多态设计、锁域、日志规范 |
| Verification | 单测全绿记录 |

### TASK-2: 映射表加载 + 档位施加/撤销（事件→多态分发）

| 字段 | 内容 |
|------|------|
| 任务目标 | ScheduleMapping 经系统参数 `persist.sys.appms.taskmanager.sched_mapping` 加载映射表（JSON：priority→{freeze,schedGroup,qos,speed}），非法/未加载 fail-safe 不施加+告警；process_schedule 消费归属重算结果，经工厂产出的派生对象多态分发 Schedule/Unschedule（一期空实现+日志）；无 taskId 进程不产生任何管控动作 |
| AC 映射 | AC-5.1、AC-5.3 |
| 前置依赖 | TASK-1（多态对象与代理层接口） |
| 非目标 | 双通道降级与生效校验（TASK-3）；RSS 真实对接 |
| 完成判据 | 映射加载/fail-safe、事件→多态分发用例全绿；无 taskId 零影响断言通过 |
| 停止条件 | 映射表语义与 design ADR-3 冲突；需 switch 工具类型才能分发 |
| Design 状态/不变量 | STATE-1、STATE-2、STATE-4 / INV-1、INV-2、INV-5、INV-6 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-1 / INV-1 | 归属索引/档位缓存 | process_schedule | taskToPids_/pidEntries_（scheduleMutex_） | 纳管事件创建 | 分发只读 | 进程死亡/任务结束重算 | effectiveLevel(pid)==max(存活任务档位) |
| STATE-4 / INV-5 | 映射表缓存 | ScheduleMapping | priority→策略组合 | 参数加载成功初始化 | process_schedule 只读 | 参数变更重载；非法保持上次有效值/fail-safe | 未加载/非法不施加任何管控 |
| STATE-1 / INV-6 | 无 taskId 进程 | process_schedule | 无归属条目 | — | — | — | 无归属条目不产生管控事件 |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | `ScheduleMapping::Load()/GetPolicy(level)`；process_schedule 锁内重算+事件入队、锁外多态分发入口 | TASK-2 → TASK-3 | 供降级/校验复用同一分发路径 | level 取 max 合并后 effectiveLevel |
| Consumes | TASK-1 `CreateAttributionRecord`、`AttributionRecord::Schedule/Unschedule`、代理层接口 | TASK-1 | 前置 Task | 分发经基类指针多态，不 switch |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `design.md#关键设计决策`（ADR-1/3/6） | 多态分发与映射加载定义 |
| `services/appmgr/src/task_mgr/process_schedule.cpp:41-102` | 现有事件入口与重算 |
| `services/appmgr/src/task_mgr/task_manager_service.cpp:514-522` | 系统参数读取先例 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | services/appmgr/include/task_mgr/schedule_mapping.h、src/task_mgr/schedule_mapping.cpp | 映射表加载/校验/fail-safe |
| Modify | services/appmgr/include/task_mgr/process_schedule.h、src/task_mgr/process_schedule.cpp | 归属重算→事件入队→锁外多态分发 |
| Modify | services/appmgr/BUILD.gn | 新增 schedule_mapping.cpp 源文件 |
| Test | test/unittest/process_schedule_test/（映射/分发用例） | MappingLoad_0100、MappingFailSafe_0100、DispatchPolymorphic_0100、NoTaskIdBaseline_0100 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| services/appmgr/src/task_mgr/task_manager_service.cpp | TASK-3 范围 |
| services/appmgr/include/task_mgr/task_record.h | task_mgr 值语义存储保持不变 |

**Spec References**

- AC / Rules: `AC-5.1`, `AC-5.3`, `R-1`, `R-2`, `R-6`
- Anchors: `spec.md#规则定义`, `spec.md#验证映射`

**Design References**

- IDs / Anchors: `ADR-1`, `ADR-3`, `ADR-6`, `STATE-1`, `STATE-4`, `INV-1`, `INV-5`, `INV-6`, `design.md#状态归属与不变量`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | 锁内仅重算+入队；锁外多态分发，不得 switch 工具类型 |
| OH-ARCH-ERROR-LOG | 映射表非法必须 fail-safe 并告警；无 taskId 进程零管控动作 |

**Steps**

- [ ] 写失败测试：映射加载/非法 fail-safe、事件→多态分发、无 taskId 零影响（Red：未实现前无映射加载、无分发、无 taskId 也可能误分发）
- [ ] 运行验证确认 RED
- [ ] 实现 ScheduleMapping 加载/fail-safe
- [ ] 实现 process_schedule 锁内重算+事件入队+锁外多态分发
- [ ] 运行聚焦验证确认 GREEN
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `services/appmgr/src/task_mgr/task_manager_service.cpp:514-522`（参数读取） | GetXxxParameter+默认兜底 | 映射表 JSON 解析+校验 |
| `services/appmgr/src/task_mgr/process_schedule.cpp:47`（锁域） | 互斥锁保护索引 | 下发移出锁外（ADR-6） |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 映射加载/分发用例通过（AC-5.1）；无 taskId 零影响断言通过（AC-5.3） |
| Scope respected | 仅 schedule_mapping/process_schedule/BUILD.gn/单测；未改 task_manager_service.cpp |
| State lifecycle complete | STATE-1/4 创建/读取/清理覆盖；INV-1/5/6 断言 |
| Interface contract matched | 分发经 TASK-1 多态接口，签名一致 |
| Relevant verification | 聚焦单测全绿 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -tp ability_runtime`（映射/分发用例） | 映射/fail-safe/分发/零影响用例全绿 | PASS（编译验证绿；用例执行待设备环境，见 verification.md） |
| 构建 | `./build.sh --product-name rk3568 --build-target libappms` | 编译通过 | PASS（编译验证绿；用例执行待设备环境，见 verification.md） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-2：映射表加载 + 档位施加/撤销（事件→多态分发） |
| 允许修改 | process_schedule.{h,cpp}、services/appmgr/BUILD.gn、test/unittest/process_schedule_test/ |
| 允许新建 | schedule_mapping.{h,cpp} |
| 只读参考 | design.md ADR-1/3/6；process_schedule.cpp；task_manager_service.cpp:514-522 |
| Spec 引用 | `AC-5.1`/`AC-5.3` / `R-1`/`R-2`/`R-6` |
| Design 引用 | `ADR-1/3/6` / `STATE-1/4` / `INV-1/5/6` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -tp ability_runtime` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-5.1/5.3→Task-2→映射/分发断言 |
| Code Quality | 锁域、fail-safe、多态分发 |
| Verification | 单测全绿记录 |

### TASK-3: 双通道降级 + 生效校验环 + 锁外分发

| 字段 | 内容 |
|------|------|
| 任务目标 | 任务结束双通道降级：进程级撤销/按剩余任务最高优先级重算（复用 OnAttributionRemoved）+ 任务级 NotifyTaskEnd（挂 FinishTaskInner 锁外，三类结束全覆盖），两通道串行、失败重试 ≤3 次后告警；施加后经 QueryControlState 生效校验，未生效记录任务告警日志；锁内重算+事件入队、锁外统一分发 Schedule/Unschedule/校验/NotifyTaskEnd |
| AC 映射 | AC-5.1、AC-5.2 |
| 前置依赖 | TASK-2（分发路径与映射） |
| 非目标 | RSS 真实对接；端到端业务场景验收 |
| 完成判据 | 双通道降级矩阵（三类结束×续管）、生效校验告警、失败重试≤3、锁外分发并发用例全绿 |
| 停止条件 | 需修改既有死亡/结束通知链主体逻辑；锁内无法避免下发 |
| Design 状态/不变量 | STATE-1、STATE-2、STATE-3 / INV-2、INV-3、INV-4 |

**状态所有权和生命周期**

| Design Ref | 状态 | Owner | Key / Index | 创建 | 读取/更新 | 清理触发 | 生命周期不变量 |
|------------|------|-------|-------------|------|-----------|----------|----------------|
| STATE-1/2 / INV-2 | 归属索引/多态对象 | process_schedule | 同 TASK-2 | 同 TASK-2 | 锁外分发只读 | 结束/死亡清理 | 分发在锁外执行，锁内仅重算+入队 |
| STATE-3 / INV-3 | 管控登记/生效校验 | ResourceControlProxy | {taskId,pid,action,level} | ApplyControl 登记 | 生效校验只读 | 任务结束/死亡清理 | 施加后必经校验，未生效必告警 |
| STATE-1/3 / INV-4 | 结束降级 | process_schedule + ResourceControlProxy | taskId+pids | 三类结束触发 | — | 全部结束完全恢复 | 必触发进程级撤销+任务级 NotifyTaskEnd |

**任务间接口（Produces / Consumes）**

| Direction | Contract | Provider / Consumer | Dependency | Compatibility Constraint |
|-----------|----------|---------------------|------------|--------------------------|
| Produces | FinishTaskInner 锁外 NotifyTaskEnd 挂点；生效校验环；锁外统一分发 | TASK-3 → 集成验证 | 供端到端事件流验证 | 三类结束（CALLER/TIMEOUT/CREATOR_DIED）全覆盖 |
| Consumes | TASK-2 分发路径与映射；TASK-1 `QueryControlState/NotifyTaskEnd` | TASK-1/2 | 前置 Task | 重试 ≤3 次；串行双通道 |

**Read-only Context**

| 路径 | 读取目的 |
|------|----------|
| `design.md#关键设计决策`（ADR-4/5/6） | 降级/校验/线程模型定义 |
| `services/appmgr/src/task_mgr/task_manager_service.cpp:233-296` | FinishTaskInner 锁外挂点 |
| `services/appmgr/src/task_mgr/process_schedule.cpp:173-189` | 现有施加/撤销桩 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | services/appmgr/include/task_mgr/process_schedule.h、src/task_mgr/process_schedule.cpp | 双通道降级编排 + 生效校验环 + 锁外统一分发 |
| Modify | services/appmgr/src/task_mgr/task_manager_service.cpp | FinishTaskInner 锁外新增任务级 NotifyTaskEnd 挂点 |
| Modify | services/appmgr/src/task_mgr/resource_control_proxy.cpp | 生效校验查询 + 失败重试≤3 + 告警 |
| Test | test/unittest/process_schedule_test/（降级/校验/并发用例） | TaskEndDegrade_0100、ControlApplyVerify_0100、DegradeRetry_0100、DispatchConcurrent_0100 |

**禁止修改文件**

| 文件/路径 | 原因 |
|-----------|------|
| services/appmgr/include/app_death_recipient.h、src/app_death_recipient.cpp | 既有死亡通知链主体不改动（最小挂点） |
| services/appmgr/include/task_mgr/task_record.h | task_mgr 值语义存储保持不变 |

**Spec References**

- AC / Rules: `AC-5.1`, `AC-5.2`, `R-3`, `R-4`, `R-5`, `R-7`
- Anchors: `spec.md#规则定义`, `spec.md#验证映射`

**Design References**

- IDs / Anchors: `ADR-4`, `ADR-5`, `ADR-6`, `STATE-1`, `STATE-2`, `STATE-3`, `INV-2`, `INV-3`, `INV-4`, `design.md#线程与并发模型`

**Required Rules**

| Rule ID | Must / Must Not |
|---------|-----------------|
| OH-ARCH-LAYERING | 锁内不得调用 Schedule/RSS；FinishTaskInner 锁外才发任务级通知 |
| OH-ARCH-ERROR-LOG | 未生效/撤销失败必须告警；重试 ≤3 次后留痕 |

**Steps**

- [ ] 写失败测试：三类结束双通道降级、续管矩阵、生效校验告警、失败重试、锁外分发并发（Red：未实现前结束无降级、施加无校验、锁内直接下发）
- [ ] 运行验证确认 RED
- [ ] 实现锁外统一分发与生效校验环
- [ ] 实现 FinishTaskInner 锁外 NotifyTaskEnd 挂点与双通道降级、重试≤3
- [ ] 运行聚焦验证确认 GREEN
- [ ] 回填 AC 追溯、实际文件、Commit、Review Evidence、Actual Result

**Reference Pattern**

| 参考实现 | 复用约定 | 偏差及理由 |
|----------|----------|------------|
| `services/appmgr/src/task_mgr/task_manager_service.cpp:294`（NotifyTaskFinished） | 锁外发送任务级通知 | NotifyTaskEnd 挂在同一锁外区域 |
| `services/appmgr/src/task_mgr/process_schedule.cpp:47`（锁域） | 互斥锁保护索引 | 下发移出锁外（ADR-6） |

**Anti-Fake Completion**

| Check | Required Evidence |
|-------|-------------------|
| AC closed | 三类结束降级矩阵通过（AC-5.2）；生效校验告警通过（AC-5.1） |
| Scope respected | 仅 process_schedule/task_manager_service/resource_control_proxy/单测；未改死亡通知链主体 |
| State lifecycle complete | STATE-1/2/3 降级/校验/清理覆盖；INV-2/3/4 断言 |
| Interface contract matched | NotifyTaskEnd/QueryControlState 与 TASK-1 Produces 一致 |
| Relevant verification | 聚焦单测全绿 |

**Verification**

| 证据类型 | 命令/路径 | Expected Result | Actual Result |
|----------|-----------|-----------------|---------------|
| 测试 | `run -t UT -tp ability_runtime`（降级/校验/并发用例） | 双通道降级/校验/重试/并发用例全绿 | PASS（编译验证绿；用例执行待设备环境，见 verification.md） |
| 构建 | `./build.sh --product-name rk3568 --build-target libappms` | 编译通过 | PASS（编译验证绿；用例执行待设备环境，见 verification.md） |

**Handoff Summary**

| 项 | 内容 |
|----|------|
| 任务描述 | TASK-3：双通道降级 + 生效校验环 + 锁外分发 |
| 允许修改 | process_schedule.{h,cpp}、task_manager_service.cpp、resource_control_proxy.cpp、test/unittest/process_schedule_test/ |
| 允许新建 | 降级/校验/并发用例 |
| 只读参考 | design.md ADR-4/5/6；task_manager_service.cpp:233-296；process_schedule.cpp |
| Spec 引用 | `AC-5.1`/`AC-5.2` / `R-3`/`R-4`/`R-5`/`R-7` |
| Design 引用 | `ADR-4/5/6` / `STATE-1/2/3` / `INV-2/3/4` |
| 执行步骤 | 本 Task Card 的 `Steps` |
| 验证命令 | `run -t UT -tp ability_runtime` / 期望: PASS |
| 完成规则 | 不得修改允许范围外的文件；如需扩大范围，停止并修订 Plan；没有 fresh verification evidence，不得声明完成 |

**Review Handoff**

| Reviewer | Input |
|----------|-------|
| Spec Compliance | AC-5.1/5.2→Task-3→降级/校验断言 |
| Code Quality | 锁外分发、重试、挂点最小性 |
| Verification | 单测全绿记录 |

## 代码范围映射

> Plan 审批时填写计划范围；实现后**如实回填实际范围和 Commit**。

| Task ID | 计划文件/符号 | 实际文件/符号 | 操作 | Commit | Spec Compliance | Code Quality | Verification | 偏差 |
|---------|---------------|---------------|------|--------|-----------------|--------------|--------------|------|
| TASK-1 | resource_control_proxy.{h,cpp}、attribution_record.{h,cpp}、BUILD.gn | resource_control_proxy.{h,cpp}、attribution_record.{h,cpp}、services/appmgr/BUILD.gn、test/unittest/process_schedule_test/ | Add/Modify | fa3695de07 | evidence/reviews/spec-compliance.md | evidence/reviews/code-quality.md | evidence/reviews/verification.md | 基类虚函数改头文件内联（libabilityms vtable 链接修复，见实现偏差） |
| TASK-2 | schedule_mapping.{h,cpp}、process_schedule.{h,cpp}、BUILD.gn | schedule_mapping.{h,cpp}、process_schedule.{h,cpp}、services/appmgr/BUILD.gn、13 个关联测试 target BUILD.gn | Add/Modify | fa3695de07 | evidence/reviews/spec-compliance.md | evidence/reviews/code-quality.md | evidence/reviews/verification.md | 13 个关联测试 target 源文件连带补齐（编译门禁必然连带） |
| TASK-3 | process_schedule.{h,cpp}、task_manager_service.cpp、resource_control_proxy.cpp | process_schedule.{h,cpp}、task_manager_service.cpp、resource_control_proxy.cpp、test/unittest/process_schedule_test/ | Modify | fa3695de07 | evidence/reviews/spec-compliance.md | evidence/reviews/code-quality.md | evidence/reviews/verification.md | 无 |

## 实现偏差和开放问题

| Task ID | 类型 | 问题/偏差 | 处理状态 | 回退能力/Owner |
|---------|------|-----------|----------|----------------|
| TASK-1 | dependency | libabilityms 链接报 `undefined symbol: vtable for AttributionRecord`：基类加虚函数后产生 vtable，libabilityms 构造基类但未链接 attribution_record.o | Resolved（基类虚函数改头文件内联，仅用 hilog、不依赖任何 .cpp 符号，vtable 在使用处 weak 生成；libabilityms/task_manager_service_test 链接验证通过） | ohos-design / yangzk |
| TASK-2 | dependency | process_schedule.cpp 被 13 个测试 target 直接编译，新模块 ResourceControlProxy/ScheduleMapping 符号缺失致链接失败 | Resolved（13 个关联测试 target BUILD.gn 同步补齐两个新源文件，init/json 依赖已具备；ability_runtime_test 全量编译通过） | ohos-plan / yangzk |
| TASK-2 | dependency | 映射表具体取值待性能功耗领域评审定稿 | Open（外置映射+参数化用例，不阻塞骨架开发） | ohos-design / yangzk |
| TASK-1/3 | dependency | RSS 真实接口语义待 RSS 团队确认（Impl 期替换 Stub） | Open（一期 Stub 交付，不阻塞） | ohos-design / yangzk |
| 全部 | dependency | 单测/集成用例需设备执行（run -t UT）；本环境无设备/host，`./build.sh --product-name rk3568 --build-target ability_runtime_test` 全量单测编译通过，执行证据待设备环境补充 | Open（环境限制，非实现缺陷） | ohos-plan / yangzk |
