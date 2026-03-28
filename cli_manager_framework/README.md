# CLI Manager Framework

CLI Manager Framework 是 ability_runtime 的一个独立框架模块，提供命令行工具管理和执行功能。

## 目录结构

```
cli_manager_framework/
├── frameworks/               # 框架层实现
│   ├── native/               # C++ 原生框架（预留）
│   │   └── cli_manager/
│   ├── js/                   # JavaScript 绑定
│   │   ├── napi/            # N-API 绑定
│   │   │   └── cli_manager/
│   │   └── js/              # JavaScript SDK（预留）
│   │       └── cli_manager/
│   └── ets/                 # ETS/ArkTS 绑定（预留）
│       └── cli_manager/
├── interfaces/              # 接口定义
│   ├── inner_api/           # 内部 API
│   │   └── cli_manager/
│   │       ├── include/     # 头文件
│   │       └── src/         # 源文件
│   └── kits/                # 公开 SDK
│       ├── native/          # Native SDK（预留）
│       │   └── cli_manager/
│       │       └── include/
│       └── js/             # JavaScript/TypeScript SDK
│           └── cli/
└── services/               # 系统服务
    └── climgr/             # CLI Manager Service
        ├── include/        # 头文件
        └── src/            # 源文件
```

## 模块说明

### services/climgr
CLI Manager 系统服务，负责：
- 管理可用的 CLI 工具列表
- 执行 shell 命令
- 处理命令执行结果和输出

### interfaces/inner_api/cli_manager
内部 IPC 接口，包括：
- `ICliManager`: 服务接口定义
- `CliManagerProxy`: 客户端代理
- `CliManagerStub`: 服务端存根
- `CliManagerClient`: 客户端封装

### frameworks/js/napi/cli_manager
JavaScript N-API 绑定，提供：
- `CliManager.queryTools()`: 查询可用工具
- `CliManager.execTool()`: 执行工具命令

### interfaces/kits/js/cli
TypeScript 接口定义，提供：
- `ToolInfo`: 工具信息接口
- `ExecOptions`: 执行选项接口
- `ExecResult`: 执行结果接口
- `CliManager`: 主要接口类

## 数据结构

### 命令行参数顺序保持

为了保持命令行参数的插入顺序，args 参数使用 `std::vector<std::pair<std::string, std::string>>` 而非 `std::map`。

例如：
```javascript
// 输入
CliManager.execTool("aa", {"start": "", "-a": "ability"}, options)

// 执行的命令
// aa start -a ability
```

## 编译

构建整个 CLI Manager Framework：

```bash
./build.sh --product-name <product> --build-target cli_manager_framework
```

构建特定模块：

```bash
# 构建服务
./build.sh --product-name <product> --build-target climgr

# 构建 NAPI 模块
./build.sh --product-name <product> --build-target climanager_napi
```

## 依赖关系

```
cli_manager_framework/
├── services/climgr
│   └── depends on → interfaces/inner_api:cli_manager_client
└── frameworks/js/napi/cli_manager
    └── depends on → interfaces/inner_api:cli_manager_client
```

## 设计原则

1. **独立性**: 作为独立框架，可以单独编译和使用
2. **模块化**: 清晰的模块边界，服务、接口、框架层分离
3. **可扩展**: 预留了 native、ets 等扩展接口
4. **顺序保证**: 命令行参数严格保持插入顺序
