# CLI Manager 使用示例

本文档提供 CLI Manager 的使用示例。

## 导入模块

```typescript
import CLI from '@ohos.cli.manager';
```

## 示例 1: 同步调用工具

```typescript
import CLI from '@ohos.cli.manager';

async function scanNetwork(targetIp: string) {
  try {
    const result = await new Promise((resolve, reject) => {
      CLI.CLIManager.execTool('network-scanner',
        { target: targetIp, ports: [80, 443, 8080] },
        {
          options: { timeout: 30000, mode: 'sync' },
          callback: (error, result) => {
            if (error) {
              reject(error);
            } else {
              resolve(result);
            }
          }
        }
      );
    });

    if (result.exitCode === 0) {
      console.log('Scan result:', result.stdout);
    } else {
      console.error('Error:', result.stderr);
    }
  } catch (error) {
    console.error('Failed to execute tool:', error);
  }
}
```

## 示例 2: 异步调用工具

```typescript
import CLI from '@ohos.cli.manager';

async function scanNetworkAsync(targetIp: string) {
  try {
    const taskId = await CLI.CLIManager.execToolAsync(
      'network-scanner',
      { target: targetIp },
      (event) => {
        if (event.type === 'stdout') {
          console.log('Output:', event.data);
        } else if (event.type === 'stderr') {
          console.error('Error:', event.data);
        } else if (event.type === 'exit') {
          console.log('Exited with code:', event.exitCode);
        } else if (event.type === 'error') {
          console.error('Tool error:', event.data);
        }
      },
      { timeout: 60000, mode: 'async' }
    );

    console.log('Task started:', taskId);

    // 如果需要取消
    // const cancelled = CLI.CLIManager.cancelTask(taskId);
    // console.log('Cancelled:', cancelled);
  } catch (error) {
    console.error('Failed to start task:', error);
  }
}
```

## 示例 3: 查询可用工具

```typescript
import CLI from '@ohos.cli.manager';

async function listAvailableTools() {
  try {
    const tools = await new Promise((resolve, reject) => {
      CLI.CLIManager.queryTools((error, tools) => {
        if (error) {
          reject(error);
        } else {
          resolve(tools);
        }
      });
    });

    tools.forEach(tool => {
      console.log(`${tool.name} (${tool.type}): ${tool.description}`);
      console.log(`  Version: ${tool.version}`);
      console.log(`  Permissions: ${tool.permissions.join(', ')}`);
      console.log(`  Timeout: ${tool.timeout}ms`);
    });
  } catch (error) {
    console.error('Failed to query tools:', error);
  }
}
```

## 示例 4: 查询特定工具

```typescript
import CLI from '@ohos.cli.manager';

async function getToolInfo(toolName: string) {
  try {
    const tool = await new Promise((resolve, reject) => {
      CLI.CLIManager.queryToolByName(toolName, (error, tool) => {
        if (error) {
          reject(error);
        } else {
          resolve(tool);
        }
      });
    });

    if (tool) {
      console.log('Tool found:', tool.name);
      console.log('Description:', tool.description);
      console.log('Type:', tool.type);
    } else {
      console.log('Tool not found');
    }
  } catch (error) {
    console.error('Failed to query tool:', error);
  }
}
```

## 示例 5: 查询任务状态

```typescript
import CLI from '@ohos.cli.manager';

async function checkTaskStatus(taskId: string) {
  try {
    const status = CLI.CLIManager.queryTaskStatus(taskId);
    console.log('Task:', status.taskId);
    console.log('Tool:', status.toolName);
    console.log('Status:', status.status);
    console.log('Start time:', new Date(status.startTime).toLocaleString());

    if (status.endTime) {
      console.log('End time:', new Date(status.endTime).toLocaleString());
    }

    if (status.exitCode !== undefined) {
      console.log('Exit code:', status.exitCode);
    }
  } catch (error) {
    console.error('Failed to query task status:', error);
  }
}
```

## 示例 6: 长时运行工具

```typescript
import CLI from '@ohos.cli.manager';

async function startLongRunningTool() {
  try {
    const taskId = await CLI.CLIManager.execToolAsync(
      'log-monitor',
      { level: 'verbose', path: '/var/log/syslog' },
      (event) => {
        if (event.type === 'stdout') {
          console.log('Log:', event.data);
        } else if (event.type === 'exit') {
          console.log('Monitor exited with code:', event.exitCode);
        }
      },
      { timeout: 0 }  // 无超时限制
    );

    console.log('Log monitor started, taskId:', taskId);

    // 定期检查状态
    const checkInterval = setInterval(() => {
      try {
        const status = CLI.CLIManager.queryTaskStatus(taskId);
        console.log('Status:', status.status);

        if (status.status === 'completed' || status.status === 'failed') {
          clearInterval(checkInterval);
        }
      } catch (error) {
        console.error('Status check failed:', error);
        clearInterval(checkInterval);
      }
    }, 5000);

  } catch (error) {
    console.error('Failed to start monitor:', error);
  }
}
```

## 示例 7: 按权限查询工具

```typescript
import CLI from '@ohos.cli.manager';

async function findToolsRequiringPermission(permission: string) {
  try {
    const tools = await new Promise((resolve, reject) => {
      CLI.CLIManager.queryToolsByPermission(permission, (error, tools) => {
        if (error) {
          reject(error);
        } else {
          resolve(tools);
        }
      });
    });

    console.log(`Tools requiring ${permission}:`);
    tools.forEach(tool => {
      console.log(`  - ${tool.name}: ${tool.description}`);
    });
  } catch (error) {
    console.error('Failed to query tools:', error);
  }
}

// 使用示例
findToolsRequiringPermission('ohos.permission.INTERNET');
```

## 错误处理

所有异步操作都可能抛出错误，建议使用 try-catch 进行错误处理：

```typescript
import CLI from '@ohos.cli.manager';

async function safeToolExecution() {
  try {
    const result = await new Promise((resolve, reject) => {
      CLI.CLIManager.execTool('some-tool',
        { arg1: 'value1' },
        {
          options: { timeout: 30000 },
          callback: (error, result) => {
            if (error) {
              reject(error);
            } else {
              resolve(result);
            }
          }
        }
      );
    });
    console.log('Success:', result);
  } catch (error) {
    if (error.code === 1) {
      console.error('Tool not found');
    } else if (error.code === 2) {
      console.error('Permission denied');
    } else if (error.code === 4) {
      console.error('Execution timeout');
    } else {
      console.error('Unknown error:', error.message);
    }
  }
}
```

## 错误码

| 错误码 | 常量 | 描述 |
|--------|------|------|
| 0 | ERR_OK | 成功 |
| 1 | ERR_NOT_FOUND | 工具不存在 |
| 2 | ERR_PERMISSION_DENIED | 权限不足 |
| 3 | ERR_INVALID_ARGS | 参数无效 |
| 4 | ERR_TIMEOUT | 执行超时 |
| 5 | ERR_PROCESS_CRASH | 进程崩溃 |
| 6 | ERR_SERVICE_NOT_READY | 服务未就绪 |
| 7 | ERR_TOO_MANY_TASKS | 任务数过多 |
| 8 | ERR_INVALID_SCHEMA | 描述文件格式错误 |
