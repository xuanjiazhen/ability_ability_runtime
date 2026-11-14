# CLI 工具取消能力开发规范

## 1. 概述

CLI 工具框架提供两种取消运行中会话的手段：

| 接口 | 方式 | 语义 | 适用场景 |
|------|------|------|----------|
| `cliManager.cancelSession(sessionId, signal?)` | 协作式（POSIX 信号 `SIGINT`/`SIGTERM`） | 可协商：工具收到信号后有机会清理现场、落盘、回传部分结果，再自行退出 | 工具声明了 `supportsCancel` |
| `cliManager.clearSession(sessionId)` | 强制式（`SIGKILL`，不可捕获） | 不可协商：进程立即被杀死，清理订阅并置会话为 CANCELLING | 兜底手段，任何会话可用 |

工具是否支持协作式取消由工具在 `config.json` 中**主动声明**（`supportsCancel` 字段）；未声明的存量工具行为完全不变，只能走 `clearSession` 强制取消。

## 2. supportsCancel 声明方式与优先级

### 2.1 声明位置

`supportsCancel` 为可选布尔字段，缺省 `false`，有两个声明层级：

- **工具级**：`config.json` 顶层字段，声明整个工具是否支持协作式取消；
- **子命令级**：`config.json` 中 `subcommands.<name>` 内的字段，声明单个子命令是否支持协作式取消。

```json
{
    "name": "ohos-timer",
    "version": "1.0.0",
    "description": "Sample timer tool",
    "executablePath": "/system/bin/cli_tool/ohos-timer",
    "requirePermissions": [],
    "inputSchema": { "type": "object", "properties": {} },
    "outputSchema": { "type": "object", "properties": {} },
    "hasSubCommand": true,
    "subcommands": {
        "run": {
            "description": "Run the timer; supports cooperative cancel",
            "requirePermissions": [],
            "inputSchema": { "type": "object", "properties": {} },
            "outputSchema": { "type": "object", "properties": {} },
            "supportsCancel": true
        },
        "version": {
            "description": "Print the version; not declared cancellable",
            "requirePermissions": [],
            "inputSchema": { "type": "object", "properties": {} },
            "outputSchema": { "type": "object", "properties": {} }
        }
    }
}
```

### 2.2 优先级规则

取值优先级与 `inputSchema` 的现有回退规则一致：

```
子命令级声明 > 工具级声明 > 缺省 false
```

- 子命令**已声明**（含显式 `false`）：以子命令值为准，覆盖工具级声明；
- 子命令**未声明**：回退工具级声明（查询接口返回的 `SubCommandInfo.supportsCancel` 为继承后的有效值）；
- 两级均未声明：为 `false`，即不支持协作式取消。

### 2.3 声明可见性

- `ToolInfo.supportsCancel` / `SubCommandInfo.supportsCancel` 经 `getToolInfoByName`/`queryTools` 全链路可查询；调用方可在执行前判断能否取消，再决定是否调用 `cancelSession`；
- `queryToolSummaries`（轻量列表）**不携带**该字段；
- `supportsCancel` 仅声明能力，不授予任何权限；调用 `cancelSession` 仍受 `ohos.permission.EXEC_CLI_TOOL` 权限与会话归属约束；
- `execCmd` 创建的 shell 会话与 SKILL 会话恒为"不支持"，调用 `cancelSession` 返回错误码 `35600037`。

### 2.4 构建校验

`config.json` 中的 `supportsCancel` 字段须先经 build 仓 CLI 配置校验白名单登记后方可通过构建；校验仅做**布尔类型检查**（非布尔类型报错），不做语义拦截。

## 3. CancelSignal 信号语义

```ts
enum CancelSignal {
    SIGINT = 2,    // 中断信号（Ctrl-C 同款），cancelSession 缺省信号
    SIGTERM = 15   // 终止信号，约定的优雅退出信号
}
```

| 成员 | 值 | 语义 |
|------|-----|------|
| SIGINT | 2 | 中断信号，`cancelSession(sessionId)` 省略 `signal` 参数时的缺省取值 |
| SIGTERM | 15 | 终止信号，工具约定完成清理后退出 |

- 取值与 Linux 信号值对齐，IDL（`ICliToolData.idl`）、NAPI（`cliManager.CancelSignal`）、d.ts 三处一致；
- 仅允许传入枚举成员值（2/15）；传入其它值（如 `9`/`0`/`-1`，即试图传 `SIGKILL` 等任意信号）在 NAPI 层与服务端双重校验拒绝，返回 `401`，不发送任何信号；
- 枚举为增量扩展设计：后续版本可新增成员（如 `SIGHUP`），已发布的成员值不得变更。

## 4. 工具侧信号处理样例

工具只需用 `sigaction` 捕获 `SIGINT`/`SIGTERM`，在清理后自行退出即可参与协作式取消，无需约定任何私有消息格式。完整样例见 `cli_tool_framework/tools/ohos-timer/src/main.cpp`，核心骨架：

```cpp
// 信号处理函数：仅允许异步信号安全操作（write、volatile sig_atomic_t 赋值）。
// 禁止在处理函数中调用 stdio（printf/fprintf）或 HiLog——可能死锁。
volatile sig_atomic_t g_cancelSignal = 0;

void OnCancelSignal(int sig)
{
    int savedErrno = errno;
    g_cancelSignal = sig;   // 仅置标志，清理动作留在主循环
    static const char msg[] = "\n[cancel] received signal\n";
    (void)write(STDERR_FILENO, msg, sizeof(msg) - 1);  // write 是异步信号安全的
    errno = savedErrno;
}

int main(int argc, char *argv[])
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = OnCancelSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGTERM, &action, nullptr);

    while (Running()) {
        if (g_cancelSignal != 0) {
            break;              // 主循环发现取消标志后退出循环
        }
        DoWork();
    }

    if (g_cancelSignal != 0) {
        FlushPartialResult();   // 清理与部分结果落盘在正常上下文中完成
        return 128 + g_cancelSignal;  // 推荐退出码：128 + 信号值（130 = SIGINT，143 = SIGTERM）
    }
    return 0;
}
```

要点：

1. **处理函数只置标志**：真正的清理（落盘、输出部分结果、释放资源）回到主循环的正常上下文中执行；
2. **处理函数内禁用 stdio/HiLog**：信号可能打断持有 stdio 内部锁的代码，在处理函数中再次调用会死锁；
3. **推荐退出码 `128 + 信号值`**：与 shell 对信号致死的编码约定一致（`SIGINT` → 130，`SIGTERM` → 143），调用方可据此区分"被取消"与"执行失败"；工具也可自行约定其它退出码，框架不做强制；
4. 收到信号后工具可运行任意时长完成清理（落盘、回传部分结果），框架不设超时、不催促。

## 5. 与 clearSession 的配合（推荐流程）

`cancelSession` 是"尽力而为"语义：resolve 仅表示**信号已投递**，不表示工具已退出，也不改变会话状态与订阅关系。推荐调用流程：

```ts
import cliManager from '@ohos.app.cli.cliManager';

// 1. 先尝试协作式取消（默认 SIGINT）
await cliManager.cancelSession(sessionId);

// 2. 若工具未退出（querySession 仍为 running 或订阅未收到 exit 事件），显式兜底强杀
await cliManager.clearSession(sessionId);
```

框架**不会**在超时后自动升级为 `SIGKILL`：是否升级由调用方根据业务节奏显式决定，避免框架隐式强杀剥夺工具的清理机会。

## 6. cancelSession 接口契约

| 属性 | 值 |
|------|-----|
| 签名 | `cancelSession(sessionId: string, signal?: CancelSignal): Promise<void>` |
| 权限 | `ohos.permission.EXEC_CLI_TOOL`（复用既有权限，不新增） |
| 前置条件 | 系统应用；持有权限；会话存在且归属调用方；会话处于 RUNNING/SPAWNING；工具/子命令已声明 `supportsCancel: true` |
| 副作用 | 一次信号投递；不改变会话状态、不清理订阅、不触发 EXIT 事件（EXIT 由工具退出后的既有回收流程产生） |
| 幂等性 | 非幂等承诺：重复调用会再次发送信号；会话已结束后调用返回 `35600032` |

错误码：

| 错误码 | 含义 | 触发条件 |
|--------|------|----------|
| 201 | 权限拒绝 | 缺 `ohos.permission.EXEC_CLI_TOOL`，或调用方非会话归属方 |
| 202 | 非系统应用 | 调用方不是系统应用 |
| 401 | 参数无效 | `sessionId` 非法，或 `signal` 非 `CancelSignal` 枚举成员值 |
| 35600032 | 会话不存在 | 会话不存在，或会话非 RUNNING/SPAWNING 态 |
| 35600037 | 工具不支持取消 | 会话对应的工具/子命令未声明 `supportsCancel`（含 execCmd/SKILL 会话） |
| 35600038 | 信号发送失败 | 进程组投递失败且回退单进程投递亦失败（如进程刚退出） |

## 7. 注意事项

1. **resolve 不等于退出**：`cancelSession` 的 Promise resolve 仅代表信号已投递到工具进程；工具何时退出由其清理逻辑决定，调用方应通过 `querySession` 或订阅 exit 事件确认最终退出；
2. **不自动升级**：框架不做"超时自动 SIGKILL"；未观察到退出时由调用方显式调用 `clearSession` 兜底；
3. **会话状态不变**：取消后立即 `querySession`，会话状态与订阅关系与取消前一致（尽力而为语义）；
4. **取消判定按会话创建时的子命令**：`cancelSession` 按会话创建时落库的子命令反查声明；会话运行期间配置文件变更不影响既有会话的取消能力判定；
5. **忽略信号的工具**：不处理 `SIGINT`/`SIGTERM` 的工具收到信号后行为不变（信号被默认处置或忽略），`clearSession` 兜底始终可用；
6. **存量工具零改造**：未声明 `supportsCancel` 的工具配置加载行为不变，对外暴露 `false`，`cancelSession` 对其返回 `35600037`，进程不受影响。
