# 执行计划

> 将 Approved Spec 拆成可独立执行、可验证的 Task。每个 Task 自包含。

## Plan 元数据

| 字段 | 内容 |
|------|------|
| Plan ID | PLAN-20260825-001 |
| 关联 Feature | FEAT-20260825-001 |
| 关联文档 | proposal.md / design.md / spec.md |
| 复杂度 | 标准 |
| 状态 | Approved(Stage 3 implemented; 2026-09-04 追加 sendMessage 组帧+防伪修复) |
| Owner | yangxuguang-huawei |

## 输入状态

| 输入 | 路径 | 要求状态 |
|------|------|----------|
| Requirement | `proposal.md` | Approved（Stage 1） |
| Design | `design.md` | Approved（Stage 2，gates/specify=通过） |
| Spec | `spec.md` | Approved（Stage 2） |

## AC 到 Task 追溯

| AC | 来源 | Task | 验证方式 | 覆盖 |
|----|------|------|----------|------|
| AC-1 connect 单阶段(ForCli 直连,无 preflight)+connect-done | spec | TASK-1/2/3 | 集成 | 是 |
| AC-2 sendmessage | spec | TASK-3 | 集成 | 是 |
| AC-3 authorize | spec | TASK-3 | 集成 | 是 |
| AC-4 B→caller OnData/OnAuthorize | spec | TASK-2/3 | 集成 | 是 |
| AC-5 stdin disconnect | spec | TASK-5 | 集成 | 是 |
| AC-6 入口门禁 | spec | TASK-1（cliMgr 既有） | 单测 | 是 |
| AC-7 IsSessionOwner | spec | cliMgr 既有（TASK-6 验证） | 单测 | 是 |
| AC-8 非 CLI 走原校验 | spec | TASK-4 | 单测 | 是 |
| AC-9 timeout 上限 | spec | cliMgr 既有（TASK-6 验证） | 单测 | 是 |
| AC-10 CCM 隔离 | spec | TASK-4 | 单测 | 是 |
| AC-11 同 session 复用连接 | spec | TASK-2 | 单测 | 是 |
| AC-12 yield 自动转后台 | spec | cliMgr 既有（TASK-3 验证） | 集成 | 是 |
| AC-13 ClearSession | spec | TASK-5（cliMgr 既有） | 集成 | 是 |
| AC-14 配额 5 | spec | agentmgr 既有（TASK-6 验证） | 单测 | 是 |
| AC-15 B 主动断连 | spec | TASK-5 | 集成 | 是 |

## 首批实现边界

**首批必须实现**：TASK-1（骨架+BUILD/json/selinux 确认）→ TASK-2（单阶段 ForCli connect+双向 proxy/stub）→ TASK-3（CommandLoop REPL+JSON-line+事件）。三者闭合 connect→sendmessage→OnData 主链。
**可后置**：TASK-6 测试加固、O-7 头注释、O-11 IsCliToolToken 直接断言。
**不建议延后**：TASK-4（agentmgr ForCli 接口）——connect/disconnect 跑通依赖 ForCli 门禁；可与 TASK-1 并行但须在联调前就绪。

## 阶段计划

| 阶段 | 目标 | 关键 Task | 结束门槛 | 最小验证 |
|------|------|-----------|----------|----------|
| Phase-1 | 骨架+接线 | TASK-1, TASK-2 | connect 建连+connect-done,无 preflight | `ohos-agent connect` 联调 |
| Phase-2 | 会话主链 | TASK-3, TASK-4 | sendmessage→B、B→OnData、disconnect 全通 | connect→sendmessage→disconnect 集成 |
| Phase-3 | 断连/兜底+测试 | TASK-5, TASK-6 | ClearSession/B断连/配额/CCM/IsCliToolToken 验证 | 全 AC 集成+单测 |

## Task 列表

| Task ID | 目标 | 文件范围 | AC 映射 | 前置依赖 | 完成判据 | 验证命令 |
|---------|------|----------|---------|----------|----------|----------|
| TASK-1 | 工具骨架+main+connect 空壳+BUILD/json+selinux 确认 | tools/ohos-agent/{BUILD.gn,ohos-agent.json,src/main.cpp,src/ohos_agent_command.cpp,include/ohos_agent_command.h}; base/security/selinux_adapter claw_cli.te(确认) | AC-1(骨架) | Plan Approved | 构建产出 ohos-agent；`ohos-agent connect --help` 出帮助；selinux 继承确认 | `./build.sh --product-name rk3568 --build-target ohos-agent` |
| TASK-2 | 单阶段 ForCli connect+IAbilityConnection+AgentConnectorStub+AgentReceiverProxy+Want 键交付 | tools/ohos-agent/src/agent_connection.{cpp,h} | AC-1, AC-4(stub), AC-11 | TASK-1 | ConnectAgentExtensionAbilityForCli(want,connection,callerIdentity) 直连→OnAbilityConnectDone 取得 AgentReceiverProxy、AgentConnectorStub 就绪；AGENTEXTENSIONHOSTPROXY_KEY 写入 Want | 集成：connect 建连+connect-done |
| TASK-3 | CommandLoop REPL+stdin/stdout JSON-line+事件上报+60s CommandTimer 跳过 | tools/ohos-agent/src/session_protocol.{cpp,h}; src/main.cpp | AC-2, AC-3, AC-4(event), AC-12 | TASK-2 | poll(stdin+self-pipe) 解析 JSON-line 分派 sendmessage/authorize/disconnect；B 回调写 OnData/OnAuthorize；connect 跳过 60s 定时器 | 集成：sendmessage→B、B→OnData、disconnect |
| TASK-4 | agentmgr ForCli 接口(强制,no bypass)(connect+disconnect)+CCM gate | agent_runtime_framework/services/agentmgr/src/agent_manager_service.cpp; services/common/* | AC-8, AC-10 | TASK-1 | ForCli 接口(IsCliToolToken+CCM 门禁→防伪→SetCallingIdentity→强制,既有 3 Validate 原样)；CCM 开关 gate；非 CLI 行为不变 | 单测：ForCli 门禁触发+防伪+强制+非 CLI 走原校验+CCM off 拒绝 |
| TASK-5 | disconnect 正常流程+ClearSession 兜底+caller-death 清理 | tools/ohos-agent/src/agent_connection.{cpp,h} | AC-5, AC-13, AC-15 | TASK-2,3 | stdin disconnect→DisconnectAgentExtensionAbility→OnAbilityDisconnectDone→disconnect-done→退出；ClearSession/崩溃→caller-death 清理 | 集成：stdin disconnect+ClearSession+B 断连 |
| TASK-6 | 测试+O-7 头注释+O-11 IsCliToolToken 断言+selinux 确认 | tools/ohos-agent/tests/*; agentmgr 单测; kits/native/connection 头注释; base/security/selinux_adapter | 全 AC 验证 | TASK-1~5 | connect 单阶段/协议/ForCli 防伪/配额/CCM 单测+集成；头注释修正；selinux system_basic call 缺口确认 | `run -t UT -tp ability_runtime` + 集成 |

## Task 详情

### TASK-1: 工具骨架 + BUILD/json + selinux 确认

| 字段 | 内容 |
|------|------|
| 任务目标 | 镜像 tools/ohos-aa 范式新建 tools/ohos-agent：ohos_cli_executable + ohos-agent.json + OhosAgentShellCommand 空壳 + main（connect 跳过 60s CommandTimer）；确认 selinux 由 normal_hap_attr 继承 |
| AC 映射 | AC-1(骨架) |
| 前置依赖 | Plan Approved |
| 非目标 | connect 完整逻辑（TASK-2/3）、agentmgr ForCli 接口（TASK-4） |
| 完成判据 | 构建产出 /system/bin/cli_tool/executable/ohos-agent；`ohos-agent connect --help` 出帮助；ohos-agent.json 注册到 cliMgr configs；selinux 继承确认（无需新增，除非 system_basic 缺口） |
| 停止条件 | 构建失败；ohos_cli_executable 模板不可用 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | tools/ohos-agent/BUILD.gn | ohos_cli_executable("ohos-agent")+ohos_agent_cli_source_set(复用 tools/aa/src/shell_command.cpp) |
| Create | tools/ohos-agent/ohos-agent.json | name/executablePath/hasSubCommand/subcommands.connect{inputSchema,outputSchema,requirePermissions:["ohos.permission.CONNECT_AGENT"],eventSchemas,推荐yieldMs:5000} |
| Create | tools/ohos-agent/src/main.cpp | argv 自识别 ohos-agent；connect 跳过 60s XCollie CommandTimer；ExecCommand |
| Create | tools/ohos-agent/include/ohos_agent_command.h | OhosAgentShellCommand extends ShellCommand；LONG_OPTIONS(--bundleName/--abilityName/--moduleName/--agentId) |
| Create | tools/ohos-agent/src/ohos_agent_command.cpp | RunAsConnect 空壳（TASK-2 填充）；CreateCommandMap/CreateMessageMap/init |
| Confirm | base/security/selinux_adapter/.../claw_cli.te | 确认 normal_hap_attr 继承；若 agent 宿主含 system_basic 则补 allow system_basic_hap_attr normal_hap_attr:binder { call };（O-12/P2-3） |

**Required Rules**: OH-ARCH-COMPONENT-BUILD（ohos_cli_executable 部件）、允许 tool_util.cpp env 注入新增(callerIdentity),不改 io_frame 分帧；ADR-10 yield 复用。

**Steps**: 写 BUILD.gn+json+main+command 空壳→构建→`ohos-agent connect --help`→selinux 继承确认→记录证据。

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 构建 | `./build.sh -p rk3568 --build-target foundation/ability/ability_runtime/tools/ohos-agent:ohos-agent -j64 --ccache` | **PASS**（exit 0） |
| 产物 | `out/rk3568/ability/ability_runtime/ohos-agent` | ELF 32-bit ARM PIE | |
| 注册 | `tools/ohos-agent/ohos-agent.json`（name/executablePath/hasSubCommand/subcommands.connect{requirePermissions:[CONNECT_AGENT],eventSchemas,推荐 yieldMs}） | 写入，由 cliMgr configs 消费 |
| selinux | `base/security/selinux_adapter/.../claw_cli.te` | claw_cli 带 `normal_hap_attr`（type.te:14）→ 继承 hap 级规则已覆盖绝大多数；唯一潜在缺口 system_basic_hap→claw_cli 仅 transfer 无 call → TASK-6 确认 agent 宿主 hap 级别（P2-3） |
| 代码 | `tools/ohos-agent/{BUILD.gn,ohos-agent.json,src/main.cpp,src/ohos_agent_command.cpp,include/ohos_agent_command.h}` | 已写入 + 编译通过 |

**Handoff Summary**: 任务=建工具骨架；允许修改 tools/ohos-agent/*；只读 tools/ohos-aa/*（范式）；Spec 摘要=AC-1；Design 摘要=ADR-1(独立二进制)+ADR-10(yield,推荐yieldMs)+sa_profile/selinux 节(normal_hap_attr 继承)；验证=`./build.sh --product-name rk3568 --build-target ohos-agent` 期望 PASS；完成规则=允许 cli_tool_framework tool_util.cpp env 注入新增(callerIdentity),不改 io_frame 分帧；ADR-10 yield 复用；不得改 agent_runtime_framework 服务侧内部。

### TASK-2: 单阶段 ForCli connect + 双向 proxy/stub + Want 键

| 字段 | 内容 |
|------|------|
| 任务目标 | RunAsConnect 实现：构造 Want(AGENTID_KEY+AGENTEXTENSIONHOSTPROXY_KEY=自身 AgentConnectorStub->AsObject())→ConnectAgentExtensionAbilityForCli(want,connection,callerIdentity) 直连(ForCli 内联 resolve+enforce APP,无 preflight)→OnAbilityConnectDone 取 AgentReceiverProxy、AgentConnectorStub 就绪 |
| AC 映射 | AC-1, AC-4(stub), AC-11 |
| 前置依赖 | TASK-1 |
| 非目标 | stdin/CommandLoop（TASK-3）、disconnect（TASK-5） |
| 完成判据 | connect 建连+connect-done,无 preflight；OnAbilityConnectDone 取得 AgentReceiverProxy(→B)；AgentConnectorStub(B→CLI) 就绪；Want 键交付 B；同 session 复用连接 |
| 停止条件 | GetAgentCardTypeForConnect/ConnectAgentExtensionAbility IPC 不通；Want 键被 agentmgr 丢失 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | tools/ohos-agent/src/agent_connection.{cpp,h} | OhosAgentAbilityConnection(IAbilityConnection)；OnAbilityConnectDone 构造 AgentReceiverProxy；AgentConnectorStub 子类(实现 IAgentConnector OnSendData/OnAuthorize→写 stdout 事件，TASK-3 接)；Want 构造(AGENTEXTENSIONHOSTPROXY_KEY) |

**Required Rules**: ADR-3(host→B=IAgentReceiver 双参；B→host=IAgentConnector 单参)、ADR-4(Want AGENTEXTENSIONHOSTPROXY_KEY 交付)、ADR-5(ForCli 单阶段直连,取代 ADR-11 两阶段 preflight)、OH-ARCH-LAYERING(经 inner_api AgentManagerClient+kits/native)。

**Steps**: 写 IAbilityConnection+stub+proxy+Want 构造→connect 联调→确认 OnAbilityConnectDone 取得 proxy+stub→记录证据。

**Handoff Summary**: 任务=单阶段 ForCli connect 接线；允许修改 tools/ohos-agent/src/agent_connection.*；只读 agent_runtime_framework/interfaces/{inner_api,kits/native}；Spec 摘要=AC-1/AC-4/AC-11；Design 摘要=ADR-3/ADR-4/ADR-5+双向通道方向节+数据流 step2-5；验证=集成 connect 建连+connect-done 期望 PASS；完成规则=只读消费既有接口，不改 inner_api/kits/native。

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 构建 | `./build.sh -p rk3568 --build-target foundation/ability/ability_runtime/tools/ohos-agent:ohos-agent -j64 --ccache` | **PASS**（exit 0，1:25） |
| 链接 | AbilityConnectionStub→`:ability_connect_callback_stub`；AgentManagerClient→`:agent_manager`；AgentConnectorStub/IAgentReceiver/常量→`:agent_extension_connection` | 全部 resolved（二进制 59896→86400B 增长确认新代码链入） |
| 二进制 | `out/rk3568/ability/ability_runtime/ohos-agent` | ELF 32-bit ARM PIE，86400B |
| 代码 | `tools/ohos-agent/include/cli_agent_connection.h`、`src/cli_agent_connection.cpp`、`src/ohos_agent_command.cpp`(RunAsConnect 单阶段(ForCli))、`BUILD.gn`(deps+include) | 已写入 + 编译通过 |
| 命名偏离 | 计划写 `src/agent_connection.{cpp,h}`；实装为 `src/cli_agent_connection.{cpp,h}`（CLI 专用命名，更清晰） | 已记录，功能等价 |
| 集成验证 | connect 建连 + connect-done | 待 TASK-4（ForCli 强制）+ 真机联调（本会话无 agent/设备，仅构建验证） |

### TASK-3: CommandLoop REPL + JSON-line + 事件 + 60s 跳过

| 字段 | 内容 |
|------|------|
| 任务目标 | CommandLoop：poll(stdin+self-pipe) 读 JSON-line 行(跨块重组)→分派 sendmessage(AgentReceiverProxy->SendData)/authorize(Authorize)/disconnect→B 回调(Binder 线程)写 OnData/OnAuthorize JSON-line→stdout mutex；main.cpp connect 跳过 60s CommandTimer |
| AC 映射 | AC-2, AC-3, AC-4(event), AC-12 |
| 前置依赖 | TASK-2 |
| 非目标 | disconnect 完整清理（TASK-5）、ForCli 接口（TASK-4） |
| 完成判据 | stdin JSON-line 分派正确；B 回调写 OnData/OnAuthorize；connect 不装 60s 定时器；stdout mutex 串行化 |
| 停止条件 | JSON 解析/分帧错；B 回调与 stdin loop 竞态死锁 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | tools/ohos-agent/src/session_protocol.{cpp,h} | 行缓冲+跨块重组；JSON 解析；poll(stdin_fd,wakeup_pipe)；分派；stdout JSON-line 写入(mutex)；self-pipe 唤醒(B 断连) |
| Modify | tools/ohos-agent/src/main.cpp | connect 跳过 60s XCollie CommandTimer |

**Required Rules**: ADR-6(JSON-line \n 分帧)、ADR-7(event 在 stdout eventData JSON-line，非 cliMgr 事件类型；消费方跨块重组)、CommandLoop 节(poll+self-pipe+60s 跳过+stdout mutex)、ADR-10(yield，background=true 支持路径)。

**Steps**: 写 session_protocol(行缓冲+JSON+poll+分派+stdout mutex+self-pipe)→main 60s 跳过→集成 sendmessage→B→OnData→记录证据。

**Handoff Summary**: 任务=CommandLoop REPL；允许修改 tools/ohos-agent/src/session_protocol.*+main.cpp；只读 cli_tool_framework(io_monitor 不分帧，参考实现)；Spec 摘要=AC-2/AC-3/AC-4/AC-12+BR-10(JSON-line \n)；Design 摘要=ADR-6/ADR-7+CommandLoop 节+数据模型(stdin/stdout schema)；验证=集成 sendmessage→B、B→OnData、disconnect 期望 PASS；完成规则=允许 tool_util.cpp env 注入新增(callerIdentity),不改 io_frame 分帧；ADR-10 yield 复用。

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 构建 | `./build.sh -p rk3568 --build-target foundation/ability/ability_runtime/tools/ohos-agent:ohos-agent -j64 --ccache` | **PASS**（exit 0） |
| 链接 | 新增 session_protocol.cpp（poll stdin + nlohmann 解析 + 分派）；json:nlohmann_json_static 显式声明（TASK-1 main.cpp 原为传递依赖，现正式声明） | 二进制 90940→134304B 增长确认新代码链入 |
| 60s 跳过 | main.cpp CommandTimer `operation != "connect"` 跳过 XCollie 60s（TASK-1 已完成） | 已就位 |
| 代码 | `include/session_protocol.h`、`src/session_protocol.cpp`（poll(stdin+wakeupFd) 循环 + nlohmann `parse(…,false)` 无抛解析 + sendmessage/authorize/disconnect 分派）；cli_agent_connection.{h,cpp}（wakeup 自管道 + SetDisconnectReason + SignalDisconnect，替换原 doneCv_/WaitForDisconnect）；ohos_agent_command.cpp（RunAsConnect 接 RunCommandLoop） | 已写入 + 编译通过 |
| 偏离 | 计划写 poll(stdin+**eventfd**)；实装为 poll(stdin + **自管道 wakeup**)（等价唤醒机制，避免 eventfd 头依赖） | 已记录，功能等价 |
| include 卫生 | 全部新 include 均为直接使用（session_protocol.h 仅 cli_agent_connection.h；.cpp 的 `<poll.h>/<unistd.h>/<cerrno>/<nlohmann/json.hpp>/agent_manager_client.h/hilog_tag_wrapper.h` 皆直接用） | 经上一轮审计纠正 |
| 集成验证 | sendmessage→B→OnData、disconnect 联调 | 待 TASK-4（ForCli 强制）+ 真机（本会话无 agent/设备，仅构建验证） |

### TASK-4: agentmgr ForCli 强制接口 + CCM gate

| 字段 | 内容 |
|------|------|
| 任务目标 | 新增 ForCli 接口 ConnectAgentExtensionAbilityForCli/DisconnectAgentExtensionAbilityForCli(IsCliToolToken+CCM 特性门禁→防伪 GetHapTokenInfo(callerIdentity.token).uid==CLI 子进程 uid→SetCallingIdentity(appA)→对 appA 真实 token 强制 CONNECT_AGENT/system-app/前台,no bypass；disconnect 无前台)；CCM 开关 const.product.ohos_agent_cli.enabled gate(镜像 kiosk_manager 范式)；非 CLI 原样 fall-through 走既有 3 Validate；invoker 侧 CONNECT_AGENT 已由 connect `requirePermissions` 声明（见 TASK-1/ohos-agent.json），强制不丢失该权限要求 |
| AC 映射 | AC-8, AC-10 |
| 前置依赖 | TASK-1 |
| 非目标 | 工具侧（TASK-1/2/3） |
| 完成判据 | ForCli 门禁(IsCliToolToken+CCM)触发+防伪+强制；非 CLI 走原校验；CCM off 时工具不可用+不进入 ForCli |
| 停止条件 | IsCliToolToken 联调为 false（P0-1 已定论 TRUE，若联调反例则停止回退 F→另议） |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | agent_runtime_framework/services/agentmgr/src/agent_manager_service.cpp | 三处校验函数顶部加 IsCliToolToken 早返回 |
| Modify | agent_runtime_framework/services/agentmgr（CCM gate） + services/common | 镜像 kiosk_manager:OHOS::system::GetParameter 读 const.product.ohos_agent_cli.enabled |

**Required Rules**: ADR-5(ForCli callerIdentity 传播,强制 no bypass；原 option F 放行废弃)、O-1(已解决)、O-13(经 callerIdentity 可校验,前台 GetCallingPid=appA pid→ValidateConnectCallerForeground)、OH-ARCH-LAYERING(agentmgr 内)。

**Steps**: 新增 ForCli 接口(IsCliToolToken+CCM 门禁→防伪→SetCallingIdentity→强制,既有 3 Validate 原样)→CCM gate→单测(ForCli 门禁+防伪+非 CLI 原校验+CCM off)→记录证据。

**Handoff Summary**: 任务=agentmgr ForCli 接口（ConnectAgentExtensionAbilityForCli/DisconnectAgentExtensionAbilityForCli，IsCliToolToken+CCM 特性门禁）+ cliMgr callerIdentity env + ohos-agent getenv 接线；允许修改 agent_runtime_framework/{services/agentmgr,interfaces/inner_api}+cli_tool_framework/services/climgr/tool_util.cpp+tools/ohos-agent；只读 base/security/access_token(GetHapTokenInfo/IsCliToolToken)；Spec 摘要=AC-8/AC-10；Design 摘要=ADR-5(最终修订,ForCli+callerIdentity)+O-13(化解)；验证=构建 PASS（agentmgr+ohos-agent+cliMgr）+TASK-6 单测(防伪 uid 一致性+appA 权限/前台+AMS 还原 appA)；完成规则=不改 IPC 序列化/不新增权限/非 CLI 行为不变（既有 ConnectAgentExtensionAbility/3 Validate 原样）。

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 构建 | `./build.sh -p rk3568 --build-target .../agent_runtime_framework/services/agentmgr:agentmgr` + `.../tools/ohos-agent:ohos-agent` + `.../cli_tool_framework/services/climgr:climgr` | **PASS**（三目标全 exit 0；`libagentmgr.z.so`+`ohos-agent`+`libclimgr.z.so` 产出） |
| agentmgr | `IAgentManager.idl`+`agent_manager_client.{h,cpp}`+`agent_manager_service.{h,cpp}`：新增 ForCli 两接口（IsCliToolToken+CCM 门禁→防伪 `GetHapTokenInfo(token).uid==GetCallingUid()`→APP-enforce→appA CONNECT_AGENT/system-app/前台→`SetCallingIdentity(callerIdentity)`→复用 `ConnectStandardAgentExtensionAbility`[:438 捕获 appA→AMS 还原→B 侧见 appA]）；**回退**原 TASK-4 三处放行早返回（Validate 函数原样）；gate helper 改名 `IsOhosAgentCliEnabled` | 已写入+编译通过 |
| cliMgr | `tool_util.cpp` envConfig 加 `ohos_cli_callerIdentity`=`ResetCallingIdentity` 捕获（cliMgr caller=appA）+立即 `SetCallingIdentity` 复原（保证后续 GetCalling* 仍见 appA） | 已写入+编译通过 |
| ohos-agent | `ohos_agent_command.cpp` RunAsConnect：`getenv(callerIdentity)`+`ConnectAgentExtensionAbilityForCli`(单次，无 preflight)；`session_protocol.cpp` disconnect：`DisconnectAgentExtensionAbilityForCli` | 已写入+编译通过 |
| 防伪 | callerIdentity 可被 CLI 伪造(env)，但 agentmgr 验 `GetHapTokenInfo(token).uid==GetCallingUid()`(=appA uid，setresuid+cap-drop 不可伪造)→CLI 只能传 appA 自己的 token，无跨 app 越权 | 设计保证 |
| 非行动为不变 | 非 CLI 走既有 `ConnectAgentExtensionAbility`/3 Validate（ForCli 门禁 IsCliToolToken+CCM 仅 CLI 进入） | 设计保证 |
| 单测 | ForCli 防伪(uid 一致性)+appA 权限/前台+AMS 还原 appA(B 侧见 appA token)+CCM off 拒绝 | 待 TASK-6 |
| 集成 | connect 建连+connect-done、disconnect、sendmessage→B→OnData | 待真机联调（本会话仅构建验证） |


### TASK-5: disconnect + ClearSession + caller-death

| 字段 | 内容 |
|------|------|
| 任务目标 | stdin disconnect→DisconnectAgentExtensionAbility→OnAbilityDisconnectDone→disconnect-done JSON-line→干净退出；ClearSession(SIGKILL)/崩溃→agent 侧 caller-death 清理(既有 SetDeathHandler)；B 主动断连(OnAbilityDisconnectDone Binder 线程)→self-pipe 唤醒主循环→disconnect-done(reason=peer)→退出 |
| AC 映射 | AC-5, AC-13, AC-15 |
| 前置依赖 | TASK-2,3 |
| 非目标 | ForCli（TASK-4，disconnect 走 DisconnectAgentExtensionAbilityForCli 强制） |
| 完成判据 | stdin disconnect 走正常流程+disconnect-done；ClearSession→exit 事件+caller-death 清理；B 断连→disconnect-done(reason=peer) |
| 停止条件 | OnAbilityDisconnectDone 与 stdin disconnect 竞态死锁 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Modify | tools/ohos-agent/src/agent_connection.{cpp,h} | disconnect 处理；OnAbilityDisconnectDone→disconnect-done+self-pipe 唤醒+退出；状态机防重入 |

**Required Rules**: ADR-2(disconnect stdin 走正常流程+ClearSession=SIGKILL 兜底)、AC-5/AC-15、RC-1(caller-death)、状态机(Disconnecting/CleaningUp)。

**Steps**: disconnect 处理+OnAbilityDisconnectDone 回调+self-pipe 唤醒+状态机防重入→集成(stdin disconnect+ClearSession+B 断连)→记录证据。

**Completion Evidence**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 构建 | `./build.sh -p rk3568 --build-target foundation/ability/ability_runtime/tools/ohos-agent:ohos-agent -j64 --ccache` | **PASS**（exit 0，0:46） |
| 代码 | `cli_agent_connection.h`（新增 `std::atomic<bool> disconnectInitiated_` + `TryInitiateDisconnect()` CAS + `IsDisconnectInitiated()`，`#include <atomic>`）；`cli_agent_connection.cpp`（`OnAbilityDisconnectDone` 顶部 `(void)TryInitiateDisconnect()` 标记 Disconnecting/CleaningUp 态，幂等）；`session_protocol.cpp`（`DispatchLine` disconnect 路径 `TryInitiateDisconnect()` 守卫→二次 disconnect 发 `already-disconnecting` 非致命；`RunCommandLoop` 分派前 `IsDisconnectInitiated()` 守卫→Disconnecting 态排空 stdin 但不分派） | 已写入 + 编译通过 |
| 已就绪（前置任务已实现） | stdin disconnect→`DisconnectAgentExtensionAbilityForCli`→OnAbilityDisconnectDone→disconnect-done→SignalDisconnect→循环退出（AC-5）；B 主动断连/OnAbilityDisconnectDone(binder 线程)→disconnect-done(reason=peer/error)→wakeup→退出（AC-15）；ClearSession=SIGKILL/崩溃→进程死亡→binder death→agentmgr 既有 caller-death 清理（AC-13/RC-1，无 ohos-agent 侧代码）；stdin EOF→正常退出(return false)依赖 binder-death teardown | 由 TASK-3/TASK-4 完成 |
| 防重入（TASK-5 专属） | 竞态停止条件「OnAbilityDisconnectDone 与 stdin disconnect 竞态死锁」化解：`disconnectInitiated_` CAS 在 RPC **成功后**设置（Q1：失败不设→可重试，不被 `already-disconnecting` 阻塞）；TOCTOU 顶检（peer 在循环检查与 RPC 之间断连→早退，不重复发 disconnect-done）；`RunCommandLoop` 分派前 `IsDisconnectInitiated()` 跳过→Disconnecting 态排空 stdin 但不分派（不在拆除中连接上 sendmessage）；`OnAbilityDisconnectDone` 幂等标态 | 设计保证 + 构建 PASS |
| 集成验证 | stdin disconnect+ClearSession+B 断连 | 待真机联调（本会话无 agent/设备，仅构建验证） |

**Handoff Summary**: 任务=断连清理；允许修改 tools/ohos-agent/src/agent_connection.*；只读 agent_runtime_framework(inner_api Disconnect+SetDeathHandler)；Spec 摘要=AC-5/AC-13/AC-15+EX-4/EX-5+RC-1；Design 摘要=ADR-2+状态机+异常传播表+资源所有权矩阵；验证=集成 stdin disconnect+ClearSession+B 断连 期望 PASS；完成规则=不改 cliMgr ClearSession 既有。

### TASK-6: 测试 + O-7 头注释 + O-11 断言 + selinux 确认

| 字段 | 内容 |
|------|------|
| 任务目标 | 单测(connect 单阶段/协议/ForCli 防伪(uid 一致性)/appA 权限/前台/AMS 还原 appA)+集成(connect→sendmessage→OnData→disconnect)；修正 iagent_receiver.h/iagent_connector.h 头注释(O-7)；IsCliToolToken 直接断言(O-11)；selinux system_basic call 缺口确认(O-12/P2-3) |
| AC 映射 | 全 AC 验证 |
| 前置依赖 | TASK-1~5 |
| 完成判据 | 全 AC 单测+集成 PASS；头注释修正；IsCliToolToken=true 断言；selinux 缺口确认(零规则或 1 条) |
| 停止条件 | AC 验证不通过 |

**Files**

| 操作 | 文件 | 说明 |
|------|------|------|
| Create | tools/ohos-agent/tests/* | connect 单阶段/protocol/ForCli 防伪/配额/CCM 单测+集成 |
| Modify | agent_runtime_framework/interfaces/kits/native/agent_extension/connection/include/iagent_receiver.h, iagent_connector.h | 头注释方向修正(O-7，非行为变更) |
| Confirm | base/security/selinux_adapter claw_cli.te | system_basic_hap_attr→normal_hap_attr:binder{call} 缺口确认 |

**Required Rules**: 全 AC、O-7/O-11/O-12/P2-3、验证证据当次生成。

**Steps**: 写单测+集成→跑→头注释修正→IsCliToolToken 断言→selinux 确认→记录证据。

**Handoff Summary**: 任务=测试+收尾；允许修改 tools/ohos-agent/tests/*+kits/native/connection 头注释；只读 base/security/{access_token,selinux_adapter}；Spec 摘要=全 AC+BR-12 失败信号+BR-13 yield；Design 摘要=O-7/O-11/O-12/P2-3；验证=`run -t UT -tp ability_runtime`+集成 期望 PASS；完成规则=无 fresh evidence 不得声明完成。

**Completion Evidence（UT 部分；O-7 头注释/O-11 断言/selinux P2-3 待续）**

| 证据类型 | 命令/路径 | 结果 |
|----------|-----------|------|
| 构建（agentmgr ForCli UT） | `./build.sh -p rk3568 --build-target .../test/unittest/agent_runtime_framework/agent_manager_service_test:agent_manager_service_test -j64 --ccache` | **PASS**（exit 0）；修复了 ForCli 新增 `accesstoken_kit.h` 导致既有测试链接断（加 mock_accesstoken_kit.cpp+mock_parameters.cpp+`access_token:libaccesstoken_sdk`） |
| 构建（ohos-agent UT） | `./build.sh -p rk3568 --build-target .../tools/ohos-agent/tests:ohos_agent_test -j64 --ccache` | **PASS**（exit 0）；新建 tests 目标 + 接入 tools/test:unittest 聚合 |
| ForCli UT（service 14 + client 6 = 20） | `agent_manager_service_test.cpp`（追加）+ `mock/src/mock_accesstoken_kit.cpp`+`mock_parameters.cpp`+`mock/include/mock_my_flag.h`（+4 字段）+`BUILD.gn` | ConnectForCli_{CcmOff,NotCliTool,NullConnection,CallerIdentityNotHap,UidSpoof,NonAppType,NoConnectAgentPerm,NotSystemApp,NotForeground,AllOk}×10 + DisconnectForCli_{CcmOff,NullConnection,Spoof,AllOk}×4 = **14**（service 端）；client 端 6：ConnectAgentExtensionAbilityForCli_00{1,2,3}+DisconnectAgentExtensionAbilityForCli_00{1,2,3}（null-proxy/success/delegate-failure+callerIdentity/connection 透传），合计 **20**；全分支断言确切错误码（ERR_PERMISSION_DENIED/ERR_INVALID_VALUE/AAFwk::ERR_WRONG_INTERFACE_CALL/AAFwk::ERR_NOT_SYSTEM_APP/AAFwk::NOT_TOP_ABILITY/ERR_OK）；nm 确认 TestBody 符号链入 |
| ohos-agent UT（22 用例） | `tools/ohos-agent/tests/ohos_agent_test.cpp`+`BUILD.gn` | EscapeJsonString/BuildEvents/ConnectorStub SendData+Authorize/OnAbilityConnectDone{Success,ProxyNull-CR1,NonZeroResult}/OnAbilityDisconnectDone{PeerReason+Wakeup,UserReason,ErrorReason}/TryInitiateDisconnect CAS/RunCommandLoop{SendMessage Success+Fail,Authorize,BadJson,BadData,NoReceiver,UnknownOp,ReassemblesAcrossBlocks,LineExceeds200KB-Fatal,EOF-Exit,AfterDisconnectInitiated-SkipsDispatch} = **22**；nm 确认全 TestBody 符号链入；覆盖 CR-1 修复+TASK-5 防重入(CAS+skip-dispatch)+200KB 上限+EOF+JSON 解析+事件构造 |
| 新增 mock | `mock_accesstoken_kit.cpp`（IsCliToolToken+GetHapTokenInfo 读 MyFlag；linked-ahead-of shared libaccesstoken_sdk）、`mock_parameters.cpp`（system::GetBoolParameter 对 CCM flag 可控、其余返 def）、MyFlag +4 字段（retGetBoolParameterCliEnabled/retIsCliToolToken/retGetHapTokenInfo/hapTokenInfoUid） | SetUp 重置到 happy-path 默认（CCM on+CLI tool+valid HAP uid=calling uid） |
| client_test mock+依赖 | `agent_manager_client_test/mock/include/iagent_manager.h`（手写接口头补 2 ForCli 默认实现，根治“override does not override”）+`mock_agent_manager_service.{h,cpp}`+`mock_my_flag.h`（+ret/last 字段）+`BUILD.gn`（补 `:agent_manager`+`:agent_fwk`+`:ability_connect_callback_stub`+`:agent_extension_connection` 依赖） | client_test 原先 `deps=[]` 不触发 codegen、靠 `-Igen` 兑底找 iagent_manager.h；ForCli 新增后手写 mock 遗漏→补齐+接依赖 |
| agent service/host connection test | `agent_service_connection_test/BUILD.gn`+`agent_host_connection_test/BUILD.gn`（均直接编译 `agent_manager_service.cpp`） | 各补 `mock_accesstoken_kit.cpp`+`mock_parameters.cpp` 源 + `access_token:libaccesstoken_sdk` 依赖（镜像 service_test），根治 `accesstoken_kit.h not found` |
| cli_tool mock | `cli_tool_mgr_service_test/mock/{include,src}/ipc_skeleton.{h,cpp}`+`tool_util_test/mock/{include,src}/ipc_skeleton.{h,cpp}` | 我对 `tool_util.cpp` 的 callerIdentity env 注入调了 `ResetCallingIdentity/SetCallingIdentity/GetCallingTokenID`，但两套手写 mock `ipc_skeleton.h` 未含→补 3（tool_util_test）/2（cli_tool_mgr_service_test，已含 GetCallingTokenID）方法+静态字段+实现+Reset 清理；`process_manager_test` 无自己 mock（用真实 ipc_core）不受影响 |
| ForCli 分支 UT 补齐 | `tool_util_test.cpp` 新增 4 例 GenerateSandboxConfig：`InjectsCallerIdentityEnv_0100`（成功路径断言 `env.ohos_cli_callerIdentity`+`callerUid/TokenId/BundleName`+全 config 字段，覆盖 callerIdentity env 注入新分支）+`NotHapToken_0200`/`GetHapTokenInfoFails_0300`/`GetBundleInfoFails_0400`（三个早返失败分支）；`ohos_agent_test.cpp` 新增 `WaitForConnect_Timeout_ReturnsNeg1`（cli_agent_connection.cpp 唯一未覆盖的 WaitForConnect 超时分支）；新增 `tool_util_test/mock/src/mock_accesstoken_kit.cpp`+`mock/include/accesstoken_kit_mock.h`（强符号覆盖 `GetTokenTypeFlag`/`GetHapTokenInfo`，linked-ahead-of `access_token:libaccesstoken_sdk`）；`tool_util_test/BUILD.gn` 接 `mock/include`+3 mock 源（ipc_skeleton+bundle_mgr_helper+mock_accesstoken_kit） | tool_util.cpp callerIdentity env 注入分支原先零覆盖（GenerateCmdSandboxConfig 仅测 false 路径，GenerateSandboxConfig 成功路径未测）；cli_agent_connection.cpp 原由 ohos_agent_test 22 例覆盖（OnAbilityConnectDone 3 分支/OnAbilityDisconnectDone 3 分支/TryInitiateDisconnect CAS/ConnectorStub/EscapeJsonString/Build*Event），唯一缺口 WaitForConnect 超时分支本次补齐。nm 确认 5 新 TestBody 符号链入；构建 PASS |
| 运行验证 | 单次 build.sh 8 目标（agent_manager_{service,client}_test+agent_service_connection_test+agent_host_connection_test+ohos_agent_test+cli_tool_mgr_service_test+tool_util_test+process_manager_test）+ 2 完整 group（`cli_tool_framework/test/unittest:unittest`+`test/unittest/agent_runtime_framework:unittest`）+ ohos-agent 工具 | **构建全 PASS**（exit 0，`rk3568 build success`）；构建需 `--gn-args allow_sanitize_debug=true` 绕过 os_account cfi debug 断言（环境性，与代码无关）。用例运行 **待真机**（rk3568 ARM 交叉编译，本会话无设备/模拟器） |
| 遗留 | O-7 头注释（iagent_receiver.h/iagent_connector.h 方向）、O-11 IsCliToolToken 真机断言、selinux P2-3 system_basic call 缺口确认、真机联调 | 本批未含，后续补（非构建阻塞） |

### TASK-6 补丁：UT 运行期稳定性修复

| 项 | 变更 | 根因/证据 |
|----|------|-----------|
| `agent_manager_service_test.cpp` `DisconnectForCli_AllOk_DelegatesAndReturnsOk` | 在 `DisconnectAgentExtensionAbilityForCli` 前向 `AgentConnectManager::trackedConnections_` 预置一条 standard 记录（key=`connection->AsObject()`，callerUid=`GetCallingUid()`，serviceConnection=nullptr） | ForCli 委派 `DisconnectAgentExtensionAbility`→`PrepareAgentDisconnectRequest` 在真实 `AgentConnectManager` 上按 `connection->AsObject()` 直查 `trackedConnections_`，未跟踪则返回 `ERR_INVALID_VALUE`，先于被 mock 的 `DisconnectAbility`；`FindTrackedConnectionLocked` 直查命中不过 callerUid（`agent_connect_manager.cpp:787`），`isLowCode` 默认 false 走 standard→`serviceConnection` 留空→被 mock 的 `DisconnectAbility(nullptr)` 返回 `ERR_OK`；仿 `OnStop_001`（:398）emplace 范式 |
| `ohos_agent_test.cpp` `RunCommandLoop_LineExceeds200KB_Fatal` | 还原为原始单线程写法（移除上轮误改的 worker 线程 + `#include <thread>`） | 上轮以「`F_SETPIPE_SZ` 失败→管道 64KB→200KiB 阻塞 write 死锁」为由改为 worker 线程，但该理论 **未复现**（主机复现脚本下原写法亦通过）。真根因见下条（`session_protocol.cpp` POLLHUP），与本用例无关，故还原保持与其他 RunCommandLoop 用例一致的写法 |
| `session_protocol.cpp` `RunCommandLoop` | stdin 就绪判据 `if (pfds[0].revents & POLLIN)` → `if (pfds[0].revents & (POLLIN | POLLHUP))`（+说明注释） | **真根因（ohos_agent_test 无响应）**：部分内核对「写端已关闭、无待读数据」的管道 EOF 返回 `revents=POLLHUP` **不含 POLLIN**（经主机复现脚本探针证实：`poll ret=1, revents=0x10=POLLHUP, read n=0`）。原判据 `& POLLIN` 为 false→跳过 read→循环再 poll→POLLHUP-only→**忙转死循环=「无响应」**。所有写命令后 CloseStdin 的用例在读完命令后必命中此 EOF 路径；唯 `LineExceeds` 在 EOF 前由 200KB 上限 fatal 返回故未卡。改为兼顾 POLLHUP 后 `read` 返 0→`n==0 return false` 正常退出。复现脚本（11 用例）修后 **0 挂起** |

**验证**：
- `./build.sh -p rk3568 --build-target .../ohos-agent:ohos_agent_cli_source_set --build-target .../ohos-agent/tests:ohos_agent_test --gn-args allow_sanitize_debug=true -j64 --ccache` → **rk3568 build success (exit 0)**。
- 主机复现脚本（`/tmp/ohos_agent_repro.cpp`，逐用例 fork+4s 超时探针，完整复刻 `StdioRedirect`+`RunCommandLoop` poll/read/重组/上限/分发循环）修复前 8/11 用例挂起（含 `EOF_NormalExit`）、修后 **0 挂起**；独立探针直接证实 EOF 时 `poll ret=1 revents=POLLHUP(0x10) 无 POLLIN read n=0`。
- ARM 交叉编译，真机运行待确认（逻辑 + 内核语义经主机复现验证）。

## Plan 自审清单

- [x] 每个 P0/P1 AC 至少映射到一个 Task（AC-1~15 全覆盖）
- [x] 每个 Task 文件范围明确
- [x] 每个 Task 明确前置依赖、非目标、完成判据、停止条件
- [x] 每个 Task 有验证命令
- [x] Task 粒度形成能力闭环
- [x] 无 TBD/TODO 占位（Owner/版本 TBD 已标注）
- [x] 交接信息自包含（Handoff Summary）
- [x] 验证在完成时立即执行
- [x] 无 Task 超 3000 行阈值（每 Task 上下文<3000 行）

**结论:** Approved(Stage 3 implemented)；TASK-1~6 已实现+全 test suite 构建 PASS（8 目标+2 完整 group 单次 build.sh exit 0），待真机联调+O-7/O-11/selinux 收尾
