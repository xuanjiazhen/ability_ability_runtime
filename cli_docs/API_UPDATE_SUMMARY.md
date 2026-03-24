# CLI 工具管理框架 - API 设计更新

## 更新日期
2025-03-26

## 更新概述

根据用户反馈，对 CLI 工具管理框架的 API 设计进行了精简和优化：

1. **精简查询接口**：移除 `queryToolByName` 和 `queryToolsByPermission`，只保留 `queryTools`
2. **统一执行接口**：将 `execTool` 和 `execToolAsync` 合并为统一的 `execTool` 接口
3. **引入 Session 概念**：采用类似 OpenAI Computer Use 的 `exec` 工具设计，使用 `background` 和 `yieldMs` 参数控制执行行为

---

## 设计变更

### 1. JS API 变更

#### 移除的接口
- ❌ `queryToolByName(name: string)` - 调用方可在获取所有工具后自行过滤
- ❌ `queryToolsByPermission(permission: string)` - 调用方可在获取所有工具后自行过滤
- ❌ `execToolAsync()` - 已合并到 `execTool()`

#### 新的接口设计

```typescript
interface ExecOptions {
  /**
   * 是否后台执行
   * - false: 前台执行，在 yieldMs 内等待结果返回，超时后自动转入后台
   * - true: 立即进入后台执行，马上返回 sessionId
   */
  background?: boolean;

  /**
   * 前台等待超时时间（毫秒）
   * 仅当 background=false 时有效
   * 默认值: 30000 (30秒)
   */
  yieldMs?: number;

  /**
   * 总超时时间（毫秒）
   * 进程运行超过此时间会被强制终止
   */
  timeout?: number;

  env?: Record<string, string>;
  cwd?: string;
}

interface SessionInfo {
  sessionId: string;
  toolName: string;
  status: 'running' | 'completed' | 'failed';
  startTime: number;
  endTime?: number;
  result?: ExecResult;  // 仅 status='completed' 时有值
}

class CLIManager {
  // 查询所有工具（保留）
  static queryTools(): Promise<ToolInfo[]>;

  // 统一的执行接口
  static execTool(
    name: string,
    args: Record<string, any>,
    options?: ExecOptions
  ): Promise<SessionInfo>;

  // 订阅后台会话事件
  static subscribeSession(
    sessionId: string,
    callback: (event: ToolEvent) => void
  ): Promise<boolean>;

  // 取消后台会话
  static cancelSession(sessionId: string): Promise<boolean>;

  // 查询会话状态
  static querySessionStatus(sessionId: string): Promise<SessionStatus>;
}
```

### 2. 执行行为说明

#### 场景 1: 前台执行，快速完成
```typescript
const session = await CLI.CLIManager.execTool('network-scanner',
  { target: '192.168.1.1' },
  { background: false, yieldMs: 30000 }
);

// session.status = 'completed'
// session.result 有值
if (session.status === 'completed' && session.result) {
  console.log(session.result.stdout);
}
```

#### 场景 2: 前台执行，超时自动转后台
```typescript
const session = await CLI.CLIManager.execTool('log-monitor',
  { filter: 'ERROR' },
  { background: false, yieldMs: 30000 }
);

// session.status = 'running'（超时转入后台）
// session.result 为 undefined
CLI.CLIManager.subscribeSession(session.sessionId, callback);
```

#### 场景 3: 立即后台执行
```typescript
const session = await CLI.CLIManager.execTool('log-monitor',
  { filter: 'ERROR' },
  { background: true }
);

// session.status = 'running'
// session.result 为 undefined
CLI.CLIManager.subscribeSession(session.sessionId, callback);
```

---

## 代码文件更新清单

### 文档文件
- ✅ `cli_docs/DESIGN.md`
  - 更新第 5.1 节 JS API 定义
  - 更新第 5.2 节 Native C++ API 定义
  - 更新第 5.3 节 IPC 接口定义
  - 更新使用示例

### 头文件
- ✅ `interfaces/inner_api/cli_manager/cli_manager_service_interface.h`
  - 移除 `QUERY_TOOL_BY_NAME` 消息 ID
  - 移除 `QueryToolByName()` 接口
  - 移除 `EXEC_TOOL_ASYNC` 消息 ID
  - 移除 `ExecToolAsync()` 接口
  - 添加 `SessionInfo` 结构体
  - 更新 `ExecTool()` 为统一接口
  - 添加 `SubscribeSession()` 接口
  - 更新 `CancelTask()` → `CancelSession()`
  - 更新 `QueryTaskStatus()` → `QuerySessionStatus()`

- ✅ `interfaces/inner_api/cli_manager/cli_execution_context.h`
  - 更新 `ExecOptions` 结构体，添加 `background` 和 `yieldMs` 字段
  - 添加 `SessionInfo` 结构体
  - 标记 `ExecutionMode` 为已废弃

- ✅ `frameworks/js/napi/cli_manager/js_cli_manager.h`
  - 移除 `QueryToolByName()` 方法
  - 移除 `QueryToolsByPermission()` 方法
  - 移除 `ExecToolAsync()` 方法
  - 更新 `ExecTool()` 为统一接口
  - 添加 `SubscribeSession()` 方法
  - 更新 `CancelTask()` → `CancelSession()`
  - 更新 `QueryTaskStatus()` → `QuerySessionStatus()`
  - 更新 `AsyncContext` 结构体
  - 添加 `CreateSessionInfoObject()` 方法

---

## 待实现的代码文件

以下实现文件需要根据头文件更新进行相应的实现更新：

### CPP 实现文件
- ⏳ `interfaces/inner_api/cli_manager/cli_manager_proxy.cpp`
  - 更新 `ExecTool()` 实现，支持 background 和 yieldMs 参数
  - 实现 `SubscribeSession()` 方法
  - 更新 `CancelTask()` → `CancelSession()`
  - 更新 `QueryTaskStatus()` → `QuerySessionStatus()`

- ⏳ `frameworks/js/napi/cli_manager/js_cli_manager.cpp`
  - 更新 `ExecTool()` 实现，解析 background 和 yieldMs 参数
  - 实现 `SubscribeSession()` 方法
  - 更新 `CancelTask()` → `CancelSession()`
  - 更新 `QueryTaskStatus()` → `QuerySessionStatus()`
  - 实现 `CreateSessionInfoObject()` 方法

- ⏳ `services/climgr/src/cli_manager_service.cpp`
  - 移除 `QueryToolByName()` 实现
  - 合并 `ExecTool()` 和 `ExecToolAsync()` 实现
  - 实现 `SubscribeSession()` 方法
  - 更新 `CancelTask()` → `CancelSession()`
  - 更新 `QueryTaskStatus()` → `QuerySessionStatus()`

---

## 迁移指南

### 对于调用方

#### 旧代码
```typescript
// 查询单个工具
const tool = await CLI.CLIManager.queryToolByName('network-scanner');

// 同步调用
const result = await CLI.CLIManager.execTool('network-scanner', args);

// 异步调用
const taskId = await CLI.CLIManager.execToolAsync('log-monitor', args, callback);
await CLI.CLIManager.cancelTask(taskId);
```

#### 新代码
```typescript
// 查询单个工具 - 在客户端过滤
const tools = await CLI.CLIManager.queryTools();
const tool = tools.find(t => t.name === 'network-scanner');

// 前台执行（快速完成）
const session = await CLI.CLIManager.execTool('network-scanner', args, {
  background: false,
  yieldMs: 30000
});
if (session.status === 'completed' && session.result) {
  console.log(session.result.stdout);
}

// 后台执行
const session = await CLI.CLIManager.execTool('log-monitor', args, {
  background: true
});
CLI.CLIManager.subscribeSession(session.sessionId, callback);
await CLI.CLIManager.cancelSession(session.sessionId);
```

---

## 优势

1. **更简洁的 API**：减少了接口数量，降低学习成本
2. **更灵活的执行控制**：通过 `background` 和 `yieldMs` 参数可以精确控制执行行为
3. **统一的返回类型**：始终返回 `SessionInfo`，调用方只需检查 `status` 和 `result` 字段
4. **更好的用户体验**：类似 OpenAI Computer Use 的设计，对用户更友好
5. **向后兼容**：保留了旧的 `TaskStatus` 和 `ExecutionMode` 定义（标记为废弃）

---

## 下一步行动

1. ⏳ 更新 CPP 实现文件（proxy、stub、service）
2. ⏳ 更新 NAPI 绑定实现
3. ⏳ 更新单元测试用例
4. ⏳ 更新可视化流程图
5. ⏳ 更新 COMMUNICATION_DESIGN.md（如果需要）

---

**更新状态**: 设计文档和头文件已更新完成，等待用户批准后继续更新实现文件。
