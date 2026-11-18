# CLI 子进程查询和纳管（工具进程锚点方案）

> 需求背景：issue 16476「CLI 工具调用 taskId 归因纳管」实施期发现——当前单锚点纳管的是 **claw_sandbox 沙箱宿主进程**，而真正消耗资源的 **CLI 工具子进程不在任务记录**。本文档沉淀问题定位、方案评估、选定的短期方案与长期路径。
>
> 状态：**已实施（sandbox2cli 分支）**——ToolPidResolver 组件 + 三挂点切换 + 归因状态机锚点参数化 + UT。方案评审期关闭事项：① children 直读路径删除（设备内核 `CONFIG_PROC_CHILDREN` 未开，`ls /proc/self/task/self/children` 实测 ENOENT；本仓所有标准产品 defconfig 均为 not set），`/proc` ppid 全扫为唯一路径；② P7 取证闭合（实测 `tool.pgid == host.pgid == hostPid`、无 setsid 逃逸，Killpg 连锅端与"嵌套生命周期"论断成立）；③ 执行口径已定（Owner）：ExecCmd 路径与工具路径统一切换（复合命令孙进程缺口记为已知残缺口）；候选过滤=存活+ppid 硬校验，comm 仅审计日志。**仍待办**：codespec v1.2 单锚点措辞升版记录（§7 条目 1）。

---

## 1. 问题定位（设备实测证据）

### 1.1 进程结构（形态 B：宿主监督者 + 工具子进程）

设备实测（`hdc shell ps -efww`，一次 ohos-location 工具调用期间）：

```
root      17242  ppid=16113(climgr)  claw_sandbox --config {...}           ← 沙箱宿主（anchor）
20020218  17247  ppid=17242         ohos-location get-current-precise-... ← 工具进程（干活者）
```

appmgr 任务 dump（hidumper）：

```
TaskManager active tasks: total=1
      task ID #10086
      managed processes [127:17242(cli)]        ← 只注册了宿主壳
```

**结论**：纳管对象是近乎零开销的 root 空壳（17242），真正消耗 CPU/内存的工具进程（17247，已降权为应用 uid）不在任务记录——任务资源画像失真，管控下发（HIGH 优先级调控）打在壳上、对实际执行者无效。

### 1.2 代码事实（两进程的诞生链）

| 进程 | 诞生点 | 代码锚点 |
|---|---|---|
| 17242（宿主） | climgr 的 fork + exec claw_sandbox | [process_manager.cpp:239](cli_tool_framework/services/climgr/src/process_manager.cpp)（fork）、:194（execve "/system/bin/claw_sandbox"） |
| 17247（工具） | **claw_sandbox 内部的 fork**（climgr 视野之外） | 不在本仓（claw_sandbox 源码在沙箱仓） |

时序：纳管注册（[cli_tool_manager_service.cpp:1296](cli_tool_framework/services/climgr/src/cli_tool_manager_service.cpp) AttachTaskAttribution，fork 返回后同栈执行）**必然早于**工具进程诞生（沙箱需先解析 config、布置 namespace/seccomp/landlock、降权、再 spawn）——不是注册晚了，是工具的诞生点在 climgr 视野之外。

17247 的生命周期完全嵌套在 17242 存活期内（工具退出→宿主 reap→宿主退出）；climgr 收不到 17247 的 SIGCHLD（非直接子进程）。

---

## 2. 方案评估纪要

约束：优先仓内改动；目标纳管对象为工具进程（17247）；沙箱每会话只派生一个工具子进程（已确认）；归因系统要求"每次调用都算清账"。

| 方案 | 仓内 | 只纳管工具 | 短命进程覆盖 | 结论 |
|---|---|---|---|---|
| A. 枚举（服务端读 /proc children） | ✓ | ✓ | **必 miss**（注册时刻早于工具诞生，快照式原理性失明） | 出局 |
| B. claw_sandbox 写回工具 pid | ✗ 动沙箱仓 | ✓ | 100%（fork 返回即写，与工具寿命无关） | 被仓内约束排除，长期备选 |
| C. proc connector（内核 fork 事件订阅） | ✓ | ✓ | 100%（事件驱动） | **设备实测出局**：demo 返回 `Protocol not supported`（内核未编译 CONFIG_CONNECTOR/PROC_CONNECTOR） |
| D. 轮询 /proc | ✓ | ✓ | 概率性（捕获率 ≈ min(1, 存活期/轮询间隔)） | **选定为短期方案**（缺陷已知、缺陷最小化设计，见 §3） |
| E. appmgr 侧 pgid 组语义 | ✓（appmgr owner 评审） | ✗（组含宿主） | 100% | **长期方案**（见 §6） |

另否决项：`LD_PRELOAD` 劫持 claw_sandbox fork（欺骗安全组件，安全评审否决）；`PR_SET_CHILD_SUBREAPER` 收养（只能收尸、无法在活时注册）。

proc connector demo 结论（2026-11-18）：`socket(AF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR)` 返回 EPROTONOSUPPORT——20 行 demo 提前止损，避免约 300 行无效实现。

---

## 3. 选定短期方案：climgr 定向轮询 `/proc`（ToolPidResolver）

### 3.1 设计要点（缺陷最小化）

1. **前密后疏节奏**：BURST 阶段前 10 次 × 10ms（覆盖沙箱布置+spawn 主窗口，工具存活 ≥10ms 必中）；BACKOFF 阶段每 200ms 兜底；总上限 2s 放弃（Owner 裁决：设备实测布置窗口百毫秒级，2s 已含冷启动余量；上板 miss 率数据如显示收紧即可再调，改 `RESOLVE_BUDGET_MS` 一处常量）。**明示接受的残缺口**：存活 <10ms 的极短命工具概率性 miss；沙箱布置超过 2s 的会话整体零归因——miss 率观测按原因码区分这两类。
2. **全扫 `/proc` 匹配 ppid 是唯一路径（评审期实证修订）**：原设计的 children 直读快路径已删除——设备内核与全部标准产品 defconfig 均未开 `CONFIG_PROC_CHILDREN`（实测 ENOENT）；且 children 文件按**线程**归属（`base.c` 注册于 `tid_base_stuff`，`array.c` 以 `real_parent == 该 task` 判定），宿主实测 3 线程，若 fork 发生在工作线程则主线程文件必 miss。全扫匹配 `/proc/<pid>/stat` 的 ppid 字段（`task_tgid_nr_ns(real_parent)`，进程级 tgid 语义）天然免疫跨线程问题；宿主工作线程不污染匹配（线程不出现在 `/proc/[0-9]*/` 顶层）。
3. **链式 ffrt 延时任务，无常驻线程且自我节流**：每次轮询为短任务，自续调度（复用 PostExecToolTask 先例）。**串行链结构自带节流**——下一轮仅在上一轮扫描返回后调度，慢扫描只会拉大实际间隔，不会堆积任务，无需自适应间隔代码。
4. **miss 可观测**：三个放弃路径均 LOGW 且带原因码（宿主先收割 / 会话先结束 / 预算超时），轮询链自身收口——快会话在首轮轮询前收割也会经链的 off-table 探测计入 miss，不会静默丢失。
5. **不回退注册宿主**：保持"只纳管工具"口径纯净，避免 dump 中两种锚点语义混杂。
6. **零成本门**：`taskId` 未携带的会话不启动轮询链（调度、扫描皆零）。

### 3.2 组件（已实现，`services/climgr/src/tool_pid_resolver.cpp`）

- `ToolPidResolver::Resolve(record)`：挂点（fork+监控注册成功后），`taskId` 为空即 no-op。
- `Poll(record, burst, deadline)`：宿主存活（`processId` 原子读）→ 记录在表（提前停链，收割后最多残留一个间隔）→ 预算检查 → 扫描 → 捕获即 `Attach(record, toolPid)` 收链。
- `ScanForChild(anchor, comm)`：遍历 `/proc/[0-9]*/stat`，从**最后一个 ')'** 解析 state/ppid（comm 可含空格与括号），Z/X 跳过；comm 净化后仅作审计日志（prctl 可注入控制字符）。
- 测试接缝：`SetScanForTest` / `SetSchedulerForTest` / `PollForTest`（同步驱动单轮）。

---

## 4. 锚点切换：AttachTaskAttribution 锚点参数化（复用现有状态机）

- **注册**：Phase 1 锁内快照（taskId/callerUid）+ 前置检查改为 `record->processId != -1（宿主未收割）∧ pidToAttach > 0 ∧ 状态 NONE ∧ 记录在表` → PENDING → Phase 2 锁外 IPC 159（appmgr `RegisterManagedProcessForCaller`，uid 门可信通道；此前文档误记 151，已核实为 159）携带 **toolPid** → Phase 3+4 `WriteBackAttachResult` 写回 ATTACHED/FAILED。
- **`record->processId` 永远保持宿主 pid 语义**：它是 WaitPid 收割时的匹配键，绝不能改存 toolPid；工具 pid 仅作注册参数（捕获时的 comm 仅审计日志）。

### 4.1 注销机制零改动（论证）

| 环节 | 为什么不用改 |
|---|---|
| 注销键 | `attributionId`（服务端句柄）与 pid 完全解耦——`PostDetachTaskAttribution(taskId, attributionId, callerUid)` 整链不读 pid |
| 注销触发 | 17247 嵌套于 17242 生命周期内：WaitPid(17242) 收割时全家必已死——现有 D1 触发时序天然正确 |
| record 三元组 | taskId/attributionId/attributionState 照常传递；注销路径不依赖 processId 之外的 pid 信息 |
| 注册在途竞态 | 工具注册 IPC 在途 × 宿主收割抢先 → **现有 PENDING relay / CONSUMED 补偿钳口原样覆盖**（RelayDetach 协议管"注册-注销汇合"，与 pid 无关） |

### 4.2 竞态矩阵

| 交错 | 处理 |
|---|---|
| 轮询捕获时宿主已死（processId==-1） | 放弃注册（工具必死，无孤儿记录） |
| 会话先结束（记录已离表） | 链在下一次在表探测时自终止（最多残留一个间隔），原因码 LOGW 可观测 |
| 注册 IPC 在途时宿主收割 | PENDING relay → 写回方补偿注销（恰一次） |
| 扫描命中→注册落账之间工具死亡（宿主仍活） | 照常注册：注销键为 attributionId、与 pid 解耦，宿主收割时 D1 正常清账；死 pid 在任务记录中短暂滞留无实害（无可归因进程，管控下发空转） |
| 同 anchor 多次捕获（理论不存在） | 首个胜出，NONE 门挡其余——宁少纳管、不多纳管（fail-safe） |
| Killpg 超时路径 | 不变：超时只杀不注销，归进程死亡收口 |

---

## 5. 实际改动清单（sandbox2cli 分支，已按生产 flags 编译验证）

| 项 | 量 |
|---|---|
| 新组件 `ToolPidResolver`（头 + 实现） | ~380 行：`/proc` 全扫（stat 末括号解析、Z/X 跳过、comm 净化）、串行自续轮询链（10×10ms 突发 → 200ms 退避 → 5s 预算）、三原因码 LOGW、测试接缝（SetScanForTest / SetSchedulerForTest / PollForTest / ResetForTest + epoch 栅栏） |
| `AttachTaskAttribution` 锚点参数化 | 签名 `(record, pidToAttach)`；宿主存活门（`processId > 0`）+ NONE 门 + 在表校验不变；注册 pid = 工具 pid |
| 三挂点切换 `ResolveToolPid(record)` | SetupAndStartSession（工具）/ SetupCmdSession（shell）/ CLI_CMD 工具模式；SKILL 路径不动（AMS 归因） |
| 服务头自包含修复 | 补 `#include "iapplication_state_observer.h"`（内联构造函数实例化 `unordered_map<sptr<前向声明>>` 需完整类型——存量头文件对既有 TU 是"侥幸通过"，新 TU 首次踩中） |
| epoch 栅栏（测试安全） | `ResetForTest` 递增 epoch，在途轮询链捕获旧值即静默止步——延时 ffrt 任务不得活过测试边界（退出期 UB 消除）；fixture TearDown 全局栅栏 + ResolveToolPid_0700 专测 |
| 单测（cli_tool_mgr_service_test） | 4 个既有 Attach 用例迁移到新签名/新语义（0100 断言注册 pid = 工具 pid ≠ 宿主）；6 个 SEC-01 竞态用例签名迁移；**新增 ResolveToolPid_0100–0700**：捕获即注册 / 宿主先死放弃 / 节奏与预算 / NONE 防重 / taskId 空零成本 / 离表放弃 / epoch 栅栏 |
| BUILD.gn × 2 | climgr 与测试目标源列表补 `tool_pid_resolver.cpp` |
| appmgr / AMS / claw_sandbox | **零改动** |

**编译验证方式与边界（2026-11-18）**：本工作区 `resource_schedule_service` checkout 不完整（docs-only，`res_sched_json_client.h` 全仓缺失）且 `ability_manager_proxy.cpp` 与本快照 graphic 仓 API 漂移——两条**预存在**坏边（干净树重建同样失败，已取证），全量链接在此环境不可行。已改用外科式验证：经 gn 生成图对全部 5 条受影响编译边（climgr 目标 ×2、测试目标 ×3）以完整生产 flags（含 CFI/LTO）逐一编译，**全部通过**；链接与用例执行留待 CI/上板。

---

## 6. 长期方案（pgid 组语义）与退役路径

**长期正解**：appmgr 归因内核把锚点语义从 pid 升级为 **pgid**——climgr 仍只注册 17242（天然组长，`Killpg` 已依赖进程组），统计/管控/dump 按组展开，100% 覆盖工具乃至孙进程、不依赖"沙箱只有一个子进程"前提。代价：appmgr 内核 owner 评审 + 消费方适配；组语义**包含宿主**（资源口径上反而更准：沙箱布置开销本属该次调用）。

**短期方案的退役**：ToolPidResolver 自包含，pgid 落地时整体删除，无牵连。

---

## 7. 待裁决与前置清单

| # | 项 | 归属 |
|---|---|---|
| 1 | **Owner 裁决**：codespec v1.2「单锚点=CLI 直接创建的进程」在形态 B 下的语义缺陷（锚在壳不在工具），以及 20260914692193 产品 spec「工具进程及其子进程」的口径矛盾——锚点切到工具进程（本方案）或 pgid 组语义（长期），需要正式修订记录 | Owner |
| 2 | ~~内核配置验证~~ **已关闭（2026-11-18）**：设备实测 `ls /proc/self/task/self/children` → ENOENT，全部标准产品 defconfig `CONFIG_PROC_CHILDREN` not set；children 直读路径从方案删除，`/proc` ppid 全扫为唯一路径 | — |
| 3 | 短期方案 miss 率实测数据（LOGW 三原因码采集）→ 决定长期方案（pgid）的启动时机；**上板复测**（本工作区链接/执行环境受损，见 §5 验证边界） | 上板后 |
| 4 | 跨仓备选登记：claw_sandbox 写回工具 pid（1-3 行，若"严格只要工具 pid"成为硬约束时启用） | 沙箱仓需求 |
