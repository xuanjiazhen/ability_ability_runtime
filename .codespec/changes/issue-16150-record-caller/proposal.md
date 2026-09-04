---
# target_release: 发布版本号（如 7.0，可选 7.0-Beta）；不填分支名(master/dev)
target_release: "7.1"
# change_type: 变更类型（new-feature/enhancement/optimization/bugfix/refactor/deprecation）
change_type: "enhancement"
issue: "16150"
author: "zhoulanzhi"
date: "2026-09-04"
status: Draft
---

# Proposal

## 背景与问题

**动机**：上层框架和底层终止应用接口缺少调用方归因信息，无法在大数据进行二级拆解分类。

**问题陈述**：
- 终止应用接口（`ForceExitApp` / `KillAppWithReason` / `KillBundleWithReason` / `RecordAppWithReason`）仅记录终止原因（exitReason/exitMsg/killId），不记录调用方 pid 和 processname。
- 应用被终止后，开发者/维测人员无法定位是哪个系统服务或进程发起的终止。
- 跨层定界困难，大数据统计分析时无法按调用方维度进行二级拆解分类。

**当前痛点**：
- 故障归因链断裂：应用退出原因记录中缺失"谁发起的终止"维度，定界停留在"被终止"层面。
- 大数据无法按调用方（AMS / WMS / UpdateService / ANR / Crash 等系统组件）拆解终止来源。

## 用户场景与业务触发

### US-1: 应用异常终止后的跨层定界
- **角色**：应用开发者 / 系统维测人员
- **业务触发**：应用被系统终止后，在应用运行记录/HiSysEvent 中排查退出原因时
- **业务价值**：通过调用方 pid + processname 一键定位发起终止的系统组件，完成二级拆解分类，缩短故障定界时间

## 初始分级判断

> 变更类型见 frontmatter `change_type`（enhancement）。
> 工作量约 3 人日；当前仅在 ability_runtime 落地 TODO 标记，未含上层框架联动。

| 判断项 | 结果 | 依据 |
|--------|------|------|
| 变更类型 | enhancement | 在既有终止接口上扩展调用方归因记录，不改 API 语义 |
| 复杂度 | 标准 | 涉及终止接口多处入口 + 应用运行记录写入链路 |
| 涉及仓数量 | 1 | ability_runtime（inner_api C 接口 + abilitymgr 服务 + appmgr 记写入） |
| 是否涉及 Public/System API | 否 | 内部维测接口扩展，不变更对外 API 签名 |
| 是否涉及安全/性能关键路径 | 是 | 终止接口为底层关键路径；通过已有 IPC 上下文获取调用方信息 |
| 是否跨 SIG | 否 | 需求自包含于 ability_runtime，自动获取方式无需调用方配合 |

## 目标

1. 在终止应用接口（`RecordAppWithReason` / `KillAppWithReason` / `KillBundleWithReason` / `ForceExitApp`）的退出原因记录中，写入调用方 pid 和 processname。
2. 通过跨进程调用上下文自动获取调用方 pid/processname，无需调用方配合修改接口签名。

## 非目标

- 修改调用方接口签名（自动获取方式无需调用方配合）。
- 新增或变更对外公共/系统 API。
- 修改 IPC Stub/Proxy 序列化格式。
- 变更 `hisysevent.yaml` 已发布事件的参数类型或删除参数。
- ANR/Crash/低内存 kill 经 `RecordProcessExitReason(pid, uid)`（hiview/memmgr 专用）与 `RecordSignalReason` 信号路径的归因记录不在本次 6 个终止接口范围内（背景中的"按 ANR/Crash 拆解"本期不完整覆盖，留待后续迭代）。
- 嵌套 IPC 链原始发起方追溯。

## Agent Scope Guard

> 约束 AI/Agent 的探索和修改边界。若需要突破边界，先更新本 proposal 并获得确认。

| 维度 | 范围/限制 | 需人工确认的触发条件 |
|------|-----------|----------------------|
| 允许仓库 | ability_runtime | 需联动上层框架仓库时 |
| 允许模块/目录 | `services/abilitymgr/`、`interfaces/inner_api/ability_manager/`、`services/appmgr/`（应用运行记录写入） | 触及 `interfaces/kits/` 需先做兼容性评估 |
| 禁止修改项 | 公共 SDK API 签名、IPC 序列化格式、`hisysevent.yaml` 已发布事件参数、Mission 持久化数据格式 | 任一命中即暂停并人工确认 |
| 外部依赖/网络访问 | 无新增第三方依赖 | 新增依赖须先过 `bundle.json` License 审查 |

## 成功标准

> 系统级能力达成（可观察、可量化），不含内部实现/接口细节。

| 标准 | 可观察指标 | 验证方式 |
|------|-----------|---------|
| 终止接口被调用后，应用退出原因记录中可查到调用方 pid | 100% | HiSysEvent / 应用运行记录日志验证 |
| 调用方 processname（经 /proc/<pid>/cmdline 解析，best-effort：调用方进程已退出时允许为空）反馈到应用运行记录中 | ≥95% | 日志验证 |
| 现有终止接口对外 API 签名保持不变 | 100% | 接口兼容性回归 |

## 影响范围

| 子系统 | 仓库 | 模块/路径 | 影响类型 |
|--------|------|-----------|---------|
| 元能力 | ability_runtime | `services/abilitymgr/src/ability_manager_service.cpp`（`KillAppWithReason` / `KillBundleWithReason` / `ForceExitApp`） | 扩展 |
| 元能力 | ability_runtime | `interfaces/inner_api/ability_manager/src/ability_manager_client_c.cpp`（`RecordAppWithReason`） | 扩展 |
| 元能力 | ability_runtime | `services/appmgr/`（应用运行记录/退出原因写入） | 扩展（联动） |

## 假设与开放问题

### 假设
- 跨进程调用上下文中可获取调用方 pid（`IPCSkeleton::GetCallingPid()` 已可用）；processname 经 `/proc/<pid>/cmdline` 解析。
- 应用退出原因记录机制（HiSysEvent / AppRunningRecord）支持扩展字段写入。

### 开放问题
- （已由 design 收敛）多线程/嵌套 IPC 场景：取 IPC 直接对端（`IPCSkeleton::GetCallingPid()`），不回溯原始发起方，见非目标。
- （已由 design 收敛）调用方为系统服务时 processname 解析：改用 `/proc/<pid>/cmdline`（`AppScheduler::GetRunningProcessInfoByPid` 仅覆盖应用进程，对系统服务恒为空），进程已退出时置空。
- （已由 design 收敛）`RecordAppWithReason`（C 接口）调用方上下文：服务侧经 IPC 上下文自动获取，记录为上层代理进程（IPC 直接对端）pid，无需上游显式传参、无需联动上层仓库。

## 资源开销审视

> 三维状态：`required` / `review-required` / `not-applicable` / `waived`。
> 「性能」行为摘要由 性能 状态推导：`required` / `review-required` → 是；`not-applicable` / `waived` → 否。

| 维度 | 状态 | 信号/依据 | 确认人/理由/范围 |
|------|------|-----------|-----------------|
| 性能 | not-applicable | 终止为低频非热路径；单次调用仅新增常量级 IPC 上下文读取 + 字段写入，开销可忽略 | 确认人=ability_runtime-owner; 理由=低频关键路径、单次开销常量级、无热路径/每帧/唤醒频率影响; 范围=终止接口 |
| 内存(RAM) | not-applicable | 仅扩展记录字段，无常驻/峰值内存增量 | 确认人=ability_runtime-owner; 理由=仅扩展落盘字段; 范围=终止记录 |
| 存储(ROM) | not-applicable | 无新增镜像产物（.so/.abc/资源/预置包）及 data 分区大体量持久化 | 确认人=ability_runtime-owner; 理由=无新增产物; 范围=终止记录 |

## 不涉及项确认

> 逐维度主动判断"涉及/不涉及"并填写依据；不允许留空；标"不涉及"必须写明理由。
> 「性能」行为摘要（过渡字段，以「资源开销审视」为准）：性能 `not-applicable` → 否。

| 维度 | 是否涉及 | 依据 |
|------|---------|------|
| 性能 | 否 | 终止为低频非热路径，单次调用开销为常量级 IPC 上下文读取，可忽略 |
| 安全/权限 | 否 | 不跨新的信任边界；调用方 pid/processname 通过已有 IPC 上下文获取，不涉及敏感数据/认证授权/权限变更 |
| 兼容性 | 否 | 不变更现有接口签名，新增记录字段不影响现有调用方 |
| API/SDK | 否 | 内部维测接口扩展，不改公共/系统 API 签名 |
| IPC/跨进程 | 是 | 通过跨进程调用上下文获取调用方 pid/processname |
| 构建/组件 | 否 | 无新增组件/依赖 |
| 国际化/无障碍 | 否 | 后台维测功能无 UI |
| 数据迁移 | 否 | 无数据迁移需求；新增记录字段向前兼容 |

## API 设计属性

### API 范围声明

不涉及。理由：本次为内部维测接口扩展，通过跨进程上下文自动获取调用方信息并写入应用退出原因记录，不变更现有对外 API 签名，不新增公共/系统 API。
