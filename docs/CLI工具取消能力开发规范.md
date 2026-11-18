# CLI 工具取消能力开发规范

## 1. 概述

CLI 工具框架在 `execTool`（启动工具）、`sendMessage`（向工具标准输入写文本）、`clearSession`（强制杀死工具进程）之外，提供了一条**协作式取消**通路：

- **工具侧**：工具在 `config.json` 中通过 `cancelable` 字段声明自身（或某个子命令）支持取消；
- **调用方侧**：调用方通过 `cliManager.cancelSession(sessionId, signal?)` 向工具进程发送取消信号（`SIGINT` 默认 / `SIGTERM`），工具捕获信号后自行完成清理并退出。

`cancelSession` 与 `clearSession` 的分工：

| 接口 | 方式 | 工具可否清理 | 典型用途 |
| ---- | ---- | ---- | ---- |
| `cancelSession` | 协作式，发送 `SIGINT`/`SIGTERM` | 可以（工具捕获信号后自行清理、落盘、回传部分结果） | 优先选择的取消方式 |
| `clearSession` | 强制式，发送 `SIGKILL` | 不可以（进程立即终止） | 工具未响应取消信号时的兜底 |

本文面向两类读者：

- **CLI 工具实现方**：如何声明 `cancelable`、如何正确处理 `SIGINT`/`SIGTERM`；
- **调用方**（系统应用，如语音助手）：`cancelSession` 的语义、错误码、与 `clearSession` 的配合方式。

## 2. cancelable 声明方式与两级优先级

### 2.1 声明位置与格式

`cancelable` 为**可选布尔字段**，缺省 `false`，可在两个位置声明：

| 级别 | 位置 | 作用范围 |
| ---- | ---- | ---- |
| 工具级 | `config.json` 顶层 | 该工具的所有子命令（未被子命令覆盖时） |
| 子命令级 | `config.json` 的 `subcommands.<name>` 内 | 仅该子命令 |

工具级声明（无子命令、整体可取消的工具）：

```json
{
  "name": "ohos-example",
  "version": "1.0.0",
  "description": "Example CLI tool",
  "executablePath": "/system/bin/cli_tool/executable/ohos-example",
  "hasSubCommand": true,
  "cancelable": true,
  "requirePermissions": [],
  ...
}
```

子命令级声明（工具级不声明，仅部分子命令可取消；本仓 `tools/ohos-timer` 即此示范）：

```json
{
  "name": "ohos-timer",
  "executablePath": "/system/bin/cli_tool/executable/ohos-timer",
  "hasSubCommand": true,
  "requirePermissions": [],
  "subcommands": {
    "run": {
      "description": "Run the timer; declares subcommand-level cancelable",
      "requirePermissions": [],
      "cancelable": true,
      ...
    },
    "version": {
      "description": "Display version information; cancelable not declared",
      "requirePermissions": [],
      ...
    }
  }
}
```

上例中：工具级**未声明** `cancelable`；子命令 `run` 声明 `true`、`version` 未声明。

### 2.2 两级优先级

取值判定规则（与 `inputSchema` 的既有回退规则一致）：

1. **子命令级声明优先**：子命令一旦声明（含显式 `false`），即以子命令级为准，覆盖工具级声明；
2. **子命令级未声明时回退工具级**；
3. **两级均未声明时为 `false`**（默认不支持取消）。

判定矩阵：

| 工具级声明 | 子命令级声明 | 该子命令实际生效值 |
| ---- | ---- | ---- |
| 未声明 | `true` | `true` |
| 未声明 | 未声明 | `false` |
| `true` | 未声明 | `true` |
| `true` | `false` | `false`（显式 false 覆盖工具级 true） |
| `false` | `true` | `true` |

### 2.3 声明的可见性与边界

- **可见性**：生效值经 `ToolInfo.cancelable` / `SubCommandInfo.cancelable` 透出，调用方可通过 `getToolInfoByName` / `queryTools` 在执行前查询；`queryToolSummaries`（轻量列表）不携带该字段。
- **只声明能力，不授予权限**：声明 `cancelable` 不新增任何权限，`cancelSession` 调用仍受系统应用身份、`ohos.permission.EXEC_CLI_TOOL` 权限与会话归属校验约束。
- **恒为"不支持"的会话**：`execCmd` 创建的 shell 会话与 SKILL 会话没有工具声明，调用 `cancelSession` 会被拒绝。
- **存量兼容**：字段缺省 `false`，未声明该字段的存量配置加载行为不变。
- **构建校验**：字段需在构建校验白名单中登记，仅做布尔类型校验，不做语义拦截；声明值为非布尔类型时构建校验报错。

## 3. CancellationSignal 语义

`cancelSession` 接口签名（System API）：

```ts
cancelSession(sessionId: string, signal?: CancellationSignal): Promise<void>
```

`CancellationSignal` 枚举取值与 Linux 信号值对齐：

| 成员 | 值 | 语义 |
| ---- | ---- | ---- |
| `SIGINT` | `2` | 中断信号（Ctrl-C 同款），取消的**默认**信号 |
| `SIGTERM` | `15` | 终止信号，工具约定的优雅退出信号 |

要点：

- 省略 `signal` 参数时默认发送 `SIGINT`；
- **仅接受枚举成员值 2 / 15**：传入其他任何值（如 `9`、`0`、`-1`）都会抛出 `401`（参数无效），且不发送任何信号——调用方无法通过该接口注入 `SIGKILL` 等任意信号；
- 枚举为增量扩展设计，后续版本可新增成员（如 `SIGHUP`），已发布成员值不得变更；
- 前置条件：系统应用、持有 `ohos.permission.EXEC_CLI_TOOL`、会话存在且归属调用方、会话处于 RUNNING/SPAWNING 态、会话对应子命令的生效 `cancelable` 为 `true`；任一不满足即拒绝且不发送信号。

## 4. 信号处理样例（工具实现方必读）

工具进程由框架以子命令形式拉起（子命令固定拼在 `argv[1]`，会话与进程、子命令一一对应）。工具只需按 POSIX 惯例处理 `SIGINT`/`SIGTERM` 即可参与协作式取消，无需任何私有消息协议。

### 4.1 完整示例

以下为 `tools/ohos-timer` 的核心实现（可直接编译运行的完整源码见该目录 `src/main.cpp`）：

```cpp
#include <signal.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>

namespace {
constexpr int SLEEP_SLICE_MS = 100;                       // 分片睡眠，保证取消响应速度
// 推荐退出码：128 + signo（SIGINT -> 130，SIGTERM -> 143），与 shell 惯例一致
constexpr int EXIT_CODE_SIGNAL_BASE = 128;
// 预分配常量字符串：处理函数内不做任何堆分配
constexpr char CANCEL_HINT[] = "ohos-timer: cancel signal received, exiting after cleanup\n";

// 取消标志：volatile sig_atomic_t 是标准保证可安全跨信号写读的唯一类型
// 0 表示未取消，非 0 为收到的信号编号
volatile sig_atomic_t gCancellationSignal = 0;

// SIGINT/SIGTERM 处理函数：只做异步信号安全操作
void HandleCancellationSignal(int signo)
{
    gCancellationSignal = signo;                                // 写标志位（异步信号安全）
    // write() 输出提示（异步信号安全），返回值可忽略：不依赖它完成清理
    (void)write(STDERR_FILENO, CANCEL_HINT, sizeof(CANCEL_HINT) - 1);
}
} // namespace

bool RegisterCancelHandler()
{
    struct sigaction cancelAction = {};
    cancelAction.sa_handler = HandleCancellationSignal;
    sigemptyset(&cancelAction.sa_mask);
    cancelAction.sa_flags = 0;                            // 不设 SA_RESTART，见 4.4
    if (sigaction(SIGINT, &cancelAction, nullptr) != 0) {
        return false;
    }
    return sigaction(SIGTERM, &cancelAction, nullptr) == 0;
}

// 分片睡眠：每片之间检查取消标志，使工具在 SLEEP_SLICE_MS 内观察到信号
void SleepWithCancelCheck(int milliseconds)
{
    int remainingMs = milliseconds;
    while (remainingMs > 0 && gCancellationSignal == 0) {
        int sliceMs = remainingMs < SLEEP_SLICE_MS ? remainingMs : SLEEP_SLICE_MS;
        std::this_thread::sleep_for(std::chrono::milliseconds(sliceMs));
        remainingMs -= sliceMs;
    }
}

int ExecuteTimer(const TimerConfig& config)
{
    auto startTime = std::chrono::steady_clock::now();
    int elapsed = 0;
    while (elapsed < config.duration && gCancellationSignal == 0) {
        SleepWithCancelCheck(config.interval * 1000);
        elapsed = /* 由 startTime 计算 */;
        // ... 正常业务：输出进度事件 ...
    }

    if (gCancellationSignal != 0) {
        // 在正常上下文完成清理：输出部分结果（actual_duration < duration），
        // 调用方仍能拿到取消前产生的数据
        EmitSuccessResult("{\"duration\":" + std::to_string(config.duration) +
            ",\"actual_duration\":" + std::to_string(elapsed) + "}");
        std::cout.flush();
        std::cerr.flush();
        _exit(EXIT_CODE_SIGNAL_BASE + gCancellationSignal);     // 推荐退出码 128 + signo
    }
    // ... 正常完成路径 ...
    return 0;
}

int main(int argc, char* argv[])
{
    std::string subcommand = argv[1];                     // 子命令固定在 argv[1]
    if (subcommand == "run") {
        if (!RegisterCancelHandler()) {                   // 仅长运行子命令注册处理函数
            return 1;
        }
        return RunCommand(argc, argv);                    // 计时循环，期间收到信号 -> 清理 -> 退出
    }
    if (subcommand == "version") {
        return VersionCommand(argc, argv);                // 打印版本即退出，不声明 cancelable
    }
    return 1;
}
```

### 4.2 处理函数内的异步信号安全清单

信号处理函数在任意指令边界打断主流程后执行，因此**只能调用异步信号安全（async-signal-safe）的操作**：

| 允许（安全） | 说明 |
| ---- | ---- |
| `write(fd, buf, count)` | 输出**预分配常量**缓冲（示例中的 `CANCEL_HINT`）；若缓冲需运行时拼接，不可在处理函数内做 |
| 给 `volatile sig_atomic_t` 标志赋值 | 标准保证的跨信号安全类型；清理动作应放回主流程，观察到标志后执行 |
| `_exit(status)` | 简单场景可直接在处理函数内 `_exit(128 + signo)`，但会失去在正常上下文输出部分结果的机会 |
| `sigaction` 内部再注册/恢复默认处理 | 如需"二次信号直接终止"策略 |

| 禁止（非异步信号安全） | 原因 |
| ---- | ---- |
| `printf` / `fprintf` / `std::cout` / `std::cerr` | stdio 持有锁与缓冲区，主流程可能正处于同一缓冲区操作中 |
| `malloc` / `free` / `new` / `delete`（含 `std::string` 拼接、流操作） | 堆操作不可重入，死锁或堆损坏风险 |
| HiLog（`OHOS::HiviewDFX` 系列） | 内部持锁、做格式化与 IO |
| 任何持锁操作（mutex、sprlock 等） | 主流程可能正持有同一把锁 |

### 4.3 推荐的两阶段清理模式

1. **第一阶段（处理函数内，异步信号安全）**：只做两件事——写标志位、（可选）`write()` 输出固定提示；立即返回；
2. **第二阶段（主流程观察到标志后，正常上下文）**：输出部分结果/进度事件、落盘、关闭资源，然后 `_exit(128 + signo)`。

两阶段模式的收益：清理逻辑可以使用完整的运行时能力（流式输出、动态内存），且与被信号打断的主流程天然互斥。

### 4.4 保证取消响应速度

- 注册 `sigaction` 时**不要设置 `SA_RESTART`**：被信号打断的系统调用应立即返回，让主循环尽快观察到取消标志；
- 长睡眠应**分片**（如示例的 `SleepWithCancelCheck`，每片 ≤100ms 检查一次标志）：`std::this_thread::sleep_for` 的底层 `nanosleep` 在 `EINTR` 后可能静默重试整段时长，一次睡满整个 interval 会把取消响应拖到最坏一个 interval；
- 忙等轮询（sleep 0）不可取：既浪费功耗，也违背样例示范意图。

### 4.5 推荐退出码

工具因取消信号退出时，推荐退出码 `128 + signo`：

| 信号 | 推荐退出码 |
| ---- | ---- |
| `SIGINT`（2） | `130` |
| `SIGTERM`（15） | `143` |

该约定与 shell 惯例一致，便于调用方/测试工具从进程退出码区分"被信号终止"与"正常返回"。框架不强制该退出码（退出码属于工具自身行为约定），但样例工具与测试用例均遵循此值。退出前若使用了流式输出，应先显式 `flush` 再 `_exit`（`_exit` 不冲刷 stdio 缓冲）。

## 5. 与 clearSession 的配合

### 5.1 推荐流程

```
调用方需要取消运行中的工具会话：
  1. cancelSession(sessionId)                  // 默认 SIGINT，协作式
  2. 等待/观察工具是否自行退出（EXIT 事件/进程退出）
  3. 若未观察到退出（工具忽略信号或清理耗时超出调用方耐心）：
       clearSession(sessionId)                 // SIGKILL 强制兜底
```

要点：

- **先协商，后强制**：`cancelSession` 给工具留下清理、落盘、回传部分结果的机会，应优先使用；
- **不自动升级**：框架不会在超时后自动把 `cancelSession` 升级为 `SIGKILL`，是否升级、何时升级完全由调用方决定（避免框架隐式强杀）；
- **clearSession 语义不变**：`clearSession` 发送 `SIGKILL`，不可协商、不可捕获，进程立即终止；框架将置会话状态为 `CANCELLING`，会话订阅与记录按既有退出流程清理（EXIT 事件由进程退出后的既有回收流程产生）；
- **cancelSession 不改会话状态**：`cancelSession` 只投递信号——不将会话置为 `CANCELLING`、不清理订阅、不触发 EXIT 事件；工具退出后的会话回收与既有流程完全一致。

### 5.2 工具侧配套要求

- 工具声明了 `cancelable: true`，即应保证 `SIGINT`/`SIGTERM` 处理路径可用（本文第 4 节）；
- 若工具因设计原因需要忽略取消信号（例如关键落盘阶段），调用方最终会以 `clearSession` 兜底，工具不应假设信号一定会被尊重；
- 未声明 `cancelable` 的工具不会被投递任何取消信号（调用方对这类会话应直接使用 `clearSession`）。

## 6. 注意事项

1. **尽力而为语义**：`cancelSession` resolve 仅表示**信号已投递**（一次 `kill` 调用成功），不表示工具已确认、已开始清理或已退出；框架不等待工具响应、不轮询进程状态。
2. **取消判定按会话创建时的子命令**：会话创建时即固定了子命令（`argv[1]`）并落库，同时快照 `cancelable` 生效值（子命令级声明优先、未声明回退工具级），`cancelSession` 读取该快照判定；**会话运行期间工具配置声明发生变更，不影响既有会话的取消资格判定**（判定始终以创建时刻的配置为准）。
3. **两字段相互独立**：`cancelable` 只声明"可被协作取消"的能力，与工具执行的时长声明（如 `ExecOptions.timeout` 等长短期属性）无任何耦合——短时工具可以声明取消能力，长时工具也可以不声明。
4. **重复调用**：接口不做幂等承诺，重复调用会再次发送信号；会话已结束后调用返回 `35600032`。
5. **投递目标**：`cancelSession` 按进程组投递，失败时回退单进程投递，两者均失败返回 `35600038`；`clearSession` 为 `SIGKILL` 进程组投递、**无单进程回退**（与 `cancelSession` 不同），投递失败返回 `35700012`。
6. **推荐退出码**：见 4.5，`128 + signo`（SIGINT→130，SIGTERM→143）。

### 6.1 错误码速查

| 错误码 | 含义 | 触发条件 |
| ---- | ---- | ---- |
| `201` | 权限校验失败 | 缺少 `ohos.permission.EXEC_CLI_TOOL` |
| `202` | 非系统应用 | 调用方不是系统应用 |
| `401` | 参数无效 | `signal` 非 `CancellationSignal` 枚举值（仅 2/15 合法）等参数问题 |
| `35600032` | 会话不存在或非运行态 | 会话已结束/不存在，或会话不处于 RUNNING/SPAWNING |
| `35600037` | 工具不支持取消 | 会话对应工具/子命令未声明 `cancelable`（含 execCmd/SKILL 会话） |
| `35600038` | 信号发送失败 | 进程组与单进程投递均失败 |
| `35600039` | 会话非归属方 | 调用方不是会话归属方（仅 `cancelSession` 抛出；`clearSession` 等其余会话接口仍返回 `201`） |

## 7. 样例工具参考

本仓提供两个声明示范（作为端到端验证载体与实现参考）：

| 样例 | 声明方式 | 信号处理 | 路径 |
| ---- | ---- | ---- | ---- |
| `ohos-timer` | **子命令级差异声明**：工具级不声明；`run` 声明 `true`，`version` 不声明（回退工具级 → `false`） | `sigaction` 捕获 `SIGINT`/`SIGTERM`，两阶段清理，`_exit(128 + signo)` | `tools/ohos-timer/` |
| `ohos-example` | **工具级声明**：顶层 `cancelable: true`，对所有子命令生效 | 无（未捕获信号时默认终止行为，可作为"声明了但未处理信号"的对照观察） | `tools/ohos-example/` |

以 `ohos-timer` 为例的端到端验证路径：

1. `execTool` 拉起 `ohos-timer` 的 `run` 子命令（`--duration 30 --showProgress`）；
2. `cancelSession(sessionId)`（省略 `signal`）→ 工具进程收到 `SIGINT`，输出取消前的进度与部分结果（`actual_duration < duration`），以退出码 `130` 退出；
3. 再次拉起，`cancelSession(sessionId, CancellationSignal.SIGTERM)` → 收到 `SIGTERM`，以退出码 `143` 退出；
4. 对 `version` 子命令调用 `cancelSession` → 返回 `35600037`，不发送信号。
