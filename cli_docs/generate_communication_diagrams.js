const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

// 确保输出目录存在
const outputDir = path.join(__dirname, 'diagrams');
if (!fs.existsSync(outputDir)) {
    fs.mkdirSync(outputDir, { recursive: true });
}

// Mermaid 图表定义
const diagrams = {
    'communication_architecture': `graph TB
    subgraph AIClaw["AIClaw Application"]
        JS["JS Layer Event Handler"]
        NAPI["NAPI Layer C++ Bridge"]
        Proxy["Client Proxy IPC Client"]
    end

    subgraph Service["CLI Manager Service SA 9001"]
        RPC["RPC Server"]
        ED["Event Dispatcher"]
        PM["Process Manager"]
        RM["Registry Manager"]
        IO["I/O Monitor epoll"]
    end

    subgraph Tool["CLI Tool Process"]
        EXE["CLI Executable"]
        OUT_P["stdout pipe"]
        ERR_P["stderr pipe"]
    end

    JS --> NAPI
    NAPI --> Proxy
    Proxy <-->|IPC Binder| RPC
    RPC --> ED
    RPC --> PM
    PM --> RM
    ED --> IO

    PM -->|spawn| EXE
    EXE --> OUT_P
    EXE --> ERR_P

    OUT_P -->|pipe read| IO
    ERR_P -->|pipe read| IO

    IO -->|OnReadable| ED
    ED -->|Callback| RPC
    RPC --> Proxy`,

    'pipe_creation_flow': `sequenceDiagram
    participant PM as Process Manager
    participant EP as Executor
    participant Tool as CLI Tool Process
    participant IO as I/O Monitor
    participant ED as Event Dispatcher

    Note over PM,ED: 1. Pipe Creation
    PM->>EP: CreateExecutionContext()
    EP->>EP: pipe(stdoutPipe)
    EP->>EP: pipe(stderrPipe)
    EP->>EP: pipe(stdinPipe) [optional]
    Note over EP: Pipes created<br/>stdoutPipe[0/1]<br/>stderrPipe[0/1]<br/>stdinPipe[0/1]

    Note over PM,ED: 2. Process Spawn
    EP->>Tool: fork() + execve()
    Note over Tool: Child process starts
    EP->>Tool: dup2(stdoutPipe[1], STDOUT)
    EP->>Tool: dup2(stderrPipe[1], STDERR)
    EP->>Tool: dup2(stdinPipe[0], STDIN)
    EP->>Tool: close(pipe fds)
    Note over Tool: FD redirection complete

    Note over PM,ED: 3. I/O Monitoring
    EP->>IO: Register stdoutPipe[0]
    EP->>IO: Register stderrPipe[0]
    IO->>IO: epoll_ctl(EPOLLIN, stdout)
    IO->>IO: epoll_ctl(EPOLLIN, stderr)
    Note over IO: Start monitoring pipes`,

    'data_flow_async': `graph TB
    subgraph AIClaw["AIClaw Application"]
        JS["JS Code<br/>(event handler)"]
        NAPI["NAPI Layer<br/>(callback wrapper)"]
        Proxy["Client Proxy<br/>(callback stub)"]
    end

    subgraph Service["CLI Manager Service"]
        RPC["RPC Server"]
        PM["Process Manager"]
        IO["I/O Monitor<br/>(epoll thread)"]
        ED["Event Dispatcher"]
    end

    subgraph Tool["CLI Tool Process"]
        EXE["Executable<br/>(long running)"]
        OUT["stdout"]
    end

    JS -->|1. execToolAsync| NAPI
    NAPI -->|2. ExecToolAsync| Proxy
    Proxy -->|3. IPC<br/>pass callback| RPC
    RPC -->|4. Register Callback| ED
    RPC -->|5. CreateContext| PM
    PM -->|6. spawn| EXE
    PM -->|7. return taskId| RPC
    RPC -->|8. return taskId| Proxy
    Proxy -->|9. resolve| JS

    EXE -->|10. printf| OUT
    OUT -->|11. write pipe| IO
    IO -->|12. epoll event| IO
    IO -->|13. OnReadable| ED
    ED -->|14. IPC Callback| Proxy
    Proxy -->|15. invoke JS callback| NAPI
    NAPI -->|16. JS event handler| JS`,

    'io_monitor_detail': `graph TB
    subgraph IOMonitor["I/O Monitor epoll thread"]
        EP["epoll instance"]
        EQ["epoll_wait"]
        RD["Read Handler"]
        BUF["Buffer 64KB"]
    end

    subgraph Pipes["Pipe File Descriptors"]
        P1["Tool1 stdout 0"]
        P2["Tool1 stderr 0"]
        P3["Tool2 stdout 0"]
        P4["Tool2 stderr 0"]
    end

    subgraph Callback["Event Callback"]
        ED["Event Dispatcher"]
        CB1["Callback Tool1"]
        CB2["Callback Tool2"]
    end

    P1 -->|EPOLLIN| EP
    P2 -->|EPOLLIN| EP
    P3 -->|EPOLLIN| EP
    P4 -->|EPOLLIN| EP

    EP --> EQ
    EQ -->|event ready| RD
    RD -->|read| BUF
    BUF -->|data| ED

    ED --> CB1
    ED --> CB2`,

    'sync_data_flow': `graph TB
    subgraph AIClaw["AIClaw Application"]
        JS["JS Code"]
        NAPI["NAPI Layer"]
        Proxy["Client Proxy"]
    end

    subgraph Service["CLI Manager Service"]
        RPC["RPC Server"]
        PM["Process Manager"]
        IO["I/O Monitor"]
    end

    subgraph Tool["CLI Tool Process"]
        EXE["Executable"]
        OUT["stdout"]
    end

    JS -->|1. execTool| NAPI
    NAPI -->|2. ExecTool| Proxy
    Proxy -->|3. IPC Sync| RPC
    RPC -->|4. CreateContext| PM
    PM -->|5. pipes| PM
    PM -->|6. fork| EXE
    PM -->|7. waitpid| PM
    OUT -->|8. write| IO
    IO -.->|9. read| IO
    IO -->|10. buffer| PM
    PM -->|11. Return| RPC
    RPC -->|12. Result| Proxy
    Proxy -->|13. Return| NAPI
    NAPI -->|14. Promise| JS`,

    // 新增：execTool 统一接口时序图
    'exec_tool_unified': `sequenceDiagram
    participant User as User Code
    participant JS as JS API (CLIManager)
    participant NAPI as NAPI Layer
    participant Proxy as Client Proxy
    participant IPC as IPC (Binder)
    participant SVC as CLI Manager Service
    participant Perm as Permission Checker
    participant Reg as Registry Manager
    participant PM as Process Manager
    participant Param as Parameter Mapper
    participant Tool as CLI Tool Process
    participant IO as I/O Monitor
    participant ED as Event Dispatcher

    Note over User,ED: === execTool 统一接口完整流程 ===

    User->>JS: execTool(name, args, options)
    JS->>NAPI: ExecTool(name, args, options)
    NAPI->>Proxy: ExecTool(name, args, options)
    Proxy->>IPC: IPC 同步调用
    IPC->>SVC: ExecTool(name, args, options)

    SVC->>Perm: CheckPermission(name, caller)
    Perm-->>SVC: 权限校验通过

    SVC->>Reg: GetToolInfo(name)
    Reg-->>SVC: ToolInfo

    SVC->>PM: CreateExecutionContext()
    PM->>PM: CreatePipes()

    PM->>Param: MapToCommandLine(toolInfo, args)
    Param-->>PM: argv (命令行参数)

    PM->>Tool: fork() + execve(argv)
    PM->>IO: RegisterPipes(context)
    PM-->>SVC: context (taskId, pid)

    alt background = true
        Note over SVC: 后台模式
        SVC->>ED: RegisterSession(context)
        SVC-->>IPC: SessionInfo(status='running')
        IPC-->>Proxy: SessionInfo
        Proxy-->>NAPI: SessionInfo
        NAPI-->>JS: Promise<SessionInfo>
        JS-->>User: { sessionId, status: 'running' }

        Note over Tool,ED: 异步事件通知
        Tool->>IO: stdout 数据
        IO->>ED: OnReadable(data)
        ED->>IPC: 事件回调
        IPC->>Proxy: 事件通知
        Proxy->>NAPI: invoke callback
        NAPI->>JS: 事件回调
        JS->>User: onEvent({type, data})
    else background = false
        Note over SVC: 前台模式
        SVC->>SVC: 启动定时器 (yieldMs)
        alt 在 yieldMs 内完成
            Tool->>Tool: 执行完成
            PM->>PM: waitpid(pid)
            PM-->>SVC: ExecResult
            SVC-->>IPC: SessionInfo(status='completed', result)
            IPC-->>Proxy: SessionInfo
            Proxy-->>NAPI: SessionInfo
            NAPI-->>JS: Promise<SessionInfo>
            JS-->>User: { sessionId, status: 'completed', result }
        else 超时未完成
            SVC-->>IPC: SessionInfo(status='running')
            IPC-->>Proxy: SessionInfo
            Proxy-->>NAPI: SessionInfo
            NAPI-->>JS: Promise<SessionInfo>
            JS-->>User: { sessionId, status: 'running' }
        end
    end`,

    // 新增：参数映射流程
    'parameter_mapping_flow': `sequenceDiagram
    participant User as User Code
    participant JS as JS API
    participant NAPI as NAPI Layer
    participant SVC as CLI Manager Service
    participant Reg as Registry Manager
    participant PM as Process Manager
    participant Param as Parameter Mapper
    participant Tool as CLI Tool Process

    User->>JS: execTool('network-scanner', args)
    Note right of User: args = {<br/>  target: "192.168.1.1",<br/>  ports: [80, 443],<br/>  timeout: 5000,<br/>  verbose: true<br/>}

    JS->>NAPI: ExecTool(name, args)
    NAPI->>SVC: IPC 调用

    SVC->>Reg: GetToolInfo('network-scanner')
    Reg-->>SVC: ToolInfo
    Note right of Reg: argumentMapping.type = "flag"<br/>templates = {<br/>  target: "--target={value}",<br/>  ports: "--port={value}",<br/>  timeout: "--timeout={value}",<br/>  verbose: {if_true: "-v"}<br/>}

    SVC->>PM: CreateExecutionContext(toolInfo, args)
    PM->>Param: MapToCommandLine(toolInfo, args)

    Param->>Param: 遍历 args 应用模板
    Note right of Param: target → "--target=192.168.1.1"<br/>ports[0] → "--port=80"<br/>ports[1] → "--port=443"<br/>timeout → "--timeout=5000"<br/>verbose → "-v"

    Param-->>PM: argv = [<br/>  "/usr/bin/net-scan",<br/>  "--target=192.168.1.1",<br/>  "--port=80",<br/>  "--port=443",<br/>  "--timeout=5000",<br/>  "-v"<br/>]

    PM->>Tool: fork() + execve(argv)

    Note right of Tool: net-scan --target=192.168.1.1 --port=80 --port=443 --timeout=5000 -v`,

    // 新增：子命令执行流程
    'subcommand_execution_flow': `sequenceDiagram
    participant User as User Code
    participant JS as JS API
    participant SVC as CLI Manager Service
    participant Reg as Registry Manager
    participant PM as Process Manager
    participant Param as Parameter Mapper
    participant Tool as CLI Tool (pkg-mgr)

    User->>JS: execTool('pkg-manager', args)
    Note right of User: args = {<br/>  subcommand: 'install',<br/>  package: '/data/app.hap',<br/>  force: true,<br/>  silent: true<br/>}

    JS->>SVC: ExecTool('pkg-manager', args)

    SVC->>Reg: GetToolInfo('pkg-manager')
    Reg-->>SVC: ToolInfo
    Note right of Reg: hasSubcommands = true<br/>subcommands = {<br/>  install: {...},<br/>  remove: {...},<br/>  list: {...}<br/>}

    SVC->>PM: CreateExecutionContext(toolInfo, args)
    PM->>Param: MapToCommandLine(toolInfo, args)

    Param->>Param: 检测 hasSubcommands = true
    Param->>Param: 提取 subcommand = "install"
    Param->>Param: 查找 subcommands["install"]

    Param->>Param: argv.push("/usr/bin/pkg-mgr")
    Param->>Param: argv.push("install")
    Note right of Param: 添加子命令名称

    Note over Param: 使用子命令的 argumentMapping
    Note right of Param: package → "/data/app.hap"<br/>force → "--force"<br/>silent → "--silent"

    Param-->>PM: argv = [<br/>  "/usr/bin/pkg-mgr",<br/>  "install",<br/>  "/data/app.hap",<br/>  "--force",<br/>  "--silent"<br/>]

    PM->>Tool: fork() + execve(argv)

    Note right of Tool: pkg-mgr install /data/app.hap --force --silent`
};

// 生成图片
async function generateDiagrams() {
    console.log('开始生成通信架构图...\n');

    for (const [name, mermaidCode] of Object.entries(diagrams)) {
        const mmdFile = path.join(outputDir, `${name}.mmd`);
        const pngFile = path.join(outputDir, `${name}.png`);

        // 写入 .mmd 文件
        fs.writeFileSync(mmdFile, mermaidCode);

        // 使用 mmdc 生成 PNG
        try {
            console.log(`生成 ${name}.png...`);
            execSync(`npx mmdc -i "${mmdFile}" -o "${pngFile}" -b transparent`, {
                stdio: 'inherit',
                cwd: __dirname
            });
            console.log(`✓ ${name}.png 生成成功\n`);
        } catch (error) {
            console.error(`✗ ${name}.png 生成失败:`, error.message);
        }
    }

    console.log('\n所有通信架构图生成完成！');
    console.log(`图片保存位置: ${outputDir}`);
}

generateDiagrams().catch(console.error);
