# eventSchemas 事件机制说明

## 概述

**eventSchemas** 是 CLI 管理框架中用于定义工具**自定义事件数据格式**的机制。它允许工具在执行过程中向调用方反馈中间状态（如进度、下载状态、解析结果等），而不仅仅是返回最终结果。

### 核心价值

- **渐进式反馈**：长时间运行的工具可以实时报告进度
- **类型安全**：调用方可以明确知道事件的字段结构
- **自动验证**：框架会验证工具输出是否符合 Schema 定义
- **统一机制**：所有事件都通过标准 JSON 行输出

## 事件类型与 Schema 对应关系

| 事件类型 | Schema 来源 | 触发方式 | 说明 |
|---------|------------|---------|------|
| `stdout` | 不需要 | 工具输出非 JSON 行 | 普通文本输出 |
| `stderr` | 不需要 | 工具向 stderr 输出 | 错误输出 |
| `exit` | 不需要 | 进程退出 | 框架自动生成 |
| `error` | 不需要 | 执行失败 | 框架自动生成 |
| **`progress`** | **`eventSchemas.progress`** | 输出 `{"event": "progress", ...}` | **自定义进度事件** |
| **`result`** | **`output`** | 输出 `{"event": "result", ...}` | 最终结果 |
| **`custom_event`** | **`eventSchemas.custom_event`** | 输出 `{"event": "custom_event", ...}` | 任意自定义事件 |

## eventSchemas 字段定义

### 在工具描述中的位置

```json
{
  "name": "my-tool",
  "version": "1.0.0",
  "description": "Tool description",
  "executable": "/usr/bin/my-tool",
  "input": { ... },           // 输入参数 Schema
  "output": { ... },          // 最终结果 Schema（用于 result 事件）
  "events": [                 // 声明支持的事件类型
    "stdout",
    "stderr",
    "progress",
    "download",
    "result"
  ],
  "eventSchemas": {           // 自定义事件 Schema 定义
    "progress": { ... },      // progress 事件的 Schema
    "download": { ... }       // 自定义 download 事件的 Schema
  }
}
```

### eventSchemas 结构

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| key | string | 是 | 事件类型名称（如 `progress`、`download`） |
| value | JSONSchema | 是 | 事件数据的 JSON Schema 定义 |

### 子命令中的 eventSchemas

对于有子命令的工具，每个子命令可以定义自己的事件 Schema：

```json
{
  "hasSubcommands": true,
  "subcommands": {
    "install": {
      "description": "Install package",
      "input": { ... },
      "output": { ... },
      "events": ["progress"],
      "eventSchemas": {
        "progress": {
          "type": "object",
          "properties": {
            "stage": {"type": "string"},
            "percentage": {"type": "integer"}
          }
        }
      }
    }
  }
}
```

## 事件输出机制

### 核心原则

工具进程通过 **stdout 输出标准格式的 JSON 行** 来触发事件，框架解析并分发事件。

### 事件输出格式

工具输出的每一行必须是有效的 JSON 对象，包含以下字段：

| 字段 | 类型 | 必填 | 说明 |
|------|------|------|------|
| `event` | string | 是 | 事件类型，必须是 `events` 数组中声明的类型 |
| `data` | any | 是 | 事件数据，必须符合对应事件的 Schema 定义 |
| `timestamp` | number | 否 | 事件时间戳（毫秒），框架会自动添加 |

### 事件输出示例

**1. progress 事件**

```bash
# 工具输出
{"event": "progress", "data": {"percentage": 50, "currentPort": 80, "totalPorts": 2, "status": "scanning"}}
```

**2. result 事件**

```bash
# 工具输出
{"event": "result", "data": {"results": [...], "summary": {"totalPorts": 2, "openPorts": 1}}}
```

**3. 混合输出（stdout + progress + result）**

```bash
# 工具输出
Starting scan...
{"event": "progress", "data": {"percentage": 50, "status": "scanning"}}
Scanning port 443...
{"event": "progress", "data": {"percentage": 100, "status": "completed"}}
{"event": "result", "data": {"results": [...], "summary": {...}}}
Scan complete.
```

**4. 自定义事件**

```bash
# 工具输出
{"event": "download", "data": {"url": "https://example.com/file.zip", "bytesDownloaded": 1024000, "totalBytes": 2048000}}
{"event": "parse", "data": {"file": "/path/to/file.json", "lineNumber": 42, "totalLines": 1000}}
```

### 非事件输出处理

**非 JSON 行**：当工具输出非 JSON 格式的文本时，框架会将其作为 `stdout` 事件处理：

```bash
# 工具输出普通文本
Scanning port 80...
Port 80 is open
Scanning port 443...
```

框架自动转换为：

```javascript
{type: 'stdout', data: 'Scanning port 80...'}
{type: 'stdout', data: 'Port 80 is open'}
{type: 'stdout', data: 'Scanning port 443...'}
```

## 框架处理流程

```
工具进程 stdout
    ↓
读取一行数据
    ↓
是否为有效 JSON？
    ├─ 否 → 作为 stdout 事件处理
    └─ 是 → 解析 JSON 对象
            ↓
        是否包含 event 字段？
            ├─ 否 → 作为 stdout 事件处理
            └─ 是 → 检查 event 类型
                    ↓
                是否在 events 数组中？
                    ├─ 否 → 生成 error 事件（未知事件类型）
                    └─ 是 → 获取对应 Schema
                            ↓
                        验证 data 字段
                            ├─ 验证失败 → 生成 error 事件
                            └─ 验证成功 → 分发事件到调用方
```

## 完整示例

### 示例 1：网络扫描工具（带进度报告）

**工具描述文件**

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tool/v1",
  "name": "network-scanner",
  "version": "1.0.0",
  "description": "Network scanning utility tool",
  "executable": "/usr/bin/net-scan",
  "permissions": [
    "ohos.permission.INTERNET",
    "ohos.permission.GET_NETWORK_INFO"
  ],
  "input": {
    "type": "object",
    "properties": {
      "target": {
        "type": "string",
        "format": "ipv4",
        "description": "Target IP address"
      },
      "ports": {
        "type": "array",
        "items": {"type": "integer"},
        "description": "Ports to scan"
      }
    },
    "required": ["target"]
  },
  "output": {
    "type": "object",
    "description": "扫描结果",
    "properties": {
      "results": {
        "type": "array",
        "items": {
          "type": "object",
          "properties": {
            "port": {"type": "integer"},
            "status": {"type": "string", "enum": ["open", "closed", "filtered"]},
            "service": {"type": "string"},
            "responseTime": {"type": "integer"}
          }
        }
      },
      "summary": {
        "type": "object",
        "properties": {
          "totalPorts": {"type": "integer"},
          "openPorts": {"type": "integer"},
          "scanTime": {"type": "integer"}
        }
      }
    }
  },
  "timeout": 30000,
  "events": ["progress", "result"],
  "eventSchemas": {
    "progress": {
      "type": "object",
      "description": "扫描进度事件",
      "properties": {
        "percentage": {
          "type": "integer",
          "minimum": 0,
          "maximum": 100,
          "description": "完成百分比"
        },
        "currentPort": {
          "type": "integer",
          "description": "当前扫描的端口"
        },
        "totalPorts": {
          "type": "integer",
          "description": "总端口数"
        },
        "status": {
          "type": "string",
          "enum": ["scanning", "completed", "failed"],
          "description": "当前状态"
        },
        "timestamp": {
          "type": "integer",
          "description": "时间戳（毫秒）"
        }
      },
      "required": ["percentage", "status"]
    }
  }
}
```

**工具端输出（C/C++ 示例）**

```c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    const char* target = argv[1];
    int ports[] = {80, 443};
    int total_ports = 2;

    // 输出进度事件
    printf("{\"event\": \"progress\", \"data\": {\"percentage\": 50, \"currentPort\": 80, \"totalPorts\": %d, \"status\": \"scanning\"}}\n", total_ports);
    fflush(stdout);

    // 执行扫描...

    // 输出进度事件
    printf("{\"event\": \"progress\", \"data\": {\"percentage\": 100, \"currentPort\": 443, \"totalPorts\": %d, \"status\": \"completed\"}}\n", total_ports);
    fflush(stdout);

    // 输出结果事件
    printf("{\"event\": \"result\", \"data\": {\"results\": [{\"port\": 80, \"status\": \"open\", \"service\": \"http\", \"responseTime\": 50}], \"summary\": {\"totalPorts\": %d, \"openPorts\": 1, \"scanTime\": 1500}}}\n", total_ports);
    fflush(stdout);

    return 0;
}
```

**调用方接收（JavaScript）**

```javascript
const session = await CLI.CLIManager.execTool('network-scanner', {
  target: "192.168.1.1",
  ports: [80, 443]
}, {
  onEvent: (event) => {
    switch (event.type) {
      case 'progress':
        // progress 事件数据符合 eventSchemas.progress 定义的格式
        console.log(`进度: ${event.data.percentage}%, 端口: ${event.data.currentPort}/${event.data.totalPorts}`);
        break;
      case 'result':
        // result 事件数据符合 output 定义的格式
        console.log('扫描完成:', event.data.summary);
        console.log('开放端口:', event.data.results);
        break;
      case 'stdout':
        console.log('输出:', event.data);
        break;
    }
  }
});
```

### 示例 2：文件下载工具（多种自定义事件）

**工具描述文件**

```json
{
  "$schema": "https://schemas.openharmony.org/cli/tool/v1",
  "name": "file-downloader",
  "version": "1.0.0",
  "description": "File download utility with progress reporting",
  "executable": "/usr/bin/file-downloader",
  "input": {
    "type": "object",
    "properties": {
      "url": {
        "type": "string",
        "format": "uri",
        "description": "Download URL"
      },
      "outputPath": {
        "type": "string",
        "description": "Local output path"
      }
    },
    "required": ["url", "outputPath"]
  },
  "output": {
    "type": "object",
    "properties": {
      "success": {"type": "boolean"},
      "filePath": {"type": "string"},
      "fileSize": {"type": "integer"},
      "downloadTime": {"type": "integer"}
    }
  },
  "events": ["download", "verify", "result"],
  "eventSchemas": {
    "download": {
      "type": "object",
      "description": "下载进度事件",
      "properties": {
        "url": {"type": "string"},
        "bytesDownloaded": {"type": "integer"},
        "totalBytes": {"type": "integer"},
        "speed": {"type": "integer", "description": "下载速度（字节/秒）"}
      },
      "required": ["url", "bytesDownloaded", "totalBytes"]
    },
    "verify": {
      "type": "object",
      "description": "文件校验事件",
      "properties": {
        "algorithm": {"type": "string", "enum": ["md5", "sha1", "sha256"]},
        "status": {"type": "string", "enum": ["verifying", "passed", "failed"]},
        "progress": {"type": "integer", "minimum": 0, "maximum": 100}
      }
    }
  }
}
```

**工具端输出（Go 示例）**

```go
package main

import (
    "encoding/json"
    "fmt"
    "os"
)

type DownloadEvent struct {
    URL            string `json:"url"`
    BytesDownloaded int64  `json:"bytesDownloaded"`
    TotalBytes      int64  `json:"totalBytes"`
    Speed           int64  `json:"speed"`
}

type VerifyEvent struct {
    Algorithm string `json:"algorithm"`
    Status    string `json:"status"`
    Progress  int    `json:"progress"`
}

type Event struct {
    Event string      `json:"event"`
    Data  interface{} `json:"data"`
}

func main() {
    // 发送下载进度事件
    event := Event{
        Event: "download",
        Data: DownloadEvent{
            URL:            "https://example.com/file.zip",
            BytesDownloaded: 1024000,
            TotalBytes:      2048000,
            Speed:           1024000,
        },
    }
    json.NewEncoder(os.Stdout).Encode(event)

    // 发送校验事件
    event = Event{
        Event: "verify",
        Data: VerifyEvent{
            Algorithm: "sha256",
            Status:    "verifying",
            Progress:  50,
        },
    }
    json.NewEncoder(os.Stdout).Encode(event)
}
```

**调用方接收**

```javascript
await CLI.CLIManager.execTool('file-downloader', {
  url: 'https://example.com/file.zip',
  outputPath: '/tmp/file.zip'
}, {
  onEvent: (event) => {
    switch (event.type) {
      case 'download':
        const percentage = (event.data.bytesDownloaded / event.data.totalBytes * 100).toFixed(2);
        console.log(`下载进度: ${percentage}% (${event.data.speed} bytes/s)`);
        break;
      case 'verify':
        console.log(`文件校验: ${event.data.algorithm} - ${event.data.status} (${event.data.progress}%)`);
        break;
      case 'result':
        console.log('下载完成:', event.data);
        break;
    }
  }
});
```

### 示例 3：带子命令的工具

```json
{
  "name": "pkg-manager",
  "version": "1.0.0",
  "description": "Package management tool with subcommands",
  "executable": "/usr/bin/pkg-mgr",
  "permissions": [
    "ohos.permission.INSTALL_BUNDLE",
    "ohos.permission.ACCESS_BUNDLE"
  ],
  "timeout": 300000,
  "hasSubcommands": true,
  "subcommands": {
    "install": {
      "description": "Install a package",
      "input": {
        "type": "object",
        "properties": {
          "package": {"type": "string"},
          "force": {"type": "boolean"}
        },
        "required": ["package"]
      },
      "output": {
        "type": "object",
        "properties": {
          "success": {"type": "boolean"},
          "packagePath": {"type": "string"}
        }
      },
      "events": ["progress"],
      "eventSchemas": {
        "progress": {
          "type": "object",
          "description": "安装进度事件",
          "properties": {
            "stage": {
              "type": "string",
              "enum": ["downloading", "installing", "configuring", "completed"],
              "description": "当前阶段"
            },
            "percentage": {
              "type": "integer",
              "minimum": 0,
              "maximum": 100,
              "description": "完成百分比"
            }
          },
          "required": ["stage", "percentage"]
        }
      }
    }
  }
}
```

## 错误处理

### 无效 JSON

如果工具输出的 JSON 格式错误，框架会生成 `error` 事件：

```javascript
{type: 'error', data: 'Invalid JSON output on line 5: Unexpected token...'}
```

### Schema 验证失败

如果事件数据不符合 Schema 定义，框架会生成 `error` 事件：

```javascript
{type: 'error', data: 'Progress event validation failed: missing required field "percentage"'}
```

### 未知事件类型

如果工具输出的 `event` 类型不在 `events` 数组中声明，框架会生成 `error` 事件：

```javascript
{type: 'error', data: 'Unknown event type "unknown_event"'}
```

## 最佳实践

### 1. 事件命名规范

- 使用小写字母和连字符：`download-progress`、`file-parse`
- 使用动词或名词：`scanning`、`download`、`result`
- 避免使用预定义事件名：`stdout`、`stderr`、`exit`、`error`

### 2. Schema 设计建议

- **必需字段**：使用 `required` 明确关键字段
- **枚举值**：使用 `enum` 限制状态值范围
- **类型约束**：使用 `minimum`、`maximum`、`format` 等约束
- **描述文档**：为每个字段添加 `description`

### 3. 性能考虑

- **控制事件频率**：避免每秒发送大量事件，可以节流或批量处理
- **保持数据精简**：只传输必要的数据，避免在大对象中嵌套过多层级
- **及时刷新缓冲**：每次输出事件后调用 `fflush(stdout)` 确保立即发送

### 4. 兼容性处理

- **渐进式事件**：即使工具不发送进度事件，也能正常工作
- **非事件输出**：允许工具输出普通文本，框架会自动转换为 `stdout` 事件
- **错误处理**：始终监听 `error` 事件，处理异常情况

### 5. 调试建议

- **日志记录**：工具可以输出调试信息到 stderr，不影响事件流
- **事件验证**：开发阶段可以使用 JSON Schema 验证工具测试事件格式
- **单元测试**：编写测试用例验证各种事件场景

## TypeScript 类型定义

```typescript
/**
 * 事件类型
 */
type CLIEventType = 'stdout' | 'stderr' | 'exit' | 'error' | string;

/**
 * 事件数据
 */
interface CLIEvent<T = any> {
  type: CLIEventType;
  data: T;
  timestamp?: number;
}

/**
 * 工具描述中的事件配置
 */
interface ToolEventsConfig {
  /**
   * 支持的事件类型
   * @example ["progress", "result"]
   */
  eventTypes: string[];

  /**
   * 事件 Schema 定义
   * key 为事件类型，value 为事件数据格式
   * @example { "progress": { type: "object", properties: {...} } }
   */
  eventSchemas: Record<string, JSONSchema>;
}

/**
 * 执行选项
 */
interface ExecOptions {
  /**
   * 事件回调
   */
  onEvent?: (event: CLIEvent) => void;

  /**
   * 超时时间（毫秒）
   */
  timeout?: number;
}
```

## 总结

**eventSchemas** 提供了一种强大而灵活的事件机制：

1. **声明式**：在工具描述中定义事件格式，调用方可以静态分析
2. **类型安全**：JSON Schema 确保事件数据的结构一致性
3. **实时反馈**：支持进度报告、状态更新等中间状态传递
4. **易于调试**：所有事件都通过 stdout 输出，便于日志记录和调试
5. **向后兼容**：工具可以不发送任何事件，仅返回最终结果

通过合理使用 eventSchemas，可以显著提升 CLI 工具的用户体验，特别是在长时间运行的任务中。
