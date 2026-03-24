# CLI 工具管理框架 - 设计文档总结

## 设计文档概览

本项目包含两份核心设计文档：

### 1. DESIGN.md - 总体设计方案
**路径：** `DESIGN.md`

**内容概要：**
- 需求分析（10 个核心功能 + 7 类非功能需求）
- 架构选型（基于 IPC 的分布式架构）
- 系统架构设计（5 张可视化架构图）
- 数据模型设计（工具描述文件、数据结构、ER 图）
- 接口定义（JS API、C++ API、IPC 接口）
- 风险评估与应对
- 目录结构与实施计划

---

### 2. COMMUNICATION_DESIGN.md - 通信详细设计 ⭐
**路径：** `COMMUNICATION_DESIGN.md`

**内容概要：**
- **通信架构概述** - 管道 + IPC 回调的混合模型（含可视化架构图）
- **管道建立机制** - 进程创建、管道重定向、文件描述符管理（含流程图）
- **I/O 监控与数据读取** - epoll 事件监控、非阻塞读取、缓冲区管理
- **事件分发与回调机制** - 事件路由、IPC 回调分发
- **进程启动与管道重定向** - fork/execve、dup2 重定向、参数构建
- **JS 层集成** - NAPI 绑定、事件转换、异步回调
- **完整通信流程** - 同步/异步调用的可视化数据流图

**核心机制：**
```
AIClaw App → JS API → NAPI → Client Proxy
                                        ↓ IPC
CLI Manager Service ← RPC Server ← Event Dispatcher
                                        ↓
Process Manager → fork() → Tool Process
                      ↓
                   pipe(stdout/stderr)
                      ↓
I/O Monitor (epoll) → Event Dispatcher → IPC → JS Callback
```

---

## 架构图清单

### 总体架构图（diagrams/）

| 图表 | 文件名 | 说明 |
|------|--------|------|
| 整体架构 | `overall_architecture.png` | 完整的系统分层架构 |
| 工具注册流程 | `tool_registration_flow.png` | 工具注册的时序流程 |
| 同步调用流程 | `sync_execution_flow.png` | 同步调用的完整流程 |
| 异步调用流程 | `async_execution_flow.png` | 异步调用和事件监听 |
| 数据库 ER 图 | `database_er.png` | 数据库表结构关系 |

### 通信架构图（diagrams/）⭐

| 图表 | 文件名 | 说明 | 在文档中 |
|------|--------|------|---------|
| 通信架构总览 | `communication_architecture.png` | AIClaw 到 Tool 进程的完整通信链路 | - |
| 管道创建流程 | `pipe_creation_flow.png` | 进程启动时管道建立和重定向的详细步骤 | ✅ COMMUNICATION_DESIGN.md 第 2.1 节 |
| 异步数据流 | `data_flow_async.png` | 异步调用时数据从 Tool 流向 AIClaw 的完整路径 | ✅ COMMUNICATION_DESIGN.md 第 8.2 节 |
| 同步数据流 | `sync_data_flow.png` | 同步调用的数据流转过程 | ✅ COMMUNICATION_DESIGN.md 第 8.1 节 |
| I/O 监控细节 | `io_monitor_detail.png` | epoll 监控多个管道的工作原理 | - |

---

## 通信机制核心要点

### 1. 管道通信
- **stdout 管道**：工具的标准输出 → 服务的 I/O Monitor
- **stderr 管道**：工具的标准错误 → 服务的 I/O Monitor
- **stdin 管道**（可选）：服务 → 工具的标准输入

### 2. 进程创建
```cpp
fork() → 子进程
  → dup2(stdoutPipe[1], STDOUT_FILENO)
  → dup2(stderrPipe[1], STDERR_FILENO)
  → execve(tool_path, argv, envp)
```

### 3. I/O 监控
```cpp
epoll_create1()           // 创建 epoll 实例
  → epoll_ctl(ADD, fd)    // 注册管道读端
  → epoll_wait()          // 等待事件
  → read(fd, buffer)      // 读取数据
  → Event Dispatcher      // 分发事件
```

### 4. 事件回调
```
Tool Output → Pipe → epoll → IOMonitor
                              ↓
                        Event Dispatcher
                              ↓ IPC
                        Client Proxy
                              ↓ NAPI
                        JS Callback
```

### 5. 同步 vs 异步

| 特性 | 同步模式 | 异步模式 |
|------|---------|---------|
| 进程等待 | waitpid() 阻塞 | 不等待，立即返回 taskId |
| 输出获取 | 进程退出后一次性返回 | 实时分发到 JS 回调 |
| 适用场景 | 短时工具、快速返回 | 长时运行、持续输出 |
| 返回方式 | Promise<ExecResult> | Promise<string> taskId + callback |

---

## 关键代码组件

### Process Manager (进程管理器)
- `CreatePipes()` - 创建 stdout/stderr/stdin 管道
- `SpawnProcess()` - fork + execve 启动工具进程
- `PrepareChildProcess()` - 子进程文件描述符重定向
- `BuildArgv()` - 根据参数构建 exec 参数

### I/O Monitor (I/O 监控器)
- `Start()` - 启动 epoll 监控线程
- `RegisterTask()` - 注册任务管道进行监控
- `MonitorThread()` - epoll_wait 主循环
- `ReadFromPipe()` - 非阻塞读取管道数据

### Event Dispatcher (事件分发器)
- `RegisterCallback()` - 注册任务的 IPC 回调
- `DispatchIOEvent()` - 分发 stdout/stderr 事件
- `DispatchExitEvent()` - 分发进程退出事件
- `DispatchErrorEvent()` - 分发错误事件

### Client Callback Stub (客户端回调)
- `OnOutput()` - 接收工具输出
- `OnExit()` - 接收退出事件
- `OnRemoteRequest()` - IPC 服务端实现

---

## 性能与安全

### 性能优化
- **非阻塞 I/O**：所有管道读端设置为 O_NONBLOCK
- **epoll 多路复用**：单线程监控多个工具进程
- **批量事件处理**：累积小数据包，减少 IPC 调用
- **零拷贝优化**：splice() 系统调用（可选）

### 安全防护
- **参数注入防护**：过滤 `; | & $ ` 等危险字符
- **管道缓冲区限制**：fcntl(F_SETPIPE_SZ) 防止内存耗尽
- **进程沙箱隔离**：setsid() 创建独立会话
- **超时控制**：定时器 + SIGTERM/SIGKILL 强制终止

---

## 快速导航

| 想了解... | 查看文档/章节 |
|----------|----------------|
| 整体架构和模块划分 | `DESIGN.md` 第 3 节 |
| 数据模型和接口定义 | `DESIGN.md` 第 4-5 节 |
| **AIClaw 如何调用工具** | `COMMUNICATION_DESIGN.md` 第 8 节 |
| **如何获取工具输出** | `COMMUNICATION_DESIGN.md` 第 4 节 |
| **如何异步监听消息** | `COMMUNICATION_DESIGN.md` 第 5-6 节 |
| 管道建立和重定向 | `COMMUNICATION_DESIGN.md` 第 2-3 节 |
| I/O 监控实现细节 | `COMMUNICATION_DESIGN.md` 第 4 节 |
| 事件分发机制 | `COMMUNICATION_DESIGN.md` 第 5 节 |
| JS 层如何集成 | `COMMUNICATION_DESIGN.md` 第 7 节 |
| 安全防护措施 | `COMMUNICATION_DESIGN.md` 第 11 节 |

---

## 使用示例

### JS 层调用示例

```typescript
import CLI from '@ohos.cli.manager';

// 1. 同步调用（短时工具）
const result = await CLI.CLIManager.execTool('network-scanner', {
  target: '192.168.1.1',
  ports: [80, 443]
});
console.log('Result:', result.stdout);

// 2. 异步调用（长时工具）
const taskId = await CLI.CLIManager.execToolAsync(
  'log-monitor',
  { filter: 'ERROR' },
  (event) => {
    if (event.type === 'stdout') {
      console.log('Log:', event.data);
    } else if (event.type === 'exit') {
      console.log('Exit code:', event.exitCode);
    }
  }
);
```

---

## 文档版本

- **DESIGN.md**: v1.0
- **COMMUNICATION_DESIGN.md**: v1.0
- **生成日期**: 2025-01-XX

---

**如需审阅具体实现细节，请参考 `COMMUNICATION_DESIGN.md`。**
