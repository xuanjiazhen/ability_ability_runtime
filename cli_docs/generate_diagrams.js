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
    'overall_architecture': `graph TB
    subgraph "应用层"
        AIClaw[AIClaw 系统应用]
    end

    subgraph "JS API 层"
        JSAPI[CLI Manager JS API]
        NAPI[NAPI Binding]
    end

    subgraph "框架层"
        ClientProxy[CLI Manager Client Proxy]
        ClientCallback[Client Callback Handler]
    end

    subgraph "服务层 (SystemAbility)"
        CLIManagerServ[CLI Manager Service]
        RegistryMgr[Tool Registry Manager]
        PermChecker[Permission Checker]
        ProcessMgr[Process Manager]
        EventDispatcher[Event Dispatcher]
    end

    subgraph "工具描述层"
        DescParser[Descriptor Parser]
        DescCache[Descriptor Cache]
        ToolDB[(Tool Metadata DB)]
    end

    subgraph "进程层"
        Tool1[CLI Tool Process 1]
        Tool2[CLI Tool Process 2]
        ToolN[CLI Tool Process N]
    end

    AIClaw --> JSAPI
    JSAPI --> NAPI
    NAPI --> ClientProxy
    ClientProxy <-->|IPC| CLIManagerServ
    ClientProxy <-->|IPC| ClientCallback

    CLIManagerServ --> RegistryMgr
    CLIManagerServ --> PermChecker
    CLIManagerServ --> ProcessMgr
    CLIManagerServ --> EventDispatcher

    RegistryMgr --> DescParser
    DescParser --> DescCache
    DescCache --> ToolDB

    ProcessMgr --> Tool1
    ProcessMgr --> Tool2
    ProcessMgr --> ToolN

    Tool1 -.->|stdout/stderr| EventDispatcher
    Tool2 -.->|stdout/stderr| EventDispatcher
    ToolN -.->|stdout/stderr| EventDispatcher`,

    'tool_registration_flow': `sequenceDiagram
    participant CLI as CLI Tool Binary
    participant Svc as CLI Manager Service
    participant Reg as Registry Manager
    participant DB as Tool Metadata DB

    CLI->>Svc: 注册请求 (tool_name, desc_path)
    Svc->>Reg: 解析描述文件
    Reg->>DB: 验证工具唯一性
    DB-->>Reg: 返回验证结果
    Reg->>DB: 存储工具元数据
    DB-->>Svc: 注册成功
    Svc-->>CLI: 返回注册结果`,

    'sync_execution_flow': `sequenceDiagram
    participant App as AIClaw App
    participant API as JS API
    participant Proxy as Client Proxy
    participant Svc as CLI Manager Service
    participant Perm as Permission Checker
    participant Proc as Process Manager
    participant Tool as CLI Tool Process

    App->>API: execTool(name, args)
    API->>Proxy: execTool(name, args)
    Proxy->>Svc: execTool(name, args)
    Svc->>Perm: checkPermission(caller, tool)
    Perm-->>Svc: 权限校验结果
    Svc->>Proc: spawnProcess(name, args)
    Proc->>Tool: 启动进程
    Tool-->>Proc: exit code + stdout
    Proc-->>Svc: execution result
    Svc-->>Proxy: return result
    Proxy-->>API: return result
    API-->>App: resolve(result)`,

    'async_execution_flow': `sequenceDiagram
    participant App as AIClaw App
    participant API as JS API
    participant Proxy as Client Proxy
    participant Svc as CLI Manager Service
    participant Proc as Process Manager
    participant Tool as CLI Tool Process
    participant Event as Event Dispatcher

    App->>API: execToolAsync(name, args, callback)
    API->>Proxy: execToolAsync(name, args, callback)
    Proxy->>Svc: execToolAsync(name, args)
    Svc->>Proc: spawnProcessAsync(name, args)
    Proc->>Tool: 启动进程
    Svc-->>Proxy: return taskId
    Proxy-->>API: return taskId
    API-->>App: resolve(taskId)

    loop 工具输出事件
        Tool->>Event: stdout/stderr
        Event->>Svc: onOutput(taskId, data)
        Svc->>Proxy: onOutput(taskId, data)
        Proxy->>API: onOutput(taskId, data)
        API->>App: callback(data)
    end

    Tool->>Proc: exit
    Proc->>Event: onExit(taskId, exitCode)
    Event->>Svc: onExit(taskId, exitCode)
    Svc->>Proxy: onExit(taskId, exitCode)
    Proxy->>API: onExit(taskId, exitCode)
    API->>App: callback({type: 'exit', code})`,

    'database_er': `erDiagram
    TOOLS ||--o{ TOOL_PERMISSIONS : has
    TOOLS ||--o{ EXECUTION_HISTORY : generates
    TOOLS {
        string tool_name PK
        string version
        string description
        string executable_path
        enum type
        uint32_t timeout_ms
        uint64_t registered_time
        bool enabled
    }

    TOOL_PERMISSIONS {
        string tool_name FK
        string permission
        uint32_t id PK
    }

    EXECUTION_HISTORY {
        uint64_t task_id PK
        string tool_name FK
        string caller_bundle_name
        string caller_ability_name
        pid_t process_id
        enum mode
        int exit_code
        uint64_t start_time
        uint64_t end_time
        bool timed_out
    }`
};

// 生成图片
async function generateDiagrams() {
    console.log('开始生成架构图...\n');

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

    console.log('\n所有架构图生成完成！');
    console.log(`图片保存位置: ${outputDir}`);
}

generateDiagrams().catch(console.error);
